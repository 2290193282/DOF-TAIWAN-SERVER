#include "old_data/db/legacy_dispatch/arad_dispatcher.h"

#include "infra/network/packet/stream_guard.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x81977B6
// IDA_NAME: _ZN4ARAD10DISPATCHER24make_internal_stream_jpnER12CStreamGuardNS_23ENUM_INTERNALPACKET_JPNEi
// DEMANGLED: ARAD::DISPATCHER::make_internal_stream_jpn(CStreamGuard &,ARAD::ENUM_INTERNALPACKET_JPN,int)
int make_internal_stream_jpn(CStreamGuard& guard,
                             ENUM_INTERNALPACKET_JPN packet,
                             int uid) {
    // The exported pseudocode narrowed the two argument values to chars.
    // The original i386 instructions call CStreamGuard::operator<<(int) for
    // all three fields: DBTask signal 0x2DC, user index, then ARAD code.
    *guard << 0x2DC;
    *guard << uid;
    *guard << static_cast<int>(packet);
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
