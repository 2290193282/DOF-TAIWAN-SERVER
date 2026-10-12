#include "old_game/activity/reward/advance_altar_sync_script_abi.h"

#include "old_data/db/core/mysql.h"
#include "common/types/code_page.h"
#include "common/manager/data_manager.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace advancealtar {

// IDA_EA: 0x8133E44
// IDA_NAME: _ZN12advancealtar10SyncScript21truncateItemDescTableEv
// DEMANGLED: advancealtar::SyncScript::truncateItemDescTable(void)
bool SyncScript::truncateItemDescTable() {
    MySQL* db_handle =
        GlobalData::s_db_mgr->GetDBHandle(
            DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    db_handle->set_query("trUncate table charac_advance_altar_item_desc");
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8133EA6
// IDA_NAME: _ZN12advancealtar10SyncScript19insertItemDescTableEv
// DEMANGLED: advancealtar::SyncScript::insertItemDescTable(void)
bool SyncScript::insertItemDescTable() {
    advancealtar::AdvanceAltarShopParameter* parameter =
        G_CDataManager()->get_advance_altar_shop_parameter();

    for (std::map<int, advancealtar::BuyShopData>::const_iterator it =
             parameter->buy_shop_data.begin();
         it != parameter->buy_shop_data.end();
         ++it) {
        std::string query =
            "inSert into charac_advance_altar_item_desc \t\t\t\t  "
            "(ridable_id, item_type, item_id, item_desc) values ";
        std::string rows;

        const advancealtar::BuyShopData& shop_data = it->second;
        const int ridable_id = shop_data.ridable_id;
        getItemValueStirng(ridable_id,
                           shop_data,
                           advancealtar::AdvanceAltarShopType::UNIT,
                           rows);
        rows += ",";
        getItemValueStirng(ridable_id,
                           shop_data,
                           advancealtar::AdvanceAltarShopType::SKILL,
                           rows);
        rows += ",";
        getItemValueStirng(ridable_id,
                           shop_data,
                           advancealtar::AdvanceAltarShopType::TOWER,
                           rows);
        query += rows;

        MySQL* db_handle =
            GlobalData::s_db_mgr->GetDBHandle(
                DB_HANDLE_3, SERVER_GROUP_DEFAULT);
        db_handle->set_query(query.c_str());
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    return true;
}

// IDA_EA: 0x8134132
// IDA_NAME: _ZN12advancealtar10SyncScript18getItemValueStirngEiRKNS_11BuyShopDataENS_20AdvanceAltarShopType1TERSs
// DEMANGLED: advancealtar::SyncScript::getItemValueStirng(int, advancealtar::BuyShopData const&, advancealtar::AdvanceAltarShopType::T, std::string&)
bool SyncScript::getItemValueStirng(
    int ridable_id,
    const advancealtar::BuyShopData& shop_data,
    advancealtar::AdvanceAltarShopType::T shop_type,
    std::string& out) {
    const std::vector<advancealtar::BuyUpgradeData>* upgrades = 0;
    if (shop_type == advancealtar::AdvanceAltarShopType::SKILL) {
        upgrades = &shop_data.skill_upgrades;
    } else if (shop_type ==
               advancealtar::AdvanceAltarShopType::TOWER) {
        upgrades = &shop_data.tower_upgrades;
    } else {
        if (shop_type != advancealtar::AdvanceAltarShopType::UNIT) {
            return false;
        }
        upgrades = &shop_data.unit_upgrades;
    }

    const int count = static_cast<int>(upgrades->size());
    int index = 0;
    for (std::vector<advancealtar::BuyUpgradeData>::const_iterator it =
             upgrades->begin();
         it != upgrades->end();
         ++it, ++index) {
        char converted[1024];
        std::memset(converted, 0, sizeof(converted));

        char* name = const_cast<char*>(it->name.c_str());
        if (CodePage::script2Database(name, converted) == 1) {
            char row[1024];
            std::sprintf(row,
                         "(%d,%d,%d,'%s')",
                         ridable_id,
                         static_cast<int>(shop_type),
                         it->id,
                         converted);
            out += row;
            if (count - 1 > index) {
                out += ", ";
            }
        } else {
            CodePage::database();
            CodePage::script();
            cMyTrace trace(
                "static bool advancealtar::SyncScript::getItemValueStirng(int, const advancealtar::BuyShopData&, advancealtar::AdvanceAltarShopType::T, std::tstring&)",
                3382,
                5);
            trace("Error!!! %s->%s [%s][%s][%d]",
                  CodePage::script(),
                  CodePage::database(),
                  name,
                  converted,
                  0xD36);
            return false;
        }
    }

    return true;
}

}  // namespace advancealtar
