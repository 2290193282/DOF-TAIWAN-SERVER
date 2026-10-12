#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_SCRIPT_GROWTH_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_SCRIPT_GROWTH_DATA_H_

#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_game/player/core/character_misc_data.h"
#include "old_game/player/core/character_status_data.h"


// IDA unit: 04_character_select_create/STCharacterScript/ActiveActionFileSet.
// Old i386 size: 0x40.
struct STCharacterScriptActiveActionFileSet {
    std::string waiting_action_;
    std::string move_action_;
    std::string dash_action_;
    std::string sit_action_;
    std::string damage_action_1_;
    std::string damage_action_2_;
    std::string down_action_;
    std::string overturn_action_;
    std::string jump_action_;
    std::string jumpattack_action_;
    std::vector<std::string> attack_actions_;
    std::vector<std::string> etc_actions_;
};

// IDA unit: 04_character_select_create/STCharacterScript/WeaponInfo.
// Old i386 size: 0x34.
struct STCharacterScriptWeaponInfo {
    std::vector<std::string> attack_files_;
    int hit_type_;
    uint8_t blood_effect_;
    uint8_t alignment_padding_11_[3];
    int hit_count_;
    float hit_value_18_;
    float hit_value_1c_;
    float hit_value_20_;
    float skill_values_[4];
};

// IDA unit: 04_character_select_create/STCharacterScript/_ComboInfo.
// Old i386 size: 0x3c.
struct STCharacterScriptComboInfo {
    std::map<int, int> first_;
    std::map<int, int> second_;
    std::vector<int> values_;
};

// IDA unit: 04_character_select_create/STCharacterScript. Old i386 size:
// 0xc1c. Every non-trivial subobject below is constructed and destroyed by
// the original C2/D2; fields deliberately omitted from Clear() must retain
// their original persistence across imports.
struct STCharacterScript {
    STCharacterScript();
    ~STCharacterScript();

    int Clear();

    static int naExpTable[200];
    static int pvpPrices_[200];

    struct STSecondGrowTypeData {
        STSecondGrowTypeData();
        ~STSecondGrowTypeData();

        bool clear();

        STRDCharacterStatus status[2];
        std::vector<std::pair<int, int> > skill_values[2];
        CharacterEtcStats etc_stats[2];
    };

    int character_job_;
    std::string growtype_names_[6];
    int width_;
    int height_;
    STRDCharacterStatus growtype_status_[7];
    uint8_t alignment_padding_285_[3];
    std::vector<std::pair<int, int> > give_skills_[7];
    CharacterEtcStats etc_stats_[7];
    std::string awakening_names_[6][2];
    STSecondGrowTypeData second_grow_type_data_[7];

    int jump_speed_;
    std::string waiting_motion_;
    std::string move_motion_;
    std::string sit_motion_;
    std::string damage_motion_1_;
    std::string damage_motion_2_;
    std::string down_motion_;
    std::string overturn_motion_;
    std::string jump_motion_;
    std::string jumpattack_motion_;
    std::string rest_motion_;
    std::string simple_rest_motion_;
    std::string simple_move_motion_;
    std::string throw_motion_1_1_;
    std::string throw_motion_2_1_;
    std::string throw_motion_3_1_;
    std::string throw_motion_4_1_;
    std::string throw_motion_1_2_;
    std::string throw_motion_2_2_;
    std::string throw_motion_3_2_;
    std::string throw_motion_4_2_;
    std::string dash_motion_;
    std::string dashattack_motion_;
    std::string getitem_motion_;
    std::string buff_motion_;
    std::string back_motion_;
    std::vector<std::string> ghost_motion_;
    std::vector<std::string> attack_motion_;
    std::vector<std::string> etc_motion_;
    STCharacterScriptActiveActionFileSet active_actions_;
    std::string last_action_;
    std::string proc_action_;
    std::string jumpattack_info_;
    std::string dashattack_info_;
    std::vector<std::string> attack_info_;
    std::vector<std::string> etc_attack_info_;
    std::vector<STCharacterScriptWeaponInfo> weapon_infos_;
    std::string source_full_path_;
    int start_town_;
    float field_bd8_;
    std::string source_filename_;
    STCharacterScriptComboInfo combo_info_;
};


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_SCRIPT_GROWTH_DATA_H_
