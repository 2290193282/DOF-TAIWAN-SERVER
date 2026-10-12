#include "infra/network/packet/stream_guard.h"
#include "old_data/db/mail/db_mailbox_req_list.h"

// IDA_EA: 0x85599ca
// IDA_NAME: _ZN6Stream11GetInBufferI16SIG_MAILBOX_LISTEEPT_v
// DEMANGLED: SIG_MAILBOX_LIST * Stream::GetInBuffer<SIG_MAILBOX_LIST>(void)
template <>
SIG_MAILBOX_LIST* Stream::GetInBuffer<SIG_MAILBOX_LIST>() {
    const int bytes = sizeof(SIG_MAILBOX_LIST);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MAILBOX_LIST* result =
        reinterpret_cast<SIG_MAILBOX_LIST*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8558AB4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI16SIG_MAILBOX_LISTEEPT_v
// DEMANGLED: SIG_MAILBOX_LIST * CStreamGuard::GetInBuffer<SIG_MAILBOX_LIST>(void)
template <>
SIG_MAILBOX_LIST* CStreamGuard::GetInBuffer<SIG_MAILBOX_LIST>() {
    return stream_->GetInBuffer<SIG_MAILBOX_LIST>();
}
