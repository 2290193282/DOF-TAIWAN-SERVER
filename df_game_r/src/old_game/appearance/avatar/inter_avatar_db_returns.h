#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_AVATAR_DB_RETURNS_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_AVATAR_DB_RETURNS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

class Inter_AvatarInsert : public ISigDispatcher {
public:
    // IDA_EA: 0x84CF5A4 / 0x84ED5CC / 0x84F9DA4 / 0x84F9DD4.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_AvatarInsert();
    virtual ~Inter_AvatarInsert();
};

class Inter_AvatarChangeStat : public ISigDispatcher {
public:
    // IDA_EA: 0x84CF6EC / 0x84ED5E8 / 0x84F9D56 / 0x84F9D86.
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_AvatarChangeStat();
    virtual ~Inter_AvatarChangeStat();
};

#if defined(__i386__)
static_assert(sizeof(Inter_AvatarInsert) == 0x04u,
              "Inter_AvatarInsert must retain the i386 vptr-only ABI");
static_assert(sizeof(Inter_AvatarChangeStat) == 0x04u,
              "Inter_AvatarChangeStat must retain the i386 vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_INTER_AVATAR_DB_RETURNS_H_
