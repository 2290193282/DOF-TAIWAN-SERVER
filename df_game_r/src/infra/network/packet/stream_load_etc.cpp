#include "old_data/db/character/load_etc_payload.h"


// IDA_EA: 0x845063C
// IDA_NAME: _ZN6Stream12GetOutBufferI12SIG_LOAD_ETCEEPT_v
// DEMANGLED: SIG_LOAD_ETC * Stream::GetOutBuffer<SIG_LOAD_ETC>(void)
template <>
SIG_LOAD_ETC* Stream::GetOutBuffer<SIG_LOAD_ETC>() {
    const int bytes = sizeof(SIG_LOAD_ETC);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_LOAD_ETC* result =
        reinterpret_cast<SIG_LOAD_ETC*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
