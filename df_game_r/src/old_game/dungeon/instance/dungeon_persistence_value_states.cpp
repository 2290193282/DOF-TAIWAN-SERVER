#include "old_game/pvp/battlefield/battle_field.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x84EC5D8
// IDA_NAME: _ZN18CDungeonGainedGold13setGainedGoldEj
// DEMANGLED: CDungeonGainedGold::setGainedGold(unsigned int)
CDungeonGainedGold* CDungeonGainedGold::setGainedGold(unsigned int gold) {
    gained_gold = gold;
    return this;
}

// IDA_EA: 0x850D4B6
// IDA_NAME: _ZN18CDungeonGainedGold13incGainedGoldEj
// DEMANGLED: CDungeonGainedGold::incGainedGold(unsigned int)
unsigned int CDungeonGainedGold::incGainedGold(unsigned int gold) {
    const unsigned int crossed =
        (gold + gained_gold) / 0x186A0u - gained_gold / 0x186A0u;
    gained_gold += gold;
    if (crossed) {
        return crossed;
    }
    return 0;
}

// IDA_EA: 0x8697044
// IDA_NAME: _ZN18CDungeonGainedGold13getGainedGoldEv
// DEMANGLED: CDungeonGainedGold::getGainedGold(void)
int CDungeonGainedGold::getGainedGold() {
    return gained_gold;
}

// IDA_EA: 0x869704E
// IDA_NAME: _ZN18CDungeonGainedGold5resetEv
// DEMANGLED: CDungeonGainedGold::reset(void)
CDungeonGainedGold* CDungeonGainedGold::reset() {
    gained_gold = 0;
    return this;
}

// IDA_EA: 0x869705C
// IDA_NAME: _ZN18CDungeonGainedGoldC2Ev
// DEMANGLED: CDungeonGainedGold::CDungeonGainedGold(void)
CDungeonGainedGold::CDungeonGainedGold() {}

// IDA_EA: 0x8697062
// IDA_NAME: _ZN18CDungeonGainedGoldD2Ev
// DEMANGLED: CDungeonGainedGold::~CDungeonGainedGold()
CDungeonGainedGold::~CDungeonGainedGold() {}

// IDA_EA: 0x84B766A
// IDA_NAME: _ZN20DungeonUserShotCountC2Ev
// DEMANGLED: DungeonUserShotCount::DungeonUserShotCount(void)
DungeonUserShotCount::DungeonUserShotCount() {
    reset();
}

// IDA_EA: 0x84B767E
// IDA_NAME: _ZN20DungeonUserShotCount5resetEv
// DEMANGLED: DungeonUserShotCount::reset(void)
DungeonUserShotCount* DungeonUserShotCount::reset() {
    shot_count = 0;
    raw_04 = 0;
    return this;
}

// IDA_EA: 0x84B73D4
// IDA_NAME: _ZN14GameResultTypeC2Ev
// DEMANGLED: GameResultType::GameResultType(void)
GameResultType::GameResultType() {
    Clear();
}

// IDA_EA: 0x84B7416
// IDA_NAME: _ZN14GameResultTypeD2Ev
// DEMANGLED: GameResultType::~GameResultType()
GameResultType::~GameResultType() {}

// IDA_EA: 0x84B741C
// IDA_NAME: _ZN14GameResultType3SetERKS_
// DEMANGLED: GameResultType::Set(GameResultType const&)
int GameResultType::Set(const GameResultType& source) {
    *this = source;
    return 1;
}

// IDA_EA: 0x84B7442
// IDA_NAME: _ZN14GameResultType5ClearEv
// DEMANGLED: GameResultType::Clear(void)
GameResultType* GameResultType::Clear() {
    raw_00 = 0;
    raw_04 = 0;
    raw_08 = 0;
    raw_09 = 0;
    raw_0a = 0;
    raw_0c = 0;
    raw_10 = 0;
    for (unsigned int index = 0; index < 4u; ++index) {
        shot_counts[index].reset();
    }
    for (unsigned int index = 0; index < 8u; ++index) {
        score_values[index] = 0;
    }
    best_time = 0;
    raw_48 = 0;
    raw_4a = 0;
    raw_4c = 0;
    return this;
}

// IDA_EA: 0x84B750C
// IDA_NAME: _ZN14GameResultType15UpdateHighScoreERKS_
// DEMANGLED: GameResultType::UpdateHighScore(GameResultType const&)
int GameResultType::UpdateHighScore(const GameResultType& source) {
    int result = 0;
    if (source.score_values[6] > score_values[6]) {
        score_values[6] = source.score_values[6];
        result = 1;
    }
    if (source.score_values[7] > score_values[7]) {
        score_values[7] = source.score_values[7];
        result |= 2;
    }
    if (source.raw_48 > raw_48) {
        raw_48 = source.raw_48;
        result |= 8;
    }
    if (source.best_time < best_time) {
        best_time = source.best_time;
        result |= 4;
    }
    return result;
}

