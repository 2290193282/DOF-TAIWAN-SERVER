#include "old_data/PVF/character/use_character_job_fatigue_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <string>
#include <utility>


// IDA_EA: 0x83CF7E0
// IDA_NAME: _ZN22useCharacterJobFatigue16fatigueMatchGiftD2Ev
// DEMANGLED: useCharacterJobFatigue::fatigueMatchGift::~fatigueMatchGift()
useCharacterJobFatigue::fatigueMatchGift::~fatigueMatchGift() {}

// IDA_EA: 0x896251C
// IDA_NAME: _ZN22useCharacterJobFatigue16fatigueMatchGiftC2Ev
// DEMANGLED: useCharacterJobFatigue::fatigueMatchGift::fatigueMatchGift(void)
useCharacterJobFatigue::fatigueMatchGift::fatigueMatchGift()
    : reward_items(),
      fatigue_match_count(0),
      fatigue_title(),
      fatigue_text() {
    reward_items.clear();
    fatigue_title.clear();
    fatigue_text.clear();
}

// IDA_EA: 0x8962C1E
// IDA_NAME: _ZN22useCharacterJobFatigue16fatigueMatchGiftC2ERKS0_
// DEMANGLED: useCharacterJobFatigue::fatigueMatchGift::fatigueMatchGift(useCharacterJobFatigue::fatigueMatchGift const&)
useCharacterJobFatigue::fatigueMatchGift::fatigueMatchGift(
    const fatigueMatchGift& other)
    : reward_items(other.reward_items),
      fatigue_match_count(other.fatigue_match_count),
      fatigue_title(other.fatigue_title),
      fatigue_text(other.fatigue_text) {}

// IDA_EA: 0x8373098
// IDA_NAME: _ZN22useCharacterJobFatigueC2Ev
// DEMANGLED: useCharacterJobFatigue::useCharacterJobFatigue(void)
useCharacterJobFatigue::useCharacterJobFatigue()
    : gifts_by_job() {
    gifts_by_job.clear();
}

// IDA_EA: 0x837C3E8
// IDA_NAME: _ZN22useCharacterJobFatigueD2Ev
// DEMANGLED: useCharacterJobFatigue::~useCharacterJobFatigue()
useCharacterJobFatigue::~useCharacterJobFatigue() {}

// IDA_EA: 0x895F817
// IDA_NAME: _Z29importUseJobFatigueGiftScriptP22useCharacterJobFatiguePKc
// DEMANGLED: importUseJobFatigueGiftScript(useCharacterJobFatigue *,char const*)
int importUseJobFatigueGiftScript(useCharacterJobFatigue* script,
                                  const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    bool result = false;
    std::string tag;
    useCharacterJobFatigue::fatigueMatchGift gift;
    int job = -1;

    while (ScanType(tag, true) == 1) {
        if (tag == "[/job fatigue]") {
            result = true;
            break;
        }

        if (tag == "[job type]") {
            std::string job_text;
            ScanStr(&job_text);
            job = getCharacterJobToEnumValue(job_text);
        } else if (tag == "[fatigue title]") {
            ScanStr(&gift.fatigue_title);
        } else if (tag == "[fatigue text]") {
            ScanStr(&gift.fatigue_text);
        } else if (tag == "[reward list]") {
            while (1) {
                bool success = false;
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                const int item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (item_count < 0) {
                    return 0;
                }

                const int fatigue_match_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                if (fatigue_match_count <= 0) {
                    return 0;
                }

                gift.fatigue_match_count = fatigue_match_count;
                gift.reward_items.insert(
                    std::make_pair(item_id, item_count));
            }
        } else if (tag == "[/job type]") {
            script->gifts_by_job.insert(std::make_pair(job, gift));
            gift.fatigue_match_count = 0;
            gift.reward_items.clear();
            job = -1;
        }
    }

    return result ? 1 : 0;
}
