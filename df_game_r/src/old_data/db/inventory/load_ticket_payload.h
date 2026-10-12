#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_LOAD_TICKET_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_LOAD_TICKET_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

// IDA_EA: DB_LoadTicket::dispatch @0x84195DC,
// DB_LoadTicket::LoadTicket @0x841968A, and CUser::GetTicket @0x867846E.
// Stream::GetOutBuffer<SIG_LOAD_TICKET> @0x8450F20 reserves 0x19c bytes.
#pragma pack(push, 1)
struct SIG_LOAD_TICKET {
    uint32_t charac_no;       // +0x000
    uint32_t result;          // +0x004
    uint32_t count;           // +0x008
    int32_t villages[100];    // +0x00c
};
#pragma pack(pop)

static_assert(sizeof(SIG_LOAD_TICKET) == 0x19cu,
              "SIG_LOAD_TICKET IDA ABI size must stay 0x19c");
static_assert(offsetof(SIG_LOAD_TICKET, charac_no) == 0x000u,
              "SIG_LOAD_TICKET character offset must stay 0");
static_assert(offsetof(SIG_LOAD_TICKET, result) == 0x004u,
              "SIG_LOAD_TICKET result offset must stay 4");
static_assert(offsetof(SIG_LOAD_TICKET, count) == 0x008u,
              "SIG_LOAD_TICKET count offset must stay 8");
static_assert(offsetof(SIG_LOAD_TICKET, villages) == 0x00cu,
              "SIG_LOAD_TICKET village list offset must stay 12");

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_LOAD_TICKET_PAYLOAD_H_
