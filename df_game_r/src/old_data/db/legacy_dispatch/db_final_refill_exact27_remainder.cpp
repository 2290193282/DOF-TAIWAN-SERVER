// Canonical B provider snapshot excluding game-option dispatches migrated to
// src/old_data/db/game_option/.  The user-modified legacy source remains
// retained and unbuilt.

#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <cstdio>
#include <ctime>

namespace {

enum {
    kTODAPCCount = 10,
    kTODUserItemsLimit = 115,
    kTODUserItemTripletCount = kTODUserItemsLimit * 3,
    kDecodedEquipmentByteLimit = 0x37a,
    kDecodedEquipmentTailCount =
        (kDecodedEquipmentByteLimit + sizeof(Inven_Item) - 1) /
        sizeof(Inven_Item),
    kTODDecodedEquipmentOriginIndex = kTODLegacyWeaponSlot,
    kTODDecodedEquipmentScratchCount =
        kTODDecodedEquipmentOriginIndex + kDecodedEquipmentTailCount,
    kTODEquipSlotCompressedBytes = kDecodedEquipmentByteLimit,
    kTODRawEquipmentSlot = kTODLegacyWeaponSlot,

    kPowerWarEquipmentBlobBytes = kDecodedEquipmentByteLimit,
    kPowerWarUserItemsLimit = oldsrv::config::kAvatarDetailItemCount,
    kPowerWarUserItemTripletCount = kPowerWarUserItemsLimit * 3,
    kPowerWarDecodedEquipmentOriginIndex = 10,
    kPowerWarDecodedEquipmentScratchCount =
        kPowerWarDecodedEquipmentOriginIndex + kDecodedEquipmentTailCount,
    kPowerWarDecodedEquipmentSlot = 10,
};

static_assert(kTODAPCCount ==
                  sizeof(((SIG_TOD_RELOADED_USER_APC*)0)->entries) /
                      sizeof(_APCInfo),
              "TOD row loop and reply payload must have the same count");
static_assert(kTODRawEquipmentSlot <
                  kTODLegacyEquipmentTypeCount,
              "TOD weapon target must remain within _APCInfo");
static_assert(kPowerWarDecodedEquipmentSlot < 11,
              "old PowerWar payload has eleven equipment dwords");
static_assert(sizeof(Inven_Item) == 0x3d,
              "PowerWar decoded inventory element size must match old ABI");
static_assert(kTODDecodedEquipmentScratchCount == 25,
              "TOD scratch must start decoding at legacy weapon slot 10");
static_assert(kPowerWarDecodedEquipmentScratchCount == 25,
              "PowerWar scratch must retain its independent slot-10 origin");
static_assert(kDecodedEquipmentTailCount * sizeof(Inven_Item) >=
                  kDecodedEquipmentByteLimit,
              "typed equipment scratch tail must cover old decode capacity");

}  // namespace

// IDA_EA: 0x844F53A
// IDA_NAME: _ZN19DB_RequestJoinGuildC2Ev
// DEMANGLED: DB_RequestJoinGuild::DB_RequestJoinGuild(void)
DB_RequestJoinGuild::DB_RequestJoinGuild() : DBDispatcher() {}

// IDA_EA: 0x845C28C
// IDA_NAME: _ZN19DB_RequestJoinGuildD2Ev
// IDA_EA: 0x845C2BC
// IDA_NAME: _ZN19DB_RequestJoinGuildD0Ev
// DEMANGLED: DB_RequestJoinGuild::~DB_RequestJoinGuild()
// The compiler emits the deleting D0 wrapper around this real D2 body.
DB_RequestJoinGuild::~DB_RequestJoinGuild() {}

// IDA_EA: 0x843CCF2
// IDA_NAME: _ZN19DB_RequestJoinGuild8dispatchEiiP6Stream
// DEMANGLED: DB_RequestJoinGuild::dispatch(int,int,Stream *)
bool DB_RequestJoinGuild::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    Stream* const reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37161);
    CStreamGuard reply(reply_stream, true);
    *reply << 0x217;
    *reply << user_idx;

    SIG_REQUEST_JOIN_GUILD_DB_REPLY* const response =
        reply->GetInBuffer<SIG_REQUEST_JOIN_GUILD_DB_REPLY>();
    SIG_REQUEST_JOIN_GUILD* const request =
        stream->GetOutBuffer<SIG_REQUEST_JOIN_GUILD>();

    db_handle->set_query(
        "seLect * from guild_join_list where charac_no=%d and server_group=%d",
        request->charac_no,
        request->server_group);
    if (!db_handle->exec(true)) {
        return false;
    }
    if (db_handle->get_n_rows() != 0) {
        response->result = 18;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
        // This queued duplicate result intentionally reports failure to the
        // DB-dispatch caller, matching the original tail path.
        return false;
    }

    db_handle->set_query(
        "seLect guild_id,guild_name,master_name,lev,member_count from "
        "guild_info where server_id=%d and expire_flag=0 and guild_name='%s'",
        request->server_group,
        request->guild_name);
    if (!db_handle->exec(true)) {
        return false;
    }
    if (db_handle->get_n_rows() == 0) {
        response->result = 21;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
        return true;
    }
    if (!db_handle->fetch() ||
        !db_handle->get_uint(0, response->guild_id) ||
        !db_handle->get_str(1, response->guild_name, 23) ||
        !db_handle->get_str(2, response->master_name, 30) ||
        !db_handle->get_int(3, response->guild_level) ||
        !db_handle->get_int(4, response->member_count)) {
        return false;
    }

    db_handle->set_query(
        "seLect * from guild_join_list where guild_id=%d",
        response->guild_id);
    if (!db_handle->exec(true)) {
        return false;
    }
    if (static_cast<unsigned int>(db_handle->get_n_rows()) > 0x31u) {
        response->result = 95;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
        return true;
    }

    char escaped_memo[53];
    db_handle->escape_string(escaped_memo, request->memo);
    db_handle->set_query(
        "inSert into guild_join_list(guild_id,m_id,server_group,charac_no,"
        "born_year,memo,occ_time) values(%d,%s,%d,%d,'%s','%s',now())",
        response->guild_id,
        NumberToString(request->m_id, 0),
        request->server_group,
        request->charac_no,
        request->born_year,
        escaped_memo);
    if (!db_handle->exec(true)) {
        return false;
    }

    response->result = db_handle->getAffectedRowCount() != 0 ? 0 : 2;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x844EA82
// IDA_NAME: _ZN17DB_BlackIPMonitorC2Ev
// DEMANGLED: DB_BlackIPMonitor::DB_BlackIPMonitor(void)
DB_BlackIPMonitor::DB_BlackIPMonitor() : DBDispatcher() {}

// IDA_EA: 0x845D82E
// IDA_NAME: _ZN17DB_BlackIPMonitorD2Ev
// DEMANGLED: DB_BlackIPMonitor::~DB_BlackIPMonitor()
// IDA_EA: 0x845D85E
// IDA_NAME: _ZN17DB_BlackIPMonitorD0Ev
// DEMANGLED: DB_BlackIPMonitor::~DB_BlackIPMonitor()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_BlackIPMonitor::~DB_BlackIPMonitor() {}

// IDA_EA: 0x844EABA
// IDA_NAME: _ZN25DB_BlackIPMonitorPartLoadC2Ev
// DEMANGLED: DB_BlackIPMonitorPartLoad::DB_BlackIPMonitorPartLoad(void)
DB_BlackIPMonitorPartLoad::DB_BlackIPMonitorPartLoad()
    : DBDispatcher() {}

// IDA_EA: 0x845D792
// IDA_NAME: _ZN25DB_BlackIPMonitorPartLoadD2Ev
// DEMANGLED: DB_BlackIPMonitorPartLoad::~DB_BlackIPMonitorPartLoad()
// IDA_EA: 0x845D7C2
// IDA_NAME: _ZN25DB_BlackIPMonitorPartLoadD0Ev
// DEMANGLED: DB_BlackIPMonitorPartLoad::~DB_BlackIPMonitorPartLoad()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_BlackIPMonitorPartLoad::~DB_BlackIPMonitorPartLoad() {}

// IDA_EA: 0x844EEFE
// IDA_NAME: _ZN34DB_BlackipPrivateSecurityValuationC2Ev
// DEMANGLED: DB_BlackipPrivateSecurityValuation::DB_BlackipPrivateSecurityValuation(void)
DB_BlackipPrivateSecurityValuation::DB_BlackipPrivateSecurityValuation()
    : DBDispatcher() {}

// IDA_EA: 0x845D570
// IDA_NAME: _ZN34DB_BlackipPrivateSecurityValuationD2Ev
// DEMANGLED: DB_BlackipPrivateSecurityValuation::~DB_BlackipPrivateSecurityValuation()
// IDA_EA: 0x845D5A0
// IDA_NAME: _ZN34DB_BlackipPrivateSecurityValuationD0Ev
// DEMANGLED: DB_BlackipPrivateSecurityValuation::~DB_BlackipPrivateSecurityValuation()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_BlackipPrivateSecurityValuation::~DB_BlackipPrivateSecurityValuation() {}

