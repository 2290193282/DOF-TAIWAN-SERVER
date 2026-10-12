#include "old_data/db/event/db_statistic_advance_altar.h"

#include "old_game/activity/reward/advance_altar_statistics.h"
#include "common/manager/common_time_game_option_abi.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

#include <cstdio>
#include <ctime>

namespace advancealtar {

// IDA_EA: 0x844FC3A
// IDA_NAME: _ZN12advancealtar24DB_StatisticAdvanceAltarC2Ev
// DEMANGLED: advancealtar::DB_StatisticAdvanceAltar::DB_StatisticAdvanceAltar(void)
DB_StatisticAdvanceAltar::DB_StatisticAdvanceAltar() : DBDispatcher() {}

// IDA_EA: 0x8142952
// IDA_NAME: _ZN12advancealtar24DB_StatisticAdvanceAltarD2Ev
// DEMANGLED: advancealtar::DB_StatisticAdvanceAltar::~DB_StatisticAdvanceAltar()
// IDA_EA: 0x8142982
// IDA_NAME: _ZN12advancealtar24DB_StatisticAdvanceAltarD0Ev
// DEMANGLED: advancealtar::DB_StatisticAdvanceAltar::~DB_StatisticAdvanceAltar() deleting destructor
DB_StatisticAdvanceAltar::~DB_StatisticAdvanceAltar() {}

// IDA_EA: 0x81412B2
// IDA_NAME: _ZN12advancealtar24DB_StatisticAdvanceAltar8dispatchEiiP6Stream
// DEMANGLED: advancealtar::DB_StatisticAdvanceAltar::dispatch(int,int,Stream *)
bool DB_StatisticAdvanceAltar::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));

    tm today;
    __GetCurrentDNFDate(today);
    char date[12] = {};
    std::sprintf(date,
                 "%04d-%02d-%02d",
                 today.tm_year + 1900,
                 today.tm_mon + 1,
                 today.tm_mday);

    int entrance_count = 0;
    *stream >> entrance_count;
    for (int index = 0; index < entrance_count; ++index) {
        STAdvanceAltarEntranceData* entrance =
            stream->GetOutBuffer<STAdvanceAltarEntranceData>();
        if (!entrance) {
            throw 1572;
        }

        cMyTrace trace(
            "virtual bool advancealtar::DB_StatisticAdvanceAltar::dispatch(int, int, Stream*)",
            1574,
            0);
        trace("[AdvanceAltar] %u,%u,%u,%u",
              entrance->stage_id,
              entrance->entrance_count,
              entrance->success_count,
              entrance->obtain_star);

        db_handle->set_query(
            " upDate log_advance_alter_entrance  set entrance_count = entrance_count + %u  , success_count = success_count + %u"
            "  , obtain_star = obtain_star + %u  where occ_date = '%s' and stage_id = %u",
            entrance->entrance_count,
            entrance->success_count,
            entrance->obtain_star,
            date,
            entrance->stage_id);
        if (db_handle->exec(true) && db_handle->getAffectedRowCount()) {
            continue;
        }

        db_handle->set_query(
            " inSert into log_advance_alter_entrance(occ_date,  stage_id, entrance_count, success_count, obtain_star)  values('"
            "%s', %u, %u, %u, %u) ",
            date,
            entrance->stage_id,
            entrance->entrance_count,
            entrance->success_count,
            entrance->obtain_star);
        db_handle->exec(true);
    }

    int layer_count = 0;
    *stream >> layer_count;
    for (int index = 0; index < layer_count; ++index) {
        STAdvanceAltarLayerStatData* layer =
            stream->GetOutBuffer<STAdvanceAltarLayerStatData>();
        if (!layer) {
            throw 1613;
        }

        cMyTrace trace(
            "virtual bool advancealtar::DB_StatisticAdvanceAltar::dispatch(int, int, Stream*)",
            1615,
            0);
        trace("[AdvanceAltar] %u,%u,%u,%u,%u",
              layer->item_type,
              layer->item_id,
              layer->item_level,
              layer->use_count,
              layer->use_star);

        db_handle->set_query(
            " upDate log_advance_alter_layer_stat  set use_count = use_count + %u, use_star = use_star + %u  where occ_date = '"
            "%s' and item_type = %u and item_id = %u and item_level = %u",
            layer->use_count,
            layer->use_star,
            date,
            layer->item_type,
            layer->item_id,
            layer->item_level);
        if (db_handle->exec(true) && db_handle->getAffectedRowCount()) {
            continue;
        }

        db_handle->set_query(
            " inSert into log_advance_alter_layer_stat(occ_date,  item_type, item_id, item_level, use_count, use_star) values('"
            "%s', %u, %u, %u, %u, %u)",
            date,
            layer->item_type,
            layer->item_id,
            layer->item_level,
            layer->use_count,
            layer->use_star);
        db_handle->exec(true);
    }

    return true;
}

}  // namespace advancealtar
