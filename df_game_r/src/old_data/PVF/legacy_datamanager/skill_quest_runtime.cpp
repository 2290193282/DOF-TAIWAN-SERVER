#include "old_data/PVF/n_quest/datamanager/skill_quest_runtime.h"

#include "common/config.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/passiveobject/passive_object_script_types.h"
#include "old_data/PVF/data/sync_script.h"
#include "old_data/PVF/skill/skill_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/npc_item_drop_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/social/party/party.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <new>
#include <stdio.h>
#include <string>
#include <vector>


extern STScriptFileList
    g_stSkillScriptFileList[oldsrv::config::kCharacterJobCount];
extern STScriptFileList g_questScriptFileList;
extern STScriptFileList g_stPassiveObjectScriptFileList;
extern char g_szPassiveObjectScriptBaseDirectory[256];

namespace {

struct NpcItemDropQuestContract {
    NpcItemDropQuestContract()
        : trigger_subtype(0),
          candidate_item_ids(),
          dungeon_info() {}

    int trigger_subtype;
    std::vector<int> candidate_item_ids;
    // The second AdditionalDungeonInfo dword is deliberately retained as an
    // opaque PVF value.  CMD588 authorization has evidence only for the first
    // dword's dungeon identity, not for a difficulty interpretation.
    std::vector<AdditionalDungeonInfo> dungeon_info;
};

struct NpcItemDropActionPoint {
    NpcItemDropActionPoint()
        : object_id(0), position_x(0), position_y(0), position_z(0) {}

    int object_id;
    int position_x;
    int position_y;
    int position_z;
};

struct NpcItemDropRawMapContract {
    std::list<int> dungeon_indices;
    std::vector<NpcItemDropActionPoint> passive_objects;
};

typedef std::map<int, NpcItemDropQuestContract> NpcItemDropQuestMap;
typedef std::map<int, std::vector<NpcItemDropActionPoint> >
    NpcItemDropMapActionMap;
typedef std::map<int, NpcItemDropRawMapContract> NpcItemDropRawMapContractMap;
typedef std::map<int, bool> NpcItemDropObjectMarkerCache;

Mutex g_npc_item_drop_mutex;
NpcItemDropQuestMap g_npc_item_drop_quests;
NpcItemDropRawMapContractMap g_npc_item_drop_raw_maps;
NpcItemDropMapActionMap g_npc_item_drop_map_actions;
NpcItemDropObjectMarkerCache g_npc_item_drop_object_markers;

bool ContainsString(const std::vector<std::string>& values,
                    const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void AppendActionPath(std::vector<std::string>& paths,
                      const std::string& path) {
    if (!path.empty() && !ContainsString(paths, path)) {
        paths.push_back(path);
    }
}

bool IsRdarPathSeparator(char value) {
    return value == '/' || value == '\\';
}

bool AppendNormalizedRdarSegment(const std::string& segment,
                                 std::string& normalized_path) {
    if (segment.empty() || segment == "." || segment == "..") {
        return false;
    }
    if (!normalized_path.empty()) {
        normalized_path.push_back('/');
    }
    normalized_path.append(segment);
    return true;
}

bool NormalizeRelativeRdarPath(const std::string& raw_path,
                               bool directory_path,
                               std::string& normalized_path) {
    normalized_path.clear();
    if (raw_path.empty() || IsRdarPathSeparator(raw_path[0])) {
        return false;
    }

    std::string segment;
    for (std::string::size_type index = 0; index != raw_path.size();
         ++index) {
        const char value = raw_path[index];
        const unsigned char byte = static_cast<unsigned char>(value);
        if (byte < 0x20u || byte == 0x7fu || value == ':') {
            return false;
        }
        if (!IsRdarPathSeparator(value)) {
            segment.push_back(value);
            continue;
        }
        if (!AppendNormalizedRdarSegment(segment, normalized_path)) {
            return false;
        }
        segment.clear();
    }

    if (!segment.empty()) {
        if (!AppendNormalizedRdarSegment(segment, normalized_path)) {
            return false;
        }
    } else if (!directory_path) {
        // An action file must not be reinterpreted as a directory.
        return false;
    }

    if (normalized_path.empty()) {
        return false;
    }
    if (directory_path) {
        normalized_path.push_back('/');
    }
    return true;
}

bool ResolveObjectRelativeActionPath(
    const STPassiveObjectScript& passive_script,
    const std::string& raw_action_path,
    std::string& object_directory,
    std::string& action_path) {
    std::string passive_object_base;
    if (!NormalizeRelativeRdarPath(g_szPassiveObjectScriptBaseDirectory,
                                   true, passive_object_base) ||
        !NormalizeRelativeRdarPath(passive_script.script_filename,
                                   true, object_directory) ||
        object_directory.size() < passive_object_base.size() ||
        object_directory.compare(0, passive_object_base.size(),
                                 passive_object_base) != 0 ||
        !NormalizeRelativeRdarPath(raw_action_path, false, action_path)) {
        return false;
    }
    return true;
}

bool ActionHasNpcItemDropMarker(
    const STPassiveObjectScript& passive_script,
    const std::string& raw_action_path) {
    std::string object_directory;
    std::string action_path;
    if (!ResolveObjectRelativeActionPath(passive_script, raw_action_path,
                                         object_directory, action_path) ||
        !loadRDARScriptFile(object_directory.c_str(),
                            action_path.c_str())) {
        return false;
    }

    bool found = false;
    for (;;) {
        const DNFLEX_TOKEN token = g_dnfLex.getToken();
        if (token == DNF_TOK_EOF || token == DNF_TOK_ERROR) {
            break;
        }
        if (token == DNF_TOK_TYPE) {
            const char* const type = g_dnfLex.getType();
            if (type && std::strcmp(type, "[NPC ITEM DROP]") == 0) {
                found = true;
                break;
            }
        }
    }
    unloadRDARScriptFile();
    return found;
}

bool PassiveObjectHasNpcItemDropMarker(int object_id) {
    const NpcItemDropObjectMarkerCache::const_iterator cached =
        g_npc_item_drop_object_markers.find(object_id);
    if (cached != g_npc_item_drop_object_markers.end()) {
        return cached->second;
    }

    bool found = false;
    const char* const filename =
        g_stPassiveObjectScriptFileList.GetFileName(object_id);
    if (filename) {
        STPassiveObjectScript passive_script;
        const int imported =
            ImportPassiveObjectScript(&passive_script, filename, false);
        // ImportPassiveObjectScript owns the active RDAR stream but does not
        // close it on its success return.  The action scans below must start
        // from an independent stream.
        unloadRDARScriptFile();
        if (imported == 1) {
            std::vector<std::string> action_paths;
            AppendActionPath(action_paths, passive_script.first_effect);
            AppendActionPath(action_paths, passive_script.second_effect);
            for (std::vector<std::string>::const_iterator action =
                     passive_script.second_string_values.begin();
                 action != passive_script.second_string_values.end();
                 ++action) {
                AppendActionPath(action_paths, *action);
            }
            for (std::vector<std::string>::const_iterator action =
                     action_paths.begin();
                 action != action_paths.end(); ++action) {
                if (ActionHasNpcItemDropMarker(passive_script, *action)) {
                    found = true;
                    break;
                }
            }
        }
    }

    g_npc_item_drop_object_markers.insert(
        std::make_pair(object_id, found));
    return found;
}

bool ContractMatchesDungeon(const NpcItemDropQuestContract& contract,
                            int dungeon_index) {
    for (std::vector<AdditionalDungeonInfo>::const_iterator dungeon =
             contract.dungeon_info.begin();
         dungeon != contract.dungeon_info.end(); ++dungeon) {
        if (dungeon->dungeon_index == dungeon_index) {
            return true;
        }
    }
    return false;
}

void AppendUniqueItemId(std::vector<unsigned int>& values,
                        unsigned int item_id) {
    if (std::find(values.begin(), values.end(), item_id) == values.end()) {
        values.push_back(item_id);
    }
}

void BuildEligibleWeaponPool(const NpcItemDropQuestContract& contract,
                             CUser* user,
                             std::vector<unsigned int>& output) {
    std::vector<unsigned int> priority;
    std::vector<unsigned int> compatible;
    const int job = user->get_charac_job();
    const int grow_type = user->getCurCharacGrowType();

    for (std::vector<int>::const_iterator candidate =
             contract.candidate_item_ids.begin();
         candidate != contract.candidate_item_ids.end(); ++candidate) {
        if (*candidate <= 0) {
            continue;
        }
        CItem* const item = G_CDataManager()->find_item(*candidate);
        if (!item) {
            continue;
        }
        Inven_Item prototype;
        item->vtable_->make_item(item, prototype);
        if (prototype.item_category_ != 1u || item->is_stackable() != 0) {
            continue;
        }
        CEquipItem* const equipment = static_cast<CEquipItem*>(item);
        if (equipment->GetItemType() !=
                oldsrv::config::kEquipmentTypeWeapon ||
            item->check_job_type(static_cast<char>(job)) != 1) {
            continue;
        }
        AppendUniqueItemId(compatible,
                           static_cast<unsigned int>(*candidate));
        if (item->IsPriorityJob(job, grow_type) == 1) {
            AppendUniqueItemId(priority,
                               static_cast<unsigned int>(*candidate));
        }
    }

    output = priority.empty() ? compatible : priority;
}

bool UserOwnsAnyCandidate(CUser* user,
                         const std::vector<unsigned int>& candidates) {
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory) {
        return false;
    }
    for (std::vector<unsigned int>::const_iterator item_id =
             candidates.begin();
         item_id != candidates.end(); ++item_id) {
        int slot = 0;
        bool equipment = false;
        if (inventory->CheckExistItemInInventoryOrEquipment(
                static_cast<int>(*item_id), slot, equipment) == 1) {
            return true;
        }
    }
    return false;
}

bool MapContainsAnyCandidate(
    const MapInfo& map,
    const std::vector<unsigned int>& candidates) {
    for (std::map<int, map_item>::const_iterator item = map.items_.begin();
         item != map.items_.end(); ++item) {
        const unsigned int item_id =
            static_cast<unsigned int>(item->second.item.item_id_);
        if (std::find(candidates.begin(), candidates.end(), item_id) !=
            candidates.end()) {
            return true;
        }
    }
    return false;
}

void SendNpcItemDropError(CUser* user, unsigned char error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EVENT_NPC_DROP_ITEM_, error);
    }
}

