#include "old_game/dungeon/instance/condition_level_chk_dungeon.h"

// IDA_EA: 0x837C9AC
// IDA_NAME: _ZN24ConditionLevelChkDungeonC2Ev
// DEMANGLED: ConditionLevelChkDungeon::ConditionLevelChkDungeon(void)
ConditionLevelChkDungeon::ConditionLevelChkDungeon()
    : dungeon_indexes() {}

// IDA_EA: 0x837C9C0
// IDA_NAME: _ZN24ConditionLevelChkDungeonD2Ev
// DEMANGLED: ConditionLevelChkDungeon::~ConditionLevelChkDungeon()
ConditionLevelChkDungeon::~ConditionLevelChkDungeon() {}

// IDA_EA: 0x89139D4
// IDA_NAME: _ZN24ConditionLevelChkDungeon14isEventDungeonEi
// DEMANGLED: ConditionLevelChkDungeon::isEventDungeon(int)
int ConditionLevelChkDungeon::isEventDungeon(int index) {
    if (dungeon_indexes.empty()) {
        return 1;
    }
    for (std::vector<int>::const_iterator it = dungeon_indexes.begin();
         it != dungeon_indexes.end();
         ++it) {
        if (*it == index) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8913A62
// IDA_NAME: _ZN24ConditionLevelChkDungeon12isApplyLevelEi
// DEMANGLED: ConditionLevelChkDungeon::isApplyLevel(int)
int ConditionLevelChkDungeon::isApplyLevel(int charac_level) {
    return min_level <= charac_level;
}
