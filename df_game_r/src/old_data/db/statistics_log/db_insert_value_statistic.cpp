#include "old_data/db/statistics_log/db_insert_value_statistic.h"

#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"



// IDA_EA: 0x844E7C6
// IDA_NAME: _ZN23DB_InsertValueStatisticC2Ev
// DEMANGLED: DB_InsertValueStatistic::DB_InsertValueStatistic(void)
DB_InsertValueStatistic::DB_InsertValueStatistic() {}

// IDA_EA: 0x845E71C
// IDA_NAME: _ZN23DB_InsertValueStatisticD2Ev
// DEMANGLED: DB_InsertValueStatistic::~DB_InsertValueStatistic()
// IDA_EA: 0x845E74C
// IDA_NAME: _ZN23DB_InsertValueStatisticD0Ev
// DEMANGLED: DB_InsertValueStatistic::~DB_InsertValueStatistic() deleting destructor
DB_InsertValueStatistic::~DB_InsertValueStatistic() {}

// IDA_EA: 0x842945E
// IDA_NAME: _ZN23DB_InsertValueStatistic8dispatchEiiP6Stream
// DEMANGLED: DB_InsertValueStatistic::dispatch(int,int,Stream *)
bool DB_InsertValueStatistic::dispatch(int signal, int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(4),static_cast<ENUM_SERVER_GROUP>( 0));
    SIG_VALUE_STATISTIC* payload = stream->GetOutBuffer<SIG_VALUE_STATISTIC>();
    CEnvironment* environment = G_CEnvironment();
    const int channel_no = environment->get_channel_no();
    db_handle->set_query(
        "inSert into log_value_stat(channel_no,occ_time ,level,uv,drop_gold,drop_item,result_card_gold,result_card_item,gold_"
        "card_item,store_item_buy,jar_item,disjoint_create,upgrade_faild_forced_disjoint,quest_reward,deathtower_card_gold,de"
        "athtower_card_item,consume_store_item_buy,consume_upgrade_attempt,consume_upgrade_faild,consume_stamina_recovery,con"
        "sume_quest_consume,consume_auction_commision,consume_item_disjoint,consume_item_repair,consume_item_use,consume_item"
        "_drop,consume_gold_drop,consume_gold_card_price) values(%d,from_unixtime(%d),%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
        "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
        channel_no,
        payload->occ_time,
        payload->level,
        payload->uv,
        payload->drop_gold,
        payload->drop_item,
        payload->result_card_gold,
        payload->result_card_item,
        payload->gold_card_item,
        payload->store_item_buy,
        payload->jar_item,
        payload->disjoint_create,
        payload->upgrade_faild_forced_disjoint,
        payload->quest_reward,
        payload->deathtower_card_gold,
        payload->deathtower_card_item,
        payload->consume_store_item_buy,
        payload->consume_upgrade_attempt,
        payload->consume_upgrade_faild,
        payload->consume_stamina_recovery,
        payload->consume_quest_consume,
        payload->consume_auction_commision,
        payload->consume_item_disjoint,
        payload->consume_item_repair,
        payload->consume_item_use,
        payload->consume_item_drop,
        payload->consume_gold_drop,
        payload->consume_gold_card_price);
    return db_handle->exec(true) == true;
}
