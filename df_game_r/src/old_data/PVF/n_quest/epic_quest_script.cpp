#include "old_data/PVF/n_quest/episode_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>
#include <utility>

extern char g_questScriptBaseDirectory[256];

// IDA_EA: 0x8A65AD0
// IDA_NAME: _Z21importEpicQuestScriptPSt4listI13EpisodeScriptSaIS0_EEPKc
// DEMANGLED: importEpicQuestScript(std::list<EpisodeScript,std::allocator<EpisodeScript>> *,char const*)
int importEpicQuestScript(std::list<EpisodeScript>* episodes,
                          const char* filename) {
    if (episodes == 0) {
        return 0;
    }
    if (!loadRDARScriptFile(g_questScriptBaseDirectory, filename)) {
        return 0;
    }

    std::string tag;
    EpisodeScript episode;
    bool scanned = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[episode]") {
            episode.clear();
            const int episode_group = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            const int episode_id = ScanInt(&scanned);
            if (!scanned) {
                break;
            }
            episode.setEpisodeIndex(episode_group, episode_id);

            // The original accepts the partial episode after an unknown inner
            // tag, then resumes the outer tag scan.
            while (ScanType(tag, true) == 0 || tag != "[/episode]") {
                if (tag == "[episode_name]") {
                    ScanStr(&episode.episode_name);
                } else if (tag == "[epic_index]") {
                    for (;;) {
                        const int quest_index = ScanInt(&scanned);
                        if (!scanned) {
                            break;
                        }
                        episode.epic_quests.push_back(
                            std::make_pair(quest_index,
                                           static_cast<QuestScript*>(0)));
                    }
                } else if (tag == "[complete_text]") {
                    ScanStr(&episode.complete_text);
                } else {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptQuest.cpp",
                        "importEpicQuestScript",
                        0x687,
                        "Unknown Type (%s)",
                        tag.c_str());
                    break;
                }
            }
        }
        episodes->push_back(episode);
    }
    return 1;
}