bool IsExactEmptyNpcItemDropRequest(PacketBuf& packet) {
    // The 0828 sender finalizes CMD588 without a business payload.  Inbound
    // decryption can leave transport-owned bytes after the declared frame, so
    // physical PacketBuf exhaustion is not a semantic end marker.  The C2S
    // header is the authoritative business boundary: reject a declared body
    // while leaving any bytes outside that boundary uninterpreted.
    if (packet.get_index() != PacketBuf::PACKET_INDEX_BEGIN ||
        packet.getLastError() != PacketBuf::PACKETBUF_ERROR_NONE) {
        return false;
    }

    const CS_Header* const header =
        reinterpret_cast<const CS_Header*>(packet.get_packet(0));
    return header && header->cmd == ENUM_PACKETCLASS_CMDPACKET &&
           header->type == ENUM_CMDPACKET_EVENT_NPC_DROP_ITEM_ &&
           header->length == sizeof(CS_Header);
}

}  // namespace

namespace npc_item_drop {

void ClearQuestContracts() {
    MutexGuard guard(&g_npc_item_drop_mutex);
    g_npc_item_drop_quests.clear();
}

void RegisterQuestContract(const QuestScript& script) {
    // importQuestScript maps [get item check index] to trigger type 18.
    // Both a candidate list and a [dungeon info] association are required;
    // generic inventory-check quests cannot authorize a map-side action.
    if (script.value_178 != 18 || script.limit_dungeon_indices.empty() ||
        script.additional_dungeon_info.empty()) {
        return;
    }

    NpcItemDropQuestContract contract;
    contract.trigger_subtype = script.value_17c;
    contract.candidate_item_ids = script.limit_dungeon_indices;
    contract.dungeon_info = script.additional_dungeon_info;

    MutexGuard guard(&g_npc_item_drop_mutex);
    g_npc_item_drop_quests[script.quest_index] = contract;
}

void ClearMapContracts() {
    MutexGuard guard(&g_npc_item_drop_mutex);
    g_npc_item_drop_raw_maps.clear();
    g_npc_item_drop_map_actions.clear();
    g_npc_item_drop_object_markers.clear();
}

void RegisterMapContract(const STMapScript& script) {
    MutexGuard guard(&g_npc_item_drop_mutex);
    NpcItemDropRawMapContract& contract =
        g_npc_item_drop_raw_maps[script.map_index];
    contract.dungeon_indices = script.dungeon_indices;
    contract.passive_objects.clear();

    for (std::list<STAssignPassiveObject>::const_iterator object =
             script.passive_objects.begin();
         object != script.passive_objects.end(); ++object) {
        NpcItemDropActionPoint action;
        action.object_id = object->object_id;
        action.position_x = object->position_x;
        action.position_y = object->position_y;
        action.position_z = object->position_z;
        contract.passive_objects.push_back(action);
    }
}

void ResolveMapContracts() {
    MutexGuard guard(&g_npc_item_drop_mutex);
    g_npc_item_drop_map_actions.clear();
    g_npc_item_drop_object_markers.clear();

    std::vector<int> authorized_dungeons;
    for (NpcItemDropQuestMap::const_iterator quest =
             g_npc_item_drop_quests.begin();
         quest != g_npc_item_drop_quests.end(); ++quest) {
        for (std::vector<AdditionalDungeonInfo>::const_iterator dungeon =
                 quest->second.dungeon_info.begin();
             dungeon != quest->second.dungeon_info.end(); ++dungeon) {
            if (std::find(authorized_dungeons.begin(),
                          authorized_dungeons.end(),
                          dungeon->dungeon_index) ==
                authorized_dungeons.end()) {
                authorized_dungeons.push_back(dungeon->dungeon_index);
            }
        }
    }

    for (NpcItemDropRawMapContractMap::const_iterator map =
             g_npc_item_drop_raw_maps.begin();
         map != g_npc_item_drop_raw_maps.end(); ++map) {
        bool authorized_map = false;
        for (std::list<int>::const_iterator dungeon =
                 map->second.dungeon_indices.begin();
             dungeon != map->second.dungeon_indices.end(); ++dungeon) {
            if (std::find(authorized_dungeons.begin(),
                          authorized_dungeons.end(), *dungeon) !=
                authorized_dungeons.end()) {
                authorized_map = true;
                break;
            }
        }
        if (!authorized_map) {
            continue;
        }

        std::vector<NpcItemDropActionPoint>& actions =
            g_npc_item_drop_map_actions[map->first];
        for (std::vector<NpcItemDropActionPoint>::const_iterator object =
                 map->second.passive_objects.begin();
             object != map->second.passive_objects.end(); ++object) {
            if (PassiveObjectHasNpcItemDropMarker(object->object_id)) {
                actions.push_back(*object);
            }
        }

        if (actions.empty()) {
            g_npc_item_drop_map_actions.erase(map->first);
        }
    }
}

}  // namespace npc_item_drop

