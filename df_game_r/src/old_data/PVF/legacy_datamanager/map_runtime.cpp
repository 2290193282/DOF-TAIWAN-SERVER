#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/dungeon/advance_altar_dungeon_script.h"
#include "old_data/PVF/etc/time_spiral_script.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <queue>
#include <string>

#include "old_data/PVF/data/cnrd_script_data.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "common/types/code_page.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/party/hellparty_characlink_abi.h"
#include "common/manager/server_utility.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/tower/cdungeon_tower_of_despair_runtime.h"
#include "old_game/progression/quest/npc_item_drop_runtime.h"
#include "old_game/social/party/quick_party_system_manager_runtime.h"
#include "old_game/world/region/world_map_pattern_abi.h"

extern char g_szMapScriptBaseDirectory[256];
extern char g_szDungeonScriptBaseDirectory[256];
extern STScriptFileList g_stMapScriptFileList;
extern STScriptFileList g_stDungeonScriptFileList;

// IDA_EA: 0x093F4EA0
// IDA_NAME: g_gridSymbols
// DEMANGLED: g_gridSymbols
const char* g_gridSymbols[16] __asm__("g_gridSymbols") = {
    "AA", "BB", "CC", "DD", "EE", "FF", "GG", "HH",
    "II", "JJ", "KK", "LL", "MM", "NN", "OO", "PP"
};

// IDA_EA: 0x08E1C6E0
// IDA_NAME: CHAMPION_FEATURE_LIST
static const int32_t CHAMPION_FEATURE_LIST[37] = {
    0x0001ff00, 0x0000ff01, 0x0002ff02, 0x0002ff03, 0x00030004,
    0x00030005, 0x00030006, 0x00030007, 0x00040008, 0x0003fe09,
    0x0003fe0a, 0x0003fe0b, 0x0003fe0c, 0x0002000d, 0x0003000e,
    0x0003000f, 0x00020010, 0x00040011, 0x00010012, 0x00020013,
    0x0002fd14, 0x0002fd15, 0x0002fd16, 0x0002fd17, 0x0002fd18,
    0x0002fd19, 0x0002fd1a, 0x0003001b, 0x0004fd1c, 0x0004fd1d,
    0x0004fd1e, 0x0004fd1f, 0x0004fd20, 0x0004fd21, 0x0004fd22,
    0x0004fd23, 0x0004fd24
};

// IDA_EA: 0x093F55A0
// IDA_NAME: CHAMPION_FEATURE_NAME
const char* CHAMPION_FEATURE_NAME[37]
    __asm__("CHAMPION_FEATURE_NAME") = {
        "[Immune-Magic]", "[Immune-Physic]",
        "[Enhance-Physicaldefense]", "[Enhance-Magicaldefense]",
        "[Immune-Fire]", "[Immune-Water]", "[Immune-Light]",
        "[Immune-Dark]", "[Immune-ActiveStatus]", "[Enchant-Fire]",
        "[Enchant-Water]", "[Enchant-Light]", "[Enchant-Dark]",
        "[Speedup-Move]", "[Speedup-Attack]", "[Speedup-Hitrecovery]",
        "[Speedup-Wakeup]", "[Speedup-Hpregen]", "[Etc-Neverfly]",
        "[Etc-Reserrection]", "[onAttack-stun]", "[onAttack-bleeding]",
        "[onAttack-confuse]", "[onAttack-slow]", "[onAttack-lightning]",
        "[onAttack-poison]", "[onAttack-blind]", "[high-activestatus]",
        "[Etc-divide]", "[Etc-invisible]", "[Etc-healed]",
        "[Etc-infected]", "[Etc-counted]", "[Etc-berserk]",
        "[Etc-sleep]", "[Etc-revive]", "[Etc-teleport]"
    };

// IDA_EA: 0x89E8712
// IDA_NAME: _ZN21STHellPartyGroupOrderC2Ev
// DEMANGLED: STHellPartyGroupOrder::STHellPartyGroupOrder(void)
STHellPartyGroupOrder::STHellPartyGroupOrder() : group_rates() {
    std::memset(rates_by_group_order, 0, sizeof(rates_by_group_order));
}

// IDA_EA: 0x89E9DFC
// IDA_NAME: _ZN21STHellPartyGroupOrderC2ERKS_
// DEMANGLED: STHellPartyGroupOrder::STHellPartyGroupOrder(STHellPartyGroupOrder const&)
STHellPartyGroupOrder::STHellPartyGroupOrder(
    const STHellPartyGroupOrder& rhs)
    : group_rates(rhs.group_rates) {
    std::memcpy(rates_by_group_order,
                rhs.rates_by_group_order,
                sizeof(rates_by_group_order));
}

// IDA_EA: 0x89E8744
// IDA_NAME: _ZN21STHellPartyGroupOrderD2Ev
// DEMANGLED: STHellPartyGroupOrder::~STHellPartyGroupOrder()
STHellPartyGroupOrder::~STHellPartyGroupOrder() {}

// IDA_EA: 0x89E813F
// IDA_NAME: _Z32Get_instance_groupOrder_map_poolv
// DEMANGLED: Get_instance_groupOrder_map_pool(void)
HellPartyGroupOrderMapPool* Get_instance_groupOrder_map_pool() {
    static HellPartyGroupOrderMapPool hell_party_group_order_map_pool;
    return &hell_party_group_order_map_pool;
}

// IDA_EA: 0x89E7958
// IDA_NAME: _Z24procSpecialPassiveObjectR18SPOBJ_ACTION_DATASR25STAssignSpecialPassiveObji
// DEMANGLED: procSpecialPassiveObject(SPOBJ_ACTION_DATAS &,STAssignSpecialPassiveObj &,int)
void procSpecialPassiveObject(SPOBJ_ACTION_DATAS& action,
                              STAssignSpecialPassiveObj& object,
                              int map_index) {
    std::string action_tag;
    std::priority_queue<int> group_indexes;

    for (int field_index = 0; field_index <= 5; ++field_index) {
        if (field_index != 0) {
            const int value = ScanInt(static_cast<bool*>(0));
            if (field_index == 1) {
                action.value_04 = value;
            } else if (field_index == 2) {
                action.value_08 = value;
            } else if (field_index == 3) {
                action.value_0c = value;
            } else if (field_index == 4) {
                action.value_10 = value;
            } else {
                action.value_14 = value;
            }
            continue;
        }

        if (ScanStr(&action_tag) != 1) {
            continue;
        }

        if (action_tag == "[monster]") {
            action.action_type = 0;
        } else if (action_tag == "[trap]") {
            action.action_type = 1;
        } else if (action_tag == "[item]") {
            action.action_type = 2;
        } else if (action_tag == "[quest]") {
            action.action_type = 3;
        } else if (action_tag == "[hellparty]") {
            HellPartyGroupOrder group_orders;
            stHellParty_t* const hell_party = Get_instance_stHellParty_t();
            bool valid_group_orders = true;
            bool success = false;

            while (true) {
                const int group_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                group_indexes.push(group_index);

                const std::map<uint16_t, stHellPartyMonsterGroupList>::const_iterator
                    group = hell_party->monster_groups.find(
                        static_cast<uint16_t>(group_index));
                if (group == hell_party->monster_groups.end()) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptMap.cpp",
                        "procSpecialPassiveObject",
                        0xD5C,
                        "HELLPARTY_LOG : GROUP INDEX NOT FOUND : %d\n",
                        group_index);
                    valid_group_orders = false;
                    break;
                }

                const int rate = ScanInt(&success);
                if (!success) {
                    valid_group_orders = false;
                    break;
                }
                const int group_order = ScanInt(&success);
                if (!success) {
                    valid_group_orders = false;
                    break;
                }
                if (group_order <= 0 || group_order > 9) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptMap.cpp",
                        "procSpecialPassiveObject",
                        0xD69,
                        "HELLPARTY_LOG : GROUP ORDER SCRIPT ERROR : %d\n",
                        group_order);
                    valid_group_orders = false;
                    break;
                }

                STHellPartyGroupOrder& order =
                    group_orders[static_cast<uint16_t>(group_order)];
                STHellPartyGroupRate group_rate;
                group_rate.group_index = group_index;
                group_rate.rate = rate;
                group_rate.group_order = group->second.group_order;
                group_rate.reserved_09[0] = 0;
                group_rate.reserved_09[1] = 0;
                group_rate.reserved_09[2] = 0;
                order.group_rates.push_back(group_rate);
                order.rates_by_group_order[group->second.group_order] +=
                    static_cast<uint16_t>(rate);
            }

            if (!valid_group_orders) {
                break;
            }

            Get_instance_groupOrder_map_pool()->insert(
                std::make_pair(map_index, group_orders));

            int previous_group_index = 0;
            int monster_slot = 0;
            while (!group_indexes.empty()) {
                const int group_index = group_indexes.top();
                if (group_index != previous_group_index && group_index != 0) {
                    previous_group_index = group_index;
                    const std::map<uint16_t, stHellPartyMonsterGroupList>::const_iterator
                        group = hell_party->monster_groups.find(
                            static_cast<uint16_t>(group_index));
                    action.action_type = 5;
                    while (group->second.monster_index[monster_slot] > 0) {
                        action.value_04 =
                            group->second.monster_index[monster_slot];
                        action.value_08 = 1;
                        action.value_0c = 1;
                        action.value_10 = 1;
                        action.value_14 = -1;
                        object.actions.push_back(action);
                        ++monster_slot;
                    }
                    monster_slot = 0;
                } else {
                    group_indexes.pop();
                }
            }
            break;
        }
    }
}

namespace {

struct DungeonInCountRecord {
    int channel_type;
    int dungeon_index;
    int maximum_count;
    int reset_group;
};

struct DungeonInCountConfig {
    bool available;
    std::vector<int> impossible_dungeons;
    std::vector<DungeonInCountRecord> ordered_records;
    std::map<int, DungeonInCountRecord> records;

    DungeonInCountConfig()
        : available(false), impossible_dungeons(), ordered_records(),
          records() {}

    void clear() {
        available = false;
        impossible_dungeons.clear();
        ordered_records.clear();
        records.clear();
    }
};

DungeonInCountConfig g_dungeon_incount_config;

struct DungeonRuntimeConfig {
    bool critical_tail_seen;
    bool extension_0828_seen;
    bool golden_commerce;
    bool power_war;
    bool move_map_even_enemy;
    bool enter_without_fatigue;
    bool has_limit_item_stat;
    bool limit_item_stat;
    bool has_limit_escape_character;
    bool limit_escape_character;
    bool has_character_respawn_position;
    bool character_respawn_position;
    bool has_room_move_keep_state;
    bool room_move_keep_state;
    bool has_use_countdown;
    bool use_countdown;
    int enable_apc_stackable;
    bool has_fixed_difficulty;
    int fixed_difficulty;
    bool has_zoom_rate;
    float zoom_rate;
    std::map<AbsoluteChangeStat::T, float> absolute_change_stats;
    std::vector<int> realdungeon_checkup_monsters;
    std::map<int, std::vector<dungeon_runtime::MapMoveMonster> > map_moves;
    std::map<std::string, std::vector<int> > rank_types;

    DungeonRuntimeConfig()
        : critical_tail_seen(false), extension_0828_seen(false),
          golden_commerce(false),
          power_war(false), move_map_even_enemy(false),
          enter_without_fatigue(false), has_limit_item_stat(false),
          limit_item_stat(false), has_limit_escape_character(false),
          limit_escape_character(false),
          has_character_respawn_position(false),
          character_respawn_position(false),
          has_room_move_keep_state(false), room_move_keep_state(false),
          has_use_countdown(false), use_countdown(false),
          enable_apc_stackable(-1), has_fixed_difficulty(false),
          fixed_difficulty(-1), has_zoom_rate(false), zoom_rate(1.0f),
          absolute_change_stats(), realdungeon_checkup_monsters(),
          map_moves(), rank_types() {}
};

typedef std::map<int, DungeonRuntimeConfig> DungeonRuntimeConfigMap;
DungeonRuntimeConfigMap g_dungeon_runtime_configs;

struct MapRuntimeExtension0828 {
    map_runtime::ChaosMapConfig0828 chaos;
    bool has_twin_gauge;
    map_runtime::TwinGauge0828 twin_gauge;

    MapRuntimeExtension0828()
        : chaos(), has_twin_gauge(false), twin_gauge() {}

    bool empty() const {
        return !chaos.chaos_zone && !chaos.has_phase_max &&
               !chaos.has_phase_time && !chaos.has_final_gate_index &&
               !chaos.has_gate_index && !chaos.has_auto_revive_interval &&
               !chaos.has_gate_bonus_fp && !chaos.has_phase_fp &&
               !has_twin_gauge;
    }
};

typedef std::map<int, MapRuntimeExtension0828> MapRuntimeExtension0828Map;
MapRuntimeExtension0828Map g_map_runtime_extensions_0828;
MapRuntimeExtension0828Map g_map_runtime_extensions_0828_staging;
bool g_map_runtime_extensions_0828_load_active = false;

void PublishMapRuntimeExtension0828(
    int map_index,
    const MapRuntimeExtension0828& extension) {
    if (g_map_runtime_extensions_0828_load_active) {
        if (extension.empty()) {
            g_map_runtime_extensions_0828_staging.erase(map_index);
        } else {
            g_map_runtime_extensions_0828_staging[map_index] = extension;
        }
        return;
    }

    // Standalone ImportMapScript callers still receive per-map atomic
    // publication: construct the replacement registry before swapping it.
    MapRuntimeExtension0828Map replacement(g_map_runtime_extensions_0828);
    if (extension.empty()) {
        replacement.erase(map_index);
    } else {
        replacement[map_index] = extension;
    }
    g_map_runtime_extensions_0828.swap(replacement);
}

bool FinishStrictMapExtensionField(std::string* next_tag, bool* pending) {
    if (!next_tag || !pending) {
        return false;
    }

    int int_value = 0;
    float float_value = 0.0f;
    std::string text;
    const DNFLEX_TOKEN token = ScanAll(&int_value, &float_value, &text);
    if (token == DNF_TOK_TYPE) {
        *next_tag = text;
        *pending = true;
        return true;
    }
    return token == DNF_TOK_EOF;
}

bool ScanStrictMapExtensionScalar(int* value,
                                  std::string* next_tag,
                                  bool* pending) {
    if (!value) {
        return false;
    }
    bool success = false;
    *value = ScanInt(&success);
    return success && FinishStrictMapExtensionField(next_tag, pending);
}

bool LoadDungeonInCountConfig() {
    g_dungeon_incount_config.clear();

    // 0828 moved the six Otherverse counters out of each DGN and into this
    // owner table.  A missing table is valid for the 0627 layout, whose DGN
    // files still carry the two legacy integers after [dimension dungeon].
    if (loadRDARScriptFile(
            "", "etc/dungeonetc/dungeonincountinfo.etc") != 1) {
        return true;
    }

    bool in_channel_info = false;
    bool in_impossible_channel = false;
    bool in_incount_info = false;
    std::vector<int> incount_values;

    while (true) {
        int int_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&int_value, &float_value, &text);
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_ERROR) {
            unloadRDARScriptFile();
            return false;
        }

        if (token == DNF_TOK_TYPE) {
            if (text == "[channel info]") {
                in_channel_info = true;
                in_impossible_channel = false;
            } else if (text == "[/channel info]") {
                in_channel_info = false;
                in_impossible_channel = false;
            } else if (text == "[dungeon incount info]") {
                in_incount_info = true;
            } else if (text == "[/dungeon incount info]") {
                in_incount_info = false;
            }
            continue;
        }

        if (token == DNF_TOK_STRING && in_channel_info) {
            in_impossible_channel = text == "[impossible]";
            continue;
        }

        if (token == DNF_TOK_DECIMAL ||
            token == DNF_TOK_HEXADECIMAL) {
            if (in_impossible_channel) {
                g_dungeon_incount_config.impossible_dungeons.push_back(
                    int_value);
            }
            if (in_incount_info) {
                incount_values.push_back(int_value);
            }
        }
    }
    unloadRDARScriptFile();

    if (g_dungeon_incount_config.impossible_dungeons.empty() ||
        g_dungeon_incount_config.impossible_dungeons.size() > 6u ||
        incount_values.empty() || incount_values.size() % 4u != 0u) {
        g_dungeon_incount_config.clear();
        return false;
    }

    for (std::size_t offset = 0; offset < incount_values.size();
         offset += 4u) {
        DungeonInCountRecord record;
        record.channel_type = incount_values[offset];
        record.dungeon_index = incount_values[offset + 1u];
        record.maximum_count = incount_values[offset + 2u];
        record.reset_group = incount_values[offset + 3u];
        if (record.dungeon_index <= 0 || record.maximum_count < 0 ||
            record.maximum_count > 127 ||
            !g_dungeon_incount_config.records
                 .insert(std::make_pair(record.dungeon_index, record))
                 .second) {
            g_dungeon_incount_config.clear();
            return false;
        }
        g_dungeon_incount_config.ordered_records.push_back(record);
    }

    std::map<int, bool> unique_impossible;
    for (std::size_t slot = 0;
         slot < g_dungeon_incount_config.impossible_dungeons.size();
         ++slot) {
        const int dungeon_index =
            g_dungeon_incount_config.impossible_dungeons[slot];
        const std::map<int, DungeonInCountRecord>::const_iterator record =
            g_dungeon_incount_config.records.find(dungeon_index);
        if (!unique_impossible.insert(std::make_pair(dungeon_index, true))
                 .second ||
            record == g_dungeon_incount_config.records.end() ||
            record->second.channel_type != 0 ||
            record->second.maximum_count <= 0 ||
            record->second.reset_group != 1) {
            g_dungeon_incount_config.clear();
            return false;
        }
    }

    g_dungeon_incount_config.available = true;
    return true;
}

bool ResolveDimensionDungeonConfig(int dungeon_index,
                                   int worldmap_icon_frame,
                                   uint8_t* dimension_slot,
                                   int8_t* maximum_count) {
    if (!g_dungeon_incount_config.available || !dimension_slot ||
        !maximum_count) {
        return false;
    }

    const std::vector<int>& ordered =
        g_dungeon_incount_config.impossible_dungeons;
    const std::vector<int>::const_iterator owner =
        std::find(ordered.begin(), ordered.end(), dungeon_index);
    if (owner == ordered.end()) {
        return false;
    }
    const std::size_t slot =
        static_cast<std::size_t>(owner - ordered.begin());
    const std::map<int, DungeonInCountRecord>::const_iterator record =
        g_dungeon_incount_config.records.find(dungeon_index);
    if (slot >= 6u || worldmap_icon_frame != static_cast<int>(slot) ||
        record == g_dungeon_incount_config.records.end()) {
        return false;
    }

    *dimension_slot = static_cast<uint8_t>(slot + 1u);
    *maximum_count = static_cast<int8_t>(record->second.maximum_count);
    return true;
}

int ResolveDungeonInCountMaximum(int dungeon_index, int fallback) {
    if (g_dungeon_incount_config.available) {
        const std::map<int, DungeonInCountRecord>::const_iterator record =
            g_dungeon_incount_config.records.find(dungeon_index);
        if (record != g_dungeon_incount_config.records.end() &&
            record->second.maximum_count > 0) {
            return record->second.maximum_count;
        }
    }
    return fallback;
}

bool LastTokenIsType(const char* expected);

int ResolveMapMoveRank(const std::string& rank) {
    if (rank == "[normal]") {
        return 0;
    }
    if (rank == "[champion]") {
        return 1;
    }
    if (rank == "[super champion]") {
        return 2;
    }
    if (rank == "[boss]") {
        return 3;
    }
    return -1;
}

bool SameMapMoveProjection(
    const dungeon_runtime::MapMoveMonster& lhs,
    const dungeon_runtime::MapMoveMonster& rhs) {
    return lhs.group == rhs.group &&
           lhs.monster_id == rhs.monster_id && lhs.level == rhs.level &&
           lhs.enabled == rhs.enabled && lhs.spawn_mode == rhs.spawn_mode &&
           lhs.position_x == rhs.position_x &&
           lhs.position_y == rhs.position_y &&
           lhs.value_18 == rhs.value_18 &&
           lhs.specific_drop_count == rhs.specific_drop_count &&
           lhs.random_drop_count == rhs.random_drop_count &&
           lhs.value_24 == rhs.value_24 && lhs.rank == rhs.rank &&
           lhs.special_class == rhs.special_class &&
           lhs.tail_value == rhs.tail_value;
}

bool ParseMapMoveMonsterBlock(DungeonRuntimeConfig* config) {
    if (!config || !config->map_moves.empty()) {
        return false;
    }

    bool saw_row = false;
    while (true) {
        bool success = false;
        dungeon_runtime::MapMoveMonster row;
        row.group = ScanInt(&success);
        if (!success) {
            return saw_row && LastTokenIsType("[/map move monster]");
        }

        int* const numeric_fields[] = {
            &row.monster_id,
            &row.level,
            &row.enabled,
            &row.spawn_mode,
            &row.position_x,
            &row.position_y,
            &row.value_18,
            &row.aggregate_value,
            &row.specific_drop_count,
            &row.random_drop_count,
            &row.value_24,
        };
        for (std::size_t index = 0;
             index < sizeof(numeric_fields) / sizeof(numeric_fields[0]);
             ++index) {
            *numeric_fields[index] = ScanInt(&success);
            if (!success) {
                return false;
            }
        }

        std::string rank;
        if (ScanStr(&rank) != 1) {
            return false;
        }
        row.rank = ResolveMapMoveRank(rank);
        row.special_class =
            (row.monster_id >= 50000 && row.monster_id <= 51000) ||
            (row.monster_id >= 56000 && row.monster_id <= 60000)
                ? 1
                : 0;
        row.tail_value = row.value_24;

        if (row.group < 0 || row.monster_id <= 0 || row.level <= 0 ||
            row.level > 255 || row.enabled < 0 || row.enabled > 1 ||
            row.spawn_mode < 0 || row.spawn_mode > 1 ||
            row.position_x < 0 || row.position_y < 0 ||
            row.aggregate_value <= 0 || row.specific_drop_count < 0 ||
            row.specific_drop_count > 255 || row.random_drop_count < 0 ||
            row.random_drop_count > 255 || row.rank < 0 ||
            row.value_24 < 0 || row.value_24 > 1) {
            return false;
        }

        std::vector<dungeon_runtime::MapMoveMonster>& group =
            config->map_moves[row.group];
        int aggregate_total = row.aggregate_value;
        for (std::vector<dungeon_runtime::MapMoveMonster>::const_iterator it =
                 group.begin();
             it != group.end(); ++it) {
            if (it->aggregate_value >
                std::numeric_limits<int>::max() - aggregate_total) {
                return false;
            }
            aggregate_total += it->aggregate_value;
        }
        group.push_back(row);
        saw_row = true;
    }
}

bool ParseRankTypeBlock(DungeonRuntimeConfig* config) {
    if (!config) {
        return false;
    }
    std::string rank_name;
    if (ScanStr(&rank_name) != 1 || rank_name.empty() ||
        config->rank_types.find(rank_name) != config->rank_types.end()) {
        return false;
    }

    std::vector<int> values;
    while (true) {
        bool success = false;
        const int value = ScanInt(&success);
        if (!success) {
            break;
        }
        if (value <= 0) {
            return false;
        }
        values.push_back(value);
    }
    if (values.empty() || !LastTokenIsType("[/rank type]")) {
        return false;
    }
    config->rank_types.insert(std::make_pair(rank_name, values));
    return true;
}

bool ValidateDungeonRuntimeConfig(
    const DungeonRuntimeConfig& config) {
    if (!config.critical_tail_seen) {
        return true;
    }
    if (config.golden_commerce) {
        if (config.map_moves.size() != 1u ||
            config.map_moves.find(0) == config.map_moves.end() ||
            !config.move_map_even_enemy ||
            !config.enter_without_fatigue) {
            return false;
        }
        for (std::map<
                 int,
                 std::vector<dungeon_runtime::MapMoveMonster> >::const_iterator
                 group = config.map_moves.begin();
             group != config.map_moves.end(); ++group) {
            if (group->second.empty()) {
                return false;
            }
            const dungeon_runtime::MapMoveMonster& projection =
                group->second.front();
            int aggregate_total = 0;
            for (std::vector<dungeon_runtime::MapMoveMonster>::const_iterator row =
                     group->second.begin();
                 row != group->second.end(); ++row) {
                if (!SameMapMoveProjection(projection, *row) ||
                    row->aggregate_value <= 0 ||
                    row->aggregate_value >
                        std::numeric_limits<int>::max() -
                                      aggregate_total) {
                    return false;
                }
                aggregate_total += row->aggregate_value;
            }
        }
    }
    return true;
}

bool LastTokenIsType(const char* expected) {
    if (g_dnfLex.getLastToken() != DNF_TOK_TYPE) {
        return false;
    }
    const char* const type = g_dnfLex.getType();
    return type != 0 && std::strcmp(type, expected) == 0;
}

bool ReadOptionalInt(int* output, std::string* pending_tag) {
    bool is_type = false;
    bool success = false;
    std::string text;
    const int value = ScanTypeOrInt(is_type, text, success, true);
    if (is_type) {
        *pending_tag = text;
        return false;
    }
    if (!success) {
        return false;
    }
    *output = value;
    pending_tag->clear();
    return true;
}

bool ReadOptionalFloat(float* output, std::string* pending_tag) {
    bool is_type = false;
    bool success = false;
    std::string text;
    const long double value =
        ScanTypeOrFloat(is_type, text, success, true);
    if (is_type) {
        *pending_tag = text;
        return false;
    }
    if (!success) {
        return false;
    }
    *output = static_cast<float>(value);
    pending_tag->clear();
    return true;
}

bool IsDungeonMazePositionInBounds(
    const MazeScript& maze,
    const std::pair<int, int>& position) {
    return position.first >= 0 && position.second >= 0 &&
           position.first < maze.grid_width &&
           position.second < maze.grid_height;
}

