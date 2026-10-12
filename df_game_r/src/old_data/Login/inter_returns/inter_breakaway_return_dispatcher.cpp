#include "old_data/Login/inter_returns/inter_breakaway_return_dispatcher.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D92B4
// IDA_NAME: _ZN33Inter_LoadBreakAwayPreventMessage12dispatch_sigEP5CUserPci
int Inter_LoadBreakAwayPreventMessage::dispatch_sig(CUser* user,
                                                    char* payload,
                                                    int) {
    if (!user) {
        return 0;
    }

    const uint32_t quest_time = ReadU32(payload, 0);
    const uint32_t reason = ReadU32(payload, 4);

    PacketGuard reason_packet;
    reason_packet.put_header(0, 273);
    reason_packet.put_byte(static_cast<int>(reason));
    reason_packet.finalize(true);
    user->Send(reason_packet);

    if (reason && reason != 2) {
        return 0;
    }

    PacketGuard reset_packet;
    reset_packet.put_header(0, 310);
    reset_packet.put_int(86400);
    reset_packet.put_byte(0);
    reset_packet.finalize(true);
    user->Send(reset_packet);
    user->setBreakAwayDungeonClearCnt(3);
    user->setBreakAwayQuestTime(quest_time);
    return 0;
}

// IDA_EA: 0x84EDF34 / 0x84F8238 / 0x84F8268
// IDA_NAME: _ZN33Inter_LoadBreakAwayPreventMessageC2Ev / D2Ev / D0Ev
Inter_LoadBreakAwayPreventMessage::Inter_LoadBreakAwayPreventMessage()
    : ISigDispatcher() {}
Inter_LoadBreakAwayPreventMessage::~Inter_LoadBreakAwayPreventMessage() {}
