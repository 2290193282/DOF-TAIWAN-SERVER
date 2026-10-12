#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SKILL_DAMAGE_RANK_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SKILL_DAMAGE_RANK_EVENT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

class Dispatcher_UpdateBestDamageRank
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_UpdateBestDamageRank();
    virtual ~Dispatcher_UpdateBestDamageRank();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestBestDamageRank
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestBestDamageRank();
    virtual ~Dispatcher_RequestBestDamageRank();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_SkillDamageRankEvent : public ISigDispatcher {
public:
    Inter_SkillDamageRankEvent();
    virtual ~Inter_SkillDamageRankEvent();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpdateBestDamageRank) == 0x04u,
              "CMD732 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_RequestBestDamageRank) == 0x04u,
              "CMD733 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Inter_SkillDamageRankEvent) == 0x04u,
              "skill-damage inter owner must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SKILL_DAMAGE_RANK_EVENT_H_

