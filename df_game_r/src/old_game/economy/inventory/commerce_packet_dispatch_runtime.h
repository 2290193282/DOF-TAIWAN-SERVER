#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_COMMERCE_PACKET_DISPATCH_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_COMMERCE_PACKET_DISPATCH_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mail/mailbox_param_runtime.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_MailBox_Extract.
// The old CMD98 reader reads one unaligned uint32 directly after the
// 13-byte MSG_BASE extent, then passes it to CMailBox::GetMail.  This is a
// real protocol field, not an opaque packet backing store.
#pragma pack(push, 1)
struct MSG_MAILBOX_EXTRACT_ITEM : public MSG_BASE {
    uint32_t mail_index;
};
#pragma pack(pop)

static_assert(sizeof(MSG_MAILBOX_EXTRACT_ITEM) == 0x11u,
              "CMD98 mail-index message must preserve its 17-byte old ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_MAILBOX_EXTRACT_ITEM, mail_index) == 0x0du,
              "CMD98 mail index must begin immediately after MSG_BASE");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// The mailbox parameter value types are intentionally provided by
// mailbox_param_runtime.{h,cpp}; do not create a second ParamBase hierarchy
// here.  This translation unit is the sole owner only of the complete CMD99
// state-0 IPacketDispatcher specialization below.
extern template class IPacketDispatcher<MSG_BASE,
                                        stMailboxOpenParam,
                                        static_cast<ch_state>(0)>;
extern template class IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,
                                        stMailboxExtractParam,
                                        static_cast<ch_state>(0)>;

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_MailBox_Open.
// This class has no fields after its state-0 packet-dispatch base.
class Dispatcher_MailBox_Open
    : public IPacketDispatcher<MSG_BASE,
                               stMailboxOpenParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595EB0
    // IDA_NAME: _ZN23Dispatcher_MailBox_OpenC2Ev
    // DEMANGLED: Dispatcher_MailBox_Open::Dispatcher_MailBox_Open(void)
    Dispatcher_MailBox_Open();

    // IDA_EA: 0x8246048
    // IDA_NAME: _ZN23Dispatcher_MailBox_OpenD2Ev
    // DEMANGLED: Dispatcher_MailBox_Open::~Dispatcher_MailBox_Open()
    // IDA_EA: 0x8246078
    // IDA_NAME: _ZN23Dispatcher_MailBox_OpenD0Ev
    // DEMANGLED: Dispatcher_MailBox_Open::~Dispatcher_MailBox_Open()
    virtual ~Dispatcher_MailBox_Open();

    // IDA_EA: 0x81CC18C
    // IDA_NAME: _ZN23Dispatcher_MailBox_Open4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Open::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CC196
    // IDA_NAME: _ZN23Dispatcher_MailBox_Open7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Open::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CC2E6
    // IDA_NAME: _ZN23Dispatcher_MailBox_Open4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Open::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CC2A2
    // IDA_NAME: _ZN23Dispatcher_MailBox_Open11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Open::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_MailBox_Extract.
// CMD98 reads the mail-index directly after the packed C2S frame header and
// delegates the package transfer to the canonical mailbox helper.
class Dispatcher_MailBox_Extract
    : public IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,
                               stMailboxExtractParam,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595EE8
    // IDA_NAME: _ZN26Dispatcher_MailBox_ExtractC2Ev
    // DEMANGLED: Dispatcher_MailBox_Extract::Dispatcher_MailBox_Extract(void)
    Dispatcher_MailBox_Extract();

    // IDA_EA: 0x8245FAC
    // IDA_NAME: _ZN26Dispatcher_MailBox_ExtractD2Ev
    // IDA_EA: 0x8245FDC
    // IDA_NAME: _ZN26Dispatcher_MailBox_ExtractD0Ev
    // DEMANGLED: Dispatcher_MailBox_Extract::~Dispatcher_MailBox_Extract()
    virtual ~Dispatcher_MailBox_Extract();

    // IDA_EA: 0x81CCDFA
    // IDA_NAME: _ZN26Dispatcher_MailBox_Extract4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Extract::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CCE50
    // IDA_NAME: _ZN26Dispatcher_MailBox_Extract7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Extract::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CCF94
    // IDA_NAME: _ZN26Dispatcher_MailBox_Extract4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_MailBox_Extract::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CCEEA
    // IDA_NAME: _ZN26Dispatcher_MailBox_Extract11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_MailBox_Extract::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

// IDA unit: 10_mail_trade_auction_cash/Dispatcher_GenCeraTicket.
// CMD68 has no C2S payload; after the common state-0 dispatch it emits the
// original DB-queue request for a newly generated Cera ticket.
class Dispatcher_GenCeraTicket
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595B68
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicketC2Ev
    // DEMANGLED: Dispatcher_GenCeraTicket::Dispatcher_GenCeraTicket(void)
    Dispatcher_GenCeraTicket();

    // IDA_EA: 0x82435EE
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicketD2Ev
    // IDA_EA: 0x824361E
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicketD0Ev
    // DEMANGLED: Dispatcher_GenCeraTicket::~Dispatcher_GenCeraTicket()
    virtual ~Dispatcher_GenCeraTicket();

    // IDA_EA: 0x81FEB12
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicket12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_GenCeraTicket::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82334E0
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicket4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GenCeraTicket::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82334EA
    // IDA_NAME: _ZN24Dispatcher_GenCeraTicket7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GenCeraTicket::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MailBox_Open) == 0x04u,
              "CMD99 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_MailBox_Extract) == 0x04u,
              "CMD98 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_GenCeraTicket) == 0x04u,
              "CMD68 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_COMMERCE_PACKET_DISPATCH_RUNTIME_H_
