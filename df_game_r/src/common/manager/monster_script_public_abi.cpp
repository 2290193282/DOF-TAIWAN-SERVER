#include "common/manager/monster_script_public_abi.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <string>
#include <vector>

std::vector<std::vector<MonsterBaseParameter> > g_monsterBaseParameters
    __asm__("g_monsterBaseParameters");
float DIFFICULTY_OF_DUNGEON_BONUS[13 * 5]
    __asm__("DIFFICULTY_OF_DUNGEON_BONUS");
float DEATH_TOWER_BONUS[13 * 4] __asm__("DEATH_TOWER_BONUS");
float BLOOD_SYSTEM_BONUS[13 * 5] __asm__("BLOOD_SYSTEM_BONUS");
std::vector<std::vector<float> > MONSTER_DIFFICULTY_BONUS[5]
    __asm__("MONSTER_DIFFICULTY_BONUS");
std::vector<DungeonPartyBalance> DUNGEON_PARTY_BALANCE_TABLE
    __asm__("DUNGEON_PARTY_BALANCE_TABLE");
std::vector<DungeonHoldBalance> NORMAL_MONSTER_HOLD_BALANCE_TABLE
    __asm__("NORMAL_MONSTER_HOLD_BALANCE_TABLE");
std::vector<DungeonHoldBalance> NAMED_MONSTER_HOLD_BALANCE_TABLE
    __asm__("NAMED_MONSTER_HOLD_BALANCE_TABLE");
std::vector<DungeonHoldBalance> BOSS_MONSTER_HOLD_BALANCE_TABLE
    __asm__("BOSS_MONSTER_HOLD_BALANCE_TABLE");
std::vector<stLimitSlowDown> NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE");
std::vector<stLimitSlowDown> NAMED_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("NAMED_MONSER_LIMIT_SLOWDOWN_TABLE");
std::vector<stLimitSlowDown> BOSS_MONSER_LIMIT_SLOWDOWN_TABLE
    __asm__("BOSS_MONSER_LIMIT_SLOWDOWN_TABLE");
float NORMAL_MONSTER_GAUGE_ADJUST __asm__("NORMAL_MONSTER_GAUGE_ADJUST");
float NAMED_MONSTER_GAUGE_ADJUST __asm__("NAMED_MONSTER_GAUGE_ADJUST");
float BOSS_MONSTER_GAUGE_ADJUST __asm__("BOSS_MONSTER_GAUGE_ADJUST");

extern const char* const MONSTER_BASIS_PARAMETER_TABLE_NAMES[5]
    __asm__("MONSTER_BASIS_PARAMETER_TABLE_NAMES") = {
        "CommonMonsterBaseParameter.tbl",
        "NamedMonsterBaseParameter.tbl",
        "BossMonsterBaseParameter.tbl",
        "SummonMonsterBaseParameter.tbl",
        "WarRoomMonsterBaseParameter.tbl",
};


char g_szMonsterScriptBaseDirectory[256]
    __asm__("g_szMonsterScriptBaseDirectory");


// IDA_EA: 0x8A3CE4A
// IDA_NAME: _ZN20MonsterBaseParameter5clearEv
// DEMANGLED: MonsterBaseParameter::clear(void)
MonsterBaseParameter* MonsterBaseParameter::clear() {
    monster_level = 0;
    hp_max = 0;
    mp_max = 0;
    physical_attack = 0;
    physical_defense = 0;
    magical_attack = 0;
    magical_defense = 0;
    equipment_physical_attack = 0;
    equipment_physical_defense = 0;
    equipment_magical_attack = 0;
    equipment_magical_defense = 0;
    fractional_combat_bonus_x100 = 0;
    stuck_resistance_bonus_x10 = 0;
    for (int index = 0; index <= 3; ++index) {
        element_tolerance[index] = 0;
        element_attack[index] = 0;
    }
    hit_recovery = 0;
    rigidity = 0;
    active_status_resistance = 0;
    return this;
}

// IDA_EA: 0x8A3CF2C
// IDA_NAME: _ZN20MonsterBaseParameterC2Ev
// DEMANGLED: MonsterBaseParameter::MonsterBaseParameter(void)
MonsterBaseParameter::MonsterBaseParameter() {
    clear();
}