// IDA_EA: 0x844F572
// IDA_NAME: _ZN18DB_CancelJoinGuildC2Ev
// DEMANGLED: DB_CancelJoinGuild::DB_CancelJoinGuild(void)
DB_CancelJoinGuild::DB_CancelJoinGuild() : DBDispatcher() {}

// IDA_EA: 0x845C1F0
// IDA_NAME: _ZN18DB_CancelJoinGuildD2Ev
// DEMANGLED: DB_CancelJoinGuild::~DB_CancelJoinGuild()
// IDA_EA: 0x845C220
// IDA_NAME: _ZN18DB_CancelJoinGuildD0Ev
// DEMANGLED: DB_CancelJoinGuild::~DB_CancelJoinGuild()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_CancelJoinGuild::~DB_CancelJoinGuild() {}

// IDA_EA: 0x844F51E
// IDA_NAME: _ZN17DB_CheckJoinGuildC2Ev
// DEMANGLED: DB_CheckJoinGuild::DB_CheckJoinGuild(void)
DB_CheckJoinGuild::DB_CheckJoinGuild() : DBDispatcher() {}

// IDA_EA: 0x845C2DA
// IDA_NAME: _ZN17DB_CheckJoinGuildD2Ev
// DEMANGLED: DB_CheckJoinGuild::~DB_CheckJoinGuild()
// IDA_EA: 0x845C30A
// IDA_NAME: _ZN17DB_CheckJoinGuildD0Ev
// DEMANGLED: DB_CheckJoinGuild::~DB_CheckJoinGuild()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_CheckJoinGuild::~DB_CheckJoinGuild() {}

// IDA_EA: 0x844F5AA
// IDA_NAME: _ZN16DB_DenyJoinGuildC2Ev
// DEMANGLED: DB_DenyJoinGuild::DB_DenyJoinGuild(void)
DB_DenyJoinGuild::DB_DenyJoinGuild() : DBDispatcher() {}

// IDA_EA: 0x845C154
// IDA_NAME: _ZN16DB_DenyJoinGuildD2Ev
// DEMANGLED: DB_DenyJoinGuild::~DB_DenyJoinGuild()
// IDA_EA: 0x845C184
// IDA_NAME: _ZN16DB_DenyJoinGuildD0Ev
// DEMANGLED: DB_DenyJoinGuild::~DB_DenyJoinGuild()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_DenyJoinGuild::~DB_DenyJoinGuild() {}

// IDA_EA: 0x844EBD2
// IDA_NAME: _ZN30DB_GoblinPadUpdateValidityTimeC2Ev
// DEMANGLED: DB_GoblinPadUpdateValidityTime::DB_GoblinPadUpdateValidityTime(void)
DB_GoblinPadUpdateValidityTime::DB_GoblinPadUpdateValidityTime()
    : DBDispatcher() {}

// IDA_EA: 0x845DDF8
// IDA_NAME: _ZN30DB_GoblinPadUpdateValidityTimeD2Ev
// DEMANGLED: DB_GoblinPadUpdateValidityTime::~DB_GoblinPadUpdateValidityTime()
// IDA_EA: 0x845DE28
// IDA_NAME: _ZN30DB_GoblinPadUpdateValidityTimeD0Ev
// DEMANGLED: DB_GoblinPadUpdateValidityTime::~DB_GoblinPadUpdateValidityTime()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_GoblinPadUpdateValidityTime::~DB_GoblinPadUpdateValidityTime() {}


// IDA_EA: 0x844F58E
// IDA_NAME: _ZN16DB_GuildJoinListC2Ev
// DEMANGLED: DB_GuildJoinList::DB_GuildJoinList(void)
DB_GuildJoinList::DB_GuildJoinList() : DBDispatcher() {}

// IDA_EA: 0x845C1A2
// IDA_NAME: _ZN16DB_GuildJoinListD2Ev
// DEMANGLED: DB_GuildJoinList::~DB_GuildJoinList()
// IDA_EA: 0x845C1D2
// IDA_NAME: _ZN16DB_GuildJoinListD0Ev
// DEMANGLED: DB_GuildJoinList::~DB_GuildJoinList()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_GuildJoinList::~DB_GuildJoinList() {}

// IDA_EA: 0x844F4E6
// IDA_NAME: _ZN27DB_InsertPoliceSaveChattingC2Ev
// DEMANGLED: DB_InsertPoliceSaveChatting::DB_InsertPoliceSaveChatting(void)
DB_InsertPoliceSaveChatting::DB_InsertPoliceSaveChatting() : DBDispatcher() {}

// IDA_EA: 0x845C376
// IDA_NAME: _ZN27DB_InsertPoliceSaveChattingD2Ev
// DEMANGLED: DB_InsertPoliceSaveChatting::~DB_InsertPoliceSaveChatting()
// IDA_EA: 0x845C3A6
// IDA_NAME: _ZN27DB_InsertPoliceSaveChattingD0Ev
// DEMANGLED: DB_InsertPoliceSaveChatting::~DB_InsertPoliceSaveChatting()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_InsertPoliceSaveChatting::~DB_InsertPoliceSaveChatting() {}

// IDA_EA: 0x844F556
// IDA_NAME: _ZN16DB_JoinGuildInfoC2Ev
// DEMANGLED: DB_JoinGuildInfo::DB_JoinGuildInfo(void)
DB_JoinGuildInfo::DB_JoinGuildInfo() : DBDispatcher() {}

// IDA_EA: 0x845C23E
// IDA_NAME: _ZN16DB_JoinGuildInfoD2Ev
// DEMANGLED: DB_JoinGuildInfo::~DB_JoinGuildInfo()
// IDA_EA: 0x845C26E
// IDA_NAME: _ZN16DB_JoinGuildInfoD0Ev
// DEMANGLED: DB_JoinGuildInfo::~DB_JoinGuildInfo()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_JoinGuildInfo::~DB_JoinGuildInfo() {}

// IDA_EA: 0x844E9DA
// IDA_NAME: _ZN17DB_LoadDnfHackLogC2Ev
// DEMANGLED: DB_LoadDnfHackLog::DB_LoadDnfHackLog(void)
DB_LoadDnfHackLog::DB_LoadDnfHackLog() : DBDispatcher() {}

// IDA_EA: 0x845F178
// IDA_NAME: _ZN17DB_LoadDnfHackLogD2Ev
// DEMANGLED: DB_LoadDnfHackLog::~DB_LoadDnfHackLog()
// IDA_EA: 0x845F1A8
// IDA_NAME: _ZN17DB_LoadDnfHackLogD0Ev
// DEMANGLED: DB_LoadDnfHackLog::~DB_LoadDnfHackLog()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_LoadDnfHackLog::~DB_LoadDnfHackLog() {}

// IDA_EA: 0x844F786
// IDA_NAME: _ZN20DB_TOD_Reset_UserAPCC2Ev
// DEMANGLED: DB_TOD_Reset_UserAPC::DB_TOD_Reset_UserAPC(void)
DB_TOD_Reset_UserAPC::DB_TOD_Reset_UserAPC() : DBDispatcher() {}

// IDA_EA: 0x845BCC2
// IDA_NAME: _ZN20DB_TOD_Reset_UserAPCD2Ev
// DEMANGLED: DB_TOD_Reset_UserAPC::~DB_TOD_Reset_UserAPC()
// IDA_EA: 0x845BCF2
// IDA_NAME: _ZN20DB_TOD_Reset_UserAPCD0Ev
// DEMANGLED: DB_TOD_Reset_UserAPC::~DB_TOD_Reset_UserAPC()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_TOD_Reset_UserAPC::~DB_TOD_Reset_UserAPC() {}

// IDA_EA: 0x844F19E
// IDA_NAME: _ZN30DB_UpdateCompoundEmblemStaticsC2Ev
// DEMANGLED: DB_UpdateCompoundEmblemStatics::DB_UpdateCompoundEmblemStatics(void)
DB_UpdateCompoundEmblemStatics::DB_UpdateCompoundEmblemStatics()
    : DBDispatcher() {}

// IDA_EA: 0x845CBB0
// IDA_NAME: _ZN30DB_UpdateCompoundEmblemStaticsD2Ev
// DEMANGLED: DB_UpdateCompoundEmblemStatics::~DB_UpdateCompoundEmblemStatics()
// IDA_EA: 0x845CBE0
// IDA_NAME: _ZN30DB_UpdateCompoundEmblemStaticsD0Ev
// DEMANGLED: DB_UpdateCompoundEmblemStatics::~DB_UpdateCompoundEmblemStatics()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_UpdateCompoundEmblemStatics::~DB_UpdateCompoundEmblemStatics() {}

// IDA_EA: 0x8432E80
// IDA_NAME: _ZN17DB_BlackIPMonitor11makeRequestEv
// DEMANGLED: DB_BlackIPMonitor::makeRequest(void)
void DB_BlackIPMonitor::makeRequest() {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 31374);
    CStreamGuard guard(stream, true);
    *guard << 0x152;
    *guard << -1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843351E
