#ifndef OLDSRV_GLOBAL_ENCHANT_SYSTEM_DATA_H_
#define OLDSRV_GLOBAL_ENCHANT_SYSTEM_DATA_H_

#include <stdint.h>


// IDA unit: 99_misc_global/STEnchantSystemData. Public base value consumed by
// IDA_EA: ctor/dtor/clear @0x854B194/0x854B1A8/0x89D17A2.
// C-owned world enchant map key/data records; map containers remain owner work.
struct STEnchantSystemData {
    STEnchantSystemData();
    ~STEnchantSystemData();

    STEnchantSystemData* clear();

    int32_t value_00;  // +0x00
    uint8_t value_04;  // +0x04
    uint8_t pad_05[3]; // +0x05
    int32_t value_08;  // +0x08
};

static_assert(sizeof(STEnchantSystemData) == 0x0Cu,
              "STEnchantSystemData IDA ABI size must stay 0x0C");


#endif  // OLDSRV_GLOBAL_ENCHANT_SYSTEM_DATA_H_
