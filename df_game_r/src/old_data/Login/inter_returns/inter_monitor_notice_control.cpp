#include "old_data/Login/inter_returns/inter_monitor_notice_control.h"

#include <cstring>

#include "common/manager/global_lifecycle.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x84D035E
// IDA_NAME: _ZN26Inter_MonitorNoticeMessage12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorNoticeMessage::dispatch_sig(CUser *,char *,int)
int Inter_MonitorNoticeMessage::dispatch_sig(CUser* /*user*/,
                                             char* payload,
                                             int /*payload_size*/) {
    const Packet_Monitor_NoticeMessage& notice =
        *reinterpret_cast<const Packet_Monitor_NoticeMessage*>(payload);
    if (notice.message_length == 0u) {
        return 11906;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GM_COMMAND);
    packet.put_byte(1);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(notice.message_length));
    packet.put_str(notice.message, static_cast<int>(notice.message_length));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84ED6AC
// IDA_NAME: _ZN26Inter_MonitorNoticeMessageC2Ev
// DEMANGLED: Inter_MonitorNoticeMessage::Inter_MonitorNoticeMessage(void)
Inter_MonitorNoticeMessage::Inter_MonitorNoticeMessage() : ISigDispatcher() {}

// IDA_EA: 0x84F9B34 / 0x84F9B64
// IDA_NAME: _ZN26Inter_MonitorNoticeMessageD2Ev / D0Ev
// DEMANGLED: Inter_MonitorNoticeMessage::~Inter_MonitorNoticeMessage()
Inter_MonitorNoticeMessage::~Inter_MonitorNoticeMessage() {}

// IDA_EA: 0x84D0466
// IDA_NAME: _ZN23Inter_MonitorForbidChat12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorForbidChat::dispatch_sig(CUser *,char *,int)
int Inter_MonitorForbidChat::dispatch_sig(CUser* /*user*/,
                                          char* payload,
                                          int /*payload_size*/) {
    const Packet_Monitor_ForbidChat& request =
        *reinterpret_cast<const Packet_Monitor_ForbidChat*>(payload);
    char charac_name[30] = {};
    std::strncpy(charac_name, request.charac_name, request.charac_name_length);

    GameWorld* const world = G_GameWorld();
    if (world->IsForbiddenChat(charac_name) != 1) {
        world->InsertUserToIDMap(charac_name, 0);
        world->DisableChat(charac_name, request.forbid_seconds);
    }
    return 0;
}

// IDA_EA: 0x84ED700
// IDA_NAME: _ZN23Inter_MonitorForbidChatC2Ev
// DEMANGLED: Inter_MonitorForbidChat::Inter_MonitorForbidChat(void)
Inter_MonitorForbidChat::Inter_MonitorForbidChat() : ISigDispatcher() {}

// IDA_EA: 0x84F9AE6 / 0x84F9B16
// IDA_NAME: _ZN23Inter_MonitorForbidChatD2Ev / D0Ev
// DEMANGLED: Inter_MonitorForbidChat::~Inter_MonitorForbidChat()
Inter_MonitorForbidChat::~Inter_MonitorForbidChat() {}
