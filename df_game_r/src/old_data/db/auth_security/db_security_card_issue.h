#ifndef OLDSRV_DB_DB_SECURITY_CARD_ISSUE_H_
#define OLDSRV_DB_DB_SECURITY_CARD_ISSUE_H_

#include "old_data/db/core/db_dispatcher.h"

enum ENUM_ERROR : int;


// IDA_EA: 0x842AF94/0x842B410/0x842B57A/0x844E8C2/0x845E45E/0x845E48E.
// IDA unit: 02_login_account/DB_SecurityCardIssue.
class DB_SecurityCardIssue : public DBDispatcher {
public:
    DB_SecurityCardIssue();
    virtual ~DB_SecurityCardIssue();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void _sendResult(int user_idx,
                            const char* phone,
                            const char* server_key,
                            const char* card,
                            const char* q_pos,
                            ENUM_ERROR error);
    static void makeRequest(int user_idx,
                            unsigned int account_id,
                            const char* cert_key,
                            const char* server_key,
                            const char* phone,
                            const char* phone_hash,
                            const char* card);
};

#endif  // OLDSRV_DB_DB_SECURITY_CARD_ISSUE_H_
