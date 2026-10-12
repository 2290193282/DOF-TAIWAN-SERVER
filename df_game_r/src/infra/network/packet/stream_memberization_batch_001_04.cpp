#include "infra/network/packet/stream_guard.h"
#include "old_data/db/anti_cheat/account_hack_payload.h"

// IDA_EA: 0x845653a
// IDA_NAME: _ZN6Stream11GetInBufferI33SIG_INSERT_AUTO_PUNISH_FIRST_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_FIRST_USER * Stream::GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_FIRST_USER* Stream::GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>() {
    const int bytes = 44;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_AUTO_PUNISH_FIRST_USER* result =
        reinterpret_cast<SIG_INSERT_AUTO_PUNISH_FIRST_USER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8452846
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI33SIG_INSERT_AUTO_PUNISH_FIRST_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_FIRST_USER * CStreamGuard::GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_FIRST_USER* CStreamGuard::GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>() {
    return stream_->GetInBuffer<SIG_INSERT_AUTO_PUNISH_FIRST_USER>();
}

// IDA_EA: 0x84565a4
// IDA_NAME: _ZN6Stream11GetInBufferI34SIG_INSERT_AUTO_PUNISH_SECOND_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_SECOND_USER * Stream::GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_SECOND_USER* Stream::GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>() {
    const int bytes = 20;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_INSERT_AUTO_PUNISH_SECOND_USER* result =
        reinterpret_cast<SIG_INSERT_AUTO_PUNISH_SECOND_USER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84528B0
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI34SIG_INSERT_AUTO_PUNISH_SECOND_USEREEPT_v
// DEMANGLED: SIG_INSERT_AUTO_PUNISH_SECOND_USER * CStreamGuard::GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>(void)
template <>
SIG_INSERT_AUTO_PUNISH_SECOND_USER* CStreamGuard::GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>() {
    return stream_->GetInBuffer<SIG_INSERT_AUTO_PUNISH_SECOND_USER>();
}

// IDA_EA: 0x845727a
// IDA_NAME: _ZN6Stream11GetInBufferI23SIG_CHECK_MY_ACCOUNT_OKEEPT_v
// DEMANGLED: SIG_CHECK_MY_ACCOUNT_OK * Stream::GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>(void)
template <>
SIG_CHECK_MY_ACCOUNT_OK* Stream::GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>() {
    const int bytes = 12;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_MY_ACCOUNT_OK* result =
        reinterpret_cast<SIG_CHECK_MY_ACCOUNT_OK*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845387A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI23SIG_CHECK_MY_ACCOUNT_OKEEPT_v
// DEMANGLED: SIG_CHECK_MY_ACCOUNT_OK * CStreamGuard::GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>(void)
template <>
SIG_CHECK_MY_ACCOUNT_OK* CStreamGuard::GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>() {
    return stream_->GetInBuffer<SIG_CHECK_MY_ACCOUNT_OK>();
}

// IDA_EA: 0x84572e4
// IDA_NAME: _ZN6Stream11GetInBufferI26SIG_CHECK_MY_ACCOUNT_IN_DBEEPT_v
// DEMANGLED: SIG_CHECK_MY_ACCOUNT_IN_DB * Stream::GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>(void)
template <>
SIG_CHECK_MY_ACCOUNT_IN_DB* Stream::GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>() {
    const int bytes = 41;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_CHECK_MY_ACCOUNT_IN_DB* result =
        reinterpret_cast<SIG_CHECK_MY_ACCOUNT_IN_DB*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453890
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI26SIG_CHECK_MY_ACCOUNT_IN_DBEEPT_v
// DEMANGLED: SIG_CHECK_MY_ACCOUNT_IN_DB * CStreamGuard::GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>(void)
template <>
SIG_CHECK_MY_ACCOUNT_IN_DB* CStreamGuard::GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>() {
    return stream_->GetInBuffer<SIG_CHECK_MY_ACCOUNT_IN_DB>();
}

// IDA_EA: 0x84573b8
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_SECOND_USER_DATA_SETEEPT_v
// DEMANGLED: SIG_SECOND_USER_DATA_SET * Stream::GetInBuffer<SIG_SECOND_USER_DATA_SET>(void)
template <>
SIG_SECOND_USER_DATA_SET* Stream::GetInBuffer<SIG_SECOND_USER_DATA_SET>() {
    const int bytes = 24;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SECOND_USER_DATA_SET* result =
        reinterpret_cast<SIG_SECOND_USER_DATA_SET*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453964
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_SECOND_USER_DATA_SETEEPT_v
// DEMANGLED: SIG_SECOND_USER_DATA_SET * CStreamGuard::GetInBuffer<SIG_SECOND_USER_DATA_SET>(void)
template <>
SIG_SECOND_USER_DATA_SET* CStreamGuard::GetInBuffer<SIG_SECOND_USER_DATA_SET>() {
    return stream_->GetInBuffer<SIG_SECOND_USER_DATA_SET>();
}

// IDA_EA: 0x8457422
// IDA_NAME: _ZN6Stream11GetInBufferI20SIG_SECOND_USER_DATAEEPT_v
// DEMANGLED: SIG_SECOND_USER_DATA * Stream::GetInBuffer<SIG_SECOND_USER_DATA>(void)
template <>
SIG_SECOND_USER_DATA* Stream::GetInBuffer<SIG_SECOND_USER_DATA>() {
    const int bytes = 20;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SECOND_USER_DATA* result =
        reinterpret_cast<SIG_SECOND_USER_DATA*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845397A
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI20SIG_SECOND_USER_DATAEEPT_v
// DEMANGLED: SIG_SECOND_USER_DATA * CStreamGuard::GetInBuffer<SIG_SECOND_USER_DATA>(void)
template <>
SIG_SECOND_USER_DATA* CStreamGuard::GetInBuffer<SIG_SECOND_USER_DATA>() {
    return stream_->GetInBuffer<SIG_SECOND_USER_DATA>();
}
