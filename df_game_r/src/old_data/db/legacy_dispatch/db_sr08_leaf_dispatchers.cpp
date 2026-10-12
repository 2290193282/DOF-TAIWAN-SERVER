#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <ctime>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace {

MySQL* GetLeafDB(ENUM_DB_HANDLE_IDX handle) {
    return GlobalData::s_db_mgr->GetDBHandle(handle, SERVER_GROUP_DEFAULT);
}

}  // namespace

// The following seven dispatch bodies are not newly invented no-ops. Their
// original old-server exports each consist solely of return 1; retaining that
// behavior preserves the binary leaf without claiming a DB side effect.

// IDA_EA: 0x844F262
// IDA_NAME: _ZN17DB_DeletePvPBuddyC2Ev
// DEMANGLED: DB_DeletePvPBuddy::DB_DeletePvPBuddy(void)
DB_DeletePvPBuddy::DB_DeletePvPBuddy() {}
// IDA_EA: 0x845C98E
// IDA_NAME: _ZN17DB_DeletePvPBuddyD2Ev
// DEMANGLED: DB_DeletePvPBuddy::~DB_DeletePvPBuddy()
// IDA_EA: 0x845C9BE
// IDA_NAME: _ZN17DB_DeletePvPBuddyD0Ev
// DEMANGLED: DB_DeletePvPBuddy::~DB_DeletePvPBuddy()
DB_DeletePvPBuddy::~DB_DeletePvPBuddy() {}
// IDA_EA: 0x843A9AC
// IDA_NAME: _ZN17DB_DeletePvPBuddy8dispatchEiiP6Stream
// DEMANGLED: DB_DeletePvPBuddy::dispatch(int,int,Stream *)
bool DB_DeletePvPBuddy::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844F246
// IDA_NAME: _ZN17DB_InsertPvPBuddyC2Ev
// DEMANGLED: DB_InsertPvPBuddy::DB_InsertPvPBuddy(void)
DB_InsertPvPBuddy::DB_InsertPvPBuddy() {}
// IDA_EA: 0x845C9DC
// IDA_NAME: _ZN17DB_InsertPvPBuddyD2Ev
// DEMANGLED: DB_InsertPvPBuddy::~DB_InsertPvPBuddy()
// IDA_EA: 0x845CA0C
// IDA_NAME: _ZN17DB_InsertPvPBuddyD0Ev
// DEMANGLED: DB_InsertPvPBuddy::~DB_InsertPvPBuddy()
DB_InsertPvPBuddy::~DB_InsertPvPBuddy() {}
// IDA_EA: 0x843A9A2
// IDA_NAME: _ZN17DB_InsertPvPBuddy8dispatchEiiP6Stream
// DEMANGLED: DB_InsertPvPBuddy::dispatch(int,int,Stream *)
bool DB_InsertPvPBuddy::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844F22A
// IDA_NAME: _ZN15DB_LoadPvPBuddyC2Ev
// DEMANGLED: DB_LoadPvPBuddy::DB_LoadPvPBuddy(void)
DB_LoadPvPBuddy::DB_LoadPvPBuddy() {}
// IDA_EA: 0x845CA2A
// IDA_NAME: _ZN15DB_LoadPvPBuddyD2Ev
// DEMANGLED: DB_LoadPvPBuddy::~DB_LoadPvPBuddy()
// IDA_EA: 0x845CA5A
// IDA_NAME: _ZN15DB_LoadPvPBuddyD0Ev
// DEMANGLED: DB_LoadPvPBuddy::~DB_LoadPvPBuddy()
DB_LoadPvPBuddy::~DB_LoadPvPBuddy() {}
// IDA_EA: 0x843A998
// IDA_NAME: _ZN15DB_LoadPvPBuddy8dispatchEiiP6Stream
// DEMANGLED: DB_LoadPvPBuddy::dispatch(int,int,Stream *)
bool DB_LoadPvPBuddy::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844F2B6
// IDA_NAME: _ZN23DB_LoadPvPLiveEventDataC2Ev
// DEMANGLED: DB_LoadPvPLiveEventData::DB_LoadPvPLiveEventData(void)
DB_LoadPvPLiveEventData::DB_LoadPvPLiveEventData() {}
// IDA_EA: 0x845C8A4
// IDA_NAME: _ZN23DB_LoadPvPLiveEventDataD2Ev
// DEMANGLED: DB_LoadPvPLiveEventData::~DB_LoadPvPLiveEventData()
// IDA_EA: 0x845C8D4
// IDA_NAME: _ZN23DB_LoadPvPLiveEventDataD0Ev
// DEMANGLED: DB_LoadPvPLiveEventData::~DB_LoadPvPLiveEventData()
DB_LoadPvPLiveEventData::~DB_LoadPvPLiveEventData() {}
// IDA_EA: 0x843AC3E
// IDA_NAME: _ZN23DB_LoadPvPLiveEventData8dispatchEiiP6Stream
// DEMANGLED: DB_LoadPvPLiveEventData::dispatch(int,int,Stream *)
bool DB_LoadPvPLiveEventData::dispatch(int signal, int user_idx,
                                       Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844E50A
// IDA_NAME: _ZN34DB_UpdateDungeonDataStatisticLogdbC2Ev
// DEMANGLED: DB_UpdateDungeonDataStatisticLogdb::DB_UpdateDungeonDataStatisticLogdb(void)
DB_UpdateDungeonDataStatisticLogdb::DB_UpdateDungeonDataStatisticLogdb() {}
// IDA_EA: 0x845EEBA
// IDA_NAME: _ZN34DB_UpdateDungeonDataStatisticLogdbD2Ev
// DEMANGLED: DB_UpdateDungeonDataStatisticLogdb::~DB_UpdateDungeonDataStatisticLogdb()
// IDA_EA: 0x845EEEA
// IDA_NAME: _ZN34DB_UpdateDungeonDataStatisticLogdbD0Ev
// DEMANGLED: DB_UpdateDungeonDataStatisticLogdb::~DB_UpdateDungeonDataStatisticLogdb()
DB_UpdateDungeonDataStatisticLogdb::~DB_UpdateDungeonDataStatisticLogdb() {}
// IDA_EA: 0x842456A
// IDA_NAME: _ZN34DB_UpdateDungeonDataStatisticLogdb8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateDungeonDataStatisticLogdb::dispatch(int,int,Stream *)
bool DB_UpdateDungeonDataStatisticLogdb::dispatch(int signal, int user_idx,
                                                  Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844F27E
// IDA_NAME: _ZN17DB_UpdatePvPBuddyC2Ev
// DEMANGLED: DB_UpdatePvPBuddy::DB_UpdatePvPBuddy(void)
DB_UpdatePvPBuddy::DB_UpdatePvPBuddy() {}
// IDA_EA: 0x845C940
// IDA_NAME: _ZN17DB_UpdatePvPBuddyD2Ev
// DEMANGLED: DB_UpdatePvPBuddy::~DB_UpdatePvPBuddy()
// IDA_EA: 0x845C970
// IDA_NAME: _ZN17DB_UpdatePvPBuddyD0Ev
// DEMANGLED: DB_UpdatePvPBuddy::~DB_UpdatePvPBuddy()
DB_UpdatePvPBuddy::~DB_UpdatePvPBuddy() {}
// IDA_EA: 0x843A9B6
// IDA_NAME: _ZN17DB_UpdatePvPBuddy8dispatchEiiP6Stream
// DEMANGLED: DB_UpdatePvPBuddy::dispatch(int,int,Stream *)
bool DB_UpdatePvPBuddy::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844F2D2
// IDA_NAME: _ZN25DB_UpdatePvPLiveEventDataC2Ev
// DEMANGLED: DB_UpdatePvPLiveEventData::DB_UpdatePvPLiveEventData(void)
DB_UpdatePvPLiveEventData::DB_UpdatePvPLiveEventData() {}
// IDA_EA: 0x845C856
// IDA_NAME: _ZN25DB_UpdatePvPLiveEventDataD2Ev
// DEMANGLED: DB_UpdatePvPLiveEventData::~DB_UpdatePvPLiveEventData()
// IDA_EA: 0x845C886
// IDA_NAME: _ZN25DB_UpdatePvPLiveEventDataD0Ev
// DEMANGLED: DB_UpdatePvPLiveEventData::~DB_UpdatePvPLiveEventData()
DB_UpdatePvPLiveEventData::~DB_UpdatePvPLiveEventData() {}
// IDA_EA: 0x843AC48
// IDA_NAME: _ZN25DB_UpdatePvPLiveEventData8dispatchEiiP6Stream
// DEMANGLED: DB_UpdatePvPLiveEventData::dispatch(int,int,Stream *)
bool DB_UpdatePvPLiveEventData::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal; (void)user_idx; (void)stream;
    return true;  // Exact old binary empty leaf.
}

// IDA_EA: 0x844E7E2
// IDA_NAME: _ZN24DB_InsertDungeonEntranceC2Ev
// DEMANGLED: DB_InsertDungeonEntrance::DB_InsertDungeonEntrance(void)
DB_InsertDungeonEntrance::DB_InsertDungeonEntrance() {}
// IDA_EA: 0x845E6CE
// IDA_NAME: _ZN24DB_InsertDungeonEntranceD2Ev
// DEMANGLED: DB_InsertDungeonEntrance::~DB_InsertDungeonEntrance()
// IDA_EA: 0x845E6FE
// IDA_NAME: _ZN24DB_InsertDungeonEntranceD0Ev
// DEMANGLED: DB_InsertDungeonEntrance::~DB_InsertDungeonEntrance()
DB_InsertDungeonEntrance::~DB_InsertDungeonEntrance() {}
// IDA_EA: 0x8429696
// IDA_NAME: _ZN24DB_InsertDungeonEntrance8dispatchEiiP6Stream
// DEMANGLED: DB_InsertDungeonEntrance::dispatch(int,int,Stream *)
bool DB_InsertDungeonEntrance::dispatch(int signal, int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    int count = 0;
    *stream >> count;
    const int occur_time = GlobalData::system_time()->getCurSec();
    const int channel_no = G_CEnvironment()->get_channel_no();
    std::string values;
    char row[1024];
    for (int index = 0; index < count; ++index) {
        int dungeon_index = 0;
        int enter_count = 0;
        int exercise_count = 0;
        *stream >> dungeon_index;
        *stream >> enter_count;
        *stream >> exercise_count;
        std::sprintf(row, values.empty() ? "(%d,from_unixtime(%d),%d,%d,%d)"
                                         : ",(%d,from_unixtime(%d),%d,%d,%d)",
                     channel_no, occur_time, dungeon_index, enter_count,
                     exercise_count);
        values += row;
    }
    db->set_query(
        "inSert into log_dungeon_entrance(channel_no,occ_date,dungeon_index,enter_count, exercise_count) values%s",
        values.c_str());
    return db->exec(true) == true;
}

// IDA_EA: 0x844E7FE
// IDA_NAME: _ZN29DB_InsertDungeonEntrance_hourC2Ev
// DEMANGLED: DB_InsertDungeonEntrance_hour::DB_InsertDungeonEntrance_hour(void)
DB_InsertDungeonEntrance_hour::DB_InsertDungeonEntrance_hour() {}
// IDA_EA: 0x845E680
// IDA_NAME: _ZN29DB_InsertDungeonEntrance_hourD2Ev
// DEMANGLED: DB_InsertDungeonEntrance_hour::~DB_InsertDungeonEntrance_hour()
// IDA_EA: 0x845E6B0
// IDA_NAME: _ZN29DB_InsertDungeonEntrance_hourD0Ev
// DEMANGLED: DB_InsertDungeonEntrance_hour::~DB_InsertDungeonEntrance_hour()
DB_InsertDungeonEntrance_hour::~DB_InsertDungeonEntrance_hour() {}
// IDA_EA: 0x84298A2
// IDA_NAME: _ZN29DB_InsertDungeonEntrance_hour8dispatchEiiP6Stream
// DEMANGLED: DB_InsertDungeonEntrance_hour::dispatch(int,int,Stream *)
bool DB_InsertDungeonEntrance_hour::dispatch(int signal, int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    int count = 0;
    *stream >> count;
    const int occur_time = GlobalData::system_time()->getCurSec();
    const int channel_no = G_CEnvironment()->get_channel_no();
    std::string values;
    char row[1024];
    for (int index = 0; index < count; ++index) {
        int dungeon_index = 0;
        int hour_enter_count = 0;
        int cnt_enter_count = 0;
        *stream >> dungeon_index;
        *stream >> hour_enter_count;
        *stream >> cnt_enter_count;
        if (hour_enter_count == 0) {
            continue;
        }
        std::sprintf(row, values.empty() ? "(%d,from_unixtime(%d),%d,%d,%d)"
                                         : ",(%d,from_unixtime(%d),%d,%d,%d)",
                     channel_no, occur_time, dungeon_index, hour_enter_count,
                     cnt_enter_count);
        values += row;
    }
    if (values.empty()) {
        return true;
    }
    db->set_query(
        "inSert into log_dungeon_entrance_hour(channel_no,occ_date,dungeon_index,hour_enter_count, cnt_enter_count) values %s",
        values.c_str());
    return db->exec(true) == true;
}

// IDA_EA: 0x844EB0E
// IDA_NAME: _ZN19DB_InsertDungeonExpC2Ev
// DEMANGLED: DB_InsertDungeonExp::DB_InsertDungeonExp(void)
DB_InsertDungeonExp::DB_InsertDungeonExp() {}
// IDA_EA: 0x845DF30
// IDA_NAME: _ZN19DB_InsertDungeonExpD2Ev
// DEMANGLED: DB_InsertDungeonExp::~DB_InsertDungeonExp()
// IDA_EA: 0x845DF60
// IDA_NAME: _ZN19DB_InsertDungeonExpD0Ev
// DEMANGLED: DB_InsertDungeonExp::~DB_InsertDungeonExp()
DB_InsertDungeonExp::~DB_InsertDungeonExp() {}
// IDA_EA: 0x842E944
// IDA_NAME: _ZN19DB_InsertDungeonExp8dispatchEiiP6Stream
// DEMANGLED: DB_InsertDungeonExp::dispatch(int,int,Stream *)
bool DB_InsertDungeonExp::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    const int occur_time = GlobalData::system_time()->getCurSec();
    int count = 0;
    *stream >> count;
    std::string values;
    char row[1024];
    for (int index = 0; index < count; ++index) {
        int level = 0;
        int exp = 0;
        int fatigue = 0;
        int user_count = 0;
        *stream >> level;
        *stream >> exp;
        *stream >> fatigue;
        *stream >> user_count;
        std::sprintf(row, "(from_unixtime(%d),%d,%d,%d,%d,%d)",
                     occur_time, G_CEnvironment()->get_channel_no(), level,
                     exp, fatigue, user_count);
        values += row;
        if (index + 1 < count) {
            values += ",";
        }
    }
    db->set_query(
        "inSert into log_dungeon_exp(occ_time,channel_no,lev,exp,fatigue,user_count) values%s",
        values.c_str());
    return db->exec(true) == true;
}

