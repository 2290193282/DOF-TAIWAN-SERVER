#include "common/config.h"
#include "old_data/PVF/data/premium_item_data.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/premium_public_abi.h"

#include <old_game/player/core/premium_runtime.h>

#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <string>
#include <vector>

// stPremiumTable_t::C2 @0x86B0898 constructs only its vector, string, and
// parser-item vector.  This parser-side record must consequently remain a
// trivially initialized view rather than construct _Additioal_info at +0xc8.
#pragma pack(push, 1)
struct PremiumAdditionalInfoFields {
    uint32_t max_hp;
    uint32_t max_mp;
    uint16_t physical_attack;
    uint16_t physical_defense;
    uint16_t magical_attack;
    uint16_t magical_defense;
    uint16_t element_resist[4];
    uint16_t spec_property[17];
    uint16_t reserved_03a;
    int32_t inventory_weight;
    int16_t hp_regen_speed;
    int16_t mp_regen_speed;
    uint16_t move_speed;
    uint16_t attack_speed;
    uint16_t cast_speed;
    int16_t hit_recovery;
    int16_t jump_power;
    uint16_t reserved_04e;
    int32_t character_weight;
};
#pragma pack(pop)

static_assert(sizeof(PremiumAdditionalInfoFields) == 0x54u,
              "premium parser additional-info fields must stay at 0x54");
static_assert(offsetof(PremiumAdditionalInfoFields, inventory_weight) ==
                  0x3cu &&
                  offsetof(PremiumAdditionalInfoFields, move_speed) ==
                      0x44u &&
                  offsetof(PremiumAdditionalInfoFields, character_weight) ==
                      0x50u,
              "premium parser additional-info field offsets moved");

struct stPremiumTable_t {
    int advantage_bucket;                  // +0x000
    int uses_time_window;                  // +0x004
    int expire_time;                       // +0x008
    SPremiumInfo::ItemReward item_rewards[5];  // +0x00c
    uint16_t fatigue_rate;                 // +0x034
    uint16_t reserved_036;                 // +0x036
    int coin_bonus;                        // +0x038
    int exp_bonus;                         // +0x03c
    bool unlimited_fatigue;                // +0x040
    uint8_t reserved_041;                  // +0x041
    uint16_t over_skill_level;             // +0x042
    uint16_t cera_shop_discount_rate;      // +0x044
    uint16_t reserved_046;                 // +0x046
    int add_exp_bonus;                     // +0x048
    int add_quest_item_drop_rate;          // +0x04c
    std::vector<unsigned int> independent_drop_items;  // +0x050
    int apply_target_server;               // +0x05c
    bool gold_bonus;                       // +0x060
    uint8_t reserved_061[3];               // +0x061
    int over_equipable_count;              // +0x064
    SPremiumInfo::IndexedRate over_equipable[5];  // +0x068
    int return_item_count;                 // +0x090
    SPremiumInfo::IndexedRate return_items[3];  // +0x094
    int apply_weekdays[7];                 // +0x0ac
    PremiumAdditionalInfoFields additional_info;  // +0x0c8
    std::string name;                      // +0x11c
    uint32_t target_item_id;               // +0x120
    std::vector<premium_script_parsing::STPremiumItemData> parser_items;
                                           // +0x124

    stPremiumTable_t();
    ~stPremiumTable_t();
    int clean();

};

typedef char stPremiumTable_t_Size_Must_Be_0x130[
    (sizeof(stPremiumTable_t) == 0x130) ? 1 : -1];
typedef char SPremiumOverEquipableInfo_Size_Must_Be_0x70[
    (sizeof(SPremiumOverEquipableInfo) == 0x70) ? 1 : -1];
typedef char SPremiumReturnItemInfo_Size_Must_Be_0x2c[
    (sizeof(SPremiumReturnItemInfo) == 0x2c) ? 1 : -1];
typedef char STPremiumItemData_Size_Must_Be_0x0c[
    (sizeof(STPremiumItemData) == 0x0c) ? 1 : -1];
