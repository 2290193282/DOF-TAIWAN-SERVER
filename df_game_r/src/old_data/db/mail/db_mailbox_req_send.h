#ifndef OLDSRV_DB_DB_MAILBOX_REQ_SEND_H_
#define OLDSRV_DB_DB_MAILBOX_REQ_SEND_H_

#include "old_data/db/core/db_dispatcher.h"

enum ENUM_LETTER_STAT : int;
enum ENUM_SERVER_GROUP : int;
#include "old_data/db/mail/mailbox_payload.h"


class MySQL;

// IDA unit: 10_mail_trade_auction_cash/DB_MailBox_Req_Send.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at old storage slot +129,
// active DB signal 128.
class DB_MailBox_Req_Send : public DBDispatcher {
public:
    // IDA_EA: 0x844E32E.
    DB_MailBox_Req_Send();
    // IDA_EA: 0x845F484 / 0x845F4B4.
    virtual ~DB_MailBox_Req_Send();

    // IDA_EA: 0x841F84E.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x841F15A.
    static bool InsertLetter(MySQL* db_handle,
                             int receive_charac_no,
                             int send_charac_no,
                             const char* send_charac_name,
                             const char* letter_text,
                             int& letter_id,
                             long reg_time,
                             ENUM_LETTER_STAT stat);
    // IDA_EA: 0x841F2FA.
    static bool insertMailData(MySQL* db_handle,
                               const SIG_MAILBOX_SEND_NEW_MAIL* payload);
    // IDA_EA: 0x841F75A.
    static bool sendResult(int user_idx,
                           int result,
                           int receive_charac_no,
                           ENUM_SERVER_GROUP server_group);
};

#endif  // OLDSRV_DB_DB_MAILBOX_REQ_SEND_H_
