#ifndef OLDSRV_PROTOCOL_ARAD_PURCHASE_COUNT_BUFFER_H_
#define OLDSRV_PROTOCOL_ARAD_PURCHASE_COUNT_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mail/mail_trade_params.h"
#include "infra/network/packet/stream_guard.h"


namespace ARAD {
namespace INTERNAL_PACKET {

// IDA_EA: CStreamGuard::GetInBuffer<PURCHASE_COUNT_CHECK> @0x818EE3C and
// Stream::GetInBuffer<PURCHASE_COUNT_CHECK> @0x818EE52 both use 0x3F85.
// The leading byte is copied from BuyItemInfo without an inferred outcome
// meaning; the remaining bytes are the original CeraShop request image.
#pragma pack(push, 1)
struct PURCHASE_COUNT_CHECK {
    uint8_t result;                     // +0x0000
    MSG_BUY_CERASHOP_ITEM request;      // +0x0001
};
#pragma pack(pop)

static_assert(sizeof(PURCHASE_COUNT_CHECK) == 0x3f85u,
              "PURCHASE_COUNT_CHECK must retain the old 0x3F85 byte ABI");
static_assert(offsetof(PURCHASE_COUNT_CHECK, request) == 0x01u,
              "PURCHASE_COUNT_CHECK request must begin after its result byte");

}  // namespace INTERNAL_PACKET
}  // namespace ARAD


// IDA_EA: 0x818EDE8
// IDA_NAME: _ZN6Stream12GetOutBufferI21MSG_BUY_CERASHOP_ITEMEEPT_v
// DEMANGLED: MSG_BUY_CERASHOP_ITEM * Stream::GetOutBuffer<MSG_BUY_CERASHOP_ITEM>(void)
template <>
MSG_BUY_CERASHOP_ITEM* Stream::GetOutBuffer<MSG_BUY_CERASHOP_ITEM>();

// IDA_EA: 0x818EE3C
// IDA_NAME: _ZN12CStreamGuard11GetInBufferIN4ARAD15INTERNAL_PACKET20PURCHASE_COUNT_CHECKEEEPT_v
// DEMANGLED: ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK * CStreamGuard::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>(void)
template <>
ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK*
CStreamGuard::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>();

// IDA_EA: 0x818EE52
// IDA_NAME: _ZN6Stream11GetInBufferIN4ARAD15INTERNAL_PACKET20PURCHASE_COUNT_CHECKEEEPT_v
// DEMANGLED: ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK * Stream::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>(void)
template <>
ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK*
Stream::GetInBuffer<ARAD::INTERNAL_PACKET::PURCHASE_COUNT_CHECK>();


#endif  // OLDSRV_PROTOCOL_ARAD_PURCHASE_COUNT_BUFFER_H_
