#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_GOLDEN_COMMERCE_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_GOLDEN_COMMERCE_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_game/dungeon/escort/golden_commerce_types.h"

class Stream;

namespace golden_commerce {

class DBGoldenCommerceReward : public TaiwanInterfaceDBDispatcher {
public:
    DBGoldenCommerceReward();
    virtual ~DBGoldenCommerceReward();
    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx,
                             const RewardClaimMessage& request);
};

#if defined(__i386__)
static_assert(sizeof(DBGoldenCommerceReward) == 0x08u,
              "Golden Commerce Taiwan DB owner must preserve base ABI");
#endif

}  // namespace golden_commerce_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_GOLDEN_COMMERCE_0828_H_
