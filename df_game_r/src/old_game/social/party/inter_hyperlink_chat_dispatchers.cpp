#include "old_game/social/party/inter_hyperlink_chat_dispatchers.h"

#include <stddef.h>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {

#pragma pack(push, 1)
struct HyperlinkRecord {
    uint8_t serialized[104];
};

// Slots 677/679 share this exact monitor return layout.  The first fourteen
// bytes belong to the InterDispatcher transport envelope.
struct GuildHyperlinkChatReply {
    uint8_t dispatcher_prefix[14];
    uint32_t target_charac_no;
    char sender_name[30];
    uint8_t hyperlink_count;
    HyperlinkRecord hyperlinks[3];
    uint8_t message_length;
    char message[255];
};

struct MegaphoneHyperlinkChatReply {
    uint8_t dispatcher_prefix[10];
    uint8_t target_server_group;
    uint8_t message_subtype;
    uint16_t message_code;
    uint8_t message_type;
    char sender_name[30];
    uint8_t hyperlink_count;
    HyperlinkRecord hyperlinks[3];
    uint8_t message_length;
    char message[255];
};
#pragma pack(pop)

static_assert(sizeof(HyperlinkRecord) == 104u,
              "hyperlink record wire ABI changed");
static_assert(sizeof(GuildHyperlinkChatReply) == 0x269u &&
                  offsetof(GuildHyperlinkChatReply, target_charac_no) == 14u &&
                  offsetof(GuildHyperlinkChatReply, sender_name) == 18u &&
                  offsetof(GuildHyperlinkChatReply, hyperlink_count) == 48u &&
                  offsetof(GuildHyperlinkChatReply, hyperlinks) == 49u &&
                  offsetof(GuildHyperlinkChatReply, message_length) == 361u &&
                  offsetof(GuildHyperlinkChatReply, message) == 362u,
              "guild hyperlink reply offsets changed");
static_assert(sizeof(MegaphoneHyperlinkChatReply) == 0x266u &&
                  offsetof(MegaphoneHyperlinkChatReply, message_type) == 14u &&
                  offsetof(MegaphoneHyperlinkChatReply, sender_name) == 15u &&
                  offsetof(MegaphoneHyperlinkChatReply, hyperlink_count) == 45u &&
                  offsetof(MegaphoneHyperlinkChatReply, hyperlinks) == 46u &&
                  offsetof(MegaphoneHyperlinkChatReply, message_length) == 358u &&
                  offsetof(MegaphoneHyperlinkChatReply, message) == 359u,
              "megaphone hyperlink reply offsets changed");

int BoundedCStringLength(const char* text, int maximum_length) {
    int length = 0;
    while (length < maximum_length && text[length]) {
        ++length;
    }
    return length;
}

void AppendHyperlinkRecords(PacketGuard& packet,
                            const HyperlinkRecord* records,
                            uint8_t count) {
    packet.put_byte(count);
    for (uint8_t index = 0; index < count; ++index) {
        packet.put_binary(
            reinterpret_cast<const char*>(records[index].serialized),
            static_cast<int>(sizeof(records[index].serialized)));
    }
}

