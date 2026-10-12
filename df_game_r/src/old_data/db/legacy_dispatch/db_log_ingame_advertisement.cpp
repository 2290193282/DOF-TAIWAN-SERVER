#include "old_data/db/statistics_log/db_log_ingame_advertisement.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

// IDA_EA: 0x844FEF6
// IDA_NAME: _ZN25DB_LogIngameAdvertisementC2Ev
// DEMANGLED: DB_LogIngameAdvertisement::DB_LogIngameAdvertisement(void)
DB_LogIngameAdvertisement::DB_LogIngameAdvertisement() {}

// IDA_EA: 0x845A95E
// IDA_NAME: _ZN25DB_LogIngameAdvertisementD2Ev
// DEMANGLED: DB_LogIngameAdvertisement::~DB_LogIngameAdvertisement()
// IDA_EA: 0x845A98E
// IDA_NAME: _ZN25DB_LogIngameAdvertisementD0Ev
// DEMANGLED: DB_LogIngameAdvertisement::~DB_LogIngameAdvertisement() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LogIngameAdvertisement::~DB_LogIngameAdvertisement() {}

// IDA_EA: 0x844C304
// IDA_NAME: _ZN25DB_LogIngameAdvertisement8dispatchEiiP6Stream
// DEMANGLED: DB_LogIngameAdvertisement::dispatch(int,int,Stream *)
bool DB_LogIngameAdvertisement::dispatch(int signal, int user_idx,
                                         Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_IN_GAME_ADVERTISEMENT_LOG log;
    if (stream->get_binary(&log, sizeof(log)) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "inSert into log_in_game_ad(occ_date, m_id, charac_no, state, view_code, type_code, banner_id, dungeon_index, village"
        "_index, area_index) values (now(), %u, %u, %d, %d, %d, %u, %d, %d, %d)",
        log.m_id,
        log.charac_no,
        static_cast<int>(log.state),
        static_cast<int>(log.view_code),
        static_cast<int>(log.type_code),
        log.banner_id,
        log.dungeon_index,
        log.village_index,
        log.area_index);
    return db_handle->exec(true) == true;
}
