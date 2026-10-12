#include "old_game/dungeon/instance/timer_elevator_dungeon_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const TIMER_MESSAGE kTimerMessageElevatorDungeon =
    static_cast<TIMER_MESSAGE>(40);
const int kElevatorTimerDelaySeconds = 15;
const int kElevatorTimerMaximumCount = 3;
const unsigned char kElevatorTimerActiveState = 2;
const int kElevatorTimerPacketCommand = 253;

}  // namespace

// IDA_EA: 0x863CC22
// IDA_NAME: _ZN20TimerElevatorDungeonC2Ev
// DEMANGLED: TimerElevatorDungeon::TimerElevatorDungeon(void)
TimerElevatorDungeon::TimerElevatorDungeon() : TimerSig() {}

// IDA_EA: 0x863F2E8 (D2), 0x863F318 (D0)
// IDA_NAME: _ZN20TimerElevatorDungeonD2Ev
// IDA_NAME: _ZN20TimerElevatorDungeonD0Ev
// DEMANGLED: TimerElevatorDungeon::~TimerElevatorDungeon()
TimerElevatorDungeon::~TimerElevatorDungeon() {}

// IDA_EA: 0x86381F2
// IDA_NAME: _ZN20TimerElevatorDungeon12dispatch_sigEiij
// DEMANGLED: TimerElevatorDungeon::dispatch_sig(int,int,unsigned int)
int TimerElevatorDungeon::dispatch_sig(int party_index,
                                       int timer_key,
                                       unsigned int) {
    CParty* const party = G_CGameManager()->GetParty(party_index);
    if (!party || !party->check_timer_key(kTimerMessageElevatorDungeon,
                                          timer_key)) {
        return 0;
    }

    party->battle_field().check_elevator_timer();
    return 1;
}

// IDA_EA: 0x830D848
// IDA_NAME: _ZN13CBattle_Field25send_elevator_time_packetEc
// DEMANGLED: CBattle_Field::send_elevator_time_packet(char)
void CBattle_Field::send_elevator_time_packet(char state) {
    PacketGuard packet;
    packet.put_header(0, kElevatorTimerPacketCommand);
    packet.put_byte(elevator_timer_count_);
    packet.put_byte(state);
    packet.finalize(true);
    party_->send_to_party(packet);
}

// IDA_EA: 0x830D902
// IDA_NAME: _ZN13CBattle_Field20check_elevator_timerEv
// DEMANGLED: CBattle_Field::check_elevator_timer(void)
void CBattle_Field::check_elevator_timer() {
    // The original casts the incremented byte to signed char before the
    // comparison.  Preserve that wrap-sensitive predicate explicitly.
    elevator_timer_count_ =
        static_cast<uint8_t>(elevator_timer_count_ + 1u);
    if (static_cast<int8_t>(elevator_timer_count_) <=
            kElevatorTimerMaximumCount &&
        elevator_timer_state_ == kElevatorTimerActiveState) {
        const int timer_key = party_->gen_timer_key(kTimerMessageElevatorDungeon);
        G_TimerQueue()->InsertTimer(1,
                                    party_->GetPartyIndex(),
                                    kTimerMessageElevatorDungeon,
                                    kElevatorTimerDelaySeconds,
                                    static_cast<unsigned int>(timer_key),
                                    0);
    }
    if (elevator_timer_state_ == kElevatorTimerActiveState) {
        send_elevator_time_packet(0);
    }
}
