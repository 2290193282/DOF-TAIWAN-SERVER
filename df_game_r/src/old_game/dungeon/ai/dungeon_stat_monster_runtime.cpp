#include "old_game/dungeon/instance/dungeon_stat_runtime.h"

// IDA unit: 99_misc_global/Monster.
// The value is owned by CDungeonNode's dungeon-statistics map and is not a
// gameplay monster instance.

// IDA_EA: 0x8486956
// IDA_NAME: _ZN7Monster5ResetEv
// DEMANGLED: Monster::Reset(void)
Monster* Monster::Reset() {
    damage_total = 0;
    damage_count = 0;
    merge_count = 0;
    return this;
}

// IDA_EA: 0x8486978
// IDA_NAME: _ZN7MonsterC2Ev
// DEMANGLED: Monster::Monster(void)
Monster::Monster()
    : IData() {
    Reset();
}

// IDA_EA: 0x8486996
// IDA_NAME: _ZN7MonsterpLERKS_
// DEMANGLED: Monster::operator+=(Monster const&)
Monster& Monster::operator+=(const Monster& other) {
    damage_total += other.damage_total;
    damage_count += other.damage_count;
    ++merge_count;
    return *this;
}

// IDA_EA: 0x84869D0
// IDA_NAME: _ZN7MonsterD2Ev
// DEMANGLED: Monster::~Monster()
Monster::~Monster() {}
