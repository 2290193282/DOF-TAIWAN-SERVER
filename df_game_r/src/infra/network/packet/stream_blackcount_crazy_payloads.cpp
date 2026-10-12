#include "infra/network/packet/stream.h"

#include "old_data/db/anti_cheat/db_req_black_count.h"
#include "old_data/db/event/db_req_save_crazy_levelup_event.h"
#include "infra/network/packet/stream_guard.h"


static_assert(sizeof(SIG_REQ_BLACK_COUNT) == 16u,
              "SIG_REQ_BLACK_COUNT Stream ABI must stay 16 bytes");
static_assert(sizeof(SIG_SAVE_CRAZY_LEVELUP_EVENT) == 13u,
              "SIG_SAVE_CRAZY_LEVELUP_EVENT Stream ABI must stay 13 bytes");

// IDA_EA: 0x8239A18
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_REQ_BLACK_COUNTEEPT_v
// DEMANGLED: SIG_REQ_BLACK_COUNT * Stream::GetInBuffer<SIG_REQ_BLACK_COUNT>(void)
template <>
SIG_REQ_BLACK_COUNT* Stream::GetInBuffer<SIG_REQ_BLACK_COUNT>() {
    const int bytes = sizeof(SIG_REQ_BLACK_COUNT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_REQ_BLACK_COUNT* result =
        reinterpret_cast<SIG_REQ_BLACK_COUNT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8236DCA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_REQ_BLACK_COUNTEEPT_v
// DEMANGLED: SIG_REQ_BLACK_COUNT * CStreamGuard::GetInBuffer<SIG_REQ_BLACK_COUNT>(void)
template <>
SIG_REQ_BLACK_COUNT* CStreamGuard::GetInBuffer<SIG_REQ_BLACK_COUNT>() {
    return stream_->GetInBuffer<SIG_REQ_BLACK_COUNT>();
}

// IDA_EA: 0x8451EEE
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_REQ_BLACK_COUNTEEPT_v
// DEMANGLED: SIG_REQ_BLACK_COUNT * Stream::GetOutBuffer<SIG_REQ_BLACK_COUNT>(void)
template <>
SIG_REQ_BLACK_COUNT* Stream::GetOutBuffer<SIG_REQ_BLACK_COUNT>() {
    const int bytes = sizeof(SIG_REQ_BLACK_COUNT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_REQ_BLACK_COUNT* result =
        reinterpret_cast<SIG_REQ_BLACK_COUNT*>(get_buffer());
    out_ptr(bytes);
    return result;
}

// IDA_EA: 0x827E518
// IDA_NAME: _ZN6Stream11GetInBufferI28SIG_SAVE_CRAZY_LEVELUP_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CRAZY_LEVELUP_EVENT * Stream::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>(void)
template <>
SIG_SAVE_CRAZY_LEVELUP_EVENT*
Stream::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>() {
    const int bytes = sizeof(SIG_SAVE_CRAZY_LEVELUP_EVENT);
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_SAVE_CRAZY_LEVELUP_EVENT* result =
        reinterpret_cast<SIG_SAVE_CRAZY_LEVELUP_EVENT*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x827DFA2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI28SIG_SAVE_CRAZY_LEVELUP_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CRAZY_LEVELUP_EVENT * CStreamGuard::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>(void)
template <>
SIG_SAVE_CRAZY_LEVELUP_EVENT*
CStreamGuard::GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>() {
    return stream_->GetInBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>();
}

// IDA_EA: 0x8451F42
// IDA_NAME: _ZN6Stream12GetOutBufferI28SIG_SAVE_CRAZY_LEVELUP_EVENTEEPT_v
// DEMANGLED: SIG_SAVE_CRAZY_LEVELUP_EVENT * Stream::GetOutBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>(void)
template <>
SIG_SAVE_CRAZY_LEVELUP_EVENT*
Stream::GetOutBuffer<SIG_SAVE_CRAZY_LEVELUP_EVENT>() {
    const int bytes = sizeof(SIG_SAVE_CRAZY_LEVELUP_EVENT);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_SAVE_CRAZY_LEVELUP_EVENT* result =
        reinterpret_cast<SIG_SAVE_CRAZY_LEVELUP_EVENT*>(get_buffer());
    out_ptr(bytes);
    return result;
}
