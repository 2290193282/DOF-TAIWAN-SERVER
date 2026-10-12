#include "old_game/activity/schedule/event_character_parameter_script.h"

#include "common/config.h"


typedef char EventCharacterParameterScript_Size_Must_Be_0x18[
    (sizeof(EventCharacterParameterScript) == 0x18) ? 1 : -1];

// IDA_EA: 0x8371FB6
// IDA_NAME: _ZN29EventCharacterParameterScript5clearEv
// DEMANGLED: EventCharacterParameterScript::clear(void)
void EventCharacterParameterScript::clear() {
    infos_by_level.clear();
}

// IDA_EA: 0x8371FDE
// IDA_NAME: _ZN29EventCharacterParameterScriptC2Ev
// DEMANGLED: EventCharacterParameterScript::EventCharacterParameterScript(void)
EventCharacterParameterScript::EventCharacterParameterScript()
    : infos_by_level() {
    clear();
}

// IDA_EA: 0x837DA98
// IDA_NAME: _ZN29EventCharacterParameterScriptD2Ev
// DEMANGLED: EventCharacterParameterScript::~EventCharacterParameterScript()
EventCharacterParameterScript::~EventCharacterParameterScript() {}

// IDA_EA: 0x895B722
// IDA_NAME: _ZNK29EventCharacterParameterScript21GetEventCharacterInfoEiiR20STEventCharacterInfo
// DEMANGLED: EventCharacterParameterScript::GetEventCharacterInfo(int,int,STEventCharacterInfo &)const
int EventCharacterParameterScript::GetEventCharacterInfo(
    int job, int level, STEventCharacterInfo& out_info) const {
    if (level < oldsrv::config::kCharacterLevelMinimum ||
        level > oldsrv::config::kCharacterLevelMaximum) {
        return 0;
    }

    std::map<int, STEventCharacterInfo_level>::const_iterator level_it =
        infos_by_level.find(level);
    if (level_it == infos_by_level.end()) {
        return 0;
    }

    std::map<int, STEventCharacterInfo>::const_iterator job_it =
        level_it->second.infos_by_job.find(job);
    if (job_it == level_it->second.infos_by_job.end()) {
        return 0;
    }

    out_info = job_it->second;
    return 1;
}
