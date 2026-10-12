#include "old_data/PVF/creature/active_status_sync_server.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/module_tag_table.h"
#include "infra/log/format/trace.h"

// IDA unit: 99_misc_global/SDCActiveStatusSyncCommon,
//           99_misc_global/SDCActiveStatusSyncServer,
//           99_misc_global/Global importActiveStatusTable.
// Restored in B because this is the DataManager/PVF active-status script load
// boundary used by CDataManager::init.

int ACTIVESTATUS_MAXLEVEL = 0;
int ACTIVESTATUS_DURATION[18] = {};
float ACTIVESTATUS_POWER1[36] = {};
float ACTIVESTATUS_POWER2[36] = {};
float ACTIVESTATUS_ETC_PARAMETERS[144] = {};
float ACTIVESTATUS_LEVEL_RATE_FOR_PROB = 0.0f;
float ACTIVESTATUS_LEVEL_RATE_FOR_POWER = 0.0f;
float ACTIVESTATUS_LEVEL_ABSOLUTE_RATE = 0.0f;
int LIGHTNING_TIME_PER_NUMBER = 0;
float ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE = 0.0f;
float ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE_DEATH_TOWER = 0.0f;

float g_physicalDamageRateConst_ = 0.0f;
float g_physicalDamageAddConst_ = 0.0f;
float g_physicalReduceRateConst_ = 0.0f;
float g_physicalReduceAddConst_ = 0.0f;
float g_physicalRandomBaseConst_ = 0.0f;
float g_physicalRandomRateConst_ = 0.0f;
float g_magicalDamageRateConst_ = 0.0f;
float g_magicalDamageAddConst_ = 0.0f;
float g_magicalReduceRateConst_ = 0.0f;
float g_magicalReduceAddConst_ = 0.0f;
float g_magicalRandomBaseConst_ = 0.0f;
float g_magicalRandomRateConst_ = 0.0f;
float g_commonElementBaseConst_ = 0.0f;
float g_commonElementRateConst_ = 0.0f;
float g_commonElementRateConstInPVP_ = 0.0f;
float g_commonElementRateConstInLadderPVP_ = 0.0f;
float g_hpIncreaseRateConst_ = 0.0f;
float g_hpRegenRateConst_ = 0.0f;
float g_mpIncreaseRateConst_ = 0.0f;
float g_mpRegenRateConst_ = 0.0f;
float g_activeStatusDamageRateConst_ = 0.0f;
float g_activeStatusDefenseRateConst_ = 0.0f;
float g_physicalDefenseAddConst_ = 0.0f;
float g_magicalDefenseAddConst_ = 0.0f;
float g_hpRegenRateAddRateConst_ = 0.0f;
float g_mpRegenRateAddRateConst_ = 0.0f;

std::vector<int> g_creature_damage_vector_;
std::map<int, float> g_creatureModuleDamageRateMap;
std::map<int, float> g_creatureIndexDamageRateMap;


extern const char* const g_module_type_start_tags[17] = {
    "[dungeon]", "[dungeon]", "[dungeon]", "[dungeon]",
    "[pvp]", "[pvp]", "[dungeon]", "[warroom]", "[dungeon]",
    "[assault]", "[death tower]", "[dungeon]", "[dungeon]",
    "[dungeon]", "[dungeon]", "[boss tower]", ""
};

extern const char* const g_module_type_end_tags[17] = {
    "[/dungeon]", "[/dungeon]", "[/dungeon]", "[/dungeon]",
    "[/pvp]", "[/pvp]", "[/dungeon]", "[/warroom]", "[/dungeon]",
    "[/assault]", "[/death tower]", "[/dungeon]", "[/dungeon]",
    "[/dungeon]", "[/dungeon]", "[/boss tower]", ""
};

extern char g_creatureScriptBaseDirectory[256];

// IDA_EA: 0x8096A2C
// IDA_NAME: _ZN20CreatureCommonScript5clearEv
// DEMANGLED: CreatureCommonScript::clear(void)
CreatureCommonScript* CreatureCommonScript::clear() {
    attack_constant = 0.0f;
    response_delay = 0;
    evolution_fade_in_delay = 0;
    evolution_fade_out_delay = 0;
    evolution_delay = 0;
    dying_delay = 0;
    damage.clear();
    mana.clear();
    second_per_fill_point_spend = 1;
    second_per_fill_point_charge = 1;
    max_fill_point = 100;
    fill_point_to_eat_feed = 0;
    ambient_in_room_list[0] = 0;
    ambient_in_room_list[1] = 0;
    ambient_in_main_game[0] = 0;
    ambient_in_main_game[1] = 0;
    ambient_in_pvp[0] = 0;
    ambient_in_pvp[1] = 0;
    ambient_in_room_list_time_term = 0;
    ambient_in_room_list_force_delay = 0;
    ambient_in_main_game_time_term = 0;
    ambient_in_main_game_force_delay = 0;
    ambient_in_pvp_time_term = 0;
    ambient_in_pvp_force_delay = 0;
    return this;
}

