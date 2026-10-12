#include "old_game/economy/inventory/packet_dispatcher_cargo_transport_item.h"

#include <stdint.h>
#include <map>
#include <string.h>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/config.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/cargo_transport_stone_script.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_data/db/inventory/db_save_account_cargo.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/equipment_growth_runtime.h"
#include "old_game/economy/equipment/item_growth_state.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/checksum.h"


namespace {

const INVEN_TYPE kCargoTransportInventory = static_cast<INVEN_TYPE>(1);
const eItemDelReason kCargoTransportMaterialDeleteReason =
    static_cast<eItemDelReason>(3);
const eItemDelReason kCargoTransportEquipmentDeleteReason =
    static_cast<eItemDelReason>(37);
const StackableAction::Type kCargoTransportStoneAction =
    static_cast<StackableAction::Type>(59);

const uint8_t kErrorNoAccountCargo = 17;
const uint8_t kErrorMissingItem = 19;
const uint8_t kErrorAccountCargoFull = 4;
const uint8_t kErrorMaterialsInsufficient = 22;
const uint8_t kErrorNotTransportable = 7;
const uint8_t kErrorItemLocked = 213;

enum {
    kCargoWaitingReset = 1,
    kCargoWaitingCompensation = 2,
    kCargoBlocked = 3,
    kCargoWaitingPersistedProof = 4,
    kCargoApplyingReset = 5,
};

enum CargoAckClaim {
    kCargoAckUnknown = 0,
    kCargoAckStaleIdentity,
    kCargoAckMessageMismatch,
    kCargoAckResetFailed,
    kCargoAckFailedRetained,
    kCargoAckApplyReset,
    kCargoAckCompensated,
    kCargoAckFinalized,
    kCargoAckOutOfOrder,
};

struct PendingCargoTransport {
    PendingCargoTransport()
        : correlation(0u),
          user_idx(-1),
          account_id(0),
          charac_no(0),
          server_group(0),
          phase(0u),
          material_slot(0u),
          equipment_slot(0u),
          required_count(0),
          remaining_count(0),
          account_cargo_slot(0),
          material_snapshot(),
          equipment_snapshot(),
          account_cargo_snapshot(),
          previous_state(),
          error(kErrorNotTransportable),
          error_sent(false) {
        memset(guid, 0, sizeof(guid));
    }

