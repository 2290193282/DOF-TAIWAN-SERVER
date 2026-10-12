#include "old_data/PVF/itemshop/quest_shop_status.h"

#include <utility>

// IDA_EA: 0x891B8B4
// IDA_NAME: _ZN17stStatusLevelInfoC2Ev
// DEMANGLED: stStatusLevelInfo::stStatusLevelInfo(void)
stStatusLevelInfo::stStatusLevelInfo() : level(0), count(0) {}

// IDA_EA: 0x8940782
// IDA_NAME: _ZN17stStatusLevelInfoD2Ev
// DEMANGLED: stStatusLevelInfo::~stStatusLevelInfo()
stStatusLevelInfo::~stStatusLevelInfo() {}

// IDA_EA: 0x83CD826
// IDA_NAME: _ZN17stQuestShopStatusD2Ev
// DEMANGLED: stQuestShopStatus::~stQuestShopStatus()
stQuestShopStatus::~stQuestShopStatus() {}

// IDA_EA: 0x85F07FE
// IDA_NAME: _ZN17stQuestShopStatusC2ERKS_
// DEMANGLED: stQuestShopStatus::stQuestShopStatus(stQuestShopStatus const&)
stQuestShopStatus::stQuestShopStatus(const stQuestShopStatus& other)
    : name(other.name),
      name2(other.name2),
      explain(other.explain),
      icon(other.icon),
      icon_frame(other.icon_frame),
      quest_point(other.quest_point),
      pad_15(),
      appendage_type(other.appendage_type),
      value_type(other.value_type),
      value(other.value),
      level_infos(other.level_infos) {}

// IDA_EA: 0x890F8EC
// IDA_NAME: _ZN17stQuestShopStatus11getMaxCountEi
// DEMANGLED: stQuestShopStatus::getMaxCount(int)
int stQuestShopStatus::getMaxCount(int charac_level) {
    int max_count = 0;
    for (std::vector<stStatusLevelInfo>::iterator it = level_infos.begin();
         it != level_infos.end();
         ++it) {
        if (it->level <= charac_level) {
            max_count += it->count;
        }
    }
    return max_count;
}

// IDA_EA: 0x890F994
// IDA_NAME: _ZN17stQuestShopStatus12getNextLevelEi
// DEMANGLED: stQuestShopStatus::getNextLevel(int)
int stQuestShopStatus::getNextLevel(int count) {
    int max_count = 0;
    for (std::vector<stStatusLevelInfo>::iterator it = level_infos.begin();
         it != level_infos.end();
         ++it) {
        max_count += it->count;
        if (max_count > count) {
            return it->level;
        }
    }
    return -1;
}

// IDA_EA: 0x891B8CC
// IDA_NAME: _ZN17stQuestShopStatus5clearEv
// DEMANGLED: stQuestShopStatus::clear(void)
int stQuestShopStatus::clear() {
    name.clear();
    name2.clear();
    explain.clear();
    icon.clear();
    icon_frame = -1;
    quest_point = 0;
    appendage_type = 77;
    value_type = 2;
    value = 0.0f;
    level_infos.clear();
    return 0;
}

// IDA_EA: 0x891B948
// IDA_NAME: _ZN17stQuestShopStatusC2Ev
// DEMANGLED: stQuestShopStatus::stQuestShopStatus(void)
stQuestShopStatus::stQuestShopStatus()
    : name(),
      name2(),
      explain(),
      icon(),
      icon_frame(0),
      quest_point(0),
      pad_15(),
      appendage_type(0),
      value_type(0),
      value(0.0f),
      level_infos() {
    clear();
}

// IDA_EA: 0x889253B
// IDA_NAME: _Z17loadAppendageTypeRKSs
// DEMANGLED: loadAppendageType(std::string const&)
int loadAppendageType(const std::string& value) {
    if (value == "[physical attack]") {
        return 0;
    }
    if (value == "[magical attack]") {
        return 1;
    }
    if (value == "[physical defense]") {
        return 3;
    }
    if (value == "[magical defense]") {
        return 2;
    }
    if (value == "[equipment physical attack]") {
        return 4;
    }
    if (value == "[equipment physical defense]") {
        return 5;
    }
    if (value == "[jump power]") {
        return 8;
    }
    if (value == "[attack speed]") {
        return 10;
    }
    if (value == "[move speed]") {
        return 11;
    }
    if (value == "[cast speed]") {
        return 12;
    }
    if (value == "[hp max]") {
        return 13;
    }
    if (value == "[hp regen rate]") {
        return 14;
    }
    if (value == "[magical critical hit rate]") {
        return 16;
    }
    if (value == "[physical critical hit rate]") {
        return 15;
    }
    if (value == "[physical back attack critical rate]") {
        return 17;
    }
    if (value == "[magical back attack critical rate]") {
        return 18;
    }
    if (value == "[fire element]") {
        return 20;
    }
    if (value == "[water element]") {
        return 21;
    }
    if (value == "[dark element]") {
        return 22;
    }
    if (value == "[light element]") {
        return 23;
    }
    if (value == "[stuck]") {
        return 27;
    }
    if (value == "[mp regen rate]") {
        return 29;
    }
    if (value == "[hit recovery]") {
        return 30;
    }
    if (value == "[equipment magical attack]") {
        return 31;
    }
    if (value == "[equipment magical defense]") {
        return 32;
    }
    if (value == "[good hit]") {
        return 33;
    }
    if (value == "[all active status resistance]") {
        return 26;
    }
    if (value == "[all element]") {
        return 24;
    }
    if (value == "[fire element attack]") {
        return 42;
    }
    if (value == "[water element attack]") {
        return 43;
    }
    if (value == "[dark element attack]") {
        return 44;
    }
    if (value == "[light element attack]") {
        return 45;
    }
    if (value == "[all element attack]") {
        return 46;
    }
    if (value == "[mp max]") {
        return 48;
    }
    if (value == "[separate attack]") {
        return 53;
    }
    if (value == "[antievil]") {
        return 55;
    }
    if (value == "[physical absolute damage]") {
        return 56;
    }
    if (value == "[village moving speed]") {
        return 63;
    }
    if (value == "[magical absolute damage]") {
        return 60;
    }
    if (value == "[physical absolute defense]") {
        return 61;
    }
    if (value == "[magical absolute defense]") {
        return 62;
    }
    if (value == "[physical attack bonus]") {
        return 6;
    }
    if (value == "[magical attack bonus]") {
        return 7;
    }
    return -1;
}

// IDA_EA: 0x8892AC4
// IDA_NAME: _Z13loadValueTypeRKSs
// DEMANGLED: loadValueType(std::string const&)
int loadValueType(const std::string& value) {
    if (value == "+") {
        return 0;
    }
    if (value == "%") {
        return 1;
    }
    return 2;
}
