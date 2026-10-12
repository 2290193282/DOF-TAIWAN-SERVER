#include "old_data/db/event/db_save_object_bring_up.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"

#include <string>



namespace {

enum {
    kObjectBringUpEventEntityType = 155,
};

}  // namespace

// IDA_EA: 0x844FB3E
// IDA_NAME: _ZN20DB_SaveObjectBringUpC2Ev
// DEMANGLED: DB_SaveObjectBringUp::DB_SaveObjectBringUp(void)
DB_SaveObjectBringUp::DB_SaveObjectBringUp() {}

// IDA_EA: 0x845B266
// IDA_NAME: _ZN20DB_SaveObjectBringUpD2Ev
// DEMANGLED: DB_SaveObjectBringUp::~DB_SaveObjectBringUp()
// IDA_EA: 0x845B296
// IDA_NAME: _ZN20DB_SaveObjectBringUpD0Ev
// DEMANGLED: DB_SaveObjectBringUp::~DB_SaveObjectBringUp() deleting destructor
DB_SaveObjectBringUp::~DB_SaveObjectBringUp() {}

// IDA_EA: 0x8446A1E
// IDA_NAME: _ZN20DB_SaveObjectBringUp8dispatchEiiP6Stream
// DEMANGLED: DB_SaveObjectBringUp::dispatch(int,int,Stream *)
bool DB_SaveObjectBringUp::dispatch(int signal,
                                    int user_idx,
                                    Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_OBJECT_BRING_UP* payload =
        stream->GetOutBuffer<SIG_OBJECT_BRING_UP>();
    if (!payload) {
        return false;
    }

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    const std::string* table_name = event_script_mng
        ? event_script_mng->get_db_table_name(
                                            kObjectBringUpEventEntityType)
        : 0;
    if (!table_name) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    if (payload->give_reward_flag) {
        db_handle->set_query(
            "upDate %s set use_material_cnt=use_material_cnt+%u, give_reward_flag=1, occ_date = now() where charac_no=%u and m_id=%s",
            ::toTString(*table_name),
            payload->use_material_count,
            payload->charac_no,
            NumberToString(payload->m_id, 0));
        if (db_handle->exec(true) != true) {
            return false;
        }
        if (!db_handle->getAffectedRowCount()) {
            db_handle->set_query(
                "inSert into %s(charac_no,m_id,use_material_cnt,give_reward_flag,occ_date) values(%u,%s,%u,%u,now())",
                ::toTString(*table_name),
                payload->charac_no,
                NumberToString(payload->m_id, 0),
                payload->use_material_count,
                payload->give_reward_flag);
            if (db_handle->exec(true) != true) {
                cMyTrace trace(
                    "virtual bool DB_SaveObjectBringUp::dispatch(int, int, Stream*)",
                    44733,
                    5);
                trace("upDate DB_SaveObjectBringUp::dispatch failed at charac_no(%u),count(%u),flag(%u)",
                      payload->charac_no,
                      payload->use_material_count,
                      payload->give_reward_flag);
                return false;
            }
        }
        return true;
    }

    db_handle->set_query(
        "upDate %s set use_material_cnt=use_material_cnt+%u where charac_no=%u and m_id=%s",
        ::toTString(*table_name),
        payload->use_material_count,
        payload->charac_no,
        NumberToString(payload->m_id, 0));
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    db_handle->set_query(
        "inSert into %s(charac_no,m_id,use_material_cnt) values(%u,%s,%u)",
        ::toTString(*table_name),
        payload->charac_no,
        NumberToString(payload->m_id, 0),
        payload->use_material_count);
    if (db_handle->exec(true) == true) {
        return true;
    }

    cMyTrace trace("virtual bool DB_SaveObjectBringUp::dispatch(int, int, Stream*)",
                   44750,
                   5);
    trace("upDate DB_SaveObjectBringUp::dispatch failed at charac_no(%u),count(%u)",
          payload->charac_no,
          payload->use_material_count);
    return false;
}
