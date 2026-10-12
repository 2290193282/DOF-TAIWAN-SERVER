#include "old_data/Login/inter_returns/inter_death_tower_return_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/player/core/cuser.h"

namespace {

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D316A
// IDA_NAME: _ZN30Inter_LoadDeathTowerBestRecord12dispatch_sigEP5CUserPci
int Inter_LoadDeathTowerBestRecord::dispatch_sig(CUser*, char* payload, int) {
    for (unsigned int record_index = 0;
         record_index < ReadU32(payload, 0);
         ++record_index) {
        for (unsigned int member_index = 0; member_index <= 3u; ++member_index) {
            const int record_offset = static_cast<int>(432u * record_index +
                                                       108u * member_index);
            const unsigned int stage = ReadU32(payload, record_offset + 108);
            if (!stage) {
                continue;
            }
            WongWork::CDeathTowerRanking::getRankTable(member_index + 1u)
                ->registBestRecord(
                    ReadU16(payload, static_cast<int>(432u * record_index + 12u)),
                    ReadU32(payload, 4),
                    reinterpret_cast<const stDeathTowerRecordMemberInfo_t*>(
                        payload + record_offset + 16),
                    stage,
                    ReadU32(payload, record_offset + 112));
        }
    }
    return 0;
}

// IDA_EA: 0x84ED9D8 / 0x84F92AC / 0x84F92DC
Inter_LoadDeathTowerBestRecord::Inter_LoadDeathTowerBestRecord()
    : ISigDispatcher() {}
Inter_LoadDeathTowerBestRecord::~Inter_LoadDeathTowerBestRecord() {}

// IDA_EA: 0x84D3364
// IDA_NAME: _ZN29Inter_DeathTowerLoadTopRanker12dispatch_sigEP5CUserPci
int Inter_DeathTowerLoadTopRanker::dispatch_sig(CUser*, char* payload, int) {
    if (!payload[4]) {
        for (unsigned int member_index = 1; member_index <= 4u; ++member_index) {
            WongWork::CDeathTowerRanking::getRankTable(member_index)->resetRanking();
        }
    }

    for (unsigned int index = 0; index < ReadU32(payload, 0); ++index) {
        const int record_offset = static_cast<int>(108u * index);
        WongWork::CDeathTowerRanking::getRankTable(
            ReadU16(payload, record_offset + 4014))
            ->registRanker(
                ReadU16(payload, record_offset + 4012),
                ReadU16(payload, static_cast<int>(2u * index + 6u)),
                reinterpret_cast<const stDeathTowerRecordMemberInfo_t*>(
                    payload + record_offset + 4016),
                ReadU32(payload, record_offset + 4108),
                ReadU32(payload, record_offset + 4112));
    }
    return 0;
}

// IDA_EA: 0x84ED9F4 / 0x84F91C2 / 0x84F91F2
Inter_DeathTowerLoadTopRanker::Inter_DeathTowerLoadTopRanker()
    : ISigDispatcher() {}
Inter_DeathTowerLoadTopRanker::~Inter_DeathTowerLoadTopRanker() {}

// IDA_EA: 0x84D3290
// IDA_NAME: _ZN30Inter_MonitorDeathTowerRanking12dispatch_sigEP5CUserPci
int Inter_MonitorDeathTowerRanking::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int) {
    for (unsigned int rank_index = 0; rank_index <= 4u; ++rank_index) {
        for (unsigned int member_index = 0; member_index <= 3u; ++member_index) {
            const unsigned int rank = ReadU32(
                payload, static_cast<int>(16u * rank_index + 18u + 4u * member_index));
            if (rank) {
                WongWork::CDeathTowerRanking::getRankTable(member_index + 1u)
                    ->registRanking(rank_index,
                                    static_cast<unsigned int>(user->getCurCharacNo()),
                                    rank);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x84EDA10 / 0x84F925E / 0x84F928E
Inter_MonitorDeathTowerRanking::Inter_MonitorDeathTowerRanking()
    : ISigDispatcher() {}
Inter_MonitorDeathTowerRanking::~Inter_MonitorDeathTowerRanking() {}

// IDA_EA: 0x84D334A
// IDA_NAME: _ZN33Inter_MonitorTowerReloadTopRanker12dispatch_sigEP5CUserPci
int Inter_MonitorTowerReloadTopRanker::dispatch_sig(CUser*, char*, int) {
    DB_DeathTowerLoadTopRanker::makeRequest(true);
    return 0;
}

// IDA_EA: 0x84EDA2C / 0x84F9210 / 0x84F9240
Inter_MonitorTowerReloadTopRanker::Inter_MonitorTowerReloadTopRanker()
    : ISigDispatcher() {}
Inter_MonitorTowerReloadTopRanker::~Inter_MonitorTowerReloadTopRanker() {}
