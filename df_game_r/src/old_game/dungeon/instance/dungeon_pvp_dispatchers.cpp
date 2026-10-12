#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "common/config.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_data/db/auth_security/db_check_3rd_party_consent.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"
#include "common/manager/disconnect_detecter_abi.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/slang_filter.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/social/party/quick_party_system_manager_runtime.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_data/PVF/dungeon/maze_script.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/player/core/cuser.h"


namespace Taiwan {
// A-owned stream ABI import; C only calls it.
// ABI_IMPORT_IDA_EA: 0x8163950
// ABI_IMPORT_NAME: _ZN6Taiwan15internal_streamER12CStreamGuardN18TaiwanInternalPack1TEi
int internal_stream(CStreamGuard& guard, int code, int src);
}

// CLIENT_IDA_EVIDENCE: native packet enum and client handler 0x8762E0.
typedef char NOTI_PCROOM_DAILY_REWARD_STATE_Must_Be_654[
    (ENUM_NOTIPACKET_PCROOM_DAYILY_REWARD_STATE == 654) ? 1 : -1];

namespace {

const char kPvpTimeoutTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_PVP_TIMEOUT, PARAM_TYPE = stOnPvPTimeOut, ch_state STATE = (ch_state)0u]";
const char kSetPvpReadyTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SET_READY_STATE, PARAM_TYPE = stOnSetPvPReadyParam, ch_state STATE = (ch_state)0u]";
const char kEventDungeonDestoryObjectTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = CMDPacketStruct::_STReqEventDungeonDestoryObject, PARAM_TYPE = CMDPacketStruct::_STResEventDungeonDestoryObject, ch_state STATE = (ch_state)3u]";
const char kEventDungeonClearRoomTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = CMDPacketStruct::_STReqEventDungeonClearRoom, PARAM_TYPE = CMDPacketStruct::_STResEventDungeonClearRoom, ch_state STATE = (ch_state)3u]";

const ENUM_CMDPACKET kBossDieCheckCommand = ENUM_CMDPACKET_SDC_DAMAGE_CHECK;
const int kBossDieCheckPartyPacket = 115;
const int kBossDieCheckTimerMessage = 100;
const WongWork::ENUM_HACKTYPE kBossDieCheckSkillChecksumHack =
    static_cast<WongWork::ENUM_HACKTYPE>(0x25cu);
const WongWork::ENUM_HACKTYPE kBossDieCheckMalformedMonsterHack =
    static_cast<WongWork::ENUM_HACKTYPE>(0xcbu);

void SetPartySelectDungeonMode(CParty* party, int mode) {
    party->battle_field().set_instance_dungeon_mode(mode);
}

int GetPartySelectDungeonMode(const CParty* party) {
    return party->battle_field().instance_dungeon_mode();
}

void AddSelectDungeonHack(CUser* user) {
    if (user && user->getHackAnalyzer()) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x1f8u), 1u, 0u,
            0u);
    }
}

bool IsInitialSpecialTutorial(CUser* user, int dungeon_index) {
    if (!user || !user->getCurCharacR() || user->get_state() != 5 ||
        user->get_charac_level() != 1 || user->getCurCharacTutorialFlag() != 0) {
        return false;
    }
    const int job = user->get_charac_job();
    // 0828 client @0x1377796 selects these two Time Gate introductions.
    return (job == oldsrv::config::kCharacterJobDemonicSwordman && dungeon_index == 10001) ||
           (job == oldsrv::config::kCharacterJobCreatorMage && dungeon_index == 10002);
}

}  // namespace

// The original state-0 CMD16 generic dispatcher owns this exact seven-symbol
// template family (C2/D2/D0, dispatch_template, dispatch_sig, check_state,
// and send).  This is its only instantiation owner.
// IDA_EA: 0x8598162, 0x8236798, 0x82367C8, 0x8258956, 0x8258AA6,
//         0x8258AB0, 0x825AD12
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_SELECT_DUNGEON20stSelectDungeonParamL8ch_state0EE*
template class IPacketDispatcher<MSG_SELECT_DUNGEON,
                                 stSelectDungeonParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x81ED09A
// IDA_NAME: _Z12EnterPVPRoomP5CUserP8PvP_Room14ENUM_CMDPACKETPKc
// DEMANGLED: EnterPVPRoom(CUser *,PvP_Room *,ENUM_CMDPACKET,char const*)
int EnterPVPRoom(CUser* user,
                 PvP_Room* room,
                 ENUM_CMDPACKET command,
                 const char* password) {
    PacketGuard packet;
    if (user->IsCurCharacGhost() || user->get_state() != 3) {
        packet.put_header(1, command);
        packet.put_byte(0);
        packet.put_byte(19);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (user->getCurCharacVill() != 7) {
        private_store::CPrivateStoreMgr* const private_store_manager =
            private_store::GetInstancePrivateStoreMgr();
        if (private_store_manager->IsBusyPrivateStore(user) ||
            user->IsBusyDisjointStore()) {
            user->SendCmdErrorPacket(command, 60, packet);
            return 0;
        }
        if (room->get_pvp_battle_mode() == 6) {
            user->SendCmdErrorPacket(command, 23, packet);
            return 0;
        }
    }

    if (user->getCurCharacVill() == 7 || user->CheckInTrade()) {
        user->SendCmdErrorPacket(command, 19, packet);
        return 0;
    }

    const int joinable = room->check_joinable(user);
    if (joinable != 0) {
        packet.put_header(1, command);
        packet.put_byte(0);
        packet.put_byte(joinable);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (room->IsExistPassword() &&
        !room->CheckPassword(password, static_cast<int>(std::strlen(password)))) {
        packet.put_header(1, command);
        packet.put_byte(0);
        packet.put_byte(6);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (user->CheckInParty()) {
        CParty* const party = user->GetParty();
        if (party) {
            party->send_invite_pvp(user->get_unique_id());
            G_CGameManager()->CheckOutParty(user, false);
        }
    }

    int seat = 0;
    const int result = room->join_room(
        user, seat, G_GameWorld()->IsSameTeamChannelJoin());
    if (result > 0) {
        packet.put_header(1, command);
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (command == ENUM_CMDPACKET_QUICK_JOIN_ROOM) {
        PacketGuard quick_join_packet;
        quick_join_packet.put_header(1, ENUM_CMDPACKET_QUICK_JOIN_ROOM);
        quick_join_packet.put_byte(1);
        quick_join_packet.finalize(true);
        user->Send(quick_join_packet);
    }
    room->make_seat_info(reinterpret_cast<char*>(&packet), seat);
    G_GameWorld()->send_all(packet);
    G_GameWorld()->goto_pvp(user);
    return 1;
}

// IDA_EA: 0x81F793C
// IDA_NAME: _Z11MakePVPRoomP5CUserR17MSG_MAKE_PVP_ROOM14ENUM_CMDPACKET15PVP_BATTLE_MODE
// DEMANGLED: MakePVPRoom(CUser *,MSG_MAKE_PVP_ROOM &,ENUM_CMDPACKET,PVP_BATTLE_MODE)
int MakePVPRoom(CUser* user,
                MSG_MAKE_PVP_ROOM& message,
                ENUM_CMDPACKET command,
                PVP_BATTLE_MODE battle_mode) {
    PacketGuard packet;
    packet.put_header(1, command);

    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type == 15 || channel_type == 16) {
        if (G_CDataManager()->GetMaxGradePvPChannel() < user->get_pvp_grade()) {
            packet.put_byte(0);
            packet.put_byte(13);
            packet.finalize(true);
            user->Send(packet);
            return 0;
        }

        fair_pvp::CFairPvPScore* const fair_score =
            static_cast<fair_pvp::CFairPvPScore*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
        ServerParameterScript* const server_parameter =
            G_CDataManager()->get_server_parameter_script();
        if (fair_score->GetGiveItem() <=
            server_parameter->get_pvp_item_give(user->get_pvp_grade())) {
            packet.put_byte(0);
            packet.put_byte(7);
            packet.finalize(true);
            user->Send(packet);
            return 0;
        }
    }

    if (user->IsCurCharacGhost() || !G_GameWorld()->IsPVPChannel() ||
        user->get_state() != 3) {
        packet.put_byte(0);
        packet.put_byte(19);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    PvP_Room* const room = G_CGameManager()->GetPvp();
    if (!room) {
        packet.put_byte(0);
        packet.put_byte(4);
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (user->CheckInParty()) {
        CParty* const party = user->GetParty();
        if (party) {
            party->send_invite_pvp(user->get_unique_id());
            G_CGameManager()->CheckOutParty(user, false);
        }
    }

    const int result = room->create_room(
        user, message.mode, message.title, message.word_2c,
        message.option_type == 1, message.option_payload,
        message.option_length, battle_mode);
    if (result > 0) {
        packet.put_byte(0);
        packet.put_byte(result);
        packet.finalize(true);
        user->Send(packet);
        G_CGameManager()->PutPvp(room);
        return 0;
    }

    if (command == ENUM_CMDPACKET_QUICK_JOIN_ROOM) {
        PacketGuard quick_join_packet;
        quick_join_packet.put_header(1, ENUM_CMDPACKET_QUICK_JOIN_ROOM);
        quick_join_packet.put_byte(1);
        quick_join_packet.finalize(true);
        user->Send(quick_join_packet);
    }

    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(1);
    packet.put_short(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);

    packet.clear();
    packet.put_header(0, 41);
    packet.put_short(1);
    room->make_room_info(reinterpret_cast<char*>(&packet));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    G_GameWorld()->goto_pvp(user);
    return 1;
}

// These are the two exact concrete state-0 template instances consumed by
// CMD56/CMD59.  They are C-owned only because this C dispatcher TU is the
// original concrete-vtable owner; this does not register either class in the
// root PacketDispatcher table.

// Keep every explicit member declaration ahead of the two dispatch_template
// bodies.  Otherwise the calls below make G++ instantiate a generic member
// before the exact old-server specialization is declared.
template <>
IPacketDispatcher<MSG_PVP_TIMEOUT,
                  stOnPvPTimeOut,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PVP_TIMEOUT,
                  stOnPvPTimeOut,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PVP_TIMEOUT,
                       stOnPvPTimeOut,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_SET_READY_STATE,
                  stOnSetPvPReadyParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SET_READY_STATE,
                  stOnSetPvPReadyParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_SET_READY_STATE,
                       stOnSetPvPReadyParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// These exact state-3 concrete instances are the original vtable owners for
// CMD539 and CMD540.  Declare every specialization before the handler bodies
// so this TU cannot instantiate a second generic COMDAT provider.
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                  CMDPacketStruct::_STResEventDungeonDestoryObject,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                  CMDPacketStruct::_STResEventDungeonDestoryObject,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                       CMDPacketStruct::_STResEventDungeonDestoryObject,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                  CMDPacketStruct::_STResEventDungeonClearRoom,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                  CMDPacketStruct::_STResEventDungeonClearRoom,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                       CMDPacketStruct::_STResEventDungeonClearRoom,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

// IDA_EA: 0x859810E
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_PVP_TIMEOUT,
                  stOnPvPTimeOut,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82366AE
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82366DE
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_PVP_TIMEOUT,
                  stOnPvPTimeOut,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8258F28
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    // MSG_BASE C2 has no stores; this source object preserves the raw
    // template's uninitialized request storage while retaining typed fields.
    MSG_PVP_TIMEOUT message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    stOnPvPTimeOut param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8259078
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8259082
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PVP_TIMEOUT,
                      stOnPvPTimeOut,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kPvpTimeoutTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825AD24
// IDA_NAME: _ZN17IPacketDispatcherI15MSG_PVP_TIMEOUT14stOnPvPTimeOutL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_PVP_TIMEOUT,stOnPvPTimeOut,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_PVP_TIMEOUT,
                       stOnPvPTimeOut,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x85981B6
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SET_READY_STATE,
                  stOnSetPvPReadyParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x8236882
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x82368B2
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SET_READY_STATE,
                  stOnSetPvPReadyParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x825838A
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    // MSG_BASE C2 has no stores; the raw concrete template reserves this
    // 14-byte request object without a constructor-side write.
    MSG_SET_READY_STATE message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    stOnSetPvPReadyParam param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82584DA
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x82584E4
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SET_READY_STATE,
                      stOnSetPvPReadyParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kSetPvpReadyTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825AD06
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_SET_READY_STATE20stOnSetPvPReadyParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SET_READY_STATE,stOnSetPvPReadyParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SET_READY_STATE,
                       stOnSetPvPReadyParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8596070
// IDA_NAME: _ZN23Dispatcher_BossDieCheckC2Ev
// DEMANGLED: Dispatcher_BossDieCheck::Dispatcher_BossDieCheck(void)
Dispatcher_BossDieCheck::Dispatcher_BossDieCheck()
    : IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                        stBossDieCheckParam,
                        static_cast<ch_state>(5)>() {}

// IDA_EA: 0x8245EC2
// IDA_NAME: _ZN23Dispatcher_BossDieCheckD2Ev
// DEMANGLED: Dispatcher_BossDieCheck::~Dispatcher_BossDieCheck()
// IDA_EA: 0x8245EF2
// IDA_NAME: _ZN23Dispatcher_BossDieCheckD0Ev
// DEMANGLED: Dispatcher_BossDieCheck::~Dispatcher_BossDieCheck() deleting destructor
Dispatcher_BossDieCheck::~Dispatcher_BossDieCheck() {}

// IDA_EA: 0x81CD642
// IDA_NAME: _ZN23Dispatcher_BossDieCheck4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BossDieCheck::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BossDieCheck::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_BOSS_DIE_CHECK& request = static_cast<MSG_BOSS_DIE_CHECK&>(message);
    if (packet.get_short(request.client_reported_monster_id) != 1) {
        return LineFunc(
            9592,
            "virtual int Dispatcher_BossDieCheck::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_short(request.monster_unique_id) != 1) {
        return LineFunc(
            9593,
            "virtual int Dispatcher_BossDieCheck::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(request.skill_checksum) == 1) {
        return 0;
    }
    return LineFunc(
        9595,
        "virtual int Dispatcher_BossDieCheck::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CD71E
// IDA_NAME: _ZN23Dispatcher_BossDieCheck11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BossDieCheck::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BossDieCheck::check_error(CUser* user,
                                         MSG_BASE&,
                                         ParamBase& base_param) {
    stBossDieCheckParam& param = static_cast<stBossDieCheckParam&>(base_param);
    param.map_order_depleted = 0;
    if (G_CGameManager()->GetParty(user->GetPartyIndex())) {
        return 0;
    }

    cMyTrace trace(
        "int Dispatcher_BossDieCheck::check_error(CUser*, MSG_BASE&, ParamBase&)",
        9611,
        0);
    trace("Party == NULL");
    return 8;
}

// IDA_EA: 0x81CD7A2
// IDA_NAME: _ZN23Dispatcher_BossDieCheck7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BossDieCheck::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BossDieCheck::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& base_param) {
    MSG_BOSS_DIE_CHECK& request = static_cast<MSG_BOSS_DIE_CHECK&>(message);
    stBossDieCheckParam& param = static_cast<stBossDieCheckParam&>(base_param);
    param.result_code = check_error(user, message, param);
    if (param.result_code > 0) {
        return 0;
    }
    if (param.result_code < 0) {
        return LineFunc(
            9630,
            "virtual int Dispatcher_BossDieCheck::process(CUser*, MSG_BASE&, ParamBase&)",
            param.result_code,
            user->get_acc_id());
    }

    if (user->getCurCharacSkillCheckSum() != request.skill_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, kBossDieCheckSkillChecksumHack, 1u, 0u, 0u);
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    CBattle_Field& battle_field = party->battle_field();
    std::vector<int>& map_order = battle_field.map_order();
    std::vector<int>::iterator monster = std::find(
        map_order.begin(), map_order.end(), request.monster_unique_id);
    if (monster != map_order.end()) {
        map_order.erase(monster);
        param.acknowledged_monster_unique_id = request.monster_unique_id;
    } else {
        param.result_code = 21;
    }

    if (map_order.empty()) {
        battle_field.boss_die_check_started_at_sec() =
            GlobalData::s_systemTime_.getCurSec();
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            party->GetPartyIndex(),
            kBossDieCheckTimerMessage,
            static_cast<int64_t>(1000),
            static_cast<unsigned int>(
                battle_field.boss_die_check_started_at_sec()),
            user->get_charac_no(-1));
        param.map_order_depleted = 1;
    }

    if (request.client_reported_monster_id == 0xffffu) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, kBossDieCheckMalformedMonsterHack, 1u, 0u, 0u);
    }
    return 0;
}

// IDA_EA: 0x81CDA62
// IDA_NAME: _ZN23Dispatcher_BossDieCheck4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_BossDieCheck::send(CUser *,ParamBase &)
void Dispatcher_BossDieCheck::send(CUser* user, ParamBase& base_param) {
    stBossDieCheckParam& param = static_cast<stBossDieCheckParam&>(base_param);
    if (param.cmd183_status == 2) {
        user->SendCmdErrorPacket(kBossDieCheckCommand,
                                 2);
    }

    PacketGuard packet;
    packet.put_header(0, kBossDieCheckPartyPacket);
    if (param.result_code != 0) {
        packet.put_byte(0);
        packet.put_byte(param.result_code);
    } else {
        packet.put_byte(1);
        packet.put_byte(param.map_order_depleted);
        packet.put_short(param.acknowledged_monster_unique_id);
    }
    packet.finalize(true);

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (party) {
        party->send_to_party(packet);
    } else {
        user->Send(packet);
    }
}

// IDA_EA: 0x81C424C
// IDA_NAME: _ZN21DisPatcher_LeaveParty4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_LeaveParty::read(PacketBuf &,MSG_BASE &)
int DisPatcher_LeaveParty::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81C4256
// IDA_NAME: _ZN21DisPatcher_LeaveParty7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_LeaveParty::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_LeaveParty::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& base_param) {
    stLeavePartyParam& param = static_cast<stLeavePartyParam&>(base_param);
    param.raw_04 = check_error(user, message);
    if (param.raw_04 > 0) {
        return 0;
    }
    if (param.raw_04 >= 0) {
        CParty* const party = user->GetParty();
        if (party->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2)) ==
            1) {
            param.raw_04 = 1;
        }
        pvp_assault::GetInstanceAssaultMgr()->ForcedMoveAtPowerWarPlace(user);
        return 0;
    }
    return LineFunc(
        4255,
        "virtual int DisPatcher_LeaveParty::process(CUser*, MSG_BASE&, ParamBase&)",
        param.raw_04,
        user->get_acc_id());
}

