#ifndef OLDSRV_GLOBAL_VENDING_MACHINE_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_VENDING_MACHINE_PUBLIC_ABI_H_

#include <stdint.h>

#include <utility>
#include <vector>


// CDataManager old embedded vending-machine table slot. The real
// importVendingMachineInfo parser remains the DataManager/PVF owner boundary.
// IDA unit: 99_misc_global/stVendingMachineOutput_t.
struct stVendingMachineOutput_t {
    ~stVendingMachineOutput_t();

    uint32_t item_index;
    uint32_t cumulative_rate;
    uint32_t item_count;
};

static_assert(sizeof(stVendingMachineOutput_t) == 0x0Cu,
              "stVendingMachineOutput_t IDA ABI size must stay 0x0c");

// IDA unit: 99_misc_global/stVendingMachineInfo_t.
struct stVendingMachineInfo_t {
    stVendingMachineInfo_t();
    stVendingMachineInfo_t(const stVendingMachineInfo_t& rhs);
    ~stVendingMachineInfo_t();

    stVendingMachineInfo_t& operator=(const stVendingMachineInfo_t& rhs);
    int clear();

    int32_t group_num;
    int32_t material_item;
    int32_t material_count;
    int32_t normal_total_rate;
    std::vector<stVendingMachineOutput_t> normal_outputs;
    int32_t bonus_total_rate;
    std::vector<stVendingMachineOutput_t> bonus_outputs;
    std::vector<std::pair<int, int> > dictionary_items;
};

static_assert(sizeof(stVendingMachineInfo_t) == 0x38u,
              "stVendingMachineInfo_t IDA ABI size must stay 0x38");

// IDA unit: 99_misc_global/stVendingMachine_t.
struct stVendingMachine_t {
    stVendingMachine_t();
    stVendingMachine_t(const stVendingMachine_t& rhs);
    ~stVendingMachine_t();

    stVendingMachine_t& operator=(const stVendingMachine_t& rhs);

    int32_t script_index;
    std::vector<stVendingMachineInfo_t> groups;
};

static_assert(sizeof(stVendingMachine_t) == 0x10u,
              "stVendingMachine_t IDA ABI size must stay 0x10");


#endif  // OLDSRV_GLOBAL_VENDING_MACHINE_PUBLIC_ABI_H_
