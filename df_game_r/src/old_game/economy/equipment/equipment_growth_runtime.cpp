#include "old_game/economy/equipment/equipment_growth_runtime.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/equipment_growth.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_game/economy/equipment/item_growth_state.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/packet_dispatcher_cargo_transport_item.h"
#include "old_game/player/core/cuser.h"
#include "infra/security/policy/checksum.h"

#include <algorithm>
#include <map>
#include <string.h>
#include <vector>


namespace equipment_growth_runtime {
namespace {

enum {
    kGrowthActionExecute = 0,
    kGrowthActionOpen = 1,
    kGrowthEntryBytes = 3,
    kGrowthRequestPrefixBytes = sizeof(CS_Header) + 1 + 4 + 1,
    kErrorProgressOrState = 23,
    kErrorTarget = 21,
    kErrorMaterialsOrPersistence = 17,
    kInventoryItemSpace = 0,
    kEquippedItemSpace = 3,
    kStageWaitingReset = 1,
    kStageWaitingCompensation = 2,
    kStageBlocked = 3,
    kStageWaitingPersistedProof = 4,
    kStageApplyingReset = 5,
};

enum StageAckClaim {
    kStageAckUnknown = 0,
    kStageAckStaleIdentity,
    kStageAckMessageMismatch,
    kStageAckResetFailed,
    kStageAckFailedRetained,
    kStageAckApplyReset,
    kStageAckCompensated,
    kStageAckFinalized,
    kStageAckOutOfOrder,
};

struct PendingStage {
    PendingStage()
        : correlation(0u),
          user_idx(-1),
          account_id(0),
          charac_no(0),
          server_group(0),
          phase(0u),
          action(0u),
          item_space(0u),
          slot(0u),
          source_item_id(0u),
          output_item_id(0u),
          previous_state(),
          error(kErrorMaterialsOrPersistence),
          error_sent(false) {
        memset(guid, 0, sizeof(guid));
    }