// IDA_EA: 0x81C4326
// IDA_NAME: _ZN21DisPatcher_LeaveParty4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_LeaveParty::send(CUser *,ParamBase &)
void DisPatcher_LeaveParty::send(CUser* user, ParamBase& base_param) {
    const int result = static_cast<stLeavePartyParam&>(base_param).raw_04;
    if (result != 0x7fffffff && result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_LEAVE_PARTY,
                                 static_cast<unsigned char>(result));
    }
}

// IDA_EA: 0x81C436E
// IDA_NAME: _ZN21DisPatcher_LeaveParty11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_LeaveParty::check_error(CUser *,MSG_BASE &)
int DisPatcher_LeaveParty::check_error(CUser* user, MSG_BASE&) {
    return user->GetParty() ? 0 : 18;
}

// IDA_EA: 0x82323C8
// IDA_NAME: _ZN21DisPatcher_LeaveParty11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_LeaveParty::check_state(CUser *,PacketBuf &)
int DisPatcher_LeaveParty::check_state(CUser* user, PacketBuf&) {
    if (user->get_state() == 3) {
        return 0;
    }
    return LineFunc(
        520,
        "virtual int DisPatcher_LeaveParty::check_state(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8246354
// IDA_NAME: _ZN21DisPatcher_LeavePartyD2Ev
// DEMANGLED: DisPatcher_LeaveParty::~DisPatcher_LeaveParty()
// IDA_EA: 0x8246384
// IDA_NAME: _ZN21DisPatcher_LeavePartyD0Ev
// DEMANGLED: DisPatcher_LeaveParty::~DisPatcher_LeaveParty()
DisPatcher_LeaveParty::~DisPatcher_LeaveParty() {}

// IDA_EA: 0x8595A18
// IDA_NAME: _ZN21DisPatcher_LeavePartyC2Ev
// DEMANGLED: DisPatcher_LeaveParty::DisPatcher_LeaveParty(void)
DisPatcher_LeaveParty::DisPatcher_LeaveParty()
    : IPacketDispatcher<MSG_BASE,
                        stLeavePartyParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81C5D78
// IDA_NAME: _ZN21DisPatcher_PvPTimeout4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_PvPTimeout::read(PacketBuf &,MSG_BASE &)
int DisPatcher_PvPTimeout::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_PVP_TIMEOUT& timeout = static_cast<MSG_PVP_TIMEOUT&>(message);

    for (int index = 0; index <= 7; ++index) {
        if (packet.get_int(timeout.scores[index]) != 1) {
            return LineFunc(5286,
                            "virtual int DisPatcher_PvPTimeout::read(PacketBuf&, MSG_BASE&)",
                            0, 0);
        }
    }
    return 0;
}

// IDA_EA: 0x81C5DEC
// IDA_NAME: _ZN21DisPatcher_PvPTimeout7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_PvPTimeout::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_PvPTimeout::process(CUser* user,
                                   MSG_BASE& base_message,
                                   ParamBase& base_param) {
    const int error = check_error(user, base_message);
    if (error > 0) {
        return -1;
    }
    if (error >= 0) {
        PvP_Room* const room = user->GetPVPRoom();
        if (!room) {
            return LineFunc(
                5308,
                "virtual int DisPatcher_PvPTimeout::process(CUser*, MSG_BASE&, ParamBase&)",
                0,
                user->get_acc_id());
        }

        MSG_PVP_TIMEOUT& message =
            static_cast<MSG_PVP_TIMEOUT&>(base_message);
        stOnPvPTimeOut& param = static_cast<stOnPvPTimeOut&>(base_param);
        param.room_id = room->time_out(message.scores);
        return error;
    }
    return LineFunc(
        5303,
        "virtual int DisPatcher_PvPTimeout::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x81C5EBE
// IDA_NAME: _ZN21DisPatcher_PvPTimeout11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_PvPTimeout::check_error(CUser *,MSG_BASE &)
int DisPatcher_PvPTimeout::check_error(CUser* user, MSG_BASE&) {
    return user->get_state() == 7 ? 0 : 19;
}

// IDA_EA: 0x81C5EE8
// IDA_NAME: _ZN21DisPatcher_PvPTimeout4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_PvPTimeout::send(CUser *,ParamBase &)
void DisPatcher_PvPTimeout::send(CUser* user, ParamBase& base_param) {
    PacketGuard response;
    const int result = static_cast<stOnPvPTimeOut&>(base_param).room_id;
    if (result > 0) {
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_PVP_TIME_OUT);
        response.put_byte(0);
        response.put_byte(result);
        response.finalize(true);
        user->Send(response);
    }
}

// IDA_EA: 0x82324FA
// IDA_NAME: _ZN21DisPatcher_PvPTimeoutD2Ev
// DEMANGLED: DisPatcher_PvPTimeout::~DisPatcher_PvPTimeout()
// IDA_EA: 0x823252A
// IDA_NAME: _ZN21DisPatcher_PvPTimeoutD0Ev
// DEMANGLED: DisPatcher_PvPTimeout::~DisPatcher_PvPTimeout()
DisPatcher_PvPTimeout::~DisPatcher_PvPTimeout() {}

// IDA_EA: 0x8595214
// IDA_NAME: _ZN21DisPatcher_PvPTimeoutC2Ev
// DEMANGLED: DisPatcher_PvPTimeout::DisPatcher_PvPTimeout(void)
DisPatcher_PvPTimeout::DisPatcher_PvPTimeout()
    : IPacketDispatcher<MSG_PVP_TIMEOUT,
                        stOnPvPTimeOut,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81C7E10
// IDA_NAME: _ZN24DisPatcher_SelectDungeon4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SelectDungeon::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SelectDungeon::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SELECT_DUNGEON& request =
        static_cast<MSG_SELECT_DUNGEON&>(message);

    if (packet.get_short(request.dungeon_index) != 1) {
        return LineFunc(6362,
                        "virtual int DisPatcher_SelectDungeon::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.dungeon_difficulty) != 1) {
        return LineFunc(6363,
                        "virtual int DisPatcher_SelectDungeon::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.dungeon_mode) != 1) {
        return LineFunc(6370,
                        "virtual int DisPatcher_SelectDungeon::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.request_flag) == 1) {
        return 0;
    }
    return LineFunc(6372,
                    "virtual int DisPatcher_SelectDungeon::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C7F32
// IDA_NAME: _ZN24DisPatcher_SelectDungeon11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SelectDungeon::check_error(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SelectDungeon::check_error(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)param;
    MSG_SELECT_DUNGEON& request = static_cast<MSG_SELECT_DUNGEON&>(message);

    if (user->get_state() != 5 && user->get_state() != 13) {
        return 0x7fffffff;
    }
    if (static_cast<unsigned char>(user->IsBusyDisjointStore())) {
        return 0x7fffffff;
    }

    // sub_13ACB40's CMD16 writer @0x13ACCD4 uses mode 4 when its
    // world-map entry is absent.  A new job 9/10 tutorial is such an entry;
    // mode 4 is therefore not exclusively the guardian altar's state 13.
    if (request.dungeon_mode == 4 && request.request_flag == 0 &&
        request.dungeon_difficulty == 0 &&
        IsInitialSpecialTutorial(user, request.dungeon_index)) {
        LogManager::logFormat(1, "PacketDispatcher.cpp", __FUNCTION__, __LINE__,
            "[SPECIAL_TUTORIAL][NORMALIZE] char=%d job=%d dungeon=%d mode=4->0",
            user->getCurCharacNo(), user->get_charac_job(), request.dungeon_index);
        request.dungeon_mode = 0;
    }

    // 140828 CLIENT_IDA_EVIDENCE: sub_13B0600 sends the guardian-altar
    // CMD16 as u16 dungeon, u8 difficulty, u8 mode(4), u8 request_flag(0).
    // Treat mode 4 as the first-class 140828 Advance Altar mode.  It is
    // intentionally separate from the 130627 ordinary-dungeon modes 0..2:
    // CMD641 owns state 13 and the selected PVF dungeon owns the defense flag.
    static const int kAdvanceAltarDungeonMode = 4;
    if (request.dungeon_mode == kAdvanceAltarDungeonMode) {
        CDataManager* const data_manager = G_CDataManager();
        CDungeon* const requested_dungeon =
            data_manager
                ? data_manager->find_dungeon(request.dungeon_index)
                : 0;
        if (user->get_state() != 13 || request.request_flag != 0 ||
            request.dungeon_difficulty < 0 ||
            request.dungeon_difficulty > 2 || !requested_dungeon ||
            !requested_dungeon->isDefenseDungeon()) {
            return -1;
        }

        // CMD641 rejects an existing party before entering state 13.  The
        // defense-dungeon process branch is deliberately solo/null-party safe
        // and calls advancealtar::Manager::selectStage directly.
        return 0;
    }

    if (request.dungeon_mode > 2 || request.dungeon_difficulty < 0) {
        return -1;
    }
    if (request.request_flag >= 2) {
        return -2;
    }
    if (request.dungeon_mode > 0 && request.request_flag > 0) {
        return 0x7fffffff;
    }
    if (request.dungeon_mode == 2) {
        return 0;
    }

    CParty* party = user->GetParty();
    if (!party || party->getManager() != user || party->get_state() != 1) {
        return 0x7fffffff;
    }
    if (request.request_flag == 1 && party->is_quick_party()) {
        return 0x7fffffff;
    }
    if (request.dungeon_mode == 2) {
        return 0;
    }

    party = user->GetParty();
    if (!party || party->getManager() != user || party->get_state() != 1) {
        return 0x7fffffff;
    }
    return 0;
}

// IDA_EA: 0x81C8102
// IDA_NAME: _ZN24DisPatcher_SelectDungeon7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SelectDungeon::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SelectDungeon::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    stSelectDungeonParam& result = static_cast<stSelectDungeonParam&>(param);
    const MSG_SELECT_DUNGEON& request =
        static_cast<const MSG_SELECT_DUNGEON&>(message);

    result.raw_04 = check_error(user, message, param);
    if (result.raw_04 > 0) {
        return 0;
    }
    if (result.raw_04 < 0) {
        return LineFunc(
            6512,
            "virtual int DisPatcher_SelectDungeon::process(CUser*, MSG_BASE&, ParamBase&)",
            result.raw_04,
            user->get_acc_id());
    }

    IsLightServer();

    CParty* party = user->GetParty();
    if (party && party->getManager() != user) {
        result.raw_04 = 0x7fffffff;
        return 0;
    }

    if (request.request_flag == 1) {
        // check_error has already established a normal managed party for this
        // packet shape.  The old loop intentionally processes every occupied
        // seat, not only checkValidUser seats.
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const member = party ? party->get_user(seat) : 0;
            if (!member) {
                continue;
            }
            LogManager::logFormat(
                1, "PacketDispatcher_Impl_1.cpp",
                "virtual int DisPatcher_SelectDungeon::process(CUser*, MSG_BASE&, ParamBase&)",
                0x198a, "[HackUser] dungeon hack user. (mid:%u)",
                member->get_acc_id());
            party->leave_user(member,
                              static_cast<ENUM_PARTY_INFO_TYPE>(3));
            member->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
        }
        return 0;
    }

    if (party) {
        SetPartySelectDungeonMode(party, request.request_flag);
    }
    if (user->isDisguiseCharac()) {
        user->setDisguiseCharac(false, 0, 0);
    }

    if (party) {
        const int member_count = party->get_member_count();
        const unsigned char manager_village =
            static_cast<unsigned char>(user->getCurCharacVill());
        const int manager_area = user->get_area(false);
        bool invalid_village_or_area = false;

        for (int seat = 0; seat < member_count; ++seat) {
            CUser* const member = party->get_user(seat);
            if (!member) {
                continue;
            }
            member->setDisguiseCharac(false, 0, 0);
            if ((manager_village !=
                     static_cast<unsigned char>(member->getCurCharacVill()) ||
                 manager_area != member->get_area(false)) &&
                party->is_quick_party() != 1) {
                LogManager::logFormat(
                    1, "PacketDispatcher_Impl_1.cpp",
                    "virtual int DisPatcher_SelectDungeon::process(CUser*, MSG_BASE&, ParamBase&)",
                    0x19df,
                    "[HACK_VILL_AREA] m_id : %u, Vill(%d,%d), Area(%d,%d)",
                    member->get_acc_id(), manager_village,
                    static_cast<unsigned char>(member->getCurCharacVill()),
                    manager_area, member->get_area(false));
                invalid_village_or_area = true;
                break;
            }
            member->SetDeleteEventDungeonInvitation(false);
        }

        if (invalid_village_or_area) {
            for (int seat = 0; seat <= 3; ++seat) {
                CUser* const member = party->get_user(seat);
                if (!member) {
                    continue;
                }
                LogManager::logFormat(
                    1, "PacketDispatcher_Impl_1.cpp",
                    "virtual int DisPatcher_SelectDungeon::process(CUser*, MSG_BASE&, ParamBase&)",
                    0x19fa, "[HACK_VILL_AREA] m_id : %u, Vill(%d), Area(%d)",
                    member->get_acc_id(), manager_village, manager_area);
                member->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
            }
            result.raw_04 = 0x7fffffff;
            return 0;
        }
    }

    CDungeon* const dungeon =
        G_CDataManager()->find_dungeon(request.dungeon_index);
    bool require_world_dungeon_membership = request.dungeon_index != 10000 &&
        !IsInitialSpecialTutorial(user, request.dungeon_index);
    if (dungeon && dungeon->isDefenseDungeon()) {
        require_world_dungeon_membership = false;
    }

    if (require_world_dungeon_membership && party) {
        const int world_map_index = G_GameWorld()->GetWorldMapIndex(user);
        if (world_map_index <= 0) {
            result.raw_08 = 8;
        } else {
            CWorldMapList* const world_map_list =
                CDataManagerWorldMapList(G_CDataManager());
            CWorldMap* const world_map =
                world_map_list
                    ? world_map_list->find_world_map(world_map_index)
                    : 0;
            if (!world_map) {
                result.raw_04 = 0x7fffffff;
                return 0;
            }
            if (!world_map->hasDungeon(
                    static_cast<unsigned int>(request.dungeon_index))) {
                for (int seat = 0; seat <= 3; ++seat) {
                    if (party->checkValidUser(seat) == 1) {
                        AddSelectDungeonHack(party->get_user(seat));
                    }
                }
                result.raw_08 = 8;
                return 0;
            }
        }
    }

    int has_death_tower = 0;
    const int current_world_map_index = G_GameWorld()->GetWorldMapIndex(user);
    if (current_world_map_index > 0) {
        CWorldMapList* const world_map_list =
            CDataManagerWorldMapList(G_CDataManager());
        CWorldMap* const world_map =
            world_map_list
                ? world_map_list->find_world_map(current_world_map_index)
                : 0;
        if (world_map) {
            has_death_tower = world_map->hasDeathTower();
        }
    }

    if (!dungeon) {
        result.raw_04 = 0x7fffffff;
        return 0;
    }

    if (dungeon->getDungeonKind() == 1) {
        if (!party) {
            result.raw_04 = 0x7fffffff;
            return 0;
        }
        WongWork::CDeathTower* const death_tower =
            G_CGameManager()->getDeathTower();
        if (!death_tower) {
            result.raw_04 = 0x7fffffff;
            return 0;
        }
        const int min_level = dungeon->get_min_level();
        for (int seat = 0; seat <= 3; ++seat) {
            CUser* const member = party->get_user(seat);
            if (member && member->get_charac_level() < min_level) {
                return 0;
            }
        }
        death_tower->attachParty(party);
        death_tower->handleStartGame(request.dungeon_index,
                                     request.dungeon_difficulty,
                                     static_cast<ENUM_DUNGEON_TYPE>(
                                         request.dungeon_mode));
        result.raw_04 = 0x7fffffff;
        return 0;
    }

    if (dungeon->getDungeonKind() == 3) {
        if (party && dungeon->get_limit_party_count() >=
                         party->get_member_count()) {
            Global::CBossTowerFunc::SelectDungeon(
                party, request.dungeon_index,
                static_cast<ENUM_DUNGEON_TYPE>(request.dungeon_mode));
            return 0;
        }

        if (party) {
            PacketGuard response;
            response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                static_cast<ENUM_CMDPACKET>(16));
            response.put_byte(0);
            response.put_int(243);
            response.finalize(true);
            party->send_to_party(response);
        }
        return 0;
    }

    if (dungeon->isDefenseDungeon()) {
        const int altar_result = advancealtar::Manager::selectStage(
            user, request.dungeon_index, request.dungeon_difficulty);
        advancealtar::Manager::procErrorSelectStage(user, altar_result);
        return 0;
    }

    if (dungeon->isEventDungeon() && party) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (party->checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = party->get_user(seat);
            if (member && member->get_charac_level() < dungeon->get_min_level()) {
                result.raw_0c = 14;
                return 0;
            }
        }
    } else if (dungeon->isRevengeDungeon()) {
        if (request.dungeon_difficulty != 0) {
            AddSelectDungeonHack(user);
            result.raw_04 = 0x7fffffff;
            return 0;
        }
        if (party) {
            const int member_count = party->get_member_count();
            if (static_cast<unsigned char>(
                    GlobalData::s_revengeDungeonMgr->IsOpenRevengeDungeon()) !=
                1u) {
                result.raw_0c = 21;
                return 0;
            }
            if (dungeon->getRevengeDungeonMaxPartyMembers() < member_count ||
                dungeon->getRevengeDungeonMinPartyMembers() > member_count) {
                result.raw_0c = 171;
                return 0;
            }
            for (int seat = 0; seat <= 3; ++seat) {
                if (party->checkValidUser(seat) != 1) {
                    continue;
                }
                CUser* const member = party->get_user(seat);
                if (!member) {
                    continue;
                }
                const int count = member->GetCurRevengeDungeonCount();
                if (count == 0 ||
                    dungeon->getRevengeDungeonEntryLimit() < count) {
                    member->SetCurRevengeDungeonCount(0);
                    result.raw_0c = 173;
                    result.raw_14 = seat;
                    return 0;
                }
            }
        }
    }

    if (party) {
        if (has_death_tower != 1 && GetPartySelectDungeonMode(party) != 1 &&
            checkBlueMarble(party) != 1 &&
            dungeon->isNoFatigueDungeon() != 1) {
            const int fatigue_member = party->CheckMemberFatigue();
            if (fatigue_member > 0) {
                result.raw_0c = 22;
                result.raw_08 = 22;
                result.raw_14 = fatigue_member - 1;
                return 0;
            }
        }

        const int start_result = party->dungeon_start(
            request.dungeon_index, request.dungeon_difficulty,
            static_cast<ENUM_DUNGEON_TYPE>(request.dungeon_mode));
        if (start_result <= 0) {
            if (!user->network_.get_nat_type()) {
                if (user->getHackAnalyzer()) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user, static_cast<WongWork::ENUM_HACKTYPE>(0x326u),
                        1u, 0u, 0u);
                }
            }
        } else {
            result.raw_0c = start_result;
            result.raw_08 = start_result;
        }
    }

    return 0;
}

