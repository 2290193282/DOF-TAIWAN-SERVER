#ifndef OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_ACCOUNT_CARGO_INTER_RECEIVERS_H_
#define OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_ACCOUNT_CARGO_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;

class Inter_CreateAccountCargo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE82C
    // IDA_NAME: _ZN24Inter_CreateAccountCargoC2Ev
    // DEMANGLED: Inter_CreateAccountCargo::Inter_CreateAccountCargo(void)
    Inter_CreateAccountCargo();

    // IDA_EA: 0x84F6D80
    // IDA_NAME: _ZN24Inter_CreateAccountCargoD2Ev
    // IDA_EA: 0x84F6DB0
    // IDA_NAME: _ZN24Inter_CreateAccountCargoD0Ev
    // DEMANGLED: Inter_CreateAccountCargo::~Inter_CreateAccountCargo()
    virtual ~Inter_CreateAccountCargo();

    // IDA_EA: 0x84E23A2
    // IDA_NAME: _ZN24Inter_CreateAccountCargo12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_CreateAccountCargo::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_UpgradeAccountCargo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE848
    // IDA_NAME: _ZN25Inter_UpgradeAccountCargoC2Ev
    // DEMANGLED: Inter_UpgradeAccountCargo::Inter_UpgradeAccountCargo(void)
    Inter_UpgradeAccountCargo();

    // IDA_EA: 0x84F6D32
    // IDA_NAME: _ZN25Inter_UpgradeAccountCargoD2Ev
    // IDA_EA: 0x84F6D62
    // IDA_NAME: _ZN25Inter_UpgradeAccountCargoD0Ev
    // DEMANGLED: Inter_UpgradeAccountCargo::~Inter_UpgradeAccountCargo()
    virtual ~Inter_UpgradeAccountCargo();

    // IDA_EA: 0x84E243A
    // IDA_NAME: _ZN25Inter_UpgradeAccountCargo12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_UpgradeAccountCargo::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_LoadAccountCargo : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE864
    // IDA_NAME: _ZN22Inter_LoadAccountCargoC2Ev
    // DEMANGLED: Inter_LoadAccountCargo::Inter_LoadAccountCargo(void)
    Inter_LoadAccountCargo();

    // IDA_EA: 0x84F6CE4
    // IDA_NAME: _ZN22Inter_LoadAccountCargoD2Ev
    // IDA_EA: 0x84F6D14
    // IDA_NAME: _ZN22Inter_LoadAccountCargoD0Ev
    // DEMANGLED: Inter_LoadAccountCargo::~Inter_LoadAccountCargo()
    virtual ~Inter_LoadAccountCargo();

    // IDA_EA: 0x84E24EC
    // IDA_NAME: _ZN22Inter_LoadAccountCargo12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_LoadAccountCargo::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CreateAccountCargo) == 0x04u,
              "Inter_CreateAccountCargo must retain its base-only i386 ABI");
static_assert(sizeof(Inter_UpgradeAccountCargo) == 0x04u,
              "Inter_UpgradeAccountCargo must retain its base-only i386 ABI");
static_assert(sizeof(Inter_LoadAccountCargo) == 0x04u,
              "Inter_LoadAccountCargo must retain its base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_ACCOUNT_CARGO_ACCOUNT_CARGO_INTER_RECEIVERS_H_
