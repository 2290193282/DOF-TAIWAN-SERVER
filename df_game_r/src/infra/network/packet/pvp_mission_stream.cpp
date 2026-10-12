#include "infra/network/packet/pvp_mission_stream.h"

// IDA_EA: 0x8453BBC
// IDA_NAME: _ZN6Stream12GetOutBufferI28SIG_REQUEST_PVP_MISSION_LISTEEPT_v
// DEMANGLED: SIG_REQUEST_PVP_MISSION_LIST * Stream::GetOutBuffer<SIG_REQUEST_PVP_MISSION_LIST>(void)
template <>
SIG_REQUEST_PVP_MISSION_LIST*
Stream::GetOutBuffer<SIG_REQUEST_PVP_MISSION_LIST>() {
    const int bytes = sizeof(SIG_REQUEST_PVP_MISSION_LIST);
    if (enable_out(bytes) != true) {
        return 0;
    }
    SIG_REQUEST_PVP_MISSION_LIST* const result =
        reinterpret_cast<SIG_REQUEST_PVP_MISSION_LIST*>(buffer_ + out_ptr_);
    out_ptr(bytes);
    return result;
}
