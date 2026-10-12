#include "infra/network/packet/stream_guard.h"
#include "common/manager/shared_server_message_abi.h"

// IDA_EA: 0x8456cae
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_UPDATE_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_UPDATE_SERVER_MESSAGE * Stream::GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>(void)
template <>
SIG_UPDATE_SERVER_MESSAGE* Stream::GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>() {
    const int bytes = 96;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_UPDATE_SERVER_MESSAGE* result =
        reinterpret_cast<SIG_UPDATE_SERVER_MESSAGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84531B2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_UPDATE_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_UPDATE_SERVER_MESSAGE * CStreamGuard::GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>(void)
template <>
SIG_UPDATE_SERVER_MESSAGE* CStreamGuard::GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>() {
    return stream_->GetInBuffer<SIG_UPDATE_SERVER_MESSAGE>();
}

// IDA_EA: 0x8456d18
// IDA_NAME: _ZN6Stream11GetInBufferI24SIG_EVENT_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_EVENT_SERVER_MESSAGE * Stream::GetInBuffer<SIG_EVENT_SERVER_MESSAGE>(void)
template <>
SIG_EVENT_SERVER_MESSAGE* Stream::GetInBuffer<SIG_EVENT_SERVER_MESSAGE>() {
    const int bytes = 4404;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_EVENT_SERVER_MESSAGE* result =
        reinterpret_cast<SIG_EVENT_SERVER_MESSAGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84532C4
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI24SIG_EVENT_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_EVENT_SERVER_MESSAGE * CStreamGuard::GetInBuffer<SIG_EVENT_SERVER_MESSAGE>(void)
template <>
SIG_EVENT_SERVER_MESSAGE* CStreamGuard::GetInBuffer<SIG_EVENT_SERVER_MESSAGE>() {
    return stream_->GetInBuffer<SIG_EVENT_SERVER_MESSAGE>();
}

// IDA_EA: 0x8602160
// IDA_NAME: _ZN6Stream11GetInBufferI25SIG_SELECT_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_SELECT_SERVER_MESSAGE * Stream::GetInBuffer<SIG_SELECT_SERVER_MESSAGE>(void)
template <>
SIG_SELECT_SERVER_MESSAGE* Stream::GetInBuffer<SIG_SELECT_SERVER_MESSAGE>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SELECT_SERVER_MESSAGE* result =
        reinterpret_cast<SIG_SELECT_SERVER_MESSAGE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8601CA6
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI25SIG_SELECT_SERVER_MESSAGEEEPT_v
// DEMANGLED: SIG_SELECT_SERVER_MESSAGE * CStreamGuard::GetInBuffer<SIG_SELECT_SERVER_MESSAGE>(void)
template <>
SIG_SELECT_SERVER_MESSAGE* CStreamGuard::GetInBuffer<SIG_SELECT_SERVER_MESSAGE>() {
    return stream_->GetInBuffer<SIG_SELECT_SERVER_MESSAGE>();
}
