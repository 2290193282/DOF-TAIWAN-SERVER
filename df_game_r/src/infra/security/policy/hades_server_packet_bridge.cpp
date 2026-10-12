#include "infra/security/policy/hades_server_packet_bridge.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "infra/security/policy/hades_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/packet/server_dispatch_packets.h"
#include "infra/network/packet/udp_packets.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "old_game/economy/mail/mail_trade_packets.h"

namespace {

// IDA_EA: CDispatchServerPacket::dispatchPacket @ 0x8473660.
// The server-frame parser has already split a complete PacketHeader frame
// before this callback is entered.
const uint16_t kHadesActionOpcode = 0x0FE6u;
const uint16_t kHadesReconnectOpcode = 0x0FE7u;
const int kInterSignalHadesAction = 0x02B6;  // decimal 694.
// IDA 0x847429F..0x847432B: Packet_Monitor_UDP_Login is also the monitor
// server's login-result frame.  CDispatchServerPacket queues the entire
// fixed-size frame, not a stripped payload, for Inter_MonitorLoginOK.
const uint16_t kMonitorLoginOpcode = 0x03E8u;
const int kInterSignalMonitorLoginOK = 65;
// Official 0627 CDispatchServerPacket branch n1807 == 1300: the monitor
// relays the complete 0x12-byte frame, writes the destination game-user UID
// at +0x0e, and queues it as Inter 130.
const uint16_t kMonitorNotifyNewMailOpcode = 0x0514u;
const int kInterSignalMonitorNotifyNewMail = 130;
// Official auction service Packet_Monitor_Notify_Auction_Mail constructor
// writes PacketHeader(3100, 0x26).  The monitor fills the +0x0e routing UID,
// and official CDispatchServerPacket @0x8478B64..0x8478BF8 copies the complete
// frame after Inter signal 239 and that UID.
const uint16_t kMonitorNotifyAuctionMailOpcode = 0x0C1Cu;
const int kInterSignalMonitorNotifyAuctionMail = 239;
// IDA 0x84747D0..0x8474863: the monitor's buddy in/out reply reuses
// opcode 0x03EF.  Its inbound form is a fixed 0x34-byte frame; the router
// reads the target user UID from +0x0E and queues the complete frame as
// Inter_MonitorBuddyInOut (68).
const uint16_t kMonitorBuddyInOutOpcode = 0x03EFu;
const int kInterSignalMonitorBuddyInOut = 68;
// Official 0627 dispatchPacket branch n1807 == 1650 (Hex-Rays lines
// 3454..3464):
// monitor opcode 0x0672 is a fixed 0x3b-byte add-buddy DB result.  Its game
// routing UID is at +0x0e and the complete frame is queued as Inter 188.
const uint16_t kMonitorAddBuddyReplyOpcode = 0x0672u;
const int kInterSignalMonitorAddBuddy = 188;
// Official 0627 dispatchPacket branch n1807 == 1010 routes the monitor's
// connected-buddy snapshot globally as Inter 70.  Packet_Monitor_Reply_Buddy_List
// is a 0x0f-byte prefix plus up to 32 packed 0x2a-byte BuddyList records.
const uint16_t kMonitorBuddyListReplyOpcode = 0x03F2u;
const int kInterSignalMonitorBuddyList = 70;
const size_t kMonitorBuddyListRecordSize = 0x2Au;
const size_t kMonitorBuddyListMaxCount = 32u;
// Official 0627 CDispatchServerPacket member/upper-member reply matrix.
// These are monitor-owned relationship records.  The game server only routes
// the complete frame to the already registered Inter receiver; it must not
// synthesize mentor rows in the character database.
struct MonitorMemberRoute {
    uint16_t opcode;
    uint16_t packet_size;
    size_t routing_uid_offset;
    int inter_signal;
};

const MonitorMemberRoute kMonitorMemberRoutes[] = {
    {0x04B6u,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Notice_Member_Login_Out)),
     offsetof(Packet_Monitor_Notice_Member_Login_Out, target_user_uid), 108},
    {0x04B7u,
     static_cast<uint16_t>(
         sizeof(Packet_Monitor_Request_Member_Enter_To_Requester)),
     offsetof(Packet_Monitor_Request_Member_Enter_To_Requester,
              target_user_uid),
     109},
    {0x04B8u,
     static_cast<uint16_t>(
         sizeof(Packet_Monitor_Request_Member_Enter_To_Responder)),
     offsetof(Packet_Monitor_Request_Member_Enter_To_Responder,
              target_user_uid),
     110},
    {0x04B9u,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Member_Enter_Result)),
     offsetof(Packet_Monitor_Member_Enter_Result, target_user_uid), 112},
    {0x04BAu,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Member_Enter_Result)),
     offsetof(Packet_Monitor_Member_Enter_Result, target_user_uid), 111},
    {0x04BBu,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Member_Secede_Result)),
     offsetof(Packet_Monitor_Member_Secede_Result, target_user_uid), 113},
    {0x04BCu,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Member_Secede_Result)),
     offsetof(Packet_Monitor_Member_Secede_Result, target_user_uid), 114},
    {0x04BDu,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Delete_Member_Id)),
     offsetof(Packet_Monitor_Delete_Member_Id, target_user_uid), 115},
    {0x04BEu,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Call_Member_List)),
     offsetof(Packet_Monitor_Call_Member_List, target_user_uid), 117},
    {0x04C0u,
     static_cast<uint16_t>(sizeof(Packet_Monitor_Pay_Tax_To_Upper)),
     offsetof(Packet_Monitor_Pay_Tax_To_Upper, target_user_uid), 118},
};
// Recovered 0828 guild-server replies that use the same complete-frame Inter
// envelope.  These opcodes are emitted directly by df_guild_r rather than by
// the monitor server, so they are not represented by the older monitor packet
// structs above.  Keep their fixed wire sizes and unaligned routing-UID
// offsets explicit at this boundary.
const MonitorMemberRoute kGuildUserRoutes[] = {
    // Packet_Monitor_Notice_Guild_Member_Info ->
    // Inter_MonitorNoticeGuildMemberInfo.  This ultimately sends client
    // NOTI127, which is the authoritative local guild-member grade update.
    {0x0431u, 0x002Cu, 0x0Au, 102},
    // Packet_Add_Guild_Fund_Result -> Inter_AddGuildFundResult.
    {0x1BBDu, 0x0017u, 0x0Fu, 532},
    // Packet_Guild_Attendance_Info_Reply -> Inter_GuildAttendanceInfo.
    {0x1BC7u, 0x002Fu, 0x0Eu, 541},
    // Packet_Achieve_Guild_Attendance -> Inter_AchieveGuildAttendance.
    {0x1BC8u, 0x001Au, 0x0Eu, 542},
};
// Official 0627 CDispatchServerPacket::dispatchPacket routes these guild
// query replies below through the normal Inter dispatcher.  They all use the
// native i386 routing UID stored immediately after PacketHeader at +0x0a.
// The member-list and multi-guild lookup replies use validated dynamic tails.
const uint16_t kGuildChatReplyOpcode = 0x0400u;
const uint16_t kGuildMembersReplyOpcode = 0x0401u;
const uint16_t kGuildInfoNoticeOpcode = 0x040Au;
const uint16_t kGuildSetKeyOpcode = 0x040Bu;
const uint16_t kGuildInfoReplyOpcode = 0x040Du;
const uint16_t kGuildMembersNextReplyOpcode = 0x0423u;
const uint16_t kGuildAllMembersReplyOpcode = 0x0424u;
const uint16_t kGuildAllMembersNextReplyOpcode = 0x0428u;
const int kInterSignalGuildMembersReply = 91;
const int kInterSignalGuildInfoReply = 95;
const int kInterSignalGuildInfoNotice = 96;
const int kInterSignalGuildSetKey = 97;
const int kInterSignalGuildMembersNextReply = 198;
const int kInterSignalGuildAllMembersReply = 199;
const int kInterSignalGuildAllMembersNextReply = 200;
const size_t kGuildMemberRecordSize = 0x3Fu;
const size_t kGuildInfoRecordSize = 0x1Du;
const uint16_t kGuildInfoNoticeSize = 0x0139u;
const uint16_t kGuildInfoReplyStorageSize = 0x018Fu;
const int kInterSignalGuildChatReply = 90;
// Official 0627 CDispatchServerPacket::dispatchPacket @0x8473660 maps the
// guild create result (0x043f) to Inter 237 using the UID at +0x0e.  Its
// 0x040c branch maps the create notification to Inter 98 using the unaligned
// UID at +0x29.
const uint16_t kGuildCreateReplyOpcode = 0x043Fu;
const uint16_t kGuildCreateNoticeOpcode = 0x040Cu;
const int kInterSignalGuildCreateReply = 237;
const int kInterSignalGuildCreateNotice = 98;
// IDA 0x84740A3..0x84740F2 branches to 0x8479A14/0x8479AAE.
// These arrive before a monitor-side login result: the monitor asks the game
// server to identify itself, then acknowledges the registration sequence.
const uint16_t kMonitorReqGameServerInfoOpcode = 0x1F40u;
const uint16_t kMonitorRegistGameServerAckOpcode = 0x1F42u;
const int kInterSignalReqGameServerInfo = 503;
const int kInterSignalMonitorRegistGameServerAck = 504;
// Official 0627 ELF dispatchPacket comparison leaves:
// 0x84740FD selects opcode 0x1F4B and 0x8479DB6 queues Inter 529 as a
// global time-sync broadcast.  0x847412A selects opcode 0x1F4C and
// 0x8479E47 queues Inter 530 for the native i386 UID stored at +0x0E.
const uint16_t kMonitorTimeSyncOpcode = 0x1F4Bu;
const uint16_t kMonitorTimeSyncLoginOpcode = 0x1F4Cu;
const int kInterSignalTimeSync = 529;
const int kInterSignalTimeSyncLogin = 530;
// IDA CDispatchServerPacket::dispatchPacket @0x8475375..0x847542F:
// monitor opcode 2526 is copied intact after signal 184 / uid -1 and placed
// on the otherwise unnamed queue index 5 for CExchangeServerThread.
const uint16_t kExchangeServerInfoOpcode = 0x09DEu;
const int kInterSignalExchangeServerInfo = 184;
const int kExchangeServerInfoPacketSize = 0x14;
// Official 0627 ELF /root/df_game_r_official_ida_19ebd222:
// dispatchPacket+0x7018 @0x847A678.  The monitor's complete 0x27DA reply is
// queued as Inter 0x2C6 and the user UID is the ingress header's user_uid.
// Inter_NpcLimitBuyItemInfoAll then emits client NOTI 403, which is part of
// the original character-select completion sequence before world entry.
const uint16_t kNpcLimitBuyItemInfoAllOpcode = 0x27DAu;
const int kInterSignalNpcLimitBuyItemInfoAll = 0x02C6;  // decimal 710.
// The adjacent inbound monitor replies use global dispatch (user index -1).
// Official dispatchPacket branches: 0x847A5DE, 0x847A70B, 0x847A79E.
const uint16_t kNpcLimitBuyItemInfoOpcode = 0x27D9u;
const uint16_t kNpcLimitBuyItemChangeInfoOpcode = 0x27DBu;
const uint16_t kNpcLimitBuyItemInfoRenewOpcode = 0x27DCu;
const int kInterSignalNpcLimitBuyItemInfo = 0x02C5;        // decimal 709.
const int kInterSignalNpcLimitBuyItemChangeInfo = 0x02C7;  // decimal 711.
const int kInterSignalNpcLimitBuyItemInfoRenew = 0x02C8;   // decimal 712.