static_assert(offsetof(stPremiumTable_t, independent_drop_items) == 0x050u &&
                  offsetof(stPremiumTable_t, additional_info) == 0x0c8u &&
                  offsetof(stPremiumTable_t, name) == 0x11cu &&
                  offsetof(stPremiumTable_t, target_item_id) == 0x120u &&
                  offsetof(stPremiumTable_t, parser_items) == 0x124u,
              "stPremiumTable_t parser ABI drift");

bool loadRDARScriptFile(const char* dir, const char* filename);

// IDA_EA: 0x8905D95
// IDA_NAME: _Z17importPremiumListPKcP16stPremiumTable_tRSt6vectorI25SPremiumOverEquipableInfoSaIS4_EERS3_I22SPremiumReturnItemInfoSaIS8_EE
// DEMANGLED: importPremiumList(char const*,stPremiumTable_t *,std::vector<SPremiumOverEquipableInfo,std::allocator<SPremiumOverEquipableInfo>> &,std::vector<SPremiumReturnItemInfo,std::allocator<SPremiumReturnItemInfo>> &)
int importPremiumList(
    const char* filename,
    stPremiumTable_t* premium_table,
    std::vector<SPremiumOverEquipableInfo>& over_equipable_info,
    std::vector<SPremiumReturnItemInfo>& return_item_info)
    __asm__("_Z17importPremiumListPKcP16stPremiumTable_tRSt6vectorI25SPremiumOverEquipableInfoSaIS4_EERS3_I22SPremiumReturnItemInfoSaIS8_EE");

// IDA_EA: 0x86B0898
// IDA_NAME: _ZN16stPremiumTable_tC2Ev
// DEMANGLED: stPremiumTable_t::stPremiumTable_t(void)
stPremiumTable_t::stPremiumTable_t() {
}

// IDA_EA: 0x86B0910
// IDA_NAME: _ZN16stPremiumTable_tD2Ev
// DEMANGLED: stPremiumTable_t::~stPremiumTable_t()
stPremiumTable_t::~stPremiumTable_t() {
}

// IDA_EA: 0x891AE68
// IDA_NAME: _ZN16stPremiumTable_t5cleanEv
// DEMANGLED: stPremiumTable_t::clean(void)
int stPremiumTable_t::clean() {
    for (int index = 0; index <= 6; ++index) {
        apply_weekdays[index] = -1;
    }
    advantage_bucket = 0;
    uses_time_window = 0;
    over_skill_level = 0;
    cera_shop_discount_rate = 0;
    apply_target_server = 0;
    expire_time = 0;
    unlimited_fatigue = false;
    fatigue_rate = 0;
    coin_bonus = 0;
    exp_bonus = 0;
    for (int index = 0; index <= 4; ++index) {
        item_rewards[index].item_id = 0;
        item_rewards[index].item_count = 0;
    }
    gold_bonus = false;
    over_equipable_count = 0;
    for (int index = 0; index <= 4; ++index) {
        over_equipable[index].list_index = 0;
        over_equipable[index].rate = 0;
    }
    return_item_count = 0;
    for (int index = 0; index <= 2; ++index) {
        return_items[index].list_index = 0;
        return_items[index].rate = 0;
    }
    additional_info = PremiumAdditionalInfoFields();
    add_exp_bonus = 0;
    add_quest_item_drop_rate = 0;
    independent_drop_items.clear();
    target_item_id = 0;
    parser_items.clear();
    return 0;
}

