#include "infra/network/packet/stream_guard.h"
#include "old_game/activity/schedule/event_messages.h"

// IDA_EA: 0x815f26c
// IDA_NAME: _ZN6Stream11GetInBufferIN6Taiwan16SigStayTimeEventEEEPT_v
// DEMANGLED: Taiwan::SigStayTimeEvent * Stream::GetInBuffer<Taiwan::SigStayTimeEvent>(void)
template <>
Taiwan::SigStayTimeEvent* Stream::GetInBuffer<Taiwan::SigStayTimeEvent>() {
    const int bytes = 4;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    Taiwan::SigStayTimeEvent* result =
        reinterpret_cast<Taiwan::SigStayTimeEvent*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x815EA76
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN6Taiwan16SigStayTimeEventEEEPT_v
// DEMANGLED: Taiwan::SigStayTimeEvent * CStreamGuard::GetInBuffer<Taiwan::SigStayTimeEvent>(void)
template <>
Taiwan::SigStayTimeEvent* CStreamGuard::GetInBuffer<Taiwan::SigStayTimeEvent>() {
    return stream_->GetInBuffer<Taiwan::SigStayTimeEvent>();
}

// IDA_EA: 0x845713c
// IDA_NAME: _ZN6Stream11GetInBufferI37SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGEREEPT_v
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER * Stream::GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>(void)
template <>
SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* Stream::GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>() {
    const int bytes = 740;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* result =
        reinterpret_cast<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x845373C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI37SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGEREEPT_v
// DEMANGLED: SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER * CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>(void)
template <>
SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* CStreamGuard::GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>() {
    return stream_->GetInBuffer<SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER>();
}

// IDA_EA: 0x8457ef0
// IDA_NAME: _ZN6Stream11GetInBufferI19SIG_EVENT_STAGE_LOGEEPT_v
// DEMANGLED: SIG_EVENT_STAGE_LOG * Stream::GetInBuffer<SIG_EVENT_STAGE_LOG>(void)
template <>
SIG_EVENT_STAGE_LOG* Stream::GetInBuffer<SIG_EVENT_STAGE_LOG>() {
    const int bytes = 120;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_EVENT_STAGE_LOG* result =
        reinterpret_cast<SIG_EVENT_STAGE_LOG*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x84546CE
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI19SIG_EVENT_STAGE_LOGEEPT_v
// DEMANGLED: SIG_EVENT_STAGE_LOG * CStreamGuard::GetInBuffer<SIG_EVENT_STAGE_LOG>(void)
template <>
SIG_EVENT_STAGE_LOG* CStreamGuard::GetInBuffer<SIG_EVENT_STAGE_LOG>() {
    return stream_->GetInBuffer<SIG_EVENT_STAGE_LOG>();
}
