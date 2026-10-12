#include "old_data/PVF/data/levelup_support_item_info.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "common/manager/global_tools.h"

#include <cstdio>
#include <new>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>


typedef char LevelupSupport2ndScript_Size_Must_Be_0x24[
    (sizeof(LevelupSupport2ndScript) == 0x24) ? 1 : -1];

// IDA_EA: 0x8AB54BE
// IDA_NAME: _ZN23LevelupSupport2ndScriptC2Ev
// DEMANGLED: LevelupSupport2ndScript::LevelupSupport2ndScript(void)
LevelupSupport2ndScript::LevelupSupport2ndScript()
    : reward_items_by_level(),
      mail_title(),
      mail_content() {}

// IDA_EA: 0x8AB5530
// IDA_NAME: _ZN23LevelupSupport2ndScriptD2Ev
// DEMANGLED: LevelupSupport2ndScript::~LevelupSupport2ndScript()
LevelupSupport2ndScript::~LevelupSupport2ndScript() {}

// IDA_EA: 0x8AB55A4
// IDA_NAME: _ZN23LevelupSupport2ndScript10loadScriptEv
// DEMANGLED: LevelupSupport2ndScript::loadScript(void)
int LevelupSupport2ndScript::loadScript() {
    if (!loadRDARScriptFile("Event/", "tw_LevelupSupport2nd.evt")) {
        return -1;
    }

    std::string token;
    bool is_type = false;
    bool success = false;
    int level = 0;
    std::vector<ItemInfo> item_info;

    while (ScanType(token, true) == 1) {
        if (token == "[job type]") {
            std::string job_type_text;
            ScanStr(&job_type_text);
            job_type = ::getCharacterJobToEnumValue(job_type_text);
        } else if (token == "[step]") {
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[level]") {
                level = ScanInt(static_cast<bool*>(0));
            }
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[reward]") {
                while (1) {
                    const int item_index =
                        ScanTypeOrInt(is_type, token, success, true);
                    if (is_type) {
                        break;
                    }
                    const int item_count = ScanInt(static_cast<bool*>(0));
                    ItemInfo info(item_index, static_cast<int16_t>(item_count));
                    item_info.push_back(info);
                }
            }
            if (ScanType(token, true) != 1) {
                break;
            }
            if (token == "[/step]") {
                if (reward_items_by_level.find(level) !=
                    reward_items_by_level.end()) {
                    std::printf("duplicate [level] %d", level);
                    return -1;
                }
                std::vector<ItemInfo>* stored =
                    new std::vector<ItemInfo>();
                stored->swap(item_info);
                reward_items_by_level.insert(
                    std::make_pair(level, stored));
            }
        } else if (token == "[mail title]") {
            ScanStr(&mail_title);
        } else if (token == "[mail content]") {
            ScanStr(&mail_content);
        }
    }

    unloadRDARScriptFile();
    return 0;
}

// IDA_EA: 0x8AB5964
// IDA_NAME: _ZN23LevelupSupport2ndScript11clearScriptEv
// DEMANGLED: LevelupSupport2ndScript::clearScript(void)
int LevelupSupport2ndScript::clearScript() {
    std::map<int, std::vector<ItemInfo>* >::iterator it =
        reward_items_by_level.begin();
    std::map<int, std::vector<ItemInfo>* >::iterator end =
        reward_items_by_level.end();

    // IDA disasm 0x8AB5964 has no iterator increment before the loop test.
    while (it != end) {
        std::vector<ItemInfo>* items = it->second;
        if (items != 0) {
            items->~vector();
            operator delete(items);
        }
    }
    reward_items_by_level.clear();
    return static_cast<int>(
        reinterpret_cast<intptr_t>(&reward_items_by_level));
}
