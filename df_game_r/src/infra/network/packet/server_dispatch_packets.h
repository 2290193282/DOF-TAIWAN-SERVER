#ifndef OLDSRV_INFRA_NETWORK_PACKET_SERVER_DISPATCH_PACKETS_H_
#define OLDSRV_INFRA_NETWORK_PACKET_SERVER_DISPATCH_PACKETS_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"


// Named 0627 server-to-server ingress records routed by
// CDispatchServerPacket.  Each packet owns its real protocol fields; no
// common raw payload overlay is used across unrelated monitor/Hades opcodes.
#pragma pack(push, 1)

struct Packet_Monitor_Buddy_InOut {
    PacketHeader header;
    uint32_t buddy_charac_no;
    int32_t target_user_uid;
    uint8_t buddy_state;
    uint8_t relation_type;
    uint8_t is_buddy_in;
    char buddy_name[30];
    uint8_t send_return_mail;
};

// Monitor opcode 0x0672 reply.  The monitor reuses the add-buddy request
// opcode for its DB result, but expands the frame from 0x30 to 0x3b bytes.
// CDispatchServerPacket routes by the game UID at +0x0e; the complete
// 40-byte buddy snapshot remains owned by Inter_MonitorAddBuddy.
struct Packet_Monitor_Add_Buddy_Reply {
    PacketHeader header;
    uint32_t requester_charac_no;
    int32_t target_user_uid;
    uint8_t buddy_snapshot[40];
    uint8_t error_code;
};

// Monitor opcode 0x03f2.  The wire body is this prefix followed by count
// contiguous 0x2a-byte BuddyList records (at most 32 in the official monitor
// object).  The reply is global-routed; account_id is resolved by GameWorld.
struct Packet_Monitor_Reply_Buddy_List_Prefix {
    PacketHeader header;
    uint32_t account_id;
    uint8_t count;
};

struct Packet_Monitor_Time_Sync_Login {
    PacketHeader header;
    uint32_t account_id;
    int32_t target_user_uid;
    uint16_t first_time_value;
    uint16_t second_time_value;
};

// Monitor opcode 0x04b6: online/offline notification for a mentor relation.
// Unlike the other member replies its routing UID starts at the unaligned
// +0x0b position, immediately after event_flag.
struct Packet_Monitor_Notice_Member_Login_Out {
    PacketHeader header;
    uint8_t event_flag;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t notification_type;
    uint8_t upper_member_changed;
    char member_name[30];
    uint8_t upper_member_exp_level;
    uint32_t upper_member_key;
};

// Monitor opcode 0x04b7: result returned to the character which requested a
// new mentor/member relationship.
struct Packet_Monitor_Request_Member_Enter_To_Requester {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t result_code;
    char member_name[30];
};

// Monitor opcode 0x04b8: request delivered to the prospective responder.
struct Packet_Monitor_Request_Member_Enter_To_Responder {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint16_t relation_request_id;
    char member_name[30];
};

// Monitor opcodes 0x04b9 and 0x04ba share the same official result record.
// The routing UID is the unaligned dword at +0x0d.
struct Packet_Monitor_Member_Enter_Result {
    PacketHeader header;
    uint8_t action;
    uint8_t relation_type;
    uint8_t upper_member_connected;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t result_code;
    char member_name[30];
    uint8_t upper_member_exp_level;
    uint32_t upper_member_key;
};

// Monitor opcodes 0x04bb and 0x04bc share the same secede record.
struct Packet_Monitor_Member_Secede_Result {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t reason;
    char member_name[30];
};

// Monitor opcode 0x04bd clears the current member/upper-member identifiers.
struct Packet_Monitor_Delete_Member_Id {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
};

// Monitor opcode 0x04be has a fixed-capacity official list: one 43-byte
// upper-member record followed by ten 42-byte member rows.
struct Packet_Monitor_Call_Member_List {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t upper_member_record[43];
    uint8_t member_records[10][42];
};

// Monitor opcode 0x04c0 is the server-side reward reply for tax/fatigue paid
// by a lower member to the upper member.
struct Packet_Monitor_Pay_Tax_To_Upper {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint32_t money;
    uint32_t fatigue;
    char member_name[30];
};

// Guild-server opcode 0x0400 is the ordinary (non-hyperlink) guild-chat
// fan-out reply.  The fixed prefix ends immediately before the
// message_length bytes.  CDispatchServerPacket routes by the composite game
// user UID at +0x0a and queues the complete frame as Inter signal 90.
struct Packet_Monitor_Guild_Chat_Reply_Prefix {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    char speaker_name[30];
    uint8_t message_length;
};

