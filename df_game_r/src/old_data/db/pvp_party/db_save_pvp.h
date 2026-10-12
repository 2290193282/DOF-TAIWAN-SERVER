#ifndef OLDSRV_DB_DB_SAVE_PVP_H_
#define OLDSRV_DB_DB_SAVE_PVP_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"
#include "old_game/pvp/battlefield/battle_field.h"


// IDA ABI: Stream::GetOutBuffer<SIG_SAVE_PVP> @0x8450ADC and
// CStreamGuard::GetInBuffer<SIG_SAVE_PVP> @0x869AA76 use 0x88 bytes.
// CUser copies the canonical 0x84-byte PvpResultType first, then writes the
// character number at dword 33 before DB signal 37 is queued.
struct SIG_SAVE_PVP {
    PvpResultType pvp_result;  // +0x00
    uint32_t charac_no;        // +0x84
};

static_assert(sizeof(SIG_SAVE_PVP) == 0x88u,
              "SIG_SAVE_PVP IDA ABI size must stay 0x88");
static_assert(offsetof(SIG_SAVE_PVP, charac_no) == 0x84u,
              "SIG_SAVE_PVP charac_no offset must stay 0x84");


// IDA_EA: 0x841720E/0x841722A/0x841725A/0x8417278/0x84172C8.
// IDA unit: 13_database_dispatch/DB_SavePvp.
class DB_SavePvp : public DBDispatcher {
public:
    DB_SavePvp();
    virtual ~DB_SavePvp();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool SavePvp(SIG_SAVE_PVP* payload);
};

#endif  // OLDSRV_DB_DB_SAVE_PVP_H_