// IDA_EA: 0x81C8E9C
// IDA_NAME: _ZN24DisPatcher_SelectDungeon4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SelectDungeon::send(CUser *,ParamBase &)
void DisPatcher_SelectDungeon::send(CUser* user, ParamBase& param) {
    stSelectDungeonParam& result = static_cast<stSelectDungeonParam&>(param);
    if (result.raw_04 == 0x7fffffff) {
        return;
    }
    if (result.raw_04) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(16),
                                 static_cast<unsigned char>(result.raw_04));
    }
    if (!result.raw_0c && !result.raw_08) {
        return;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return;
    }
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        static_cast<ENUM_CMDPACKET>(16));
    response.put_byte(0);

    if (result.raw_08 == result.raw_0c) {
        response.put_byte(static_cast<char>(result.raw_08));
        if (result.raw_08 == 22) {
            response.put_byte(static_cast<char>(result.raw_14));
        }
        response.finalize(true);
        party->send_to_party(response);
    } else if (result.raw_08) {
        response.put_byte(static_cast<char>(result.raw_08));
        response.finalize(true);
        user->Send(response);
    } else {
        // The original +0x0c-only branch writes +0x08 (zero) here; preserve
        // that wire behavior and only append the affected member for 173.
        response.put_byte(static_cast<char>(result.raw_08));
        if (result.raw_0c == 173) {
            response.put_byte(static_cast<char>(result.raw_14));
        }
        response.finalize(true);
        party->send_to_party(response);
    }
}

// IDA_EA: 0x81C9172
// IDA_NAME: _ZN24DisPatcher_SelectDungeon15checkBlueMarbleEP6CParty
// DEMANGLED: DisPatcher_SelectDungeon::checkBlueMarble(CParty *)
int DisPatcher_SelectDungeon::checkBlueMarble(CParty* party) {
    if (!party) {
        return 0;
    }
    CUser* const manager = party->getManager();
    if (!manager) {
        return 0;
    }
    return party->IsSinglePlay() && manager->getMoveSpace() == 1 ? 1 : 0;
}

// IDA_EA: 0x82325E4
// IDA_NAME: _ZN24DisPatcher_SelectDungeonD2Ev
// DEMANGLED: DisPatcher_SelectDungeon::~DisPatcher_SelectDungeon()
// IDA_EA: 0x8232614
// IDA_NAME: _ZN24DisPatcher_SelectDungeonD0Ev
// DEMANGLED: DisPatcher_SelectDungeon::~DisPatcher_SelectDungeon()
DisPatcher_SelectDungeon::~DisPatcher_SelectDungeon() {}

// IDA_EA: 0x8595268
// IDA_NAME: _ZN24DisPatcher_SelectDungeonC2Ev
// DEMANGLED: DisPatcher_SelectDungeon::DisPatcher_SelectDungeon(void)
DisPatcher_SelectDungeon::DisPatcher_SelectDungeon()
    : IPacketDispatcher<MSG_SELECT_DUNGEON,
                        stSelectDungeonParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81C99BE
// IDA_NAME: _ZN22DisPatcher_SetPVPReady4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPVPReady::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPVPReady::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SET_READY_STATE& ready_state =
        static_cast<MSG_SET_READY_STATE&>(message);

    if (packet.get_byte(ready_state.ready) == 1) {
        return 0;
    }
    return LineFunc(7379,
                    "virtual int DisPatcher_SetPVPReady::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C9A14
// IDA_NAME: _ZN22DisPatcher_SetPVPReady7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPVPReady::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPVPReady::process(CUser* user,
                                    MSG_BASE& base_message,
                                    ParamBase& base_param) {
    const int error = check_error(user, base_message);
    if (error > 0) {
        return -1;
    }
    if (error >= 0) {
        PvP_Room* const room = user->GetPVPRoom();
        if (!room) {
            return LineFunc(
                7398,
                "virtual int DisPatcher_SetPVPReady::process(CUser*, MSG_BASE&, ParamBase&)",
                0,
                user->get_acc_id());
        }
        if (room->get_pvp_battle_mode() == 6) {
            return -1;
        }

        const MSG_SET_READY_STATE& message =
            static_cast<const MSG_SET_READY_STATE&>(base_message);
        stOnSetPvPReadyParam& param =
            static_cast<stOnSetPvPReadyParam&>(base_param);
        param.raw_04 = room->set_ready_state(user, message.ready != 0);
        if (param.raw_04 >= 0) {
            return error;
        }

        room->start_pvp();
        PacketGuard state_info;
        // The raw ABI is exactly PvP_Room::make_state_info(char *); this is
        // the address of the complete PacketGuard object, with no invented
        // buffer offset or wrapper layout.
        room->make_state_info(reinterpret_cast<char*>(&state_info));
        G_GameWorld()->send_all(state_info);
        return -1;
    }
    return LineFunc(
        7395,
        "virtual int DisPatcher_SetPVPReady::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x81C9B9C
// IDA_NAME: _ZN22DisPatcher_SetPVPReady11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_SetPVPReady::check_error(CUser *,MSG_BASE &)
int DisPatcher_SetPVPReady::check_error(CUser* user, MSG_BASE&) {
    return user->get_state() == 6 ? 0 : 19;
}

// IDA_EA: 0x81C9BC6
// IDA_NAME: _ZN22DisPatcher_SetPVPReady4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SetPVPReady::send(CUser *,ParamBase &)
void DisPatcher_SetPVPReady::send(CUser* user, ParamBase& base_param) {
    const int result = static_cast<stOnSetPvPReadyParam&>(base_param).raw_04;
    if (result > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_PVP_READY_STATE,
                                 static_cast<unsigned char>(result));
    }
}

// IDA_EA: 0x82326CE
// IDA_NAME: _ZN22DisPatcher_SetPVPReadyD2Ev
// DEMANGLED: DisPatcher_SetPVPReady::~DisPatcher_SetPVPReady()
// IDA_EA: 0x82326FE
// IDA_NAME: _ZN22DisPatcher_SetPVPReadyD0Ev
// DEMANGLED: DisPatcher_SetPVPReady::~DisPatcher_SetPVPReady()
DisPatcher_SetPVPReady::~DisPatcher_SetPVPReady() {}

// IDA_EA: 0x85952BC
// IDA_NAME: _ZN22DisPatcher_SetPVPReadyC2Ev
// DEMANGLED: DisPatcher_SetPVPReady::DisPatcher_SetPVPReady(void)
DisPatcher_SetPVPReady::DisPatcher_SetPVPReady()
    : IPacketDispatcher<MSG_SET_READY_STATE,
                        stOnSetPvPReadyParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81CAF42
// IDA_NAME: _ZN23DisPatcher_WalkoutParty4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_WalkoutParty::read(PacketBuf &,MSG_BASE &)
int DisPatcher_WalkoutParty::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_WALKOUT_PARTY& walkout_message =
        static_cast<MSG_WALKOUT_PARTY&>(message);

    if (packet.get_byte(walkout_message.seat) == 1) {
        return 0;
    }
    return LineFunc(8235,
                    "virtual int DisPatcher_WalkoutParty::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81CAF98
// IDA_NAME: _ZN23DisPatcher_WalkoutParty7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_WalkoutParty::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_WalkoutParty::process(CUser* user,
                                     MSG_BASE& base_message,
                                     ParamBase& base_param) {
    const int error = check_error(user, base_message);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    stWalkoutPartyParam& param =
        static_cast<stWalkoutPartyParam&>(base_param);
    const MSG_WALKOUT_PARTY& message =
        static_cast<const MSG_WALKOUT_PARTY&>(base_message);
    param.raw_04 = 0;
    CParty* const party = user->GetParty();
    if (party) {
        const int seat = static_cast<int>(message.seat);
        party->SendWalkout(seat, static_cast<ENUM_WALKOUT_TYPE>(0));
        param.raw_04 = party->walkout_member(user, seat);
    } else {
        param.raw_04 = 19;
    }
    return 0;
}

// IDA_EA: 0x81CB062
// IDA_NAME: _ZN23DisPatcher_WalkoutParty11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_WalkoutParty::check_error(CUser *,MSG_BASE &)
int DisPatcher_WalkoutParty::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return LineFunc(
            8275,
            "int DisPatcher_WalkoutParty::check_error(CUser*, MSG_BASE&)",
            0,
            0);
    }
    return user->get_state() == 3 ? 0 : -1;
}