// Guild-server opcode 0x0401 has a variable member tail.  The fixed prefix
// ends immediately before count contiguous 0x3f-byte ST_Guild_Mem_Info
// records.  CDispatchServerPacket routes the complete declared frame.
struct Packet_Monitor_Call_Guild_Members_Reply_Prefix {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint32_t guild_key;
    char guild_name[23];
    uint8_t member_count;
    uint32_t member_list_cursor;
    uint16_t member_list_page;
};

// Guild-server opcode 0x0423 is the subsequent-page companion to 0x0401.
// Inter_MonitorCallGuildMembersNext consumes count at +0x16 followed by the
// same packed 0x3f-byte ST_Guild_Mem_Info records.
struct Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint32_t guild_key;
    uint8_t member_count;
};

// Guild-server opcode 0x040d is a bounded guild-search response.  Its real
// declared length is this prefix plus count packed 29-byte result rows; the
// original 0x18f object extent is storage capacity, not a wire-size promise.
struct Packet_Monitor_Call_Guild_Info_Reply_Prefix {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint8_t guild_count;
};

// Guild-server opcode 0x040a is a fixed 0x0139-byte current-guild snapshot.
// STGuildDBInfoOnly and the trailing text remain owned by the Inter receiver.
struct Packet_Monitor_Notice_Guild_Info_Reply_Prefix {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint32_t guild_key;
};

// Guild-server opcode 0x040b.  The official CDispatchServerPacket route
// reads the game-user UID at +0x0a, queues Inter_MonitorSetGuildKey (97),
// and copies the complete fixed 0x16-byte frame.  This is the authoritative
// guild-key synchronization used both while selecting a character and after
// a guild membership transition.
struct Packet_Monitor_Set_Guild_Key {
    PacketHeader header;
    int32_t target_user_uid;
    uint32_t target_charac_no;
    uint32_t guild_key;
};

// Guild-server opcode 0x043f.  The official CDispatchServerPacket route
// reads target_user_uid at +0x0e, queues Inter_ReplyGuildCreate (237), and
// copies the complete fixed 0x2d-byte frame.
struct Packet_Guild_Create_Reply {
    PacketHeader header;
    uint32_t target_charac_no;
    int32_t target_user_uid;
    int32_t result_code;
    char guild_name[23];
};

// Guild-server opcode 0x040c.  This is the create notification broadcast
// sent after the guild DB transaction succeeds.  Its routing UID is the
// unaligned native dword at +0x29, after the 23-byte guild-name field.
struct Packet_Monitor_Notice_Guild_Create_ToUser {
    PacketHeader header;
    uint32_t guild_key;
    uint32_t target_charac_no;
    char guild_name[23];
    int32_t target_user_uid;
};

struct Packet_Hades_Action {
    PacketHeader header;
    uint32_t account_id;
    int32_t user_uid;
    uint32_t action_flags;
    uint32_t punish_days;
    uint32_t apply_switch;
    uint32_t reserved;
};

// Opcode 0x27da has a variable body.  Its routing identity is the complete
// fixed prefix through user_uid; later fields remain owned by the NPC-limit
// receiver instead of being represented by an invented raw tail here.
struct Packet_Npc_Limit_Buy_Item_Info_All_Header {
    PacketHeader header;
    int32_t user_uid;
};

#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 0x0au,
              "server packet header must retain its 0627 extent");
static_assert(sizeof(Packet_Monitor_Buddy_InOut) == 0x34u,
              "monitor buddy in/out packet must retain its 0627 extent");
static_assert(offsetof(Packet_Monitor_Buddy_InOut, buddy_charac_no) == 0x0au &&
                  offsetof(Packet_Monitor_Buddy_InOut, target_user_uid) ==
                      0x0eu &&
                  offsetof(Packet_Monitor_Buddy_InOut, buddy_state) == 0x12u &&
                  offsetof(Packet_Monitor_Buddy_InOut, relation_type) ==
                      0x13u &&
                  offsetof(Packet_Monitor_Buddy_InOut, is_buddy_in) == 0x14u &&
                  offsetof(Packet_Monitor_Buddy_InOut, buddy_name) == 0x15u &&
                  offsetof(Packet_Monitor_Buddy_InOut, send_return_mail) ==
                      0x33u,
              "monitor buddy in/out packet fields moved");

static_assert(sizeof(Packet_Monitor_Add_Buddy_Reply) == 0x3bu,
              "monitor add-buddy reply must retain its 0627 extent");
static_assert(
    offsetof(Packet_Monitor_Add_Buddy_Reply, requester_charac_no) == 0x0au &&
        offsetof(Packet_Monitor_Add_Buddy_Reply, target_user_uid) == 0x0eu &&
        offsetof(Packet_Monitor_Add_Buddy_Reply, buddy_snapshot) == 0x12u &&
        offsetof(Packet_Monitor_Add_Buddy_Reply, error_code) == 0x3au,
    "monitor add-buddy reply fields moved");

