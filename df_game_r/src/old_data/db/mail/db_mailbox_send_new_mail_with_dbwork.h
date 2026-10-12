#ifndef OLDSRV_OLD_DATA_DB_MAIL_DB_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK_H_
#define OLDSRV_OLD_DATA_DB_MAIL_DB_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK_H_

#include "old_data/db/core/db_dispatcher.h"

class DB_MailBoxSendNewMailWithDBWork : public DBDispatcher {
public:
    // IDA_EA: 0x844E676
    // IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkC2Ev
    // DEMANGLED: DB_MailBoxSendNewMailWithDBWork::DB_MailBoxSendNewMailWithDBWork(void)
    DB_MailBoxSendNewMailWithDBWork();

    // IDA_EA: 0x845EA76
    // IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkD2Ev
    // DEMANGLED: DB_MailBoxSendNewMailWithDBWork::~DB_MailBoxSendNewMailWithDBWork()
    // IDA_EA: 0x845EAA6
    // IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWorkD0Ev
    // DEMANGLED: DB_MailBoxSendNewMailWithDBWork::~DB_MailBoxSendNewMailWithDBWork()
    virtual ~DB_MailBoxSendNewMailWithDBWork();

    // IDA_EA: 0x84272DE
    // IDA_NAME: _ZN31DB_MailBoxSendNewMailWithDBWork8dispatchEiiP6Stream
    // DEMANGLED: DB_MailBoxSendNewMailWithDBWork::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_MAIL_DB_MAILBOX_SEND_NEW_MAIL_WITH_DBWORK_H_
