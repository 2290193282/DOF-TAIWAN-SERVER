#include "old_data/db/statistics_log/db_insert_circulation_statistic.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844EE3A
// IDA_NAME: _ZN29DB_InsertCirculationStatisticC2Ev
// DEMANGLED: DB_InsertCirculationStatistic::DB_InsertCirculationStatistic(void)
DB_InsertCirculationStatistic::DB_InsertCirculationStatistic() {}

// IDA_EA: 0x845D87C
// IDA_NAME: _ZN29DB_InsertCirculationStatisticD2Ev
// DEMANGLED: DB_InsertCirculationStatistic::~DB_InsertCirculationStatistic()
// IDA_EA: 0x845D8AC
// IDA_NAME: _ZN29DB_InsertCirculationStatisticD0Ev
// DEMANGLED: DB_InsertCirculationStatistic::~DB_InsertCirculationStatistic() deleting destructor
DB_InsertCirculationStatistic::~DB_InsertCirculationStatistic() {}

// IDA_EA: 0x84327BA
// IDA_NAME: _ZN29DB_InsertCirculationStatistic8dispatchEiiP6Stream
// DEMANGLED: DB_InsertCirculationStatistic::dispatch(int,int,Stream *)
bool DB_InsertCirculationStatistic::dispatch(int signal, int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_CIRCULATION_STATISTIC* payload =
        stream->GetOutBuffer<SIG_CIRCULATION_STATISTIC>();
    CEnvironment* environment = G_CEnvironment();
    const int channel_no = environment->get_channel_no();
    db_handle->set_query(
        "inSert into log_gold_stat(channel_no,occ_time ,level,dungeon_drop,result_card,sell_store,quest_reward,death_tower_re"
        "ward,illusion_tower_reward,war_area_drop,member_tax,blood_dungeon_reward,blood_dungeon_lotto,power_dungeon_drop,powe"
        "r_dungeon_result_card,buy_store,stamina_recovery,repair_item,private_store_commission,gold_card,gold_drop,upgrade,qu"
        "est_use,mail_commission,punish_user,restrict_trade,guild_level_up,guild_skill,guild_mail,item_compound,blood_dungeon"
        "_enter,buy_cerashop,war_area_enter,assault_gold,upgrade_guild_agit,upgrade_guild_cargo) values(%d,from_unixtime(%d),"
        "%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
        channel_no,
        payload->occ_time,
        payload->level,
        payload->dungeon_drop,
        payload->result_card,
        payload->sell_store,
        payload->quest_reward,
        payload->death_tower_reward,
        payload->illusion_tower_reward,
        payload->war_area_drop,
        payload->member_tax,
        payload->blood_dungeon_reward,
        payload->blood_dungeon_lotto,
        payload->power_dungeon_drop,
        payload->power_dungeon_result_card,
        payload->buy_store,
        payload->stamina_recovery,
        payload->repair_item,
        payload->private_store_commission,
        payload->gold_card,
        payload->gold_drop,
        payload->upgrade,
        payload->quest_use,
        payload->mail_commission,
        payload->punish_user,
        payload->restrict_trade,
        payload->guild_level_up,
        payload->guild_skill,
        payload->guild_mail,
        payload->item_compound,
        payload->blood_dungeon_enter,
        payload->buy_cerashop,
        payload->war_area_enter,
        payload->assault_gold,
        payload->upgrade_guild_agit,
        payload->upgrade_guild_cargo);
    return db_handle->exec(true) == true;
}
