#include "old_game/world/region/game_world.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_data/channel/channel_db_payloads.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/transport/network_endpoint.h"
#include "common/manager/system_time.h"

#include <cstring>
#include <map>
#include <stdint.h>

namespace {

typedef std::map<int, int> NatTypeCounts;

#pragma pack(push, 1)
struct SIG_NAT_TYPE_USER {
    int32_t nat_type;
    int32_t nat_count;
    int32_t occ_time;
};
#pragma pack(pop)

static_assert(sizeof(SIG_NAT_TYPE_USER) == 0x0cu,
              "SIG_NAT_TYPE_USER must retain the original DB wire size");

}  // namespace

// IDA_EA: 0x86CD0B0
// IDA_NAME: _ZN9GameWorld30UpdateStatistic4ChannelUserLevEv
// DEMANGLED: GameWorld::UpdateStatistic4ChannelUserLev(void)
void GameWorld::UpdateStatistic4ChannelUserLev() {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("world.cpp", 5291), true);
    *guard << 107;
    *guard << -1;

    SIG_UPDATE_STATISTIC4LEV* const payload =
        static_cast<SIG_UPDATE_STATISTIC4LEV*>(
            guard.GetInBufferUntyped(sizeof(SIG_UPDATE_STATISTIC4LEV)));
    std::memset(payload, 0, sizeof(*payload));
    payload->channel_no = G_CEnvironment()->get_channel_no();
    payload->occ_time = GlobalData::system_time()->getCurSec();

    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user && user->getCurCharacR()) {
            const int level = user->get_charac_level();
            if (level >= oldsrv::config::kCharacterLevelMinimum &&
                level <= oldsrv::config::kChannelLevelStatisticMaximum) {
                ++payload->level_counts[
                    level - oldsrv::config::kCharacterLevelMinimum];
            }
        }
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
}

// IDA_EA: 0x86CD2F6
// IDA_NAME: _ZN9GameWorld17UpdateNatTypeUserEv
// DEMANGLED: GameWorld::UpdateNatTypeUser(void)
int GameWorld::UpdateNatTypeUser() {
    NatTypeCounts counts;
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user) {
            const int nat_type = static_cast<int>(
                static_cast<int8_t>(user->network_.get_nat_type()));
            ++counts[nat_type];
        }
    }

    const int occ_time = GlobalData::system_time()->getCurSec();
    for (NatTypeCounts::const_iterator it = counts.begin();
         it != counts.end(); ++it) {
        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("world.cpp", 5359), true);
        *guard << 132;
        *guard << -1;

        SIG_NAT_TYPE_USER* const payload =
            static_cast<SIG_NAT_TYPE_USER*>(
                guard.GetInBufferUntyped(sizeof(SIG_NAT_TYPE_USER)));
        std::memset(payload, 0, sizeof(*payload));
        payload->nat_type = it->first;
        payload->nat_count = it->second;
        payload->occ_time = occ_time;
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    }

    return 0;
}
