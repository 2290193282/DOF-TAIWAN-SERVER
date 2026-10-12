#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x8374F24
// IDA_NAME: _ZN8WongWork13CDungeonClear17addClearedDungeonEsc
// DEMANGLED: WongWork::CDungeonClear::addClearedDungeon(short,char)
void WongWork::CDungeonClear::addClearedDungeon(short dungeon_index,
                                                char difficulty) {
    Base::iterator found = Base::find(dungeon_index);
    if (found == Base::end()) {
        Base::insert(std::make_pair(dungeon_index, difficulty));
        return;
    }
    found->second = difficulty;
}

// IDA_EA: 0x85BF96C
// IDA_NAME: _ZNK8WongWork13CDungeonClear21getClearedDungeonDiffEs
// DEMANGLED: WongWork::CDungeonClear::getClearedDungeonDiff(short)const
int WongWork::CDungeonClear::getClearedDungeonDiff(short dungeon_index) const {
    Base::const_iterator found = Base::find(dungeon_index);
    if (found == Base::end()) {
        return 0;
    }
    return static_cast<unsigned char>(found->second);
}