// IDA_EA: 0x8A4AF16
// IDA_NAME: _ZN20MonsterBaseParameterD2Ev
// DEMANGLED: MonsterBaseParameter::~MonsterBaseParameter()
MonsterBaseParameter::~MonsterBaseParameter() {}

// IDA_EA: 0x8A3DF02
// IDA_NAME: _ZN15stLimitSlowDown5clearEv
// DEMANGLED: stLimitSlowDown::clear(void)
stLimitSlowDown* stLimitSlowDown::clear() {
    raw_00 = 0;
    raw_04 = 0;
    return this;
}

// IDA_EA: 0x8A3DF1A
// IDA_NAME: _ZN15stLimitSlowDownC2Ev
// DEMANGLED: stLimitSlowDown::stLimitSlowDown(void)
stLimitSlowDown::stLimitSlowDown() {
    clear();
}

// IDA_EA: 0x8A4BADC
// IDA_NAME: _ZN15stLimitSlowDownD2Ev
// DEMANGLED: stLimitSlowDown::~stLimitSlowDown()
stLimitSlowDown::~stLimitSlowDown() {}

// IDA_EA: 0x8A11311
// IDA_NAME: _Z17InitMonsterScriptPKcS0_S0_
// DEMANGLED: InitMonsterScript(char const*,char const*,char const*)
int InitMonsterScript(const char* src,
                      const char* list_file,
                      const char* bonus_table) {
    if (list_file != 0) {
        std::strncpy(g_szMonsterScriptBaseDirectory, src, 0x100u);
        if (g_szMonsterScriptBaseDirectory[
                    std::strlen(g_szMonsterScriptBaseDirectory) - 1] !=
                '\\' &&
            g_szMonsterScriptBaseDirectory[
                    std::strlen(g_szMonsterScriptBaseDirectory) - 1] !=
                '/') {
            const size_t length =
                std::strlen(g_szMonsterScriptBaseDirectory);
            std::memcpy(g_szMonsterScriptBaseDirectory + length,
                        "\\",
                        2u);
        }

        if (loadRDARScriptListFile(
                &g_stMonsterScriptFileList,
                g_szMonsterScriptBaseDirectory,
                list_file) != 1) {
            return 0;
        }

        g_monsterBaseParameters.resize(
            5,
            std::vector<MonsterBaseParameter>());
        for (int table_index = 0; table_index <= 4; ++table_index) {
            if (!loadRDARScriptFile(
                    g_szMonsterScriptBaseDirectory,
                    MONSTER_BASIS_PARAMETER_TABLE_NAMES[table_index])) {
                return 0;
            }

            bool success = false;
            MonsterBaseParameter row;
            for (unsigned int index = 0;; ++index) {
                const int monster_index = ScanInt(&success);
                if (!success) {
                    break;
                }

                row.monster_level = monster_index;
                row.physical_attack = ScanInt(static_cast<bool*>(0));
                row.physical_defense = ScanInt(static_cast<bool*>(0));
                row.magical_attack = ScanInt(static_cast<bool*>(0));
                row.magical_defense = ScanInt(static_cast<bool*>(0));
                row.hp_max = ScanInt(static_cast<bool*>(0));
                row.mp_max = ScanInt(static_cast<bool*>(0));
                row.equipment_physical_attack =
                    ScanInt(static_cast<bool*>(0));
                row.equipment_physical_defense =
                    ScanInt(static_cast<bool*>(0));
                row.equipment_magical_attack =
                    ScanInt(static_cast<bool*>(0));
                row.equipment_magical_defense =
                    ScanInt(static_cast<bool*>(0));
                row.fractional_combat_bonus_x100 =
                    ScanInt(static_cast<bool*>(0));
                row.stuck_resistance_bonus_x10 =
                    ScanInt(static_cast<bool*>(0));
                for (int field = 0; field <= 3; ++field) {
                    row.element_tolerance[field] =
                        ScanInt(static_cast<bool*>(0));
                }
                for (int field = 0; field <= 3; ++field) {
                    row.element_attack[field] =
                        ScanInt(static_cast<bool*>(0));
                }
                row.hit_recovery = ScanInt(static_cast<bool*>(0));
                row.rigidity = ScanInt(static_cast<bool*>(0));
                row.active_status_resistance =
                    ScanInt(static_cast<bool*>(0));

                std::vector<MonsterBaseParameter>& table =
                    g_monsterBaseParameters[table_index];
                if (index == table.size()) {
                    table.push_back(row);
                } else {
                    if (index > table.size()) {
                        table.resize(index + 1);
                    }
                    table[index] = row;
                }
            }
        }
    }

    if (!loadRDARScriptFile(src, bonus_table)) {
        LogManager::logFormat(
            0,
            "../RDARScriptObject.cpp",
            "InitMonsterScript",
            0x7E6,
            "error : MonsterApcDifficultyBonus.tbl is missing!!");
        return 0;
    }

    bool success = false;
    for (int row = 0; row <= 12; ++row) {
        for (int column = 0; column <= 4; ++column) {
            DIFFICULTY_OF_DUNGEON_BONUS[row * 5 + column] =
                ScanFloat(&success);
            if (!success && column != 4 && row != 12) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptObject.cpp",
                    "InitMonsterScript",
                    0x7FC,
                    "error : MonsterApcDifficultyBonus.tbl is incorrect!! index %d i %d",
                    row,
                    column);
                break;
            }
        }
        if (!success) {
            break;
        }
    }

    for (int row = 0; row <= 12; ++row) {
        for (int column = 0; column <= 3; ++column) {
            DEATH_TOWER_BONUS[row * 4 + column] =
                ScanFloat(&success);
            if (!success && column != 3 && row != 12) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptObject.cpp",
                    "InitMonsterScript",
                    0x818,
                    "error : MonsterApcDifficultyBonus.tbl is incorrect!! index %d i %d",
                    row,
                    column);
                break;
            }
        }
        if (!success) {
            break;
        }
    }

    for (int row = 0; row <= 12; ++row) {
        for (int column = 0; column <= 3; ++column) {
            BLOOD_SYSTEM_BONUS[row * 5 + column] =
                ScanFloat(&success);
            if (!success && column != 3 && row != 12) {
                LogManager::logFormat(
                    0,
                    "../RDARScriptObject.cpp",
                    "InitMonsterScript",
                    0x833,
                    "error : MonsterApcDifficultyBonus.tbl is incorrect!! index %d i %d",
                    row,
                    column);
                break;
            }
        }
        if (!success) {
            break;
        }
    }

    for (int index = 0; index <= 4; ++index) {
        MONSTER_DIFFICULTY_BONUS[index].clear();
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[easy]" || tag == "[medium]" || tag == "[hard]" ||
            tag == "[ultimate]" || tag == "[hero]") {
            int difficulty = 0;
            if (tag == "[medium]") {
                difficulty = 1;
            } else if (tag == "[hard]") {
                difficulty = 2;
            } else if (tag == "[ultimate]") {
                difficulty = 3;
            } else if (tag == "[hero]") {
                difficulty = 4;
            }

            while (1) {
                ScanFloat(&success);
                if (!success) {
                    break;
                }

                std::vector<float> values;
                values.reserve(13);
                for (int index = 0; index <= 12; ++index) {
                    const float value = ScanFloat(&success);
                    if (!success) {
                        break;
                    }
                    values.push_back(value);
                }
                if (!success) {
                    break;
                }
                MONSTER_DIFFICULTY_BONUS[difficulty].push_back(values);
            }
        } else if (tag == "[dungeon party balance]") {
            DUNGEON_PARTY_BALANCE_TABLE.clear();
            while (1) {
                DungeonPartyBalance balance;
                int scanned = ScanInt(&success);
                balance.raw_d0 = static_cast<float>(scanned);
                if (!success) {
                    break;
                }
                scanned = ScanInt(&success);
                balance.raw_d4 = static_cast<float>(scanned);
                if (!success) {
                    break;
                }
                for (int row = 0; row <= 12; ++row) {
                    for (int column = 0; column <= 3; ++column) {
                        balance.balances[row][column] =
                            ScanFloat(&success);
                        if (!success) {
                            break;
                        }
                    }
                    if (!success) {
                        break;
                    }
                }
                if (!success) {
                    break;
                }
                DUNGEON_PARTY_BALANCE_TABLE.push_back(balance);
            }
        } else if (tag == "[mesmerization damage balance]") {
            std::string subtag;
            DungeonHoldBalance hold_balance;
            NORMAL_MONSTER_HOLD_BALANCE_TABLE.clear();
            NAMED_MONSTER_HOLD_BALANCE_TABLE.clear();
            BOSS_MONSTER_HOLD_BALANCE_TABLE.clear();
            NORMAL_MONSTER_HOLD_BALANCE_TABLE.reserve(200);
            NAMED_MONSTER_HOLD_BALANCE_TABLE.reserve(200);
            BOSS_MONSTER_HOLD_BALANCE_TABLE.reserve(200);

            while (ScanType(subtag, true) == 1 &&
                   subtag != "[/mesmerization damage balance]") {
                std::vector<DungeonHoldBalance>* table = 0;
                if (subtag == "[normal]") {
                    table = &NORMAL_MONSTER_HOLD_BALANCE_TABLE;
                } else if (subtag == "[boss]") {
                    table = &BOSS_MONSTER_HOLD_BALANCE_TABLE;
                } else if (subtag == "[named]") {
                    table = &NAMED_MONSTER_HOLD_BALANCE_TABLE;
                }

                if (table != 0) {
                    for (int index = 0; index <= 199; ++index) {
                        hold_balance.raw_00 = ScanInt(&success);
                        hold_balance.raw_04 = ScanInt(&success);
                        table->push_back(hold_balance);
                    }
                }
            }
        } else if (tag == "[limit slowdown]") {
            std::string subtag;
            stLimitSlowDown slowdown;
            while (ScanType(subtag, true) == 1 &&
                   subtag != "[/limit slowdown]") {
                std::vector<stLimitSlowDown>* table = 0;
                if (subtag == "[normal]") {
                    NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE.clear();
                    table = &NORMAL_MONSER_LIMIT_SLOWDOWN_TABLE;
                } else if (subtag == "[boss]") {
                    BOSS_MONSER_LIMIT_SLOWDOWN_TABLE.clear();
                    table = &BOSS_MONSER_LIMIT_SLOWDOWN_TABLE;
                } else if (subtag == "[named]") {
                    NAMED_MONSER_LIMIT_SLOWDOWN_TABLE.clear();
                    table = &NAMED_MONSER_LIMIT_SLOWDOWN_TABLE;
                }

                if (table != 0) {
                    for (int index = 0; index <= 199; ++index) {
                        ScanInt(&success);
                        slowdown.raw_00 = ScanInt(&success);
                        slowdown.raw_04 = ScanInt(&success);
                        table->push_back(slowdown);
                    }
                }
            }
        } else if (tag == "[adjust hp gauge]") {
            std::string subtag;
            DungeonHoldBalance hold_balance;
            while (ScanType(subtag, true) == 1 &&
                   subtag != "[/adjust hp gauge]") {
                if (subtag == "[normal]") {
                    NORMAL_MONSTER_GAUGE_ADJUST =
                        ScanFloat(&success);
                } else if (subtag == "[boss]") {
                    BOSS_MONSTER_GAUGE_ADJUST =
                        ScanFloat(&success);
                } else if (subtag == "[named]") {
                    NAMED_MONSTER_GAUGE_ADJUST =
                        ScanFloat(&success);
                }
            }
        }
    }

    return 1;
}

