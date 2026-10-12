#ifndef OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_ITEM_HANDLE_H_
#define OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_ITEM_HANDLE_H_

#include <vector>

#include "old_data/PVF/equipment/upgrade_table.h"

class CMTRand;
class CRandomOptionGive;
struct RegenerationROI;
struct RandomOption;
struct RandomOptionScript;

namespace random_option {

// IDA_EA: 0x85F0EB0.  This is the process-wide, loaded PVF script owned by
// CDataManager; it is not an independently allocated cache.
RandomOptionScript* GetRandomOption();

class CRandomOptionItemHandle {
public:
    // IDA_EA: 0x85F0D64 / 0x85F0E46
    CRandomOptionItemHandle();
    virtual ~CRandomOptionItemHandle();

    // IDA_EA: 0x8539E42 / 0x85F0EA2
    CRandomOptionItemHandle* set_disable_drop();
    CRandomOptionItemHandle* set_random_option_give(
        CRandomOptionGive* random_option_give);

    // IDA_EA: 0x85F2CC6
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle11give_optionEm11ENUM_RARITYiiiP12RandomOption
    // DEMANGLED: random_option::CRandomOptionItemHandle::give_option(unsigned long,ENUM_RARITY,int,int,int,RandomOption *)
    int give_option(unsigned long item_index,
                    ENUM_RARITY rarity,
                    int usable_level,
                    int item_group_name,
                    int option_grade,
                    RandomOption* option);

    // IDA_EA: 0x85F3340
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle13change_optionEm11ENUM_RARITYiiiiP12RandomOption
    int change_option(unsigned long item_index,
                      ENUM_RARITY rarity,
                      int usable_level,
                      int item_group_name,
                      int option_grade,
                      int option_number,
                      RandomOption* option);

    // IDA_EA: 0x85F39D2
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle12reset_optionEm11ENUM_RARITYiP12RandomOption
    int reset_option(unsigned long item_index,
                     ENUM_RARITY rarity,
                     int option_number,
                     RandomOption* option);

    // IDA_EA: 0x85F1560
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle11choose_itemER11ENUM_RARITYi
    int choose_item(ENUM_RARITY& rarity, int usable_level);

    // IDA_EA: 0x85F1802
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle20RegenVer_choose_itemE11ENUM_RARITYiRKSt6vectorIiSaIiEE
    int RegenVer_choose_item(ENUM_RARITY rarity,
                             int usable_level,
                             const std::vector<int>& requested_item_groups);
    // IDA_EA: 0x85F1A62
    int RegenVer_get_option_quantity(ENUM_RARITY rarity,
                                     int option_grade,
                                     int option_type,
                                     int usable_level,
                                     int& quantity,
                                     RegenerationROI* regeneration);
    // IDA_EA: 0x85F1BB0
    int RegenVer_give_option(unsigned long item_index,
                             ENUM_RARITY rarity,
                             int usable_level,
                             int item_group_name,
                             int option_grade,
                             RandomOption* option,
                             RegenerationROI* regeneration,
                             int regeneration_mode);

private:
    // IDA_EA: 0x85F0EC2
    int _random_application(ENUM_RARITY rarity, int usable_level);
    // IDA_EA: 0x85F0F9A
    int _get_level_limit(ENUM_RARITY rarity, int usable_level);
    // IDA_EA: 0x85F1088
    // IDA_NAME: _ZN13random_option23CRandomOptionItemHandle14_get_part_typeEi
    // DEMANGLED: random_option::CRandomOptionItemHandle::_get_part_type(int)
    int _get_part_type(int usable_level);
    // IDA_EA: 0x85F11A4
    int _get_part_type_by_ItemGroup(
        int usable_level,
        const std::vector<int>& requested_item_groups) const;
    // IDA_EA: 0x85F1426
    int _get_base_item(ENUM_RARITY rarity, int usable_level);
    // IDA_EA: 0x85F2338
    int _get_option_type(unsigned long item_index,
                         ENUM_RARITY rarity,
                         int usable_level,
                         int option_grade,
                         unsigned char& option_type);
    // IDA_EA: 0x85F25B0
    int _get_option_quantity(ENUM_RARITY rarity,
                             int option_grade,
                             int option_type,
                             int usable_level,
                             int& quantity);
    // IDA_EA: 0x85F2882
    int _choose_option_group(ENUM_RARITY rarity,
                             int option_grade,
                             int item_group_name,
                             int usable_level,
                             int option_type,
                             int& option_group);
    // IDA_EA: 0x85F2B02
    unsigned int _get_option_value(int begin_value, int end_value);

    CMTRand* random_streams_;                 // +0x04, 13 CMTRand values
    int random_stream_index_;                  // +0x08
    bool disable_drop_;                        // +0x0c
    unsigned char pad_0d_[3];
    int last_option_group_;                    // +0x10
    CRandomOptionGive* random_option_give_;    // +0x14
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CRandomOptionItemHandle) == 0x18u,
              "CRandomOptionItemHandle ABI");
#endif

}  // namespace random_option

#endif  // OLDSRV_OLD_DATA_PVF_EQUIPMENT_RANDOM_OPTION_ITEM_HANDLE_H_
