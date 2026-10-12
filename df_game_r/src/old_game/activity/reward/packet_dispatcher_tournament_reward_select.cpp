#include "old_game/activity/reward/packet_dispatcher_tournament_reward_select.h"
#include "infra/network/packet/PacketEnum.h"

#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_TournamentRewardSelect::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_TournamentRewardSelect::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8597B26
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelectC2Ev
// DEMANGLED: Dispatcher_TournamentRewardSelect::Dispatcher_TournamentRewardSelect(void)
Dispatcher_TournamentRewardSelect::Dispatcher_TournamentRewardSelect()
    : IPacketDispatcher<MsgTournamentRewardSelect,
                        stTournamentRewardSelect,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244356
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelectD2Ev
// IDA_EA: 0x8244386
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelectD0Ev
// DEMANGLED: Dispatcher_TournamentRewardSelect::~Dispatcher_TournamentRewardSelect()
Dispatcher_TournamentRewardSelect::~Dispatcher_TournamentRewardSelect() {}

// IDA_EA: 0x81DFFDA
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TournamentRewardSelect::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TournamentRewardSelect::read(PacketBuf& packet,
                                             MSG_BASE& base_message) {
    MsgTournamentRewardSelect& message =
        static_cast<MsgTournamentRewardSelect&>(base_message);
    if (packet.get_byte(message.card_group) != 1) {
        return LineFunc(21245, kReadSignature, 0, 0);
    }
    if (message.card_group >= 2u) {
        return LineFunc(21248, kReadSignature, 0, 0);
    }
    if (packet.get_byte(message.reward_kind) != 1) {
        return LineFunc(21251, kReadSignature, 0, 0);
    }
    if (message.reward_kind < 2u) {
        return 0;
    }
    return LineFunc(21254, kReadSignature, 0, 0);
}

// IDA_EA: 0x81E00F0
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TournamentRewardSelect::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TournamentRewardSelect::process(CUser* user,
                                                MSG_BASE& base_message,
                                                ParamBase& base_param) {
    MsgTournamentRewardSelect& message =
        static_cast<MsgTournamentRewardSelect&>(base_message);
    stTournamentRewardSelect& result =
        static_cast<stTournamentRewardSelect&>(base_param);
    result.error_code = check_error(user, message);
    if (result.error_code > 0) {
        return 0;
    }
    if (result.error_code < 0) {
        return LineFunc(21269, kProcessSignature, result.error_code, 0);
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return LineFunc(21276, kProcessSignature, 0, 0);
    }

    std::vector<int> valid_members;
    party->GetValidPartyMember(valid_members);
    CTournamentDungeonReward& reward = party->battle_field_.tournament_reward();
    if (reward.isCardSelectComplete(
            ENUM_TOURNAMENT_REWARD_CARD_TYPE_ALL, valid_members)) {
        result.error_code = 0x7fffffff;
        return 0;
    }

    const int member_slot = party->GetMemberSlotNo(user);
    if (!reward.SelectCard(member_slot,
                           static_cast<char>(message.card_group),
                           static_cast<char>(message.reward_kind))) {
        result.error_code = 0x7fffffff;
    }
    return 0;
}

// IDA_EA: 0x81E02B0
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_TournamentRewardSelect::send(CUser *,ParamBase &)
void Dispatcher_TournamentRewardSelect::send(CUser* user, ParamBase& base_param) {
    const stTournamentRewardSelect& result =
        static_cast<const stTournamentRewardSelect&>(base_param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT);

    if (result.error_code != 0) {
        return;
    }

    packet.put_byte(1);
    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return;
    }

    CTournamentDungeonReward& reward = party->battle_field_.tournament_reward();
    reward.MakeSelectPacket(&packet);
    packet.finalize(true);
    party->send_to_party(packet);

    if (party->CheckRecvFromAllMember(user) == 1) {
        std::vector<int> valid_members;
        party->GetValidPartyMember(valid_members);
        if (reward.isCardSelectComplete(
                ENUM_TOURNAMENT_REWARD_CARD_TYPE_ALL, valid_members)) {
            const bool tournament_victory = party->isTournamentVictory() != 0;
            party->SetTournamentDungeonClearState(3);
            party->TournamentDungeonClear(tournament_victory);
        }
    }
}

// IDA_EA: 0x81E046C
// IDA_NAME: _ZN33Dispatcher_TournamentRewardSelect11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_TournamentRewardSelect::check_error(CUser *,MSG_BASE &)
int Dispatcher_TournamentRewardSelect::check_error(CUser* user, MSG_BASE&) {
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
    return party->GetTournamentDungeonClearState() == 2 ? 0 : 0x7fffffff;
}
