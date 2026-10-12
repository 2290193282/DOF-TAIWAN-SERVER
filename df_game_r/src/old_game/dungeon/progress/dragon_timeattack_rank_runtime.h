#ifndef OLDSRV_OLD_GAME_DUNGEON_PROGRESS_DRAGON_TIMEATTACK_RANK_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PROGRESS_DRAGON_TIMEATTACK_RANK_RUNTIME_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CParty;
class CUser;
class PacketBuf;

namespace dragon_timeattack_rank {

// Called after CParty::setBestClearTime for a successfully cleared dungeon.
// The owner filters the dedicated 0828 time-attack dungeon/difficulty.
void RecordPartyClear(CParty* party,
                      int dungeon_index,
                      int dungeon_difficulty,
                      int clear_time_ms);

}  // namespace dragon_timeattack_rank_0828

class Dispatcher_DragonDungeonTimeAttackRankerBoard
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_DragonDungeonTimeAttackRankerBoard();
    virtual ~Dispatcher_DragonDungeonTimeAttackRankerBoard();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_DragonTimeAttackRank : public ISigDispatcher {
public:
    Inter_DragonTimeAttackRank();
    virtual ~Inter_DragonTimeAttackRank();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DragonDungeonTimeAttackRankerBoard) == 0x04u,
              "CMD696 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Inter_DragonTimeAttackRank) == 0x04u,
              "CMD696 inter owner must remain vptr-only on target i386");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PROGRESS_DRAGON_TIMEATTACK_RANK_RUNTIME_H_
