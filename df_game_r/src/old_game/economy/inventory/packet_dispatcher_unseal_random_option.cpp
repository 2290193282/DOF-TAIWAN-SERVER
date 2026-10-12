#include "old_game/economy/inventory/packet_dispatcher_unseal_random_option.h"

#include <map>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/statistc_proxy_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_UnsealRandomOption::dispatch_sig(CUser*, PacketBuf&)";

const ENUM_CMDPACKET kCmdUnsealRandomOption =
    ENUM_CMDPACKET_UNSEAL_RANDOM_OPTION;

enum {
    kInventoryType = 1,
    kItemNotFoundError = 17,
    kRandomOptionAlreadyPresentError = 19,
    kNoScrollQuantityError = 22,
    kGoldInsufficientError = 10,
    kItemLockedError = 213,
    kMoneySubReason = 44,
    kScrollUseReason = 1,
};

int make_break_seal_cost_key(const CItem* item) {
    const uint32_t rarity = static_cast<uint8_t>(item->get_rarity());
    const uint32_t random_option_grade = static_cast<uint8_t>(
        item->vtable_->get_random_option_grade(item));
    const uint32_t usable_level = static_cast<uint16_t>(item->getUsableLevel());
    return static_cast<int>(rarity | (random_option_grade << 8) |
                            (usable_level << 16));
}

}  // namespace

// IDA_EA: 0x859771A
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOptionC2Ev
// DEMANGLED: Dispatcher_UnsealRandomOption::Dispatcher_UnsealRandomOption(void)
Dispatcher_UnsealRandomOption::Dispatcher_UnsealRandomOption()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824009C
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOptionD2Ev
// IDA_EA: 0x82400CC
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOptionD0Ev
// DEMANGLED: Dispatcher_UnsealRandomOption::~Dispatcher_UnsealRandomOption()
Dispatcher_UnsealRandomOption::~Dispatcher_UnsealRandomOption() {}

// IDA_EA: 0x8228E5E
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOption12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UnsealRandomOption::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UnsealRandomOption::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || user->getCurCharacR() == 0 ||
        user->CheckInTrade()) {
        return 0;
    }

    int16_t target_slot = 0;
    if (packet.get_short(target_slot) != 1) {
        return LineFunc(59004, kDispatchSignature, 0, 0);
    }
    uint16_t scroll_slot = 0;
    if (packet.get_short(scroll_slot) != 1) {
        return LineFunc(59009, kDispatchSignature, 0, 0);
    }

    if (user->CheckItemLock(kInventoryType, target_slot)) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kItemLockedError);
        return 0;
    }

    CInventory* const write_inventory = user->getCurCharacInvenW();
    Inven_Item* const target =
        write_inventory->GetInvenRef(kInventoryType, target_slot);
    CItem* const item = target ? G_CDataManager()->find_item(target->getKey())
                               : 0;
    if (!target || !item) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kItemNotFoundError);
        return 0;
    }

    if (item->get_rarity() > 2 || target->GetUpgrade() > 10) {
        const int security_error = GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(37));
        if (security_error != 0) {
            user->SendCmdErrorPacket(
                kCmdUnsealRandomOption,
                static_cast<uint8_t>(security_error));
            return 0;
        }
    }

    if (!target->chronicle_random_option_.random_option_.empty()) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kRandomOptionAlreadyPresentError);
        return 0;
    }

    const int scroll_error =
        _check_unseal_scroll_item(user, target, scroll_slot);
    if (scroll_error != 0) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            static_cast<uint8_t>(scroll_error));
        return 0;
    }

    // IDA 0x8229175 addresses CDataManager+0x4f88, which is the already
    // constructed RandomOptionScript+0x178 break_seal_cost map.  Its four
    // byte key is not merely rarity: it packs rarity, PVF random-option grade
    // and usable level exactly as importRandomOptionScript's [break seal cost]
    // parser stores it.
    RandomOptionScript* const random_option_script =
        random_option::GetRandomOption();
    const std::map<int, int>::const_iterator cost =
        random_option_script->break_seal_cost.find(
            make_break_seal_cost_key(item));
    if (cost == random_option_script->break_seal_cost.end()) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kItemNotFoundError);
        return 0;
    }

    if (scroll_slot == 0xffffu) {
        if (cost->second > user->getCurCharacInvenR()->get_money()) {
            user->SendCmdErrorPacket(
                kCmdUnsealRandomOption,
                kGoldInsufficientError);
            return 0;
        }
    } else if (user->getCurCharacInvenR()
                   ->GetInvenSlot(kInventoryType, scroll_slot)
                   .get_add_info() <= 0) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kNoScrollQuantityError);
        return 0;
    }

    if (GlobalData::g_randomOptionHandle->give_option(
            target->getKey(),
            static_cast<ENUM_RARITY>(item->get_rarity()),
            item->getUsableLevel(),
            item->getItemGroupName(),
            item->vtable_->get_random_option_grade(item),
            &target->chronicle_random_option_.random_option_) != 1) {
        user->SendCmdErrorPacket(
            kCmdUnsealRandomOption,
            kRandomOptionAlreadyPresentError);
        return 0;
    }

    statistc_proxy::add(user, "log_random_option", "unseal_cnt", 1,
                         "%d,'%s'", user->get_charac_10_level_section(),
                         GlobalData::s_systemTime_.getCurDate());
    user->history_log().RandomOption(
        *target, static_cast<randomOptionReason::T>(0));

    if (scroll_slot == 0xffffu) {
        statistc_proxy::add(user, "log_random_option", "unseal_gold",
                             static_cast<unsigned int>(cost->second),
                             "%d,'%s'", user->get_charac_10_level_section(),
                             GlobalData::s_systemTime_.getCurDate());
        if (write_inventory->use_money(
                cost->second,
                static_cast<eMoneySubReason>(kMoneySubReason), true) != 1) {
            user->SendCmdErrorPacket(
                kCmdUnsealRandomOption,
                kGoldInsufficientError);
            return 0;
        }
    } else {
        // The old use_item ABI returns zero on success.  Keep that inverted
        // condition and its post-mutation CMD401/22 error path intact.
        if (write_inventory->use_item(scroll_slot, 1, kScrollUseReason) != 0) {
            user->SendCmdErrorPacket(
                kCmdUnsealRandomOption,
                kNoScrollQuantityError);
            return 0;
        }
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), scroll_slot);
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);
    user->SendCmdOkPacket(
        kCmdUnsealRandomOption);

    PacketGuard item_packet;
    item_packet.put_header(0, 14);
    item_packet.put_byte(0);
    item_packet.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(kInventoryType), target_slot, item_packet);
    item_packet.finalize(true);
    user->Send(item_packet);
    return 0;
}

