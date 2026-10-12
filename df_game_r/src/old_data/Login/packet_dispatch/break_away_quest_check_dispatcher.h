#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BREAK_AWAY_QUEST_CHECK_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BREAK_AWAY_QUEST_CHECK_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

#pragma pack(push, 1)
struct MSG_BREAK_AWAY : public MSG_BASE {
    // IDA C2 @0x822DC4C explicitly clears only this byte after the 13-byte
    // base header.
    uint8_t accept_reward;  // +0x0d

    // IDA_EA: 0x822DC4C
    // IDA_NAME: _ZN14MSG_BREAK_AWAYC2Ev
    MSG_BREAK_AWAY();
};
#pragma pack(pop)

struct stBreakAwayParam : public ParamBase {
    int result;  // +0x04

    // IDA_EA: 0x8231572
    // IDA_NAME: _ZN16stBreakAwayParamC2Ev
    stBreakAwayParam();

    // IDA_EA: 0x824708A / 0x82470BA
    // IDA_NAME: _ZN16stBreakAwayParamD2Ev / _ZN16stBreakAwayParamD0Ev
    virtual ~stBreakAwayParam();
};

static_assert(sizeof(MSG_BREAK_AWAY) == 0x0eu,
              "CMD365 request must retain the old 14-byte ABI");
#if defined(__i386__)
static_assert(sizeof(stBreakAwayParam) == 0x08u,
              "CMD365 parameter must retain the old ParamBase+result ABI");
#endif

// CMD365.  The provider is intentionally not registered into PacketDispatcher
// until the complete 606-slot owner is reconstructed.
class Dispatcher_Break_Away_Quest_Check
    : public IPacketDispatcher<MSG_BREAK_AWAY,
                               stBreakAwayParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597228
    // IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_CheckC2Ev
    Dispatcher_Break_Away_Quest_Check();

    // IDA_EA: 0x824537C / 0x82453AC
    // IDA_NAME: _ZN33Dispatcher_Break_Away_Quest_CheckD2Ev /
    //           _ZN33Dispatcher_Break_Away_Quest_CheckD0Ev
    virtual ~Dispatcher_Break_Away_Quest_Check();

    // IDA_EA: 0x81D38BA / 0x81D390A / 0x81D3A72
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Break_Away_Quest_Check) == 0x04u,
              "CMD365 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BREAK_AWAY_QUEST_CHECK_DISPATCHER_H_
