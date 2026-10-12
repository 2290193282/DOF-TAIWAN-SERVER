#include "infra/network/packet/stream.h"

#include "old_data/db/character/load_etc_payload.h"


// IDA_EA: 0x8454418
// IDA_NAME: _ZN6Stream12GetOutBufferI21SIG_BOOSTER_GAGE_DATAEEPT_v
// DEMANGLED: SIG_BOOSTER_GAGE_DATA * Stream::GetOutBuffer<SIG_BOOSTER_GAGE_DATA>(void)
template <>
SIG_BOOSTER_GAGE_DATA* Stream::GetOutBuffer<SIG_BOOSTER_GAGE_DATA>() {
    const int bytes = sizeof(SIG_BOOSTER_GAGE_DATA);
    if (!enable_out(bytes)) {
        return 0;
    }
    SIG_BOOSTER_GAGE_DATA* result =
        reinterpret_cast<SIG_BOOSTER_GAGE_DATA*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
