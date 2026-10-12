#include "old_game/social/party/timer_dungeon_clear_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

CParty* FindTimerParty(int party_index) {
    return G_CGameManager()->GetParty(party_index);
}

}  // namespace

// IDA_EA: 0x863C7FA
// IDA_NAME: _ZN19TimerBossDieTimeoutC2Ev
TimerBossDieTimeout::TimerBossDieTimeout()
    : TimerSig() {}

// IDA_EA: 0x863FE2E / 0x863FE5E
// IDA_NAME: _ZN19TimerBossDieTimeoutD2Ev / _ZN19TimerBossDieTimeoutD0Ev
TimerBossDieTimeout::~TimerBossDieTimeout() {}

// IDA_EA: 0x863647E
// IDA_NAME: _ZN19TimerBossDieTimeout12dispatch_sigEiij
// DEMANGLED: TimerBossDieTimeout::dispatch_sig(int,int,unsigned int)
int TimerBossDieTimeout::dispatch_sig(int party_index,
                                      int timer_key,
                                      unsigned int charac_no) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    return party ? party->OnTimeoutBossDie(timer_key, charac_no) : 0;
}

// IDA_EA: 0x8633108
// IDA_NAME: _ZN17TimerDungeonClear12dispatch_sigEiij
int TimerDungeonClear::dispatch_sig(int party_index,
                                    int timer_key,
                                    unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(TIMER_MESSAGE_DUNGEON_CLEAR,
                                          static_cast<int>(timer_key))) {
        return 0;
    }
    party->dungeon_clear();
    return 1;
}

// IDA_EA: 0x863C22E
// IDA_NAME: _ZN16TimerDungeonFailC2Ev
// DEMANGLED: TimerDungeonFail::TimerDungeonFail(void)
TimerDungeonFail::TimerDungeonFail()
    : TimerSig() {}

// IDA_EA: 0x8640EF0 (D2), 0x8640F20 (D0)
// IDA_NAME: _ZN16TimerDungeonFailD2Ev / D0Ev
// DEMANGLED: TimerDungeonFail::~TimerDungeonFail()
TimerDungeonFail::~TimerDungeonFail() {}

// IDA_EA: 0x863316C
// IDA_NAME: _ZN16TimerDungeonFail12dispatch_sigEiij
// DEMANGLED: TimerDungeonFail::dispatch_sig(int,int,unsigned int)
int TimerDungeonFail::dispatch_sig(int party_index,
                                   int timer_key,
                                   unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(TIMER_MESSAGE_DUNGEON_FAIL,
                                          static_cast<int>(timer_key))) {
        return 0;
    }
    party->party_dungeon_fail();
    return 1;
}

// IDA_EA: 0x863C29E
// IDA_NAME: _ZN15TimerCardSelectC2Ev
TimerCardSelect::TimerCardSelect()
    : TimerSig() {}

// IDA_EA: 0x8640B48 (D2), 0x8640B78 (D0)
// IDA_NAME: _ZN15TimerCardSelectD2Ev / D0Ev
TimerCardSelect::~TimerCardSelect() {}

// IDA_EA: 0x86343BA
// IDA_NAME: _ZN15TimerCardSelect12dispatch_sigEiij
// DEMANGLED: TimerCardSelect::dispatch_sig(int,int,unsigned int)
int TimerCardSelect::dispatch_sig(int party_index,
                                  int timer_key,
                                  unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int TimerCardSelect::dispatch_sig(int,int,unsigned int)",
        0,
        "[CARD_FLOW][TIMER16_ENTER] party=%d key=%d found=%d",
        party_index,
        timer_key,
        party ? 1 : 0);
    if (!party) {
        return 0;
    }

    const bool key_matches =
        party->check_timer_key(TIMER_MESSAGE_CARD_SELECT, timer_key);
    if (!key_matches) {
        LogManager::logFormat(
            1,
            "card_flow_diag.cpp",
            "int TimerCardSelect::dispatch_sig(int,int,unsigned int)",
            0,
            "[CARD_FLOW][TIMER16_KEY_REJECT] party=%d key=%d state=%d",
            party_index,
            timer_key,
            party->get_dungeon_clear_state());
        return 0;
    }

    const unsigned char clear_state = party->get_dungeon_clear_state();
    if (clear_state == 0u || clear_state == 5u) {
        LogManager::logFormat(
            1,
            "card_flow_diag.cpp",
            "int TimerCardSelect::dispatch_sig(int,int,unsigned int)",
            0,
            "[CARD_FLOW][TIMER16_STATE_SKIP] party=%d key=%d state=%u",
            party_index,
            timer_key,
            static_cast<unsigned int>(clear_state));
        return 1;
    }
    if (champion_break::HandleCardSelectTimeout(party)) {
        return 1;
    }
    const int normal_complete = party->IsCompleteCardSelectOfMembers(
        static_cast<eClearRewardCardType_t>(0));
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int TimerCardSelect::dispatch_sig(int,int,unsigned int)",
        0,
        "[CARD_FLOW][TIMER16_STATE] party=%d key=%d state=%u normal_complete=%d",
        party_index,
        timer_key,
        static_cast<unsigned int>(clear_state),
        normal_complete);
    if (static_cast<unsigned char>(normal_complete) != 1u) {
        party->CompleteCardSelect();
        party->SendEachCardInfo();
    }
    party->set_dungeon_clear_state(5);
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int TimerCardSelect::dispatch_sig(int,int,unsigned int)",
        0,
        "[CARD_FLOW][TIMER16_DUNGEON_CLEAR] party=%d key=%d",
        party_index,
        timer_key);
    party->dungeon_clear();
    return 1;
}

