#include "old_data/PVF/etc/action_point_sync_script.h"

#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/mysql.h"
#include "common/types/code_page.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace APSystem {

// IDA_EA: 0x812262A
// IDA_NAME: _ZN8APSystem11CSyncScript17TruncateDescTableEv
// DEMANGLED: APSystem::CSyncScript::TruncateDescTable(void)
bool CSyncScript::TruncateDescTable() {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    db_handle->set_query("trUncate table charac_action_point_desc");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x812268C
// IDA_NAME: _ZN8APSystem11CSyncScript15InsertDescTableEv
// DEMANGLED: APSystem::CSyncScript::InsertDescTable(void)
bool CSyncScript::InsertDescTable() {
    CDataManager* data_manager = G_CDataManager();
    APSystem::CActionPointEtcParameter* parameter =
        data_manager->GetActionPointEtcParameter(false);
    const std::map<uint16_t, const APSystem::CActionPointEx*>&
        action_index = parameter->action_index();

    std::string query;
    std::string v12;
    query =
        "inSert into charac_action_point_desc "
        "(action_index, action_group_index, action_group_name) values ";

    const int count = static_cast<int>(action_index.size());
    int loop_index = 0;
    for (std::map<uint16_t,
                  const APSystem::CActionPointEx*>::const_iterator it =
             action_index.begin();
         it != action_index.end();
         ++it, ++loop_index) {
        const APSystem::CActionPointEx* action = it->second;
        if (action == 0) {
            continue;
        }

        std::string row;
        char action_index_text[68];
        char action_group_index_text[68];
        std::sprintf(action_index_text, "%u", action->action_index);
        std::sprintf(action_group_index_text, "%u",
                     action->action_group_index);

        char converted[1024];
        std::memset(converted, 0, sizeof(converted));
        char* group_name =
            const_cast<char*>(action->user_message.c_str());
        if (CodePage::script2Database(group_name, converted) == 1) {
            row += "(";
            row += action_index_text;
            row += ", ";
            row += action_group_index_text;
            row += ", ";
            row += "'";
            row += converted;
            row += "'";
            row += ")";
            if (count - 1 > loop_index) {
                row += ",";
            }
            query += row;
        } else {
            CodePage::database();
            CodePage::script();
            cMyTrace trace(
                "static bool APSystem::CSyncScript::InsertDescTable()",
                1033,
                5);
            trace("Error!!! %s->%s [%s][%s][%d]",
                  CodePage::script(),
                  CodePage::database(),
                  group_name,
                  converted,
                  action->action_index);
            return false;
        }
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    db_handle->set_query(query.c_str());
    return db_handle->exec(true) == true;
}

}  // namespace APSystem
