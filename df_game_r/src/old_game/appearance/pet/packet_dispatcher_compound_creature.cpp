#include "old_game/appearance/pet/packet_dispatcher_compound_creature.h"

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <map>

#include "common/config.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/system_time.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/etc/compound_creature_script.h"
#include "old_data/db/creature/compound_creature_marker.h"
#include "old_data/db/creature/creature_item_payload.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"


namespace {

const int kCompounderItemId = 2675001;
const int kActiveCreatureSlot =
    oldsrv::config::kCreatureInventoryEquippedCreatureSlot;
const INVEN_TYPE kMaterialInventory = static_cast<INVEN_TYPE>(1);
const INVEN_TYPE kCreatureInventory = static_cast<INVEN_TYPE>(3);
const eItemDelReason kCompoundDeleteReason =
    static_cast<eItemDelReason>(8);
const eItemAddReason kCompoundAddReason =
    static_cast<eItemAddReason>(8);

const uint8_t kErrorMissingItem = 17;
const uint8_t kErrorInvalidRequest = 22;
const uint8_t kErrorInvalidCreature = 23;
const uint8_t kErrorItemLocked = 213;

struct PendingCompound {
    PendingCompound()
        : token(0),
          charac_no(0),
          material_slot(0),
          first_slot(0),
          second_slot(0),
          first_uid(0),
          second_uid(0),
          result_item_id(0),
          result_slot(0),
          result_rare(false),
          result_duration_days(0),
          result_delete_tick(0),
          created_at(0),
          processing(false),
          material_snapshot(),
          first_snapshot(),
          second_snapshot(),
          result_item() {}

    uint32_t token;
    int charac_no;
    int material_slot;
    int first_slot;
    int second_slot;
    int first_uid;
    int second_uid;
    int result_item_id;
    int result_slot;
    bool result_rare;
    int result_duration_days;
    int result_delete_tick;
    time_t created_at;
    bool processing;
    Inven_Item material_snapshot;
    Inven_Item first_snapshot;
    Inven_Item second_snapshot;
    Inven_Item result_item;
};

typedef std::map<int, PendingCompound> PendingTable;

PendingTable& PendingCompounds() {
    static PendingTable transactions;
    return transactions;
}

pthread_mutex_t g_pending_mutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t g_next_token = 1;

class PendingLock {
public:
    PendingLock() { pthread_mutex_lock(&g_pending_mutex); }
    ~PendingLock() { pthread_mutex_unlock(&g_pending_mutex); }

private:
    PendingLock(const PendingLock&);
    PendingLock& operator=(const PendingLock&);
};

void SendCompoundError(CUser* user, uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_CREATURE, error);
    }
}

bool SameSnapshot(const Inven_Item* item, const Inven_Item& snapshot) {
    return item && memcmp(item, &snapshot, sizeof(snapshot)) == 0;
}

bool IsValidCreaturePrototype(CItem* prototype,
                              bool require_egg,
                              int* subtype) {
    if (!prototype || prototype->is_stackable() != 0) return false;
    CEquipItem* const equipment = static_cast<CEquipItem*>(prototype);
    const int type = equipment->get_sub_type();
    if (equipment->GetItemType() != 23 ||
        equipment->IsCreatureItem() == 0 || type < 0 || type > 1 ||
        (require_egg && type != 1)) {
        return false;
    }
    if (subtype) *subtype = type;
    return true;
}

bool IsLiveCreatureRecord(user_creature::CCreatureMgr* manager,
                          const Inven_Item& item,
                          int slot) {
    if (!manager || item.item_category_ != 5u || item.item_id_ <= 0 ||
        item.get_add_info() <= 0 || slot == kActiveCreatureSlot ||
        item.upgrade_separate_.IsTradeRestriction()) {
        return false;
    }
    user_creature::CCreatureItem* const creature =
        const_cast<user_creature::CCreatureItem*>(
            manager->FindCreatureItem(item.get_add_info()));
    return creature && creature->GetUid() == item.get_add_info() &&
           creature->GetSlotNo() == slot &&
           creature->GetItemId() == item.item_id_;
}

