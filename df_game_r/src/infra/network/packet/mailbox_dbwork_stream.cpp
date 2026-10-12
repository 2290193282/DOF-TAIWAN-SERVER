#include "infra/network/packet/mailbox_dbwork_stream.h"

static_assert(sizeof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK) == 420u,
              "mail DBWORK stream ABI size must stay 420");

// IDA_EA: 0x8558B10
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI37SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORKEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK * CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK*
CStreamGuard::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>() {
    return stream_->GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>();
}

// IDA_EA: 0x8559A7C
// IDA_NAME: _ZN6Stream11GetInBufferI37SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORKEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK * Stream::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK*
Stream::GetInBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>() {
    const int bytes = sizeof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const result =
        reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK*>(
            buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845213E
// IDA_NAME: _ZN6Stream12GetOutBufferI37SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORKEEPT_v
// DEMANGLED: SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK * Stream::GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>(void)
template <>
SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK*
Stream::GetOutBuffer<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK>() {
    const int bytes = sizeof(SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK* const result =
        reinterpret_cast<SIG_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK*>(
            buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
