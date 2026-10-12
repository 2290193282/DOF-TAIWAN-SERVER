#include "old_data/db/auction_cashshop/db_save_cerashop_add_restrict_type.h"

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/mysql.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

enum {
    kDbSignalSaveCerashopAddRestrictType = 832,
    kAccountDbHandle = 1,
    kCharacterDbHandle = 2,
};

}  // namespace

// IDA_EA: 0x844FF66
// IDA_NAME: _ZN30DB_SaveCerashopAddRestrictTypeC2Ev
DB_SaveCerashopAddRestrictType::DB_SaveCerashopAddRestrictType() {}

// IDA_EA: 0x80E472C
// IDA_NAME: _ZN30DB_SaveCerashopAddRestrictTypeD2Ev
// IDA_EA: 0x80E475C
// IDA_NAME: _ZN30DB_SaveCerashopAddRestrictTypeD0Ev
DB_SaveCerashopAddRestrictType::~DB_SaveCerashopAddRestrictType() {}

// IDA_EA: 0x80E14F6
// IDA_NAME: _ZN30DB_SaveCerashopAddRestrictType8dispatchEiiP6Stream
bool DB_SaveCerashopAddRestrictType::dispatch(int /*signal*/,
                                              int /*user_idx*/,
                                              Stream* stream) {
    int restrict_type = 0;
    unsigned int ipg_no = 0;
    unsigned int count = 0;
    int next_date = 0;
    int end_date = 0;
    *stream >> restrict_type;
    *stream >> ipg_no;
    *stream >> count;
    *stream >> next_date;
    *stream >> end_date;

    if (restrict_type == 0) {
        unsigned int account_id = 0;
        *stream >> account_id;
        MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(kAccountDbHandle),
            static_cast<ENUM_SERVER_GROUP>(0));
        if (!database) {
            return false;
        }
        database->set_query(
            "upDate account_cerashop_restrict set count=%u, next_date=%d, "
            "last_access_date=UNIX_TIMESTAMP() where m_id = %u and ipg_no=%u",
            count, next_date, account_id, ipg_no);
        if (!database->exec(true)) {
            return false;
        }
        if (!database->getAffectedRowCount()) {
            database->set_query(
                "inSert into account_cerashop_restrict(m_id, ipg_no, count, "
                "next_date, end_date, last_access_date) values(%u, %u, %u, "
                "%d, %d, UNIX_TIMESTAMP())",
                account_id, ipg_no, count, next_date, end_date);
            if (!database->exec(true)) {
                return false;
            }
        }
    } else if (restrict_type == 1) {
        unsigned int charac_no = 0;
        *stream >> charac_no;
        MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
            static_cast<ENUM_DB_HANDLE_IDX>(kCharacterDbHandle),
            static_cast<ENUM_SERVER_GROUP>(0));
        if (!database) {
            return false;
        }
        database->set_query(
            "upDate charac_cerashop_restrict set count=%u, next_date=%d, "
            "last_access_date=UNIX_TIMESTAMP() where charac_no = %u and "
            "ipg_no=%u",
            count, next_date, charac_no, ipg_no);
        if (!database->exec(true)) {
            return false;
        }
        if (!database->getAffectedRowCount()) {
            database->set_query(
                "inSert into charac_cerashop_restrict(charac_no, ipg_no, "
                "count, next_date, end_date, last_access_date) values(%u, "
                "%u, %u, %d, %d, UNIX_TIMESTAMP())",
                charac_no, ipg_no, count, next_date, end_date);
            if (!database->exec(true)) {
                return false;
            }
        }
    }
    return true;
}

// IDA_EA: 0x80E17BA
// IDA_NAME: _ZN30DB_SaveCerashopAddRestrictType11makeRequestEP5CUserRN19CerashopAddRestrict11DBInputDataE
void DB_SaveCerashopAddRestrictType::makeRequest(
    CUser* user,
    CerashopAddRestrict::DBInputData& input) {
    const CerashopAddRestrict::InfoDaily* const restrict_info =
        input.restrict_info;
    const CerashopAddRestrict::paramDaily* const parameter = input.parameter;
    if (!restrict_info || !parameter) {
        return;
    }

    Stream* const stream = GlobalData::stream_pool()->Acquire(
        "CerashopAddRestrictType.cpp",
        restrict_info->type == CerashopAddRestrict::RestrictType::ACCOUNT
            ? 396
            : 410);
    CStreamGuard guard(stream, true);
    *guard << kDbSignalSaveCerashopAddRestrictType;
    *guard << -1;
    *guard << static_cast<int>(restrict_info->type);
    *guard << parameter->ipg_no;
    *guard << parameter->count;
    *guard << static_cast<int>(parameter->next_date);
    *guard << static_cast<int>(restrict_info->end_time);
    if (restrict_info->type == CerashopAddRestrict::RestrictType::ACCOUNT) {
        *guard << static_cast<unsigned int>(user->get_acc_id());
    } else if (restrict_info->type == CerashopAddRestrict::RestrictType::CHARACTER) {
        *guard << static_cast<unsigned int>(user->getCurCharacNo());
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
