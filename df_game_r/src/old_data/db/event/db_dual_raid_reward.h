#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_DUAL_RAID_REWARD_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_DUAL_RAID_REWARD_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_game/dungeon/raid/dual_raid_types.h"

class Stream;

namespace dual_raid {

class DBDualRaidReward : public TaiwanInterfaceDBDispatcher {
public:
    DBDualRaidReward();
    virtual ~DBDualRaidReward();
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx,
                             const RewardClaimMessage& request);
};

#if defined(__i386__)
static_assert(sizeof(DBDualRaidReward) == 0x08u,
              "Dual Raid Taiwan DB owner must preserve base ABI");
#endif

}  // namespace dual_raid_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_DUAL_RAID_REWARD_0828_H_
