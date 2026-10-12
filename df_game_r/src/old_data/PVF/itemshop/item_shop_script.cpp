#include "old_data/PVF/itemshop/item_shop_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/common/script_file_list.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "common/manager/system_time.h"

#include <string.h>
#include <time.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

char g_itemShopScriptBaseDirectory[0x101] = {};
STScriptFileList g_itemShopScriptFileList;

}  // namespace

typedef char ItemShopScript_Size_Must_Be_0x50[
    (sizeof(ItemShopScript) == 0x50) ? 1 : -1];
typedef char CItemShop_Size_Must_Be_0x24[
    (sizeof(CItemShop) == 0x24) ? 1 : -1];

// IDA_EA: 0x83D076A
// IDA_NAME: _ZN14ItemShopScriptD2Ev
// DEMANGLED: ItemShopScript::~ItemShopScript()
ItemShopScript::~ItemShopScript() {}

// IDA_EA: 0x8514046
// IDA_NAME: _ZN14ItemShopScript5clearEv
// DEMANGLED: ItemShopScript::clear(void)
ItemShopScript* ItemShopScript::clear() {
    script_index = 0;
    shop_type = 0;
    npc_index = 0;
    only_buy = false;
    message.clear();
    sell_items.clear();
    tab_names.clear();
    sell_limit_items.clear();
    one_a_day_item_lists.clear();
    one_a_day_enabled = false;
    one_a_day_start_time = 0;
    reserved_4c = 0;
    return this;
}

// IDA_EA: 0x85140C8
// IDA_NAME: _ZN14ItemShopScriptC2Ev
// DEMANGLED: ItemShopScript::ItemShopScript(void)
ItemShopScript::ItemShopScript()
    : script_index(0),
      shop_type(0),
      npc_index(0),
      only_buy(false),
      pad_0d(),
      message(),
      sell_items(),
      tab_names(),
      sell_limit_items(),
      one_a_day_item_lists(),
      one_a_day_enabled(false),
      pad_45(),
      one_a_day_start_time(0),
      reserved_4c(0) {
    clear();
}

// IDA_EA: 0x8518E32
// IDA_NAME: _ZN14ItemShopScriptC2ERKS_
// DEMANGLED: ItemShopScript::ItemShopScript(ItemShopScript const&)
ItemShopScript::ItemShopScript(const ItemShopScript& other)
    : script_index(other.script_index),
      shop_type(other.shop_type),
      npc_index(other.npc_index),
      only_buy(other.only_buy),
      pad_0d(),
      message(other.message),
      sell_items(other.sell_items),
      tab_names(other.tab_names),
      sell_limit_items(other.sell_limit_items),
      one_a_day_item_lists(other.one_a_day_item_lists),
      one_a_day_enabled(other.one_a_day_enabled),
      pad_45(),
      one_a_day_start_time(other.one_a_day_start_time),
      reserved_4c(other.reserved_4c) {}

// IDA_EA: 0x8374E76
// IDA_NAME: _ZN9CItemShopC2Ev
// DEMANGLED: CItemShop::CItemShop(void)
CItemShop::CItemShop() : scripts_(), one_a_day_selected_() {}

// IDA_EA: 0x8512AA0
// IDA_NAME: _ZN9CItemShop17loadItemShopFilesEv
// DEMANGLED: CItemShop::loadItemShopFiles(void)
int CItemShop::loadItemShopFiles() {
    ItemShopScript script;

    for (STScriptFileList::iterator it =
             g_itemShopScriptFileList.begin();
         it != g_itemShopScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (filename == 0) {
            break;
        }
        if (importItemShopScript(&script, filename) != 1) {
            LogManager::logFormat(
                1,
                "item.cpp",
                "bool CItemShop::loadItemShopFiles()",
                0x0C26,
                "ImportEquipmentScript() fail file - %s",
                filename);
            return 0;
        }

        script.script_index =
            g_itemShopScriptFileList.FindIndexByFullScan(filename);
        if (script.one_a_day_enabled) {
            setOneADayItemListNo(script);
        }

        scripts_.insert(std::make_pair(script.script_index, script));
    }

    return 1;
}

