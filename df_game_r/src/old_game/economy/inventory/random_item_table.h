#ifndef OLDSRV_OLD_GAME_INVENTORY_RANDOM_ITEM_TABLE_H_
#define OLDSRV_OLD_GAME_INVENTORY_RANDOM_ITEM_TABLE_H_

#include <utility>
#include <vector>

// IDA unit: 99_misc_global/RandomItemTable. This value type is embedded in
// CEquipItem and converts alternating item/weight values into cumulative
// weighted-selection boundaries.
class RandomItemTable {
public:
    // IDA_EA: 0x8515468
    // IDA_NAME: _ZN15RandomItemTableC2Ev
    // DEMANGLED: RandomItemTable::RandomItemTable(void)
    RandomItemTable();

    // IDA_EA: 0x8514B46
    // IDA_NAME: _ZN15RandomItemTableD2Ev
    // DEMANGLED: RandomItemTable::~RandomItemTable()
    ~RandomItemTable();

    // IDA_EA: 0x89BBA40
    // IDA_NAME: _ZN15RandomItemTable3SetEPSt6vectorIiSaIiEE
    // DEMANGLED: RandomItemTable::Set(std::vector<int,std::allocator<int>> *)
    bool Set(std::vector<int>* item_and_weight_values);

    // IDA_EA: 0x89BBB6C
    // IDA_NAME: _ZNK15RandomItemTable18GetRandomItemIndexEi
    // DEMANGLED: RandomItemTable::GetRandomItemIndex(int)const
    int GetRandomItemIndex(int random_value) const;

    // The converter draws in the same [0,total_weight_) domain used to build
    // cumulative_items_.  This inline view exposes the physical named member
    // without creating a new exported ABI symbol.
    int GetTotalWeight() const { return total_weight_; }

private:
    int total_weight_;
    std::vector<std::pair<int, int> > cumulative_items_;
};

static_assert(sizeof(RandomItemTable) == 0x10u,
              "RandomItemTable IDA ABI size must stay 0x10");

#endif  // OLDSRV_OLD_GAME_INVENTORY_RANDOM_ITEM_TABLE_H_