// IDA_EA: 0x84B75AE
// IDA_NAME: _ZN14GameResultType12GetUniqueKeyEv
// DEMANGLED: GameResultType::GetUniqueKey(void)
int GameResultType::GetUniqueKey() {
    return 10 * raw_04 + raw_08;
}

// IDA_EA: 0x82FED32
// IDA_NAME: _ZN19CDungeonClearTracer5TraceEPKc
// DEMANGLED: CDungeonClearTracer::Trace(char const*)
int CDungeonClearTracer::Trace(const char* function) {
    if (IsKilledBoss()) {
        cMyTrace trace("void CDungeonClearTracer::Trace(const char*)", 264, 0);
        trace("FUNC(%s) Map ID(%d), Boss(%d/%d), Monster(%d/%d) Cleared(%d), End Point(%d)\n",
              function,
              map_id,
              killed_boss_count,
              max_boss_count,
              killed_monster_count,
              max_monster_count,
              cleared_dungeon,
              end_point);
    }
    return reinterpret_cast<int>(Reset());
}

// IDA_EA: 0x82FEE04
// IDA_NAME: _ZN19CDungeonClearTracer8SetMapIdEi
// DEMANGLED: CDungeonClearTracer::SetMapId(int)
CDungeonClearTracer* CDungeonClearTracer::SetMapId(int value) {
    map_id = value;
    return this;
}

// IDA_EA: 0x82FEE12
// IDA_NAME: _ZN19CDungeonClearTracer15SetMaxBossCountEi
// DEMANGLED: CDungeonClearTracer::SetMaxBossCount(int)
CDungeonClearTracer* CDungeonClearTracer::SetMaxBossCount(int count) {
    max_boss_count = count;
    return this;
}

// IDA_EA: 0x82FEE20
// IDA_NAME: _ZN19CDungeonClearTracer18SetKilledBossCountEi
// DEMANGLED: CDungeonClearTracer::SetKilledBossCount(int)
CDungeonClearTracer* CDungeonClearTracer::SetKilledBossCount(int count) {
    killed_boss_count = count;
    return this;
}

// IDA_EA: 0x82FEE2E
// IDA_NAME: _ZN19CDungeonClearTracer18SetMaxMonsterCountEi
// DEMANGLED: CDungeonClearTracer::SetMaxMonsterCount(int)
CDungeonClearTracer* CDungeonClearTracer::SetMaxMonsterCount(int count) {
    max_monster_count = count;
    return this;
}

// IDA_EA: 0x82FEE3C
// IDA_NAME: _ZN19CDungeonClearTracer21SetKilledMonsterCountEi
// DEMANGLED: CDungeonClearTracer::SetKilledMonsterCount(int)
CDungeonClearTracer* CDungeonClearTracer::SetKilledMonsterCount(int count) {
    killed_monster_count = count;
    return this;
}

// IDA_EA: 0x82FEE4A
// IDA_NAME: _ZN19CDungeonClearTracer17SetClearedDungeonEb
// DEMANGLED: CDungeonClearTracer::SetClearedDungeon(bool)
CDungeonClearTracer* CDungeonClearTracer::SetClearedDungeon(bool value) {
    cleared_dungeon = value;
    return this;
}

// IDA_EA: 0x82FEE62
// IDA_NAME: _ZN19CDungeonClearTracer11SetEndPointEb
// DEMANGLED: CDungeonClearTracer::SetEndPoint(bool)
CDungeonClearTracer* CDungeonClearTracer::SetEndPoint(bool value) {
    end_point = value;
    return this;
}

// IDA_EA: 0x82FEE7A
// IDA_NAME: _ZN19CDungeonClearTracer24IncrementKilledBossCountEv
// DEMANGLED: CDungeonClearTracer::IncrementKilledBossCount(void)
CDungeonClearTracer* CDungeonClearTracer::IncrementKilledBossCount() {
    ++killed_boss_count;
    return this;
}

// IDA_EA: 0x82FEE8E
// IDA_NAME: _ZN19CDungeonClearTracer27IncrementKilledMonsterCountEv
// DEMANGLED: CDungeonClearTracer::IncrementKilledMonsterCount(void)
CDungeonClearTracer* CDungeonClearTracer::IncrementKilledMonsterCount() {
    ++killed_monster_count;
    return this;
}

// IDA_EA: 0x82FEEA2
// IDA_NAME: _ZN19CDungeonClearTracer12IsKilledBossEv
// DEMANGLED: CDungeonClearTracer::IsKilledBoss(void)
bool CDungeonClearTracer::IsKilledBoss() {
    return killed_boss_count > 0 && killed_boss_count >= max_boss_count;
}

// IDA_EA: 0x82FEECE
// IDA_NAME: _ZN19CDungeonClearTracer5ResetEv
// DEMANGLED: CDungeonClearTracer::Reset(void)
CDungeonClearTracer* CDungeonClearTracer::Reset() {
    map_id = 0;
    max_boss_count = 0;
    killed_boss_count = 0;
    max_monster_count = 0;
    killed_monster_count = 0;
    cleared_dungeon = 0;
    end_point = 0;
    return this;
}
