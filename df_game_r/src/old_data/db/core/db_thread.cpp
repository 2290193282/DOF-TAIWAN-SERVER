#include "old_data/db/core/db_thread.h"

#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/db_shutdown_quiescence.h"
#include "old_data/db/core/db_task.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/task_queue/message_counter.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>
#include <iostream>
#include <new>
#include <atomic>

#if defined(__i386__)
typedef char DBThread_Size_Must_Be_76[(sizeof(DBThread) == 76) ? 1 : -1];
#endif

namespace {

// DBThread is an old fixed-layout type, so shutdown state cannot be appended
// as a member.  There is exactly one DB_Q worker and one LOGDB_Q worker.
std::atomic<bool> g_db_stop_after_drain(false);
std::atomic<bool> g_log_db_stop_after_drain(false);

std::atomic<uint32_t> g_shutdown_activity_epoch[
    db_shutdown_quiescence::kActivityDomainCount];
std::atomic<unsigned int> g_shutdown_activity_inflight[
    db_shutdown_quiescence::kActivityDomainCount];
std::atomic<uint32_t> g_shutdown_generation(0u);
std::atomic<uint32_t> g_shutdown_active_generation(0u);
std::atomic<int> g_shutdown_round_status(0);

std::atomic<bool>& StopAfterDrainFlag(bool log_db) {
    return log_db ? g_log_db_stop_after_drain : g_db_stop_after_drain;
}

}  // namespace

namespace db_shutdown_quiescence {
namespace {

bool ValidDomain(ActivityDomain domain) {
    return domain >= kActivityDispatch && domain < kActivityDomainCount;
}

bool IsBarrierSignal(ActivityDomain domain, int signal) {
    return (domain == kActivityLegacyDb &&
            signal == legacy_feature_fence::kDbSignal) ||
           (domain == kActivityNativeDb && signal == kNativeSignal);
}

void CompleteRound(uint32_t generation, bool success) {
    if (generation != 0u &&
        g_shutdown_active_generation.load(std::memory_order_acquire) ==
            generation) {
        int expected = 0;
        (void)g_shutdown_round_status.compare_exchange_strong(
            expected,
            success ? 1 : -1,
            std::memory_order_release,
            std::memory_order_relaxed);
    }
}

bool ReadEnvelope(const unsigned char* bytes,
                  uint32_t size,
                  int32_t& signal,
                  int32_t& user_idx,
                  Message& message) {
    if (!bytes || size != 8u + sizeof(Message)) {
        return false;
    }
    std::memcpy(&signal, bytes, sizeof(signal));
    std::memcpy(&user_idx, bytes + 4u, sizeof(user_idx));
    std::memcpy(&message, bytes + 8u, sizeof(message));
    return true;
}

Stream* AcquireShutdownControlStream() {
    SmallStreamPool* const small_pool = GlobalData::small_stream_pool();
    Stream* stream = small_pool
                         ? small_pool->Acquire(__FILE__, __LINE__)
                         : 0;
    if (!stream) {
        StreamPool* const default_pool = GlobalData::stream_pool();
        stream = default_pool
                     ? default_pool->Acquire(__FILE__, __LINE__)
                     : 0;
    }
    return stream;
}

}  // namespace

void BeginActivity(ActivityDomain domain, int signal) {
    if (!ValidDomain(domain)) {
        return;
    }
    g_shutdown_activity_inflight[domain].fetch_add(
        1u, std::memory_order_seq_cst);
    if (!IsBarrierSignal(domain, signal)) {
        g_shutdown_activity_epoch[domain].fetch_add(
            1u, std::memory_order_seq_cst);
    }
}

void EndActivity(ActivityDomain domain) {
    if (!ValidDomain(domain)) {
        return;
    }
    g_shutdown_activity_inflight[domain].fetch_sub(
        1u, std::memory_order_seq_cst);
}

uint32_t ActivityEpoch(ActivityDomain domain) {
    return ValidDomain(domain)
               ? g_shutdown_activity_epoch[domain].load(
                     std::memory_order_seq_cst)
               : 0u;
}

bool ActivityInFlight(ActivityDomain domain) {
    return ValidDomain(domain) &&
           g_shutdown_activity_inflight[domain].load(
               std::memory_order_seq_cst) != 0u;
}

uint32_t AllocateGeneration() {
    uint32_t generation = g_shutdown_generation.fetch_add(
                              1u, std::memory_order_seq_cst) +
                          1u;
    if (generation == 0u) {
        generation = g_shutdown_generation.fetch_add(
                         1u, std::memory_order_seq_cst) +
                     1u;
    }
    return generation;
}

bool QueueBegin(uint32_t generation) {
    if (generation == 0u) {
        return false;
    }
    g_shutdown_active_generation.store(generation,
                                       std::memory_order_release);
    g_shutdown_round_status.store(0, std::memory_order_release);

    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    Stream* const stream = AcquireShutdownControlStream();
    if (!stream || !queues) {
        if (stream) {
            CStreamGuard cleanup(stream, true);
        }
        CompleteRound(generation, false);
        return false;
    }

    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kNativeSignal);
    *guard << -1;
    Message message;
    message.magic = static_cast<uint32_t>(kMagic);
    message.version = static_cast<uint32_t>(kVersion);
    message.phase = static_cast<uint32_t>(kPhaseBegin);
    message.generation = generation;
    if (guard.put_binary(&message, static_cast<int>(sizeof(message))) != 1 ||
        queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) != 1) {
        CompleteRound(generation, false);
        return false;
    }
    return true;
}

