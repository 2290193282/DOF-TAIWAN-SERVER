#include "old_game/dungeon/instance/packet_dispatcher_give_up_game.h"

#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8595AA4
// IDA_NAME: _ZN21DisPatcher_GiveUpGameC2Ev
// DEMANGLED: DisPatcher_GiveUpGame::DisPatcher_GiveUpGame(void)
DisPatcher_GiveUpGame::DisPatcher_GiveUpGame()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82463A2
// IDA_NAME: _ZN21DisPatcher_GiveUpGameD2Ev
// DEMANGLED: DisPatcher_GiveUpGame::~DisPatcher_GiveUpGame()
// IDA_EA: 0x82463D2
// IDA_NAME: _ZN21DisPatcher_GiveUpGameD0Ev
// DEMANGLED: DisPatcher_GiveUpGame::~DisPatcher_GiveUpGame()
DisPatcher_GiveUpGame::~DisPatcher_GiveUpGame() {}

// IDA_EA: 0x81C40AA
// IDA_NAME: _ZN21DisPatcher_GiveUpGame4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_GiveUpGame::read(PacketBuf &,MSG_BASE &)
int DisPatcher_GiveUpGame::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81C41E8
// IDA_NAME: _ZN21DisPatcher_GiveUpGame11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_GiveUpGame::check_error(CUser *,MSG_BASE &)
int DisPatcher_GiveUpGame::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    const int state = user->get_state();
    return state != 5 && state != 10 && state != 12 && state != 13 ? 19 : 0;
}

// IDA_EA: 0x81C40B4
// IDA_NAME: _ZN21DisPatcher_GiveUpGame7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_GiveUpGame::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_GiveUpGame::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)param;
    const int error = check_error(user, message);
    if (error > 0) {
        return -1;
    }
    if (error < 0) {
        return LineFunc(4087,
                        "virtual int DisPatcher_GiveUpGame::process(CUser*, MSG_BASE&, ParamBase&)",
                        0,
                        user->get_acc_id());
    }

    switch (user->get_state()) {
    case 10: {
        WongWork::CDeathTower* const tower = user->getDeathTower();
        if (tower) {
            tower->handleLeaveUser(user);
        }
        break;
    }
    case 12: {
        WongWork::CBossTower* const tower = user->getBossTower();
        if (tower) {
            tower->handleLeaveUser(user);
        }
        break;
    }
    case 13:
        advancealtar::Manager::giveUpGame(user);
        break;
    default: {
        CParty* const party = user->GetParty();
        if (party) {
            party->giveup_game(user, 1, 0, 0);
        }
        break;
    }
    }
    return error;
}
