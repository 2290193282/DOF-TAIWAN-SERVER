#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD242. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_CompoundItemByExpertJob
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CompoundItemByExpertJob();
    virtual ~Dispatcher_CompoundItemByExpertJob();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD243.  The old object contains only the inherited state-0 dispatcher
// vptr; its send slot remains the shared IPacketDispatcher empty provider.
class Dispatcher_GiveupExpertJob
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_GiveupExpertJob();
    virtual ~Dispatcher_GiveupExpertJob();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CompoundItemByExpertJob) == 0x04u,
              "CMD242 must remain the old vptr-only dispatcher object");
static_assert(sizeof(Dispatcher_GiveupExpertJob) == 0x04u,
              "CMD243 must remain the old vptr-only dispatcher object");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_JOB_H_
