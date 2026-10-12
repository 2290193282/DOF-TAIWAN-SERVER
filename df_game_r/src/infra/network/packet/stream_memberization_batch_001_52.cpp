#include "infra/network/packet/stream_guard.h"
#include "old_data/db/quest/db_update_quest.h"

// IDA_EA: 0x869fc88
// IDA_NAME: _ZN6Stream11GetInBufferI16SIG_UPDATE_QUESTEEPT_v
// DEMANGLED: SIG_UPDATE_QUEST * Stream::GetInBuffer<SIG_UPDATE_QUEST>(void)
template <>
SIG_UPDATE_QUEST* Stream::GetInBuffer<SIG_UPDATE_QUEST>() {
    const int bytes = 30209;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_QUEST* result =
        reinterpret_cast<SIG_UPDATE_QUEST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869AAA2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_UPDATE_QUESTEEPT_v
// DEMANGLED: SIG_UPDATE_QUEST * CStreamGuard::GetInBuffer<SIG_UPDATE_QUEST>(void)
template <>
SIG_UPDATE_QUEST* CStreamGuard::GetInBuffer<SIG_UPDATE_QUEST>() {
    return stream_->GetInBuffer<SIG_UPDATE_QUEST>();
}