bool IsCreatureExpired(user_creature::CCreatureMgr* manager,
                       int creature_uid) {
    if (!manager) return true;
    const int expire_tick = manager->getExpireDate(creature_uid);
    CSystemTime* const system_time = GlobalData::system_time();
    return expire_tick > 0 && system_time &&
           expire_tick <= system_time->getCurSec();
}

void RestoreSources(CInventory* inventory,
                    const PendingCompound& pending) {
    if (!inventory) return;
    (void)inventory->update_item(kMaterialInventory,
                                 pending.material_slot,
                                 pending.material_snapshot);
    (void)inventory->update_item(kCreatureInventory,
                                 pending.first_slot,
                                 pending.first_snapshot);
    (void)inventory->update_item(kCreatureInventory,
                                 pending.second_slot,
                                 pending.second_snapshot);
}

bool PrepareSoftDelete(user_creature::CDBMsgSender& sender,
                       CUser* user,
                       const Inven_Item& item,
                       int charac_no,
                       int slot,
                       int creature_count) {
    Inven_Item copy = item;
    if (!user || copy.get_add_info() <= 0 ||
        sender.SendMail(&copy,
                        charac_no,
                        slot,
                        18,
                        creature_count) != 1) {
        return false;
    }
    SIG_CREATURE_ITEM* const payload =
        sender.GetInBuffer<SIG_CREATURE_ITEM>();
    if (!payload) return false;
    // update_kind=0 deliberately suppresses SendMail's unrelated
    // creature_flag owner transition while retaining update_mask 0x100/stat=1.
    payload->update_kind = 0;
    return true;
}

bool QueueNewRowCompensation(CUser* user, SIG_CREATURE_ITEM* signal) {
    if (!user || !signal || signal->detail.ui_id == 0) return false;
    Inven_Item item;
    item.item_id_ = static_cast<int>(signal->inven_item_copy.item_id);
    item.item_category_ = signal->inven_item_copy.item_type;
    item.set_add_info(static_cast<int>(signal->detail.ui_id));
    user_creature::CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(138),
                                       user);
    if (!PrepareSoftDelete(sender,
                           user,
                           item,
                           static_cast<int>(signal->detail.charac_no),
                           static_cast<int>(signal->detail.slot),
                           1)) {
        return false;
    }
    return sender.Send() == 1;
}

void ErasePending(int user_uid, uint32_t token) {
    PendingLock lock;
    PendingTable& transactions = PendingCompounds();
    PendingTable::iterator found = transactions.find(user_uid);
    if (found != transactions.end() && found->second.token == token) {
        transactions.erase(found);
    }
}

bool BeginPending(CUser* user, PendingCompound& pending) {
    if (!user) return false;
    const int user_uid = user->GetUID();
    const time_t now = time(0);
    PendingLock lock;
    PendingTable& transactions = PendingCompounds();
    PendingTable::iterator found = transactions.find(user_uid);
    if (found != transactions.end()) {
        if (found->second.processing ||
            now <= found->second.created_at ||
            now - found->second.created_at <= 60) {
            return false;
        }
        // A late signal136 is token-tagged and will soft-delete its unused DB
        // row.  Releasing this timed-out reservation never consumes sources.
        transactions.erase(found);
    }
    if (++g_next_token == 0) ++g_next_token;
    pending.token = g_next_token;
    pending.created_at = now;
    transactions.insert(std::make_pair(user_uid, pending));
    return true;
}

