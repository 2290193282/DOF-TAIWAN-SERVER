#include "old_game/player/core/character_status_data.h"


typedef char STRDCharacterStatus_Size_Must_Be_0x57[
    (sizeof(STRDCharacterStatus) == 0x57) ? 1 : -1];
typedef char STRDCharacterStatusClient_Size_Must_Be_0x98[
    (sizeof(STRDCharacterStatusClient) == 0x98) ? 1 : -1];
typedef char STRDCharacterStatusRate_Size_Must_Be_0xa4[
    (sizeof(STRDCharacterStatusRate) == 0xa4) ? 1 : -1];

// IDA_EA: 0x8367642
// IDA_NAME: _ZN19STRDCharacterStatus5ClearEv
// DEMANGLED: STRDCharacterStatus::Clear(void)
STRDCharacterStatus* STRDCharacterStatus::Clear() {
    raw_00 = 0;
    raw_04 = 0;
    raw_08 = 0;
    raw_0a = 0;
    raw_0c = 0;
    raw_0e = 0;
    for (int index = 0; index <= 3; ++index) {
        four_values[index] = 0;
    }
    for (int index = 0; index <= 16; ++index) {
        seventeen_values[index] = 0;
    }
    raw_3a = 0;
    raw_3e = 0;
    raw_40 = 0;
    raw_42 = 0;
    raw_46 = 0;
    raw_48 = 0;
    raw_4a = 0;
    raw_4c = 0;
    raw_4e = 0;
    raw_52 = 0;
    raw_53 = 0;
    return this;
}

// IDA_EA: 0x836773E
// IDA_NAME: _ZN19STRDCharacterStatusC2Ev
// DEMANGLED: STRDCharacterStatus::STRDCharacterStatus(void)
STRDCharacterStatus::STRDCharacterStatus() {
    Clear();
}

// IDA_EA: 0x8A0C942
// IDA_NAME: _ZN19STRDCharacterStatusaSERK25STRDCharacterStatusClient
// DEMANGLED: STRDCharacterStatus::operator=(STRDCharacterStatusClient const&)
STRDCharacterStatus& STRDCharacterStatus::operator=(
    const STRDCharacterStatusClient& client) {
    raw_00 = client.raw[0];
    raw_04 = client.raw[1];
    raw_08 = static_cast<uint16_t>(client.raw[2]);
    raw_0a = static_cast<uint16_t>(client.raw[3]);
    raw_0c = static_cast<uint16_t>(client.raw[4]);
    raw_0e = static_cast<uint16_t>(client.raw[5]);
    for (int index = 0; index <= 3; ++index) {
        four_values[index] = static_cast<int16_t>(client.raw[index + 6]);
    }
    for (int index = 0; index <= 16; ++index) {
        seventeen_values[index] = static_cast<int16_t>(client.raw[index + 10]);
    }
    raw_3a = client.raw[27];
    raw_3e = static_cast<int16_t>(client.raw[28]);
    raw_40 = static_cast<int16_t>(client.raw[29]);
    raw_42 = client.raw[30];
    raw_46 = static_cast<uint16_t>(client.raw[31]);
    raw_48 = static_cast<uint16_t>(client.raw[32]);
    raw_4a = static_cast<int16_t>(client.raw[33]);
    raw_4c = static_cast<int16_t>(client.raw[34]);
    raw_4e = client.raw[35];
    raw_52 = static_cast<uint8_t>(client.raw[36]);
    raw_53 = client.raw[37];
    return *this;
}

// IDA_EA: 0x8A0CAB8
// IDA_NAME: _ZN25STRDCharacterStatusClientaSERK19STRDCharacterStatus
// DEMANGLED: STRDCharacterStatusClient::operator=(STRDCharacterStatus const&)
STRDCharacterStatusClient& STRDCharacterStatusClient::operator=(
    const STRDCharacterStatus& status) {
    raw[0] = status.raw_00;
    raw[1] = status.raw_04;
    raw[2] = status.raw_08;
    raw[3] = status.raw_0a;
    raw[4] = status.raw_0c;
    raw[5] = status.raw_0e;
    for (int index = 0; index <= 3; ++index) {
        raw[index + 6] = static_cast<int>(status.four_values[index]);
    }
    for (int index = 0; index <= 16; ++index) {
        raw[index + 10] = static_cast<int>(status.seventeen_values[index]);
    }
    raw[27] = status.raw_3a;
    raw[28] = static_cast<int>(status.raw_3e);
    raw[29] = static_cast<int>(status.raw_40);
    raw[30] = status.raw_42;
    raw[31] = status.raw_46;
    raw[32] = status.raw_48;
    raw[33] = static_cast<int>(status.raw_4a);
    raw[34] = static_cast<int>(status.raw_4c);
    raw[35] = status.raw_4e;
    raw[36] = status.raw_52;
    raw[37] = status.raw_53;
    return *this;
}

// IDA_EA: 0x88CC1A0
// IDA_NAME: _ZN23STRDCharacterStatusRate5ClearEv
// DEMANGLED: STRDCharacterStatusRate::Clear(void)
STRDCharacterStatusRate* STRDCharacterStatusRate::Clear() {
    for (int index = 0; index <= 40; ++index) {
        rates[index] = 1.0f;
    }
    return this;
}

// IDA_EA: 0x88CC2FA
// IDA_NAME: _ZN23STRDCharacterStatusRateC2Ev
// DEMANGLED: STRDCharacterStatusRate::STRDCharacterStatusRate(void)
STRDCharacterStatusRate::STRDCharacterStatusRate() {
    Clear();
}
