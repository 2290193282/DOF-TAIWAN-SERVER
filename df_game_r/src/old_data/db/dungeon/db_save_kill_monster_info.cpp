#include "old_data/db/dungeon/db_save_kill_monster_info.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"


// IDA_EA: 0x844F7F6
// IDA_NAME: _ZN22DB_SaveKillMonsterInfoC2Ev
// DEMANGLED: DB_SaveKillMonsterInfo::DB_SaveKillMonsterInfo(void)
DB_SaveKillMonsterInfo::DB_SaveKillMonsterInfo() : DBDispatcher() {}

// IDA_EA: 0x845BBD8
// IDA_NAME: _ZN22DB_SaveKillMonsterInfoD2Ev
// DEMANGLED: DB_SaveKillMonsterInfo::~DB_SaveKillMonsterInfo()
// IDA_EA: 0x845BC08
// IDA_NAME: _ZN22DB_SaveKillMonsterInfoD0Ev
// DEMANGLED: DB_SaveKillMonsterInfo::~DB_SaveKillMonsterInfo()
DB_SaveKillMonsterInfo::~DB_SaveKillMonsterInfo() {}

// IDA_EA: 0x8441200
// IDA_NAME: _ZN22DB_SaveKillMonsterInfo8dispatchEiiP6Stream
// DEMANGLED: DB_SaveKillMonsterInfo::dispatch(int,int,Stream *)
bool DB_SaveKillMonsterInfo::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_LOAD_KILL_MONSTER_INFO* const payload =
        stream->GetOutBuffer<SIG_LOAD_KILL_MONSTER_INFO>();
    if (!payload) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* boss_blob = 0;
    char* named_blob = 0;
    char* apc_boss_blob = 0;

    if (put_compressed_blob_data(
            db_handle,
            0,
            reinterpret_cast<char*>(payload->boss_info),
            sizeof(payload->boss_info),
            &boss_blob) != 1) {
        cMyTrace trace(
            "virtual bool DB_SaveKillMonsterInfo::dispatch(int, int, Stream*)",
            40976,
            5);
        trace("DB_SaveKillMonsterInfo::dispatch boss_info compress_zip(%u) ERROR charac_no=%u",
              0,
              payload->charac_no);
        return false;
    }
    if (put_compressed_blob_data(
            db_handle,
            1,
            reinterpret_cast<char*>(payload->named_info),
            sizeof(payload->named_info),
            &named_blob) != 1) {
        cMyTrace trace(
            "virtual bool DB_SaveKillMonsterInfo::dispatch(int, int, Stream*)",
            40986,
            5);
        trace("DB_SaveKillMonsterInfo::dispatch named_info compress_zip(%u) ERROR charac_no=%u",
              1,
              payload->charac_no);
        return false;
    }
    if (put_compressed_blob_data(
            db_handle,
            2,
            reinterpret_cast<char*>(payload->apc_boss_info),
            sizeof(payload->apc_boss_info),
            &apc_boss_blob) != 1) {
        cMyTrace trace(
            "virtual bool DB_SaveKillMonsterInfo::dispatch(int, int, Stream*)",
            40996,
            5);
        trace("DB_SaveKillMonsterInfo::dispatch apc_boss_info compress_zip(%u) ERROR charac_no=%u",
              2,
              payload->charac_no);
        return false;
    }

    db_handle->set_query(
        "upDate charac_kill_monster_info set boss_info='%s', named_info='%s', apc_boss_info='%s' where charac_no=%u",
        boss_blob,
        named_blob,
        apc_boss_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into charac_kill_monster_info(charac_no, boss_info, named_info, apc_boss_info) values(%u, '%s', '%s', '%s')",
        payload->charac_no,
        boss_blob,
        named_blob,
        apc_boss_blob);
    return db_handle->exec(true) == true;
}
