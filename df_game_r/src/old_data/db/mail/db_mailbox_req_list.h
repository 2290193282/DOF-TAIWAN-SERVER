#ifndef OLDSRV_DB_DB_MAILBOX_REQ_LIST_H_
#define OLDSRV_DB_DB_MAILBOX_REQ_LIST_H_

#include "old_data/db/core/db_dispatcher.h"

struct SIG_MAILBOX_LIST;


class MySQL;

// IDA unit: 10_mail_trade_auction_cash/DB_MailBox_Req_List.
// Registered by DBTask::RegistDispatcher @0x83FBFEE at old storage slot +128,
// active DB signal 127.
class DB_MailBox_Req_List : public DBDispatcher {
public:
    // IDA_EA: 0x844E2F6.
    DB_MailBox_Req_List();
    // IDA_EA: 0x845F520 / 0x845F550.
    virtual ~DB_MailBox_Req_List();

    // IDA_EA: 0x841DE4A.
    virtual bool dispatch(int signal, int user_idx, Stream* stream);

    // IDA_EA: 0x841D52C.
    bool SelectLetter(MySQL* db_handle,
                      int user_idx,
                      int charac_no,
                      unsigned int last_letter_id,
                      SIG_MAILBOX_LIST* list);
    // IDA_EA: 0x841DC84.
    bool setCreatureData(MySQL* db_handle,
                         unsigned int charac_no,
                         unsigned int* ui_id_list,
                         SIG_MAILBOX_LIST* list);
};

#endif  // OLDSRV_DB_DB_MAILBOX_REQ_LIST_H_