// IDA_EA: 0x844E9F6
// IDA_NAME: _ZN25DB_InsertLevelDungeonPlayC2Ev
// DEMANGLED: DB_InsertLevelDungeonPlay::DB_InsertLevelDungeonPlay(void)
DB_InsertLevelDungeonPlay::DB_InsertLevelDungeonPlay() {}
// IDA_EA: 0x845E152
// IDA_NAME: _ZN25DB_InsertLevelDungeonPlayD2Ev
// DEMANGLED: DB_InsertLevelDungeonPlay::~DB_InsertLevelDungeonPlay()
// IDA_EA: 0x845E182
// IDA_NAME: _ZN25DB_InsertLevelDungeonPlayD0Ev
// DEMANGLED: DB_InsertLevelDungeonPlay::~DB_InsertLevelDungeonPlay()
DB_InsertLevelDungeonPlay::~DB_InsertLevelDungeonPlay() {}
// IDA_EA: 0x842CC98
// IDA_NAME: _ZN25DB_InsertLevelDungeonPlay8dispatchEiiP6Stream
// DEMANGLED: DB_InsertLevelDungeonPlay::dispatch(int,int,Stream *)
bool DB_InsertLevelDungeonPlay::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    int count = 0;
    *stream >> count;
    for (int index = 0; index < count; ++index) {
        int level = 0;
        int charac_count = 0;
        int dungeon_play_count = 0;
        *stream >> level;
        *stream >> charac_count;
        *stream >> dungeon_play_count;
        db->set_query(
            "upDate log_level_dungeon_play_count set total_charac_count=total_charac_count+%d,total_dungeon_play_count=total_dungeon_play_count+%d where level=%d",
            charac_count, dungeon_play_count, level);
        if (db->exec(true) == true && db->getAffectedRowCount() != 0) {
            continue;
        }
        db->set_query(
            "inSert into log_level_dungeon_play_count (level,total_charac_count,total_dungeon_play_count) values(%d,%d,%d)",
            level, charac_count, dungeon_play_count);
        if (db->exec(true) != true) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x844F182
// IDA_NAME: _ZN29DB_UpdateMemberPlayInfoPcRoomC2Ev
// DEMANGLED: DB_UpdateMemberPlayInfoPcRoom::DB_UpdateMemberPlayInfoPcRoom(void)
DB_UpdateMemberPlayInfoPcRoom::DB_UpdateMemberPlayInfoPcRoom() {}
// IDA_EA: 0x845CBFE
// IDA_NAME: _ZN29DB_UpdateMemberPlayInfoPcRoomD2Ev
// DEMANGLED: DB_UpdateMemberPlayInfoPcRoom::~DB_UpdateMemberPlayInfoPcRoom()
// IDA_EA: 0x845CC2E
// IDA_NAME: _ZN29DB_UpdateMemberPlayInfoPcRoomD0Ev
// DEMANGLED: DB_UpdateMemberPlayInfoPcRoom::~DB_UpdateMemberPlayInfoPcRoom()
DB_UpdateMemberPlayInfoPcRoom::~DB_UpdateMemberPlayInfoPcRoom() {}
// IDA_EA: 0x8439BE2
// IDA_NAME: _ZN29DB_UpdateMemberPlayInfoPcRoom8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateMemberPlayInfoPcRoom::dispatch(int,int,Stream *)
bool DB_UpdateMemberPlayInfoPcRoom::dispatch(int signal, int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned int member_id = 0;
    *stream >> member_id;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG_REF);
    db->set_query(
        "upDate member_play_info set pcbang_flag=1 where occ_date=now() and m_id=%s",
        NumberToString(member_id, 0));
    return db->exec(true) == true;
}

