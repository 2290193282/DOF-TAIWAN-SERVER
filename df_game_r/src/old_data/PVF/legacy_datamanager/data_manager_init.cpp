#include "old_data/PVF/data/data_manager.h"

#include "common/config.h"
#include "old_data/PVF/event/bingo_script.h"
#include "old_data/PVF/event/blue_marble_script.h"
#include "old_data/PVF/character/avatar_color_variation.h"
#include "old_data/PVF/equipment/datamanager/broadcast_item_script_data.h"
#include "old_data/PVF/character/datamanager/character_create_creature_gift_script.h"
#include "old_data/PVF/creature/datamanager/creature_script.h"
#include "old_data/PVF/data/datamanager/create_chrac_script.h"
#include "old_data/PVF/data/datamanager/daily_schedule_script.h"
#include "old_data/PVF/data/datamanager/db_log_item.h"
#include "old_data/PVF/etc/delete_invalid_item_data.h"
#include "old_data/PVF/etc/cargo_transport_stone_script.h"
#include "old_data/PVF/etc/compound_creature_script.h"
#include "old_data/PVF/etc/datamanager_optional_features_0828.h"
#include "old_data/PVF/etc/equipment_growth.h"
#include "old_data/PVF/etc/instant_reinforce.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_data/PVF/etc/emotion_type_tags.h"
#include "old_data/PVF/event/event_avatar_list_script.h"
#include "old_data/PVF/event/condition_event_script.h"
#include "old_data/PVF/event/event_etc_script.h"
#include "old_data/PVF/event/in_game_event_info_import.h"
#include "old_data/PVF/event/event_upgrade_jar_script.h"
#include "old_data/PVF/event/level_reward_scripts.h"
#include "old_data/PVF/etc/etc_parameter_script.h"
#include "old_data/PVF/etc/avatar_roulette_script.h"
#include "old_data/PVF/data/datamanager/etc_parameter_script.h"
#include "old_data/PVF/event/event_reward_script.h"
#include "old_data/PVF/etc/equipment_grade_sample.h"
#include "old_data/Login/account_cargo/account_cargo_script.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/PVF/data/datamanager/active_status_sync_server.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/data/datamanager/avatar_roulette_server.h"
#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "old_data/PVF/stackable/stackable_script.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"
#include "old_data/PVF/pvp_mission/pvp_reference_tables.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_data/PVF/data/datamanager/image_communication_info.h"
#include "old_data/PVF/data/datamanager/in_game_advertisement_script.h"
#include "old_data/PVF/equipment/growth_equipment_script.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/equipment/item_amplifier_abi.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_data/PVF/etc/independent_drop_parameter_script.h"
#include "old_data/PVF/localization/invalid_name_script.h"
#include "old_data/PVF/localization/collect_box_script.h"
#include "old_data/PVF/localization/slang_name_script.h"
#include "old_data/PVF/etc/item_making_skill_script.h"
#include "old_data/PVF/etc/mercenary_system_script.h"
#include "old_data/PVF/etc/pcbang_dimension_script.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_data/PVF/equipment/datamanager/limit_item_usage_info.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/npc/need_material_data_manager.h"
#include "old_data/PVF/event/datamanager/npc_relation_event_script.h"
#include "old_data/PVF/etc/power_parameter_script.h"
#include "old_data/PVF/pvp_mission/pvp_channel_list.h"
#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"
#include "old_data/PVF/n_quest/quest_parameter_script.h"
#include "old_data/PVF/itemshop/datamanager/quest_shop_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/common/script_initializers.h"
#include "old_data/PVF/n_quest/training_quest_script.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/event/datamanager/seria_room_deco_animation_event_script.h"
#include "old_data/PVF/event/seria_room_deco_event_script.h"
#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/dungeon/instance/zombie_itembox_runtime.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_data/PVF/event/seria_blessing_script.h"
#include "old_data/PVF/event/unlimit_challenge_script.h"
#include "old_data/PVF/itemshop/secret_shop_script.h"
#include "old_data/PVF/aicharacter/st_ai_info_script.h"
#include "old_data/PVF/map/datamanager/stage_map_script.h"
#include "old_data/PVF/etc/striker_script.h"
#include "old_data/PVF/data/datamanager/sync_script.h"
#include "old_data/PVF/etc/title_book_info.h"
#include "old_data/PVF/etc/vending_machine_script.h"
#include "old_data/PVF/character/datamanager/use_character_job_fatigue_script.h"
#include "old_data/PVF/character/expert_job_script_list.h"
#include "old_data/PVF/town/datamanager/village_attacked_script.h"
#include "old_data/PVF/map/datamanager/world_drop_script.h"
#include "old_data/PVF/map/datamanager/world_map_script.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_game/player/core/character_manage_script.h"
#include "old_game/player/core/character_script_growth_data.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_game/activity/schedule/event_character_parameter_script.h"
#include "old_game/world/region/world_value_types.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/dungeon/instance/compound_extream_dun_item_runtime.h"
#include "old_game/economy/mail/mail_trade_value_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/appearance/avatar/compound_avatar.h"
#include "old_data/PVF/etc/action_point_etc_parameter.h"
#include "old_data/db/core/mysql.h"
#include "core/config/env/environment.h"
#include "old_game/activity/reward/advance_altar_config_parameter.h"
#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "common/manager/chatting_emoticon.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"
#include "common/manager/cdata_manager_tail_public_abi.h"
#include "old_game/progression/career/expert_job_script_abi.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/db_mgr.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/progression/career/disjoint_abi.h"
#include "old_game/economy/equipment/equipment_ani_script.h"
#include "old_game/economy/equipment/equipment_part_set_public_abi.h"
#include "common/manager/find_price_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_game/social/party/hellparty_characlink_abi.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "common/manager/invalid_char_data.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/monster_script_public_abi.h"
#include "old_game/economy/equipment/old_equipment_stat_info.h"
#include "old_game/pvp/battlefield/pvp_map_public_abi.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_data/channel/legacy_runtime/channel_proxy.h"
#include "old_data/PVF/genuine_upgrade_table.h"
#include "old_data/PVF/npc/npc_script.h"
#include "old_data/PVF/equipment/random_option_script.h"
#include "old_data/PVF/equipment/reseal_table_script.h"
#include "common/manager/reseal_table.h"
#include "common/manager/slang_filter.h"
#include "common/manager/slang_name_data.h"
#include "common/manager/server_utility.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "old_game/pvp/battlefield/war_area_counter.h"
#include "infra/log/format/trace.h"

#include <new>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <map>
#include <set>
#include <string>
#include <vector>

struct ConditionLevelChkDungeon;
struct stConditionEventInfo;
struct InGameEventInfoList;

namespace QuickParty {
struct CQuickPartyRewardManager;
}  // namespace QuickParty

char g_szCharacterScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_szPassiveObjectScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_szEquipmentScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_szMapScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_szDungeonScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_questScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_szSkillScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_eventCharacterScriptBaseDirectory[256]
    __attribute__((used, externally_visible));
char g_creatureScriptBaseDirectory[256];
STScriptFileList g_stCharacterScriptFileList;
STScriptFileList g_stMapScriptFileList;
STScriptFileList g_stDungeonScriptFileList;
STScriptFileList
    g_stSkillScriptFileList[oldsrv::config::kCharacterJobCount];
STScriptFileList g_eventCharacterScriptFileList;
STScriptFileList g_creatureScriptFileList;
int STCharacterScript::naExpTable[200];
int STCharacterScript::pvpPrices_[200];
int STEquipmentScript::priceTable_[3000];

int initCreatureScript(const char* src,
                       const char* list_file,
                       const char* exp_table)
    __asm__("_Z18initCreatureScriptPKcS0_S0_");

// IDA_EA: 0x895AF18
// IDA_NAME: _ZN29EventCharacterParameterScript25load_event_character_listEv
// DEMANGLED: EventCharacterParameterScript::load_event_character_list(void)
int EventCharacterParameterScript::load_event_character_list() {
    for (STScriptFileList::iterator it =
             g_eventCharacterScriptFileList.begin();
         it != g_eventCharacterScriptFileList.end();
         ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            break;
        }
        if (importEventCharacterParameterScript(filename) != 1) {
            LogManager::logFormat(
                1,
                "../RDARScriptEventCharacterParameter.cpp",
                "load_event_character_list",
                0x38,
                "EventCharacterParameterScript File Script Error: %s\n",
                filename);
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x895B006
// IDA_NAME: _ZN29EventCharacterParameterScript35importEventCharacterParameterScriptEPKc
// DEMANGLED: EventCharacterParameterScript::importEventCharacterParameterScript(char const*)
int EventCharacterParameterScript::importEventCharacterParameterScript(
    const char* filename) {
    if (!loadRDARScriptFile(g_eventCharacterScriptBaseDirectory, filename)) {
        return 0;
    }

    std::string tag;
    int level = 0;
    int job = 0;
    bool success = false;
    STEventCharacterInfo_level level_info;

    if (ScanType(tag, true) != 1) {
        return 0;
    }

    if (tag == "[level]") {
        level = ScanInt(&success);
        if (!success) {
            LogManager::logFormat(
                0,
                "../RDARScriptEventCharacterParameter.cpp",
                "importEventCharacterParameterScript",
                0x5F,
                "file name(%s) job name [%d]",
                filename,
                job);
            return 0;
        }
    }

    while (1) {
        STEventCharacterInfo info;
        bool keep_loading_jobs = false;

        while (ScanType(tag, true) == 1) {
            if (tag == "[job const]") {
                job = ScanInt(&success);
                if (!success) {
                    return 0;
                }
            } else if (tag == "[grow type]") {
                info.grow_type =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEventCharacterParameter.cpp",
                        "importEventCharacterParameterScript",
                        0x79,
                        "file name(%s) job name [%d]",
                        filename,
                        job);
                    return 0;
                }
            } else if (tag == "[awakening type]") {
                info.awakening_type =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEventCharacterParameter.cpp",
                        "importEventCharacterParameterScript",
                        0x82,
                        "file name(%s) job name [%d]",
                        filename,
                        job);
                    return 0;
                }
            } else if (tag == "[skill]") {
                while (1) {
                    std::pair<int, int> skill;
                    skill.first = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    skill.second = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    info.skills.push_back(skill);
                }
            } else if (tag == "[item]") {
                while (1) {
                    STEventCharacterItem item;
                    item.item_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    item.count = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    item.upgrade = ScanInt(&success);
                    if (item.upgrade < 0 || item.upgrade >= 0x20) {
                        LogManager::logFormat(
                            0,
                            "../RDARScriptEventCharacterParameter.cpp",
                            "importEventCharacterParameterScript",
                            0xA4,
                            "file name(%s) upgrade script error [%d] upgrade < 0 || upgrade > 31",
                            filename,
                            item.upgrade);
                        return 0;
                    }
                    if (!success) {
                        break;
                    }
                    info.items.push_back(item);
                }
            } else if (tag == "[/job const]") {
                if (!level_info.infos_by_job.insert(
                         std::make_pair(job, info)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEventCharacterParameter.cpp",
                        "importEventCharacterParameterScript",
                        0xB3,
                        "file name(%s) job name [%d]",
                        filename,
                        job);
                    return 0;
                }
                keep_loading_jobs = true;
                break;
            } else if (tag == "[/level]") {
                if (!this->infos_by_level.insert(
                         std::make_pair(level, level_info)).second) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEventCharacterParameter.cpp",
                        "importEventCharacterParameterScript",
                        0xBD,
                        "file name(%s) same level exist[%d]",
                        filename,
                        level);
                    return 0;
                }
                return 1;
            }
        }

        if (!keep_loading_jobs) {
            break;
        }
    }

    return 0;
}

// IDA_EA: 0x8A35D37
// IDA_NAME: _Z18initCreatureScriptPKcS0_S0_
// DEMANGLED: initCreatureScript(char const*,char const*,char const*)
int initCreatureScript(const char* src,
                       const char* list_file,
                       const char* exp_table) {
    strncpy(g_creatureScriptBaseDirectory, src, 0x100u);
    if (g_creatureScriptBaseDirectory
                [strlen(g_creatureScriptBaseDirectory) - 1] != '\\' &&
        g_creatureScriptBaseDirectory
                [strlen(g_creatureScriptBaseDirectory) - 1] != '/') {
        const size_t length = strlen(g_creatureScriptBaseDirectory);
        memcpy(g_creatureScriptBaseDirectory + length, "\\", 2u);
    }

    if (!loadRDARScriptFile(g_creatureScriptBaseDirectory, exp_table)) {
        return 0;
    }

    for (int index = 0; index <= 98; ++index) {
        CreatureScript::ExpTable_[index] = ScanInt(static_cast<bool*>(0));
    }

    return loadRDARScriptListFile(&g_creatureScriptFileList,
                                  g_creatureScriptBaseDirectory,
                                  list_file) == 1;
}

// IDA_EA: 0x8977D9C
// IDA_NAME: _Z23importPairPvPStatScriptR17FairPvPStatScriptPKc
// DEMANGLED: importPairPvPStatScript(FairPvPStatScript&,char const*)
int importPairPvPStatScript(FairPvPStatScript& script,
                            const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    script.clear();

    std::string tag;
    bool success = false;
    CharacterKind kind;
    CharacterStatus* current_status = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[job index]") {
            kind.charac_job =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[grow type index]") {
            kind.first_grow_type =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }

            CharacterStatus status;
            memset(&status, 0, sizeof(status));
            std::pair<std::map<CharacterKind, CharacterStatus>::iterator,
                      bool> inserted =
                script.character_stats.insert(std::make_pair(kind, status));
            if (!inserted.second) {
                return 0;
            }
            current_status = &inserted.first->second;
        } else if (tag == "[HP MAX]") {
            if (!current_status) {
                return 0;
            }
            current_status->max_hp =
                static_cast<uint32_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[MP MAX]") {
            if (!current_status) {
                return 0;
            }
            current_status->max_mp =
                static_cast<uint32_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[physical attack]") {
            if (!current_status) {
                return 0;
            }
            current_status->physical_attack =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[physical defense]") {
            if (!current_status) {
                return 0;
            }
            current_status->physical_defense =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[magical attack]") {
            if (!current_status) {
                return 0;
            }
            current_status->magical_attack =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[magical defense]") {
            if (!current_status) {
                return 0;
            }
            current_status->magical_defense =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[inventory limit]") {
            if (!current_status) {
                return 0;
            }
            current_status->inventory_limit =
                static_cast<int32_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[MP regen speed]") {
            if (!current_status) {
                return 0;
            }
            current_status->mp_regen_speed =
                static_cast<int16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[move speed]") {
            if (!current_status) {
                return 0;
            }
            current_status->move_speed =
                static_cast<uint32_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[attack speed]") {
            current_status->attack_speed =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[cast speed]") {
            if (!current_status) {
                return 0;
            }
            current_status->cast_speed =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[hit recovery]") {
            if (!current_status) {
                return 0;
            }
            current_status->hit_recovery =
                static_cast<int16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[jump power]") {
            if (!current_status) {
                return 0;
            }
            current_status->jump_power =
                static_cast<int16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[jump speed]") {
            ScanInt(&success);
        } else if (tag == "[weight]") {
            if (!current_status) {
                return 0;
            }
            current_status->weight =
                static_cast<int32_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[dark resistance]") {
            if (!current_status) {
                return 0;
            }
            current_status->element_resist[2] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[light resistance]") {
            if (!current_status) {
                return 0;
            }
            current_status->element_resist[3] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[fire resistance]") {
            if (!current_status) {
                return 0;
            }
            current_status->element_resist[0] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[water resistance]") {
            if (!current_status) {
                return 0;
            }
            current_status->element_resist[1] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        }
    }

    return success ? 1 : 0;
}

char g_PvpMissionScriptBaseDirectory[256];
STScriptFileList g_PvpMissionScriptFileList;

class CEquipItem;
// IDA_EA: 0x819A336
// IDA_NAME: _ZN20AvatarRouletteServer16PickedAvatarInfoC2Ev
// DEMANGLED: AvatarRouletteServer::PickedAvatarInfo::PickedAvatarInfo(void)
AvatarRouletteServer::PickedAvatarInfo::PickedAvatarInfo()
    : item_no(0), value_04(0), avatar_option(0), value_09(0), value_0a(0),
      picked_group(0) {}

