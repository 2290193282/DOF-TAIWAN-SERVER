#include "old_game/activity/schedule/advance_altar_statistics_log_runtime.h"

#include "old_game/activity/reward/advance_altar.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/thread/guard_mutex.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <cstring>

// IDA_EA: 0x8156AAA
// IDA_NAME: _ZN16CAdvanceAltarLogC2Ev
// DEMANGLED: CAdvanceAltarLog::CAdvanceAltarLog(void)
CAdvanceAltarLog::CAdvanceAltarLog()
    : mutex_(), entrance_data_(), layer_data_() {
    initStatistics();
}

// IDA_EA: 0x8156B38
// IDA_NAME: _ZN16CAdvanceAltarLogD2Ev
// DEMANGLED: CAdvanceAltarLog::~CAdvanceAltarLog()
CAdvanceAltarLog::~CAdvanceAltarLog() {
    initStatistics();
}

// IDA_EA: 0x8156BCA
// IDA_NAME: _ZN16CAdvanceAltarLog14initStatisticsEv
// DEMANGLED: CAdvanceAltarLog::initStatistics(void)
int CAdvanceAltarLog::initStatistics() {
    Guard<Mutex> guard(&mutex_);
    entrance_data_.clear();
    layer_data_.clear();
    return 0;
}

// IDA_EA: 0x8156C30
// IDA_NAME: _ZN16CAdvanceAltarLog21incrementEntranceDataEjjjj
// DEMANGLED: CAdvanceAltarLog::incrementEntranceData(unsigned int,unsigned int,unsigned int,unsigned int)
int CAdvanceAltarLog::incrementEntranceData(uint32_t stage_id,
                                            uint32_t entrance_count,
                                            uint32_t success_count,
                                            uint32_t obtain_star) {
    Guard<Mutex> guard(&mutex_);
    std::map<uint32_t, STAdvanceAltarEntranceData>::iterator found =
        entrance_data_.find(stage_id);
    if (found == entrance_data_.end()) {
        STAdvanceAltarEntranceData& entry = entrance_data_[stage_id];
        entry.stage_id = stage_id;
        entry.entrance_count += entrance_count;
        entry.success_count += success_count;
        entry.obtain_star += obtain_star;
    } else {
        found->second.entrance_count += entrance_count;
        found->second.success_count += success_count;
        found->second.obtain_star += obtain_star;
    }
    return 0;
}

// IDA_EA: 0x8156D60
// IDA_NAME: _ZN16CAdvanceAltarLog22incrementLayerStatDataEtjttt
// DEMANGLED: CAdvanceAltarLog::incrementLayerStatData(unsigned short,unsigned int,unsigned short,unsigned short,unsigned short)
int CAdvanceAltarLog::incrementLayerStatData(uint16_t item_type,
                                             uint32_t item_id,
                                             uint16_t item_level,
                                             uint16_t use_count,
                                             uint16_t use_star) {
    Guard<Mutex> guard(&mutex_);
    const int64_t key = makeKey(item_type, item_id, item_level);
    std::map<int64_t, STAdvanceAltarLayerStatData>::iterator found =
        layer_data_.find(key);
    if (found == layer_data_.end()) {
        STAdvanceAltarLayerStatData& entry = layer_data_[key];
        entry.item_type = item_type;
        entry.item_id = item_id;
        entry.item_level = item_level;
        entry.use_count += use_count;
        entry.use_star += use_star;
    } else {
        found->second.use_count += use_count;
        found->second.use_star += use_star;
    }
    return 0;
}

