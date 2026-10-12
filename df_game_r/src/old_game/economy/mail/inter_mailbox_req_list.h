#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_LIST_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_LIST_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 10_mail_trade_auction_cash/Inter_MailBox_Req_List.
// Signal 127 is the DB mailbox-list reply.  The caller's load-generation
// check belongs here, before CMailBoxHelper mutates the live mailbox state.
class Inter_MailBox_Req_List : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED604
    Inter_MailBox_Req_List();
    // IDA_EA: 0x84F9D08 / 0x84F9D38
    virtual ~Inter_MailBox_Req_List();

    // IDA_EA: 0x84CF7FA
    // IDA_NAME: _ZN22Inter_MailBox_Req_List12dispatch_sigEP5CUserPci
    // DEMANGLED: Inter_MailBox_Req_List::dispatch_sig(CUser *,char *,int)
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MailBox_Req_List) == 0x04u,
              "Inter_MailBox_Req_List must retain the original base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_REQ_LIST_H_
