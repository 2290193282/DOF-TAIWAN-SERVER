#ifndef OLDSRV_DB_DB_COUNT_OF_HUMAN_CERTIFY_H_
#define OLDSRV_DB_DB_COUNT_OF_HUMAN_CERTIFY_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_CountOfHumanCertify.
// IDA_EA: dispatch @0x84282A0; makeRequest @0x84283CA; ctor/dtor
// @0x844E73A/@0x845E854/@0x845E884. makeRequest emits signal 273.
class DB_CountOfHumanCertify : public DBDispatcher {
public:
    DB_CountOfHumanCertify();
    virtual ~DB_CountOfHumanCertify();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id);
};

#endif  // OLDSRV_DB_DB_COUNT_OF_HUMAN_CERTIFY_H_