// IDA_EA: 0x844F20E
// IDA_NAME: _ZN30DB_SaveProperDungeonClearCountC2Ev
// DEMANGLED: DB_SaveProperDungeonClearCount::DB_SaveProperDungeonClearCount(void)
DB_SaveProperDungeonClearCount::DB_SaveProperDungeonClearCount() {}
// IDA_EA: 0x845CA78
// IDA_NAME: _ZN30DB_SaveProperDungeonClearCountD2Ev
// DEMANGLED: DB_SaveProperDungeonClearCount::~DB_SaveProperDungeonClearCount()
// IDA_EA: 0x845CAA8
// IDA_NAME: _ZN30DB_SaveProperDungeonClearCountD0Ev
// DEMANGLED: DB_SaveProperDungeonClearCount::~DB_SaveProperDungeonClearCount()
DB_SaveProperDungeonClearCount::~DB_SaveProperDungeonClearCount() {}
// IDA_EA: 0x843A8B2
// IDA_NAME: _ZN30DB_SaveProperDungeonClearCount8dispatchEiiP6Stream
// DEMANGLED: DB_SaveProperDungeonClearCount::dispatch(int,int,Stream *)
bool DB_SaveProperDungeonClearCount::dispatch(int signal, int user_idx,
                                              Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_SAVE_BLOOD_CLEAR_COUNT* const payload =
        stream->GetOutBuffer<SIG_SAVE_BLOOD_CLEAR_COUNT>();
    MySQL* const db = GetLeafDB(DB_HANDLE_2);
    db->set_query(
        "upDate event_dungeon_clear set clear_cnt=%d, update_time=now() where charac_no=%u",
        payload->clear_count, payload->charac_no);
    if (db->exec(true) != true) {
        return false;
    }
    if (db->getAffectedRowCount() != 0) {
        return true;
    }
    db->set_query(
        "inSert into event_dungeon_clear(charac_no, clear_cnt, update_time) values(%u, %d, now())",
        payload->charac_no, payload->clear_count);
    return db->exec(true) == true;
}

