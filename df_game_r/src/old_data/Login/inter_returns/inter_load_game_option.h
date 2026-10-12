#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_GAME_OPTION_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_GAME_OPTION_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 99_misc_global/Inter_LoadGameOption.
// InterDispatcher C2 installs this base-only receiver at outer signal 303.
class Inter_LoadGameOption : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDEA8 / 0x84F85E0 / 0x84F8610
    // IDA_NAME: _ZN20Inter_LoadGameOptionC2Ev / D2Ev / D0Ev
    Inter_LoadGameOption();
    virtual ~Inter_LoadGameOption();

    // IDA_EA: 0x84D7E84
    // IDA_NAME: _ZN20Inter_LoadGameOption12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadGameOption) == 0x04u,
              "Inter_LoadGameOption must remain the base-only i386 dispatcher");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_GAME_OPTION_H_