int GetRoundStatus(uint32_t generation) {
    if (generation == 0u ||
        g_shutdown_active_generation.load(std::memory_order_acquire) !=
            generation) {
        return -1;
    }
    return g_shutdown_round_status.load(std::memory_order_acquire);
}

bool ValidateFenceAckTarget(const unsigned char* bytes, uint32_t size) {
    int32_t signal = 0;
    int32_t user_idx = 0;
    Message message;
    std::memset(&message, 0, sizeof(message));
    return ReadEnvelope(bytes, size, signal, user_idx, message) &&
           signal == kNativeSignal && user_idx == -1 &&
           message.magic == static_cast<uint32_t>(kMagic) &&
           message.version == static_cast<uint32_t>(kVersion) &&
           message.phase == static_cast<uint32_t>(kPhaseAck) &&
           message.generation != 0u;
}

DB_ShutdownQuiescence::DB_ShutdownQuiescence()
    : DBDispatcher() {}

DB_ShutdownQuiescence::~DB_ShutdownQuiescence() {}

bool DB_ShutdownQuiescence::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    if (signal != kNativeSignal || user_idx != -1 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }

    Message message;
    std::memset(&message, 0, sizeof(message));
    if (stream->get_binary(&message, static_cast<int>(sizeof(message))) != 1 ||
        message.magic != static_cast<uint32_t>(kMagic) ||
        message.version != static_cast<uint32_t>(kVersion) ||
        message.generation == 0u ||
        g_shutdown_active_generation.load(std::memory_order_acquire) !=
            message.generation) {
        return false;
    }

    if (message.phase == static_cast<uint32_t>(kPhaseAck)) {
        CompleteRound(message.generation, true);
        return true;
    }
    if (message.phase != static_cast<uint32_t>(kPhaseBegin)) {
        CompleteRound(message.generation, false);
        return false;
    }

    // The ACK target is copied synchronously into the signal-823 wrapper; it
    // need not consume another shared pool entry while queue6 still owns the
    // begin stream.  This removes the smallest control-path pool deadlock.
    Stream ack_stream(64);
    CStreamGuard ack_guard(&ack_stream, false);
    *ack_guard << static_cast<int>(kNativeSignal);
    *ack_guard << -1;
    message.phase = static_cast<uint32_t>(kPhaseAck);
    if (ack_guard.put_binary(&message,
                             static_cast<int>(sizeof(message))) != 1 ||
        !legacy_feature_fence::QueueAfterLegacy(
            legacy_feature_fence::kFenceShutdownQuiescence,
            ack_guard)) {
        CompleteRound(message.generation, false);
        return false;
    }
    return true;
}

}  // namespace db_shutdown_quiescence_0828

// IDA_EA: 0x83FF5F2
// IDA_NAME: _ZN8DBThreadC2Ev
// DEMANGLED: DBThread::DBThread(void)
DBThread::DBThread() : ThreadInterface(), log_db_(false), query_counter_() {}

