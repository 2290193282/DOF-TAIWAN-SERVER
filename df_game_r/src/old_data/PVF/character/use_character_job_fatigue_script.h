#ifndef OLDSRV_DATAMANAGER_USE_CHARACTER_JOB_FATIGUE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_USE_CHARACTER_JOB_FATIGUE_SCRIPT_H_

// Canonical B-owned PVF job-fatigue declarations.

#include <map>
#include <string>


// IDA_EA: 0x83CF7E0, 0x896251C, 0x8962C1E, 0x8373098, 0x837C3E8, 0x895F817.
class useCharacterJobFatigue {
public:
    struct fatigueMatchGift {
        fatigueMatchGift();
        fatigueMatchGift(const fatigueMatchGift& other);
        ~fatigueMatchGift();

        std::map<int, int> reward_items;
        int fatigue_match_count;
        std::string fatigue_title;
        std::string fatigue_text;
    };

    useCharacterJobFatigue();
    ~useCharacterJobFatigue();

    std::map<int, fatigueMatchGift> gifts_by_job;

private:
    useCharacterJobFatigue(const useCharacterJobFatigue&);
    useCharacterJobFatigue& operator=(const useCharacterJobFatigue&);
};

int importUseJobFatigueGiftScript(useCharacterJobFatigue* script,
                                  const char* filename);

static_assert(sizeof(useCharacterJobFatigue::fatigueMatchGift) == 0x24u,
              "useCharacterJobFatigue::fatigueMatchGift IDA ABI size must stay 0x24");
static_assert(sizeof(useCharacterJobFatigue) == 0x18u,
              "useCharacterJobFatigue IDA ABI size must stay 0x18");

#endif  // OLDSRV_DATAMANAGER_USE_CHARACTER_JOB_FATIGUE_SCRIPT_H_