    uint32_t correlation;
    int32_t user_idx;
    int32_t account_id;
    int32_t charac_no;
    int32_t server_group;
    uint8_t phase;
    uint8_t action;
    uint8_t item_space;
    uint16_t slot;
    uint32_t source_item_id;
    uint32_t output_item_id;
    uint8_t guid[item_growth_state::kGuidBytes];
    item_growth_state::State previous_state;
    uint8_t error;
    bool error_sent;
};

typedef std::map<int, PendingStage> PendingStageMap;

PendingStageMap& PendingStages() {
    static PendingStageMap values;
    return values;
}

Mutex& PendingStageMutex() {
    static Mutex mutex;
    return mutex;
}

uint16_t LoadU16(const uint8_t* bytes) {
    return static_cast<uint16_t>(bytes[0]) |
           static_cast<uint16_t>(bytes[1] << 8);
}

uint32_t LoadU32(const uint8_t* bytes) {
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 24);
}

bool QueueStateSave(CUser* user,
                    const itemGloballyUniqueIdentifier_t& guid,
                    const item_growth_state::State& state) {
    return user && item_growth_db::DB_ItemGrowthState::QueueSave(
                       user->GetUID(), user->GetServerGroup(), guid, state);
}

bool QueueCorrelatedSave(CUser* user,
                         const uint8_t* guid,
                          const item_growth_state::State& state,
                          uint8_t write_mode,
                          uint32_t correlation,
                          const item_growth_db::SagaProof* proof = 0) {
    if (!user || !guid ||
        !item_growth_state::IsValidGuid(guid)) {
        return false;
    }
    itemGloballyUniqueIdentifier_t identifier;
    memcpy(&identifier, guid, sizeof(identifier));
    return item_growth_db::DB_ItemGrowthState::QueueSave(
        user->GetUID(),
        user->GetServerGroup(),
        identifier,
        state,
        write_mode,
        correlation,
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

bool GuidEquals(const itemGloballyUniqueIdentifier_t& guid,
                const uint8_t* expected) {
    return expected && memcmp(&guid, expected, sizeof(guid)) == 0;
}

bool HasPendingStage(int user_idx) {
    MutexGuard guard(&PendingStageMutex());
    return PendingStages().find(user_idx) != PendingStages().end();
}

bool AddPendingStage(int user_idx, const PendingStage& pending) {
    MutexGuard guard(&PendingStageMutex());
    return PendingStages().insert(
               std::make_pair(user_idx, pending)).second;
}

bool PreparePendingStageCompensation(int user_idx, PendingStage* pending) {
    MutexGuard guard(&PendingStageMutex());
    PendingStageMap::iterator found = PendingStages().find(user_idx);
    if (found == PendingStages().end()) return false;
    if (found->second.phase == kStageWaitingCompensation ||
        found->second.phase == kStageBlocked) {
        if (pending) *pending = found->second;
        return false;
    }
    found->second.phase = kStageWaitingCompensation;
    found->second.error = kErrorMaterialsOrPersistence;
    found->second.error_sent = false;
    if (pending) *pending = found->second;
    return true;
}

bool QueuePendingStageCompensation(const PendingStage& pending) {
    if (pending.user_idx < 0 || pending.server_group < 0 ||
        !item_growth_state::IsValidGuid(pending.guid)) {
        return false;
    }
    itemGloballyUniqueIdentifier_t identifier;
    memcpy(&identifier, pending.guid, sizeof(identifier));
    return item_growth_db::DB_ItemGrowthState::QueueSave(
        pending.user_idx,
        pending.server_group,
        identifier,
        pending.previous_state,
        item_growth_db::kWriteStageCompensate,
        pending.correlation);
}

bool PendingIdentityMatches(CUser* user, const PendingStage& pending) {
    return user && user->GetUID() == pending.user_idx &&
           user->get_acc_id() == pending.account_id &&
           user->getCurCharacNo() == pending.charac_no &&
           user->GetServerGroup() == pending.server_group;
}

StageAckClaim ClaimPendingStageAck(
    CUser* user,
    const item_growth_db::Message& message,
    PendingStage* pending) {
    if (!user) return kStageAckUnknown;
    const int user_idx = user->GetUID();
    MutexGuard guard(&PendingStageMutex());
    PendingStageMap::iterator found = PendingStages().find(user_idx);
    if (found == PendingStages().end() ||
        found->second.correlation != message.correlation) {
        return kStageAckUnknown;
    }
    PendingStage& value = found->second;
    if (pending) *pending = value;
    if (!PendingIdentityMatches(user, value)) {
        value.phase = kStageBlocked;
        value.error = kErrorMaterialsOrPersistence;
        value.error_sent = false;
        return kStageAckStaleIdentity;
    }
    if (message.owner_user_idx != value.user_idx ||
        message.server_group != value.server_group ||
        memcmp(message.item_guid, value.guid, sizeof(value.guid)) != 0) {
        value.phase = kStageBlocked;
        value.error = kErrorMaterialsOrPersistence;
        value.error_sent = false;
        return kStageAckMessageMismatch;
    }
    if (message.result != item_growth_db::kResultOk) {
        if (value.phase == kStageWaitingReset &&
            message.write_mode == item_growth_db::kWriteStageReset) {
            if (pending) *pending = value;
            PendingStages().erase(found);
            return kStageAckResetFailed;
        }
        value.phase = kStageBlocked;
        value.error_sent = false;
        if (pending) *pending = value;
        return kStageAckFailedRetained;
    }
    if (value.phase == kStageWaitingReset &&
        message.write_mode == item_growth_db::kWriteStageReset) {
        value.phase = kStageApplyingReset;
        if (pending) *pending = value;
        return kStageAckApplyReset;
    }
    if (value.phase == kStageWaitingCompensation &&
        message.write_mode == item_growth_db::kWriteStageCompensate) {
        if (pending) *pending = value;
        PendingStages().erase(found);
        return kStageAckCompensated;
    }
    if (value.phase == kStageWaitingPersistedProof &&
        message.write_mode == item_growth_db::kWriteStageFinalize) {
        if (pending) *pending = value;
        PendingStages().erase(found);
        return kStageAckFinalized;
    }
    value.phase = kStageBlocked;
    value.error = kErrorMaterialsOrPersistence;
    value.error_sent = false;
    if (pending) *pending = value;
    return kStageAckOutOfOrder;
}

void ErasePendingStage(int user_idx, uint32_t correlation) {
    MutexGuard guard(&PendingStageMutex());
    PendingStageMap::iterator found = PendingStages().find(user_idx);
    if (found != PendingStages().end() &&
        found->second.correlation == correlation) {
        PendingStages().erase(found);
    }
}

void SetPendingStagePhase(int user_idx,
                          uint32_t correlation,
                          uint8_t phase,
                          uint8_t error,
                          bool error_sent) {
    MutexGuard guard(&PendingStageMutex());
    PendingStageMap::iterator found = PendingStages().find(user_idx);
    if (found != PendingStages().end() &&
        found->second.correlation == correlation) {
        found->second.phase = phase;
        found->second.error = error;
        found->second.error_sent = error_sent;
    }
}

void SendRenew(CUser* user,
               uint8_t item_space,
               uint16_t slot,
               uint16_t delta,
               const item_growth_state::State& state) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_GROWTH_ITEM_RENEW_DATA);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.put_short(delta);
    packet.put_short(state.progress);
    packet.put_byte(state.fatigue_state);
    packet.finalize(true);
    user->Send(packet);
}

void SendChange(CUser* user,
                uint8_t item_space,
                uint16_t slot,
                uint32_t replacement_item_id,
                const item_growth_state::State& state) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_GROWTH_ITEM_CHANGE_DATA);
    packet.put_byte(1);  // mode 1: replace the prototype in the same slot
    packet.put_byte(1);
    packet.put_byte(item_space);
    packet.put_short(slot);
    packet.put_int(replacement_item_id);
    packet.put_short(state.progress);
    packet.put_byte(state.fatigue_state);
    packet.finalize(true);
    user->Send(packet);
}