// IDA_EA: 0x844FE6A
// IDA_NAME: _ZN20DB_TestTradeRestrictC2Ev
// DEMANGLED: DB_TestTradeRestrict::DB_TestTradeRestrict(void)
DB_TestTradeRestrict::DB_TestTradeRestrict() {}
// IDA_EA: 0x845AC4E
// IDA_NAME: _ZN20DB_TestTradeRestrictD2Ev
// DEMANGLED: DB_TestTradeRestrict::~DB_TestTradeRestrict()
// IDA_EA: 0x845AC7E
// IDA_NAME: _ZN20DB_TestTradeRestrictD0Ev
// DEMANGLED: DB_TestTradeRestrict::~DB_TestTradeRestrict()
DB_TestTradeRestrict::~DB_TestTradeRestrict() {}
// IDA_EA: 0x844A0BE
// IDA_NAME: _ZN20DB_TestTradeRestrict8dispatchEiiP6Stream
// DEMANGLED: DB_TestTradeRestrict::dispatch(int,int,Stream *)
bool DB_TestTradeRestrict::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned int charac_no = 0;
    *stream >> charac_no;
    MySQL* const db = GetLeafDB(DB_HANDLE_3);
    db->set_query(
        "upDate user_items set trade_restrict = 1 where charac_no = %u",
        charac_no);
    db->exec(true);
    return true;  // Original leaf ignores the MySQL result.
}
// IDA_EA: 0x844A132
// IDA_NAME: _ZN20DB_TestTradeRestrict11makeRequestEj
// DEMANGLED: DB_TestTradeRestrict::makeRequest(unsigned int)
void DB_TestTradeRestrict::makeRequest(unsigned int charac_no) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 47731);
    CStreamGuard guard(stream, true);
    *guard << 796;
    *guard << -1;
    *guard << charac_no;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844DA6E
// IDA_NAME: _ZN16DB_GenCeraTicketC2Ev
// DEMANGLED: DB_GenCeraTicket::DB_GenCeraTicket(void)
DB_GenCeraTicket::DB_GenCeraTicket() {}
// IDA_EA: 0x844DA8A
// IDA_NAME: _ZN16DB_GenCeraTicketD2Ev
// DEMANGLED: DB_GenCeraTicket::~DB_GenCeraTicket()
// IDA_EA: 0x844DABA
// IDA_NAME: _ZN16DB_GenCeraTicketD0Ev
// DEMANGLED: DB_GenCeraTicket::~DB_GenCeraTicket()
DB_GenCeraTicket::~DB_GenCeraTicket() {}
// IDA_EA: 0x841A2F8
// IDA_NAME: _ZN16DB_GenCeraTicket8dispatchEiiP6Stream
// DEMANGLED: DB_GenCeraTicket::dispatch(int,int,Stream *)
bool DB_GenCeraTicket::dispatch(int signal, int user_idx, Stream* stream) {
    unsigned int member_id = 0;
    int ticket_key = 0;
    char ticket_id[32] = {};
    *stream >> member_id;
    *stream >> ticket_key;
    *stream >> ticket_id;

    MySQL* const db = GetLeafDB(DB_HANDLE_3);
    db->set_query(
        "inSert into user_cera_ticket(m_id,ticket_id,ticketKey,expire_time,delete_flag,update_time) values(%s,'%s',%d,adddate(now(),interval %d hour), 0, now())",
        NumberToString(member_id, 0), ticket_id, ticket_key, 3);
    if (db->exec(true) != true) {
        db->set_query(
            "upDate user_cera_ticket set ticket_id='%s',ticketKey=%d,expire_time=adddate(now(),interval %d hour), delete_flag=%d, update_time=now() where m_id=%s",
            ticket_id, ticket_key, 3, 0, NumberToString(member_id, 0));
        if (db->exec(true) != true) {
            ticket_key = 0;
        }
    }

    Stream* const response =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 19565);
    CStreamGuard guard(response, true);
    *guard << signal;
    *guard << user_idx;
    *guard << static_cast<char>(ticket_key);
    *guard << ticket_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;  // Exact old DB worker replies even after an SQL failure.
}

