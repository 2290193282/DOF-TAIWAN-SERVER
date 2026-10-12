#ifndef OLDSRV_GLOBAL_EQUIPMENT_ANI_SCRIPT_H_
#define OLDSRV_GLOBAL_EQUIPMENT_ANI_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

// IDA unit: 06_userinfo_stat_inventory/GrowthEquipmentInfo.
// IDA_EA: GrowthEquipmentInfo @0x89C2FC4/0x89C300C; STEquipmentAniScript
// @0x89C4FBE/0x89CA12C; EquipmentAniInfoScript @0x89C0586/0x89C05DD.
// Public value record used by EquipmentAniInfoScript.
struct GrowthEquipmentInfo {
    GrowthEquipmentInfo();
    GrowthEquipmentInfo(const GrowthEquipmentInfo& other);
    ~GrowthEquipmentInfo();

    GrowthEquipmentInfo& operator=(const GrowthEquipmentInfo& other);
    GrowthEquipmentInfo* clear();

    int32_t item_index;       // +0x00
    int32_t equipment_slot;   // +0x04, default -1
    int32_t required_level;   // +0x08
    int32_t grow_type;        // +0x0C
    int32_t unknown_10;       // +0x10
    std::string ani_name;     // +0x14
};

static_assert(sizeof(GrowthEquipmentInfo) == 0x18u,
              "GrowthEquipmentInfo IDA ABI size must stay 0x18");

// IDA unit: 06_userinfo_stat_inventory/STEquipmentAniScript.
struct STEquipmentAniScript {
    STEquipmentAniScript();
    STEquipmentAniScript(const STEquipmentAniScript& other);
    ~STEquipmentAniScript();

    STEquipmentAniScript& operator=(const STEquipmentAniScript& other);
    std::string* clear();

    int32_t layer;                         // +0x00
    std::string waiting_motion;            // +0x04
    std::string move_motion;               // +0x08
    std::string sit_motion;                // +0x0C
    std::string damage_motion[2];          // +0x10
    std::string down_motion;               // +0x18
    std::string overturn_motion;           // +0x1C
    std::string jump_motion;               // +0x20
    std::string jumpattack_motion;         // +0x24
    std::string rest_motion;               // +0x28
    std::string throw_motion_first[4];     // +0x2C
    std::string throw_motion_second[4];    // +0x3C
    std::string dash_motion;               // +0x4C
    std::string dashattack_motion;         // +0x50
    std::string getitem_motion;            // +0x54
    std::string buff_motion;               // +0x58
    std::string back_motion;               // +0x5C
    std::string reserved_motion;           // +0x60
    std::vector<std::string> attack_motion; // +0x64
    std::vector<std::string> etc_motion;    // +0x70
};

static_assert(sizeof(STEquipmentAniScript) == 0x7Cu,
              "STEquipmentAniScript IDA ABI size must stay 0x7C");

// IDA unit: 06_userinfo_stat_inventory/EquipmentAniInfoScript.
struct EquipmentAniInfoScript {
    EquipmentAniInfoScript();
    EquipmentAniInfoScript(const EquipmentAniInfoScript& other);
    ~EquipmentAniInfoScript();

    int clear();

    std::vector<STEquipmentAniScript> base_ani;      // +0x00
    int32_t unknown_0C;                              // +0x0C, default -1
    int32_t unknown_10;                              // +0x10, default -1
    uint8_t unknown_14;                              // +0x14
    uint8_t pad_15[3];
    std::vector<STEquipmentAniScript> layer_ani;     // +0x18
    int32_t unknown_24;                              // +0x24, default -1
    int32_t unknown_28;                              // +0x28, default 1
    int32_t unknown_2C;                              // +0x2C
    int32_t unknown_30;                              // +0x30, default -1
    int32_t unknown_34;                              // +0x34, default -1
    int32_t unknown_38;                              // +0x38
    int32_t unknown_3C;                              // +0x3C
    std::vector<std::string> string_list;            // +0x40
    std::string unknown_string_4C;                   // +0x4C
    std::vector<GrowthEquipmentInfo> growth_infos;   // +0x50
    std::vector<std::vector<STEquipmentAniScript> > layer_lists; // +0x5C
};

static_assert(sizeof(EquipmentAniInfoScript) == 0x68u,
              "EquipmentAniInfoScript IDA ABI size must stay 0x68");

extern std::map<std::string, STEquipmentAniScript>* g_mapEquipmentAniScript;

int InitMapEquipmentAniScript();
std::map<std::string, STEquipmentAniScript>* getMapEquipmentAniScript();
int ImportEquipmentAniScript(
    std::map<std::string, STEquipmentAniScript>* scripts);

#endif  // OLDSRV_GLOBAL_EQUIPMENT_ANI_SCRIPT_H_
