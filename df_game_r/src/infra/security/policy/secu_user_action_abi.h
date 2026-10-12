#ifndef OLDSRV_GLOBAL_SECU_USER_ACTION_ABI_H_
#define OLDSRV_GLOBAL_SECU_USER_ACTION_ABI_H_

#include <stdint.h>

// IDA unit: 99_misc_global/cSecuUserAction.  CUser owns only a pointer to
// this 4-byte allocation at expanded +0x8ec98; it is allocated for every accepted TCP
// session and initialized before the client handshake proceeds.
class cSecuUserAction {
public:
    // IDA_EA: 0x86978BA
    // IDA_NAME: _ZN15cSecuUserActionC2Ev
    // DEMANGLED: cSecuUserAction::cSecuUserAction(void)
    cSecuUserAction();

    // IDA_EA: 0x827B3F4
    // IDA_NAME: _ZN15cSecuUserAction4InitEv
    // DEMANGLED: cSecuUserAction::Init(void)
    cSecuUserAction* Init();

    // IDA_EA: 0x827B402
    // IDA_NAME: _ZN15cSecuUserAction18IsSecurityStateOffEv
    // DEMANGLED: cSecuUserAction::IsSecurityStateOff(void)
    bool IsSecurityStateOff();

private:
    uint32_t security_state_;
};

#if defined(__i386__)
static_assert(sizeof(cSecuUserAction) == 0x04u,
              "cSecuUserAction must preserve its old i386 extent");
#endif

#endif  // OLDSRV_GLOBAL_SECU_USER_ACTION_ABI_H_