void SendCommandResult(CUser* user,
                       uint8_t action,
                       bool changed,
                       uint8_t item_space,
                       uint16_t slot,
                       uint32_t replacement_item_id) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_TRY_ENCHANT);
    packet.put_byte(1);
    packet.put_byte(action);
    packet.put_byte(changed ? 1 : 0);
    packet.put_int(user->GetCurCharacExpertJobExp());
    const CInventory* const inventory = user->getCurCharacInvenR();
    packet.put_int(inventory ? inventory->get_money() : 0);
    packet.put_byte(changed ? 1 : 0);
    if (changed) {
        packet.put_byte(item_space);
        packet.put_short(slot);
        packet.put_int(replacement_item_id);
    }
    packet.finalize(true);
    user->Send(packet);
}

bool ValidateTransition(uint32_t item_id,
                        equipment_growth::PrototypeIdentity* identity,
                        CEquipItem** prototype,
                        uint32_t* output_item_id) {
    if (identity) *identity = equipment_growth::PrototypeIdentity();
    if (prototype) *prototype = 0;
    if (output_item_id) *output_item_id = 0;

    equipment_growth::PrototypeIdentity current;
    if (!equipment_growth::FindPrototype(
            static_cast<int>(item_id), &current) ||
        equipment_growth::GetStageMaximum(current.type, current.stage) ==
            0) {
        return false;
    }
    CItem* const base = G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!base) return false;
    CEquipItem* const equipment = static_cast<CEquipItem*>(base);
    if (equipment->emancipate_results_.size() != 1u ||
        equipment->emancipate_results_[0].first <= 0 ||
        equipment->emancipate_results_[0].second != 1) {
        return false;
    }

    equipment_growth::PrototypeIdentity output;
    const int output_id = equipment->emancipate_results_[0].first;
    if (!equipment_growth::FindPrototype(output_id, &output) ||
        output.type != current.type ||
        output.stage != static_cast<uint8_t>(current.stage + 1u)) {
        return false;
    }
    if (identity) *identity = current;
    if (prototype) *prototype = equipment;
    if (output_item_id) *output_item_id = static_cast<uint32_t>(output_id);
    return true;
}

