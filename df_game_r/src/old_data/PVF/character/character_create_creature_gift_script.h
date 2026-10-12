#ifndef OLDSRV_DATAMANAGER_CHARACTER_CREATE_CREATURE_GIFT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_CHARACTER_CREATE_CREATURE_GIFT_SCRIPT_H_

// Canonical B-owned PVF character-create creature-gift declarations.

#include <map>


// IDA_EA: 0x89625D0, 0x83730EE, 0x837C3FC, 0x896001A.
// IDA unit: 12_pvf_datamanager_scripts/CharacterCreateCreatureGiftScript.
class CharacterCreateCreatureGiftScript {
public:
    struct eventJob {
        eventJob();

        int start;
        int end;
    };

    CharacterCreateCreatureGiftScript();
    ~CharacterCreateCreatureGiftScript();

    std::map<int, eventJob> event_jobs_by_job;
    int change_able_creature_level;
    std::map<int, int> change_infinity_creatures;
    std::map<int, int> client_infinity_creatures;

private:
    CharacterCreateCreatureGiftScript(
        const CharacterCreateCreatureGiftScript&);
    CharacterCreateCreatureGiftScript& operator=(
        const CharacterCreateCreatureGiftScript&);
};

int importCharacterCreateCreatureGiftScript(
    CharacterCreateCreatureGiftScript* script,
    const char* filename);

static_assert(sizeof(CharacterCreateCreatureGiftScript::eventJob) == 0x08u,
              "CharacterCreateCreatureGiftScript::eventJob IDA ABI size must stay 0x08");
static_assert(sizeof(CharacterCreateCreatureGiftScript) == 0x4Cu,
              "CharacterCreateCreatureGiftScript IDA ABI size must stay 0x4c");

#endif  // OLDSRV_DATAMANAGER_CHARACTER_CREATE_CREATURE_GIFT_SCRIPT_H_
