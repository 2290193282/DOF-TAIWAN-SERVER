#include "infra/security/policy/secu_user_action_abi.h"

// IDA_EA: 0x86978BA
// IDA_NAME: _ZN15cSecuUserActionC2Ev
// DEMANGLED: cSecuUserAction::cSecuUserAction(void)
cSecuUserAction::cSecuUserAction() {}

// IDA_EA: 0x827B3F4
// IDA_NAME: _ZN15cSecuUserAction4InitEv
// DEMANGLED: cSecuUserAction::Init(void)
cSecuUserAction* cSecuUserAction::Init() {
    security_state_ = 0u;
    return this;
}

// IDA_EA: 0x827B402
// IDA_NAME: _ZN15cSecuUserAction18IsSecurityStateOffEv
// DEMANGLED: cSecuUserAction::IsSecurityStateOff(void)
bool cSecuUserAction::IsSecurityStateOff() {
    return security_state_ == 1u;
}