void SendError(CUser* user, uint8_t error) {
    if (user) user->SendCmdErrorPacket(ENUM_CMDPACKET_TRY_ENCHANT, error);
}

bool ExecuteTransition(CUser* user,
                       uint8_t action,
                       uint8_t item_space,
                       uint16_t slot,
                       Inven_Item* target) {
    if (user && HasPendingStage(user->GetUID())) {
        SendError(user, kErrorMaterialsOrPersistence);
        return true;
    }
    if (!user || !target || target->item_id_ <= 0 ||
        !item_growth_state::IsValidGuid(
            &target->globally_unique_id_)) {
        SendError(user, kErrorMaterialsOrPersistence);
        return true;
    }

    equipment_growth::PrototypeIdentity identity;
    CEquipItem* prototype = 0;
    uint32_t output_item_id = 0;
    if (!ValidateTransition(static_cast<uint32_t>(target->item_id_),
                            &identity,
                            &prototype,
                            &output_item_id)) {
        SendError(user, kErrorTarget);
        return true;
    }
    if (user->CheckItemLock(
            item_space == kEquippedItemSpace ? 0 : 1,
            static_cast<int>(slot))) {
        SendError(user, kErrorTarget);
        return true;
    }

    item_growth_state::State current;
    item_growth_state::Lookup(target->globally_unique_id_, &current);
    const uint16_t maximum = equipment_growth::GetStageMaximum(
        identity.type, identity.stage);
    if (!maximum || current.progress < maximum) {
        SendError(user, kErrorProgressOrState);
        return true;
    }

    CInventory* const readable = user->getCurCharacInvenRefR();
    if (!readable || prototype->emancipate_materials_.empty()) {
        SendError(user, kErrorMaterialsOrPersistence);
        return true;
    }

    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*readable);
    if (preview.tryDeleteEventItems(prototype->emancipate_materials_) != 1) {
        SendError(user, kErrorMaterialsOrPersistence);
        return true;
    }
    Inven_Item* const preview_target = preview.GetInvenRef(1, slot);
    item_growth_db::SagaProof proof;
    memset(&proof, 0, sizeof(proof));
    if (!preview_target ||
        !GuidEquals(preview_target->globally_unique_id_,
                    reinterpret_cast<const uint8_t*>(
                        &target->globally_unique_id_))) {
        SendError(user, kErrorTarget);
        return true;
    }
    preview_target->item_id_ = static_cast<int32_t>(output_item_id);
    proof.saga_owner = item_growth_db::kSagaOwnerStage;
    proof.source_space = item_growth_db::kSagaSpaceInventory;
    proof.target_space = item_growth_db::kSagaSpaceInventory;
    proof.proof_flags = item_growth_db::kSagaProofInventory;
    proof.source_slot = slot;
    proof.target_slot = slot;
    proof.owner_user_idx = user->GetUID();
    proof.account_id = static_cast<uint32_t>(user->get_acc_id());
    proof.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    proof.source_item_id = static_cast<uint32_t>(target->item_id_);
    proof.target_item_id = output_item_id;
    proof.baseline_progress = current.progress;
    proof.baseline_fatigue = current.fatigue_state;
    if (!HashMainInventory(readable, proof.pre_inventory_sha256) ||
        !HashMainInventory(&preview, proof.post_inventory_sha256) ||
        memcmp(proof.pre_inventory_sha256,
               proof.post_inventory_sha256,
               32u) == 0) {
        SendError(user, kErrorMaterialsOrPersistence);
        return true;
    }

    // Correlated reset barrier.  Do not consume materials, change the item
    // prototype, enable the legacy inventory saver, or tell the client that
    // the stage changed until SQL006 has acknowledged this exact operation.
    PendingStage pending;
    pending.correlation = item_growth_db::NextCorrelation();
    pending.user_idx = user->GetUID();
    pending.account_id = user->get_acc_id();
    pending.charac_no = user->getCurCharacNo();
    pending.server_group = user->GetServerGroup();
    pending.phase = kStageWaitingReset;
    pending.action = action;
    pending.item_space = item_space;
    pending.slot = slot;
    pending.source_item_id = static_cast<uint32_t>(target->item_id_);
    pending.output_item_id = output_item_id;
    memcpy(pending.guid,
           &target->globally_unique_id_,
           sizeof(pending.guid));
    pending.previous_state = current;
    const item_growth_state::State reset_state(0, 0);
    if (!AddPendingStage(user->GetUID(), pending) ||
        !QueueCorrelatedSave(user,
                             pending.guid,
                             reset_state,
                             item_growth_db::kWriteStageReset,
                             pending.correlation,
                             &proof)) {
        ErasePendingStage(user->GetUID(), pending.correlation);
        SendError(user, kErrorMaterialsOrPersistence);
    }
    return true;
}

