#include "old_data/db/legacy_dispatch/arad_dispatcher.h"

#include "old_data/channel/arad_server_state.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/event/reward_event_request.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <new>

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x81867A4
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_IDatabaseDispatcherC2ENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_IDatabaseDispatcher::Arad_IDatabaseDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
Arad_IDatabaseDispatcher::Arad_IDatabaseDispatcher(ENUM_INTERNALPACKET_JPN code)
    : code_(code) {}

// IDA_EA: 0x81867BC
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_IDatabaseDispatcherD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_IDatabaseDispatcher::~Arad_IDatabaseDispatcher()
// IDA_EA: 0x81867E2
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_IDatabaseDispatcherD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_IDatabaseDispatcher::~Arad_IDatabaseDispatcher()
Arad_IDatabaseDispatcher::~Arad_IDatabaseDispatcher() {}

// IDA_EA: 0x8186800
// IDA_NAME: _ZNK4ARAD10DISPATCHER24Arad_IDatabaseDispatcher8get_codeEv
// DEMANGLED: ARAD::DISPATCHER::Arad_IDatabaseDispatcher::get_code(void)const
ENUM_INTERNALPACKET_JPN Arad_IDatabaseDispatcher::get_code() const {
    return code_;
}

// IDA_EA: 0x8184C7C
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcherC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::Arad_DatabaseDispatcher(void)
Arad_DatabaseDispatcher::Arad_DatabaseDispatcher() : dispatchers_() {
    dispatchers_.clear();
}

// IDA_EA: 0x8184CBE
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcherD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::~Arad_DatabaseDispatcher()
Arad_DatabaseDispatcher::~Arad_DatabaseDispatcher() {
    std::for_each(dispatchers_.begin(), dispatchers_.end(),
                  DeleteDispatcher());
}

// IDA_EA: 0x81850E0
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcher13GetDispatcherENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::GetDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
Arad_IDatabaseDispatcher* Arad_DatabaseDispatcher::GetDispatcher(
    ENUM_INTERNALPACKET_JPN code) {
    DispatcherMap::iterator it = dispatchers_.find(code);
    return it != dispatchers_.end() ? it->second : 0;
}

// IDA_EA: 0x8185144
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcher17_registDispatcherENS_23ENUM_INTERNALPACKET_JPNEPNS0_24Arad_IDatabaseDispatcherE
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::_registDispatcher(ARAD::ENUM_INTERNALPACKET_JPN,ARAD::DISPATCHER::Arad_IDatabaseDispatcher *)
bool Arad_DatabaseDispatcher::_registDispatcher(
    ENUM_INTERNALPACKET_JPN code,
    Arad_IDatabaseDispatcher* dispatcher) {
    if (GetDispatcher(code)) {
        return false;
    }
    dispatchers_[code] = dispatcher;
    return true;
}

// IDA_EA: 0x818518A
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcher19_unregistDispatcherENS_23ENUM_INTERNALPACKET_JPNE
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::_unregistDispatcher(ARAD::ENUM_INTERNALPACKET_JPN)
bool Arad_DatabaseDispatcher::_unregistDispatcher(ENUM_INTERNALPACKET_JPN code) {
    if (!GetDispatcher(code)) {
        return false;
    }
    dispatchers_.erase(code);
    return true;
}

// IDA_EA: 0x818680C
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcher16DeleteDispatcherclERSt4pairIKNS_23ENUM_INTERNALPACKET_JPNEPNS0_24Arad_IDatabaseDispatcherEE
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::DeleteDispatcher::operator()(std::pair<ARAD::ENUM_INTERNALPACKET_JPN const,ARAD::DISPATCHER::Arad_IDatabaseDispatcher *> &)
void Arad_DatabaseDispatcher::DeleteDispatcher::operator()(
    std::pair<const ENUM_INTERNALPACKET_JPN,
              Arad_IDatabaseDispatcher*>& entry) {
    if (entry.second) {
        delete entry.second;
    }
}

// IDA_EA: 0x8186836
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_DB_kAradEventItemLogC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAradEventItemLog::Arad_DB_kAradEventItemLog(void)
Arad_DB_kAradEventItemLog::Arad_DB_kAradEventItemLog()
    : Arad_IDatabaseDispatcher(kInternalPacketAradEventItemLog) {}
