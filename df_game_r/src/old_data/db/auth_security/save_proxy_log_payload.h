#ifndef OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SAVE_PROXY_LOG_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SAVE_PROXY_LOG_PAYLOAD_H_

#include <stdint.h>

// IDA payload consumed by DB_SaveProxyLog and produced by CMD 2
// DisPatcher_SetIP through CStreamGuard::GetInBuffer<SIG_SAVE_PROXY_LOG>().
// The DB wire read is exactly 45 bytes; preserve its packed field order.
#pragma pack(push, 1)
struct SIG_SAVE_PROXY_LOG {
    uint32_t m_id;         // +0x00
    char first_ip[20];     // +0x04
    char second_ip[20];    // +0x18
    uint8_t proxy_flag;    // +0x2C
};
#pragma pack(pop)

typedef char SIG_SAVE_PROXY_LOG_Size_Must_Be_45[
    (sizeof(SIG_SAVE_PROXY_LOG) == 45) ? 1 : -1];

#endif  // OLDSRV_OLD_DATA_DB_AUTH_SECURITY_SAVE_PROXY_LOG_PAYLOAD_H_
