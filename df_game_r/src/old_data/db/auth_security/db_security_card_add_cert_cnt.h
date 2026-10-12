#ifndef OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_ADD_CERT_CNT_H_
#define OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_ADD_CERT_CNT_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: dispatch @0x843630E; makeRequest @0x8436388; ctor/dtor
// @0x844EF8A/@0x845D3EA/@0x845D41A. DBTask signal 380.
class DB_SecurityCardAddCertCnt : public DBDispatcher {
public:
    DB_SecurityCardAddCertCnt();
    virtual ~DB_SecurityCardAddCertCnt();

    // IDA_NAME: _ZN25DB_SecurityCardAddCertCnt8dispatchEiiP6Stream
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id);
};


#endif  // OLDSRV_OLD_DATA_DB_AUTH_SECURITY_DB_SECURITY_CARD_ADD_CERT_CNT_H_
