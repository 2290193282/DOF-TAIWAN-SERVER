#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MONITOR_NOTIFY_NEW_MAIL_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MONITOR_NOTIFY_NEW_MAIL_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 10_mail_trade_auction_cash/Inter_Monitor_Notify_New_Mail.
class Inter_Monitor_Notify_New_Mail : public ISigDispatcher {
public:
    Inter_Monitor_Notify_New_Mail();
    virtual ~Inter_Monitor_Notify_New_Mail();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Monitor_Notify_New_Mail) == 0x04u,
              "Inter_Monitor_Notify_New_Mail must retain its base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MONITOR_NOTIFY_NEW_MAIL_H_