// IDA_EA: 0x81CB0B8
// IDA_NAME: _ZN23DisPatcher_WalkoutParty4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_WalkoutParty::send(CUser *,ParamBase &)
void DisPatcher_WalkoutParty::send(CUser* user, ParamBase& base_param) {
    const int result = static_cast<stWalkoutPartyParam&>(base_param).raw_04;
    if (result == 19) {
        PacketGuard response;
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_WALKOUT_PARTY_MEMBER);
        response.put_byte(0);
        response.put_byte(19);
        response.finalize(true);
        user->Send(response);
    } else if (result > 0) {
        PacketGuard response;
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_WALKOUT_PARTY_MEMBER);
        response.put_byte(0);
        response.put_byte(result);
        response.finalize(true);
        user->Send(response);
    }
}

// IDA_EA: 0x8232806
// IDA_NAME: _ZN23DisPatcher_WalkoutPartyD2Ev
// DEMANGLED: DisPatcher_WalkoutParty::~DisPatcher_WalkoutParty()
// IDA_EA: 0x8232836
// IDA_NAME: _ZN23DisPatcher_WalkoutPartyD0Ev
// DEMANGLED: DisPatcher_WalkoutParty::~DisPatcher_WalkoutParty()
DisPatcher_WalkoutParty::~DisPatcher_WalkoutParty() {}

// IDA_EA: 0x859532C
// IDA_NAME: _ZN23DisPatcher_WalkoutPartyC2Ev
// DEMANGLED: DisPatcher_WalkoutParty::DisPatcher_WalkoutParty(void)
DisPatcher_WalkoutParty::DisPatcher_WalkoutParty()
    : IPacketDispatcher<MSG_WALKOUT_PARTY,
                        stWalkoutPartyParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x81DDF36
// IDA_NAME: _ZN31Dispatcher_Register_Quick_Party4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Register_Quick_Party::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Register_Quick_Party::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    MSG_QUICK_PARTY& request = static_cast<MSG_QUICK_PARTY&>(message);

    if (packet.get_short(request.pool_id) != 1) {
        return LineFunc(20254,
                        "virtual int Dispatcher_Register_Quick_Party::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.channel_type) == 1) {
        return 0;
    }
    return LineFunc(20256,
                    "virtual int Dispatcher_Register_Quick_Party::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81DDFCE
// IDA_NAME: _ZN31Dispatcher_Register_Quick_Party11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Register_Quick_Party::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Register_Quick_Party::check_error(CUser* user,
                                                  MSG_BASE&,
                                                  ParamBase&) {
    return (user && user->get_state() > 3 && user->getCurCharacR()) ? 0 : -1;
}

// IDA_EA: 0x81DE012
// IDA_NAME: _ZN31Dispatcher_Register_Quick_Party7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Register_Quick_Party::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Register_Quick_Party::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& base_param) {
    const int error = check_error(user, message, base_param);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    const MSG_QUICK_PARTY& request =
        static_cast<const MSG_QUICK_PARTY&>(message);
    QuickParty::STQuickPartyPoolMap_Key key;
    key.pool_id = request.pool_id;
    key.channel_type = request.channel_type;

    stQuickPartyParam& result = static_cast<stQuickPartyParam&>(base_param);
    result.result_code = G_CGameManager()->GetQuickPartySystemManager()
                             ->match_proper_party(key, user->GetParty(), user);
    return 0;
}

// IDA_EA: 0x81DE0C6
// IDA_NAME: _ZN31Dispatcher_Register_Quick_Party4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Register_Quick_Party::send(CUser *,ParamBase &)
void Dispatcher_Register_Quick_Party::send(CUser* user, ParamBase& base_param) {
    const stQuickPartyParam& result =
        static_cast<const stQuickPartyParam&>(base_param);
    if (result.result_code == 0) {
        user->SendCmdOkPacket(ENUM_CMDPACKET_REGISTER_QUICK_PARTY);
        return;
    }
    if (result.result_code == 1) {
        PacketGuard response;
        response.put_header(1, ENUM_CMDPACKET_REGISTER_QUICK_PARTY);
        response.put_byte(0);
        response.put_int(result.result_code);
        response.finalize(1);
        user->Send(response);
    }
}

// IDA_EA: 0x8244662
// IDA_NAME: _ZN31Dispatcher_Register_Quick_PartyD2Ev
// DEMANGLED: Dispatcher_Register_Quick_Party::~Dispatcher_Register_Quick_Party()
// IDA_EA: 0x8244692
// IDA_NAME: _ZN31Dispatcher_Register_Quick_PartyD0Ev
// DEMANGLED: Dispatcher_Register_Quick_Party::~Dispatcher_Register_Quick_Party() deleting destructor
Dispatcher_Register_Quick_Party::~Dispatcher_Register_Quick_Party() {}

// IDA_EA: 0x85979F2
// IDA_NAME: _ZN31Dispatcher_Register_Quick_PartyC2Ev
// DEMANGLED: Dispatcher_Register_Quick_Party::Dispatcher_Register_Quick_Party(void)
Dispatcher_Register_Quick_Party::Dispatcher_Register_Quick_Party()
    : IPacketDispatcher<MSG_QUICK_PARTY,
                        stQuickPartyParam,
                        static_cast<ch_state>(5)>() {}

// IDA_EA: 0x81DE1AA
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_Party4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Cancel_Quick_Party::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Cancel_Quick_Party::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81DE1B4
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_Party11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Cancel_Quick_Party::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Cancel_Quick_Party::check_error(CUser* user,
                                               MSG_BASE&,
                                               ParamBase&) {
    if (user->get_state() <= 3 || !user->getCurCharacR()) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81DE1F8
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_Party7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Cancel_Quick_Party::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Cancel_Quick_Party::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    const int error = check_error(user, message, param);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    CParty* const party = user->GetParty();
    QuickParty::CQuickPartySystemManager* const quick_party_manager =
        G_CGameManager()->GetQuickPartySystemManager();
    stQuickPartyParam& result = static_cast<stQuickPartyParam&>(param);
    if (!quick_party_manager->cancel_quick_party(party, true, user)) {
        result.result_code = 1;
    }
    return 0;
}

// IDA_EA: 0x81DE298
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_Party4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Cancel_Quick_Party::send(CUser *,ParamBase &)
void Dispatcher_Cancel_Quick_Party::send(CUser* user, ParamBase& param) {
    const stQuickPartyParam& result =
        static_cast<const stQuickPartyParam&>(param);
    if (result.result_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CANCEL_QUICK_PARTY,
                                 static_cast<unsigned char>(result.result_code));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_CANCEL_QUICK_PARTY);
}

// IDA_EA: 0x8244614
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_PartyD2Ev
// DEMANGLED: Dispatcher_Cancel_Quick_Party::~Dispatcher_Cancel_Quick_Party()
// IDA_EA: 0x8244644
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_PartyD0Ev
// DEMANGLED: Dispatcher_Cancel_Quick_Party::~Dispatcher_Cancel_Quick_Party()
Dispatcher_Cancel_Quick_Party::~Dispatcher_Cancel_Quick_Party() {}

// IDA_EA: 0x8597A0E
// IDA_NAME: _ZN29Dispatcher_Cancel_Quick_PartyC2Ev
// DEMANGLED: Dispatcher_Cancel_Quick_Party::Dispatcher_Cancel_Quick_Party(void)
Dispatcher_Cancel_Quick_Party::Dispatcher_Cancel_Quick_Party()
    : IPacketDispatcher<MSG_BASE,
                        stQuickPartyParam,
                        static_cast<ch_state>(5)>() {}

// IDA_EA: 0x81DE32A
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_Party4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Direct_Entrance_Quick_Party::read(PacketBuf& packet,
                                                 MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81DE2E6
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_Party11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Direct_Entrance_Quick_Party::check_error(CUser* user,
                                                        MSG_BASE&,
                                                        ParamBase&) {
    return (user && user->get_state() > 3 && user->getCurCharacR()) ? 0 : -1;
}

// IDA_EA: 0x81DE334
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_Party7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Direct_Entrance_Quick_Party::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& base_param) {
    const int error = check_error(user, message, base_param);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }
    stQuickPartyParam& result = static_cast<stQuickPartyParam&>(base_param);
    result.result_code = G_CGameManager()->GetQuickPartySystemManager()
                             ->direct_entrance_quick_party(user);
    return 0;
}

// IDA_EA: 0x81DE3B8
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_Party4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::send(CUser *,ParamBase &)
void Dispatcher_Direct_Entrance_Quick_Party::send(CUser* user,
                                                  ParamBase& base_param) {
    const stQuickPartyParam& result =
        static_cast<const stQuickPartyParam&>(base_param);
    if (result.result_code != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_DIRECT_ENTRANCE_DUNGEON_QUICK_PARTY,
            static_cast<unsigned char>(result.result_code));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_DIRECT_ENTRANCE_DUNGEON_QUICK_PARTY);
}

// IDA_EA: 0x82445C6
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_PartyD2Ev
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::~Dispatcher_Direct_Entrance_Quick_Party()
// IDA_EA: 0x82445F6
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_PartyD0Ev
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::~Dispatcher_Direct_Entrance_Quick_Party() deleting destructor
Dispatcher_Direct_Entrance_Quick_Party::~Dispatcher_Direct_Entrance_Quick_Party() {}

// IDA_EA: 0x8597A2A
// IDA_NAME: _ZN38Dispatcher_Direct_Entrance_Quick_PartyC2Ev
// DEMANGLED: Dispatcher_Direct_Entrance_Quick_Party::Dispatcher_Direct_Entrance_Quick_Party(void)
Dispatcher_Direct_Entrance_Quick_Party::Dispatcher_Direct_Entrance_Quick_Party()
    : IPacketDispatcher<MSG_BASE,
                        stQuickPartyParam,
                        static_cast<ch_state>(5)>() {}

// IDA_EA: 0x8597D72
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeRewardC2Ev
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::Dispatcher_PcRoomPlayTimeReward(void)
Dispatcher_PcRoomPlayTimeReward::Dispatcher_PcRoomPlayTimeReward()
    : IPacketDispatcher<MSG_BASE,
                        PcRoomPlayTimeRewardParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8243DDA
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeRewardD2Ev
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::~Dispatcher_PcRoomPlayTimeReward()
// IDA_EA: 0x8243E0A
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeRewardD0Ev
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::~Dispatcher_PcRoomPlayTimeReward()
Dispatcher_PcRoomPlayTimeReward::~Dispatcher_PcRoomPlayTimeReward() {}

// IDA_EA: 0x81E4F52
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PcRoomPlayTimeReward::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81E4F5C
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PcRoomPlayTimeReward::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& base_param) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 3;
    }

    PcRoomPlayTimeRewardParam& param =
        static_cast<PcRoomPlayTimeRewardParam&>(base_param);
    EventClassify::CPcRoomPlayTime* const event =
        static_cast<EventClassify::CPcRoomPlayTime*>(
            G_CDataManager()->get_event_script_mng()->get_event_entity(92));
    if (!event) {
        param.result_ = 178;
        return 0;
    }

    param.result_ = event->verifyUserPcRoomPlayEvent(user);
    if (param.result_ == 0) {
        param.result_ = event->processEventPcRoomPlayTime(user);
    }
    return 0;
}

// IDA_EA: 0x81E5020
// IDA_NAME: _ZN31Dispatcher_PcRoomPlayTimeReward4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PcRoomPlayTimeReward::send(CUser *,ParamBase &)
void Dispatcher_PcRoomPlayTimeReward::send(CUser* user,
                                           ParamBase& base_param) {
    const PcRoomPlayTimeRewardParam& param =
        static_cast<const PcRoomPlayTimeRewardParam&>(base_param);
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_PC_ROOM_PLAY_TIME_REWARD);
    if (param.result_ != 0) {
        packet.put_byte(0);
        packet.put_byte(param.result_);
        if (param.result_ == 19) {
            packet.put_int(user->getTotalPcRoomPlayTime());
        }
    } else {
        packet.put_byte(1);
        packet.put_byte(user->getPcRoomPlayTimeRewardCnt());
    }
    packet.finalize(true);
    user->Send(packet);
}



// IDA_EA: 0x81616CE
// IDA_NAME: _ZN28InterSelectPcroomDailyReward12dispatch_sigEP5CUserPci
// DEMANGLED: InterSelectPcroomDailyReward::dispatch_sig(CUser *,char *,int)
int InterSelectPcroomDailyReward::dispatch_sig(CUser* user,
                                               char* msg,
                                               int length) {
    (void)length;
    if (!msg) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectPcroomDailyReward::dispatch_sig(CUser*, char*, int)",
            410,
            "[Taiwan, pcroom] interMsg is null.");
        return 411;
    }

    user->setPcroomDailyRewardRemain(msg[4]);
    const unsigned int account_id =
        *reinterpret_cast<unsigned int*>(msg);
    const char remain_reward_count = msg[4];
    const char* account_id_text = NumberToString(account_id, 0);
    cMyTrace trace(
        "virtual int InterSelectPcroomDailyReward::dispatch_sig(CUser*, char*, int)",
        415,
        0);
    trace("[Taiwan, pcroom] m_id:%s, remain_reward_count:%d",
          account_id_text,
          remain_reward_count);
    InterSelectPcroomDailyReward::SendPacket(user);
    return 0;
}

// IDA_EA: 0x816179C
// IDA_NAME: _ZN28InterSelectPcroomDailyReward11makeRequestEijc
// DEMANGLED: InterSelectPcroomDailyReward::makeRequest(int,unsigned int,char)
void InterSelectPcroomDailyReward::makeRequest(int uid,
                                               unsigned int account_id,
                                               char remain_reward_count) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
        424);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 13, uid);
    Msg* msg = guard.GetInBuffer<Msg>();
    if (msg) {
        msg->account_id = account_id;
        msg->remain_reward_count = remain_reward_count;
        msg->reserved_05[0] = 0;
        msg->reserved_05[1] = 0;
        msg->reserved_05[2] = 0;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "static void InterSelectPcroomDailyReward::makeRequest(int, memberIdentificationNumber_t, char)",
            429,
            "[Taiwan, pcroom] m_id:%d",
            account_id);
    }
}