// IDA_EA: 0x8229644
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOption25_check_unseal_scroll_itemEP5CUserPK10Inven_Itemt
// DEMANGLED: Dispatcher_UnsealRandomOption::_check_unseal_scroll_item(CUser *,Inven_Item const*,unsigned short)
int Dispatcher_UnsealRandomOption::_check_unseal_scroll_item(
    CUser* user,
    const Inven_Item* target,
    uint16_t scroll_slot) {
    if (scroll_slot == 0xffffu) {
        return 0;
    }

    const Inven_Item scroll =
        user->getCurCharacInvenR()->GetInvenSlot(kInventoryType, scroll_slot);
    CItem* const scroll_item = G_CDataManager()->find_item(scroll.getKey());
    if (!scroll_item) {
        return 21;
    }
    if (scroll_item->is_stackable() != 1) {
        return kItemNotFoundError;
    }

    // The old body reads the dword at CStackableItem+0x328, then passes its
    // low byte to ExpertJob type 2.  CStackablePrivateStoreInfo is the typed
    // owner of this PVF-fed segment; keeping the byte conversion here avoids
    // reopening a raw CStackableItem overlay in the dispatcher.
    const CStackableItem* const stackable_scroll =
        static_cast<const CStackableItem*>(scroll_item);
    const int scroll_index = static_cast<unsigned char>(
        stackable_scroll->private_store_info_.expert_job_value_3);
    if (stackable_scroll->private_store_info_.expert_job_value_3 == -1) {
        return 1;
    }

    STExpertJobScript* const expert_job =
        G_CDataManager()->GetExpertJobScript(2);
    const int possible_level =
        expert_job->GetUnsealScrollPossibleLevel(scroll_index);
    if (possible_level == -1) {
        return 1;
    }

    CItem* const target_item = G_CDataManager()->find_item(target->getKey());
    return target_item->getUsableLevel() <= possible_level
               ? 0
               : kItemNotFoundError;
}

// IDA_EA: 0x823432A
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UnsealRandomOption::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UnsealRandomOption::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234334
// IDA_NAME: _ZN29Dispatcher_UnsealRandomOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UnsealRandomOption::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UnsealRandomOption::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
