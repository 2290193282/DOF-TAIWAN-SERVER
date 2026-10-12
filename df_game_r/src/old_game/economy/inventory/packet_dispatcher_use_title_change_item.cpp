#include "old_game/economy/inventory/packet_dispatcher_use_title_change_item.h"

#include <stddef.h>
#include <stdint.h>

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"


namespace {

const INVEN_TYPE kTitleChangeInventory = static_cast<INVEN_TYPE>(1);
const ENUM_ITEMSPACE kTitleChangeItemSpace = static_cast<ENUM_ITEMSPACE>(0);
const eItemDelReason kTitleChangeMaterialDeleteReason =
    static_cast<eItemDelReason>(3);
const int32_t kTitleChangeMaterialItemId = 690001386;

const uint8_t kErrorMissingItem = 4;
const uint8_t kErrorInvalidItem = 17;
const uint8_t kErrorOnlyEquipment = 19;
const uint8_t kErrorItemLocked = 213;

struct TitleChangeRule {
    int32_t source_item_id;
    int32_t success_item_id;
    int32_t failure_item_id;
    uint16_t success_rate;
};

// PVF etc/aradtitlechange_main.etc supplies the per-source 10000-base rates;
// etc/aradtitlechange_sub.etc supplies both result lists.  The second final
// ID in aradtitlechange_main.etc is mistyped as 600330054, while the complete
// sub chain and the actual title prototypes resolve it as 600330064.  Final
// titles are intentionally absent: the client rejects them before CMD672.
const TitleChangeRule kTitleChangeRules[] = {
    {600330057, 600330058, 600330057, 5000},
    {600330058, 600330059, 600330058, 2500},
    {600330059, 600330060, 600330059, 1000},
    {600330061, 600330062, 600330061, 5000},
    {600330062, 600330063, 600330062, 2500},
    {600330063, 600330064, 600330063, 1000},
    {600330065, 600330066, 600330065, 5000},
    {600330066, 600330067, 600330066, 2500},
    {600330067, 600330068, 600330067, 1000},
    {600330069, 600330070, 600330069, 5000},
    {600330070, 600330071, 600330070, 2500},
    {600330071, 600330072, 600330071, 1000},
    {600330073, 600330074, 600330073, 5000},
    {600330074, 600330075, 600330074, 2500},
    {600330075, 600330076, 600330075, 1000},
};

const TitleChangeRule* FindTitleChangeRule(int32_t source_item_id) {
    for (size_t index = 0;
         index < sizeof(kTitleChangeRules) / sizeof(kTitleChangeRules[0]);
         ++index) {
        if (kTitleChangeRules[index].source_item_id == source_item_id) {
            return &kTitleChangeRules[index];
        }
    }
    return 0;
}

void SendTitleChangeError(CUser* user, uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_TITLE_CHANGE_ITEM, error);
    }
}

bool IsTitlePrototype(CItem* item) {
    return item && !item->is_stackable() && item->vtable_ &&
           item->vtable_->get_item_type(item) ==
               oldsrv::config::kEquipmentTypeTitle;
}

}  // namespace


Dispatcher_UseTitleChangeItem::Dispatcher_UseTitleChangeItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_UseTitleChangeItem::~Dispatcher_UseTitleChangeItem() {}

