#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MULTIMAILBOX_SEND_RETURN_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MULTIMAILBOX_SEND_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 10_mail_trade_auction_cash/Inter_MultiMailBoxReqSend.
// DB_Multi_MailBox_Req_Send returns outer signal 501 (0x1f5) to this
// one-vptr database-reply receiver.
class Inter_MultiMailBoxReqSend : public ISigDispatcher {
public:
    // IDA_EA: 0x84EE880
    Inter_MultiMailBoxReqSend();
    // IDA_EA: 0x84F6B10 / 0x84F6B40
    virtual ~Inter_MultiMailBoxReqSend();

    // IDA_EA: 0x84E27B8
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MULTIMAILBOX_SEND_RETURN_H_
