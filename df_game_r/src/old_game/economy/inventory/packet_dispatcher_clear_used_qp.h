#ifndef OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_CLEAR_USED_QP_H_
#define OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_CLEAR_USED_QP_H_

#include "old_game/economy/mall/npc_shop_params.h"


// IDA unit: 13_database_dispatch/Dispatcher_Clear_Used_QP.
// CMD400 owns the quest-point reset request, but its business provider is the
// NPC quest shop; it therefore has its own canonical NPC source unit.
class Dispatcher_Clear_Used_QP
    : public IPacketDispatcher<MSG_BASE,
                               STQuestShopParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_Clear_Used_QP();
    virtual ~Dispatcher_Clear_Used_QP();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Clear_Used_QP) == 0x04u,
              "Dispatcher_Clear_Used_QP must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_PACKET_DISPATCHER_CLEAR_USED_QP_H_
