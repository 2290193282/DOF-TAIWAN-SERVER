#ifndef OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_INFO_H_
#define OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_INFO_H_

#include <map>
#include <utility>
#include <vector>

#include <stdint.h>


// IDA unit: 04_character_select_create/STEventCharacterItem.
struct STEventCharacterItem {
    ~STEventCharacterItem();

    int item_index;
    int count;
    int upgrade;
};

// IDA unit: 04_character_select_create/STEventCharacterInfo.
struct STEventCharacterInfo {
    STEventCharacterInfo();
    STEventCharacterInfo(const STEventCharacterInfo& other);
    ~STEventCharacterInfo();

    STEventCharacterInfo& operator=(const STEventCharacterInfo& other);

    uint8_t grow_type;
    uint8_t awakening_type;
    uint8_t pad_02[2];
    std::vector<std::pair<int, int> > skills;
    std::vector<STEventCharacterItem> items;
};

// IDA unit: 04_character_select_create/STEventCharacterInfo_level.
struct STEventCharacterInfo_level {
    STEventCharacterInfo_level();
    STEventCharacterInfo_level(const STEventCharacterInfo_level& other);
    ~STEventCharacterInfo_level();

    std::map<int, STEventCharacterInfo> infos_by_job;
};


#endif  // OLDSRV_CHARACTER_SELECT_EVENT_CHARACTER_INFO_H_
