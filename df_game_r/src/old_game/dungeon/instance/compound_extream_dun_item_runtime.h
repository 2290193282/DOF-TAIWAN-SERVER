#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_COMPOUND_EXTREAM_DUN_ITEM_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_COMPOUND_EXTREAM_DUN_ITEM_RUNTIME_H_

#include "old_game/dungeon/instance/bind_cube_lv_bonus_script_data.h"
#include "old_game/dungeon/instance/bind_cube_script_data.h"
#include "common/manager/mt_rand.h"

#include <stddef.h>
#include <utility>

// The original signatures retain this global item-policy enum rather than an
// anonymous int.  The enumerator table is not part of the recovered ABI, but
// the named int-sized type is required for the old member symbol identities.
enum ENUM_UNUSABLE_EQUIPMENT_TYPE : int;
class CEquipItem;
struct Inven_Item;

namespace ExtreamDungeon {

struct CCompound_ExtreamDun_Item_LayoutCheck;

// Named, i386 int-sized ABI parameters from the bind-cube member signatures.
// The original binary does not expose their enumerator tables here, so retain
// them as opaque enums instead of inventing values.
enum ENUM_INPUTITEMS_RARITY : int;
enum ENUM_OUTPUTITEMS_RARITY : int;
enum eBindSphereType : int;

// IDA unit: 09_dungeon_pvp_party/ExtreamDungeon/CCompound_ExtreamDun_Item.
class CCompound_ExtreamDun_Item {
public:
    // IDA_EA: 0x832C402
    // IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_ItemC2Ev
    // DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::CCompound_ExtreamDun_Item(void)
    CCompound_ExtreamDun_Item();

    // IDA_EA: 0x832C524
    // IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_ItemD2Ev
    // DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::~CCompound_ExtreamDun_Item()
    ~CCompound_ExtreamDun_Item();

    // IDA_EA: 0x832C65E
    // IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item10LoadScriptEPKcS2_
    // DEMANGLED: ExtreamDungeon::CCompound_ExtreamDun_Item::LoadScript(char const*,char const*)
    int LoadScript(const char* directory, const char* filename);

    // IDA_EA: 0x832C5AC
    // IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item18_convertOutputItemE28ENUM_UNUSABLE_EQUIPMENT_TYPE
    int _convertOutputItem(ENUM_UNUSABLE_EQUIPMENT_TYPE type) const;

    // IDA_EA: 0x832C5F0
    // IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item23_convertInput_To_OutputENS_22ENUM_INPUTITEMS_RARITYE
    int _convertInput_To_Output(ENUM_INPUTITEMS_RARITY rarity) const;

    // IDA_EA: 0x832C620
    // IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item17_convertInputItemE28ENUM_UNUSABLE_EQUIPMENT_TYPE
    int _convertInputItem(ENUM_UNUSABLE_EQUIPMENT_TYPE type) const;

    // IDA_EA: 0x832C976
    // IDA_NAME: _ZNK14ExtreamDungeon25CCompound_ExtreamDun_Item17_determine_rarityEjNS_22ENUM_INPUTITEMS_RARITYE
    int _determine_rarity(unsigned int random_value,
                          ENUM_INPUTITEMS_RARITY input_rarity) const;

    // IDA_EA: 0x832C9CA / 0x832CADC / 0x832CC02.  Both overloads return
    // inclusive [first, second] level-adjustment ranges.
    std::pair<unsigned int, unsigned int> _determine_level_bonus(
        unsigned int level,
        ENUM_INPUTITEMS_RARITY input_rarity,
        unsigned int& index,
        eBindSphereType sphere_type) const;
    std::pair<unsigned int, unsigned int> _determine_level_bonus(
        ENUM_INPUTITEMS_RARITY input_rarity,
        unsigned int random_value,
        ENUM_OUTPUTITEMS_RARITY output_rarity,
        eBindSphereType sphere_type) const;
    unsigned int _determine_level_bonus_index(
        unsigned int level,
        ENUM_INPUTITEMS_RARITY input_rarity,
        eBindSphereType sphere_type) const;

    // IDA_EA: 0x832CCC0
    // IDA_NAME: _ZN14ExtreamDungeon25CCompound_ExtreamDun_Item16makeCompoundItemEPPK10CEquipItemcR10Inven_ItemNS_15eBindSphereTypeE
    int makeCompoundItem(const CEquipItem* const input_items[2],
                         char selection_type,
                         Inven_Item& output,
                         eBindSphereType sphere_type);

    // IDA_EA: 0x832CFFE .. 0x832D4B4.
    int _exception_Rare(const CEquipItem* const input_items[2],
                        char selection_type,
                        ENUM_OUTPUTITEMS_RARITY output_rarity,
                        unsigned int base_level) const;
    int _FindSelectItem(unsigned int item_group,
                        ENUM_OUTPUTITEMS_RARITY output_rarity,
                        const std::pair<unsigned int, unsigned int>& level_range) const;
    int _FindRandomItem(ENUM_OUTPUTITEMS_RARITY output_rarity,
                        const std::pair<unsigned int, unsigned int>& level_range) const;
    int _FindItemAll_BindSphere(
        const CEquipItem* const input_items[2],
        char selection_type,
        ENUM_OUTPUTITEMS_RARITY output_rarity,
        const std::pair<unsigned int, unsigned int>& level_range) const;
    int _FindItemRelateCreationRate(
        std::vector<OUTPUT_ITEM_STR>::const_iterator first,
        std::vector<OUTPUT_ITEM_STR>::const_iterator last) const;

    // IDA_EA: 0x832D784 .. 0x832E40C.
    int ForceResultItemRule_Setting(int item_index,
                                    bool force,
                                    int creation_rate);
    int checkCompoundable(Inven_Item input_items[2],
                          const CEquipItem* const item_prototypes[2],
                          char sphere_type) const;
    bool check_N_addItem(CEquipItem& item);
    bool _isPossibleResultItem(CEquipItem& item) const;
    bool _AddItem(CEquipItem& item);
    bool after_addItem();
    unsigned int _get_base_level(
        const CEquipItem* const input_items[2]) const;

private:
    friend struct CCompound_ExtreamDun_Item_LayoutCheck;

    CMTRand* random_;
    BindCube_ScriptData script_data_;
    BindCube_LvBonusScriptData level_bonus_script_data_[2];
};

// The fields remain private; this friend is solely the i386 ABI layout check.
struct CCompound_ExtreamDun_Item_LayoutCheck {
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(CCompound_ExtreamDun_Item, script_data_) == 0x004u &&
                  offsetof(CCompound_ExtreamDun_Item,
                           level_bonus_script_data_) == 0x0b8cu &&
                  sizeof(CCompound_ExtreamDun_Item) == 0x0becu,
              "CCompound_ExtreamDun_Item must retain its original i386 layout");
#endif
};

}  // namespace ExtreamDungeon

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_COMPOUND_EXTREAM_DUN_ITEM_RUNTIME_H_
