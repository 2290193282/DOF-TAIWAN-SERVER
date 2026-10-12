#include "old_game/player/core/action_point_packet_dispatchers.h"

#include <cstdio>
#include <cstring>

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


namespace APSystem {

// IDA_EA: 0x812238A
// IDA_NAME: _ZN8APSystem9CUserProc33SetTodayActionAndCheckMedalRewardEP5CUser
// DEMANGLED: APSystem::CUserProc::SetTodayActionAndCheckMedalReward(CUser *)
// The original implementation is an empty body.  The CUser argument remains
// part of the public ABI even though no value is consumed.
void CUserProc::SetTodayActionAndCheckMedalReward(CUser* /*user*/) {}

// IDA_EA: 0x8122AAA
// IDA_NAME: _ZN8APSystem30_MSG_ACTION_POINT_ACTION_CLEARC2Ev
// DEMANGLED: APSystem::_MSG_ACTION_POINT_ACTION_CLEAR::_MSG_ACTION_POINT_ACTION_CLEAR(void)
_MSG_ACTION_POINT_ACTION_CLEAR::_MSG_ACTION_POINT_ACTION_CLEAR() {
    action_group_index = static_cast<_ActionGroupIndex>(0);
    action_value_or_reserved = 0;
    action_index = 0;
    clear_state = false;
}

// IDA_EA: 0x8122390
// IDA_NAME: _ZN8APSystem9CUserProc24ClearActionAndSendtoUserEP5CUserNS_17_ActionGroupIndexEi16ENUM_PACKETCLASS
// DEMANGLED: APSystem::CUserProc::ClearActionAndSendtoUser(CUser *,APSystem::_ActionGroupIndex,int,ENUM_PACKETCLASS)
// The old-server target is an empty body.  Its AP/DB behavior must not be
// invented merely to make CMD552 appear end-to-end complete.
void CUserProc::ClearActionAndSendtoUser(CUser*,
                                         _ActionGroupIndex,
                                         int,
                                         ENUM_PACKETCLASS) {}

// IDA_EA: 0x8122398
// IDA_NAME: _ZN8APSystem9CUserProc19SendActionListStateEP5CUser
// DEMANGLED: APSystem::CUserProc::SendActionListState(CUser *)
// Original `neople/game/df_game_r` SHA256:
// dd0ae672b1dce3ff9c6c403beaec71b3ef666b14deb567d4cab0da99d8711f9e.
// ELF32/i386 .symtab records a 28-byte function at this EA.  Its exact
// control flow is `if (user != 0) SendActionListState(*user); return;`;
// xrefs are the Inter_LoadEtc call at 0x84C21DD and the local overload call.
// This is the original pointer overload, not a closure-only forwarding shim.
void CUserProc::SendActionListState(CUser* user) {
    if (user) {
        SendActionListState(*user);
    }
}

// IDA_EA: 0x81223B4
// IDA_NAME: _ZN8APSystem9CUserProc15SendActionClearER5CUsertNS_17_ActionGroupIndexEb16ENUM_PACKETCLASS
// DEMANGLED: APSystem::CUserProc::SendActionClear(CUser &,unsigned short,APSystem::_ActionGroupIndex,bool,ENUM_PACKETCLASS)
void CUserProc::SendActionClear(CUser& user,
                                uint16_t action_index,
                                _ActionGroupIndex action_group_index,
                                bool clear_state,
                                ENUM_PACKETCLASS packet_class) {
    _MSG_ACTION_POINT_ACTION_CLEAR message;
    message.action_group_index = action_group_index;
    message.action_index = action_index;
    message.clear_state = clear_state;

    PacketGuard packet;
    if (packet_class) {
        packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                          ENUM_CMDPACKET_ACTION_POINT_ACTION_CLEAR);
    } else {
        packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                          ENUM_NOTIPACKET_ACTION_POINT_ACTION_CLEAR);
    }
    packet.put_binary(reinterpret_cast<const char*>(&message), sizeof(message));
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x81224A0
// IDA_NAME: _ZN8APSystem9CUserProc19SendActionListStateER5CUser
// DEMANGLED: APSystem::CUserProc::SendActionListState(CUser &)
// Original `neople/game/df_game_r` SHA256:
// dd0ae672b1dce3ff9c6c403beaec71b3ef666b14deb567d4cab0da99d8711f9e.
// ELF32/i386 .symtab size is 11 bytes.  The entire function is:
//   55 89 e5 81 ec 20 0d 00 00 c9 c3
// (`push ebp; mov ebp,esp; sub esp,0xd20; leave; ret`).  It contains no
// call, branch, memory access, packet serialization, or return value; the
// observed empty body is therefore the original ABI behavior.
void CUserProc::SendActionListState(CUser& /*user*/) {}

// IDA_EA: 0x81224AC
// IDA_NAME: _ZN8APSystem9CUserProc17SendGetRewardItemER5CUserNS_22_SIG_LOAD_ACTION_POINT16_TodayRewardItemEb
// DEMANGLED: APSystem::CUserProc::SendGetRewardItem(CUser &,APSystem::_SIG_LOAD_ACTION_POINT::_TodayRewardItem,bool)
void CUserProc::SendGetRewardItem(
    CUser& user,
    _SIG_LOAD_ACTION_POINT::_TodayRewardItem reward_item,
    bool result) {
    _MSG_ACTION_POINT_GET_REWARD_ITEM message;
    message.result = result;
    message.reward_item = reward_item;

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ACTION_POINT_GET_REWARD_ITEM);
    packet.put_binary(reinterpret_cast<const char*>(&message), sizeof(message));
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x812255C
// IDA_NAME: _ZN8APSystem9CUserProc27GetTodayRewardAndSendPostalEP5CUserNS_22_SIG_LOAD_ACTION_POINT16_TodayRewardItemE
// DEMANGLED: APSystem::CUserProc::GetTodayRewardAndSendPostal(CUser *,APSystem::_SIG_LOAD_ACTION_POINT::_TodayRewardItem)
// The old-server target is an empty body.  Do not synthesize postal or reward
// side effects that are absent from the original function.
void CUserProc::GetTodayRewardAndSendPostal(
    CUser*,
    _SIG_LOAD_ACTION_POINT::_TodayRewardItem) {}

