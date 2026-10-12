#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_RETURN_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_RETURN_H_

#include <stdint.h>

#include "old_game/activity/schedule/event_value_types.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD441 input.  The charac number starts at +0x10, after the one-byte reason
// and two original alignment bytes following the 13-byte C2S MSG_BASE.
#pragma pack(push, 1)
struct MsgMercenaryReturn : public MSG_BASE {
    char reason;
    uint8_t reserved_0e_0f[2];
    unsigned int charac_no;
};

// The old generic dispatcher allocates exactly 24 bytes for this result on
// i386.  Its C2 initializes ParamBase and the nested reward result only.
struct SendMercenaryReturnResult : public ParamBase {
    // IDA_EA: 0x824D624
    // IDA_NAME: _ZN25SendMercenaryReturnResultC2Ev
    // DEMANGLED: SendMercenaryReturnResult::SendMercenaryReturnResult(void)
    SendMercenaryReturnResult();

    // IDA_EA: 0x824D64E
    // IDA_NAME: _ZN25SendMercenaryReturnResultD2Ev
    // DEMANGLED: SendMercenaryReturnResult::~SendMercenaryReturnResult()
    // IDA_EA: 0x824D67E
    // IDA_NAME: _ZN25SendMercenaryReturnResultD0Ev
    // DEMANGLED: SendMercenaryReturnResult::~SendMercenaryReturnResult()
    virtual ~SendMercenaryReturnResult();

    uint8_t return_result;
    char reason;
    uint8_t reserved_06_07[2];
    unsigned int charac_no;
    competition_reward_result reward_result;
};
#pragma pack(pop)

static_assert(sizeof(MsgMercenaryReturn) == 0x14u,
              "CMD441 input must retain the old reason/charac-no offsets");
#if defined(__i386__)
static_assert(sizeof(SendMercenaryReturnResult) == 0x18u,
              "CMD441 result parameter must retain its old i386 extent");
#endif

class Dispatcher_Mercenary_Return
    : public IPacketDispatcher<MsgMercenaryReturn,
                               SendMercenaryReturnResult,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859799E
    // IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnC2Ev
    // DEMANGLED: Dispatcher_Mercenary_Return::Dispatcher_Mercenary_Return(void)
    Dispatcher_Mercenary_Return();

    // IDA_EA: 0x824474C
    // IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnD2Ev
    // DEMANGLED: Dispatcher_Mercenary_Return::~Dispatcher_Mercenary_Return()
    // IDA_EA: 0x824477C
    // IDA_NAME: _ZN27Dispatcher_Mercenary_ReturnD0Ev
    // DEMANGLED: Dispatcher_Mercenary_Return::~Dispatcher_Mercenary_Return()
    virtual ~Dispatcher_Mercenary_Return();

    // IDA_EA: 0x81DD904
    // IDA_NAME: _ZN27Dispatcher_Mercenary_Return4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Mercenary_Return::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DD99C
    // IDA_NAME: _ZN27Dispatcher_Mercenary_Return7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Return::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DDA1C
    // IDA_NAME: _ZN27Dispatcher_Mercenary_Return4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Return::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Mercenary_Return) == 0x04u,
              "CMD441 dispatcher must retain its old vptr-only object ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_MERCENARY_RETURN_H_
