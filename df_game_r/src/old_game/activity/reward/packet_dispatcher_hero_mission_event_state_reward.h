#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_HERO_MISSION_EVENT_STATE_REWARD_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_HERO_MISSION_EVENT_STATE_REWARD_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/Dispatcher_HeroMissionEventState_Reward.
// The object is restored but deliberately not inserted into the frozen root.
class Dispatcher_HeroMissionEventState_Reward
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597F86
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_RewardC2Ev
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::Dispatcher_HeroMissionEventState_Reward(void)
    Dispatcher_HeroMissionEventState_Reward();

    // IDA_EA: 0x82608BA / 0x82608EA
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_RewardD2Ev / D0Ev
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::~Dispatcher_HeroMissionEventState_Reward()
    virtual ~Dispatcher_HeroMissionEventState_Reward();

    // IDA_EA: 0x8260094
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x826060E
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8260618
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8260622
    // IDA_NAME: _ZN39Dispatcher_HeroMissionEventState_Reward4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_HeroMissionEventState_Reward::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_HeroMissionEventState_Reward) == 0x04u,
              "CMD602 dispatcher must retain its vptr-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_HERO_MISSION_EVENT_STATE_REWARD_H_
