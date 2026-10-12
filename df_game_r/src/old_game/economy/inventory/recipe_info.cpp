#include "old_game/economy/inventory/recipe_info.h"

#include <stdint.h>

#include "old_data/PVF/stackable/stackable_script.h"

namespace {

uint8_t LowAddressByte(const void* address) {
    return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(address));
}

}  // namespace

// IDA_EA: 0x8513BF2
// IDA_NAME: _ZN10RecipeInfo5clearEv
// DEMANGLED: RecipeInfo::clear(void)
int RecipeInfo::clear() {
    pairs_00.clear();
    pairs_0c.clear();
    pairs_18.clear();
    flag_24 = 0;
    flag_25 = 0;
    pairs_28.clear();
    value_34 = 0;
    value_38 = 0;
    value_3c = 0;
    pairs_40.clear();

    // The old i386 body tail-calls vector::clear for the +0x40 vector.  That
    // implementation leaves the vector receiver in EAX; retain the IDB's
    // recorded int return without inventing a success value.
    return static_cast<int>(reinterpret_cast<uintptr_t>(&pairs_40));
}

// IDA_EA: 0x8513C6A
// IDA_NAME: _ZN10RecipeInfoC2Ev
// DEMANGLED: RecipeInfo::RecipeInfo(void)
RecipeInfo::RecipeInfo() {
    clear();
}

// IDA_EA: 0x8513D44
// IDA_NAME: _ZN10RecipeInfo14additionalInfoER17STStackableScript
// DEMANGLED: RecipeInfo::additionalInfo(STStackableScript &)
bool RecipeInfo::additionalInfo(STStackableScript& script) {
    value_34 = script.values_4ec[1];
    value_38 = script.values_4ec[2];
    value_3c = script.values_4ec[3];

    unsigned int index = 0;
    while (index < script.values_4fc.size()) {
        const int first = script.values_4fc[index++];
        if (index >= script.values_4fc.size()) {
            return true;
        }
        const int second = script.values_4fc[index++];
        pairs_40.push_back(std::make_pair(first, second));
    }
    return false;
}

// IDA_EA: 0x85144F4
// IDA_NAME: _ZN10RecipeInfoD2Ev
// DEMANGLED: RecipeInfo::~RecipeInfo()
RecipeInfo::~RecipeInfo() {}

// IDA_EA: 0x8693A78
// IDA_NAME: _ZN10RecipeInfo12changeOutputERSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: RecipeInfo::changeOutput(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> &)
int* RecipeInfo::changeOutput(std::vector<std::pair<int, int> >& output) {
    output.clear();
    output = pairs_40;
    return reinterpret_cast<int*>(&output);
}

// IDA_EA: 0x89BA83C
// IDA_NAME: _Z16createRecipeInfoP10RecipeInfoPSt6vectorIiSaIiEEPS1_ISt4pairIiiESaIS6_EE
// DEMANGLED: createRecipeInfo(RecipeInfo *,std::vector<int,std::allocator<int>> *,std::vector*<std::pair<int,int>,std::allocator<std::pair>>)
uint8_t createRecipeInfo(
    RecipeInfo* info,
    std::vector<int>* values,
    std::vector<std::pair<int, int> >* output) {
    info->clear();

    std::vector<int>::iterator it = values->begin();
    if (it == values->end()) {
        return 1;
    }

    const int count_00 = *it;
    ++it;
    for (int index = 0; index < count_00; ++index) {
        std::pair<int, int> value;
        value.first = *it;
        ++it;
        value.second = *it;
        ++it;
        info->pairs_00.push_back(value);
    }

    const int count_0c = *it;
    ++it;
    for (int index = 0; index < count_0c; ++index) {
        std::pair<int, int> value;
        value.first = *it;
        ++it;
        value.second = *it;
        ++it;
        info->pairs_0c.push_back(value);
    }

    const int count_18 = *it;
    ++it;
    for (int index = 0; index < count_18; ++index) {
        std::pair<int, int> value;
        value.first = *it;
        ++it;
        value.second = *it;
        ++it;
        info->pairs_18.push_back(value);
    }

    if (it == values->end()) {
        return 1;
    }

    info->flag_24 = *it == 1;
    info->flag_25 = *it == 2;
    if (output) {
        info->pairs_28 = *output;
        return LowAddressByte(&info->pairs_28);
    }
    return LowAddressByte(info);
}