// IDA_NAME: _ZN25DB_BlackIPMonitorPartLoad11makeRequestEv
// DEMANGLED: DB_BlackIPMonitorPartLoad::makeRequest(void)
void DB_BlackIPMonitorPartLoad::makeRequest() {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 31549);
    CStreamGuard guard(stream, true);
    *guard << 0x155;
    *guard << -1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84355CA
// IDA_NAME: _ZN34DB_BlackipPrivateSecurityValuation11makeRequestEj
// DEMANGLED: DB_BlackipPrivateSecurityValuation::makeRequest(unsigned int)
void DB_BlackipPrivateSecurityValuation::makeRequest(unsigned int m_id) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 32673);
    CStreamGuard guard(stream, true);
    *guard << 0x164;
    *guard << -1;
    *guard << m_id;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843D78C
// IDA_NAME: _ZN18DB_CancelJoinGuild11makeRequestEijj
// DEMANGLED: DB_CancelJoinGuild::makeRequest(int,unsigned int,unsigned int)
void DB_CancelJoinGuild::makeRequest(int user_idx, unsigned int guild_id,
                                     unsigned int charac_no) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37428);
    CStreamGuard guard(stream, true);
    *guard << 0x218;
    *guard << user_idx;
    *guard << guild_id;
    *guard << charac_no;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843CC06
// IDA_NAME: _ZN17DB_CheckJoinGuild11makeRequestEiP20SIG_CHECK_JOIN_GUILD
// DEMANGLED: DB_CheckJoinGuild::makeRequest(int,SIG_CHECK_JOIN_GUILD *)
void DB_CheckJoinGuild::makeRequest(int user_idx,
                                    SIG_CHECK_JOIN_GUILD* request) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37146);
    CStreamGuard guard(stream, true);
    *guard << 0x216;
    *guard << user_idx;
    SIG_CHECK_JOIN_GUILD* destination =
        guard->GetInBuffer<SIG_CHECK_JOIN_GUILD>();
    std::memcpy(destination, request, sizeof(*destination));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843DF0E
// IDA_NAME: _ZN16DB_DenyJoinGuild11makeRequestEijj
// DEMANGLED: DB_DenyJoinGuild::makeRequest(int,unsigned int,unsigned int)
void DB_DenyJoinGuild::makeRequest(int user_idx, unsigned int guild_id,
                                   unsigned int charac_no) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37556);
    CStreamGuard guard(stream, true);
    *guard << 0x21C;
    *guard << user_idx;
    *guard << guild_id;
    *guard << charac_no;

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x842F2AE
// IDA_NAME: _ZN30DB_GoblinPadUpdateValidityTime11makeRequestEjj
// DEMANGLED: DB_GoblinPadUpdateValidityTime::makeRequest(unsigned int,unsigned int)
void DB_GoblinPadUpdateValidityTime::makeRequest(
        unsigned int m_id, unsigned int validity_seconds) {
    if (m_id != 0) {
        Stream* stream =
            GlobalData::s_stream_pool->Acquire("DBThread.cpp", 29779);
        CStreamGuard guard(stream, true);
        *guard << 0x12C;
        *guard << -1;
        SIG_GOBLIN_UPDATE_VALIDITYTIME* request =
            guard->GetInBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>();
        request->m_id = m_id;
        request->validity_seconds = validity_seconds;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }
}

// IDA_EA: 0x843DD7C
// IDA_NAME: _ZN16DB_GuildJoinList11makeRequestEij17ENUM_SERVER_GROUP
// DEMANGLED: DB_GuildJoinList::makeRequest(int,unsigned int,ENUM_SERVER_GROUP)
void DB_GuildJoinList::makeRequest(int user_idx, unsigned int guild_id,
                                   ENUM_SERVER_GROUP server_group) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37529);
    CStreamGuard guard(stream, true);
    *guard << 0x21A;
    *guard << user_idx;
    *guard << guild_id;
    *guard << static_cast<int>(server_group);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x843C6AA
// IDA_NAME: _ZN27DB_InsertPoliceSaveChatting11makeRequestEjcPc
// DEMANGLED: DB_InsertPoliceSaveChatting::makeRequest(unsigned int,char,char *)
void DB_InsertPoliceSaveChatting::makeRequest(unsigned int charac_no,
                                              char message_type,
                                              char* message) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 36940);
    CStreamGuard guard(stream, true);
    *guard << 0x20C;
    *guard << -1;
    *guard << charac_no;
    *guard << message_type;
    *guard << message;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x843D5FA
// IDA_NAME: _ZN16DB_JoinGuildInfo11makeRequestEi17ENUM_SERVER_GROUPj
// DEMANGLED: DB_JoinGuildInfo::makeRequest(int,ENUM_SERVER_GROUP,unsigned int)
void DB_JoinGuildInfo::makeRequest(int user_idx,
                                   ENUM_SERVER_GROUP server_group,
                                   unsigned int charac_no) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37400);
    CStreamGuard guard(stream, true);
    *guard << 0x219;
    *guard << user_idx;
    *guard << static_cast<int>(server_group);
    *guard << charac_no;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x84224CE
// IDA_NAME: _ZN17DB_LoadDnfHackLog11makeRequestEij
// DEMANGLED: DB_LoadDnfHackLog::makeRequest(int,unsigned int)
void DB_LoadDnfHackLog::makeRequest(int user_idx, unsigned int m_id) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 23231);
    CStreamGuard guard(stream, true);
    *guard << 0x106;
    *guard << user_idx;
    SIG_LOAD_DNF_HACK_LOG* request =
        guard->GetInBuffer<SIG_LOAD_DNF_HACK_LOG>();
    request->m_id = m_id;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x8439DEC
// IDA_NAME: _ZN30DB_UpdateCompoundEmblemStatics11makeRequestEijjjjjjj
// DEMANGLED: DB_UpdateCompoundEmblemStatics::makeRequest(int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int)
void DB_UpdateCompoundEmblemStatics::makeRequest(
        int channel_no, unsigned int grade0, unsigned int grade1,
        unsigned int grade2, unsigned int grade3, unsigned int grade4,
        unsigned int grade5, unsigned int grade6) {
    Stream* stream = GlobalData::s_stream_pool->Acquire("DBThread.cpp", 34812);
    CStreamGuard guard(stream, true);
    *guard << 0x1C4;
    *guard << -1;
    SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC* request =
        guard->GetInBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>();
    request->channel_no = static_cast<uint32_t>(channel_no);
    request->grade_count[0] = grade0;
    request->grade_count[1] = grade1;
    request->grade_count[2] = grade2;
    request->grade_count[3] = grade3;
    request->grade_count[4] = grade4;
    request->grade_count[5] = grade5;
    request->grade_count[6] = grade6;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x8432AD2
// IDA_NAME: _ZN17DB_BlackIPMonitor8dispatchEiiP6Stream
// DEMANGLED: DB_BlackIPMonitor::dispatch(int,int,Stream *)
bool DB_BlackIPMonitor::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    db_handle->set_query(
        "seLect ip, type, m_id_cnt, unix_timestamp(start_time), "
        "unix_timestamp(end_time) from ip_monitor_punish ORDER BY "
        "start_time ASC");
    if (!db_handle->exec(true)) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    cMyTrace trace("virtual bool DB_BlackIPMonitor::dispatch(int, int, Stream*)",
                   31235, 0);
    trace("BLACKIP GET DB ROWS COUNT : %d", row_count);

    SIG_IP_MONITOR_PUNISH_ENTRY entries[100];
    uint32_t entry_count = 0;
    bool first_batch = true;
    for (int row = 0; row < row_count; ++row) {
        if (!db_handle->fetch()) {
            return false;
        }
        if (entry_count == 0) {
            std::memset(entries, 0, sizeof(entries));
        }

        SIG_IP_MONITOR_PUNISH_ENTRY& entry = entries[entry_count];
        if (!db_handle->get_str(0, entry.ip, 16) ||
            !db_handle->get_int(1, entry.punish_type) ||
            !db_handle->get_uint(2, entry.member_id_count) ||
            !db_handle->get_uint(3, entry.start_time) ||
            !db_handle->get_uint(4, entry.end_time)) {
            return false;
        }

        // The original tests the zero-based in-batch index against 0x62,
        // yielding packets of at most 100 records.
        if (entry_count >= 99u || row >= row_count - 1) {
            Stream* reply_stream =
                GlobalData::s_stream_pool->Acquire("DBThread.cpp", 31272);
            CStreamGuard reply(reply_stream, true);
            // Raw instruction at 0x843342E passes the full DB signal 0x156;
            // Hex-Rays' decimal 86 is a narrowed-literal recovery artifact.
            *reply << 0x156;
            *reply << -1;
            SIG_IP_MONITOR_PUNISH* payload =
                reply->GetInBuffer<SIG_IP_MONITOR_PUNISH>();

            payload->count = entry_count + 1;
            std::memcpy(payload->entries, entries, sizeof(entries));
            payload->initial_load = first_batch ? 1 : 0;
            first_batch = false;
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
            entry_count = 0;
        } else {
            ++entry_count;
        }
    }
    return true;
}

