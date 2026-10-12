#ifndef OLDSRV_PROTOCOL_LOGOUT_PAYLOAD_H_
#define OLDSRV_PROTOCOL_LOGOUT_PAYLOAD_H_

#include <stdint.h>

// IDA_EA: 0x869AC86 / 0x86A0108
// IDA_NAME: _ZN12CStreamGuard11GetInBufferI17SIG_UPDATE_LOGOUTEEPT_v /
//           _ZN6Stream11GetInBufferI17SIG_UPDATE_LOGOUTEEPT_v
// DEMANGLED: SIG_UPDATE_LOGOUT * CStreamGuard::GetInBuffer<SIG_UPDATE_LOGOUT>(void)
//            SIG_UPDATE_LOGOUT * Stream::GetInBuffer<SIG_UPDATE_LOGOUT>(void)
// The same 139-byte record is consumed by DB_UpdateLogout at 0x8418A96.
#pragma pack(push, 1)
struct SIG_UPDATE_LOGOUT {
    uint32_t m_id;                   // +0x00
    uint32_t charac_no;              // +0x04
    int32_t channel_no;              // +0x08
    uint16_t fatigue;                // +0x0C
    int32_t server_id;               // +0x0E
    uint8_t write_member_play_info;  // +0x12
    int32_t play_time;               // +0x13
    int32_t trade_count;             // +0x17
    uint32_t exp;                    // +0x1B
    uint16_t used_fatigue;           // +0x1F
    uint8_t ting_flag;               // +0x21
    uint8_t pcbang_flag;             // +0x22
    uint32_t last_play_time;         // +0x23
    char peer_ip[16];                // +0x27
    char ip[12];                     // +0x37
    char end_ip[4];                  // +0x43
    char mac_addr[65];               // +0x47
    uint8_t save_member_bonus;       // +0x88
    int16_t member_bonus_fatigue;    // +0x89
};
#pragma pack(pop)

static_assert(sizeof(SIG_UPDATE_LOGOUT) == 139u,
              "SIG_UPDATE_LOGOUT IDA ABI size must stay 139");

#endif  // OLDSRV_PROTOCOL_LOGOUT_PAYLOAD_H_
