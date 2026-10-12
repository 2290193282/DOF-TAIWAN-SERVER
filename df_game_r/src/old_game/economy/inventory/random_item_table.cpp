#include "old_game/economy/inventory/random_item_table.h"

// IDA_EA: 0x8515468
// IDA_NAME: _ZN15RandomItemTableC2Ev
// DEMANGLED: RandomItemTable::RandomItemTable(void)
RandomItemTable::RandomItemTable() {}

// IDA_EA: 0x8514B46
// IDA_NAME: _ZN15RandomItemTableD2Ev
// DEMANGLED: RandomItemTable::~RandomItemTable()
RandomItemTable::~RandomItemTable() {}

// IDA_EA: 0x89BBA40
// IDA_NAME: _ZN15RandomItemTable3SetEPSt6vectorIiSaIiEE
// DEMANGLED: RandomItemTable::Set(std::vector<int,std::allocator<int>> *)
bool RandomItemTable::Set(std::vector<int>* item_and_weight_values) {
    cumulative_items_.clear();
    total_weight_ = 0;

    std::vector<int>::iterator it = item_and_weight_values->begin();
    while (it != item_and_weight_values->end()) {
        std::pair<int, int> entry;
        entry.first = *it++;
        if (it == item_and_weight_values->end()) {
            return true;
        }

        total_weight_ += *it++;
        entry.second = total_weight_;
        cumulative_items_.push_back(entry);
    }
    return false;
}

// IDA_EA: 0x89BBB6C
// IDA_NAME: _ZNK15RandomItemTable18GetRandomItemIndexEi
// DEMANGLED: RandomItemTable::GetRandomItemIndex(int)const
int RandomItemTable::GetRandomItemIndex(int random_value) const {
    for (std::vector<std::pair<int, int> >::const_iterator it =
             cumulative_items_.begin();
         it != cumulative_items_.end();
         ++it) {
        if (it->second > random_value) {
            return it->first;
        }
    }
    return 0;
}