Stream* AcquireDispatchStream(int packet_size) {
    // IDA 0x847366F..0x8473745: dispatchPacket adds its two queued int
    // fields, then selects the matching real stream-pool family.  In
    // particular, the 0x33-byte monitor login-result frame must be owned by
    // SmallStreamPool; falling back to a different pool would change the
    // CStreamGuard release contract.
    const int queued_size = packet_size + 8;
    if (queued_size <= 0x0FFF) {
        SmallStreamPool* const small_pool = GlobalData::small_stream_pool();
        return small_pool ? small_pool->Acquire("DispatchServerPacket.cpp", 54)
                          : 0;
    }

    if (queued_size <= 0x3FFF) {
        StreamPool* const stream_pool = GlobalData::stream_pool();
        return stream_pool ? stream_pool->Acquire("DispatchServerPacket.cpp", 58)
                           : 0;
    }

    BigStreamPool* const big_pool = GlobalData::big_stream_pool();
    return big_pool ? big_pool->Acquire("DispatchServerPacket.cpp", 62) : 0;
}

int32_t ReadServerPacketRoutingUid(const char* packet, size_t offset) {
    // Several official server frames place the i386 UID at an unaligned
    // byte offset (the guild-create notification uses +0x29).  Preserve the
    // native little-endian bytes without forming an unaligned uint32_t
    // reference in reconstructed C++.
    int32_t uid = 0;
    std::memcpy(&uid, packet + offset, sizeof(uid));
    return uid;
}

