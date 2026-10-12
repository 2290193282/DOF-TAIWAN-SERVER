#include "old_game/economy/mall/auction_public_values.h"

// IDA_EA: 0x822AC10
// IDA_NAME: _ZN12ROI_CategoryC2Ev
// DEMANGLED: ROI_Category::ROI_Category(void)
ROI_Category::ROI_Category() {
    category_0 = 0;
    category_1 = 0;
    category_2 = 0;
    reserved_06[0] = 0;
    reserved_06[1] = 0;
    flag_08 = 0;
    flag_09 = 0;
    flag_0a = 0;
    reserved_0b = 0;
}

// IDA_EA: 0x822AC30
// IDA_NAME: _ZN12ROI_Category5_sortEv
// DEMANGLED: ROI_Category::_sort(void)
ROI_Category* ROI_Category::_sort() {
    if (category_0 > category_1) {
        const int16_t category = category_0;
        category_0 = category_1;
        category_1 = category;

        const uint8_t flag = flag_08;
        flag_08 = flag_09;
        flag_09 = flag;
    }
    if (category_1 > category_2) {
        const int16_t category = category_1;
        category_1 = category_2;
        category_2 = category;

        const uint8_t flag = flag_09;
        flag_09 = flag_0a;
        flag_0a = flag;
    }

    ROI_Category* result =
        reinterpret_cast<ROI_Category*>(
            static_cast<uintptr_t>(
                static_cast<uint16_t>(category_1)));
    if (category_0 > static_cast<int16_t>(category_1)) {
        const int16_t category = category_0;
        category_0 = category_1;
        category_1 = category;

        const uint8_t flag = flag_08;
        flag_08 = flag_09;
        result = this;
        flag_09 = flag;
    }
    return result;
}

// IDA_EA: 0x84E9094
// IDA_NAME: _ZN11DnfItemInfo5resetEv
// DEMANGLED: DnfItemInfo::reset(void)
ReservedCapacity* DnfItemInfo::reset() {
    seal_or_state_ = 0;
    item_id_ = 0;
    item_attr_ = 0;
    add_info_ = 0;
    avatar_select_ = 0;
    extend_info_ = 0;
    amplify_type_ = 0;
    amplify_value_ = 0;
    random_option_.reset();
    upgrade_separate_.reset();
    return reserved_capacity_.reset();
}

// IDA_EA: 0x84E910A
// IDA_NAME: _ZN11DnfItemInfoC2Ev
// DEMANGLED: DnfItemInfo::DnfItemInfo(void)
DnfItemInfo::DnfItemInfo()
    : upgrade_separate_(), reserved_capacity_() {
    reset();
}

// IDA_EA: 0x822E694
// IDA_NAME: _ZN16TSearchByItemId_C2Ev
// DEMANGLED: TSearchByItemId_::TSearchByItemId_(void)
TSearchByItemId_::TSearchByItemId_() : category_() {}

// IDA_EA: 0x822E720
// IDA_NAME: _ZN18TSearchByCategory_C2Ev
// DEMANGLED: TSearchByCategory_::TSearchByCategory_(void)
TSearchByCategory_::TSearchByCategory_() : category_() {}
