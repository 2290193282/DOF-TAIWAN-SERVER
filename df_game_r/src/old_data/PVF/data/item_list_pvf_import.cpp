#include "old_game/economy/inventory/item_list.h"

#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/stackable/arad_gift_item_set.h"
#include "old_data/PVF/stackable/disguise_stackable_registry.h"
#include "old_data/PVF/common/script_file_list.h"
#include "old_data/PVF/data/sync_script.h"
#include "old_data/PVF/stackable/stackable_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/global_data.h"
#include "old_game/dungeon/instance/compound_extream_dun_item_runtime.h"

#include <stdio.h>
#include <utility>
#include <vector>

extern STScriptFileList g_stEquipmentScriptFileList;

namespace {

const char kLoadItemFilesPrettyFunction[] =
    "bool CItemList::load_item_files()";

bool ImportEquipmentPVF(CItemList* item_list,
                        sync_script::CSyncScript* sync_script) {
    if (!GlobalData::s_pCompoundBindCube) {
        LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
            __LINE__, "[BIND_CUBE][LOAD_FAIL] missing candidate-table owner");
        return false;
    }
    unsigned int compound_candidates = 0;
    for (STScriptFileList::const_iterator it =
             g_stEquipmentScriptFileList.begin();
         it != g_stEquipmentScriptFileList.end();
         ++it) {
        const char* const filename = it->second.c_str();
        STEquipmentScript script;
        if (ImportEquipmentScript(&script, filename, false, 0, it->first) !=
            1) {
            LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
                                  0x8E8,
                                  "ImportEquipmentScript() fail file - %s",
                                  filename);
            return false;
        }
        if (sync_script->insert_equiptment_item_to_db(&script, 0, 0, 0) != 1 &&
            script.item.value_24 != 22 && script.item.value_24 != 23 &&
            script.item.value_24 != 24 && script.item.value_24 != 25) {
            ::printf("insert_equiptment_item_to_db() fail file - %s", filename);
        }

        CEquipItem* const item = new CEquipItem();
        item->set_item(script);
        if (!item->verify()) {
            LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
                                  0x90A,
                                  "Equip item verify fail. [%s]", filename);
            return false;
        }
        item_list->insert_item(item->get_grade(), item->get_index(), item);
        // Original CItemList::load_item_files registers every
        // verified equipment here; CItemList::init sorts the completed table.
        if (GlobalData::s_pCompoundBindCube->check_N_addItem(*item)) {
            ++compound_candidates;
        }
    }
    LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
        __LINE__, "[BIND_CUBE][CANDIDATES_LOADED] count=%u", compound_candidates);
    return true;
}

bool HasValidStackableMaterialVectors(const CStackableItem& item) {
    const std::vector<std::pair<int, int> >& pairs =
        item.lottery_info_.pair_values;
    const std::vector<int>& values = item.lottery_info_.int_values;
    return pairs.size() == values.size();
}

bool ImportStackablePVF(CItemList* item_list,
                        sync_script::CSyncScript* sync_script) {
    disguise_pvf::ResetDisguiseStackableRegistry();
    for (STScriptFileList::const_iterator it =
             g_stStackableScriptFileList.begin();
         it != g_stStackableScriptFileList.end();
         ++it) {
        const char* const filename = it->second.c_str();
        STStackableScript script;
        if (ImportStackableScript(&script, filename, false, it->first) != 1) {
            LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
                                  0x926,
                                  "importStackableScript fail file - %s",
                                  filename);
            return false;
        }
        if (GlobalInstance<Arad_GiftItem_Set>::inst()->open(&script) != 0) {
            LogManager::logFormat(
                1, "item.cpp", kLoadItemFilesPrettyFunction, 0x931,
                "[ERROR] Arad GiftItem Setting!! [item_idx:%d][levelt:%d]",
                script.item.index, script.item.values_5c[0]);
            return false;
        }
        if (sync_script->insert_stackable_item_to_db(&script) != 1) {
            ::printf("insert_stackable_item_to_db() fail file - %s", filename);
        }

        CStackableItem* const item = new CStackableItem();
        item->set_item(script);
        if (!item->verify()) {
            LogManager::logFormat(1, "item.cpp", kLoadItemFilesPrettyFunction,
                                  0x948,
                                  "Stackable item verify fail. [%s]", filename);
            return false;
        }
        if (!disguise_pvf::RegisterDisguiseStackable(script)) {
            LogManager::logFormat(
                1, "item.cpp", kLoadItemFilesPrettyFunction, 0x949,
                "Invalid [disguise] stackable. [item:%d][file:%s]",
                script.item.index, filename);
            return false;
        }
        const int item_type = item->GetItemType();
        if ((item_type == 7 || item_type == 13) &&
            !HasValidStackableMaterialVectors(*item)) {
            if (item_type == 7) {
                cMyTrace trace(kLoadItemFilesPrettyFunction, 2383, 5);
                trace("Stackable item set_item fail (file [%s])", filename);
            } else {
                cMyTrace trace(kLoadItemFilesPrettyFunction, 2391, 5);
                trace("Stackable item set_item fail 2 (file [%s])", filename);
            }
            return false;
        }
        item_list->insert_item(item->get_grade(), item->get_index(), item);
    }
    return true;
}

}  // namespace

// IDA_EA: 0x8511738
// IDA_NAME: _ZN9CItemList15load_item_filesEv
// DEMANGLED: CItemList::load_item_files(void)
int CItemList::load_item_files() {
    sync_script::CSyncScript* const sync_script =
        sync_script::GetInstanceSyncScript();
    sync_script->truncate_dnf_item_info_table();
    if (!ImportEquipmentPVF(this, sync_script) ||
        !ImportStackablePVF(this, sync_script)) {
        return 0;
    }
    sync_script::DisplayMaxLength();
    return 1;
}
