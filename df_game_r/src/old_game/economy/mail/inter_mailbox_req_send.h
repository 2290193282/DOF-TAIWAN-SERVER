#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_SEND_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_SEND_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 10_mail_trade_auction_cash/Inter_MailBox_Req_Send.
// DB_MailBox_Req_Send returns signal 128 after a normal mailbox insert.  The
// reply is not addressed to the sending user (the DB provider writes UID -1).
// Legacy success wakes the monitor-side recipient mailbox path.  A rebuilt
// tracked tail can additionally complete an owner-side persistence journal;
// this class remains base-only and its virtual ABI is unchanged.
class Inter_MailBox_Req_Send : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED63C
    Inter_MailBox_Req_Send();
    // IDA_EA: 0x84F9C6C / 0x84F9C9C
    virtual ~Inter_MailBox_Req_Send();

    // IDA_EA: 0x84D0130
    // IDA_NAME: _ZN22Inter_MailBox_Req_Send12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MailBox_Req_Send) == 0x04u,
              "Inter_MailBox_Req_Send must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_SEND_H_
