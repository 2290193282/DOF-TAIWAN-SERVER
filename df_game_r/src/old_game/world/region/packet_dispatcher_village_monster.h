#ifndef OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_VILLAGE_MONSTER_H_
#define OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_VILLAGE_MONSTER_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_FightVillageMonster
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_FightVillageMonster();
    virtual ~Dispatcher_FightVillageMonster();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_FinishVillageMonsterFighting
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_FinishVillageMonsterFighting();
    virtual ~Dispatcher_FinishVillageMonsterFighting();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_FightVillageMonster) == 0x04u,
              "CMD267 must remain vptr-only");
static_assert(sizeof(Dispatcher_FinishVillageMonsterFighting) == 0x04u,
              "CMD268 must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_VILLAGE_MONSTER_H_
