#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_LV_BONUS_SCRIPT_DATA_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_LV_BONUS_SCRIPT_DATA_H_

#include <vector>

namespace ExtreamDungeon {

struct BindCube_LvBonusScriptData {
    // IDA_EA: 0x832E318
    // IDA_NAME: _ZN14ExtreamDungeon26BindCube_LvBonusScriptDataC2Ev
    // DEMANGLED: ExtreamDungeon::BindCube_LvBonusScriptData::BindCube_LvBonusScriptData(void)
    BindCube_LvBonusScriptData();

    // IDA_EA: 0x832E3AC
    // IDA_NAME: _ZN14ExtreamDungeon26BindCube_LvBonusScriptDataD2Ev
    // DEMANGLED: ExtreamDungeon::BindCube_LvBonusScriptData::~BindCube_LvBonusScriptData()
    ~BindCube_LvBonusScriptData();

    std::vector<unsigned int> level_boundaries;
    std::vector<unsigned int> decision_threshold_by_input_rarity[3];
};

static_assert(sizeof(BindCube_LvBonusScriptData) == 0x30u,
              "BindCube_LvBonusScriptData i386 layout drift");

}  // namespace ExtreamDungeon

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_BIND_CUBE_LV_BONUS_SCRIPT_DATA_H_