    uint32_t correlation;
    int32_t user_idx;
    int32_t account_id;
    int32_t charac_no;
    int32_t server_group;
    uint8_t phase;
    uint16_t material_slot;
    uint16_t equipment_slot;
    int required_count;
    int remaining_count;
    int account_cargo_slot;
    Inven_Item material_snapshot;
    Inven_Item equipment_snapshot;
    Inven_Item account_cargo_snapshot;
    uint8_t guid[item_growth_state::kGuidBytes];
    item_growth_state::State previous_state;
    uint8_t error;
    bool error_sent;
};

typedef std::map<int, PendingCargoTransport> PendingCargoMap;

PendingCargoMap& PendingCargoTransports() {
    static PendingCargoMap values;
    return values;
}

Mutex& PendingCargoMutex() {
    static Mutex mutex;
    return mutex;
}

bool HasPendingCargo(int user_idx) {
    MutexGuard guard(&PendingCargoMutex());
    return PendingCargoTransports().find(user_idx) !=
           PendingCargoTransports().end();
}

bool AddPendingCargo(int user_idx, const PendingCargoTransport& pending) {
    MutexGuard guard(&PendingCargoMutex());
    return PendingCargoTransports().insert(
               std::make_pair(user_idx, pending)).second;
}

bool PreparePendingCargoCompensation(int user_idx,
                                     PendingCargoTransport* pending) {
    MutexGuard guard(&PendingCargoMutex());
    PendingCargoMap::iterator found = PendingCargoTransports().find(user_idx);
    if (found == PendingCargoTransports().end()) return false;
    if (found->second.phase == kCargoWaitingCompensation ||
        found->second.phase == kCargoBlocked) {
        if (pending) *pending = found->second;
        return false;
    }
    found->second.phase = kCargoWaitingCompensation;
    found->second.error = kErrorNotTransportable;
    found->second.error_sent = false;
    if (pending) *pending = found->second;
    return true;
}

bool QueueCargoCompensation(const PendingCargoTransport& pending) {
    if (pending.user_idx < 0 || pending.server_group < 0 ||
        !item_growth_state::IsValidGuid(pending.guid)) {
        return false;
    }
    itemGloballyUniqueIdentifier_t guid;
    memcpy(&guid, pending.guid, sizeof(guid));
    return item_growth_db::DB_ItemGrowthState::QueueSave(
        pending.user_idx,
        pending.server_group,
        guid,
        pending.previous_state,
        item_growth_db::kWriteCargoCompensate,
        pending.correlation);
}

bool PendingIdentityMatches(CUser* user,
                            const PendingCargoTransport& pending) {
    return user && user->GetUID() == pending.user_idx &&
           user->get_acc_id() == pending.account_id &&
           user->getCurCharacNo() == pending.charac_no &&
           user->GetServerGroup() == pending.server_group;
}

CargoAckClaim ClaimPendingCargoAck(
    CUser* user,
    const item_growth_db::Message& message,
    PendingCargoTransport* pending) {
    if (!user) return kCargoAckUnknown;
    const int user_idx = user->GetUID();
    MutexGuard guard(&PendingCargoMutex());
    PendingCargoMap::iterator found = PendingCargoTransports().find(user_idx);
    if (found == PendingCargoTransports().end() ||
        found->second.correlation != message.correlation) {
        return kCargoAckUnknown;
    }
    PendingCargoTransport& value = found->second;
    if (pending) *pending = value;
    if (!PendingIdentityMatches(user, value)) {
        value.phase = kCargoBlocked;
        value.error = kErrorNotTransportable;
        value.error_sent = false;
        return kCargoAckStaleIdentity;
    }
    if (message.owner_user_idx != value.user_idx ||
        message.server_group != value.server_group ||
        memcmp(message.item_guid, value.guid, sizeof(value.guid)) != 0) {
        value.phase = kCargoBlocked;
        value.error = kErrorNotTransportable;
        value.error_sent = false;
        return kCargoAckMessageMismatch;
    }
    if (message.result != item_growth_db::kResultOk) {
        if (value.phase == kCargoWaitingReset &&
            message.write_mode == item_growth_db::kWriteCargoReset) {
            if (pending) *pending = value;
            PendingCargoTransports().erase(found);
            return kCargoAckResetFailed;
        }
        value.phase = kCargoBlocked;
        value.error_sent = false;
        if (pending) *pending = value;
        return kCargoAckFailedRetained;
    }
    if (value.phase == kCargoWaitingReset &&
        message.write_mode == item_growth_db::kWriteCargoReset) {
        value.phase = kCargoApplyingReset;
        if (pending) *pending = value;
        return kCargoAckApplyReset;
    }
    if (value.phase == kCargoWaitingCompensation &&
        message.write_mode == item_growth_db::kWriteCargoCompensate) {
        if (pending) *pending = value;
        PendingCargoTransports().erase(found);
        return kCargoAckCompensated;
    }
    if (value.phase == kCargoWaitingPersistedProof &&
        message.write_mode == item_growth_db::kWriteCargoFinalize) {
        if (pending) *pending = value;
        PendingCargoTransports().erase(found);
        return kCargoAckFinalized;
    }
    value.phase = kCargoBlocked;
    value.error = kErrorNotTransportable;
    value.error_sent = false;
    if (pending) *pending = value;
    return kCargoAckOutOfOrder;
}

void ErasePendingCargo(int user_idx, uint32_t correlation) {
    MutexGuard guard(&PendingCargoMutex());
    PendingCargoMap::iterator found = PendingCargoTransports().find(user_idx);
    if (found != PendingCargoTransports().end() &&
        found->second.correlation == correlation) {
        PendingCargoTransports().erase(found);
    }
}

void SetPendingCargoPhase(int user_idx,
                          uint32_t correlation,
                          uint8_t phase,
                          uint8_t error,
                          bool error_sent) {
    MutexGuard guard(&PendingCargoMutex());
    PendingCargoMap::iterator found = PendingCargoTransports().find(user_idx);
    if (found != PendingCargoTransports().end() &&
        found->second.correlation == correlation) {
        found->second.phase = phase;
        found->second.error = error;
        found->second.error_sent = error_sent;
    }
}

bool QueueCargoState(CUser* user,
                     const PendingCargoTransport& pending,
                     const item_growth_state::State& state,
                     uint8_t write_mode,
                     const item_growth_db::SagaProof* proof = 0) {
    if (!user ||
        !item_growth_state::IsValidGuid(pending.guid)) {
        return false;
    }
    itemGloballyUniqueIdentifier_t guid;
    memcpy(&guid, pending.guid, sizeof(guid));
    return item_growth_db::DB_ItemGrowthState::QueueSave(
        user->GetUID(),
        user->GetServerGroup(),
        guid,
        state,
        write_mode,
        pending.correlation,
        proof);
}

bool HashMainInventory(const CInventory* inventory, uint8_t digest[32]) {
    if (!inventory || !digest) return false;
    std::vector<unsigned char> raw(
        static_cast<size_t>(oldsrv::config::kMainInventoryItemCount) *
        sizeof(Inven_Item));
    for (int slot = 0; slot < oldsrv::config::kMainInventoryItemCount;
         ++slot) {
        const Inven_Item* const item = inventory->GetInvenRef(1, slot);
        if (!item) return false;
        memcpy(&raw[static_cast<size_t>(slot) * sizeof(Inven_Item)],
               item,
               sizeof(Inven_Item));
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

bool HashAccountCargo(CAccountCargo* cargo,
                      int replacement_slot,
                      const Inven_Item* replacement,
                      uint8_t digest[32]) {
    if (!cargo || !digest || replacement_slot < -1 || replacement_slot >= 56)
        return false;
    std::vector<unsigned char> raw(56u * sizeof(Inven_Item), 0u);
    for (int slot = 0; slot < 56; ++slot) {
        Inven_Item item = cargo->GetSlot(slot);
        if (slot == replacement_slot && replacement) item = *replacement;
        memcpy(&raw[static_cast<size_t>(slot) * sizeof(Inven_Item)],
               &item,
               sizeof(item));
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

void SendCargoTransportError(CUser* user, uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CARGO_TRANSPORT_ITEM, error);
    }
}

void RestoreInventorySnapshots(CInventory* inventory,
                               int material_slot,
                               const Inven_Item& material,
                               bool restore_material,
                               int equipment_slot,
                               const Inven_Item& equipment,
                               bool restore_equipment) {
    if (!inventory) return;
    if (restore_equipment) {
        inventory->update_item(kCargoTransportInventory,
                               equipment_slot,
                               equipment);
    }
    if (restore_material) {
        inventory->update_item(kCargoTransportInventory,
                               material_slot,
                               material);
    }
}

bool IsCargoTransportTradeStateEligible(const CItem& prototype,
                                        const Inven_Item& item) {
    const int attach_type = prototype.GetAttachType();
    // CLIENT_DSTR_EVIDENCE: 35294 is "tradable items cannot be transported"
    // and 35295 is "account-bound items cannot be transported".  This stone
    // intentionally bypasses the ordinary account-cargo rule for character-
    // bound equipment, so the instance trade-restriction bit is not itself a
    // rejection; it can make an otherwise free/sealed item non-tradable.
    if (attach_type == 5) return false;
    if (item.upgrade_separate_.IsTradeRestriction()) return true;
    if (attach_type == 0) return false;
    if ((attach_type == 3 || attach_type == 4) &&
        item.seal_or_state_ == 1) {
        return false;
    }
    if (attach_type == 7 && item.GetTradeLimitCount() > 0) return false;
    // 0828 does not stamp a lifetime "already transported" bit.  The client
    // re-evaluates only this current trade state plus CargoTransportStone.etc;
    // withdrawing the unchanged record can therefore make it eligible for a
    // later, separately paid CMD551 again.
    return true;
}

}  // namespace


Dispatcher_CargoTransportItem::Dispatcher_CargoTransportItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_CargoTransportItem::~Dispatcher_CargoTransportItem() {}

int Dispatcher_CargoTransportItem::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    uint16_t material_slot = 0;
    uint16_t equipment_slot = 0;
    if (packet.get_short(material_slot) != 1 ||
        packet.get_short(equipment_slot) != 1) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }
    if (user && HasPendingCargo(user->GetUID())) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: sub_6421F0 emits exactly two u16 ordinary-
    // inventory slots.  The material's `[seria room]` usable-place rule is
    // checked below through the real CStackableItem owner.
    if (!user || !user->getCurCharacR() || user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }
    if (!user->IsExistAccountCargo()) {
        SendCargoTransportError(user, kErrorNoAccountCargo);
        return 0;
    }
    if (material_slot == equipment_slot) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }

    const CInventory* const readable_inventory = user->getCurCharacInvenR();
    if (!readable_inventory ||
        readable_inventory->CheckValidSlot(
            static_cast<int>(kCargoTransportInventory),
            static_cast<int>(material_slot)) != 1 ||
        readable_inventory->CheckValidSlot(
            static_cast<int>(kCargoTransportInventory),
            static_cast<int>(equipment_slot)) != 1) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }

    const Inven_Item* const material_ref = readable_inventory->GetInvenRef(
        kCargoTransportInventory, static_cast<int>(material_slot));
    const Inven_Item* const equipment_ref = readable_inventory->GetInvenRef(
        kCargoTransportInventory, static_cast<int>(equipment_slot));
    if (!material_ref || material_ref->isEmpty() || !equipment_ref ||
        equipment_ref->isEmpty()) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }
    if (user->CheckItemLock(kCargoTransportInventory,
                            static_cast<int>(material_slot)) ||
        user->CheckItemLock(kCargoTransportInventory,
                            static_cast<int>(equipment_slot))) {
        SendCargoTransportError(user, kErrorItemLocked);
        return 0;
    }

    CItem* const material_item =
        G_CDataManager()->find_item(material_ref->item_id_);
    if (!material_item || material_item->is_stackable() != 1) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }
    CStackableItem* const stone =
        static_cast<CStackableItem*>(material_item);
    const std::vector<int>& action_values = stone->action_values();
    if (!stone->verifyStackableAction(kCargoTransportStoneAction) ||
        action_values.size() != 1u || stone->verifyUsablePlace(user) != 1) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }
    const cargo_transport::Rule* const rule =
        cargo_transport::FindRule(action_values[0]);
    if (!rule) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }

    CItem* const equipment_item =
        G_CDataManager()->find_item(equipment_ref->item_id_);
    if (!equipment_item || equipment_item->is_stackable() != 0 ||
        equipment_ref->item_category_ != 1 ||
        !equipment_ref->isEquipableItemType() ||
        !IsCargoTransportTradeStateEligible(*equipment_item,
                                            *equipment_ref)) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }
    CEquipItem* const equipment = static_cast<CEquipItem*>(equipment_item);
    const int required_count = cargo_transport::GetRequiredMaterialCount(
        *rule, *equipment, equipment_ref->item_id_);
    if (required_count <= 0) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }

    const int material_count = material_ref->get_add_info();
    if (material_count < required_count) {
        SendCargoTransportError(user, kErrorMaterialsInsufficient);
        return 0;
    }
    const int remaining_count = material_count - required_count;
    if (remaining_count < 0 || remaining_count > 0xffff) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }

    CAccountCargo* const account_cargo = user->GetAccountCargo();
    const int account_cargo_slot = account_cargo->GetEmptySlot();
    if (account_cargo_slot < 0 || account_cargo_slot > 0xffff ||
        !account_cargo->CheckSlotEmpty(account_cargo_slot)) {
        SendCargoTransportError(user, kErrorAccountCargoFull);
        return 0;
    }

    // Preflight both ordinary-inventory mutations against an isolated copy.
    // The account-cargo insertion has no extra condition after the chosen
    // destination is proven valid and empty for this equipment record.
    CInventory shadow_inventory;
    InventoryMemory shadow_memory;
    shadow_inventory.SetInventoryMemory(&shadow_memory);
    shadow_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (shadow_inventory.delete_item(
            kCargoTransportInventory,
            static_cast<int>(material_slot),
            required_count,
            kCargoTransportMaterialDeleteReason,
            false) != 1 ||
        shadow_inventory.delete_item(
            kCargoTransportInventory,
            static_cast<int>(equipment_slot),
            1,
            kCargoTransportEquipmentDeleteReason,
            false) != 1) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }

    const Inven_Item material_snapshot = *material_ref;
    const Inven_Item equipment_snapshot = *equipment_ref;
    const Inven_Item account_cargo_snapshot =
        account_cargo->GetSlot(account_cargo_slot);

    if (cargo_transport::IsEquipmentGrowthPrototype(
            equipment_snapshot.item_id_)) {
        if (!item_growth_state::IsValidGuid(
                &equipment_snapshot.globally_unique_id_)) {
            SendCargoTransportError(user, kErrorNotTransportable);
            return 0;
        }
        PendingCargoTransport pending;
        pending.correlation = item_growth_db::NextCorrelation();
        pending.user_idx = user->GetUID();
        pending.account_id = user->get_acc_id();
        pending.charac_no = user->getCurCharacNo();
        pending.server_group = user->GetServerGroup();
        pending.phase = kCargoWaitingReset;
        pending.material_slot = material_slot;
        pending.equipment_slot = equipment_slot;
        pending.required_count = required_count;
        pending.remaining_count = remaining_count;
        pending.account_cargo_slot = account_cargo_slot;
        pending.material_snapshot = material_snapshot;
        pending.equipment_snapshot = equipment_snapshot;
        pending.account_cargo_snapshot = account_cargo_snapshot;
        memcpy(pending.guid,
               &equipment_snapshot.globally_unique_id_,
               sizeof(pending.guid));
        if (!item_growth_state::Lookup(
                equipment_snapshot.globally_unique_id_,
                &pending.previous_state)) {
            item_growth_state::Install(
                equipment_snapshot.globally_unique_id_,
                pending.previous_state);
        }
        CInventory shadow;
        InventoryMemory shadow_memory;
        shadow.SetInventoryMemory(&shadow_memory);
        item_growth_db::SagaProof proof;
        memset(&proof, 0, sizeof(proof));
        if (!shadow.setCopy(*readable_inventory) ||
            shadow.delete_item(kCargoTransportInventory,
                               static_cast<int>(material_slot),
                               required_count,
                               kCargoTransportMaterialDeleteReason,
                               false) != 1 ||
            shadow.delete_item(kCargoTransportInventory,
                               static_cast<int>(equipment_slot),
                               1,
                               kCargoTransportEquipmentDeleteReason,
                               false) != 1 ||
            !HashMainInventory(readable_inventory,
                               proof.pre_inventory_sha256) ||
            !HashMainInventory(&shadow, proof.post_inventory_sha256) ||
            !HashAccountCargo(account_cargo,
                              -1,
                              0,
                              proof.pre_account_cargo_sha256) ||
            !HashAccountCargo(account_cargo,
                              account_cargo_slot,
                              &equipment_snapshot,
                              proof.post_account_cargo_sha256)) {
            SendCargoTransportError(user, kErrorNotTransportable);
            return 0;
        }
        proof.saga_owner = item_growth_db::kSagaOwnerCargo;
        proof.source_space = item_growth_db::kSagaSpaceInventory;
        proof.target_space = item_growth_db::kSagaSpaceAccountCargo;
        proof.proof_flags = item_growth_db::kSagaProofInventory |
                            item_growth_db::kSagaProofAccountCargo;
        proof.source_slot = equipment_slot;
        proof.target_slot = static_cast<uint16_t>(account_cargo_slot);
        proof.owner_user_idx = user->GetUID();
        proof.account_id = static_cast<uint32_t>(user->get_acc_id());
        proof.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        proof.source_item_id =
            static_cast<uint32_t>(equipment_snapshot.item_id_);
        proof.target_item_id = proof.source_item_id;
        proof.baseline_progress = pending.previous_state.progress;
        proof.baseline_fatigue = pending.previous_state.fatigue_state;
        const item_growth_state::State reset_state(0, 0);
        if (!AddPendingCargo(user->GetUID(), pending) ||
            !QueueCargoState(user,
                             pending,
                             reset_state,
                             item_growth_db::kWriteCargoReset,
                             &proof)) {
            ErasePendingCargo(user->GetUID(), pending.correlation);
            SendCargoTransportError(user, kErrorNotTransportable);
        }
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory || inventory->delete_item(
            kCargoTransportInventory,
            static_cast<int>(material_slot),
            required_count,
            kCargoTransportMaterialDeleteReason,
            true) != 1) {
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }
    if (inventory->delete_item(kCargoTransportInventory,
                               static_cast<int>(equipment_slot),
                               1,
                               kCargoTransportEquipmentDeleteReason,
                               true) != 1) {
        RestoreInventorySnapshots(inventory,
                                  static_cast<int>(material_slot),
                                  material_snapshot,
                                  true,
                                  static_cast<int>(equipment_slot),
                                  equipment_snapshot,
                                  false);
        SendCargoTransportError(user, kErrorMissingItem);
        return 0;
    }

    Inven_Item moving_equipment = equipment_snapshot;
    const int inserted_slot =
        account_cargo->InsertExactEquipmentForCargoTransport(
            moving_equipment, account_cargo_slot);
    if (inserted_slot != account_cargo_slot) {
        // The narrow insert checks the empty equipment destination before
        // copying, but keep a fully executable compensation path if that
        // invariant is changed later.  Both existing blobs return to exact
        // snapshots and the account-cargo owner remains marked for save.
        account_cargo->RestoreSlotSnapshotForTransaction(
            account_cargo_slot, account_cargo_snapshot);
        RestoreInventorySnapshots(inventory,
                                  static_cast<int>(material_slot),
                                  material_snapshot,
                                  true,
                                  static_cast<int>(equipment_slot),
                                  equipment_snapshot,
                                  true);
        SendCargoTransportError(user, kErrorNotTransportable);
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CARGO_TRANSPORT_ITEM);
    response.put_byte(1);
    response.put_short(static_cast<int>(material_slot));
    response.put_short(static_cast<int>(equipment_slot));
    response.put_short(remaining_count);
    response.put_short(account_cargo_slot);
    response.finalize(true);
    user->Send(response);

    // Mutation-idempotency: after this commit the equipment source slot is
    // empty.  A duplicate CMD551 therefore returns error 19 before consuming
    // another material stack.  The client handler itself applies these two
    // inventory changes from the four returned u16 values, so no duplicate
    // item-update notification is emitted here.
    return 0;
}

