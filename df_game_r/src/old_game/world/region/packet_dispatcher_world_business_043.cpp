#include "old_game/world/region/packet_dispatcher_world_business_043.h"

#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8230EAE
// IDA_NAME: _ZN16stStartGameParamC2Ev
// DEMANGLED: stStartGameParam::stStartGameParam(void)
stStartGameParam::stStartGameParam()
    : ParamBase(),
      result(0),
      response_detail(0),
      forbidden_move_left_seconds(0u) {}

// IDA_EA: 0x8247DA4
// IDA_NAME: _ZN16stStartGameParamD2Ev
// DEMANGLED: stStartGameParam::~stStartGameParam()
// IDA_EA: 0x8247DD4
// IDA_NAME: _ZN16stStartGameParamD0Ev
// DEMANGLED: stStartGameParam::~stStartGameParam()
stStartGameParam::~stStartGameParam() {}


// IDA_EA: 0x8595A34
// IDA_NAME: _ZN20DisPatcher_StartGameC2Ev
// DEMANGLED: DisPatcher_StartGame::DisPatcher_StartGame(void)
DisPatcher_StartGame::DisPatcher_StartGame() {}

// IDA_EA: 0x82462B8
// IDA_NAME: _ZN20DisPatcher_StartGameD2Ev
// DEMANGLED: DisPatcher_StartGame::~DisPatcher_StartGame()
// IDA_EA: 0x82462E8
// IDA_NAME: _ZN20DisPatcher_StartGameD0Ev
// DEMANGLED: DisPatcher_StartGame::~DisPatcher_StartGame()
DisPatcher_StartGame::~DisPatcher_StartGame() {}

// IDA_EA: 0x81C9F6C
// IDA_NAME: _ZN20DisPatcher_StartGame11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_StartGame::check_error(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_StartGame::check_error(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)message;
    stStartGameParam& start_game_param =
        static_cast<stStartGameParam&>(param);

    if (user->get_state() != 3 && user->get_state() != 11) {
        return 0x7fffffff;
    }
    if (G_GameWorld()->IsPVPChannel()) {
        start_game_param.response_detail = -1;
        return 8;
    }
    if (static_cast<unsigned char>(user->IsBusyDisjointStore()) != 0u) {
        start_game_param.response_detail = -1;
        return 19;
    }
    if (user->isCompetitionMercenary()) {
        start_game_param.response_detail = -1;
        return 234;
    }

    CParty* const party = user->GetParty();
    if (party) {
        CPartyTelePort* const teleport = party->GetPartyTelePort();
        if ((static_cast<unsigned char>(teleport->get_teleport_state()) &
             0x80u) == 0u &&
            static_cast<signed char>(teleport->get_teleport_state()) <= 3) {
            teleport->reset_teleport_data();
            teleport->send_teleport_status(0);
        }
    }

    if (G_GameWorld()->IsForbiddenMove(user->getCurCharacName())) {
        const uint32_t left_seconds =
            G_GameWorld()->GetLeftTimeOfForbiddenMove(
                user->getCurCharacName());
        if (left_seconds != 0u) {
            start_game_param.response_detail = 0;
            start_game_param.forbidden_move_left_seconds = left_seconds;
            return 68;
        }
        G_GameWorld()->EnableMove(user->getCurCharacName());
    }

    if (static_cast<unsigned char>(user->CheckInParty()) == 0u ||
        user->get_state() != 3) {
        return 0;
    }

    CParty* const current_party = user->GetParty();
    if (!current_party) {
        return 0;
    }
    if (static_cast<unsigned char>(current_party->CheckMemberArea()) != 1u &&
        static_cast<unsigned char>(current_party->is_quick_party()) != 1u) {
        start_game_param.response_detail = -1;
        return 21;
    }

    if (excludeAddHackCount(user)) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(502u),
            1u,
            0u,
            0u);
    }

    const int world_map_index = G_GameWorld()->GetWorldMapIndex(user);
    if (world_map_index <= 0) {
        return 0;
    }
    CWorldMapList* const world_map_list =
        CDataManagerWorldMapList(G_CDataManager());
    CWorldMap* const world_map = world_map_list
                                    ? world_map_list->find_world_map(
                                          world_map_index)
                                    : 0;
    if (!world_map) {
        start_game_param.response_detail = -1;
        return 21;
    }
    if (static_cast<unsigned char>(
            GlobalData::s_revengeDungeonMgr->IsOpenRevengeDungeon()) == 1u) {
        return 0;
    }
    if (world_map->getDungeonMinLevel() <
        current_party->getMemberMinLevel() + 6) {
        return 0;
    }
    start_game_param.response_detail = -1;
    return 14;
}

