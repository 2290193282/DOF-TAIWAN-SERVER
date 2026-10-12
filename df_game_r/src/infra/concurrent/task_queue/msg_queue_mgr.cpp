#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include "infra/network/packet/stream_guard.h"


// IDA_EA: 0x844D7B4
// IDA_NAME: _ZN11MsgQueueMgr14get_queue_nameENS_9QUEUE_IDXE
// DEMANGLED: MsgQueueMgr::get_queue_name(MsgQueueMgr::QUEUE_IDX)
const char* MsgQueueMgr::get_queue_name(QUEUE_IDX index) {
    switch (index) {
        case NETWORK_Q:
            return "NETWORK_Q";
        case DISPATCH_Q:
            return "DISPATCH_Q";
        case DB_Q:
            return "DB_Q";
        case TIMER_Q:
            return "TIMER_Q";
        case LOGDB_Q:
            return "LOGDB_Q";
        case EXCHANGE_Q:
            return "EXCHANGE_Q";
        case FEATURE_DB_Q:
            return "NATIVE_0828_DB_Q";
        default:
            return "?????";
    }
}

// IDA_EA: 0x8570E40
// IDA_NAME: _ZN11MsgQueueMgrC2Ev
// DEMANGLED: MsgQueueMgr::MsgQueueMgr(void)
MsgQueueMgr::MsgQueueMgr() : mutexes_(), queues_() {}

// IDA_EA: 0x8570F42
// IDA_NAME: _ZN11MsgQueueMgrD2Ev
// DEMANGLED: MsgQueueMgr::~MsgQueueMgr()
MsgQueueMgr::~MsgQueueMgr() {}

bool MsgQueueMgr::valid_queue(QUEUE_IDX index) const {
    return index >= NETWORK_Q && index < QUEUE_COUNT;
}

// IDA_EA: 0x8570FDE
// IDA_NAME: _ZN11MsgQueueMgr3putENS_9QUEUE_IDXER12CStreamGuard
// DEMANGLED: MsgQueueMgr::put(MsgQueueMgr::QUEUE_IDX,CStreamGuard &)
int MsgQueueMgr::put(QUEUE_IDX index, CStreamGuard& guard) {
    return put(index, guard.Get());
}

// IDA_EA: 0x8571008
// IDA_NAME: _ZN11MsgQueueMgr3putENS_9QUEUE_IDXEP6Stream
// DEMANGLED: MsgQueueMgr::put(MsgQueueMgr::QUEUE_IDX,Stream *)
int MsgQueueMgr::put(QUEUE_IDX index, Stream* stream) {
    if (!stream || !valid_queue(index)) {
        return 0;
    }
    stream->get_buffer();
    stream->set_pool_live(false);
    MutexGuard guard(&mutexes_[index]);
    queues_[index].push(stream);
    return 1;
}

// IDA_EA: 0x85710AE
// IDA_NAME: _ZN11MsgQueueMgr3getENS_9QUEUE_IDXE
// DEMANGLED: MsgQueueMgr::get(MsgQueueMgr::QUEUE_IDX)
Stream* MsgQueueMgr::get(QUEUE_IDX index) {
    if (!valid_queue(index)) {
        return 0;
    }
    MutexGuard guard(&mutexes_[index]);
    if (queues_[index].empty()) {
        return 0;
    }
    Stream* stream = queues_[index].front();
    queues_[index].pop();
    return stream;
}

// IDA_EA: 0x8571170
// IDA_NAME: _ZN11MsgQueueMgr5emptyENS_9QUEUE_IDXE
// DEMANGLED: MsgQueueMgr::empty(MsgQueueMgr::QUEUE_IDX)
int MsgQueueMgr::empty(QUEUE_IDX index) {
    if (!valid_queue(index)) {
        return 1;
    }
    MutexGuard guard(&mutexes_[index]);
    return queues_[index].empty() ? 1 : 0;
}

// IDA_EA: 0x85711E4
// IDA_NAME: _ZN11MsgQueueMgr6remainENS_9QUEUE_IDXE
// DEMANGLED: MsgQueueMgr::remain(MsgQueueMgr::QUEUE_IDX)
int MsgQueueMgr::remain(QUEUE_IDX index) {
    if (!valid_queue(index)) {
        return 0;
    }
    MutexGuard guard(&mutexes_[index]);
    return static_cast<int>(queues_[index].size());
}
