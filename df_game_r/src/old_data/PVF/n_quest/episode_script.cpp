#include "old_data/PVF/n_quest/episode_script.h"


typedef char EpisodeScript_Size_Must_Be_0x18[
    (sizeof(EpisodeScript) == 0x18) ? 1 : -1];

// IDA_EA: 0x8A66028
// IDA_NAME: _ZN13EpisodeScriptC2Ev
// DEMANGLED: EpisodeScript::EpisodeScript(void)
EpisodeScript::EpisodeScript()
    : episode_name(), complete_text(), epic_quests() {
    clear();
}

// IDA_EA: 0x8A660BC
// IDA_NAME: _ZN13EpisodeScript5clearEv
// DEMANGLED: EpisodeScript::clear(void)
int EpisodeScript::clear() {
    episode_index = -1;
    episode_name = "";
    complete_text = "";
    epic_quests.clear();
    return 0;
}

// IDA_EA: 0x8A66108
// IDA_NAME: _ZN13EpisodeScript15setEpisodeIndexEii
// DEMANGLED: EpisodeScript::setEpisodeIndex(int,int)
EpisodeScript* EpisodeScript::setEpisodeIndex(int episode_group,
                                              int episode_id) {
    episode_index = episode_id | (episode_group << 16);
    return this;
}

// IDA_EA: 0x8A66154
// IDA_NAME: _ZN13EpisodeScriptD2Ev
// DEMANGLED: EpisodeScript::~EpisodeScript()
EpisodeScript::~EpisodeScript() {}

// IDA_EA: 0x8A6A830
// IDA_NAME: _ZN13EpisodeScriptC2ERKS_
// DEMANGLED: EpisodeScript::EpisodeScript(EpisodeScript const&)
EpisodeScript::EpisodeScript(const EpisodeScript& other)
    : episode_index(other.episode_index),
      episode_name(other.episode_name),
      complete_text(other.complete_text),
      epic_quests(other.epic_quests) {}