// IDA_EA: 0x844E772
// IDA_NAME: _ZN25DB_UpdateMemberDoubtTradeC2Ev
// DEMANGLED: DB_UpdateMemberDoubtTrade::DB_UpdateMemberDoubtTrade(void)
DB_UpdateMemberDoubtTrade::DB_UpdateMemberDoubtTrade() {}
// IDA_EA: 0x845E806
// IDA_NAME: _ZN25DB_UpdateMemberDoubtTradeD2Ev
// DEMANGLED: DB_UpdateMemberDoubtTrade::~DB_UpdateMemberDoubtTrade()
// IDA_EA: 0x845E836
// IDA_NAME: _ZN25DB_UpdateMemberDoubtTradeD0Ev
// DEMANGLED: DB_UpdateMemberDoubtTrade::~DB_UpdateMemberDoubtTrade()
DB_UpdateMemberDoubtTrade::~DB_UpdateMemberDoubtTrade() {}
// IDA_EA: 0x84284A2
// IDA_NAME: _ZN25DB_UpdateMemberDoubtTrade8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateMemberDoubtTrade::dispatch(int,int,Stream *)
bool DB_UpdateMemberDoubtTrade::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned int member_id = 0;
    *stream >> member_id;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG_REF);
    const int occur_time = GlobalData::system_time()->getCurSec();
    db->set_query(
        "upDate member_doubt_trade set last_update_time=from_unixtime(%d),over_count=over_count+1 where m_id=%s",
        occur_time, NumberToString(member_id, 0));
    if (db->exec(true) == true && db->getAffectedRowCount() != 0) {
        return true;
    }
    db->set_query(
        "inSert into member_doubt_trade(m_id,last_update_time,over_count) values(%s,from_unixtime(%d),1)",
        NumberToString(member_id, 0), occur_time);
    return db->exec(true) == true;
}

// IDA_EA: 0x844F6A6
// IDA_NAME: _ZN37DB_UpdateGoblinPadAdvicePwdRewardTimeC2Ev
// DEMANGLED: DB_UpdateGoblinPadAdvicePwdRewardTime::DB_UpdateGoblinPadAdvicePwdRewardTime(void)
DB_UpdateGoblinPadAdvicePwdRewardTime::DB_UpdateGoblinPadAdvicePwdRewardTime() {}
// IDA_EA: 0x845BE96
// IDA_NAME: _ZN37DB_UpdateGoblinPadAdvicePwdRewardTimeD2Ev
// DEMANGLED: DB_UpdateGoblinPadAdvicePwdRewardTime::~DB_UpdateGoblinPadAdvicePwdRewardTime()
// IDA_EA: 0x845BEC6
// IDA_NAME: _ZN37DB_UpdateGoblinPadAdvicePwdRewardTimeD0Ev
// DEMANGLED: DB_UpdateGoblinPadAdvicePwdRewardTime::~DB_UpdateGoblinPadAdvicePwdRewardTime()
DB_UpdateGoblinPadAdvicePwdRewardTime::~DB_UpdateGoblinPadAdvicePwdRewardTime() {}
// IDA_EA: 0x843F5D2
// IDA_NAME: _ZN37DB_UpdateGoblinPadAdvicePwdRewardTime8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateGoblinPadAdvicePwdRewardTime::dispatch(int,int,Stream *)
bool DB_UpdateGoblinPadAdvicePwdRewardTime::dispatch(int signal, int user_idx,
                                                      Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned int member_id = 0;
    *stream >> member_id;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG_REF);
    db->set_query(
        "upDate member_mousepass set reward_time=0 where m_id=%s and enable_flag = '1'",
        NumberToString(member_id, 0));
    return db->exec(true) == true && db->getAffectedRowCount() != 0;
}
// IDA_EA: 0x843F684
// IDA_NAME: _ZN37DB_UpdateGoblinPadAdvicePwdRewardTime11makeRequestEij
// DEMANGLED: DB_UpdateGoblinPadAdvicePwdRewardTime::makeRequest(int,unsigned int)
void DB_UpdateGoblinPadAdvicePwdRewardTime::makeRequest(int user_idx,
                                                        unsigned int member_id) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 39207);
    CStreamGuard guard(stream, true);
    *guard << 583;
    *guard << user_idx;
    *guard << member_id;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844F9D2