int Dispatcher_CargoTransportItem::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_CargoTransportItem::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

void SendCargoTransportSuccess(CUser* user,
                               const PendingCargoTransport& pending) {
    if (!user) return;
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CARGO_TRANSPORT_ITEM);
    response.put_byte(1);
    response.put_short(static_cast<int>(pending.material_slot));
    response.put_short(static_cast<int>(pending.equipment_slot));
    response.put_short(pending.remaining_count);
    response.put_short(pending.account_cargo_slot);
    response.finalize(true);
    user->Send(response);
}

void BlockCargo(CUser* user,
                const PendingCargoTransport& pending,
                uint8_t error,
                const char* reason) {
    SetPendingCargoPhase(user ? user->GetUID() : -1,
                         pending.correlation,
                         kCargoBlocked,
                         error,
                         true);
    LogManager::logFormat(
        1,
        "packet_dispatcher_cargo_transport_item.cpp",
        "BlockCargo",
        0,
        "cargo transport growth saga blocked uid=%d corr=%u reason=%s",
        user ? user->GetUID() : -1,
        pending.correlation,
        reason ? reason : "unknown");
    if (!pending.error_sent) SendCargoTransportError(user, error);
}

void BeginCargoCompensation(CUser* user,
                            const PendingCargoTransport& pending,
                            uint8_t error,
                            const char* reason) {
    if (!user) return;
    SetPendingCargoPhase(user->GetUID(),
                         pending.correlation,
                         kCargoWaitingCompensation,
                         error,
                         false);
    if (!QueueCargoState(user,
                         pending,
                         pending.previous_state,
                         item_growth_db::kWriteCargoCompensate)) {
        BlockCargo(user, pending, error, reason);
    }
}

