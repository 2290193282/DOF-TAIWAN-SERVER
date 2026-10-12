#ifndef DB_PURCHASE_CASH_ITEM_BONUS_PAYLOAD_H_
#define DB_PURCHASE_CASH_ITEM_BONUS_PAYLOAD_H_

#include <stdint.h>

// IDA_EA: CStreamGuard::GetInBuffer<SIG_PURCHASE_CASHITEM_BONUS> @0x8453BA6.
// DB_UpdatePurchaseCashItemBonus::dispatch @0x843E252 writes the database
// purchase count through this four-byte result slot.
#pragma pack(push, 1)
struct SIG_PURCHASE_CASHITEM_BONUS {
    uint32_t purchase_count;  // +0x00
};
#pragma pack(pop)

typedef char SIG_PURCHASE_CASHITEM_BONUS_Size_Must_Be_4[
    (sizeof(SIG_PURCHASE_CASHITEM_BONUS) == 4) ? 1 : -1];

#endif  // DB_PURCHASE_CASH_ITEM_BONUS_PAYLOAD_H_