typedef char MonsterBaseParameter_Size_Must_Be_0x60[
    (sizeof(MonsterBaseParameter) == 0x60u) ? 1 : -1];
typedef char DungeonHoldBalance_Size_Must_Be_0x08[
    (sizeof(DungeonHoldBalance) == 0x08u) ? 1 : -1];
typedef char DungeonPartyBalance_Size_Must_Be_0xD8[
    (sizeof(DungeonPartyBalance) == 0xD8u) ? 1 : -1];
typedef char stLimitSlowDown_Size_Must_Be_0x08[
    (sizeof(stLimitSlowDown) == 0x08u) ? 1 : -1];
typedef char DIFFICULTY_OF_DUNGEON_BONUS_Count_Must_Be_65[
    (sizeof(DIFFICULTY_OF_DUNGEON_BONUS) == 13u * 5u * sizeof(float)) ? 1 : -1];
typedef char DEATH_TOWER_BONUS_Count_Must_Be_52[
    (sizeof(DEATH_TOWER_BONUS) == 13u * 4u * sizeof(float)) ? 1 : -1];
typedef char BLOOD_SYSTEM_BONUS_Count_Must_Be_65[
    (sizeof(BLOOD_SYSTEM_BONUS) == 13u * 5u * sizeof(float)) ? 1 : -1];
