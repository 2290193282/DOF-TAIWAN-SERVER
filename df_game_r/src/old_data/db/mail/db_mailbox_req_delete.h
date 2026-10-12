#ifndef OLDSRV_DB_DB_MAILBOX_REQ_DELETE_H_
#define OLDSRV_DB_DB_MAILBOX_REQ_DELETE_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/mail/mailbox_payload.h"


// IDA unit: 10_mail_trade_auction_cash/DB_MailBox_Req_Delete.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at old storage slot +130,
// active DB signal 129.
class DB_MailBox_Req_Delete : public DBDispatcher {
public:
    // IDA_EA: 0x844E312.
    DB_MailBox_Req_Delete();
    // IDA_EA: 0x845F4D2 / 0x845F502.
    virtual ~DB_MailBox_Req_Delete();

    // IDA_EA: 0x841F084.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_MAILBOX_REQ_DELETE_H_
