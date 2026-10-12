#include "old_game/pvp/battlefield/war_field_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x86B9FE6
// IDA_NAME: _ZN8WarField12GetFieldItemEiR8map_item
// DEMANGLED: WarField::GetFieldItem(int,map_item &)
int WarField::GetFieldItem(int unique_id, map_item& item) const {
    std::map<int, map_item>::const_iterator found = map_info_.items_.find(unique_id);
    if (found == map_info_.items_.end()) return 0;
    item = found->second;
    return 1;
}

// IDA_EA: 0x86BA082
// IDA_NAME: _ZN8WarField10PickupItemESt17_Rb_tree_iteratorISt4pairIKi8map_itemEE
// DEMANGLED: WarField::PickupItem(std::_Rb_tree_iterator<std::pair<int const,map_item>>)
int WarField::PickupItem(std::map<int, map_item>::iterator item) {
    if (item == map_info_.items_.end()) return 0;
    map_info_.items_.erase(item);
    return 1;
}

// IDA_EA: 0x86BA0A0
// IDA_NAME: _ZN8WarField15CheckPickupItemEiiRSt17_Rb_tree_iteratorISt4pairIKi8map_itemEE
// DEMANGLED: WarField::CheckPickupItem(int,int,std::_Rb_tree_iterator<std::pair<int const,map_item>> &)
int WarField::CheckPickupItem(int unique_id,
                              int picker_unique_id,
                              std::map<int, map_item>::iterator& item) {
    std::map<int, map_item>::iterator found = map_info_.items_.find(unique_id);
    if (found == map_info_.items_.end()) return 21;

    const int current_second = GlobalData::s_systemTime_.getCurSec();
    if (current_second - static_cast<int>(found->second.created_at_sec_) <= 9 &&
        found->second.monster_type_ != 0xffffu &&
        static_cast<int>(found->second.monster_type_) != picker_unique_id) {
        return 28;
    }
    item = found;
    return 0;
}
