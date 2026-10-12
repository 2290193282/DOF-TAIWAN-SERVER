#include "old_data/PVF/itemshop/datamanager/quest_shop_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string.h>

#include <string>
#include <utility>

typedef char stQuestShop_Size_Must_Be_0x39c[
    (sizeof(stQuestShop) == 0x39c) ? 1 : -1];

// IDA_EA: 0x83710D4
// IDA_NAME: _ZN11stQuestShopC2Ev
// DEMANGLED: stQuestShop::stQuestShop(void)
stQuestShop::stQuestShop() : statuses(), init_cost() {
    statuses.clear();
    max_status_id = -1;
    memset(appendage_status_map, 0, sizeof(appendage_status_map));
}

// IDA_EA: 0x837D07A
// IDA_NAME: _ZN11stQuestShopD2Ev
// DEMANGLED: stQuestShop::~stQuestShop()
stQuestShop::~stQuestShop() {}

// IDA_EA: 0x890FA3D
// IDA_NAME: _Z21importQuestShopScriptPKcR11stQuestShop
// DEMANGLED: importQuestShopScript(char const*,stQuestShop &)
int importQuestShopScript(const char* filename, stQuestShop& quest_shop) {
    if (!loadRDARScriptFile("", filename)) {
        return 10261;
    }

    std::string tag;
    bool success = false;
    std::string text;
    stQuestShopStatus status;
    while (ScanType(tag, true) == 1) {
        if (tag == "[quest status]") {
            status.clear();
            const int status_id = ScanInt(&success);
            if (!success) {
                return 10281;
            }
            if (quest_shop.max_status_id < status_id) {
                quest_shop.max_status_id = status_id;
            }

            while (ScanType(tag, true) == 1 &&
                   tag != "[/quest status]") {
                if (tag == "[name]") {
                    ScanStr(&status.name);
                } else if (tag == "[name2]") {
                    ScanStr(&status.name2);
                } else if (tag == "[explain]") {
                    ScanStr(&status.explain);
                } else if (tag == "[icon]") {
                    ScanStr(&status.icon);
                    status.icon_frame = ScanInt(static_cast<bool*>(0));
                } else if (tag == "[quest status info]") {
                    ScanStr(&text);
                    status.appendage_type = loadAppendageType(text);
                    if (status.appendage_type == -1) {
                        return 10319;
                    }
                    ScanStr(&text);
                    status.value_type = loadValueType(text);
                    status.value = ScanFloat(static_cast<bool*>(0));
                    quest_shop.appendage_status_map[status.appendage_type] =
                        static_cast<uint8_t>(status_id);
                } else if (tag == "[level info]") {
                    while (1) {
                        stStatusLevelInfo level_info;
                        level_info.level =
                            static_cast<uint8_t>(ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        level_info.count =
                            static_cast<uint8_t>(ScanInt(&success));
                        if (!success) {
                            break;
                        }
                        status.level_infos.push_back(level_info);
                    }
                } else if (tag == "[quest point]") {
                    const int quest_point = ScanInt(&success);
                    if (!success) {
                        return 10349;
                    }
                    status.quest_point =
                        static_cast<uint8_t>(quest_point);
                }
            }

            quest_shop.statuses.insert(std::make_pair(status_id, status));
        } else if (tag == "[initialization costs]") {
            stQuestShoptInitCost init_cost;
            init_cost.unknown_00 = ScanInt(&success);
            if (!success) {
                break;
            }
            init_cost.unknown_04 = ScanInt(&success);
            if (!success) {
                break;
            }
            init_cost.unknown_08 = ScanInt(&success);
            if (!success) {
                break;
            }
            quest_shop.init_cost = init_cost;
        }
    }
    return 0;
}
