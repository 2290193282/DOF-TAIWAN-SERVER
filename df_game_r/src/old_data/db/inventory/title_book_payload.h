#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_TITLE_BOOK_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_TITLE_BOOK_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)
struct stTitleBook_Save {
    uint8_t general_section[0x10AE];   // +0x0000
    uint8_t specific_section[0x2FA8];  // +0x10AE
    uint8_t despair[0x1C98];           // +0x4056
    uint8_t event[0x0E4C];             // +0x5CEE
    uint32_t charac_no;                 // +0x6B3A
};
#pragma pack(pop)

static_assert(sizeof(stTitleBook_Save) == 0x6B3Eu,
              "stTitleBook_Save IDA ABI size must stay 0x6b3e");
static_assert(offsetof(stTitleBook_Save, specific_section) == 0x10AEu,
              "specific title section offset must stay 0x10ae");
static_assert(offsetof(stTitleBook_Save, despair) == 0x4056u,
              "despair title section offset must stay 0x4056");
static_assert(offsetof(stTitleBook_Save, event) == 0x5CEEu,
              "event title section offset must stay 0x5cee");
static_assert(offsetof(stTitleBook_Save, charac_no) == 0x6B3Au,
              "title-book charac_no offset must stay 0x6b3a");

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_TITLE_BOOK_PAYLOAD_H_
