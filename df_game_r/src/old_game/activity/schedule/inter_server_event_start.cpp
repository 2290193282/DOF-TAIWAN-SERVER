#include "old_game/activity/schedule/inter_server_event_start.h"

#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x84CF2DE
// IDA_NAME: _ZN29Inter_MonitorServerEventStart19SendEventNotiPacketEbssP5CUser
// DEMANGLED: Inter_MonitorServerEventStart::SendEventNotiPacket(bool,short,short,CUser *)
void Inter_MonitorServerEventStart::SendEventNotiPacket(bool event_started,
                                                        int16_t event_id,
                                                        int16_t event_interval,
                                                        CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 69);
    packet.put_byte(event_started ? 1 : 0);
    packet.put_short(event_id);
    packet.put_short(event_interval);
    packet.finalize(true);
    if (user) {
        user->Send(packet);
    } else {
        G_GameWorld()->send_all(packet);
    }
}

// IDA_EA: 0x84CF3CA
// IDA_NAME: _ZN29Inter_MonitorServerEventStart12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorServerEventStart::dispatch_sig(CUser *,char *,int)
int Inter_MonitorServerEventStart::dispatch_sig(CUser* /*user*/,
                                                char* payload,
                                                int /*payload_size*/) {
    const Packet_Monitor_ServerEventStart& request =
        *reinterpret_cast<const Packet_Monitor_ServerEventStart*>(payload);

    // The original constructs this guard before selecting the event branch.
    // Its lifetime is intentionally retained even though notification framing
    // is performed by SendEventNotiPacket.
    PacketGuard branch_guard;

    if (request.event_id == 1u) {
        SendEventNotiPacket(true,
                            static_cast<int16_t>(request.event_id),
                            static_cast<int16_t>(request.event_interval_seconds),
                            0);
        CServerEvent::AddExpRate(
            static_cast<int>(request.exp_rate_delta));
        CServerEvent::TurnOnEvent(
            static_cast<int>(request.event_id),
            static_cast<int>(request.event_interval_seconds));
        G_TimerQueue()->InsertTimer(
            2,
            0,
            83,
            static_cast<int>(request.event_interval_seconds),
            request.exp_rate_delta,
            static_cast<int>(request.event_id));
        cMyTrace trace(
            "virtual int Inter_MonitorServerEventStart::dispatch_sig(CUser*, char*, int)",
            11336,
            0);
        trace("X-Mas Event Start");
    } else {
        cMyTrace trace(
            "virtual int Inter_MonitorServerEventStart::dispatch_sig(CUser*, char*, int)",
            11339,
            5);
        trace("Unknown ServerEvent (%d)", request.event_id);
    }
    return 0;
}

// IDA_EA: 0x84ED4EC / 0x84F9E8E / 0x84F9EBE
Inter_MonitorServerEventStart::Inter_MonitorServerEventStart()
    : ISigDispatcher() {}
Inter_MonitorServerEventStart::~Inter_MonitorServerEventStart() {}
