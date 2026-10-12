#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_ARAD_REQUEST_PCROOM_DAILY_REWARD_DISPATCHER_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_ARAD_REQUEST_PCROOM_DAILY_REWARD_DISPATCHER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

namespace ARAD {
namespace DISPATCHER {

// CMD601 / ENUM_CMDPACKET_REQUEST_PCROOM_DAYILY_REWARD.  This old object is
// a state-0 dispatcher and intentionally remains unregistered until the
// verified 606-slot PacketDispatcher construction transaction is restored.
class Dispatcher_RequestPrcoomDayilyReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597F6A
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardC2Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::Dispatcher_RequestPrcoomDayilyReward(void)
    Dispatcher_RequestPrcoomDayilyReward();

    // IDA_EA: 0x81A0BCE
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardD2Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::~Dispatcher_RequestPrcoomDayilyReward()
    // IDA_EA: 0x81A0BFE
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardD0Ev
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::~Dispatcher_RequestPrcoomDayilyReward()
    virtual ~Dispatcher_RequestPrcoomDayilyReward();

    // IDA_EA: 0x81A0612
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x81A0B86
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward4readER9PacketBufR8MSG_BASE
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81A0B90
    // IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestPrcoomDayilyReward) == 0x04u,
              "CMD601 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_ARAD_REQUEST_PCROOM_DAILY_REWARD_DISPATCHER_H_
