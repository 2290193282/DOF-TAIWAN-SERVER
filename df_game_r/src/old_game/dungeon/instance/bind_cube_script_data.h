#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_SCRIPT_DATA_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_SCRIPT_DATA_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

#include <stddef.h>
#include <map>
#include <stdint.h>
#include <vector>

namespace ExtreamDungeon {

// IDA unit: 09_dungeon_pvp_party/ExtreamDungeon/BindCube_ScriptData.
// The original C2 constructs the 59 x 4 primary table at +0x30, four
// secondary lists at +0xb40, and the force-rule map at +0xb70.
struct BindCube_ScriptData {
    // IDA_EA: 0x832E058
    // IDA_NAME: _ZN14ExtreamDungeon19BindCube_ScriptDataC2Ev
    // DEMANGLED: ExtreamDungeon::BindCube_ScriptData::BindCube_ScriptData(void)
    BindCube_ScriptData();

    // IDA_EA: 0x832E220
    // IDA_NAME: _ZN14ExtreamDungeon19BindCube_ScriptDataD2Ev
    // DEMANGLED: ExtreamDungeon::BindCube_ScriptData::~BindCube_ScriptData()
    ~BindCube_ScriptData();

    uint32_t rarity_decision_thresholds[3][4];
    std::vector<OUTPUT_ITEM_STR> output_items[59][4];
    std::vector<OUTPUT_ITEM_STR> secondary_output_items[4];
    std::map<int, forceRule_ResultItem> force_rule_result_items;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(BindCube_ScriptData, rarity_decision_thresholds) ==
                  0x000u &&
                  offsetof(BindCube_ScriptData, output_items) == 0x030u &&
                  offsetof(BindCube_ScriptData, secondary_output_items) ==
                  0x0b40u &&
                  offsetof(BindCube_ScriptData, force_rule_result_items) ==
                  0x0b70u &&
                  sizeof(BindCube_ScriptData) == 0x0b88u,
              "BindCube_ScriptData must retain its original i386 layout");
#endif

}  // namespace ExtreamDungeon

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_SCRIPT_DATA_H_