// IDA_EA: 0x8096AB4
// IDA_NAME: _ZN20CreatureCommonScriptC2Ev
// DEMANGLED: CreatureCommonScript::CreatureCommonScript(void)
CreatureCommonScript::CreatureCommonScript() : damage(), mana() {
    clear();
}

// IDA_EA: 0x8096B22
// IDA_NAME: _ZN20CreatureCommonScriptD2Ev
// DEMANGLED: CreatureCommonScript::~CreatureCommonScript()
CreatureCommonScript::~CreatureCommonScript() {}

// IDA_EA: 0x88C2233
// IDA_NAME: _Z20getModuleTypeFromTagRSs
// DEMANGLED: getModuleTypeFromTag(std::string &)
int getModuleTypeFromTag(std::string& tag) {
    for (int index = 0; index <= 16; ++index) {
        if (tag == g_module_type_start_tags[index]) {
            return index;
        }
    }
    return -1;
}

// IDA_EA: 0x8A38A08
// IDA_NAME: _Z21addCreatureModuleRate16ENUM_MODULE_TYPEf
// DEMANGLED: addCreatureModuleRate(ENUM_MODULE_TYPE,float)
float* addCreatureModuleRate(ENUM_MODULE_TYPE module_type, float rate) {
    const int module_value = static_cast<int>(module_type);
    std::map<int, float>::iterator it =
        g_creatureModuleDamageRateMap.find(module_value);
    if (it != g_creatureModuleDamageRateMap.end()) {
        rate *= it->second;
    }

    g_creatureModuleDamageRateMap[module_value] = rate;
    return &g_creatureModuleDamageRateMap[module_value];
}

// IDA_EA: 0x809510E
// IDA_NAME: _Z29prepareServerDelayCheckDamagev
// DEMANGLED: prepareServerDelayCheckDamage(void)
int prepareServerDelayCheckDamage();

// IDA_EA: 0x80966FA
// IDA_NAME: _Z33importEtcParameterScriptForServerv
// DEMANGLED: importEtcParameterScriptForServer(void)
int importEtcParameterScriptForServer();

// IDA_EA: 0x8A383F6
// IDA_NAME: _Z26importCreatureCommonScriptP20CreatureCommonScriptPKcPSt6vectorIiSaIiEE
// DEMANGLED: importCreatureCommonScript(CreatureCommonScript *,char const*,std::vector<int,std::allocator<int>> *)
int importCreatureCommonScript(CreatureCommonScript* script,
                               const char* filename,
                               std::vector<int>* damage_vector);

int prepareServerDelayCheckDamage() {
    CreatureCommonScript script;
    return importCreatureCommonScript(
               &script,
               "Creature/Common/creatureCommon.cre",
               &g_creature_damage_vector_) == 1 &&
           importEtcParameterScriptForServer() == 1;
}

int importEtcParameterScriptForServer() {
    if (loadRDARScriptFile("", "Etc/EtcParameter.etc") != 1) {
        return 0;
    }

    std::string tag;
    // IDA 0x80966FA deliberately ignores ScanType's status while skipping
    // arbitrary preceding sections, and stops only once this exact tag is
    // observed.  ScanType returns false for each scalar skipped between tags.
    while (tag != "[damage fomula constant]") {
        const int scan_result = ScanType(tag, false);
        if (tag == "[damage fomula constant]") {
            break;
        }

        const DNFLEX_TOKEN token = g_dnfLex.getLastToken();
        if (token == DNF_TOK_EOF || token == DNF_TOK_ERROR ||
            (scan_result != 1 && token != DNF_TOK_TYPE)) {
            LogManager::logFormat(
                0,
                "../RDARScriptObject.cpp",
                "importEtcParameterScriptForServer",
                0x80966FA,
                "EtcParameter Script Error: missing "
                "[damage fomula constant], line=%d token='%s'",
                g_dnfLex.getLastLineNo(),
                g_dnfLex.getLastTokenDataAsString().c_str());
            return 0;
        }
    }

    g_physicalDamageRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalDamageAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalReduceRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalReduceAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalRandomBaseConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalRandomRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalDamageRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalDamageAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalReduceRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalReduceAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalRandomBaseConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalRandomRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_commonElementBaseConst_ = ScanFloat(static_cast<bool*>(0));
    g_commonElementRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_commonElementRateConstInPVP_ = ScanFloat(static_cast<bool*>(0));
    g_commonElementRateConstInLadderPVP_ = ScanFloat(static_cast<bool*>(0));
    g_hpIncreaseRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_hpRegenRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_mpIncreaseRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_mpRegenRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_activeStatusDamageRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_activeStatusDefenseRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_physicalDefenseAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_magicalDefenseAddConst_ = ScanFloat(static_cast<bool*>(0));
    g_hpRegenRateAddRateConst_ = ScanFloat(static_cast<bool*>(0));
    g_mpRegenRateAddRateConst_ = ScanFloat(static_cast<bool*>(0));
    return 1;
}

