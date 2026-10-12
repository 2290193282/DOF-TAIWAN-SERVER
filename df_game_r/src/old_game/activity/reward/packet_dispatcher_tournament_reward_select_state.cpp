#include "old_game/activity/reward/packet_dispatcher_tournament_reward_select_state.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kProcessSignature[] =
    "virtual int Dispatcher_TournamentRewardSelectState::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x81DFC98
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TournamentRewardSelectState::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TournamentRewardSelectState::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x81DFCA2
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TournamentRewardSelectState::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TournamentRewardSelectState::process(CUser* user,
                                                     MSG_BASE& message,
                                                     ParamBase& param) {
    stTournamentRewardSelectState& result =
        static_cast<stTournamentRewardSelectState&>(param);
    result.error_code = check_error(user, message);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(21167, kProcessSignature, result.error_code, 0);
    }

    const int party_index = user->GetPartyIndex();
    if (G_CGameManager()->GetParty(party_index)) {
        return 0;
    }
    return LineFunc(21172, kProcessSignature, 0, user->get_acc_id());
}

// IDA_EA: 0x81DFD74
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_TournamentRewardSelectState::send(CUser *,ParamBase &)
void Dispatcher_TournamentRewardSelectState::send(CUser* user,
                                                  ParamBase& param) {
    const stTournamentRewardSelectState& result =
        static_cast<const stTournamentRewardSelectState&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT_STATE);
    if (result.error_code != 0) {
        return;
    }

    packet.put_byte(1);
    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const member = party->get_user(seat);
        if (!member) {
            packet.put_byte(-1);
        } else if (member->getCurCharacTotalFatigue() ==
                   member->getCurCharacTotalMaxFatigue()) {
            packet.put_byte(0);
        } else {
            packet.put_byte(1);
        }
    }
    // The 0828 client always reads eight state bytes after the success byte.
    // CParty still owns four real seats, so preserve the official empty-seat
    // sentinel for the remaining client-side slots.
    for (int seat = 4; seat < 8; ++seat) {
        packet.put_byte(-1);
    }
    packet.finalize(true);
    party->send_to_party(packet);
    party->SetTournamentDungeonClearState(2);
    party->InsertTournamentDungeonTimer(
        TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT);
}

// IDA_EA: 0x81DFF32
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectState11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_TournamentRewardSelectState::check_error(CUser *,MSG_BASE &)
int Dispatcher_TournamentRewardSelectState::check_error(CUser* user,
                                                         MSG_BASE&) {
    if (!user) {
        return -1;
    }
    if (user->get_state() != 5) {
        return 0x7fffffff;
    }
    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return -1;
    }
    if (party->GetTournamentDungeonClearState() != 1) {
        return 0x7fffffff;
    }
    return party->CheckRecvFromAllMember(user) == 1 ? 0 : 0x7fffffff;
}

// IDA_EA: 0x82443A4
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectStateD2Ev
// IDA_EA: 0x82443D4
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectStateD0Ev
// DEMANGLED: Dispatcher_TournamentRewardSelectState::~Dispatcher_TournamentRewardSelectState()
Dispatcher_TournamentRewardSelectState::~Dispatcher_TournamentRewardSelectState() {}

// IDA_EA: 0x8597B0A
// IDA_NAME: _ZN38Dispatcher_TournamentRewardSelectStateC2Ev
// DEMANGLED: Dispatcher_TournamentRewardSelectState::Dispatcher_TournamentRewardSelectState(void)
Dispatcher_TournamentRewardSelectState::Dispatcher_TournamentRewardSelectState()
    : IPacketDispatcher<MSG_BASE,
                        stTournamentRewardSelectState,
                        static_cast<ch_state>(0)>() {}