// IDA_EA: 0x84330D4
// IDA_NAME: _ZN25DB_BlackIPMonitorPartLoad8dispatchEiiP6Stream
// DEMANGLED: DB_BlackIPMonitorPartLoad::dispatch(int,int,Stream *)
bool DB_BlackIPMonitorPartLoad::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    std::tm* local = std::localtime(&GlobalData::g_IPMonitorLastDBAccessTime);
    char timestamp[32];
    std::sprintf(timestamp, "%d-%d-%d %d:%d:%d",
                 local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                 local->tm_hour, local->tm_min, local->tm_sec);
    db_handle->set_query(
        "seLect ip, type, m_id_cnt, unix_timestamp(start_time), "
        "unix_timestamp(end_time) from ip_monitor_punish where "
        "start_time > '%s'",
        timestamp);
    if (!db_handle->exec(true) || db_handle->get_n_rows() == 0) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    cMyTrace trace(
        "virtual bool DB_BlackIPMonitorPartLoad::dispatch(int, int, Stream*)",
        31428,
        0);
    trace("BLACKIP GET DB ROWS COUNT : %d", row_count);

    SIG_IP_MONITOR_PUNISH_ENTRY entries[100];
    uint32_t entry_count = 0;
    for (int row = 0; row < row_count; ++row) {
        if (!db_handle->fetch()) {
            return false;
        }
        if (entry_count == 0) {
            std::memset(entries, 0, sizeof(entries));
        }

        SIG_IP_MONITOR_PUNISH_ENTRY& entry = entries[entry_count];
        if (!db_handle->get_str(0, entry.ip, 16) ||
            !db_handle->get_int(1, entry.punish_type) ||
            !db_handle->get_uint(2, entry.member_id_count) ||
            !db_handle->get_uint(3, entry.start_time) ||
            !db_handle->get_uint(4, entry.end_time)) {
            return false;
        }

        if (entry_count >= 99u || row >= row_count - 1) {
            Stream* reply_stream =
                GlobalData::s_stream_pool->Acquire("DBThread.cpp", 31465);
            CStreamGuard reply(reply_stream, true);
            *reply << 0x156;
            *reply << -1;
            SIG_IP_MONITOR_PUNISH* payload =
                reply->GetInBuffer<SIG_IP_MONITOR_PUNISH>();
            payload->count = entry_count + 1;
            std::memcpy(payload->entries, entries, sizeof(entries));
            payload->initial_load = 0;
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
            entry_count = 0;
        } else {
            ++entry_count;
        }
    }
    return true;
}

// IDA_EA: 0x8422242
// IDA_NAME: _ZN17DB_LoadDnfHackLog8dispatchEiiP6Stream
// DEMANGLED: DB_LoadDnfHackLog::dispatch(int,int,Stream *)
bool DB_LoadDnfHackLog::dispatch(int signal, int user_idx, Stream* stream) {
    if (!DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    SIG_LOAD_DNF_HACK_LOG* request =
        stream->GetOutBuffer<SIG_LOAD_DNF_HACK_LOG>();
    const uint32_t m_id = request->m_id;

    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 23182);
    CStreamGuard reply(reply_stream, true);
    // DBTask stores this dispatcher at signal 0x106 and the matching
    // Inter_LoadDnfHackLog receiver occupies InterDispatcher slot 0x106.
    // DispatchThread forwards the first reply int unchanged. The raw
    // Hex-Rays literal `6` conflicts with both constructor tables; preserve
    // the executable route selected by the paired request and table evidence.
    *reply << 0x106;
    *reply << user_idx;
    SIG_LOAD_DNF_HACK_LOG* payload =
        reply->GetInBuffer<SIG_LOAD_DNF_HACK_LOG>();
    std::memset(payload, 0, sizeof(*payload));
    payload->m_id = m_id;

    db_handle->set_query(
        "seLect hack_type,cnt from dnf_hack_log where m_id=%s and "
        "(hack_type>=%d and hack_type<%d) and occ_date=CAST(DATE_FOR"
        "MAT(now(), '%%Y-%%m-%%d') AS CHAR)",
        NumberToString(m_id, 0), 101, 4001);
    if (!db_handle->exec(true)) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    for (int row = 0; row < row_count; ++row) {
        unsigned int hack_type = 0;
        unsigned int count = 0;
        if (!db_handle->fetch() || !db_handle->get_uint(0, hack_type)) {
            return false;
        }
        // The SQL predicate establishes the lower bound; retaining it here
        // keeps the original 101..4000 index mapping typed and bounded.
        if (hack_type >= 101u && hack_type <= 4000u) {
            if (!db_handle->get_uint(1, count)) {
                return false;
            }
            payload->counts_by_hack_type[hack_type - 101u] = count;
        }
    }
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x843D286
// IDA_NAME: _ZN16DB_JoinGuildInfo8dispatchEiiP6Stream
// DEMANGLED: DB_JoinGuildInfo::dispatch(int,int,Stream *)
bool DB_JoinGuildInfo::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);

    int server_group = 0;
    int charac_no = 0;
    *stream >> server_group;
    *stream >> charac_no;

    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37309);
    CStreamGuard reply(reply_stream, true);
    *reply << 0x219;
    *reply << user_idx;

    SIG_JOIN_GUILD_INFO* payload =
        reply->GetInBuffer<SIG_JOIN_GUILD_INFO>();

    db_handle->set_query(
        "seLect guild_id,memo from guild_join_list where charac_no=%d "
        "and server_group=%d",
        charac_no, server_group);
    if (!db_handle->exec(true)) {
        return false;
    }
    if (db_handle->get_n_rows() == 0) {
        payload->found = 0;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
        return true;
    }
    if (!db_handle->fetch() ||
        !db_handle->get_uint(0, payload->guild_id) ||
        !db_handle->get_str(1, payload->memo, 26)) {
        return false;
    }

    db_handle->set_query(
        "seLect guild_name,master_name,lev,member_count from guild_info "
        "where expire_flag=0 and guild_id=%d",
        payload->guild_id);
    if (!db_handle->exec(true)) {
        return false;
    }
    // The old routine returns success without a reply when the join-list row
    // survives but the guild-info row no longer does.
    if (db_handle->get_n_rows() == 0) {
        return true;
    }
    if (!db_handle->fetch() ||
        !db_handle->get_str(0, payload->guild_name, 23) ||
        !db_handle->get_str(1, payload->master_name, 30) ||
        !db_handle->get_int(2, payload->guild_level) ||
        !db_handle->get_int(3, payload->member_count)) {
        return false;
    }
    payload->found = 1;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x843D87C
// IDA_NAME: _ZN16DB_GuildJoinList8dispatchEiiP6Stream
// DEMANGLED: DB_GuildJoinList::dispatch(int,int,Stream *)
bool DB_GuildJoinList::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    MySQL* guild_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    MySQL* charac_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);

    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37443);
    CStreamGuard reply(reply_stream, true);
    *reply << 0x21A;
    *reply << user_idx;

    int guild_id = 0;
    int server_group = 0;
    *stream >> guild_id;
    *stream >> server_group;
    guild_db->set_query(
        "delete from guild_join_list where guild_id=%d and server_group != %d",
        guild_id, server_group);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "seLect charac_no,memo,unix_timestamp(occ_time) from "
        "guild_join_list where guild_id=%d",
        guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }

    SIG_GUILD_JOIN_LIST* payload =
        reply->GetInBuffer<SIG_GUILD_JOIN_LIST>();
    const int row_count = guild_db->get_n_rows();
    const int entry_count = row_count > 50 ? 50 : row_count;
    payload->count = static_cast<uint32_t>(entry_count);
    const std::time_t now = std::time(0);

    for (int index = 0; index < entry_count; ++index) {
        SIG_GUILD_JOIN_LIST_ENTRY& entry = payload->entries[index];
        int occurred_at = 0;
        if (!guild_db->fetch() ||
            !guild_db->get_uint(0, entry.charac_no) ||
            !guild_db->get_str(1, entry.memo, 26) ||
            !guild_db->get_int(2, occurred_at)) {
            return false;
        }
        entry.elapsed_seconds = static_cast<int32_t>(
            now - static_cast<std::time_t>(occurred_at));
    }

    for (int index = 0; index < entry_count; ++index) {
        SIG_GUILD_JOIN_LIST_ENTRY& entry = payload->entries[index];
        charac_db->set_query(
            "seLect charac_name,lev,job,grow_type,sex from charac_info "
            "where charac_no=%d",
            entry.charac_no);
        if (!charac_db->exec(true)) {
            return false;
        }
        if (charac_db->get_n_rows() == 0) {
            continue;
        }
        if (!charac_db->fetch() ||
            !charac_db->get_str(0, entry.charac_name, 30) ||
            !charac_db->get_byte(1, entry.level) ||
            !charac_db->get_byte(2, entry.job) ||
            !charac_db->get_byte(3, entry.grow_type) ||
            !charac_db->get_byte(4, entry.sex)) {
            return false;
        }
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x84354DA
// IDA_NAME: _ZN34DB_BlackipPrivateSecurityValuation8dispatchEiiP6Stream
// DEMANGLED: DB_BlackipPrivateSecurityValuation::dispatch(int,int,Stream *)
bool DB_BlackipPrivateSecurityValuation::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    unsigned int m_id = 0;
    *stream >> m_id;
    db_handle->set_query(
        "upDate member_security_grade set black_ip_try_time=now() "
        "where m_id=%s",
        NumberToString(m_id, 0));
    bool need_insert = true;
    if (db_handle->exec(true) && db_handle->getAffectedRowCount() != 0) {
        need_insert = false;
    }
    if (need_insert) {
        db_handle->set_query(
            "inSert into member_security_grade(m_id, black_ip_try_time) "
            "values(%s, now())",
            NumberToString(m_id, 0));
        if (!db_handle->exec(true)) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843D6EA
// IDA_NAME: _ZN18DB_CancelJoinGuild8dispatchEiiP6Stream
// DEMANGLED: DB_CancelJoinGuild::dispatch(int,int,Stream *)
bool DB_CancelJoinGuild::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);

    int guild_id = 0;
    int charac_no = 0;
    *stream >> guild_id;
    *stream >> charac_no;
    db_handle->set_query(
        "deLete from guild_join_list where guild_id=%d and charac_no=%d",
        guild_id, charac_no);
    return db_handle->exec(true);
}

