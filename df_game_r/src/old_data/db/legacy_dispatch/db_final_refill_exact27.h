#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_FINAL_REFILL_EXACT27_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_FINAL_REFILL_EXACT27_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/core/db_types.h"
#include "old_game/dungeon/instance/sending_apc_info_runtime.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)

struct SIG_LOAD_DNF_HACK_LOG {
    uint32_t m_id;
    uint32_t counts_by_hack_type[3900];
};

struct SIG_GOBLIN_UPDATE_VALIDITYTIME {
    uint32_t m_id;
    uint32_t validity_seconds;
};

// IDA_EA: DB_BlackIPMonitor::dispatch @0x8432AD2 and
// DB_BlackIPMonitorPartLoad::dispatch @0x84330D4 fill one 0x20-byte entry
// per ip_monitor_punish row.  Inter_BlackIPMonitor::dispatch_sig @0x84D8650
// consumes the same offsets (+0x04 ip through +0x20 end_time).
struct SIG_IP_MONITOR_PUNISH_ENTRY {
    char ip[16];
    int32_t punish_type;
    uint32_t member_id_count;
    uint32_t start_time;
    uint32_t end_time;
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_IP_MONITOR_PUNISH> @0x8453048.
// The initial-load byte lives at +0xC84; the remaining three bytes are the
// old four-byte tail alignment and are not initialized by either producer.
struct SIG_IP_MONITOR_PUNISH {
    uint32_t count;
    SIG_IP_MONITOR_PUNISH_ENTRY entries[100];
    uint8_t initial_load;
    uint8_t raw_0c85[3];
};

struct SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC {
    uint32_t channel_no;
    uint32_t grade_count[7];
};

struct SIG_DOUBLE_CHECK_GUILD_NAME {
    char guild_name[23];
};

struct SIG_DOUBLE_CHECK_GUILD_ADDRESS {
    char guild_address[13];
};

struct SIG_CHECK_JOIN_GUILD {
    char guild_name[24];
    int32_t server_id;
};

struct SIG_CHECK_JOIN_GUILD_DB_REPLY {
    int32_t result;
    char master_name[30];
    char guild_name[23];
    uint8_t align_after_guild_name_0;
    uint8_t align_after_guild_name_1;
    uint8_t align_after_guild_name_2;
    int32_t guild_level;
    int32_t member_count;
};

// IDA_EA: DB_RequestJoinGuild::makeRequest @0x843D19A copies exactly this
// 0x44-byte record to the DB queue.  The dispatcher clears all bytes, writes
// at most 22 guild-name bytes, then overlays character/account/server fields,
// two SSN bytes, and at most 25 memo bytes.
struct SIG_REQUEST_JOIN_GUILD {
    char guild_name[24];         // +0x00
    uint32_t charac_no;          // +0x18
    uint32_t m_id;               // +0x1c
    int32_t server_group;        // +0x20
    char born_year[3];           // +0x24; old code copies two SSN bytes
    char memo[29];               // +0x27
};

// IDA_EA: DB_RequestJoinGuild::dispatch @0x843CCF2 and
// Inter_RequestJoinGuild::dispatch_sig @0x84E3924.  Stream specialization
// 0x8453AFA reserves this exact 0x48-byte reply body.
struct SIG_REQUEST_JOIN_GUILD_DB_REPLY {
    int32_t result;              // +0x00
    uint32_t guild_id;           // +0x04
    char master_name[30];        // +0x08
    char guild_name[23];         // +0x26
    uint8_t raw_3d[3];           // +0x3d
    int32_t guild_level;         // +0x40
    int32_t member_count;        // +0x44
};

// IDA_EA: DB_JoinGuildInfo::dispatch @0x843D286 and
// Inter_JoinGuildInfo::dispatch_sig @0x84E3ACE.
struct SIG_JOIN_GUILD_INFO {
    uint8_t found;
    uint8_t raw_01[3];
    uint32_t guild_id;
    char master_name[30];
    char guild_name[23];
    uint8_t raw_3d[3];
    int32_t guild_level;
    int32_t member_count;
    char memo[26];
    uint8_t raw_62[2];
};

// IDA_EA: DB_GuildJoinList::dispatch @0x843D87C and
// Inter_GuildJoinList::dispatch_sig @0x84E3C60.
struct SIG_GUILD_JOIN_LIST_ENTRY {
    uint32_t charac_no;
    char charac_name[30];
    char job;
    char grow_type;
    char sex;
    char level;
    char memo[26];
    int32_t elapsed_seconds;
};

struct SIG_GUILD_JOIN_LIST {
    uint32_t count;
    SIG_GUILD_JOIN_LIST_ENTRY entries[50];
};

struct InformNoticeFlagData {
    uint32_t m_id;
    char inform_notice[16];
};

// DB_TOD_Reset_UserAPC writes this ten-entry reply into a dispatch-queue
// Stream.  Each entry retains the original 0xBC-byte DB/Inter stride.
struct SIG_TOD_RELOADED_USER_APC {
    _APCInfo entries[10];
};

// IDA_EA: DB_LoadPowerWarStatueInfo::dispatch @0x8434886 constructs this
// packed 0x80-byte database reply.  The counterpart inter dispatcher reads
// every listed field by its old byte offset; no unmodelled payload tail is
// present.
struct SIG_LOAD_POWER_WAR_STATUE_INFO {
    uint32_t charac_no;                 // +0x00
    char charac_name[30];               // +0x04
    uint8_t level;                      // +0x22
    uint8_t job;                        // +0x23
    uint8_t grow_type;                  // +0x24
    char guild_name[40];                // +0x25
    uint8_t raw_4d[3];                  // +0x4D
    uint32_t guild_id;                  // +0x50
    uint32_t equipment_item_ids[11];    // +0x54
};

#pragma pack(pop)

static_assert(sizeof(SIG_LOAD_DNF_HACK_LOG) == 0x3CF4u,
              "SIG_LOAD_DNF_HACK_LOG must match IDA");
static_assert(sizeof(SIG_GOBLIN_UPDATE_VALIDITYTIME) == 0x08u,
              "SIG_GOBLIN_UPDATE_VALIDITYTIME must match IDA");
static_assert(sizeof(SIG_IP_MONITOR_PUNISH_ENTRY) == 0x20u,
              "SIG_IP_MONITOR_PUNISH_ENTRY must match IDA");
static_assert(sizeof(SIG_IP_MONITOR_PUNISH) == 0xC88u,
              "SIG_IP_MONITOR_PUNISH must match IDA");
static_assert(offsetof(SIG_IP_MONITOR_PUNISH, entries) == 0x04u,
              "SIG_IP_MONITOR_PUNISH entries must start at +0x04");
static_assert(offsetof(SIG_IP_MONITOR_PUNISH, initial_load) == 0xC84u,
              "SIG_IP_MONITOR_PUNISH initial_load must stay at +0xC84");
static_assert(sizeof(SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC) == 0x20u,
              "SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC must match IDA");
static_assert(sizeof(SIG_DOUBLE_CHECK_GUILD_NAME) == 0x17u,
              "SIG_DOUBLE_CHECK_GUILD_NAME must match IDA");
static_assert(sizeof(SIG_DOUBLE_CHECK_GUILD_ADDRESS) == 0x0Du,
              "SIG_DOUBLE_CHECK_GUILD_ADDRESS must match IDA");
static_assert(sizeof(SIG_CHECK_JOIN_GUILD) == 0x1Cu,
              "SIG_CHECK_JOIN_GUILD must match IDA");
static_assert(sizeof(SIG_CHECK_JOIN_GUILD_DB_REPLY) == 0x44u,
              "SIG_CHECK_JOIN_GUILD_DB_REPLY must match IDA");
static_assert(sizeof(SIG_REQUEST_JOIN_GUILD) == 0x44u,
              "SIG_REQUEST_JOIN_GUILD must match IDA");
static_assert(offsetof(SIG_REQUEST_JOIN_GUILD, charac_no) == 0x18u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD, m_id) == 0x1cu &&
                  offsetof(SIG_REQUEST_JOIN_GUILD, server_group) == 0x20u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD, born_year) == 0x24u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD, memo) == 0x27u,
              "SIG_REQUEST_JOIN_GUILD offsets must match IDA");
