#include "infra/network/packet/stream_guard.h"
#include "old_data/db/mail/mailbox_payload.h"

// IDA_EA: 0x819808c
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_MAILBOX_SEND_NEW_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL * Stream::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL* Stream::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>() {
    const int bytes = sizeof(SIG_MAILBOX_SEND_NEW_MAIL);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MAILBOX_SEND_NEW_MAIL* result =
        reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8197F8E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_MAILBOX_SEND_NEW_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL * CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL* CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>() {
    return stream_->GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL>();
}

// IDA_EA: 0x845503e
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_MAILBOX_CHECK_RECEIVEREEPT_v
// DEMANGLED: SIG_MAILBOX_CHECK_RECEIVER * Stream::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>(void)
template <>
SIG_MAILBOX_CHECK_RECEIVER* Stream::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>() {
    const int bytes = 16350;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MAILBOX_CHECK_RECEIVER* result =
        reinterpret_cast<SIG_MAILBOX_CHECK_RECEIVER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84515AE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_MAILBOX_CHECK_RECEIVEREEPT_v
// DEMANGLED: SIG_MAILBOX_CHECK_RECEIVER * CStreamGuard::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>(void)
template <>
SIG_MAILBOX_CHECK_RECEIVER* CStreamGuard::GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>() {
    return stream_->GetInBuffer<SIG_MAILBOX_CHECK_RECEIVER>();
}

// IDA_EA: 0x8559ae6
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_MAILBOX_DEL_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_DEL_MAIL * Stream::GetInBuffer<SIG_MAILBOX_DEL_MAIL>(void)
template <>
SIG_MAILBOX_DEL_MAIL* Stream::GetInBuffer<SIG_MAILBOX_DEL_MAIL>() {
    const int bytes = 88;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_MAILBOX_DEL_MAIL* result =
        reinterpret_cast<SIG_MAILBOX_DEL_MAIL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8558B26
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_MAILBOX_DEL_MAILEEPT_v
// DEMANGLED: SIG_MAILBOX_DEL_MAIL * CStreamGuard::GetInBuffer<SIG_MAILBOX_DEL_MAIL>(void)
template <>
SIG_MAILBOX_DEL_MAIL* CStreamGuard::GetInBuffer<SIG_MAILBOX_DEL_MAIL>() {
    return stream_->GetInBuffer<SIG_MAILBOX_DEL_MAIL>();
}