void BlockStage(CUser* user,
                const PendingStage& pending,
                uint8_t error,
                const char* reason) {
    SetPendingStagePhase(user ? user->GetUID() : -1,
                         pending.correlation,
                         kStageBlocked,
                         error,
                         true);
    LogManager::logFormat(1,
                          "equipment_growth_runtime.cpp",
                          "BlockStage",
                          0,
                          "equipment growth saga blocked uid=%d corr=%u reason=%s",
                          user ? user->GetUID() : -1,
                          pending.correlation,
                          reason ? reason : "unknown");
    SendError(user, error);
}

void BeginStageCompensation(CUser* user,
                            const PendingStage& pending,
                            uint8_t error,
                            const char* reason) {
    if (!user) return;
    SetPendingStagePhase(user->GetUID(),
                         pending.correlation,
                         kStageWaitingCompensation,
                         error,
                         false);
    if (!QueueCorrelatedSave(user,
                             pending.guid,
                             pending.previous_state,
                             item_growth_db::kWriteStageCompensate,
                             pending.correlation)) {
        BlockStage(user, pending, error, reason);
    }
}

void CompleteStageReset(CUser* user, const PendingStage& pending) {
    if (!user || !user->getCurCharacR()) {
        BlockStage(user,
                   pending,
                   kErrorMaterialsOrPersistence,
                   "user state unavailable after reset ACK");
        return;
    }

    Inven_Item* target = user->GetCurCharacInventoryRef(
        pending.item_space, pending.slot);
    equipment_growth::PrototypeIdentity identity;
    CEquipItem* prototype = 0;
    uint32_t output_item_id = 0;
    item_growth_state::State current;
    const bool current_found = target && item_growth_state::Lookup(
                                              target->globally_unique_id_,
                                              &current);
    if (!target || static_cast<uint32_t>(target->item_id_) !=
                       pending.source_item_id ||
        !GuidEquals(target->globally_unique_id_, pending.guid) ||
        user->CheckItemLock(pending.item_space == kEquippedItemSpace ? 0 : 1,
                            static_cast<int>(pending.slot)) ||
        !ValidateTransition(pending.source_item_id,
                            &identity,
                            &prototype,
                            &output_item_id) ||
        output_item_id != pending.output_item_id || !current_found ||
        current.progress != pending.previous_state.progress ||
        current.fatigue_state != pending.previous_state.fatigue_state) {
        BeginStageCompensation(
            user, pending, kErrorTarget, "post-reset revalidation failed");
        return;
    }

    CInventory* const readable = user->getCurCharacInvenRefR();
    CInventory* const writable = user->getCurCharacInvenW();
    if (!readable || !writable || !prototype ||
        prototype->emancipate_materials_.empty()) {
        BeginStageCompensation(
            user, pending, kErrorMaterialsOrPersistence, "inventory unavailable");
        return;
    }

    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*readable);
    if (preview.tryDeleteEventItems(prototype->emancipate_materials_) != 1) {
        BeginStageCompensation(
            user, pending, kErrorMaterialsOrPersistence, "materials changed");
        return;
    }

    CInventory rollback;
    InventoryMemory rollback_memory;
    rollback.SetInventoryMemory(&rollback_memory);
    rollback.setCopy(*readable);
    if (writable->tryDeleteEventItems(prototype->emancipate_materials_) != 1) {
        writable->setCopy(rollback);
        BeginStageCompensation(
            user, pending, kErrorMaterialsOrPersistence, "material commit failed");
        return;
    }
    target = user->GetCurCharacInventoryRef(pending.item_space, pending.slot);
    if (!target || static_cast<uint32_t>(target->item_id_) !=
                       pending.source_item_id ||
        !GuidEquals(target->globally_unique_id_, pending.guid)) {
        writable->setCopy(rollback);
        BeginStageCompensation(
            user, pending, kErrorTarget, "target changed during commit");
        return;
    }

    // SQL006 reset is durable at this point.  Only now may the legacy
    // inventory snapshot become dirty, so its save cannot overtake reset.
    target->item_id_ = static_cast<int32_t>(pending.output_item_id);
    const item_growth_state::State reset_state(0, 0);
    item_growth_state::Update(target->globally_unique_id_, reset_state);
    SetPendingStagePhase(user->GetUID(),
                         pending.correlation,
                         kStageWaitingPersistedProof,
                         kErrorMaterialsOrPersistence,
                         false);
    user->enableSaveInven();
    if (user->SaveInventory() != 1) {
        writable->setCopy(rollback);
        item_growth_state::InstallBytes(pending.guid,
                                             pending.previous_state);
        BeginStageCompensation(user,
                               pending,
                               kErrorMaterialsOrPersistence,
                               "legacy inventory snapshot queue rejected");
    }
}