static_assert(sizeof(SIG_REQUEST_JOIN_GUILD_DB_REPLY) == 0x48u,
              "SIG_REQUEST_JOIN_GUILD_DB_REPLY must match IDA");
static_assert(offsetof(SIG_REQUEST_JOIN_GUILD_DB_REPLY, guild_id) == 0x04u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD_DB_REPLY, master_name) ==
                      0x08u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD_DB_REPLY, guild_name) ==
                      0x26u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD_DB_REPLY, guild_level) ==
                      0x40u &&
                  offsetof(SIG_REQUEST_JOIN_GUILD_DB_REPLY, member_count) ==
                      0x44u,
              "SIG_REQUEST_JOIN_GUILD_DB_REPLY offsets must match IDA");
static_assert(sizeof(SIG_JOIN_GUILD_INFO) == 0x64u,
              "SIG_JOIN_GUILD_INFO must match IDA");
static_assert(offsetof(SIG_JOIN_GUILD_INFO, guild_id) == 0x04u,
              "SIG_JOIN_GUILD_INFO guild_id must stay at +0x04");
static_assert(offsetof(SIG_JOIN_GUILD_INFO, master_name) == 0x08u,
              "SIG_JOIN_GUILD_INFO master_name must stay at +0x08");
static_assert(offsetof(SIG_JOIN_GUILD_INFO, guild_name) == 0x26u,
              "SIG_JOIN_GUILD_INFO guild_name must stay at +0x26");