bool IsCompleteDungeonMaze(const MazeScript& maze) {
    if (maze.grid_width <= 0 || maze.grid_height <= 0) {
        return false;
    }

    const std::size_t width =
        static_cast<std::size_t>(maze.grid_width);
    const std::size_t height =
        static_cast<std::size_t>(maze.grid_height);
    if (width > std::numeric_limits<std::size_t>::max() / height ||
        maze.grids.size() != width * height) {
        return false;
    }

    // Preserve the official VerifyMaze policy: 1x1 mazes (including the
    // village-attack (-1,-1) boss sentinel) return before coordinate checks.
    // Do not reject those official records in this earlier structural gate.
    if (maze.grid_width == 1 && maze.grid_height == 1) {
        return true;
    }
    if (maze.values_14.empty() || maze.values_20.empty()) {
        return false;
    }

    for (std::vector<std::pair<int, int> >::const_iterator position =
             maze.values_14.begin();
         position != maze.values_14.end(); ++position) {
        if (!IsDungeonMazePositionInBounds(maze, *position)) {
            return false;
        }
    }
    for (std::vector<std::pair<int, int> >::const_iterator position =
             maze.values_20.begin();
         position != maze.values_20.end(); ++position) {
        if (!IsDungeonMazePositionInBounds(maze, *position)) {
            return false;
        }
    }
    return true;
}

bool CommitCompleteDungeonMaze(std::vector<MazeScript>* mazes,
                               const MazeScript& maze) {
    if (!mazes || !IsCompleteDungeonMaze(maze)) {
        return false;
    }
    // Official ImportDungeonScript stores the current all-maze vector size
    // in MazeScript +0x64 immediately before every push_back.  NOTI28 later
    // sends that ordinal to the client, which indexes the original DGN maze
    // sequence (normal and quest mazes share one namespace).  Leaving the
    // constructor default 0 here makes every quest maze look like maze 0 on
    // the client while the server runs the selected quest maze.
    MazeScript committed(maze);
    committed.maze_index = static_cast<int32_t>(mazes->size());
    mazes->push_back(committed);
    return true;
}

bool IsCompleteSyntheticTowerMaze(const STDungeonScript& script,
                                  const MazeScript& maze) {
    if (maze.grid_width != 2 || maze.grid_height != 2 ||
        maze.grids.size() != 4u) {
        return false;
    }

    if (script.dungeon_kind == 1) {
        const std::size_t maximum_stage =
            static_cast<std::size_t>(script.value_368);
        if (maximum_stage == 0u ||
            script.value_35c.size() != maximum_stage) {
            return false;
        }
        for (std::size_t index = 0; index < maximum_stage; ++index) {
            if (script.value_35c[index].first !=
                static_cast<int>(index + 1u)) {
                return false;
            }
        }
        return true;
    }

    if (script.dungeon_kind == 3) {
        if (script.random_tower_max_stage <= 0 ||
            script.random_tower_map_indexes.size() !=
                static_cast<std::size_t>(script.random_tower_max_stage)) {
            return false;
        }
        for (int stage = 1; stage <= script.random_tower_max_stage; ++stage) {
            const RandomTowerMapIndexTable::const_iterator maps =
                script.random_tower_map_indexes.find(stage);
            if (maps == script.random_tower_map_indexes.end() ||
                maps->second.empty()) {
                return false;
            }
        }
        return true;
    }

    return false;
}

}  // namespace

namespace dungeon_runtime {

bool GetInCountRecords(std::vector<InCountRecord>* records) {
    if (!records || !g_dungeon_incount_config.available) {
        return false;
    }
    records->clear();
    for (std::vector<DungeonInCountRecord>::const_iterator it =
             g_dungeon_incount_config.ordered_records.begin();
         it != g_dungeon_incount_config.ordered_records.end(); ++it) {
        InCountRecord record;
        record.channel_type = it->channel_type;
        record.dungeon_index = it->dungeon_index;
        record.maximum_count = it->maximum_count;
        record.reset_value = it->reset_group;
        records->push_back(record);
    }
    return !records->empty();
}

bool GetLimitedInCountRecords(std::vector<InCountRecord>* records) {
    if (!records || !g_dungeon_incount_config.available) {
        return false;
    }
    records->clear();
    for (std::vector<DungeonInCountRecord>::const_iterator it =
             g_dungeon_incount_config.ordered_records.begin();
         it != g_dungeon_incount_config.ordered_records.end(); ++it) {
        const DungeonRuntimeConfigMap::const_iterator runtime =
            g_dungeon_runtime_configs.find(it->dungeon_index);
        if (runtime == g_dungeon_runtime_configs.end() ||
            !runtime->second.golden_commerce || it->maximum_count <= 0) {
            continue;
        }
        InCountRecord record;
        record.channel_type = it->channel_type;
        record.dungeon_index = it->dungeon_index;
        record.maximum_count = it->maximum_count;
        record.reset_value = it->reset_group;
        records->push_back(record);
    }
    return !records->empty() && records->size() <= 3u;
}

int GetInCountMaximum(int dungeon_index) {
    return ResolveDungeonInCountMaximum(dungeon_index, 0);
}

bool IsLimitedInCountDungeon(int dungeon_index) {
    std::vector<InCountRecord> records;
    if (!GetLimitedInCountRecords(&records)) {
        return false;
    }
    for (std::vector<InCountRecord>::const_iterator it = records.begin();
         it != records.end(); ++it) {
        if (it->dungeon_index == dungeon_index) {
            return true;
        }
    }
    return false;
}

bool IsPowerWarDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.power_war;
}

bool IsGoldenCommerceDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.golden_commerce;
}

bool IsMoveMapEvenEnemyDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.move_map_even_enemy;
}

bool GetFixedDifficulty(int dungeon_index, int* difficulty) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    if (!difficulty || it == g_dungeon_runtime_configs.end() ||
        !it->second.has_fixed_difficulty) {
        return false;
    }
    *difficulty = it->second.fixed_difficulty;
    return true;
}

bool GetMapMoveMonsterProjection(int dungeon_index,
                                 int group,
                                 MapMoveMonster* projection) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    if (!projection || runtime == g_dungeon_runtime_configs.end()) {
        return false;
    }
    const std::map<int, std::vector<MapMoveMonster> >::const_iterator rows =
        runtime->second.map_moves.find(group);
    if (rows == runtime->second.map_moves.end() || rows->second.empty()) {
        return false;
    }

    const MapMoveMonster& common = rows->second.front();
    int accumulated_value = 0;
    for (std::vector<MapMoveMonster>::const_iterator row =
             rows->second.begin();
         row != rows->second.end(); ++row) {
        if (!SameMapMoveProjection(common, *row) ||
            row->aggregate_value <= 0 || row->aggregate_value >
                std::numeric_limits<int>::max() - accumulated_value) {
            return false;
        }
        accumulated_value += row->aggregate_value;
    }
    if (accumulated_value <= 0) {
        return false;
    }
    *projection = common;
    return true;
}

bool IsLimitItemStatDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.limit_item_stat;
}

bool IsLimitEscapeCharacterDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.limit_escape_character;
}

bool UsesCharacterRespawnPosition(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator it =
        g_dungeon_runtime_configs.find(dungeon_index);
    return it != g_dungeon_runtime_configs.end() &&
           it->second.character_respawn_position;
}

bool GetAbsoluteChangeStat(int dungeon_index,
                           AbsoluteChangeStat::T stat,
                           float* value) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    if (!value || runtime == g_dungeon_runtime_configs.end()) {
        return false;
    }
    const std::map<AbsoluteChangeStat::T, float>::const_iterator entry =
        runtime->second.absolute_change_stats.find(stat);
    if (entry == runtime->second.absolute_change_stats.end()) {
        return false;
    }
    *value = entry->second;
    return true;
}

bool GetRealDungeonMonsterCheckup(int dungeon_index,
                                  std::vector<int>* monster_ids) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    if (!monster_ids || runtime == g_dungeon_runtime_configs.end() ||
        runtime->second.realdungeon_checkup_monsters.empty()) {
        return false;
    }
    *monster_ids = runtime->second.realdungeon_checkup_monsters;
    return true;
}

bool GetDungeonZoomRate(int dungeon_index, float* zoom_rate) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    if (!zoom_rate || runtime == g_dungeon_runtime_configs.end() ||
        !runtime->second.has_zoom_rate) {
        return false;
    }
    *zoom_rate = runtime->second.zoom_rate;
    return true;
}

bool IsRoomMoveKeepStateDungeon(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    return runtime != g_dungeon_runtime_configs.end() &&
           runtime->second.room_move_keep_state;
}

bool UsesDungeonCountdown(int dungeon_index) {
    const DungeonRuntimeConfigMap::const_iterator runtime =
        g_dungeon_runtime_configs.find(dungeon_index);
    return runtime != g_dungeon_runtime_configs.end() &&
           runtime->second.use_countdown;
}

}  // namespace dungeon_runtime

namespace map_runtime {

bool GetChaosMapConfig0828(int map_index, ChaosMapConfig0828* config) {
    const MapRuntimeExtension0828Map::const_iterator runtime =
        g_map_runtime_extensions_0828.find(map_index);
    if (!config || runtime == g_map_runtime_extensions_0828.end()) {
        return false;
    }
    const ChaosMapConfig0828& chaos = runtime->second.chaos;
    if (!chaos.chaos_zone && !chaos.has_phase_max &&
        !chaos.has_phase_time && !chaos.has_final_gate_index &&
        !chaos.has_gate_index && !chaos.has_auto_revive_interval &&
        !chaos.has_gate_bonus_fp && !chaos.has_phase_fp) {
        return false;
    }
    *config = chaos;
    return true;
}

bool GetTwinGauge0828(int map_index, TwinGauge0828* gauge) {
    const MapRuntimeExtension0828Map::const_iterator runtime =
        g_map_runtime_extensions_0828.find(map_index);
    if (!gauge || runtime == g_map_runtime_extensions_0828.end() ||
        !runtime->second.has_twin_gauge) {
        return false;
    }
    *gauge = runtime->second.twin_gauge;
    return true;
}

void BeginMapExtensionLoad0828() {
    g_map_runtime_extensions_0828_staging.clear();
    g_map_runtime_extensions_0828_load_active = true;
}

void CommitMapExtensionLoad0828() {
    if (!g_map_runtime_extensions_0828_load_active) {
        return;
    }
    g_map_runtime_extensions_0828.swap(
        g_map_runtime_extensions_0828_staging);
    g_map_runtime_extensions_0828_staging.clear();
    g_map_runtime_extensions_0828_load_active = false;
}

void AbortMapExtensionLoad0828() {
    g_map_runtime_extensions_0828_staging.clear();
    g_map_runtime_extensions_0828_load_active = false;
}

void ClearMapExtensions0828() {
    g_map_runtime_extensions_0828.clear();
    g_map_runtime_extensions_0828_staging.clear();
    g_map_runtime_extensions_0828_load_active = false;
}

}  // namespace map_runtime

// IDA_EA: 0x89E6CF4
// IDA_NAME: _Z20ScanChampionFeaturesiR15STAssignMonster
// DEMANGLED: ScanChampionFeatures(int,STAssignMonster &)
void ScanChampionFeatures(int feature_count, STAssignMonster& monster) {
    std::string feature_name;
    for (int index = 0; index < feature_count; ++index) {
        ScanStr(&feature_name);
        for (int table_index = 0; table_index <= 36; ++table_index) {
            const int32_t feature = CHAMPION_FEATURE_LIST[table_index];
            if (feature_name ==
                CHAMPION_FEATURE_NAME[static_cast<uint8_t>(feature)]) {
                monster.champion_features.push_back(feature);
                break;
            }
        }
    }
}

// IDA_EA: 0x89E6DC4
// IDA_NAME: _Z20ScanChampionFeaturesiR27STAICharacterMapArrangeData
// DEMANGLED: ScanChampionFeatures(int,STAICharacterMapArrangeData &)
void ScanChampionFeatures(int feature_count,
                          STAICharacterMapArrangeData& character) {
    std::string feature_name;
    for (int index = 0; index < feature_count; ++index) {
        ScanStr(&feature_name);
        for (int table_index = 0; table_index <= 36; ++table_index) {
            const int32_t feature = CHAMPION_FEATURE_LIST[table_index];
            if (feature_name ==
                CHAMPION_FEATURE_NAME[static_cast<uint8_t>(feature)]) {
                character.int_values.push_back(feature);
                break;
            }
        }
    }
}

// IDA_EA: 0x836CC94
// IDA_NAME: _ZN11STMapScriptC2Ev
// DEMANGLED: STMapScript::STMapScript(void)
STMapScript::STMapScript() {
    Clear();
}

// IDA_EA: copy ctor emitted by CDataManager::load_map_files @0x835FE02.
// IDA_NAME: _ZN11STMapScriptC2ERKS_
// DEMANGLED: STMapScript::STMapScript(STMapScript const&)
STMapScript::STMapScript(const STMapScript& rhs) = default;

// IDA_EA: destructor emitted by CDataManager::load_map_files @0x835FE02.
// IDA_NAME: _ZN11STMapScriptD2Ev
// DEMANGLED: STMapScript::~STMapScript()
STMapScript::~STMapScript() = default;

// IDA_EA: 0x836C688
// IDA_NAME: _ZN11STMapScript5ClearEv
// DEMANGLED: STMapScript::Clear(void)
STMapScript* STMapScript::Clear() {
    map_index = 0;
    dungeon_indices.clear();
    map_type = 0;
    greed_cells.clear();
    animations.clear();
    quest_animations.clear();
    passive_objects.clear();
    monsters.clear();
    additional_monsters.clear();
    champion = 0;
    items.clear();
    special_passive_objects.clear();
    npcs.clear();
    elemental_property = 4;
    tiles.clear();
    extended_tiles.clear();
    far_sight_scroll = 100;
    middle_sight_scroll = 100;
    near_sight_scroll = 100;
    value_094 = 100;
    value_098 = 100;
    value_09c = 100;
    background_animations.clear();
    background_effects.clear();
    background_effect_blocks.clear();
    dialogs.clear();
    absolute_start_path = 4;
    block_path_0 = 0;
    block_path_1 = 0;
    block_path_2 = 0;
    block_path_3 = 0;
    tile_upper = 0;
    reserved_0d9 = 0;
    reserved_0da = 0;
    reserved_0db = 0;
    town_paths.clear();
    map_areas.clear();
    town_path_links.clear();
    for (int index = 0; index != 4; ++index) {
        pathgate_positions[index][0] = 0;
        pathgate_positions[index][1] = 0;
        pathgate_recognize_ranges[index][0] = 0;
        pathgate_recognize_ranges[index][1] = 0;
    }
    dungeon_start_area.clear();
    value_15c_area.clear();
    value_178 = 0;
    value_17c = 0;
    camera_recovery_objects.clear();
    opening_bgm.clear();
    opening_bgm_index = 0;
    sound_files.clear();
    directory.clear();
    show_dust_left.clear();
    show_dust_right.clear();
    sort_bottom = 0;
    reserved_1ad = 0;
    reserved_1ae = 0;
    reserved_1af = 0;
    map_name.clear();
    player_number_min = 2;
    player_number_max = 8;
    pvp_start_area_0.clear();
    pvp_start_area_1.clear();
    pvp_start_area_2.clear();
    pvp_start_area_3.clear();
    pvp_start_area_4.clear();
    pvp_start_area_by_party_slot = 0;
    map_move_keep_state_axis_x = 0;
    map_move_keep_state_axis_y = 0;
    map_move_keep_state_axis_z = 0;
    shadow_type = 0;
    rival_positions.clear();
    apc_arrangements.clear();
    apc_random_point = 0;
    map_loading_image_path.clear();
    value_270 = 0;
    value_274 = 0;
    event_monster_positions.clear();
    heroes_mode_map_index = 0;
    blood_systems.clear();
    blood_phase_times.clear();
    defense_systems.clear();
    defense_phase_times.clear();
    tournament_monsters.clear();
    tournament_start_area_1.clear();
    tournament_start_area_2.clear();
    static_player_start_positions.clear();
    static_player_start_positions_alt.clear();
    limit_player_move_areas.clear();
    attacked_monsters.clear();
    max_monster_drop_exception = 0;
    max_monster_count_exception = 0;
    background_pos = 0;
    advance_altar_timeline.reset();
    map_dialog.clear();
    value_350 = 4;
    value_354 = -1;
    value_358 = 0;
    tournament_enemy_role_bases[0] = 0;
    tournament_enemy_role_bases[1] = 0;
    reserved_35b = 0;
    customized_screen_edge_left = -1;
    customized_screen_edge_right = -1;
    customized_screen_edge_top = -1;
    customized_screen_edge_bottom = -1;
    value_36c = 0;
    value_36d = 0;
    value_36e = 0;
    reserved_36f = 0;
    value_370.clear();
    value_374 = 1;
    value_378 = 1;
    pathgate_object_ids.clear();
    return this;
}

