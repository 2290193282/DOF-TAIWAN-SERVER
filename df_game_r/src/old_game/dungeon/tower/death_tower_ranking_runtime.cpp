#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

namespace WongWork {

namespace {

void copyRecordMembers(CDeathTowerRanking::stBestRecord_t& record,
                       const stDeathTowerRecordMemberInfo_t* members,
                       unsigned int member_count) {
    // IDA 0x84686C8..0x84687A8 / 0x8468943..0x84689BF copies exactly
    // 0x1e bytes with strncpy, then consumes the compact 23-byte DB input
    // record's level/job/grow bytes.  Do not add a terminator policy here:
    // the old implementation deliberately left strncpy's exact behavior.
    for (unsigned int index = 0; index < member_count; ++index) {
        std::strncpy(record.charac_name_[index],
                     members[index].charac_name,
                     sizeof(record.charac_name_[index]));
        record.character_level_[index] = members[index].level;
        record.character_job_[index] = members[index].job;
        record.character_grow_type_[index] = members[index].grow_type;
    }
}

unsigned int absoluteRankDelta(unsigned int left, unsigned int right) {
    return static_cast<unsigned int>(
        std::abs(static_cast<int>(left) - static_cast<int>(right)));
}

unsigned int interpolateOffset(unsigned int span,
                               unsigned int rank_delta,
                               unsigned int distance) {
    const float unit_delta =
        static_cast<float>(rank_delta) / static_cast<float>(span);
    const unsigned int interpolated = static_cast<unsigned int>(
        static_cast<float>(distance) * unit_delta);
    return std::max(1u, interpolated);
}

}  // namespace

// IDA_EA: 0x82A76C0
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking14stBestRecord_tC2Ev
// DEMANGLED: WongWork::CDeathTowerRanking::stBestRecord_t::stBestRecord_t(void)
CDeathTowerRanking::stBestRecord_t::stBestRecord_t() {
    // The original constructor intentionally leaves charac_no_ and the tail
    // bytes of each name alone.  Those bytes are initialized by the owning
    // static BSS object or by the exact input-copy paths, not by a blanket
    // clear here.
    valid_ = 0;
    virtual_rank_ = -1;
    for (unsigned int index = 0; index <= 3u; ++index) {
        charac_name_[index][0] = '\0';
        character_level_[index] = 0;
        character_job_[index] = 0;
        character_grow_type_[index] = 0;
    }
    current_stage_ = 0;
    play_time_ = 0;
}

// IDA_EA: 0x82A774C
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking12getRankTableEj
// DEMANGLED: WongWork::CDeathTowerRanking::getRankTable(unsigned int)
CDeathTowerRanking* CDeathTowerRanking::getRankTable(unsigned int member_count) {
    // This function-local four-object aggregate is the original guarded
    // static, not a byte reservation.  The original keeps its unchecked
    // mapping of 0/1 to the first table and 2..4 to the remaining entries.
    static CDeathTowerRanking tower_ranking_table[4];
    if (member_count == 0u) {
        return &tower_ranking_table[0];
    }
    return &tower_ranking_table[member_count - 1u];
}

// IDA_EA: 0x82A787C
// IDA_NAME: _ZN8WongWork18CDeathTowerRankingC2Ev
// DEMANGLED: WongWork::CDeathTowerRanking::CDeathTowerRanking(void)
CDeathTowerRanking::CDeathTowerRanking() {
    // Member construction order is the source declaration order and matches
    // 0x82A787C: 5x6 records, five rank hash maps, five best-record maps,
    // 5x200 stage maps, one top record, then the scalar member count.
    member_count_ = 0;
}

// IDA_EA: 0x82A7B06
// IDA_NAME: _ZN8WongWork18CDeathTowerRankingD2Ev
// DEMANGLED: WongWork::CDeathTowerRanking::~CDeathTowerRanking()
CDeathTowerRanking::~CDeathTowerRanking() {
    // The compiler destroys stage_ranking_, then best_record_table_, then
    // ranking_table_ in reverse declaration order, exactly as 0x82A7B06.
}

// IDA_EA: 0x84680A4
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking4initEj
CDeathTowerRanking* CDeathTowerRanking::init(unsigned int member_count) {
    for (unsigned int rank_index = 0; rank_index <= 4u; ++rank_index) {
        for (unsigned int slot = 1; slot <= 5u; ++slot) {
            top_rankers_[rank_index][slot].valid_ = 0;
        }

        std::memcpy(top_rankers_[rank_index][1].charac_name_[0],
                    "AAAAAA", 7u);
        std::memcpy(top_rankers_[rank_index][2].charac_name_[0],
                    "BBBBBB", 7u);
        std::memcpy(top_rankers_[rank_index][3].charac_name_[0],
                    "CCCCCC", 7u);
        std::memcpy(top_rankers_[rank_index][4].charac_name_[0],
                    "DDDDDD", 7u);
        std::memcpy(top_rankers_[rank_index][5].charac_name_[0],
                    "EEEEEE", 7u);

        for (unsigned int slot = 1; slot <= 5u; ++slot) {
            top_rankers_[rank_index][slot].virtual_rank_ =
                static_cast<int32_t>(slot);
            top_rankers_[rank_index][slot].play_time_ = 0;
            top_rankers_[rank_index][slot].current_stage_ = 200u - slot;
        }
    }

    member_count_ = member_count;
    top_record_.virtual_rank_ = 200;
    return this;
}

// IDA_EA: 0x8468290
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking19checkRenewTopRecordEjjj
int CDeathTowerRanking::checkRenewTopRecord(unsigned int rank_index,
                                            unsigned int stage,
                                            unsigned int play_time) {
    if (top_rankers_[rank_index][1].valid_ != 1u) {
        return 0;
    }

    stBestRecord_t candidate;
    candidate.current_stage_ = stage;
    candidate.play_time_ = play_time;
    return candidate > top_rankers_[rank_index][1];
}

// IDA_EA: 0x84682FE
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking18checkRenewMyRecordEjjjj
int CDeathTowerRanking::checkRenewMyRecord(unsigned int rank_index,
                                           unsigned int charac_no,
                                           unsigned int stage,
                                           unsigned int play_time) {
    BestRecordTable* const table = _getBestRecordTable(rank_index);
    BestRecordTable::const_iterator found = table->find(charac_no);
    if (found == table->end()) {
        return -1;
    }

    stBestRecord_t candidate;
    candidate.current_stage_ = stage;
    candidate.play_time_ = play_time;
    return candidate > found->second;
}

// IDA_EA: 0x84683DA
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking13registRankingEjjj
char CDeathTowerRanking::registRanking(unsigned int rank_index,
                                       unsigned int charac_no,
                                       unsigned int virtual_rank) {
    RankingTable* const ranks = _getRankingTable(rank_index);
    std::pair<RankingTable::iterator, bool> inserted =
        ranks->insert(std::make_pair(charac_no, virtual_rank));
    if (!inserted.second) {
        inserted.first->second = virtual_rank;
    }

    BestRecordTable* const records = _getBestRecordTable(rank_index);
    BestRecordTable::iterator record = records->find(charac_no);
    if (record != records->end()) {
        record->second.virtual_rank_ = static_cast<int32_t>(virtual_rank);
    }

    // 0x84684CB falls through without a value-producing instruction; all
    // callers discard this historical char return.  EAX is this at the exit
    // on the original path, so preserve its observable i386 low-byte form.
    return static_cast<char>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x84684CE
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking13updateRankingEjjj
int CDeathTowerRanking::updateRanking(unsigned int rank_index,
                                      unsigned int charac_no,
                                      unsigned int virtual_rank) {
    RankingTable* const ranks = _getRankingTable(rank_index);
    std::pair<RankingTable::iterator, bool> inserted =
        ranks->insert(std::make_pair(charac_no, virtual_rank));
    if (inserted.second) {
        // The original returns (inserted ^ 1), therefore zero for a new row.
        return 0;
    }

    inserted.first->second = virtual_rank;
    // 0x846854E returns the hash-table value pointer in EAX after an update.
    return static_cast<int>(reinterpret_cast<uintptr_t>(&inserted.first->second));
}

// IDA_EA: 0x8468552
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking15unregistRankingEj
// DEMANGLED: WongWork::CDeathTowerRanking::unregistRanking(unsigned int)
bool CDeathTowerRanking::unregistRanking(unsigned int charac_no) {
    for (unsigned int rank_index = 0; rank_index <= 4u; ++rank_index) {
        _getRankingTable(rank_index)->erase(charac_no);
    }
    // The original loop sets its bool from the final index (5), so it exits
    // false even after completing all five erases.
    return false;
}

// IDA_EA: 0x8468594
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking12resetRankingEv
bool CDeathTowerRanking::resetRanking() {
    for (unsigned int rank_index = 0; rank_index <= 4u; ++rank_index) {
        _getRankingTable(rank_index)->clear();
        for (unsigned int stage = 0; stage <= 0xc7u; ++stage) {
            stage_ranking_[rank_index][stage].clear();
        }
    }
    // See 0x8468594: same final-loop-bool behavior as unregistRanking.
    return false;
}

// IDA_EA: 0x8468614
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking12registRankerEjjPK30stDeathTowerRecordMemberInfo_tjj
char CDeathTowerRanking::registRanker(
    unsigned int rank_index,
    unsigned int member_slot,
    const stDeathTowerRecordMemberInfo_t* members,
    unsigned int stage,
    unsigned int play_time) {
    if (member_slot <= 5u) {
        stBestRecord_t& record = top_rankers_[rank_index][member_slot];
        if (record.valid_) {
            stage_ranking_[rank_index][record.current_stage_].erase(
                record.play_time_);
        }

        copyRecordMembers(record, members, member_count_);
        record.current_stage_ = stage;
        record.play_time_ = play_time;
        record.valid_ = 1;
    }

    std::pair<StageRankingTable::iterator, bool> inserted =
        stage_ranking_[rank_index][stage].insert(
            std::make_pair(play_time, member_slot));
    if (!inserted.second) {
        inserted.first->second = member_slot;
    }

    stBestRecord_t candidate;
    candidate.current_stage_ = stage;
    candidate.play_time_ = play_time;

    // The direction is intentionally top_record_ > candidate, matching the
    // actual register order at 0x84688CE.  It is counterintuitive, but it is
    // the original behavior and must not be normalized to a new policy.
    if (top_record_ > candidate) {
        std::memcpy(&top_record_, &candidate, sizeof(candidate));
        return 0x26;
    }

    return 0;
}

// IDA_EA: 0x8468914
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking16registBestRecordEjjPK30stDeathTowerRecordMemberInfo_tjj
int CDeathTowerRanking::registBestRecord(
    unsigned int rank_index,
    unsigned int charac_no,
    const stDeathTowerRecordMemberInfo_t* members,
    unsigned int stage,
    unsigned int play_time) {
    stBestRecord_t record;
    record.charac_no_ = charac_no;
    copyRecordMembers(record, members, member_count_);
    record.current_stage_ = stage;
    record.play_time_ = play_time;

    RankingTable* const rankings = _getRankingTable(rank_index);
    RankingTable::const_iterator rank = rankings->find(charac_no);
    if (rank != rankings->end()) {
        record.virtual_rank_ = static_cast<int32_t>(rank->second);
    }

    BestRecordTable* const records = _getBestRecordTable(rank_index);
    std::pair<BestRecordTable::iterator, bool> inserted =
        records->insert(std::make_pair(charac_no, record));
    if (inserted.second) {
        // 0x8468B0C computes (inserted ^ 1), which is zero on first insert.
        return 0;
    }

    // The old body calls operator[] and copies all 0x98 bytes when replacing.
    // Using the found element preserves that same value update without adding
    // a distinct default-construction path.
    inserted.first->second = record;
    return 0x26;
}

// IDA_EA: 0x8468B4C
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking18unregistBestRecordEj
// DEMANGLED: WongWork::CDeathTowerRanking::unregistBestRecord(unsigned int)
bool CDeathTowerRanking::unregistBestRecord(unsigned int charac_no) {
    for (unsigned int rank_index = 0; rank_index <= 4u; ++rank_index) {
        _getBestRecordTable(rank_index)->erase(charac_no);
    }
    // 0x8468B4C has the same final loop-condition return as its rank twin.
    return false;
}

// IDA_EA: 0x8468B8E
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking22makeMyBestRecordPacketEjjR11PacketGuard
int CDeathTowerRanking::makeMyBestRecordPacket(unsigned int rank_index,
                                               unsigned int charac_no,
                                               PacketGuard& packet) {
    BestRecordTable* const records = _getBestRecordTable(rank_index);
    BestRecordTable::const_iterator found = records->find(charac_no);
    if (found == records->end()) {
        return packet.put_byte(0);
    }

    packet.put_byte(1);
    return _makeRecordPacket(found->second, packet);
}

// IDA_EA: 0x8468C68
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking25makeTopRankerRecordPacketEjR11PacketGuard
bool CDeathTowerRanking::makeTopRankerRecordPacket(unsigned int rank_index,
                                                   PacketGuard& packet) {
    for (unsigned int member_slot = 1; member_slot <= 5u; ++member_slot) {
        _makeRecordPacket(top_rankers_[rank_index][member_slot], packet);
    }

    // As at 0x8468C68, the final loop test is the returned bool (false).
    return false;
}

// IDA_EA: 0x8468CB6
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking17_makeRecordPacketERKNS0_14stBestRecord_tER11PacketGuard
int CDeathTowerRanking::_makeRecordPacket(const stBestRecord_t& record,
                                          PacketGuard& packet) {
    for (unsigned int index = 0; index <= 3u; ++index) {
        const size_t name_length = std::strlen(record.charac_name_[index]);
        packet.put_int(static_cast<int>(name_length));
        packet.put_str(record.charac_name_[index],
                       static_cast<int>(name_length));
        packet.put_byte(record.character_job_[index]);
        packet.put_byte(record.character_grow_type_[index]);
    }
    packet.put_short(static_cast<int>(record.current_stage_));
    packet.put_int(static_cast<int>(record.play_time_));
    return packet.put_int(record.virtual_rank_);
}

// IDA_EA: 0x8468DE0
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking14getVirtualRankEjjj
int CDeathTowerRanking::getVirtualRank(unsigned int rank_index,
                                       unsigned int stage,
                                       unsigned int play_time) {
    stBestRecord_t candidate;
    candidate.current_stage_ = stage;
    candidate.play_time_ = play_time;

    if (top_record_ > candidate) {
        return -1;
    }
    if (candidate == top_record_) {
        return top_record_.virtual_rank_;
    }
    if (candidate > top_rankers_[rank_index][1]) {
        return 1;
    }

    StageRankingTable::const_iterator lower;
    StageRankingTable::const_iterator upper;
    unsigned int time_span = 0;
    unsigned int rank_offset = static_cast<unsigned int>(-1);
    StageRankingTable& same_stage = stage_ranking_[rank_index][stage];

    if (same_stage.size() > 1u) {
        lower = same_stage.begin();
        upper = same_stage.end();
        --upper;
        if (lower->first <= play_time && upper->first >= play_time) {
            lower = same_stage.lower_bound(play_time);
            if (lower->first > play_time && lower != same_stage.begin()) {
                --lower;
            }
            upper = lower;
            ++upper;

            if (lower->first == play_time) {
                return static_cast<int>(lower->second);
            }
            if (upper->first == play_time) {
                return static_cast<int>(upper->second);
            }

            time_span = std::max(1u, upper->first - lower->first);
            rank_offset = interpolateOffset(
                time_span,
                absoluteRankDelta(upper->second, lower->second),
                play_time - lower->first);
        }
    }

    if (time_span == 0u) {
        unsigned int lower_stage = 0;
        unsigned int upper_stage = 0;
        if (!same_stage.empty()) {
            upper = same_stage.end();
            --upper;
            if (upper->first == play_time) {
                return static_cast<int>(upper->second);
            }
            if (upper->first >= play_time) {
                lower_stage = stage;
            } else {
                upper_stage = stage;
            }
        }

        if (lower_stage == 0u) {
            lower_stage = stage;
            do {
                const bool reached_one = lower_stage-- == 1u;
                if (reached_one || lower_stage == 1u ||
                    !stage_ranking_[rank_index][lower_stage].empty()) {
                    break;
                }
            } while (true);
        }

        if (upper_stage == 0u) {
            upper_stage = stage;
            while (++upper_stage <= 0xc7u &&
                   stage_ranking_[rank_index][upper_stage].empty()) {
            }
        }

        if (stage_ranking_[rank_index][lower_stage].empty() ||
            stage_ranking_[rank_index][upper_stage].empty()) {
            return -1;
        }

        lower = stage_ranking_[rank_index][lower_stage].begin();
        upper = stage_ranking_[rank_index][upper_stage].end();
        --upper;
        const unsigned int stage_span =
            std::max(1u, upper_stage - lower_stage);
        rank_offset = interpolateOffset(
            stage_span,
            absoluteRankDelta(lower->second, upper->second),
            upper_stage - stage);
    }

    return static_cast<int>(lower->second + rank_offset);
}

// IDA_EA: 0x8469CE4
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking14stBestRecord_tgtERKS1_
// DEMANGLED: WongWork::CDeathTowerRanking::stBestRecord_t::operator>(WongWork::CDeathTowerRanking::stBestRecord_t const&)
bool CDeathTowerRanking::stBestRecord_t::operator>(
    const stBestRecord_t& right) {
    if (current_stage_ == right.current_stage_) {
        return play_time_ < right.play_time_;
    }
    return current_stage_ > right.current_stage_;
}

// IDA_EA: 0x8469D30
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking14stBestRecord_teqERKS1_
// DEMANGLED: WongWork::CDeathTowerRanking::stBestRecord_t::operator==(WongWork::CDeathTowerRanking::stBestRecord_t const&)
bool CDeathTowerRanking::stBestRecord_t::operator==(
    const stBestRecord_t& right) {
    return current_stage_ == right.current_stage_ &&
           play_time_ == right.play_time_;
}

// IDA_EA: 0x8469D69
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking23makeDungeonIdx2TowerIdxEj
int CDeathTowerRanking::makeDungeonIdx2TowerIdx(unsigned int dungeon_index) {
    if (dungeon_index > 0x2af7u && dungeon_index <= 0x2afdu) {
        return static_cast<int>(dungeon_index) - 11000;
    }
    return -1;
}

// IDA_EA: 0x8469D90
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking16_getRankingTableEj
CDeathTowerRanking::RankingTable* CDeathTowerRanking::_getRankingTable(
    unsigned int rank_index) {
    return &ranking_table_[rank_index];
}

// IDA_EA: 0x8469DAE
// IDA_NAME: _ZN8WongWork18CDeathTowerRanking19_getBestRecordTableEj
CDeathTowerRanking::BestRecordTable* CDeathTowerRanking::_getBestRecordTable(
    unsigned int rank_index) {
    return &best_record_table_[rank_index];
}

}  // namespace WongWork