static_assert(offsetof(SIG_JOIN_GUILD_INFO, guild_level) == 0x40u,
              "SIG_JOIN_GUILD_INFO guild_level must stay at +0x40");
static_assert(offsetof(SIG_JOIN_GUILD_INFO, memo) == 0x48u,
              "SIG_JOIN_GUILD_INFO memo must stay at +0x48");
static_assert(sizeof(SIG_GUILD_JOIN_LIST_ENTRY) == 0x44u,
              "SIG_GUILD_JOIN_LIST_ENTRY must match IDA");
static_assert(sizeof(SIG_GUILD_JOIN_LIST) == 0xD4Cu,
              "SIG_GUILD_JOIN_LIST must match IDA");
static_assert(offsetof(SIG_GUILD_JOIN_LIST, entries) == 0x04u,
              "SIG_GUILD_JOIN_LIST entries must start at +0x04");
static_assert(offsetof(SIG_GUILD_JOIN_LIST_ENTRY, job) == 0x22u,
              "SIG_GUILD_JOIN_LIST_ENTRY job must stay at +0x22");
static_assert(offsetof(SIG_GUILD_JOIN_LIST_ENTRY, memo) == 0x26u,
              "SIG_GUILD_JOIN_LIST_ENTRY memo must stay at +0x26");
static_assert(offsetof(SIG_GUILD_JOIN_LIST_ENTRY, elapsed_seconds) == 0x40u,
              "SIG_GUILD_JOIN_LIST_ENTRY elapsed_seconds must stay at +0x40");
static_assert(sizeof(InformNoticeFlagData) == 0x14u,
              "InformNoticeFlagData must match IDA");
static_assert(sizeof(SIG_TOD_RELOADED_USER_APC) == 0x758u,
              "SIG_TOD_RELOADED_USER_APC must match the original Stream ABI");
static_assert(offsetof(SIG_TOD_RELOADED_USER_APC, entries) == 0x00u,
              "TOD reply entries must begin at offset zero");
static_assert(sizeof(SIG_LOAD_POWER_WAR_STATUE_INFO) == 0x80u,
              "SIG_LOAD_POWER_WAR_STATUE_INFO must match the old Stream ABI");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, charac_name) == 0x04u,
              "PowerWar charac_name must remain at +0x04");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, level) == 0x22u,
              "PowerWar level must remain at +0x22");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, guild_name) == 0x25u,
              "PowerWar guild_name must remain at +0x25");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO, guild_id) == 0x50u,
              "PowerWar guild_id must remain at +0x50");
static_assert(offsetof(SIG_LOAD_POWER_WAR_STATUE_INFO,
                       equipment_item_ids) == 0x54u,
              "PowerWar equipment ids must remain at +0x54");

class DB_BlackIPMonitor : public DBDispatcher {
public:
    DB_BlackIPMonitor();
    virtual ~DB_BlackIPMonitor();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

class DB_BlackIPMonitorPartLoad : public DBDispatcher {
public:
    DB_BlackIPMonitorPartLoad();
    virtual ~DB_BlackIPMonitorPartLoad();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest();
};

class DB_BlackipPrivateSecurityValuation : public DBDispatcher {
public:
    DB_BlackipPrivateSecurityValuation();
    virtual ~DB_BlackipPrivateSecurityValuation();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int m_id);
};

class DB_CancelJoinGuild : public DBDispatcher {
public:
    DB_CancelJoinGuild();
    virtual ~DB_CancelJoinGuild();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int guild_id,
                            unsigned int charac_no);
};

