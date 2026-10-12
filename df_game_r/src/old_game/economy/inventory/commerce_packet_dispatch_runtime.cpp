#include "old_game/economy/inventory/commerce_packet_dispatch_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// This inclusion is translation-unit local.  The explicit instantiation
// below owns the one old state-0 template vtable; no consumer can emit a
// competing COMDAT provider.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8256A4C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8256B9C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8256BA6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D990
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x825D1DA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D20A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x85985C2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE18stMailboxOpenParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stMailboxOpenParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 stMailboxOpenParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8256668
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82567B8
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82567C2
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D984
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x825D13E
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EED2Ev
// IDA_EA: 0x825D16E
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x85985FA
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_MAILBOX_EXTRACT_ITEM21stMailboxExtractParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,stMailboxExtractParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,
                                 stMailboxExtractParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595EB0
// IDA_NAME: _ZN23Dispatcher_MailBox_OpenC2Ev
// DEMANGLED: Dispatcher_MailBox_Open::Dispatcher_MailBox_Open(void)
Dispatcher_MailBox_Open::Dispatcher_MailBox_Open()
    : IPacketDispatcher<MSG_BASE,
                        stMailboxOpenParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8246048
// IDA_NAME: _ZN23Dispatcher_MailBox_OpenD2Ev
// DEMANGLED: Dispatcher_MailBox_Open::~Dispatcher_MailBox_Open()
// IDA_EA: 0x8246078
// IDA_NAME: _ZN23Dispatcher_MailBox_OpenD0Ev
// DEMANGLED: Dispatcher_MailBox_Open::~Dispatcher_MailBox_Open()
Dispatcher_MailBox_Open::~Dispatcher_MailBox_Open() {}

// IDA_EA: 0x81CC18C
// IDA_NAME: _ZN23Dispatcher_MailBox_Open4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Open::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MailBox_Open::read(PacketBuf& packet, MSG_BASE& message) {
    // CMD99 has no payload.  The old body returns before touching either
    // argument, so preserve that wire contract exactly.
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81CC2A2
// IDA_NAME: _ZN23Dispatcher_MailBox_Open11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Open::check_error(CUser *,MSG_BASE &)
int Dispatcher_MailBox_Open::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81CC196
// IDA_NAME: _ZN23Dispatcher_MailBox_Open7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MailBox_Open::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MailBox_Open::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param_base) {
    stMailboxOpenParam& param = static_cast<stMailboxOpenParam&>(param_base);
    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        WongWork::CMailBox* const mailbox = user->GetMailBox();
        if (mailbox->GetNotLoadedMailCount() != 0 &&
            mailbox->GetRecvSize() + mailbox->GetLoadedLetterCount() <= 19) {
            WongWork::CMailBoxHelper::ReqDBLoadMail(
                user, static_cast<unsigned int>(user->getCurCharacNo()));
            param.result = 22;
            return 0;
        }

        param.not_loaded_mail_count = mailbox->GetNotLoadedMailCount();
        return 0;
    }

    return LineFunc(
        8904,
        "virtual int Dispatcher_MailBox_Open::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81CC2E6
// IDA_NAME: _ZN23Dispatcher_MailBox_Open4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_MailBox_Open::send(CUser *,ParamBase &)
void Dispatcher_MailBox_Open::send(CUser* user, ParamBase& param_base) {
    stMailboxOpenParam& param = static_cast<stMailboxOpenParam&>(param_base);
    if (param.result == 0x7fffffff) {
        return;
    }

    if (param.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_MAILBOX_OPEN,
            static_cast<unsigned char>(param.result));
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_MAILBOX_OPEN);
    packet.put_byte(1);
    packet.put_short(param.not_loaded_mail_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8595EE8
// IDA_NAME: _ZN26Dispatcher_MailBox_ExtractC2Ev
// DEMANGLED: Dispatcher_MailBox_Extract::Dispatcher_MailBox_Extract(void)
Dispatcher_MailBox_Extract::Dispatcher_MailBox_Extract()
    : IPacketDispatcher<MSG_MAILBOX_EXTRACT_ITEM,
                        stMailboxExtractParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8245FAC
// IDA_NAME: _ZN26Dispatcher_MailBox_ExtractD2Ev
// DEMANGLED: Dispatcher_MailBox_Extract::~Dispatcher_MailBox_Extract()
// IDA_EA: 0x8245FDC
// IDA_NAME: _ZN26Dispatcher_MailBox_ExtractD0Ev
// DEMANGLED: Dispatcher_MailBox_Extract::~Dispatcher_MailBox_Extract()
Dispatcher_MailBox_Extract::~Dispatcher_MailBox_Extract() {}

// IDA_EA: 0x81CCDFA
// IDA_NAME: _ZN26Dispatcher_MailBox_Extract4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Extract::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MailBox_Extract::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MAILBOX_EXTRACT_ITEM& request =
        static_cast<MSG_MAILBOX_EXTRACT_ITEM&>(message);
    if (packet.get_int(request.mail_index) == 1) {
        return 0;
    }
    return LineFunc(
        9292,
        "virtual int Dispatcher_MailBox_Extract::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CCEEA
// IDA_NAME: _ZN26Dispatcher_MailBox_Extract11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Extract::check_error(CUser *,MSG_BASE &)
int Dispatcher_MailBox_Extract::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    // Preserve the original evaluation order: its state/character check
    // precedes the subsequent null/trade guard.
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }
    if (!user || user->CheckInTrade()) {
        return -1;
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        return 60;
    }
    if (user->CheckInTrade()) {
        return 218;
    }
    return 0;
}

// IDA_EA: 0x81CCE50
// IDA_NAME: _ZN26Dispatcher_MailBox_Extract7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MailBox_Extract::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MailBox_Extract::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param_base) {
    MSG_MAILBOX_EXTRACT_ITEM& request =
        static_cast<MSG_MAILBOX_EXTRACT_ITEM&>(message);
    stMailboxExtractParam& param =
        static_cast<stMailboxExtractParam&>(param_base);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        WongWork::CMailBoxHelper::GetPackege(user, request.mail_index);
        return 0;
    }
    return LineFunc(
        9309,
        "virtual int Dispatcher_MailBox_Extract::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81CCF94
// IDA_NAME: _ZN26Dispatcher_MailBox_Extract4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_MailBox_Extract::send(CUser *,ParamBase &)
void Dispatcher_MailBox_Extract::send(CUser* user, ParamBase& param_base) {
    stMailboxExtractParam& param =
        static_cast<stMailboxExtractParam&>(param_base);
    if (param.result != 0x7fffffff && param.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_MAILBOX_EXTRACT_ITEM,
            static_cast<unsigned char>(param.result));
    }
}

