#ifndef OLDSRV_DB_QUERY_CHARAC_INFO_PAYLOAD_H_
#define OLDSRV_DB_QUERY_CHARAC_INFO_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// IDA_EA: Stream::GetOutBuffer<SIG_QUERY_CHARAC_INFO> @0x84520D4,
// Stream::GetInBuffer<SIG_QUERY_CHARAC_INFO> @0x8455C02, and
// CStreamGuard::GetInBuffer<SIG_QUERY_CHARAC_INFO> @0x8452128.
// The old payload uses the compiler's natural alignment; the unnamed gaps at
// +0x1E..+0x1F, +0x29, and +0x2F are ABI alignment, not serialized fields.
struct SIG_QUERY_CHARAC_INFO {
    char charac_name[30];       // +0x00
    int32_t result;             // +0x20
    uint32_t charac_no;         // +0x24
    int8_t job;                 // +0x28
    int16_t level;              // +0x2A
    int8_t grow_type;           // +0x2C
    int8_t sex;                 // +0x2D
    int8_t gift_recv_count;     // +0x2E
};

static_assert(sizeof(SIG_QUERY_CHARAC_INFO) == 0x30u,
              "SIG_QUERY_CHARAC_INFO IDA ABI size must stay 0x30");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, result) == 0x20u,
              "SIG_QUERY_CHARAC_INFO result offset must stay 0x20");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, charac_no) == 0x24u,
              "SIG_QUERY_CHARAC_INFO charac_no offset must stay 0x24");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, job) == 0x28u,
              "SIG_QUERY_CHARAC_INFO job offset must stay 0x28");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, level) == 0x2Au,
              "SIG_QUERY_CHARAC_INFO level offset must stay 0x2A");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, grow_type) == 0x2Cu,
              "SIG_QUERY_CHARAC_INFO grow_type offset must stay 0x2C");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, sex) == 0x2Du,
              "SIG_QUERY_CHARAC_INFO sex offset must stay 0x2D");
static_assert(offsetof(SIG_QUERY_CHARAC_INFO, gift_recv_count) == 0x2Eu,
              "SIG_QUERY_CHARAC_INFO gift count offset must stay 0x2E");


#endif  // OLDSRV_DB_QUERY_CHARAC_INFO_PAYLOAD_H_