// IDA_EA: 0x8350842
// IDA_NAME: _ZN10CSkillListC2Ev
// DEMANGLED: CSkillList::CSkillList(void)
CSkillList::CSkillList()
    : skill_by_index_(),
      skill_by_group_(),
      post_learning_skill_() {}

// IDA_EA: 0x83509EC
// IDA_NAME: _ZN10CSkillListD2Ev
// DEMANGLED: CSkillList::~CSkillList()
CSkillList::~CSkillList() {
    destroy();
}

// IDA_EA: 0x8350B42
// IDA_NAME: _ZN10CSkillList7destroyEv
// DEMANGLED: CSkillList::destroy(void)
bool CSkillList::destroy() {
    for (int job_type = 0;
         job_type < oldsrv::config::kCharacterJobCount;
         ++job_type) {
        for (__gnu_cxx::hash_map<int, CSkill*>::iterator it =
                 skill_by_index_[job_type].begin();
             it != skill_by_index_[job_type].end(); ++it) {
            CSkill* skill = it->second;
            if (skill) {
                skill->~CSkill();
                ::operator delete(skill);
            }
        }
        skill_by_index_[job_type].clear();
        skill_by_group_[job_type].clear();
        post_learning_skill_[job_type].clear();
    }
    return false;
}

// IDA_EA: 0x8350C26
// IDA_NAME: _ZN10CSkillList4initEv
// DEMANGLED: CSkillList::init(void)
int CSkillList::init() {
    ResetSkillAddDataRegistry0828();
    sync_script::CSyncScript* sync =
        sync_script::GetInstanceSyncScript();
    sync->truncate_skill_index();
    sync = sync_script::GetInstanceSyncScript();
    sync->trancate_skill_info();

    for (int job_type = 0;
         job_type < oldsrv::config::kCharacterJobCount;
         ++job_type) {
        for (STScriptFileList::const_iterator entry =
                 g_stSkillScriptFileList[job_type].begin();
             entry != g_stSkillScriptFileList[job_type].end(); ++entry) {
            const char* const filename = entry->second.c_str();
            if (!filename) {
                break;
            }

            STSkillScript script;
            if (ImportSkillScript(&script, filename,
                                  static_cast<ENUM_CHARACTERJOB>(job_type),
                                  0, -1) != 1) {
                printf("Skill Script Error: %s\n", filename);
                return 0;
            }

            sync = sync_script::GetInstanceSyncScript();
            if (sync->insert_skill_index(&script, job_type) != true) {
                printf("insert_skill_index Error: %s\n", filename);
                return 0;
            }

            sync = sync_script::GetInstanceSyncScript();
            if (sync->IsEnable() != 0) {
                sync = sync_script::GetInstanceSyncScript();
                if (sync->insert_skill_info(&script, job_type, 0) != true) {
                    printf("insert_skill_info Error: %s\n", filename);
                    return 0;
                }

                const int kExtraModuleTypes[] = {4, 6, 11};
                for (size_t module_index = 0;
                     module_index != sizeof(kExtraModuleTypes) /
                                         sizeof(kExtraModuleTypes[0]);
                     ++module_index) {
                    const int module_type = kExtraModuleTypes[module_index];
                    STSkillScript module_script;
                    if (ImportSkillScript(
                            &module_script, filename,
                            static_cast<ENUM_CHARACTERJOB>(job_type),
                            module_type, -1) != 1) {
                        printf("Skill Script Error: %s\n", filename);
                        return 0;
                    }
                    sync = sync_script::GetInstanceSyncScript();
                    if (sync->insert_skill_info(&module_script, job_type,
                                                module_type) != true) {
                        printf("insert_skill_info Error: %s\n", filename);
                        return 0;
                    }
                }
            }

            CSkill* const skill = new CSkill;
            skill->set_index(script.skill_index);
            skill->reform_ui_group_no(
                script.skill_class, script.feature_skill_type,
                static_cast<int>(script.skill_fitness_growtype.size()));
            skill->set_group(script.skill_class);
            skill->set_job_type(job_type);
            skill->set_skill_type(static_cast<char>(script.type));
            skill->set_level_limit(script.required_level);
            skill->level_interval_ = script.required_level_range;
            skill->SetStealable(script.steel_learning_enabled);
            skill->SetStealLevelPenalty(script.steel_learning_value_2);
            skill->SetStealSPPenalty(script.steel_learning_value);

            for (size_t index = 0;
                 index != script.skill_fitness_growtype.size(); ++index) {
                skill->set_fitGrowType(
                    static_cast<int>(index),
                    script.skill_fitness_growtype[index]);
            }
            for (int first_grow_type = 0; first_grow_type <= 5;
                 ++first_grow_type) {
                skill->set_fitness(first_grow_type,
                                   script.growtype_fitness[first_grow_type]);
                skill->set_type_max_level(
                    first_grow_type,
                    script.growtype_maximum_level[first_grow_type]);
                for (int second_grow_type = 1; second_grow_type <= 2;
                     ++second_grow_type) {
                    skill->set_second_growtype_fitness(
                        first_grow_type, second_grow_type,
                        script.second_growtype[first_grow_type]
                            .fitness[second_grow_type - 1]);
                    skill->set_second_growtype_max_level(
                        first_grow_type, second_grow_type,
                        script.second_growtype[first_grow_type]
                            .maximum_level[second_grow_type - 1]);
                }
            }
            for (size_t index = 0;
                 index != script.skill_fitness_second_growtype.size() &&
                 index != 2u; ++index) {
                skill->set_fitness_second_growtype(
                    static_cast<int>(index) + 1,
                    script.skill_fitness_second_growtype[index]);
            }
            for (size_t index = 0; index != script.purchase_cost.size();
                 ++index) {
                skill->add_need_sp(script.purchase_cost[index]);
            }
            for (size_t index = 0;
                 index != script.special_purchase_cost.size(); ++index) {
                skill->add_need_sfp(script.special_purchase_cost[index]);
            }
            for (size_t index = 0;
                 index != script.pre_required_skill_indices.size(); ++index) {
                _pre_skill prerequisite = {
                    script.pre_required_skill_indices[index],
                    script.pre_required_skill_levels.at(index), 0};
                skill->add_learned_skill(prerequisite);
            }
            skill->guild_need_gsp_ = script.purchase_gsp;
            skill->guild_need_gold_ = script.purchase_gold;
            skill->guild_need_level_base_ = script.required_guild_level;
            skill->guild_need_level_step_ = script.required_guild_level_range;
            skill->maximum_level_ = script.maximum_level;
            skill->level_values_.clear();
            for (size_t row = 0; row != script.cooltime_level_info.size();
                 ++row) {
                std::vector<int> level_values;
                const std::vector<float>& source_values =
                    script.cooltime_level_info[row];
                level_values.reserve(source_values.size());
                for (size_t column = 0; column != source_values.size();
                     ++column) {
                    level_values.push_back(
                        static_cast<int>(source_values[column]));
                }
                skill->level_values_.push_back(level_values);
            }
            skill->special_skill_ = script.feature_skill_type ? 1 : 0;
            skill->key_commands_ = script.command.key_commands;
            skill->command_customizing_ =
                script.command_customizing ? 1 : 0;
            skill->usable_passiveobject_ids_ =
                script.usable_passiveobject_ids;
            skill->ignore_active_status_ =
                script.ignore_active_status ? 1 : 0;
            skill->ignored_active_status_states_ =
                script.ignored_active_status_states;
            skill->setSkillName(script.name.c_str());
            skill->required_rank_ = script.required_rank;
            insert_skill(job_type, skill);
        }
    }

    for (int job_type = 0;
         job_type < oldsrv::config::kCharacterJobCount;
         ++job_type) {
        for (__gnu_cxx::hash_map<int, CSkill*>::iterator entry =
                 skill_by_index_[job_type].begin();
             entry != skill_by_index_[job_type].end(); ++entry) {
            CSkill* const skill = entry->second;
            skill->set_prelearned_skill();
            set_post_learning_skill(job_type, skill);
        }
    }
    return 1;
}

