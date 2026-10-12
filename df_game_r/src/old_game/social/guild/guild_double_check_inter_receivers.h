#ifndef OLDSRV_GLOBAL_GUILD_DOUBLE_CHECK_INTER_RECEIVERS_H_
#define OLDSRV_GLOBAL_GUILD_DOUBLE_CHECK_INTER_RECEIVERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;


// IDA unit: 99_misc_global/Inter_DBCheckDoubleGuildName.
class Inter_DBCheckDoubleGuildName : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDAB8
    // IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameC2Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildName::Inter_DBCheckDoubleGuildName(void)
    Inter_DBCheckDoubleGuildName();

    // IDA_EA: 0x84F9174
    // IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameD2Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildName::~Inter_DBCheckDoubleGuildName()
    // IDA_EA: 0x84F91A4
    // IDA_NAME: _ZN28Inter_DBCheckDoubleGuildNameD0Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildName::~Inter_DBCheckDoubleGuildName()
    virtual ~Inter_DBCheckDoubleGuildName();

    // IDA_EA: 0x84D347E
    // IDA_NAME: _ZN28Inter_DBCheckDoubleGuildName12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_DBCheckDoubleGuildName::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 99_misc_global/Inter_DBCheckDoubleGuildAddress.
class Inter_DBCheckDoubleGuildAddress : public ISigDispatcher {
public:
    // IDA_EA: 0x84EDAD4
    // IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressC2Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildAddress::Inter_DBCheckDoubleGuildAddress(void)
    Inter_DBCheckDoubleGuildAddress();

    // IDA_EA: 0x84F9126
    // IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressD2Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildAddress::~Inter_DBCheckDoubleGuildAddress()
    // IDA_EA: 0x84F9156
    // IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddressD0Ev
    // DEMANGLED: Inter_DBCheckDoubleGuildAddress::~Inter_DBCheckDoubleGuildAddress()
    virtual ~Inter_DBCheckDoubleGuildAddress();

    // IDA_EA: 0x84D358E
    // IDA_NAME: _ZN31Inter_DBCheckDoubleGuildAddress12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_DBCheckDoubleGuildAddress::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_DBCheckDoubleGuildName) == 0x04u,
              "guild-name inter receiver must retain the i386 base-only ABI");
static_assert(sizeof(Inter_DBCheckDoubleGuildAddress) == 0x04u,
              "guild-address inter receiver must retain the i386 base-only ABI");
#endif

#endif  // OLDSRV_GLOBAL_GUILD_DOUBLE_CHECK_INTER_RECEIVERS_H_
