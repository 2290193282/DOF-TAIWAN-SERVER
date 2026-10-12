#ifndef OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_H_
#define OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_H_

#include <stdint.h>

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_game/dungeon/raid/dual_raid_types.h"

class CParty;
class CUser;
class PacketBuf;

namespace dual_raid {

bool IsPolicyLoaded();

// Applies the official [kill monster exp rate] bracket only while `user` is
// a snapshotted member of an active Dual Raid party.  The ordinary monster
// EXP owner calls this after its base/quick-party calculation and before the
// standard per-user bonus aggregation.
unsigned int ScaleMonsterExperience(CParty* party,
                                    CUser* user,
                                    unsigned int experience);

// Shared lifecycle integration points.  They are all idempotent and may be
// called by both the ordinary CParty lifecycle and disconnect cleanup.
void OnPeriodicTick(uint32_t now_epoch);
void OnUserDisconnect(CUser* user);
void OnPartyMemberLeave(CParty* party, CUser* user);
void OnPartyDestroyed(CParty* party);
void OnPartyMoveMap(CParty* party, unsigned int map_x, unsigned int map_y);
void OnPartyMapClear(CParty* party,
                     unsigned int progress,
                     bool clear_flag);
// Returns true when the result belongs to Dual Raid.  The shared result owner
// must then skip the ordinary card-reward path: 3049 transitions through
// NOTI461 into 3050, while 3050 is settled by NOTI464/CMD528.
bool OnPartyDungeonResult(CParty* party,
                          int dungeon_index,
                          bool cleared);

// Taiwan inner-23 response owner.  Public only so the concrete Inter
// dispatcher can hand the typed response back to the session coordinator.
void OnRewardClaimResult(CUser* user, const RewardClaimMessage& response);

}  // namespace dual_raid_0828

class Dispatcher_DualRaidJoin
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DualRaidJoin();
    virtual ~Dispatcher_DualRaidJoin();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DualRaidCommand
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DualRaidCommand();
    virtual ~Dispatcher_DualRaidCommand();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DualRaidReady
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DualRaidReady();
    virtual ~Dispatcher_DualRaidReady();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DualRaidReward
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DualRaidReward();
    virtual ~Dispatcher_DualRaidReward();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterDualRaidReward : public TaiwanInterfaceInterDispatcher {
public:
    InterDualRaidReward();
    virtual ~InterDualRaidReward();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DualRaidJoin) == 0x04u,
              "CMD514 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_DualRaidCommand) == 0x04u,
              "CMD515 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_DualRaidReady) == 0x04u,
              "CMD527 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_DualRaidReward) == 0x04u,
              "CMD528 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterDualRaidReward) == 0x08u,
              "Dual Raid Taiwan Inter owner must preserve base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_RAID_DUAL_RAID_0828_H_