bool CargoSnapshotsStillMatch(CUser* user,
                              const PendingCargoTransport& pending) {
    if (!user || !user->getCurCharacR() || user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user) ||
        !user->IsExistAccountCargo() ||
        user->CheckItemLock(kCargoTransportInventory,
                            static_cast<int>(pending.material_slot)) ||
        user->CheckItemLock(kCargoTransportInventory,
                            static_cast<int>(pending.equipment_slot))) {
        return false;
    }
    const CInventory* const inventory = user->getCurCharacInvenR();
    CAccountCargo* const cargo = user->GetAccountCargo();
    if (!inventory || !cargo ||
        inventory->CheckValidSlot(kCargoTransportInventory,
                                  static_cast<int>(pending.material_slot)) != 1 ||
        inventory->CheckValidSlot(kCargoTransportInventory,
                                  static_cast<int>(pending.equipment_slot)) != 1 ||
        !cargo->CheckSlotEmpty(pending.account_cargo_slot)) {
        return false;
    }
    const Inven_Item* const material = inventory->GetInvenRef(
        kCargoTransportInventory, static_cast<int>(pending.material_slot));
    const Inven_Item* const equipment = inventory->GetInvenRef(
        kCargoTransportInventory, static_cast<int>(pending.equipment_slot));
    if (!material || !equipment ||
        memcmp(material,
               &pending.material_snapshot,
               sizeof(pending.material_snapshot)) != 0 ||
        memcmp(equipment,
               &pending.equipment_snapshot,
               sizeof(pending.equipment_snapshot)) != 0) {
        return false;
    }
    const Inven_Item current_cargo = cargo->GetSlot(pending.account_cargo_slot);
    if (memcmp(&current_cargo,
               &pending.account_cargo_snapshot,
               sizeof(current_cargo)) != 0) {
        return false;
    }
    item_growth_state::State current_state;
    return item_growth_state::LookupBytes(pending.guid, &current_state) &&
           current_state.progress == pending.previous_state.progress &&
           current_state.fatigue_state == pending.previous_state.fatigue_state;
}