// IDA_EA: 0x863449C
// IDA_NAME: _ZN23TimerClearDungeonReward12dispatch_sigEiij
int TimerClearDungeonReward::dispatch_sig(int party_index,
                                          int timer_key,
                                          unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(TIMER_MESSAGE_CLEAR_DUNGEON_REWARD,
                                          static_cast<int>(timer_key))) {
        return 0;
    }
    if (party->get_dungeon_clear_state() == 2) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_SCORE_SCROLL_STATE);
        packet.put_byte(1);
        packet.finalize(true);
        party->send_to_party(packet);
        party->set_dungeon_clear_state(3);
        party->insert_time_dungeon_score_scroll_msg(30);
    }
    return 1;
}

// IDA_EA: 0x86345D2
// IDA_NAME: _ZN27TimerClearDungeonScoreScoll12dispatch_sigEiij
int TimerClearDungeonScoreScoll::dispatch_sig(int party_index,
                                              int timer_key,
                                              unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(TIMER_MESSAGE_CLEAR_DUNGEON_SCORE_SCROLL,
                                          static_cast<int>(timer_key))) {
        return 0;
    }
    if (party->get_dungeon_clear_state() == 3) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_CARD_SELECT_RIGHT_STATE);
        packet.put_byte(1);
        champion_break::WriteCardSelectRights(party, packet);
        packet.finalize(true);
        party->send_to_party(packet);
        party->set_dungeon_clear_state(4);
        party->insert_time_dungeon_score_scroll_msg(30);
        party->insert_time_card_select_msg(4);
    }
    return 1;
}

// IDA_EA: 0x863C2F2
// IDA_NAME: _ZN32TimerClearDungeonCardSelectRightC2Ev
// DEMANGLED: TimerClearDungeonCardSelectRight::TimerClearDungeonCardSelectRight(void)
TimerClearDungeonCardSelectRight::TimerClearDungeonCardSelectRight()
    : TimerSig() {}

// IDA_EA: 0x8640A5E (D2), 0x8640A8E (D0)
// IDA_NAME: _ZN32TimerClearDungeonCardSelectRightD2Ev / D0Ev
// DEMANGLED: TimerClearDungeonCardSelectRight::~TimerClearDungeonCardSelectRight()
TimerClearDungeonCardSelectRight::~TimerClearDungeonCardSelectRight() {}

// IDA_EA: 0x86347C6
// IDA_NAME: _ZN32TimerClearDungeonCardSelectRight12dispatch_sigEiij
// DEMANGLED: TimerClearDungeonCardSelectRight::dispatch_sig(int,int,unsigned int)
int TimerClearDungeonCardSelectRight::dispatch_sig(int party_index,
                                                   int timer_key,
                                                   unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(
                      TIMER_MESSAGE_CLEAR_DUNGEON_CARD_SELECT_RIGHT,
                      static_cast<int>(timer_key))) {
        return 0;
    }
    if (party->get_dungeon_clear_state() == 4) {
        party->set_dungeon_clear_state(5);
        party->insert_time_dungeon_clear_msg(5);
    }
    return 1;
}

// IDA_EA: 0x8636398
// IDA_NAME: _ZN18TimerSetPlayResult12dispatch_sigEiij
int TimerSetPlayResult::dispatch_sig(int party_index,
                                     int timer_key,
                                     unsigned int) {
    CParty* const party = FindTimerParty(party_index);
    if (!party || !party->check_timer_key(TIMER_MESSAGE_SET_PLAY_RESULT,
                                          static_cast<int>(timer_key))) {
        return 0;
    }
    party->SetPlayResult(party->getManager());
    return 1;
}
