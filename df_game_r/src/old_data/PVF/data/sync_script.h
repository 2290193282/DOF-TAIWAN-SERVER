#ifndef OLDSRV_DATAMANAGER_SYNC_SCRIPT_H_
#define OLDSRV_DATAMANAGER_SYNC_SCRIPT_H_

// Canonical B-owned PVF-to-MySQL synchronization declarations.

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/equipment/datamanager/equipment_script.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/n_quest/quest_skill_script.h"

struct STEquipmentPartSetPiece;
struct STMonsterScript;
struct STAICharacterCommonScript;
struct CreatureScript;
struct EquipmentParameterInfo;
struct EquipmentSkillDataUp;
struct ItemAuraInfo;
struct LevelUpSkill;
struct SkillUseItem;
struct STEquipmentScript;
struct STStackableScript;
struct TownScript;

// IDA_EA: 0x861CA1B. Socket categories are serialized into the equipment
// jewel_type DB field as their original one-byte character codes.
int GetSocketTypeCharacter(uint16_t socket_type);

// IDA_EA: 0x861CBB4.
namespace sync_script {

// IDA unit: 12_pvf_datamanager_scripts/sync_script.
class CStringMaker {
public:
    CStringMaker();
    CStringMaker(const CStringMaker& other);
    ~CStringMaker();
    CStringMaker& operator()(int value);
    CStringMaker& operator()(float value);
    CStringMaker& operator()(SkillUseItem value);
    CStringMaker& operator()(EquipmentSkillDataUp value);
    CStringMaker& operator()(LevelUpSkill value);
    CStringMaker& operator()(ItemAuraInfo value);
    CStringMaker& operator()(std::vector<float>& values);
    const char* c_str();

private:
    std::string value_;
};

class CSyncScript {
public:
    struct SWEAPONTYPE {
        SWEAPONTYPE(ENUM_CHARACTERJOB character_job, int weapon_type);
        bool operator<(const SWEAPONTYPE& other) const;

        ENUM_CHARACTERJOB character_job;
        int32_t weapon_type;
    };

    struct SARMORTYPE {
        SARMORTYPE(uint8_t master_type, std::string armor_name);
        SARMORTYPE(const SARMORTYPE& other);
        ~SARMORTYPE();
        bool operator<(const SARMORTYPE& other) const;

        uint8_t master_type;
        char padding_01[3];
        std::string armor_name;
    };

    struct SITEMTYPE {
        SITEMTYPE();
        SITEMTYPE(bool item_type,
                  uint8_t master_type,
                  uint8_t sub_type,
                  uint16_t sub_no,
                  std::string description);
        SITEMTYPE(const SITEMTYPE& other);
        ~SITEMTYPE();
        SITEMTYPE& operator=(const SITEMTYPE& other);

        uint8_t item_type;
        uint8_t master_type;
        uint8_t sub_type;
        char padding_03;
        uint16_t sub_no;
        char padding_06[2];
        std::string description;
    };

    CSyncScript();
    ~CSyncScript();

