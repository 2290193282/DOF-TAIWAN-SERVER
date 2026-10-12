#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_KEY_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_KEY_H_

#include <stdint.h>

struct combo_script_key {
    combo_script_key();

    bool operator<(const combo_script_key& rhs) const;
    combo_script_key* clear();

    uint8_t job;
    uint8_t padding_01;
    uint16_t skill_index;
};

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_COMBO_SCRIPT_KEY_H_
