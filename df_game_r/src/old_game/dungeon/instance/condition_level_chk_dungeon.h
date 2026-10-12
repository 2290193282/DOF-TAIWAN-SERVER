#ifndef OLDSRV_OLD_GAME_DUNGEON_INSTANCE_CONDITION_LEVEL_CHK_DUNGEON_H_
#define OLDSRV_OLD_GAME_DUNGEON_INSTANCE_CONDITION_LEVEL_CHK_DUNGEON_H_

#include <vector>

// IDA unit: 09_dungeon_pvp_party/ConditionLevelChkDungeon.
struct ConditionLevelChkDungeon {
    // IDA_EA: 0x837C9AC
    ConditionLevelChkDungeon();
    // IDA_EA: 0x837C9C0
    ~ConditionLevelChkDungeon();
    // IDA_EA: 0x89139D4
    int isEventDungeon(int index);
    // IDA_EA: 0x8913A62
    int isApplyLevel(int charac_level);

    std::vector<int> dungeon_indexes;
    int min_level;
};

static_assert(sizeof(ConditionLevelChkDungeon) == 0x10u,
              "ConditionLevelChkDungeon IDA ABI size must stay 0x10");

#endif  // OLDSRV_OLD_GAME_DUNGEON_INSTANCE_CONDITION_LEVEL_CHK_DUNGEON_H_