// IDA_EA: 0x843DE6C
// IDA_NAME: _ZN16DB_DenyJoinGuild8dispatchEiiP6Stream
// DEMANGLED: DB_DenyJoinGuild::dispatch(int,int,Stream *)
bool DB_DenyJoinGuild::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    int guild_id = 0;
    int charac_no = 0;
    *stream >> guild_id;
    *stream >> charac_no;
    db_handle->set_query(
        "deLete from guild_join_list where guild_id=%d and charac_no=%d",
        guild_id, charac_no);
    return db_handle->exec(true);
}

// IDA_EA: 0x842F22A
// IDA_NAME: _ZN30DB_GoblinPadUpdateValidityTime8dispatchEiiP6Stream
// DEMANGLED: DB_GoblinPadUpdateValidityTime::dispatch(int,int,Stream *)
bool DB_GoblinPadUpdateValidityTime::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    SIG_GOBLIN_UPDATE_VALIDITYTIME* request =
        stream->GetOutBuffer<SIG_GOBLIN_UPDATE_VALIDITYTIME>();
    db_handle->set_query(
        "upDate member_mousepass set validity_time=unix_timestamp(now())+%d "
        "where m_id=%s",
        request->validity_seconds, NumberToString(request->m_id, 0));
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x8439C6C
// IDA_NAME: _ZN30DB_UpdateCompoundEmblemStatics8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCompoundEmblemStatics::dispatch(int,int,Stream *)
bool DB_UpdateCompoundEmblemStatics::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC* request =
        stream->GetOutBuffer<SIG_UPDATE_COMPOUND_EMBLEM_STATISTIC>();
    db_handle->set_query(
        "inSert into log_emblem_compound(occ_date, channel_no, grade0, "
        "grade1, grade2, grade3, grade4, grade5, grade6) values"
        "(DATE_SUB(CURDATE(), INTERVAL 1 DAY), %d, %d, %d, %d, %d, %d, %d, %d)",
        request->channel_no,
        request->grade_count[0], request->grade_count[1],
        request->grade_count[2], request->grade_count[3],
        request->grade_count[4], request->grade_count[5],
        request->grade_count[6]);
    bool need_update = true;
    if (db_handle->exec(true) && db_handle->getAffectedRowCount() != 0) {
        need_update = false;
    }
    if (need_update) {
        db_handle->set_query(
            "upDate log_emblem_compound set grade0=grade0+%d, "
            "grade1=grade1+%d, grade2=grade2+%d, grade3=grade3+%d, "
            "grade4=grade4+%d, grade5=grade5+%d, grade6=grade6+%d "
            "where occ_date=DATE_SUB(CURDATE(), INTERVAL 1 DAY) "
            "and channel_no=%d",
            request->grade_count[0], request->grade_count[1],
            request->grade_count[2], request->grade_count[3],
            request->grade_count[4], request->grade_count[5],
            request->grade_count[6], request->channel_no);
        if (!db_handle->exec(true)) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x844E86E
// IDA_NAME: _ZN23DB_CheckDoubleGuildNameC2Ev
// DEMANGLED: DB_CheckDoubleGuildName::DB_CheckDoubleGuildName(void)
DB_CheckDoubleGuildName::DB_CheckDoubleGuildName() : DBDispatcher() {
}

// IDA_EA: 0x845E548
// IDA_NAME: _ZN23DB_CheckDoubleGuildNameD2Ev
// DEMANGLED: DB_CheckDoubleGuildName::~DB_CheckDoubleGuildName()
// IDA_EA: 0x845E578
// IDA_NAME: _ZN23DB_CheckDoubleGuildNameD0Ev
// DEMANGLED: DB_CheckDoubleGuildName::~DB_CheckDoubleGuildName()
DB_CheckDoubleGuildName::~DB_CheckDoubleGuildName() {
}

// IDA_EA: 0x844E88A
// IDA_NAME: _ZN26DB_CheckDoubleGuildAddressC2Ev
// DEMANGLED: DB_CheckDoubleGuildAddress::DB_CheckDoubleGuildAddress(void)
DB_CheckDoubleGuildAddress::DB_CheckDoubleGuildAddress() : DBDispatcher() {
}

// IDA_EA: 0x845E4FA
// IDA_NAME: _ZN26DB_CheckDoubleGuildAddressD2Ev
// DEMANGLED: DB_CheckDoubleGuildAddress::~DB_CheckDoubleGuildAddress()
// IDA_EA: 0x845E52A
// IDA_NAME: _ZN26DB_CheckDoubleGuildAddressD0Ev
// DEMANGLED: DB_CheckDoubleGuildAddress::~DB_CheckDoubleGuildAddress()
DB_CheckDoubleGuildAddress::~DB_CheckDoubleGuildAddress() {
}

// IDA_EA: 0x842A9FC
// IDA_NAME: _ZN23DB_CheckDoubleGuildName8dispatchEiiP6Stream
// DEMANGLED: DB_CheckDoubleGuildName::dispatch(int,int,Stream *)
bool DB_CheckDoubleGuildName::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    SIG_DOUBLE_CHECK_GUILD_NAME* request =
        stream->GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>();
    db_handle->set_query(
        "seLect * from guild_info where expire_flag=0 and guild_name='%s'",
        request->guild_name);
    if (!db_handle->exec(true)) {
        return false;
    }
    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 27782);
    CStreamGuard reply(reply_stream, true);
    *reply << 0xEB;
    *reply << user_idx;
    *reply << static_cast<char>(db_handle->get_n_rows() != 0 ? 2 : 1);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x842AB78
// IDA_NAME: _ZN26DB_CheckDoubleGuildAddress8dispatchEiiP6Stream
// DEMANGLED: DB_CheckDoubleGuildAddress::dispatch(int,int,Stream *)
bool DB_CheckDoubleGuildAddress::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    SIG_DOUBLE_CHECK_GUILD_ADDRESS* request =
        stream->GetOutBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>();
    db_handle->set_query(
        "seLect * from guild_info where guild_url='%s'",
        request->guild_address);
    if (!db_handle->exec(true)) {
        return false;
    }
    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 27807);
    CStreamGuard reply(reply_stream, true);
    *reply << 0xEC;
    *reply << user_idx;
    *reply << static_cast<char>(db_handle->get_n_rows() != 0 ? 2 : 1);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);

    return true;
}

// IDA_EA: 0x844F29A
// IDA_NAME: _ZN14DB_DeleteGuildC2Ev
// DEMANGLED: DB_DeleteGuild::DB_DeleteGuild(void)
DB_DeleteGuild::DB_DeleteGuild() : DBDispatcher() {}

// IDA_EA: 0x845C8F2
// IDA_NAME: _ZN14DB_DeleteGuildD2Ev
// DEMANGLED: DB_DeleteGuild::~DB_DeleteGuild()
// IDA_EA: 0x845C922
// IDA_NAME: _ZN14DB_DeleteGuildD0Ev
// DEMANGLED: DB_DeleteGuild::~DB_DeleteGuild()
DB_DeleteGuild::~DB_DeleteGuild() {}

// IDA_EA: 0x843A9C0
// IDA_NAME: _ZN14DB_DeleteGuild8dispatchEiiP6Stream
// DEMANGLED: DB_DeleteGuild::dispatch(int,int,Stream *)
bool DB_DeleteGuild::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* charac_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    MySQL* guild_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    int master_charac_no = 0;
    int guild_id = 0;
    *stream >> master_charac_no;
    *stream >> guild_id;
    if (master_charac_no == 0 || guild_id == 0) {
        return true;
    }
    charac_db->set_query(
        "upDate charac_info set guild_right=0, guild_id=0 where charac_no=%d",
        master_charac_no);
    if (!charac_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "upDate guild_info set expire_flag=1, expire_time=now() "
        "where guild_id=%d",
        guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "deLete from guild_introduce where guild_id=%d", guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "deLete from guild_member where guild_id=%d", guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "deLete from guild_visit where guild_id=%d", guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "deLete from guild_notice where guild_id=%d", guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    guild_db->set_query(
        "deLete from guild_skill where guild_id=%d", guild_id);
    if (!guild_db->exec(true)) {
        return false;
    }
    charac_db->set_query(
        "upDate charac_info set guild_id=0, guild_secede=1 where guild_id = %u",
        guild_id);
    return charac_db->exec(true);
}

