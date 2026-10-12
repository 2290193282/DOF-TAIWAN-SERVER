#include "old_data/PVF/data/datamanager/create_chrac_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <string>
#include <utility>


// IDA_EA: 0x83DD574
// IDA_NAME: _ZN17createChracScript10rewardInfoD2Ev
// DEMANGLED: createChracScript::rewardInfo::~rewardInfo()
createChracScript::rewardInfo::~rewardInfo() {}

// IDA_EA: 0x83ECCBC
// IDA_NAME: _ZN17createChracScript10rewardInfoC2ERKS0_
// DEMANGLED: createChracScript::rewardInfo::rewardInfo(createChracScript::rewardInfo const&)
createChracScript::rewardInfo::rewardInfo(const rewardInfo& other)
    : item_indexes(other.item_indexes),
      start_time(other.start_time),
      end_time(other.end_time),
      create_title(other.create_title),
      create_text(other.create_text) {}

// IDA_EA: 0x83EFC90
// IDA_NAME: _ZN17createChracScript10rewardInfoaSERKS0_
// DEMANGLED: createChracScript::rewardInfo::operator=(createChracScript::rewardInfo const&)
createChracScript::rewardInfo&
createChracScript::rewardInfo::operator=(const rewardInfo& other) {
    item_indexes = other.item_indexes;
    start_time = other.start_time;
    end_time = other.end_time;
    create_title = other.create_title;
    create_text = other.create_text;
    return *this;
}

// IDA_EA: 0x8962460
// IDA_NAME: _ZN17createChracScript10rewardInfoC2Ev
// DEMANGLED: createChracScript::rewardInfo::rewardInfo(void)
createChracScript::rewardInfo::rewardInfo()
    : item_indexes(),
      start_time(0),
      end_time(0),
      create_title(),
      create_text() {
    item_indexes.clear();
    create_title.clear();
    create_text.clear();
    start_time = 0;
    end_time = 0;
}

// IDA_EA: 0x83CF71C
// IDA_NAME: _ZN17createChracScript14rewardInfoListD2Ev
// DEMANGLED: createChracScript::rewardInfoList::~rewardInfoList()
createChracScript::rewardInfoList::~rewardInfoList() {}

// IDA_EA: 0x84E91FA
// IDA_NAME: _ZN17createChracScript14rewardInfoListC2Ev
// DEMANGLED: createChracScript::rewardInfoList::rewardInfoList(void)
createChracScript::rewardInfoList::rewardInfoList()
    : rewards() {
    rewards.clear();
}

// IDA_EA: 0x8962B4C
// IDA_NAME: _ZN17createChracScript14rewardInfoListC2ERKS0_
// DEMANGLED: createChracScript::rewardInfoList::rewardInfoList(createChracScript::rewardInfoList const&)
createChracScript::rewardInfoList::rewardInfoList(
    const rewardInfoList& other)
    : rewards(other.rewards) {}

// IDA_EA: 0x8373042
// IDA_NAME: _ZN17createChracScriptC2Ev
// DEMANGLED: createChracScript::createChracScript(void)
createChracScript::createChracScript()
    : rewards_by_job() {
    rewards_by_job.clear();
}

// IDA_EA: 0x837C3D4
// IDA_NAME: _ZN17createChracScriptD2Ev
// DEMANGLED: createChracScript::~createChracScript()
createChracScript::~createChracScript() {}

// IDA_EA: 0x895EDA1
// IDA_NAME: _Z31importCharacterCreateGiftScriptP17createChracScriptPKc
// DEMANGLED: importCharacterCreateGiftScript(createChracScript *,char const*)
int importCharacterCreateGiftScript(createChracScript* script,
                                    const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    unsigned char result = 0;
    std::string tag;
    createChracScript::rewardInfo reward_info;
    int job = -1;

    bool is_type = true;
    bool scan_success = false;
    while (1) {
        (void)ScanTypeOrInt(is_type, tag, scan_success, true);
        if (!scan_success) {
            break;
        }
        // Newer PVF revisions place the applicable job number directly in
        // each create-character-gift block.  The legacy reward map has no
        // payload for a bare job number, but the token must still be consumed.
        if (!is_type) {
            continue;
        }

        if (tag == "[/create character gift]") {
            result = 1;
            continue;
        }

        if (tag == "[apply job]") {
            std::string job_text;
            ScanStr(&job_text);
            job = getCharacterJobToEnumValue(job_text);
        } else if (tag == "[item]") {
            int item_index = 0;
            ScanInt(&item_index);
            reward_info.item_indexes.push_back(item_index);
        } else if (tag == "[new account event start time]") {
            std::string time_text;
            ScanStr(&time_text);
            if (!ConvertStringToTime(time_text, reward_info.start_time)) {
                result = 0;
                break;
            }
        } else if (tag == "[new account event end time]") {
            std::string time_text;
            ScanStr(&time_text);
            if (!ConvertStringToTime(time_text, reward_info.end_time)) {
                result = 0;
                break;
            }
        } else if (tag == "[create title]") {
            ScanStr(&reward_info.create_title);
        } else if (tag == "[create text]") {
            ScanStr(&reward_info.create_text);
        } else if (tag == "[/apply job]") {
            if (job != -1) {
                createChracScript::rewardMap::iterator it =
                    script->rewards_by_job.find(job);
                if (it == script->rewards_by_job.end()) {
                    createChracScript::rewardInfoList reward_list;
                    reward_list.rewards.push_back(reward_info);
                    script->rewards_by_job.insert(
                        std::make_pair(job, reward_list));
                } else {
                    it->second.rewards.push_back(reward_info);
                }
            }

            reward_info.item_indexes.clear();
            reward_info.create_title.clear();
            reward_info.create_text.clear();
            reward_info.start_time = 0;
            reward_info.end_time = 0;
            job = -1;
        }
    }

    return result;
}
