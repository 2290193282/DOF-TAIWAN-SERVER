#ifndef DB_ZERO_TRADE_PAYLOAD_H_
#define DB_ZERO_TRADE_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// IDA_EA: Stream::GetOutBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG> @0x8451D08
// and CStreamGuard::GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG> @0x85321A0.
#pragma pack(push, 1)
struct SIG_SAVE_GOLD_ZERO_TRADE_LOG {
    uint32_t to_m_id;                       // +0x00
    uint32_t from_m_id;                     // +0x04
    char from_nat_ip[16];                   // +0x08
    uint8_t reserved_18[4];                 // +0x18
    char from_peer_ip[20];                  // +0x1C
    uint8_t from_nat_type;                  // +0x30
    char from_mac[65];                      // +0x31
    char to_nat_ip[16];                     // +0x72
    char to_peer_ip[20];                    // +0x82
    uint8_t to_nat_type;                    // +0x96
    char to_mac[65];                        // +0x97
    uint8_t reserved_d8[4];                 // +0xD8
    int32_t injury_money;                   // +0xDC
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_GOLD_ZERO_TRADE_LOG) == 0xE0u,
              "SIG_SAVE_GOLD_ZERO_TRADE_LOG must stay 0xE0");
static_assert(offsetof(SIG_SAVE_GOLD_ZERO_TRADE_LOG, from_nat_type) == 0x30u,
              "SIG_SAVE_GOLD_ZERO_TRADE_LOG from NAT type must stay 0x30");
static_assert(offsetof(SIG_SAVE_GOLD_ZERO_TRADE_LOG, to_nat_ip) == 0x72u,
              "SIG_SAVE_GOLD_ZERO_TRADE_LOG to NAT IP must stay 0x72");
static_assert(offsetof(SIG_SAVE_GOLD_ZERO_TRADE_LOG, injury_money) == 0xDCu,
              "SIG_SAVE_GOLD_ZERO_TRADE_LOG injury money must stay 0xDC");

#endif  // DB_ZERO_TRADE_PAYLOAD_H_