// IDA_EA: 0x844F0F6
// IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageC2Ev
// DEMANGLED: DB_DeletePowerWarStatueMessage::DB_DeletePowerWarStatueMessage(void)
DB_DeletePowerWarStatueMessage::DB_DeletePowerWarStatueMessage()
    : DBDispatcher() {}

// IDA_EA: 0x845CFF4
// IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageD2Ev
// DEMANGLED: DB_DeletePowerWarStatueMessage::~DB_DeletePowerWarStatueMessage()
// IDA_EA: 0x845D024
// IDA_NAME: _ZN30DB_DeletePowerWarStatueMessageD0Ev
// DEMANGLED: DB_DeletePowerWarStatueMessage::~DB_DeletePowerWarStatueMessage()
DB_DeletePowerWarStatueMessage::~DB_DeletePowerWarStatueMessage() {}

// IDA_EA: 0x8437D4E
// IDA_NAME: _ZN30DB_DeletePowerWarStatueMessage8dispatchEiiP6Stream
// DEMANGLED: DB_DeletePowerWarStatueMessage::dispatch(int,int,Stream *)
bool DB_DeletePowerWarStatueMessage::dispatch(
        int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG_REF, SERVER_GROUP_DEFAULT);
    int server_info = 0;
    *stream >> server_info;
    db_handle->set_query(
        "deLete from event_server_message where server_info = %d "
        "and message_index in (1, 2, 3)",
        server_info);
    if (!db_handle->exec(true)) {
        LogManager::logFormat(
            1, "DBThread.cpp",
            "virtual bool DB_DeletePowerWarStatueMessage::dispatch(int, int, Stream*)",
            34109, "deLete_power_war_statue_message Query Error");
    }
    return true;
}

// IDA_EA: 0x844FC72
// IDA_NAME: _ZN19DB_SaveInformNoticeC2Ev
// DEMANGLED: DB_SaveInformNotice::DB_SaveInformNotice(void)
DB_SaveInformNotice::DB_SaveInformNotice() : DBDispatcher() {}

// IDA_EA: 0x845B1CA
// IDA_NAME: _ZN19DB_SaveInformNoticeD2Ev
// DEMANGLED: DB_SaveInformNotice::~DB_SaveInformNotice()
// IDA_EA: 0x845B1FA
// IDA_NAME: _ZN19DB_SaveInformNoticeD0Ev
// DEMANGLED: DB_SaveInformNotice::~DB_SaveInformNotice()
DB_SaveInformNotice::~DB_SaveInformNotice() {}

// IDA_EA: 0x8446FCC
// IDA_NAME: _ZN19DB_SaveInformNotice8dispatchEiiP6Stream
// DEMANGLED: DB_SaveInformNotice::dispatch(int,int,Stream *)
bool DB_SaveInformNotice::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOGIN, SERVER_GROUP_DEFAULT);
    InformNoticeFlagData* request =
        stream->GetOutBuffer<InformNoticeFlagData>();
    char escaped[256];
    std::memset(escaped, 0, sizeof(escaped));
    db_handle->escape_string(escaped, request->inform_notice);
    db_handle->set_query(
        "upDate login_common set inform_notice = '%s' where m_id = %s",
        escaped, NumberToString(request->m_id, 0));
    if (!db_handle->exec(true)) {
        return false;
    }
    if (db_handle->getAffectedRowCount() == 0) {
        db_handle->set_query(
            "inSert into login_common(m_id, inform_notice) values(%s, '%s')",
            NumberToString(request->m_id, 0), escaped);
        if (!db_handle->exec(true)) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x843C98E
// IDA_NAME: _ZN17DB_CheckJoinGuild8dispatchEiiP6Stream
// DEMANGLED: DB_CheckJoinGuild::dispatch(int,int,Stream *)
bool DB_CheckJoinGuild::dispatch(int signal, int user_idx, Stream* stream) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    SIG_CHECK_JOIN_GUILD* request =
        stream->GetOutBuffer<SIG_CHECK_JOIN_GUILD>();
    char escaped_guild_name[47];
    std::memset(escaped_guild_name, 0, sizeof(escaped_guild_name));
    db_handle->escape_string(escaped_guild_name, request->guild_name);
    db_handle->set_query(
        "seLect guild_name,master_name,lev,member_count from guild_info "
        "where server_id=%d and expire_flag=0 and guild_name='%s'",
        request->server_id, escaped_guild_name);
    if (!db_handle->exec(true)) {
        return false;
    }

    Stream* reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 37094);
    CStreamGuard reply_guard(reply_stream, true);
    *reply_guard << 0x216;
    *reply_guard << user_idx;
    SIG_CHECK_JOIN_GUILD_DB_REPLY* reply =
        reply_guard->GetInBuffer<SIG_CHECK_JOIN_GUILD_DB_REPLY>();
    std::memset(reply, 0, sizeof(*reply));
    if (db_handle->get_n_rows() == 0) {
        reply->result = 21;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply_guard);
        return true;
    }
    if (!db_handle->fetch()) {
        return false;
    }
    reply->result = 0;
    int guild_level = 0;
    int member_count = 0;
    if (!db_handle->get_str(0, reply->guild_name, 23) ||
        !db_handle->get_str(1, reply->master_name, 30) ||
        !db_handle->get_int(2, guild_level) ||
        !db_handle->get_int(3, member_count)) {
        return false;
    }
    reply->guild_level = guild_level;
    reply->member_count = member_count;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply_guard);
    return true;
}

