#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EXP_KEEPING_EVENT_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EXP_KEEPING_EVENT_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CUser;
class PacketBuf;

namespace exp_keeping {

// Queue the DB-authoritative NOTI530 state at the first selected-character
// town-ready boundary.  ResetSession invalidates every delayed Taiwan reply.
void BeginSession(CUser* user);
void ResetSession(CUser* user);

// CParty::kill_monster captures source_level immediately before, then calls
// this after, the ordinary reason-1 gain_exp_sp(final_member_exp, ...)
// mutation.  final_experience must be the post-QuickParty/
// post-sumGainedEachExp/post-BlueMarble value.
void OnMonsterExperienceAwarded(CUser* user,
                                uint32_t final_experience,
                                uint32_t dungeon_index,
                                uint32_t source_level,
                                uint64_t source_token);

// Stable source identity for the future CParty hook.  The tuple is available
// at the authoritative monster-reward boundary and includes the receiving
// character so one party kill can legitimately accrue once per member.
uint64_t MakeMonsterSourceToken(uint32_t party_index,
                                uint64_t map_start_time,
                                uint32_t map_index,
                                uint32_t monster_unique_id,
                                uint32_t charac_no);

// A claim is crossing its reservation/target/confirmation boundary.  Session
// teardown may use this to defer voluntary character switches; DB recovery is
// still safe if the transport itself disappears.
bool HasUnconfirmedClaim(CUser* user);

// Shared CUser::gain_exp_sp wiring uses this single check.  It remains false
// only for the feature's own correlated rewardExp call, and blocks every
// unrelated EXP writer until the absolute target is DB-confirmed.
bool RejectUnrelatedExperienceMutation(CUser* user);

}  // namespace exp_keeping_0828

class Dispatcher_RequestExpKeepingEventReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestExpKeepingEventReward();
    virtual ~Dispatcher_RequestExpKeepingEventReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterExpKeepingEvent : public TaiwanInterfaceInterDispatcher {
public:
    InterExpKeepingEvent();
    virtual ~InterExpKeepingEvent();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestExpKeepingEventReward) == 0x04u,
              "CMD594 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterExpKeepingEvent) == 0x08u,
              "CMD594 Taiwan Inter owner must preserve the base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_EXP_KEEPING_EVENT_0828_H_
