#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_OWNER_IS_VIP_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_OWNER_IS_VIP_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 99_misc_global/Inter_OwnerisVIP.  Though the original dump groups
// this DB receiver under misc_global, its complete business chain belongs to
// the Cera point-auction and Taiwan billing functional owner.
class Inter_OwnerisVIP : public ISigDispatcher {
public:
    // IDA_EA: 0x84EB4D4
    // IDA_NAME: _ZN16Inter_OwnerisVIPC2Ev
    Inter_OwnerisVIP();

    // IDA_EA: 0x84EB4F0 / 0x84EB520
    // IDA_NAME: _ZN16Inter_OwnerisVIPD2Ev / _ZN16Inter_OwnerisVIPD0Ev
    virtual ~Inter_OwnerisVIP();

    // IDA_EA: 0x84E7C90
    // IDA_NAME: _ZN16Inter_OwnerisVIP12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Inter_OwnerisVIP) == 0x04u,
              "Inter_OwnerisVIP must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_OWNER_IS_VIP_H_
