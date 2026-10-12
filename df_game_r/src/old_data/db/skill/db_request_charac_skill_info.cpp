#include "old_data/db/skill/db_request_charac_skill_info.h"

#include "old_data/db/core/mysql.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_common_public_abi.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>


// IDA_EA: 0x848F34C
// IDA_NAME: _ZN15_Mastered_skillC2Ev
// DEMANGLED: _Mastered_skill::_Mastered_skill(void)
_Mastered_skill::_Mastered_skill()
    : skill_index(0),
      skill_level(0) {}

namespace {

enum {
    kRequestCharacSkillInfoInterSignal = 765,
    kRequestCharacSkillInfoLineTreeExec = 45466,
    kRequestCharacSkillInfoLineTreeFetch = 45475,
    kRequestCharacSkillInfoLineTreeNoResult = 45482,
    kRequestCharacSkillInfoLineTreeGet = 45490,
    kRequestCharacSkillInfoLineSkillExec = 45509,
    kRequestCharacSkillInfoLineSkillFetch = 45518,
    kRequestCharacSkillInfoLineSkillNoResult = 45525,
    kRequestCharacSkillInfoLineSkillBlob = 45536,
    kRequestCharacSkillInfoLineSend = 45570,
};

}  // namespace

// IDA_EA: 0x844FCAA
// IDA_NAME: _ZN25DB_RequestCharacSkillInfoC2Ev
// DEMANGLED: DB_RequestCharacSkillInfo::DB_RequestCharacSkillInfo(void)
DB_RequestCharacSkillInfo::DB_RequestCharacSkillInfo() {}

// IDA_EA: 0x845B12E
// IDA_NAME: _ZN25DB_RequestCharacSkillInfoD2Ev
// DEMANGLED: DB_RequestCharacSkillInfo::~DB_RequestCharacSkillInfo()
// IDA_EA: 0x845B15E
// IDA_NAME: _ZN25DB_RequestCharacSkillInfoD0Ev
// DEMANGLED: DB_RequestCharacSkillInfo::~DB_RequestCharacSkillInfo() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_RequestCharacSkillInfo::~DB_RequestCharacSkillInfo() {}

// IDA_EA: 0x8447162
// IDA_NAME: _ZN25DB_RequestCharacSkillInfo18_GetSkillTreeIndexEjPc
// DEMANGLED: DB_RequestCharacSkillInfo::_GetSkillTreeIndex(unsigned int,char *)
bool DB_RequestCharacSkillInfo::_GetSkillTreeIndex(
    unsigned int charac_no,
    char* skill_tree_index) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect skill_tree_index from charac_info where charac_no=%u",
        charac_no);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillTreeIndex(unsigned int, char*)",
            kRequestCharacSkillInfoLineTreeExec,
            "DB_RequestCharacSkillInfo::_GetSkillTreeIndex exec() failed, charac_no(%d)",
            charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillTreeIndex(unsigned int, char*)",
            kRequestCharacSkillInfoLineTreeFetch,
            "DB_RequestCharacSkillInfo::_GetSkillTreeIndex fetch() failed, charac_no(%d)",
            charac_no);
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillTreeIndex(unsigned int, char*)",
            kRequestCharacSkillInfoLineTreeNoResult,
            "DB_RequestCharacSkillInfo::_GetSkillTreeIndex no result, charac_no(%d)",
            charac_no);
        return false;
    }
    unsigned char stored_skill_tree_index = 0;
    if (db_handle->get_ubyte(0, stored_skill_tree_index) == true) {
        *skill_tree_index = static_cast<char>(stored_skill_tree_index);
        return true;
    }
    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "bool DB_RequestCharacSkillInfo::_GetSkillTreeIndex(unsigned int, char*)",
        kRequestCharacSkillInfoLineTreeGet,
        "DB_RequestCharacSkillInfo::_GetSkillTreeIndex get_byte() failed, charac_no(%d)",
        charac_no);
    return false;
}

