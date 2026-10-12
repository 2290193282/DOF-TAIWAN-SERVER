#include "old_game/economy/equipment/enchant_system_data.h"


// IDA_EA: 0x854B194
// IDA_NAME: _ZN19STEnchantSystemDataC2Ev
// DEMANGLED: STEnchantSystemData::STEnchantSystemData(void)
STEnchantSystemData::STEnchantSystemData() {
    clear();
}

// IDA_EA: 0x854B1A8
// IDA_NAME: _ZN19STEnchantSystemData5clearEv
// DEMANGLED: STEnchantSystemData::clear(void)
STEnchantSystemData* STEnchantSystemData::clear() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    return this;
}

// IDA_EA: 0x89D17A2
// IDA_NAME: _ZN19STEnchantSystemDataD2Ev
// DEMANGLED: STEnchantSystemData::~STEnchantSystemData()
STEnchantSystemData::~STEnchantSystemData() {
}