// IDA_EA: 0x89DCF54
// IDA_NAME: _Z15ImportMapScriptP11STMapScriptPKc
// DEMANGLED: ImportMapScript(STMapScript *,char const*)
int ImportMapScript(STMapScript* script, const char* filename) {
    if (!script ||
        loadRDARScriptFile(g_szMapScriptBaseDirectory,
                                   filename) != 1) {
        std::fprintf(stderr,
                     "[map-parser] RDAR open fail: %s%s\n",
                     g_szMapScriptBaseDirectory,
                     filename ? filename : "(null)");
        return 0;
    }

#define MAP_FAIL() \
    do { \
        std::fprintf(stderr, \
                     "[map-parser] fail file=%s tag=%s line=%d token=%s\n", \
                     filename ? filename : "(null)", \
                     tag.c_str(), \
                     g_dnfLex.getLastLineNo(), \
                     g_dnfLex.getLastTokenDataAsString().c_str()); \
        unloadRDARScriptFile(); \
        return 0; \
    } while (false)

    static unsigned int named_monster_count = 0;
    static unsigned int boss_monster_count = 0;
    static unsigned int apc_boss_count = 0;

    script->Clear();
    MapRuntimeExtension0828 runtime_extension_0828;
    getDirectoryPath(script->directory,
                     "%s%s",
                     g_szMapScriptBaseDirectory,
                     filename);

    std::string tag;
    bool pending = false;
    while (pending || ScanType(tag, true) == 1) {
        if (pending) {
            pending = false;
        }

        if (tag == "[dungeon]") {
            script->dungeon_indices.clear();
            while (1) {
                bool is_type = false;
                bool success = false;
                std::string text;
                const int dungeon =
                    ScanTypeOrInt(is_type, text, success, true);
                if (is_type) {
                    if (text != "[/dungeon]") {
                        tag = text;
                        pending = true;
                    }
                    break;
                }
                if (!success) {
                    break;
                }
                script->dungeon_indices.push_back(dungeon);
            }
        } else if (tag == "[type]") {
            std::string type_tag;
            if (ScanStr(&type_tag) != 1) {
                MAP_FAIL();
            }
            if (type_tag == "[normal]") {
                script->map_type = 0;
            } else if (type_tag == "[boss]") {
                script->map_type = 1;
            } else if (type_tag == "[dummy]") {
                script->map_type = 2;
            } else if (type_tag == "[cover]") {
                // Newer PVFs add one cover-map kind.  Preserve its ordinal
                // in the existing integer ABI: the legacy dungeon router
                // already routes every non-normal, non-dummy type through
                // the boss-map path while retaining the raw map type.
                script->map_type = 3;
            } else {
                MAP_FAIL();
            }
        } else if (tag == "[dungeon start area]") {
            bool success = false;
            const int left = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            const int top = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            const int width = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            const int height = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            script->dungeon_start_area.x = left;
            script->dungeon_start_area.y = top;
            script->dungeon_start_area.width = width;
            script->dungeon_start_area.height = height;
        } else if (tag == "[town movable area]") {
            script->town_paths.clear();
            bool success = false;
            int first_value = ScanInt(&success);
            while (success) {
                TownPath path;
                path.x = first_value;
                path.y = ScanInt(static_cast<bool*>(0));
                path.width = ScanInt(static_cast<bool*>(0));
                path.height = ScanInt(static_cast<bool*>(0));
                path.start_area = ScanInt(static_cast<bool*>(0));
                path.end_area = ScanInt(static_cast<bool*>(0));

                bool is_type = false;
                bool scan_success = false;
                std::string lookahead;
                first_value =
                    ScanTypeOrInt(is_type, lookahead, scan_success, true);
                if (is_type) {
                    if (lookahead == "[quest condition]") {
                        int quest_index = ScanInt(&success);
                        while (success) {
                            path.path_nodes.push_back(quest_index);
                            quest_index = ScanInt(&success);
                        }
                        first_value = ScanInt(&success);
                    } else {
                        success = false;
                    }
                } else {
                    success = scan_success;
                }
                script->town_paths.push_back(path);
            }
        } else if (tag == "[virtual movable area]") {
            script->map_areas.clear();
            bool success = false;
            int first_value = ScanInt(&success);
            while (success) {
                MapArea area;
                area.x = first_value;
                area.y = ScanInt(static_cast<bool*>(0));
                area.width = ScanInt(static_cast<bool*>(0));
                area.height = ScanInt(static_cast<bool*>(0));

                bool is_type = false;
                bool scan_success = false;
                std::string lookahead;
                first_value =
                    ScanTypeOrInt(is_type, lookahead, scan_success, true);
                if (is_type) {
                    if (lookahead == "[quest condition]") {
                        int quest_index = ScanInt(&success);
                        while (success) {
                            area.map_indices.push_back(quest_index);
                            quest_index = ScanInt(&success);
                        }
                        first_value = ScanInt(&success);
                    } else {
                        success = false;
                    }
                } else {
                    success = scan_success;
                }
                script->map_areas.push_back(area);
            }
        } else if (tag == "[guild agit entrance info]") {
            script->town_path_links.clear();
            while (true) {
                bool success = false;
                const int first_value = ScanInt(&success);
                if (!success) {
                    break;
                }
                TownPath path;
                path.x = first_value;
                path.y = ScanInt(static_cast<bool*>(0));
                path.width = ScanInt(static_cast<bool*>(0));
                path.height = ScanInt(static_cast<bool*>(0));
                path.start_area = ScanInt(static_cast<bool*>(0));
                path.end_area = ScanInt(static_cast<bool*>(0));
                script->town_path_links.push_back(path);
            }
        } else if (tag == "[pathgate pos]") {
            for (int index = 0; index != 4; ++index) {
                script->pathgate_positions[index][0] =
                    ScanInt(static_cast<bool*>(0));
                script->pathgate_positions[index][1] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[pathgate recognize range]") {
            for (int index = 0; index != 4; ++index) {
                script->pathgate_recognize_ranges[index][0] =
                    ScanInt(static_cast<bool*>(0));
                script->pathgate_recognize_ranges[index][1] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[pathgate info]") {
            // The newer PVF contributes an explicit [disable] marker.  The
            // recovered STMapScript/CMap ABI has only the two pathgate
            // geometry arrays above and no flag/member consumer, so retain
            // that legacy server state while consuming this closed extension
            // block exactly.
            std::string info_tag;
            if (ScanType(info_tag, true) != 1 ||
                info_tag != "[disable]" ||
                ScanType(info_tag, true) != 1 ||
                info_tag != "[/pathgate info]") {
                MAP_FAIL();
            }
        } else if (tag == "[buff]") {
            // Map-local buff identifiers were added after the legacy map
            // object ABI.  There is no list member or server consumer to
            // populate in this build; consume the closed integer sequence
            // exactly rather than manufacture a parallel state container.
            while (true) {
                bool success = false;
                (void)ScanInt(&success);
                if (!success) {
                    break;
                }
            }
        } else if (tag == "[pathgate object]") {
            // CLIENT_EA: ImportMapScript @0x161810E.  The target PVF carries
            // an unclosed repeated passive-object ID list (for example
            // 361..370 in the Elvengard pathgate maps).  Preserve the next
            // top-level tag with the existing pending-tag mechanism.
            script->pathgate_object_ids.clear();
            while (true) {
                int value = 0;
                float float_value = 0.0f;
                std::string next;
                const DNFLEX_TOKEN token = ScanAll(&value, &float_value, &next);
                if (token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL) {
                    script->pathgate_object_ids.push_back(value);
                    continue;
                }
                if (token == DNF_TOK_TYPE) {
                    tag = next;
                    pending = true;
                    break;
                }
                if (token == DNF_TOK_EOF) {
                    break;
                }
                MAP_FAIL();
            }
        } else if (tag == "[before rendering info]") {
            // A presentation-only transition block from newer PVFs.  Its
            // target map, monster-lock mode, and delay have no slots in the
            // IDA-recovered server map ABI, but their grammar is validated
            // here to preserve scanner alignment.
            std::string rendering_tag;
            std::string monster_lock;
            bool success = false;
            if (ScanType(rendering_tag, true) != 1 ||
                rendering_tag != "[map index]") {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success || ScanType(rendering_tag, true) != 1 ||
                rendering_tag != "[monster lock]" ||
                ScanStr(&monster_lock) != 1 ||
                (monster_lock != "true" && monster_lock != "false") ||
                ScanType(rendering_tag, true) != 1 ||
                rendering_tag != "[change delay]") {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success || ScanType(rendering_tag, true) != 1 ||
                rendering_tag != "[/before rendering info]") {
                MAP_FAIL();
            }
        } else if (tag == "[character escape stuck move pos]") {
            // Newer map data can supply a client-side anti-stuck fallback
            // position.  The legacy CMap ABI has no corresponding members or
            // movement consumer; validate the closed coordinate pair only.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            std::string close_tag;
            if (!success || ScanType(close_tag, true) != 1 ||
                close_tag != "[/character escape stuck move pos]") {
                MAP_FAIL();
            }
        } else if (tag == "[map over move ani]") {
            // This newer PVF block is client presentation data.  Keep the
            // legacy server ABI intact, but validate and consume its full
            // grammar so it cannot desynchronize subsequent map fields.
            std::string animation_tag;
            std::string ignored_text;
            bool success = false;
            if (ScanType(animation_tag, true) != 1 ||
                animation_tag != "[ani]" ||
                ScanStr(&ignored_text) != 1 ||
                ScanStr(&ignored_text) != 1 ||
                ScanType(animation_tag, true) != 1 ||
                animation_tag != "[start pos]") {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success || ScanType(animation_tag, true) != 1 ||
                animation_tag != "[end pos]") {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success || ScanType(animation_tag, true) != 1 ||
                animation_tag != "[move duration time]") {
                MAP_FAIL();
            }
            (void)ScanInt(&success);
            if (!success || ScanType(animation_tag, true) != 1 ||
                animation_tag != "[/map over move ani]") {
                MAP_FAIL();
            }
        } else if (tag == "[chaos zone]") {
            // Official 140828 chaostest/chaos_20.map uses this as a valueless
            // top-level marker.  No 0627 gameplay consumer was recovered, so
            // retain only typed metadata and require the next token boundary.
            if (!FinishStrictMapExtensionField(&tag, &pending)) {
                MAP_FAIL();
            }
            runtime_extension_0828.chaos.chaos_zone = true;
        } else if (tag == "[chaos phase max]" ||
                   tag == "[chaos phase time]" ||
                   tag == "[chaos final gate index]" ||
                   tag == "[chaos gate index]" ||
                   tag == "[auto revive interval]" ||
                   tag == "[chaos gate bonus fp]" ||
                   tag == "[chaos phase fp]") {
            // These seven official fields are one integer each.  Do not add
            // guessed range or gameplay rules: the 0627 server has no owner.
            const std::string current_tag(tag);
            int value = 0;
            if (!ScanStrictMapExtensionScalar(&value, &tag, &pending)) {
                MAP_FAIL();
            }
            map_runtime::ChaosMapConfig0828& chaos =
                runtime_extension_0828.chaos;
            if (current_tag == "[chaos phase max]") {
                chaos.has_phase_max = true;
                chaos.phase_max = value;
            } else if (current_tag == "[chaos phase time]") {
                chaos.has_phase_time = true;
                chaos.phase_time = value;
            } else if (current_tag == "[chaos final gate index]") {
                chaos.has_final_gate_index = true;
                chaos.final_gate_index = value;
            } else if (current_tag == "[chaos gate index]") {
                chaos.has_gate_index = true;
                chaos.gate_index = value;
            } else if (current_tag == "[auto revive interval]") {
                chaos.has_auto_revive_interval = true;
                chaos.auto_revive_interval = value;
            } else if (current_tag == "[chaos gate bonus fp]") {
                chaos.has_gate_bonus_fp = true;
                chaos.gate_bonus_fp = value;
            } else {
                chaos.has_phase_fp = true;
                chaos.phase_fp = value;
            }
        } else if (tag == "[twin gauge]") {
            // CLIENT_EA: DNF.exe ImportMapScript @0x1612B90, pseudocode
            // lines 3786..3806.  It reads exactly two integers, closes the
            // field, and writes +1468/+1472 only when both values are positive.
            bool success = false;
            const int first = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            const int second = ScanInt(&success);
            std::string close_tag;
            if (!success || ScanType(close_tag, true) != 1 ||
                close_tag != "[/twin gauge]") {
                MAP_FAIL();
            }
            if (first > 0 && second > 0) {
                runtime_extension_0828.has_twin_gauge = true;
                runtime_extension_0828.twin_gauge.first_monster_index = first;
                runtime_extension_0828.twin_gauge.second_monster_index = second;
            }
        } else if (tag == "[map name]") {
            if (ScanStr(&script->map_name) != 1) {
                MAP_FAIL();
            }
        } else if (tag == "[player number]") {
            bool success = false;
            script->player_number_min = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            script->player_number_max = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[background pos]") {
            bool success = false;
            script->background_pos = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[background correction]") {
            // CLIENT_EA 0x161852F: the newer client importer reads one signed
            // integer into its map object +0x688.  The recovered server map
            // ABI has no corresponding field or consumer; do not alias it to
            // the distinct legacy [background pos] member.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[greed]") {
            script->greed_cells.clear();

            std::string greed_text;
            ScanStr(&greed_text);

            std::size_t cursor = 0;
            int invalid_symbol_count = 0;
            int first_grid_index = -1;
            while (cursor + 1u < greed_text.size()) {
                const std::string grid_symbol =
                    greed_text.substr(cursor, 2u);
                if (grid_symbol[0] == '\r' || grid_symbol[0] == '\n' ||
                    grid_symbol[0] == ' ') {
                    ++cursor;
                    continue;
                }

                int grid_index = 0;
                for (; grid_index != 16; ++grid_index) {
                    if (grid_symbol[0] == g_gridSymbols[grid_index][0] &&
                        grid_symbol[1] == g_gridSymbols[grid_index][1]) {
                        break;
                    }
                }
                if (grid_index == 16) {
                    ++cursor;
                    ++invalid_symbol_count;
                    continue;
                }

                if (first_grid_index == -1) {
                    first_grid_index = grid_index;
                } else {
                    script->greed_cells.push_back(
                        std::make_pair(first_grid_index, grid_index));
                    first_grid_index = -1;
                }
                cursor += 2u;
            }
            if (invalid_symbol_count != 0) {
                MAP_FAIL();
            }
        } else if (tag == "[tile]") {
            script->tiles.clear();
            std::string value;
            while (ScanStr(&value) == 1) {
                script->tiles.push_back(value);
            }
        } else if (tag == "[extended tile]") {
            script->extended_tiles.clear();
            std::string value;
            while (ScanStr(&value) == 1) {
                script->extended_tiles.push_back(value);
            }
        } else if (tag == "[elemental property]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                MAP_FAIL();
            }
            script->elemental_property = getElement(value.c_str());
        } else if (tag == "[far sight scroll]") {
            bool success = false;
            script->far_sight_scroll = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[middle sight scroll]") {
            bool success = false;
            script->middle_sight_scroll = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[near sight scroll]") {
            bool success = false;
            script->near_sight_scroll = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[background animation]") {
            script->background_animations.clear();
            while (true) {
                std::string nested_tag;
                if (ScanType(nested_tag, true) != 1) {
                    MAP_FAIL();
                }
                if (nested_tag == "[/background animation]") {
                    break;
                }
                if (nested_tag != "[ani info]") {
                    MAP_FAIL();
                }

                BackAniScript animation;
                while (true) {
                    if (ScanType(nested_tag, true) != 1) {
                        MAP_FAIL();
                    }
                    if (nested_tag == "[/ani info]") {
                        script->background_animations.push_back(animation);
                        break;
                    }
                    if (nested_tag == "[filename]") {
                        if (ScanStr(&animation.animation_name) != 1) {
                            MAP_FAIL();
                        }
                    } else if (nested_tag == "[layer]") {
                        std::string layer;
                        if (ScanStr(&layer) != 1) {
                            MAP_FAIL();
                        }
                        animation.layer = getDrawLayer(layer.c_str());
                    } else if (nested_tag == "[order]") {
                        std::string order;
                        if (ScanStr(&order) != 1) {
                            MAP_FAIL();
                        }
                        if (order == "[below]") {
                            animation.flag_08 = 0;
                        } else if (order == "[upper]") {
                            animation.flag_08 = 1;
                        } else {
                            MAP_FAIL();
                        }
                    } else if (nested_tag == "[loop y axis]") {
                        bool success = false;
                        animation.flag_09 = ScanInt(&success) != 0;
                        if (!success) {
                            MAP_FAIL();
                        }
                    } else if (nested_tag == "[screen pos]") {
                        bool success = false;
                        animation.value_0c = ScanInt(&success);
                        if (!success) {
                            MAP_FAIL();
                        }
                        animation.value_10 = ScanInt(&success);
                        if (!success) {
                            MAP_FAIL();
                        }
                        animation.flag_0a = 1;
                    } else {
                        MAP_FAIL();
                    }
                }
            }
        } else if (tag == "[background effect]") {
            script->background_effects.clear();
            std::string effect_tag;
            while (ScanStr(&effect_tag) == 1) {
                BackGroundEffect effect;
                if (effect_tag == "[swing]") {
                    std::string layer;
                    if (ScanStr(&layer) != 1) {
                        MAP_FAIL();
                    }
                    effect.effect_type = 0;
                    effect.swing_type = getDrawLayer(layer.c_str());
                    bool success = false;
                    effect.position_x = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.position_y = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.position_z = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.layer = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.value_18 = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    ScanStr(&effect.filename);
                } else if (effect_tag == "[dust]") {
                    std::string layer;
                    if (ScanStr(&layer) != 1) {
                        MAP_FAIL();
                    }
                    effect.effect_type = 1;
                    effect.swing_type = getDrawLayer(layer.c_str());
                    bool success = false;
                    effect.position_y = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.position_z = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    effect.layer = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                } else {
                    MAP_FAIL();
                }
                script->background_effects.push_back(effect);
            }
        } else if (tag == "[block effect]") {
            script->background_effect_blocks.clear();
            std::string block_tag;
            while (ScanStr(&block_tag) == 1) {
                BackGroundEffectBlock block;
                if (block_tag == "[swing]") {
                    block.block_type = 0;
                } else if (block_tag == "[dust]") {
                    block.block_type = 1;
                } else {
                    MAP_FAIL();
                }

                std::string source_tag;
                if (ScanStr(&source_tag) != 1) {
                    MAP_FAIL();
                }
                if (source_tag == "[object]") {
                    block.position_x = 0;
                    bool success = false;
                    block.position_y = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                } else if (source_tag == "[animation]") {
                    block.position_x = 1;
                    if (ScanStr(&block.filename) != 1) {
                        MAP_FAIL();
                    }
                } else {
                    MAP_FAIL();
                }
                script->background_effect_blocks.push_back(block);
            }
        } else if (tag == "[absolute start path]") {
            script->absolute_start_path = 4;
            std::string direction;
            while (ScanStr(&direction) == 1) {
                if (direction == "[left]") {
                    script->absolute_start_path = 0;
                } else if (direction == "[right]") {
                    script->absolute_start_path = 1;
                } else if (direction == "[up]") {
                    script->absolute_start_path = 2;
                } else if (direction == "[down]") {
                    script->absolute_start_path = 3;
                }
            }
        } else if (tag == "[block path]") {
            uint8_t* paths[] = {
                &script->block_path_0,
                &script->block_path_1,
                &script->block_path_2,
                &script->block_path_3
            };
            std::string state;
            for (int index = 0; index != 4 && ScanStr(&state) == 1;
                 ++index) {
                *paths[index] = state == "[close]";
            }
        } else if (tag == "[tile option]") {
            std::string option;
            while (ScanStr(&option) == 1) {
                if (option == "[upper]") {
                    script->tile_upper = 1;
                }
            }
        } else if (tag == "[shadow type]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                MAP_FAIL();
            }
            if (value == "[normal]") {
                script->shadow_type = 0;
            } else if (value == "[mirror]") {
                script->shadow_type = 1;
            } else if (value == "[more darker]") {
                script->shadow_type = 2;
            } else {
                MAP_FAIL();
            }
        } else if (tag == "[show dust]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                MAP_FAIL();
            }
            script->show_dust_left.append(value);
            if (ScanStr(&value) != 1) {
                MAP_FAIL();
            }
            script->show_dust_right.append(value);
        } else if (tag == "[sort bottom]") {
            bool success = false;
            script->sort_bottom = ScanInt(&success) == 1;
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[pvp start area]") {
            MapArea* areas[] = {
                &script->pvp_start_area_0,
                &script->pvp_start_area_1,
                &script->pvp_start_area_2
            };
            for (int i = 0; i != 3; ++i) {
                // Original 0x89DD246..0x89DD2D3 reads four coordinates per
                // area through the nullable-status overload.  Zero is a
                // valid coordinate, not a parse error.
                areas[i]->x = ScanInt(static_cast<bool*>(0));
                areas[i]->y = ScanInt(static_cast<bool*>(0));
                areas[i]->width = ScanInt(static_cast<bool*>(0));
                areas[i]->height = ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[pvp start area by party slot]") {
            bool success = false;
            script->pvp_start_area_by_party_slot = ScanInt(&success) != 0;
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[map move keep state axis pos]") {
            bool success = false;
            script->map_move_keep_state_axis_x = ScanInt(&success) != 0;
            if (!success) {
                MAP_FAIL();
            }
            script->map_move_keep_state_axis_y = ScanInt(&success) != 0;
            if (!success) {
                MAP_FAIL();
            }
            script->map_move_keep_state_axis_z = ScanInt(&success) != 0;
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[pvp practice start area]") {
            MapArea* areas[] = {
                &script->pvp_start_area_3,
                &script->pvp_start_area_4
            };
            for (int i = 0; i != 2; ++i) {
                bool success = false;
                areas[i]->x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                areas[i]->y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                areas[i]->width = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                areas[i]->height = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
            }
        } else if (tag == "[opening bgm]") {
            if (ScanStr(&script->opening_bgm) != 1) {
                MAP_FAIL();
            }
            bool success = false;
            script->opening_bgm_index = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[sound]") {
            script->sound_files.clear();
            std::string value;
            while (ScanStr(&value) == 1) {
                script->sound_files.push_back(value);
            }
        } else if (tag == "[animation]") {
            script->animations.clear();
            STAssignAnimation animation;
            bool have_animation_name = false;
            bool animation_section_done = false;
            while (!animation_section_done) {
                if (!have_animation_name &&
                    ScanStr(&animation.animation_name) != 1) {
                    break;
                }
                have_animation_name = false;

                std::string layer;
                if (ScanStr(&layer) != 1) {
                    MAP_FAIL();
                }
                animation.layer = getDrawLayer(layer.c_str());
                bool success = false;
                animation.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                animation.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                animation.position_z = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->animations.push_back(animation);

                // The legacy STAssignAnimation ABI ends after the three
                // integer values above.  Later PVFs may append one visual
                // value (for example DecayArea/18901); it has no backing
                // field or server-side consumer in the recovered ABI.  Read
                // its token explicitly so the next animation name or the
                // [/animation] marker remains aligned.
                int extension_value = 0;
                float extension_float = 0.0f;
                std::string next;
                const DNFLEX_TOKEN next_token =
                    ScanAll(&extension_value, &extension_float, &next);
                if (next_token == DNF_TOK_DECIMAL ||
                    next_token == DNF_TOK_HEXADECIMAL) {
                    continue;
                }
                if (next_token == DNF_TOK_STRING) {
                    animation.animation_name = next;
                    have_animation_name = true;
                    continue;
                }
                if (next_token == DNF_TOK_TYPE) {
                    if (next == "[/animation]") {
                        animation_section_done = true;
                    } else {
                        tag = next;
                        pending = true;
                        animation_section_done = true;
                    }
                    continue;
                }
                MAP_FAIL();
            }
        } else if (tag == "[/animation]") {
            // The currently deployed PVF can emit the generic animation
            // close marker twice.  The first one is consumed by the bounded
            // legacy animation reader above; the second is a syntactic
            // sentinel and has no STMapScript state to apply.
            continue;
        } else if (tag == "[passive object]") {
            script->passive_objects.clear();
            while (true) {
                STAssignPassiveObject object;
                bool success = false;
                object.object_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                object.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                object.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                object.position_z = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->passive_objects.push_back(object);
            }
        } else if (tag == "[monster]" || tag == "[monster condition]") {
            std::vector<STAssignMonster>* monsters = &script->monsters;
            if (tag == "[monster]") {
                script->monsters.clear();
            } else {
                monsters = &script->additional_monsters;
                script->additional_monsters.clear();
            }

            while (true) {
                bool success = false;
                const int monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                STAssignMonster monster;
                monster.Clear();
                monster.monster_id = monster_id;
                if (((monster_id > 49999 && monster_id <= 51000) ||
                     (monster_id > 55999 && monster_id <= 60000)) &&
                    ++named_monster_count > 0x1770u) {
                    MAP_FAIL();
                }

                monster.enabled = ScanInt(static_cast<bool*>(0)) != 0;
                monster.position_x = ScanInt(static_cast<bool*>(0));
                monster.position_y = ScanInt(static_cast<bool*>(0));
                monster.position_z = ScanInt(static_cast<bool*>(0));
                monster.direction = ScanInt(static_cast<bool*>(0));
                monster.summon_count = ScanInt(static_cast<bool*>(0));
                monster.respawn_delay = ScanInt(static_cast<bool*>(0));

                std::string spawn_type;
                if (ScanStr(&spawn_type) != 1) {
                    MAP_FAIL();
                }
                if (spawn_type == "[fixed]") {
                    monster.spawn_type = 0;
                } else if (spawn_type == "[half]") {
                    monster.spawn_type = 1;
                } else if (spawn_type == "[ratio]") {
                    monster.spawn_type = 2;
                }

                std::string rank_type;
                if (ScanStr(&rank_type) != 1) {
                    MAP_FAIL();
                }
                if (rank_type == "[NPC]") {
                    monster.npc_id = ScanInt(static_cast<bool*>(0));
                    if (ScanStr(&rank_type) != 1) {
                        MAP_FAIL();
                    }
                }
                if (rank_type == "[normal]") {
                    monster.rank_type = 0;
                } else if (rank_type == "[champion]") {
                    monster.rank_type = 1;
                    ScanChampionFeatures(
                        ScanInt(static_cast<bool*>(0)), monster);
                } else if (rank_type == "[super champion]") {
                    monster.rank_type = 2;
                    ScanChampionFeatures(
                        ScanInt(static_cast<bool*>(0)), monster);
                } else if (rank_type == "[boss]") {
                    if (++boss_monster_count > 0x5DCu) {
                        MAP_FAIL();
                    }
                    monster.rank_type = 3;
                } else {
                    MAP_FAIL();
                }

                monster.specific_ai_type = 0;
                monster.spawn_order = static_cast<int32_t>(
                    script->monsters.size() +
                    script->additional_monsters.size());
                monsters->push_back(monster);
            }
        } else if (tag == "[monster team]") {
            for (std::vector<STAssignMonster>::iterator monster =
                     script->monsters.begin();
                 monster != script->monsters.end();
                 ++monster) {
                monster->team = ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[/monster team]") {
            // [monster team] has a value for each already-imported monster.
            // Its terminal token remains in the outer scanner once the
            // bounded value loop completes.
            continue;
        } else if (tag == "[monster specific AI]") {
            std::size_t monster_index = 0;
            while (true) {
                bool is_type = false;
                std::string ai_type;
                if (ScanTypeOrStr(is_type, ai_type, true) != 1) {
                    MAP_FAIL();
                }
                if (is_type) {
                    if (ai_type == "[/monster specific AI]") {
                        break;
                    }
                    tag = ai_type;
                    pending = true;
                    break;
                }

                STAssignMonster* monster = monster_index < script->monsters.size()
                    ? &script->monsters[monster_index] : 0;
                if (ai_type == "[normal]") {
                    if (monster != 0) {
                        monster->specific_ai_type = 0;
                    }
                } else if (ai_type == "[zone defence]") {
                    const int value_0 = ScanInt(static_cast<bool*>(0));
                    const int value_1 = ScanInt(static_cast<bool*>(0));
                    if (monster != 0) {
                        monster->specific_ai_type = 1;
                        monster->specific_ai_value_0 = value_0;
                        monster->specific_ai_value_1 = value_1;
                    }
                } else if (ai_type == "[roaming]") {
                    const int point_count = ScanInt(static_cast<bool*>(0));
                    if (monster != 0) {
                        monster->specific_ai_type = 2;
                    }
                    for (int point_index = 0;
                         point_index < point_count;
                         ++point_index) {
                        const int position_x = ScanInt(static_cast<bool*>(0));
                        const int position_y = ScanInt(static_cast<bool*>(0));
                        if (monster != 0) {
                            monster->roaming_points.push_back(
                                std::make_pair(position_x, position_y));
                        }
                    }
                } else {
                    MAP_FAIL();
                }
                ++monster_index;
            }
        } else if (tag == "[monster collision group]") {
            for (std::vector<STAssignMonster>::iterator monster =
                     script->monsters.begin();
                 monster != script->monsters.end();
                 ++monster) {
                monster->collision_values.clear();
                bool success = false;
                const int count = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                for (int value_index = 0; value_index < count; ++value_index) {
                    const int value = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    monster->collision_values.push_back(
                        static_cast<uint8_t>(value));
                }
            }
        } else if (tag == "[apc collision group]") {
            for (std::vector<STAICharacterMapArrangeData>::iterator apc =
                     script->apc_arrangements.begin();
                 apc != script->apc_arrangements.end();
                 ++apc) {
                apc->byte_values.clear();
                bool success = false;
                const int count = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                for (int value_index = 0; value_index < count; ++value_index) {
                    const int value = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                    apc->byte_values.push_back(static_cast<uint8_t>(value));
                }
            }
        } else if (tag == "[champion]") {
            bool success = false;
            script->champion = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[special passive object]") {
            std::list<STAssignItem> item_assignments(script->items);
            (void)item_assignments;
            script->special_passive_objects.clear();
            int special_item_action_index = 0;
            while (true) {
                bool success = false;
                const int object_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                STAssignSpecialPassiveObj object;
                object.Clear();
                object.object_id = object_id;
                object.position_x = ScanInt(static_cast<bool*>(0));
                object.position_y = ScanInt(static_cast<bool*>(0));
                object.position_z = ScanInt(static_cast<bool*>(0));
                const int action_count = ScanInt(static_cast<bool*>(0));
                bool has_item_action = false;
                for (int action_index = 0;
                     action_index < action_count;
                     ++action_index) {
                    SPOBJ_ACTION_DATAS action;
                    const int map_index =
                        g_stMapScriptFileList.FindIndexByFullScan(filename);
                    procSpecialPassiveObject(action, object, map_index);
                    if (action.getActionType() == 2) {
                        has_item_action = true;
                    }
                    if (action.getActionType() != 5) {
                        object.actions.push_back(action);
                    }
                }
                if (has_item_action) {
                    object.action_index = special_item_action_index++;
                }
                script->special_passive_objects.push_back(object);
            }
        } else if (tag == "[NPC]") {
            script->npcs.clear();
            while (true) {
                AssignNpc npc;
                bool success = false;
                npc.npc_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                bool is_string = false;
                std::string direction;
                const int dungeon_id =
                    ScanStrOrInt(is_string, direction, success, true);
                if (!success) {
                    MAP_FAIL();
                }
                if (!is_string) {
                    npc.dungeon_id = dungeon_id;
                    if (ScanStr(&direction) != 1) {
                        MAP_FAIL();
                    }
                }
                if (direction == "[visible on dungeon clear]") {
                    npc.flag = 1;
                    if (ScanStr(&direction) != 1) {
                        MAP_FAIL();
                    }
                }
                if (direction == "[left]") {
                    npc.area = 0;
                } else if (direction == "[right]") {
                    npc.area = 1;
                }
                npc.x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                npc.y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                npc.z = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->npcs.push_back(npc);
            }
        } else if (tag == "[pvp channel limit npc]") {
            // Newer PVFs add a single NPC ID for a client-only PvP-channel
            // presentation restriction.  The old ImportMapScript binary and
            // its 0x37c STMapScript ABI have neither this tag nor a storage
            // member/consumer.  Validate the complete closed record without
            // creating state that the legacy server cannot own.
            bool success = false;
            (void)ScanInt(&success);
            std::string close_tag;
            if (!success || ScanType(close_tag, true) != 1 ||
                close_tag != "[/pvp channel limit npc]") {
                MAP_FAIL();
            }
        } else if (tag == "[wide mode camera vertical correction]") {
            // This post-legacy wide-screen camera correction is one
            // presentation integer.  It has no counterpart in the old map
            // object ABI, so retain scanner alignment without manufacturing
            // a server-only camera state.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[limit map camera move]") {
            // Later PVFs add client camera-edge limits.  The legacy server
            // neither stores nor consumes them; nevertheless require the
            // documented edge forms and their complete coordinate pairs.
            while (true) {
                std::string edge_tag;
                if (ScanType(edge_tag, true) != 1) {
                    MAP_FAIL();
                }
                if (edge_tag == "[/limit map camera move]") {
                    break;
                }
                if (edge_tag != "[left edge]" &&
                    edge_tag != "[right edge]" &&
                    edge_tag != "[top edge]" &&
                    edge_tag != "[bottom edge]") {
                    MAP_FAIL();
                }
                bool success = false;
                (void)ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                (void)ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
            }
        } else if (tag == "[monster spawn pos]") {
            bool success = false;
            const int count = ScanInt(&success);
            if (!success || count < 0) {
                MAP_FAIL();
            }
            script->rival_positions.clear();
            script->rival_positions.reserve(3 * count);
            for (int index = 0; index < count; ++index) {
                STRivalMapData position;
                position.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                position.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                position.spawn_weight = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->rival_positions.push_back(position);
            }
        } else if (tag == "[ai character]") {
            while (true) {
                bool success = false;
                const int character_id = ScanInt(&success);
                if (!success) {
                    break;
                }

                STAICharacterMapArrangeData character;
                character.Clear();
                character.raw_00 = character_id;
                character.raw_04 = ScanInt(static_cast<bool*>(0));
                character.raw_08 = ScanInt(static_cast<bool*>(0));
                character.raw_0c = ScanInt(static_cast<bool*>(0));

                std::string category;
                bool category_is_type = false;
                if (ScanTypeOrStr(category_is_type, category, true) != 1) {
                    MAP_FAIL();
                }
                if (category == "[character]") {
                    character.raw_10 = 0;
                } else if (category == "[monster]") {
                    character.raw_10 = 100;
                } else if (category == "[neutral]") {
                    character.raw_10 = 200;
                }

                std::string rank_type;
                const int rank_read = ScanStr(&rank_type);
                if (rank_type == "[NPC]") {
                    character.raw_28 = ScanInt(static_cast<bool*>(0));
                    if (ScanStr(&rank_type) != 1) {
                        MAP_FAIL();
                    }
                }
                if (rank_read != 1) {
                    rank_type = "[normal]";
                }

                if (rank_type == "[normal]") {
                    character.raw_14 = 5;
                } else if (rank_type == "[champion]") {
                    character.raw_14 = 6;
                    ScanChampionFeatures(
                        ScanInt(static_cast<bool*>(0)), character);
                } else if (rank_type == "[super champion]") {
                    character.raw_14 = 7;
                    ScanChampionFeatures(
                        ScanInt(static_cast<bool*>(0)), character);
                } else if (rank_type == "[boss]") {
                    if (++apc_boss_count > 0x5DCu) {
                        MAP_FAIL();
                    }
                    character.raw_14 = 8;
                } else {
                    MAP_FAIL();
                }

                int byte_value = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                character.raw_18 = static_cast<uint8_t>(byte_value);
                byte_value = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                character.raw_19 = static_cast<uint8_t>(byte_value);
                script->apc_arrangements.push_back(character);
            }
        } else if (tag == "[center index]") {
            bool success = false;
            script->value_178 = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[camera edge exception]") {
            bool success = false;
            script->value_17c = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[camera edge recovery object]") {
            script->camera_recovery_objects.clear();
            while (true) {
                bool success = false;
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->camera_recovery_objects.push_back(
                    std::make_pair(first, second));
            }
        } else if (tag == "[heroes mode map index]") {
            bool success = false;
            script->heroes_mode_map_index = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[apc random point]") {
            bool success = false;
            script->apc_random_point = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[event monster position]") {
            script->event_monster_positions.clear();
            while (true) {
                STRivalMapData value;
                bool success = false;
                value.position_x = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.spawn_weight = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->event_monster_positions.push_back(value);
            }
        } else if (tag == "[map loading image path]") {
            if (ScanStr(&script->map_loading_image_path) != 1) {
                MAP_FAIL();
            }
        } else if (tag == "[max monster drop exception]") {
            bool success = false;
            script->max_monster_drop_exception = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[max monster count exception]") {
            bool success = false;
            script->max_monster_count_exception = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[time line]") {
            const int result =
                reinterpret_cast<advancealtar::StageTimeLineParameter*>(
                    &script->advance_altar_timeline)
                    ->importPhase(script->advance_altar_timeline);
            if (result != 0) {
                MAP_FAIL();
            }
        } else if (tag == "[map dialog]") {
            if (ScanStr(&script->map_dialog) != 1) {
                MAP_FAIL();
            }
        } else if (tag == "[block use active skill]") {
            script->value_36c = 1;
        } else if (tag == "[block use stackable item]") {
            script->value_36d = 1;
            // Legacy maps use this as a leaf flag.  Newer PVFs can instead
            // append a closed [exceptions] item-id list.  The recovered
            // server ABI has only the flag above (and no exception-list
            // member/consumer), therefore validate the extension precisely
            // while preserving the original server-side flag semantics.
            int value = 0;
            float float_value = 0.0f;
            std::string next;
            const DNFLEX_TOKEN first_token =
                ScanAll(&value, &float_value, &next);
            if (first_token == DNF_TOK_TYPE && next == "[exceptions]") {
                while (true) {
                    const DNFLEX_TOKEN token =
                        ScanAll(&value, &float_value, &next);
                    if (token == DNF_TOK_DECIMAL ||
                        token == DNF_TOK_HEXADECIMAL) {
                        continue;
                    }
                    if (token != DNF_TOK_TYPE || next != "[/exceptions]") {
                        MAP_FAIL();
                    }
                    break;
                }
                if (ScanAll(&value, &float_value, &next) != DNF_TOK_TYPE ||
                    next != "[/block use stackable item]") {
                    MAP_FAIL();
                }
            } else if (first_token == DNF_TOK_TYPE &&
                       next == "[/block use stackable item]") {
                // Explicitly closed empty block: the closing token belongs
                // to this field and must not be replayed as a top-level tag.
            } else if (first_token == DNF_TOK_TYPE) {
                tag = next;
                pending = true;
            } else if (first_token != DNF_TOK_EOF) {
                MAP_FAIL();
            }
        } else if (tag == "[fix champion]") {
            script->value_36e = 1;
        } else if (tag == "[basic action]") {
            if (ScanStr(&script->value_370) != 1) {
                MAP_FAIL();
            }
        } else if (tag == "[visible minimap]") {
            bool success = false;
            script->value_374 = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[visible town minimap]") {
            bool success = false;
            script->value_378 = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[random start map]") {
            // This one-integer map-list extension is present in the newer
            // PVF shipped with the service.  The IDA-recovered legacy
            // STMapScript/CMap ABI has no storage slot or consumer for it,
            // so it must be consumed here without fabricating state or
            // shifting the next top-level tag out of the scanner.
            bool success = false;
            (void)ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[quest start map]") {
            // load_map_files reads STMapScript +0x148 as its existing
            // start-map flag before calling CDungeon::insertStart.  The
            // newer no-payload quest form denotes the same state.
            script->value_178 = 1;
        } else if (tag == "[IGA animation]") {
            // The current PVF adds an IGA-only visual sequence to town maps.
            // The IDA-recovered STMapScript (0x37c) has no member and no
            // server-side consumer for it, so retain the old physical ABI
            // while consuming the exact filename/layer/XYZ record grammar.
            // PVF revisions use either [IGA animation] or [/IGA animation]
            // as the terminating token.
            while (true) {
                bool is_type = false;
                std::string animation_name;
                if (ScanTypeOrStr(is_type, animation_name, true) != 1) {
                    MAP_FAIL();
                }
                if (is_type) {
                    if (animation_name == "[IGA animation]" ||
                        animation_name == "[/IGA animation]") {
                        break;
                    }
                    tag = animation_name;
                    pending = true;
                    break;
                }

                bool layer_is_type = false;
                std::string layer;
                if (ScanTypeOrStr(layer_is_type, layer, true) != 1 ||
                    (layer != "[close]" && layer != "[normal]" &&
                     layer != "[bottom]" && layer != "[closeback]" &&
                     layer != "[middleback]" && layer != "[distantback]" &&
                     layer != "[cover]")) {
                    MAP_FAIL();
                }

                bool success = false;
                (void)ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                (void)ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                (void)ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
            }
        } else if (tag == "[quest animation]") {
            script->quest_animations.clear();
            STQuestAnimation animation;
            while (ScanStr(&animation.animation_name) == 1) {
                std::string layer;
                if (ScanStr(&layer) != 1) {
                    MAP_FAIL();
                }
                animation.layer = getDrawLayer(layer.c_str());
                bool success = false;
                animation.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                animation.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                animation.position_z = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                animation.quest_id = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                // The 0828 PVF record omits the legacy fifth integer.  The
                // native value type already defaults this runtime flag to 1.
                script->quest_animations.push_back(animation);
            }
        } else if (tag == "[dialog]") {
            STDialog dialog;
            if (ScanStr(&dialog.dialog) != 1) {
                MAP_FAIL();
            }
            std::string trigger;
            if (ScanStr(&trigger) != 1) {
                MAP_FAIL();
            }
            if (trigger == "on start") {
                dialog.on_start = 1;
            } else if (trigger == "on complete") {
                dialog.on_complete = 1;
            } else {
                MAP_FAIL();
            }
            script->dialogs.push_back(dialog);
        } else if (tag == "[attacked monster info]") {
            script->attacked_monsters.clear();
            int cumulative_count = 0;
            while (true) {
                STAttackedMonster monster;
                bool success = false;
                monster.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                monster.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                monster.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                monster.position_z = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                cumulative_count += ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                monster.cumulative_count = cumulative_count;
                monster.value_14 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                monster.value_18 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->attacked_monsters.push_back(monster);
            }
        } else if (tag == "[tournament monster]") {
            bool success = false;
            const int tournament_index = ScanInt(&success);
            if (!success || tournament_index <= 0 || tournament_index > 2) {
                MAP_FAIL();
            }
            std::vector<TournamentMonster> monsters;
            while (true) {
                TournamentMonster monster;
                monster.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                monster.count = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                monsters.push_back(monster);
            }
            if (!script->tournament_monsters.insert(
                    std::make_pair(static_cast<char>(tournament_index),
                                   monsters)).second) {
                MAP_FAIL();
            }
        } else if (tag == "[tournament enemies]") {
            bool success = false;
            const int tournament_index = ScanInt(&success);
            if (!success || tournament_index <= 0 || tournament_index > 2) {
                MAP_FAIL();
            }

            std::string enemy_kind;
            if (ScanStr(&enemy_kind) != 1 ||
                (enemy_kind != "[monster]" && enemy_kind != "[apc]")) {
                MAP_FAIL();
            }

            std::vector<TournamentMonster> monsters;
            while (true) {
                int monster_id = 0;
                float float_value = 0.0f;
                std::string next;
                const DNFLEX_TOKEN token =
                    ScanAll(&monster_id, &float_value, &next);
                if (token == DNF_TOK_TYPE &&
                    next == "[/tournament enemies]") {
                    break;
                }
                if (token != DNF_TOK_DECIMAL &&
                    token != DNF_TOK_HEXADECIMAL) {
                    MAP_FAIL();
                }

                TournamentMonster monster;
                monster.monster_id = monster_id;
                monster.count = ScanInt(&success);
                if (!success || monster.monster_id <= 0 ||
                    monster.count <= 0) {
                    MAP_FAIL();
                }

                // The display name follows each ID/weight pair.  The legacy
                // TournamentMonster owner retains only the two numeric fields.
                std::string display_name;
                if (ScanStr(&display_name) != 1 || display_name.empty()) {
                    MAP_FAIL();
                }
                monsters.push_back(monster);
            }

            if (monsters.empty() ||
                !script->tournament_monsters.insert(
                    std::make_pair(static_cast<char>(tournament_index),
                                   monsters)).second) {
                MAP_FAIL();
            }
            script->tournament_enemy_role_bases[tournament_index - 1] =
                enemy_kind == "[apc]" ? 5u : 0u;
        } else if (tag == "[tournament start area]") {
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success || enabled <= 0 || enabled > 2) {
                MAP_FAIL();
            }
            if (enabled == 1) {
                TournamentStartPos position;
                position.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                position.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                const int start_area = ScanInt(&success);
                if (!success || start_area <= 0 || start_area > 2) {
                    MAP_FAIL();
                }
                if (start_area == 1) {
                    script->tournament_start_area_1.push_back(position);
                } else {
                    script->tournament_start_area_2.push_back(position);
                }
            }
        } else if (tag == "[move layered map]") {
            std::string nested_tag;
            while (ScanType(nested_tag, true) == 1) {
                if (nested_tag == "[/move layered map]") {
                    break;
                }
                if (nested_tag == "[zpos under]") {
                    bool success = false;
                    script->value_350 = 1;
                    script->value_354 = ScanInt(&success);
                    if (!success) {
                        MAP_FAIL();
                    }
                } else if (nested_tag == "[all dead case passable]") {
                    script->value_358 = 1;
                } else {
                    MAP_FAIL();
                }
            }
        } else if (tag == "[static player start pos]") {
            while (true) {
                std::string player_tag;
                if (ScanType(player_tag, true) != 1) {
                    MAP_FAIL();
                }
                if (player_tag == "[player 1]") {
                } else if (player_tag == "[player 2]") {
                } else if (player_tag == "[player 3]") {
                } else if (player_tag == "[player 4]") {
                } else if (player_tag == "[/static player start pos]") {
                    break;
                } else {
                    MAP_FAIL();
                }
                StaticStartPos position;
                bool success = false;
                position.player_slot = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                position.position_x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                position.position_y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                script->static_player_start_positions.push_back(position);
            }
        } else if (tag == "[limit player move area]") {
            while (true) {
                std::string player_tag;
                if (ScanType(player_tag, true) != 1) {
                    MAP_FAIL();
                }
                int player_slot = -1;
                if (player_tag == "[player 1]") {
                    player_slot = 0;
                } else if (player_tag == "[player 2]") {
                    player_slot = 1;
                } else if (player_tag == "[player 3]") {
                    player_slot = 2;
                } else if (player_tag == "[player 4]") {
                    player_slot = 3;
                } else if (player_tag == "[/limit player move area]") {
                    break;
                } else {
                    MAP_FAIL();
                }

                MapArea area;
                bool success = false;
                area.x = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                area.y = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                area.width = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                area.height = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                if (script->limit_player_move_areas.find(player_slot) ==
                    script->limit_player_move_areas.end()) {
                    script->limit_player_move_areas.insert(
                        std::make_pair(player_slot, area));
                }
            }
        } else if (tag == "[blood monster]" ||
                   tag == "[ultimate monster]") {
            script->blood_systems.clear();
            while (true) {
                STAssignBloodSystem value;
                bool success = false;
                value.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.value_04 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_08 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_0c = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_10 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_14 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_18 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_1c = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_20 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_24 = ScanInt(&success);
                if (!success ||
                    value.value_14 - 500 <= value.value_18 * value.value_24) {
                    MAP_FAIL();
                }
                script->blood_systems.push_back(value);
            }
        } else if (tag == "[defense monster]") {
            script->defense_systems.clear();
            while (true) {
                STAssignDefenseSystem value;
                bool success = false;
                value.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                value.value_04 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_08 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_0c = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_10 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_14 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_18 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_1c = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_20 = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
                value.value_24 = ScanInt(&success);
                if (!success ||
                    value.value_14 - 500 <= value.value_18 * value.value_24) {
                    MAP_FAIL();
                }
                script->defense_systems.push_back(value);
            }
        } else if (tag == "[blood phase time]" ||
                   tag == "[ultimate phase time]") {
            const bool is_ultimate_phase = tag == "[ultimate phase time]";
            script->blood_phase_times.clear();
            int entry_count = 0;
            int spawn_count = 0;
            int expected_spawn_count = 0;
            int previous_timer_count = 0;
            int previous_phase = 0;
            int previous_spawn_order = -1;
            int previous_interval = 0;

            while (true) {
                bool success = false;
                STAssignBloodPhaseTime entry;
                entry.phase = ScanInt(&success);
                if (!success) {
                    break;
                }
                entry.monster_id = ScanInt(static_cast<bool*>(0));
                entry.value_08 = ScanInt(static_cast<bool*>(0));
                entry.rate = ScanFloat(static_cast<bool*>(0));
                entry.value_10 = ScanInt(static_cast<bool*>(0));
                entry.value_14 = ScanInt(static_cast<bool*>(0));

                if (is_ultimate_phase) {
                    std::string ultimate_group;
                    if (ScanStr(&ultimate_group) != 1) {
                        MAP_FAIL();
                    }
                    if (ultimate_group == "A") {
                        entry.enabled = 1;
                    } else if (ultimate_group == "B") {
                        entry.enabled = 2;
                    } else if (ultimate_group == "C") {
                        entry.enabled = 3;
                    } else if (ultimate_group == "D") {
                        entry.enabled = 4;
                    } else if (ultimate_group == "E") {
                        entry.enabled = 5;
                    } else {
                        MAP_FAIL();
                    }
                }

                if (entry.monster_id < 0 ||
                    static_cast<size_t>(entry.monster_id) >=
                        script->blood_systems.size() ||
                    (entry.phase != previous_phase && entry.value_10 != 0) ||
                    entry.value_14 > 10) {
                    MAP_FAIL();
                }

                if (!is_ultimate_phase && entry.phase == previous_phase) {
                    if (entry.value_10 == previous_spawn_order) {
                        if (previous_timer_count == 0 || entry.value_14 == 0 ||
                            entry.value_14 > previous_timer_count ||
                            (entry.value_08 < previous_interval &&
                             entry.value_10 == 0)) {
                            MAP_FAIL();
                        }
                    }
                    if (entry.phase < previous_phase ||
                        entry.value_10 < previous_spawn_order) {
                        MAP_FAIL();
                    }
                }
                if (entry_count == 0 &&
                    (entry.value_10 != 0 || entry.phase != 0)) {
                    MAP_FAIL();
                }

                if (expected_spawn_count != 0 &&
                    entry.value_14 > previous_timer_count) {
                    if (spawn_count != expected_spawn_count) {
                        MAP_FAIL();
                    }
                    spawn_count = 0;
                    expected_spawn_count = 0;
                }
                if (entry.value_14 <= 0) {
                    if (spawn_count != 0) {
                        if (spawn_count != expected_spawn_count) {
                            MAP_FAIL();
                        }
                        spawn_count = 0;
                        expected_spawn_count = 0;
                    }
                } else {
                    if (entry.value_14 != 1) {
                        expected_spawn_count = entry.value_14;
                    }
                    ++spawn_count;
                }

                previous_phase = entry.phase;
                previous_spawn_order = entry.value_10;
                previous_timer_count = entry.value_14;
                previous_interval = entry.value_08;
                ++entry_count;
                script->blood_phase_times.push_back(entry);
            }
            if (script->blood_phase_times.size() < 2u) {
                MAP_FAIL();
            }
            const STAssignBloodPhaseTime& last =
                script->blood_phase_times[script->blood_phase_times.size() - 1u];
            const STAssignBloodPhaseTime& previous =
                script->blood_phase_times[script->blood_phase_times.size() - 2u];
            if (last.phase == previous.phase && last.value_14 != 0) {
                MAP_FAIL();
            }
        } else if (tag == "[defense phase time]") {
            script->defense_phase_times.clear();
            int entry_count = 0;
            int previous_phase = 0;
            int previous_spawn_order = -1;
            int previous_timer_count = 0;
            int previous_interval = 0;
            while (true) {
                bool success = false;
                STAssignDefensePhaseTime entry;
                entry.phase = ScanInt(&success);
                if (!success) {
                    break;
                }
                entry.monster_id = ScanInt(static_cast<bool*>(0));
                entry.value_08 = ScanInt(static_cast<bool*>(0));
                entry.rate = ScanFloat(static_cast<bool*>(0));
                entry.value_10 = ScanInt(static_cast<bool*>(0));
                entry.value_14 = ScanInt(static_cast<bool*>(0));

                if (entry.monster_id < 0 ||
                    static_cast<size_t>(entry.monster_id) >=
                        script->defense_systems.size() ||
                    (entry.phase != previous_phase && entry.value_10 != 0) ||
                    entry.value_14 > 10) {
                    MAP_FAIL();
                }

                // The old source tests this exact mixed-case token here.
                if (tag == "[Defense phase time]" &&
                    entry.phase == previous_phase) {
                    if (entry.value_10 == previous_spawn_order &&
                        (previous_timer_count == 0 || entry.value_14 == 0 ||
                         entry.value_14 > previous_timer_count ||
                         (entry.value_08 < previous_interval &&
                          entry.value_10 == 0))) {
                        MAP_FAIL();
                    }
                    if (entry.phase < previous_phase ||
                        entry.value_10 < previous_spawn_order) {
                        MAP_FAIL();
                    }
                }
                if (entry_count == 0 &&
                    (entry.value_10 != 0 || entry.phase != 0)) {
                    MAP_FAIL();
                }

                previous_phase = entry.phase;
                previous_spawn_order = entry.value_10;
                previous_timer_count = entry.value_14;
                previous_interval = entry.value_08;
                ++entry_count;
                script->defense_phase_times.push_back(entry);
            }
        } else if (tag == "[summon start area]") {
            bool success = false;
            script->value_15c_area.x = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            script->value_15c_area.y = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            script->value_15c_area.width = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
            script->value_15c_area.height = ScanInt(&success);
            if (!success) {
                MAP_FAIL();
            }
        } else if (tag == "[customized screen edge]") {
            while (true) {
                std::string edge_tag;
                if (ScanType(edge_tag, true) != 1) {
                    MAP_FAIL();
                }
                if (edge_tag == "[/customized screen edge]") {
                    break;
                }

                int32_t* edge_value = 0;
                if (edge_tag == "[left]") {
                    edge_value = &script->customized_screen_edge_left;
                } else if (edge_tag == "[right]") {
                    edge_value = &script->customized_screen_edge_right;
                } else if (edge_tag == "[top]") {
                    edge_value = &script->customized_screen_edge_top;
                } else if (edge_tag == "[bottom]") {
                    edge_value = &script->customized_screen_edge_bottom;
                } else {
                    continue;
                }

                bool success = false;
                *edge_value = ScanInt(&success);
                if (!success) {
                    MAP_FAIL();
                }
            }
        } else if (tag == "[combo training]") {
            // CLIENT_PVF_EVIDENCE: Act2/traningRoom.dgn selects a map whose
            // [combo training] flag gates the persistent three-slot practice
            // score UI.  Reuse STMapScript's verified +0x36f padding byte;
            // no ABI size or following-member offset changes.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success || (enabled != 0 && enabled != 1)) {
                MAP_FAIL();
            }
            script->reserved_36f = static_cast<uint8_t>(enabled);
        } else if (tag == "[Use SkillChange]" ||
                   tag == "[Use Fatigue]" ||
                   tag == "[Use Endurance]" ||
                   tag == "[Use GiveUpPenalty]") {
            // The remaining newer training-room switches are client behavior
            // controls with no recovered legacy-server gameplay owner.
            bool success = false;
            const int enabled = ScanInt(&success);
            if (!success || (enabled != 0 && enabled != 1)) {
                MAP_FAIL();
            }
        } else if (tag == "[generation time]" ||
                   tag == "[extend generation time]") {
            // Zombie-event maps store generation rules as closed records of
            // seven integers.  The recovered legacy server map ABI has no
            // matching owner, so retain scanner alignment and reject partial
            // records instead of inventing a runtime layout.
            const std::string close_tag =
                tag == "[generation time]"
                    ? "[/generation time]"
                    : "[/extend generation time]";
            int field_count = 0;
            while (true) {
                int value = 0;
                float float_value = 0.0f;
                std::string next;
                const DNFLEX_TOKEN token =
                    ScanAll(&value, &float_value, &next);
                if (token == DNF_TOK_DECIMAL ||
                    token == DNF_TOK_HEXADECIMAL) {
                    ++field_count;
                    continue;
                }
                if (token != DNF_TOK_TYPE || next != close_tag ||
                    field_count % 7 != 0) {
                    MAP_FAIL();
                }
                break;
            }
        } else if (tag == "[revive with dlg]") {
            // Super-fight maps add a revive NPC and its client-facing dialog.
            // The recovered legacy map object has no matching runtime owner;
            // validate both fields without aliasing the text to [map dialog].
            bool success = false;
            const int revive_npc_id = ScanInt(&success);
            std::string revive_dialog;
            if (!success || revive_npc_id <= 0 ||
                ScanStr(&revive_dialog) != 1 || revive_dialog.empty()) {
                MAP_FAIL();
            }
        } else if (tag == "[/center index]" ||
                   tag == "[/camera edge exception]" ||
                   tag == "[/NPC]") {
            // Some time-gate maps contain one redundant NPC closing token
            // after the NPC record parser has already consumed its own end.
            continue;
        } else {
            MAP_FAIL();
        }
    }
    script->map_index = g_stMapScriptFileList.FindIndexByFullScan(filename);
    unloadRDARScriptFile();
    PublishMapRuntimeExtension0828(script->map_index,
                                   runtime_extension_0828);

