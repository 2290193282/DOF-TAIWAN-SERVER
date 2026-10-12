#ifndef OLDSRV_OLD_DATA_DB_MAIL_DB_MULTI_MAIL_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_MAIL_DB_MULTI_MAIL_DISPATCHERS_H_

#include "old_data/db/core/db_dispatcher.h"

class MySQL;
enum ENUM_SERVER_GROUP : int;
struct SIG_MAILBOX_CHECK_RECEIVER;
struct SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL;

// IDA units: 10_mail_trade_auction_cash/DB_Multi_MailBox_Req_Send and
// DB_MailBox_Req_System_Multi_Mail.  DBTask slots 501 and 676.
class DB_Multi_MailBox_Req_Send : public DBDispatcher {
public:
    DB_Multi_MailBox_Req_Send();
    virtual ~DB_Multi_MailBox_Req_Send();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x843BC16
    // IDA_NAME: _ZN25DB_Multi_MailBox_Req_Send11_sendResultEiP26SIG_MAILBOX_CHECK_RECEIVER
    // DEMANGLED: DB_Multi_MailBox_Req_Send::_sendResult(int,SIG_MAILBOX_CHECK_RECEIVER *)
    bool _sendResult(int user_idx, SIG_MAILBOX_CHECK_RECEIVER* payload);
};

class DB_MailBox_Req_System_Multi_Mail : public DBDispatcher {
public:
    DB_MailBox_Req_System_Multi_Mail();
    virtual ~DB_MailBox_Req_System_Multi_Mail();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x84435D4
    // IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail13_InsertLetterEP5MySQLP34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL
    // DEMANGLED: DB_MailBox_Req_System_Multi_Mail::_InsertLetter(MySQL *,SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL *)
    bool _InsertLetter(MySQL* db_handle,
                       SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* payload);

    // IDA_EA: 0x844372E
    // IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail13_InsertPostalEP5MySQLP34SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL
    // DEMANGLED: DB_MailBox_Req_System_Multi_Mail::_InsertPostal(MySQL *,SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL *)
    bool _InsertPostal(MySQL* db_handle,
                       SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* payload);

    // IDA_EA: 0x8443B6E
    // IDA_NAME: _ZN32DB_MailBox_Req_System_Multi_Mail10sendResultEi17ENUM_SERVER_GROUP
    // DEMANGLED: DB_MailBox_Req_System_Multi_Mail::sendResult(int,ENUM_SERVER_GROUP)
    bool sendResult(int charac_no, ENUM_SERVER_GROUP server_group);
};

#endif  // OLDSRV_OLD_DATA_DB_MAIL_DB_MULTI_MAIL_DISPATCHERS_H_
