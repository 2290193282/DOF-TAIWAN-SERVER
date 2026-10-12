#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_CHARAC_INFORM_NOTICE_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_CHARAC_INFORM_NOTICE_PAYLOAD_H_

#include <stdint.h>

#pragma pack(push, 1)
struct SIG_SAVE_CHARAC_INFORM_NOTICE {
    uint32_t charac_no;  // IDA +0x00
    char flags[16];      // IDA +0x04
};
#pragma pack(pop)

static_assert(sizeof(SIG_SAVE_CHARAC_INFORM_NOTICE) == 0x14u,
              "SIG_SAVE_CHARAC_INFORM_NOTICE IDA ABI size must stay 0x14");

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_CHARAC_INFORM_NOTICE_PAYLOAD_H_