#undef MAP_FAIL
    return 1;
}
// IDA_EA: constructor emitted by CDungeonList::load_list @0x834D376.
// IDA_NAME: _ZN15STDungeonScriptC2Ev
// DEMANGLED: STDungeonScript::STDungeonScript(void)
STDungeonScript::STDungeonScript() {
    Clear();
}

// IDA_EA: destructor emitted by CDungeonList::load_list @0x834D376.
// IDA_NAME: _ZN15STDungeonScriptD2Ev
// DEMANGLED: STDungeonScript::~STDungeonScript()
STDungeonScript::~STDungeonScript() {}

// IDA_EA: 0x836ABC6
// IDA_NAME: _ZN15STDungeonScript5ClearEv
// DEMANGLED: STDungeonScript::Clear(void)
STDungeonScript* STDungeonScript::Clear() {
    dungeon_index = 0;
    name = "";
    value_020 = "";
    value_024 = "";
    value_028 = "";
    value_02c = 0;
    value_030 = "";
    value_034 = 0;
    value_038.clear();
    value_044 = 0;
    value_048.clear();
    heroes_required_quest = 0;
    named_monster_positions.clear();
    value_000 = 0;
    value_00c = 0;
    std::fill(value_05c, value_05c + 6, 0);
    value_010.clear();
    value_074 = "";
    min_required_level = 0;
    required_item.clear();
    coin_limit = -1;
    value_054 = "";
    event_dungeon = 0;
    event_required_item.clear();
    clear_reward_items.clear();
    value_0a8 = -1;
    basis_level = 1;
    exp_weight = 1.0f;
    std::fill(value_0b4, value_0b4 + 5, 0);
    dungeon_drop_items.clear();
    std::fill(value_0f8, value_0f8 + 12, -1);
    maze_scripts.clear();
    directory = "";
    name.clear();
    value_020.clear();
    war_room_data.clear();
    dead_tower_data.clear();
    ancient_dungeon = 0;
    value_28d = 0;
    value_28e = 0;
    value_28f = 0;
    value_2a8 = 0;
    tower_of_despair = 0;
    value_2aa = 0;
    defense_dungeon = 0;
    value_2ac = 0;
    std::fill(value_2b0, value_2b0 + 3, 0);
    std::fill(value_2bc, value_2bc + 3, 0);
    value_2c8.clear();
    clear_reward_data.clear();
    survival_reward_data.round_reward_data.clear();
    dimension_possible = 0;
    dimension_inout_count = 0;
    dimension_min_party_members = 0;
    revenge_dungeon_min_party_members = 0;
    revenge_dungeon_max_party_members = 0;
    village_attack_revenge_dungeon = 0;
    revenge_dungeon_entry_limit = 0;
    daily_schedules.clear();
    limit_party_count = 4;
    limit_inout_count = 0;
    enter_boss_room_condition.clear();
    value_140 = 0;
    value_141 = 0;
    value_142 = 1;
    value_144.clear();
    value_150 = 0;
    value_154 = 0;
    value_158 = 0;
    value_159 = 1;
    std::fill(value_128, value_128 + 5, 0);
    value_13c = 0;
    value_168 = -1;
    dungeon_object_drop_items.clear();
    value_0e0.clear();
    special_passive_object_items.clear();
    value_16c = 0;
    value_338 = 0;
    random_tower_max_stage = 0;
    random_tower_map_indexes.clear();
    dungeon_kind = 0;
    value_35c.clear();
    value_368 = 0;
    value_36c = 1;
    value_370 = 1;
    value_374 = 1.0f;
    value_378 = 1.0f;
    value_37c = 1;
    limit_stackable_item_in_tower = 1;
    recovery_stairs.clear();
    tower_dialogs.clear();
    tower_item_drop = 1;
    tower_max_clear_item_num = 0;
    value_3bc = 0;
    value_3c0 = 0;
    try_gen_event_monster_count = 0;
    try_gen_event_monster_probability = 0;
    event_monster_total_probability = 0;
    event_monsters.clear();
    try_gen_second_event_monster_count = 0;
    try_gen_second_event_monster_probability = 0;
    second_event_monster_total_probability = 0;
    second_event_monsters.clear();
    monster_difficulty_bonus.clear();
    std::fill(value_410, value_410 + 5, 0);
    std::fill(value_424, value_424 + 5, 0);
    value_438 = 0.0f;
    value_43c = 2.0f;
    value_440 = 0.0f;
    value_444 = 1.5f;
    value_448.clear();
    value_44c = 1;
    value_450 = -1;
    value_454 = -1;
    value_458.clear();
    value_464.clear();
    value_470.clear();
    value_47c = 0;
    value_480 = 0;
    value_484 = 0;
    value_488 = 0;
    value_48c = 0;
    value_490 = -1;
    value_494 = 0;
    value_498 = 1;
    tournament_rewards.clear();
    tournament_reward_items.Clear();
    tournament_reward_weight = 1.0f;
    tower_reward_rates.clear();
    value_4e0 = 0;
    use_coin_difficulty.clear();
    value_4fc = 0;
    value_500 = -1;
    value_504 = 0;
    value_505 = 0;
    value_508 = 0;
    value_50c = 0;
    std::fill(reserved_50d, reserved_50d + 3, 0);
    sao_dungeon = 0;
    tower_scenario_enabled = 0;
    zombie_dungeon = 0;
    reserved_513 = 0;
    event_monster_random_map_value_0 = 0;
    event_monster_random_map_value_1 = 0;
    multi_start_point = 0;
    usable_stackable = 1;
    use_spectator_mode = 0;
    reserved_51f = 0;
    absolute_change_stats.clear();
    return this;
}