// IDA_EA: 0x818685A
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_DB_kAradEventItemLogD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAradEventItemLog::~Arad_DB_kAradEventItemLog()
// IDA_EA: 0x818688A
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_DB_kAradEventItemLogD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAradEventItemLog::~Arad_DB_kAradEventItemLog()
Arad_DB_kAradEventItemLog::~Arad_DB_kAradEventItemLog() {}

// IDA_EA: 0x81868A8
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSetCharacCreateTimeC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSetCharacCreateTime::Arad_DB_kSetCharacCreateTime(void)
Arad_DB_kSetCharacCreateTime::Arad_DB_kSetCharacCreateTime()
    : Arad_IDatabaseDispatcher(kInternalPacketSetCharacCreateTime) {}
// IDA_EA: 0x81868CC
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSetCharacCreateTimeD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSetCharacCreateTime::~Arad_DB_kSetCharacCreateTime()
// IDA_EA: 0x81868FC
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSetCharacCreateTimeD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSetCharacCreateTime::~Arad_DB_kSetCharacCreateTime()
Arad_DB_kSetCharacCreateTime::~Arad_DB_kSetCharacCreateTime() {}

// IDA_EA: 0x818691A
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kLoadRewardEventItemC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadRewardEventItem::Arad_DB_kLoadRewardEventItem(void)
Arad_DB_kLoadRewardEventItem::Arad_DB_kLoadRewardEventItem()
    : Arad_IDatabaseDispatcher(kInternalPacketLoadRewardEventItem) {}
// IDA_EA: 0x818693E
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kLoadRewardEventItemD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadRewardEventItem::~Arad_DB_kLoadRewardEventItem()
// IDA_EA: 0x818696E
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kLoadRewardEventItemD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadRewardEventItem::~Arad_DB_kLoadRewardEventItem()
Arad_DB_kLoadRewardEventItem::~Arad_DB_kLoadRewardEventItem() {}

// IDA_EA: 0x818698C
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSaveRewardEventItemC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveRewardEventItem::Arad_DB_kSaveRewardEventItem(void)
Arad_DB_kSaveRewardEventItem::Arad_DB_kSaveRewardEventItem()
    : Arad_IDatabaseDispatcher(kInternalPacketSaveRewardEventItem) {}
// IDA_EA: 0x81869B0
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSaveRewardEventItemD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveRewardEventItem::~Arad_DB_kSaveRewardEventItem()
// IDA_EA: 0x81869E0
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSaveRewardEventItemD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveRewardEventItem::~Arad_DB_kSaveRewardEventItem()
Arad_DB_kSaveRewardEventItem::~Arad_DB_kSaveRewardEventItem() {}

// IDA_EA: 0x81869FE
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kSaveServerStateC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveServerState::Arad_DB_kSaveServerState(void)
Arad_DB_kSaveServerState::Arad_DB_kSaveServerState()
    : Arad_IDatabaseDispatcher(kInternalPacketSaveServerState) {}
// IDA_EA: 0x8186A22
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kSaveServerStateD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveServerState::~Arad_DB_kSaveServerState()
// IDA_EA: 0x8186A52
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kSaveServerStateD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveServerState::~Arad_DB_kSaveServerState()
Arad_DB_kSaveServerState::~Arad_DB_kSaveServerState() {}

// IDA_EA: 0x8186A70
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kLoadServerStateC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadServerState::Arad_DB_kLoadServerState(void)
Arad_DB_kLoadServerState::Arad_DB_kLoadServerState()
    : Arad_IDatabaseDispatcher(kInternalPacketLoadServerState) {}
// IDA_EA: 0x8186A94
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kLoadServerStateD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadServerState::~Arad_DB_kLoadServerState()
// IDA_EA: 0x8186AC4
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kLoadServerStateD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadServerState::~Arad_DB_kLoadServerState()
Arad_DB_kLoadServerState::~Arad_DB_kLoadServerState() {}

// IDA_EA: 0x8186AE2
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_DB_kAvatarHiddenOptionSaveC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarHiddenOptionSave::Arad_DB_kAvatarHiddenOptionSave(void)
Arad_DB_kAvatarHiddenOptionSave::Arad_DB_kAvatarHiddenOptionSave()
    : Arad_IDatabaseDispatcher(kInternalPacketAvatarHiddenOptionSave) {}