int importCreatureCommonScript(CreatureCommonScript* script,
                               const char* filename,
                               std::vector<int>* damage_vector) {
    if (!loadRDARScriptFile(g_creatureScriptBaseDirectory, filename)) {
        return 0;
    }

    script->clear();
    damage_vector->clear();

    std::string tag;
    std::string module_tag;
    bool success = true;

    while (ScanType(tag, true) == 1) {
        if (tag == "[attack constant]") {
            script->attack_constant = ScanFloat(static_cast<bool*>(0));
        } else if (tag == "[response delay]") {
            script->response_delay = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[second per fill point spend]") {
            script->second_per_fill_point_spend =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[second per fill point charge]") {
            script->second_per_fill_point_charge =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[MAX fill point]") {
            script->max_fill_point = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[fill point to eat feed]") {
            script->fill_point_to_eat_feed =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[evolution fade in delay]") {
            script->evolution_fade_in_delay =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[evolution fade out delay]") {
            script->evolution_fade_out_delay =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[evolution delay]") {
            script->evolution_delay = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[dying delay]") {
            script->dying_delay = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ambient in room list]") {
            script->ambient_in_room_list[0] =
                ScanInt(static_cast<bool*>(0));
            script->ambient_in_room_list[1] =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ambient in room list time term]") {
            script->ambient_in_room_list_time_term = ScanInt(&success);
            if (!success || script->ambient_in_room_list_time_term < 0) {
                return 0;
            }
        } else if (tag == "[ambient in room list force delay]") {
            script->ambient_in_room_list_force_delay = ScanInt(&success);
            if (!success || script->ambient_in_room_list_force_delay < 0) {
                return 0;
            }
        } else if (tag == "[ambient in main game]") {
            script->ambient_in_main_game[0] =
                ScanInt(static_cast<bool*>(0));
            script->ambient_in_main_game[1] =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ambient in main game time term]") {
            script->ambient_in_main_game_time_term = ScanInt(&success);
            if (!success || script->ambient_in_main_game_time_term < 0) {
                return 0;
            }
        } else if (tag == "[ambient in main game force delay]") {
            script->ambient_in_main_game_force_delay = ScanInt(&success);
            if (!success || script->ambient_in_main_game_force_delay < 0) {
                return 0;
            }
        } else if (tag == "[ambient in pvp]") {
            script->ambient_in_pvp[0] =
                ScanInt(static_cast<bool*>(0));
            script->ambient_in_pvp[1] =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[ambient in pvp time term]") {
            script->ambient_in_pvp_time_term = ScanInt(&success);
            if (!success || script->ambient_in_pvp_time_term < 0) {
                return 0;
            }
        } else if (tag == "[ambient in pvp force delay]") {
            script->ambient_in_pvp_force_delay = ScanInt(&success);
            if (!success || script->ambient_in_pvp_force_delay < 0) {
                return 0;
            }
        } else if (tag == "[damage]") {
            script->damage.clear();
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->damage.push_back(value);
                damage_vector->push_back(value);
            }
        } else if (tag == "[mana]") {
            script->mana.clear();
            while (1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->mana.push_back(value);
            }
        } else if (tag == "[module damage percent]") {
            g_creatureModuleDamageRateMap.clear();
            while (ScanStr(&module_tag) == 1) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int module_type = getModuleTypeFromTag(module_tag);
                addCreatureModuleRate(static_cast<ENUM_MODULE_TYPE>(module_type),
                                      static_cast<float>(value) / 100.0f);
            }
        } else if (tag == "[assault creature damage penalty]") {
            g_creatureIndexDamageRateMap.clear();
            while (1) {
                const int index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                g_creatureIndexDamageRateMap[index] =
                    static_cast<float>(value) / 100.0f;
            }
        } else {
            LogManager::logFormat(
                0,
                "../RDARScriptObject.cpp",
                "importCreatureCommonScript",
                0x2701,
                "script loading failed. | file <%s> tag <%s>",
                filename,
                tag.c_str());
            return 0;
        }
    }

    return 1;
}