void CompleteStageFinalize(CUser* user, const PendingStage& pending) {
    if (!user) return;
    const item_growth_state::State reset_state(0, 0);
    SendCommandResult(user,
                      pending.action,
                      true,
                      pending.item_space,
                      pending.slot,
                      pending.output_item_id);
    SendChange(user,
               pending.item_space,
               pending.slot,
               pending.output_item_id,
               reset_state);
    user->send_itemspace(kInventoryItemSpace);
}

}  // namespace


bool TryHandleCommand(CUser* user, PacketBuf& packet) {
    const uint8_t* const raw =
        reinterpret_cast<const uint8_t*>(packet.get_packet(0));
    if (!user || !raw ||
        packet.get_index() != static_cast<int>(sizeof(CS_Header)) ||
        packet.is_len_end(kGrowthRequestPrefixBytes - sizeof(CS_Header))) {
        return false;
    }
    const CS_Header* const header = reinterpret_cast<const CS_Header*>(raw);
    if (header->cmd != ENUM_PACKETCLASS_CMDPACKET ||
        header->type != ENUM_CMDPACKET_TRY_ENCHANT ||
        header->length < kGrowthRequestPrefixBytes) {
        return false;
    }

    const uint8_t action = raw[sizeof(CS_Header)];
    if (action != kGrowthActionExecute && action != kGrowthActionOpen) {
        return false;
    }
    const uint32_t item_id = LoadU32(raw + sizeof(CS_Header) + 1u);
    equipment_growth::PrototypeIdentity identity;
    if (!equipment_growth::FindPrototype(
            static_cast<int>(item_id), &identity)) {
        return false;
    }

    const uint8_t count = raw[sizeof(CS_Header) + 5u];
    const unsigned int required = static_cast<unsigned int>(
        kGrowthRequestPrefixBytes + count * kGrowthEntryBytes);
    if (header->length != required ||
        packet.is_len_end(static_cast<int>(required - sizeof(CS_Header)))) {
        // The opcode is shared with the legacy expert-job protocol.  A
        // growth-looking u32 alone is insufficient ownership evidence: only
        // the exact counted 0828 frame may be claimed here.  Ambiguous frames
        // must remain on the legacy path, including action 0/1 payloads.
        return false;
    }

    if (action == kGrowthActionOpen) {
        if (count != 0u) {
            SendError(user, kErrorTarget);
        } else {
            // Action 1 only opens/refreshes the growth UI.  The 0828 sender
            // carries no slot, so consuming materials here would act on an
            // arbitrary duplicate.  Action 0 is the sole stage transaction.
            SendCommandResult(user, action, false, 0, 0, 0);
        }
        return true;
    }

    if (count != 1u) {
        SendError(user, kErrorTarget);
        return true;
    }
    const uint8_t* const entry = raw + kGrowthRequestPrefixBytes;
    const uint8_t item_space = entry[0];
    const uint16_t slot = LoadU16(entry + 1);
    // Validate the wire container before handing it to the legacy inventory
    // accessor.  The recovered execute sender uses ordinary inventory only;
    // an arbitrary client byte must never become an unchecked space index.
    if (item_space != kInventoryItemSpace ||
        slot > oldsrv::config::kMainInventoryLastSlot) {
        SendError(user, kErrorTarget);
        return true;
    }
    Inven_Item* const target = user->GetCurCharacInventoryRef(item_space, slot);
    if (!target ||
        static_cast<uint32_t>(target->item_id_) != item_id) {
        SendError(user, kErrorTarget);
        return true;
    }
    return ExecuteTransition(user, action, item_space, slot, target);
}

