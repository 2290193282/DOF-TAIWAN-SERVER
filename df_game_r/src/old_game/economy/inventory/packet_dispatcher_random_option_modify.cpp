#include "old_game/economy/inventory/packet_dispatcher_random_option_modify.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistc_proxy_abi.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const char kChangeReadSignature[] =
    "virtual int Dispatcher_ChangeRandomOption::read(PacketBuf&, MSG_BASE&)";
const char kResetReadSignature[] =
    "virtual int DispatcherResetRandomOption::read(PacketBuf&, MSG_BASE&)";

const ENUM_CMDPACKET kChangeRandomOptionCmd =
    ENUM_CMDPACKET_CHANGE_RANDOM_OPTION;
const ENUM_CMDPACKET kResetRandomOptionCmd =
    ENUM_CMDPACKET_RESET_RANDOM_OPTION;

enum {
    kInventoryType = 1,
    kItemNotFoundError = 17,
    kInvalidRandomOptionError = 19,
    kNoResetMaterialError = 21,
    kNoMaterialQuantityError = 22,
    kGoldInsufficientError = 10,
    kInvalidPendingChangeError = 4,
    kAuthMobileRestrictedError = 209,
    kItemLockedError = 213,
    kMoneySubReason = 44,
};

bool is_reset_material(const Inven_Item& item) {
    const int item_index = item.getKey();
    return item.get_add_info() > 0 &&
           (item_index == 15 || item_index == 897 || item_index == 690000114);
}

}  // namespace

// IDA_EA: 0x822E012
// IDA_NAME: _ZN24MSG_CHANGE_RANDOM_OPTIONC2Ev
// DEMANGLED: MSG_CHANGE_RANDOM_OPTION::MSG_CHANGE_RANDOM_OPTION(void)
MSG_CHANGE_RANDOM_OPTION::MSG_CHANGE_RANDOM_OPTION()
    : MSG_BASE(), item_slot(0), payment_slot(0), option_number(0) {}

// IDA_EA: 0x822E03E
// IDA_NAME: _ZN23MSG_RESET_RANDOM_OPTIONC2Ev
// DEMANGLED: MSG_RESET_RANDOM_OPTION::MSG_RESET_RANDOM_OPTION(void)
MSG_RESET_RANDOM_OPTION::MSG_RESET_RANDOM_OPTION()
    : MSG_BASE(), item_slot(0), reset_item_slot(0), option_number(0) {}

// IDA_EA: 0x859794A
// IDA_NAME: _ZN29Dispatcher_ChangeRandomOptionC2Ev
// DEMANGLED: Dispatcher_ChangeRandomOption::Dispatcher_ChangeRandomOption(void)
Dispatcher_ChangeRandomOption::Dispatcher_ChangeRandomOption()
    : IPacketDispatcher<MSG_CHANGE_RANDOM_OPTION,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244836
// IDA_NAME: _ZN29Dispatcher_ChangeRandomOptionD2Ev
// DEMANGLED: Dispatcher_ChangeRandomOption::~Dispatcher_ChangeRandomOption()
// IDA_EA: 0x8244866
// IDA_NAME: _ZN29Dispatcher_ChangeRandomOptionD0Ev
// DEMANGLED: Dispatcher_ChangeRandomOption::~Dispatcher_ChangeRandomOption()
Dispatcher_ChangeRandomOption::~Dispatcher_ChangeRandomOption() {}

// IDA_EA: 0x81DC8DE
// IDA_NAME: _ZN29Dispatcher_ChangeRandomOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeRandomOption::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeRandomOption::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    MSG_CHANGE_RANDOM_OPTION& request =
        static_cast<MSG_CHANGE_RANDOM_OPTION&>(message);
    if (packet.get_short(request.item_slot) != 1) {
        return LineFunc(19672, kChangeReadSignature, 0, 0);
    }
    if (packet.get_short(request.payment_slot) != 1) {
        return LineFunc(19675, kChangeReadSignature, 0, 0);
    }
    if (packet.get_byte(request.option_number) == 1) {
        return 0;
    }
    return LineFunc(19678, kChangeReadSignature, 0, 0);
}

