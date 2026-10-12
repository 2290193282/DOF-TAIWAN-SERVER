#include "old_game/player/core/packet_dispatcher_stackable_action_use.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/stackable_action_runtime.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_StackableActionUse::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_StackableActionUse::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// The old binary has exactly one concrete state-3 generic owner for this
// request/response pair.  Keep it next to the real CMD516 class instead of
// the SR08 aggregate shell.
template class IPacketDispatcher<MSG_STACKABLE_ACTION_USE,
                                 stStackableActionUse,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x826033E
// IDA_NAME: _ZN24MSG_STACKABLE_ACTION_USEC2Ev
// DEMANGLED: MSG_STACKABLE_ACTION_USE::MSG_STACKABLE_ACTION_USE(void)
MSG_STACKABLE_ACTION_USE::MSG_STACKABLE_ACTION_USE()
    : MSG_BASE(),
      slot(0),
      item_space(0),
      inventory_sum(0),
      action_type(0),
      arg3(0),
      arg4(0) {}

// IDA_EA: 0x8261E7C / 0x8261E98 / 0x8261EC8
// IDA_NAME: _ZN20stStackableActionUseC2Ev / D2Ev / D0Ev
stStackableActionUse::stStackableActionUse()
    : ParamBase() {}

stStackableActionUse::~stStackableActionUse() {}

// IDA_EA: 0x825DF72
// IDA_NAME: _ZN29Dispatcher_StackableActionUse4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_StackableActionUse::read(PacketBuf &,MSG_BASE &)
int Dispatcher_StackableActionUse::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_STACKABLE_ACTION_USE& request =
        static_cast<MSG_STACKABLE_ACTION_USE&>(message);
    int16_t slot = 0;
    if (packet.get_short(slot) != 1) {
        return LineFunc(251, kReadSignature, 0, 0);
    }
    request.slot = slot;
    if (packet.get_byte(request.item_space) != 1) {
        return LineFunc(254, kReadSignature, 0, 0);
    }
    uint32_t inventory_sum = 0;
    if (packet.get_int(inventory_sum) != 1) {
        return LineFunc(257, kReadSignature, 0, 0);
    }
    request.inventory_sum = inventory_sum;
    if (packet.get_byte(request.action_type) != 1) {
        return LineFunc(260, kReadSignature, 0, 0);
    }
    uint32_t arg3 = 0;
    if (packet.get_int(arg3) != 1) {
        return LineFunc(263, kReadSignature, 0, 0);
    }
    request.arg3 = arg3;
    uint32_t arg4 = 0;
    if (packet.get_int(arg4) != 1) {
        return LineFunc(266, kReadSignature, 0, 0);
    }
    request.arg4 = arg4;
    return 0;
}

// IDA_EA: 0x825E332
// IDA_NAME: _ZN29Dispatcher_StackableActionUse11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_StackableActionUse::check_error(CUser *,MSG_BASE &)
int Dispatcher_StackableActionUse::check_error(CUser* user, MSG_BASE& message) {
    MSG_STACKABLE_ACTION_USE& request =
        static_cast<MSG_STACKABLE_ACTION_USE&>(message);
    if (!user || !user->getCurCharacR()) {
        return -1;
    }
    if (user->getCurCharacInvenCheckSum() !=
        static_cast<int>(request.inventory_sum)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(0x25b),
            1,
            0,
            0);
    }
    if (user->CheckInTrade()) {
        return 216;
    }
    if (!request.action_type) {
        return 23;
    }

    const Inven_Item inventory_item =
        user->getCurCharacInvenR()->GetInvenSlot(1, request.slot);
    const unsigned int item_index =
        static_cast<unsigned int>(inventory_item.getKey());
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(item_index));
    if (!item) {
        return 17;
    }
    if (!user->verifyStackableItemProcess(
            static_cast<StackableAction::Type>(request.action_type), item)) {
        int action_matches = 0;
        int place_matches = 0;
        int server_matches = 0;
        if (item->is_stackable() == 1) {
            CStackableItem* const stackable =
                static_cast<CStackableItem*>(item);
            action_matches = stackable->verifyStackableAction(
                static_cast<StackableAction::Type>(request.action_type));
            place_matches = stackable->verifyUsablePlace(user);
            server_matches = stackable->verifyUsableServerType();
        }
        LogManager::logFormat(
            1,
            "packet_dispatcher_stackable_action_use.cpp",
            kProcessSignature,
            __LINE__,
            "[STACKABLE_ACTION][VERIFY_FAIL] item=%u slot=%d item_space=%u request_action=%u stackable=%d action_ok=%d place_ok=%d server_ok=%d state=%d village=%d area=%d",
            item_index,
            static_cast<int>(request.slot),
            static_cast<unsigned int>(request.item_space),
            static_cast<unsigned int>(request.action_type),
            item->is_stackable() == 1 ? 1 : 0,
            action_matches,
            place_matches,
            server_matches,
            user->get_state(),
            user->getCurCharacVill(),
            user->get_area(false));
        return 23;
    }
    if (user->CheckCoolTimeItem(item_index)) {
        return 19;
    }
    return user->getCurCharacInvenRefR()->try_use_item(
        request.slot, 1, request.item_space);
}