// IDA_EA: 0x81618A6
// IDA_NAME: _ZN28InterSelectPcroomDailyReward10SendPacketEP5CUser
// DEMANGLED: InterSelectPcroomDailyReward::SendPacket(CUser *)
void InterSelectPcroomDailyReward::SendPacket(CUser* user) {
    if (user) {
        const char remain_reward_count = user->getPcroomDailyRewardRemain();
        PacketGuard guard;
        guard.put_header(0, ENUM_NOTIPACKET_PCROOM_DAYILY_REWARD_STATE);
        guard.put_byte(remain_reward_count);
        guard.finalize(true);
        user->Send(guard);
    }
}

// IDA_EA: 0x8162170
// IDA_NAME: _ZN28InterSelectPcroomDailyRewardC2Ev
// DEMANGLED: InterSelectPcroomDailyReward::InterSelectPcroomDailyReward(void)
InterSelectPcroomDailyReward::InterSelectPcroomDailyReward()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::SELECT_PCROOM_DAILY_REWARD) {}

// IDA_EA: 0x8162194
// IDA_NAME: _ZN28InterSelectPcroomDailyRewardD2Ev
// DEMANGLED: InterSelectPcroomDailyReward::~InterSelectPcroomDailyReward()
// IDA_EA: 0x81621C4
// IDA_NAME: _ZN28InterSelectPcroomDailyRewardD0Ev
// DEMANGLED: InterSelectPcroomDailyReward::~InterSelectPcroomDailyReward()
InterSelectPcroomDailyReward::~InterSelectPcroomDailyReward() {}

// IDA_EA: 0x84EA070
// IDA_NAME: _ZN22Inter_LoadDungeonClearC2Ev
// DEMANGLED: Inter_LoadDungeonClear::Inter_LoadDungeonClear(void)
Inter_LoadDungeonClear::Inter_LoadDungeonClear() {}

// IDA_EA: 0x84EA08C
// IDA_NAME: _ZN22Inter_LoadDungeonClearD2Ev
// DEMANGLED: Inter_LoadDungeonClear::~Inter_LoadDungeonClear()
// IDA_EA: 0x84EA0BC
// IDA_NAME: _ZN22Inter_LoadDungeonClearD0Ev
// DEMANGLED: Inter_LoadDungeonClear::~Inter_LoadDungeonClear()
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_LoadDungeonClear::~Inter_LoadDungeonClear() {}

// IDA_EA: 0x84C7C42
// IDA_NAME: _ZN22Inter_LoadDungeonClear12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadDungeonClear::dispatch_sig(CUser *,char *,int)
int Inter_LoadDungeonClear::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)payload_size;
    SIG_LOAD_DUNGEON_CLEAR& input =
        *reinterpret_cast<SIG_LOAD_DUNGEON_CLEAR*>(payload);

    if (!user) {
        return 8206;
    }
    if (!user->get_state()) {
        return 8210;
    }
    if (user->IsProgLogout()) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_LoadDungeonClear::dispatch_sig(CUser*, char*, int)",
            8216,
            "[Inter_LoadDungeonClear::dispatch_sig] State(PROG_LOGOUT)\n");
        return 0;
    }
    if (input.result == 1u) {
        return 8224;
    }

    WongWork::CDungeonClear* clear = user->getDungeonClearRefW();
    clear->fromString(user, input.dungeon);
    if (input.needs_member_save != 1u) {
        user->disableSaveDungeonClear();
    }
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(5));
    return 0;
}