// IDA_NAME: _ZN26DB_UpdateFatigueAttendanceC2Ev
// DEMANGLED: DB_UpdateFatigueAttendance::DB_UpdateFatigueAttendance(void)
DB_UpdateFatigueAttendance::DB_UpdateFatigueAttendance() {}
// IDA_EA: 0x845B6AA
// IDA_NAME: _ZN26DB_UpdateFatigueAttendanceD2Ev
// DEMANGLED: DB_UpdateFatigueAttendance::~DB_UpdateFatigueAttendance()
// IDA_EA: 0x845B6DA
// IDA_NAME: _ZN26DB_UpdateFatigueAttendanceD0Ev
// DEMANGLED: DB_UpdateFatigueAttendance::~DB_UpdateFatigueAttendance()
DB_UpdateFatigueAttendance::~DB_UpdateFatigueAttendance() {}
// IDA_EA: 0x84444EC
// IDA_NAME: _ZN26DB_UpdateFatigueAttendance8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateFatigueAttendance::dispatch(int,int,Stream *)
bool DB_UpdateFatigueAttendance::dispatch(int signal, int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned int member_id = 0;
    int entry_count = 0;
    std::map<unsigned char, short> fatigue_by_day;
    *stream >> member_id;
    *stream >> entry_count;
    for (int index = 0; index < entry_count; ++index) {
        unsigned char day = 0;
        short fatigue = 0;
        *stream >> day;
        *stream >> fatigue;
        fatigue_by_day.insert(std::make_pair(day, fatigue));
    }

    CDataManager* const data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* const event_manager =
        data_manager ? data_manager->get_event_script_mng() : 0;
    const std::string* const table_name =
        event_manager ? event_manager->get_db_table_name(86) : 0;
    if (!table_name) {
        return false;
    }

    MySQL* const db = GetLeafDB(DB_HANDLE_LOGIN);
    for (std::map<unsigned char, short>::const_iterator it =
             fatigue_by_day.begin();
         it != fatigue_by_day.end(); ++it) {
        if (it->second <= 0) {
            continue;
        }
        db->set_query(
            "upDate %s set fatigue = fatigue + %d where occ_day = %d and m_id = %s",
            toTString(*table_name), it->second, it->first,
            NumberToString(member_id, 0));
        if (db->exec(true) == true && db->getAffectedRowCount() != 0) {
            continue;
        }
        db->set_query(
            "inSert into %s(occ_day,m_id,fatigue) values(%d,%s,%d)",
            toTString(*table_name), it->first, NumberToString(member_id, 0),
            it->second);
        db->exec(true);  // Original dispatcher ignores this insertion result.
    }
    return true;
}
// IDA_EA: 0x84447D6
// IDA_NAME: _ZN26DB_UpdateFatigueAttendance11makeRequestEjRKSt3mapIhsSt4lessIhESaISt4pairIKhsEEE
// DEMANGLED: DB_UpdateFatigueAttendance::makeRequest(unsigned int,std::map<unsigned char,short,std::less<unsigned char>,std::allocator<std::pair<unsigned char const,short>>> const&)
void DB_UpdateFatigueAttendance::makeRequest(
    unsigned int member_id,
    const std::map<unsigned char, short>& fatigue_by_day) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 43890);
    CStreamGuard guard(stream, true);
    *guard << 684;
    *guard << -1;
    *guard << member_id;
    *guard << static_cast<char>(fatigue_by_day.size());
    for (std::map<unsigned char, short>::const_iterator it =
             fatigue_by_day.begin();
         it != fatigue_by_day.end(); ++it) {
        *guard << it->first;
        *guard << it->second;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844F0DA
// IDA_NAME: _ZN25DB_SaveConditionEventInfoC2Ev
// DEMANGLED: DB_SaveConditionEventInfo::DB_SaveConditionEventInfo(void)
DB_SaveConditionEventInfo::DB_SaveConditionEventInfo() {}
// IDA_EA: 0x845D090
// IDA_NAME: _ZN25DB_SaveConditionEventInfoD2Ev
// DEMANGLED: DB_SaveConditionEventInfo::~DB_SaveConditionEventInfo()
// IDA_EA: 0x845D0C0
// IDA_NAME: _ZN25DB_SaveConditionEventInfoD0Ev
// DEMANGLED: DB_SaveConditionEventInfo::~DB_SaveConditionEventInfo()
DB_SaveConditionEventInfo::~DB_SaveConditionEventInfo() {}
// IDA_EA: 0x8437930
// IDA_NAME: _ZN25DB_SaveConditionEventInfo8dispatchEiiP6Stream
// DEMANGLED: DB_SaveConditionEventInfo::dispatch(int,int,Stream *)
bool DB_SaveConditionEventInfo::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_SAVE_CONDITION_EVENT* const payload =
        stream->GetOutBuffer<SIG_SAVE_CONDITION_EVENT>();
    MySQL* const db = GetLeafDB(DB_HANDLE_2);
    db->set_query(
        "upDate event_conditionable_info set current_step=%d, reward_step=%d, update_time=now() where charac_no=%u",
        payload->current_step, payload->reward_step, payload->charac_no);
    if (db->exec(true) != true) {
        return false;
    }
    if (db->getAffectedRowCount() != 0) {
        return true;
    }
    db->set_query(
        "inSert into event_conditionable_info(charac_no, current_step, reward_step, update_time) values(%u, %d, %d, now())",
        payload->charac_no, payload->current_step, payload->reward_step);
    return db->exec(true) == true;
}

// IDA_EA: 0x844EAD6
// IDA_NAME: _ZN30DB_Insert0712WinterEventCouponC2Ev
// DEMANGLED: DB_Insert0712WinterEventCoupon::DB_Insert0712WinterEventCoupon(void)
DB_Insert0712WinterEventCoupon::DB_Insert0712WinterEventCoupon() {}
// IDA_EA: 0x845DFCC
// IDA_NAME: _ZN30DB_Insert0712WinterEventCouponD2Ev
// DEMANGLED: DB_Insert0712WinterEventCoupon::~DB_Insert0712WinterEventCoupon()
// IDA_EA: 0x845DFFC
// IDA_NAME: _ZN30DB_Insert0712WinterEventCouponD0Ev
// DEMANGLED: DB_Insert0712WinterEventCoupon::~DB_Insert0712WinterEventCoupon()
DB_Insert0712WinterEventCoupon::~DB_Insert0712WinterEventCoupon() {}
// IDA_EA: 0x842E45E
// IDA_NAME: _ZN30DB_Insert0712WinterEventCoupon8dispatchEiiP6Stream
// DEMANGLED: DB_Insert0712WinterEventCoupon::dispatch(int,int,Stream *)
bool DB_Insert0712WinterEventCoupon::dispatch(int signal, int user_idx,
                                              Stream* stream) {
    (void)user_idx;
    SIG_INSERT_0712_WINTER_EVENT_COUPON* const payload =
        stream->GetOutBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>();
    if (signal == 274) {
        MySQL* const db = GetLeafDB(DB_HANDLE_1);
        db->set_query(
            "upDate event_goldcard_info set coupon=coupon+%d where m_id=%s",
            payload->coupon_count, NumberToString(payload->member_id, 0));
        if (db->exec(true) != true || db->getAffectedRowCount() == 0) {
            db->set_query(
                "inSert into event_goldcard_info(m_id,coupon) values(%s,%d)",
                NumberToString(payload->member_id, 0), payload->coupon_count);
            db->exec(true);
        }
    } else {
        MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
        db->set_query(
            "upDate event_goldcard_log set cnt=cnt+%d where m_id=%s and occ_date=now()",
            payload->coupon_count, NumberToString(payload->member_id, 0));
        if (db->exec(true) != true || db->getAffectedRowCount() == 0) {
            db->set_query(
                "inSert into event_goldcard_log(m_id,cnt,occ_date) values(%s,%d,now())",
                NumberToString(payload->member_id, 0), payload->coupon_count);
            db->exec(true);
        }
    }
    return true;  // Exact old binary leaf ignores update/insert failure.
}
// IDA_EA: 0x842E648
// IDA_NAME: _ZN30DB_Insert0712WinterEventCoupon11makeRequestEjjj
// DEMANGLED: DB_Insert0712WinterEventCoupon::makeRequest(unsigned int,unsigned int,unsigned int)
Stream* DB_Insert0712WinterEventCoupon::makeRequest(
    unsigned int member_id,
    unsigned int charac_no,
    unsigned int coupon_count) {
    Stream* result = _makeRequest(static_cast<ENUM_INTERNALPACKET>(18),
                                  member_id, charac_no, coupon_count);
    if (!result) {
        return 0;
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, result);
    result = _makeRequest(static_cast<ENUM_INTERNALPACKET>(19), member_id,
                          charac_no, coupon_count);
    if (result) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, result);
    }
    return result;
}
// IDA_EA: 0x842E6E0
// IDA_NAME: _ZN30DB_Insert0712WinterEventCoupon12_makeRequestE19ENUM_INTERNALPACKETjjj
// DEMANGLED: DB_Insert0712WinterEventCoupon::_makeRequest(ENUM_INTERNALPACKET,unsigned int,unsigned int,unsigned int)
Stream* DB_Insert0712WinterEventCoupon::_makeRequest(
    ENUM_INTERNALPACKET signal,
    unsigned int member_id,
    unsigned int charac_no,
    unsigned int coupon_count) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 29489);
    CStreamGuard guard(stream, false);
    *guard << static_cast<int>(signal);
    *guard << -1;
    SIG_INSERT_0712_WINTER_EVENT_COUPON* const payload =
        guard.Get()->GetInBuffer<SIG_INSERT_0712_WINTER_EVENT_COUPON>();
    payload->member_id = member_id;
    payload->charac_no = charac_no;
    payload->coupon_count = coupon_count;
    return guard.Get();
}

