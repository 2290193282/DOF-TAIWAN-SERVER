#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_ZOMBIE_MODE_RESULT_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_ZOMBIE_MODE_RESULT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_game/dungeon/instance/zombie_mode_result_types.h"

class Stream;

namespace zombie_mode {

class DBZombieModeResult : public TaiwanInterfaceDBDispatcher {
public:
    DBZombieModeResult();
    virtual ~DBZombieModeResult();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    static bool QueueRequest(int user_idx, const Message& request);
};

#if defined(__i386__)
static_assert(sizeof(DBZombieModeResult) == 0x08u,
              "Zombie Taiwan DB owner must preserve the base-only ABI");
#endif

}  // namespace zombie_mode_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_ZOMBIE_MODE_RESULT_0828_H_
