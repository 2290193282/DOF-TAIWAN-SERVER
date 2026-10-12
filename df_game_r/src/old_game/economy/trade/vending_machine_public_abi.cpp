#include "old_game/economy/trade/vending_machine_public_abi.h"


// IDA_EA: 0x893DA64
// IDA_NAME: _ZN24stVendingMachineOutput_tD2Ev
// DEMANGLED: stVendingMachineOutput_t::~stVendingMachineOutput_t()
stVendingMachineOutput_t::~stVendingMachineOutput_t() {
}

// IDA_EA: 0x891B36A
// IDA_NAME: _ZN22stVendingMachineInfo_tC2Ev
// DEMANGLED: stVendingMachineInfo_t::stVendingMachineInfo_t(void)
stVendingMachineInfo_t::stVendingMachineInfo_t()
    : normal_outputs(), bonus_outputs(), dictionary_items() {
    clear();
}

// IDA_EA: 0x892AD04
// IDA_NAME: _ZN22stVendingMachineInfo_tC2ERKS_
// DEMANGLED: stVendingMachineInfo_t::stVendingMachineInfo_t(stVendingMachineInfo_t const&)
stVendingMachineInfo_t::stVendingMachineInfo_t(
    const stVendingMachineInfo_t& rhs)
    : group_num(rhs.group_num),
      material_item(rhs.material_item),
      material_count(rhs.material_count),
      normal_total_rate(rhs.normal_total_rate),
      normal_outputs(rhs.normal_outputs),
      bonus_total_rate(rhs.bonus_total_rate),
      bonus_outputs(rhs.bonus_outputs),
      dictionary_items(rhs.dictionary_items) {
}

// IDA_EA: 0x83F2F3A
// IDA_NAME: _ZN22stVendingMachineInfo_tD2Ev
// DEMANGLED: stVendingMachineInfo_t::~stVendingMachineInfo_t()
stVendingMachineInfo_t::~stVendingMachineInfo_t() {
}

// IDA_EA: 0x892AE52
// IDA_NAME: _ZN22stVendingMachineInfo_taSERKS_
// DEMANGLED: stVendingMachineInfo_t::operator=(stVendingMachineInfo_t const&)
stVendingMachineInfo_t& stVendingMachineInfo_t::operator=(
    const stVendingMachineInfo_t& rhs) {
    group_num = rhs.group_num;
    material_item = rhs.material_item;
    material_count = rhs.material_count;
    normal_total_rate = rhs.normal_total_rate;
    normal_outputs = rhs.normal_outputs;
    bonus_total_rate = rhs.bonus_total_rate;
    bonus_outputs = rhs.bonus_outputs;
    dictionary_items = rhs.dictionary_items;
    return *this;
}

// IDA_EA: 0x891B3FE
// IDA_NAME: _ZN22stVendingMachineInfo_t5clearEv
// DEMANGLED: stVendingMachineInfo_t::clear(void)
int stVendingMachineInfo_t::clear() {
    group_num = 0;
    material_item = -1;
    material_count = 0;
    normal_total_rate = 0;
    normal_outputs.clear();
    bonus_total_rate = 0;
    bonus_outputs.clear();
    return 0;
}

// IDA_EA: 0x891C820
// IDA_NAME: _ZN18stVendingMachine_tC2Ev
// DEMANGLED: stVendingMachine_t::stVendingMachine_t(void)
stVendingMachine_t::stVendingMachine_t() : script_index(0), groups() {
}

// IDA_EA: 0x892B55C
// IDA_NAME: _ZN18stVendingMachine_tC2ERKS_
// DEMANGLED: stVendingMachine_t::stVendingMachine_t(stVendingMachine_t const&)
stVendingMachine_t::stVendingMachine_t(const stVendingMachine_t& rhs)
    : script_index(rhs.script_index), groups(rhs.groups) {
}

// IDA_EA: 0x83E636C
// IDA_NAME: _ZN18stVendingMachine_tD2Ev
// DEMANGLED: stVendingMachine_t::~stVendingMachine_t()
stVendingMachine_t::~stVendingMachine_t() {
}

// IDA_EA: 0x892B60A
// IDA_NAME: _ZN18stVendingMachine_taSERKS_
// DEMANGLED: stVendingMachine_t::operator=(stVendingMachine_t const&)
stVendingMachine_t& stVendingMachine_t::operator=(
    const stVendingMachine_t& rhs) {
    script_index = rhs.script_index;
    groups = rhs.groups;
    return *this;
}
