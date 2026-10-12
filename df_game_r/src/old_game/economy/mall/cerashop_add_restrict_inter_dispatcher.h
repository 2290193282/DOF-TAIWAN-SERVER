#ifndef OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_INTER_DISPATCHER_H_
#define OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_INTER_DISPATCHER_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

// IDA unit: Inter_LoadCerashopAddRestrictType.  This is the one-vptr
// database-reply receiver allocated by InterDispatcher; it has no data fields
// of its own on i386.
class Inter_LoadCerashopAddRestrictType : public ISigDispatcher {
public:
    // IDA_EA: 0x84EEEF4
    Inter_LoadCerashopAddRestrictType();

    // IDA_EA: 0x80E4690 / 0x80E46C0
    virtual ~Inter_LoadCerashopAddRestrictType();

    // IDA_EA: 0x80E2104
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadCerashopAddRestrictType) == 0x04u,
              "Inter_LoadCerashopAddRestrictType must be one ISigDispatcher vptr");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_INTER_DISPATCHER_H_