class DB_CheckJoinGuild : public DBDispatcher {
public:
    DB_CheckJoinGuild();
    virtual ~DB_CheckJoinGuild();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, SIG_CHECK_JOIN_GUILD* request);
};

class DB_DenyJoinGuild : public DBDispatcher {
public:
    DB_DenyJoinGuild();
    virtual ~DB_DenyJoinGuild();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int guild_id,
                            unsigned int charac_no);
};

class DB_RequestJoinGuild : public DBDispatcher {
public:
    DB_RequestJoinGuild();
    virtual ~DB_RequestJoinGuild();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, SIG_REQUEST_JOIN_GUILD* request);
};

class DB_GoblinPadUpdateValidityTime : public DBDispatcher {
public:
    DB_GoblinPadUpdateValidityTime();
    virtual ~DB_GoblinPadUpdateValidityTime();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int m_id,
                            unsigned int validity_seconds);
};

class DB_GuildJoinList : public DBDispatcher {
public:
    DB_GuildJoinList();
    virtual ~DB_GuildJoinList();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int guild_id,
                            ENUM_SERVER_GROUP server_group);
};

class DB_InsertPoliceSaveChatting : public DBDispatcher {
public:
    DB_InsertPoliceSaveChatting();
    virtual ~DB_InsertPoliceSaveChatting();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int charac_no, char message_type,
                            char* message);
};

class DB_JoinGuildInfo : public DBDispatcher {
public:
    DB_JoinGuildInfo();
    virtual ~DB_JoinGuildInfo();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, ENUM_SERVER_GROUP server_group,
                            unsigned int charac_no);
};

class DB_LoadDnfHackLog : public DBDispatcher {
public:
    DB_LoadDnfHackLog();
    virtual ~DB_LoadDnfHackLog();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, unsigned int m_id);
};

class DB_TOD_Reset_UserAPC : public DBDispatcher {
public:
    // IDA_EA: 0x844F786
    // IDA_NAME: _ZN20DB_TOD_Reset_UserAPCC2Ev
    // DEMANGLED: DB_TOD_Reset_UserAPC::DB_TOD_Reset_UserAPC(void)
    DB_TOD_Reset_UserAPC();

    // IDA_EA: 0x845BCC2 (D2), 0x845BCF2 (D0)
    // IDA_NAME: _ZN20DB_TOD_Reset_UserAPCD2Ev,
    //           _ZN20DB_TOD_Reset_UserAPCD0Ev
    // DEMANGLED: DB_TOD_Reset_UserAPC::~DB_TOD_Reset_UserAPC()
    virtual ~DB_TOD_Reset_UserAPC();

    // IDA_EA: 0x84400C8
    // IDA_NAME: _ZN20DB_TOD_Reset_UserAPC8dispatchEiiP6Stream
    // DEMANGLED: DB_TOD_Reset_UserAPC::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x8440548
    // IDA_NAME: _ZN20DB_TOD_Reset_UserAPC14GetAPCUserInfoER8_APCInfo
    // DEMANGLED: DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo &)
    bool GetAPCUserInfo(_APCInfo& apc_info);
};

class DB_LoadPowerWarStatueInfo : public DBDispatcher {
public:
    // IDA_EA: 0x844EEE2
    // IDA_NAME: _ZN25DB_LoadPowerWarStatueInfoC2Ev
    // DEMANGLED: DB_LoadPowerWarStatueInfo::DB_LoadPowerWarStatueInfo(void)
    DB_LoadPowerWarStatueInfo();

    // IDA_EA: 0x845D5BE (D2), 0x845D5EE (D0)
    // IDA_NAME: _ZN25DB_LoadPowerWarStatueInfoD2Ev,
    //           _ZN25DB_LoadPowerWarStatueInfoD0Ev
    // DEMANGLED: DB_LoadPowerWarStatueInfo::~DB_LoadPowerWarStatueInfo()
    virtual ~DB_LoadPowerWarStatueInfo();

    // IDA_EA: 0x8434886
    // IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo8dispatchEiiP6Stream
    // DEMANGLED: DB_LoadPowerWarStatueInfo::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x8434A8A
    // IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo13GetCharacInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFOj
    // DEMANGLED: DB_LoadPowerWarStatueInfo::GetCharacInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *,unsigned int)
    bool GetCharacInfo(SIG_LOAD_POWER_WAR_STATUE_INFO* payload,
                       unsigned int charac_no);

