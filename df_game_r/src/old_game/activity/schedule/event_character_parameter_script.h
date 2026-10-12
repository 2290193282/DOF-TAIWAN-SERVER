#ifndef OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_PARAMETER_SCRIPT_H_
#define OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_PARAMETER_SCRIPT_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8371FB6 in the companion source.


#include <map>

#include "old_game/activity/schedule/event_character_info.h"


// IDA unit: 04_character_select_create/EventCharacterParameterScript.
struct EventCharacterParameterScript {
    EventCharacterParameterScript();
    ~EventCharacterParameterScript();

    void clear();
    int load_event_character_list();
    int importEventCharacterParameterScript(const char* filename);
    int GetEventCharacterInfo(int job, int level,
                              STEventCharacterInfo& out_info) const;

    std::map<int, STEventCharacterInfo_level> infos_by_level;
};


#endif  // OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_PARAMETER_SCRIPT_H_
