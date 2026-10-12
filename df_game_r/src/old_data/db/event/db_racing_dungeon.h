#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_RACING_DUNGEON_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_RACING_DUNGEON_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_game/dungeon/racing/racing_dungeon_types.h"

class Stream;

namespace racing_dungeon {

class DBRacingDungeon : public TaiwanInterfaceDBDispatcher {
public:
    DBRacingDungeon();
    virtual ~DBRacingDungeon();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx,
                             const RewardClaimMessage& request);
};

#if defined(__i386__)
static_assert(sizeof(DBRacingDungeon) == 0x08u,
              "Racing Taiwan DB owner must preserve the base-only ABI");
#endif

}  // namespace racing_dungeon_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_RACING_DUNGEON_0828_H_