bool AcquirePendingForResult(CUser* user,
                             uint32_t token,
                             PendingCompound& pending) {
    if (!user) return false;
    PendingLock lock;
    PendingTable& transactions = PendingCompounds();
    PendingTable::iterator found = transactions.find(user->GetUID());
    if (found == transactions.end() || found->second.token != token ||
        found->second.processing) {
        return false;
    }
    found->second.processing = true;
    pending = found->second;
    return true;
}

bool RevalidatePending(CUser* user,
                       CInventory* inventory,
                       user_creature::CCreatureMgr* manager,
                       const PendingCompound& pending) {
    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        user->getCurCharacNo() != pending.charac_no || user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user) ||
        !inventory || !manager) {
        return false;
    }
    if (user->CheckItemLock(kMaterialInventory, pending.material_slot) ||
        user->CheckItemLock(kCreatureInventory, pending.first_slot) ||
        user->CheckItemLock(kCreatureInventory, pending.second_slot)) {
        return false;
    }
    const Inven_Item* const material = inventory->GetInvenRef(
        kMaterialInventory, pending.material_slot);
    const Inven_Item* const first = inventory->GetInvenRef(
        kCreatureInventory, pending.first_slot);
    const Inven_Item* const second = inventory->GetInvenRef(
        kCreatureInventory, pending.second_slot);
    return SameSnapshot(material, pending.material_snapshot) &&
           SameSnapshot(first, pending.first_snapshot) &&
           SameSnapshot(second, pending.second_snapshot) &&
           IsLiveCreatureRecord(manager, *first, pending.first_slot) &&
           IsLiveCreatureRecord(manager, *second, pending.second_slot) &&
           !IsCreatureExpired(manager, pending.first_uid) &&
           !IsCreatureExpired(manager, pending.second_uid);
}

void SendCompoundSuccess(CUser* user, const PendingCompound& pending) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_COMPOUND_CREATURE);
    response.put_byte(1);
    response.put_byte(3);
    response.put_byte(0);
    response.put_short(pending.material_slot);
    response.put_int(1);
    response.put_byte(7);
    response.put_short(pending.first_slot);
    response.put_int(1);
    response.put_byte(7);
    response.put_short(pending.second_slot);
    response.put_int(1);
    response.put_short(pending.result_slot);
    response.put_int(pending.result_item_id);
    response.put_byte(pending.result_rare ? 1 : 0);
    response.finalize(true);
    user->Send(response);
}

}  // namespace


Dispatcher_CompoundCreature::Dispatcher_CompoundCreature()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_CompoundCreature::~Dispatcher_CompoundCreature() {}

