#include "old_game/activity/reward/advance_altar_statistics.h"

// IDA_EA: 0x81573BE
// IDA_NAME: _ZN26STAdvanceAltarEntranceDataC2Ev
// DEMANGLED: STAdvanceAltarEntranceData::STAdvanceAltarEntranceData(void)
STAdvanceAltarEntranceData::STAdvanceAltarEntranceData()
    : stage_id(0), entrance_count(0), success_count(0), obtain_star(0) {}

// IDA_EA: 0x81573EA
// IDA_NAME: _ZN27STAdvanceAltarLayerStatDataC2Ev
// DEMANGLED: STAdvanceAltarLayerStatData::STAdvanceAltarLayerStatData(void)
STAdvanceAltarLayerStatData::STAdvanceAltarLayerStatData()
    : item_type(0), item_id(0), item_level(0), use_count(0), use_star(0) {}