bool RouteGuildQueryReply(const char* packet,
                          uint16_t declared_size,
                          int inter_signal,
                          int packet_uid) {
    // Despite the historical helper name this is the common official shape
    // for a complete user-routed Inter frame.  Member replies below use the
    // same queue envelope; the packet body remains monitor-owned.
    if (!GlobalData::msgq_mgr()) {
        return false;
    }

    CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
    if (!stream_guard.Get()) {
        return false;
    }

    (*stream_guard) << inter_signal;
    (*stream_guard) << packet_uid;
    stream_guard->put_binary(const_cast<char*>(packet), declared_size);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
    return true;
}

}  // namespace

// IDA_EA: CDispatchServerPacket::dispatchPacket @ 0x8473EDB..0x847A15A.
// The recovered cases below are exact original routes.  Other packet kinds
// remain deliberately unclaimed until their owning receiver is restored.
bool DispatchServerPacket(const char* packet, int packet_size) {
    if (!packet || packet_size < static_cast<int>(sizeof(PacketHeader))) {
        return false;
    }

    const PacketHeader& packet_header =
        *reinterpret_cast<const PacketHeader*>(packet);
    const uint16_t opcode = packet_header.type;
    const uint16_t declared_size = packet_header.size;

    // CNetwork::Parsing_ServerPacket guarantees this for live traffic.  Keep
    // the check at the bridge boundary so a direct caller cannot make the
    // original +0x0e UID read or binary copy escape the received frame.
    if (declared_size < sizeof(PacketHeader) ||
        packet_size != static_cast<int>(declared_size)) {
        return false;
    }

    if (opcode == kMonitorNotifyNewMailOpcode) {
        if (declared_size != sizeof(Packet_Monitor_Notify_New_Mail)) {
            return false;
        }
        const int packet_uid = ReadServerPacketRoutingUid(
            packet,
            offsetof(Packet_Monitor_Notify_New_Mail, routing_user_uid));
        if (!RouteGuildQueryReply(packet,
                                  declared_size,
                                  kInterSignalMonitorNotifyNewMail,
                                  packet_uid)) {
            return false;
        }
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE130] opcode=%u size=%u uid=%d",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    if (opcode == kMonitorNotifyAuctionMailOpcode) {
        if (declared_size != sizeof(Packet_Monitor_Notify_Auction_Mail)) {
            LogManager::logFormat(
                1, "CDispatchServerPacket.cpp",
                "static bool CDispatchServerPacket::dispatchPacket(char*)",
                0,
                "[AUCTION_MAIL_DIAG][ROUTE239_REJECT_SIZE] "
                "opcode=%u declared=%u received=%d expected=%u",
                static_cast<unsigned int>(opcode),
                static_cast<unsigned int>(declared_size), packet_size,
                static_cast<unsigned int>(
                    sizeof(Packet_Monitor_Notify_Auction_Mail)));
            return false;
        }

        const Packet_Monitor_Notify_Auction_Mail& notification =
            *reinterpret_cast<const Packet_Monitor_Notify_Auction_Mail*>(
                packet);
        const int packet_uid = notification.routing_user_uid;
        if (!RouteGuildQueryReply(packet,
                                  declared_size,
                                  kInterSignalMonitorNotifyAuctionMail,
                                  packet_uid)) {
            return false;
        }
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0,
            "[AUCTION_MAIL_DIAG][ROUTE239] opcode=%u size=%u uid=%d "
            "receiver=%u mail_item_id=%u notice_type=%u item_type=%u",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid,
            notification.receiver_charac_no, notification.mail_item_id,
            static_cast<unsigned int>(notification.notification_type),
            static_cast<unsigned int>(notification.mail_item_type));
        return true;
    }

    if (opcode == kMonitorLoginOpcode) {
        // Raw 0x84742B6 writes 65, 0x84742CC takes the complete dword at
        // Packet_Monitor_LoginReply::uid, and 0x84742F7 copies exactly 0x33
        // bytes.  The monitor
        // reply is therefore a distinct concrete route, not a synthetic
        // local login-success response.
        if (declared_size != sizeof(Packet_Monitor_LoginReply) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        const Packet_Monitor_LoginReply& reply =
            *reinterpret_cast<const Packet_Monitor_LoginReply*>(packet);
        CStreamGuard stream_guard(
            AcquireDispatchStream(sizeof(reply)), true);
        if (!stream_guard.Get()) {
            return false;
        }

        const int packet_uid = reply.uid;
        (*stream_guard) << kInterSignalMonitorLoginOK;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet), sizeof(reply));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE65] opcode=%u size=%u uid=%d",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    if (opcode == kMonitorBuddyInOutOpcode) {
        // Official 0627 ELF 0x84747E7 writes Inter signal 68,
        // 0x84747FD reads Packet_Monitor_Buddy_InOut::target_user_uid, and
        // 0x847482A copies exactly 0x34 bytes before queueing on DISPATCH_Q.
        if (declared_size < sizeof(Packet_Monitor_Buddy_InOut) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        const Packet_Monitor_Buddy_InOut& reply =
            *reinterpret_cast<const Packet_Monitor_Buddy_InOut*>(packet);
        const int packet_uid = reply.target_user_uid;
        (*stream_guard) << kInterSignalMonitorBuddyInOut;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet),
                                 sizeof(reply));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE68] opcode=%u size=%u uid=%d",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    if (opcode == kMonitorBuddyListReplyOpcode) {
        if (declared_size < sizeof(Packet_Monitor_Reply_Buddy_List_Prefix) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        const Packet_Monitor_Reply_Buddy_List_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Reply_Buddy_List_Prefix*>(packet);
        const size_t expected_size =
            sizeof(reply) + reply.count * kMonitorBuddyListRecordSize;
        if (reply.count > kMonitorBuddyListMaxCount ||
            declared_size != expected_size) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        (*stream_guard) << kInterSignalMonitorBuddyList;
        (*stream_guard) << -1;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE70] opcode=%u size=%u account=%u count=%u",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), reply.account_id,
            static_cast<unsigned int>(reply.count));
        return true;
    }

    if (opcode == kMonitorAddBuddyReplyOpcode) {
        // Official 0627 ELF: opcode 1650 writes Inter signal 188, reads the
        // routing UID at +0x0e, copies exactly 0x3b bytes, then queues the
        // stream on DISPATCH_Q.  Preserve the full monitor frame because
        // Inter_MonitorAddBuddy consumes the character number and buddy
        // snapshot at their original packet offsets.
        if (declared_size != sizeof(Packet_Monitor_Add_Buddy_Reply) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        const Packet_Monitor_Add_Buddy_Reply& reply =
            *reinterpret_cast<const Packet_Monitor_Add_Buddy_Reply*>(packet);
        CStreamGuard stream_guard(AcquireDispatchStream(sizeof(reply)), true);
        if (!stream_guard.Get()) {
            return false;
        }

        const int packet_uid = reply.target_user_uid;
        (*stream_guard) << kInterSignalMonitorAddBuddy;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet), sizeof(reply));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE188] opcode=%u size=%u uid=%d",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    for (size_t route_index = 0;
         route_index <
         sizeof(kMonitorMemberRoutes) / sizeof(kMonitorMemberRoutes[0]);
         ++route_index) {
        const MonitorMemberRoute& route = kMonitorMemberRoutes[route_index];
        if (opcode != route.opcode) {
            continue;
        }
        if (declared_size != route.packet_size) {
            return false;
        }

        const int packet_uid =
            ReadServerPacketRoutingUid(packet, route.routing_uid_offset);
        return RouteGuildQueryReply(packet,
                                    declared_size,
                                    route.inter_signal,
                                    packet_uid);
    }

    for (size_t route_index = 0;
         route_index < sizeof(kGuildUserRoutes) / sizeof(kGuildUserRoutes[0]);
         ++route_index) {
        const MonitorMemberRoute& route = kGuildUserRoutes[route_index];
        if (opcode != route.opcode) {
            continue;
        }
        if (declared_size != route.packet_size) {
            return false;
        }

        const int packet_uid =
            ReadServerPacketRoutingUid(packet, route.routing_uid_offset);
        return RouteGuildQueryReply(packet,
                                    declared_size,
                                    route.inter_signal,
                                    packet_uid);
    }

    if (opcode == kGuildChatReplyOpcode) {
        // Official 0627 CDispatchServerPacket @0x8474F46..0x8474FCD:
        // opcode 0x0400 writes Inter signal 90, reads the composite routing
        // UID at +0x0a and copies the complete declared frame.  The restored
        // Inter receiver consumes target charac_no/name/length at
        // +0x0e/+0x12/+0x30 respectively.
        if (declared_size <
            sizeof(Packet_Monitor_Guild_Chat_Reply_Prefix)) {
            return false;
        }
        const Packet_Monitor_Guild_Chat_Reply_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Guild_Chat_Reply_Prefix*>(packet);
        const size_t expected_size = sizeof(reply) + reply.message_length;
        if (declared_size != expected_size) {
            return false;
        }
        return RouteGuildQueryReply(packet,
                                    declared_size,
                                    kInterSignalGuildChatReply,
                                    reply.target_user_uid);
    }

    if (opcode == kGuildMembersReplyOpcode || opcode == kGuildAllMembersReplyOpcode) {
        if (declared_size <
            sizeof(Packet_Monitor_Call_Guild_Members_Reply_Prefix)) {
            return false;
        }

        const Packet_Monitor_Call_Guild_Members_Reply_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Call_Guild_Members_Reply_Prefix*>(
                packet);
        // The two first-page prefixes are both 0x34 bytes, but NOT identical:
        // online 0x401: count +0x2d, cursor +0x2e, page +0x32;
        // all    0x424: cursor +0x2d, page +0x31, count +0x33.
        // Original Inter_MonitorCallGuildAllMembers @0x84CABDA reads [51].
        const unsigned char member_count = opcode == kGuildAllMembersReplyOpcode
            ? static_cast<unsigned char>(packet[0x33]) : reply.member_count;
        const size_t expected_size = sizeof(reply) +
                                     member_count * kGuildMemberRecordSize;
        if (declared_size != expected_size) {
            if (opcode == kGuildAllMembersReplyOpcode) {
                LogManager::logFormat(1, "CDispatchServerPacket.cpp", __FUNCTION__, __LINE__,
                    "[GUILD][ALL_MEMBERS_REJECT] count=%u size=%u expected=%u",
                    static_cast<unsigned int>(member_count), static_cast<unsigned int>(declared_size),
                    static_cast<unsigned int>(expected_size));
            }
            return false;
        }
        const int signal = opcode == kGuildAllMembersReplyOpcode
            ? kInterSignalGuildAllMembersReply : kInterSignalGuildMembersReply;
        if (opcode == kGuildAllMembersReplyOpcode) {
            LogManager::logFormat(1, "CDispatchServerPacket.cpp", __FUNCTION__, __LINE__,
                "[GUILD][ALL_MEMBERS_ROUTE] opcode=%u uid=%d count=%u size=%u",
                static_cast<unsigned int>(opcode), reply.target_user_uid,
                static_cast<unsigned int>(member_count), static_cast<unsigned int>(declared_size));
        }
        return RouteGuildQueryReply(
            packet,
            declared_size,
            signal,
            reply.target_user_uid);
    }

    if (opcode == kGuildMembersNextReplyOpcode || opcode == kGuildAllMembersNextReplyOpcode) {
        // Official 0627 CDispatchServerPacket @0x8475C0D..0x8475CA7:
        // opcode 0x0423 writes Inter signal 198, routes by UID +0x0a and
        // copies the complete declared frame.  Validate the variable member
        // tail before preserving that route.
        if (declared_size <
            sizeof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix)) {
            return false;
        }

        const Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix*>(
                packet);
        const size_t expected_size = sizeof(reply) +
                                     reply.member_count *
                                         kGuildMemberRecordSize;
        if (declared_size != expected_size) {
            return false;
        }
        // Original CDispatchServerPacket routes 0x424/0x428 to Inter 199/200
        // with the same +0x0a UID and complete variable-length member frames.
        const int signal = opcode == kGuildAllMembersNextReplyOpcode
            ? kInterSignalGuildAllMembersNextReply : kInterSignalGuildMembersNextReply;
        if (opcode == kGuildAllMembersNextReplyOpcode) {
            LogManager::logFormat(1, "CDispatchServerPacket.cpp", __FUNCTION__, __LINE__,
                "[GUILD][ALL_MEMBERS_ROUTE] opcode=%u uid=%d count=%u size=%u",
                static_cast<unsigned int>(opcode), reply.target_user_uid,
                static_cast<unsigned int>(reply.member_count), static_cast<unsigned int>(declared_size));
        }
        return RouteGuildQueryReply(
            packet,
            declared_size,
            signal,
            reply.target_user_uid);
    }

    if (opcode == kGuildInfoReplyOpcode) {
        if (declared_size <
            sizeof(Packet_Monitor_Call_Guild_Info_Reply_Prefix)) {
            return false;
        }
        const Packet_Monitor_Call_Guild_Info_Reply_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Call_Guild_Info_Reply_Prefix*>(packet);
        const size_t expected_size = sizeof(reply) +
                                     reply.guild_count *
                                         kGuildInfoRecordSize;
        if (declared_size != expected_size ||
            expected_size > kGuildInfoReplyStorageSize) {
            return false;
        }
        return RouteGuildQueryReply(packet,
                                    declared_size,
                                    kInterSignalGuildInfoReply,
                                    reply.target_user_uid);
    }

    if (opcode == kGuildInfoNoticeOpcode) {
        if (declared_size != kGuildInfoNoticeSize) {
            return false;
        }
        const Packet_Monitor_Notice_Guild_Info_Reply_Prefix& reply =
            *reinterpret_cast<
                const Packet_Monitor_Notice_Guild_Info_Reply_Prefix*>(packet);
        return RouteGuildQueryReply(packet,
                                    declared_size,
                                    kInterSignalGuildInfoNotice,
                                    reply.target_user_uid);
    }

    if (opcode == kGuildSetKeyOpcode) {
        // Official 0627 CDispatchServerPacket @0x84750B5..0x8475148:
        // opcode 0x040b writes Inter signal 97, reads the routing UID at
        // +0x0a, copies exactly 0x16 bytes, and queues the stream on
        // DISPATCH_Q.  Inter_MonitorSetGuildKey is already the owner of the
        // login-state and in-world key/update-notification semantics.
        if (declared_size != sizeof(Packet_Monitor_Set_Guild_Key)) {
            return false;
        }

        const Packet_Monitor_Set_Guild_Key& reply =
            *reinterpret_cast<const Packet_Monitor_Set_Guild_Key*>(packet);
        if (!RouteGuildQueryReply(packet,
                                  declared_size,
                                  kInterSignalGuildSetKey,
                                  reply.target_user_uid)) {
            return false;
        }
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE97] opcode=%u size=%u uid=%d charac_no=%u guild_key=%u",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size),
            reply.target_user_uid,
            reply.target_charac_no,
            reply.guild_key);
        return true;
    }

    if (opcode == kGuildCreateReplyOpcode) {
        if (declared_size != sizeof(Packet_Guild_Create_Reply) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(
            AcquireDispatchStream(sizeof(Packet_Guild_Create_Reply)), true);
        if (!stream_guard.Get()) {
            return false;
        }

        const int packet_uid = ReadServerPacketRoutingUid(
            packet, offsetof(Packet_Guild_Create_Reply, target_user_uid));
        (*stream_guard) << kInterSignalGuildCreateReply;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet),
                                 sizeof(Packet_Guild_Create_Reply));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        return true;
    }

    if (opcode == kGuildCreateNoticeOpcode) {
        if (declared_size !=
                sizeof(Packet_Monitor_Notice_Guild_Create_ToUser) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(
                                      sizeof(Packet_Monitor_Notice_Guild_Create_ToUser)),
                                  true);
        if (!stream_guard.Get()) {
            return false;
        }

        const int packet_uid = ReadServerPacketRoutingUid(
            packet,
            offsetof(Packet_Monitor_Notice_Guild_Create_ToUser,
                     target_user_uid));
        (*stream_guard) << kInterSignalGuildCreateNotice;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(
            const_cast<char*>(packet),
            sizeof(Packet_Monitor_Notice_Guild_Create_ToUser));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        return true;
    }

    if (opcode == kMonitorReqGameServerInfoOpcode ||
        opcode == kMonitorRegistGameServerAckOpcode) {
        // IDA 0x8479A14..0x8479B3E: both monitor registration frames write
        // the exact outer Inter signal, write uid -1, and copy the complete
        // header-sized frame.  Unlike 0x03E8, neither branch has a fixed
        // packet length; the downstream receiver validates the fields it
        // consumes (currently byte offsets +0x0a/+0x0b).
        const int signal = opcode == kMonitorReqGameServerInfoOpcode
            ? kInterSignalReqGameServerInfo
            : kInterSignalMonitorRegistGameServerAck;
        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        (*stream_guard) << signal;
        (*stream_guard) << -1;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            opcode == kMonitorReqGameServerInfoOpcode
                ? "[MONITOR][ROUTE503] opcode=%u size=%u"
                : "[MONITOR][ROUTE504] opcode=%u size=%u",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size));
        return true;
    }

    if (opcode == kMonitorTimeSyncOpcode ||
        opcode == kMonitorTimeSyncLoginOpcode) {
        // IDA 0x8479DA2..0x8479ED4: both branches copy the complete declared
        // monitor frame.  The periodic form is global (uid -1); the login
        // form reads Packet_Monitor_Time_Sync_Login::target_user_uid.
        if (!GlobalData::msgq_mgr() ||
            (opcode == kMonitorTimeSyncLoginOpcode &&
             declared_size < sizeof(Packet_Monitor_Time_Sync_Login))) {
            return false;
        }

        const int signal = opcode == kMonitorTimeSyncOpcode
            ? kInterSignalTimeSync
            : kInterSignalTimeSyncLogin;
        int packet_uid = -1;
        if (opcode == kMonitorTimeSyncLoginOpcode) {
            const Packet_Monitor_Time_Sync_Login& reply =
                *reinterpret_cast<const Packet_Monitor_Time_Sync_Login*>(
                    packet);
            packet_uid = reply.target_user_uid;
        }
        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        (*stream_guard) << signal;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE%d] opcode=%u size=%u uid=%d", signal,
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    if (opcode == kExchangeServerInfoOpcode) {
        if (declared_size != kExchangeServerInfoPacketSize ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        (*stream_guard) << kInterSignalExchangeServerInfo;
        (*stream_guard) << -1;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(
            static_cast<MsgQueueMgr::QUEUE_IDX>(5), stream_guard);
        return true;
    }

    if (opcode == kNpcLimitBuyItemInfoAllOpcode) {
        // IDA 0x847A678..0x847A706: write signal 710, write the packet's
        // unmodified +0x0A UID, copy the complete declared frame, then put
        // the guard on DISPATCH_Q.  Do not byte-swap or repair the UID here:
        // the official router treats this field as the native i386 dword.
        if (declared_size <
                sizeof(Packet_Npc_Limit_Buy_Item_Info_All_Header) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        const Packet_Npc_Limit_Buy_Item_Info_All_Header& reply =
            *reinterpret_cast<
                const Packet_Npc_Limit_Buy_Item_Info_All_Header*>(packet);
        const int packet_uid = reply.user_uid;
        (*stream_guard) << kInterSignalNpcLimitBuyItemInfoAll;
        (*stream_guard) << packet_uid;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE710] opcode=%u size=%u uid=%d",
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size), packet_uid);
        return true;
    }

    if (opcode == kNpcLimitBuyItemInfoOpcode ||
        opcode == kNpcLimitBuyItemChangeInfoOpcode ||
        opcode == kNpcLimitBuyItemInfoRenewOpcode) {
        // Official 0627 dispatchPacket copies each complete declared frame
        // after writing its exact Inter signal and global user index -1.
        // 0x27DD is intentionally absent: it is the game-to-monitor update
        // packet and has no inbound branch in the official comparison tree.
        if (!GlobalData::msgq_mgr()) {
            return false;
        }

        int signal = kInterSignalNpcLimitBuyItemInfo;
        if (opcode == kNpcLimitBuyItemChangeInfoOpcode) {
            signal = kInterSignalNpcLimitBuyItemChangeInfo;
        } else if (opcode == kNpcLimitBuyItemInfoRenewOpcode) {
            signal = kInterSignalNpcLimitBuyItemInfoRenew;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        (*stream_guard) << signal;
        (*stream_guard) << -1;
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        LogManager::logFormat(
            1, "CDispatchServerPacket.cpp",
            "static bool CDispatchServerPacket::dispatchPacket(char*)", 0x41,
            "[MONITOR][ROUTE%d] opcode=%u size=%u uid=-1", signal,
            static_cast<unsigned int>(opcode),
            static_cast<unsigned int>(declared_size));
        return true;
    }

    if (opcode == kHadesActionOpcode) {
        if (declared_size < sizeof(Packet_Hades_Action) ||
            !GlobalData::msgq_mgr()) {
            return false;
        }

        CStreamGuard stream_guard(AcquireDispatchStream(declared_size), true);
        if (!stream_guard.Get()) {
            return false;
        }

        // Raw @0x847A095 writes full immediate 0x2B6, then takes the UID at
        // Packet_Hades_Action::user_uid.  A zero UID is represented as -1
        // for InterDispatcher;
        // nonzero values retain their full signed 32-bit bit pattern.
        const Packet_Hades_Action& action =
            *reinterpret_cast<const Packet_Hades_Action*>(packet);
        const int packet_uid = action.user_uid;
        (*stream_guard) << kInterSignalHadesAction;
        (*stream_guard) << (packet_uid ? packet_uid : -1);
        stream_guard->put_binary(const_cast<char*>(packet), declared_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        return true;
    }

    if (opcode == kHadesReconnectOpcode) {
        // Raw @0x847A148 calls the proxy directly; this is not an Inter slot.
        CHadesServerProxy* const hades_proxy = GlobalData::s_hades_proxy;
        if (hades_proxy) {
            hades_proxy->ConnTcpHadesSvr();
        }
        return true;
    }

    return false;
}

void DispatchHadesServerPacket(const char* packet, int packet_size) {
    (void)DispatchServerPacket(packet, packet_size);
}

void InstallHadesServerPacketBridge() {
    SetServerPacketDispatch(&DispatchHadesServerPacket);
}
