#ifndef OLDSRV_DB_DB_MAILBOX_REQ_CHECK_RECEIVER_H_
#define OLDSRV_DB_DB_MAILBOX_REQ_CHECK_RECEIVER_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/mail/mailbox_payload.h"


// IDA unit: 10_mail_trade_auction_cash/DB_MailBox_Req_CheckReceiver.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at old storage slot +127,
// active DB signal 126.
class DB_MailBox_Req_CheckReceiver : public DBDispatcher {
public:
    DB_MailBox_Req_CheckReceiver();
    virtual ~DB_MailBox_Req_CheckReceiver();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_MAILBOX_REQ_CHECK_RECEIVER_H_
