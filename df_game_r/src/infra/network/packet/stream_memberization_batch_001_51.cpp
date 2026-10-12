#include "infra/network/packet/stream_guard.h"
#include "old_data/db/quest/db_load_quest.h"

// IDA_EA: 0x8239806
// IDA_NAME: _ZN6Stream11GetInBufferI14SIG_LOAD_QUESTEEPT_v
// DEMANGLED: SIG_LOAD_QUEST * Stream::GetInBuffer<SIG_LOAD_QUEST>(void)
template <>
SIG_LOAD_QUEST* Stream::GetInBuffer<SIG_LOAD_QUEST>() {
    const int bytes = 30216;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_LOAD_QUEST* result =
        reinterpret_cast<SIG_LOAD_QUEST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236D5C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI14SIG_LOAD_QUESTEEPT_v
// DEMANGLED: SIG_LOAD_QUEST * CStreamGuard::GetInBuffer<SIG_LOAD_QUEST>(void)
template <>
SIG_LOAD_QUEST* CStreamGuard::GetInBuffer<SIG_LOAD_QUEST>() {
    return stream_->GetInBuffer<SIG_LOAD_QUEST>();
}
