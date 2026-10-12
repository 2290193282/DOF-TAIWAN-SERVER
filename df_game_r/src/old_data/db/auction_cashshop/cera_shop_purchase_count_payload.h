#ifndef DNF_DB_CERA_SHOP_PURCHASE_COUNT_PAYLOAD_H_
#define DNF_DB_CERA_SHOP_PURCHASE_COUNT_PAYLOAD_H_

#include <stdint.h>


// IDA_EA: Stream::Get{In,Out}Buffer<SIG_CERASHOP_PURCHASE_COUNT>
// @0x8457FC4/@0x84547A2. DB_UpdateCeraShopPurchaseCount uses each field.
#pragma pack(push, 1)
struct SIG_CERASHOP_PURCHASE_COUNT {
    uint32_t member_id;       // +0x00
    uint32_t item_index;      // +0x04
    uint16_t purchase_count;  // +0x08
    uint8_t abi_pad_0a[2];    // +0x0A
    int32_t server_id;        // +0x0C
};
#pragma pack(pop)

static_assert(sizeof(SIG_CERASHOP_PURCHASE_COUNT) == 16,
              "SIG_CERASHOP_PURCHASE_COUNT must match the IDA stream ABI");

#endif  // DNF_DB_CERA_SHOP_PURCHASE_COUNT_PAYLOAD_H_