namespace {

enum WarRoomTagImportResult {
    WAR_ROOM_TAG_ERROR = -1,
    WAR_ROOM_TAG_NOT_HANDLED = 0,
    WAR_ROOM_TAG_HANDLED = 1
};

bool ReadWarRoomInt(int32_t* output) {
    bool success = false;
    const int value = ScanInt(&success);
    if (!success) {
        return false;
    }
    *output = static_cast<int32_t>(value);
    return true;
}

bool ReadWarRoomFloat(float* output) {
    bool success = false;
    const float value = ScanFloat(&success);
    if (!success) {
        return false;
    }
    *output = value;
    return true;
}

bool ReadClosedWarRoomIntList(std::vector<int>* output,
                              const char* closing_tag) {
    std::vector<int> parsed;
    while (true) {
        bool success = false;
        const int value = ScanInt(&success);
        if (!success) {
            break;
        }
        parsed.push_back(value);
    }
    if (!LastTokenIsType(closing_tag)) {
        return false;
    }
    output->swap(parsed);
    return true;
}

// ORIGINAL_SERVER_EA: ImportDungeonScript @0x88E5FAA, cases 1057..1324.
// The 0627 object already contained the complete 0x100-byte WarRoom record,
// but the reconstructed tag dispatcher never populated it.  These mappings
// follow the recovered writes exactly and validate every closed 0828 list so
// a partial row cannot be published as a valid dungeon.
WarRoomTagImportResult ImportWarRoomDungeonTag(STWarRoomData* data,
                                               const std::string& tag) {
    if (tag == "[point by type]") {
        for (int index = 0; index != 3; ++index) {
            if (!ReadWarRoomInt(&data->header_values_[index])) {
                return WAR_ROOM_TAG_ERROR;
            }
        }
    } else if (tag == "[phase fatigue]") {
        if (!ReadWarRoomInt(&data->mode_)) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[warroom map index]") {
        if (!ReadWarRoomInt(&data->map_index_)) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[monster type spawn prob]") {
        std::vector<STMonsterTypeSpawnProb> probabilities;
        while (true) {
            bool success = false;
            STMonsterTypeSpawnProb probability;
            probability.group_weights_[0] = ScanInt(&success);
            if (!success) {
                break;
            }
            for (int index = 1; index != 4; ++index) {
                probability.group_weights_[index] = ScanInt(&success);
                if (!success) {
                    return WAR_ROOM_TAG_ERROR;
                }
            }
            probabilities.push_back(probability);
        }
        if (!LastTokenIsType("[/monster type spawn prob]")) {
            return WAR_ROOM_TAG_ERROR;
        }
        data->spawn_probs_.swap(probabilities);
    } else if (tag == "[monster type spawn cost]") {
        for (int index = 0; index != 4; ++index) {
            if (!ReadWarRoomInt(&data->condition_values_[index])) {
                return WAR_ROOM_TAG_ERROR;
            }
        }
    } else if (tag == "[spawn common monster index]") {
        if (!ReadClosedWarRoomIntList(
                &data->monster_groups_[0],
                "[/spawn common monster index]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[spawn common champion index]") {
        if (!ReadClosedWarRoomIntList(
                &data->monster_groups_[1],
                "[/spawn common champion index]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[spawn super champion index]") {
        if (!ReadClosedWarRoomIntList(
                &data->monster_groups_[2],
                "[/spawn super champion index]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[spawn boss index]") {
        if (!ReadClosedWarRoomIntList(
                &data->monster_groups_[3], "[/spawn boss index]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[spawn step max]") {
        if (!ReadWarRoomInt(&data->timing_values_[0])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[spawn step resource pool]") {
        if (!ReadClosedWarRoomIntList(
                &data->monster_group_order_,
                "[/spawn step resource pool]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[monster type spawn interval rate]") {
        for (int index = 0; index != 4; ++index) {
            if (!ReadWarRoomFloat(&data->event_values_[index])) {
                return WAR_ROOM_TAG_ERROR;
            }
        }
    } else if (tag == "[monster spawn base interval]") {
        if (!ReadWarRoomInt(&data->tail_values_[0])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[monster spawn random interval]") {
        if (!ReadWarRoomInt(&data->tail_values_[1])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[max monster]") {
        if (!ReadWarRoomInt(&data->condition_value_)) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[kill count const]") {
        if (!ReadWarRoomFloat(&data->finish_point_reward_factor_)) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[join cost gold]") {
        if (!ReadWarRoomInt(&data->timing_values_[1])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[gold drop prob]") {
        if (!ReadWarRoomInt(&data->timing_values_[2])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common monster item drop prob]") {
        if (!ReadWarRoomInt(&data->spawn_values_[0])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common champion item drop prob]") {
        if (!ReadWarRoomInt(&data->spawn_values_[1])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[super champion item drop prob]") {
        if (!ReadWarRoomInt(&data->spawn_values_[2])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[boss item drop prob]") {
        if (!ReadWarRoomInt(&data->spawn_values_[3])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common monster exp const]") {
        if (!ReadWarRoomInt(&data->extra_group_values_[0])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common champion exp const]") {
        if (!ReadWarRoomInt(&data->extra_group_values_[1])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[super champion exp const]") {
        if (!ReadWarRoomInt(&data->extra_group_values_[2])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[boss exp const]") {
        if (!ReadWarRoomInt(&data->extra_group_values_[3])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common monster item drop list]") {
        if (!ReadClosedWarRoomIntList(
                &data->extra_monster_groups_[0],
                "[/common monster item drop list]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[common champion item drop list]") {
        if (!ReadClosedWarRoomIntList(
                &data->extra_monster_groups_[1],
                "[/common champion item drop list]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[super champion item drop list]") {
        if (!ReadClosedWarRoomIntList(
                &data->extra_monster_groups_[2],
                "[/super champion item drop list]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[boss item drop list]") {
        if (!ReadClosedWarRoomIntList(
                &data->extra_monster_groups_[3],
                "[/boss item drop list]")) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[player kc]") {
        if (!ReadWarRoomInt(&data->condition_mode_)) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[battle spawn time]") {
        if (!ReadWarRoomInt(&data->tail_values_[4])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[monster exp bonus per user decrease]") {
        if (!ReadWarRoomFloat(&data->post_spawn_values_[0])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[result exp bonus per user decrease]") {
        if (!ReadWarRoomFloat(&data->post_spawn_values_[1])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[fatigue]") {
        if (!ReadWarRoomInt(&data->tail_values_[2])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else if (tag == "[fatigue result]") {
        if (!ReadWarRoomInt(&data->tail_values_[3])) {
            return WAR_ROOM_TAG_ERROR;
        }
    } else {
        return WAR_ROOM_TAG_NOT_HANDLED;
    }
    return WAR_ROOM_TAG_HANDLED;
}

}  // namespace

// IDA_EA: 0x88E5FAA
// IDA_NAME: _Z19ImportDungeonScriptP15STDungeonScriptPKc
// DEMANGLED: ImportDungeonScript(STDungeonScript *,char const*)
int ImportDungeonScript(STDungeonScript* script, const char* filename) {
    if (!script ||
        loadRDARScriptFile(g_szDungeonScriptBaseDirectory,
                                   filename) != 1) {
        return 0;
    }

    script->Clear();
    getDirectoryPath(script->directory,
                     "%s%s",
                     g_szDungeonScriptBaseDirectory,
                     filename);
    const int dungeon_index =
        g_stDungeonScriptFileList.FindIndexByFullScan(filename);
    DungeonRuntimeConfig runtime_config;
    g_dungeon_runtime_configs.erase(dungeon_index);

    std::string tag;
    std::string pending_tag;
    bool pending = false;
    bool maze_info_started = false;
    bool war_room_data_seen = false;
    bool unhandled_tag_seen_before_war_room = false;
    MazeScript current_maze;
    while (pending || ScanType(tag, true) == 1) {
        if (pending) {
            tag = pending_tag;
            pending = false;
            pending_tag.clear();
        }

        if (tag == "[background pos]") {
            script->value_488 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[entering title]") {
            std::string entering_title;
            if (ScanStr(&entering_title) == 1) {
                if (entering_title == "ByDifficulty") {
                    script->value_034 = 1;
                    while (ScanStr(&entering_title) == 1) {
                        script->value_038.push_back(entering_title);
                    }
                } else {
                    script->value_024 = entering_title;
                }
            }
        } else if (tag == "[cutscene image]") {
            ScanStr(&script->value_028);
            script->value_02c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[minimap image]") {
            ScanStr(&script->value_030);
        } else if (tag == "[worldmap info]") {
            for (int row = 0; row != 2; ++row) {
                for (int column = 0; column != 2; ++column) {
                    script->value_05c[row * 2 + column] =
                        ScanInt(static_cast<bool*>(0));
                }
            }
            ScanStr(&script->value_074);
        } else if (tag == "[worldmap pattern info]") {
            const int map_type = ScanInt(static_cast<bool*>(0));
            const int pattern_index = ScanInt(static_cast<bool*>(0));
            STWorldmapPattern pattern;
            GetWorldMapCoordFromTable(map_type, pattern_index, pattern);
            script->value_05c[0] = static_cast<int32_t>(pattern.raw_08);
            script->value_05c[1] = static_cast<int32_t>(pattern.raw_0c);
            script->value_05c[2] = static_cast<int32_t>(pattern.raw_10);
            script->value_05c[3] = static_cast<int32_t>(pattern.raw_14);
            script->value_05c[4] = static_cast<int32_t>(pattern.raw_18);
            script->value_05c[5] = static_cast<int32_t>(pattern.raw_1c);
            ScanStr(&script->value_074);
            script->value_000 = ScanInt(static_cast<bool*>(0));
            script->value_004 = map_type - 1;
            script->value_008 = pattern_index - 1;
        } else if (tag == "[champion]") {
            for (int index = 0; index != 5; ++index) {
                script->value_0b4[index] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[maze info]") {
            if (maze_info_started) {
                if (!CommitCompleteDungeonMaze(
                        &script->maze_scripts, current_maze)) {
                    return 0;
                }
                current_maze.clear();
            } else {
                maze_info_started = true;
            }
        } else if (tag == "[quest connection]") {
            current_maze.value_4c = ScanInt(static_cast<bool*>(0));
            current_maze.value_50 = ScanInt(static_cast<bool*>(0));
            if (current_maze.value_4c < -1 ||
                current_maze.value_4c > 1) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptDungeon.cpp",
                    "ImportDungeonScript",
                    0x282,
                    "error! maze quest type(%d)",
                    current_maze.value_4c);
                return 0;
            }
            if (current_maze.value_4c == 0) {
                current_maze.value_54 =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[clear condition]") {
            bool condition_token_is_type = false;
            std::string condition_tag;
            while (ScanTypeOrStr(condition_token_is_type,
                                 condition_tag,
                                 true) == 1 &&
                   condition_tag != "[/clear condition]") {
                DungeonClearCondition condition;
                if (condition_tag == "[destroy object]") {
                    condition.value_00 = 0;
                } else if (condition_tag == "[clear map]") {
                    condition.value_00 = 1;
                } else if (condition_tag == "[hunt monster]") {
                    condition.value_00 = 2;
                } else if (condition_tag == "[hunt apc]") {
                    condition.value_00 = 3;
                } else if (condition_tag == "[hunt boss]") {
                    condition.value_00 = 4;
                } else {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptDungeon.cpp",
                        "ImportDungeonScript",
                        0x2a1,
                        "error! dungeon clear condition(%s)",
                        condition_tag.c_str());
                    return 0;
                }

                bool success = false;
                condition.value_04 = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptDungeon.cpp",
                        "ImportDungeonScript",
                        0x2a8,
                        "error! dungeon clear condition left variable");
                    return 0;
                }
                condition.value_08 = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptDungeon.cpp",
                        "ImportDungeonScript",
                        0x2ae,
                        "error! dungeon clear condition right variable");
                    return 0;
                }
                if (condition.value_00 == 1) {
                    condition.value_08 = 1;
                }
                current_maze.clear_conditions.push_back(condition);
            }
        } else if (tag == "[escape hell]") {
            // IDA 0x88E5FAA case 708 stores this conversion only in the
            // parser-local v99; it deliberately does not persist in MazeScript.
            (void)(ScanInt(static_cast<bool*>(0)) != 0);
        } else if (tag == "[size]") {
            bool success = false;
            current_maze.grid_width = ScanInt(&success);
            if (!success || current_maze.grid_width <= 0) {
                return 0;
            }
            current_maze.grid_height = ScanInt(&success);
            if (!success || current_maze.grid_height <= 0) {
                return 0;
            }
            const std::size_t width = static_cast<std::size_t>(
                current_maze.grid_width);
            const std::size_t height = static_cast<std::size_t>(
                current_maze.grid_height);
            if (width > std::numeric_limits<std::size_t>::max() / height) {
                return 0;
            }
            current_maze.grids.resize(width * height, GridScript());
        } else if (tag == "[randomized object creation]") {
            // IDA ImportDungeonScript @0x88E5FAA: this closed block projects
            // directly into MazeScript::ridable.  Official risk dungeons put
            // it between [maze info] and [size], so the outer scanner cannot
            // safely consume it one nested tag at a time.
            current_maze.ridable.clear();
            while (true) {
                std::string randomized_tag;
                if (ScanType(randomized_tag, true) != 1) {
                    return 0;
                }
                if (randomized_tag ==
                    "[/randomized object creation]") {
                    break;
                }

                bool success = false;
                if (randomized_tag == "[select]") {
                    current_maze.ridable.value_00 = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (randomized_tag == "[regenerate]") {
                    current_maze.ridable.value_04 = static_cast<uint8_t>(
                        ScanInt(&success) != 0);
                    if (!success) {
                        return 0;
                    }
                } else if (randomized_tag == "[minimap icon]") {
                    script->value_500 = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                } else if (randomized_tag == "[object]") {
                    RidableObject object;
                    while (true) {
                        std::string object_tag;
                        if (ScanType(object_tag, true) != 1) {
                            return 0;
                        }
                        if (object_tag == "[/object]") {
                            break;
                        }
                        if (object_tag == "[map]") {
                            object.value_00 = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                            object.value_04 = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                        } else if (object_tag == "[index]") {
                            object.value_08 = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                        } else if (object_tag == "[team]") {
                            std::string team_tag;
                            if (ScanType(team_tag, true) != 1) {
                                return 0;
                            }
                            if (team_tag == "[neutral]") {
                                object.value_14 = 200;
                            } else if (team_tag == "[monster]") {
                                object.value_14 = 100;
                            } else if (team_tag == "[character]") {
                                object.value_14 = 0;
                            } else {
                                return 0;
                            }
                        } else if (object_tag == "[pos]") {
                            object.value_0c = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                            object.value_10 = ScanInt(&success);
                            if (!success) {
                                return 0;
                            }
                        } else {
                            return 0;
                        }
                    }
                    current_maze.ridable.objects.push_back(object);
                } else {
                    return 0;
                }
            }
        } else if (tag == "[greed]") {
            std::string greed_text;
            ScanStr(&greed_text);

            std::size_t cursor = 0;
            int invalid_symbol_count = 0;
            std::size_t grid_index = 0;
            while (cursor + 1u < greed_text.size()) {
                const std::string grid_symbol =
                    greed_text.substr(cursor, 2u);
                if (grid_symbol[0] == '\r' || grid_symbol[0] == '\n' ||
                    grid_symbol[0] == ' ') {
                    ++cursor;
                    continue;
                }

                int symbol_index = 0;
                for (; symbol_index != 16; ++symbol_index) {
                    if (grid_symbol[0] == g_gridSymbols[symbol_index][0] &&
                        grid_symbol[1] == g_gridSymbols[symbol_index][1]) {
                        break;
                    }
                }
                if (symbol_index == 16) {
                    ++cursor;
                    ++invalid_symbol_count;
                    continue;
                }

                if (grid_index >= current_maze.grids.size()) {
                    return 0;
                }
                current_maze.grids[grid_index].grid_type = symbol_index;
                ++grid_index;
                cursor += 2u;
            }
            // Short greed rows are legal in the official SAO tower scripts;
            // resize() already default-constructs their remaining cells.
            if (invalid_symbol_count != 0) {
                return 0;
            }
        } else if (tag == "[map specification]" ||
                   tag == "[boss map specification]" ||
                   tag == "[layered map specification]") {
            bool success = false;
            const int x = ScanInt(&success);
            if (!success) {
                return 0;
            }
            const int y = ScanInt(&success);
            if (!success || x < 0 || y < 0 ||
                x >= current_maze.grid_width ||
                y >= current_maze.grid_height) {
                return 0;
            }
            GridScript& grid = current_maze.getGrid(x, y);
            std::vector<int>* values = &grid.values_04;
            if (tag == "[boss map specification]") {
                values = &grid.values_10;
            } else if (tag == "[layered map specification]") {
                values = &grid.values_1c;
            }
            values->clear();
            while (true) {
                bool success = false;
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                values->push_back(value);
            }
        } else if (tag == "[start map]" || tag == "[boss map]") {
            std::vector<std::pair<int, int> >* positions =
                tag == "[start map]" ? &current_maze.values_14
                                     : &current_maze.values_20;
            positions->clear();
            while (true) {
                bool success = false;
                const int x = ScanInt(&success);
                if (!success) {
                    break;
                }
                // IDA 0x88E5FAA scans Y without a success pointer.  Two
                // official advance-altar test scripts carry an odd trailing
                // start value; the original projects its missing Y as zero.
                const int y = ScanInt(static_cast<bool*>(0));
                positions->push_back(std::make_pair(x, y));
            }
        } else if (tag == "[seal door map index]") {
            current_maze.value_2c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[seal door pos]") {
            current_maze.value_30 = ScanInt(static_cast<bool*>(0));
            current_maze.value_34 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[map move monster]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 2686.  The client
            // stores a map<int, vector<44-byte-record>>; repeated keys add
            // record+0x18 into a group accumulator.  No client-side selector
            // was found, so this value is deliberately not called a weight.
            runtime_config.critical_tail_seen = true;
            if (!ParseMapMoveMonsterBlock(&runtime_config)) {
                return 0;
            }
        } else if (tag == "[rank type]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 2651.
            runtime_config.critical_tail_seen = true;
            if (!ParseRankTypeBlock(&runtime_config)) {
                return 0;
            }
        } else if (tag == "[difficulty]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 2789, fixed DGN
            // difficulty at STDungeonScript+0x86c.  Keep it sidecar-only so
            // the recovered 0627 server ABI stays unchanged.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || value < 0 || value > 4 ||
                runtime_config.has_fixed_difficulty) {
                return 0;
            }
            runtime_config.critical_tail_seen = true;
            runtime_config.has_fixed_difficulty = true;
            runtime_config.fixed_difficulty = value;
        } else if (tag == "[no fatigue]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript valueless flag.
            script->value_48c = 1;
        } else if (tag == "[hell dungeon]") {
            script->value_140 =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[prohibit practice]") {
            script->value_141 =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[impossible skills]") {
            std::vector<std::pair<int, int> > impossible_skills;
            std::string job_name;
            while (ScanStr(&job_name) == 1) {
                bool success = false;
                const int skill_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                impossible_skills.push_back(std::make_pair(
                    getCharacterJobToEnumValue(job_name), skill_index));
            }
            if (!LastTokenIsType("[/impossible skills]")) {
                return 0;
            }
            script->value_144.swap(impossible_skills);
        } else if (tag == "[item]" ||
                   tag == "[dungeon drop]" ||
                   tag == "[dungeon object drop]") {
            std::vector<std::pair<int, int> >* drops =
                &script->dungeon_drop_items;
            if (tag == "[dungeon drop]") {
                drops = &script->dungeon_object_drop_items;
            } else if (tag == "[dungeon object drop]") {
                drops = &script->value_0e0;
            }
            drops->clear();
            while (true) {
                bool success = false;
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int rate = ScanInt(static_cast<bool*>(0));
                drops->push_back(std::make_pair(item_id, rate));
            }
        } else if (tag == "[clear reward item]") {
            std::vector<std::pair<int, int> > reward_items;
            while (true) {
                bool success = false;
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                reward_items.push_back(
                    std::make_pair(item_id, item_count));
            }
            if (!LastTokenIsType("[/clear reward item]")) {
                return 0;
            }
            script->clear_reward_items.swap(reward_items);
        } else if (tag == "[special passive object item]") {
            script->special_passive_object_items.clear();
            while (true) {
                bool success = false;
                (void)ScanInt(&success);
                if (!success) {
                    break;
                }

                const int dungeon_id = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                const int item_pair_count = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                stDungeonAssignItem_t assigned_item;
                assigned_item.dungeon_index = dungeon_id;
                for (int index = 0; index != item_pair_count; ++index) {
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    const int rate = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    assigned_item.items.push_back(
                        std::make_pair(item_id, rate));
                }
                script->special_passive_object_items.push_back(
                    assigned_item);
            }
        } else if (tag == "[pathgate object]") {
            for (int index = 0; index != 3; ++index) {
                script->value_0f8[index] =
                    ScanInt(static_cast<bool*>(0));
            }
            for (int index = 0; index != 3; ++index) {
                script->value_0f8[index + 3] =
                    ScanInt(static_cast<bool*>(0));
            }
            for (int index = 0; index != 2; ++index) {
                script->value_0f8[index + 6] =
                    ScanInt(static_cast<bool*>(0));
            }
            for (int index = 0; index != 2; ++index) {
                script->value_0f8[index + 9] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[hit count]") {
            for (int index = 0; index != 5; ++index) {
                bool success = false;
                script->value_128[index] = ScanInt(&success);
                if (!success) {
                    break;
                }
            }
        } else if (tag == "[seal door appear rate]") {
            script->value_13c = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[boss room entrance condition]") {
            // IDA ImportDungeonScript @0x88E5FAA, case 1704.  This is a
            // dungeon-wide block even though the official files place it
            // immediately after the final maze.  Shadow Maze and Dark Elf
            // Cemetery rely on it to select rooms carrying condition
            // monsters (the seal stones); treating its first scalar as an
            // unsupported tail commits the maze without ever spawning those
            // objects.
            std::string condition_mode;
            if (ScanStr(&condition_mode) != 1) {
                return 0;
            }

            bool success = false;
            const int condition_count = ScanInt(&success);
            if (!success || condition_count < 0) {
                return 0;
            }

            int values_per_condition = 0;
            if (condition_mode == "[hunt monster]") {
                script->enter_boss_room_condition.enabled = 1;
                values_per_condition = 3;
            } else if (condition_mode == "[seeking]") {
                script->enter_boss_room_condition.enabled = 2;
                values_per_condition = 2;
            } else {
                return 0;
            }

            if (!script->enter_boss_room_condition.conditions.empty() ||
                condition_count >
                    std::numeric_limits<int>::max() /
                        values_per_condition) {
                return 0;
            }
            script->enter_boss_room_condition.conditions.reserve(
                static_cast<std::size_t>(condition_count) *
                static_cast<std::size_t>(values_per_condition));
            for (int condition = 0; condition < condition_count;
                 ++condition) {
                for (int field = 0; field < values_per_condition; ++field) {
                    const int value = ScanInt(&success);
                    if (!success) {
                        return 0;
                    }
                    script->enter_boss_room_condition.conditions.push_back(
                        value);
                }
            }
        } else if (tag == "[event monster]") {
            bool success = false;
            script->try_gen_event_monster_count = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->try_gen_event_monster_probability = ScanInt(&success);
            if (!success) {
                return 0;
            }
            while (true) {
                stEventMonster_t event_monster;
                event_monster.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                event_monster.status_value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                event_monster.probability = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                event_monster.status_first = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                event_monster.status_second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->event_monster_total_probability +=
                    event_monster.probability;
                script->event_monsters.push_back(event_monster);
            }
            if (script->try_gen_event_monster_count != 0 &&
                script->event_monsters.empty()) {
                return 0;
            }
        } else if (tag == "[event monster2]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript @0x15CB486.  The 0828
            // dual-raid script uses the same five-dword row as the original
            // event-monster table, but writes the already existing secondary
            // CDungeon owner consumed by CBattle_Field::SetGridPath.
            if (script->try_gen_second_event_monster_count != 0 ||
                !script->second_event_monsters.empty()) {
                return 0;
            }
            bool success = false;
            const int try_count = ScanInt(&success);
            if (!success || try_count < 0) {
                return 0;
            }
            const int try_probability = ScanInt(&success);
            if (!success || try_probability < 0 || try_probability > 10000) {
                return 0;
            }

            std::vector<stSecondEventMonster_t> rows;
            int total_probability = 0;
            while (true) {
                stSecondEventMonster_t row;
                row.monster_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                row.status_value = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                row.probability = ScanInt(&success);
                if (!success || row.monster_id <= 0 || row.probability <= 0 ||
                    row.probability >
                        std::numeric_limits<int>::max() - total_probability) {
                    return 0;
                }
                row.status_first = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                row.status_second = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                total_probability += row.probability;
                rows.push_back(row);
            }
            if (!LastTokenIsType("[/event monster2]") ||
                (try_count != 0 && rows.empty())) {
                return 0;
            }
            script->try_gen_second_event_monster_count = try_count;
            script->try_gen_second_event_monster_probability =
                try_probability;
            script->second_event_monster_total_probability =
                total_probability;
            script->second_event_monsters.swap(rows);
            runtime_config.extension_0828_seen = true;
        } else if (tag == "[named monster map pos]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript @0x15CB402.  Rows are
            // uncounted x/y pairs terminated by the physical closing tag.
            if (!script->named_monster_positions.empty()) {
                return 0;
            }
            std::vector<STNamedMonsterMapPos> positions;
            while (true) {
                bool success = false;
                const int x = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int y = ScanInt(&success);
                if (!success || x < 0 || y < 0) {
                    return 0;
                }
                STNamedMonsterMapPos position;
                position.x = x;
                position.y = y;
                positions.push_back(position);
            }
            if (positions.empty() ||
                !LastTokenIsType("[/named monster map pos]")) {
                return 0;
            }
            script->named_monster_positions.swap(positions);
            runtime_config.extension_0828_seen = true;
        } else if (tag == "[dummy appear count]") {
            bool success = false;
            const int value = ScanInt(&success);
            // CDungeon owns sixteen fixed dummy-map slots and SetGridPath
            // iterates this count directly.
            if (!success || value < 0 || value > 16) {
                return 0;
            }
            script->value_16c = value;
            runtime_config.extension_0828_seen = true;
        } else if (tag == "[realdungeon checkup]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript @0x15C9679.  0828 ships
            // only the [monster] category: a count followed by that many
            // prototype IDs.  Preserve the client validation list without
            // inventing meanings for the two unused client containers.
            if (!runtime_config.realdungeon_checkup_monsters.empty()) {
                return 0;
            }
            bool saw_monsters = false;
            bool closed = false;
            while (true) {
                bool is_type = false;
                std::string entry;
                if (ScanTypeOrStr(is_type, entry, true) != 1) {
                    return 0;
                }
                if (is_type) {
                    if (entry != "[/realdungeon checkup]") {
                        return 0;
                    }
                    closed = true;
                    break;
                }
                if (entry != "[monster]" || saw_monsters) {
                    return 0;
                }
                bool success = false;
                const int count = ScanInt(&success);
                if (!success || count <= 0 || count > 4096) {
                    return 0;
                }
                for (int index = 0; index < count; ++index) {
                    const int monster_id = ScanInt(&success);
                    if (!success || monster_id <= 0) {
                        return 0;
                    }
                    runtime_config.realdungeon_checkup_monsters.push_back(
                        monster_id);
                }
                saw_monsters = true;
            }
            if (!closed || !saw_monsters) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
        } else if (tag == "[coin limit]") {
            script->coin_limit = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[character coin limit]") {
            // ORIGINAL_SERVER_EA: ImportDungeonScript case 405 stores the
            // integer verbatim at STDungeonScript+0x0a8.  Official 0828
            // scripts use both 0 and 5, so this is not a boolean flag.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_0a8 = value;
        } else if (tag == "[disable exit]") {
            // ORIGINAL_SERVER_EA: ImportDungeonScript case 2396 stores the
            // integer verbatim at STDungeonScript+0x508.  Official 0828
            // scripts use both 1 and 2, so preserve the raw mode value.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_508 = value;
        } else if (tag == "[event dungeon]") {
            script->event_dungeon =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[SAO dungeon]") {
            // CLIENT_EA: ImportDungeonScript_ReadSAODungeonFlag @ 0x15CDA9B.
            script->sao_dungeon =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[zombie dungeon]") {
            // 0828 zombi.dgn/zombi2.dgn are the only scripts carrying this
            // numeric feature switch.  Do not infer it from a filename.
            script->zombie_dungeon =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[time spiral]" ||
                   tag == "[tower of despair sao]") {
            // CLIENT_EA: ImportDungeonScript_EnableTowerScenarioFlag
            // @0x15CFABD.  The handler is valueless; the duplicated client
            // integer-reader registration for [time spiral] is unreachable.
            script->tower_scenario_enabled = 1;
        } else if (tag == "[event monster random map]") {
            // CLIENT_EA: ImportDungeonScript_ReadEventMonsterRandomMapValues
            // @0x15CF33F.  ReadCurrentScriptNumericValue returns zero and
            // clears its success flag when the second token is absent; the
            // client ignores that flag, so preserve both raw results exactly.
            // ScanInt cannot be used for the optional second value: it consumes
            // the following TYPE token on failure.  VillageOfTemptation has a
            // single zero followed immediately by [event monster], so the old
            // restore swallowed that tag and terminated on its closing marker.
            // Reuse the outer parser's pending-tag channel to preserve it.
            int first_value = 0;
            if (!ReadOptionalInt(&first_value, &pending_tag)) {
                return 0;
            }
            script->event_monster_random_map_value_0 = first_value;

            int second_value = 0;
            if (!ReadOptionalInt(&second_value, &pending_tag)) {
                if (!pending_tag.empty()) {
                    pending = true;
                }
            }
            script->event_monster_random_map_value_1 = second_value;
        } else if (tag == "[ancient dungeon]") {
            script->ancient_dungeon =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
        } else if (tag == "[village attack revenge dungeon]") {
            // IDA ImportDungeonScript @0x88C7AD6 stores the four byte-sized
            // values in flag, entry-limit, minimum-party, maximum-party
            // order (+0x316, +0x317, +0x314, +0x315).
            script->village_attack_revenge_dungeon =
                ScanInt(static_cast<bool*>(0)) != 0 ? 1 : 0;
            script->revenge_dungeon_entry_limit = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)));
            script->revenge_dungeon_min_party_members = static_cast<int8_t>(
                ScanInt(static_cast<bool*>(0)));
            script->revenge_dungeon_max_party_members = static_cast<int8_t>(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[quest npc dungeon]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_2aa = value != 0 ? 1 : 0;
        } else if (tag == "[multi start point]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            script->multi_start_point = static_cast<uint8_t>(value);
        } else if (tag == "[usable stackable]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            script->usable_stackable = static_cast<uint8_t>(value);
        } else if (tag == "[use spectator mode]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            script->use_spectator_mode = static_cast<uint8_t>(value);
        } else if (tag == "[limit party count]") {
            script->limit_party_count = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[limit inout count]") {
            script->limit_inout_count = static_cast<uint8_t>(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[dimension dungeon]") {
            int legacy_slot = 0;
            if (ReadOptionalInt(&legacy_slot, &pending_tag)) {
                int legacy_count = 0;
                if (!ReadOptionalInt(&legacy_count, &pending_tag) ||
                    legacy_slot < 1 || legacy_slot > 6 ||
                    legacy_count <= 0 || legacy_count > 127) {
                    return 0;
                }
                script->dimension_possible =
                    static_cast<uint8_t>(legacy_slot);
                script->dimension_inout_count =
                    static_cast<int8_t>(legacy_count);
                if (!pending_tag.empty()) {
                    pending = true;
                }
            } else {
                // 0828's tag is valueless.  Its exact counter owner comes
                // from the ordered [impossible] channel in
                // DungeonInCountInfo.etc; the DGN icon frame independently
                // corroborates that slot, so no dungeon-id sequence is
                // guessed here.
                if (pending_tag.empty() ||
                    !ResolveDimensionDungeonConfig(
                        dungeon_index,
                        script->value_000,
                        &script->dimension_possible,
                        &script->dimension_inout_count)) {
                    return 0;
                }
                pending = true;
            }
        } else if (tag == "[necessary party]") {
            bool success = false;
            const int party_members = ScanInt(&success);
            if (!success || party_members < 1 || party_members > 4) {
                return 0;
            }
            script->dimension_min_party_members =
                static_cast<uint8_t>(party_members);
        } else if (tag == "[evil]") {
            // IDA ImportDungeonScript @0x88E5FAA: exactly five integer
            // thresholds are stored at STDungeonScript +0x410.
            for (int index = 0; index != 5; ++index) {
                bool success = false;
                script->value_410[index] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
            }
        } else if (tag == "[evil high level]") {
            // The high-level table is the adjacent five integers at +0x424.
            for (int index = 0; index != 5; ++index) {
                bool success = false;
                script->value_424[index] = ScanInt(&success);
                if (!success) {
                    return 0;
                }
            }
        } else if (tag == "[evil rate]") {
            // The original consumes integer percentages in physical-slot
            // order +0x438, +0x440, +0x43c, +0x444.  All four destinations
            // are floats; the declaration order reflects their ABI offsets.
            bool success = false;
            script->value_438 =
                static_cast<float>(ScanInt(&success)) / 100.0f;
            if (!success) {
                return 0;
            }
            script->value_440 =
                static_cast<float>(ScanInt(&success)) / 100.0f;
            if (!success) {
                return 0;
            }
            script->value_43c =
                static_cast<float>(ScanInt(&success)) / 100.0f;
            if (!success) {
                return 0;
            }
            script->value_444 =
                static_cast<float>(ScanInt(&success)) / 100.0f;
            if (!success) {
                return 0;
            }
        } else if (tag == "[monster difficulty bonus]") {
            // The 0627 parser owns a fixed 13-by-5 matrix.  Unlike its
            // log-and-continue error path, reject truncated 0828 input so it
            // cannot be mistaken for the end of the complete DGN file.
            std::vector<std::vector<float> > bonuses;
            bonuses.reserve(13);
            for (int row = 0; row != 13; ++row) {
                std::vector<float> values;
                values.reserve(5);
                for (int column = 0; column != 5; ++column) {
                    bool success = false;
                    const float value = ScanFloat(&success);
                    if (!success) {
                        return 0;
                    }
                    values.push_back(value);
                }
                bonuses.push_back(values);
            }
            std::string closing_tag;
            if (ScanType(closing_tag, true) != 1 ||
                closing_tag != "[/monster difficulty bonus]") {
                return 0;
            }
            script->monster_difficulty_bonus.swap(bonuses);
        } else if (tag == "[coin info]") {
            // The map key is difficulty 0..3.  The first integer is the coin
            // limit stored in UseCoinDungeonDiff +0; the second is its byte
            // flag at +4.  The legacy member names are retained for ABI.
            std::map<int, UseCoinDungeonDiff> parsed_coin_info;
            std::string difficulty_tag;
            while (ScanStr(&difficulty_tag) == 1) {
                int difficulty = -1;
                if (difficulty_tag == "[easy]") {
                    difficulty = 0;
                } else if (difficulty_tag == "[medium]") {
                    difficulty = 1;
                } else if (difficulty_tag == "[hard]") {
                    difficulty = 2;
                } else if (difficulty_tag == "[ultimate]") {
                    difficulty = 3;
                } else {
                    return 0;
                }

                bool success = false;
                UseCoinDungeonDiff coin_info;
                coin_info.dungeon_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                coin_info.difficulty = static_cast<uint8_t>(
                    ScanInt(&success) != 0);
                if (!success ||
                    !parsed_coin_info
                         .insert(std::make_pair(difficulty, coin_info))
                         .second) {
                    return 0;
                }
            }
            if (!LastTokenIsType("[/coin info]")) {
                return 0;
            }
            script->use_coin_difficulty.swap(parsed_coin_info);
        } else if (tag == "[risk dungeon]") {
            bool success = false;
            script->value_4e0 = static_cast<uint8_t>(
                ScanInt(&success) != 0);
            if (!success) {
                return 0;
            }
        } else if (tag == "[blood max round]") {
            bool success = false;
            script->value_3c4 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[blood dungeon]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_28e = static_cast<uint8_t>(value);
        } else if (tag == "[ai character appear rate]") {
            script->value_338 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[death tower map indexes]") {
            bool success = false;
            script->value_368 = static_cast<uint8_t>(ScanInt(&success));
            while (success) {
                const int stage = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int map_index = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                script->value_35c.push_back(
                    std::make_pair(stage, map_index));
            }
        } else if (tag == "[tower high skill initial cool time]") {
            bool success = false;
            script->value_36c = static_cast<int>(ScanFloat(&success));
            script->value_370 = static_cast<int>(ScanFloat(&success));
        } else if (tag == "[tower high skill initial cool time rate]") {
            bool success = false;
            script->value_374 = ScanFloat(&success);
            script->value_378 = ScanFloat(&success);
        } else if (tag == "[tower fp cubepiece]") {
            bool success = false;
            script->value_37c = static_cast<int>(ScanFloat(&success));
        } else if (tag == "[tower limit of stackable item]") {
            bool success = false;
            script->limit_stackable_item_in_tower =
                static_cast<int>(ScanFloat(&success));
        } else if (tag == "[tower recovery]") {
            bool success = false;
            RecoveryStairInfo recovery_stair;
            recovery_stair.value_00 =
                static_cast<int>(ScanFloat(&success));
            recovery_stair.value_04 =
                static_cast<int>(ScanFloat(&success));
            recovery_stair.value_08 = ScanFloat(&success);
            while (true) {
                RecoveryInfo recovery;
                recovery.value_00 = ScanFloat(&success);
                if (!success) {
                    break;
                }
                recovery.value_04 = ScanFloat(&success);
                recovery.value_08 = ScanFloat(&success);
                recovery_stair.recovery_infos.push_back(recovery);
            }
            script->recovery_stairs.push_back(recovery_stair);
        } else if (tag == "[tower dialog]") {
            bool success = false;
            STTowerDialog dialog;
            dialog.raw_00 = static_cast<uint32_t>(ScanInt(&success));
            if (!success || ScanStr(&dialog.dialog) != 1) {
                return 0;
            }

            std::string timing;
            if (ScanStr(&timing) != 1) {
                return 0;
            }
            if (timing == "on start") {
                dialog.raw_08 = 1;
                script->tower_dialogs.start_dialogs.push_back(dialog);
            } else if (timing == "on complete") {
                dialog.raw_09 = 1;
                script->tower_dialogs.end_dialogs.push_back(dialog);
            } else if (timing == "on die") {
                dialog.raw_0a = 1;
                script->tower_dialogs.clear_dialogs.push_back(dialog);
            } else {
                return 0;
            }
        } else if (tag == "[tower item drop]") {
            bool success = false;
            script->tower_item_drop = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[tower max clear item num]") {
            bool success = false;
            script->tower_max_clear_item_num = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[tower random map indexes]") {
            // IDA ImportDungeonScript case 1781 owns this scalar.
            // Boss-tower DGN files place it before every
            // [tower stage] block; leaving its integer payload unread makes
            // the next outer ScanType fail and silently hides the later
            // [schedule] block from CDailyScheduleManager.
            // The 0627 handler stores this scalar directly; preserve that
            // behavior instead of adding a new validation policy here.
            script->random_tower_max_stage =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[tower stage]") {
            // IDA ImportDungeonScript case 1784: one stage number is followed
            // by map/rate integer pairs until [/tower stage].
            // ScanInt consumes that closing TYPE token when the pair loop
            // ends, exactly as the original parser does.
            bool success = false;
            const int stage = ScanInt(&success);
            if (!success) {
                return 0;
            }

            std::vector<RandomList> maps;
            while (true) {
                const int map_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int rate = ScanInt(&success);
                if (!success || rate == 0) {
                    return 0;
                }

                RandomList entry;
                entry.map_index = map_index;
                entry.weight = rate;
                maps.push_back(entry);
            }

            // std::map::insert's result is intentionally ignored in the
            // recovered handler as well.
            script->random_tower_map_indexes.insert(
                std::make_pair(stage, maps));
        } else if (tag == "[revision table]") {
            ScanStr(&script->value_448);
        } else if (tag == "[gold card use]") {
            // IDA ImportDungeonScript case 2146: consume the numeric switch
            // and store its boolean form in STDungeonScript +0x44c.  Leaving
            // this value unread terminates the outer type scanner before a
            // following [maze info] block can be imported.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_44c = value != 0 ? 1 : 0;
        } else if (tag == "[recommended level]") {
            script->value_450 = ScanInt(static_cast<bool*>(0));
            script->value_454 = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[designate dungeon difficulty]") {
            // 140828 uses this DGN value as the first-entry difficulty
            // contract for more than TimeSpiral (for example the three
            // Chinese Dragon-Nest variants).  Publish it through the common
            // sidecar consumed by CParty::dungeon_start; keeping it only in
            // STDungeonScript::value_490 made ordinary fixed-difficulty
            // dungeons fall back to the historical-clear gate.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || value < 0 || value > 4 ||
                runtime_config.has_fixed_difficulty) {
                return 0;
            }
            script->value_490 = value;
            runtime_config.has_fixed_difficulty = true;
            runtime_config.fixed_difficulty = value;
        } else if (tag == "[tournament dungeon]") {
            bool success = false;
            const int value = ScanInt(&success);
            script->value_494 = static_cast<uint8_t>(value);
            if (!success) {
                return 0;
            }
        } else if (tag == "[tournament round fatigue]") {
            bool success = false;
            script->value_498 = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[result card]") {
            std::string result_tag;
            if (ScanType(result_tag, true) != 1 ||
                result_tag != "[reward item rate]") {
                return 0;
            }

            TournamentRewardItemRate parsed_reward_items;
            while (true) {
                bool success = false;
                TournamentRewardItem item;
                item.item_id = ScanInt(&success);
                if (!success) {
                    if (!LastTokenIsType("[/reward item rate]")) {
                        return 0;
                    }
                    break;
                }
                item.selection_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                item.item_count = ScanInt(&success);
                if (!success || item.item_id <= 0 ||
                    item.selection_rate < 0 || item.item_count <= 0) {
                    return 0;
                }

                const long long total_rate =
                    static_cast<long long>(parsed_reward_items.total_rate) +
                    item.selection_rate;
                if (total_rate > std::numeric_limits<int32_t>::max()) {
                    return 0;
                }
                parsed_reward_items.total_rate =
                    static_cast<int32_t>(total_rate);
                parsed_reward_items.reward_items.push_back(item);
            }
            if (parsed_reward_items.reward_items.empty() ||
                parsed_reward_items.total_rate <= 0) {
                return 0;
            }

            std::map<int, TournamentRewardRate> parsed_rewards;
            while (true) {
                bool success = false;
                const int round = ScanInt(&success);
                if (!success) {
                    if (!LastTokenIsType("[/result card]")) {
                        return 0;
                    }
                    break;
                }

                TournamentRewardRate reward;
                reward.gold_reward_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                reward.item_reward_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                reward.no_reward_rate = ScanInt(&success);
                if (!success) {
                    return 0;
                }
                if (reward.gold_reward_rate < 0 ||
                    reward.item_reward_rate < 0 ||
                    reward.no_reward_rate < 0) {
                    return 0;
                }
                const long long total_rate =
                    static_cast<long long>(reward.gold_reward_rate) +
                    reward.item_reward_rate + reward.no_reward_rate;
                if (total_rate <= 0 ||
                    total_rate > std::numeric_limits<int32_t>::max()) {
                    return 0;
                }
                reward.total_rate = static_cast<int32_t>(total_rate);

                const std::pair<std::map<int, TournamentRewardRate>::iterator,
                                bool> inserted =
                    parsed_rewards.insert(std::make_pair(round, reward));
                if (!inserted.second) {
                    return 0;
                }
            }
            if (parsed_rewards.empty()) {
                return 0;
            }
            script->tournament_reward_items = parsed_reward_items;
            script->tournament_rewards.swap(parsed_rewards);
        } else if (tag == "[tournament clear reward exp]") {
            std::map<uint8_t, uint32_t> parsed_exp_rewards;
            while (true) {
                bool success = false;
                const int round = ScanInt(&success);
                if (!success) {
                    if (!LastTokenIsType(
                            "[/tournament clear reward exp]")) {
                        return 0;
                    }
                    break;
                }
                const int reward_exp = ScanInt(&success);
                if (!success) {
                    return 0;
                }

                const std::pair<
                    std::map<uint8_t, uint32_t>::iterator, bool> inserted =
                    parsed_exp_rewards.insert(std::make_pair(
                        static_cast<uint8_t>(round),
                        static_cast<uint32_t>(reward_exp)));
                if (!inserted.second) {
                    return 0;
                }
            }
            script->tower_reward_rates.swap(parsed_exp_rewards);
        } else if (tag == "[tournament clear reward gold rate]") {
            bool success = false;
            script->tournament_reward_weight = ScanFloat(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[party member coin limit]") {
            bool success = false;
            script->value_4fc = ScanInt(&success);
            if (!success) {
                return 0;
            }
        } else if (tag == "[individual map movement]") {
            // IDA ImportDungeonScript case 2381 stores true only for the
            // literal value one at STDungeonScript+0x504.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_504 = static_cast<uint8_t>(value == 1);
        } else if (tag == "[powerwar dungeon]") {
            // ORIGINAL_SERVER_EA: ImportDungeonScript stores the strict
            // scalar flag at STDungeonScript+0x28d.  set_dungeon already
            // projects that byte to CDungeon+0x89e, whose reward/drop/EXP
            // consumers are restored.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            runtime_config.critical_tail_seen = true;
            runtime_config.power_war = value == 1;
            script->value_28d = static_cast<uint8_t>(value);
        } else if (tag == "[golden commerce dungeon]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 1359.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            runtime_config.critical_tail_seen = true;
            runtime_config.golden_commerce = value == 1;
        } else if (tag == "[move map even enemy]") {
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.move_map_even_enemy = value == 1;
        } else if (tag == "[enter without fatigue]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 1362.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || (value != 0 && value != 1)) {
                return 0;
            }
            runtime_config.enter_without_fatigue = value == 1;
        } else if (tag == "[enable apc stackable]") {
            // CLIENT_EA: DNF.exe ImportDungeonScript case 1365.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success || value < 0 || value > 1) {
                return 0;
            }
            runtime_config.enable_apc_stackable = value;
        } else if (tag == "[limit item stat]") {
            if (runtime_config.has_limit_item_stat) {
                return 0;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_limit_item_stat = true;
            runtime_config.limit_item_stat = value != 0;
        } else if (tag == "[limit escape character]") {
            if (runtime_config.has_limit_escape_character) {
                return 0;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_limit_escape_character = true;
            runtime_config.limit_escape_character = value != 0;
        } else if (tag == "[absolute change stat]") {
            if (!runtime_config.absolute_change_stats.empty() ||
                !script->absolute_change_stats.empty()) {
                return 0;
            }
            bool closed = false;
            while (true) {
                bool is_type = false;
                std::string stat_name;
                if (ScanTypeOrStr(is_type, stat_name, true) != 1) {
                    return 0;
                }
                if (is_type) {
                    if (stat_name != "[/absolute change stat]") {
                        return 0;
                    }
                    closed = true;
                    break;
                }

                AbsoluteChangeStat::T stat;
                if (stat_name == "[move speed]") {
                    stat = AbsoluteChangeStat::MOVE_SPEED;
                } else if (stat_name == "[physical attack]") {
                    stat = AbsoluteChangeStat::PHYSICAL_ATTACK_ADD;
                } else if (stat_name == "[physical defense]") {
                    stat = AbsoluteChangeStat::PHYSICAL_DEFENSE_ADD;
                } else if (stat_name == "[magical attack]") {
                    stat = AbsoluteChangeStat::MAGICAL_ATTACK_ADD;
                } else if (stat_name == "[magical defense]") {
                    stat = AbsoluteChangeStat::MAGICAL_DEFENSE_ADD;
                } else {
                    return 0;
                }

                bool success = false;
                const float value = ScanFloat(&success);
                if (!success ||
                    !runtime_config.absolute_change_stats
                         .insert(std::make_pair(stat, value)).second) {
                    return 0;
                }
            }
            if (!closed || runtime_config.absolute_change_stats.empty()) {
                return 0;
            }
            script->absolute_change_stats =
                runtime_config.absolute_change_stats;
            runtime_config.extension_0828_seen = true;
        } else if (tag == "[character respawn position]") {
            if (runtime_config.has_character_respawn_position) {
                return 0;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_character_respawn_position = true;
            runtime_config.character_respawn_position = value != 0;
        } else if (tag == "[use zoom rate]") {
            if (runtime_config.has_zoom_rate) {
                return 0;
            }
            bool success = false;
            const float value = ScanFloat(&success);
            if (!success || value <= 0.0f) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_zoom_rate = true;
            runtime_config.zoom_rate = value;
        } else if (tag == "[room move keep state]") {
            if (runtime_config.has_room_move_keep_state) {
                return 0;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_room_move_keep_state = true;
            runtime_config.room_move_keep_state = value == 1;
        } else if (tag == "[use countdown]") {
            if (runtime_config.has_use_countdown) {
                return 0;
            }
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            runtime_config.extension_0828_seen = true;
            runtime_config.has_use_countdown = true;
            runtime_config.use_countdown = value == 1;
        } else if (tag == "[open door even enemy]") {
            // IDA ImportDungeonScript case 2388 uses the same literal-one
            // projection for STDungeonScript+0x505.  Leaving this payload
            // unread truncated the remaining 0828 zombie/race fields.
            bool success = false;
            const int value = ScanInt(&success);
            if (!success) {
                return 0;
            }
            script->value_505 = static_cast<uint8_t>(value == 1);
        } else if (tag == "[minimum required level]") {
            int value = 0;
            if (ReadOptionalInt(&value, &pending_tag)) {
                script->min_required_level = value;
            } else if (!pending_tag.empty()) {
                pending = true;
            }
        } else if (tag == "[required item]") {
            int item_id = -1;
            int item_count = 0;
            int delete_after_start = 0;
            if (ReadOptionalInt(&item_id, &pending_tag) &&
                ReadOptionalInt(&item_count, &pending_tag) &&
                ReadOptionalInt(&delete_after_start, &pending_tag)) {
                script->required_item.item_id = item_id;
                script->required_item.item_count = item_count;
                script->required_item.delete_after_start =
                    delete_after_start != 0;
            } else if (!pending_tag.empty()) {
                pending = true;
            }
        } else if (tag == "[name]") {
            ScanStr(&script->name);
        } else if (tag == "[explain]") {
            ScanStr(&script->value_020);
        } else if (tag == "[monsterapc diff table]") {
            std::string monster_apc_diff_table;
            ScanStr(&monster_apc_diff_table);
            script->value_010 = monster_apc_diff_table;
        } else if (tag == "[basis level]") {
            int value = 0;
            if (ReadOptionalInt(&value, &pending_tag)) {
                script->basis_level = value;
            } else if (!pending_tag.empty()) {
                pending = true;
            }
        } else if (tag == "[experience increasing point]") {
            float value = 0.0f;
            if (ReadOptionalFloat(&value, &pending_tag)) {
                script->exp_weight = value;
            } else if (!pending_tag.empty()) {
                pending = true;
            }
        } else if (tag == "[schedule]") {
            std::string start_date;
            std::string end_date;
            std::string weekday_text;
            std::string start_time_text;
            std::string end_time_text;

            while (true) {
                _DailySchedule schedule;
                std::memset(&schedule, 0, sizeof(schedule));
                schedule.schedule_type = DAILY_SCHEDULE_DUNGEON;
                schedule.schedule_id =
                    static_cast<unsigned int>(dungeon_index);

                bool success = false;
                schedule.repeat_event_id() = ScanInt(&success);
                if (!success) {
                    break;
                }

                // IDA treats a missing first date as the normal end of this
                // optional row list.  Missing later fields invalidate the
                // complete dungeon import.
                if (ScanStr(&start_date) != 1) {
                    break;
                }
                if (ScanStr(&end_date) != 1 ||
                    ScanStr(&weekday_text) != 1 ||
                    ScanStr(&start_time_text) != 1 ||
                    ScanStr(&end_time_text) != 1) {
                    return 0;
                }

                long start_date_value = 0;
                long end_date_value = 0;
                unsigned int weekday = 0;
                unsigned char start_hour = 0;
                unsigned char start_minute = 0;
                unsigned char start_second = 0;
                unsigned char end_hour = 0;
                unsigned char end_minute = 0;
                unsigned char end_second = 0;
                if (ConvertStringToTime(start_date, start_date_value) != 1 ||
                    ConvertStringToTime(end_date, end_date_value) != 1 ||
                    ConvertWeekdayStringToInt(weekday_text, weekday) != 1 ||
                    ConvertHHMMSSStringToInt(start_time_text,
                                             start_hour,
                                             start_minute,
                                             start_second) != 1 ||
                    ConvertHHMMSSStringToInt(end_time_text,
                                             end_hour,
                                             end_minute,
                                             end_second) != 1) {
                    return 0;
                }

                schedule.timing.start_date_sec =
                    static_cast<int32_t>(start_date_value);
                schedule.timing.end_date_sec =
                    static_cast<int32_t>(end_date_value);
                schedule.timing.weekday = weekday;
                schedule.timing.start_hour = start_hour;
                schedule.timing.start_minute = start_minute;
                schedule.timing.start_second = start_second;
                schedule.timing.end_hour = end_hour;
                schedule.timing.end_minute = end_minute;
                schedule.timing.end_second = end_second;
                script->daily_schedules.push_back(schedule);
            }
        } else if (tag == "[tower of despair]") {
            // IDA 0x88EF950 stores a literal one at STDungeonScript+0x2a9.
            // This tag has no value payload.
            script->tower_of_despair = 1;
        } else if (tag == "[dungeon type]") {
            std::string dungeon_type;
            if (ScanStr(&dungeon_type) != 1) {
                return 0;
            }

            if (dungeon_type == "tower of death") {
                script->dungeon_kind = 1;
                current_maze.grid_width = 2;
                current_maze.grid_height = 2;
                current_maze.grids.resize(4, GridScript());
            } else if (dungeon_type == "tower of boss") {
                script->dungeon_kind = 3;
                current_maze.grid_width = 2;
                current_maze.grid_height = 2;
                current_maze.grids.resize(4, GridScript());
            } else if (dungeon_type == "advance altar") {
                script->dungeon_kind = 4;
                script->defense_dungeon = 1;
            } else if (dungeon_type == "power dungeon") {
                script->dungeon_kind = 2;
            } else {
                script->dungeon_kind = 0;
            }
        } else {
            const WarRoomTagImportResult war_room_result =
                ImportWarRoomDungeonTag(&script->war_room_data, tag);
            if (war_room_result == WAR_ROOM_TAG_ERROR) {
                return 0;
            }
            if (war_room_result == WAR_ROOM_TAG_HANDLED) {
                // A valueless unknown tag encountered before the first
                // WarRoom field would otherwise disappear from the token
                // stream.  Once this is proven to be a WarRoom DGN, reject
                // that partial projection instead of publishing it.
                if (unhandled_tag_seen_before_war_room) {
                    return 0;
                }
                war_room_data_seen = true;
                continue;
            }
            if (ImportAdvanceAltarDungeonTag(script, tag, filename) == 1) {
                continue;
            }
            if (war_room_data_seen) {
                // All official 0828 WarRoom tags are mapped above.  A marker
                // that belongs to neither the common DGN dispatcher nor the
                // other typed extension is unknown and must fail closed.
                return 0;
            }
            unhandled_tag_seen_before_war_room = true;
        }
    }
    // ScanType returns zero both at true EOF and after encountering an
    // unsupported scalar payload.  Never let either path publish the default
    // MazeScript shell: it makes DataManager startup look successful and then
    // fails much later in SelectDungeon/SetGridPath.
    const DNFLEX_TOKEN termination_token = g_dnfLex.getLastToken();
    if (war_room_data_seen && termination_token != DNF_TOK_EOF) {
        // Fixed-width WarRoom fields must be followed by another type token
        // or true EOF.  Extra scalar payload is malformed, not a compatible
        // tail that may be logged and accepted.
        return 0;
    }
    bool maze_committed = false;
    if (maze_info_started) {
        maze_committed = CommitCompleteDungeonMaze(
            &script->maze_scripts, current_maze);
    } else if (termination_token == DNF_TOK_EOF &&
               IsCompleteSyntheticTowerMaze(*script, current_maze)) {
        // The official [dungeon type] handlers synthesize a coordinate-less
        // 2x2 maze for both death towers and boss towers, then the common
        // ScanType termination path publishes it.  These tower stages use
        // their own map tables and may legally terminate after a numeric tower
        // table rather than a normal [maze info] block.  They are selected
        // from those tables and deliberately do not declare normal start/boss
        // positions.  Admit only that exact typed shape with a complete
        // fixed/random stage table; an ordinary default 1x1/empty shell (the
        // d65 regression) or a truncated tower table still fails closed.
        MazeScript committed(current_maze);
        committed.maze_index =
            static_cast<int32_t>(script->maze_scripts.size());
        script->maze_scripts.push_back(committed);
        maze_committed = true;
    }
    if (!maze_committed) {
        return 0;
    }
    // A recovered 0828 critical tail is atomic: publishing its maze while a
    // scalar/block remains unread recreates the exact Largus failure where
    // difficulty, no-fatigue and the cart definition silently disappeared.
    if ((runtime_config.critical_tail_seen ||
         runtime_config.extension_0828_seen) &&
        termination_token != DNF_TOK_EOF) {
        return 0;
    }
    if (runtime_config.critical_tail_seen &&
        !ValidateDungeonRuntimeConfig(runtime_config)) {
        return 0;
    }

    // The legacy importer still lacks several unrelated post-maze 0828
    // blocks.  Keep those structurally complete official dungeons loadable,
    // but make the compatibility truncation observable instead of silently
    // calling it EOF.
    if (termination_token != DNF_TOK_EOF) {
        std::fprintf(
            stderr,
            "[dungeon-parser] trailing unsupported payload "
            "file=%s line=%d token=%s\n",
            filename ? filename : "(null)",
            g_dnfLex.getLastLineNo(),
            g_dnfLex.getLastTokenDataAsString().c_str());
    }
    script->dungeon_index = dungeon_index;
    // [designate dungeon difficulty] is an ordinary pre-maze scalar, not a
    // member of the atomic 0828 critical tail.  It still owns a typed runtime
    // projection, so publish the sidecar even when no critical-tail tag was
    // present in this DGN.
    if (runtime_config.critical_tail_seen ||
        runtime_config.extension_0828_seen ||
        runtime_config.has_fixed_difficulty) {
        g_dungeon_runtime_configs[dungeon_index] = runtime_config;
    }
    return 1;
}

// IDA_EA: 0x834DCDA
// IDA_NAME: _ZN4CMapC2Ev
// DEMANGLED: CMap::CMap(void)
CMap::CMap() {
    // The declarations in map_runtime.h deliberately retain IDA construction
    // order: four lists, five vectors, the blood-system vectors, TimeLine,
    // then the tournament map.  The original C2 only initializes these two
    // scalar members after those nested objects have been constructed.
    boss_count_ = 0;
    assigned_npc_count_ = 0;
    reserved_12d_[0] = 0;
    reserved_12d_[1] = 0;
    reserved_12d_[2] = 0;
}

// IDA_EA: 0x834EBDA
// IDA_NAME: _ZNK4CMap14box_item_indexESt4listISt4pairIiiESaIS2_EE
// DEMANGLED: CMap::box_item_index(std::list<std::pair<int,int>,std::allocator<std::pair<int,int>>>)const
int CMap::box_item_index(
    std::list<std::pair<int, int> > item_boxes) const {
    const int random_value = static_cast<int>(get_rand_int(100)) + 1;
    int cumulative_rate = 0;

    for (std::list<std::pair<int, int> >::const_iterator it =
             item_boxes.begin();
         it != item_boxes.end();
         ++it) {
        cumulative_rate += it->second;
        // Keep the original strict comparison: a roll equal to the running
        // total continues to the next entry rather than selecting this one.
        if (random_value < cumulative_rate) {
            return it->first;
        }
    }
    return -2;
}

// IDA_EA: 0x834DEF6
// IDA_NAME: _ZN4CMapD2Ev
// DEMANGLED: CMap::~CMap()
CMap::~CMap() {
    destroy();
}

// IDA_EA: 0x834E13E
// IDA_NAME: _ZN4CMap7destroyEv
// DEMANGLED: CMap::destroy(void)
void CMap::destroy() {
    monsters_.clear();
    items_.clear();
    boss_count_ = 0;
    dungeon_lists_.clear();
    event_monster_positions_.clear();
    reserved_12d_[0] = 0;
    reserved_12d_[1] = 0;
    reserved_12d_[2] = 0;
}

// IDA_EA: 0x834ED1E
// IDA_NAME: _ZN4CMap7destoryEv
// DEMANGLED: CMap::destory(void)
void CMap::destory() {
    // The original body is a single `ret`; preserve the exported misspelled
    // method without inventing a second destruction path beside destroy().
}

// IDA_EA: 0x834E18A
// IDA_NAME: _ZN4CMap7set_mapE11STMapScript
// DEMANGLED: CMap::set_map(STMapScript)
int CMap::set_map(STMapScript script) {
    map_index_ = script.map_index;
    map_type_ = script.map_type;
    champion_ = script.champion;
    greed_cells_ = script.greed_cells;
    dungeon_lists_ = script.dungeon_indices;
    blood_systems_ = script.blood_systems;
    blood_phase_times_ = script.blood_phase_times;
    heroes_mode_map_index_ = script.heroes_mode_map_index;
    std::memcpy(pathgate_positions_,
                script.pathgate_positions,
                sizeof(pathgate_positions_));
    std::memcpy(pathgate_recognize_ranges_,
                script.pathgate_recognize_ranges,
                sizeof(pathgate_recognize_ranges_));

    STAssignMonster monster;
    for (std::vector<STAssignMonster>::iterator it = script.monsters.begin();
         it != script.monsters.end();
         ++it) {
        STAssignMonster assigned(*it);
        monster = assigned;
        if (map_type_ == 2 && monster.rank_type == 3) {
            monster.rank_type = 1;
            monster.no_hit_animation = 1;
        }
        if (monster.rank_type == 3) {
            ++boss_count_;
        }
        insert_monster(monster, -1);
    }

    for (std::vector<STAssignMonster>::iterator it =
             script.additional_monsters.begin();
         it != script.additional_monsters.end();
         ++it) {
        STAssignMonster assigned(*it);
        monster = assigned;
        insert_condition_monster(monster);
    }

    bool has_validated_time_spiral_trap = false;
    int special_passive_index = 0;
    for (std::list<STAssignSpecialPassiveObj>::iterator special =
             script.special_passive_objects.begin();
         special != script.special_passive_objects.end();
         ++special, ++special_passive_index) {
        if (time_spiral::IsValidatedTrapSpecialObject(*special)) {
            has_validated_time_spiral_trap = true;
            if (std::find(passive_object_ids_.begin(),
                          passive_object_ids_.end(),
                          special->object_id) == passive_object_ids_.end()) {
                passive_object_ids_.push_back(special->object_id);
            }
        }
        for (std::vector<SPOBJ_ACTION_DATAS>::iterator action =
                 special->actions.begin();
             action != special->actions.end();
             ++action) {
            if (action->action_type != 0) {
                if (action->action_type == 2) {
                    insert_item(action->value_04,
                                action->value_08,
                                action->value_0c,
                                special->action_index);
                }
            } else if (action->value_10 &&
                       static_cast<int>(get_rand_int(100)) <=
                           100 * action->value_0c / action->value_10 &&
                       action->value_08 <= 200 && action->value_08 > 0) {
                monster.Clear();
                monster.monster_id = action->value_04;
                monster.position_x = action->value_08;
                monster.enabled = 0;
                insert_monster(monster, special_passive_index);
            }
        }
    }

    rival_positions_ = script.rival_positions;
    event_monster_positions_ = script.event_monster_positions;
    apc_random_point_ = script.apc_random_point;
    ai_characters_ = script.apc_arrangements;
    assigned_npc_count_ = static_cast<int32_t>(script.npcs.size());
    for (std::list<STAssignPassiveObject>::iterator passive =
             script.passive_objects.begin();
         passive != script.passive_objects.end();
         ++passive) {
        if (std::find(passive_object_ids_.begin(),
                      passive_object_ids_.end(),
                      passive->object_id) == passive_object_ids_.end()) {
            passive_object_ids_.push_back(passive->object_id);
        }
    }
    for (std::vector<int>::const_iterator pathgate =
             script.pathgate_object_ids.begin();
         pathgate != script.pathgate_object_ids.end(); ++pathgate) {
        if (std::find(passive_object_ids_.begin(),
                      passive_object_ids_.end(),
                      *pathgate) == passive_object_ids_.end()) {
            passive_object_ids_.push_back(*pathgate);
        }
    }

    max_monster_drop_exception_ = script.max_monster_drop_exception;
    max_monster_count_exception_ = script.max_monster_count_exception;
    advance_altar_timeline_ = script.advance_altar_timeline;
    script_value_178_ = script.value_178;
    advancealtar::StageTimeLineParameter::checkAndSetActionDefine(
        advance_altar_timeline_,
        G_CDataManager()->get_advance_altar_action_defines());
    tournament_monsters_ = script.tournament_monsters;
    script_value_36e_ = script.value_36e;
    reserved_12d_[0] = script.reserved_36f != 0 ? 1 : 0;
    reserved_12d_[1] = script.tournament_enemy_role_bases[0];
    reserved_12d_[2] = script.tournament_enemy_role_bases[1];
    if (has_validated_time_spiral_trap) {
        time_spiral::RegisterValidatedTrapMap(map_index_);
    }
    npc_item_drop::RegisterMapContract(script);
    return 1;
}

bool CMap::HasPassiveObject(int object_id) const {
    return std::find(passive_object_ids_.begin(),
                     passive_object_ids_.end(),
                     object_id) != passive_object_ids_.end();
}

// IDA_EA: 0x8134776
// IDA_NAME: _ZNK4CMap9get_indexEv
// DEMANGLED: CMap::get_index(void) const
int CMap::get_index() const {
    return map_index_;
}

// IDA_EA: 0x830E5C0
// IDA_NAME: _ZNK4CMap12get_map_typeEv
// DEMANGLED: CMap::get_map_type(void) const
int CMap::get_map_type() const {
    return map_type_;
}

int CMap::getTournamentEnemyRoleBase(int party_member_count) const {
    if (party_member_count < 1 || party_member_count > 2) {
        return 0;
    }
    return reserved_12d_[party_member_count];
}

bool CMap::isComboTraining() const {
    return reserved_12d_[0] != 0;
}

// IDA_EA: 0x822B492
// IDA_NAME: _ZNK4CMap19getAssignedNPCCountEv
// DEMANGLED: CMap::getAssignedNPCCount(void) const
int CMap::getAssignedNPCCount() const {
    return assigned_npc_count_;
}

// IDA_EA: 0x830E5B6
// IDA_NAME: _ZNK4CMap12GetBossCountEv
// DEMANGLED: CMap::GetBossCount(void) const
int CMap::GetBossCount() const {
    return boss_count_;
}

// IDA_EA: 0x830E5CC
// IDA_NAME: _ZNK4CMap26get_condition_monster_listEv
// DEMANGLED: CMap::get_condition_monster_list(void) const
std::list<_mapMonster> CMap::get_condition_monster_list() const {
    return condition_monsters_;
}

// IDA_EA: 0x834E9D4
// IDA_NAME: _ZN4CMap14insert_monsterE15STAssignMonsteri
// DEMANGLED: CMap::insert_monster(STAssignMonster,int)
int CMap::insert_monster(STAssignMonster monster, int special_passive_index) {
    _mapMonster map_monster;
    map_monster.map_index = monster.monster_id;
    map_monster.enabled = monster.enabled;
    map_monster.raw_08 = monster.position_x;
    map_monster.raw_0c = monster.spawn_type;
    map_monster.raw_10 = monster.respawn_delay;
    map_monster.raw_14 = monster.summon_count;
    map_monster.raw_18 = monster.rank_type;
    map_monster.raw_24 = monster.direction / 10;
    map_monster.raw_2c = monster.spawn_order;
    map_monster.raw_28 = monster.team;
    map_monster.raw_30 = monster.no_hit_animation;
    map_monster.raw_34 = monster.champion_features;
    if (special_passive_index != -1) {
        map_monster.raw_1c = 1;
        map_monster.raw_20 = special_passive_index;
    }
    monsters_.push_back(map_monster);
    return 1;
}

// IDA_EA: 0x834EAD4
// IDA_NAME: _ZN4CMap24insert_condition_monsterE15STAssignMonster
// DEMANGLED: CMap::insert_condition_monster(STAssignMonster)
void CMap::insert_condition_monster(STAssignMonster monster) {
    _mapMonster map_monster;
    map_monster.map_index = monster.monster_id;
    map_monster.enabled = monster.enabled;
    map_monster.raw_08 = monster.position_x;
    map_monster.raw_0c = monster.spawn_type;
    map_monster.raw_10 = monster.respawn_delay;
    map_monster.raw_14 = monster.summon_count;
    map_monster.raw_18 = monster.rank_type;
    map_monster.raw_24 = monster.direction / 10;
    map_monster.raw_2c = monster.spawn_order;
    condition_monsters_.push_back(map_monster);
}

// IDA_EA: 0x834EB94
// IDA_NAME: _ZN4CMap11insert_itemEiiii
// DEMANGLED: CMap::insert_item(int,int,int,int)
int CMap::insert_item(int item_id,
                      int item_count,
                      int drop_id,
                      int special_passive_index) {
    _mapItem item;
    item.raw_00 = item_id;
    item.item_id = item_count;
    item.item_count = drop_id;
    item.drop_id = special_passive_index;
    items_.push_back(item);
    return 1;
}
// IDA_EA: 0x836A9A8
// IDA_NAME: _ZN12RequiredItemC2Ev
// DEMANGLED: RequiredItem::RequiredItem(void)
RequiredItem::RequiredItem() {
    clear();
}

// IDA_EA: 0x836A988
// IDA_NAME: _ZN12RequiredItem5clearEv
// DEMANGLED: RequiredItem::clear(void)
RequiredItem* RequiredItem::clear() {
    item_id = -1;
    item_count = 0;
    delete_after_start = 0;
    return this;
}

// IDA_EA: 0x88F2C50
// IDA_NAME: _ZN22STMonsterTypeSpawnProbC2Ev
// DEMANGLED: STMonsterTypeSpawnProb::STMonsterTypeSpawnProb(void)
STMonsterTypeSpawnProb::STMonsterTypeSpawnProb() {
    clear();
}

// IDA_EA: 0x83B6076
// IDA_NAME: _ZN22STMonsterTypeSpawnProbD2Ev
// DEMANGLED: STMonsterTypeSpawnProb::~STMonsterTypeSpawnProb(void)
STMonsterTypeSpawnProb::~STMonsterTypeSpawnProb() {}

// IDA_EA: 0x88F2C64
// IDA_NAME: _ZN22STMonsterTypeSpawnProb5clearEv
// DEMANGLED: STMonsterTypeSpawnProb::clear(void)
STMonsterTypeSpawnProb* STMonsterTypeSpawnProb::clear() {
    group_weights_[0] = 0;
    group_weights_[1] = 0;
    group_weights_[2] = 0;
    group_weights_[3] = 0;
    return this;
}

// IDA_EA: 0x836A1F6
// IDA_NAME: _ZN13STWarRoomDataC2Ev
// DEMANGLED: STWarRoomData::STWarRoomData(void)
STWarRoomData::STWarRoomData() {
    clear();
}

// IDA_EA: 0x836B430
// IDA_NAME: _ZN13STWarRoomDataD2Ev
// DEMANGLED: STWarRoomData::~STWarRoomData()
STWarRoomData::~STWarRoomData() {}

// IDA_EA: 0x836A398
// IDA_NAME: _ZN13STWarRoomData5clearEv
// DEMANGLED: STWarRoomData::clear(void)
STWarRoomData* STWarRoomData::clear() {
    for (int index = 0; index != 3; ++index) {
        header_values_[index] = 0;
    }
    mode_ = 0;
    map_index_ = -1;
    for (int index = 0; index != 4; ++index) {
        monster_groups_[index].clear();
    }
    monster_group_order_.clear();
    for (int index = 0; index != 3; ++index) {
        timing_values_[index] = 0;
    }
    for (int index = 0; index != 4; ++index) {
        spawn_values_[index] = 0;
    }
    for (int index = 0; index != 4; ++index) {
        extra_monster_groups_[index].clear();
    }
    for (int index = 0; index != 4; ++index) {
        extra_group_values_[index] = 0;
    }
    finish_point_reward_factor_ = 0.0f;
    spawn_probs_.clear();
    post_spawn_values_[0] = 0;
    post_spawn_values_[1] = 0;
    for (int index = 0; index != 4; ++index) {
        condition_values_[index] = 0;
    }
    condition_mode_ = 0;
    for (int index = 0; index != 4; ++index) {
        event_values_[index] = 0;
    }
    condition_value_ = 0;
    for (int index = 0; index != 5; ++index) {
        tail_values_[index] = 0;
    }
    return this;
}

// IDA_EA: 0x836A578
// IDA_NAME: _ZN13STWarRoomData4copyEPS_
// DEMANGLED: STWarRoomData::copy(STWarRoomData*)
STWarRoomData* STWarRoomData::copy(STWarRoomData* destination) {
    if (!destination) {
        return 0;
    }
    if (destination == this) {
        return destination;
    }
    // copy() can be reached repeatedly while the dungeon projection is rebuilt.
    // Preserve the recovered deep-copy semantics without appending a second copy
    // of every vector-backed PVF list to a reused destination.
    destination->clear();
    for (int index = 0; index != 3; ++index) {
        destination->header_values_[index] = header_values_[index];
    }
    destination->mode_ = mode_;
    destination->map_index_ = map_index_;
    for (int index = 0; index != 4; ++index) {
        destination->monster_groups_[index].reserve(monster_groups_[index].size());
        for (std::vector<int>::const_iterator value =
                 monster_groups_[index].begin();
             value != monster_groups_[index].end(); ++value) {
            destination->monster_groups_[index].push_back(*value);
        }
        destination->extra_monster_groups_[index].reserve(
            extra_monster_groups_[index].size());
        for (std::vector<int>::const_iterator value =
                 extra_monster_groups_[index].begin();
             value != extra_monster_groups_[index].end(); ++value) {
            destination->extra_monster_groups_[index].push_back(*value);
        }
        destination->event_values_[index] = event_values_[index];
    }
    destination->monster_group_order_.reserve(monster_group_order_.size());
    for (std::vector<int>::const_iterator value = monster_group_order_.begin();
         value != monster_group_order_.end(); ++value) {
        destination->monster_group_order_.push_back(*value);
    }
    for (int index = 0; index != 3; ++index) {
        destination->timing_values_[index] = timing_values_[index];
    }
    for (int index = 0; index != 4; ++index) {
        destination->spawn_values_[index] = spawn_values_[index];
    }
    for (int index = 0; index != 4; ++index) {
        destination->extra_group_values_[index] = extra_group_values_[index];
    }
    destination->finish_point_reward_factor_ = finish_point_reward_factor_;
    destination->spawn_probs_.reserve(spawn_probs_.size());
    for (std::vector<STMonsterTypeSpawnProb>::const_iterator value =
             spawn_probs_.begin();
         value != spawn_probs_.end(); ++value) {
        destination->spawn_probs_.push_back(*value);
    }
    destination->post_spawn_values_[0] = post_spawn_values_[0];
    destination->post_spawn_values_[1] = post_spawn_values_[1];
    for (int index = 0; index != 4; ++index) {
        destination->condition_values_[index] = condition_values_[index];
    }
    destination->condition_mode_ = condition_mode_;
    destination->condition_value_ = condition_value_;
    for (int index = 0; index != 5; ++index) {
        destination->tail_values_[index] = tail_values_[index];
    }
    return destination;
}

// IDA_EA: 0x8373F3C
// IDA_NAME: _ZN8CDungeonC2Ev
// DEMANGLED: CDungeon::CDungeon(void)
CDungeon::CDungeon()
    : dungeon_inheritance_type_(0),
      sao_dungeon_(0),
      tower_scenario_enabled_(0),
      zombie_dungeon_(0),
      event_monster_random_map_value_0_(0),
      event_monster_random_map_value_1_(0),
      multi_start_point_(0),
      usable_stackable_(1),
      use_spectator_mode_(0),
      reserved_943_(0) {
    // IDA writes only these four scalars after the DimensionPartyCount
    // vector.  All other scalar bytes deliberately retain their C2 state.
    dimension_party_count_enabled_ = 0;
    runtime_value_6cc_ = 0;
    enter_each_map_ = 0;
    open_even_enemy_ = 0;
    reserved_937_ = 0;
}

// IDA_EA: 0x8376FD8
// IDA_NAME: _ZN8CDungeonD2Ev
// DEMANGLED: CDungeon::~CDungeon()
CDungeon::~CDungeon() {}

// IDA_EA: 0x837468E
// IDA_NAME: _ZNK8CDungeon23_do_after_dungeon_startEP5CUser
// DEMANGLED: CDungeon::_do_after_dungeon_start(CUser*)const
int CDungeon::_do_after_dungeon_start(CUser* user) const {
    (void)user;
    return 1;
}

// IDA_EA: 0x834ACC4
// IDA_NAME: _ZN8CDungeon11set_dungeonER15STDungeonScript
// DEMANGLED: CDungeon::set_dungeon(STDungeonScript &)
//
// The importer now reconstructs the full typed advance-altar block.  These
// fields use the original vector and reward-object lifecycles, so transfer
// them in the same order as the old set_dungeon body.
CDungeon* CDungeon::set_dungeon(STDungeonScript& script) {
    dungeon_index_ = script.dungeon_index;
    standard_level_ = script.basis_level;
    min_level_ = script.min_required_level;
    exp_weight_ = script.exp_weight;
    if (script.dungeon_index == time_spiral::kDungeonIndex &&
        script.tower_scenario_enabled) {
        time_spiral::RegisterDesignatedDifficulty(
            script.dungeon_index, script.value_490);
    }

    char converted_name[500] = {};
    std::strcpy(converted_name, script.name.c_str());
    if (CodePage::script2Database(converted_name, converted_name) != 1) {
        cMyTrace trace("void CDungeon::set_dungeon(STDungeonScript&)",
                       0x58a,
                       5);
        trace("Error!!! %s->%s [%s][%s][%d]",
              CodePage::script(),
              CodePage::database(),
              script.name.c_str(),
              converted_name,
              dungeon_index_);
    }
    dungeon_name_ = converted_name;

    std::copy(script.value_0b4,
              script.value_0b4 + 5,
              champion_spawn_rates_);
    heroes_required_quest_ = script.heroes_required_quest;
    required_item_ = script.required_item;
    if (script.tower_of_despair) {
        dungeon_inheritance_type_ = 1;
    }
    dimension_possible_ = script.dimension_possible;
    dimension_inout_count_ = script.dimension_inout_count;
    dimension_min_party_members_ = script.dimension_min_party_members;
    if (dimension_possible_) {
        G_CDataManager()->set_dimensionInout(
            static_cast<int>(dimension_possible_) - 1,
            dimension_inout_count_);
        G_CDataManager()->set_original_dimensionInout(
            static_cast<int>(dimension_possible_) - 1,
            dimension_inout_count_);
    }
    limit_party_count_ = script.limit_party_count;
    limit_inout_count_ = script.limit_inout_count;
    // 140828 moved generic per-character entry limits out of DGN files and
    // into DungeonInCountInfo.etc.  Project every table-owned maximum onto
    // CDungeon; the two legacy blood owners below still publish their values
    // into CDataManager's historical enum slots.
    if (limit_inout_count_ == 0) {
        limit_inout_count_ = static_cast<uint8_t>(
            ResolveDungeonInCountMaximum(dungeon_index_, 0));
    }
    if (dungeon_index_ == 11007) {
        // The 0828 DGN omits this scalar.  DungeonInCountInfo.etc is the
        // authoritative owner; retain the official 3-entry fallback for the
        // 0627 layout where that table is absent.
        if (limit_inout_count_ == 0) {
            limit_inout_count_ = static_cast<uint8_t>(
                ResolveDungeonInCountMaximum(11007, 3));
        }
        G_CDataManager()->set_limit_inout_count(limit_inout_count_, 2);
    }
    if (dungeon_index_ == 11006) {
        // The corresponding official-server fallback for Endless Altar is 5.
        if (limit_inout_count_ == 0) {
            limit_inout_count_ = static_cast<uint8_t>(
                ResolveDungeonInCountMaximum(11006, 5));
        }
        G_CDataManager()->set_limit_inout_count(limit_inout_count_, 1);
    }

    for (std::vector<MazeScript>::const_iterator maze =
             script.maze_scripts.begin();
         maze != script.maze_scripts.end();
         ++maze) {
        if (maze->value_4c == 0) {
            quest_maze_scripts_[0].push_back(*maze);
        } else if (maze->value_4c == 1) {
            quest_maze_scripts_[1].push_back(*maze);
        } else if (maze->value_4c == -1) {
            maze_scripts_.push_back(*maze);
        }
    }

    clear_item_rates_.clear();
    int cumulative_rate = 0;
    for (std::vector<std::pair<int, int> >::const_iterator item =
             script.dungeon_drop_items.begin();
         item != script.dungeon_drop_items.end();
         ++item) {
        if (item->first != 0) {
            cumulative_rate += item->second;
            clear_item_rates_.insert(
                std::make_pair(cumulative_rate, item->first));
            if (cumulative_rate > 100) {
                LogManager::logFormat(
                    1,
                    "data_manager.cpp",
                    "void CDungeon::set_dungeon(STDungeonScript&)",
                    0x5e1,
                    "total_rate(%d) > 100",
                    cumulative_rate);
                break;
            }
        }
    }
    drop_items_ = script.dungeon_object_drop_items;
    object_drop_items_ = script.value_0e0;
    special_passive_object_items_ =
        script.special_passive_object_items;
    script.war_room_data.copy(&war_room_data_);

    additional_required_item_ = script.event_required_item;
    recovery_stair_items_ = script.clear_reward_items;
    revenge_dungeon_min_party_members_ =
        script.revenge_dungeon_min_party_members;
    revenge_dungeon_max_party_members_ =
        script.revenge_dungeon_max_party_members;
    village_attack_revenge_dungeon_ =
        script.village_attack_revenge_dungeon ? 1 : 0;
    revenge_dungeon_entry_limit_ = script.revenge_dungeon_entry_limit;
    coin_limit_ = script.coin_limit;
    std::copy(script.value_128,
              script.value_128 + 5,
              recovery_stair_values_);
    script_value_894_ = script.value_13c;
    script_value_898_ = script.value_168;
    ancient_dungeon_ = script.ancient_dungeon;
    hell_dungeon_ = script.value_140;
    circulation_statistic_class_ = script.value_28d;
    blood_dungeon_type_ = script.value_28e;

    defense_dungeon_ = script.defense_dungeon;
    advance_altar_stage_type_ = script.value_2ac;
    std::copy(script.value_2b0,
              script.value_2b0 + 3,
              script_values_808_);
    dungeon_runtime_values_ = script.value_2c8;
    std::copy(script.value_2bc,
              script.value_2bc + 3,
              script_values_820_);
    clear_reward_data_ = script.clear_reward_data;
    survival_reward_data_ = script.survival_reward_data;
    event_dungeon_ = script.event_dungeon;

    has_dummy_maps_ = 0;
    dummy_appear_count_ = script.value_16c;
    ai_character_appear_rate_ = script.value_338;
    dungeon_kind_ = script.dungeon_kind;
    map_index_weight_count_ = script.value_368;
    if (map_index_weight_count_ != 0) {
        map_index_weights_.reserve(map_index_weight_count_ + 1);
        for (std::vector<std::pair<int, int> >::const_iterator value =
                 script.value_35c.begin();
             value != script.value_35c.end();
             ++value) {
            map_index_weights_[value->first] = value->second;
        }
    }
    random_tower_max_stage_ = script.random_tower_max_stage;
    if (random_tower_max_stage_ != 0) {
        random_tower_map_indexes_ = script.random_tower_map_indexes;
    }

    blood_max_round_ = script.value_3c4;
    named_monster_positions_ = script.named_monster_positions;
    try_gen_event_monster_count_ =
        script.try_gen_event_monster_count;
    try_gen_event_monster_probability_ =
        script.try_gen_event_monster_probability;
    event_monster_total_probability_ =
        script.event_monster_total_probability;
    event_monsters_ = script.event_monsters;
    try_gen_second_event_monster_count_ =
        script.try_gen_second_event_monster_count;
    try_gen_second_event_monster_probability_ =
        script.try_gen_second_event_monster_probability;
    second_event_monster_total_probability_ =
        script.second_event_monster_total_probability;
    second_event_monsters_ = script.second_event_monsters;
    limit_stackable_item_in_tower_ =
        script.limit_stackable_item_in_tower == 1;
    tower_item_drop_ = script.tower_item_drop == 1;
    runtime_value_6b8_ = script.tower_max_clear_item_num;
    monster_difficulty_bonus_ = script.monster_difficulty_bonus;
    no_fatigue_dungeon_ = script.value_48c;
    quest_npc_dungeon_ = script.value_2aa;
    tournament_dungeon_state_ = script.value_494;
    script_value_8a4_ = script.value_498;
    tournament_reward_items_ = script.tournament_reward_items;
    tournament_rewards_ = script.tournament_rewards;
    tournament_reward_weight_ = script.tournament_reward_weight;
    tower_reward_rates_ = script.tower_reward_rates;
    use_coin_difficulty_ = script.use_coin_difficulty;
    dimension_party_counts_.swap(script.dimension_party_counts);
    dimension_party_count_enabled_ = script.value_4e0;
    runtime_value_6cc_ = script.value_4fc;
    enter_each_map_ = script.value_504;
    open_even_enemy_ = script.value_505;
    enter_boss_room_condition_ = script.enter_boss_room_condition;
    // CLIENT_EA: CDungeon_LoadScriptData @ 0x9D8BB0.
    sao_dungeon_ = script.sao_dungeon;
    tower_scenario_enabled_ = script.tower_scenario_enabled;
    zombie_dungeon_ = script.zombie_dungeon;
    event_monster_random_map_value_0_ =
        script.event_monster_random_map_value_0;
    event_monster_random_map_value_1_ =
        script.event_monster_random_map_value_1;
    multi_start_point_ = script.multi_start_point;
    usable_stackable_ = script.usable_stackable;
    use_spectator_mode_ = script.use_spectator_mode;
    reserved_943_ = 0;
    return this;
}

// IDA_EA: 0x834B988
// IDA_NAME: _ZN8CDungeon9InsertMapEiiP4CMap
// DEMANGLED: CDungeon::InsertMap(int,int,CMap *)
CDungeon* CDungeon::InsertMap(int area, int direction, CMap* map) {
    DirectionMapList* target = &normal_maps_[area];
    if (map->get_map_type() != 0) {
        if (map->get_map_type() == 2) {
            target = &dummy_maps_[area];
            has_dummy_maps_ = 1;
        } else {
            target = &boss_maps_[area];
        }
    }
    if (direction & 1) {
        target->insert(std::make_pair(1, map));
    }
    if (direction & 2) {
        target->insert(std::make_pair(2, map));
    }
    if (direction & 4) {
        target->insert(std::make_pair(4, map));
    }
    if (direction & 8) {
        target->insert(std::make_pair(8, map));
    }
    return this;
}

// IDA_EA: 0x834BE5A
// IDA_NAME: _ZN8CDungeon11insertStartEiP4CMap
// DEMANGLED: CDungeon::insertStart(int,CMap *)
int CDungeon::insertStart(int area, CMap* map) {
    start_maps_[area].insert(std::make_pair(map->get_index(), map));
    return 1;
}

// IDA_EA: 0x80FDCF0
// IDA_NAME: _ZNK8CDungeon9get_indexEv
// DEMANGLED: CDungeon::get_index(void) const
int CDungeon::get_index() const {
    return dungeon_index_;
}

// IDA_EA: 0x814559A
// IDA_NAME: _ZNK8CDungeon13get_min_levelEv
// DEMANGLED: CDungeon::get_min_level(void) const
int CDungeon::get_min_level() const {
    return min_level_;
}

// IDA_EA: 0x80F9810
// IDA_NAME: _ZNK8CDungeon18get_standard_levelEv
// DEMANGLED: CDungeon::get_standard_level(void) const
int CDungeon::get_standard_level() const {
    return standard_level_;
}

// IDA_EA: 0x846993A
// IDA_NAME: _ZNK8CDungeon21getDeathTowerMapIndexEj
// DEMANGLED: CDungeon::getDeathTowerMapIndex(unsigned int)const
int CDungeon::getDeathTowerMapIndex(unsigned int stage) const {
    return map_index_weights_[stage];
}

// IDA_EA: 0x846995C
// IDA_NAME: _ZNK8CDungeon21getDeathTowerMaxStageEv
// DEMANGLED: CDungeon::getDeathTowerMaxStage(void)const
int CDungeon::getDeathTowerMaxStage() const {
    return map_index_weight_count_;
}

// IDA_EA: 0x8484854
// IDA_NAME: _ZNK8CDungeon22getHeroesRequiredQuestEv
// DEMANGLED: CDungeon::getHeroesRequiredQuest(void)const
int CDungeon::getHeroesRequiredQuest() const {
    return heroes_required_quest_;
}

// IDA_EA: 0x81455A6
// IDA_NAME: _ZNK8CDungeon14GetDungeonNameEv
// DEMANGLED: CDungeon::GetDungeonName(void) const
const char* CDungeon::GetDungeonName() const {
    return dungeon_name_.c_str();
}

// IDA_EA: 0x85BED90
// IDA_NAME: _ZNK8CDungeon14get_exp_weightEv
// DEMANGLED: CDungeon::get_exp_weight(void) const
long double CDungeon::get_exp_weight() const {
    return exp_weight_;
}

// IDA_EA: 0x822B4DE
// IDA_NAME: _ZNK8CDungeon18isNoFatigueDungeonEv
// DEMANGLED: CDungeon::isNoFatigueDungeon(void) const
int CDungeon::isNoFatigueDungeon() const {
    return no_fatigue_dungeon_;
}

// CLIENT_EA: 0x9CA850
// CLIENT_NAME: CDungeon_IsSAODungeon
bool CDungeon::isSAODungeon() const {
    return sao_dungeon_ != 0;
}

// IDA_EA: 0x826B928
// IDA_NAME: _ZNK8CDungeon22get_blood_dungeon_typeEv
// DEMANGLED: CDungeon::get_blood_dungeon_type(void) const
int CDungeon::get_blood_dungeon_type() const {
    return blood_dungeon_type_;
}

// IDA_EA: 0x826B938
// IDA_NAME: _ZNK8CDungeon22get_dimension_possibleEv
// DEMANGLED: CDungeon::get_dimension_possible(void) const
int CDungeon::get_dimension_possible() const {
    return dimension_possible_;
}

// IDA_EA: 0x822B4C0
// IDA_NAME: _ZNK8CDungeon14getDungeonKindEv
// DEMANGLED: CDungeon::getDungeonKind(void) const
int CDungeon::getDungeonKind() const {
    return dungeon_kind_;
}

// IDA_EA: 0x822B4CE
// IDA_NAME: _ZNK8CDungeon27limitOfStackableItemInTowerEv
// DEMANGLED: CDungeon::limitOfStackableItemInTower(void) const
int CDungeon::limitOfStackableItemInTower() const {
    return limit_stackable_item_in_tower_;
}

// IDA_EA: 0x8365298
// IDA_NAME: _ZNK8CDungeon25checkFreeRevivalConditionEi
// DEMANGLED: CDungeon::checkFreeRevivalCondition(int) const
int CDungeon::checkFreeRevivalCondition(int dungeon_diff) const {
    const std::map<int, UseCoinDungeonDiff>::const_iterator it =
        use_coin_difficulty_.find(dungeon_diff);
    return it != use_coin_difficulty_.end() && it->second.difficulty != 1;
}

// IDA_EA: 0x8365316
// IDA_NAME: _ZNK8CDungeon16getLimitCoinDiffEiRi
// DEMANGLED: CDungeon::getLimitCoinDiff(int,int &)const
int CDungeon::getLimitCoinDiff(int dungeon_diff, int& coin_limit) const {
    if (use_coin_difficulty_.empty()) {
        return 0;
    }
    const std::map<int, UseCoinDungeonDiff>::const_iterator it =
        use_coin_difficulty_.find(dungeon_diff);
    if (it == use_coin_difficulty_.end()) {
        return 0;
    }
    // UseCoinDungeonDiff +0 is the integer coin limit; +4 is the independent
    // free-revival flag consumed by checkFreeRevivalCondition.
    coin_limit = it->second.dungeon_index;
    return 1;
}

// IDA_EA: 0x834D320
// IDA_NAME: _ZN12CDungeonListC2Ev
// DEMANGLED: CDungeonList::CDungeonList(void)
CDungeonList::CDungeonList() : dungeons_() {}

// IDA_EA: 0x834D352
// IDA_NAME: _ZN12CDungeonListD2Ev
// DEMANGLED: CDungeonList::~CDungeonList()
CDungeonList::~CDungeonList() {
    destroy();
}

// IDA_EA: 0x834D376
// IDA_NAME: _ZN12CDungeonList9load_listEv
// DEMANGLED: CDungeonList::load_list(void)
int CDungeonList::load_list() {
    g_dungeon_runtime_configs.clear();
    if (!LoadDungeonInCountConfig()) {
        std::printf("Dungeon InCount Script Error: "
                    "etc/dungeonetc/dungeonincountinfo.etc\n");
        return 0;
    }

    STDungeonScript script;
    for (STScriptFileList::iterator it =
             g_stDungeonScriptFileList.begin();
         it != g_stDungeonScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            return 1;
        }

        if (ImportDungeonScript(&script, filename) != 1) {
            std::printf("Dungeon Script Error: %s\n", filename);
            return 0;
        }

        if (!script.daily_schedules.empty()) {
            GlobalData::s_DailyScheduleManager->AddDailSchedule(
                script.daily_schedules);
        }

        CDungeon* dungeon =
            static_cast<CDungeon*>(::operator new(sizeof(CDungeon)));
        if (script.tower_of_despair) {
            new (dungeon) CDungeon_TowerOfDespair();
        } else {
            new (dungeon) CDungeon();
        }
        dungeon->set_dungeon(script);
        // IDA 0x834D4FA ignores insert_dungeon's return value here.
        insert_dungeon(dungeon);
        G_CGameManager()->GetQuickPartySystemManager()->init_quick_party_pool(
            dungeon);
        if (size() > 1000) {
            cMyTrace trace("bool CDungeonList::load_list()", 0x8ec, 5);
            trace("Best Clear Time Save Dungeon Index Over Error: %d\n", size());
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x834D568
// IDA_NAME: _ZN12CDungeonList7destroyEv
// DEMANGLED: CDungeonList::destroy(void)
int CDungeonList::destroy() {
    for (DungeonHash::iterator it = dungeons_.begin(); it != dungeons_.end();
         ++it) {
        if (it->second) {
            it->second->~CDungeon();
            ::operator delete(it->second);
        }
    }
    dungeons_.clear();
    return 1;
}

// IDA_EA: 0x834D6D0
// IDA_NAME: _ZN12CDungeonList14insert_dungeonEP8CDungeon
// DEMANGLED: CDungeonList::insert_dungeon(CDungeon *)
int CDungeonList::insert_dungeon(CDungeon* dungeon) {
    if (!dungeon) {
        return 0;
    }
    const int index = dungeon->get_index();
    if (dungeons_.find(index) != dungeons_.end()) {
        std::printf("Already registed dungeon index detected : %d\n", index);
        return 0;
    }
    dungeons_.insert(std::make_pair(index, dungeon));
    return 1;
}

// IDA_EA: 0x835B67C
// IDA_NAME: _ZNK12CDungeonList12find_dungeonEi
// DEMANGLED: CDungeonList::find_dungeon(int) const
CDungeon* CDungeonList::find_dungeon(int dungeon_index) const {
    DungeonHash::const_iterator it = dungeons_.find(dungeon_index);
    return it == dungeons_.end() ? 0 : it->second;
}

// IDA_EA: 0x835B6AA
// IDA_NAME: _ZNK12CDungeonList20get_dungeon_at_indexEi
// DEMANGLED: CDungeonList::get_dungeon_at_index(int) const
CDungeon* CDungeonList::get_dungeon_at_index(int ordinal) const {
    if (ordinal < 0) {
        return 0;
    }
    int current = 0;
    for (DungeonHash::const_iterator it = dungeons_.begin();
         it != dungeons_.end(); ++it, ++current) {
        if (current == ordinal) {
            return it->second;
        }
    }
    return 0;
}

// IDA_EA: 0x835B710
// IDA_NAME: _ZN12CDungeonList19getDungeonIndexListEv
// DEMANGLED: CDungeonList::getDungeonIndexList(void)
CDungeonList* CDungeonList::getDungeonIndexList() {
    return this;
}

// IDA_EA: 0x834D744
// IDA_NAME: _ZN12CDungeonList4sizeEv
// DEMANGLED: CDungeonList::size(void)
int CDungeonList::size() {
    return static_cast<int>(dungeons_.size());
}
std::map<int, PvPMapInfo> CMapList::mapPvpMapInfo_;

// IDA_EA: 0x834EDA0
// IDA_NAME: _ZN8CMapListC2Ev
// DEMANGLED: CMapList::CMapList(void)
CMapList::CMapList()
    : maps_(),
      normal_maps_(),
      boss_maps_(),
      dummy_maps_(),
      pvp_map_count_(0) {}

// IDA_EA: 0x834EF00
// IDA_NAME: _ZN8CMapListD2Ev
// DEMANGLED: CMapList::~CMapList()
CMapList::~CMapList() {
    destroy();
}

// IDA_EA: 0x834F0C0
// IDA_NAME: _ZN8CMapList7destroyEv
// DEMANGLED: CMapList::destroy(void)
bool CMapList::destroy() {
    npc_item_drop::ClearMapContracts();
    map_runtime::ClearMapExtensions0828();
    for (MapHash::iterator it = maps_.begin(); it != maps_.end(); ++it) {
        if (it->second) {
            it->second->~CMap();
            ::operator delete(it->second);
        }
    }
    maps_.clear();
    for (int i = 0; i < 16; ++i) {
        normal_maps_[i].clear();
        boss_maps_[i].clear();
        dummy_maps_[i].clear();
    }
    pvp_map_count_ = 0;
    return true;
}

// IDA_EA: 0x834F18E
// IDA_NAME: _ZN8CMapList10insert_mapEiiP4CMapRSt6vectorI11PVPMAP_DATASaIS3_EE
// DEMANGLED: CMapList::insert_map(int,int,CMap *,std::vector<PVPMAP_DATA> &)
int CMapList::insert_map(int area,
                         int direction,
                         CMap* map,
                         std::vector<PVPMAP_DATA>& pvp_map_data) {
    if (!map) {
        return 0;
    }

    const int map_index = map->get_index();
    if (find_map(map_index)) {
        return 0;
    }

    if (map_index > 50000 && map_index <= 51000) {
        ++pvp_map_count_;

        PvPMapInfo info;
        info.map_index = static_cast<uint32_t>(map_index);
        info.enabled = 0;

        int pvp_order = 0;
        for (std::vector<PVPMAP_DATA>::const_iterator it =
                 pvp_map_data.begin();
             it != pvp_map_data.end(); ++it, ++pvp_order) {
            if (static_cast<int>(it->map_index) == map_index) {
                break;
            }
        }
        mapPvpMapInfo_[pvp_order] = info;
    }

    maps_.insert(std::make_pair(map_index, map));
    if (area < 0 || area >= 16) {
        return 1;
    }

    const int map_type = map->get_map_type();
    if (map_type == 0) {
        normal_maps_[area].insert(std::make_pair(direction, map));
    } else if (map_type == 2) {
        dummy_maps_[area].insert(std::make_pair(direction, map));
    } else {
        boss_maps_[area].insert(std::make_pair(direction, map));
    }
    return 1;
}

// IDA_EA: 0x834F43E
// IDA_NAME: _ZN8CMapList8find_mapEi
// DEMANGLED: CMapList::find_map(int)
CMap* CMapList::find_map(int map_index) {
    MapHash::iterator it = maps_.find(map_index);
    return it == maps_.end() ? 0 : it->second;
}

// IDA_EA: 0x834F4BA
// IDA_NAME: _ZN8CMapList24LoadPvPSinglePlayableMapEPc
// DEMANGLED: CMapList::LoadPvPSinglePlayableMap(char *)
void CMapList::LoadPvPSinglePlayableMap(char* filename) {
    CNRDScriptData script;
    if (script.import(filename) != 1) {
        return;
    }

    for (std::map<int, PvPMapInfo>::iterator it =
             mapPvpMapInfo_.begin();
         it != mapPvpMapInfo_.end(); ++it) {
        char key[128] = {};
        std::snprintf(key,
                      sizeof(key),
                      "pvp deathmatch single play %u",
                      it->second.map_index);
        it->second.enabled = script.getInt(key) == 1 ? 1 : 0;
    }
}

// IDA_EA: 0x84B414E
// IDA_NAME: _ZN8CMapList10GetMapListEv
// DEMANGLED: CMapList::GetMapList(void)
CMapList* CMapList::GetMapList() {
    return this;
}

// IDA_EA: 0x85DF796
// IDA_NAME: _ZNK8CMapList14GetPVPMapCountEv
// DEMANGLED: CMapList::GetPVPMapCount(void) const
int CMapList::GetPVPMapCount() const {
    return pvp_map_count_;
}
