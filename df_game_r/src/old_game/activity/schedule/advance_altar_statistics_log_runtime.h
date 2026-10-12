#ifndef OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STATISTICS_LOG_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STATISTICS_LOG_RUNTIME_H_

#include <stdint.h>

#include <map>

#include "old_game/activity/reward/advance_altar_statistics.h"
#include "infra/concurrent/thread/mutex.h"

// IDA unit: 99_misc_global/CAdvanceAltarLog.  This is the real process-wide
// accumulator consumed by advancealtar::DB_StatisticAdvanceAltar, rather than
// a CMD573-only accounting facade.
class CAdvanceAltarLog {
public:
    // IDA_EA: 0x8156AAA / 0x8156B38
    // IDA_NAME: _ZN16CAdvanceAltarLogC2Ev / _ZN16CAdvanceAltarLogD2Ev
    CAdvanceAltarLog();
    ~CAdvanceAltarLog();

    // IDA_EA: 0x8156BCA
    // IDA_NAME: _ZN16CAdvanceAltarLog14initStatisticsEv
    int initStatistics();
    // IDA_EA: 0x8156C30
    // IDA_NAME: _ZN16CAdvanceAltarLog21incrementEntranceDataEjjjj
    int incrementEntranceData(uint32_t stage_id,
                              uint32_t entrance_count,
                              uint32_t success_count,
                              uint32_t obtain_star);
    // IDA_EA: 0x8156D60
    // IDA_NAME: _ZN16CAdvanceAltarLog22incrementLayerStatDataEtjttt
    int incrementLayerStatData(uint16_t item_type,
                               uint32_t item_id,
                               uint16_t item_level,
                               uint16_t use_count,
                               uint16_t use_star);
    // IDA_EA: 0x8156ED2
    // IDA_NAME: _ZN16CAdvanceAltarLog14updateDatabaseEv
    int updateDatabase();

    // IDA_EA: 0x8157298 / 0x81572EA / 0x815730E / 0x8157336.
    static int64_t makeKey(uint16_t item_type,
                           uint32_t item_id,
                           uint16_t item_level);
    static uint16_t getType(int64_t key);
    static uint32_t getIndex(int64_t key);
    static uint16_t getLevel(int64_t key);

private:
    Mutex mutex_;  // +0x00
    std::map<uint32_t, STAdvanceAltarEntranceData> entrance_data_;  // +0x18
    std::map<int64_t, STAdvanceAltarLayerStatData> layer_data_;     // +0x30
};

#if defined(__i386__)
static_assert(sizeof(CAdvanceAltarLog) == 0x48u,
              "CAdvanceAltarLog old-server ABI size must stay 0x48");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_ADVANCE_ALTAR_STATISTICS_LOG_RUNTIME_H_
