#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_STATISTICS_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_STATISTICS_H_

#include <stdint.h>

// IDA stream records used by advancealtar::DB_StatisticAdvanceAltar.
struct STAdvanceAltarEntranceData {
    STAdvanceAltarEntranceData();

    uint32_t stage_id;        // +0x00
    uint32_t entrance_count;  // +0x04
    uint32_t success_count;   // +0x08
    uint32_t obtain_star;     // +0x0C
};

// The old i386 object has natural alignment: +0x02..+0x03 and +0x0E..+0x0F
// are padding.  They are not initialized by the original constructor.
struct STAdvanceAltarLayerStatData {
    STAdvanceAltarLayerStatData();

    uint16_t item_type;  // +0x00
    uint32_t item_id;    // +0x04
    uint16_t item_level; // +0x08
    uint16_t use_count;  // +0x0A
    uint16_t use_star;   // +0x0C
};

static_assert(sizeof(STAdvanceAltarEntranceData) == 0x10u,
              "STAdvanceAltarEntranceData IDA ABI size must stay 0x10");
static_assert(sizeof(STAdvanceAltarLayerStatData) == 0x10u,
              "STAdvanceAltarLayerStatData IDA ABI size must stay 0x10");

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_STATISTICS_H_
