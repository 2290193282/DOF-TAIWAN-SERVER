#ifndef OLDSRV_DB_DB_REQ_CHANGE_CHARAC_NAME_H_
#define OLDSRV_DB_DB_REQ_CHANGE_CHARAC_NAME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/db_dispatcher.h"


// IDA ABI: the request and response intentionally reuse the same 0x30-byte
// stream type.  The unions name the two meanings observed at the same slots:
// request_flag/guild_id/server_id on DB_Q and
// result_code/request_flag/guild_id on DISPATCH_Q.
struct SIG_CHANGE_CHARAC_NAME {
    uint32_t charac_no;       // +0x00
    char charac_name[32];     // +0x04
    union RequestStatus {
        int32_t request_flag;
        int32_t result_code;
    } status;                 // +0x24
    union GuildContext {
        uint32_t guild_id;
        int32_t result_request_flag;
    } guild_context;          // +0x28
    union ServerOrGuild {
        uint8_t server_id;
        uint32_t result_guild_id;
    } server_or_guild;        // +0x2C
};

static_assert(sizeof(SIG_CHANGE_CHARAC_NAME) == 0x30u,
              "SIG_CHANGE_CHARAC_NAME IDA ABI size must stay 0x30");
static_assert(offsetof(SIG_CHANGE_CHARAC_NAME, charac_name) == 0x04u,
              "SIG_CHANGE_CHARAC_NAME charac_name offset must stay 0x04");
static_assert(offsetof(SIG_CHANGE_CHARAC_NAME, status) == 0x24u,
              "SIG_CHANGE_CHARAC_NAME status offset must stay 0x24");
static_assert(offsetof(SIG_CHANGE_CHARAC_NAME, guild_context) == 0x28u,
              "SIG_CHANGE_CHARAC_NAME guild_context offset must stay 0x28");
static_assert(offsetof(SIG_CHANGE_CHARAC_NAME, server_or_guild) == 0x2Cu,
              "SIG_CHANGE_CHARAC_NAME server_or_guild offset must stay 0x2C");


class MySQL;

// IDA unit: 04_character_select_create/DB_ReqChangeCharacName.
// IDA_EA: dispatch @0x84262EA; makeRequest @0x8426590; _sendResult
// @0x84266A4; _hasSameCharacName @0x84267C2; ctor/dtor
// @0x844E63E/@0x845EB60/@0x845EB90. DBTask slot 195.
class DB_ReqChangeCharacName : public DBDispatcher {
public:
    DB_ReqChangeCharacName();
    virtual ~DB_ReqChangeCharacName();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx,
                            int charac_no,
                            const char* charac_name,
                            int request_flag);

private:
    static void _sendResult(const SIG_CHANGE_CHARAC_NAME* payload,
                            int user_idx,
                            int result_code);
    int _hasSameCharacName(MySQL* db_handle,
                           const char* database_name,
                           const char* escaped_name);
};

#endif  // OLDSRV_DB_DB_REQ_CHANGE_CHARAC_NAME_H_
