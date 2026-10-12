#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_CHANGE_LETTER_STAT_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_CHANGE_LETTER_STAT_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD137 input.  The old reader writes the letter id at MSG_BASE + 0x0d and
// the requested state at +0x11; the packed extent is part of the C2S ABI.
#pragma pack(push, 1)
struct MSG_CHANGE_LETTER_STAT : public MSG_BASE {
    int letter_id;
    short stat;
};

// The old template creates this 8-byte i386 result object after a successful
// read.  Only result is initialized by its C2.
struct stChangeLetterStatParam : public ParamBase {
    // IDA_EA: 0x82311F2
    // IDA_NAME: _ZN23stChangeLetterStatParamC2Ev
    // DEMANGLED: stChangeLetterStatParam::stChangeLetterStatParam(void)
    stChangeLetterStatParam();

    // IDA_EA: 0x824778C
    // IDA_NAME: _ZN23stChangeLetterStatParamD2Ev
    // DEMANGLED: stChangeLetterStatParam::~stChangeLetterStatParam()
    // IDA_EA: 0x82477BC
    // IDA_NAME: _ZN23stChangeLetterStatParamD0Ev
    // DEMANGLED: stChangeLetterStatParam::~stChangeLetterStatParam()
    virtual ~stChangeLetterStatParam();

    int result;
};
#pragma pack(pop)

static_assert(sizeof(MSG_CHANGE_LETTER_STAT) == 0x13u,
              "CMD137 input must retain MSG_BASE+0x0d/+0x11 offsets");
#if defined(__i386__)
static_assert(sizeof(stChangeLetterStatParam) == 0x08u,
              "CMD137 result parameter must retain its old i386 extent");
#endif

// CMD137.  This is a C/mailbox owner: it validates character state, sends
// the old mailbox request, and only emits the original low-byte error reply.
class Dispatcher_ChangeLetterStat
    : public IPacketDispatcher<MSG_CHANGE_LETTER_STAT,
                               stChangeLetterStatParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596188
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStatC2Ev
    // DEMANGLED: Dispatcher_ChangeLetterStat::Dispatcher_ChangeLetterStat(void)
    Dispatcher_ChangeLetterStat();

    // IDA_EA: 0x8245D8A
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStatD2Ev
    // DEMANGLED: Dispatcher_ChangeLetterStat::~Dispatcher_ChangeLetterStat()
    // IDA_EA: 0x8245DBA
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStatD0Ev
    // DEMANGLED: Dispatcher_ChangeLetterStat::~Dispatcher_ChangeLetterStat()
    virtual ~Dispatcher_ChangeLetterStat();

    // IDA_EA: 0x81CE152
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStat4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeLetterStat::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CE1FC
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStat7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ChangeLetterStat::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CE304
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStat4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_ChangeLetterStat::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81CE2BA
    // IDA_NAME: _ZN27Dispatcher_ChangeLetterStat11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeLetterStat::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeLetterStat) == 0x04u,
              "CMD137 dispatcher must retain its old vptr-only object ABI");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PACKET_DISPATCHER_CHANGE_LETTER_STAT_H_