static_assert(sizeof(Packet_Monitor_Reply_Buddy_List_Prefix) == 0x0fu,
              "monitor buddy-list reply prefix must retain its 0627 extent");
static_assert(
    offsetof(Packet_Monitor_Reply_Buddy_List_Prefix, account_id) == 0x0au &&
        offsetof(Packet_Monitor_Reply_Buddy_List_Prefix, count) == 0x0eu,
    "monitor buddy-list reply fields moved");

static_assert(sizeof(Packet_Monitor_Time_Sync_Login) == 0x16u,
              "monitor login time-sync packet must retain its 0627 extent");
static_assert(offsetof(Packet_Monitor_Time_Sync_Login, account_id) == 0x0au &&
                  offsetof(Packet_Monitor_Time_Sync_Login, target_user_uid) ==
                      0x0eu &&
                  offsetof(Packet_Monitor_Time_Sync_Login, first_time_value) ==
                      0x12u &&
                  offsetof(Packet_Monitor_Time_Sync_Login,
                           second_time_value) == 0x14u,
              "monitor login time-sync packet fields moved");

static_assert(
    sizeof(Packet_Monitor_Notice_Member_Login_Out) == 0x38u &&
        offsetof(Packet_Monitor_Notice_Member_Login_Out, event_flag) == 0x0au &&
        offsetof(Packet_Monitor_Notice_Member_Login_Out, target_user_uid) ==
            0x0bu &&
        offsetof(Packet_Monitor_Notice_Member_Login_Out, target_charac_no) ==
            0x0fu &&
        offsetof(Packet_Monitor_Notice_Member_Login_Out, member_name) == 0x15u &&
        offsetof(Packet_Monitor_Notice_Member_Login_Out, upper_member_key) ==
            0x34u,
    "member login/out reply ABI moved");

static_assert(
    sizeof(Packet_Monitor_Request_Member_Enter_To_Requester) == 0x31u &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Requester,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Requester,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Requester,
                 result_code) == 0x12u &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Requester,
                 member_name) == 0x13u,
    "member-enter requester reply ABI moved");

static_assert(
    sizeof(Packet_Monitor_Request_Member_Enter_To_Responder) == 0x32u &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Responder,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Responder,
                 relation_request_id) == 0x12u &&
        offsetof(Packet_Monitor_Request_Member_Enter_To_Responder,
                 member_name) == 0x14u,
    "member-enter responder request ABI moved");

static_assert(
    sizeof(Packet_Monitor_Member_Enter_Result) == 0x39u &&
        offsetof(Packet_Monitor_Member_Enter_Result, action) == 0x0au &&
        offsetof(Packet_Monitor_Member_Enter_Result, target_user_uid) == 0x0du &&
        offsetof(Packet_Monitor_Member_Enter_Result, target_charac_no) == 0x11u &&
        offsetof(Packet_Monitor_Member_Enter_Result, result_code) == 0x15u &&
        offsetof(Packet_Monitor_Member_Enter_Result, member_name) == 0x16u &&
        offsetof(Packet_Monitor_Member_Enter_Result, upper_member_key) == 0x35u,
    "member-enter result ABI moved");

static_assert(
    sizeof(Packet_Monitor_Member_Secede_Result) == 0x31u &&
        offsetof(Packet_Monitor_Member_Secede_Result, target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Member_Secede_Result, target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Member_Secede_Result, reason) == 0x12u &&
        offsetof(Packet_Monitor_Member_Secede_Result, member_name) == 0x13u,
    "member-secede result ABI moved");

static_assert(
    sizeof(Packet_Monitor_Delete_Member_Id) == 0x12u &&
        offsetof(Packet_Monitor_Delete_Member_Id, target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Delete_Member_Id, target_charac_no) == 0x0eu,
    "delete-member-id reply ABI moved");

static_assert(
    sizeof(Packet_Monitor_Call_Member_List) == 0x1e1u &&
        offsetof(Packet_Monitor_Call_Member_List, target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Call_Member_List, target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Call_Member_List, upper_member_record) == 0x12u &&
        offsetof(Packet_Monitor_Call_Member_List, member_records) == 0x3du,
    "member-list reply ABI moved");

static_assert(
    sizeof(Packet_Monitor_Pay_Tax_To_Upper) == 0x38u &&
        offsetof(Packet_Monitor_Pay_Tax_To_Upper, target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Pay_Tax_To_Upper, target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Pay_Tax_To_Upper, money) == 0x12u &&
        offsetof(Packet_Monitor_Pay_Tax_To_Upper, fatigue) == 0x16u &&
        offsetof(Packet_Monitor_Pay_Tax_To_Upper, member_name) == 0x1au,
    "member tax reply ABI moved");

