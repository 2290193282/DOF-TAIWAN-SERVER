#include "old_game/pvp/arena/pvp_room_timer_callbacks_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {

PvP_Room* GetTimerPvPRoom(int pvp_index) {
    return G_CGameManager()->GetPvp(pvp_index, 0, 0);
}

}  // namespace

// IDA_EA: 0x8636555
// IDA_NAME: _Z16PvPDieUserReviveiiji
// DEMANGLED: PvPDieUserRevive(int,int,unsigned int,int)
int PvPDieUserRevive(int pvp_index,
                     int timer_key,
                     unsigned int,
                     int seat) {
    PvP_Room* const room = GetTimerPvPRoom(pvp_index);
    if (!room ||
        !room->CheckTimerKey(static_cast<TIMER_MESSAGE>(seat + 51),
                             timer_key)) {
        return 0;
    }

    room->HandleTimerUserRevive(seat);
    return 1;
}

// IDA_EA: 0x8633BE8
// IDA_NAME: _ZN15TimerPvpRankRes12dispatch_sigEiij
// DEMANGLED: TimerPvpRankRes::dispatch_sig(int,int,unsigned int)
int TimerPvpRankRes::dispatch_sig(int pvp_index, int timer_key,
                                  unsigned int) {
    PvP_Room* const room = GetTimerPvPRoom(pvp_index);
    if (!room || room->get_timer_key(static_cast<TIMER_MESSAGE>(50)) !=
                     timer_key) {
        return 0;
    }

    room->pvp_reward();
    return 1;
}

// IDA_EA: 0x8633E86
// IDA_NAME: _ZN23TimerPVPWalkOutLackUser12dispatch_sigEiij
// DEMANGLED: TimerPVPWalkOutLackUser::dispatch_sig(int,int,unsigned int)
int TimerPVPWalkOutLackUser::dispatch_sig(int pvp_index, int timer_key,
                                           unsigned int) {
    PvP_Room* const room = GetTimerPvPRoom(pvp_index);
    if (!room || room->get_timer_key(
                     TIMER_MESSAGE_PVP_WALK_OUT_LACK_USER) != timer_key) {
        return 0;
    }

    if (room->OnTimerWalkOutLackUser() != 1) {
        room->reset();
        G_CGameManager()->PutPvp(room);
    }
    return 1;
}

// IDA_EA: 0x86365DA
// IDA_NAME: _ZN22TimerPvPDieUserRevive012dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive0::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive0::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 0);
}

// IDA_EA: 0x8636604
// IDA_NAME: _ZN22TimerPvPDieUserRevive112dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive1::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive1::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 1);
}

// IDA_EA: 0x863662E
// IDA_NAME: _ZN22TimerPvPDieUserRevive212dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive2::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive2::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 2);
}

// IDA_EA: 0x8636658
// IDA_NAME: _ZN22TimerPvPDieUserRevive312dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive3::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive3::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 3);
}

// IDA_EA: 0x8636682
// IDA_NAME: _ZN22TimerPvPDieUserRevive412dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive4::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive4::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 4);
}

// IDA_EA: 0x86366AC
// IDA_NAME: _ZN22TimerPvPDieUserRevive512dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive5::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive5::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 5);
}

// IDA_EA: 0x86366D6
// IDA_NAME: _ZN22TimerPvPDieUserRevive612dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive6::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive6::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 6);
}

// IDA_EA: 0x8636700
// IDA_NAME: _ZN22TimerPvPDieUserRevive712dispatch_sigEiij
// DEMANGLED: TimerPvPDieUserRevive7::dispatch_sig(int,int,unsigned int)
int TimerPvPDieUserRevive7::dispatch_sig(int pvp_index, int timer_key,
                                          unsigned int payload) {
    return PvPDieUserRevive(pvp_index, timer_key, payload, 7);
}

// IDA_EA: 0x8636F90
// IDA_NAME: _ZN21TimerCheckPvPRoomUser12dispatch_sigEiij
// DEMANGLED: TimerCheckPvPRoomUser::dispatch_sig(int,int,unsigned int)
int TimerCheckPvPRoomUser::dispatch_sig(int pvp_index, int timer_key,
                                        unsigned int) {
    PvP_Room* const room = GetTimerPvPRoom(pvp_index);
    if (!room || room->get_timer_key(static_cast<TIMER_MESSAGE>(60)) !=
                     timer_key ||
        room->get_state() != 2) {
        return 0;
    }

    room->check_recv_pvp_rank();
    if (room->IsEndPvpBattle() != 1) {
        room->send_pvp_end();
    }
    return 1;
}

// IDA_EA: 0x863703A
// IDA_NAME: _ZN26TimerCheckRecvPvpEndResult12dispatch_sigEiij
// DEMANGLED: TimerCheckRecvPvpEndResult::dispatch_sig(int,int,unsigned int)
int TimerCheckRecvPvpEndResult::dispatch_sig(int pvp_index, int timer_key,
                                             unsigned int) {
    PvP_Room* const room = GetTimerPvPRoom(pvp_index);
    if (!room || room->get_timer_key(static_cast<TIMER_MESSAGE>(61)) !=
                     timer_key) {
        return 0;
    }

    if (room->get_recv_pvp_end_result_count()) {
        room->end_pvp_result();
        PacketGuard packet;
        room->make_state_info(reinterpret_cast<char*>(&packet));
        G_GameWorld()->send_all(packet);
        room->end_pvp();
        room->battle_reset();
    }
    return 1;
}
