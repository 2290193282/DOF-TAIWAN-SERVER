#include "infra/network/packet/stream_guard.h"
#include "old_game/progression/skill/combo_skill.h"

// IDA_EA: 0x86a2860
// IDA_NAME: _ZN6Stream11GetInBufferI15SIG_COMBO_SKILLEEPT_v
// DEMANGLED: SIG_COMBO_SKILL * Stream::GetInBuffer<SIG_COMBO_SKILL>(void)
template <>
SIG_COMBO_SKILL* Stream::GetInBuffer<SIG_COMBO_SKILL>() {
    const int bytes = 178;
    while (!enable_in(bytes)) {
        resize(size() * 2);
    }
    SIG_COMBO_SKILL* result =
        reinterpret_cast<SIG_COMBO_SKILL*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x869CCCA
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI15SIG_COMBO_SKILLEEPT_v
// DEMANGLED: SIG_COMBO_SKILL * CStreamGuard::GetInBuffer<SIG_COMBO_SKILL>(void)
template <>
SIG_COMBO_SKILL* CStreamGuard::GetInBuffer<SIG_COMBO_SKILL>() {
    return stream_->GetInBuffer<SIG_COMBO_SKILL>();
}
