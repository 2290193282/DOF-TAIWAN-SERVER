#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_VERIFY_CREATURE_QUEST_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_VERIFY_CREATURE_QUEST_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_VerifyCreatureQuest
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)> {
public:
    Dispatcher_VerifyCreatureQuest();
    virtual ~Dispatcher_VerifyCreatureQuest();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_VerifyCreatureQuest) == 0x04u,
              "CMD480 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_VERIFY_CREATURE_QUEST_H_