// IDA_EA: 0x8186B06
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_DB_kAvatarHiddenOptionSaveD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarHiddenOptionSave::~Arad_DB_kAvatarHiddenOptionSave()
// IDA_EA: 0x8186B36
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_DB_kAvatarHiddenOptionSaveD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarHiddenOptionSave::~Arad_DB_kAvatarHiddenOptionSave()
Arad_DB_kAvatarHiddenOptionSave::~Arad_DB_kAvatarHiddenOptionSave() {}

// IDA_EA: 0x8186B54
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kAvatarEnduranceSaveC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarEnduranceSave::Arad_DB_kAvatarEnduranceSave(void)
Arad_DB_kAvatarEnduranceSave::Arad_DB_kAvatarEnduranceSave()
    : Arad_IDatabaseDispatcher(kInternalPacketAvatarEnduranceSave) {}
// IDA_EA: 0x8186B78
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kAvatarEnduranceSaveD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarEnduranceSave::~Arad_DB_kAvatarEnduranceSave()
// IDA_EA: 0x8186BA8
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kAvatarEnduranceSaveD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarEnduranceSave::~Arad_DB_kAvatarEnduranceSave()
Arad_DB_kAvatarEnduranceSave::~Arad_DB_kAvatarEnduranceSave() {}

// IDA_EA: 0x8186BC6
// IDA_NAME: _ZN4ARAD10DISPATCHER39Arad_DB_kResetLimitedCreateCharacPerMidC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kResetLimitedCreateCharacPerMid::Arad_DB_kResetLimitedCreateCharacPerMid(void)
Arad_DB_kResetLimitedCreateCharacPerMid::
    Arad_DB_kResetLimitedCreateCharacPerMid()
    : Arad_IDatabaseDispatcher(kInternalPacketResetLimitedCreateCharacPerMid) {}
// IDA_EA: 0x8186BEA
// IDA_NAME: _ZN4ARAD10DISPATCHER39Arad_DB_kResetLimitedCreateCharacPerMidD2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kResetLimitedCreateCharacPerMid::~Arad_DB_kResetLimitedCreateCharacPerMid()
// IDA_EA: 0x8186C1A
// IDA_NAME: _ZN4ARAD10DISPATCHER39Arad_DB_kResetLimitedCreateCharacPerMidD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kResetLimitedCreateCharacPerMid::~Arad_DB_kResetLimitedCreateCharacPerMid()
Arad_DB_kResetLimitedCreateCharacPerMid::
    ~Arad_DB_kResetLimitedCreateCharacPerMid() {}

// IDA_EA: 0x8184D40
// IDA_NAME: _ZN4ARAD10DISPATCHER23Arad_DatabaseDispatcher4InitEv
// DEMANGLED: ARAD::DISPATCHER::Arad_DatabaseDispatcher::Init(void)
bool Arad_DatabaseDispatcher::Init() {
    Arad_IDatabaseDispatcher* dispatcher =
        new (std::nothrow) Arad_DB_kAradEventItemLog();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kSetCharacCreateTime();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kLoadRewardEventItem();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kSaveRewardEventItem();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kSaveServerState();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kLoadServerState();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kAvatarHiddenOptionSave();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow) Arad_DB_kAvatarEnduranceSave();
    if (dispatcher && !_registDispatcher(dispatcher->get_code(), dispatcher)) {
        return false;
    }
    dispatcher = new (std::nothrow)
        Arad_DB_kResetLimitedCreateCharacPerMid();
    return !dispatcher || _registDispatcher(dispatcher->get_code(), dispatcher);
}

