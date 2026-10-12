#include "old_data/db/channel/db_save_channel_user_level.h"

#include "common/config.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844E1DE
// IDA_NAME: _ZN23DB_SaveChannelUserLevelC2Ev
// DEMANGLED: DB_SaveChannelUserLevel::DB_SaveChannelUserLevel(void)
DB_SaveChannelUserLevel::DB_SaveChannelUserLevel() {}

// IDA_EA: 0x845F82C
// IDA_NAME: _ZN23DB_SaveChannelUserLevelD2Ev
// DEMANGLED: DB_SaveChannelUserLevel::~DB_SaveChannelUserLevel()
// IDA_EA: 0x845F85C
// IDA_NAME: _ZN23DB_SaveChannelUserLevelD0Ev
// DEMANGLED: DB_SaveChannelUserLevel::~DB_SaveChannelUserLevel() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveChannelUserLevel::~DB_SaveChannelUserLevel() {}

// IDA_EA: 0x841BE50
// IDA_NAME: _ZN23DB_SaveChannelUserLevel8dispatchEiiP6Stream
// DEMANGLED: DB_SaveChannelUserLevel::dispatch(int,int,Stream *)
bool DB_SaveChannelUserLevel::dispatch(int signal, int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_STATISTIC4LEV* statistic =
        stream->GetOutBuffer<SIG_UPDATE_STATISTIC4LEV>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    for (int level = oldsrv::config::kCharacterLevelMinimum;
         level <= oldsrv::config::kChannelLevelStatisticMaximum;
         ++level) {
        db_handle->set_query(
            "inSert into channel_lev_status(occ_time,channel_no,lev,occ_count) values(from_unixtime(%d),%d,%d,%d)",
            statistic->occ_time,
            statistic->channel_no,
            level,
            statistic->level_counts[level - 1]);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