// IDA_EA: 0x817F354
// IDA_NAME: _ZN20AvatarRouletteServerC2Ev
// DEMANGLED: AvatarRouletteServer::AvatarRouletteServer(void)
AvatarRouletteServer::AvatarRouletteServer()
    : character_skills_(), avatar_roulette_(0), total_avatar_rate_(0) {}

// IDA_EA: 0x817F372
// IDA_NAME: _ZN20AvatarRouletteServerD2Ev
// DEMANGLED: AvatarRouletteServer::~AvatarRouletteServer()
AvatarRouletteServer::~AvatarRouletteServer() {}

// IDA_EA: 0x817F386
// IDA_NAME: _ZN20AvatarRouletteServer18InitAddCharacSkillEv
// DEMANGLED: AvatarRouletteServer::InitAddCharacSkill(void)
int AvatarRouletteServer::InitAddCharacSkill() {
    if (G_CEnvironment()->validate_script()) {
        for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
            const AvatarRoulette::AvatarRateGroup& avatar_groups =
                avatar_roulette_->avatar_lists_by_job[job];
            for (unsigned int group_index = 0;
                 group_index < avatar_groups.size();
                 ++group_index) {
                const AvatarRoulette::AvatarRateList& avatar_items =
                    avatar_groups[group_index];
                for (int item_part = 0; item_part <= 8; ++item_part) {
                    if (item_part != 3 ||
                        (job != oldsrv::config::kCharacterJobSwordman &&
                         job !=
                             oldsrv::config::kCharacterJobDemonicSwordman)) {
                        continue;
                    }

                    const char* filename = g_stEquipmentScriptFileList.GetFileName(
                        static_cast<int>(avatar_items[item_part].first));
                    if (!filename) {
                        continue;
                    }

                    STEquipmentScript equipment_script;
                    if (ImportEquipmentScript(&equipment_script,
                                              filename,
                                              false,
                                              0,
                                              -1) != 1) {
                        LogManager::logFormat(
                            1,
                            "localjapan/Arad_AvatarRoulette.cpp",
                            "bool AvatarRouletteServer::InitAddCharacSkill()",
                            0x39,
                            "InitAddCharacSkill() - ImportEquipmentScript() fail file - %s",
                            filename);
                        return 0;
                    }

                    CharacterSkillMap character_skills;
                    const std::vector<STAvatarAbilityVariation>& variations =
                        equipment_script.parameter_1c0.avatar_select_abilities_264;
                    for (unsigned int variation_index = 0;
                         variation_index < variations.size();
                         ++variation_index) {
                        if (variations[variation_index].ability_index < 0) {
                            continue;
                        }
                        LevelUpSkill skill =
                            variations[variation_index].level_up_skill;
                        CharacterSkillMap::iterator found =
                            character_skills.find(skill.skill_type);
                        if (found == character_skills.end()) {
                            character_skills[skill.skill_type] =
                                std::vector<LevelUpSkill>();
                        }
                        skill.skill_id = static_cast<int>(variation_index);
                        character_skills[skill.skill_type].push_back(skill);
                    }
                    character_skills_[avatar_items[item_part].first] =
                        character_skills;
                }
            }
        }
    }
    return 1;
}

// IDA_EA: 0x817F706
// IDA_NAME: _ZN20AvatarRouletteServer4InitEP14AvatarRoulette
// DEMANGLED: AvatarRouletteServer::Init(AvatarRoulette *)
int AvatarRouletteServer::Init(AvatarRoulette* roulette) {
    avatar_roulette_ = new AvatarRoulette();
    avatar_roulette_->swap(roulette);
    total_avatar_rate_ = static_cast<int>(avatar_roulette_->upper_avatar_rate +
                                          avatar_roulette_->normal_avatar_rate);

    if (G_CEnvironment()->validate_script()) {
        for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
            const AvatarRoulette::AvatarRateGroup& avatar_groups =
                avatar_roulette_->avatar_lists_by_job[job];
            for (unsigned int line_number = 0;
                 line_number < avatar_groups.size();
                 ++line_number) {
                const AvatarRoulette::AvatarRateList& avatar_items =
                    avatar_groups[line_number];
                for (int item_part = 0; item_part <= 8; ++item_part) {
                    const unsigned long item_no = avatar_items[item_part].first;
                    CItem* item = G_CDataManager()->find_item(
                        static_cast<int>(item_no));
                    if (!item) {
                        LogManager::logFormat(
                            1,
                            "localjapan/Arad_AvatarRoulette.cpp",
                            "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                            0x80,
                            "AvatarRouletteServer not exist item_no(%d)",
                            item_no);
                    }

                    const CEquipItem* equipment =
                        reinterpret_cast<const CEquipItem*>(item);
                    if (equipment->IsAvatarItem() != 1) {
                        LogManager::logFormat(
                            1,
                            "localjapan/Arad_AvatarRoulette.cpp",
                            "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                            0x86,
                            "AvatarRouletteServer no avatar item_no(%d)",
                            item_no);
                    }
                    if (item->check_job_type(static_cast<char>(job)) != 1) {
                        LogManager::logFormat(
                            1,
                            "localjapan/Arad_AvatarRoulette.cpp",
                            "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                            0x8d,
                            "AvatarRouletteServer item_no(%d) check_job_type(%d)==false",
                            item_no,
                            job);
                    }

                    const int item_type = equipment->GetItemType();
                    if (item_type != item_part) {
                        LogManager::logFormat(
                            1,
                            "localjapan/Arad_AvatarRoulette.cpp",
                            "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                            0x95,
                            "AvatarRouletteServer item_no(%d) GetItemType(%d)!=%d",
                            item_no,
                            item_type,
                            item_part);
                    }

                    if (item_part != 8) {
                        const int grade = item->get_grade();
                        if (line_number <= 1 && grade != 2) {
                            LogManager::logFormat(
                                1,
                                "localjapan/Arad_AvatarRoulette.cpp",
                                "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                                0xa3,
                                "AvatarRouletteServer item_no(%d) get_grade(%d)!=%d, line_num(%d)",
                                item_no,
                                grade,
                                2,
                                line_number);
                        } else if (line_number > 1 && grade != 1) {
                            LogManager::logFormat(
                                1,
                                "localjapan/Arad_AvatarRoulette.cpp",
                                "bool AvatarRouletteServer::Init(AvatarRoulette*)",
                                0xa9,
                                "AvatarRouletteServer item_no(%d) get_grade(%d)!=%d, line_num(%d)",
                                item_no,
                                grade,
                                1,
                                line_number);
                        }
                    }
                }
            }
        }
    }
    return 1;
}

// IDA_EA: 0x817FB0A
// IDA_NAME: _ZN20AvatarRouletteServer15isAvatarPotteryEm
// DEMANGLED: AvatarRouletteServer::isAvatarPottery(unsigned long)
bool AvatarRouletteServer::isAvatarPottery(unsigned long item_no) {
    return avatar_roulette_->avatar_coin_item_no == item_no;
}

// IDA_EA: 0x817FB28
// IDA_NAME: _ZN20AvatarRouletteServer6PickupERNS_16PickedAvatarInfoEii
// DEMANGLED: AvatarRouletteServer::Pickup(AvatarRouletteServer::PickedAvatarInfo &,int,int)
int AvatarRouletteServer::Pickup(PickedAvatarInfo& picked,
                                 int character_job,
                                 int item_part) {
    const int random_rate = get_rand_int(total_avatar_rate_);
    if (static_cast<int>(avatar_roulette_->upper_avatar_rate) <= random_rate) {
        picked.picked_group = static_cast<uint8_t>(
            get_rand_int(static_cast<int>(
                             avatar_roulette_->avatar_lists_by_job[character_job].size()) -
                         2) +
            2);
    } else {
        picked.picked_group = static_cast<uint8_t>(get_rand_int(2));
    }

    if (character_job < 0 ||
        character_job >= oldsrv::config::kCharacterJobCount) {
        return 0;
    }
    const AvatarRoulette::AvatarRateGroup& avatar_groups =
        avatar_roulette_->avatar_lists_by_job[character_job];
    if (avatar_groups.size() <= picked.picked_group) {
        return 0;
    }
    const AvatarRoulette::AvatarRateList& avatar_items =
        avatar_groups[picked.picked_group];
    if (avatar_items.size() <= static_cast<unsigned int>(item_part)) {
        return 0;
    }

    picked.item_no = static_cast<uint32_t>(avatar_items[item_part].first);
    picked.value_0a = static_cast<uint8_t>(avatar_items[item_part].second);
    CItem* item = G_CDataManager()->find_item(static_cast<int>(picked.item_no));
    if (!item) {
        return 0;
    }
    const CEquipItem* equipment = reinterpret_cast<const CEquipItem*>(item);
    if (equipment->IsAvatarItem() != 1) {
        return 0;
    }

    const int avatar_option_count = equipment->getAvatarOptionSize();
    if (avatar_option_count <= 0) {
        return 0;
    }
    const int avatar_option_key = equipment->getAvatarOptionKeyByOrdinal(
        get_rand_int(avatar_option_count));
    if (avatar_option_key < 0 || avatar_option_key > 0xff) {
        return 0;
    }
    picked.avatar_option = static_cast<uint8_t>(avatar_option_key);
    if (item_part == 3 &&
        (character_job == oldsrv::config::kCharacterJobSwordman ||
         character_job ==
             oldsrv::config::kCharacterJobDemonicSwordman)) {
        AvatarCharacterSkills::const_iterator outer =
            character_skills_.find(picked.item_no);
        if (outer == character_skills_.end()) {
            return 0;
        }
        CharacterSkillMap::const_iterator inner =
            outer->second.find(character_job);
        if (inner == outer->second.end()) {
            return 0;
        }
        const std::vector<LevelUpSkill>& skills = inner->second;
        picked.avatar_option = static_cast<uint8_t>(
            skills[get_rand_int(static_cast<int>(skills.size()))].skill_id);
    }
    picked.value_09 = 0;
    return 1;
}

int importPairPvPStatScript(FairPvPStatScript& script, const char* filename)
    __asm__("_Z23importPairPvPStatScriptR17FairPvPStatScriptPKc");

char g_pvpSkillTreeScriptBaseDirectory[256];
STScriptFileList g_pvpSkillTreeScriptFileList;



// The account-cargo request implementation consumes this existing typed
// data-manager boundary.  Keep the verified +0xA7E0 layout access here,
// alongside the PVF importer that initializes the same embedded script.
AccountCargoScript* AccountCargoAt(CDataManager* data_manager) {
    return data_manager ? data_manager->get_account_cargo_script() : 0;
}

// IDA_EA: 0x8365ECE
// IDA_NAME: _ZNK12CDataManager18GetItemMakingSkillEv
// DEMANGLED: CDataManager::GetItemMakingSkill(void)const
stItemMakingSkill* CDataManager::GetItemMakingSkill() const {
    return const_cast<stItemMakingSkill*>(&item_making_skill_);
}

// IDA_EA: 0x835FE02
// IDA_NAME: _ZN12CDataManager14load_map_filesERSt6vectorI11PVPMAP_DATASaIS1_EE
// DEMANGLED: CDataManager::load_map_files(std::vector<PVPMAP_DATA,std::allocator<PVPMAP_DATA> >&)
int CDataManager::load_map_files(std::vector<PVPMAP_DATA>& pvp_map_data) {
    STMapScript script;
    map_runtime::BeginMapExtensionLoad0828();

    for (STScriptFileList::iterator it = g_stMapScriptFileList.begin();
         it != g_stMapScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            map_runtime::CommitMapExtensionLoad0828();
            return 1;
        }

        if (it->first == 50000) {
            script.Clear();
        } else if (ImportMapScript(&script, filename) != 1) {
            printf("Error in script('%s')\n", filename);
            map_runtime::AbortMapExtensionLoad0828();
            return 0;
        }

        CMap* map = static_cast<CMap*>(::operator new(sizeof(CMap)));
        new (map) CMap();

        STMapScript copied_script(script);
        map->set_map(copied_script);

        std::list<int>& dungeon_indices = script.dungeon_indices;
        std::vector<std::pair<int, int> >& entrance_pairs =
            script.greed_cells;
        const int start_flag = script.dungeon_start_area.width;

        if (!dungeon_indices.empty()) {
            for (std::list<int>::iterator dungeon_it =
                     dungeon_indices.begin();
                 dungeon_it != dungeon_indices.end(); ++dungeon_it) {
                const int dungeon_index = *dungeon_it;
                CDungeon* dungeon =
                    dungeon_index_list_->find_dungeon(dungeon_index);
                if (!dungeon) {
                    cMyTrace trace(
                        "bool CDataManager::load_map_files(PVPMAP_VECTOR&)",
                        9547,
                        5);
                    trace("[Load Map] Not exist dungeon. [Index:%d] filename : [%s]",
                          dungeon_index,
                          filename);
                    map_runtime::AbortMapExtensionLoad0828();
                    return 0;
                }

                if (entrance_pairs.empty()) {
                    dungeon->InsertMap(15, 15, map);
                    map_list_->insert_map(15, 15, map, pvp_map_data);
                    if (start_flag > 0) {
                        dungeon->insertStart(15, map);
                    }
                }

                for (std::vector<std::pair<int, int> >::iterator entrance_it =
                         entrance_pairs.begin();
                     entrance_it != entrance_pairs.end(); ++entrance_it) {
                    dungeon->InsertMap(entrance_it->first,
                                       entrance_it->second,
                                       map);
                    map_list_->insert_map(
                        entrance_it->first,
                        entrance_it->second,
                        map,
                        pvp_map_data);
                    if (start_flag > 0) {
                        dungeon->insertStart(entrance_it->first, map);
                    }
                }
            }
        } else {
            map_list_->insert_map(15, 15, map, pvp_map_data);
        }
    }

    map_runtime::CommitMapExtensionLoad0828();
    return 1;
}

// IDA_EA: 0x83580AA
// IDA_NAME: _ZN12CDataManager20initRandomOptionItemEv
// DEMANGLED: CDataManager::initRandomOptionItem(void)
int CDataManager::initRandomOptionItem() {
    typedef std::map<int, BaseItemValue> BaseItemValueMap;
    typedef std::map<int, BaseItemValueMap> BaseItemTable;
    typedef std::map<unsigned long, int> ItemRefMap;

    BaseItemTable& base_item_table = random_option_script_.base_items;
    ItemRefMap& item_ref_map = random_option_script_.item_ref;

    for (BaseItemTable::iterator group_it = base_item_table.begin();
         group_it != base_item_table.end();
         ++group_it) {
        int ordinal = 0;
        BaseItemValueMap& base_items = group_it->second;
        for (BaseItemValueMap::iterator item_it = base_items.begin();
             item_it != base_items.end();
             ++item_it) {
            const std::pair<ItemRefMap::iterator, bool> inserted =
                item_ref_map.insert(
                    std::make_pair(item_it->second.item_index, ordinal));
            if (!inserted.second) {
                return 0;
            }
            ++ordinal;
        }
    }

    return 1;
}

// IDA_EA: 0x835FDA2
// IDA_NAME: _ZNK12CDataManager10find_skillEii
// DEMANGLED: CDataManager::find_skill(int,int)const
CSkill* CDataManager::find_skill(int job_type, int skill_index) const {
    return skill_list_->find_skill(job_type, skill_index);
}

