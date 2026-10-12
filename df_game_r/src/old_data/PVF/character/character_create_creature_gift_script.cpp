#include "old_data/PVF/character/character_create_creature_gift_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"

#include <string>
#include <utility>


// IDA_EA: 0x89625D0
// IDA_NAME: _ZN33CharacterCreateCreatureGiftScript8eventJobC2Ev
// DEMANGLED: CharacterCreateCreatureGiftScript::eventJob::eventJob(void)
CharacterCreateCreatureGiftScript::eventJob::eventJob()
    : start(0),
      end(0) {}

// IDA_EA: 0x83730EE
// IDA_NAME: _ZN33CharacterCreateCreatureGiftScriptC2Ev
// DEMANGLED: CharacterCreateCreatureGiftScript::CharacterCreateCreatureGiftScript(void)
CharacterCreateCreatureGiftScript::CharacterCreateCreatureGiftScript()
    : event_jobs_by_job(),
      change_able_creature_level(0),
      change_infinity_creatures(),
      client_infinity_creatures() {
    event_jobs_by_job.clear();
    change_infinity_creatures.clear();
    client_infinity_creatures.clear();
}

// IDA_EA: 0x837C3FC
// IDA_NAME: _ZN33CharacterCreateCreatureGiftScriptD2Ev
// DEMANGLED: CharacterCreateCreatureGiftScript::~CharacterCreateCreatureGiftScript()
CharacterCreateCreatureGiftScript::~CharacterCreateCreatureGiftScript() {}

// IDA_EA: 0x896001A
// IDA_NAME: _Z39importCharacterCreateCreatureGiftScriptP33CharacterCreateCreatureGiftScriptPKc
// DEMANGLED: importCharacterCreateCreatureGiftScript(CharacterCreateCreatureGiftScript *,char const*)
int importCharacterCreateCreatureGiftScript(
    CharacterCreateCreatureGiftScript* script,
    const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }
    if (!script) {
        return 0;
    }

    std::string tag;
    bool success = false;
    int job = -1;
    CharacterCreateCreatureGiftScript::eventJob event_job;

    while (ScanType(tag, true) == 1) {
        if (tag == "[event job]") {
            std::string job_text;
            ScanStr(&job_text);
            job = getCharacterJobToEnumValue(job_text);
        } else if (tag == "[start]") {
            ScanInt(&event_job.start);
        } else if (tag == "[end]") {
            ScanInt(&event_job.end);
        } else if (tag == "[/event job]") {
            script->event_jobs_by_job.insert(std::make_pair(job, event_job));
            event_job.start = -1;
            event_job.end = -1;
            job = -1;
        } else if (tag == "[change able creature lev]") {
            ScanInt(&script->change_able_creature_level);
        } else if (tag == "[change infinity creature]") {
            while (1) {
                int creature_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                int change_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                script->change_infinity_creatures.insert(
                    std::make_pair(creature_id, change_id));
            }
        } else if (tag == "[client infinity creature]") {
            while (1) {
                int creature_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                int client_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                script->client_infinity_creatures.insert(
                    std::make_pair(creature_id, client_id));
            }
        }
    }

    return 1;
}
