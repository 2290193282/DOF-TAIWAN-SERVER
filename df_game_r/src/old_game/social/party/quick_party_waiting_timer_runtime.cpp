#include "old_game/social/party/quick_party_waiting_timer_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/quick_party_runtime.h"
#include "old_game/social/party/quick_party_system_manager_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x863C24A
// IDA_NAME: _ZN25TimerPartyWalkOutLackUserC2Ev
// DEMANGLED: TimerPartyWalkOutLackUser::TimerPartyWalkOutLackUser(void)
TimerPartyWalkOutLackUser::TimerPartyWalkOutLackUser() : TimerSig() {}

// IDA_EA: 0x8640D6A
// IDA_NAME: _ZN25TimerPartyWalkOutLackUserD2Ev
// IDA_EA: 0x8640D9A
// IDA_NAME: _ZN25TimerPartyWalkOutLackUserD0Ev
// DEMANGLED: TimerPartyWalkOutLackUser::~TimerPartyWalkOutLackUser()
TimerPartyWalkOutLackUser::~TimerPartyWalkOutLackUser() {}

// IDA_EA: 0x8633E14
// IDA_NAME: _ZN25TimerPartyWalkOutLackUser12dispatch_sigEiij
// DEMANGLED: TimerPartyWalkOutLackUser::dispatch_sig(int,int,unsigned int)
int TimerPartyWalkOutLackUser::dispatch_sig(int party_index,
                                            int timer_key,
                                            unsigned int) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(TIMER_MESSAGE_PARTY_WALK_OUT_LACK_USER,
                                timer_key)) {
        return 0;
    }
    return party->OnTimerWalkOutLackUser() == 1;
}

// IDA_EA: 0x863D04E
// IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatchingC2Ev
// DEMANGLED: Timer_QuickPartyWaitingPartyMatching::Timer_QuickPartyWaitingPartyMatching(void)
Timer_QuickPartyWaitingPartyMatching::Timer_QuickPartyWaitingPartyMatching()
    : TimerSig() {}

// IDA_EA: 0x863E6FA
// IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatchingD2Ev
// IDA_EA: 0x863E72A
// IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatchingD0Ev
// DEMANGLED: Timer_QuickPartyWaitingPartyMatching::~Timer_QuickPartyWaitingPartyMatching()
Timer_QuickPartyWaitingPartyMatching::~Timer_QuickPartyWaitingPartyMatching() {}

// IDA_EA: 0x863AC18
// IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatching12dispatch_sigEiij
// DEMANGLED: Timer_QuickPartyWaitingPartyMatching::dispatch_sig(int,int,unsigned int)
int Timer_QuickPartyWaitingPartyMatching::dispatch_sig(
    int quick_party_index,
    int timer_key,
    unsigned int timer_message) {
    QuickParty::CQuickParty* const quick_party =
        G_CGameManager()->GetQuickParty(quick_party_index);
    if (!quick_party) {
        return 0;
    }

    const TIMER_MESSAGE message =
        static_cast<TIMER_MESSAGE>(timer_message);
    if (!quick_party->check_timer_key(message, timer_key)) {
        return 0;
    }

    const int quick_party_count = quick_party->get_quick_party_count();
    if (quick_party_count <= 1 &&
        static_cast<signed char>(quick_party->get_first_matching_cnt()) ==
            quick_party_count) {
        return 0;
    }

    if (quick_party->get_timer_key(message) == 1 &&
        cancel_quick_party(quick_party) != 1) {
        return 0;
    }

    if (static_cast<signed char>(quick_party->get_first_matching_cnt()) >=
            quick_party_count &&
        quick_party->is_change_quick_party_count() != 1 &&
        cancel_quick_party(quick_party) != 1) {
        return 0;
    }

    QuickParty::STQuickPartyPoolMap_Key key;
    key.pool_id = static_cast<uint16_t>(quick_party->get_dungeon_index());
    key.channel_type =
        static_cast<uint8_t>(quick_party->get_dungeon_diff());
    G_CGameManager()->GetQuickPartySystemManager()->start_game(quick_party,
                                                                 key);
    return 1;
}

// IDA_EA: 0x863AD96
// IDA_NAME: _ZN36Timer_QuickPartyWaitingPartyMatching18cancel_quick_partyEPN10QuickParty11CQuickPartyE
// DEMANGLED: Timer_QuickPartyWaitingPartyMatching::cancel_quick_party(QuickParty::CQuickParty *)
int Timer_QuickPartyWaitingPartyMatching::cancel_quick_party(
    QuickParty::CQuickParty* quick_party) {
    CParty* const party = quick_party->get_party_object();
    if (!party) {
        return 0;
    }

    if (!G_CGameManager()->GetQuickPartySystemManager()->cancel_quick_party(
            party, true, 0)) {
        return 1;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 383);
    packet.finalize(true);
    party->send_to_party(packet);
    return 0;
}

// IDA_EA: 0x863D06A
// IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGameC2Ev
// DEMANGLED: Timer_QuickPartyWaitingStartGame::Timer_QuickPartyWaitingStartGame(void)
Timer_QuickPartyWaitingStartGame::Timer_QuickPartyWaitingStartGame()
    : TimerSig() {}

// IDA_EA: 0x863E6AC
// IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGameD2Ev
// IDA_EA: 0x863E6DC
// IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGameD0Ev
// DEMANGLED: Timer_QuickPartyWaitingStartGame::~Timer_QuickPartyWaitingStartGame()
Timer_QuickPartyWaitingStartGame::~Timer_QuickPartyWaitingStartGame() {}

// IDA_EA: 0x863AE82
// IDA_NAME: _ZN32Timer_QuickPartyWaitingStartGame12dispatch_sigEiij
// DEMANGLED: Timer_QuickPartyWaitingStartGame::dispatch_sig(int,int,unsigned int)
int Timer_QuickPartyWaitingStartGame::dispatch_sig(int party_index,
                                                   int timer_key,
                                                   unsigned int timer_message) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party ||
        !party->check_timer_key(static_cast<TIMER_MESSAGE>(timer_message),
                                timer_key)) {
        return 0;
    }

    party->send_quick_party_matching_result(false, 0);
    party->set_quick_party(false);

    cMyTrace trace(
        "virtual bool Timer_QuickPartyWaitingStartGame::dispatch_sig(int, int, memberIdentificationNumber_t)",
        5800,
        0);
    trace("QUICK_PARTY_LOG : PARTY DESTROY AND LEAVE DUNGEON !! : MANAGER (%s)",
          party->getManager()->getCurCharacName());

    for (int seat = 0; seat <= 3; ++seat) {
        if (party->checkValidUser(seat) != 1) {
            continue;
        }

        CUser* const user = party->get_user(seat);
        if (party->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2)) !=
            1) {
            G_GameWorld()->out_from_dungeon(user);

            PacketGuard packet;
            packet.clear();
            packet.put_header(0, 2);
            packet.put_byte(0);
            packet.put_short(1);
            user->make_basic_info(packet, 0);
            packet.finalize(true);
            G_GameWorld()->send_all(packet, user);
        }
    }
    return 1;
}