// IDA_EA: 0x81DC9BA
// IDA_NAME: _ZN29Dispatcher_ChangeRandomOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeRandomOption::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeRandomOption::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)param;
    MSG_CHANGE_RANDOM_OPTION& request =
        static_cast<MSG_CHANGE_RANDOM_OPTION&>(message);
    if (user->get_state() <= 2 || user->getCurCharacR() == 0 ||
        user->CheckInTrade()) {
        return 0;
    }

    if (user->CheckItemLock(kInventoryType, request.item_slot)) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kItemLockedError);
        return 0;
    }

    CInventory* const write_inventory = user->getCurCharacInvenW();
    Inven_Item* const target =
        write_inventory->GetInvenRef(kInventoryType, request.item_slot);
    CItem* const item = target ? G_CDataManager()->find_item(target->getKey())
                               : 0;
    if (!target || !item) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kItemNotFoundError);
        return 0;
    }
    // The old byte-addressed test precedes the selector-range test.  Express
    // its defined 0..2 domain through the typed three-entry owner; malformed
    // selectors still take the later original CMD438/19 branch.
    if (request.option_number <= 2 &&
        target->chronicle_random_option_.random_option_
                .option_fields_[request.option_number].option_index_ == 0) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kItemNotFoundError);
        return 0;
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 15) != 0) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kAuthMobileRestrictedError);
        return 0;
    }
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(37));
    if (security_error != 0) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            static_cast<unsigned char>(security_error));
        return 0;
    }

    RandomOption& option = target->chronicle_random_option_.random_option_;
    if (option.empty() || request.option_number > 2) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kInvalidRandomOptionError);
        return 0;
    }
    if (!option.changed_option_.empty() &&
        (option.changed_option_seed_.seed_ & 3u) != request.option_number) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kInvalidPendingChangeError);
        return 0;
    }

    RandomOptionScript* const script = random_option::GetRandomOption();
    const int cost = script->getOptionModificationCost(
        static_cast<short>(item->getUsableLevel()),
        static_cast<ENUM_RARITY>(item->get_rarity()));
    if (cost == -1) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kItemNotFoundError);
        return 0;
    }

    // The original body deliberately discards the client-supplied second
    // slot before payment selection.  CMD438 is therefore gold-only even
    // though the wire request still carries that historical field.
    request.payment_slot = 0xffffu;
    if (request.payment_slot == 0xffffu &&
        user->getCurCharacInvenR()->get_money() < cost) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kGoldInsufficientError);
        return 0;
    }

    static random_option::CRandomOptionItemHandle handle;
    handle.set_random_option_give(&script->modified_give);
    if (handle.change_option(
            target->getKey(), static_cast<ENUM_RARITY>(item->get_rarity()),
            item->getUsableLevel(), item->getItemGroupName(),
            item->vtable_->get_random_option_grade(item), request.option_number,
            &option) != 1) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kInvalidRandomOptionError);
        return 0;
    }

    user->history_log().RandomOption(
        *target, static_cast<randomOptionReason::T>(2));
    statistc_proxy::add(user, "log_random_option", "change_cnt", 1,
                         "%d,'%s'", user->get_charac_10_level_section(),
                         GlobalData::s_systemTime_.getCurDate());
    statistc_proxy::add(user, "log_random_option", "change_gold", cost,
                         "%d,'%s'", user->get_charac_10_level_section(),
                         GlobalData::s_systemTime_.getCurDate());
    if (write_inventory->use_money(cost,
                                   static_cast<eMoneySubReason>(kMoneySubReason),
                                   true) != 1) {
        user->SendCmdErrorPacket(
            kChangeRandomOptionCmd,
            kGoldInsufficientError);
        return 0;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);
    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(kInventoryType), request.item_slot, packet);
    packet.finalize(true);
    user->Send(packet);
    user->SendCmdOkPacket(kChangeRandomOptionCmd);
    return 0;
}

// IDA_EA: 0x8597966
// IDA_NAME: _ZN27DispatcherResetRandomOptionC2Ev
// DEMANGLED: DispatcherResetRandomOption::DispatcherResetRandomOption(void)
DispatcherResetRandomOption::DispatcherResetRandomOption()
    : IPacketDispatcher<MSG_RESET_RANDOM_OPTION,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82447E8