// IDA_EA: 0x8512CDA
// IDA_NAME: _ZN9CItemShop18checkValidShopItemEjm
// DEMANGLED: CItemShop::checkValidShopItem(unsigned int,unsigned long)
int CItemShop::checkValidShopItem(unsigned int shop_id,
                                  unsigned long item_id) {
    ScriptMap::iterator it = scripts_.find(static_cast<int>(shop_id));
    if (it == scripts_.end()) {
        return 0;
    }

    const std::vector<int>& items = it->second.sell_items;
    return std::find(items.begin(), items.end(), static_cast<int>(item_id)) !=
                   items.end()
               ? 1
               : 0;
}

// IDA_EA: 0x8512E98
// IDA_NAME: _ZN9CItemShop21updateOneADayItemListEv
// DEMANGLED: CItemShop::updateOneADayItemList(void)
int CItemShop::updateOneADayItemList() {
    one_a_day_selected_.clear();
    for (ScriptMap::iterator it = scripts_.begin(); it != scripts_.end();
         ++it) {
        if (!it->second.one_a_day_enabled) {
            continue;
        }

        const int list_no = setOneADayItemListNo(it->second);
        cMyTrace trace("void CItemShop::updateOneADayItemList()",
                               3177,
                               0);
        trace("updateOneADayItemList shop_id:%d list_no:%d",
              it->first,
              list_no);
    }
    return 0;
}

// IDA_EA: 0x8512F88
// IDA_NAME: _ZN9CItemShop25testupdateOneADayItemListEv
// DEMANGLED: CItemShop::testupdateOneADayItemList(void)
int CItemShop::testupdateOneADayItemList() {
    return updateOneADayItemList();
}

// IDA_EA: 0x8512F9C
// IDA_NAME: _ZN9CItemShop20setOneADayItemListNoER14ItemShopScript
// DEMANGLED: CItemShop::setOneADayItemListNo(ItemShopScript &)
int CItemShop::setOneADayItemListNo(ItemShopScript& script) {
    std::pair<int, int> selected(script.script_index, 0);
    CSystemTime* system_time = GlobalData::system_time();
    const int current_second = system_time != 0
                                   ? system_time->getCurSec()
                                   : static_cast<int>(::time(0));
    const int delta = current_second - script.one_a_day_start_time;
    if (delta > 0) {
        const int day_offset = delta / 86400;
        if (day_offset >= 0 &&
            day_offset < static_cast<int>(script.one_a_day_item_lists.size())) {
            selected.second = day_offset;
        }
    }

    one_a_day_selected_.push_back(selected);
    if (!script.one_a_day_item_lists.empty()) {
        script.sell_items =
            script.one_a_day_item_lists.at(selected.second);
    }
    return selected.second;
}

// IDA_EA: 0x8513076
// IDA_NAME: _ZNK9CItemShop18GetOneADayItemListEv
// DEMANGLED: CItemShop::GetOneADayItemList(void)const
const CItemShop::OneADaySelectionList& CItemShop::GetOneADayItemList() const {
    return one_a_day_selected_;
}

// IDA_EA: 0x8513082
// IDA_NAME: _ZNK9CItemShop13IsOneADayShopEi
// DEMANGLED: CItemShop::IsOneADayShop(int)const
int CItemShop::IsOneADayShop(int shop_id) const {
    ScriptMap::const_iterator it = scripts_.find(shop_id);
    if (it == scripts_.end()) {
        return 0;
    }
    return it->second.one_a_day_enabled ? 1 : 0;
}

// IDA_EA: 0x8513580
// IDA_NAME: _ZN9CItemShop24checkLimitNpcBuyItemListEjm
// DEMANGLED: CItemShop::checkLimitNpcBuyItemList(unsigned int,unsigned long)
int CItemShop::checkLimitNpcBuyItemList(unsigned int shop_id,
                                        unsigned long item_id) {
    ScriptMap::iterator it = scripts_.find(static_cast<int>(shop_id));
    if (it == scripts_.end()) {
        return 0;
    }

    const std::vector<int>& items = it->second.sell_limit_items;
    return std::find(items.begin(), items.end(), static_cast<int>(item_id)) !=
                   items.end()
               ? 1
               : 0;
}

