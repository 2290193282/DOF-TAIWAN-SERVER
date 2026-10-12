#ifndef OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_MAILBOX_H_
#define OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_MAILBOX_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/character/query_charac_info_payload.h"


// IDA unit: 04_character_select_create/DB_ReqQueryCharacInfoMailbox.
// IDA_EA: dispatch @0x843BD46; makeRequest @0x843C138; _sendResult
// @0x843C224; ctor/dtor @0x844F4AE/@0x845C412/@0x845C442.
// DB request signal 246 (the response returns through DISPATCH_Q).
class DB_ReqQueryCharacInfoMailbox : public DBDispatcher {
public:
    DB_ReqQueryCharacInfoMailbox();
    virtual ~DB_ReqQueryCharacInfoMailbox();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(int user_idx, const char* charac_name);

private:
    static int _sendResult(int result,
                           int user_idx,
                           const char* charac_name,
                           int charac_no,
                           int job,
                           int level,
                           int grow_type,
                           int sex);
};

#endif  // OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_MAILBOX_H_