void AwardDeathProgress(CUser* user,
                        int monster_kind,
                        int monster_level,
                        int dungeon_difficulty) {
    if (!user || !user->getCurCharacR()) return;
    for (int slot = oldsrv::config::kOrdinaryEquipmentTypeFirst;
         slot <= oldsrv::config::kOrdinaryEquipmentTypeLast; ++slot) {
        Inven_Item* const item = user->GetCurCharacInventoryRef(
            kEquippedItemSpace, static_cast<uint16_t>(slot));
        if (!item || item->item_id_ <= 0 ||
            !item_growth_state::IsValidGuid(
                &item->globally_unique_id_)) {
            continue;
        }

        equipment_growth::PrototypeIdentity identity;
        if (!equipment_growth::FindPrototype(item->item_id_, &identity)) {
            continue;
        }
        const uint16_t maximum = equipment_growth::GetStageMaximum(
            identity.type, identity.stage);
        if (!maximum) continue;

        item_growth_state::State state;
        item_growth_state::Lookup(item->globally_unique_id_, &state);
        uint8_t next_fatigue = state.fatigue_state;
        const uint16_t offered = equipment_growth::CalculateDeathAward(
            identity.type,
            identity.stage,
            monster_kind,
            monster_level,
            user->get_charac_level(),
            dungeon_difficulty,
            state.fatigue_state,
            &next_fatigue);
        if (!offered || state.progress >= maximum) continue;

        const uint16_t room = static_cast<uint16_t>(maximum - state.progress);
        const uint16_t applied = std::min(offered, room);
        state.progress = static_cast<uint16_t>(state.progress + applied);
        state.fatigue_state = next_fatigue;
        item_growth_state::Update(item->globally_unique_id_, state);
        if (!QueueStateSave(user, item->globally_unique_id_, state)) {
            LogManager::logFormat(
                1,
                "equipment_growth_runtime.cpp",
                "AwardDeathProgress",
                0,
                "equipment growth save queue failed uid=%d",
                user->GetUID());
        }
        SendRenew(user,
                  kEquippedItemSpace,
                  static_cast<uint16_t>(slot),
                  applied,
                  state);
    }
}

bool ClearForCargoTransport(CUser* user, const Inven_Item& item) {
    equipment_growth::PrototypeIdentity identity;
    if (!user ||
        !equipment_growth::FindPrototype(item.item_id_, &identity) ||
        !item_growth_state::IsValidGuid(&item.globally_unique_id_)) {
        return false;
    }
    const item_growth_state::State reset_state(0, 0);
    item_growth_state::Update(item.globally_unique_id_, reset_state);
    return QueueStateSave(user, item.globally_unique_id_, reset_state);
}

