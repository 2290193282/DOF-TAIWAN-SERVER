#ifndef OLDSRV_OLD_GAME_INVENTORY_ARAD_ITEM_INFO_H_
#define OLDSRV_OLD_GAME_INVENTORY_ARAD_ITEM_INFO_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "old_data/PVF/stackable/stackable_script_types.h"

// IDA unit: 99_misc_global/AradItemInfo.
struct AradItemInfo {
    AradItemInfo();
    ~AradItemInfo();

    int32_t values_00[2];                 // +0x00
    std::string string_08;                // +0x08
    std::string string_0c;                // +0x0c
    std::string string_10;                // +0x10
    std::vector<int> values_14;           // +0x14
    std::vector<CRabbitBox> rabbit_20;     // +0x20
};

static_assert(sizeof(AradItemInfo) == 0x2cu,
              "AradItemInfo IDA ABI size must stay 0x2C");
static_assert(offsetof(AradItemInfo, string_08) == 0x08u &&
                  offsetof(AradItemInfo, string_0c) == 0x0cu &&
                  offsetof(AradItemInfo, string_10) == 0x10u &&
                  offsetof(AradItemInfo, values_14) == 0x14u &&
                  offsetof(AradItemInfo, rabbit_20) == 0x20u,
              "AradItemInfo IDA ABI offsets must stay exact");

#endif  // OLDSRV_OLD_GAME_INVENTORY_ARAD_ITEM_INFO_H_