// IDA_EA: 0x8447312
// IDA_NAME: _ZN25DB_RequestCharacSkillInfo13_GetSkillInfoEjcP15_Mastered_skill
// DEMANGLED: DB_RequestCharacSkillInfo::_GetSkillInfo(unsigned int,char,_Mastered_skill *)
bool DB_RequestCharacSkillInfo::_GetSkillInfo(
    unsigned int charac_no,
    char skill_tree_index,
    _Mastered_skill* mastered_skill_data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* skill_slot_name = get_skill_slot_name(skill_tree_index);
    db_handle->set_query("seLect %s from skill where charac_no=%d",
                         skill_slot_name,
                         charac_no);
    if (db_handle->exec(true) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillInfo(unsigned int, char, _Mastered_skill*)",
            kRequestCharacSkillInfoLineSkillExec,
            "DB_RequestCharacSkillInfo::_GetSkillInfo exec() failed, charac_no(%d)",
            charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillInfo(unsigned int, char, _Mastered_skill*)",
            kRequestCharacSkillInfoLineSkillFetch,
            "DB_RequestCharacSkillInfo::_GetSkillInfo fetch() failed, charac_no(%d)",
            charac_no);
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "bool DB_RequestCharacSkillInfo::_GetSkillInfo(unsigned int, char, _Mastered_skill*)",
            kRequestCharacSkillInfoLineSkillNoResult,
            "DB_RequestCharacSkillInfo::_GetSkillInfo no result, charac_no(%d)",
            charac_no);
        return false;
    }
    if (get_compressed_blob_data(db_handle,
                                 0,
                                 reinterpret_cast<char*>(mastered_skill_data),
                                 sizeof(_Mastered_skill) * 204,
                                 sizeof(_Mastered_skill)) == 1) {
        return true;
    }
    LogManager::logFormat(
        1,
        "DBThread.cpp",
        "bool DB_RequestCharacSkillInfo::_GetSkillInfo(unsigned int, char, _Mastered_skill*)",
        kRequestCharacSkillInfoLineSkillBlob,
        "DB_RequestCharacSkillInfo::_GetSkillInfo get_compressed_blob_data failed, charac_no(%d)",
        charac_no);
    return false;
}

// IDA_EA: 0x84474F2
// IDA_NAME: _ZN25DB_RequestCharacSkillInfo8dispatchEiiP6Stream
// DEMANGLED: DB_RequestCharacSkillInfo::dispatch(int,int,Stream *)
bool DB_RequestCharacSkillInfo::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;
    SIG_REQUEST_CHARAC_SKILL_INFO* payload =
        stream->GetOutBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>();
    if (!payload) {
        return false;
    }
    if (_GetSkillTreeIndex(
            payload->charac_no, &payload->skill_tree_index) != true) {
        return false;
    }
    if (_GetSkillInfo(payload->charac_no,
                      payload->skill_tree_index,
                      payload->mastered_skills) != true) {
        return false;
    }
    sendResult(user_idx, payload);
    return true;
}

// IDA_EA: 0x84475A0
// IDA_NAME: _ZN25DB_RequestCharacSkillInfo10sendResultEiP29SIG_REQUEST_CHARAC_SKILL_INFO
// DEMANGLED: DB_RequestCharacSkillInfo::sendResult(int,SIG_REQUEST_CHARAC_SKILL_INFO *)
void DB_RequestCharacSkillInfo::sendResult(
    int user_idx,
    SIG_REQUEST_CHARAC_SKILL_INFO* payload) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        kRequestCharacSkillInfoLineSend);
    CStreamGuard guard(stream, true);
    *guard << kRequestCharacSkillInfoInterSignal;
    *guard << user_idx;
    SIG_REQUEST_CHARAC_SKILL_INFO* dest =
        guard->GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>();
    if (dest) {
        memcpy(dest, payload, sizeof(SIG_REQUEST_CHARAC_SKILL_INFO));
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}
