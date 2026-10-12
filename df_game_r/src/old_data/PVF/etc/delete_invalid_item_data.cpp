#include "old_data/PVF/etc/delete_invalid_item_data.h"

#include "old_data/PVF/common/rdar_script.h"


typedef char DeleteInvalidItemScript_Size_Must_Be_0x20[
    (sizeof(DeleteInvalidItemScript) == 0x20) ? 1 : -1];
typedef char DeleteInvalidItemData_Size_Must_Be_0x0c[
    (sizeof(DeleteInvalidItemScript::DeleteInvalidItemData) == 0x0c) ? 1 : -1];

// IDA_EA: 0x8AC1D40
// IDA_NAME: _ZN23DeleteInvalidItemScriptC2Ev
// DEMANGLED: DeleteInvalidItemScript::DeleteInvalidItemScript(void)
DeleteInvalidItemScript::DeleteInvalidItemScript()
    : mail_title(), mail_content(), invalid_items() {
    clearScript();
}

// IDA_EA: 0x8AC1DCE
// IDA_NAME: _ZN23DeleteInvalidItemScriptD2Ev
// DEMANGLED: DeleteInvalidItemScript::~DeleteInvalidItemScript()
DeleteInvalidItemScript::~DeleteInvalidItemScript() {
    clearScript();
}

// IDA_EA: 0x8AC1E60
// IDA_NAME: _ZN23DeleteInvalidItemScript10loadScriptEv
// DEMANGLED: DeleteInvalidItemScript::loadScript(void)
int DeleteInvalidItemScript::loadScript() {
    if (!loadRDARScriptFile("Etc/", "tw_DeleteInvalidItem.etc")) {
        return 0;
    }

    std::string tag;
    bool success = false;
    DeleteInvalidItemData data;

    while (ScanType(tag, true) == 1) {
        if (tag == "[delete item]") {
            while (1) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int reward_item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int reward_count = ScanInt(&success);
                if (!success) {
                    break;
                }

                data.init();
                data.value_00 = item_id;
                data.value_04 = reward_item_id;
                data.value_08 = reward_count;
                invalid_items.insert(std::make_pair(item_id, data));
            }
        } else if (tag == "[send mail title]") {
            success = ScanStr(&mail_title);
            if (!success) {
                break;
            }
        } else if (tag == "[send mail content]") {
            success = ScanStr(&mail_content);
            if (!success) {
                break;
            }
        }
    }

    unloadRDARScriptFile();
    return 1;
}

// IDA_EA: 0x8AC2066
// IDA_NAME: _ZN23DeleteInvalidItemScript11clearScriptEv
// DEMANGLED: DeleteInvalidItemScript::clearScript(void)
int DeleteInvalidItemScript::clearScript() {
    mail_title.clear();
    mail_content.clear();
    invalid_items.clear();
    return 0;
}

// IDA_EA: 0x8AC2096
// IDA_NAME: _ZN23DeleteInvalidItemScript20IsInvalidItem2DeleteEi
// DEMANGLED: DeleteInvalidItemScript::IsInvalidItem2Delete(int)
int DeleteInvalidItemScript::IsInvalidItem2Delete(int32_t item_id) {
    return invalid_items.find(item_id) != invalid_items.end();
}

// IDA_EA: 0x8AC20F8
// IDA_NAME: _ZN23DeleteInvalidItemScript17GetRewardItemInfoEiRiS0_
// DEMANGLED: DeleteInvalidItemScript::GetRewardItemInfo(int,int &,int &)
int DeleteInvalidItemScript::GetRewardItemInfo(int32_t item_id,
                                               int32_t& reward_item_id,
                                               int32_t& reward_count) {
    std::map<int, DeleteInvalidItemData>::iterator found =
        invalid_items.find(item_id);
    if (found == invalid_items.end()) {
        return 0;
    }

    reward_item_id = found->second.value_04;
    reward_count = found->second.value_08;
    return 1;
}

// IDA_EA: 0x8AC2180
// IDA_NAME: _ZNK23DeleteInvalidItemScript11GetMailInfoERSsS0_
// DEMANGLED: DeleteInvalidItemScript::GetMailInfo(std::string &,std::string &)const
std::string* DeleteInvalidItemScript::GetMailInfo(
    std::string& title,
    std::string& content) const {
    title = mail_title;
    content = mail_content;
    return &content;
}

// IDA_EA: 0x8AC220C
// IDA_NAME: _ZN23DeleteInvalidItemScript21DeleteInvalidItemDataC2Ev
// DEMANGLED: DeleteInvalidItemScript::DeleteInvalidItemData::DeleteInvalidItemData(void)
DeleteInvalidItemScript::DeleteInvalidItemData::DeleteInvalidItemData() {
    init();
}

// IDA_EA: 0x8AC2220
// IDA_NAME: _ZN23DeleteInvalidItemScript21DeleteInvalidItemData4initEv
// DEMANGLED: DeleteInvalidItemScript::DeleteInvalidItemData::init(void)
DeleteInvalidItemScript::DeleteInvalidItemData*
DeleteInvalidItemScript::DeleteInvalidItemData::init() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    return this;
}
