#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_SERIA_BUFF_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_SERIA_BUFF_H_

#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD433 owns only the state-0 IPacketDispatcher vptr.  Its source provider
// intentionally has no registration effect on the frozen 606-slot table.
class Dispatcher_Request_Seria_Buff
    : public IPacketDispatcher<MSG_BASE,
                               stGrowthPowerParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_Request_Seria_Buff();
    virtual ~Dispatcher_Request_Seria_Buff();
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Request_Seria_Buff) == 0x04u,
              "CMD433 must preserve the old vptr-only dispatcher ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_SERIA_BUFF_H_