int Dispatcher_CompoundCreature::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    uint16_t material_slot_wire = 0;
    uint16_t first_slot_wire = 0;
    uint16_t second_slot_wire = 0;
    if (packet.get_short(material_slot_wire) != 1 ||
        packet.get_short(first_slot_wire) != 1 ||
        packet.get_short(second_slot_wire) != 1) {
        SendCompoundError(user, kErrorMissingItem);
        return 0;
    }

    if (!user || user->get_state() != 3 || !user->getCurCharacR() ||
        user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }

    const int material_slot = static_cast<int>(material_slot_wire);
    const int first_slot = static_cast<int>(first_slot_wire);
    const int second_slot = static_cast<int>(second_slot_wire);
    if (first_slot == second_slot || first_slot == kActiveCreatureSlot ||
        second_slot == kActiveCreatureSlot) {
        SendCompoundError(user, kErrorInvalidCreature);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    user_creature::CCreatureMgr* const manager =
        inventory ? inventory->GetCreatureMgrW() : 0;
    if (!inventory || !manager ||
        inventory->CheckValidSlot(kMaterialInventory, material_slot) != 1 ||
        inventory->CheckValidSlot(kCreatureInventory, first_slot) != 1 ||
        inventory->CheckValidSlot(kCreatureInventory, second_slot) != 1) {
        SendCompoundError(user, kErrorMissingItem);
        return 0;
    }
    if (user->CheckItemLock(kMaterialInventory, material_slot) ||
        user->CheckItemLock(kCreatureInventory, first_slot) ||
        user->CheckItemLock(kCreatureInventory, second_slot)) {
        SendCompoundError(user, kErrorItemLocked);
        return 0;
    }

    const Inven_Item* const material_ref = inventory->GetInvenRef(
        kMaterialInventory, material_slot);
    const Inven_Item* const first_ref = inventory->GetInvenRef(
        kCreatureInventory, first_slot);
    const Inven_Item* const second_ref = inventory->GetInvenRef(
        kCreatureInventory, second_slot);
    if (!material_ref || material_ref->item_id_ != kCompounderItemId ||
        material_ref->get_add_info() < 1 || !first_ref || !second_ref) {
        SendCompoundError(user, kErrorMissingItem);
        return 0;
    }
    CItem* const material_prototype =
        G_CDataManager()->find_item(material_ref->item_id_);
    if (!material_prototype || material_prototype->is_stackable() != 1) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }

    int first_subtype = -1;
    int second_subtype = -1;
    CItem* const first_prototype =
        G_CDataManager()->find_item(first_ref->item_id_);
    CItem* const second_prototype =
        G_CDataManager()->find_item(second_ref->item_id_);
    if (!IsValidCreaturePrototype(first_prototype, false, &first_subtype) ||
        !IsValidCreaturePrototype(second_prototype, false, &second_subtype) ||
        compound_creature::IsUncompoundable(first_ref->item_id_) ||
        compound_creature::IsUncompoundable(second_ref->item_id_) ||
        first_ref->get_add_info() == second_ref->get_add_info() ||
        !IsLiveCreatureRecord(manager, *first_ref, first_slot) ||
        !IsLiveCreatureRecord(manager, *second_ref, second_slot) ||
        IsCreatureExpired(manager, first_ref->get_add_info()) ||
        IsCreatureExpired(manager, second_ref->get_add_info())) {
        SendCompoundError(user, kErrorInvalidCreature);
        return 0;
    }

    int result_item_id = 0;
    bool result_rare = false;
    const int egg_count = (first_subtype == 1 ? 1 : 0) +
                          (second_subtype == 1 ? 1 : 0);
    if (!compound_creature::SelectResult(egg_count,
                                          result_item_id,
                                          result_rare)) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }
    CItem* const result_prototype =
        G_CDataManager()->find_item(result_item_id);
    if (!IsValidCreaturePrototype(result_prototype, true, 0)) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }

    Inven_Item result_item;
    result_item.item_id_ = result_item_id;
    static_cast<CEquipItem*>(result_prototype)->make_item(result_item);
    if (result_item.item_category_ != 5u ||
        !user_creature::CCreatureItemConverter::SetEggItem(&result_item) ||
        result_item.item_id_ != result_item_id) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }

    // Shadow the exact three deletes and deterministic reuse of the first
    // creature slot before allocating any DB request or mutating live state.
    CInventory shadow;
    InventoryMemory shadow_memory;
    shadow.SetInventoryMemory(&shadow_memory);
    shadow.setCopy(*user->getCurCharacInvenRefR());
    if (shadow.delete_item(kMaterialInventory,
                           material_slot,
                           1,
                           kCompoundDeleteReason,
                           false) != 1 ||
        shadow.delete_item(kCreatureInventory,
                           first_slot,
                           1,
                           kCompoundDeleteReason,
                           false) != 1 ||
        shadow.delete_item(kCreatureInventory,
                           second_slot,
                           1,
                           kCompoundDeleteReason,
                           false) != 1 ||
        shadow.insertCreatureIntoSpecificSlot(result_item,
                                               first_slot,
                                               kCompoundAddReason,
                                               false) != first_slot) {
        SendCompoundError(user, kErrorMissingItem);
        return 0;
    }

    PendingCompound pending;
    pending.charac_no = user->getCurCharacNo();
    pending.material_slot = material_slot;
    pending.first_slot = first_slot;
    pending.second_slot = second_slot;
    pending.first_uid = first_ref->get_add_info();
    pending.second_uid = second_ref->get_add_info();
    pending.result_item_id = result_item_id;
    pending.result_slot = first_slot;
    pending.result_rare = result_rare;
    pending.result_duration_days = result_prototype->getUsablePeriod();
    pending.result_delete_tick = result_prototype->getExpirationDate();
    pending.material_snapshot = *material_ref;
    pending.first_snapshot = *first_ref;
    pending.second_snapshot = *second_ref;
    pending.result_item = result_item;
    if (!BeginPending(user, pending)) {
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }

    user_creature::CDBMsgSender sender(static_cast<ENUM_INTERNALPACKET>(135),
                                       user);
    if (sender.InsertCreatureItem(user,
                                  &pending.result_item,
                                  pending.result_slot,
                                  8,
                                  manager->GetCreatureItemSize(),
                                  pending.result_duration_days,
                                  pending.result_delete_tick) != 1) {
        ErasePending(user->GetUID(), pending.token);
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }
    SIG_CREATURE_ITEM* const insert =
        sender.GetInBuffer<SIG_CREATURE_ITEM>();
    if (!insert) {
        ErasePending(user->GetUID(), pending.token);
        SendCompoundError(user, kErrorInvalidRequest);
        return 0;
    }
    insert->update_kind = compound_creature::kInsertUpdateKind;
    insert->previous_charac_no = pending.token;
    if (sender.Send() != 1) {
        ErasePending(user->GetUID(), pending.token);
        SendCompoundError(user, kErrorInvalidRequest);
    }
    // Success is intentionally deferred until signal136 supplies a durable
    // creature_items.ui_id.  No source record has changed at this point.
    return 0;
}