// IDA_EA: 0x89DC4F8
// IDA_NAME: _Z18initItemShopScriptPcS_
// DEMANGLED: initItemShopScript(char *,char *)
int initItemShopScript(char* src, char* filename) {
    ::strncpy(g_itemShopScriptBaseDirectory, src, 0x100u);
    g_itemShopScriptBaseDirectory[0x100] = 0;
    const size_t length = ::strlen(g_itemShopScriptBaseDirectory);
    if (length != 0 &&
        g_itemShopScriptBaseDirectory[length - 1] != '\\' &&
        g_itemShopScriptBaseDirectory[length - 1] != '/') {
        ::memcpy(g_itemShopScriptBaseDirectory + length, "\\", 2u);
    }
    return loadRDARScriptListFile(&g_itemShopScriptFileList,
                                          g_itemShopScriptBaseDirectory,
                                          filename) == 1;
}

// IDA_EA: 0x89DC5B4
// IDA_NAME: _Z20importItemShopScriptP14ItemShopScriptPKc
// DEMANGLED: importItemShopScript(ItemShopScript *,char const*)
int importItemShopScript(ItemShopScript* script, const char* filename) {
    if (loadRDARScriptFile(g_itemShopScriptBaseDirectory, filename) !=
        1) {
        return 0;
    }

    script->clear();

    std::string tag;
    std::string type_text;
    bool success = false;
    int value = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[NPC]") {
            script->npc_index = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[message]") {
            ScanStr(&script->message);
        } else if (tag == "[type]") {
            ScanStr(&type_text);
            if (type_text == "[etc shop]") {
                script->shop_type = 0;
            } else if (type_text == "[weapon shop]") {
                script->shop_type = 1;
            } else if (type_text == "[disjoint shop]") {
                script->shop_type = 2;
            }
        } else if (tag == "[only buy]") {
            script->only_buy = ScanInt(static_cast<bool*>(0)) != 0;
        } else if (tag == "[sell item]") {
            script->sell_items.clear();
            while (1) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->sell_items.push_back(value);
            }
        } else if (tag == "[sell limit item]") {
            script->sell_limit_items.clear();
            while (1) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->sell_limit_items.push_back(value);
            }
        } else if (tag == "[tab name]") {
            std::string tab_name;
            while (ScanStr(&tab_name) == 1) {
                script->tab_names.push_back(tab_name);
            }
        } else if (tag == "[one a day start time]") {
            time_t timer = ::time(0);
            tm local_tm;
            tm* time_value = ::localtime_r(&timer, &local_tm);
            if (time_value == 0) {
                return 0;
            }

            value = ScanInt(&success);
            if (!success) {
                break;
            }
            time_value->tm_year = value - 1900;

            value = ScanInt(&success);
            if (!success) {
                break;
            }
            time_value->tm_mon = value - 1;

            value = ScanInt(&success);
            if (!success) {
                break;
            }
            time_value->tm_mday = value;

            value = ScanInt(&success);
            if (!success) {
                break;
            }
            time_value->tm_hour = value;
            time_value->tm_min = 0;
            time_value->tm_sec = 0;
            script->one_a_day_start_time =
                static_cast<int>(::mktime(time_value));
        } else if (tag == "[one a day item]") {
            std::vector<int> item_list;
            while (1) {
                value = ScanInt(&success);
                if (!success) {
                    break;
                }
                item_list.push_back(value);
            }
            script->one_a_day_item_lists.push_back(item_list);
        }
    }

    script->script_index =
        g_itemShopScriptFileList.FindIndexByFullScan(filename);
    if (!script->one_a_day_item_lists.empty()) {
        script->one_a_day_enabled = true;
    }
    return 1;
}
