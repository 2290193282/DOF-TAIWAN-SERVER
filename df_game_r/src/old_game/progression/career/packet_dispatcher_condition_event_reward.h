#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CONDITION_EVENT_REWARD_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CONDITION_EVENT_REWARD_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD263 is a direct state-0 command.  Its wire transaction is implemented
// in dispatch_sig; read/process are the original empty template callbacks.
class Dispatcher_RequestConditionEventReward
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestConditionEventReward();
    virtual ~Dispatcher_RequestConditionEventReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestConditionEventReward) == 0x04u,
              "CMD263 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_CONDITION_EVENT_REWARD_H_
