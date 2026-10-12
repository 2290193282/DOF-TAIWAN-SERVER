#ifndef OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_ST_STACKABLE_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_ST_STACKABLE_SCRIPT_H_

#include <stdint.h>

#include <string>


// IDA_EA: 0x89C33FA, 0x89C348E, 0x89C3A46, 0x89C4052.
// IDA unit: 12_pvf_datamanager_scripts/Arad_STStackableScript.
// Old i386 size: 0x14.
class Arad_STStackableScript {
public:
    Arad_STStackableScript();
    ~Arad_STStackableScript();

    Arad_STStackableScript& operator=(const Arad_STStackableScript& other);
    Arad_STStackableScript* AradStClear();

    int32_t value_00;
    uint8_t value_04;
    std::string value_08;
    std::string value_0C;
    std::string value_10;
};

static_assert(sizeof(Arad_STStackableScript) == 0x14u,
              "Arad_STStackableScript IDA ABI size must stay 0x14");

#endif  // OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_ST_STACKABLE_SCRIPT_H_
