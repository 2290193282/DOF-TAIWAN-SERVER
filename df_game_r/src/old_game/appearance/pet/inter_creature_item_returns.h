#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_CREATURE_ITEM_RETURNS_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_CREATURE_ITEM_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_InsertCreatureItemResult : public ISigDispatcher {
public:
    // IDA_EA: 0x84D027E
    // IDA_NAME: _ZN30Inter_InsertCreatureItemResult12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED674 / 0x84F9BD0 / 0x84F9C00
    Inter_InsertCreatureItemResult();
    virtual ~Inter_InsertCreatureItemResult();
};

class Inter_UpdateCreatureItemResult : public ISigDispatcher {
public:
    // IDA_EA: 0x84D02EE
    // IDA_NAME: _ZN30Inter_UpdateCreatureItemResult12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84ED690 / 0x84F9B82 / 0x84F9BB2
    Inter_UpdateCreatureItemResult();
    virtual ~Inter_UpdateCreatureItemResult();
};

#if defined(__i386__)
static_assert(sizeof(Inter_InsertCreatureItemResult) == 0x04u,
              "Inter_InsertCreatureItemResult must remain vptr-only on i386");
static_assert(sizeof(Inter_UpdateCreatureItemResult) == 0x04u,
              "Inter_UpdateCreatureItemResult must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_CREATURE_ITEM_RETURNS_H_
