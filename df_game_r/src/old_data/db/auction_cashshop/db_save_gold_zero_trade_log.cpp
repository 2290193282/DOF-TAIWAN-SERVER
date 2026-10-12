#include "old_data/db/auction_cashshop/db_save_gold_zero_trade_log.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"



namespace {

const ENUM_DB_HANDLE_IDX kZeroTradeLogDbHandle = DB_HANDLE_LOG;
const ENUM_SERVER_GROUP kZeroTradeLogDefaultServerGroup =
    SERVER_GROUP_DEFAULT;

}  // namespace

// IDA_EA: 0x844E49A
// IDA_NAME: _ZN23DB_SaveGoldZeroTradeLogC2Ev
// DEMANGLED: DB_SaveGoldZeroTradeLog::DB_SaveGoldZeroTradeLog(void)
DB_SaveGoldZeroTradeLog::DB_SaveGoldZeroTradeLog() {}

// IDA_EA: 0x845F040
// IDA_NAME: _ZN23DB_SaveGoldZeroTradeLogD2Ev
// DEMANGLED: DB_SaveGoldZeroTradeLog::~DB_SaveGoldZeroTradeLog()
// IDA_EA: 0x845F070
// IDA_NAME: _ZN23DB_SaveGoldZeroTradeLogD0Ev
// DEMANGLED: DB_SaveGoldZeroTradeLog::~DB_SaveGoldZeroTradeLog()
DB_SaveGoldZeroTradeLog::~DB_SaveGoldZeroTradeLog() {}

// IDA_EA: 0x84227F6
// IDA_NAME: _ZN23DB_SaveGoldZeroTradeLog8dispatchEiiP6Stream
// DEMANGLED: DB_SaveGoldZeroTradeLog::dispatch(int,int,Stream *)
bool DB_SaveGoldZeroTradeLog::dispatch(int signal,
                                       int user_idx,
                                       Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream->enable_out(sizeof(SIG_SAVE_GOLD_ZERO_TRADE_LOG))) {
        return false;
    }
    SIG_SAVE_GOLD_ZERO_TRADE_LOG* payload =
        reinterpret_cast<SIG_SAVE_GOLD_ZERO_TRADE_LOG*>(stream->get_buffer());
    stream->out_ptr(sizeof(*payload));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        kZeroTradeLogDbHandle,
        kZeroTradeLogDefaultServerGroup);
    db_handle->set_query(
        "inSert into dnf_zeromoney_log(to_m_id,to_nat_ip,to_peer_ip,to_nat_type,from_m_id,from_nat_ip,from_peer_ip,from_nat_type,occ_time,injury_money,to_mac_addr,from_mac_addr) values(%s,'%s','%s',%d,%s,'%s','%s',%d,now(),%d,'%s','%s')",
        NumberToString(payload->to_m_id, 0),
        payload->to_nat_ip,
        payload->to_peer_ip,
        static_cast<int>(payload->to_nat_type),
        NumberToString(payload->from_m_id, 1),
        payload->from_nat_ip,
        payload->from_peer_ip,
        static_cast<int>(payload->from_nat_type),
        payload->injury_money,
        payload->to_mac,
        payload->from_mac);
    return db_handle->exec(true) == true;
}