// IDA_EA: 0x843C514
// IDA_NAME: _ZN27DB_InsertPoliceSaveChatting8dispatchEiiP6Stream
// DEMANGLED: DB_InsertPoliceSaveChatting::dispatch(int,int,Stream *)
bool DB_InsertPoliceSaveChatting::dispatch(
        int signal, int user_idx, Stream* stream) {
    unsigned int charac_no = 0;
    char message_type = 0;
    char message[256];
    char escaped_message[512];
    std::memset(message, 0, sizeof(message));
    std::memset(escaped_message, 0, sizeof(escaped_message));
    *stream >> charac_no;
    *stream >> message_type;

    // Both old producers constrain this C string to a 256-byte PacketBuf
    // destination.  Keep their valid-frame cursor behavior while refusing a
    // malformed queued frame before it can overflow this local destination.
    const int remaining_bytes = stream->length();
    if (remaining_bytes <= 0) {
        return false;
    }
    const char* const message_begin = stream->get_buffer();
    const char* const message_end = static_cast<const char*>(std::memchr(
        message_begin, '\0', static_cast<size_t>(remaining_bytes)));
    if (message_end == 0) {
        return false;
    }
    const int message_bytes =
        static_cast<int>(message_end - message_begin) + 1;
    if (message_bytes > static_cast<int>(sizeof(message))) {
        return false;
    }
    std::memcpy(message, message_begin, static_cast<size_t>(message_bytes));
    // Preserve Stream::operator>>(char*)'s copy-then-advance order.  The
    // bounded scan above is the only behavior added for malformed frames.
    if (stream->out_ptr(message_bytes) == -1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    db_handle->escape_string(escaped_message, message);
    db_handle->set_query(
        "inSert into log_police_chatting(charac_no, msg_type, occ_time, "
        "message) values(%u, %d, unix_timestamp(now()), '%s')",
        charac_no, message_type, escaped_message);
    db_handle->exec(true);
    return true;
}

// IDA_EA: 0x84400C8
// IDA_NAME: _ZN20DB_TOD_Reset_UserAPC8dispatchEiiP6Stream
// DEMANGLED: DB_TOD_Reset_UserAPC::dispatch(int,int,Stream *)
bool DB_TOD_Reset_UserAPC::dispatch(int signal, int user_idx, Stream* stream) {
    (void)stream;

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);

    db_handle->set_query(
        "seLect charac_no from charac_tower_despair_apc limit 10");
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "virtual bool DB_TOD_Reset_UserAPC::dispatch(int, int, Stream*)",
            40615,
            5);
        trace("TOD : load apc from DB");
        return false;
    }

    Stream* const reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 40619);
    CStreamGuard reply(reply_stream, true);
    *reply << signal;
    *reply << user_idx;
    SIG_TOD_RELOADED_USER_APC* const payload =
        reply->GetInBuffer<SIG_TOD_RELOADED_USER_APC>();
    std::memset(payload, 0, sizeof(*payload));

    const int row_count = db_handle->get_n_rows();
    if (row_count < 0 || row_count > kTODAPCCount) {
        cMyTrace trace(
            "virtual bool DB_TOD_Reset_UserAPC::dispatch(int, int, Stream*)",
            40629,
            5);
        trace("TOD : get_n_rows() %d", row_count);
        return false;
    }
    if (row_count != kTODAPCCount) {
        cMyTrace trace(
            "virtual bool DB_TOD_Reset_UserAPC::dispatch(int, int, Stream*)",
            40629,
            5);
        trace("[TOD_APC][ZERO_FILL] rows=%d expected=%d", row_count,
              kTODAPCCount);
    }

    for (int index = 0; index < row_count && index <= 9; ++index) {
        if (db_handle->fetch() != true) {
            cMyTrace trace(
                "virtual bool DB_TOD_Reset_UserAPC::dispatch(int, int, Stream*)",
                40637,
                5);
            trace("TOD : db->fetch(), int i=%d", index);
            return false;
        }
        // Raw code ignores this return and leaves the pre-zeroed entry in
        // place if the column conversion fails.
        db_handle->get_int(0, payload->entries[index].user_charac_no);
    }

    // The old dispatcher deliberately keeps the ten-entry reply even when a
    // particular profile lookup fails; its receiver consumes all ten slots.
    for (int index = 0; index < row_count && index <= 9; ++index) {
        if (payload->entries[index].user_charac_no != 0) {
            GetAPCUserInfo(payload->entries[index]);
        }
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x8440548
// IDA_NAME: _ZN20DB_TOD_Reset_UserAPC14GetAPCUserInfoER8_APCInfo
// DEMANGLED: DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo &)
bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo& apc_info) {
    MySQL* const charac_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    MySQL* const guild_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);
    MySQL* const inventory_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);

    charac_db->set_query(
        "seLect charac_name,lev,job,grow_type,guild_id from charac_info "
        "where charac_no=%d",
        apc_info.user_charac_no);
    // The original makes this first call and intentionally discards its
    // result before the second, checked execution.
    charac_db->exec(true);
    if (charac_db->exec(true) != true || charac_db->fetch() != true) {
        return false;
    }
    if (charac_db->get_n_rows() != 1) {
        return false;
    }

    if (charac_db->get_str(0, apc_info.user_charac_name, 30) != true ||
        charac_db->get_byte(1, apc_info.level) != true ||
        charac_db->get_byte(2, apc_info.job) != true ||
        charac_db->get_byte(3, apc_info.grow_type) != true ||
        charac_db->get_uint(4, apc_info.guild_id) != true) {
        return false;
    }

    if (apc_info.guild_id != 0) {
        // Raw code reads the old CEnvironment dword at +888 only for this
        // diagnostic.  The canonical accessor preserves that read without
        // coupling this DB provider to the environment's private layout.
        const int environment_888 = static_cast<int>(
            G_CEnvironment()->get_accept_handshake_flag_888());
        guild_db->set_query(
            "seLect guild_name from guild_info where guild_id=%d",
            apc_info.guild_id);
        if (guild_db->exec(true) != true || guild_db->fetch() != true) {
            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40774,
                5);
            trace("TOD:select guild db fail, guild_id:%d, server_id:%d",
                  apc_info.guild_id,
                  environment_888);
        } else if (guild_db->get_n_rows() != 0 &&
                   guild_db->get_str(0, apc_info.guild_name, 22) != true) {
            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40782,
                5);
            trace("TOD:select guild db fail, guild_id:%d, server_id:%d",
                  apc_info.guild_id,
                  environment_888);
        }
    }

    int user_item_triplets[kTODUserItemTripletCount];
    std::memset(user_item_triplets, 0, sizeof(user_item_triplets));
    inventory_db->set_query(
        "seLect slot,it_id,clear_avatar_id from user_items where "
        "charac_no=%u and expire_date>now() and stat=0 limit %d",
        apc_info.user_charac_no,
        kTODUserItemsLimit);
    if (inventory_db->exec(true) != true) {
        cMyTrace trace(
            "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
            40797,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, exec() ERROR "
              "charac_no=%u",
              apc_info.user_charac_no);
        return false;
    }

    const int row_count = inventory_db->get_n_rows();
    for (int index = 0; index < row_count; ++index) {
        int* const row = user_item_triplets + index * 3;
        if (inventory_db->fetch() != true) {
            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40807,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, exec() ERROR "
                  "charac_no=%u",
                  apc_info.user_charac_no);
            break;
        }
        if (inventory_db->get_int(0, row[0]) != true) {
            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40813,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(0) "
                  "ERROR charac_no=%u",
                  apc_info.user_charac_no);
            return false;
        }
        if (inventory_db->get_int(1, row[1]) != true) {

            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40819,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(1) "
                  "ERROR charac_no=%u",
                  apc_info.user_charac_no);
            return false;
        }
        if (inventory_db->get_int(2, row[2]) != true) {
            cMyTrace trace(
                "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
                40825,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(2) "
                  "ERROR charac_no=%u",
                  apc_info.user_charac_no);
            return false;
        }
        if (row[0] >= 0 && row[0] <= kTODLegacyAvatarLastSlot) {
            apc_info.equipment_item_ids[row[0]] =
                static_cast<uint32_t>(row[1]);
        }
    }

    // The old code trusted the selected rows.  The 140828 DB can additionally
    // contain weapon-avatar slot 10, which is not part of this legacy IPC
    // record, so keep only the original avatar slots 0..9 here.
    for (int index = 0; index < row_count; ++index) {
        const int* const row = user_item_triplets + index * 3;
        if (row[2] == 0 || row[0] < 0 ||
            row[0] > kTODLegacyAvatarLastSlot) {
            continue;
        }
        inventory_db->set_query(
            "seLect it_id from user_items where ui_id=%d", row[2]);
        // Keep the original two-stage condition exactly: fetch is evaluated
        // only after a successful exec, and get_int's result is discarded.
        if (inventory_db->exec(true) == true && inventory_db->fetch() == true) {
            inventory_db->get_int(0,
                                  apc_info.equipment_item_ids[row[0]]);
        }
    }

    unsigned char compressed_equip_slot[kTODEquipSlotCompressedBytes];
    std::memset(compressed_equip_slot, 0, sizeof(compressed_equip_slot));
    unsigned int decoded_equip_slot_bytes = kDecodedEquipmentByteLimit;

    inventory_db->set_query(
        "seLect equipslot from inventory where charac_no=%u",
        apc_info.user_charac_no);
    // The source calls exec/fetch here without branching on either result;
    // the get_binary failure path below remains its effective guard.
    inventory_db->exec(true);
    inventory_db->fetch();
    const unsigned int compressed_equip_slot_bytes =
        static_cast<unsigned int>(inventory_db->get_binary_length(0));
    if (inventory_db->get_binary(0,
                                 compressed_equip_slot,
                                 sizeof(compressed_equip_slot)) != true) {
        cMyTrace trace(
            "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
            40861,
            5);
        trace("DB_LoadInventory::GetUserInven, get_binary(3) ERROR "
              "charac_no=%u",
              apc_info.user_charac_no);
        return false;
    }

    // The old frame begins decompression at item[10] (+0x262) and reads that
    // decoded item's +2 key dword.  Keep the origin and the original 890-byte
    // bound with canonical inventory records instead of an anonymous byte
    // scratch region.
    Inven_Item decoded_equip_slot[kTODDecodedEquipmentScratchCount];
    if (uncompress_zip(
                       reinterpret_cast<char*>(
                           decoded_equip_slot +
                           kTODDecodedEquipmentOriginIndex),
                       decoded_equip_slot_bytes,
                       reinterpret_cast<const char*>(compressed_equip_slot),
                       compressed_equip_slot_bytes) != 1) {
        return false;
    }

    apc_info.equipment_item_ids[kTODRawEquipmentSlot] =
        static_cast<uint32_t>(
            decoded_equip_slot[kTODDecodedEquipmentOriginIndex].getKey());

    inventory_db->set_query(
        "seLect it_id,name from creature_items where slot=238 and "
        "charac_no=%u and expire_date > now()",
        apc_info.user_charac_no);
    if (inventory_db->exec(true) == true) {
        if (inventory_db->fetch() == true) {
            inventory_db->get_int(0, apc_info.creature_id);
            inventory_db->get_str(1, apc_info.creature_name, 12);
        }
    } else {
        cMyTrace trace(
            "bool DB_TOD_Reset_UserAPC::GetAPCUserInfo(_APCInfo&)",
            40895,
            5);
        trace("TOD: read DB creature_items error");
    }
    return true;
}

// IDA_EA: 0x844EEE2
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfoC2Ev
// DEMANGLED: DB_LoadPowerWarStatueInfo::DB_LoadPowerWarStatueInfo(void)
DB_LoadPowerWarStatueInfo::DB_LoadPowerWarStatueInfo() : DBDispatcher() {}

// IDA_EA: 0x845D5BE
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfoD2Ev
// DEMANGLED: DB_LoadPowerWarStatueInfo::~DB_LoadPowerWarStatueInfo()
// IDA_EA: 0x845D5EE
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfoD0Ev
// DEMANGLED: DB_LoadPowerWarStatueInfo::~DB_LoadPowerWarStatueInfo()
// NOTE: the compiler-generated D0 wrapper invokes this D2 body then delete.
DB_LoadPowerWarStatueInfo::~DB_LoadPowerWarStatueInfo() {}