// IDA_EA: 0x81851DC
// IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_DB_kAradEventItemLog8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAradEventItemLog::dispatch(int,int,Stream *)
bool Arad_DB_kAradEventItemLog::dispatch(int, int, Stream* stream) {
    arad::SigAradEventItemLog* input =
        stream->GetOutBuffer<arad::SigAradEventItemLog>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kAradEventItemLog::dispatch(int, int, Stream*)",
            0x643, "EVENT ITEM LOG, SigAradEventItemLog is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            " inSert into log_arad_event_stat ( occ_time, m_id, charac_no, item_id, item_cnt )  values ( now(), %u, %u, %u, %d ) ",
            input->account_id, input->charac_no, input->item_id,
            input->item_count)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x81853B2
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSetCharacCreateTime8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSetCharacCreateTime::dispatch(int,int,Stream *)
bool Arad_DB_kSetCharacCreateTime::dispatch(int, int, Stream* stream) {
    arad::SigSetCharacCreateTime* input =
        stream->GetOutBuffer<arad::SigSetCharacCreateTime>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kSetCharacCreateTime::dispatch(int, int, Stream*)",
            0x773, "SigSetCharacCreateTime is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            " upDate charac_info set create_time = from_unixtime(%u)  where charac_no = %u ",
            input->create_time, input->charac_no)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x8185574
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kLoadRewardEventItem8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadRewardEventItem::dispatch(int,int,Stream *)
bool Arad_DB_kLoadRewardEventItem::dispatch(int, int user_idx, Stream* stream) {
    arad::SigLoadRewardEventItem* input =
        stream->GetOutBuffer<arad::SigLoadRewardEventItem>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kLoadRewardEventItem::dispatch(int, int, Stream*)",
            0x796, "SigRewardEventItem is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    const int now = GlobalData::system_time()->getCurSec();
    if (!db->set_query(
            " seLect event_id, charac_no from event_reward_item_arad  where (m_id = %u and end_date > from_unixtime(%u))  order by occ_date desc limit 10 ",
            input->account_id, now)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";

    arad::SigLoadRewardEventItem result;
    while (db->fetch()) {
        const int index = result.reward_count;
        if (!db->get_int(0, result.event_codes[index])) throw "EventCode";
        if (!db->get_uint(1, result.charac_nos[index])) throw "Character";
        ++result.reward_count;
    }

    Stream* reply = GlobalData::stream_pool()->Acquire(
        "localjapan/Arad_DatabaseDispatcher.cpp", 1976);
    CStreamGuard guard(reply, true);
    make_internal_stream_jpn(guard, kInternalPacketLoadRewardEventItem, user_idx);
    guard.put_binary(&result, sizeof(result));
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard) != 0;
}

// IDA_EA: 0x81858DE
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kSaveRewardEventItem8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveRewardEventItem::dispatch(int,int,Stream *)
bool Arad_DB_kSaveRewardEventItem::dispatch(int, int, Stream* stream) {
    arad::SigSaveRewardEventItem* input =
        stream->GetOutBuffer<arad::SigSaveRewardEventItem>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kSaveRewardEventItem::dispatch(int, int, Stream*)",
            0x7CC, "SigRewardEventItem is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(9), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            " inSert into event_reward_item_arad values (%d, %u, %u, %u, from_unixtime(%u), now())",
            input->event_code, input->dword_04, input->dword_08,
            input->dword_10, input->dword_0c)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x8185ABE
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kSaveServerState8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kSaveServerState::dispatch(int,int,Stream *)
bool Arad_DB_kSaveServerState::dispatch(int, int, Stream* stream) {
    arad::SigServerState* input = stream->GetOutBuffer<arad::SigServerState>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kSaveServerState::dispatch(int, int, Stream*)",
            0x7EF, "SigServerState is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    const arad::SigServerState::tagData& data = input->data[0];
    if (input->active) {
        const char* state = db->blob_to_str(0, const_cast<AradServerStateMessage*>(&data.message), 12);
        if (!db->set_query(
                " inSert into server_state_info values (%d, %d, '%s', now(), from_unixtime(%u)) ",
                data.notify_category, data.effect, state, data.message.end_sec)) {
            throw "set_query";
        }
    } else if (!db->set_query(
                   " deLete from server_state_info where category=%d and code=%d ",
                   data.notify_category, data.effect)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x8185D20
// IDA_NAME: _ZN4ARAD10DISPATCHER24Arad_DB_kLoadServerState8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kLoadServerState::dispatch(int,int,Stream *)
bool Arad_DB_kLoadServerState::dispatch(int, int, Stream* stream) {
    if (!stream->GetOutBuffer<arad::SigServerState>()) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kLoadServerState::dispatch(int, int, Stream*)",
            0x81E, "SigServerState is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            " seLect category, code, state from server_state_info where end_time >= now()")) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";

    arad::SigServerState result;
    while (db->fetch()) {
        const int index = result.count;
        if (!db->get_int(0, result.data[index].notify_category)) throw "category";
        if (!db->get_int(1, result.data[index].effect)) throw "category";
        if (!db->get_binary(2, &result.data[index].message, 12)) throw "state";
        ++result.count;
    }

    Stream* reply = GlobalData::stream_pool()->Acquire(
        "localjapan/Arad_DatabaseDispatcher.cpp", 2111);
    CStreamGuard guard(reply, true);
    make_internal_stream_jpn(guard, kInternalPacketLoadServerState, -1);
    guard.put_binary(&result, sizeof(result));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x81860F4
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_DB_kAvatarHiddenOptionSave8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarHiddenOptionSave::dispatch(int,int,Stream *)
bool Arad_DB_kAvatarHiddenOptionSave::dispatch(int, int, Stream* stream) {
    arad::SigAradAvatarHiddenOptionSave* input =
        stream->GetOutBuffer<arad::SigAradAvatarHiddenOptionSave>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kAvatarHiddenOptionSave::dispatch(int, int, Stream*)",
            0x854, "SigAradAvatarHiddenOptionSave is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            "upDate user_items set hidden_option=%d where charac_no=%d and slot=%d and it_id=%d",
            input->hidden_option, input->charac_no, input->slot, input->item_id)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x81862D0
// IDA_NAME: _ZN4ARAD10DISPATCHER28Arad_DB_kAvatarEnduranceSave8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kAvatarEnduranceSave::dispatch(int,int,Stream *)
bool Arad_DB_kAvatarEnduranceSave::dispatch(int, int, Stream* stream) {
    arad::SigAradAvatarEnduranceSave* input =
        stream->GetOutBuffer<arad::SigAradAvatarEnduranceSave>();
    if (!input) {
        LogManager::logFormat(
            1, "localjapan/Arad_DatabaseDispatcher.cpp",
            "virtual bool ARAD::DISPATCHER::Arad_DB_kAvatarEnduranceSave::dispatch(int, int, Stream*)",
            0x876, "SigAradAvatarEnduranceSave is null.");
        return false;
    }
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query(
            "upDate user_items set emblem_endurance=%d where ui_id=%d",
            input->endurance, input->ui_id)) {
        throw "set_query";
    }
    if (!db->exec(true)) throw "exec";
    return true;
}

// IDA_EA: 0x8186492
// IDA_NAME: _ZN4ARAD10DISPATCHER39Arad_DB_kResetLimitedCreateCharacPerMid8dispatchEiiP6Stream
// DEMANGLED: ARAD::DISPATCHER::Arad_DB_kResetLimitedCreateCharacPerMid::dispatch(int,int,Stream *)
bool Arad_DB_kResetLimitedCreateCharacPerMid::dispatch(int, int, Stream*) {
    MySQL* db = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(1), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db) throw "handle null";
    if (!db->set_query("truncate limit_create_character")) throw "set_query";
    if (!db->exec(true)) throw "exec";
    return true;
}

}  // namespace DISPATCHER
}  // namespace ARAD