// IDA_EA: 0x83519A6
// IDA_NAME: _ZN10CSkillList12insert_skillEiP6CSkill
// DEMANGLED: CSkillList::insert_skill(int,CSkill *)
int CSkillList::insert_skill(int job_type, CSkill* skill) {
    if (!skill || job_type < 0 ||
        job_type >= oldsrv::config::kCharacterJobCount) {
        return 0;
    }

    const int skill_index = skill->get_index();
    if (skill_by_index_[job_type].find(skill_index) !=
        skill_by_index_[job_type].end()) {
        return 0;
    }

    skill_by_index_[job_type].insert(std::make_pair(skill_index, skill));
    skill_by_group_[job_type].insert(
        std::make_pair(skill->get_group(), skill));
    return 1;
}

// IDA_EA: 0x8351B74
// IDA_NAME: _ZN10CSkillList10find_skillEii
// DEMANGLED: CSkillList::find_skill(int,int)
CSkill* CSkillList::find_skill(int job_type, int skill_index) {
    if (job_type < 0 || job_type >= oldsrv::config::kCharacterJobCount) {
        return 0;
    }

    __gnu_cxx::hash_map<int, CSkill*>::iterator it =
        skill_by_index_[job_type].find(skill_index);
    if (it == skill_by_index_[job_type].end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8351C1E
// IDA_NAME: _ZN10CSkillList24get_ran_groupskill_indexEiii
// DEMANGLED: CSkillList::get_ran_groupskill_index(int,int,int)
int CSkillList::get_ran_groupskill_index(int job_type,
                                         int group,
                                         int exclude_skill_index) {
    if (job_type < 0 || job_type >= oldsrv::config::kCharacterJobCount) {
        return -1;
    }

    typedef std::multimap<int, CSkill*>::iterator Iterator;
    std::pair<Iterator, Iterator> range =
        skill_by_group_[job_type].equal_range(group);
    if (range.first == range.second) {
        return -1;
    }

    for (Iterator it = range.first; it != range.second; ++it) {
        if (it->second && it->second->get_index() != exclude_skill_index) {
            return it->second->get_index();
        }
    }
    return range.first->second->get_index();
}

// IDA_EA: 0x8351E18
// IDA_NAME: _ZN10CSkillList23set_post_learning_skillEiP6CSkill
// DEMANGLED: CSkillList::set_post_learning_skill(int,CSkill *)
void CSkillList::set_post_learning_skill(int job_type, CSkill* skill) {
    if (job_type < 0 || job_type >= oldsrv::config::kCharacterJobCount ||
        !skill) {
        return;
    }

    for (std::list<_pre_skill>::const_iterator prerequisite =
             skill->learned_skills_.begin();
         prerequisite != skill->learned_skills_.end(); ++prerequisite) {
        const std::pair<int, int> dependent(skill->get_index(),
                                            prerequisite->level);
        std::map<int, std::vector<std::pair<int, int> > >::iterator found =
            post_learning_skill_[job_type].find(prerequisite->skill_index);
        if (found == post_learning_skill_[job_type].end()) {
            std::vector<std::pair<int, int> > dependents;
            dependents.push_back(dependent);
            post_learning_skill_[job_type].insert(
                std::make_pair(prerequisite->skill_index, dependents));
        } else {
            found->second.push_back(dependent);
        }
    }
}

// IDA_EA: 0x8352028
// IDA_NAME: _ZN10CSkillList23get_post_learning_skillEiiRSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CSkillList::get_post_learning_skill(int,int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int CSkillList::get_post_learning_skill(
    int job_type,
    int skill_index,
    std::vector<std::pair<int, int> >& skills) {
    if (job_type < 0 || job_type >= oldsrv::config::kCharacterJobCount) {
        return 0;
    }

    std::map<int, std::vector<std::pair<int, int> > >::iterator it =
        post_learning_skill_[job_type].find(skill_index);
    if (it == post_learning_skill_[job_type].end()) {
        return 0;
    }
    skills = it->second;
    return 1;
}

// IDA_EA: 0x8374C58
// IDA_NAME: _ZN10CSkillList12getSkillListEi
// DEMANGLED: CSkillList::getSkillList(int)
__gnu_cxx::hash_map<int, CSkill*>* CSkillList::getSkillList(int job_type) {
    if (job_type >= 0 && job_type < oldsrv::config::kCharacterJobCount) {
        return &skill_by_index_[job_type];
    }
    return 0;
}

// IDA_EA: 0x83538F0
// IDA_NAME: _ZN9QuestListC2Ev
// DEMANGLED: QuestList::QuestList(void)
QuestList::QuestList()
    : quest_by_index_(),
      selector_0_(),
      selector_1_(),
      selector_2_(),
      selector_3_(),
      selector_4_(),
      selector_5_(),
      selector_6_(),
      selector_7_(),
      mail_quest_ids_(),
      quest_npc_ids_() {}

// IDA_EA: 0x8353AA0
// IDA_NAME: _ZN9QuestListD2Ev
// DEMANGLED: QuestList::~QuestList()
QuestList::~QuestList() {
    destroy();
}

// IDA_EA: 0x8353C86
// IDA_NAME: _ZN9QuestList7destroyEv
// DEMANGLED: QuestList::destroy(void)
int QuestList::destroy() {
    npc_item_drop::ClearQuestContracts();
    for (__gnu_cxx::hash_map<int, Quest*>::iterator it =
             quest_by_index_.begin();
         it != quest_by_index_.end(); ++it) {
        Quest* quest = it->second;
        if (quest) {
            quest->~Quest();
            ::operator delete(quest);
        }
    }
    quest_by_index_.clear();
    selector_0_.clear();
    selector_1_.clear();
    selector_2_.clear();
    selector_3_.clear();
    selector_4_.clear();
    selector_5_.clear();
    selector_6_.clear();
    selector_7_.clear();
    mail_quest_ids_.clear();
    quest_npc_ids_.clear();
    return 0;
}

// IDA_EA: 0x8353D98
// IDA_NAME: _ZN9QuestList9load_listEv
// DEMANGLED: QuestList::load_list(void)
int QuestList::load_list() {
    // load_list may be retried after a partial PVF failure without an
    // intervening destroy(), so the sidecar follows this load transaction.
    npc_item_drop::ClearQuestContracts();
    QuestScript script;
    sync_script::CSyncScript* const sync =
        sync_script::GetInstanceSyncScript();
    sync->truncate_quest_category();

    for (STScriptFileList::const_iterator file =
             g_questScriptFileList.begin();
         file != g_questScriptFileList.end(); ++file) {
        const char* const filename = file->second.c_str();
        if (!filename || importQuestScript(&script, filename) != 1) {
            LogManager::logFormat(1, "data_manager.cpp",
                                  "bool QuestList::load_list()", 0x1357,
                                  "Quest Script Error: %s\n", filename);
            return 0;
        }

        if (!script.schedules.empty()) {
            GlobalData::s_DailyScheduleManager->AddDailSchedule(
                script.schedules);
        }

        for (std::vector<MonsterKillItem>::const_iterator kill_item =
                 script.monster_kill_items.begin();
             kill_item != script.monster_kill_items.end(); ++kill_item) {
            // MonsterKillItem +0x10 is the quest-item ID consumed by the
            // original CDataManager::find_item call at 0x8353D98.
            CItem* const item = G_CDataManager()->find_item(
                kill_item->map_index);
            if (!item) {
                LogManager::logFormat(
                    1, "data_manager.cpp", "bool QuestList::load_list()",
                    0x136F,
                    "Quest Script Error: %s - Monster Kill Item %d is not Exist\n",
                    filename, kill_item->map_index);
                return 0;
            }
            if (item->is_stackable() != 1 ||
                item->vtable_->get_item_type(item) != 8) {
                LogManager::logFormat(
                    1, "data_manager.cpp", "bool QuestList::load_list()",
                    0x1375,
                    "Quest Script Error: %s - Monster Kill Item %d is not Quest Item\n",
                    filename, kill_item->map_index);
                return 0;
            }
        }

        const int quest_data_type = script.value_178;
        const int quest_data_subtype = script.value_17c;
        const std::vector<int>& quest_data = script.limit_dungeon_indices;
        if (quest_data_type == 2 || quest_data_type == 6) {
            // CLIENT_IDA_EVIDENCE: sub_B206F0 @ 0xB207A8 switches to the
            // 0828 five-value regional [hunt monster] schema only when the
            // first value is the -2 sentinel:
            // sentinel, world-map area, minimum difficulty, selector, count.
            // Legacy type-2 records remain four integers wide.
            const bool extended_hunt_monster =
                quest_data_type == 2 && !quest_data.empty() &&
                quest_data.front() == -2 &&
                quest_data.size() % 5u == 0u;
            const unsigned int group_size =
                quest_data_type == 6 || extended_hunt_monster ? 5u : 4u;
            if (quest_data.size() % group_size != 0u ||
                quest_data.size() / group_size > 3u) {
                LogManager::logFormat(
                    1, "data_manager.cpp", "bool QuestList::load_list()",
                    0x1383,
                    "Quest Script Error: %s - Quest Data Type is %d, But Argumemt Size is Wrong(%d)\n",
                    filename, quest_data_type,
                    static_cast<int>(quest_data.size()));
                return 0;
            }
            for (std::vector<int>::const_iterator value =
                     quest_data.begin();
                 value != quest_data.end();) {
                ++value;
                ++value;
                ++value;
                if (quest_data_type == 6) {
                    const unsigned int enemy_type =
                        static_cast<unsigned int>(*value++);
                    if (enemy_type > 10u ||
                        ((1u << enemy_type) & 0x40Eu) == 0u) {
                        LogManager::logFormat(
                            1, "data_manager.cpp",
                            "bool QuestList::load_list()", 0x139D,
                            "Quest Script Error: %s - Unknown enemy type(%d)\n",
                            filename, enemy_type);
                        return 0;
                    }
                } else if (extended_hunt_monster) {
                    const int selector = *value++;
                    if (selector >= 0) {
                        LogManager::logFormat(
                            1, "data_manager.cpp",
                            "bool QuestList::load_list()", 0x139D,
                            "Quest Script Error: %s - Unknown monster selector(%d)\n",
                            filename, selector);
                        return 0;
                    }
                }
                const unsigned int trigger_count =
                    static_cast<unsigned int>(*value++);
                if (trigger_count > 511u) {
                    LogManager::logFormat(
                        1, "data_manager.cpp", "bool QuestList::load_list()",
                        0x13A4,
                        "Quest Script Error: %s - Quest Data Type is %d, But Trigger Size is Wrong(%d)\n",
                        filename, quest_data_type, trigger_count);
                    return 0;
                }
            }
        } else if (quest_data_type == 1) {
            unsigned int required_size = 3u;
            bool valid_variable_size = false;
            if (quest_data_subtype == 4) {
                required_size = 2u;
            } else if (quest_data_subtype == 6) {
                if (quest_data.size() == 2u || quest_data.size() == 3u) {
                    required_size = static_cast<unsigned int>(quest_data.size());
                } else {
                    required_size = 0u;
                }
            } else if (quest_data_subtype == 11) {
                // 0828 urgent-area quests prepend a negative area selector
                // to the legacy three-field condition-under-clear record.
                valid_variable_size =
                    quest_data.size() == 3u ||
                    (quest_data.size() == 4u && quest_data[0] < 0);
            } else if (quest_data_subtype >= 12) {
                // Both three-field and four-field records are present in the
                // 0828 quest table.  Preserve their source shape.
                valid_variable_size =
                    quest_data.size() == 3u || quest_data.size() == 4u;
            }
            if (!valid_variable_size && quest_data.size() != required_size) {
                LogManager::logFormat(
                    1, "data_manager.cpp", "bool QuestList::load_list()",
                    0x13C4,
                    "Quest Script Error: %s - Quest Data Type is %d, But Argumemt Size is Wrong(%d)\n",
                    filename, quest_data_type,
                    static_cast<int>(quest_data.size()));
                return 0;
            }
        } else if (quest_data_type == 0 &&
                   (quest_data.size() & 1u) != 0u) {
            LogManager::logFormat(
                1, "data_manager.cpp", "bool QuestList::load_list()", 0x13BA,
                "Quest Script Error: %s - Quest Data Type is %d, But Argumemt Size is Wrong(%d)\n",
                filename, quest_data_type,
                static_cast<int>(quest_data.size()));
            return 0;
        }

        // QuestScript +0x150 is the dungeon association.  The original
        // stack locals skip +0x154 and pass +0x158 (the third [appear map]
        // integer) to find_map; +0x154 is used later by Quest display logic.
        if (script.appear_map.map_id != -1) {
            CMap* const map = G_CDataManager()->find_map(script.appear_map.y);
            if (!map) {
                LogManager::logFormat(
                    1, "data_manager.cpp", "bool QuestList::load_list()",
                    0x13E9,
                    "Quest Script Error: %s - NPC wrong npc appear map index %d.",
                    filename, script.appear_map.y);
                return 0;
            }
            if (G_CDataManager()->VerifyMap(*map, script.appear_map.map_id) !=
                1) {
                return 0;
            }
        }

        if (sync->insert_quest_category(&script) != true) {
            printf("insert_quest_category Error: %s\n", filename);
            return 0;
        }

        Quest* const quest = new (std::nothrow) Quest();
        if (!quest) {
            cMyTrace trace("bool QuestList::load_list()", 0x13FF, 5);
            trace("[%s][%d]", "bool QuestList::load_list()");
            return 0;
        }
        quest->set_quest(script);
        insert_NPC(script.value_0e4);
        quest_by_index_.insert(std::make_pair(script.quest_index, quest));
        npc_item_drop::RegisterQuestContract(script);

        std::multimap<int, int>* selector = 0;
        switch (script.quest_grade) {
            case 0:
                selector = &selector_0_;
                break;
            case 1:
                selector = &selector_3_;
                break;
            case 2:
                selector = &selector_4_;
                break;
            case 3:
                selector = &selector_7_;
                break;
            case 4:
                selector = &selector_2_;
                break;
            case 5:
                selector = &selector_1_;
                break;
            case 6:
                selector = &selector_5_;
                break;
            case 8:
                selector = &selector_6_;
                break;
            default:
                break;
        }
        if (selector) {
            selector->insert(std::make_pair(script.value_0e4,
                                             script.quest_index));
        }
        if (script.mail_time != -1) {
            mail_quest_ids_.push_back(script.quest_index);
        }
    }

    for (std::vector<int>::const_iterator quest_index =
             mail_quest_ids_.begin();
         quest_index != mail_quest_ids_.end(); ++quest_index) {
        Quest* const quest = find_quest(*quest_index);
        if (!quest) {
            continue;
        }
        if (quest->mail_time == -1) {
            LogManager::logFormat(1, "data_manager.cpp",
                                  "bool QuestList::load_list()", 0x145C,
                                  "Mail Quest Error (quest index : %d)",
                                  *quest_index);
            return 0;
        }
        if (quest->mail_title.empty()) {
            LogManager::logFormat(
                1, "data_manager.cpp", "bool QuestList::load_list()", 0x1462,
                "Mail Quest Error! mail title err(quest index : %d)",
                *quest_index);
            return 0;
        }
        if (quest->mail_contents.empty()) {
            LogManager::logFormat(
                1, "data_manager.cpp", "bool QuestList::load_list()", 0x1467,
                "Mail Quest Error! mail contents err(quest index : %d)",
                *quest_index);
            return 0;
        }
        if ((quest->mail_enclose.size() & 1u) != 0u) {
            LogManager::logFormat(
                1, "data_manager.cpp", "bool QuestList::load_list()", 0x146C,
                "Mail Quest Error! mail enclose item err(quest index : %d)",
                *quest_index);
            return 0;
        }
    }

    // CQuestClear::syncScript @0x808BB34 is a five-byte original body which
    // unconditionally returns one and reads no state; the original final
    // branch therefore always returns success after the preceding checks.
    // Maps are already loaded at this point.  Resolve only maps associated
    // with a trigger-18 dungeon, keeping all RDAR action scans in the
    // single-threaded data-load phase rather than in CMD588 runtime.
    npc_item_drop::ResolveMapContracts();
    return 1;
}

// IDA_EA: 0x8354F02
// IDA_NAME: _ZN9QuestList10insert_NPCEi
// DEMANGLED: QuestList::insert_NPC(int)
char QuestList::insert_NPC(int npc_id) {
    if (!npc_id) {
        return 0;
    }
    if (std::find(quest_npc_ids_.begin(), quest_npc_ids_.end(), npc_id) !=
        quest_npc_ids_.end()) {
        return 1;
    }
    quest_npc_ids_.push_back(npc_id);
    return 1;
}

// IDA_EA: 0x8355A5C
// IDA_NAME: _ZN9QuestList10find_questEi
// DEMANGLED: QuestList::find_quest(int)
Quest* QuestList::find_quest(int quest_index) {
    __gnu_cxx::hash_map<int, Quest*>::iterator it =
        quest_by_index_.find(quest_index);
    if (it == quest_by_index_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x8354FC8
// IDA_NAME: _ZN9QuestList17check_ahead_questEiRKN8WongWork11CQuestClearE
// DEMANGLED: QuestList::check_ahead_quest(int,WongWork::CQuestClear const&)
int QuestList::check_ahead_quest(
    int quest_index,
    const WongWork::CQuestClear& cleared_quests) {
    Quest* const quest = find_quest(quest_index);
    if (!quest) {
        LogManager::logFormat(
            1, "data_manager.cpp",
            "bool QuestList::check_ahead_quest(int, const WongWork::CQuestClear&)",
            0x14B7, "find_quest(%d)", quest_index);
        return 0;
    }

    if (quest->conditions.empty()) {
        return 1;
    }

    for (std::vector<std::vector<int> >::const_iterator condition =
             quest->conditions.begin();
         condition != quest->conditions.end(); ++condition) {
        bool prerequisite_missing = false;
        for (std::vector<int>::const_iterator required_quest =
                 condition->begin();
             required_quest != condition->end(); ++required_quest) {
            if (cleared_quests.isClearedQuest(
                    static_cast<unsigned int>(*required_quest)) != 1) {
                prerequisite_missing = true;
                break;
            }
        }
        if (!prerequisite_missing) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x835511E
// IDA_NAME: _ZN9QuestList16check_anti_questEiRKN8WongWork11CQuestClearE
// DEMANGLED: QuestList::check_anti_quest(int,WongWork::CQuestClear const&)
int QuestList::check_anti_quest(
    int quest_index,
    const WongWork::CQuestClear& cleared_quests) {
    Quest* const quest = find_quest(quest_index);
    if (!quest) {
        LogManager::logFormat(
            1, "data_manager.cpp",
            "bool QuestList::check_anti_quest(int, const WongWork::CQuestClear&)",
            0x150A, "find_quest(%d)", quest_index);
        return 0;
    }

    for (std::vector<int>::const_iterator anti_quest =
             quest->condition_values.begin();
         anti_quest != quest->condition_values.end(); ++anti_quest) {
        const int required_index = *anti_quest;
        if (required_index > 29999) {
            LogManager::logFormat(
                1, "data_manager.cpp",
                "bool QuestList::check_anti_quest(int, const WongWork::CQuestClear&)",
                5402, "[%s][%d]",
                "bool QuestList::check_anti_quest(int, const WongWork::CQuestClear&)",
                5402);
            return 0;
        }
        if (cleared_quests.isClearedQuest(
                static_cast<unsigned int>(required_index)) == 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8355284
// IDA_NAME: _ZN9QuestList11check_clearEiRKN8WongWork11CQuestClearE
// DEMANGLED: QuestList::check_clear(int,WongWork::CQuestClear const&)
int QuestList::check_clear(
    int quest_index,
    const WongWork::CQuestClear& cleared_quests) {
    if (quest_index <= 29999) {
        return cleared_quests.isClearedQuest(
                   static_cast<unsigned int>(quest_index)) == 0;
    }
    LogManager::logFormat(
        1, "data_manager.cpp",
        "bool QuestList::check_clear(int, const WongWork::CQuestClear&)",
        5423, "[%s][%d]",
        "bool QuestList::check_clear(int, const WongWork::CQuestClear&)",
        5423);
    return 0;
}

// IDA_EA: 0x8355304
// IDA_NAME: _ZN9QuestList17select_mail_questERSt4listIiSaIiEERKN8WongWork11CQuestClearERK18stSelectQuestParam
// DEMANGLED: QuestList::select_mail_quest(
//             std::list<int>&,WongWork::CQuestClear const&,
//             stSelectQuestParam const&)
bool QuestList::select_mail_quest(
    std::list<int>& selected_quests,
    const WongWork::CQuestClear& cleared_quests,
    const stSelectQuestParam& parameter) {
    selected_quests.clear();

    for (std::vector<int>::const_iterator quest_index =
             mail_quest_ids_.begin();
         quest_index != mail_quest_ids_.end();
         ++quest_index) {
        Quest* const quest = find_quest(*quest_index);
        if (quest &&
            quest->check_possible(parameter) &&
            check_clear(*quest_index, cleared_quests) &&
            check_ahead_quest(*quest_index, cleared_quests) == 1 &&
            check_anti_quest(*quest_index, cleared_quests) == 1) {
            selected_quests.push_back(*quest_index);
        }
    }
    return !selected_quests.empty();
}

// IDA_EA: 0x8355474
// IDA_NAME: _ZN9QuestList11Check_QuestEP5QuestR18stSelectQuestParamRKN8WongWork11CQuestClearE
// DEMANGLED: QuestList::Check_Quest(Quest *,stSelectQuestParam &,WongWork::CQuestClear const&)
int QuestList::Check_Quest(
    Quest* quest,
    stSelectQuestParam& parameter,
    const WongWork::CQuestClear& cleared_quests) {
    if (quest->exposeQuest() != 1 || !quest->check_possible(parameter) ||
        check_ahead_quest(static_cast<int>(parameter.urgent_quest_index),
                          cleared_quests) != 1 ||
        check_anti_quest(static_cast<int>(parameter.urgent_quest_index),
                         cleared_quests) != 1) {
        return 0;
    }
    return quest->check_power_side(static_cast<char>(parameter.power_side)) ==
           1;
}

// IDA_EA: 0x835556C
// IDA_NAME: _ZN9QuestList26allowable_questlist_as_npcERSt4listIiSaIiEERKSt8multimapIiiSt4lessIiESaISt4pairIKiiEEER18stSelectQuestParamRKN8WongWork11CQuestClearEb
// DEMANGLED: QuestList::allowable_questlist_as_npc(std::list<int,std::allocator<int>> &,std::multimap<int,int,std::less<int>,std::allocator<std::pair<int const,int>>> const&,stSelectQuestParam &,WongWork::CQuestClear const&,bool)
bool QuestList::allowable_questlist_as_npc(
    std::list<int>& selected_quests,
    const std::multimap<int, int>& quest_npcs,
    stSelectQuestParam& parameter,
    const WongWork::CQuestClear& cleared_quests,
    bool repeatable_quest) {
    for (std::list<int>::const_iterator npc = quest_npc_ids_.begin();
         npc != quest_npc_ids_.end(); ++npc) {
        const std::pair<std::multimap<int, int>::const_iterator,
                        std::multimap<int, int>::const_iterator> range =
            quest_npcs.equal_range(*npc);
        for (std::multimap<int, int>::const_iterator candidate = range.first;
             candidate != range.second; ++candidate) {
            parameter.urgent_quest_index =
                static_cast<uint32_t>(candidate->second);
            Quest* const quest = find_quest(candidate->second);
            if (quest && Check_Quest(quest, parameter, cleared_quests) &&
                (repeatable_quest ||
                 check_clear(candidate->second, cleared_quests))) {
                selected_quests.push_back(candidate->second);
            }
        }
    }
    // In the original body this is the final list-iterator comparison after
    // the loop, therefore it is always false once all NPC entries are seen.
    return false;
}

// IDA_EA: 0x8355726
// IDA_NAME: _ZN9QuestList12select_questERSt4listIiSaIiEER18stSelectQuestParamRKN8WongWork11CQuestClearE
// DEMANGLED: QuestList::select_quest(std::list<int,std::allocator<int>> &,stSelectQuestParam &,WongWork::CQuestClear const&)
bool QuestList::select_quest(
    std::list<int>& selected_quests,
    stSelectQuestParam& parameter,
    const WongWork::CQuestClear& cleared_quests) {
    selected_quests.clear();
    allowable_questlist_as_npc(selected_quests, selector_0_, parameter,
                               cleared_quests, false);
    allowable_questlist_as_npc(selected_quests, selector_1_, parameter,
                               cleared_quests, false);
    allowable_questlist_as_npc(selected_quests, selector_2_, parameter,
                               cleared_quests, true);
    allowable_questlist_as_npc(selected_quests, selector_7_, parameter,
                               cleared_quests, false);
    allowable_questlist_as_npc(selected_quests, selector_3_, parameter,
                               cleared_quests, false);
    allowable_questlist_as_npc(selected_quests, selector_4_, parameter,
                               cleared_quests, false);
    return allowable_questlist_as_npc(selected_quests, selector_5_, parameter,
                                      cleared_quests, false);
}

// IDA_EA: 0x835589E
// IDA_NAME: _ZN9QuestList17GetDailyQuestListERSt4listIiSaIiEE
// DEMANGLED: QuestList::GetDailyQuestList(std::list<int>&)
bool QuestList::GetDailyQuestList(std::list<int>& daily_quests) {
    for (std::list<int>::const_iterator npc = quest_npc_ids_.begin();
         npc != quest_npc_ids_.end(); ++npc) {
        const std::pair<std::multimap<int, int>::const_iterator,
                        std::multimap<int, int>::const_iterator> range =
            selector_7_.equal_range(*npc);
        for (std::multimap<int, int>::const_iterator candidate = range.first;
             candidate != range.second; ++candidate) {
            Quest* const quest = find_quest(candidate->second);
            if (quest && quest->quest_grade == 3) {
                daily_quests.push_back(candidate->second);
            }
        }
    }
    // The original return register is the final list-iterator comparison.
    return false;
}

// IDA_EA: 0x8355A10
// IDA_NAME: _ZN9QuestList20GetTrainingQuestListERSt6vectorIiSaIiEEs
// DEMANGLED: QuestList::GetTrainingQuestList(std::vector<int>&,short)
std::vector<int>* QuestList::GetTrainingQuestList(
    std::vector<int>& training_quests,
    int16_t charac_level) {
    if (charac_level >= oldsrv::config::kCharacterLevelMinimum &&
        charac_level <= oldsrv::config::kTrainingQuestLevelMaximum) {
        training_quests = *G_CDataManager()->getDailyTrainingQuest(
            static_cast<int>(charac_level));
    }
    return &training_quests;
}

// IDA_EA: 0x8355AD8
// IDA_NAME: _ZN9QuestList15getQuestNPCListE16ENUM_QUEST_GRADE
// DEMANGLED: QuestList::getQuestNPCList(ENUM_QUEST_GRADE)
std::multimap<int, int>* QuestList::getQuestNPCList(ENUM_QUEST_GRADE grade) {
    switch (grade) {
        case 0:
            return &selector_0_;
        case 1:
            return &selector_3_;
        case 2:
            return &selector_4_;
        case 4:
            return &selector_2_;
        case 5:
            return &selector_1_;
        case 6:
            return &selector_5_;
        case 8:
            return &selector_6_;
        default:
            return 0;
    }
}

// IDA_EA: 0x8374C7A
// IDA_NAME: _ZN9QuestList12getQuestListEv
// DEMANGLED: QuestList::getQuestList(void)
__gnu_cxx::hash_map<int, Quest*>* QuestList::getQuestList() {
    return &quest_by_index_;
}

Dispatcher_EventNpcDropItem::Dispatcher_EventNpcDropItem() {}

Dispatcher_EventNpcDropItem::~Dispatcher_EventNpcDropItem() {}

int Dispatcher_EventNpcDropItem::read(PacketBuf& packet,
                                      MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_EventNpcDropItem::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// 0828 CLIENT_EA 0x4F01C0 sends CMD588 with no body.  The matching receive
// branch at 0x4F01F0 accepts exactly: u8(1), or u8(0)+u8(error 18/19).
int Dispatcher_EventNpcDropItem::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    if (!user || !IsExactEmptyNpcItemDropRequest(packet)) {
        SendNpcItemDropError(user, 19);
        return 0;
    }
    if (user->get_state() != 5) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    CParty* const party = user->GetParty();
    if (!party || party->get_member_count() != 1) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    const int dungeon_index = user->getPlayingDungeonIndex();
    if (dungeon_index < 0 ||
        party->battle_field().get_dungeon_index() != dungeon_index) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    MapInfo* const current_map = party->battle_field().GetCurrentMapInfo();
    UserQuest* const user_quest = user->getCurCharacQuestR();
    if (!current_map || !user_quest) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    NpcItemDropQuestContract contract;
    std::vector<NpcItemDropActionPoint> action_points;
    int matched_quest_index = 0;
    bool ambiguous_quest = false;
    // This guard is the CMD588 transaction boundary, not merely a registry
    // read lock.  It remains held through replay checks, selection and
    // Add_Item/NOTI40 so concurrent body-less requests cannot both observe an
    // empty map and create two outcomes.
    MutexGuard transaction_guard(&g_npc_item_drop_mutex);
    for (int slot = 0; slot != 20; ++slot) {
        const int quest_index =
            static_cast<int>(user_quest->play_quest[slot]);
        if (quest_index == 0 || quest_index == matched_quest_index) {
            continue;
        }
        const NpcItemDropQuestMap::const_iterator registered =
            g_npc_item_drop_quests.find(quest_index);
        if (registered == g_npc_item_drop_quests.end() ||
            !ContractMatchesDungeon(registered->second,
                                    dungeon_index)) {
            continue;
        }
        Quest* const quest = G_CDataManager()->find_quest(quest_index);
        if (!quest || quest->trigger_type != 18) {
            continue;
        }
        if (matched_quest_index != 0) {
            ambiguous_quest = true;
            break;
        }
        matched_quest_index = quest_index;
        contract = registered->second;
    }

    const NpcItemDropMapActionMap::const_iterator map_actions =
        g_npc_item_drop_map_actions.find(current_map->map_index_);
    if (map_actions != g_npc_item_drop_map_actions.end()) {
        action_points = map_actions->second;
    }

    if (matched_quest_index == 0 || ambiguous_quest) {
        SendNpcItemDropError(user, 19);
        return 0;
    }
    if (action_points.empty()) {
        SendNpcItemDropError(user, 18);
        return 0;
    }
    // A body-less request cannot identify which object fired.  Refuse maps
    // with multiple eligible actions instead of guessing a producer.
    if (action_points.size() != 1u) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    std::vector<unsigned int> eligible_items;
    BuildEligibleWeaponPool(contract, user, eligible_items);
    if (eligible_items.empty()) {
        SendNpcItemDropError(user, 18);
        return 0;
    }

    // Session-local replay closure: the exact active quest/job pool may have
    // at most one outcome, whether it is still on this map or was collected.
    if (UserOwnsAnyCandidate(user, eligible_items) ||
        MapContainsAnyCandidate(*current_map, eligible_items)) {
        SendNpcItemDropError(user, 19);
        return 0;
    }

    const unsigned int selected_item =
        eligible_items[static_cast<size_t>(get_rand_int(
            static_cast<int>(eligible_items.size())))];
    const NpcItemDropActionPoint& action = action_points.front();
    if (action.position_x < std::numeric_limits<short>::min() ||
        action.position_x > std::numeric_limits<short>::max() ||
        action.position_y < std::numeric_limits<short>::min() ||
        action.position_y > std::numeric_limits<short>::max()) {
        SendNpcItemDropError(user, 18);
        return 0;
    }
    const int drop_result = party->drop_item_generated(
        user, selected_item, action.position_x, action.position_y);
    if (drop_result != 0) {
        SendNpcItemDropError(user,
                             static_cast<unsigned char>(
                                 drop_result == 19 ? 19 : 18));
        return 0;
    }

    // CParty::drop_item_generated emits NOTI40 first; acknowledge CMD588 only
    // the authoritative map item and its broadcast exist.
    user->SendCmdOkPacket(ENUM_CMDPACKET_EVENT_NPC_DROP_ITEM_);
    return 0;
}
