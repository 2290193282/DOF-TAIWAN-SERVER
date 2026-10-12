#ifndef OLDSRV_DB_DB_SECURITY_CARD_RETRANSFER_H_
#define OLDSRV_DB_DB_SECURITY_CARD_RETRANSFER_H_

#include "old_data/db/core/db_dispatcher.h"

enum ENUM_ERROR : int;


// IDA unit: 02_login_account/DB_SecurityCardRetransfer.
// IDA_EA: makeRequest @0x842C1F6; dispatch @0x842C362;
// _sendResult @0x842C7BE; ctor/dtor @0x844E96A/@0x845E28A/@0x845E2BA.
class DB_SecurityCardRetransfer : public DBDispatcher {
public:
    DB_SecurityCardRetransfer();
    virtual ~DB_SecurityCardRetransfer();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void _sendResult(int user_idx,
                            char* phone,
                            const char* server_key,
                            const char* card,
                            ENUM_ERROR error);
    static void makeRequest(int user_idx,
                            const char* phone,
                            const char* current_phone_hash,
                            unsigned int account_id,
                            const char* cert_key,
                            const char* server_key,
                            const char* card);
};

#endif  // OLDSRV_DB_DB_SECURITY_CARD_RETRANSFER_H_