// IDA_EA: 0x844EBB6
// IDA_NAME: _ZN24DB_GoblinPadUpdateRewardC2Ev
// DEMANGLED: DB_GoblinPadUpdateReward::DB_GoblinPadUpdateReward(void)
DB_GoblinPadUpdateReward::DB_GoblinPadUpdateReward() {}
// IDA_EA: 0x845EF08
// IDA_NAME: _ZN24DB_GoblinPadUpdateRewardD2Ev
// DEMANGLED: DB_GoblinPadUpdateReward::~DB_GoblinPadUpdateReward()
// IDA_EA: 0x845EF38
// IDA_NAME: _ZN24DB_GoblinPadUpdateRewardD0Ev
// DEMANGLED: DB_GoblinPadUpdateReward::~DB_GoblinPadUpdateReward()
DB_GoblinPadUpdateReward::~DB_GoblinPadUpdateReward() {}
// IDA_EA: 0x842413C
// IDA_NAME: _ZN24DB_GoblinPadUpdateReward8dispatchEiiP6Stream
// DEMANGLED: DB_GoblinPadUpdateReward::dispatch(int,int,Stream *)
bool DB_GoblinPadUpdateReward::dispatch(int signal, int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG_REF);
    SIG_GOBLIN_UPDATE_REWARD* const payload =
        stream->GetOutBuffer<SIG_GOBLIN_UPDATE_REWARD>();
    char previous_mousepass[36] = {};
    db->set_query("seLect mousepass from member_mousepass where m_id=%s",
                  NumberToString(payload->member_id, 0));
    if (db->exec(true) != true || db->get_n_rows() == 0 ||
        db->fetch() != true ||
        db->get_str(0, previous_mousepass, sizeof(previous_mousepass)) != true) {
        return false;
    }

    std::tm local_time;
    std::time_t now = GlobalData::system_time()->getCurSec();
    ::localtime_r(&now, &local_time);
    char month[10] = {};
    std::strftime(month, sizeof(month), "%Y%m", &local_time);

    int modify_type = 16;
    if (payload->modification_type == 2 || !payload->reward) {
        modify_type = 15;
    } else {
        db->set_query(
            "upDate member_mousepass set reward_time = unix_timestamp(now()) where m_id=%s and enable_flag = '1'",
            NumberToString(payload->member_id, 0));
        if (db->exec(true) != true || db->getAffectedRowCount() == 0) {
            return false;
        }
    }
    db->set_query(
        "inSert into member_mousepass_history_%s (occ_time, m_id, pre_mousepass, modify_type, ip_info, port_info) values (now(),%s,'%s',%d,'%s','')",
        month, NumberToString(payload->member_id, 0), previous_mousepass,
        modify_type, payload->ip_info);
    return db->exec(true) == true && db->getAffectedRowCount() != 0;
}
// IDA_EA: 0x8424452
// IDA_NAME: _ZN24DB_GoblinPadUpdateReward11makeRequestEjcbPc
// DEMANGLED: DB_GoblinPadUpdateReward::makeRequest(unsigned int,char,bool,char *)
void DB_GoblinPadUpdateReward::makeRequest(unsigned int member_id,
                                           char modification_type,
                                           bool reward,
                                           char* ip_info) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 24515);
    CStreamGuard guard(stream, true);
    *guard << 299;
    *guard << -1;
    SIG_GOBLIN_UPDATE_REWARD* const payload =
        guard.Get()->GetInBuffer<SIG_GOBLIN_UPDATE_REWARD>();
    payload->member_id = member_id;
    payload->modification_type = modification_type;
    payload->reward = reward;
    std::strncpy(payload->ip_info, ip_info, sizeof(payload->ip_info));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844EAF2