// IDA_EA: 0x835F1D6
// IDA_NAME: _ZN12CDataManager21initFairPVPSkillPointEv
// DEMANGLED: CDataManager::initFairPVPSkillPoint(void)
int CDataManager::initFairPVPSkillPoint() {
    unsigned char result = 1;
    PvPSkillTreeParameterScript* script =
        &pvp_skill_tree_parameter_script_;

    if (!script->skill_tree.empty()) {
        for (PvPSkillTreeMap::iterator tree_it =
                 script->skill_tree.begin();
             tree_it != script->skill_tree.end();
             ++tree_it) {
            std::map<int, PvPSkillTreeValue>& levels = tree_it->second;
            for (std::map<int, PvPSkillTreeValue>::iterator level_it =
                     levels.begin();
                 level_it != levels.end();
                 ++level_it) {
                if (script->max_pvp_skill_tree_level < level_it->first) {
                    script->max_pvp_skill_tree_level = level_it->first;
                }
            }
        }
    }

    if (!script->skill_points.empty()) {
        for (PvPSkillPointMap::iterator point_it =
                 script->skill_points.begin();
             point_it != script->skill_points.end();
             ++point_it) {
            if (script->max_pvp_skill_point_grade < point_it->first) {
                script->max_pvp_skill_point_grade = point_it->first;
            }
        }
    }

    if (!script->fair_skill_points.empty()) {
        for (FairPvpSkillPointMap::iterator fair_it =
                 script->fair_skill_points.begin();
             fair_it != script->fair_skill_points.end();
             ++fair_it) {
            std::map<int, int> skill_point_sum;
            int total_skill_point = 0;

            for (std::map<int, int>::iterator skill_it =
                     fair_it->second.skill_point.begin();
                 skill_it != fair_it->second.skill_point.end();
                 ++skill_it) {
                const int job_type =
                    static_cast<unsigned char>(fair_it->first.charac_job);
                CSkill* skill = G_CDataManager()->find_skill(
                    job_type, skill_it->first);
                if (skill) {
                    const int need_sp =
                        skill->get_need_sp_for_level(0, skill_it->second);
                    total_skill_point += need_sp;
                    skill_point_sum.insert(
                        std::make_pair(skill_it->first, need_sp));
                }
            }

            int diff_skill_point = 0;
            for (std::map<int, int>::iterator static_it =
                     fair_it->second.static_basic_skill_point.begin();
                 static_it !=
                 fair_it->second.static_basic_skill_point.end();
                 ++static_it) {
                const int job_type =
                    static_cast<unsigned char>(fair_it->first.charac_job);
                CSkill* skill = G_CDataManager()->find_skill(
                    job_type, static_it->first);
                if (skill) {
                    const int need_sp =
                        skill->get_need_sp_for_level(0, static_it->second);
                    total_skill_point -= need_sp;

                    std::map<int, int>::iterator sum_it =
                        skill_point_sum.find(static_it->first);
                    if (sum_it != skill_point_sum.end()) {
                        sum_it->second -= need_sp;
                        if (sum_it->second < 0) {
                            cMyTrace trace(
                                "bool CDataManager::initFairPVPSkillPoint()",
                                9081,
                                0);
                            trace("skill point (-) error");
                            result = 0;
                        }
                    }

                    fair_it->second.skill_per_point.push_back(
                        std::make_pair(static_it->first, static_it->second));

                    int current_sum = 0;
                    for (std::map<int, int>::iterator sum_calc_it =
                             skill_point_sum.begin();
                         sum_calc_it != skill_point_sum.end();
                         ++sum_calc_it) {
                        current_sum += sum_calc_it->second;
                    }

                    if (current_sum + diff_skill_point !=
                        total_skill_point) {
                        diff_skill_point += total_skill_point - current_sum;
                        cMyTrace trace(
                            "bool CDataManager::initFairPVPSkillPoint()",
                            9104,
                            0);
                        trace("skill point diff job(%d), skill(%d)",
                              job_type,
                              static_it->first);
                        result = 0;
                    }
                }
            }

            int final_sum = 0;
            for (std::map<int, int>::iterator sum_it =
                     skill_point_sum.begin();
                 sum_it != skill_point_sum.end();
                 ++sum_it) {
                final_sum += sum_it->second;
            }
            if (final_sum != total_skill_point) {
                cMyTrace trace(
                    "bool CDataManager::initFairPVPSkillPoint()",
                    9119,
                    0);
                trace("skill point diff");
                result = 0;
            }

            fair_it->second.total_skill_point = total_skill_point;
        }
    }

    return result;
}

// IDA_EA: 0x8360400
// IDA_NAME: _ZN12CDataManager13set_level_expEv
// DEMANGLED: CDataManager::set_level_exp(void)
int CDataManager::set_level_exp() {
    for (int index = 0; index <= 199; ++index) {
        level_exp_[index] = STCharacterScript::naExpTable[index];
    }
    return 1;
}

// IDA_EA: 0x83605A8
// IDA_NAME: _ZN12CDataManager14set_mob_rewardEv
// DEMANGLED: CDataManager::set_mob_reward(void)
int CDataManager::set_mob_reward() {
    MySQL* db_handle = GlobalData::s_db_mgr
                           ? GlobalData::s_db_mgr->GetDBHandle(
                                 static_cast<ENUM_DB_HANDLE_IDX>(2),
                                 static_cast<ENUM_SERVER_GROUP>(0))
                           : 0;
    if (!db_handle) {
        return 0;
    }
    db_handle->set_query("seLect level, exp from monster_reward_ref");
    if (db_handle->exec(true) != true) {
        return 0;
    }
    const int rows = db_handle->get_n_rows();
    if (rows <= 0) {
        return 0;
    }

    unsigned int parsed_rewards[200] = {};
    int highest_configured_level = 0;
    for (int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) {
            return 0;
        }
        short level = 0;
        if (db_handle->get_short(0, level) != true) {
            return 0;
        }
        unsigned int reward = 0;
        if (db_handle->get_uint(1, reward) != true ||
            level <= 0 || level >= 200) {
            return 0;
        }
        parsed_rewards[level] = reward;
        if (level > highest_configured_level) {
            highest_configured_level = level;
        }
    }

    // The legacy DB shipped with levels 1..99, while the 0828 Requiem PVF
    // deliberately spawns level-100 monsters.  Clamp only the unconfigured
    // upper tail to the highest configured level; preserve every explicit
    // record (including zero rewards) and every interior gap.  Both kill and
    // clear EXP read this table, so they must share the same fallback.
    for (int level = highest_configured_level + 1; level < 200; ++level) {
        parsed_rewards[level] = parsed_rewards[highest_configured_level];
    }
    ::memcpy(mob_rewards_, parsed_rewards, sizeof(mob_rewards_));
    ::memcpy(original_mob_rewards_, mob_rewards_, sizeof(mob_rewards_));
    LogManager::logFormat(
        1, __FILE__, "CDataManager::set_mob_reward", __LINE__,
        "[MOB_REWARD][TABLE_READY] max_configured_level=%d tail_reward=%u level100=%u",
        highest_configured_level, parsed_rewards[highest_configured_level],
        parsed_rewards[100]);
    return 1;
}

// IDA_EA: 0x8360AA2
// IDA_NAME: _ZN12CDataManager13set_pvp_gradeEv
// DEMANGLED: CDataManager::set_pvp_grade(void)
int CDataManager::set_pvp_grade() {
    int previous_exp = 0;
    int last_grade = 0;
    stPvpRefTable_t table;
    if (importPvPReferenceTable("Etc/pvp_ref.etc", table)) {
        return 0;
    }

    RefPvpGrade staged_grade;
    for (unsigned int index = 1;
         table.max_grade_plus_one > static_cast<int>(index);
         ++index) {
        ++last_grade;
        RefPvpGrade::RefPvpGradeData range;
        range.curr_rank_point = previous_exp;
        range.next_rank_point =
            static_cast<int>(table.grade_exp_table[index].exp);
        previous_exp = static_cast<int>(table.grade_exp_table[index].exp);
        if (!staged_grade.SetPvpGrade(
                table.grade_exp_table[index].grade, range)) {
            return 0;
        }
    }
    staged_grade.set(table);
    if (!staged_grade.SetLastGrade(last_grade)) {
        return 0;
    }
    GlobalData::s_ref_pvp_grade->swap(staged_grade);
    return 1;
}

// IDA_EA: 0x8360BDE
// IDA_NAME: _ZN12CDataManager13set_reward_spEv
// DEMANGLED: CDataManager::set_reward_sp(void)
int CDataManager::set_reward_sp() {
    stSpPerLevelTable table;
    if (importSpPerLevelReferenceTable("Etc/spTable.etc", table)) {
        return 0;
    }

    // The importer supplies index 0 plus every configured character level.
    // Reject a malformed/truncated PVF table before indexing it; the original
    // body otherwise performed an unchecked vector access after its heap OOB.
    if (table.values.size() < SpAtLevelUpTable::kElementCount) {
        return 0;
    }

    for (unsigned int index = 0;
         index < SpAtLevelUpTable::kElementCount;
         ++index) {
        sp_at_level_up_.values_[index] = 0;
    }
    for (unsigned int level = oldsrv::config::kCharacterLevelMinimum;
         level <= static_cast<unsigned int>(
                      oldsrv::config::kCharacterLevelMaximum);
         ++level) {
        sp_at_level_up_.values_[level] = static_cast<int>(table.values[level]);
    }
    return 1;
}

