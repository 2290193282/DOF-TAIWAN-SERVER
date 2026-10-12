#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_MAILBOX_SEND_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_MAILBOX_SEND_H_

#include "old_game/economy/mail/mail_trade_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The concrete CMD97 base is emitted only by
// packet_dispatcher_mailbox_send.cpp.  Keeping the declaration external
// prevents unrelated users of this header from producing a second template
// vtable provider.
extern template class IPacketDispatcher<MSG_MAILBOX_SEND,
                                        stMailboxSendParam,
                                        static_cast<ch_state>(0)>;

// CMD97 / MAILBOX_SEND.  This remains unregistered until the complete
// PacketDispatcher root construction transaction is restored.
class Dispatcher_MailBox_Send
    : public IPacketDispatcher<MSG_MAILBOX_SEND,
                               stMailboxSendParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595ECC
    // IDA_NAME: _ZN23Dispatcher_MailBox_SendC2Ev
    // DEMANGLED: Dispatcher_MailBox_Send::Dispatcher_MailBox_Send(void)
    Dispatcher_MailBox_Send();

    // IDA_EA: 0x8245FFA
    // IDA_NAME: _ZN23Dispatcher_MailBox_SendD2Ev
    // IDA_EA: 0x824602A
    // IDA_NAME: _ZN23Dispatcher_MailBox_SendD0Ev
    // DEMANGLED: Dispatcher_MailBox_Send::~Dispatcher_MailBox_Send()
    virtual ~Dispatcher_MailBox_Send();

    // IDA_EA: 0x81CC3DA
    // IDA_NAME: _ZN23Dispatcher_MailBox_Send4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Send::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CC636
    // IDA_NAME: _ZN23Dispatcher_MailBox_Send11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Send::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81CC8C2
    // IDA_NAME: _ZN23Dispatcher_MailBox_Send14GuildMailCheckEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Send::GuildMailCheck(CUser *,MSG_BASE &)
    int GuildMailCheck(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81CC958
    // IDA_NAME: _ZN23Dispatcher_MailBox_Send7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Send::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CCDB2
    // IDA_NAME: _ZN23Dispatcher_MailBox_Send4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Send::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

static_assert(sizeof(void*) != 4 || sizeof(Dispatcher_MailBox_Send) == 0x04u,
              "CMD97 dispatcher must be a base-only i386 object");

// CMD316 / MULTI_MAILBOX_SEND has a direct parser rather than a typed
// template dispatch.  Its native object is the existing MSG_BASE/ParamBase
// state-0 specialization only.
class Dispatcher_MultiMailBoxSend
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85972D6
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendC2Ev
    // DEMANGLED: Dispatcher_MultiMailBoxSend::Dispatcher_MultiMailBoxSend(void)
    Dispatcher_MultiMailBoxSend();

    // IDA_EA: 0x8240492
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendD2Ev
    // IDA_EA: 0x82404C2
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendD0Ev
    // DEMANGLED: Dispatcher_MultiMailBoxSend::~Dispatcher_MultiMailBoxSend()
    virtual ~Dispatcher_MultiMailBoxSend();

    // IDA_EA: 0x82262A2
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_MultiMailBoxSend::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234226
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MultiMailBoxSend::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234230
    // IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MultiMailBoxSend::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

static_assert(sizeof(void*) != 4 || sizeof(Dispatcher_MultiMailBoxSend) == 0x04u,
              "CMD316 dispatcher must be a base-only i386 object");

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_MAILBOX_SEND_H_
