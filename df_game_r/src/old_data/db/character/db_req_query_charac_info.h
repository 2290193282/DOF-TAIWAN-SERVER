#ifndef OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_H_
#define OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/character/query_charac_info_payload.h"


// IDA unit: 04_character_select_create/DB_ReqQueryCharacInfo.
// IDA_EA: dispatch @0x842688C; makeRequest @0x8426D18; _sendResult
// @0x8426E04; _getGiftRecvCount @0x8426F46; ctor/dtor
// @0x844E65A/@0x845EB12/@0x845EB42. DBTask active slot 196.
class DB_ReqQueryCharacInfo : public DBDispatcher {
public:
    DB_ReqQueryCharacInfo();
    virtual ~DB_ReqQueryCharacInfo();

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
                           int sex,
                           int gift_recv_count);
    int _getGiftRecvCount(unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_REQ_QUERY_CHARAC_INFO_H_