// IDA_EA: 0x844BC38
// IDA_NAME: _ZN25DB_AradDatabaseDispatcherC2Ev
// DEMANGLED: DB_AradDatabaseDispatcher::DB_AradDatabaseDispatcher(void)
DB_AradDatabaseDispatcher::DB_AradDatabaseDispatcher() : dispatcher_() {
    dispatcher_.Init();
}

// IDA_EA: 0x845AA96
// IDA_NAME: _ZN25DB_AradDatabaseDispatcherD2Ev
// DEMANGLED: DB_AradDatabaseDispatcher::~DB_AradDatabaseDispatcher()
// IDA_EA: 0x845AAF8
// IDA_NAME: _ZN25DB_AradDatabaseDispatcherD0Ev
// DEMANGLED: DB_AradDatabaseDispatcher::~DB_AradDatabaseDispatcher()
DB_AradDatabaseDispatcher::~DB_AradDatabaseDispatcher() {}

// IDA_EA: 0x844BCAC
// IDA_NAME: _ZN25DB_AradDatabaseDispatcher8dispatchEiiP6Stream
// DEMANGLED: DB_AradDatabaseDispatcher::dispatch(int,int,Stream *)
bool DB_AradDatabaseDispatcher::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    int code = 0;
    *stream >> code;
    ARAD::DISPATCHER::Arad_IDatabaseDispatcher* dispatcher =
        dispatcher_.GetDispatcher(
            static_cast<ARAD::ENUM_INTERNALPACKET_JPN>(code));
    if (dispatcher) {
        return dispatcher->dispatch(signal, user_idx, stream);
    }
    cMyTrace trace(
        "virtual bool DB_AradDatabaseDispatcher::dispatch(int, int, Stream*)",
        48267, 5);
    trace("[DISPATCHER_ARAD] Invalid Internal Packet Code. (code:%d)", code);
    return false;
}
