#include "old_data/PVF/etc/crane_minigame_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/event/action_mail_script.h"

#include <string>
#include <utility>


typedef char CraneItemInfo_Size_Must_Be_0x14[
    (sizeof(CraneItemInfo) == 0x14) ? 1 : -1];
typedef char CraneMinigameScript_Size_Must_Be_0x68[
    (sizeof(CraneMinigameScript) == 0x68) ? 1 : -1];

// IDA_EA: 0x88E3708
// IDA_NAME: _ZN13CraneItemInfoaSERKS_
// DEMANGLED: CraneItemInfo::operator=(CraneItemInfo const&)
CraneItemInfo& CraneItemInfo::operator=(const CraneItemInfo& rhs) {
    // The original copy operation intentionally leaves the two rates and all
    // padding alone.  It is not a whole-record assignment.
    item_index = rhs.item_index;
    item_id = rhs.item_id;
    item_count = rhs.item_count;
    return *this;
}

// IDA_EA: 0x88E41F6
// IDA_NAME: _ZN13CraneItemInfoC2Ev
// DEMANGLED: CraneItemInfo::CraneItemInfo(void)
CraneItemInfo::CraneItemInfo() : item_index(-1), item_id(-1) {}

// IDA_EA: 0x88E3736
// IDA_NAME: _ZN19CraneMinigameScriptC2Ev
// DEMANGLED: CraneMinigameScript::CraneMinigameScript(void)
CraneMinigameScript::CraneMinigameScript()
    : items(), send_mail(), need_materials() {}

// IDA_EA: 0x88E37A8
// IDA_NAME: _ZN19CraneMinigameScriptD2Ev
// DEMANGLED: CraneMinigameScript::~CraneMinigameScript()
CraneMinigameScript::~CraneMinigameScript() {
    for (std::vector<CraneItemInfo*>::iterator it = items.begin();
         it != items.end(); ++it) {
        if (*it != 0) {
            delete *it;
        }
        *it = 0;
    }
    items.clear();
}

// IDA_EA: 0x88E38D6
// IDA_NAME: _ZN19CraneMinigameScript7getItemEs
// DEMANGLED: CraneMinigameScript::getItem(short)
CraneItemInfo* CraneMinigameScript::getItem(int16_t item_index) {
    if (static_cast<unsigned int>(item_index) >= items.size()) {
        return 0;
    }
    return items.at(item_index);
}

// IDA_EA: 0x88E3924
// IDA_NAME: _Z25importCraneMinigameScriptP19CraneMinigameScriptPKc
// DEMANGLED: importCraneMinigameScript(CraneMinigameScript *,char const*)
int importCraneMinigameScript(CraneMinigameScript* script,
                              const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    CraneItemInfo* current_item = 0;
    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[viewCnt]") {
            script->view_count =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[item]") {
            current_item = new CraneItemInfo();
            current_item->item_id = ScanInt(static_cast<bool*>(0));
            current_item->item_index =
                static_cast<int16_t>(script->items.size());
        } else if (tag == "[cnt]") {
            current_item->item_count =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[viewRatio]") {
            current_item->view_ratio = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[pickRatio]") {
            current_item->pick_ratio = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[/item]") {
            script->items.push_back(current_item);
        } else if (tag == "[material]") {
            script->material_item_id = ScanInt(static_cast<bool*>(0));
            script->material_count =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[send mail]") {
            importActionMailScript(&script->send_mail);
        } else if (tag == "[need material]") {
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(static_cast<bool*>(0));
                script->need_materials.push_back(
                    std::make_pair(item_id, item_count));
            }
        }
    }

    // IDA's opened-script EOF path succeeds: this table has no terminator.
    return 1;
}