    // IDA_EA: 0x8434D36
    // IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo13GetAvatarInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFOj
    // DEMANGLED: DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *,unsigned int)
    bool GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO* payload,
                       unsigned int charac_no);
};

class DB_UpdateCompoundEmblemStatics : public DBDispatcher {
public:
    DB_UpdateCompoundEmblemStatics();
    virtual ~DB_UpdateCompoundEmblemStatics();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int channel_no, unsigned int grade0,
                            unsigned int grade1, unsigned int grade2,
                            unsigned int grade3, unsigned int grade4,
                            unsigned int grade5, unsigned int grade6);
};

class DB_CheckDoubleGuildName : public DBDispatcher {
public:
    // IDA_EA: 0x844E86E
    // IDA_NAME: _ZN23DB_CheckDoubleGuildNameC2Ev
    // DEMANGLED: DB_CheckDoubleGuildName::DB_CheckDoubleGuildName(void)
    DB_CheckDoubleGuildName();

    // IDA_EA: 0x845E548
    // IDA_NAME: _ZN23DB_CheckDoubleGuildNameD2Ev
    // DEMANGLED: DB_CheckDoubleGuildName::~DB_CheckDoubleGuildName()
    // IDA_EA: 0x845E578
    // IDA_NAME: _ZN23DB_CheckDoubleGuildNameD0Ev
    // DEMANGLED: DB_CheckDoubleGuildName::~DB_CheckDoubleGuildName()
    virtual ~DB_CheckDoubleGuildName();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_CheckDoubleGuildAddress : public DBDispatcher {
public:
    // IDA_EA: 0x844E88A
    // IDA_NAME: _ZN26DB_CheckDoubleGuildAddressC2Ev
    // DEMANGLED: DB_CheckDoubleGuildAddress::DB_CheckDoubleGuildAddress(void)
    DB_CheckDoubleGuildAddress();

    // IDA_EA: 0x845E4FA
    // IDA_NAME: _ZN26DB_CheckDoubleGuildAddressD2Ev
    // DEMANGLED: DB_CheckDoubleGuildAddress::~DB_CheckDoubleGuildAddress()
    // IDA_EA: 0x845E52A
    // IDA_NAME: _ZN26DB_CheckDoubleGuildAddressD0Ev
    // DEMANGLED: DB_CheckDoubleGuildAddress::~DB_CheckDoubleGuildAddress()
    virtual ~DB_CheckDoubleGuildAddress();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_DeleteGuild : public DBDispatcher {
public:
    // IDA_EA: 0x844F29A
    // IDA_NAME: _ZN14DB_DeleteGuildC2Ev
    // DEMANGLED: DB_DeleteGuild::DB_DeleteGuild(void)
    DB_DeleteGuild();

    // IDA_EA: 0x845C8F2
    // IDA_NAME: _ZN14DB_DeleteGuildD2Ev
    // IDA_EA: 0x845C922
    // IDA_NAME: _ZN14DB_DeleteGuildD0Ev
    // DEMANGLED: DB_DeleteGuild::~DB_DeleteGuild()
    virtual ~DB_DeleteGuild();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_DeletePowerWarStatueMessage : public DBDispatcher {
public:
    // IDA_EA: 0x844F0F6
    // IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageC2Ev
    // DEMANGLED: DB_DeletePowerWarStatueMessage::DB_DeletePowerWarStatueMessage(void)
    DB_DeletePowerWarStatueMessage();

    // IDA_EA: 0x845CFF4
    // IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageD2Ev
    // IDA_EA: 0x845D024
    // IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageD0Ev
    // DEMANGLED: DB_DeletePowerWarStatueMessage::~DB_DeletePowerWarStatueMessage()
    virtual ~DB_DeletePowerWarStatueMessage();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

class DB_SaveInformNotice : public DBDispatcher {
public:
    // IDA_EA: 0x844FC72
    // IDA_NAME: _ZN19DB_SaveInformNoticeC2Ev
    // DEMANGLED: DB_SaveInformNotice::DB_SaveInformNotice(void)
    DB_SaveInformNotice();

    // IDA_EA: 0x845B1CA
    // IDA_NAME: _ZN19DB_SaveInformNoticeD2Ev
    // IDA_EA: 0x845B1FA
    // IDA_NAME: _ZN19DB_SaveInformNoticeD0Ev
    // DEMANGLED: DB_SaveInformNotice::~DB_SaveInformNotice()
    virtual ~DB_SaveInformNotice();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_FINAL_REFILL_EXACT27_H_
