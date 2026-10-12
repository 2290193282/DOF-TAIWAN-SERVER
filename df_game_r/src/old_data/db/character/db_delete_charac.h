#ifndef OLDSRV_DB_DB_DELETE_CHARAC_H_
#define OLDSRV_DB_DB_DELETE_CHARAC_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>


#pragma pack(push, 1)
// Stream::GetOutBuffer<SIG_DELETE_CHARAC> @0x8402C7F returns this exact
// 20-byte request/result object.
struct SIG_DELETE_CHARAC {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t result;
    uint32_t guild_id;
    uint8_t event_character_deleted;
    uint8_t reserved[3];
};
#pragma pack(pop)

static_assert(sizeof(SIG_DELETE_CHARAC) == 20u,
              "SIG_DELETE_CHARAC IDA ABI size must stay 20");


// IDA_EA: 0x8402BCE/0x8402BEA/0x8402C1A/0x8402C38/0x8402F4C/0x84032AA/0x8403324/0x840340C/0x84034FE/0x84035E0.
// IDA unit: 04_character_select_create/DB_DeleteCharac; DBTask signal 5.
class DB_DeleteCharac : public DBDispatcher {
public:
    DB_DeleteCharac();
    virtual ~DB_DeleteCharac();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool DeleteGuildInfo(SIG_DELETE_CHARAC* payload);
    bool DeleteEventCharacFlag(SIG_DELETE_CHARAC* payload);
    bool CheckEventCharacter40(SIG_DELETE_CHARAC* payload);
    bool DeleteCharac(SIG_DELETE_CHARAC* payload);
    bool SendResult(int signal, int user_idx, SIG_DELETE_CHARAC* payload);
    bool DeleteCharacOption(SIG_DELETE_CHARAC* payload);
};

#endif  // OLDSRV_DB_DB_DELETE_CHARAC_H_
