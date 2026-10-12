#include "old_data/db/dungeon/best_clear_time_payload.h"
#include "infra/network/packet/stream.h"

// IDA_EA: 0x84545D2
// IDA_NAME: _ZN6Stream12GetOutBufferI19SIG_BEST_CLEAR_TIMEEEPT_v
// DEMANGLED: SIG_BEST_CLEAR_TIME * Stream::GetOutBuffer<SIG_BEST_CLEAR_TIME>(void)
template <>
SIG_BEST_CLEAR_TIME* Stream::GetOutBuffer<SIG_BEST_CLEAR_TIME>() {
    const int bytes = sizeof(SIG_BEST_CLEAR_TIME);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_BEST_CLEAR_TIME* result =
        reinterpret_cast<SIG_BEST_CLEAR_TIME*>(get_buffer());
    out_ptr(bytes);
    return result;
}
