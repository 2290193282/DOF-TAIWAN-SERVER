#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "infra/network/packet/fair_pvp_score_stream.h"

static_assert(sizeof(SIG_FAIR_PVP_SCORE) == 1672u,
              "SIG_FAIR_PVP_SCORE stream ABI size must stay 1672");

// IDA_EA: 0x84A67F2
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI18SIG_FAIR_PVP_SCOREEEPT_v
// DEMANGLED: SIG_FAIR_PVP_SCORE * CStreamGuard::GetInBuffer<SIG_FAIR_PVP_SCORE>(void)
template <>
SIG_FAIR_PVP_SCORE*
CStreamGuard::GetInBuffer<SIG_FAIR_PVP_SCORE>() {
    return stream_->GetInBuffer<SIG_FAIR_PVP_SCORE>();
}

// IDA_EA: 0x84A6E98
// IDA_NAME: _ZN6Stream11GetInBufferI18SIG_FAIR_PVP_SCOREEEPT_v
// DEMANGLED: SIG_FAIR_PVP_SCORE * Stream::GetInBuffer<SIG_FAIR_PVP_SCORE>(void)
template <>
SIG_FAIR_PVP_SCORE*
Stream::GetInBuffer<SIG_FAIR_PVP_SCORE>() {
    const int bytes = sizeof(SIG_FAIR_PVP_SCORE);
    while (enable_in(bytes) != true) {
        resize(size() * 2);
    }
    SIG_FAIR_PVP_SCORE* const result =
        reinterpret_cast<SIG_FAIR_PVP_SCORE*>(buffer_ + in_ptr_);
    in_ptr(bytes);
    return result;
}

// IDA_EA: 0x8453C10
// IDA_NAME: _ZN6Stream12GetOutBufferI18SIG_FAIR_PVP_SCOREEEPT_v
// DEMANGLED: SIG_FAIR_PVP_SCORE * Stream::GetOutBuffer<SIG_FAIR_PVP_SCORE>(void)
template <>
SIG_FAIR_PVP_SCORE*
Stream::GetOutBuffer<SIG_FAIR_PVP_SCORE>() {
    const int bytes = sizeof(SIG_FAIR_PVP_SCORE);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_FAIR_PVP_SCORE* const result =
        reinterpret_cast<SIG_FAIR_PVP_SCORE*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