static_assert(
    sizeof(Packet_Monitor_Guild_Chat_Reply_Prefix) == 0x31u &&
        offsetof(Packet_Monitor_Guild_Chat_Reply_Prefix, target_user_uid) ==
            0x0au &&
        offsetof(Packet_Monitor_Guild_Chat_Reply_Prefix, target_charac_no) ==
            0x0eu &&
        offsetof(Packet_Monitor_Guild_Chat_Reply_Prefix, speaker_name) ==
            0x12u &&
        offsetof(Packet_Monitor_Guild_Chat_Reply_Prefix, message_length) ==
            0x30u,
    "guild-chat reply prefix fields moved");

static_assert(
    sizeof(Packet_Monitor_Call_Guild_Members_Reply_Prefix) == 0x34u &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix, guild_key) ==
            0x12u &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix, guild_name) ==
            0x16u &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix,
                 member_count) == 0x2du &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix,
                 member_list_cursor) == 0x2eu &&
        offsetof(Packet_Monitor_Call_Guild_Members_Reply_Prefix,
                 member_list_page) == 0x32u,
    "guild-members reply prefix fields moved");

static_assert(
    sizeof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix) == 0x17u &&
        offsetof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix,
                 guild_key) == 0x12u &&
        offsetof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix,
                 member_count) == 0x16u,
    "guild-members-next reply prefix fields moved");

static_assert(
    sizeof(Packet_Monitor_Call_Guild_Info_Reply_Prefix) == 0x13u &&
        offsetof(Packet_Monitor_Call_Guild_Info_Reply_Prefix,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Call_Guild_Info_Reply_Prefix,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Call_Guild_Info_Reply_Prefix, guild_count) ==
            0x12u,
    "guild-info reply prefix fields moved");

static_assert(
    sizeof(Packet_Monitor_Notice_Guild_Info_Reply_Prefix) == 0x16u &&
        offsetof(Packet_Monitor_Notice_Guild_Info_Reply_Prefix,
                 target_user_uid) == 0x0au &&
        offsetof(Packet_Monitor_Notice_Guild_Info_Reply_Prefix,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Notice_Guild_Info_Reply_Prefix, guild_key) ==
            0x12u,
    "guild-info notice reply prefix fields moved");

static_assert(sizeof(Packet_Monitor_Set_Guild_Key) == 0x16u &&
                  offsetof(Packet_Monitor_Set_Guild_Key, target_user_uid) ==
                      0x0au &&
                  offsetof(Packet_Monitor_Set_Guild_Key, target_charac_no) ==
                      0x0eu &&
                  offsetof(Packet_Monitor_Set_Guild_Key, guild_key) == 0x12u,
              "guild-key synchronization fields moved");

static_assert(sizeof(Packet_Guild_Create_Reply) == 0x2du &&
                  offsetof(Packet_Guild_Create_Reply, target_charac_no) ==
                      0x0au &&
                  offsetof(Packet_Guild_Create_Reply, target_user_uid) ==
                      0x0eu &&
                  offsetof(Packet_Guild_Create_Reply, result_code) == 0x12u &&
                  offsetof(Packet_Guild_Create_Reply, guild_name) == 0x16u,
              "guild-create reply routing fields moved");

static_assert(
    sizeof(Packet_Monitor_Notice_Guild_Create_ToUser) == 0x2du &&
        offsetof(Packet_Monitor_Notice_Guild_Create_ToUser, guild_key) ==
            0x0au &&
        offsetof(Packet_Monitor_Notice_Guild_Create_ToUser,
                 target_charac_no) == 0x0eu &&
        offsetof(Packet_Monitor_Notice_Guild_Create_ToUser, guild_name) ==
            0x12u &&
        offsetof(Packet_Monitor_Notice_Guild_Create_ToUser,
                 target_user_uid) == 0x29u,
    "guild-create notification routing fields moved");

static_assert(sizeof(Packet_Hades_Action) == 0x22u,
              "Hades action packet must retain its 0627 extent");
static_assert(offsetof(Packet_Hades_Action, account_id) == 0x0au &&
                  offsetof(Packet_Hades_Action, user_uid) == 0x0eu &&
                  offsetof(Packet_Hades_Action, action_flags) == 0x12u &&
                  offsetof(Packet_Hades_Action, punish_days) == 0x16u &&
                  offsetof(Packet_Hades_Action, apply_switch) == 0x1au &&
                  offsetof(Packet_Hades_Action, reserved) == 0x1eu,
              "Hades action packet fields moved");

static_assert(sizeof(Packet_Npc_Limit_Buy_Item_Info_All_Header) == 0x0eu &&
                  offsetof(Packet_Npc_Limit_Buy_Item_Info_All_Header,
                           user_uid) == 0x0au,
              "NPC limit-buy routing header fields moved");

#endif  // OLDSRV_INFRA_NETWORK_PACKET_SERVER_DISPATCH_PACKETS_H_
