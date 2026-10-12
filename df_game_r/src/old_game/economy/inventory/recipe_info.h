#ifndef OLDSRV_OLD_GAME_INVENTORY_RECIPE_INFO_H_
#define OLDSRV_OLD_GAME_INVENTORY_RECIPE_INFO_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

struct STStackableScript;

// IDA unit: 99_misc_global/RecipeInfo.
// The neutral offset names are intentional: the old IDB proves the five
// pair-vector positions and scalar widths, but does not publish source field
// names for this record.
struct RecipeInfo {
    RecipeInfo();
    ~RecipeInfo();

    int clear();
    bool additionalInfo(STStackableScript& script);
    int* changeOutput(std::vector<std::pair<int, int> >& output);

    std::vector<std::pair<int, int> > pairs_00;  // +0x00
    std::vector<std::pair<int, int> > pairs_0c;  // +0x0c
    std::vector<std::pair<int, int> > pairs_18;  // +0x18
    uint8_t flag_24;                             // +0x24
    uint8_t flag_25;                             // +0x25
    uint8_t reserved_26[2];                      // +0x26
    std::vector<std::pair<int, int> > pairs_28;  // +0x28
    int32_t value_34;                            // +0x34
    int32_t value_38;                            // +0x38
    int32_t value_3c;                            // +0x3c
    std::vector<std::pair<int, int> > pairs_40;  // +0x40
};

static_assert(sizeof(RecipeInfo) == 0x4cu,
              "RecipeInfo IDA ABI size must stay 0x4C");
static_assert(offsetof(RecipeInfo, pairs_00) == 0x00u &&
                  offsetof(RecipeInfo, pairs_0c) == 0x0cu &&
                  offsetof(RecipeInfo, pairs_18) == 0x18u &&
                  offsetof(RecipeInfo, flag_24) == 0x24u &&
                  offsetof(RecipeInfo, flag_25) == 0x25u &&
                  offsetof(RecipeInfo, pairs_28) == 0x28u &&
                  offsetof(RecipeInfo, value_34) == 0x34u &&
                  offsetof(RecipeInfo, value_38) == 0x38u &&
                  offsetof(RecipeInfo, value_3c) == 0x3cu &&
                  offsetof(RecipeInfo, pairs_40) == 0x40u,
              "RecipeInfo IDA ABI offsets must stay exact");

// IDA_EA: 0x89BA83C
// IDA_NAME: _Z16createRecipeInfoP10RecipeInfoPSt6vectorIiSaIiEEPS1_ISt4pairIiiESaIS6_EE
// DEMANGLED: createRecipeInfo(RecipeInfo *,std::vector<int,std::allocator<int>> *,std::vector*<std::pair<int,int>,std::allocator<std::pair>>)
uint8_t createRecipeInfo(
    RecipeInfo* info,
    std::vector<int>* values,
    std::vector<std::pair<int, int> >* output);

#endif  // OLDSRV_OLD_GAME_INVENTORY_RECIPE_INFO_H_