// IDA_EA: 0x8156ED2
// IDA_NAME: _ZN16CAdvanceAltarLog14updateDatabaseEv
// DEMANGLED: CAdvanceAltarLog::updateDatabase(void)
int CAdvanceAltarLog::updateDatabase() {
    Guard<Mutex> guard(&mutex_);
    CStreamGuard stream_guard(
        GlobalData::s_stream_pool->Acquire("localglobal/global_Statistics.cpp",
                                           229),
        true);
    *stream_guard << 827;
    *stream_guard << -1;

    *stream_guard << static_cast<int>(entrance_data_.size());
    for (std::map<uint32_t, STAdvanceAltarEntranceData>::const_iterator it =
             entrance_data_.begin();
         it != entrance_data_.end(); ++it) {
        STAdvanceAltarEntranceData* const output =
            stream_guard->GetInBuffer<STAdvanceAltarEntranceData>();
        std::memcpy(output, &it->second, 0x10u);
        cMyTrace trace("void CAdvanceAltarLog::updateDatabase()", 244, 0);
        trace("[AdvanceAltar] stageIndex:%d, enterCount:%d, successCount:%d, obtainStar:%d",
              it->second.stage_id,
              it->second.entrance_count,
              it->second.success_count,
              it->second.obtain_star);
    }
    entrance_data_.clear();

    *stream_guard << static_cast<int>(layer_data_.size());
    for (std::map<int64_t, STAdvanceAltarLayerStatData>::const_iterator it =
             layer_data_.begin();
         it != layer_data_.end(); ++it) {
        STAdvanceAltarLayerStatData* const output =
            stream_guard->GetInBuffer<STAdvanceAltarLayerStatData>();
        std::memcpy(output, &it->second, 0x10u);
        cMyTrace trace("void CAdvanceAltarLog::updateDatabase()", 262, 0);
        trace("[AdvanceAltar] itemType:%u, itemIndex:%u, itemLevel:%u, useCount:%u, useStar:%u",
              static_cast<unsigned int>(it->second.item_type),
              it->second.item_id,
              static_cast<unsigned int>(it->second.item_level),
              static_cast<unsigned int>(it->second.use_count),
              static_cast<unsigned int>(it->second.use_star));
    }
    layer_data_.clear();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream_guard);
    return 0;
}

// IDA_EA: 0x8157298
// IDA_NAME: _ZN16CAdvanceAltarLog7makeKeyEtjt
// DEMANGLED: CAdvanceAltarLog::makeKey(unsigned short,unsigned int,unsigned short)
int64_t CAdvanceAltarLog::makeKey(uint16_t item_type,
                                  uint32_t item_id,
                                  uint16_t item_level) {
    const uint32_t high = (static_cast<uint32_t>(item_type) << 16) |
                          static_cast<uint32_t>(item_level);
    return static_cast<int64_t>((static_cast<uint64_t>(high) << 32) |
                                static_cast<uint64_t>(item_id));
}

// IDA_EA: 0x81572EA
// IDA_NAME: _ZN16CAdvanceAltarLog7getTypeEx
// DEMANGLED: CAdvanceAltarLog::getType(long long)
uint16_t CAdvanceAltarLog::getType(int64_t key) {
    return static_cast<uint16_t>(static_cast<uint64_t>(key) >> 48);
}

// IDA_EA: 0x815730E
// IDA_NAME: _ZN16CAdvanceAltarLog8getIndexEx
// DEMANGLED: CAdvanceAltarLog::getIndex(long long)
uint32_t CAdvanceAltarLog::getIndex(int64_t key) {
    return static_cast<uint32_t>(key);
}

// IDA_EA: 0x8157336
// IDA_NAME: _ZN16CAdvanceAltarLog8getLevelEx
// DEMANGLED: CAdvanceAltarLog::getLevel(long long)
uint16_t CAdvanceAltarLog::getLevel(int64_t key) {
    return static_cast<uint16_t>(static_cast<uint64_t>(key) >> 32);
}

