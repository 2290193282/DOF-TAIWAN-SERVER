#ifndef OLDSRV_DATAMANAGER_PET_SCRIPT_H_
#define OLDSRV_DATAMANAGER_PET_SCRIPT_H_

#include <stdint.h>

#include <string>
#include <vector>


// IDA_EA: 0x8A3DBAA, 0x8A3581B, 0x8A358D7.
// IDA unit: 12_pvf_datamanager_scripts/PetScript.
class PetScript {
public:
    PetScript* clear();

    int32_t value_00;
    std::string value_04;
    int32_t value_08;
    int32_t value_0C;
    int32_t value_10;
    int32_t value_14;
    int32_t value_18;
    uint8_t value_1C;
    uint8_t padding_1D;
    uint8_t padding_1E;
    uint8_t padding_1F;
    int32_t value_20[4];
    int32_t value_30;
    int32_t value_34;
    std::string value_38;
    std::vector<std::string> value_3C;
    std::vector<std::string> value_48;
    std::vector<int> value_54;
    std::vector<std::string> value_60;
    std::string value_6C;
};

bool initPetScript(const char* src, const char* filename);
int importPetScript(PetScript* script, const char* filename);

#endif  // OLDSRV_DATAMANAGER_PET_SCRIPT_H_
