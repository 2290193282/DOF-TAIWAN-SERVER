#ifndef OLDSRV_GLOBAL_ITEM_AMPLIFIER_ABI_H_
#define OLDSRV_GLOBAL_ITEM_AMPLIFIER_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <ext/hash_map>
#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/equipment/upgrade_table.h"
#include "old_game/economy/inventory/inventory.h"


class CItem;
struct stAmplifyOption_t;


enum ENUM_AMPLIFY_TYPE {
    ENUM_AMPLIFY_TYPE_NONE = 0,
};

// IDA keeps this named enum in the old CMD208 material-map API (CMD205 in
// the 0828 client).  Only values
// 0..2 are consumed by the original dispatcher; retain the ABI identity
// without creating a separate policy table here.
enum ENUM_ITEM_AMPLIFY_INVEST_TYPE : int;

struct stAmplifyOptionInfo_t {
    int ability_type;
    uint16_t rate;
    uint16_t value;
};

struct stInvestAmplifyOption_t {
    int ability_type;
    uint16_t value;
    uint16_t reserved;
};

struct stAmplifyItemScript_t {
    stAmplifyItemScript_t();
    ~stAmplifyItemScript_t();

    std::map<std::string, ENUM_AMPLIFY_TYPE> amplify_types;
    std::vector<stAmplifyOptionInfo_t> option_infos;
    float invest_rates[6];
    int invest_min_grade;
    uint8_t reserved_40[0x5C];
    int purify_material_item_idx[2];
    int purify_material_item_count[2];
    __gnu_cxx::hash_map<unsigned long, stInvestAmplifyOption_t>
        invest_materials[3];
    int random_option_rates[6];
};

class CItemAmplifier : public stAmplifyItemScript_t {
public:
    CItemAmplifier();

    bool checkInvestableItem(const CItem* item,
                             const Inven_Item& inven_item,
                             int charac_level) const;
    char investAmplifyOption(const CItem* item,
                             Inven_Item& inven_item,
                             const stInvestAmplifyOption_t& option) const;
    int enableInvestAmplifyOption(
        const Inven_Item& inven_item,
        ENUM_ITEM_AMPLIFY_INVEST_TYPE invest_type) const;
    int getInvestMaterialInfo(
        ENUM_ITEM_AMPLIFY_INVEST_TYPE invest_type,
        unsigned long material_item_id,
        stInvestAmplifyOption_t& option) const;
    stAmplifyOption_t* generateAmplifyItem(
        const CItem* item,
        Inven_Item& inven_item) const;
    int loadScript(const char* item_filename, const char* upgrade_filename);
    // Runtime publication gate for the two scripts used by CMD205.  This is
    // computed from existing fields and therefore does not change the ABI.
    bool isScriptReady() const;
    char _generateOption(const CItem* item,
                         ENUM_AMPLIFY_TYPE& ability_type,
                         uint16_t& ability_value) const;
    // IDA_EA: 0x8234CD4 / 0x8234CFA / 0x8234D20.  CMD204 uses these
    // script-owned material mappings before it mutates the inventory.
    int getPurifyMaterialItemIdx(int material_type) const;
    int getPurifyMaterialItemCount(int material_type) const;
    int getPurifyMaterialType(int item_index) const;
    upgrade_table_t* getUpgradeInfoTable() const;

private:
    upgrade_table_t upgrade_table_;

    void swapLoadedState(CItemAmplifier& other);

    CItemAmplifier(const CItemAmplifier&);
    CItemAmplifier& operator=(const CItemAmplifier&);
};

int importAmplifyItem(const char* filename, stAmplifyItemScript_t& script)
    __asm__("_Z17importAmplifyItemPKcR21stAmplifyItemScript_t");
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stAmplifyOptionInfo_t) == 0x08u,
              "stAmplifyOptionInfo_t IDA ABI size must stay 0x08");
static_assert(sizeof(stInvestAmplifyOption_t) == 0x08u,
              "stInvestAmplifyOption_t IDA ABI size must stay 0x08");
static_assert(sizeof(stAmplifyItemScript_t) == 0x100u,
              "stAmplifyItemScript_t IDA ABI size must stay 0x100");
static_assert(offsetof(stAmplifyItemScript_t, option_infos) == 0x18u,
              "stAmplifyItemScript_t option vector offset must stay 0x18");
static_assert(offsetof(stAmplifyItemScript_t, invest_rates) == 0x24u,
              "stAmplifyItemScript_t invest rates offset must stay 0x24");
static_assert(offsetof(stAmplifyItemScript_t, invest_min_grade) == 0x3Cu,
              "stAmplifyItemScript_t invest min grade offset must stay 0x3C");
static_assert(offsetof(stAmplifyItemScript_t, purify_material_item_idx) ==
                  0x9Cu,
              "stAmplifyItemScript_t purify item idx offset must stay 0x9C");
static_assert(offsetof(stAmplifyItemScript_t, invest_materials) == 0xACu,
              "stAmplifyItemScript_t invest hash offset must stay 0xAC");
static_assert(offsetof(stAmplifyItemScript_t, random_option_rates) == 0xE8u,
              "stAmplifyItemScript_t random option rates offset must stay 0xE8");
static_assert(sizeof(CItemAmplifier) == 0x61Cu,
              "typed-table CItemAmplifier size must stay 0x61C");
#endif

#endif  // OLDSRV_GLOBAL_ITEM_AMPLIFIER_ABI_H_