void CompleteCargoReset(CUser* user,
                        const PendingCargoTransport& pending) {
    if (!CargoSnapshotsStillMatch(user, pending)) {
        BeginCargoCompensation(
            user, pending, kErrorNotTransportable, "post-reset revalidation failed");
        return;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    CAccountCargo* const cargo = user->GetAccountCargo();
    if (!inventory || !cargo ||
        inventory->delete_item(
            kCargoTransportInventory,
            static_cast<int>(pending.material_slot),
            pending.required_count,
            kCargoTransportMaterialDeleteReason,
            true) != 1) {
        BeginCargoCompensation(
            user, pending, kErrorMissingItem, "material commit failed");
        return;
    }
    if (inventory->delete_item(
            kCargoTransportInventory,
            static_cast<int>(pending.equipment_slot),
            1,
            kCargoTransportEquipmentDeleteReason,
            true) != 1) {
        RestoreInventorySnapshots(inventory,
                                  static_cast<int>(pending.material_slot),
                                  pending.material_snapshot,
                                  true,
                                  static_cast<int>(pending.equipment_slot),
                                  pending.equipment_snapshot,
                                  false);
        BeginCargoCompensation(
            user, pending, kErrorMissingItem, "equipment commit failed");
        return;
    }

    Inven_Item moving_equipment = pending.equipment_snapshot;
    if (cargo->InsertExactEquipmentForCargoTransport(
            moving_equipment, pending.account_cargo_slot) !=
        pending.account_cargo_slot) {
        cargo->RestoreSlotSnapshotForTransaction(
            pending.account_cargo_slot, pending.account_cargo_snapshot);
        RestoreInventorySnapshots(inventory,
                                  static_cast<int>(pending.material_slot),
                                  pending.material_snapshot,
                                  true,
                                  static_cast<int>(pending.equipment_slot),
                                  pending.equipment_snapshot,
                                  true);
        BeginCargoCompensation(
            user, pending, kErrorNotTransportable, "cargo insert failed");
        return;
    }

    // The SQL006 reset ACK preceded every mutation above.  delete_item and
    // InsertItem may now arm the official inventory/cargo savers without any
    // possibility of those legacy snapshots overtaking the sidecar reset.
    const item_growth_state::State reset_state(0, 0);
    item_growth_state::InstallBytes(pending.guid, reset_state);
    SetPendingCargoPhase(user->GetUID(),
                         pending.correlation,
                         kCargoWaitingPersistedProof,
                         kErrorNotTransportable,
                         false);
    user->enableSaveInven();
    const bool inventory_queued = user->SaveInventory() == 1;
    const bool cargo_queued = DB_SaveAccountCargo::makeRequest(
        user->GetUID(),
        static_cast<unsigned int>(user->get_acc_id()),
        cargo,
        user->GetServerGroup(),
        static_cast<unsigned int>(user->getCurCharacNo()));
    if (!inventory_queued || !cargo_queued) {
        BlockCargo(user,
                   pending,
                   kErrorNotTransportable,
                   "legacy inventory/account-cargo snapshot queue rejected");
    }
}

void CompleteCargoFinalize(CUser* user,
                           const PendingCargoTransport& pending) {
    if (!user) return;
    SendCargoTransportSuccess(user, pending);
}

}  // namespace

