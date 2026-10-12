#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_CHECK_RECEIVER_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_CHECK_RECEIVER_H_

#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
struct SIG_MAILBOX_CHECK_RECEIVER;
enum ENUM_CMDPACKET : uint16_t;

// IDA unit: 10_mail_trade_auction_cash/Inter_MailBox_Req_CheckReceiver.
// DB_MailBox_Req_CheckReceiver returns signal 126 to this one-vptr receiver.
// Its multi-mail branch validates the checked recipient then submits DB slot
// 501 (0x1f5), whose reply is Inter_MultiMailBoxReqSend.
class Inter_MailBox_Req_CheckReceiver : public ISigDispatcher {
public:
    // IDA_EA: 0x84ED620
    Inter_MailBox_Req_CheckReceiver();
    // IDA_EA: 0x84F9CBA / 0x84F9CEA
    virtual ~Inter_MailBox_Req_CheckReceiver();

    // IDA_EA: 0x84CF95A
    // IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

private:
    // IDA_EA: 0x84CFF18
    // IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver21SendDBThread_forchinaEP5CUserP26SIG_MAILBOX_CHECK_RECEIVER
    int SendDBThread_forchina(CUser* user,
                              SIG_MAILBOX_CHECK_RECEIVER* request);

    // IDA_EA: 0x84D008A
    // IDA_NAME: _ZN31Inter_MailBox_Req_CheckReceiver15SendErrorPacketEP5CUser14ENUM_CMDPACKETt
    int SendErrorPacket(CUser* user,
                        ENUM_CMDPACKET command,
                        uint16_t error);
};

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_INTER_MAILBOX_CHECK_RECEIVER_H_