int Dispatcher_UseTitleChangeItem::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    uint16_t material_slot = 0;
    uint16_t target_slot = 0;
    if (packet.get_short(material_slot) != 1 ||
        packet.get_short(target_slot) != 1) {
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: sub_9AC5F0 writes exactly the two u16 slots.
    // The title-change window is a town inventory action; preserve the same
    // mutation guards used by the other ordinary-inventory item dispatchers.
    // GetInstancePrivateStoreMgr owns a function-local static manager and
    // therefore cannot return null during this guard.
    if (!user || user->get_state() != 3 || user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user) ||
        !user->getCurCharacR()) {
        SendTitleChangeError(user, kErrorOnlyEquipment);
        return 0;
    }
    if (material_slot == target_slot) {
        SendTitleChangeError(user, kErrorInvalidItem);
        return 0;
    }

    const CInventory* const readable_inventory = user->getCurCharacInvenR();
    if (readable_inventory->CheckValidSlot(
            static_cast<int>(kTitleChangeInventory),
            static_cast<int>(material_slot)) != 1 ||
        readable_inventory->CheckValidSlot(
            static_cast<int>(kTitleChangeInventory),
            static_cast<int>(target_slot)) != 1) {
        SendTitleChangeError(user, kErrorMissingItem);
        return 0;
    }
    const Inven_Item* const material = readable_inventory->GetInvenRef(
        kTitleChangeInventory, static_cast<int>(material_slot));
    const Inven_Item* const target = readable_inventory->GetInvenRef(
        kTitleChangeInventory, static_cast<int>(target_slot));
    if (!material || material->item_id_ != kTitleChangeMaterialItemId ||
        material->get_add_info() < 1 || !target || !target->item_id_) {
        SendTitleChangeError(user, kErrorMissingItem);
        return 0;
    }

    CItem* const material_prototype =
        G_CDataManager()->find_item(kTitleChangeMaterialItemId);
    if (!material_prototype || !material_prototype->is_stackable()) {
        SendTitleChangeError(user, kErrorInvalidItem);
        return 0;
    }

    const TitleChangeRule* const rule =
        FindTitleChangeRule(target->item_id_);
    if (!rule) {
        SendTitleChangeError(user, kErrorInvalidItem);
        return 0;
    }
    CItem* const source_prototype =
        G_CDataManager()->find_item(target->item_id_);
    if (target->item_category_ != 1 || !IsTitlePrototype(source_prototype)) {
        SendTitleChangeError(user, kErrorOnlyEquipment);
        return 0;
    }
    if (user->CheckItemLock(kTitleChangeInventory,
                            static_cast<int>(material_slot)) ||
        user->CheckItemLock(kTitleChangeInventory,
                            static_cast<int>(target_slot))) {
        SendTitleChangeError(user, kErrorItemLocked);
        return 0;
    }

    const bool succeeded =
        static_cast<int>(get_rand_int(10000)) < rule->success_rate;
    const int32_t result_item_id = succeeded
                                       ? rule->success_item_id
                                       : rule->failure_item_id;
    CItem* const result_prototype = G_CDataManager()->find_item(result_item_id);
    if (!IsTitlePrototype(result_prototype)) {
        SendTitleChangeError(user, kErrorInvalidItem);
        return 0;
    }

    // Both outcome descriptions say the old magic/enchant state disappears.
    // Generate from the result prototype instead of retaining any per-instance
    // fields from the source.  CEquipItem::make_item resets attributes,
    // extension/amplification/random-option state and creates a new identity.
    Inven_Item replacement;
    replacement.item_id_ = result_item_id;
    result_prototype->vtable_->make_item(result_prototype, replacement);
    replacement.item_id_ = result_item_id;
    if (replacement.item_category_ != 1) {
        SendTitleChangeError(user, kErrorOnlyEquipment);
        return 0;
    }

    // Preflight against an isolated inventory copy.  The live update then has
    // no slot/capacity failure point after the material is consumed.
    CInventory shadow_inventory;
    InventoryMemory shadow_memory;
    shadow_inventory.SetInventoryMemory(&shadow_memory);
    shadow_inventory.setCopy(*user->getCurCharacInvenRefR());
    if (shadow_inventory.delete_item(kTitleChangeInventory,
                                     static_cast<int>(material_slot),
                                     1,
                                     kTitleChangeMaterialDeleteReason,
                                     false) != 1 ||
        shadow_inventory.update_item(kTitleChangeInventory,
                                     static_cast<int>(target_slot),
                                     replacement) != 1) {
        SendTitleChangeError(user, kErrorMissingItem);
        return 0;
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    const Inven_Item material_snapshot = *material;
    if (writable_inventory->delete_item(kTitleChangeInventory,
                                        static_cast<int>(material_slot),
                                        1,
                                        kTitleChangeMaterialDeleteReason,
                                        true) != 1) {
        SendTitleChangeError(user, kErrorMissingItem);
        return 0;
    }
    if (writable_inventory->update_item(kTitleChangeInventory,
                                        static_cast<int>(target_slot),
                                        replacement) != 1) {
        // This cannot occur after the shadow preflight in the single-threaded
        // user transaction, but restore the consumed stack if the invariant is
        // ever broken by a later inventory implementation.
        writable_inventory->update_item(kTitleChangeInventory,
                                        static_cast<int>(material_slot),
                                        material_snapshot);
        SendTitleChangeError(user, kErrorInvalidItem);
        return 0;
    }

    // CLIENT_IDA_EVIDENCE: sub_569AE0 consumes outcome(u8), current title ID
    // (u32), then material ID(u32) after the standard CMD success byte.
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_TITLE_CHANGE_ITEM);
    response.put_byte(1);
    response.put_byte(succeeded ? 1 : 0);
    response.put_int(result_item_id);
    response.put_int(kTitleChangeMaterialItemId);
    response.finalize(true);
    user->Send(response);

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             kTitleChangeItemSpace,
                             static_cast<int>(material_slot));
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         kTitleChangeItemSpace,
                         static_cast<int>(target_slot));
    return 0;
}

int Dispatcher_UseTitleChangeItem::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_UseTitleChangeItem::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