int Dispatcher_CompoundCreature::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_CompoundCreature::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

bool HandleCompoundCreatureInsertResult(CUser* user,
                                        SIG_CREATURE_ITEM* signal) {
    if (!signal ||
        signal->update_kind != compound_creature::kInsertUpdateKind) {
        return false;
    }

    const uint32_t token = signal->previous_charac_no;
    PendingCompound pending;
    if (!AcquirePendingForResult(user, token, pending)) {
        // Timed-out, duplicate, or session-mismatched callback: the result
        // row was never exposed in inventory, so delete it and consume none.
        (void)QueueNewRowCompensation(user, signal);
        return true;
    }

    const int user_uid = user ? user->GetUID() : 0;
    if (!user || signal->detail.ui_id == 0 ||
        static_cast<int>(signal->detail.charac_no) != pending.charac_no ||
        static_cast<int>(signal->detail.slot) != pending.result_slot ||
        static_cast<int>(signal->inven_item_copy.item_id) !=
            pending.result_item_id) {
        (void)QueueNewRowCompensation(user, signal);
        if (user) SendCompoundError(user, kErrorInvalidRequest);
        ErasePending(user_uid, token);
        return true;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    user_creature::CCreatureMgr* const manager =
        inventory ? inventory->GetCreatureMgrW() : 0;
    if (!RevalidatePending(user, inventory, manager, pending)) {
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorInvalidCreature);
        ErasePending(user_uid, token);
        return true;
    }

    // Allocate both old-row update streams before touching memory.  They use
    // the existing signal138/stat=1 owner and deliberately suppress its mail
    // ownership branch with update_kind=0.
    user_creature::CDBMsgSender delete_first(
        static_cast<ENUM_INTERNALPACKET>(138), user);
    user_creature::CDBMsgSender delete_second(
        static_cast<ENUM_INTERNALPACKET>(138), user);
    if (!PrepareSoftDelete(delete_first,
                           user,
                           pending.first_snapshot,
                           pending.charac_no,
                           pending.first_slot,
                           manager->GetCreatureItemSize()) ||
        !PrepareSoftDelete(delete_second,
                           user,
                           pending.second_snapshot,
                           pending.charac_no,
                           pending.second_slot,
                           manager->GetCreatureItemSize())) {
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorInvalidRequest);
        ErasePending(user_uid, token);
        return true;
    }

    if (!RevalidatePending(user, inventory, manager, pending)) {
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorInvalidCreature);
        ErasePending(user_uid, token);
        return true;
    }

    if (inventory->delete_item(kMaterialInventory,
                               pending.material_slot,
                               1,
                               kCompoundDeleteReason,
                               true) != 1 ||
        inventory->delete_item(kCreatureInventory,
                               pending.first_slot,
                               1,
                               kCompoundDeleteReason,
                               true) != 1 ||
        inventory->delete_item(kCreatureInventory,
                               pending.second_slot,
                               1,
                               kCompoundDeleteReason,
                               true) != 1) {
        RestoreSources(inventory, pending);
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorInvalidCreature);
        ErasePending(user_uid, token);
        return true;
    }

    if (inventory->insertCreatureIntoSpecificSlot(pending.result_item,
                                                   pending.result_slot,
                                                   kCompoundAddReason,
                                                   true) !=
        pending.result_slot) {
        RestoreSources(inventory, pending);
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorMissingItem);
        ErasePending(user_uid, token);
        return true;
    }

    const int expire_time = pending.result_duration_days != 0 ||
                            pending.result_delete_tick == 0
        ? 86400 * pending.result_duration_days
        : pending.result_delete_tick -
              (GlobalData::system_time()
                   ? GlobalData::system_time()->getCurSec()
                   : 0);
    manager->setTempExpireTime(expire_time);
    if (manager->InsertCreatureItemResult(signal) != 1 ||
        manager->ConsumeCompoundCreatureItems(pending.first_uid,
                                              pending.second_uid) != 1) {
        manager->setTempExpireTime(-1);
        user_creature::CCreatureItem* const inserted =
            const_cast<user_creature::CCreatureItem*>(
                manager->FindCreatureItem(signal->detail.ui_id));
        if (inserted && inserted->GetSlotNo() == pending.result_slot &&
            inserted->GetItemId() == pending.result_item_id) {
            (void)manager->UnregisterCreatureItem(signal->detail.ui_id);
        }
        RestoreSources(inventory, pending);
        (void)QueueNewRowCompensation(user, signal);
        SendCompoundError(user, kErrorInvalidRequest);
        ErasePending(user_uid, token);
        return true;
    }

    // RECONSTRUCTED_NO_XA: these existing owners persist to MyISAM through
    // independent async queues.  All synchronous mutation is compensated,
    // but a process/DB crash between these three writes remains observable.
    (void)delete_first.Send();
    (void)delete_second.Send();
    (void)user->SaveInventory();

    SendCompoundSuccess(user, pending);
    manager->SendCreatureItemInfo(signal);
    ErasePending(user_uid, token);
    return true;
}

namespace user_creature {

int CCreatureMgr::ConsumeCompoundCreatureItems(int first_uid,
                                               int second_uid) {
    if (first_uid <= 0 || second_uid <= 0 || first_uid == second_uid) {
        return 0;
    }
    CCreatureItemMap::iterator first = creature_items_.find(first_uid);
    CCreatureItemMap::iterator second = creature_items_.find(second_uid);
    if (first == creature_items_.end() || second == creature_items_.end() ||
        !first->second || !second->second || first->second == second->second) {
        return 0;
    }

    CCreatureItem* const first_item = first->second;
    CCreatureItem* const second_item = second->second;
    creature_items_.erase(first);
    creature_items_.erase(second);
    GetInstanceCreatureItemFactory()->FreeCreatureItem(first_item);
    GetInstanceCreatureItemFactory()->FreeCreatureItem(second_item);
    return 1;
}

}  // namespace user_creature
