#ifndef OLDSRV_DB_ARAD_DISPATCHER_REQUEST_H_
#define OLDSRV_DB_ARAD_DISPATCHER_REQUEST_H_

#include <stdint.h>

#pragma pack(push, 1)

namespace arad {

// IDA unit: 04_character_select_create/ARAD/SigSetCharacCreateTime.
struct SigSetCharacCreateTime {
    SigSetCharacCreateTime();

    int init();
    SigSetCharacCreateTime* set(uint32_t charac_no_value,
                                uint32_t create_time_value);

    uint32_t charac_no;
    uint32_t create_time;
};

// IDA unit: 06_userinfo_stat_inventory/ARAD/SigAradAvatarHiddenOptionSave.
struct SigAradAvatarHiddenOptionSave {
    SigAradAvatarHiddenOptionSave();

    SigAradAvatarHiddenOptionSave* init();

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t item_id;
    int16_t slot;
    int16_t hidden_option;
};

// IDA unit: 06_userinfo_stat_inventory/ARAD/SigAradAvatarEnduranceSave.
struct SigAradAvatarEnduranceSave {
    SigAradAvatarEnduranceSave();

    SigAradAvatarEnduranceSave* init();

    uint32_t ui_id;
    int32_t endurance;
};

}  // namespace arad

#pragma pack(pop)

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(arad::SigSetCharacCreateTime) == 0x08u,
              "SigSetCharacCreateTime IDA ABI size must stay 0x08");
static_assert(sizeof(arad::SigAradAvatarHiddenOptionSave) == 0x10u,
              "SigAradAvatarHiddenOptionSave IDA ABI size must stay 0x10");
static_assert(sizeof(arad::SigAradAvatarEnduranceSave) == 0x08u,
              "SigAradAvatarEnduranceSave IDA ABI size must stay 0x08");
#endif

#endif  // OLDSRV_DB_ARAD_DISPATCHER_REQUEST_H_