namespace advancealtar {

// IDA data: _ZN12advancealtar10HistoryLog22advanceAltarstatistics_E
CAdvanceAltarLog HistoryLog::advanceAltarstatistics_;

// IDA_EA: 0x81339B0
// IDA_NAME: _ZN12advancealtar10HistoryLog12enterDungeonER5CUserRKNS_9ProcStageE
// DEMANGLED: advancealtar::HistoryLog::enterDungeon(CUser &,advancealtar::ProcStage const&)
int HistoryLog::enterDungeon(CUser& user, const ProcStage& stage) {
    char member_names[256] = {};
    getMemberNames(&user, member_names);
    user.history_log().EnterDungeon(stage.getStageIndex(),
                                    stage.getStageDifficulty(),
                                    1,
                                    member_names,
                                    2,
                                    0);
    return advanceAltarstatistics_.incrementEntranceData(
        static_cast<uint32_t>(stage.getStageIndex()), 1u, 0u, 0u);
}

// IDA_EA: 0x8133ABC
// IDA_NAME: _ZN12advancealtar10HistoryLog12leaveDungeonER5CUserRKNS_9ProcStageE
// DEMANGLED: advancealtar::HistoryLog::leaveDungeon(CUser &,advancealtar::ProcStage const&)
int HistoryLog::leaveDungeon(CUser& user, const ProcStage& stage) {
    char member_names[256] = {};
    getMemberNames(&user, member_names);
    return user.history_log().LeaveDungeon(stage.getStageIndex(),
                                           stage.getStageDifficulty(),
                                           member_names,
                                           0);
}

// IDA_EA: 0x8133BD4
// IDA_NAME: _ZN12advancealtar10HistoryLog16dungeonClearInfoER5CUserRNS_12StageControlEb
// DEMANGLED: advancealtar::HistoryLog::dungeonClearInfo(
//     CUser &,advancealtar::StageControl &,bool)
int HistoryLog::dungeonClearInfo(CUser& user,
                                 StageControl& stage_control,
                                 bool count_entrance) {
    ProcStage* const stage = stage_control.getProcStage();
    const int started_second = stage->getStartStageTime();
    const int elapsed_second = started_second > 0
                                   ? GlobalData::system_time()->getCurSec() -
                                         started_second
                                   : 0;

    char member_names[256] = {};
    getMemberNames(&user, member_names);

    // StageControl::getCharacData @0x81348A0 returns its embedded
    // CharacData at +0x98.  The old call forwards that record's +0x00 word
    // (the named update marker), not a projected CUser offset.
    const int result = user.history_log().DungeonClearInfo(
        static_cast<int>(count_entrance),
        static_cast<long>(elapsed_second),
        stage->getStageIndex(),
        0,
        1,
        static_cast<ENUM_DUNGEON_MODE>(0),
        member_names,
        stage_control.charac_data.updated);

    if (count_entrance) {
        return advanceAltarstatistics_.incrementEntranceData(
            static_cast<uint32_t>(stage->getStageIndex()), 0u, 1u, 0u);
    }
    return result;
}

// IDA_EA: 0x8133D56
// IDA_NAME: _ZN12advancealtar10HistoryLog7starAddER5CUserNS_11AddStarType1TEii
// DEMANGLED: advancealtar::HistoryLog::starAdd(CUser &,advancealtar::AddStarType::T,int,int)
int HistoryLog::starAdd(CUser& user,
                        AddStarType::T type,
                        int stage_index,
                        int star) {
    user.LogHistory("star+,%d,%d,%d", static_cast<int>(type), stage_index,
                    star);
    return advanceAltarstatistics_.incrementEntranceData(
        static_cast<uint32_t>(stage_index), 0u, 0u,
        static_cast<uint32_t>(star));
}

// IDA_EA: 0x8133DB0
// IDA_NAME: _ZN12advancealtar10HistoryLog7starSubER5CUserNS_11SubStarType1TEi
// DEMANGLED: advancealtar::HistoryLog::starSub(CUser &,advancealtar::SubStarType::T,int)
int HistoryLog::starSub(CUser& user, SubStarType::T type, int star) {
    return user.LogHistory("star-,%d,%d", static_cast<int>(type), star);
}

// IDA_EA: 0x8133DDA
// IDA_NAME: _ZN12advancealtar10HistoryLog7buyItemENS_20AdvanceAltarShopType1TEissi
// DEMANGLED: advancealtar::HistoryLog::buyItem(advancealtar::AdvanceAltarShopType::T,int,short,short,int)
int HistoryLog::buyItem(AdvanceAltarShopType::T shop_type,
                        int item_id,
                        int16_t first_level,
                        int16_t last_level,
                        int star) {
    (void)first_level;
    return advanceAltarstatistics_.incrementLayerStatData(
        static_cast<uint16_t>(shop_type), static_cast<uint32_t>(item_id),
        static_cast<uint16_t>(last_level), 1u,
        static_cast<uint16_t>(star));
}

// IDA_EA: 0x8133E30
// IDA_NAME: _ZN12advancealtar10HistoryLog16updateStatisticsEv
// DEMANGLED: advancealtar::HistoryLog::updateStatistics(void)
int HistoryLog::updateStatistics() {
    return advanceAltarstatistics_.updateDatabase();
}

}  // namespace advancealtar