// IDA_EA: 0x8122564
// IDA_NAME: _ZN8APSystem9CUserProc28CheckAndSendToMessageAllUserEjjPKc
// DEMANGLED: APSystem::CUserProc::CheckAndSendToMessageAllUser(
//     unsigned int,unsigned int,char const*)
// The old-server target only allocates and releases a stack frame; no input
// is read and no message is emitted.
void CUserProc::CheckAndSendToMessageAllUser(uint32_t,
                                             uint32_t,
                                             const char*) {}

// IDA_EA: 0x812256C
// IDA_NAME: _ZN8APSystem9CUserProc17UpdateActionPointEP5CUser
// DEMANGLED: APSystem::CUserProc::UpdateActionPoint(CUser *)
// The original function is a no-op (`push ebp; mov ebp,esp; pop ebp; ret`).
void CUserProc::UpdateActionPoint(CUser*) {}

namespace {

struct LegacyGameServerMessage {
    const char* channel;
    uint32_t message_length;
    uint32_t reserved_0;
    uint32_t reserved_1;
};

}  // namespace

// IDA_EA: 0x8122572
// IDA_NAME: _ZN8APSystem9CUserProc20SendToMessageAllUserEPKcS2_
// DEMANGLED: APSystem::CUserProc::SendToMessageAllUser(char const*,char const*)
void CUserProc::SendToMessageAllUser(const char* message_argument,
                                     const char* format) {
    if (!message_argument || !format) {
        return;
    }

    char text[1024];
    std::sprintf(text, format, message_argument);

    LegacyGameServerMessage message;
    message.channel = "chn_game_server_msg_82";
    message.message_length = static_cast<uint32_t>(std::strlen(text));
    message.reserved_0 = 0;
    message.reserved_1 = 0;
    static_cast<void>(message);
}

// IDA_EA: 0x81225D2
// IDA_NAME: _ZN8APSystem9CUserProc8GM_ResetEP5CUser
// DEMANGLED: APSystem::CUserProc::GM_Reset(CUser *)
void CUserProc::GM_Reset(CUser* user) {
    if (user) {
        SetTodayActionAndCheckMedalReward(user);
    }
}

// IDA_EA: 0x81225EE
// IDA_NAME: _ZN8APSystem9CUserProc6GM_SetEP5CUserj
// DEMANGLED: APSystem::CUserProc::GM_Set(CUser *,unsigned int)
void CUserProc::GM_Set(CUser* user, uint32_t action_point) {
    if (user) {
        user->action_point_manager_.GM_Set(*user, action_point);
        SendActionListState(*user);
    }
}

// IDA_EA: 0x859755A
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearActionC2Ev
// DEMANGLED: APSystem::Dispatcher_ClearAction::Dispatcher_ClearAction(void)
Dispatcher_ClearAction::Dispatcher_ClearAction() {}

// IDA_EA: 0x8125868
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearActionD2Ev
// DEMANGLED: APSystem::Dispatcher_ClearAction::~Dispatcher_ClearAction()
// IDA_EA: 0x8125898
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearActionD0Ev
// DEMANGLED: APSystem::Dispatcher_ClearAction::~Dispatcher_ClearAction() deleting destructor
Dispatcher_ClearAction::~Dispatcher_ClearAction() {}

// IDA_EA: 0x812566E
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearAction4readER9PacketBufR8MSG_BASE
// DEMANGLED: APSystem::Dispatcher_ClearAction::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ClearAction::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8125678
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearAction7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: APSystem::Dispatcher_ClearAction::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ClearAction::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x812513C
// IDA_NAME: _ZN8APSystem22Dispatcher_ClearAction12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: APSystem::Dispatcher_ClearAction::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ClearAction::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 632;
    }

    _MSG_ACTION_POINT_ACTION_CLEAR request;
    if (packet.get_binary(reinterpret_cast<char*>(&request), sizeof(request)) !=
        1) {
        return 636;
    }

    CUserProc::ClearActionAndSendtoUser(user,
                                        request.action_group_index,
                                        request.action_value_or_reserved,
                                        ENUM_PACKETCLASS_CMDPACKET);
    return 0;
}

// IDA_EA: 0x8597576
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayRewardC2Ev
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::Dispatcher_GetTodayReward(void)
Dispatcher_GetTodayReward::Dispatcher_GetTodayReward() {}

// IDA_EA: 0x812581A
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayRewardD2Ev
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::~Dispatcher_GetTodayReward()
// IDA_EA: 0x812584A
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayRewardD0Ev
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::~Dispatcher_GetTodayReward() deleting destructor
Dispatcher_GetTodayReward::~Dispatcher_GetTodayReward() {}

// IDA_EA: 0x8125682
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GetTodayReward::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x812568C
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GetTodayReward::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x81251DA
// IDA_NAME: _ZN8APSystem25Dispatcher_GetTodayReward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: APSystem::Dispatcher_GetTodayReward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GetTodayReward::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 647;
    }

    _MSG_ACTION_POINT_GET_REWARD_ITEM request;
    if (packet.get_binary(reinterpret_cast<char*>(&request), sizeof(request)) !=
        1) {
        return 651;
    }

    CUserProc::GetTodayRewardAndSendPostal(user, request.reward_item);
    return 0;
}

}  // namespace APSystem