// IDA_EA: 0x81C9F62
// IDA_NAME: _ZN20DisPatcher_StartGame4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_StartGame::read(PacketBuf &,MSG_BASE &)
int DisPatcher_StartGame::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81CA2A8
// IDA_NAME: _ZN20DisPatcher_StartGame19excludeAddHackCountEP5CUser
// DEMANGLED: DisPatcher_StartGame::excludeAddHackCount(CUser *)
bool DisPatcher_StartGame::excludeAddHackCount(CUser* user) {
    if (!user) {
        return false;
    }
    return !G_GameWorld()->isDungeonEntranceArea(user) &&
           user->getMoveSpace() != 1;
}

// IDA_EA: 0x81CA2FE
// IDA_NAME: _ZN20DisPatcher_StartGame7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_StartGame::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_StartGame::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    stStartGameParam& start_game_param =
        static_cast<stStartGameParam&>(param);
    start_game_param.result = check_error(user, message, param);
    if (start_game_param.result > 0) {
        return 0;
    }
    if (start_game_param.result >= 0) {
        const int world_map_index = G_GameWorld()->GetWorldMapIndex(user);
        if (world_map_index > 0) {
            CWorldMapList* const world_map_list =
                CDataManagerWorldMapList(G_CDataManager());
            CWorldMap* const world_map = world_map_list
                                            ? world_map_list->find_world_map(
                                                  world_map_index)
                                            : 0;
            if (world_map) {
                (void)world_map->hasDeathTower();
            }
        }

        if (static_cast<unsigned char>(user->CheckInParty()) != 0u) {
            if (user->get_state() == 3) {
                CParty* const party = user->GetParty();
                if (party) {
                    party->game_start(user);
                }
            }
        } else {
            CParty* const party = G_CGameManager()->GetParty();
            if (party) {
                if (excludeAddHackCount(user)) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(502u),
                        1u,
                        0u,
                        0u);
                }
                party->set_single_play(user);
                party->game_start(user);
            } else {
                LogManager::logFormat(
                    1,
                    "PacketDispatcher_Impl_1.cpp",
                    "virtual int DisPatcher_StartGame::process(CUser*, MSG_BASE&, ParamBase&)",
                    0x1e99,
                    "cant party memory : you must free party memory!!!");
            }
        }
        return 0;
    }

    return LineFunc(
        7771,
        "virtual int DisPatcher_StartGame::process(CUser*, MSG_BASE&, ParamBase&)",
        start_game_param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81CA4F2
// IDA_NAME: _ZN20DisPatcher_StartGame4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_StartGame::send(CUser *,ParamBase &)
void DisPatcher_StartGame::send(CUser* user, ParamBase& param) {
    stStartGameParam& start_game_param =
        static_cast<stStartGameParam&>(param);
    if (start_game_param.result == 0x7FFFFFFF ||
        start_game_param.result == 0) {
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ENTER_SELECT_DUNGEON);
    packet.put_byte(0);
    packet.put_byte(start_game_param.result);
    if (start_game_param.response_detail >= 0) {
        packet.put_byte(start_game_param.response_detail);
    }
    if (start_game_param.result == 68) {
        packet.put_int(start_game_param.forbidden_move_left_seconds);
    }
    packet.finalize(true);
    user->Send(packet);
}


// IDA_EA: 0x8595E5C
// IDA_NAME: _ZN21Dispatcher_MoveToGateC2Ev
// DEMANGLED: Dispatcher_MoveToGate::Dispatcher_MoveToGate(void)
Dispatcher_MoveToGate::Dispatcher_MoveToGate() {}

// IDA_EA: 0x8242E9E
// IDA_NAME: _ZN21Dispatcher_MoveToGateD2Ev
// DEMANGLED: Dispatcher_MoveToGate::~Dispatcher_MoveToGate()
// IDA_EA: 0x8242ECE
// IDA_NAME: _ZN21Dispatcher_MoveToGateD0Ev
// DEMANGLED: Dispatcher_MoveToGate::~Dispatcher_MoveToGate()
Dispatcher_MoveToGate::~Dispatcher_MoveToGate() {}

// IDA_EA: 0x8203350
// IDA_NAME: _ZN21Dispatcher_MoveToGate12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_MoveToGate::dispatch_sig(CUser *,PacketBuf &)
// Original command 91 implementation: the private-store manager returns the
// seller owned by this user, and MoveToGate occupies vtable slot +0x0c.
int Dispatcher_MoveToGate::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(
            39437,
            "virtual int Dispatcher_MoveToGate::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    private_store::ISeller* seller = 0;
    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreSeller(&seller, user);
    seller->MoveToGate();
    return 0;
}

// IDA_EA: 0x82336C0
// IDA_NAME: _ZN21Dispatcher_MoveToGate4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MoveToGate::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MoveToGate::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82336CA
// IDA_NAME: _ZN21Dispatcher_MoveToGate7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MoveToGate::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MoveToGate::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