// IDA_EA: 0x8905D95
// IDA_NAME: _Z17importPremiumListPKcP16stPremiumTable_tRSt6vectorI25SPremiumOverEquipableInfoSaIS4_EERS3_I22SPremiumReturnItemInfoSaIS8_EE
// DEMANGLED: importPremiumList(char const*,stPremiumTable_t *,std::vector<SPremiumOverEquipableInfo,std::allocator<SPremiumOverEquipableInfo>> &,std::vector<SPremiumReturnItemInfo,std::allocator<SPremiumReturnItemInfo>> &)
int importPremiumList(
    const char* filename,
    stPremiumTable_t* premium_table,
    std::vector<SPremiumOverEquipableInfo>& over_equipable_info,
    std::vector<SPremiumReturnItemInfo>& return_item_info) {
    if (!loadRDARScriptFile("", filename)) {
        return 5286;
    }

    for (int index = 0;
         index < oldsrv::config::kPremiumDefinitionTypeCount;
         ++index) {
        premium_table[index].clean();
    }

    stPremiumTable_t* current = premium_table;
    std::string tag;
    std::string text;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[overequip list]") {
            SPremiumOverEquipableInfo value;
            value.equipment_type_count = 0;
            std::fill(value.equipment_types,
                      value.equipment_types +
                          oldsrv::config::kEquipmentTypeCount,
                      0u);
            over_equipable_info.push_back(value);
            while (true) {
                value.equipment_type_count =
                    static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                if (value.equipment_type_count >
                    oldsrv::config::kEquipmentTypeCount) {
                    return 5321;
                }
                for (int index = 0;
                     index < static_cast<int>(value.equipment_type_count);
                     ++index) {
                    if (ScanStr(&text) != 1) {
                        return 5317;
                    }

                    int equip_type;
                    if (text == "[hat avatar]") {
                        equip_type = 0;
                    } else if (text == "[hair avatar]") {
                        equip_type = 1;
                    } else if (text == "[face avatar]") {
                        equip_type = 2;
                    } else if (text == "[coat avatar]") {
                        equip_type = 3;
                    } else if (text == "[pants avatar]") {
                        equip_type = 4;
                    } else if (text == "[shoes avatar]") {
                        equip_type = 5;
                    } else if (text == "[breast avatar]") {
                        equip_type = 6;
                    } else if (text == "[waist avatar]") {
                        equip_type = 7;
                    } else if (text == "[skin avatar]") {
                        equip_type = 8;
                    } else if (text == "[aurora avatar]") {
                        equip_type = 9;
                    } else if (text == "[weapon avatar]") {
                        equip_type = 10;
                    } else if (text == "[weapon]") {
                        equip_type = 11;
                    } else if (text == "[title name]") {
                        equip_type = 12;
                    } else if (text == "[coat]") {
                        equip_type = 13;
                    } else if (text == "[shoulder]") {
                        equip_type = 14;
                    } else if (text == "[pants]") {
                        equip_type = 15;
                    } else if (text == "[shoes]") {
                        equip_type = 16;
                    } else if (text == "[waist]") {
                        equip_type = 17;
                    } else if (text == "[amulet]") {
                        equip_type = 18;
                    } else if (text == "[wrist]") {
                        equip_type = 19;
                    } else if (text == "[ring]") {
                        equip_type = 20;
                    } else if (text == "[support]") {
                        equip_type = 21;
                    } else if (text == "[magic stone]") {
                        equip_type = 22;
                    } else {
                        equip_type = oldsrv::config::kEquipmentTypeInvalid;
                    }
                    if (equip_type == oldsrv::config::kEquipmentTypeInvalid) {
                        return 5322;
                    }
                    value.equipment_types[index] =
                        static_cast<uint32_t>(equip_type);
                }
                over_equipable_info.push_back(value);
            }
        } else if (tag == "[returnitem list]") {
            SPremiumReturnItemInfo value;
            value.item_count = 0;
            std::fill(value.item_ids, value.item_ids + 10, 0u);
            return_item_info.push_back(value);
            while (true) {
                value.item_count = static_cast<uint32_t>(ScanInt(&success));
                if (!success) {
                    break;
                }
                for (int index = 0;
                     index < static_cast<int>(value.item_count);
                     ++index) {
                    value.item_ids[index] =
                        static_cast<uint32_t>(ScanInt(&success));
                    if (!success) {
                        return 5394;
                    }
                }
                return_item_info.push_back(value);
            }
        } else if (tag == "[type]") {
            const int type = ScanInt(static_cast<bool*>(0));
            if (type < 0 ||
                type >= oldsrv::config::kPremiumDefinitionTypeCount) {
                return 5403;
            }
            current = premium_table + type;
        } else if (tag == "[name]") {
            if (ScanStr(&text) != 1) {
                return 5408;
            }
            current->name = text;
        } else if (tag == "[attr]") {
            if (ScanStr(&text) != 1) {
                return 5413;
            }
            if (text == "[term]") {
                current->advantage_bucket = 2;
            } else if (text == "[bonus]") {
                current->advantage_bucket = 1;
            } else {
                return 5419;
            }
        } else if (tag == "[term]") {
            current->uses_time_window = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[expire time]") {
            current->expire_time = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[items]") {
            for (int index = 0;; ++index) {
                const int item_id = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int item_count = ScanInt(&success);
                if (!success) {
                    break;
                }
                current->item_rewards[index].item_id =
                    static_cast<uint32_t>(item_id);
                current->item_rewards[index].item_count =
                    static_cast<uint32_t>(item_count);
            }
        } else if (tag == "[fatigue]") {
            current->fatigue_rate =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[unlimit fatigue]") {
            current->unlimited_fatigue =
                ScanInt(static_cast<bool*>(0)) == 1;
        } else if (tag == "[bonus exp]") {
            current->add_exp_bonus = ScanInt(&success);
            if (!success) {
                break;
            }
        } else if (tag == "[quest item drop rate]") {
            current->add_quest_item_drop_rate = ScanInt(&success);
            if (!success) {
                break;
            }
        } else if (tag == "[independent drop rate]") {
            while (true) {
                const unsigned int item_id =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                current->independent_drop_items.push_back(item_id);
            }
        } else if (tag == "[exp]") {
            current->exp_bonus = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[coin]") {
            current->coin_bonus = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[gold bonus]") {
            ScanStr(&text);
            if (text == "[true]") {
                current->gold_bonus = true;
            } else if (text == "[false]") {
                current->gold_bonus = false;
            } else {
                return 5485;
            }
        } else if (tag == "[over skill]") {
            current->over_skill_level =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[cerashop discount rate]") {
            current->cera_shop_discount_rate =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[apply target server]") {
            if (ScanStr(&text) != 1) {
                return 5507;
            }
            if (text == "self") {
                current->apply_target_server = 1;
            } else if (text == "all") {
                current->apply_target_server = 0;
            } else {
                return 5512;
            }
        } else if (tag == "[over equipinfo]") {
            current->over_equipable_count =
                ScanInt(static_cast<bool*>(0));
            if (current->over_equipable_count > 5) {
                return 5521;
            }
            for (int index = 0;
                 index < current->over_equipable_count;
                 ++index) {
                const int first = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    break;
                }
                current->over_equipable[index].list_index = first;
                current->over_equipable[index].rate = second;
            }
        } else if (tag == "[return iteminfo]") {
            current->return_item_count = ScanInt(static_cast<bool*>(0));
            if (current->return_item_count > 3) {
                return 5542;
            }
            bool incomplete = false;
            for (int index = 0;
                 index < current->return_item_count;
                 ++index) {
                if (index > 2) {
                    return 5546;
                }
                const int first = ScanInt(&success);
                if (!success) {
                    incomplete = true;
                    break;
                }
                const int second = ScanInt(&success);
                if (!success) {
                    incomplete = true;
                    break;
                }
                current->return_items[index].list_index = first;
                current->return_items[index].rate = second;
            }
            if (incomplete) {
                continue;
            }
        } else if (tag == "[apply date]") {
            for (int index = 0;; ++index) {
                const int value = ScanInt(&success);
                if (!success) {
                    break;
                }
                current->apply_weekdays[index] = value;
            }
        } else if (tag == "[HP MAX]") {
            current->additional_info.max_hp =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[MP MAX]") {
            current->additional_info.max_mp =
                static_cast<uint32_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[physical attack]") {
            current->additional_info.physical_attack =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[physical defense]") {
            current->additional_info.physical_defense =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[magical attack]") {
            current->additional_info.magical_attack =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[magical defense]") {
            current->additional_info.magical_defense =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[fire resistance]") {
            current->additional_info.element_resist[0] =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[water resistance]") {
            current->additional_info.element_resist[1] =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[dark resistance]") {
            current->additional_info.element_resist[2] =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[light resistance]") {
            current->additional_info.element_resist[3] =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[inventory limit]") {
            current->additional_info.inventory_weight =
                ScanInt(static_cast<bool*>(0));
        } else if (tag == "[HP regen speed]") {
            current->additional_info.hp_regen_speed =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[MP regen speed]") {
            current->additional_info.mp_regen_speed =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[move speed]") {
            current->additional_info.move_speed =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[attack speed]") {
            current->additional_info.attack_speed =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[cast speed]") {
            current->additional_info.cast_speed =
                static_cast<uint16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[hit recovery]") {
            current->additional_info.hit_recovery =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[jump power]") {
            current->additional_info.jump_power =
                static_cast<int16_t>(ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[target item]") {
            const int item_id = ScanInt(&success);
            if (!success) {
                break;
            }
            current->target_item_id = static_cast<uint32_t>(item_id);
        } else if (tag == "[item]") {
            premium_script_parsing::STPremiumItemData item_data;
            item_data.item_id = ScanInt(&success);
            if (!success) {
                return 5627;
            }
            do {
                if (ScanType(tag, true) != 1) {
                    return 5632;
                }
                if (tag == "[term]") {
                    while (true) {
                        const int term = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        item_data.flag_00 = static_cast<uint8_t>(term);
                    }
                } else if (tag == "[restrict]") {
                    while (true) {
                        const int restricted = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        item_data.flag_01 = static_cast<uint8_t>(restricted);
                    }
                }
            } while (tag != "[/item]");
            current->parser_items.push_back(item_data);
        }
    }
    return 0;
}