// IDA_NAME: _ZN25DB_InsertArchieveEventLogC2Ev
// DEMANGLED: DB_InsertArchieveEventLog::DB_InsertArchieveEventLog(void)
DB_InsertArchieveEventLog::DB_InsertArchieveEventLog() {}
// IDA_EA: 0x845DF7E
// IDA_NAME: _ZN25DB_InsertArchieveEventLogD2Ev
// DEMANGLED: DB_InsertArchieveEventLog::~DB_InsertArchieveEventLog()
// IDA_EA: 0x845DFAE
// IDA_NAME: _ZN25DB_InsertArchieveEventLogD0Ev
// DEMANGLED: DB_InsertArchieveEventLog::~DB_InsertArchieveEventLog()
DB_InsertArchieveEventLog::~DB_InsertArchieveEventLog() {}
// IDA_EA: 0x842E7BE
// IDA_NAME: _ZN25DB_InsertArchieveEventLog8dispatchEiiP6Stream
// DEMANGLED: DB_InsertArchieveEventLog::dispatch(int,int,Stream *)
bool DB_InsertArchieveEventLog::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_INSERT_ACHIEVE_EVENT* const payload =
        stream->GetOutBuffer<SIG_INSERT_ACHIEVE_EVENT>();
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    db->set_query(
        "inSert into log_charac_lev_log(occ_time,m_id,charac_no,achieve_type) values(now(),%s,%d,%d)",
        NumberToString(payload->member_id, 0), payload->charac_no,
        static_cast<int>(payload->achieve_type));
    db->exec(true);
    return true;  // Original log leaf ignores the MySQL result.
}
// IDA_EA: 0x842E856
// IDA_NAME: _ZN25DB_InsertArchieveEventLog11makeRequestEjjN24SIG_INSERT_ACHIEVE_EVENT17ENUM_ACHIEVE_TYPEE
// DEMANGLED: DB_InsertArchieveEventLog::makeRequest(unsigned int,unsigned int,SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE)
void DB_InsertArchieveEventLog::makeRequest(
    unsigned int member_id,
    unsigned int charac_no,
    SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE achieve_type) {
    Stream* const stream =
        GlobalData::stream_pool()->Acquire("DBThread.cpp", 29513);
    CStreamGuard guard(stream, true);
    *guard << 279;
    *guard << -1;
    SIG_INSERT_ACHIEVE_EVENT* const payload =
        guard.Get()->GetInBuffer<SIG_INSERT_ACHIEVE_EVENT>();
    payload->member_id = member_id;
    payload->charac_no = charac_no;
    payload->achieve_type = achieve_type;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x844FF12
// IDA_NAME: _ZN19DB_LogCeraShopErrorC2Ev
// DEMANGLED: DB_LogCeraShopError::DB_LogCeraShopError(void)
DB_LogCeraShopError::DB_LogCeraShopError() {}
// IDA_EA: 0x845A910
// IDA_NAME: _ZN19DB_LogCeraShopErrorD2Ev
// DEMANGLED: DB_LogCeraShopError::~DB_LogCeraShopError()
// IDA_EA: 0x845A940
// IDA_NAME: _ZN19DB_LogCeraShopErrorD0Ev
// DEMANGLED: DB_LogCeraShopError::~DB_LogCeraShopError()
DB_LogCeraShopError::~DB_LogCeraShopError() {}
// IDA_EA: 0x844C40A
// IDA_NAME: _ZN19DB_LogCeraShopError8dispatchEiiP6Stream
// DEMANGLED: DB_LogCeraShopError::dispatch(int,int,Stream *)
bool DB_LogCeraShopError::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    MySQL* const db = GetLeafDB(static_cast<ENUM_DB_HANDLE_IDX>(13));
    if (!db) {
        return false;
    }
    SIG_CERASHOP_ERROR_LOG* const payload =
        stream->GetOutBuffer<SIG_CERASHOP_ERROR_LOG>();
    if (!payload) {
        return false;
    }
    db->set_query(
        "inSert into cerashop_err_log(m_id, charac_no, server_id, channel_no, type, err_code, goods_id, time) values (%u, %u, %u, %u, %d, %u, %d, now())",
        payload->member_id, payload->charac_no, payload->server_id,
        payload->channel_no, payload->type, payload->error_code,
        payload->goods_id);
    return db->exec(true);
}

// IDA_EA: 0x844F90E
// IDA_NAME: _ZN27DB_StatisticCeraShopGiftLogC2Ev
// DEMANGLED: DB_StatisticCeraShopGiftLog::DB_StatisticCeraShopGiftLog(void)
DB_StatisticCeraShopGiftLog::DB_StatisticCeraShopGiftLog() {}
// IDA_EA: 0x845B746
// IDA_NAME: _ZN27DB_StatisticCeraShopGiftLogD2Ev
// DEMANGLED: DB_StatisticCeraShopGiftLog::~DB_StatisticCeraShopGiftLog()
// IDA_EA: 0x845B776
// IDA_NAME: _ZN27DB_StatisticCeraShopGiftLogD0Ev
// DEMANGLED: DB_StatisticCeraShopGiftLog::~DB_StatisticCeraShopGiftLog()
DB_StatisticCeraShopGiftLog::~DB_StatisticCeraShopGiftLog() {}
// IDA_EA: 0x8444066
// IDA_NAME: _ZN27DB_StatisticCeraShopGiftLog8dispatchEiiP6Stream
// DEMANGLED: DB_StatisticCeraShopGiftLog::dispatch(int,int,Stream *)
bool DB_StatisticCeraShopGiftLog::dispatch(int signal, int user_idx,
                                           Stream* stream) {
    (void)signal;
    (void)user_idx;
    SIG_STATISTIC_CERASHOP_GIFT_LOG* const payload =
        stream->GetOutBuffer<SIG_STATISTIC_CERASHOP_GIFT_LOG>();
    MySQL* const db = GetLeafDB(DB_HANDLE_LOG);
    if (!db) {
        return false;
    }
    db->set_query(
        "inSert into log_cerashop_gift(occ_time, from_mid, from_charac_no, to_mid, to_charac_no, item_idx, item_cnt) values(unix_timestamp(), %u, %u, %u, %u, %u, %u)",
        payload->from_member_id, payload->from_charac_no,
        payload->to_member_id, payload->to_charac_no, payload->item_index,
        payload->item_count);
    if (db->exec(true) == true) {
        return true;
    }
    cMyTrace trace(
        "virtual bool DB_StatisticCeraShopGiftLog::dispatch(int, int, Stream*)",
        43457, 5);
    trace("DB_StatisticAmazingBoxProtectLucky_USE inSert exec fail");
    cMyTrace query_trace(
        "virtual bool DB_StatisticCeraShopGiftLog::dispatch(int, int, Stream*)",
        43459, 5);
    query_trace(
        "inSert into log_cerashop_gift(occ_time, from_mid, from_charac_no, to_m_id, to_charac_no, item_idx, count) values(unix_timestamp(), %u, %u, %u, %u, %u, %u)",
        payload->from_member_id, payload->from_charac_no,
        payload->to_member_id, payload->to_charac_no, payload->item_index,
        payload->item_count);
    return false;
}