namespace cargo_transport_item_runtime {

void HandlePersistenceResult(
    CUser* user,
    const item_growth_db::Message& message) {
    if (!user ||
        (message.write_mode != item_growth_db::kWriteCargoReset &&
         message.write_mode !=
             item_growth_db::kWriteCargoCompensate &&
         message.write_mode !=
             item_growth_db::kWriteCargoFinalize)) {
        return;
    }

    PendingCargoTransport pending;
    const CargoAckClaim claim = ClaimPendingCargoAck(user, message, &pending);
    if (claim == kCargoAckUnknown) {
        LogManager::logFormat(
            1,
            "packet_dispatcher_cargo_transport_item.cpp",
            "HandlePersistenceResult",
            0,
            "unknown cargo growth ACK uid=%d corr=%u mode=%u",
            user->GetUID(),
            message.correlation,
            static_cast<unsigned int>(message.write_mode));
        return;
    }
    if (claim == kCargoAckStaleIdentity) {
        LogManager::logFormat(
            1,
            "packet_dispatcher_cargo_transport_item.cpp",
            "HandlePersistenceResult",
            0,
            "stale cargo growth ACK retained for SQL036 recovery uid=%d corr=%u",
            pending.user_idx,
            pending.correlation);
        return;
    }
    if (claim == kCargoAckMessageMismatch) {
        BlockCargo(user,
                   pending,
                   kErrorNotTransportable,
                   "mismatched SQL006 ACK identity");
        return;
    }

    if (claim == kCargoAckResetFailed) {
        SendCargoTransportError(user, kErrorNotTransportable);
        return;
    }
    if (claim == kCargoAckFailedRetained) {
        BlockCargo(user,
                   pending,
                   pending.error,
                   "SQL006 compensation failed");
        return;
    }
    if (claim == kCargoAckApplyReset) {
        CompleteCargoReset(user, pending);
        return;
    }
    if (claim == kCargoAckCompensated) {
        item_growth_state::InstallBytes(pending.guid,
                                             pending.previous_state);
        SendCargoTransportError(user, pending.error);
        return;
    }
    if (claim == kCargoAckFinalized) {
        CompleteCargoFinalize(user, pending);
        return;
    }

    BlockCargo(user,
               pending,
               kErrorNotTransportable,
               "out-of-order SQL006 ACK");
}

void ResetUserPersistence(CUser* user) {
    if (!user) return;
    PendingCargoTransport pending;
    if (!PreparePendingCargoCompensation(user->GetUID(), &pending)) return;

    if (!QueueCargoCompensation(pending)) {
        SetPendingCargoPhase(user->GetUID(),
                             pending.correlation,
                             kCargoBlocked,
                             kErrorNotTransportable,
                             false);
        LogManager::logFormat(
            1,
            "packet_dispatcher_cargo_transport_item.cpp",
            "ResetUserPersistence",
            0,
            "failed to queue reset compensation uid=%d account=%d charac=%d corr=%u phase=%u",
            pending.user_idx,
            pending.account_id,
            pending.charac_no,
            pending.correlation,
            static_cast<unsigned int>(pending.phase));
    }
}

}  // namespace cargo_transport_item_runtime
