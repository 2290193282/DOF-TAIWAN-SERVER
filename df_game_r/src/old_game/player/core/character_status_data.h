#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_STATUS_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_STATUS_DATA_H_

#include <stdint.h>


struct STRDCharacterStatusClient;

// IDA unit: 04_character_select_create/STRDCharacterStatus.
#pragma pack(push, 1)
struct STRDCharacterStatus {
    STRDCharacterStatus();

    STRDCharacterStatus* Clear();
    STRDCharacterStatus& operator=(const STRDCharacterStatusClient& client);

    uint32_t raw_00;
    uint32_t raw_04;
    uint16_t raw_08;
    uint16_t raw_0a;
    uint16_t raw_0c;
    uint16_t raw_0e;
    int16_t four_values[4];
    int16_t seventeen_values[17];
    uint32_t raw_3a;
    int16_t raw_3e;
    int16_t raw_40;
    uint32_t raw_42;
    uint16_t raw_46;
    uint16_t raw_48;
    int16_t raw_4a;
    int16_t raw_4c;
    uint32_t raw_4e;
    uint8_t raw_52;
    uint32_t raw_53;
};
#pragma pack(pop)

// IDA unit: 04_character_select_create/STRDCharacterStatusClient.
struct STRDCharacterStatusClient {
    STRDCharacterStatusClient& operator=(const STRDCharacterStatus& status);

    uint32_t raw[38];
};

// IDA unit: 04_character_select_create/STRDCharacterStatusRate.
struct STRDCharacterStatusRate {
    STRDCharacterStatusRate();

    STRDCharacterStatusRate* Clear();

    float rates[41];
};


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_STATUS_DATA_H_
