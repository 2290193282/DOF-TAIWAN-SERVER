#include "old_game/pvp/battlefield/timer_tournament_dungeon_clear_state_select_runtime.h"
#include "infra/network/packet/PacketEnum.h"

#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x863D0DA
// IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectC2Ev
// DEMANGLED: Timer_TournametDungeonClearStateSelect::Timer_TournametDungeonClearStateSelect(void)
Timer_TournametDungeonClearStateSelect::
    Timer_TournametDungeonClearStateSelect()
    : TimerSig() {}

// IDA_EA: 0x863E574
// IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectD2Ev
// DEMANGLED: Timer_TournametDungeonClearStateSelect::~Timer_TournametDungeonClearStateSelect()
// IDA_EA: 0x863E5A4
// IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelectD0Ev
// DEMANGLED: Timer_TournametDungeonClearStateSelect::~Timer_TournametDungeonClearStateSelect()
Timer_TournametDungeonClearStateSelect::
    ~Timer_TournametDungeonClearStateSelect() {}

// IDA_EA: 0x863B4EA
// IDA_NAME: _ZN38Timer_TournametDungeonClearStateSelect12dispatch_sigEiij
// DEMANGLED: Timer_TournametDungeonClearStateSelect::dispatch_sig(int,int,unsigned int)
int Timer_TournametDungeonClearStateSelect::dispatch_sig(
    int party_index,
    int timer_key,
    unsigned int) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(
            TIMER_MESSAGE_TOURNAMENT_DUNGEON_CLEAR_STATE_SELECT,
            timer_key)) {
        return 0;
    }

    if (party->GetTournamentDungeonClearState() != 2) {
        return 1;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_TOURNAMENT_REWARD_SELECT);
    packet.put_byte(1);

    CTournamentDungeonReward& reward =
        party->battle_field().tournament_reward();
    std::vector<int> valid_members;
    party->GetValidPartyMember(valid_members);
    const bool tournament_victory = party->isTournamentVictory() != 0;
    if (!reward.isCardSelectComplete(
            ENUM_TOURNAMENT_REWARD_CARD_TYPE_ALL,
            valid_members)) {
        reward.ForceSelectCard(valid_members, tournament_victory);
    }

    reward.MakeSelectPacket(&packet);
    packet.finalize(true);
    party->send_to_party(packet);
    party->SetTournamentDungeonClearState(3);
    party->TournamentDungeonClear(tournament_victory);
    return 1;
}
