#include "old_game/social/friend/buddy_monitor_dispatchers_a.h"

#include <cstddef>
#include <cstring>
#include <stdint.h>

#include "old_data/PVF/common/rdar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/os_api.h"

namespace {

#pragma pack(push, 1)

// The monitor transport header occupies bytes +0x00..+0x09.  The original
// function reads the following fields directly, including the fixed 30-byte
// character-name storage used by both client packet and return-user mail.
struct BuddyInOutPayload {
    uint8_t transport_prefix[0x0a];
    uint32_t buddy_charac_no;  // +0x0a, forwarded as client dword
    uint8_t reserved_0e_11[4];
    uint8_t buddy_state;       // +0x12, first client byte
    uint8_t relation_type;     // +0x13, final client byte
    uint8_t is_buddy_in;       // +0x14, selects NOTIPACKET 54/55
    char buddy_name[0x1e];     // +0x15
    uint8_t send_return_mail;  // +0x33
};

// The monitor reply starts with the same ten-byte transport header and then
// contains the target account id, a count byte and contiguous 42-byte raw
// BuddyList records.  GameWorld is the sole consumer of the record body.
struct ReplyBuddyListPayload {
    uint8_t transport_prefix[0x0a];
    uint32_t account_id;       // +0x0a
    uint8_t count;             // +0x0e
    BuddyList entries[1];      // +0x0f
};

#pragma pack(pop)

static_assert(offsetof(BuddyInOutPayload, buddy_charac_no) == 0x0au &&
                  offsetof(BuddyInOutPayload, buddy_state) == 0x12u &&
                  offsetof(BuddyInOutPayload, relation_type) == 0x13u &&
                  offsetof(BuddyInOutPayload, is_buddy_in) == 0x14u &&
                  offsetof(BuddyInOutPayload, buddy_name) == 0x15u &&
                  offsetof(BuddyInOutPayload, send_return_mail) == 0x33u,
              "Inter_MonitorBuddyInOut payload must retain IDA offsets");
static_assert(offsetof(ReplyBuddyListPayload, account_id) == 0x0au &&
                  offsetof(ReplyBuddyListPayload, count) == 0x0eu &&
                  offsetof(ReplyBuddyListPayload, entries) == 0x0fu,
              "Inter_MonitorReplyBuddyList payload must retain IDA offsets");

}  // namespace

// IDA_EA: 0x84C8EC0
// IDA_NAME: _ZN23Inter_MonitorBuddyInOut12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorBuddyInOut::dispatch_sig(CUser *,char *,int)
int Inter_MonitorBuddyInOut::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state()) {
        return 0;
    }

    const BuddyInOutPayload& reply =
        *reinterpret_cast<const BuddyInOutPayload*>(payload);

    PacketGuard packet;
    packet.put_header(0, reply.is_buddy_in == 1 ? 54 : 55);
    packet.put_byte(reply.buddy_state);
    packet.put_int(static_cast<int>(reply.buddy_charac_no));
    const int buddy_name_length = static_cast<int>(std::strlen(reply.buddy_name));
    packet.put_int(buddy_name_length);
    packet.put_str(reply.buddy_name, buddy_name_length);
    packet.put_byte(reply.relation_type);
    packet.finalize(true);
    user->Send(packet);

    // The old body queues a system mail only for return-user replies and only
    // when a current character is available.  It builds a real empty
    // Inven_Item and routes it through the ordinary mailbox DB transaction.
    if (reply.send_return_mail == 1 && user->getCurCharacR()) {
        Inven_Item item;
        char text[256];
        const char* const format =
            g_scriptStringManager_.findString(4, "return_user_msg_3", 0);
        OS_API::snprintf(text, 0xffu, format, user->getCurCharacName(),
                         reply.buddy_name);

        const int server_group = user->GetServerGroup();
        const int text_length = static_cast<int>(std::strlen(text));
        const int charac_no = user->getCurCharacNo();
        const char* const title =
            g_scriptStringManager_.findString(4, "return_user_msg_title", 0);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title, item, 0, static_cast<unsigned int>(charac_no), text,
            text_length, 0, static_cast<ENUM_SERVER_GROUP>(server_group),
            false, false);
    }
    return 0;
}

// IDA_EA: 0x84EA42A
// IDA_NAME: _ZN23Inter_MonitorBuddyInOutC2Ev
Inter_MonitorBuddyInOut::Inter_MonitorBuddyInOut() : ISigDispatcher() {}

// IDA_EA: 0x84EA446
// IDA_NAME: _ZN23Inter_MonitorBuddyInOutD2Ev
// IDA_EA: 0x84EA476
// IDA_NAME: _ZN23Inter_MonitorBuddyInOutD0Ev
Inter_MonitorBuddyInOut::~Inter_MonitorBuddyInOut() {}

// IDA_EA: 0x84C9148
// IDA_NAME: _ZN27Inter_MonitorReplyBuddyList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_MonitorReplyBuddyList::dispatch_sig(CUser *,char *,int)
int Inter_MonitorReplyBuddyList::dispatch_sig(CUser*, char* payload, int) {
    const ReplyBuddyListPayload& reply =
        *reinterpret_cast<const ReplyBuddyListPayload*>(payload);
    // The legacy member ABI takes a non-const BuddyList pointer even though
    // the sender performs only binary reads.  Preserve that exact signature
    // and pass the monitor-owned record base unchanged.
    G_GameWorld()->send_buddy_conn_list(
        reply.account_id, const_cast<BuddyList*>(reply.entries),
        static_cast<int>(reply.count));
    return 0;
}

// IDA_EA: 0x84EA494
// IDA_NAME: _ZN27Inter_MonitorReplyBuddyListC2Ev
Inter_MonitorReplyBuddyList::Inter_MonitorReplyBuddyList() : ISigDispatcher() {}

// IDA_EA: 0x84EA4B0
// IDA_NAME: _ZN27Inter_MonitorReplyBuddyListD2Ev
// IDA_EA: 0x84EA4E0
// IDA_NAME: _ZN27Inter_MonitorReplyBuddyListD0Ev
Inter_MonitorReplyBuddyList::~Inter_MonitorReplyBuddyList() {}