void SendGuildHyperlinkChat(CUser* user,
                            const GuildHyperlinkChatReply& reply,
                            int chat_type) {
    const int sender_length = BoundedCStringLength(reply.sender_name,
                                                   sizeof(reply.sender_name));
    PacketGuard packet;
    packet.put_header(0, 371);
    packet.put_byte(chat_type);
    packet.put_byte(0);
    packet.put_int(sender_length);
    packet.put_str(reply.sender_name, sender_length);
    packet.put_byte(0);
    packet.put_int(reply.message_length);
    packet.put_str(reply.message, reply.message_length);
    AppendHyperlinkRecords(packet, reply.hyperlinks, reply.hyperlink_count);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

// IDA_EA: 0x84E503C
// IDA_NAME: _ZN40Inter_MonitorNoticeGuildChatMsgHyperLink12dispatch_sigEP5CUserPci
int Inter_MonitorNoticeGuildChatMsgHyperLink::dispatch_sig(CUser* user,
                                                           char* payload,
                                                           int payload_size) {
    if (!user || user->get_state() == 0 || !payload ||
        payload_size < static_cast<int>(sizeof(GuildHyperlinkChatReply))) {
        return 0;
    }
    const GuildHyperlinkChatReply& reply =
        *reinterpret_cast<const GuildHyperlinkChatReply*>(payload);
    if (reply.hyperlink_count > 3u ||
        user->get_charac_no(-1) != static_cast<int>(reply.target_charac_no)) {
        return 0;
    }
    SendGuildHyperlinkChat(user, reply, 6);
    return 0;
}

// IDA_EA: 0x84EEC1C / 0x84F6018 / 0x84F6048
Inter_MonitorNoticeGuildChatMsgHyperLink::
    Inter_MonitorNoticeGuildChatMsgHyperLink()
    : ISigDispatcher() {}
Inter_MonitorNoticeGuildChatMsgHyperLink::
    ~Inter_MonitorNoticeGuildChatMsgHyperLink() {}

// IDA_EA: 0x84E5282
// IDA_NAME: _ZN41Inter_MonitorNoticeMemberChatMsgHyperLink12dispatch_sigEP5CUserPci
int Inter_MonitorNoticeMemberChatMsgHyperLink::dispatch_sig(CUser* user,
                                                            char* payload,
                                                            int payload_size) {
    if (!user || user->get_state() == 0 || user->get_charac_memberkey() == 0 ||
        !payload || payload_size < static_cast<int>(sizeof(GuildHyperlinkChatReply))) {
        return 0;
    }
    const GuildHyperlinkChatReply& reply =
        *reinterpret_cast<const GuildHyperlinkChatReply*>(payload);
    if (reply.hyperlink_count > 3u ||
        user->get_charac_no(-1) != static_cast<int>(reply.target_charac_no)) {
        return 0;
    }
    SendGuildHyperlinkChat(user, reply, 8);
    return 0;
}

// IDA_EA: 0x84EEC38 / 0x84F5FCA / 0x84F5FFA
Inter_MonitorNoticeMemberChatMsgHyperLink::
    Inter_MonitorNoticeMemberChatMsgHyperLink()
    : ISigDispatcher() {}
Inter_MonitorNoticeMemberChatMsgHyperLink::
    ~Inter_MonitorNoticeMemberChatMsgHyperLink() {}

// IDA_EA: 0x84E5C26
// IDA_NAME: _ZN35Inter_Monitor_MegaphoneMsgHyperLink12dispatch_sigEP5CUserPci
int Inter_Monitor_MegaphoneMsgHyperLink::dispatch_sig(CUser*,
                                                      char* payload,
                                                      int payload_size) {
    if (!payload || payload_size <
                        static_cast<int>(sizeof(MegaphoneHyperlinkChatReply))) {
        return 0;
    }
    const MegaphoneHyperlinkChatReply& reply =
        *reinterpret_cast<const MegaphoneHyperlinkChatReply*>(payload);
    if (reply.hyperlink_count > 3u) {
        return 0;
    }

    const int sender_length = BoundedCStringLength(reply.sender_name,
                                                   sizeof(reply.sender_name));
    PacketGuard packet;
    packet.put_header(0, 372);
    packet.put_byte(reply.message_type);
    packet.put_byte(reply.message_subtype);
    packet.put_short(reply.message_code);
    packet.put_int(sender_length);
    packet.put_str(reply.sender_name, sender_length);
    packet.put_int(reply.message_length);
    packet.put_str(reply.message, reply.message_length);
    AppendHyperlinkRecords(packet, reply.hyperlinks, reply.hyperlink_count);
    packet.finalize(true);

    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        G_GameWorld()->send_server_group(
            packet, static_cast<char>(reply.target_server_group));
    } else {
        G_GameWorld()->send_all(packet);
    }
    return 0;
}

// IDA_EA: 0x84EEC70 / 0x84F5F2E / 0x84F5F5E
Inter_Monitor_MegaphoneMsgHyperLink::Inter_Monitor_MegaphoneMsgHyperLink()
    : ISigDispatcher() {}
Inter_Monitor_MegaphoneMsgHyperLink::~Inter_Monitor_MegaphoneMsgHyperLink() {}
