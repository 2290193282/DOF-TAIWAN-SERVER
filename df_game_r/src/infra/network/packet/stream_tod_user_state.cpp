#include "old_data/db/character/load_etc_payload.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x8453CCE
// IDA_NAME: _ZN6Stream12GetOutBufferI26SIG_REQUEST_TOD_USER_STATEEEPT_v
// DEMANGLED: SIG_REQUEST_TOD_USER_STATE * Stream::GetOutBuffer<SIG_REQUEST_TOD_USER_STATE>(void)
template <>
SIG_REQUEST_TOD_USER_STATE*
Stream::GetOutBuffer<SIG_REQUEST_TOD_USER_STATE>() {
    const int bytes = sizeof(SIG_REQUEST_TOD_USER_STATE);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_REQUEST_TOD_USER_STATE* result =
        reinterpret_cast<SIG_REQUEST_TOD_USER_STATE*>(get_buffer());
    out_ptr(bytes);
    return result;
}
