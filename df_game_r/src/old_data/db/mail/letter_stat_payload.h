#ifndef OLDSRV_OLD_DATA_DB_MAIL_LETTER_STAT_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_MAIL_LETTER_STAT_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

// IDA_EA: DB_ChangeLetterStat::makeRequest @0x84261FC; the A-owned
// CStreamGuard::GetInBuffer<SIG_LETTER_STAT> provider is @0x8452054.
#pragma pack(push, 1)
struct SIG_LETTER_STAT {
    int32_t charac_no;      // +0x00
    int32_t letter_id;      // +0x04
    int16_t letter_stat;    // +0x08
    uint8_t reserved_0a[2]; // +0x0A
};
#pragma pack(pop)

static_assert(sizeof(SIG_LETTER_STAT) == 0x0Cu,
              "SIG_LETTER_STAT must preserve the original 12-byte wire ABI");
static_assert(offsetof(SIG_LETTER_STAT, letter_stat) == 0x08u,
              "SIG_LETTER_STAT letter state must stay at +0x08");

#endif  // OLDSRV_OLD_DATA_DB_MAIL_LETTER_STAT_PAYLOAD_H_
