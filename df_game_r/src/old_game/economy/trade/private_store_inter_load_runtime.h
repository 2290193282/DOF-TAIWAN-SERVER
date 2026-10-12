#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PRIVATE_STORE_INTER_LOAD_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PRIVATE_STORE_INTER_LOAD_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

// IDA unit: 13_database_dispatch/Inter_LoadPrivateStore.
// It is the database-reply ingress for private-store loading.  The class is
// only the one-vptr ISigDispatcher base on i386; do not add state here.
class Inter_LoadPrivateStore : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED5B0
    // IDA_NAME: _ZN22Inter_LoadPrivateStoreC2Ev
    Inter_LoadPrivateStore();

    // IDA_EA: 0x84F9DF2 / 0x84F9E22
    // IDA_NAME: _ZN22Inter_LoadPrivateStoreD2Ev / _ZN22Inter_LoadPrivateStoreD0Ev
    virtual ~Inter_LoadPrivateStore();

    // IDA_EA: 0x84CF55A
    // IDA_NAME: _ZN22Inter_LoadPrivateStore12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_LoadPrivateStore::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadPrivateStore) == 0x04u,
              "Inter_LoadPrivateStore must remain its one-vptr IDA layout");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PRIVATE_STORE_INTER_LOAD_RUNTIME_H_
