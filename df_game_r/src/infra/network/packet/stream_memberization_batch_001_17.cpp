#include "infra/network/packet/stream_guard.h"
#include "old_data/channel/channel_db_payloads.h"

// IDA_EA: 0x86d575e
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_UPDATE_STATISTIC4LEVEEPT_v
// DEMANGLED: SIG_UPDATE_STATISTIC4LEV * Stream::GetInBuffer<SIG_UPDATE_STATISTIC4LEV>(void)
template <>
SIG_UPDATE_STATISTIC4LEV* Stream::GetInBuffer<SIG_UPDATE_STATISTIC4LEV>() {
    const int bytes = sizeof(SIG_UPDATE_STATISTIC4LEV);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_STATISTIC4LEV* result =
        reinterpret_cast<SIG_UPDATE_STATISTIC4LEV*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x86D40EE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_UPDATE_STATISTIC4LEVEEPT_v
// DEMANGLED: SIG_UPDATE_STATISTIC4LEV * CStreamGuard::GetInBuffer<SIG_UPDATE_STATISTIC4LEV>(void)
template <>
SIG_UPDATE_STATISTIC4LEV* CStreamGuard::GetInBuffer<SIG_UPDATE_STATISTIC4LEV>() {
    return stream_->GetInBuffer<SIG_UPDATE_STATISTIC4LEV>();
}
