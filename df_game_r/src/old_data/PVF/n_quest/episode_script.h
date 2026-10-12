#ifndef OLDSRV_DATAMANAGER_EPISODE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_EPISODE_SCRIPT_H_

#include <stdint.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

class QuestScript;

// IDA_EA: 0x8A66028, 0x8A660BC, 0x8A66108, 0x8A66154, 0x8A6A830.
// IDA unit: 12_pvf_datamanager_scripts/EpisodeScript.
class EpisodeScript {
public:
    EpisodeScript();
    EpisodeScript(const EpisodeScript& other);
    ~EpisodeScript();

    int clear();
    EpisodeScript* setEpisodeIndex(int episode_group, int episode_id);

    int32_t episode_index;
    std::string episode_name;
    std::string complete_text;
    std::vector<std::pair<int, QuestScript*> > epic_quests;
};

// IDA_EA: 0x8A65AD0
// IDA_NAME: _Z21importEpicQuestScriptPSt4listI13EpisodeScriptSaIS0_EEPKc
// DEMANGLED: importEpicQuestScript(std::list<EpisodeScript,std::allocator<EpisodeScript>> *,char const*)
int importEpicQuestScript(std::list<EpisodeScript>* episodes,
                          const char* filename);

#endif  // OLDSRV_DATAMANAGER_EPISODE_SCRIPT_H_
