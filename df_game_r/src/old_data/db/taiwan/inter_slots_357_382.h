#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_357_382_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_357_382_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// The dispatcher owns only the wire envelopes and virtual receiver surface.
// Concrete behaviour is deliberately implemented by the functional owner
// (world, group, userinfo, or dungeon/PvP), not in this transport directory.
#pragma pack(push, 1)

struct SIG_LOAD_POWER_WAR_STATUE_RANKER {
    uint32_t charac_no[3];  // +0x00
};

// DB statue reply consumed intact by CPowerManager.  Its named fields are the
// exact byte ranges read by SetPowerWarUserStatueInfo.
struct SIG_LOAD_POWER_WAR_STATUE_INFO {
    uint32_t charac_no;      // +0x00
    char charac_name[30];    // +0x04
    uint8_t job;             // +0x22
    uint8_t level;           // +0x23
    uint8_t grow_type;       // +0x24
    char guild_name[40];     // +0x25
    uint8_t reserved_4d[3];  // +0x4d
    uint32_t guild_id;       // +0x50
    int32_t equip_item_id[11];  // +0x54
};

struct SIG_NOTICE_POWER_WAR_RANK {
    uint8_t routing_envelope[10];  // +0x00
    uint32_t charac_no[3];         // +0x0a
};

struct SIG_POWER_WAR_PROCESS_INFO {
    uint8_t routing_envelope[14];  // +0x00
    uint32_t charac_no;            // +0x0e
    int32_t process_value[5];      // +0x12
};

struct SIG_POWER_WAR_END_INFO {
    uint8_t routing_envelope[14];  // +0x00
    uint32_t charac_no;            // +0x0e
    uint8_t result;                // +0x12
    int32_t end_value[5];          // +0x13
};

// Monitor-to-channel shared-message notice.  This is intentionally distinct
// from SIG_UPDATE_SERVER_MESSAGE: its source channel and variable fields begin
// at the offsets observed by Inter_NoticeServerMessageInfo.
struct SIG_NOTICE_SERVER_MESSAGE_INFO {
    uint8_t routing_envelope[10];  // +0x00
    uint8_t source_channel;        // +0x0a
    uint8_t reserved_0b;           // +0x0b
    uint8_t kind;                  // +0x0c
    uint32_t message_len;          // +0x0d
    char message[50];              // +0x11
    uint32_t charac_name_len;      // +0x43
    char charac_name[20];          // +0x47
    uint32_t update_time;          // +0x5b
};

struct SIG_NOTICE_CREATE_GUILD_AGIT {
    uint8_t routing_envelope[18];  // +0x00
    uint32_t charac_no;            // +0x12
};

struct SIG_NOTICE_DELETE_GUILD_AGIT {
    uint8_t routing_envelope[18];  // +0x00
    uint32_t charac_no;            // +0x12
    uint8_t error;                 // +0x16
};

struct SIG_NOTICE_LOAD_GUILD_AGIT {
    uint8_t routing_envelope[22];  // +0x00
    uint8_t guild_agit_flag;       // +0x16
};

struct SIG_NOTICE_GUILD_MEMO {
    uint8_t routing_envelope[14];  // +0x00
    uint32_t charac_no;            // +0x0e
    char guild_memo[30];           // +0x12
    char guild_notice[30];         // +0x30
};

struct SIG_PCROOM_RESPONSE {
    uint8_t routing_envelope[18];  // +0x00
    uint32_t pc_room_no;           // +0x12
    int32_t premium_end_time;      // +0x16
    uint8_t ip_restriction_type;   // +0x1a
    uint8_t restriction_message;   // +0x1b
    uint8_t security_result;       // +0x1c
    uint8_t ban_ip_login;          // +0x1d
};

#pragma pack(pop)

static_assert(sizeof(SIG_LOAD_POWER_WAR_STATUE_RANKER) == 0x0cu,
              "Power-war ranker reply ABI changed");
static_assert(sizeof(SIG_LOAD_POWER_WAR_STATUE_INFO) == 0x80u,
              "Power-war statue reply ABI changed");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, job) == 0x22u &&
                  offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, guild_name) ==
                      0x25u &&
                  offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, guild_id) ==
                      0x50u &&
                  offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, equip_item_id) ==
                      0x54u,
              "Power-war statue fields moved");
static_assert(offsetof(SIG_NOTICE_POWER_WAR_RANK, charac_no) == 0x0au,
              "Power-war rank notice must start at +0x0a");
static_assert(offsetof(SIG_POWER_WAR_PROCESS_INFO, charac_no) == 0x0eu &&
                  offsetof(SIG_POWER_WAR_PROCESS_INFO, process_value) ==
                      0x12u,
              "Power-war process reply fields moved");
static_assert(offsetof(SIG_POWER_WAR_END_INFO, charac_no) == 0x0eu &&
                  offsetof(SIG_POWER_WAR_END_INFO, result) == 0x12u &&
                  offsetof(SIG_POWER_WAR_END_INFO, end_value) == 0x13u,
              "Power-war end reply fields moved");
static_assert(offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, source_channel) ==
                  0x0au &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, kind) == 0x0cu &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, message_len) ==
                      0x0du &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, message) == 0x11u &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, charac_name_len) ==
                      0x43u &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, charac_name) ==
                      0x47u &&
                  offsetof(SIG_NOTICE_SERVER_MESSAGE_INFO, update_time) ==
                      0x5bu,
              "Server-message notice fields moved");