// IDA_EA: 0x83FF648
// IDA_NAME: _ZN8DBThreadD2Ev
// DEMANGLED: DBThread::~DBThread()
// IDA_EA: 0x83FF6AA
// IDA_NAME: _ZN8DBThreadD0Ev
// DEMANGLED: DBThread::~DBThread()
DBThread::~DBThread() {}

// IDA_EA: 0x83FF6C8
// IDA_NAME: _ZN8DBThread8setLogDBEb
// DEMANGLED: DBThread::setLogDB(bool)
bool DBThread::setLogDB(bool is_log_db) {
    log_db_ = is_log_db;
    query_counter_.setLogDB(log_db_);

    for (int handle_idx = 0; handle_idx < DB_HANDLE_COUNT; ++handle_idx) {
        const ENUM_DB_HANDLE_IDX handle =
            static_cast<ENUM_DB_HANDLE_IDX>(handle_idx);
        if (isLogDB()) {
            if (handle != DB_HANDLE_LOG) {
                continue;
            }
        } else if (handle == DB_HANDLE_LOG ||
                   IsFeatureDBHandle(handle)) {
            // Native 0828 connections belong exclusively to their dedicated
            // worker and intentionally have no shared CQueryCounter owner.
            continue;
        }
        MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandleExact(
            handle, SERVER_GROUP_DEFAULT);
        if (db_handle) {
            db_handle->SetQueryCounter(&query_counter_);
        }
    }
    return true;
}

// IDA_EA: 0x83FF77A
// IDA_NAME: _ZN8DBThread7isLogDBEv
// DEMANGLED: DBThread::isLogDB(void)
bool DBThread::isLogDB() {
    return log_db_;
}

void DBThread::requestStopAfterDrain() {
    StopAfterDrainFlag(isLogDB()).store(true, std::memory_order_release);
}

// IDA_EA: 0x83FF786
// IDA_NAME: _ZN8DBThread8dispatchEPv
// DEMANGLED: DBThread::dispatch(void *)
void DBThread::dispatch(void* user_data) {
    (void)user_data;

    DBTask* const task = new (std::nothrow) DBTask();
    if (!task) {
        std::cerr << "[DBThread::dispatch] Memory allocation fail!"
                  << std::endl;
        OS_API::Exit(0);
    }

    const MsgQueueMgr::QUEUE_IDX queue_index =
        isLogDB() ? MsgQueueMgr::LOGDB_Q : MsgQueueMgr::DB_Q;
    message_counter::CMessageCounter message_counter(20000);
    int current_second = 0;
    int last_log_second = 0;
    message_counter::CTimeInterval* const interval =
        message_counter::CTimeInterval::Create();
    int signal = 0;

    // GlobalData::server_stop() stops ingress, not this consumer.  DB_Q can
    // still receive work from the Exchange drain, and DB_Q can in turn
    // forward signal-823/828 continuations to queue 6.  Exit only after the
    // owner publishes the queue-specific producer barrier and this worker
    // observes its queue empty.
    std::atomic<bool>& stop_after_drain =
        StopAfterDrainFlag(isLogDB());
    for (;;) {
        current_second = GlobalData::system_time()->getCurSec();
        interval->SetLastTime();

        signal = 0;
        // Signal 63 is the old in-band termination marker.  DBTask logs it
        // and returns its stream to the owning pool, but it is not a producer
        // barrier: valid work may still be queued behind it.  The explicit
        // drain flag is the only authority that can terminate this worker.
        (void)task->HandleEvents(queue_index, signal);
        if (signal != 0) {
            message_counter.IncreaseMessage(static_cast<unsigned int>(signal),
                                            0,
                                            0,
                                            interval->GetTimeInterval());
            if (current_second - last_log_second > 600) {
                last_log_second = current_second;
                message_counter.WriteLog();
            }
        }
        MsgQueueMgr* const queues = GlobalData::msgq_mgr();
        if (stop_after_drain.load(std::memory_order_acquire) &&
            (!queues || queues->empty(queue_index))) {
            break;
        }
    }

    message_counter::CTimeInterval::Destory(interval);
    delete task;
}