void HandlePersistenceResult(
    CUser* user,
    const item_growth_db::Message& message) {
    if (!user ||
        (message.write_mode != item_growth_db::kWriteStageReset &&
         message.write_mode !=
             item_growth_db::kWriteStageCompensate &&
         message.write_mode !=
             item_growth_db::kWriteStageFinalize)) {
        return;
    }

    PendingStage pending;
    const StageAckClaim claim = ClaimPendingStageAck(user, message, &pending);
    if (claim == kStageAckUnknown) {
        LogManager::logFormat(
            1,
            "equipment_growth_runtime.cpp",
            "HandlePersistenceResult",
            0,
            "unknown equipment growth ACK uid=%d corr=%u mode=%u",
            user->GetUID(),
            message.correlation,
            static_cast<unsigned int>(message.write_mode));
        return;
    }
    if (claim == kStageAckStaleIdentity) {
        LogManager::logFormat(
            1,
            "equipment_growth_runtime.cpp",
            "HandlePersistenceResult",
            0,
            "stale equipment growth ACK retained for SQL036 recovery uid=%d corr=%u",
            pending.user_idx,
            pending.correlation);
        return;
    }
    if (claim == kStageAckMessageMismatch) {
        BlockStage(user,
                   pending,
                   kErrorMaterialsOrPersistence,
                   "mismatched SQL006 ACK identity");
        return;
    }

    if (claim == kStageAckResetFailed) {
        SendError(user, kErrorMaterialsOrPersistence);
        return;
    }
    if (claim == kStageAckFailedRetained) {
        BlockStage(user,
                   pending,
                   pending.error,
                   "SQL006 compensation failed");
        return;
    }
    if (claim == kStageAckApplyReset) {
        CompleteStageReset(user, pending);
        return;
    }
    if (claim == kStageAckCompensated) {
        item_growth_state::InstallBytes(pending.guid,
                                             pending.previous_state);
        SendError(user, pending.error);
        return;
    }
    if (claim == kStageAckFinalized) {
        CompleteStageFinalize(user, pending);
        return;
    }

    BlockStage(user,
               pending,
               kErrorMaterialsOrPersistence,
                "out-of-order SQL006 ACK");
}

void ResetUserPersistence(CUser* user) {
    if (!user) return;
    PendingStage pending;
    if (!PreparePendingStageCompensation(user->GetUID(), &pending)) return;

    // A compensation already queued behind the reset is sufficient.  Every
    // other phase may have made the reset durable, so append a restoration on
    // the same queue6 FIFO before this CUser slot can be reused.
    if (!QueuePendingStageCompensation(pending)) {
        SetPendingStagePhase(user->GetUID(),
                             pending.correlation,
                             kStageBlocked,
                             kErrorMaterialsOrPersistence,
                             false);
        LogManager::logFormat(
            1,
            "equipment_growth_runtime.cpp",
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

}  // namespace equipment_growth_runtime


Inter_ItemGrowthState::Inter_ItemGrowthState() {}
Inter_ItemGrowthState::~Inter_ItemGrowthState() {}

int Inter_ItemGrowthState::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (!payload ||
        payload_size !=
            static_cast<int>(sizeof(item_growth_db::Message))) {
        return 0;
    }
    const item_growth_db::Message* const message =
        reinterpret_cast<const item_growth_db::Message*>(payload);
    if (message->magic != item_growth_db::kMessageMagic ||
        message->version != item_growth_db::kMessageVersion ||
        message->operation != item_growth_db::kOperationSave) {
        return 0;
    }
    if (message->write_mode == item_growth_db::kWriteStageReset ||
        message->write_mode ==
            item_growth_db::kWriteStageCompensate ||
        message->write_mode ==
            item_growth_db::kWriteStageFinalize) {
        equipment_growth_runtime::HandlePersistenceResult(user, *message);
        return 0;
    }
    if (message->write_mode == item_growth_db::kWriteCargoReset ||
        message->write_mode ==
            item_growth_db::kWriteCargoCompensate ||
        message->write_mode ==
            item_growth_db::kWriteCargoFinalize) {
        cargo_transport_item_runtime::HandlePersistenceResult(user, *message);
        return 0;
    }
    if (message->result != item_growth_db::kResultOk) {
        LogManager::logFormat(
            1,
            "equipment_growth_runtime.cpp",
            "Inter_ItemGrowthState0828::dispatch_sig",
            0,
            "equipment growth database save failed");
    }
    return 0;
}