// IDA_EA: 0x8434886
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo8dispatchEiiP6Stream
// DEMANGLED: DB_LoadPowerWarStatueInfo::dispatch(int,int,Stream *)
bool DB_LoadPowerWarStatueInfo::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    Stream* const reply_stream =
        GlobalData::s_stream_pool->Acquire("DBThread.cpp", 32413);
    CStreamGuard reply(reply_stream, true);
    *reply << signal;
    *reply << user_idx;
    SIG_LOAD_POWER_WAR_STATUE_INFO* const payload =
        reply->GetInBuffer<SIG_LOAD_POWER_WAR_STATUE_INFO>();
    std::memset(payload, 0, sizeof(*payload));

    unsigned int charac_no = 0;
    *stream >> charac_no;
    if (GetCharacInfo(payload, charac_no) != true) {
        std::memset(payload, 0, sizeof(*payload));
        cMyTrace trace(
            "virtual bool DB_LoadPowerWarStatueInfo::dispatch(int, int, Stream*)",
            32431,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetInvenInfo, GetCharacInfo ERROR "
              "charac_no=%u",
              charac_no);
    }
    if (GetAvatarInfo(payload, charac_no) != true) {
        std::memset(payload, 0, sizeof(*payload));
        cMyTrace trace(
            "virtual bool DB_LoadPowerWarStatueInfo::dispatch(int, int, Stream*)",
            32439,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetInvenInfo, exec GetInvenInfo "
              "charac_no=%u",
              charac_no);
    }

    // The old routine writes the requested number after either helper has
    // failed, so consumers can still associate its zeroed reply with a user.

    payload->charac_no = charac_no;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, reply);
    return true;
}

// IDA_EA: 0x8434A8A
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo13GetCharacInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFOj
// DEMANGLED: DB_LoadPowerWarStatueInfo::GetCharacInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *,unsigned int)
bool DB_LoadPowerWarStatueInfo::GetCharacInfo(
    SIG_LOAD_POWER_WAR_STATUE_INFO* payload,
    unsigned int charac_no) {
    MySQL* const charac_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    MySQL* const guild_db = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_GUILD, SERVER_GROUP_DEFAULT);

    charac_db->set_query(
        "seLect charac_name,job,lev,grow_type,guild_id from charac_info "
        "where charac_no=%d and delete_flag!=1",
        charac_no);
    // The first execution is observable in the old body and its return is
    // discarded before the second checked execution.
    charac_db->exec(true);
    if (charac_db->exec(true) != true || charac_db->fetch() != true) {
        return false;
    }
    if (charac_db->get_n_rows() != 1) {
        return false;
    }

    char job = 0;
    char level = 0;
    char grow_type = 0;
    if (charac_db->get_str(0, payload->charac_name, 29) != true ||
        charac_db->get_byte(1, job) != true ||
        charac_db->get_byte(2, level) != true ||
        charac_db->get_byte(3, grow_type) != true ||
        charac_db->get_uint(4, payload->guild_id) != true) {
        return false;
    }
    payload->job = static_cast<uint8_t>(job);
    payload->level = static_cast<uint8_t>(level);
    payload->grow_type = static_cast<uint8_t>(grow_type);

    if (payload->guild_id == 0) {
        return true;
    }

    // Raw code loads the environment's server-group scalar before issuing
    // this query.  The accessor keeps the query semantic rather than relying
    // on a private physical CEnvironment offset.
    const int server_group = G_CEnvironment()->get_server_group();
    guild_db->set_query(
        "seLect guild_name from guild_info where guild_id=%d and server_id=%d",
        payload->guild_id,
        server_group);
    if (guild_db->exec(true) != true || guild_db->fetch() != true) {
        return false;
    }
    if (guild_db->get_n_rows() != 1) {
        return false;
    }
    return guild_db->get_str(0, payload->guild_name, 40) == true;
}

// IDA_EA: 0x8434D36
// IDA_NAME: _ZN25DB_LoadPowerWarStatueInfo13GetAvatarInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFOj
// DEMANGLED: DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *,unsigned int)
bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(
    SIG_LOAD_POWER_WAR_STATUE_INFO* payload,
    unsigned int charac_no) {
    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);

    unsigned char compressed_equip_slot[kPowerWarEquipmentBlobBytes];
    std::memset(compressed_equip_slot, 0, sizeof(compressed_equip_slot));
    unsigned int decoded_bytes = kDecodedEquipmentByteLimit;

    db_handle->set_query(
        "seLect equipslot from inventory where charac_no=%u", charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        cMyTrace trace(
            "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
            32521,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, exec() or fetch() "
              "ERROR charac_no=%u",
              charac_no);
        return false;
    }

    // The old function constructs inventory records, starts its decoder at
    // record 10 (+0x262), then uses that record's +2 item key.  The 25-record
    // typed scratch keeps all 890 bytes available after the old origin.
    Inven_Item decoded_equipment[kPowerWarDecodedEquipmentScratchCount];

    const unsigned int compressed_bytes =
        static_cast<unsigned int>(db_handle->get_binary_length(0));
    if (db_handle->get_binary(0,
                              compressed_equip_slot,
                              sizeof(compressed_equip_slot)) != true) {
        cMyTrace trace(
            "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
            32533,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_binary(0) "
              "ERROR charac_no=%u",
              charac_no);
        return false;
    }
    if (compressed_bytes > sizeof(compressed_equip_slot)) {
        // The old function throws its source-line number as an unsigned int.
        throw static_cast<unsigned int>(32540);
    }
    if (uncompress_zip(reinterpret_cast<char*>(
                           decoded_equipment +
                           kPowerWarDecodedEquipmentOriginIndex),
                       decoded_bytes,
                       reinterpret_cast<const char*>(compressed_equip_slot),
                       compressed_bytes) != 1) {
        throw static_cast<unsigned int>(32542);
    }
    if (decoded_bytes != 0 && decoded_bytes % sizeof(Inven_Item) != 0) {
        cMyTrace trace(
            "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
            32547,
            5);
        trace("incorrect equipment size charac_no(%d), equipment, "
              "original_size(%d), db_size(%d)\n",
              static_cast<int>(charac_no),
              static_cast<int>(decoded_bytes),
              static_cast<int>(compressed_bytes));
        return false;
    }

    payload->equipment_item_ids[kPowerWarDecodedEquipmentSlot] =
        static_cast<uint32_t>(
            decoded_equipment[
                kPowerWarDecodedEquipmentOriginIndex].getKey());

    int user_item_triplets[kPowerWarUserItemTripletCount];
    std::memset(user_item_triplets, 0, sizeof(user_item_triplets));
    db_handle->set_query(
        "seLect slot,it_id,clear_avatar_id from user_items where "
        "charac_no=%u and expire_date>now() and stat=0 limit %d",
        charac_no,
        kPowerWarUserItemsLimit);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
            32565,
            5);
        trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, exec() ERROR "
              "charac_no=%u",
              charac_no);
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    for (int index = 0; index < row_count; ++index) {
        int* const row = user_item_triplets + index * 3;
        if (db_handle->fetch() != true) {

            cMyTrace trace(
                "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
                32575,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, exec() ERROR "
                  "charac_no=%u",
                  charac_no);
            break;
        }
        if (db_handle->get_int(0, row[0]) != true) {
            cMyTrace trace(
                "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
                32581,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(0) "
                  "ERROR charac_no=%u",
                  charac_no);
            return false;
        }
        if (db_handle->get_int(1, row[1]) != true) {
            cMyTrace trace(
                "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
                32587,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(1) "
                  "ERROR charac_no=%u",
                  charac_no);
            return false;
        }
        if (db_handle->get_int(2, row[2]) != true) {
            cMyTrace trace(
                "bool DB_LoadPowerWarStatueInfo::GetAvatarInfo(SIG_LOAD_POWER_WAR_STATUE_INFO*, unsigned int)",
                32593,
                5);
            trace("DB_LoadPowerWarStatueInfo::GetAvatarInfo, get_int(2) "
                  "ERROR charac_no=%u",
                  charac_no);
            return false;
        }
        if (static_cast<unsigned int>(row[0]) <
            static_cast<unsigned int>(
                oldsrv::config::kPersistedClearAvatarCount)) {
            payload->equipment_item_ids[row[0]] =
                static_cast<uint32_t>(row[1]);
        }
    }

    for (int index = 0; index < row_count; ++index) {
        const int* const row = user_item_triplets + index * 3;
        if (row[2] == 0 || row[0] < 0 ||
            static_cast<unsigned int>(row[0]) >=
                static_cast<unsigned int>(
                    oldsrv::config::kPersistedClearAvatarCount)) {
            continue;
        }
        db_handle->set_query("seLect it_id from user_items where ui_id=%d",
                             row[2]);
        if (db_handle->exec(true) == true && db_handle->fetch() == true) {
            db_handle->get_int(0, payload->equipment_item_ids[row[0]]);
        }
    }
    return true;
}