    CSyncScript* SetEnable(bool enable);
    int IsEnable();
    int GetType(std::string type_name,
                ENUM_CHARACTERJOB character_job,
                int equipment_type,
                int& master_type,
                int& sub_no,
                bool equipment);
    const char* GetUserJob(ENUM_CHARACTERJOB character_job);
    const char* GetUsableCharacter(STEquipmentScript* script);
    const char* GetUsableCharacter(STStackableScript* script);
    const char* GetAttachType(int attach_type);
    const char* GetAttackElement(EquipmentParameterInfo* info);
    const char* GetLevelUpSkill(
        STEquipmentScript* script,
        EquipmentParameterInfo::ENUM_SKILL_UP_VECTOR_INDEX index);
    const char* GetURL(const char* filename, int filename_length, int icon_id);
    int LoadItemInfoMasterScript(const char* filename);
    bool truncate_dnf_item_info_table();
    bool insert_equiptment_item_to_db(STEquipmentScript* script,
                                      unsigned int item_id,
                                      int sync_type,
                                      int db_piece_count);
    bool insert_stackable_item_to_db(STStackableScript* script);
    bool insert_quest_category(QuestScript* script);
    bool truncate_quest_category();
    bool insert_skill_index(STSkillScript* script, int job);
    bool truncate_skill_index();
    bool trancate_skill_info();
    bool insert_skill_info(STSkillScript* script, int job, int module_type);
    bool truncate_exp_level_ref_table();
    bool insert_exp_level_to_db(unsigned int exp, int level);
    bool truncate_charac_quest_ref();
    bool insert_charac_quest_category(unsigned int origin_index,
                                      unsigned int mapped_index);
    int truncateCeraShopItemTable();
    int insertCeraShopItemIntoDB(unsigned int ipg_no,
                                 unsigned int item_no,
                                 int item_count,
                                 int cera_price,
                                 short shop_type,
                                 const char* item_name,
                                 const char* item_comment);
    bool truncate_item_info_master_table();
    int insert_item_info_master_to_db();
    bool insert_one_item_info_master(unsigned char master_no,
                                     unsigned short sub_no,
                                     const char* name,
                                     const char* master_explain);
    bool trancate_item_parts_set();
    bool insert_item_parts_set(int part_set_index,
                               ::STEquipmentPartSetPiece& piece);
    bool truncate_random_option_ref();
    int insert_random_option_ref();
    bool truncate_old_equip_info_table();
    int insert_old_equip_info_to_db();
    bool truncate_charac_action_point_desc();
    bool insert_charac_action_point_desc();
    bool truncate_charac_advance_altar_item_desc();
    bool insert_charac_advance_altar_item_desc();
    bool truncate_mapping_equip_info_table();
    int insert_mapping_equip_info_to_db(std::map<int, int>& mapping);
    bool truncate_monster_info_table();
    int insert_monster_info_to_db(const ::STMonsterScript& script);
    bool truncate_creature_info_table();
    int insert_creature_info_to_db(unsigned int item_id,
                                   const ::CreatureScript* script,
                                   const char* skill_info,
                                   const char* overskill_info);
    bool truncate_aicharacter_info_table();
    int insert_aicharacter_info_to_db(
        int index,
        const ::STAICharacterCommonScript& script);
    bool truncate_artifact_info_table();
    bool insert_artifact_info_to_db(STEquipmentScript* script);
    bool truncate_town_info_table();
    bool insert_town_info_to_db(const TownScript& script);

private:
    uint8_t enable_;
    char padding_01[3];
    std::vector<unsigned char> type_additions_;
    std::vector<unsigned char> reverse_desc_;
    std::map<std::string, SITEMTYPE> item_types_;
    std::map<SWEAPONTYPE, SITEMTYPE> weapon_types_;
    std::map<SARMORTYPE, SITEMTYPE> armor_types_;
    std::map<unsigned char, std::string> master_type_names_;
    std::map<unsigned char, std::string> material_type_names_;
};

CSyncScript* GetInstanceSyncScript();
bool DisplayMaxLength();

static_assert(sizeof(CSyncScript::SWEAPONTYPE) == 0x08u,
              "CSyncScript::SWEAPONTYPE IDA ABI size must stay 0x08");
static_assert(sizeof(CSyncScript::SARMORTYPE) == 0x08u,
              "CSyncScript::SARMORTYPE IDA ABI size must stay 0x08");
static_assert(sizeof(CSyncScript::SITEMTYPE) == 0x0Cu,
              "CSyncScript::SITEMTYPE IDA ABI size must stay 0x0C");
static_assert(sizeof(CSyncScript) == 0x94u,
              "CSyncScript IDA ABI size must stay 0x94");

}  // namespace sync_script

#endif  // OLDSRV_DATAMANAGER_SYNC_SCRIPT_H_
