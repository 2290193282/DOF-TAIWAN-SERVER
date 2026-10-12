#ifndef OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOURNAMENT_H_
#define OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOURNAMENT_H_

#include "old_data/db/core/taiwan_interface_db_dispatcher.h"
#include "old_game/pvp/tournament/pvp_tournament_types.h"

class Stream;

namespace pvp_tournament {

class DBPvpTournament : public TaiwanInterfaceDBDispatcher {
public:
    DBPvpTournament();
    virtual ~DBPvpTournament();

    virtual int dispatch(int signal, int user_idx, Stream* stream);

    // Client operations use a live UID.  Room result, release and expiry
    // operations use -1 so their durable transition survives disconnect.
    static bool QueueRequest(int user_idx, const Message& request);
};

#if defined(__i386__)
static_assert(sizeof(DBPvpTournament) == 0x08u,
              "0828 tournament Taiwan DB owner must preserve base ABI");
#endif

}  // namespace pvp_tournament_0828

#endif  // OLDSRV_OLD_DATA_DB_PVP_PARTY_DB_PVP_TOURNAMENT_0828_H_
