#ifndef OLDSRV_OLD_DATA_PVF_EVENT_BLUE_MARBLE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_BLUE_MARBLE_SCRIPT_H_

#include "old_data/PVF/common/script_file_list.h"

#include <stddef.h>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>


// IDA_EA: 0x88D8EE0, 0x88D97E0, 0x88D509C, 0x88D53D6, 0x88D5418, 0x88D54CE, 0x88D54E2, 0x88D5644, 0x837E218, 0x88D5690, 0x88D57C0, 0x88D589C, 0x88D5E92, 0x88D5F38, 0x88D5FDE, 0x88D6084, 0x837E354, 0x88D60F2, 0x88D614A, 0x88D622C, 0x88D626E, 0x88D6438, 0x88D6622, 0x88D6646, 0x88D66B6, 0x88D6726.
// IDA unit: DataManager/PVF BlueMarble script import boundary.
struct BlueMarbleRandomDungeonInfo {
    ~BlueMarbleRandomDungeonInfo();

    int32_t min_level;
    int32_t max_level;
    int32_t dungeon_index;
};

struct BlueMarbleDungeonDifficulty {
    ~BlueMarbleDungeonDifficulty();

    int32_t dungeon_index;
    int32_t difficulty;
};

// IDA_EA: 0x88D9120.
// IDA_NAME: _ZN24BlueMarbleRandomItemInfoD2Ev
// DEMANGLED: BlueMarbleRandomItemInfo::~BlueMarbleRandomItemInfo()
struct BlueMarbleRandomItemInfo {
    ~BlueMarbleRandomItemInfo();

    int32_t first_dword;
    int32_t second_dword;
};

// IDA_EA: 0x88D9360.
// IDA_NAME: _ZN20BlueMarbleRewardInfoD2Ev
// DEMANGLED: BlueMarbleRewardInfo::~BlueMarbleRewardInfo()
struct BlueMarbleRewardInfo {
    ~BlueMarbleRewardInfo();

    int32_t first_dword;
    int32_t second_dword;
};

// IDA_EA: 0x88D95A0.
// IDA_NAME: _ZN17BlueMarbleTilePosD2Ev
// DEMANGLED: BlueMarbleTilePos::~BlueMarbleTilePos()
struct BlueMarbleTilePos {
    ~BlueMarbleTilePos();

    int32_t first_dword;
    int32_t second_dword;
};

// IDA_EA: ctor/clear/toType/import @0x88D5068/0x88D507C/0x88D520A/0x88D509C.
// IDA unit: 12_pvf_datamanager_scripts/BlueMarbleTileScript.
struct BlueMarbleTileScript {
    // IDA uses this nested enum in the public BlueMarble gameplay ABI.  It is
    // an int-sized tag and therefore does not change the PVF record layout.
    enum Type : int {
        TYPE_NONE = 0,
    };

    BlueMarbleTileScript();

    BlueMarbleTileScript* clear();
    int toType(std::string type_name);
    int importScript(const char* filename);

    int32_t tile_type;
    uint8_t enter_dungeon;
    int32_t value;
};

class BlueMarbleMapScript {
public:
    BlueMarbleMapScript();
    ~BlueMarbleMapScript();

    int clear();
    int importScript(const char* filename);
    BlueMarbleTileScript* getTile(unsigned int board_zone);

    std::vector<BlueMarbleTileScript*> tiles;
};

class BlueMarbleInfoScript {
public:
    BlueMarbleInfoScript();
    ~BlueMarbleInfoScript();

    BlueMarbleInfoScript* clear();
    int importScript(const char* dir, const char* filename);
    std::vector<int> getRandomDungeonIndex(int charac_level);
    std::vector<int> getBossDungeonIndex(int charac_level);
    std::vector<int> getUniqueDungeonIndex(int charac_level);
    int getDungeonDifficulty(int dungeon_index);

    int32_t player_max;
    int32_t enter_level;
    int32_t enter_max_level;
    int32_t enter_count;
    int32_t die_count;
    int32_t dice_min;
    int32_t dice_max;
    int32_t item_probability_total;
    int32_t default_dungeon_difficulty;
    std::vector<BlueMarbleDungeonDifficulty> dungeon_difficulties;
    int32_t super_armor_time;
    int32_t buff_max_value;
    float gold_base;
    float gold_multi;
    int32_t equipment_upgrade_max;
    std::vector<BlueMarbleRandomDungeonInfo> random_dungeons;
    std::vector<BlueMarbleRandomDungeonInfo> boss_dungeons;
    std::vector<BlueMarbleRandomDungeonInfo> unique_dungeons;
    std::vector<BlueMarbleRandomItemInfo> random_items;
    std::vector<BlueMarbleRewardInfo> reward_items;
    std::vector<BlueMarbleTilePos> tile_positions;
};

class BlueMarbleScriptManager {
public:
    static BlueMarbleScriptManager* getInsance();

    BlueMarbleScriptManager();
    ~BlueMarbleScriptManager();

    int clear();
    int initTileList(const char* dir, const char* filename);
    int initMapList(const char* dir, const char* filename);
    int initBlueMarbleInfo(const char* dir, const char* filename);
    BlueMarbleTileScript* getTile(int tile_index);
    BlueMarbleMapScript* getMap(int map_index);
    int getMapTotalCount();

    STScriptFileList map_file_list;
    STScriptFileList tile_file_list;
    BlueMarbleInfoScript info_script;
    std::map<int, BlueMarbleTileScript*> tile_scripts;
    std::map<int, BlueMarbleMapScript*> map_scripts;
};

static_assert(sizeof(BlueMarbleRandomDungeonInfo) == 0x0Cu,
              "BlueMarbleRandomDungeonInfo IDA ABI size must stay 0x0C");
static_assert(sizeof(BlueMarbleDungeonDifficulty) == 0x08u,
              "BlueMarbleDungeonDifficulty IDA ABI size must stay 0x08");
static_assert(sizeof(BlueMarbleRandomItemInfo) == 0x08u,
              "BlueMarbleRandomItemInfo IDA ABI size must stay 0x08");
static_assert(sizeof(BlueMarbleRewardInfo) == 0x08u,
              "BlueMarbleRewardInfo IDA ABI size must stay 0x08");
static_assert(sizeof(BlueMarbleTilePos) == 0x08u,
              "BlueMarbleTilePos IDA ABI size must stay 0x08");
static_assert(sizeof(BlueMarbleTileScript) == 0x0Cu,
              "BlueMarbleTileScript IDA ABI size must stay 0x0C");
static_assert(sizeof(BlueMarbleMapScript) == 0x0Cu,
              "BlueMarbleMapScript IDA ABI size must stay 0x0C");
static_assert(sizeof(BlueMarbleInfoScript) == 0x8Cu,
              "BlueMarbleInfoScript IDA ABI size must stay 0x8C");
static_assert(sizeof(BlueMarbleScriptManager) == 0xECu,
              "BlueMarbleScriptManager IDA ABI size must stay 0xEC");
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(BlueMarbleScriptManager, info_script) == 0x30u,
              "BlueMarbleScriptManager info script must stay at +0x30");
static_assert(offsetof(BlueMarbleInfoScript, gold_base) == 0x38u,
              "BlueMarbleInfoScript gold base must stay at +0x38");
static_assert(offsetof(BlueMarbleInfoScript, gold_multi) == 0x3cu,
              "BlueMarbleInfoScript gold multiplier must stay at +0x3c");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_BLUE_MARBLE_SCRIPT_H_
