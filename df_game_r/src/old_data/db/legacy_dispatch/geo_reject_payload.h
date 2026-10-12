#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_GEO_REJECT_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_GEO_REJECT_PAYLOAD_H_

#include <stdint.h>


// IDA unit: localtaiwan/System/RestrictGeolocation.cpp and
// 13_database_dispatch/DB_WriteGeoReject.
// The producer writes the rejected peer IP followed by the country code.  The
// DB dispatcher consumes the same two NUL-terminated 16-byte strings.
struct SIG_SAVE_GEO_REJECT {
    char reject_ip[16];       // +0x00
    char country_code[16];    // +0x10
};

static_assert(sizeof(SIG_SAVE_GEO_REJECT) == 0x20u,
              "SIG_SAVE_GEO_REJECT must retain the old 32-byte ABI");

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_GEO_REJECT_PAYLOAD_H_
