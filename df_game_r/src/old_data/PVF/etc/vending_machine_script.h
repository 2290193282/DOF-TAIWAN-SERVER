#pragma once

#include "old_game/economy/trade/vending_machine_public_abi.h"

#include <vector>


// IDA_EA: 0x890B2A3
// IDA_NAME: _Z28importVendingMachineInfoCorePKcS0_R18stVendingMachine_t
// DEMANGLED: importVendingMachineInfoCore(char const*,char const*,stVendingMachine_t&)
int importVendingMachineInfoCore(const char* dir,
                                 const char* filename,
                                 stVendingMachine_t& script);

// IDA_EA: 0x890B74E
// IDA_NAME: _Z24importVendingMachineInfoPKcS0_RSt6vectorI18stVendingMachine_tSaIS2_EE
// DEMANGLED: importVendingMachineInfo(char const*,char const*,std::vector<stVendingMachine_t>&)
int importVendingMachineInfo(
    const char* dir,
    const char* filename,
    std::vector<stVendingMachine_t>& table);
