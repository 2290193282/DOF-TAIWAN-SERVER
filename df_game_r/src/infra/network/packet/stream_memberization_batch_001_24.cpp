#include "infra/network/packet/stream_guard.h"
#include "old_data/db/auth_security/security_card_payloads.h"

// IDA_EA: 0x84559f0
// IDA_NAME: _ZN6Stream11GetInBufferI33SIG_SECURITY_CARD_UPDATE_FAIL_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_UPDATE_FAIL_CNT * Stream::GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>(void)
template <>
SIG_SECURITY_CARD_UPDATE_FAIL_CNT* Stream::GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>() {
    const int bytes = sizeof(SIG_SECURITY_CARD_UPDATE_FAIL_CNT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SECURITY_CARD_UPDATE_FAIL_CNT* result =
        reinterpret_cast<SIG_SECURITY_CARD_UPDATE_FAIL_CNT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8451E1A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI33SIG_SECURITY_CARD_UPDATE_FAIL_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_UPDATE_FAIL_CNT * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>(void)
template <>
SIG_SECURITY_CARD_UPDATE_FAIL_CNT* CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>() {
    return stream_->GetInBuffer<SIG_SECURITY_CARD_UPDATE_FAIL_CNT>();
}

// IDA_EA: 0x84562be
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_SECURITY_CARD_DISUSEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_DISUSE * Stream::GetInBuffer<SIG_SECURITY_CARD_DISUSE>(void)
template <>
SIG_SECURITY_CARD_DISUSE* Stream::GetInBuffer<SIG_SECURITY_CARD_DISUSE>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SECURITY_CARD_DISUSE* result =
        reinterpret_cast<SIG_SECURITY_CARD_DISUSE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452672
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_SECURITY_CARD_DISUSEEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_DISUSE * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_DISUSE>(void)
template <>
SIG_SECURITY_CARD_DISUSE* CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_DISUSE>() {
    return stream_->GetInBuffer<SIG_SECURITY_CARD_DISUSE>();
}

// IDA_EA: 0x8456d82
// IDA_NAME: _ZN6Stream11GetInBufferI30SIG_SECURITY_CARD_ADD_CERT_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_ADD_CERT_CNT * Stream::GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>(void)
template <>
SIG_SECURITY_CARD_ADD_CERT_CNT* Stream::GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SECURITY_CARD_ADD_CERT_CNT* result =
        reinterpret_cast<SIG_SECURITY_CARD_ADD_CERT_CNT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845332E
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI30SIG_SECURITY_CARD_ADD_CERT_CNTEEPT_v
// DEMANGLED: SIG_SECURITY_CARD_ADD_CERT_CNT * CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>(void)
template <>
SIG_SECURITY_CARD_ADD_CERT_CNT* CStreamGuard::GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>() {
    return stream_->GetInBuffer<SIG_SECURITY_CARD_ADD_CERT_CNT>();
}