// IDA_EA: 0x825E11E
// IDA_NAME: _ZN29Dispatcher_StackableActionUse7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_StackableActionUse::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_StackableActionUse::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    MSG_STACKABLE_ACTION_USE& request =
        static_cast<MSG_STACKABLE_ACTION_USE&>(message);
    stStackableActionUse& response = static_cast<stStackableActionUse&>(param);
    response.result_code = check_error(user, message);
    response.item_space = request.item_space;
    response.slot = request.slot;
    if (response.result_code > 0) {
        return 0;
    }
    if (response.result_code < 0) {
        LogManager::logFormat(1,
                               "PacketDispatcher_Impl_2.cpp",
                               kProcessSignature,
                               0x11F,
                               "Dispatcher Check_Error No(%d)",
                               response.result_code);
        return LineFunc(288, kProcessSignature, 0, 0);
    }

    const Inven_Item inventory_item =
        user->getCurCharacInvenR()->GetInvenSlot(1, request.slot);
    const unsigned int item_index =
        static_cast<unsigned int>(inventory_item.getKey());
    CStackableItem* const item = static_cast<CStackableItem*>(
        G_CDataManager()->find_item(static_cast<int>(item_index)));
    if (!item) {
        response.result_code = 17;
        return 0;
    }

    StackableAction::input_param action = {};
    action.user = user;
    action.stackable_item = item;
    response.result_code = GlobalData::g_Actions->check(
        request.action_type, action);
    if (!response.result_code) {
        response.result_code = GlobalData::g_Actions->run(
            request.action_type, action);
    }
    if (response.result_code) {
        LogManager::logFormat(
            1,
            "packet_dispatcher_stackable_action_use.cpp",
            kProcessSignature,
            __LINE__,
            "[STACKABLE_ACTION][ACTION_FAIL] item=%u slot=%d item_space=%u action=%u result=%d",
            item_index,
            static_cast<int>(request.slot),
            static_cast<unsigned int>(request.item_space),
            static_cast<unsigned int>(request.action_type),
            response.result_code);
    }
    if (!response.result_code) {
        response.result_code = user->getCurCharacInvenW()->use_item(
            request.slot, 1, request.item_space);
        if (!response.result_code) {
            user->_processContinuousItem(item_index, item);
        } else {
            LogManager::logFormat(
                1,
                "packet_dispatcher_stackable_action_use.cpp",
                kProcessSignature,
                __LINE__,
                "[STACKABLE_ACTION][CONSUME_FAIL] item=%u slot=%d item_space=%u action=%u result=%d",
                item_index,
                static_cast<int>(request.slot),
                static_cast<unsigned int>(request.item_space),
                static_cast<unsigned int>(request.action_type),
                response.result_code);
        }
    }
    return 0;
}

// IDA_EA: 0x825E4EC
// IDA_NAME: _ZN29Dispatcher_StackableActionUse4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_StackableActionUse::send(CUser *,ParamBase &)
void Dispatcher_StackableActionUse::send(CUser* user, ParamBase& param) {
    stStackableActionUse& response = static_cast<stStackableActionUse&>(param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_USE_STACKABLE_ACTION);
    if (response.result_code) {
        packet.put_byte(0);
        packet.put_byte(response.result_code);
        packet.put_byte(response.item_space);
    } else {
        packet.put_byte(1);
        packet.put_short(response.slot);
        packet.put_byte(response.item_space);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8260B78
// IDA_NAME: _ZN29Dispatcher_StackableActionUseD2Ev
// DEMANGLED: Dispatcher_StackableActionUse::~Dispatcher_StackableActionUse()
// IDA_EA: 0x8260BA8
// IDA_NAME: _ZN29Dispatcher_StackableActionUseD0Ev
// DEMANGLED: Dispatcher_StackableActionUse::~Dispatcher_StackableActionUse()
Dispatcher_StackableActionUse::~Dispatcher_StackableActionUse() {}

// IDA_EA: 0x859583C
// IDA_NAME: _ZN29Dispatcher_StackableActionUseC2Ev
// DEMANGLED: Dispatcher_StackableActionUse::Dispatcher_StackableActionUse(void)
Dispatcher_StackableActionUse::Dispatcher_StackableActionUse()
    : IPacketDispatcher<MSG_STACKABLE_ACTION_USE,
                        stStackableActionUse,
                        static_cast<ch_state>(3)>() {}
