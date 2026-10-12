#ifndef OLDSRV_OLD_GAME_PVP_MATCH_MATCHING_CHANNEL_HANDOFF_H_
#define OLDSRV_OLD_GAME_PVP_MATCH_MATCHING_CHANNEL_HANDOFF_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <stdint.h>

class CParty;
class CUser;
class PacketBuf;
class PvP_Room;

namespace matching_channel_handoff {

// Server-side context released only after CMD521 has atomically claimed the
// complete matching-channel cohort.  cohort_id is the 140828 project-owned
// meaning assigned to NOTI606.field1; it is not claimed as official behavior.
struct ClaimedHandoff {
    uint32_t handoff_id;
    uint32_t cohort_id;
    uint32_t endpoint_key;
    uint32_t endpoint_port_like;
    int32_t source_server_group;
    int32_t source_channel;
    int32_t target_server_group;
    int32_t target_channel;
    uint8_t field0;
    uint8_t matching_bound;
    uint16_t matching_mode;
    uint8_t cohort_slot;
    uint8_t cohort_size;
    uint8_t cohort_flags;
};

typedef void (*ClaimReadyCallback)(CUser* user);

// Parses the first record of official Etc/matchingmode.etc and accepts only
// the channel-bound zombie 3+3/min5/max6/creator-mage policy.  It never maps
// the textual bound to a guessed numeric client enum.
bool LoadMatchingModePolicy();
bool IsMatchingModePolicyLoaded();

// Explicit authoritative container bindings.  The matching producer supplies
// the exact client pair.  Party binding accepts only a live zombie CDungeon;
// room binding accepts only a live 8-seat PvP_Room with 5..6 members.
bool BindZombieParty(CParty* party,
                     uint8_t matching_bound,
                     uint16_t matching_mode);
bool BindMatchingRoom(PvP_Room* room,
                      uint8_t matching_bound,
                      uint16_t matching_mode);
void UnbindZombieParty(CParty* party);
void UnbindMatchingRoom(PvP_Room* room);

// Called by the authoritative route owner after it has selected a target
// channel and supplied the exact five-field NOTI606 writer values except for
// field4, which this owner allocates as the durable one-shot handoff id.
// cohort_id/slot/size are allocated by the project-owned DB route cohort.
// Source and target must remain in the user's DB_HANDLE_2 server group.
bool BeginChannelMove(CUser* user,
                      uint8_t matching_bound,
                      uint16_t matching_mode,
                      uint8_t field0,
                      uint32_t cohort_id,
                      uint32_t endpoint_key,
                      uint16_t endpoint_port_like,
                      int target_server_group,
                      int target_channel,
                      uint8_t cohort_slot,
                      uint8_t cohort_size);

// Target coordinator consumes this once after CMD521/SQL025 succeeds.  A
// failed, stale, wrong-target or replayed claim never becomes consumable.
bool ConsumeClaimedHandoff(CUser* user, ClaimedHandoff& handoff);

// Optional target-coordinator notification.  It fires once for each durable
// successful member claim; the authority collects all slots and requires the
// final ALL_CLAIMED barrier before target-room reconstruction.  Replay, stale
// and wrong-target results never invoke it.
void SetClaimReadyCallback(ClaimReadyCallback callback);

// User/container lifecycle hooks.  Reset keeps an already armed ticket alive
// for the target connection, but cancels a merely reserved ticket best-effort.
void ResetSession(CUser* user);

}  // namespace matching_channel_handoff

class Dispatcher_MatchingDungeonExit
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_MatchingDungeonExit();
    virtual ~Dispatcher_MatchingDungeonExit();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ChannelMoveSuccess
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ChannelMoveSuccess();
    virtual ~Dispatcher_ChannelMoveSuccess();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class InterMatchingChannelHandoff
    : public TaiwanInterfaceInterDispatcher {
public:
    InterMatchingChannelHandoff();
    virtual ~InterMatchingChannelHandoff();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_MatchingDungeonExit) == 0x04u,
              "CMD520 dispatcher must remain vptr-only on i386");
static_assert(sizeof(Dispatcher_ChannelMoveSuccess) == 0x04u,
              "CMD521 dispatcher must remain vptr-only on i386");
static_assert(sizeof(InterMatchingChannelHandoff) == 0x08u,
              "matching handoff Inter owner must preserve base ABI");
#endif

#endif  // OLDSRV_OLD_GAME_PVP_MATCH_MATCHING_CHANNEL_HANDOFF_H_