// IDA_NAME: _ZN27DispatcherResetRandomOptionD2Ev
// DEMANGLED: DispatcherResetRandomOption::~DispatcherResetRandomOption()
// IDA_EA: 0x8244818
// IDA_NAME: _ZN27DispatcherResetRandomOptionD0Ev
// DEMANGLED: DispatcherResetRandomOption::~DispatcherResetRandomOption()
DispatcherResetRandomOption::~DispatcherResetRandomOption() {}

// IDA_EA: 0x81DD1E2
// IDA_NAME: _ZN27DispatcherResetRandomOption4readER9PacketBufR8MSG_BASE
// DEMANGLED: DispatcherResetRandomOption::read(PacketBuf &,MSG_BASE &)
int DispatcherResetRandomOption::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_RESET_RANDOM_OPTION& request =
        static_cast<MSG_RESET_RANDOM_OPTION&>(message);
    if (packet.get_short(request.item_slot) != 1) {
        return LineFunc(19890, kResetReadSignature, 0, 0);
    }
    if (packet.get_short(request.reset_item_slot) != 1) {
        return LineFunc(19893, kResetReadSignature, 0, 0);
    }
    if (packet.get_short(request.option_number) == 1) {
        return 0;
    }
    return LineFunc(19896, kResetReadSignature, 0, 0);
}

// IDA_EA: 0x81DD2BE
// IDA_NAME: _ZN27DispatcherResetRandomOption7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DispatcherResetRandomOption::process(CUser *,MSG_BASE &,ParamBase &)
int DispatcherResetRandomOption::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)param;
    MSG_RESET_RANDOM_OPTION& request =
        static_cast<MSG_RESET_RANDOM_OPTION&>(message);
    if (user->get_state() != 3 || user->getCurCharacR() == 0 ||
        user->CheckInTrade()) {
        return 0;
    }

    if (user->CheckItemLock(kInventoryType, request.item_slot)) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kItemLockedError);
        return 0;
    }

    CInventory* const write_inventory = user->getCurCharacInvenW();
    Inven_Item* const target =
        write_inventory->GetInvenRef(kInventoryType, request.item_slot);
    CItem* const item = target ? G_CDataManager()->find_item(target->getKey())
                               : 0;
    if (!target || !item) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kItemNotFoundError);
        return 0;
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 15) != 0) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kAuthMobileRestrictedError);
        return 0;
    }
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(38));
    if (security_error != 0) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            static_cast<unsigned char>(security_error));
        return 0;
    }

    RandomOption& option = target->chronicle_random_option_.random_option_;
    if (option.empty() || request.option_number > 2) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kInvalidRandomOptionError);
        return 0;
    }

    const Inven_Item reset_item = user->getCurCharacInvenR()->GetInvenSlot(
        kInventoryType, request.reset_item_slot);
    if (!is_reset_material(reset_item)) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kNoResetMaterialError);
        return 0;
    }

    static random_option::CRandomOptionItemHandle handle;
    const int reset_error = handle.reset_option(
        target->getKey(), static_cast<ENUM_RARITY>(item->get_rarity()),
        request.option_number, &option);
    if (reset_error != 0) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            static_cast<unsigned char>(reset_error));
        return 0;
    }

    user->history_log().RandomOption(
        *target, static_cast<randomOptionReason::T>(3));
    statistc_proxy::add(user, "log_random_option", "reset_cnt", 1,
                         "%d,'%s'", user->get_charac_10_level_section(),
                         GlobalData::s_systemTime_.getCurDate());
    if (write_inventory->use_item(request.reset_item_slot, 1, 1) != 0) {
        user->SendCmdErrorPacket(
            kResetRandomOptionCmd,
            kNoMaterialQuantityError);
        return 0;
    }

    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0), request.item_slot);
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0),
                         request.reset_item_slot);
    user->SendCmdOkPacket(kResetRandomOptionCmd);
    return 0;
}

// IDA_EA: 0x824DC74 / 0x824DDD0 / 0x824DDDA / 0x824DE6C /
//         0x825BC86 / 0x825BCB6 / 0x8598DDA
template class IPacketDispatcher<MSG_CHANGE_RANDOM_OPTION,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x824DA76 / 0x824DBD2 / 0x824DBDC / 0x824DC6E /
//         0x825BC38 / 0x825BC68 / 0x8598DF6
template class IPacketDispatcher<MSG_RESET_RANDOM_OPTION,
                                 ParamBase,
                                 static_cast<ch_state>(0)>;