namespace WongWork {

namespace {

// The temporary parser record deliberately keeps its vector at +0x50 while
// the published record keeps it at +0x4c.  Copy every scalar and aggregate in
// the same order as LoadPremiumInfo @0x86AECA4; a byte copy would corrupt the
// published vector and its ownership.
void PublishPremiumInfo(const stPremiumTable_t& source,
                        SPremiumInfo* destination) {
    destination->advantage_bucket = source.advantage_bucket;
    destination->uses_time_window = source.uses_time_window;
    destination->expire_time = source.expire_time;
    for (int index = 0; index <= 4; ++index) {
        destination->item_rewards[index] = source.item_rewards[index];
    }
    destination->fatigue_rate = source.fatigue_rate;
    destination->coin_bonus = source.coin_bonus;
    destination->exp_bonus = source.exp_bonus;
    destination->unlimited_fatigue = source.unlimited_fatigue;
    destination->add_exp_bonus = source.add_exp_bonus;
    destination->add_quest_item_drop_rate = source.add_quest_item_drop_rate;
    destination->independent_drop_items = source.independent_drop_items;
    destination->over_skill_level = source.over_skill_level;
    destination->cera_shop_discount_rate = source.cera_shop_discount_rate;
    destination->apply_target_server = source.apply_target_server;
    destination->gold_bonus = source.gold_bonus;
    destination->over_equipable_count = source.over_equipable_count;
    for (int index = 0; index <= 4; ++index) {
        destination->over_equipable[index] = source.over_equipable[index];
    }
    destination->return_item_count = source.return_item_count;
    for (int index = 0; index <= 2; ++index) {
        destination->return_items[index] = source.return_items[index];
    }
    for (int index = 0; index <= 6; ++index) {
        destination->apply_weekdays[index] = source.apply_weekdays[index];
    }
    destination->additional_info.max_hp = source.additional_info.max_hp;
    destination->additional_info.max_mp = source.additional_info.max_mp;
    destination->additional_info.physical_attack =
        source.additional_info.physical_attack;
    destination->additional_info.physical_defense =
        source.additional_info.physical_defense;
    destination->additional_info.magical_attack =
        source.additional_info.magical_attack;
    destination->additional_info.magical_defense =
        source.additional_info.magical_defense;
    for (int index = 0; index <= 3; ++index) {
        destination->additional_info.element_resist[index] =
            source.additional_info.element_resist[index];
    }
    for (int index = 0; index <= 16; ++index) {
        destination->additional_info.spec_property[index] =
            source.additional_info.spec_property[index];
    }
    destination->additional_info.inventory_weight =
        source.additional_info.inventory_weight;
    destination->additional_info.hp_regen_speed =
        source.additional_info.hp_regen_speed;
    destination->additional_info.mp_regen_speed =
        source.additional_info.mp_regen_speed;
    destination->additional_info.move_speed = source.additional_info.move_speed;
    destination->additional_info.attack_speed =
        source.additional_info.attack_speed;
    destination->additional_info.cast_speed = source.additional_info.cast_speed;
    destination->additional_info.hit_recovery =
        source.additional_info.hit_recovery;
    destination->additional_info.jump_power = source.additional_info.jump_power;
    destination->additional_info.character_weight =
        source.additional_info.character_weight;
    destination->name = source.name;
}

}  // namespace

// IDA_EA: 0x86AECA4
// IDA_NAME: _ZN8WongWork12CUserPremium15LoadPremiumInfoEPKc
// DEMANGLED: WongWork::CUserPremium::LoadPremiumInfo(char const*)
int CUserPremium::LoadPremiumInfo(const char* filename) {
    stPremiumTable_t
        premium_table[oldsrv::config::kPremiumDefinitionTypeCount];
    if (importPremiumList(filename,
                          premium_table,
                          g_vecPremiumOverEquipableInfo,
                          g_vecPremiumReturnItemInfo) != 0) {
        return 348;
    }

    for (int premium_type = 0;
         premium_type < oldsrv::config::kPremiumDefinitionTypeCount;
         ++premium_type) {
        const stPremiumTable_t& source = premium_table[premium_type];
        SPremiumInfo* const destination = &g_SPremiumInfo[premium_type];
        PublishPremiumInfo(source, destination);

        STPremiumItemData runtime_data;
        runtime_data.premium_type = static_cast<uint32_t>(premium_type);
        runtime_data.target_item_id = source.target_item_id;

        const std::vector<premium_script_parsing::STPremiumItemData>&
            parser_items = source.parser_items;
        if (!parser_items.empty()) {
            for (std::vector<premium_script_parsing::STPremiumItemData>::
                     const_iterator item = parser_items.begin();
                 item != parser_items.end();
                 ++item) {
                runtime_data.term = item->flag_00;
                runtime_data.restrict_flag = item->flag_01 != 0;
                if (SetPremiumItemData(
                        static_cast<unsigned long>(item->item_id),
                        runtime_data) != 1) {
                    return 444;
                }
            }
        } else {
            runtime_data.term =
                static_cast<uint8_t>(destination->uses_time_window);
            runtime_data.restrict_flag = 0;
            if (SetPremiumItemData(
                    static_cast<unsigned long>(source.target_item_id),
                    runtime_data) != 1) {
                return 452;
            }
        }
    }

    return 0;
}

// IDA_EA: 0x86AF70A
// IDA_NAME: _ZN8WongWork12CUserPremium18SetPremiumItemDataEmRK17STPremiumItemData
// DEMANGLED: WongWork::CUserPremium::SetPremiumItemData(unsigned long,STPremiumItemData const&)
int CUserPremium::SetPremiumItemData(unsigned long item_id,
                                     const STPremiumItemData& item_data) {
    if (item_id == 0) {
        return 1;
    }

    const std::map<unsigned long, STPremiumItemData>::iterator existing =
        g_map_premium_item_data.find(item_id);
    if (existing != g_map_premium_item_data.end()) {
        return 0;
    }

    g_map_premium_item_data.insert(std::make_pair(item_id, item_data));
    return 1;
}

// IDA_EA: 0x86AF7C4
// IDA_NAME: _ZN8WongWork12CUserPremium19IsRestrictedPremiumEi
// DEMANGLED: WongWork::CUserPremium::IsRestrictedPremium(int)
int CUserPremium::IsRestrictedPremium(int premium_type) {
    return premium_type > 59999 && premium_type <= 69999;
}

}  // namespace WongWork
#include "common/config.h"