// IDA_EA: 0x81ECE64
// IDA_NAME: _ZN17DisPatcher_DiePvP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DiePvP::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_DiePvP::dispatch_sig(CUser* user, PacketBuf& packet) {
    PvP_Room* const room = user ? user->GetPVPRoom() : 0;
    const bool zombie_state5 =
        user && user->get_state() == 5 && room &&
        zombie_mode::IsActiveRoom(room);
    if (!user || (user->get_state() != 7 && !zombie_state5)) {
        return 0;
    }

    uint16_t target_uid = 0xffffu;
    if (packet.get_short(target_uid) != 1) {
        return LineFunc(
            28434,
            "virtual int DisPatcher_DiePvP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CUser* target = G_GameWorld()->find_from_world(target_uid);
    if (!room) {
        return LineFunc(
            28444,
            "virtual int DisPatcher_DiePvP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PacketGuard response;
    const int result = room->die_charac(user, target);
    CSwitchLog trace(
        "virtual int DisPatcher_DiePvP::dispatch_sig(CUser*, PacketBuf&)",
        28450,
        0,
        0);
    trace("pvp@log %s,%s,room(%d),%d,%d,%d",
          user->get_acc_name(),
          user->getCurCharacName(),
          room->get_index(),
          room->get_pvp_battle_mode(),
          result,
          58);
    if (result > 0) {
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_DIE_PVP_CHARACTER);
        response.put_byte(0);
        response.put_byte(result);
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x8232B6A
// IDA_NAME: _ZN17DisPatcher_DiePvP4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DiePvP::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DiePvP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232B74
// IDA_NAME: _ZN17DisPatcher_DiePvP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DiePvP::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DiePvP::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8243810
// IDA_NAME: _ZN17DisPatcher_DiePvPD2Ev
// DEMANGLED: DisPatcher_DiePvP::~DisPatcher_DiePvP()
// IDA_EA: 0x8243840
// IDA_NAME: _ZN17DisPatcher_DiePvPD0Ev
// DEMANGLED: DisPatcher_DiePvP::~DisPatcher_DiePvP()
DisPatcher_DiePvP::~DisPatcher_DiePvP() {}

// IDA_EA: 0x8595ADC
// IDA_NAME: _ZN17DisPatcher_DiePvPC2Ev
// DEMANGLED: DisPatcher_DiePvP::DisPatcher_DiePvP(void)
DisPatcher_DiePvP::DisPatcher_DiePvP()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81ED6C8
// IDA_NAME: _ZN19DisPatcher_EnterPVP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_EnterPVP::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_EnterPVP::dispatch_sig(CUser* user, PacketBuf& packet) {
    short pvp_index = 0;
    if (packet.get_short(pvp_index) != 1) {
        return LineFunc(
            28615,
            "virtual int DisPatcher_EnterPVP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char password[9] = {};
    int password_length = 0;
    char has_password = 0;
    if (packet.get_byte(has_password) != 1) {
        return LineFunc(
            28622,
            "virtual int DisPatcher_EnterPVP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (has_password == 1) {
        if (packet.get_int(password_length) != 1) {
            return LineFunc(
                28626,
                "virtual int DisPatcher_EnterPVP::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (password_length > 8) {
            return LineFunc(
                28628,
                "virtual int DisPatcher_EnterPVP::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (packet.get_binary(password, password_length) != 1) {
            return LineFunc(
                28631,
                "virtual int DisPatcher_EnterPVP::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
    }

    const char narrowed_pvp_index = static_cast<char>(pvp_index);
    PvP_Room* const room = G_CGameManager()->GetPvp(
        static_cast<int>(narrowed_pvp_index), user, 0);
    if (room) {
        EnterPVPRoom(user,
                     room,
                     ENUM_CMDPACKET_ENTER_PVP_ROOM,
                     password);
        return 0;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_ENTER_PVP_ROOM);
    response.put_byte(0);
    response.put_byte(22);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8232B7E
// IDA_NAME: _ZN19DisPatcher_EnterPVP4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_EnterPVP::read(PacketBuf &,MSG_BASE &)
int DisPatcher_EnterPVP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232B88
// IDA_NAME: _ZN19DisPatcher_EnterPVP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_EnterPVP::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_EnterPVP::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8232B92
// IDA_NAME: _ZN19DisPatcher_EnterPVPD2Ev
// DEMANGLED: DisPatcher_EnterPVP::~DisPatcher_EnterPVP()
// IDA_EA: 0x8232BC2
// IDA_NAME: _ZN19DisPatcher_EnterPVPD0Ev
// DEMANGLED: DisPatcher_EnterPVP::~DisPatcher_EnterPVP()
DisPatcher_EnterPVP::~DisPatcher_EnterPVP() {}

// IDA_EA: 0x8595430
// IDA_NAME: _ZN19DisPatcher_EnterPVPC2Ev
// DEMANGLED: DisPatcher_EnterPVP::DisPatcher_EnterPVP(void)
DisPatcher_EnterPVP::DisPatcher_EnterPVP()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81F803E
// IDA_NAME: _ZN18DisPatcher_MakePVP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_MakePVP::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_MakePVP::dispatch_sig(CUser* user, PacketBuf& packet) {
    MSG_MAKE_PVP_ROOM message;
    std::memset(&message, 0, sizeof(message));
    if (ExtractPacket(packet, message) != 1) {
        return LineFunc(
            34230,
            "virtual int DisPatcher_MakePVP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const std::string title(message.title);
    if (CSyncSlangFilter::GetInstance()->HasSlang(title)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MAKE_PVP_ROOM, 159);
        return 0;
    }

    MakePVPRoom(user,
                message,
                ENUM_CMDPACKET_MAKE_PVP_ROOM,
                static_cast<PVP_BATTLE_MODE>(message.option_enabled ? 6 : 2));
    return 0;
}

// IDA_EA: 0x8233214
// IDA_NAME: _ZN18DisPatcher_MakePVP4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_MakePVP::read(PacketBuf &,MSG_BASE &)
int DisPatcher_MakePVP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81F81BA
// IDA_NAME: _ZN18DisPatcher_MakePVP13ExtractPacketER9PacketBufR17MSG_MAKE_PVP_ROOM
// DEMANGLED: DisPatcher_MakePVP::ExtractPacket(PacketBuf &,MSG_MAKE_PVP_ROOM &)
int DisPatcher_MakePVP::ExtractPacket(PacketBuf& packet,
                                      MSG_MAKE_PVP_ROOM& message) {
    int title_len = 0;
    char option_enabled = 0;

    if (packet.get_byte(message.mode) != 1 || message.mode < 0) {
        return 0;
    }
    if (!message.mode) {
        if (packet.get_int(title_len) != 1 ||
            static_cast<unsigned int>(title_len) >= 0x1e) {
            return 0;
        }
        if (packet.get_str(message.title, sizeof(message.title), title_len) !=
            1) {
            return 0;
        }
    }
    if (packet.get_short(message.word_2c) != 1) {
        return 0;
    }
    if (packet.get_byte(message.option_type) != 1) {
        return 0;
    }
    if (message.option_type == 1) {
        if (packet.get_int(message.option_length) != 1) {
            return 0;
        }
        if (message.option_length > 8) {
            return 0;
        }
        if (packet.get_binary(message.option_payload, message.option_length) !=
            1) {
            return 0;
        }
    }
    if (packet.get_byte(option_enabled) != 1) {
        return 0;
    }
    message.option_enabled = 0;
    if (option_enabled == 1) {
        message.option_enabled = 1;
    }
    return 1;
}

// IDA_EA: 0x823321E
// IDA_NAME: _ZN18DisPatcher_MakePVP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_MakePVP::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_MakePVP::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8233228
// IDA_NAME: _ZN18DisPatcher_MakePVPD2Ev
// DEMANGLED: DisPatcher_MakePVP::~DisPatcher_MakePVP()
// IDA_EA: 0x8233258
// IDA_NAME: _ZN18DisPatcher_MakePVPD0Ev
// DEMANGLED: DisPatcher_MakePVP::~DisPatcher_MakePVP()
DisPatcher_MakePVP::~DisPatcher_MakePVP() {}

// IDA_EA: 0x859560C
// IDA_NAME: _ZN18DisPatcher_MakePVPC2Ev
// DEMANGLED: DisPatcher_MakePVP::DisPatcher_MakePVP(void)
DisPatcher_MakePVP::DisPatcher_MakePVP()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823333A
// IDA_NAME: _ZN23DisPatcher_EndPvPResult4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_EndPvPResult::read(PacketBuf &,MSG_BASE &)
int DisPatcher_EndPvPResult::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82335E4
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTemp4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_StartWarRoomTemp::read(PacketBuf &,MSG_BASE &)
int DisPatcher_StartWarRoomTemp::read(PacketBuf& packet,
                                      MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8201B06
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTemp12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_StartWarRoomTemp::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_StartWarRoomTemp::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)user;
    (void)packet;
    return LineFunc(
        38711,
        "virtual int DisPatcher_StartWarRoomTemp::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x82335EE
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTemp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_StartWarRoomTemp::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_StartWarRoomTemp::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82431F8
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTempD2Ev
// DEMANGLED: DisPatcher_StartWarRoomTemp::~DisPatcher_StartWarRoomTemp()
// IDA_EA: 0x8243228
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTempD0Ev
// DEMANGLED: DisPatcher_StartWarRoomTemp::~DisPatcher_StartWarRoomTemp()
DisPatcher_StartWarRoomTemp::~DisPatcher_StartWarRoomTemp() {}

// IDA_EA: 0x8595D98
// IDA_NAME: _ZN27DisPatcher_StartWarRoomTempC2Ev
// DEMANGLED: DisPatcher_StartWarRoomTemp::DisPatcher_StartWarRoomTemp(void)
DisPatcher_StartWarRoomTemp::DisPatcher_StartWarRoomTemp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8233760
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeat4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_PvpHeartBeat::read(PacketBuf &,MSG_BASE &)
int DisPatcher_PvpHeartBeat::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8204458
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeat12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_PvpHeartBeat::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_PvpHeartBeat::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 1 && user->getCurCharacR()) {
        disconnect_detecter::GetInstanceDisconnectDetecter()->RefreshRecvTime(
            user);
        // The heartbeat is the existing server-observed PvP liveness event.
        // Zombie survival uses elapsed server time, so replayed/duplicate
        // heartbeats in the same second cannot add score twice.
        zombie_mode::RecordSurvival(user->GetPVPRoom(), user);
    }
    return 0;
}

// IDA_EA: 0x823376A
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeat7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_PvpHeartBeat::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_PvpHeartBeat::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8242C7C
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeatD2Ev
// DEMANGLED: DisPatcher_PvpHeartBeat::~DisPatcher_PvpHeartBeat()
// IDA_EA: 0x8242CAC
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeatD0Ev
// DEMANGLED: DisPatcher_PvpHeartBeat::~DisPatcher_PvpHeartBeat()
DisPatcher_PvpHeartBeat::~DisPatcher_PvpHeartBeat() {}

// IDA_EA: 0x8595FC8
// IDA_NAME: _ZN23DisPatcher_PvpHeartBeatC2Ev
// DEMANGLED: DisPatcher_PvpHeartBeat::DisPatcher_PvpHeartBeat(void)
DisPatcher_PvpHeartBeat::DisPatcher_PvpHeartBeat()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x85970D8
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPC2Ev
// DEMANGLED: Dispatcher_CompleteLoadPvP::Dispatcher_CompleteLoadPvP(void)
Dispatcher_CompleteLoadPvP::Dispatcher_CompleteLoadPvP()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240666
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPD2Ev
// DEMANGLED: Dispatcher_CompleteLoadPvP::~Dispatcher_CompleteLoadPvP()
// IDA_EA: 0x8240696
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPD0Ev
// DEMANGLED: Dispatcher_CompleteLoadPvP::~Dispatcher_CompleteLoadPvP()
Dispatcher_CompleteLoadPvP::~Dispatcher_CompleteLoadPvP() {}

// IDA_EA: 0x82252FE
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompleteLoadPvP::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompleteLoadPvP::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 1) {
        return LineFunc(
            57349,
            "virtual int Dispatcher_CompleteLoadPvP::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (room && channel_zombie_matching_authority::ConsumeCompleteLoad(
                    user, room)) {
        return 0;
    }
    if (!room || !room->current_match_) {
        return 0;
    }
    room->current_match_->OnCompleteLoad(user);
    return 0;
}

// IDA_EA: 0x823415E
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompleteLoadPvP::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompleteLoadPvP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234168
// IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompleteLoadPvP::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompleteLoadPvP::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85970F4
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPC2Ev
// DEMANGLED: Dispatcher_ConnectP2PPvP::Dispatcher_ConnectP2PPvP(void)
Dispatcher_ConnectP2PPvP::Dispatcher_ConnectP2PPvP()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {
}

// IDA_EA: 0x8240618
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPD2Ev
// DEMANGLED: Dispatcher_ConnectP2PPvP::~Dispatcher_ConnectP2PPvP()
// IDA_EA: 0x8240648
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPD0Ev
// DEMANGLED: Dispatcher_ConnectP2PPvP::~Dispatcher_ConnectP2PPvP()
Dispatcher_ConnectP2PPvP::~Dispatcher_ConnectP2PPvP() {
}

// IDA_EA: 0x8225394
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ConnectP2PPvP::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(
            57366,
            "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    char count = -1;
    if (packet.get_byte(count) != 1) {
        return LineFunc(
            57370,
            "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (static_cast<unsigned char>(count) > 8u) {
        return LineFunc(
            57374,
            "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    // IDA initializes two dwords then writes each client-reported seat state
    // by its validated 0..7 index.  This is that exact eight-byte result
    // record, represented as its real byte array rather than an offset cast.
    char p2p_result[8] = {};
    char seat_result = -1;
    for (int index = 0; index < count; ++index) {
        if (packet.get_byte(seat_result) != 1) {
            return LineFunc(
                57381,
                "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
        if (static_cast<unsigned char>(seat_result) >= 8u) {
            return LineFunc(
                57384,
                "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
        if (packet.get_byte(p2p_result[static_cast<unsigned char>(seat_result)]) !=
            1) {
            return LineFunc(
                57387,
                "virtual int Dispatcher_ConnectP2PPvP::dispatch_sig(CUser*, PacketBuf&)",
                0, 0);
        }
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (room && channel_zombie_matching_authority::ConsumeConnectP2P(
                    user, room, p2p_result,
                    static_cast<unsigned char>(count))) {
        return 0;
    }
    if (room && room->current_match_) {
        room->current_match_->OnConnectP2P(user, room, p2p_result, count);
    }
    return 0;
}

// IDA_EA: 0x8234172
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ConnectP2PPvP::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ConnectP2PPvP::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823417C
// IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ConnectP2PPvP::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ConnectP2PPvP::process(CUser* user, MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x822759C
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPosition12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Change_PartyMemberPosition::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return 0;
    }
    CParty* const party = user->GetParty();
    if (!party || party->getManager() != user) {
        return 0;
    }

    char origin = 4;
    char direction = 0;
    if (packet.get_byte(origin) != 1) {
        return LineFunc(
            58244,
            "virtual int Dispatcher_Change_PartyMemberPosition::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_byte(direction) != 1) {
        return LineFunc(
            58245,
            "virtual int Dispatcher_Change_PartyMemberPosition::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_CHANGE_PARTY_MEMBER_POSITION);
    if (static_cast<unsigned char>(origin) < 4u &&
        (direction == 3 || direction == 1)) {
        const char destination = static_cast<char>(
            (static_cast<int>(origin) + static_cast<int>(direction)) & 3);
        party->switchPostion(origin, destination);
        party->send_party_realtime_info();
        party->send_party_ipinfo();
        response.put_byte(1);
        response.put_byte(origin);
        response.put_byte(destination);
    } else {
        response.put_byte(0);
        response.put_byte(origin);
        response.put_byte(direction);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82342C6
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPosition4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Change_PartyMemberPosition::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82342D0
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPosition7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Change_PartyMemberPosition::process(CUser* user,
                                                    MSG_BASE& message,
                                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8240222
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPositionD2Ev
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::~Dispatcher_Change_PartyMemberPosition()
// IDA_EA: 0x8240252
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPositionD0Ev
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::~Dispatcher_Change_PartyMemberPosition()
Dispatcher_Change_PartyMemberPosition::~Dispatcher_Change_PartyMemberPosition() {}

// IDA_EA: 0x85973EE
// IDA_NAME: _ZN37Dispatcher_Change_PartyMemberPositionC2Ev
// DEMANGLED: Dispatcher_Change_PartyMemberPosition::Dispatcher_Change_PartyMemberPosition(void)
Dispatcher_Change_PartyMemberPosition::Dispatcher_Change_PartyMemberPosition()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x85976C6
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonsterC2Ev
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::Dispatcher_WarRoomWpPerMonster(void)
Dispatcher_WarRoomWpPerMonster::Dispatcher_WarRoomWpPerMonster()
    : IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                        stWarRoomWp,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244EEA
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonsterD2Ev
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::~Dispatcher_WarRoomWpPerMonster()
// IDA_EA: 0x8244F1A
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonsterD0Ev
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::~Dispatcher_WarRoomWpPerMonster()
Dispatcher_WarRoomWpPerMonster::~Dispatcher_WarRoomWpPerMonster() {}

// IDA_EA: 0x81D5BCC
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonster4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::read(PacketBuf &,MSG_BASE &)
int Dispatcher_WarRoomWpPerMonster::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    MSG_WARROOM_WP_PER_MONSTER& request =
        static_cast<MSG_WARROOM_WP_PER_MONSTER&>(message);

    if (packet.get_int(request.monster_index) != 1) {
        return LineFunc(14717,
                        "virtual int Dispatcher_WarRoomWpPerMonster::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_int(request.monster_hp) != 1) {
        return LineFunc(14720,
                        "virtual int Dispatcher_WarRoomWpPerMonster::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(request.target_world_id) != 1) {
        return LineFunc(14723,
                        "virtual int Dispatcher_WarRoomWpPerMonster::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(request.target_party_seat) == 1) {
        return 0;
    }
    return LineFunc(14726,
                    "virtual int Dispatcher_WarRoomWpPerMonster::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81D5CA8
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonster7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_WarRoomWpPerMonster::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    const int error = check_error(user, message);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    const MSG_WARROOM_WP_PER_MONSTER& request =
        static_cast<const MSG_WARROOM_WP_PER_MONSTER&>(message);
    stWarRoomWp& result = static_cast<stWarRoomWp&>(param);
    WarRoom* const room = G_CGameManager()->GetWarRoom(
        static_cast<signed char>(user->GetWarRoomIndex()));
    if (!room) {
        return LineFunc(
            14745,
            "virtual int Dispatcher_WarRoomWpPerMonster::process(CUser*, MSG_BASE&, ParamBase&)",
            0,
            user->get_acc_id());
    }

    CUser* target_user = 0;
    if (request.target_world_id != 0xffffu) {
        target_user = G_GameWorld()->find_from_world(request.target_world_id);
        if (!target_user) {
            return LineFunc(
                14756,
                "virtual int Dispatcher_WarRoomWpPerMonster::process(CUser*, MSG_BASE&, ParamBase&)",
                0,
                user->get_acc_id());
        }
    }

    result.raw_04 = room->HandleWpPerMonster(user,
                                              request.monster_index,
                                              request.monster_hp,
                                              target_user);
    return 0;
}

// IDA_EA: 0x81D5DF2
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonster11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::check_error(CUser *,MSG_BASE &)
int Dispatcher_WarRoomWpPerMonster::check_error(CUser* user,
                                                MSG_BASE& /* message */) {
    if (!user) {
        return LineFunc(14767,
                        "int Dispatcher_WarRoomWpPerMonster::check_error(CUser*, MSG_BASE&)",
                        0,
                        0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(14770,
                        "int Dispatcher_WarRoomWpPerMonster::check_error(CUser*, MSG_BASE&)",
                        0,
                        0);
    }
    return user->get_state() == 8 ? 0 : -1;
}

// IDA_EA: 0x81D5E82
// IDA_NAME: _ZN30Dispatcher_WarRoomWpPerMonster4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_WarRoomWpPerMonster::send(CUser *,ParamBase &)
void Dispatcher_WarRoomWpPerMonster::send(CUser* user, ParamBase& param) {
    const stWarRoomWp& result = static_cast<const stWarRoomWp&>(param);
    if (result.raw_04 != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WARROOM_WP_PER_MONSTER,
                                 static_cast<unsigned char>(result.raw_04));
    }
}

// IDA_EA: 0x859778A
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoomC2Ev
// DEMANGLED: Dispatcher_ChangeHostWarRoom::Dispatcher_ChangeHostWarRoom(void)
Dispatcher_ChangeHostWarRoom::Dispatcher_ChangeHostWarRoom()
    : IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                        stChangeHostWarRoom,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244D16
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoomD2Ev
// DEMANGLED: Dispatcher_ChangeHostWarRoom::~Dispatcher_ChangeHostWarRoom()
// IDA_EA: 0x8244D46
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoomD0Ev
// DEMANGLED: Dispatcher_ChangeHostWarRoom::~Dispatcher_ChangeHostWarRoom()
Dispatcher_ChangeHostWarRoom::~Dispatcher_ChangeHostWarRoom() {}

// IDA_EA: 0x81D735E
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoom4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeHostWarRoom::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeHostWarRoom::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    MSG_CHANGE_HOST_WARROOM& request =
        static_cast<MSG_CHANGE_HOST_WARROOM&>(message);

    if (packet.get_byte(request.requested_seat) != 1) {
        return LineFunc(16782,
                        "virtual int Dispatcher_ChangeHostWarRoom::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_int(request.host_change_value) == 1) {
        return 0;
    }
    return LineFunc(16785,
                    "virtual int Dispatcher_ChangeHostWarRoom::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81D73F6
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoom7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeHostWarRoom::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeHostWarRoom::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    const int error = check_error(user, message);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    MSG_CHANGE_HOST_WARROOM& request =
        static_cast<MSG_CHANGE_HOST_WARROOM&>(message);
    stChangeHostWarRoom& response =
        static_cast<stChangeHostWarRoom&>(param);
    WarRoom* const room =
        G_CGameManager()->GetWarRoom(user->GetWarRoomIndex());
    if (room) {
        response.result_code = room->HostChangeWarRoom(
            user, request.requested_seat, request.host_change_value);
        return 0;
    }

    return LineFunc(
        16807,
        "virtual int Dispatcher_ChangeHostWarRoom::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        user->get_acc_id());
}

// IDA_EA: 0x81D74D4
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoom11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeHostWarRoom::check_error(CUser *,MSG_BASE &)
int Dispatcher_ChangeHostWarRoom::check_error(CUser* user,
                                              MSG_BASE& message) {
    if (!user) {
        return LineFunc(16817,
                        "int Dispatcher_ChangeHostWarRoom::check_error(CUser*, MSG_BASE&)",
                        0,
                        0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(16820,
                        "int Dispatcher_ChangeHostWarRoom::check_error(CUser*, MSG_BASE&)",
                        0,
                        0);
    }

    const MSG_CHANGE_HOST_WARROOM& request =
        static_cast<const MSG_CHANGE_HOST_WARROOM&>(message);
    if (static_cast<unsigned char>(request.requested_seat) < 6u) {
        return user->get_state() == 8 ? 0 : -1;
    }

    return LineFunc(16824,
                    "int Dispatcher_ChangeHostWarRoom::check_error(CUser*, MSG_BASE&)",
                    0,
                    user->get_acc_id());
}

// IDA_EA: 0x81D75B0
// IDA_NAME: _ZN28Dispatcher_ChangeHostWarRoom4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ChangeHostWarRoom::send(CUser *,ParamBase &)
void Dispatcher_ChangeHostWarRoom::send(CUser* user, ParamBase& param) {
    const stChangeHostWarRoom& response =
        static_cast<const stChangeHostWarRoom&>(param);
    if (response.result_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_HOST_WARROOM,
                                 static_cast<unsigned char>(response.result_code));
    }
}

// IDA_EA: 0x8595954
// IDA_NAME: _ZN24Dispatcher_PartyTeleportC2Ev
// DEMANGLED: Dispatcher_PartyTeleport::Dispatcher_PartyTeleport(void)
Dispatcher_PartyTeleport::Dispatcher_PartyTeleport()
    : IPacketDispatcher<MSG_PARTY_TELEPORT,
                        stPartyTeleport,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82448D2
// IDA_NAME: _ZN24Dispatcher_PartyTeleportD2Ev
// DEMANGLED: Dispatcher_PartyTeleport::~Dispatcher_PartyTeleport()
// IDA_EA: 0x8244902
// IDA_NAME: _ZN24Dispatcher_PartyTeleportD0Ev
// DEMANGLED: Dispatcher_PartyTeleport::~Dispatcher_PartyTeleport()
Dispatcher_PartyTeleport::~Dispatcher_PartyTeleport() {}

// IDA_EA: 0x81DBE70
// IDA_NAME: _ZN24Dispatcher_PartyTeleport4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PartyTeleport::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PartyTeleport::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_PARTY_TELEPORT& request =
        static_cast<MSG_PARTY_TELEPORT&>(message);

    if (packet.get_byte(request.village_index) != 1) {
        return LineFunc(19231,
                        "virtual int Dispatcher_PartyTeleport::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.area_index) != 1) {
        return LineFunc(19232,
                        "virtual int Dispatcher_PartyTeleport::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(request.position_x) != 1) {
        return LineFunc(19233,
                        "virtual int Dispatcher_PartyTeleport::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_short(request.position_y) != 1) {
        return LineFunc(19234,
                        "virtual int Dispatcher_PartyTeleport::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_byte(request.direction) == 1) {
        return 0;
    }
    return LineFunc(19235,
                    "virtual int Dispatcher_PartyTeleport::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81DBFD6
// IDA_NAME: _ZN24Dispatcher_PartyTeleport7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PartyTeleport::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PartyTeleport::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    MSG_PARTY_TELEPORT& request =
        static_cast<MSG_PARTY_TELEPORT&>(message);
    stPartyTeleport& result = static_cast<stPartyTeleport&>(param);

    result.result_code = check_error(user, message);
    result.village_index = request.village_index;
    result.aura_cooldown_remaining =
        static_cast<uint32_t>(user->get_aura_avatar_option_value(0));

    const uint32_t now = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurSec());
    if (result.aura_cooldown_remaining <= now) {
        result.aura_cooldown_remaining = 0;
    } else {
        result.aura_cooldown_remaining -= now;
    }

    if (result.result_code > 0) {
        return 0;
    }
    if (result.result_code < 0) {
        return LineFunc(
            19260,
            "virtual int Dispatcher_PartyTeleport::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result_code,
            0);
    }

    CParty* const party = user->GetParty();
    if (!party) {
        result.result_code = 19;
        return 0;
    }

    CPartyTelePort* const teleport = party->GetPartyTelePort();
    teleport->set_teleport_data(request.village_index,
                                request.area_index,
                                request.position_x,
                                request.position_y,
                                request.direction,
                                0);
    teleport->set_teleport_member_state(party->GetMemberSlotNo(user), 1);
    teleport->check_cur_teleport_state();
    return 0;
}

// IDA_EA: 0x81DC19A
// IDA_NAME: _ZN24Dispatcher_PartyTeleport11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_PartyTeleport::check_error(CUser *,MSG_BASE &)
int Dispatcher_PartyTeleport::check_error(CUser* user, MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 19;
    }

    GameWorld* const world = G_GameWorld();
    if (world->IsPVPChannel() || world->IsPvPSkilTreeChannel()) {
        return 19;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 19;
    }
    if (party->getManager() != user) {
        return 23;
    }

    MSG_PARTY_TELEPORT& request =
        static_cast<MSG_PARTY_TELEPORT&>(message);
    Village* const destination = world->getUserVillage(request.village_index);
    if (!destination ||
        request.area_index == destination->get_gate_area()) {
        return 19;
    }

    if (!user->isGMUser()) {
        int aura_option_value = 0;
        const int aura_error =
            user->is_equip_aura_avatar(0, aura_option_value);
        if (aura_error > 0) {
            return aura_error;
        }
    }

    const int aura_expires_at = user->get_aura_avatar_option_value(0);
    if (!user->isGMUser() &&
        GlobalData::s_systemTime_.getCurSec() < aura_expires_at) {
        return 22;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (!party->checkValidUser(seat)) {
            continue;
        }
        CUser* const party_user = party->get_user(seat);
        if (!party_user) {
            continue;
        }
        const int move_error = world->check_move_area(
            party_user,
            request.village_index,
            request.area_index,
            request.position_x,
            request.position_y,
            request.direction,
            true,
            0);
        if (move_error > 0) {
            return move_error;
        }
    }
    return 0;
}

// IDA_EA: 0x81DC426
// IDA_NAME: _ZN24Dispatcher_PartyTeleport4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PartyTeleport::send(CUser *,ParamBase &)
void Dispatcher_PartyTeleport::send(CUser* user, ParamBase& param) {
    stPartyTeleport& result = static_cast<stPartyTeleport&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_PARTY_TELEPORT);
    if (result.result_code != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<uint8_t>(result.result_code));
        packet.put_byte(result.village_index);
        packet.put_int(static_cast<int>(result.aura_cooldown_remaining));
    } else {
        packet.put_byte(1);
    }
    packet.finalize(true);
    user->Send(packet);

    if (result.result_code == 0) {
        CParty* const party = user->GetParty();
        if (party) {
            CPartyTelePort* const teleport = party->GetPartyTelePort();
            teleport->send_teleport_effect_at_different_place(3);
            teleport->send_teleport_status(0);
            teleport->process_teleport();
        }
    }
}

// IDA_EA: 0x81DC598
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirm4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PartyTeleportConfirm::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PartyTeleportConfirm::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    MSG_PARTY_TELEPORT_CONFIRM& request =
        static_cast<MSG_PARTY_TELEPORT_CONFIRM&>(message);
    if (packet.get_byte(request.byte_0d) == 1) {
        return 0;
    }
    return LineFunc(19415,
                    "virtual int Dispatcher_PartyTeleportConfirm::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81DC5EE
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirm7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PartyTeleportConfirm::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PartyTeleportConfirm::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    MSG_PARTY_TELEPORT_CONFIRM& request =
        static_cast<MSG_PARTY_TELEPORT_CONFIRM&>(message);
    stPartyTeleportConfirm& result =
        static_cast<stPartyTeleportConfirm&>(param);

    result.result_code = check_error(user, message);
    if (result.result_code > 0) {
        return 0;
    }
    if (result.result_code < 0) {
        return LineFunc(
            19429,
            "virtual int Dispatcher_PartyTeleportConfirm::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result_code,
            0);
    }

    result.requested_state = request.byte_0d;
    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }

    const int member_slot = party->GetMemberSlotNo(user);
    CPartyTelePort* const teleport = party->GetPartyTelePort();
    if (request.byte_0d == 1u) {
        teleport->set_teleport_member_state(member_slot, 1);
        teleport->check_cur_teleport_state();
    } else if (request.byte_0d == 3u) {
        teleport->set_teleport_state(3);
    } else if (request.byte_0d == 0u) {
        teleport->set_teleport_member_state(member_slot, 0);
        teleport->set_teleport_state(2);
    }
    return 0;
}

// IDA_EA: 0x81DC75C
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirm11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_PartyTeleportConfirm::check_error(CUser *,MSG_BASE &)
int Dispatcher_PartyTeleportConfirm::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 3) {
        return 19;
    }

    GameWorld* const world = G_GameWorld();
    if (world->IsPVPChannel() || world->IsPvPSkilTreeChannel()) {
        return 19;
    }

    CParty* const party = user->GetParty();
    if (party) {
        const signed char teleport_state = static_cast<signed char>(
            party->GetPartyTelePort()->get_teleport_state());
        if (teleport_state < -1 || teleport_state > 3) {
            return 19;
        }
    }
    return 0;
}

// IDA_EA: 0x81DC830
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirm4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_PartyTeleportConfirm::send(CUser *,ParamBase &)
void Dispatcher_PartyTeleportConfirm::send(CUser* user, ParamBase& param) {
    stPartyTeleportConfirm& result =
        static_cast<stPartyTeleportConfirm&>(param);
    if (result.result_code != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_PARTY_TELEPORT_CONFIRM,
                                 static_cast<unsigned char>(result.result_code));
        return;
    }

    user->SendCmdOkPacket(ENUM_CMDPACKET_PARTY_TELEPORT_CONFIRM);
    CParty* const party = user->GetParty();
    if (!party) {
        return;
    }

    CPartyTelePort* const teleport = party->GetPartyTelePort();
    teleport->send_teleport_status(0);
    teleport->process_teleport();
    if (result.requested_state != 1u) {
        teleport->reset_teleport_data();
    }
}

// IDA_EA: 0x8244884
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirmD2Ev
// DEMANGLED: Dispatcher_PartyTeleportConfirm::~Dispatcher_PartyTeleportConfirm()
// IDA_EA: 0x82448B4
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirmD0Ev
// DEMANGLED: Dispatcher_PartyTeleportConfirm::~Dispatcher_PartyTeleportConfirm()
Dispatcher_PartyTeleportConfirm::~Dispatcher_PartyTeleportConfirm() {}

// IDA_EA: 0x8595938
// IDA_NAME: _ZN31Dispatcher_PartyTeleportConfirmC2Ev
// DEMANGLED: Dispatcher_PartyTeleportConfirm::Dispatcher_PartyTeleportConfirm(void)
Dispatcher_PartyTeleportConfirm::Dispatcher_PartyTeleportConfirm()
    : IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                        stPartyTeleportConfirm,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81DF3C6
// IDA_NAME: _ZN31Dispatcher_Check3rdPartyConcent4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Check3rdPartyConcent::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Check3rdPartyConcent::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x85967A8
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPauseC2Ev
// DEMANGLED: Dispatcher_DungeonEventStoryPause::Dispatcher_DungeonEventStoryPause(void)
Dispatcher_DungeonEventStoryPause::Dispatcher_DungeonEventStoryPause()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241A82
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPauseD2Ev
// DEMANGLED: Dispatcher_DungeonEventStoryPause::~Dispatcher_DungeonEventStoryPause()
// IDA_EA: 0x8241AB2
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPauseD0Ev
// DEMANGLED: Dispatcher_DungeonEventStoryPause::~Dispatcher_DungeonEventStoryPause()
Dispatcher_DungeonEventStoryPause::~Dispatcher_DungeonEventStoryPause() {}

// IDA_EA: 0x8216DCC
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPause12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DungeonEventStoryPause::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DungeonEventStoryPause::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() != 5) {
        return 0;
    }

    char pause_state = 0;
    char story_type = 0;
    if (packet.get_byte(pause_state) != 1) {
        return LineFunc(
            51334,
            "virtual int Dispatcher_DungeonEventStoryPause::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_byte(story_type) != 1) {
        return LineFunc(
            51339,
            "virtual int Dispatcher_DungeonEventStoryPause::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    if (!user->getCurCharacR()) {
        return 0;
    }
    CParty* const party = user->GetParty();
    UserQuest* const user_quest = user->getCurCharacQuestR();
    if (!user_quest || !party) {
        return 0;
    }

    char forward_story_state = 0;
    if (story_type != 0) {
        if (story_type == 1 || story_type == 2) {
            // CLIENT_140828_EVIDENCE: NPC/dialogue completion senders
            // sub_1233D10 and sub_12427E0 encode the second byte as 0 or 2
            // from the dialogue object's +0x74 flag.  The first byte is the
            // pause/resume state; NOTI170 handler sub_581A10 accepts story
            // type 2 in the same state-application branch as type 0.  The
            // older server body knew only type 1 and silently dropped type 2,
            // leaving later-room monsters and map-move carts paused.
            forward_story_state = 1;
        }
    } else {
        MazeScript* const maze = party->battle_field().getMaze();
        if (maze && user_quest->CheckDungeonEventStoryPause(
                        user->getPlayingDungeonIndex(),
                        party->battle_field().GetCurrentMapIndex(),
                        maze->value_50) != 0) {
            forward_story_state = 1;
        }
    }

    if (forward_story_state != 0) {
        PacketGuard response;
        response.put_header(0, ENUM_NOTIPACKET_DUNGEON_EVENT_STORY_PAUSE);
        response.put_short(user->get_unique_id());
        response.put_byte(pause_state);
        response.put_byte(story_type);
        response.finalize(true);
        party->send_to_party(response);
    }
    return 0;
}

// IDA_EA: 0x8233CAE
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPause4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DungeonEventStoryPause::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DungeonEventStoryPause::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233CB8
// IDA_NAME: _ZN33Dispatcher_DungeonEventStoryPause7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DungeonEventStoryPause::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DungeonEventStoryPause::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x812BE82
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoom4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::read(PacketBuf &,MSG_BASE &)
int DisPatcher_EventDungeon_ClearRoom::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    CMDPacketStruct::_STReqEventDungeonClearRoom& request =
        static_cast<CMDPacketStruct::_STReqEventDungeonClearRoom&>(message);
    packet.get_binary(static_cast<char*>(static_cast<void*>(&request)),
                      sizeof(request));
    return 0;
}

// IDA_EA: 0x812BD54
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObject4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::read(PacketBuf &,MSG_BASE &)
int DisPatcher_EventDungeon_DestoryObject::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    CMDPacketStruct::_STReqEventDungeonDestoryObject& request =
        static_cast<CMDPacketStruct::_STReqEventDungeonDestoryObject&>(message);
    packet.get_binary(static_cast<char*>(static_cast<void*>(&request)),
                      sizeof(request));
    return 0;
}

// IDA_EA: 0x820350E
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoom12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_QuickJoinRoom::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_QuickJoinRoom::dispatch_sig(CUser* user, PacketBuf& packet) {
    unsigned char join_type = 0;
    if (packet.get_byte(join_type) != 1) {
        return LineFunc(
            39474,
            "virtual int Dispatcher_QuickJoinRoom::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (user->isCompetitionMercenary()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_QUICK_JOIN_ROOM, 21);
        return 0;
    }

    if (join_type) {
        if (join_type == 1) {
            PacketGuard response;
            response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                ENUM_CMDPACKET_QUICK_JOIN_ROOM);
            response.put_byte(0);
            response.put_byte(4);
            response.finalize(true);
            user->Send(response);
            return 0;
        }
        if (join_type == 2) {
            CEventBase* const event =
                GlobalData::s_event_manager->GetRepeatEvent(95);
            if (event->IsEventing(0) != 1) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_QUICK_JOIN_ROOM, 8);
                return 0;
            }
            BlueMarble* const blue_marble =
                G_CGameManager()->findJoinableBlueMarble();
            if (!blue_marble) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_QUICK_JOIN_ROOM, 190);
                return 0;
            }
            const int result = blue_marble->enterBlueMarble(user);
            if (result > 0) {
                user->SendCmdErrorPacket(
                    ENUM_CMDPACKET_QUICK_JOIN_ROOM,
                    static_cast<unsigned char>(result));
                return 0;
            }
        }
        return 0;
    }

    PvP_Room* const room =
        G_CGameManager()->FindQuickJoinablePvPRoom(true, user);
    if (room) {
        char password[9] = {};
        if (EnterPVPRoom(user,
                         room,
                         ENUM_CMDPACKET_QUICK_JOIN_ROOM,
                         password) != 1) {
            return 0;
        }
        return 0;
    }

    MSG_MAKE_PVP_ROOM message;
    std::memset(&message, 0, sizeof(message));
    message.word_2c = 1;
    message.mode = 1;
    if (MakePVPRoom(user,
                    message,
                    ENUM_CMDPACKET_QUICK_JOIN_ROOM,
                    PVP_BATTLE_MODE_2) != 1) {
        return 0;
    }
    return 0;
}

// IDA_EA: 0x82336E8
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoom4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_QuickJoinRoom::read(PacketBuf &,MSG_BASE &)
int Dispatcher_QuickJoinRoom::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82336F2
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoom7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_QuickJoinRoom::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_QuickJoinRoom::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8242E02
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoomD2Ev
// DEMANGLED: Dispatcher_QuickJoinRoom::~Dispatcher_QuickJoinRoom()
// IDA_EA: 0x8242E32
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoomD0Ev
// DEMANGLED: Dispatcher_QuickJoinRoom::~Dispatcher_QuickJoinRoom()
Dispatcher_QuickJoinRoom::~Dispatcher_QuickJoinRoom() {}

// IDA_EA: 0x8595E94
// IDA_NAME: _ZN24Dispatcher_QuickJoinRoomC2Ev
// DEMANGLED: Dispatcher_QuickJoinRoom::Dispatcher_QuickJoinRoom(void)
Dispatcher_QuickJoinRoom::Dispatcher_QuickJoinRoom()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81F5EBA
// IDA_NAME: _ZN21DisPatcher_SetPVPSeat12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetPVPSeat::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetPVPSeat::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 5) {
        return 0;
    }

    unsigned char seat = 0;
    unsigned char state = 0;
    if (packet.get_byte(seat) != 1 || seat >= 8u) {
        return LineFunc(
            33444,
            "virtual int DisPatcher_SetPVPSeat::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_byte(state) != 1) {
        return LineFunc(
            33446,
            "virtual int DisPatcher_SetPVPSeat::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return LineFunc(
            33449,
            "virtual int DisPatcher_SetPVPSeat::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (static_cast<unsigned char>(room->OnSetPVPSeatState(
            user,
            static_cast<int>(seat),
            static_cast<ENUM_SEAT_STATE>(state),
            static_cast<ENUM_WALKOUT_TYPE>(2))) != 1u) {
        room->reset();
        G_CGameManager()->PutPvp(room);
    }
    return 0;
}

// IDA_EA: 0x823308C
// IDA_NAME: _ZN21DisPatcher_SetPVPSeat4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPVPSeat::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPVPSeat::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233096
// IDA_NAME: _ZN21DisPatcher_SetPVPSeat7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPVPSeat::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPVPSeat::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82330A0
// IDA_NAME: _ZN21DisPatcher_SetPVPSeatD2Ev
// DEMANGLED: DisPatcher_SetPVPSeat::~DisPatcher_SetPVPSeat()
// IDA_EA: 0x82330D0
// IDA_NAME: _ZN21DisPatcher_SetPVPSeatD0Ev
// DEMANGLED: DisPatcher_SetPVPSeat::~DisPatcher_SetPVPSeat()
DisPatcher_SetPVPSeat::~DisPatcher_SetPVPSeat() {}

// IDA_EA: 0x859559C
// IDA_NAME: _ZN21DisPatcher_SetPVPSeatC2Ev
// DEMANGLED: DisPatcher_SetPVPSeat::DisPatcher_SetPVPSeat(void)
DisPatcher_SetPVPSeat::DisPatcher_SetPVPSeat()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82330EE
// IDA_NAME: _ZN21DisPatcher_SetPVPTeam4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPVPTeam::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPVPTeam::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81F6020
// IDA_NAME: _ZN21DisPatcher_SetPVPTeam12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetPVPTeam::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetPVPTeam::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 6) {
        return 0;
    }

    PacketGuard response;
    char requested_mode = 0;
    if (packet.get_byte(requested_mode) != 1) {
        return LineFunc(
            33470,
            "virtual int DisPatcher_SetPVPTeam::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return LineFunc(
            33475,
            "virtual int DisPatcher_SetPVPTeam::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    bool map_mode_allowed = requested_mode != 4;
    if (!map_mode_allowed) {
        // The old short-circuit constructs PvPMapInfo only for mode 4 and
        // deliberately ignores the lookup's return value.
        PvPMapInfo map_info;
        (void)CMapList::GetPvpMapInfo(room->GetRequestedMapNo(), map_info);
        map_mode_allowed = map_info.enabled == 1;
    }
    if (!map_mode_allowed) {
        return LineFunc(
            33482,
            "virtual int DisPatcher_SetPVPTeam::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int result = room->set_pvp_mode(user, requested_mode);
    if (result <= 0) {
        // The raw ABI is exactly PvP_Room::make_seat_info(char *,int); no
        // packet-buffer offset is inferred or introduced here.
        room->make_seat_info(reinterpret_cast<char*>(&response), 0xffff);
        G_GameWorld()->send_all(response);
    } else {
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_SET_PVP_TEAM_MODE);
        response.put_byte(0);
        response.put_byte(result);
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x82330F8
// IDA_NAME: _ZN21DisPatcher_SetPVPTeam7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPVPTeam::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPVPTeam::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8233102
// IDA_NAME: _ZN21DisPatcher_SetPVPTeamD2Ev
// DEMANGLED: DisPatcher_SetPVPTeam::~DisPatcher_SetPVPTeam()
// IDA_EA: 0x8233132
// IDA_NAME: _ZN21DisPatcher_SetPVPTeamD0Ev
// DEMANGLED: DisPatcher_SetPVPTeam::~DisPatcher_SetPVPTeam()
DisPatcher_SetPVPTeam::~DisPatcher_SetPVPTeam() {}

// IDA_EA: 0x85955B8
// IDA_NAME: _ZN21DisPatcher_SetPVPTeamC2Ev
// DEMANGLED: DisPatcher_SetPVPTeam::DisPatcher_SetPVPTeam(void)
DisPatcher_SetPVPTeam::DisPatcher_SetPVPTeam()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x812C090
// IDA_NAME: _ZN15CMDPacketStruct31_STReqEventDungeonDestoryObjectC2Ev
// DEMANGLED: CMDPacketStruct::_STReqEventDungeonDestoryObject::_STReqEventDungeonDestoryObject(void)
CMDPacketStruct::_STReqEventDungeonDestoryObject::
    _STReqEventDungeonDestoryObject()
    : MSG_BASE() {
    std::memset(this, 0, 0x18u);
}

// IDA_EA: 0x812C0DA
// IDA_NAME: _ZN15CMDPacketStruct31_STResEventDungeonDestoryObjectC2Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonDestoryObject::_STResEventDungeonDestoryObject(void)
CMDPacketStruct::_STResEventDungeonDestoryObject::
    _STResEventDungeonDestoryObject()
    : ParamBase() {
    // The original C2 stores the derived vptr and immediately clears exactly
    // the six-byte wire object.  Keep that observed order/extent: the typed
    // destructor below is invoked directly by the concrete template.
    std::memset(this, 0, 0x06u);
}

// IDA_EA: 0x812C2FE
// IDA_NAME: _ZN15CMDPacketStruct31_STResEventDungeonDestoryObjectD2Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonDestoryObject::~_STResEventDungeonDestoryObject()
// IDA_EA: 0x812C32E
// IDA_NAME: _ZN15CMDPacketStruct31_STResEventDungeonDestoryObjectD0Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonDestoryObject::~_STResEventDungeonDestoryObject()
CMDPacketStruct::_STResEventDungeonDestoryObject::
    ~_STResEventDungeonDestoryObject() {}

// IDA_EA: 0x812C112
// IDA_NAME: _ZN15CMDPacketStruct27_STReqEventDungeonClearRoomC2Ev
// DEMANGLED: CMDPacketStruct::_STReqEventDungeonClearRoom::_STReqEventDungeonClearRoom(void)
CMDPacketStruct::_STReqEventDungeonClearRoom::_STReqEventDungeonClearRoom()
    : MSG_BASE() {
    std::memset(this, 0, 0x14u);
}

// IDA_EA: 0x812C140
// IDA_NAME: _ZN15CMDPacketStruct27_STResEventDungeonClearRoomC2Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonClearRoom::_STResEventDungeonClearRoom(void)
CMDPacketStruct::_STResEventDungeonClearRoom::_STResEventDungeonClearRoom()
    : ParamBase() {
    // This C2 likewise zeros the old binary's complete 12-byte response
    // extent after STBaseResponse/ParamBase construction.
    std::memset(this, 0, 0x0cu);
}

// IDA_EA: 0x812C2B0
// IDA_NAME: _ZN15CMDPacketStruct27_STResEventDungeonClearRoomD2Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonClearRoom::~_STResEventDungeonClearRoom()
// IDA_EA: 0x812C2E0
// IDA_NAME: _ZN15CMDPacketStruct27_STResEventDungeonClearRoomD0Ev
// DEMANGLED: CMDPacketStruct::_STResEventDungeonClearRoom::~_STResEventDungeonClearRoom()
CMDPacketStruct::_STResEventDungeonClearRoom::~_STResEventDungeonClearRoom() {}

// IDA_EA: 0x859827A
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                  CMDPacketStruct::_STResEventDungeonDestoryObject,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x812C214
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x812C244
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                  CMDPacketStruct::_STResEventDungeonDestoryObject,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8598296
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                  CMDPacketStruct::_STResEventDungeonClearRoom,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x812C262
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x812C292
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                  CMDPacketStruct::_STResEventDungeonClearRoom,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x812C546
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    CMDPacketStruct::_STReqEventDungeonDestoryObject message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    CMDPacketStruct::_STResEventDungeonDestoryObject param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x812C6A2
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x812C6AC
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                      CMDPacketStruct::_STResEventDungeonDestoryObject,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "localchina/../PacketDispatcher.h",
                           kEventDungeonDestoryObjectTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x812C746
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct31_STReqEventDungeonDestoryObjectENS0_31_STResEventDungeonDestoryObjectEL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,CMDPacketStruct::_STResEventDungeonDestoryObject,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                       CMDPacketStruct::_STResEventDungeonDestoryObject,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x812C34C
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    CMDPacketStruct::_STReqEventDungeonClearRoom message;
    int result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    CMDPacketStruct::_STResEventDungeonClearRoom param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x812C4A8
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x812C4B2
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                      CMDPacketStruct::_STResEventDungeonClearRoom,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "localchina/../PacketDispatcher.h",
                           kEventDungeonClearRoomTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x812C740
// IDA_NAME: _ZN17IPacketDispatcherIN15CMDPacketStruct27_STReqEventDungeonClearRoomENS0_27_STResEventDungeonClearRoomEL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,CMDPacketStruct::_STResEventDungeonClearRoom,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                       CMDPacketStruct::_STResEventDungeonClearRoom,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x812BEB0
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoom11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::check_error(CUser *,MSG_BASE &)
int DisPatcher_EventDungeon_ClearRoom::check_error(CUser*, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x812BEBA
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoom7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_EventDungeon_ClearRoom::process(CUser* user,
                                               MSG_BASE& base_message,
                                               ParamBase& base_param) {
    CMDPacketStruct::_STReqEventDungeonClearRoom& message =
        static_cast<CMDPacketStruct::_STReqEventDungeonClearRoom&>(base_message);
    CMDPacketStruct::_STResEventDungeonClearRoom& param =
        static_cast<CMDPacketStruct::_STResEventDungeonClearRoom&>(base_param);

    param.word_04 = message.word_0e;
    param.word_08 = message.word_12;
    param.word_06 = message.word_10;
    check_error(user, base_message);
    return 0;
}

// IDA_EA: 0x812BF16
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoom4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::send(CUser *,ParamBase &)
void DisPatcher_EventDungeon_ClearRoom::send(CUser* user,
                                             ParamBase& base_param) {
    CMDPacketStruct::_STResEventDungeonClearRoom& param =
        static_cast<CMDPacketStruct::_STResEventDungeonClearRoom&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_EVENT_DUNGEON_CLEAR_ROOM);
    response.put_byte(1);
    response.put_binary(static_cast<char*>(static_cast<void*>(&param)), 12);
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x812C1C6
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoomD2Ev
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::~DisPatcher_EventDungeon_ClearRoom()
// IDA_EA: 0x812C1F6
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoomD0Ev
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::~DisPatcher_EventDungeon_ClearRoom()
DisPatcher_EventDungeon_ClearRoom::~DisPatcher_EventDungeon_ClearRoom() {}

// IDA_EA: 0x85957E8
// IDA_NAME: _ZN33DisPatcher_EventDungeon_ClearRoomC2Ev
// DEMANGLED: DisPatcher_EventDungeon_ClearRoom::DisPatcher_EventDungeon_ClearRoom(void)
DisPatcher_EventDungeon_ClearRoom::DisPatcher_EventDungeon_ClearRoom()
    : IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                        CMDPacketStruct::_STResEventDungeonClearRoom,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x812BD82
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObject11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::check_error(CUser *,MSG_BASE &)
int DisPatcher_EventDungeon_DestoryObject::check_error(CUser*, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x812BD8C
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObject7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_EventDungeon_DestoryObject::process(CUser* user,
                                                    MSG_BASE& base_message,
                                                    ParamBase& base_param) {
    CMDPacketStruct::_STReqEventDungeonDestoryObject& message =
        static_cast<CMDPacketStruct::_STReqEventDungeonDestoryObject&>(base_message);
    CMDPacketStruct::_STResEventDungeonDestoryObject& param =
        static_cast<CMDPacketStruct::_STResEventDungeonDestoryObject&>(base_param);

    param.word_04 = message.word_0e;
    check_error(user, base_message);
    return 0;
}

// IDA_EA: 0x812BDCC
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObject4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::send(CUser *,ParamBase &)
void DisPatcher_EventDungeon_DestoryObject::send(CUser* user,
                                                 ParamBase& base_param) {
    CMDPacketStruct::_STResEventDungeonDestoryObject& param =
        static_cast<CMDPacketStruct::_STResEventDungeonDestoryObject&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_EVENT_DUNGEON_DESTORY_OBJECT);
    response.put_byte(1);
    response.put_binary(static_cast<char*>(static_cast<void*>(&param)), 6);
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x812C178
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObjectD2Ev
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::~DisPatcher_EventDungeon_DestoryObject()
// IDA_EA: 0x812C1A8
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObjectD0Ev
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::~DisPatcher_EventDungeon_DestoryObject()
DisPatcher_EventDungeon_DestoryObject::~DisPatcher_EventDungeon_DestoryObject() {}

// IDA_EA: 0x85957CC
// IDA_NAME: _ZN37DisPatcher_EventDungeon_DestoryObjectC2Ev
// DEMANGLED: DisPatcher_EventDungeon_DestoryObject::DisPatcher_EventDungeon_DestoryObject(void)
DisPatcher_EventDungeon_DestoryObject::DisPatcher_EventDungeon_DestoryObject()
    : IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                        CMDPacketStruct::_STResEventDungeonDestoryObject,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x81DF3D0
// IDA_NAME: _ZN31Dispatcher_Check3rdPartyConcent7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Check3rdPartyConcent::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Check3rdPartyConcent::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)message;
    (void)param;

    if (user->get_state() == 3) {
        DB_Check3rdPartyConcent::makeRequest(
            user->GetUID(), static_cast<unsigned int>(user->get_acc_id()));
    }
    return 0;
}

// IDA_EA: 0x824448E
// IDA_NAME: _ZN31Dispatcher_Check3rdPartyConcentD2Ev
// DEMANGLED: Dispatcher_Check3rdPartyConcent::~Dispatcher_Check3rdPartyConcent()
// IDA_EA: 0x82444BE
// IDA_NAME: _ZN31Dispatcher_Check3rdPartyConcentD0Ev
// DEMANGLED: Dispatcher_Check3rdPartyConcent::~Dispatcher_Check3rdPartyConcent()
Dispatcher_Check3rdPartyConcent::~Dispatcher_Check3rdPartyConcent() {}

// IDA_EA: 0x8597A9A
// IDA_NAME: _ZN31Dispatcher_Check3rdPartyConcentC2Ev
// DEMANGLED: Dispatcher_Check3rdPartyConcent::Dispatcher_Check3rdPartyConcent(void)
Dispatcher_Check3rdPartyConcent::Dispatcher_Check3rdPartyConcent()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x84E6096
// IDA_NAME: _ZN26Inter_Check3rdPartyConcent12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Check3rdPartyConcent::dispatch_sig(CUser *,char *,int)
int Inter_Check3rdPartyConcent::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    (void)payload_size;

    if (user->get_state() > 2) {
        PacketGuard response;
        response.put_header(1, ENUM_CMDPACKET_CHECK_3RDPARTY_CONCENT);
        response.put_byte(1);
        response.put_byte(*payload);
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x84EECC4
// IDA_NAME: _ZN26Inter_Check3rdPartyConcentC2Ev
// DEMANGLED: Inter_Check3rdPartyConcent::Inter_Check3rdPartyConcent(void)
Inter_Check3rdPartyConcent::Inter_Check3rdPartyConcent() {}

// IDA_EA: 0x84F5E44
// IDA_NAME: _ZN26Inter_Check3rdPartyConcentD2Ev
// DEMANGLED: Inter_Check3rdPartyConcent::~Inter_Check3rdPartyConcent()
// IDA_EA: 0x84F5E74
// IDA_NAME: _ZN26Inter_Check3rdPartyConcentD0Ev
// DEMANGLED: Inter_Check3rdPartyConcent::~Inter_Check3rdPartyConcent()
Inter_Check3rdPartyConcent::~Inter_Check3rdPartyConcent() {}