// IDA_EA: 0x8595B68
// IDA_NAME: _ZN24Dispatcher_GenCeraTicketC2Ev
// DEMANGLED: Dispatcher_GenCeraTicket::Dispatcher_GenCeraTicket(void)
Dispatcher_GenCeraTicket::Dispatcher_GenCeraTicket()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82435EE
// IDA_NAME: _ZN24Dispatcher_GenCeraTicketD2Ev
// DEMANGLED: Dispatcher_GenCeraTicket::~Dispatcher_GenCeraTicket()
// IDA_EA: 0x824361E
// IDA_NAME: _ZN24Dispatcher_GenCeraTicketD0Ev
// DEMANGLED: Dispatcher_GenCeraTicket::~Dispatcher_GenCeraTicket()
Dispatcher_GenCeraTicket::~Dispatcher_GenCeraTicket() {}

// IDA_EA: 0x81FEB12
// IDA_NAME: _ZN24Dispatcher_GenCeraTicket12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GenCeraTicket::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GenCeraTicket::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            37360,
            "virtual int Dispatcher_GenCeraTicket::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const int random_key = static_cast<int>(get_rand_int(0xffff));
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "PacketDispatcher_Impl_1.cpp", 37365);
    CStreamGuard guard(stream, true);
    guard << 73;
    guard << user->GetUID();
    guard << static_cast<unsigned int>(user->get_acc_id());
    guard << random_key;

    char unique_no[32];
    guard << WongWork::CGenUniqueNo::GenUniqueNo(unique_no);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x82334E0
// IDA_NAME: _ZN24Dispatcher_GenCeraTicket4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GenCeraTicket::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GenCeraTicket::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82334EA
// IDA_NAME: _ZN24Dispatcher_GenCeraTicket7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GenCeraTicket::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GenCeraTicket::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