// IDA_EA: 0x8358332
// IDA_NAME: _ZN12CDataManager4initEv
// DEMANGLED: CDataManager::init(void)
int CDataManager::init() {
    CEnvironment* environment = G_CEnvironment();
    const std::vector<std::string>& priority_dirs =
        environment->priority_script_dirs();
    for (std::size_t index = 0; index < priority_dirs.size(); ++index) {
        SetPriorityBaseDirectory(priority_dirs[index].c_str());
    }

    charac_name_filter_pass_ = 0;

    CDataManager* data_manager = G_CDataManager();
    const char* script_packs = data_manager->GetScriptPacks();
    const char use_script_pack =
        environment->scriptpacks() ? static_cast<char>(1) : static_cast<char>(0);
    const char* script_dir = environment->script_dir().c_str();
    InitRDARScript(script_dir, use_script_pack != 0, script_packs, 0);

    if (InitRDARScriptStringManager("n_String.lst", 0, 0) == 1) {
        cMyTrace trace("bool CDataManager::init()", 6517, 9, true, true);
        trace("\t- InitRDARScriptStringManager Success");
        ::InitMapEquipmentAniScript();

        // 0627 server: importPremiumServiceData @0x8912E47,
        // importPartyMatchingInfo @0x890F311, importQuestMovieList
        // @0x89174F5, ImportWarRoomWPTable @0x8902543.  The 0828 client owns
        // the same four grammars in sub_5D6FB0/sub_1501F20/sub_5CD100/
        // sub_1511590.  The same generation also carries all 29 ordered
        // values from EtcParameter [damage fomula constant].  Publish them
        // together so no runtime query can observe a partially upgraded ETC
        // set.
        if (import_datamanager_etc_0828() != 1) {
            trace("\t- Failed to load < DATAMANAGER ETC 0828 >");
            return 0;
        }
        trace("\t- Load < DATAMANAGER ETC 0828 > Success");

        // These four official 0828 resource families have no proven 0627
        // gameplay consumer.  Load their complete grammar atomically into a
        // read-only sidecar, without enabling any policy transaction.
        if (ImportDataManagerOptionalFeatures() != 1) {
            trace("\t- Failed to load < DATAMANAGER OPTIONAL FEATURES >");
            return 0;
        }
        trace("\t- Load < DATAMANAGER OPTIONAL FEATURES > Success");

        sync_script::CSyncScript* sync =
            sync_script::GetInstanceSyncScript();
        if (sync->LoadItemInfoMasterScript("Etc/ItemTypeMaster.etc") == 1) {
            sync = sync_script::GetInstanceSyncScript();
            if (sync->truncate_item_info_master_table() != true) {
                cMyTrace truncate_trace("bool CDataManager::init()",
                                        6534,
                                        9,
                                        true,
                                        true);
                truncate_trace(
                    "\t- Failed to < TRUNCATE ITEM INFO MASTER TABLE >");
            }

            sync = sync_script::GetInstanceSyncScript();
            if (sync->insert_item_info_master_to_db() != 1) {
                cMyTrace insert_trace("bool CDataManager::init()",
                                      6537,
                                      9,
                                      true,
                                      true);
                insert_trace(
                    "\t- Failed to < INSERT ITEM INFO MASTER TO DB >");
            }
        } else {
            cMyTrace item_trace("bool CDataManager::init()",
                                6530,
                                9,
                                true,
                                true);
            item_trace("\t- Failed to load < ITEM INFO MASTER SCRIPT >");
        }

        if (::importOldEquipmentStatInfo("") == 1) {
            sync = sync_script::GetInstanceSyncScript();
            if (sync->truncate_old_equip_info_table() != true) {
                cMyTrace truncate_old_trace("bool CDataManager::init()",
                                            6547,
                                            9,
                                            true,
                                            true);
                truncate_old_trace(
                    "\t- Failed to < TRUNCATE OLD EQUIP INFO TABLE >");
            }

            sync = sync_script::GetInstanceSyncScript();
            if (sync->insert_old_equip_info_to_db() != 1) {
                cMyTrace insert_old_trace("bool CDataManager::init()",
                                          6550,
                                          9,
                                          true,
                                          true);
                insert_old_trace(
                    "\t- Failed to < INSERT OLD EQUIP INFO TO DB >");
            }
        } else {
            cMyTrace old_equip_trace("bool CDataManager::init()",
                                     6543,
                                     9,
                                     true,
                                     true);
            old_equip_trace(
                "\t- Failed to import < OldEquipmentStatInfo.dat >");
        }

        if (importServerParameterScript(
                &server_parameter_script_,
                "Etc/ServerParameter.etc") == 1) {
            if (environment->get_accept_handshake_flag_424() == 2) {
                server_parameter_script_.
                    apply_accept_handshake_reward_weights();
            }

            if (importInGameAdvertisementEtcScript(
                    &in_game_advertisement_script_,
                    "Etc/IngameAdvertisementParameter.etc") == 1) {
                if (importSeriaRoomDecoEvent(
                        &seria_room_deco_event_script_,
                        "Event/SeriaRoomDecoEvent.evt") == 1) {
                    if (importGrowthEquipmentScript(
                            &growth_equipment_script_,
                            "Event/GrowthWeapon.evt") == 1) {
                        if (importCharacterCreateGiftScript(
                                &create_character_script_,
                                "Event/CreateCharacterGift.evt") == 1) {
                            if (importUseJobFatigueGiftScript(
                                    &use_character_job_fatigue_,
                                    "Event/JobFatigue.evt") == 1) {
                                if (importCharacterCreateCreatureGiftScript(
                                        &character_create_creature_gift_script_,
                                        "Event/CreateCharacterGiftCreature.evt") == 1) {
                                    if (importSeriaRoomAnimationDecoEvent(
                                            &seria_room_deco_animation_event_script_,
                                            "Event/SeriaRoomDecoAnimation.evt") == 1) {
                                        if (event_reward_.importRewardTable(
                                                "Event/EventReward/",
                                                "reward.evt")) {
                                            cMyTrace event_reward_trace(
                                                "bool CDataManager::init()",
                                                6787,
                                                9,
                                                true,
                                                true);
                                            event_reward_trace(
                                                "\t- JumpingCharacterEventScript Fail!");
                                            return 0;
                                        }

                                        const unsigned int version_0 =
                                            server_parameter_script_.
                                                get_antibot_version_digit(0);
                                        const unsigned int version_1 =
                                            server_parameter_script_.
                                                get_antibot_version_digit(1);
                                        const unsigned int version_2_div =
                                            server_parameter_script_.
                                                get_antibot_version_digit(2) /
                                            100u;
                                        const unsigned int version_2_mod =
                                            server_parameter_script_.
                                                get_antibot_version_digit(2) -
                                            100u * version_2_div;
                                        const unsigned int version_3 =
                                            server_parameter_script_.
                                                get_antibot_version_digit(3);
                                        const int antibot_version =
                                            static_cast<int>(
                                                10000000u * version_0 +
                                                100000u * version_1 +
                                                100u * version_2_mod +
                                                10000u * version_2_div +
                                                version_3);
                                        server_parameter_script_.
                                            set_antibot_server_version(
                                                static_cast<uint32_t>(
                                                    antibot_version));

                                        cMyTrace version_trace(
                                            "bool CDataManager::init()",
                                            6823,
                                            9,
                                            true,
                                            true);
                                        version_trace(
                                            "\t- Server version for antibot : %d",
                                            antibot_version);

                                        if (importImageCommunicationInfo(
                                                "Etc/ImageCommunication.etc",
                                                &image_communication_data_) == 1) {
                                            if (server_parameter_script_.
                                                    CheckTraningFirstGive() ==
                                                1) {
                                                if (initExpertJobScript(
                                                        const_cast<char*>(
                                                            "Character/"),
                                                        const_cast<char*>(
                                                            "expertjob.lst")) ==
                                                    1) {
                                                    CExpertJobList* expert_jobs =
                                                        new CExpertJobList();
                                                    expert_job_list_ =
                                                        expert_jobs;

                                                    if (expert_job_list_->init() ==
                                                        1) {
                                                        if (importTowerInfo(
                                                                "Etc/",
                                                                "TowerList.lst",
                                                                tower_scripts_)) {
                                                            puts("importTowerInfo Faile");
                                                            return 0;
                                                        }

                                                        const int death_result =
                                                            importDeathTowerScript(
                                                                "Etc/DeathTower.etc",
                                                                death_tower_);
                                                        if (death_result) {
                                                            cMyTrace death_trace(
                                                                "bool CDataManager::init()",
                                                                6868,
                                                                9,
                                                                true,
                                                                true);
                                                            death_trace(
                                                                "\t- importDeathTowerScript Fail(%d)",
                                                                death_result);
                                                            return 0;
                                                        }

                                                        if (importRankSystemScript(
                                                                "Etc/RankSystemInfo.etc",
                                                                &rank_system_info_) !=
                                                            1) {
                                                            cMyTrace rank_trace(
                                                                "bool CDataManager::init()",
                                                                6874,
                                                                9,
                                                                true,
                                                                true);
                                                            rank_trace(
                                                                "\t- importRankSystemScript Fail");
                                                            return 0;
                                                        }

                                                        const int blood_result =
                                                            importBloodDungeonScript(
                                                                "Etc/BloodClearReward.etc",
                                                                blood_dungeon_);
                                                        if (blood_result) {
                                                            cMyTrace blood_trace(
                                                                "bool CDataManager::init()",
                                                                6881,
                                                                9,
                                                                true,
                                                                true);
                                                            blood_trace(
                                                                "\t- importBloodDungeonScript Fail(%d)",
                                                                blood_result);
                                                            return 0;
                                                        }

                                                        const int hellparty_result =
                                                            importHellPartyScript(
                                                                "Etc/HellParty.etc",
                                                                *Get_instance_stHellParty_t());
                                                        if (hellparty_result) {
                                                            cMyTrace hellparty_trace(
                                                                "bool CDataManager::init()",
                                                                6888,
                                                                9,
                                                                true,
                                                                true);
                                                            hellparty_trace(
                                                                "\t- importHellPartyScript Fail(%d)",
                                                                hellparty_result);
                                                            return 0;
                                                        }

                                                        const int charac_link_result =
                                                            importCharacLinkSystemScript(
                                                                "Etc/LinkSystem/CharacLinkSystem.etc",
                                                                charac_link_system_);
                                                        if (charac_link_result) {
                                                            cMyTrace charac_link_trace(
                                                                "bool CDataManager::init()",
                                                                6899,
                                                                9,
                                                                true,
                                                                true);
                                                            charac_link_trace(
                                                                "\t- importCharacLinkSystemScript Fail(%d)",
                                                                charac_link_result);
                                                            return 0;
                                                        }

                                                        StrikerScript* striker =
                                                            StrikerScript::getInsance();
                                                        striker_script_ = striker;
                                                        if (striker->importScript(
                                                                "Etc/LinkSystem/striker.etc") ==
                                                            1) {
                                                            CharacManageScript* charac_manage =
                                                                CharacManageScript::getInsance();
                                                            character_manage_script_ =
                                                                charac_manage;
                                                            if (charac_manage->importScript(
                                                                    "Etc/LinkSystem/CharacterManage.etc") ==
                                                                1) {
                                                                BlueMarbleScriptManager* blue_marble =
                                                                    BlueMarbleScriptManager::getInsance();
                                                                blue_marble_script_ =
                                                                    blue_marble;
                                                                if (blue_marble->initTileList(
                                                                        "Event/BlueMarble/",
                                                                        "BlueMarbleTile.lst") ==
                                                                    1) {
                                                                    if (blue_marble->initMapList(
                                                                            "Event/BlueMarble/",
                                                                            "BlueMarbleMap.lst") ==
                                                                        1) {
                                                                        if (blue_marble->initBlueMarbleInfo(
                                                                                "Event/BlueMarble/",
                                                                                "BlueMarble.blu") ==
                                                                            1) {
                                                                            const int quest_shop_result =
                                                                                importQuestShopScript(
                                                                                    "Etc/questshop.etc",
                                                                                    quest_shop_script_);
                                                                            if (quest_shop_result) {
                                                                                cMyTrace quest_shop_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    6991,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                quest_shop_trace(
                                                                                    "\t- importQuestShopScript Fail(%d)",
                                                                                    quest_shop_result);
                                                                                return 0;
                                                                            }

                                                                            if (initIndependentDropScript(
                                                                                    const_cast<char*>("Etc/"),
                                                                                    const_cast<char*>("IndependentDrop.lst")) !=
                                                                                1) {
                                                                                cMyTrace independent_drop_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    6998,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                independent_drop_trace(
                                                                                    "\t- Load IndependentDrop.lst fail");
                                                                                return 0;
                                                                            }

                                                                            IndependentDropParameterScript* independent_drop =
                                                                                get_independent_drop_parameter_script();
                                                                            if (independent_drop->load_independent_drop_list() !=
                                                                                1) {
                                                                                cMyTrace independent_drop_list_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7004,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                independent_drop_list_trace(
                                                                                    "\t- Load IndependentDrop.lst fail");
                                                                                return 0;
                                                                            }

                                                                            const int independent_drop_result =
                                                                                independent_drop->ImportIndependentDrop(
                                                                                    "Etc/Independent_Drop.etc");
                                                                            if (independent_drop_result) {
                                                                                cMyTrace independent_drop_import_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7011,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                independent_drop_import_trace(
                                                                                    "\t- importIndependent_DropScript Fail(%d)",
                                                                                    independent_drop_result);
                                                                                return 0;
                                                                            }

                                                                            const int seria_blessing_result =
                                                                                ImportSeriaBlessingScript(
                                                                                    "Event/SeriaBlessing.evt",
                                                                                    seria_blessing_script_);
                                                                            if (seria_blessing_result) {
                                                                                cMyTrace seria_blessing_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7043,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                seria_blessing_trace(
                                                                                    "\t- ImportSeriaBlessingScript Fail(%d)",
                                                                                    seria_blessing_result);
                                                                                return 0;
                                                                            }

                                                                            const int level_up_reward_result =
                                                                                ImportLevelUpRewardItemScript(
                                                                                    "Event/LevelUpRewardItem.evt",
                                                                                    level_up_reward_item_script_);
                                                                            if (level_up_reward_result) {
                                                                                cMyTrace level_up_reward_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7050,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                level_up_reward_trace(
                                                                                    "\t- ImportLevelUpRewardItemScript Fail(%d)",
                                                                                    level_up_reward_result);
                                                                                return 0;
                                                                            }

                                                                            const int growth_capsule_result =
                                                                                ImportGrowthCapsuleScript(
                                                                                    "Event/GrowthCapsule.evt",
                                                                                    growth_capsule_script_);
                                                                            if (growth_capsule_result) {
                                                                                printf(
                                                                                    "ImportGrowthCapsuleScript Fail(%d)\n",
                                                                                    growth_capsule_result);
                                                                                return 0;
                                                                            }

                                                                            const int new_account_result =
                                                                                ImportNewAccountLeventUpToJobScript(
                                                                                    "Event/NewAccountLevelUpToJob.evt",
                                                                                    new_account_level_up_to_job_script_);
                                                                            if (new_account_result) {
                                                                                cMyTrace new_account_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7079,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                new_account_trace(
                                                                                    "\t- NewAccountLevelUpToJob StackableReward Fail(%d)",
                                                                                    new_account_result);
                                                                                return 0;
                                                                            }

                                                                            const int return_user_result =
                                                                                ImportReturnUserRewardScript(
                                                                                    "Event/ReturnUserReward.evt",
                                                                                    return_user_reward_script_);
                                                                            if (return_user_result) {
                                                                                cMyTrace return_user_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7088,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                return_user_trace(
                                                                                    "\t- ReturnUserReward Fail(%d)",
                                                                                    return_user_result);
                                                                                return 0;
                                                                            }

                                                                            const int bingo_result =
                                                                                ImportBingoScript(
                                                                                    "Event/BingoEvent.evt",
                                                                                    bingo_script_);
                                                                            if (bingo_result) {
                                                                                cMyTrace bingo_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7099,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                bingo_trace(
                                                                                    "\t- ImportBingoScript Fail(%d)",
                                                                                    bingo_result);
                                                                                return 0;
                                                                            }

                                                                            const int broadcast_result =
                                                                                ImportBroadCastItemScript(
                                                                                    "Etc/BroadCastItem.etc",
                                                                                    broadcast_item_script_);
                                                                            if (broadcast_result) {
                                                                                cMyTrace broadcast_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7119,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                broadcast_trace(
                                                                                    "\t- ImportBroadCastItemScript Fail(%d)",
                                                                                    broadcast_result);
                                                                                return 0;
                                                                            }

                                                                            if (ImportSecurityReward(
                                                                                    &security_reward_) !=
                                                                                1) {
                                                                                cMyTrace security_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7127,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                security_trace(
                                                                                    "\t- ImportSecurityReward Fail");
                                                                                return 0;
                                                                            }

                                                                            const int item_making_result =
                                                                                ImportItemMakingSkill(
                                                                                    "Etc/ItemMakingSkill.etc",
                                                                                    item_making_skill_);
                                                                            if (item_making_result) {
                                                                                cMyTrace item_making_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7136,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                item_making_trace(
                                                                                    "\t- importItemMakingSkillScript Fail(%d)",
                                                                                    item_making_result);
                                                                                return 0;
                                                                            }

                                                                            const int ultimate_dungeon_result =
                                                                                importUltimateDungeonList(
                                                                                    "Etc/UltimateDungeonList.etc",
                                                                                    ultimate_dungeon_list_);
                                                                            if (ultimate_dungeon_result != 1) {
                                                                                cMyTrace ultimate_dungeon_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7144,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                ultimate_dungeon_trace(
                                                                                    "\t- import UltimateDungeonList Script Fail(%d)",
                                                                                    ultimate_dungeon_result);
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            if (sync->truncate_mapping_equip_info_table() !=
                                                                                true) {
                                                                                cMyTrace truncate_mapping_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7174,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                truncate_mapping_trace(
                                                                                    "\t- Fail truncate_mapping_equip_info_table");
                                                                                return 0;
                                                                            }

                                                                            if (importExperienceItemMapInfo(
                                                                                    item_dictionary_item_to_flag_,
                                                                                    false) !=
                                                                                1) {
                                                                                cMyTrace mapping_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7180,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                mapping_trace(
                                                                                    "\t- Equipment Script mapping table load fail");
                                                                                return 0;
                                                                            }

                                                                            if (importExperienceItemMapInfo(
                                                                                    item_dictionary_flag_to_item_,
                                                                                    true) !=
                                                                                1) {
                                                                                cMyTrace mapping_reverse_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7186,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                mapping_reverse_trace(
                                                                                    "\t- Equipment Script mapping table load fail");
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            if (sync->insert_mapping_equip_info_to_db(
                                                                                    item_dictionary_flag_to_item_) !=
                                                                                1) {
                                                                                cMyTrace insert_mapping_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7192,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                insert_mapping_trace(
                                                                                    "\t- Fail insert_mapping_equip_info_table");
                                                                                return 0;
                                                                            }

                                                                            const int village_attacked_result =
                                                                                importVillageAttackedScript(
                                                                                    "Etc/VillageAttacked.etc",
                                                                                    village_attacked_);
                                                                            if (village_attacked_result) {
                                                                                cMyTrace village_attacked_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7200,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                village_attacked_trace(
                                                                                    "\t- importVillageAttackedScript Fail(%d)",
                                                                                    village_attacked_result);
                                                                                return 0;
                                                                            }

                                                                            int action_point_result =
                                                                                APSystem::CActionPointEtcParameter::
                                                                                    importActionPointSystemEtcScript(
                                                                                        "Etc/Chn_ActionPointSystem.etc",
                                                                                        *GetActionPointEtcParameter(false));
                                                                            if (action_point_result) {
                                                                                cMyTrace action_point_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7226,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                action_point_trace(
                                                                                    "\t- importActionPointSystemEtcScript Chn_ActionPointSystem.etc Fail(%d)",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            action_point_result =
                                                                                APSystem::CActionPointEtcParameter::
                                                                                    importActionPointSystemEtcScript(
                                                                                        "Etc/Chn_ActionPointSystem_GM.etc",
                                                                                        *GetActionPointEtcParameter(true));
                                                                            if (action_point_result) {
                                                                                cMyTrace action_point_gm_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7233,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                action_point_gm_trace(
                                                                                    "\t- importActionPointSystemEtcScript Chn_ActionPointSystemGM.etc Fail(%d)",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            sync->truncate_charac_action_point_desc();
                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            sync->insert_charac_action_point_desc();

                                                                            std::vector<_DailySchedule>
                                                                                daily_schedules;
                                                                            action_point_result =
                                                                                importDailyScheduleEtcScript(
                                                                                    "Etc/Chn_ServerCheckSchedule.etc",
                                                                                    daily_schedules);
                                                                            if (action_point_result) {
                                                                                printf(
                                                                                    "importDDayEventEtcScript Fail(%d)\n",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            if (!daily_schedules.empty()) {
                                                                                GlobalData::s_DailyScheduleManager
                                                                                    ->AddDailSchedule(
                                                                                        daily_schedules);
                                                                            }

                                                                            action_point_result =
                                                                                loadAdvanceAltarTimeLineParameter(
                                                                                    advance_altar_stage_timeline_);
                                                                            if (action_point_result) {
                                                                                printf(
                                                                                    "loadAdvanceAltarTimeLineParameter Fail(%d)\n",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            action_point_result =
                                                                                advance_altar_shop_parameter_.
                                                                                    importAdvanceAltarShopParameter(
                                                                                        "Etc/AdvanceAltar/AdvanceAltarShop.etc");
                                                                            if (action_point_result) {
                                                                                printf(
                                                                                    "importAdvanceAltarShopParameter Fail(%d)\n",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            action_point_result =
                                                                                advance_altar_reward_parameter_.
                                                                                    importAchievementRewardParameter(
                                                                                        "Etc/AdvanceAltar/AdvanceAltarAchievementReward.etc");
                                                                            if (action_point_result) {
                                                                                printf(
                                                                                    "importAchievementRewardParameter Fail(%d)\n",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            action_point_result =
                                                                                advance_altar_config_parameter_.
                                                                                    importConfigParameter(
                                                                                        "Etc/AdvanceAltar/AdvanceAltarConfig.etc");
                                                                            if (action_point_result) {
                                                                                printf(
                                                                                    "importConfigParameter Fail(%d)\n",
                                                                                    action_point_result);
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            sync->truncate_charac_advance_altar_item_desc();
                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            sync->insert_charac_advance_altar_item_desc();

                                                                            if (importPVPChannelList(
                                                                                    "Etc/pvpChannelList.etc",
                                                                                    &live_server_info_,
                                                                                    0) !=
                                                                                1) {
                                                                                cMyTrace pvp_channel_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7381,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_channel_trace(
                                                                                    "\t- importPVPChannelList Fail()");
                                                                                return 0;
                                                                            }

                                                                            if (importGuildParameterScript(
                                                                                    &guild_parameter_script_,
                                                                                    "Etc/Guild.etc") !=
                                                                                1) {
                                                                                cMyTrace guild_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7394,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                guild_trace(
                                                                                    "\t- importGuildParameterScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (importPowerParameterScript(
                                                                                    &power_parameter_script_,
                                                                                    "Etc/Power.etc") !=
                                                                                1) {
                                                                                cMyTrace power_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7401,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                power_trace(
                                                                                    "\t- importPowerParameterScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (importSecretShopScript(
                                                                                    &secret_shop_script_,
                                                                                    "Etc/SecretShop.etc") !=
                                                                                1) {
                                                                                cMyTrace secret_shop_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7439,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                secret_shop_trace(
                                                                                    "\t- importSecretShopScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (importAccountCargoScript(
                                                                                    &account_cargo_script_,
                                                                                    "Etc/AccountCargo.etc") !=
                                                                                1) {
                                                                                cMyTrace account_cargo_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7446,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                account_cargo_trace(
                                                                                    "\t- importAccountCargoScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!cargo_transport::ImportCargoTransportStoneScript(
                                                                                    "Etc/CargoTransportStone.etc")) {
                                                                                cMyTrace cargo_transport_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7447,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                cargo_transport_trace(
                                                                                    "\t- ImportCargoTransportStoneScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!equipment_growth::ImportEquipmentGrowthScript(
                                                                                    "Etc/equipment_grow.etc")) {
                                                                                cMyTrace equipment_growth_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7448,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                equipment_growth_trace(
                                                                                    "\t- ImportEquipmentGrowthScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!instant_reinforce::ImportConfig(
                                                                                    "Etc/DungeonEtc/InstantReinforceInfo.etc")) {
                                                                                cMyTrace instant_reinforce_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7448,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                instant_reinforce_trace(
                                                                                    "\t- ImportInstantReinforceConfig Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!time_spiral::ImportConfig(
                                                                                    "Etc/global_timespiral.etc")) {
                                                                                cMyTrace time_spiral_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7448,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                time_spiral_trace(
                                                                                    "\t- ImportTimeSpiralConfig Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!compound_creature::ImportCompoundCreatureScript(
                                                                                    "Etc/compoundcreaturejpn.etc")) {
                                                                                cMyTrace compound_creature_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7448,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                compound_creature_trace(
                                                                                    "\t- ImportCompoundCreatureScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (importTitleBookEtcScript(
                                                                                    "Etc/TitleBook.etc",
                                                                                    &title_book_info_) !=
                                                                                1) {
                                                                                cMyTrace title_book_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7454,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                title_book_trace(
                                                                                    "\t- importTitleBookEtcScript Fail");
                                                                                return 0;
                                                                            }

                                                                            char event_character_dir[] =
                                                                                "Event/EventCharacter/";
                                                                            char event_character_list[] =
                                                                                "EventCharacter.lst";
                                                                            if (initEventCharacterScript(
                                                                                    event_character_dir,
                                                                                    event_character_list) !=
                                                                                1) {
                                                                                cMyTrace event_character_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7480,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                event_character_trace(
                                                                                    "\t- Load EventCharacter.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (event_character_parameter_script_.
                                                                                    load_event_character_list() !=
                                                                                1) {
                                                                                cMyTrace event_character_load_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7486,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                event_character_load_trace(
                                                                                    "\t- Load EventCharacter.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (importQuestParameterScript(
                                                                                    &quest_parameter_script_,
                                                                                    "n_Quest/questParameter.etc") !=
                                                                                1) {
                                                                                cMyTrace quest_parameter_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7511,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                quest_parameter_trace(
                                                                                    "\t- importQuestParameterScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (importTrainingQuestScript(
                                                                                    &training_quest_script_,
                                                                                    "n_Quest/TrainingQuest.lst") !=
                                                                                1) {
                                                                                cMyTrace training_quest_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7521,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                training_quest_trace(
                                                                                    "\t- importTrainingQuestScript Fail");
                                                                                return 0;
                                                                            }

                                                                            if (server_parameter_script_.
                                                                                    CheckCollectDungeonStatisticServer(
                                                                                        environment->get_server_group(),
                                                                                        environment->get_channel_no())) {
                                                                                ::SetCollectDungeonStatistic(
                                                                                    true);
                                                                                cMyTrace statistic_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7556,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                statistic_trace(
                                                                                    "\t- Collect Dungeon Data Statistic start!");
                                                                            }

                                                                            pvp_assault::CAssaultMgr* assault_mgr =
                                                                                pvp_assault::
                                                                                    GetInstanceAssaultMgr();
                                                                            assault_mgr->SetAssaultChannel(
                                                                                    true);

                                                                            char slang_name_file[] =
                                                                                "Etc/slangname.etc";
                                                                            if (importSlangName(
                                                                                    slang_name_file,
                                                                                    &slang_name_data_) !=
                                                                                1) {
                                                                                cMyTrace slang_name_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7563,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                slang_name_trace(
                                                                                    "\t- slangname.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            char invalid_name_file[] =
                                                                                "Etc/invalidName.etc";
                                                                            if (importInvalidName(
                                                                                    invalid_name_file,
                                                                                    &invalid_char_data_) !=
                                                                                1) {
                                                                                cMyTrace invalid_name_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7568,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                invalid_name_trace(
                                                                                    "\t- invalidName.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            slang_filter_ =
                                                                                static_cast<SlangFilter*>(
                                                                                    ::operator new(0x80u));
                                                                            new (slang_filter_)
                                                                                SlangFilter();

                                                                            char fair_pvp_file[] =
                                                                                "Etc/FairPVPEquipment.etc";
                                                                            if (importFairPVPEquipment(
                                                                                    fair_pvp_file,
                                                                                    &fair_pvp_equipment_) !=
                                                                                1) {
                                                                                cMyTrace fair_pvp_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7577,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                fair_pvp_trace(
                                                                                    "\t- FairPVPEquipment.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            char growth_power_file[] =
                                                                                "Etc/GrowthPowerRewardBuff.etc";
                                                                            if (importGrowthPower(
                                                                                    growth_power_file,
                                                                                    &growth_power_data_) !=
                                                                                1) {
                                                                                cMyTrace growth_power_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7585,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                growth_power_trace(
                                                                                    "\t- GrowthPowerRewardBuff.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            char booster_gage_file[] =
                                                                                "Etc/BoosterGage.etc";
                                                                            if (importBoosterGage(
                                                                                    booster_gage_file,
                                                                                    &booster_gage_data_) !=
                                                                                1) {
                                                                                cMyTrace booster_gage_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7596,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                booster_gage_trace(
                                                                                    "\t- BoosterGage.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            if (initAICharacterScript(
                                                                                    "AICharacter/",
                                                                                    "AICharacter.lst") !=
                                                                                1) {
                                                                                cMyTrace ai_character_list_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7609,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                ai_character_list_trace(
                                                                                    "\t- Load AICharacter.lst fail");
                                                                                return 0;
                                                                            }

                                                                            CAICharacterList* ai_character_list =
                                                                                new CAICharacterList();
                                                                            set_ai_character_list(
                                                                                ai_character_list);
                                                                            if (ai_character_list->init() !=
                                                                                1) {
                                                                                cMyTrace ai_character_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7615,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                ai_character_trace(
                                                                                    "\t- Load AI Character fail");
                                                                                return 0;
                                                                            }

                                                                            if (server_parameter_script_.
                                                                                    CheckLightServer(
                                                                                        environment->get_server_group())) {
                                                                                cMyTrace light_server_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7621,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                light_server_trace(
                                                                                    "\t- Light Server Mode start!");
                                                                            }

                                                                            ImportWorldDrop(
                                                                                world_drop_info_,
                                                                                "Etc/WorldDrop.etc");
                                                                            ImportWorldDrop(
                                                                                original_world_drop_info_,
                                                                                "Etc/WorldDropWarArea.etc");

                                                                            const int premium_result =
                                                                                WongWork::CUserPremium::
                                                                                    LoadPremiumInfo(
                                                                                        "Etc/PremiumList_New.etc");
                                                                            if (premium_result) {
                                                                                cMyTrace premium_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7695,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                premium_trace(
                                                                                    "\t- LoadPremiumInfo Fail(Line: %d)",
                                                                                    premium_result);
                                                                                return 0;
                                                                            }

                                                                            if (IsLightServer() != 1 &&
                                                                                ImportWarAreaCounter(
                                                                                    &war_area_counter_) !=
                                                                                    1) {
                                                                                cMyTrace war_area_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7704,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                war_area_trace(
                                                                                    "\t- WarArea.etc file load failed");
                                                                                return 0;
                                                                            }

                                                                            if (InitCharacterScript(
                                                                                    "Character/",
                                                                                    "Character.lst",
                                                                                    "ExpTable.tbl",
                                                                                    0) !=
                                                                                1) {
                                                                                cMyTrace character_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7715,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                character_trace(
                                                                                    "\t- Load Character.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (InitMonsterScript(
                                                                                    "Monster/",
                                                                                    "Monster.lst",
                                                                                    "MonsterApcDifficultyBonus.tbl") !=
                                                                                1) {
                                                                                cMyTrace monster_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7720,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                monster_trace(
                                                                                    "\t- Load Monster.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (InitPassiveObjectScript(
                                                                                    "PassiveObject/",
                                                                                    "PassiveObject.lst") !=
                                                                                1) {
                                                                                cMyTrace passive_object_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7725,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                passive_object_trace(
                                                                                    "\t- Load PassiveObject.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (InitEquipmentScript(
                                                                                    "equipment/",
                                                                                    "equipment.lst",
                                                                                    "pricetable.tbl") !=
                                                                                1) {
                                                                                cMyTrace equipment_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7731,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                equipment_trace(
                                                                                    "\t- Load equipment.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (InitStackableScript(
                                                                                    "stackable/",
                                                                                    "stackable.lst",
                                                                                    "pricetable.tbl") !=
                                                                                1) {
                                                                                puts(
                                                                                    "Load stackable.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (InitMapScript(
                                                                                    "Map/",
                                                                                    "Map.lst") !=
                                                                                1) {
                                                                                cMyTrace map_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7752,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                map_trace(
                                                                                    "\t- Load Map.lst fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (InitDungeonScript(
                                                                                    const_cast<char*>("Dungeon/"),
                                                                                    const_cast<char*>("Dungeon.lst")) !=
                                                                                1) {
                                                                                cMyTrace dungeon_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7757,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                dungeon_trace(
                                                                                    "\t- Load Dungeon.lst fail\n");
                                                                                return 0;
                                                                            }

                                                                            QuickParty::CQuickPartyRewardManager*
                                                                                quick_party_reward =
                                                                                    G_CGameManager()->GetQuickPartyRewardManager();
                                                                            if (quick_party_reward->init() !=
                                                                                1) {
                                                                                return 0;
                                                                            }

                                                                            if (initQuestScript(
                                                                                    "n_Quest/",
                                                                                    "Quest.lst") !=
                                                                                1) {
                                                                                cMyTrace quest_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7772,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                quest_trace(
                                                                                    "\t- Load Quest.lst fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (InitSkillScript(
                                                                                    "Skill/",
                                                                                    "SkillList.lst") !=
                                                                                1) {
                                                                                cMyTrace skill_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7777,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                skill_trace(
                                                                                    "\t- Load SkillList.lst fail\n");
                                                                                return 0;
                                                                            }

                                                                            CItemShop* item_shop =
                                                                                static_cast<CItemShop*>(
                                                                                    ::operator new(
                                                                                        sizeof(CItemShop)));
                                                                            new (item_shop) CItemShop();
                                                                            item_shop_ = item_shop;
                                                                            if (initItemShopScript(
                                                                                    const_cast<char*>(
                                                                                        "ItemShop/"),
                                                                                    const_cast<char*>(
                                                                                        "ItemShop.lst")) !=
                                                                                1) {
                                                                                cMyTrace itemshop_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7784,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                itemshop_trace(
                                                                                    "\t- Load ItemShop.lst fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (item_shop_->loadItemShopFiles() != 1) {
                                                                                cMyTrace itemshop_file_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7789,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                itemshop_file_trace(
                                                                                    "\t- Load ItemShop file fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (initPvpMissionSystem(
                                                                                    const_cast<char*>(
                                                                                        "PVP_Mission/"),
                                                                                    const_cast<char*>(
                                                                                        "Mission.lst")) !=
                                                                                1) {
                                                                                return 0;
                                                                            }

                                                                            PvP_MissionSystem*
                                                                                pvp_mission_system =
                                                                                    static_cast<PvP_MissionSystem*>(
                                                                                        ::operator new(
                                                                                            sizeof(PvP_MissionSystem)));
                                                                            new (pvp_mission_system)
                                                                                PvP_MissionSystem();
                                                                            pvp_mission_system_ =
                                                                                pvp_mission_system;
                                                                            if (pvp_mission_system_->Load_list() != 1) {
                                                                                cMyTrace pvp_mission_script_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7801,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_mission_script_trace(
                                                                                    "\t- Mission Script file fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (pvp_mission_system_->Load_EtcParameter() != 1) {
                                                                                cMyTrace pvp_mission_parameter_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7806,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_mission_parameter_trace(
                                                                                    "\t- Mission Parameter Script file fail\n");
                                                                                return 0;
                                                                            }

                                                                            cera_shop_goods_name_ =
                                                                                "Etc/NewCashShop.etc";
                                                                            cash_shop_file_name_ =
                                                                                "CashShop/NewArad_CashShop.shp";
                                                                            sudden_shop_file_name_1_ =
                                                                                "CashShop/Sudden_Shop_1.shp";
                                                                            sudden_shop_file_name_2_ =
                                                                                "CashShop/Sudden_Shop_2.shp";
                                                                            sudden_shop_file_name_3_ =
                                                                                "CashShop/Sudden_Shop_3.shp";
                                                                            cMyTrace gold_limit_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7860,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            gold_limit_loading_trace(
                                                                                "\t- Loading Money Limit per Level ");
                                                                            if (importGoldLimitByLevel(
                                                                                    quest_shop_script_.money_limit_per_level) !=
                                                                                1) {
                                                                                cMyTrace gold_limit_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7863,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                gold_limit_fail_trace(
                                                                                    "\t- Loading Money Limit per Level Fail");
                                                                            }
                                                                            cMyTrace gold_limit_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7865,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            gold_limit_success_trace(
                                                                                "Success");

                                                                            cMyTrace compound_item_trace(
                                                                                "bool CDataManager::init()",
                                                                                7867,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            compound_item_trace(
                                                                                "\t- Loading Compound Item ");
                                                                            ExtreamDungeon::
                                                                                CCompound_ExtreamDun_Item*
                                                                                    compound_item =
                                                                                        static_cast<ExtreamDungeon::CCompound_ExtreamDun_Item*>(
                                                                                            ::operator new(
                                                                                                0xBECu));
                                                                            new (compound_item) ExtreamDungeon::CCompound_ExtreamDun_Item();
                                                                            GlobalData::s_pCompoundBindCube =
                                                                                compound_item;
                                                                            if (compound_item->LoadScript(
                                                                                    "Etc/",
                                                                                    "bindsphere.etc") !=
                                                                                0) {
                                                                                cMyTrace compound_item_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7871,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                compound_item_fail_trace(
                                                                                    "\t- Fail load Compound Item Bind Cube Script File");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace compound_item_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7874,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            compound_item_success_trace(
                                                                                "Success");

                                                                            cMyTrace event_upgrade_jar_trace(
                                                                                "bool CDataManager::init()",
                                                                                7877,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            event_upgrade_jar_trace(
                                                                                "\t- Loading ARAD - Event Upgrade Jar ");
                                                                            Event_Upgrade_Jar*
                                                                                event_upgrade_jar =
                                                                                    static_cast<Event_Upgrade_Jar*>(
                                                                                        ::operator new(
                                                                                            0x8Cu));
                                                                            new (event_upgrade_jar)
                                                                                Event_Upgrade_Jar();
                                                                            GlobalData::s_pEvent_UpGrade_Jar =
                                                                                event_upgrade_jar;
                                                                            if (!event_upgrade_jar->LoadScript(
                                                                                    "Etc/",
                                                                                    "Etc/EventUpgradeJar.etc")) {
                                                                                cMyTrace event_upgrade_jar_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7881,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                event_upgrade_jar_fail_trace(
                                                                                    "\t- Fail load Event.Upgrad.Jar Script File");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace event_upgrade_jar_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7884,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            event_upgrade_jar_success_trace(
                                                                                "Success");

                                                                            cMyTrace compound_avatar_trace(
                                                                                "bool CDataManager::init()",
                                                                                7897,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            compound_avatar_trace(
                                                                                "\t- Loading Compound Avatar ");
                                                                            WongWork::CCompoundAvatar*
                                                                                compound_avatar =
                                                                                    static_cast<WongWork::CCompoundAvatar*>(
                                                                                        ::operator new(
                                                                                            sizeof(WongWork::CCompoundAvatar)));
                                                                            if (compound_avatar) {
                                                                                new (compound_avatar)
                                                                                    WongWork::CCompoundAvatar();
                                                                                GlobalData::s_pCompoundAvatar =
                                                                                    compound_avatar;
                                                                            } else {
                                                                                GlobalData::s_pCompoundAvatar =
                                                                                    0;
                                                                            }
                                                                            if (GlobalData::s_pCompoundAvatar->LoadScript(
                                                                                    "Etc/",
                                                                                    "CompoundAvatar.etc") !=
                                                                                1) {
                                                                                cMyTrace compound_avatar_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7906,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                compound_avatar_fail_trace(
                                                                                    "\t- Loading Compound Avatar Fail\n");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace compound_avatar_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7909,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            compound_avatar_success_trace(
                                                                                "Success");

                                                                            importVendingMachineInfo(
                                                                                "Etc/",
                                                                                "VendingMachine.lst",
                                                                                vending_machine_table_);
                                                                            if (initWorldMapScript(
                                                                                    const_cast<char*>(
                                                                                        "WorldMap/"),
                                                                                    const_cast<char*>(
                                                                                        "WorldMap.lst")) !=
                                                                                1) {
                                                                                puts("Load WorldMap.lst fail");
                                                                                return 0;
                                                                            }
                                                                            initStageMapScript(
                                                                                const_cast<char*>(
                                                                                    "StageMap/"),
                                                                                const_cast<char*>(
                                                                                    "StageMap.lst"));

                                                                            cMyTrace dungeon_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7923,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            dungeon_loading_trace(
                                                                                "\t- Loading Dungeons ");
                                                                            CDungeonList* dungeon_list =
                                                                                static_cast<CDungeonList*>(
                                                                                    ::operator new(
                                                                                        sizeof(CDungeonList)));
                                                                            new (dungeon_list) CDungeonList();
                                                                            dungeon_index_list_ =
                                                                                dungeon_list;
                                                                            memset(dimension_inout_,
                                                                                   0,
                                                                                   sizeof(dimension_inout_));
                                                                            memset(original_dimension_inout_,
                                                                                   0,
                                                                                   sizeof(original_dimension_inout_));
                                                                            memset(limit_inout_count_,
                                                                                   0,
                                                                                   3u);
                                                                            if (dungeon_index_list_->load_list() != 1) {
                                                                                cMyTrace dungeon_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7941,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                dungeon_fail_trace(
                                                                                    "\t- Loading Dungeons Fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace dungeon_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7944,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            dungeon_success_trace(
                                                                                "Success");

                                                                            cMyTrace tower_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7947,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            tower_loading_trace(
                                                                                "\t- Loading Tower Of Despair System ");
                                                                            TowerOfDespairMgr*
                                                                                tower_mgr =
                                                                                    new TowerOfDespairMgr();
                                                                            GlobalData::s_pTowerOfDespairMgr =
                                                                                tower_mgr;
                                                                            if (tower_mgr->LoadScript(
                                                                                    "Etc/",
                                                                                    "TowerOfDespair.etc") !=
                                                                                0) {
                                                                                cMyTrace tower_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7951,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                tower_fail_trace(
                                                                                    "\t- Fail load TowerOfDespair System Script");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace tower_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7954,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            tower_success_trace(
                                                                                "Success");

                                                                            cMyTrace emoticon_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7958,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            emoticon_loading_trace(
                                                                                "\t- Loading Chatting Emoticon ");
                                                                            CChattingEmoticonList*
                                                                                emoticon_list =
                                                                                    static_cast<CChattingEmoticonList*>(
                                                                                        ::operator new(
                                                                                            0x48u));
                                                                            new (emoticon_list)
                                                                                CChattingEmoticonList();
                                                                            chatting_emoticon_list_ =
                                                                                emoticon_list;
                                                                            if (initChattingEmoticonScriptList(
                                                                                    const_cast<char*>(
                                                                                        "ChatEmoticon/"),
                                                                                    const_cast<char*>(
                                                                                        "ChatEmoticon.lst")) !=
                                                                                1) {
                                                                                cMyTrace emoticon_list_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7963,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                emoticon_list_fail_trace(
                                                                                    "\t- Emoticon List file error");
                                                                                return 0;
                                                                            }
                                                                            if (chatting_emoticon_list_->Load_list() !=
                                                                                1) {
                                                                                cMyTrace emoticon_script_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7968,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                emoticon_script_fail_trace(
                                                                                    "\t- Emoticon Script file fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace emoticon_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7971,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            emoticon_success_trace(
                                                                                "Success");

                                                                            std::vector<PVPMAP_DATA>
                                                                                pvp_map_data;
                                                                            if (loadPvpMapBasicDataScript(
                                                                                    "Etc/pvpmapbasicdata.etc",
                                                                                    &pvp_map_data) !=
                                                                                1) {
                                                                                cMyTrace pvp_map_basic_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7977,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_map_basic_fail_trace(
                                                                                    "Load Etc/pvpmapbasicdata.etc fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace map_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7981,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            map_loading_trace(
                                                                                "\t- Loading Maps ");
                                                                            CMapList* map_list =
                                                                                static_cast<CMapList*>(
                                                                                    ::operator new(
                                                                                        sizeof(CMapList)));
                                                                            new (map_list) CMapList();
                                                                            map_list_ = map_list;
                                                                            if (load_map_files(pvp_map_data) != 1) {
                                                                                cMyTrace map_files_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7987,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                map_files_fail_trace(
                                                                                    "\t- Loading Maps Fail");
                                                                                return 0;
                                                                            }
                                                                            if (!time_spiral::FinalizeConfig(this)) {
                                                                                cMyTrace time_spiral_finalize_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7989,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                time_spiral_finalize_trace(
                                                                                    "\t- FinalizeTimeSpiralConfig Fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace map_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                7990,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            map_success_trace(
                                                                                "Success");

                                                                            cMyTrace monster_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                7992,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            monster_loading_trace(
                                                                                "\t- Loading Monsters ");
                                                                            CMonsterManager*
                                                                                monster_manager =
                                                                                    static_cast<CMonsterManager*>(
                                                                                        ::operator new(
                                                                                            sizeof(CMonsterManager)));
                                                                            new (monster_manager)
                                                                                CMonsterManager();
                                                                            monster_manager_ =
                                                                                monster_manager;
                                                                            if (monster_manager_->load_monsters() !=
                                                                                1) {
                                                                                cMyTrace monster_files_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    7998,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                monster_files_fail_trace(
                                                                                    "\t- Loading Monsters Fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace monster_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8001,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            monster_success_trace(
                                                                                "Success");

                                                                            get_independent_drop_parameter_script()
                                                                                ->clear();
                                                                            cMyTrace skill_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8007,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            skill_loading_trace(
                                                                                "\t- Loading Skills ");
                                                                            CSkillList* skill_list =
                                                                                static_cast<CSkillList*>(
                                                                                    ::operator new(
                                                                                        sizeof(CSkillList)));
                                                                            new (skill_list) CSkillList();
                                                                            skill_list_ = skill_list;
                                                                            if (skill_list_->init() != 1) {
                                                                                cMyTrace skill_files_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8013,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                skill_files_fail_trace(
                                                                                    "\t- Loading Skills Fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace skill_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8016,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            skill_success_trace(
                                                                                "Success");

                                                                            cMyTrace creature_damage_trace(
                                                                                "bool CDataManager::init()",
                                                                                8018,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            creature_damage_trace(
                                                                                "\t- Loading Creatrue Damage ");
                                                                            if (prepareServerDelayCheckDamage() !=
                                                                                1) {
                                                                                cMyTrace creature_damage_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8021,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                creature_damage_fail_trace(
                                                                                    "\t- Loading Creatrue Damage Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace creature_damage_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8024,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            creature_damage_success_trace(
                                                                                "Success");

                                                                            SDCActiveStatusSyncServer active_status_storage;
                                                                            SDCActiveStatusSyncServer* active_status =
                                                                                &active_status_storage;
                                                                            cMyTrace active_status_trace(
                                                                                "bool CDataManager::init()",
                                                                                8028,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            active_status_trace(
                                                                                "\t- Loading ActiveStatus Damage ");
                                                                            if (active_status->loadActiveStatusScript() != 1) {
                                                                                cMyTrace active_status_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8031,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                active_status_fail_trace(
                                                                                    "\t- Loading ActiveStatus Damage Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace active_status_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8034,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            active_status_success_trace(
                                                                                "Success");
                                                                            std::map<std::string, int>* item_group_table =
                                                                                getItemGroupNameTable();
                                                                            const int item_group_result =
                                                                                importItmeGroupTable(
                                                                                    "Etc/",
                                                                                    "ItemGroupName.tbl",
                                                                                    *item_group_table);
                                                                            if (item_group_result) {
                                                                                return 0;
                                                                            }

                                                                            cMyTrace item_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8041,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            item_loading_trace(
                                                                                "\t- Loading Items ");
                                                                            CItemList* item_list =
                                                                                static_cast<CItemList*>(
                                                                                    ::operator new(
                                                                                        0x74u));
                                                                            new (item_list) CItemList();
                                                                            item_list_ = item_list;
                                                                            if (item_list_->init() !=
                                                                                1) {
                                                                                cMyTrace item_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8047,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                item_fail_trace(
                                                                                    "\t- Loading items Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace item_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8050,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            item_success_trace(
                                                                                "Success");
                                                                            cMyTrace avatar_roulette_trace(
                                                                                "bool CDataManager::init()",
                                                                                8074,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            avatar_roulette_trace(
                                                                                "\t- Loading Avatar Roulette ");
                                                                            AvatarRoulette
                                                                                avatar_roulette_storage;
                                                                            AvatarRoulette* avatar_roulette =
                                                                                &avatar_roulette_storage;
                                                                            char avatar_roulette_file[] =
                                                                                "Etc/avatar_roulette_jar.etc";
                                                                            if (importAvatarRouletteList(
                                                                                    avatar_roulette_file,
                                                                                    *avatar_roulette) !=
                                                                                1) {
                                                                                cMyTrace avatar_roulette_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8080,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                avatar_roulette_fail_trace(
                                                                                    "\t- Loading Avatar Roulette Fail");
                                                                                return 0;
                                                                            }

                                                                            AvatarRouletteServer*
                                                                                avatar_roulette_server =
                                                                                    new AvatarRouletteServer();
                                                                            GlobalData::s_pAvatarRoulette =
                                                                                avatar_roulette_server;
                                                                            if (!GlobalData::s_pAvatarRoulette) {
                                                                                return 0;
                                                                            }

                                                                            if (GlobalData::s_pAvatarRoulette->Init(
                                                                                    avatar_roulette) !=
                                                                                1) {
                                                                                cMyTrace avatar_init_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8090,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                avatar_init_fail_trace(
                                                                                    "\t- Initalizing Avatar Roulette Fail");
                                                                                return 0;
                                                                            }

                                                                            if (GlobalData::s_pAvatarRoulette->InitAddCharacSkill() !=
                                                                                1) {
                                                                                return 0;
                                                                            }

                                                                            cMyTrace avatar_roulette_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8102,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            avatar_roulette_success_trace(
                                                                                "Success");

                                                                            const char* random_option_files[7] = {
                                                                                "Etc/RandomOption/OptionGrouping.etc",
                                                                                "Etc/RandomOption/OptionGroupSelection.etc",
                                                                                "Etc/RandomOption/OptionNumbering.etc",
                                                                                "Etc/RandomOption/OptionQuantity.etc",
                                                                                "Etc/RandomOption/PartSelection.etc",
                                                                                "Etc/RandomOption/RandomizedOptionOverall1.etc",
                                                                                "Etc/RandomOption/RandomizedOptionOverall2.etc"};
                                                                            cMyTrace random_option_trace(
                                                                                "bool CDataManager::init()",
                                                                                8118,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            random_option_trace(
                                                                                "\t- Loading RandomOption ");
                                                                            for (int random_index = 0;
                                                                                 random_index <= 6;
                                                                                 ++random_index) {
                                                                                const int random_result =
                                                                                    ::importRandomOptionScript(
                                                                                        random_option_script_,
                                                                                        random_option_files
                                                                                            [random_index]);
                                                                                if (random_result) {
                                                                                    cMyTrace random_fail_trace(
                                                                                        "bool CDataManager::init()",
                                                                                        8123,
                                                                                        9,
                                                                                        true,
                                                                                        true);
                                                                                    random_fail_trace(
                                                                                        "\t- Fail load RandomOption Script File name(%s), error_line(%d)\n",
                                                                                        random_option_files
                                                                                            [random_index],
                                                                                        random_result);
                                                                                    return 0;
                                                                                }
                                                                            }

                                                                            cMyTrace random_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8127,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            random_success_trace(
                                                                                "Success");
                                                                            if (initRandomOptionItem() != 1) {
                                                                                cMyTrace random_item_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8131,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                random_item_fail_trace(
                                                                                    "Fail load initRandomOptionItem");
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            if (sync->truncate_random_option_ref() !=
                                                                                true) {
                                                                                cMyTrace truncate_random_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8137,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                truncate_random_fail_trace(
                                                                                    "\t- Fail truncate_random_option_ref");
                                                                                return 0;
                                                                            }

                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            if (sync->insert_random_option_ref() !=
                                                                                1) {
                                                                                cMyTrace insert_random_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8143,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                insert_random_fail_trace(
                                                                                    "\t- Fail truncate_random_option_ref");
                                                                                return 0;
                                                                            }

                                                                            if (regeneration_roi_.
                                                                                    _0_Load_RegenerationRandomOptionItem()) {
                                                                                return 0;
                                                                            }

                                                                            if (world_map_list_.load_list() != 1) {
                                                                                cMyTrace world_map_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8161,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                world_map_fail_trace(
                                                                                    "\t- Loading World Map List Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace quest_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8165,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            quest_loading_trace(
                                                                                "\t- Loading Quests ");
                                                                            QuestList* quest_list =
                                                                                static_cast<QuestList*>(
                                                                                    ::operator new(
                                                                                        sizeof(QuestList)));
                                                                            new (quest_list) QuestList();
                                                                            quest_list_ = quest_list;
                                                                            if (quest_list_->load_list() !=
                                                                                1) {
                                                                                cMyTrace quest_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8169,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                quest_fail_trace(
                                                                                    "\t- Loading Quests Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace quest_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8172,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            quest_success_trace(
                                                                                "Success");
                                                                            cMyTrace charac_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8174,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            charac_loading_trace(
                                                                                "\t- Loading Characters ");
                                                                            CCharacList* charac_list =
                                                                                static_cast<CCharacList*>(
                                                                                    ::operator new(
                                                                                        sizeof(CCharacList)));
                                                                            new (charac_list)
                                                                                CCharacList();
                                                                            character_list_ = charac_list;
                                                                            if (character_list_->init() !=
                                                                                1) {
                                                                                cMyTrace charac_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8178,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                charac_fail_trace(
                                                                                    "\t- Loading Characters Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace charac_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8181,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            charac_success_trace(
                                                                                "Success");
                                                                            cMyTrace disjoint_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8183,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            disjoint_loading_trace(
                                                                                "\t- Loading Disjoints ");
                                                                            DisJoint* disjoint =
                                                                                static_cast<DisJoint*>(
                                                                                    ::operator new(
                                                                                        sizeof(DisJoint)));
                                                                            new (disjoint)
                                                                                DisJoint();
                                                                            disjoint_ = disjoint;
                                                                            if (disjoint_->LoadDisJoint() !=
                                                                                1) {
                                                                                cMyTrace disjoint_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8187,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                disjoint_fail_trace(
                                                                                    "\t- Loading Disjoints Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace disjoint_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8190,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            disjoint_success_trace(
                                                                                "Success");
                                                                            cMyTrace stage_map_loading_trace(
                                                                                "bool CDataManager::init()",
                                                                                8193,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            stage_map_loading_trace(
                                                                                "\t- Loading Advance Altar ");
                                                                            if (stage_map_list_.loadList() != 1) {
                                                                                cMyTrace stage_map_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8196,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                stage_map_fail_trace(
                                                                                    "\t- Loading Advance Altar Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace stage_map_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8199,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            stage_map_success_trace(
                                                                                "Success");
                                                                            if (item_generator_ == 0) {
                                                                                WongWork::CItemGeneratorMgr* item_generator =
                                                                                    static_cast<
                                                                                        WongWork::CItemGeneratorMgr*>(
                                                                                        ::operator new(
                                                                                            0xA28u));
                                                                                new (item_generator) WongWork::CItemGeneratorMgr();
                                                                                item_generator_ = item_generator;
                                                                            }

                                                                            if (item_generator_->initialize() !=
                                                                                1) {
                                                                                cMyTrace item_generator_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8210,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                item_generator_fail_trace(
                                                                                    "\t- Loading Item Generator Manager Fail\n");
                                                                                return 0;
                                                                            }

                                                                            if (auto_market_control_ == 0) {
                                                                                CAutoMarketConditionsControl* control =
                                                                                    static_cast<
                                                                                        CAutoMarketConditionsControl*>(
                                                                                        ::operator new(
                                                                                            0x444u));
                                                                                new (control)
                                                                                    CAutoMarketConditionsControl();
                                                                                auto_market_control_ = control;
                                                                            }

                                                                            const int auto_market_result =
                                                                                auto_market_control_->LoadScript(
                                                                                        "Etc/ItemDropInfo_Control.etc");
                                                                            if (auto_market_result) {
                                                                                cMyTrace auto_market_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8218,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                auto_market_fail_trace(
                                                                                    "\t- LoadScript() error Etc/ItemDropInfo_Control.etc line(%d)",
                                                                                    auto_market_result);
                                                                                return 0;
                                                                            }

                                                                            cMyTrace level_exp_trace(
                                                                                "bool CDataManager::init()",
                                                                                8226,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            level_exp_trace(
                                                                                "\t- Init Level Exp ");
                                                                            set_level_exp();
                                                                            cMyTrace level_exp_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8232,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            level_exp_success_trace(
                                                                                "Success");
                                                                            cMyTrace mob_reward_trace(
                                                                                "bool CDataManager::init()",
                                                                                8234,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            mob_reward_trace(
                                                                                "\t- Init Mob Reward ");
                                                                            set_mob_reward();
                                                                            cMyTrace mob_reward_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8240,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            mob_reward_success_trace(
                                                                                "Success");
                                                                            cMyTrace pvp_grade_trace(
                                                                                "bool CDataManager::init()",
                                                                                8242,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            pvp_grade_trace(
                                                                                "\t- Init PVP Grade ");
                                                                            if (set_pvp_grade() != 1) {
                                                                                cMyTrace pvp_grade_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8245,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_grade_fail_trace(
                                                                                    "\t- Init PVP Grade Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace pvp_grade_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8248,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            pvp_grade_success_trace(
                                                                                "Success");
                                                                            cMyTrace reward_sp_trace(
                                                                                "bool CDataManager::init()",
                                                                                8250,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            reward_sp_trace(
                                                                                "\t- Init Reward SP ");
                                                                            if (set_reward_sp() != 1) {
                                                                                cMyTrace reward_sp_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8253,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                reward_sp_fail_trace(
                                                                                    "\t- Init Reward SP Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace reward_sp_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8256,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            reward_sp_success_trace(
                                                                                "Success");
                                                                            cMyTrace channel_data_trace(
                                                                                "bool CDataManager::init()",
                                                                                8260,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            channel_data_trace(
                                                                                "\t- Init Get Channel Data(channel_info.etc) ");
                                                                            if ((GlobalData::s_channel_proxy)->SendCheckChannelScript() !=
                                                                                1) {
                                                                                cMyTrace channel_data_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8263,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                channel_data_fail_trace(
                                                                                    "\t- Init Get Channel Data(channel_info.etc) Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace channel_data_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8266,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            channel_data_success_trace(
                                                                                "Success");
                                                                            cMyTrace channel_script_trace(
                                                                                "bool CDataManager::init()",
                                                                                8270,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            channel_script_trace(
                                                                                "\t- Init Channel Script ");
                                                                            if (importChannelScript(
                                                                                    &channel_script_) !=
                                                                                1) {
                                                                                cMyTrace channel_script_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8280,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                channel_script_fail_trace(
                                                                                    "\t- Init Channel Script Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace channel_script_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8283,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            channel_script_success_trace(
                                                                                "Success");
                                                                            WongWork::CItemUpgrade* item_upgrade =
                                                                                new WongWork::CItemUpgrade();
                                                                            item_upgrade_ = item_upgrade;
                                                                            cMyTrace item_upgrade_trace(
                                                                                "bool CDataManager::init()",
                                                                                8286,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            item_upgrade_trace(
                                                                                "\t- Init Item Upgrade Info ");
                                                                            if (item_upgrade_->LoadUpgradeScript(
                                                                                        "Etc/Upgrade.etc") !=
                                                                                1) {
                                                                                cMyTrace item_upgrade_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8289,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                item_upgrade_fail_trace(
                                                                                    "\t- Init Item Upgrade Info Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace item_upgrade_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8292,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            item_upgrade_success_trace(
                                                                                "Success");
                                                                            WongWork::CItemUpgrade_Separate* item_upgrade_separate =
                                                                                static_cast<
                                                                                    WongWork::CItemUpgrade_Separate*>(
                                                                                    ::operator new(
                                                                                        0x194u));
                                                                            new (item_upgrade_separate) WongWork::CItemUpgrade_Separate();
                                                                            item_upgrade_sep_ =
                                                                                item_upgrade_separate;
                                                                            if (item_upgrade_sep_->LoadUpgradeScript(
                                                                                    "Etc/Upgrade_Separate.etc") !=
                                                                                1) {
                                                                                cMyTrace item_upgrade_separate_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8298,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                item_upgrade_separate_fail_trace(
                                                                                    "\t- Item Upgrade Separate Fail");
                                                                                return 0;
                                                                            }

                                                                            if (item_upgrade_->Load3rdChronicleUpgradeScript(
                                                                                        "Etc/chronicle.etc") !=
                                                                                1) {
                                                                                cMyTrace chronicle_upgrade_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8306,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                chronicle_upgrade_fail_trace(
                                                                                    "\t- Loading 3rd Chronice Upgrade Script Fail");
                                                                                return 0;
                                                                            }

                                                                            if (!GlobalData::s_itemAmplifier_ ||
                                                                                GlobalData::s_itemAmplifier_->loadScript(
                                                                                    "Etc/AmplifyItem.etc",
                                                                                    "Etc/AmplifyUpgrade.etc") != 1) {
                                                                                cMyTrace amplifier_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8321,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                amplifier_fail_trace(
                                                                                    "\t- Loading Amplify Item/Upgrade Script Fail");
                                                                                return 0;
                                                                            }
                                                                            cMyTrace reseal_trace(
                                                                                "bool CDataManager::init()",
                                                                                8324,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            reseal_trace(
                                                                                "\t- Init Reseal table ");
                                                                            if (importResealTable(
                                                                                    "Etc/Reseal.etc",
                                                                                    &reseal_table_) !=
                                                                                1) {
                                                                                cMyTrace reseal_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8332,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                reseal_fail_trace(
                                                                                    "\t- Init Reseal table Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace reseal_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8335,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            reseal_success_trace(
                                                                                "Success");
                                                                            cMyTrace unlimit_challenge_trace(
                                                                                "bool CDataManager::init()",
                                                                                8338,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            unlimit_challenge_trace(
                                                                                "\t- Init Unlimit Challenge Info ");
                                                                            if (importUnlimitChallengeScript(
                                                                                    "Etc/UnlimitChallenge.etc",
                                                                                    &unlimit_challenge_info_) !=
                                                                                1) {
                                                                                cMyTrace unlimit_challenge_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8341,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                unlimit_challenge_fail_trace(
                                                                                    "\t- Init Unlimit Challenge Info Fail");
                                                                                return 0;
                                                                            }

                                                                            level_reward_table_manager_ =
                                                                                CLevelRewardTableMgr::GetInstance();
                                                                            level_reward_table_manager_->
                                                                                SetChallengeInfo(
                                                                                    unlimit_challenge_info_);
                                                                            cMyTrace unlimit_challenge_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8350,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            unlimit_challenge_success_trace(
                                                                                "Success");
                                                                            if (importLimitItemUsageInfo(
                                                                                    const_cast<char*>(
                                                                                        "Etc/Chn_Server_LimititemUsageinfo.etc"),
                                                                                    &limit_item_usage_info_) !=
                                                                                1) {
                                                                                cMyTrace limit_item_usage_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8357,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                limit_item_usage_fail_trace(
                                                                                    "\t- importLimitItemUsageInfo Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace limit_item_usage_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8360,
                                                                                9,
                                                                                true,
                                                                                true);
                                                                            limit_item_usage_success_trace(
                                                                                "\t- Import Limit Item Usage Info Success");
                                                                            cMyTrace db_log_item_trace(
                                                                                "bool CDataManager::init()",
                                                                                8375,
                                                                                9,
                                                                                true,
                                                                                true);
                                                                            db_log_item_trace(
                                                                                "\t- Import DB Log Item - ");
                                                                            if (importDBLogItem(
                                                                                    const_cast<char*>(
                                                                                        "Etc/Chn_DBLogItem.etc"),
                                                                                    &db_log_items_) !=
                                                                                1) {
                                                                                cMyTrace db_log_item_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8382,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                db_log_item_fail_trace(
                                                                                    "\t- Import DB Log Item  Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace db_log_item_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8385,
                                                                                9,
                                                                                true,
                                                                                true);
                                                                            db_log_item_success_trace(
                                                                                "\t- Import DB Log Item Success");
                                                                            cMyTrace condition_event_trace(
                                                                                "bool CDataManager::init()",
                                                                                8411,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            condition_event_trace(
                                                                                "\t- Import Condition Event Info ");
                                                                            if (importConditionEventScript(
                                                                                    "Event/ConditionEvent.evt",
                                                                                    &condition_event_info_) !=
                                                                                1) {
                                                                                cMyTrace condition_event_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8418,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                condition_event_fail_trace(
                                                                                    "\t- Import Condition Event Info Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace condition_event_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8421,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            condition_event_success_trace(
                                                                                "Success");
                                                                            cMyTrace condition_event_dungeon_trace(
                                                                                "bool CDataManager::init()",
                                                                                8450,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            condition_event_dungeon_trace(
                                                                                "\t- Import Condition Event Script Dungeon ");
                                                                            if (importConditionEventChkDungeonScript(
                                                                                    "Event/ConditionEventChkDungeon.evt",
                                                                                    &condition_level_dungeon_) !=
                                                                                1) {
                                                                                cMyTrace condition_event_dungeon_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8457,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                condition_event_dungeon_fail_trace(
                                                                                    "\t- Import Condition Event Script Dungeon Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace condition_event_dungeon_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8460,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            condition_event_dungeon_success_trace(
                                                                                "Success");
                                                                            cMyTrace npc_relation_event_trace(
                                                                                "bool CDataManager::init()",
                                                                                8465,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            npc_relation_event_trace(
                                                                                "\t- Import NPC Relation Event Info Success");
                                                                            if (importNPCRelationEventScript(
                                                                                    "Event/NPCRelationEvent.evt",
                                                                                    &npc_relation_event_manager_) !=
                                                                                1) {
                                                                                cMyTrace npc_relation_event_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8472,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                npc_relation_event_fail_trace(
                                                                                    "\t- Import NPC Relation Event Info Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace npc_relation_event_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8475,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            npc_relation_event_success_trace(
                                                                                "Success");
                                                                            cMyTrace emotion_type_tags_trace(
                                                                                "bool CDataManager::init()",
                                                                                8478,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            emotion_type_tags_trace(
                                                                                "\t- Import Emotion Type Tags Info ");
                                                                            if (importEmotionTypeTagsScript(
                                                                                    "Etc/EmotionExpressionData.etc",
                                                                                    &emotion_type_tags_info_) !=
                                                                                1) {
                                                                                cMyTrace emotion_type_tags_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8481,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                emotion_type_tags_fail_trace(
                                                                                    "\t- Import Emotion Type Tags Info Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace emotion_type_tags_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8484,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            emotion_type_tags_success_trace(
                                                                                "Success");
                                                                            cMyTrace equipment_grade_sample_trace(
                                                                                "bool CDataManager::init()",
                                                                                8486,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            equipment_grade_sample_trace(
                                                                                "\t- Import Equipment Grade Sample Info ");
                                                                            if (importEquipmentGradeSampleInfo(
                                                                                    "Etc/EquipmentGradeSample.etc",
                                                                                    &equipment_grade_sample_) !=
                                                                                1) {
                                                                                cMyTrace equipment_grade_sample_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8489,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                equipment_grade_sample_fail_trace(
                                                                                    "\t- Import Equipment Grade Sample Info Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace equipment_grade_sample_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8492,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            equipment_grade_sample_success_trace(
                                                                                "Success");
                                                                            cMyTrace creature_script_trace(
                                                                                "bool CDataManager::init()",
                                                                                8540,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            creature_script_trace(
                                                                                "\t- Import Creature Script");
                                                                            char creature_src[] =
                                                                                "Creature/";
                                                                            char creature_list_file[] =
                                                                                "Creature.lst";
                                                                            char creature_exp_table[] =
                                                                                "ExpTable.tbl";
                                                                            if (initCreatureScript(
                                                                                    creature_src,
                                                                                    creature_list_file,
                                                                                    creature_exp_table) !=
                                                                                1) {
                                                                                cMyTrace creature_list_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8543,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                creature_list_fail_trace(
                                                                                    "\t- Import Creature Creature.lst fail");
                                                                                return 0;
                                                                            }

                                                                            user_creature::
                                                                                CCreatureScriptMgr*
                                                                                    creature_script_mgr =
                                                                                        user_creature::
                                                                                            GetInstanceCreatureScriptMgr();
                                                                            if (creature_script_mgr
                                                                                    ->LoadCreatureScriptFile() !=
                                                                                1) {
                                                                                cMyTrace creature_runtime_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8549,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                creature_runtime_fail_trace(
                                                                                    "\t- Import Creature Script Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace creature_script_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8552,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            creature_script_success_trace(
                                                                                "Success");
                                                                            char pvp_single_map_file[] =
                                                                                "extern.sd";
                                                                            CMapList::LoadPvPSinglePlayableMap(
                                                                                pvp_single_map_file);
                                                                            CNPCScriptList*
                                                                                npc_script_list =
                                                                                    static_cast<CNPCScriptList*>(
                                                                                        ::operator new(
                                                                                            0x64u));
                                                                            new (npc_script_list) CNPCScriptList();
                                                                            npc_script_list_ = npc_script_list;
                                                                            CNPCDynamicInfoManager*
                                                                                npc_dynamic_info_manager =
                                                                                    static_cast<CNPCDynamicInfoManager*>(
                                                                                        ::operator new(
                                                                                            0x18u));
                                                                            new (npc_dynamic_info_manager) CNPCDynamicInfoManager();
                                                                            npc_dynamic_info_manager_ =
                                                                                npc_dynamic_info_manager;
                                                                            cMyTrace npc_script_trace(
                                                                                "bool CDataManager::init()",
                                                                                8570,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            npc_script_trace(
                                                                                "\t- Import Npc Script ");
                                                                            char npc_src[] =
                                                                                "Npc/";
                                                                            char npc_list_file[] =
                                                                                "Npc.lst";
                                                                            char npc_common_file[] =
                                                                                "common.dat";
                                                                            if (npc_script_list_->initScript(
                                                                                    npc_src,
                                                                                    npc_list_file,
                                                                                    npc_common_file)) {
                                                                                cMyTrace npc_script_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8573,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                npc_script_fail_trace(
                                                                                    "\t- Import Npc.lst Fail\n");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace npc_script_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8576,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            npc_script_success_trace(
                                                                                "Success");
                                                                            CItemLimitEditionMgr*
                                                                                item_limit_edition_mgr =
                                                                                    static_cast<CItemLimitEditionMgr*>(
                                                                                        ::operator new(
                                                                                            0x14u));
                                                                            new (item_limit_edition_mgr) CItemLimitEditionMgr();
                                                                            item_limit_edition_manager_ =
                                                                                item_limit_edition_mgr;
                                                                            sync =
                                                                                sync_script::GetInstanceSyncScript();
                                                                            sync->trancate_item_parts_set();
                                                                            cMyTrace equipment_part_set_trace(
                                                                                "bool CDataManager::init()",
                                                                                8620,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            equipment_part_set_trace(
                                                                                "\t- Import Equipment Part Set Info ");
                                                                            char equipment_part_set_file[] =
                                                                                "Etc/EquipmentPartSet.etc";
                                                                            if (importEquipmentPartSetInfo(
                                                                                    equipment_part_set_file,
                                                                                    3) !=
                                                                                1) {
                                                                                cMyTrace equipment_part_set_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8623,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                equipment_part_set_fail_trace(
                                                                                    "\t- Import Equipment Part Set Info Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace equipment_part_set_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8627,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            equipment_part_set_success_trace(
                                                                                "Success");
                                                                            for (std::map<int, STEquipmentPartSetData>::iterator set_it =
                                                                                     ::g_equipmentPartSetMap_.begin();
                                                                                 set_it !=
                                                                                 ::g_equipmentPartSetMap_.end();
                                                                                 ++set_it) {
                                                                                for (std::vector<STEquipmentPartSetPiece>::iterator piece_it =
                                                                                         set_it->second.pieces.begin();
                                                                                     piece_it !=
                                                                                     set_it->second.pieces.end();
                                                                                     ++piece_it) {
                                                                                    sync_script::GetInstanceSyncScript()
                                                                                        ->insert_item_parts_set(
                                                                                            set_it->first,
                                                                                            *piece_it);
                                                                                }
                                                                            }
                                                                            cMyTrace fair_pvp_stat_trace(
                                                                                "bool CDataManager::init()",
                                                                                8650,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            fair_pvp_stat_trace(
                                                                                "\t- Import FairPvPStat ");
                                                                            if (importPairPvPStatScript(
                                                                                    fair_pvp_stat_script_,
                                                                                    "Etc/FairPvPStat.etc") !=
                                                                                1) {
                                                                                cMyTrace fair_pvp_stat_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8653,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                fair_pvp_stat_fail_trace(
                                                                                    "\t- Fail load FairPvPStat Script File");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace fair_pvp_stat_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8657,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            fair_pvp_stat_success_trace(
                                                                                "Success");
                                                                            char pvp_skill_tree_src[] =
                                                                                "Etc/";
                                                                            char pvp_skill_tree_list[] =
                                                                                "PvPSkillTree.lst";
                                                                            if (initPvPSkillTreeScript(
                                                                                    pvp_skill_tree_src,
                                                                                    pvp_skill_tree_list) !=
                                                                                1) {
                                                                                cMyTrace pvp_skill_tree_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8663,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_skill_tree_trace(
                                                                                    "\t- Load PvPSkillTree.lst fail");
                                                                                return 0;
                                                                            }

                                                                            if (pvp_skill_tree_parameter_script_.
                                                                                    load_pvp_skill_tree_list() !=
                                                                                1) {
                                                                                cMyTrace pvp_skill_tree_list_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8669,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_skill_tree_list_trace(
                                                                                    "\t- Load load_pvp_skill_tree_list fail");
                                                                                return 0;
                                                                            }

                                                                            if (pvp_skill_tree_parameter_script_.
                                                                                    importPairPvPSkillPointScript(
                                                                                        "Etc/PvPSkillPoint.etc") !=
                                                                                1) {
                                                                                cMyTrace pvp_skill_point_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8675,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pvp_skill_point_trace(
                                                                                    "\t- Load PairPvPSkillPointScript fail");
                                                                                return 0;
                                                                            }

                                                                            if (initFairPVPSkillPoint() != 1) {
                                                                                cMyTrace fair_pvp_skill_point_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8683,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                fair_pvp_skill_point_trace(
                                                                                    "\t- initFairPVPSkillPoint fail");
                                                                                return 0;
                                                                            }

                                                                            EventClassify::CEventScriptMng*
                                                                                event_script_mng =
                                                                                    new EventClassify::CEventScriptMng();
                                                                            event_script_manager_ =
                                                                                event_script_mng;
                                                                            cMyTrace ingame_event_trace(
                                                                                "bool CDataManager::init()",
                                                                                8733,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            ingame_event_trace(
                                                                                "\t- Loading Ingame Event Script List ");
                                                                            if (importInGameEventInfoList(
                                                                                    &event_script_manager_->get_info_list(),
                                                                                    "Event/InGameEventList.evt") !=
                                                                                1) {
                                                                                cMyTrace ingame_event_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8740,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                ingame_event_fail_trace(
                                                                                    "\t- Fail load ingame_event_script_list");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace ingame_event_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8744,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            ingame_event_success_trace(
                                                                                "Success");
                                                                            if (event_script_manager_->initScriptlist(
                                                                                    event_script_manager_->get_info_list()) !=
                                                                                1) {
                                                                                return 0;
                                                                            }

                                                                            cMyTrace event_avatar_trace(
                                                                                "bool CDataManager::init()",
                                                                                8753,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            event_avatar_trace(
                                                                                "\t- Loading Event Avatar List ");
                                                                            if (importEventAvatarListScript(
                                                                                    &event_avatar_list_script_,
                                                                                    "Event/EventAvatarList.evt") !=
                                                                                1) {
                                                                                cMyTrace event_avatar_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8765,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                event_avatar_fail_trace(
                                                                                    "\t- Fail Loading Event Avatar List");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace event_avatar_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8768,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            event_avatar_success_trace(
                                                                                "Success");
                                                                            cMyTrace event_etc_trace(
                                                                                "bool CDataManager::init()",
                                                                                8772,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            event_etc_trace(
                                                                                "\t- Loading Event Etc Info ");
                                                                            if (importEventEtcScript(
                                                                                    &event_etc_script_,
                                                                                    "Event/EventEtc.evt") !=
                                                                                1) {
                                                                                cMyTrace event_etc_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8782,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                event_etc_fail_trace(
                                                                                    "\t- Fail Loading Event Etc Info");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace event_etc_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8786,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            event_etc_success_trace(
                                                                                "Success");
                                                                            if (::AvatarVariation::
                                                                                    importAvatarColorVariation(
                                                                                        &avatar_color_info_) !=
                                                                                1) {
                                                                                cMyTrace avatar_color_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8827,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                avatar_color_fail_trace(
                                                                                    "\t- Loading importAvatarColorVariation  Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace mercenary_trace(
                                                                                "bool CDataManager::init()",
                                                                                8859,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            mercenary_trace(
                                                                                "\t- Loading Mercenary System ");
                                                                            const int mercenary_result =
                                                                                importMercenarySystemEtcScript(
                                                                                    "Etc/mercenary.etc",
                                                                                    &mercenary_system_info_);
                                                                            if (mercenary_result) {
                                                                                cMyTrace mercenary_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8864,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                mercenary_fail_trace(
                                                                                    "\t- Loading Mercenary System Fail - %d line \n",
                                                                                    mercenary_result);
                                                                                return 0;
                                                                            }

                                                                            cMyTrace mercenary_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                8867,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            mercenary_success_trace(
                                                                                "Success");
                                                                            NeedMaterialDataManager::
                                                                                getInstance();
                                                                            if (importPcBangEquipmentRendar(
                                                                                    "Etc/PcBangRentalService.etc",
                                                                                    &pc_bang_item_rental_data_) !=
                                                                                1) {
                                                                                cMyTrace pcbang_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8964,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                pcbang_fail_trace(
                                                                                    "\t- PcBangRentalService Fail!\n");
                                                                                return 0;
                                                                            }

                                                                            if (importDimensionActivationData(
                                                                                    "Event/DimensionActivationEvent.evt",
                                                                                    &dimension_activation_data_) !=
                                                                                1) {
                                                                                cMyTrace dimension_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    8971,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                dimension_fail_trace(
                                                                                    "\t- DimensionActivationEvent Fail!\n");
                                                                                return 0;
                                                                            }

                                                                            if (::GlobalData::s_rentalSystem
                                                                                    ->Ready() !=
                                                                                1) {
                                                                                std::cerr
                                                                                    << "[GlobalData::Init] The rental system is not ready!"
                                                                                    << std::endl;
                                                                                return 0;
                                                                            }

                                                                            cMyTrace delete_invalid_trace(
                                                                                "bool CDataManager::init()",
                                                                                9009,
                                                                                9,
                                                                                true,
                                                                                false);
                                                                            delete_invalid_trace(
                                                                                "\t- Load Delete Invalid Item Script");
                                                                            if (delete_invalid_item_script_.
                                                                                    loadScript() !=
                                                                                1) {
                                                                                cMyTrace delete_invalid_fail_trace(
                                                                                    "bool CDataManager::init()",
                                                                                    9012,
                                                                                    9,
                                                                                    true,
                                                                                    true);
                                                                                delete_invalid_fail_trace(
                                                                                    "\t- Load Delete Invalid Item Script Fail");
                                                                                return 0;
                                                                            }

                                                                            cMyTrace delete_invalid_success_trace(
                                                                                "bool CDataManager::init()",
                                                                                9015,
                                                                                9,
                                                                                false,
                                                                                true);
                                                                            delete_invalid_success_trace(
                                                                                "Success");
                                                                            // 0828-only zombie-mode metadata is optional for a 0627 PVF.
                                                                            // The importer disables its formal owner on absence/malformed
                                                                            // input instead of rejecting the common data-manager startup.
                                                                            (void)ZombieItemBox::LoadConfig();
                                                                            const bool zombie_matching_ready =
                                                                                channel_zombie_matching_authority::Initialize() &&
                                                                                channel_zombie_matching_producer::CanRegisterPackets();
                                                                            if (zombie_matching_ready) {
                                                                                PacketDispatcher* const packet_dispatcher =
                                                                                    G_PacketDispatcher();
                                                                                if (!packet_dispatcher ||
                                                                                    !packet_dispatcher->ConfigureChannelZombieMatchingDispatchers()) {
                                                                                    channel_zombie_matching_authority::Shutdown();
                                                                                }
                                                                            } else {
                                                                                channel_zombie_matching_authority::Shutdown();
                                                                            }
                                                                            (void)fatigue_acceleration::LoadConfig();
                                                                            (void)champion_break::LoadConfig();
                                                                            // 0828-only CollectBox data is absent from a stock 0627 PVF.
                                                                            // Keep common startup compatible, while every CollectBox
                                                                            // request fails closed unless both official ETC files publish.
                                                                            (void)collect_box_script::LoadConfig();
                                                                            return 1;
                                                                        }

                                                                        cMyTrace blue_marble_info_trace(
                                                                            "bool CDataManager::init()",
                                                                            6944,
                                                                            9,
                                                                            true,
                                                                            true);
                                                                        blue_marble_info_trace(
                                                                            "\t- BlueMarble initBlueMarbleInfo Fail\n");
                                                                        return 0;
                                                                    }

                                                                    cMyTrace blue_marble_map_trace(
                                                                        "bool CDataManager::init()",
                                                                        6938,
                                                                        9,
                                                                        true,
                                                                        true);
                                                                    blue_marble_map_trace(
                                                                        "\t- BlueMarble initMapList Fail\n");
                                                                    return 0;
                                                                }

                                                                cMyTrace blue_marble_tile_trace(
                                                                    "bool CDataManager::init()",
                                                                    6932,
                                                                    9,
                                                                    true,
                                                                    true);
                                                                blue_marble_tile_trace(
                                                                    "\t- BlueMarble initTileList Fail\n");
                                                                return 0;
                                                            }

                                                            cMyTrace charac_manage_trace(
                                                                "bool CDataManager::init()",
                                                                6914,
                                                                9,
                                                                true,
                                                                true);
                                                            charac_manage_trace(
                                                                "\t- importCharacterManageScript Fail");
                                                            return 0;
                                                        }

                                                        cMyTrace striker_trace(
                                                            "bool CDataManager::init()",
                                                            6907,
                                                            9,
                                                            true,
                                                            true);
                                                        striker_trace(
                                                            "\t- importStrikerScript Fail");
                                                        return 0;
                                                    }

                                                    cMyTrace expertjob_trace(
                                                        "bool CDataManager::init()",
                                                        6853,
                                                        9,
                                                        true,
                                                        true);
                                                    expertjob_trace(
                                                        "\t- Load expertjob fail");
                                                    return 0;
                                                }

                                                cMyTrace expertjob_lst_trace(
                                                    "bool CDataManager::init()",
                                                    6847,
                                                    9,
                                                    true,
                                                    true);
                                                expertjob_lst_trace(
                                                    "\t- Load expertjob.lst fail");
                                                return 0;
                                            }

                                            cMyTrace training_trace(
                                                "bool CDataManager::init()",
                                                6839,
                                                9,
                                                true,
                                                true);
                                            training_trace(
                                                "\t- [training first give] error");
                                            return 0;
                                        }

                                        cMyTrace image_trace(
                                            "bool CDataManager::init()",
                                            6829,
                                            9,
                                            true,
                                            true);
                                        image_trace(
                                            "\t- Import ImageCommunicationInfo Failed\n");
                                        return 0;
                                    }

                                    cMyTrace seria_room_animation_trace(
                                        "bool CDataManager::init()",
                                        6779,
                                        9,
                                        true,
                                        true);
                                    seria_room_animation_trace(
                                        "\t- SeriaRoomAnimationDecoEventScript Fail!");
                                    return 0;
                                }

                                cMyTrace creature_gift_trace(
                                    "bool CDataManager::init()",
                                    6771,
                                    9,
                                    true,
                                    true);
                                creature_gift_trace(
                                    "\t- CharacterCreateCreatureGiftScript Fail!");
                                return 0;
                            }

                            cMyTrace use_job_fatigue_trace(
                                "bool CDataManager::init()",
                                6763,
                                9,
                                true,
                                true);
                            use_job_fatigue_trace(
                                "\t- UseJobFatigueGiftScript Fail!");
                            return 0;
                        }

                        cMyTrace create_character_gift_trace(
                            "bool CDataManager::init()",
                            6755,
                            9,
                            true,
                            true);
                        create_character_gift_trace(
                            "\t- CharacterCreateGiftScript Fail!");
                        return 0;
                    }

                    cMyTrace growth_equipment_trace(
                        "bool CDataManager::init()", 6747, 9, true, true);
                    growth_equipment_trace(
                        "\t- GrownEquipmentScript Fail!");
                    return 0;
                }

                cMyTrace seria_room_deco_trace("bool CDataManager::init()",
                                               6739,
                                               9,
                                               true,
                                               true);
                seria_room_deco_trace(
                    "\t- SeriaRoomDecoEventScript Fail!");
                return 0;
            }

            cMyTrace next_trace("bool CDataManager::init()",
                                6730,
                                9,
                                true,
                                true);
            next_trace("\t- importInGameAdvertisementEtcScript Fail");
            return 0;
        }

        cMyTrace server_parameter_trace("bool CDataManager::init()",
                                        6709,
                                        9,
                                        true,
                                        true);
        server_parameter_trace("\t- importServerParameterScript Fail");
        return 0;
    }

    cMyTrace trace("bool CDataManager::init()", 6514, 9, true, true);
    trace("\t- InitRDARScriptStringManager Fail");
    return 0;
}