// IDA_EA: 0x8906F88
// IDA_NAME: _Z23importActiveStatusTablePKc
// DEMANGLED: importActiveStatusTable(char const*)
int importActiveStatusTable(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                                      "../RDARScriptEtcParameter.cpp",
                                      "importActiveStatusTable",
                                      0x162B,
                                      "ERROR : %s",
                                      filename);
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[active status max level]") {
            ACTIVESTATUS_MAXLEVEL = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[active status duration]") {
            for (int status = 0; status <= 17; ++status) {
                ACTIVESTATUS_DURATION[status] =
                    ScanInt(static_cast<bool*>(0));
            }
        } else if (tag == "[active status power 1]") {
            for (int status = 0; status <= 17; ++status) {
                ACTIVESTATUS_POWER1[2 * status] =
                    ScanFloat(static_cast<bool*>(0));
                ACTIVESTATUS_POWER1[2 * status + 1] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[active status power 2]") {
            for (int status = 0; status <= 17; ++status) {
                ACTIVESTATUS_POWER2[2 * status] =
                    ScanFloat(static_cast<bool*>(0));
                ACTIVESTATUS_POWER2[2 * status + 1] =
                    ScanFloat(static_cast<bool*>(0));
            }
        } else if (tag == "[active status etc parameters]") {
            for (int status = 0; status <= 17; ++status) {
                for (int index = 0; index <= 7; ++index) {
                    ACTIVESTATUS_ETC_PARAMETERS[8 * status + index] =
                        ScanFloat(static_cast<bool*>(0));
                }
            }
        } else if (tag == "[active status level rate for prob]") {
            ACTIVESTATUS_LEVEL_RATE_FOR_PROB =
                static_cast<float>(ScanInt(static_cast<bool*>(0))) *
                0.0099999998f;
        } else if (tag == "[active status level rate for power]") {
            ACTIVESTATUS_LEVEL_RATE_FOR_POWER =
                static_cast<float>(ScanInt(static_cast<bool*>(0))) *
                0.0099999998f;
        } else if (tag == "[active status level absolute rate]") {
            ACTIVESTATUS_LEVEL_ABSOLUTE_RATE =
                static_cast<float>(ScanInt(static_cast<bool*>(0))) *
                0.0099999998f;
        } else if (tag == "[lightning time per number]") {
            LIGHTNING_TIME_PER_NUMBER = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[active status user to user damage rate]") {
            ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE =
                ScanFloat(static_cast<bool*>(0));
        } else if (tag ==
                   "[dead tower active status user to user damage rate]") {
            ACTIVESTATUS_USER_TO_USER_DAMAGE_RATE_DEATH_TOWER =
                ScanFloat(static_cast<bool*>(0));
        }
    }

    return 1;
}

// IDA_EA: 0x8234870
// IDA_NAME: _ZN25SDCActiveStatusSyncCommonC2Ev
// DEMANGLED: SDCActiveStatusSyncCommon::SDCActiveStatusSyncCommon(void)
SDCActiveStatusSyncCommon::SDCActiveStatusSyncCommon() {}

// IDA_EA: 0x8234876
// IDA_NAME: _ZN25SDCActiveStatusSyncCommonD2Ev
// DEMANGLED: SDCActiveStatusSyncCommon::~SDCActiveStatusSyncCommon()
SDCActiveStatusSyncCommon::~SDCActiveStatusSyncCommon() {}

// IDA_EA: 0x808E13C
// IDA_NAME: _ZN25SDCActiveStatusSyncServer22loadActiveStatusScriptEv
// DEMANGLED: SDCActiveStatusSyncServer::loadActiveStatusScript(void)
int SDCActiveStatusSyncServer::loadActiveStatusScript() {
    return importActiveStatusTable("Etc/ActiveStatusTable_ex.etc");
}

// IDA_EA: 0x823487C
// IDA_NAME: _ZN25SDCActiveStatusSyncServerC2Ev
// DEMANGLED: SDCActiveStatusSyncServer::SDCActiveStatusSyncServer(void)
SDCActiveStatusSyncServer::SDCActiveStatusSyncServer()
    : SDCActiveStatusSyncCommon() {}

// IDA_EA: 0x8234890
// IDA_NAME: _ZN25SDCActiveStatusSyncServerD2Ev
// DEMANGLED: SDCActiveStatusSyncServer::~SDCActiveStatusSyncServer()
SDCActiveStatusSyncServer::~SDCActiveStatusSyncServer() {}