static_assert(offsetof(SIG_NOTICE_CREATE_GUILD_AGIT, charac_no) == 0x12u &&
                  offsetof(SIG_NOTICE_DELETE_GUILD_AGIT, error) == 0x16u &&
                  offsetof(SIG_NOTICE_LOAD_GUILD_AGIT, guild_agit_flag) ==
                      0x16u,
              "Guild-agit notice fields moved");
static_assert(offsetof(SIG_NOTICE_GUILD_MEMO, charac_no) == 0x0eu &&
                  offsetof(SIG_NOTICE_GUILD_MEMO, guild_memo) == 0x12u &&
                  offsetof(SIG_NOTICE_GUILD_MEMO, guild_notice) == 0x30u,
              "Guild memo notice fields moved");
static_assert(offsetof(SIG_PCROOM_RESPONSE, pc_room_no) == 0x12u &&
                  offsetof(SIG_PCROOM_RESPONSE, premium_end_time) == 0x16u &&
                  offsetof(SIG_PCROOM_RESPONSE, ip_restriction_type) ==
                      0x1au &&
                  offsetof(SIG_PCROOM_RESPONSE, ban_ip_login) == 0x1du,
              "PC-room response fields moved");

class Inter_LoadPowerWarStatueRanker : public ISigDispatcher {
public:
    Inter_LoadPowerWarStatueRanker();
    virtual ~Inter_LoadPowerWarStatueRanker();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadPowerWarStatueInfo : public ISigDispatcher {
public:
    Inter_LoadPowerWarStatueInfo();
    virtual ~Inter_LoadPowerWarStatueInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_UpdateServerMessage : public ISigDispatcher {
public:
    Inter_UpdateServerMessage();
    virtual ~Inter_UpdateServerMessage();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeServerMessageInfo : public ISigDispatcher {
public:
    Inter_NoticeServerMessageInfo();
    virtual ~Inter_NoticeServerMessageInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeCreateGuildAgit : public ISigDispatcher {
public:
    Inter_NoticeCreateGuildAgit();
    virtual ~Inter_NoticeCreateGuildAgit();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeDeleteGuildAgit : public ISigDispatcher {
public:
    Inter_NoticeDeleteGuildAgit();
    virtual ~Inter_NoticeDeleteGuildAgit();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeLoadGuildAgit : public ISigDispatcher {
public:
    Inter_NoticeLoadGuildAgit();
    virtual ~Inter_NoticeLoadGuildAgit();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticePowerWarRank : public ISigDispatcher {
public:
    Inter_NoticePowerWarRank();
    virtual ~Inter_NoticePowerWarRank();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_NoticeGuildMemo : public ISigDispatcher {
public:
    Inter_NoticeGuildMemo();
    virtual ~Inter_NoticeGuildMemo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PowerWarProcessInfo : public ISigDispatcher {
public:
    Inter_PowerWarProcessInfo();
    virtual ~Inter_PowerWarProcessInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PowerWarEndInfo : public ISigDispatcher {
public:
    Inter_PowerWarEndInfo();
    virtual ~Inter_PowerWarEndInfo();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_PcroomResponse : public ISigDispatcher {
public:
    Inter_PcroomResponse();
    virtual ~Inter_PcroomResponse();

    // IDA_EA: 0x84DB40E
    // IDA_NAME: _ZN20Inter_PcroomResponse22IsShutdonTimeOverLoginEv
    // DEMANGLED: Inter_PcroomResponse::IsShutdonTimeOverLogin(void)
    bool IsShutdonTimeOverLogin();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
// The original C2 receivers are vptr-only objects.  The dispatcher stores and
// deletes them through ISigDispatcher, so their i386 object size is ABI data.
static_assert(sizeof(Inter_LoadPowerWarStatueRanker) == 0x04u,
              "Inter_LoadPowerWarStatueRanker must remain vptr-only on i386");
static_assert(sizeof(Inter_LoadPowerWarStatueInfo) == 0x04u,
              "Inter_LoadPowerWarStatueInfo must remain vptr-only on i386");
static_assert(sizeof(Inter_UpdateServerMessage) == 0x04u,
              "Inter_UpdateServerMessage must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticeServerMessageInfo) == 0x04u,
              "Inter_NoticeServerMessageInfo must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticeCreateGuildAgit) == 0x04u,
              "Inter_NoticeCreateGuildAgit must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticeDeleteGuildAgit) == 0x04u,
              "Inter_NoticeDeleteGuildAgit must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticeLoadGuildAgit) == 0x04u,
              "Inter_NoticeLoadGuildAgit must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticePowerWarRank) == 0x04u,
              "Inter_NoticePowerWarRank must remain vptr-only on i386");
static_assert(sizeof(Inter_NoticeGuildMemo) == 0x04u,
              "Inter_NoticeGuildMemo must remain vptr-only on i386");
static_assert(sizeof(Inter_PowerWarProcessInfo) == 0x04u,
              "Inter_PowerWarProcessInfo must remain vptr-only on i386");
static_assert(sizeof(Inter_PowerWarEndInfo) == 0x04u,
              "Inter_PowerWarEndInfo must remain vptr-only on i386");
static_assert(sizeof(Inter_PcroomResponse) == 0x04u,
              "Inter_PcroomResponse must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_INTER_SLOTS_357_382_H_
