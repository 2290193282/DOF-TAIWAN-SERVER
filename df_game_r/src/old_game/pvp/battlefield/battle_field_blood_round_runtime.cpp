#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x8304DD2
// IDA_NAME: _ZN13CBattle_Field22BloodSpawnTimerManagerEbb
// DEMANGLED: CBattle_Field::BloodSpawnTimerManager(bool,bool)
char CBattle_Field::BloodSpawnTimerManager(bool finish_previous_round,
                                           bool suppress_round_start) {
    const short blood_round = static_cast<short>(
        blood_round_.getBloodRoundOrder());
    const short small_round = static_cast<short>(
        blood_round_.getBloodSmallRoundOrder());
    const short last_small_round = static_cast<short>(
        blood_round_.getBloodLastSmallRoundOrder());
    char result = static_cast<char>(small_round);
    if (small_round > last_small_round) {
        return result;
    }
    if (static_cast<unsigned char>(checkBloodSpawnOrder(small_round)) != 1u) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodSpawnTimerManager(bool, bool)",
            0xD0C,
            "BLOOD_LOG : round_order_ (%d), small_round_order_ (%d) : SET ORDER ERROR!!\n",
            blood_round,
            small_round);
        return 0;
    }

    const int interval =
        get_blood_spawn_small_round_interval(small_round);
    if (interval <= 0) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodSpawnTimerManager(bool, bool)",
            0xD19,
            "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ORDER!!\n",
            small_round);
        return 0;
    }

    if ((finish_previous_round && !suppress_round_start) || !small_round) {
        blood_clear_reward_.onStartBloodRound();
        BloodRoundIntervalPacket(blood_round, interval);
    }
    result = static_cast<char>(
        BloodPhaseSpawnTimerResgister(small_round) == -1);
    if (result) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodSpawnTimerManager(bool, bool)",
            0xD2B,
            "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ORDER!!\n",
            small_round);
        return 0;
    }
    return result;
}

// IDA_EA: 0x8304F8E
// IDA_NAME: _ZN13CBattle_Field29checkKilledSpawnBloodMonstersEi
// DEMANGLED: CBattle_Field::checkKilledSpawnBloodMonsters(int)
void CBattle_Field::checkKilledSpawnBloodMonsters(int monster_count) {
    const short blood_round =
        static_cast<short>(blood_round_.blood_round_order);
    const short small_round =
        static_cast<short>(blood_round_.blood_small_round_order);
    const short last_small_round =
        static_cast<short>(blood_round_.blood_last_small_round_order);
    if (last_small_round >= small_round) {
        // The unoptimized archive constructs this local value before walking
        // MapInfo::monsters_, even though only the iterator is consumed.
        map_monster scratch_monster;
        (void)scratch_monster;
        MapInfo* const map_info = GetCurrentMapInfo();
        for (std::map<int, map_monster>::iterator monster =
                 map_info->monsters_.begin();
             monster != map_info->monsters_.end();
             ++monster) {
            if (static_cast<short>(
                    monster->second.blood_small_round_order_) < small_round) {
                break;
            }
            const int current_round =
                get_blood_round_of_small_round(small_round);
            const int previous_round =
                get_blood_round_of_small_round(
                    static_cast<short>(small_round - 1));
            const int current_difficulty =
                get_blood_round_difficylty_of_small_round(small_round);
            const int previous_difficulty =
                get_blood_round_difficylty_of_small_round(
                    static_cast<short>(small_round - 1));
            if (current_round < 0 || previous_round < 0) {
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "void CBattle_Field::checkKilledSpawnBloodMonsters(int)",
                    0xD59,
                    "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ROUND!!\n",
                    small_round);
                break;
            }
            if (previous_round != current_round ||
                current_difficulty != previous_difficulty) {
                blood_clear_reward_.onFinishBloodRound(
                    false, blood_round, party_, dungeon_);
                if (static_cast<unsigned char>(
                        dungeon_->get_blood_dungeon_type()) == 2u) {
                    const int next_difficulty =
                        getNextRoundDifficulty(
                            static_cast<short>(blood_round - 1));
                    blood_clear_reward_.incUltimateRoundClearPoint(
                        next_difficulty);
                    if (forceValuesToChangeRound(
                            blood_round, small_round, 1)) {
                        setUltimateDifficulty(0);
                        const int timer_key =
                            party_->gen_timer_key(
                                static_cast<TIMER_MESSAGE>(38));
                        if (blood_round == getBloodMaxRound() - 1) {
                            G_TimerQueue()->InsertTimer(
                                1,
                                party_->GetPartyIndex(),
                                38,
                                1,
                                timer_key,
                                38);
                        } else {
                            G_TimerQueue()->InsertTimer(
                                1,
                                party_->GetPartyIndex(),
                                38,
                                10,
                                timer_key,
                                38);
                            sendUltimateRandomDifficulty(
                                0, blood_round, 10);
                        }
                    }
                } else {
                    BloodSpawnTimerManager(true, false);
                }
                break;
            }
        }
    } else if (!monster_count && current_map_->get_map_type() != 1) {
        blood_clear_reward_.onFinishBloodRound(
            false, blood_round, party_, dungeon_);
    }
}

// IDA_EA: 0x83053D2
// IDA_NAME: _ZN13CBattle_Field29BloodPhaseSpawnTimerResgisterEs
// DEMANGLED: CBattle_Field::BloodPhaseSpawnTimerResgister(short)
int CBattle_Field::BloodPhaseSpawnTimerResgister(short small_round) {
    unsigned int spawn_count = static_cast<unsigned int>(
        get_blood_spawn_cnt_of_small_round(small_round));
    if (spawn_count > 10u) {
        return -1;
    }
    if (!spawn_count) {
        spawn_count = 1;
    }

    short registered_small_round = small_round;
    for (unsigned int index = 0; index < spawn_count; ++index) {
        const int interval =
            get_blood_spawn_small_round_interval(registered_small_round);
        if (interval <= 0) {
            return -1;
        }
        if (blood_spawn_runtime_.selected_small_round ==
            static_cast<uint16_t>(registered_small_round)) {
            BloodCrazyMonsterSpawnPacket();
        }
        blood_spawn_runtime_.registered_small_rounds[index] =
            static_cast<int16_t>(registered_small_round);
        blood_spawn_runtime_.phase_finished[index] = 0;
        const TIMER_MESSAGE message =
            static_cast<TIMER_MESSAGE>(index + 23u);
        const int timer_key = party_->gen_timer_key(message);
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            party_->GetPartyIndex(),
            static_cast<int>(message),
            static_cast<int64_t>(interval),
            static_cast<unsigned int>(timer_key),
            0);
        ++registered_small_round;
        ++blood_round_.blood_small_round_order;
    }
    return 0;
}

// IDA_EA: 0x830553A
// IDA_NAME: _ZN13CBattle_Field15BloodPhaseSpawnEsRsii
// DEMANGLED: CBattle_Field::BloodPhaseSpawn(short,short &,int,int)
void CBattle_Field::BloodPhaseSpawn(short small_round,
                                    short& phase,
                                    int timer_message,
                                    int timer_slot) {
    const short monster_count = static_cast<short>(
        get_blood_spawn_cnt_of_phase(small_round, phase));
    int phase_time_msec = get_blood_spawn_time_of_phase(small_round);
    const short blood_round = static_cast<short>(
        blood_round_.getBloodRoundOrder());
    (void)blood_round;
    const short current_small_round = static_cast<short>(
        blood_round_.getBloodSmallRoundOrder());
    const short last_small_round = static_cast<short>(
        blood_round_.getBloodLastSmallRoundOrder());

    if (monster_count < 0 || phase_time_msec < 0) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodPhaseSpawn(short int, short int&, int, int)",
            0xDB8,
            "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ORDER!!\n",
            small_round);
        return;
    }

    if (current_map_ && last_small_round == small_round) {
        phase_time_msec = current_map_->get_map_type() == 1 ? -1 : 0;
    }

    BloodMonsterSpawnPacket(monster_count,
                            phase_time_msec,
                            small_round,
                            phase);

    const int phase_count =
        get_blood_spawn_phase_total_cnt_of_small_round(small_round);
    if (phase_count < 0) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodPhaseSpawn(short int, short int&, int, int)",
            0xDCC,
            "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ORDER!!\n",
            small_round);
        return;
    }

    if (phase_count - 1 <= phase) {
        blood_spawn_runtime_.phase_finished[timer_slot] = 0;
        blood_flags_[static_cast<unsigned int>(small_round)] = true;

        if (static_cast<unsigned char>(
                checkBloodSpawnOrder(current_small_round)) == 1u) {
            if (last_small_round < current_small_round) {
                blood_round_.incBloodRoundOrder();
                blood_round_.setPreMapBloodRoundOrder();
                return;
            }

            const int current_round =
                get_blood_round_of_small_round(current_small_round);
            const int previous_round = get_blood_round_of_small_round(
                static_cast<short>(current_small_round - 1));
            const int current_difficulty =
                get_blood_round_difficylty_of_small_round(
                    current_small_round);
            const int previous_difficulty =
                get_blood_round_difficylty_of_small_round(
                    static_cast<short>(current_small_round - 1));
            if (current_round < 0 || previous_round < 0) {
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "void CBattle_Field::BloodPhaseSpawn(short int, short int&, int, int)",
                    0xDEE,
                    "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ROUND!!\n",
                    current_small_round);
                return;
            }

            const int next_small_round_time_msec =
                get_blood_spawn_time_of_next_small_round(small_round);
            if (next_small_round_time_msec <= 0) {
                LogManager::logFormat(
                    1,
                    "battle_field.cpp",
                    "void CBattle_Field::BloodPhaseSpawn(short int, short int&, int, int)",
                    0xDF5,
                    "BLOOD_LOG : small_round_order_ (%d) : ABNOMAL ORDER!!\n",
                    current_small_round);
                return;
            }

            if (previous_round == current_round &&
                current_difficulty == previous_difficulty) {
                const int timer_key = party_->gen_timer_key(
                    TIMER_MESSAGE_BLOOD_SMALL_ROUND_REGISTER);
                G_TimerQueue()->InsertTimerInMilisecond(
                    1,
                    party_->GetPartyIndex(),
                    TIMER_MESSAGE_BLOOD_SMALL_ROUND_REGISTER,
                    static_cast<int64_t>(next_small_round_time_msec),
                    static_cast<unsigned int>(timer_key),
                    0);
            } else {
                blood_round_.incBloodRoundOrder();
                blood_round_.setPreMapBloodRoundOrder();
            }
        }
    } else {
        const TIMER_MESSAGE message =
            static_cast<TIMER_MESSAGE>(timer_message);
        const int timer_key = party_->gen_timer_key(message);
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            party_->GetPartyIndex(),
            timer_message,
            static_cast<int64_t>(phase_time_msec),
            static_cast<unsigned int>(timer_key),
            0);
    }

    ++phase;
}

// IDA_EA: 0x8305992
// IDA_NAME: _ZN13CBattle_Field23BloodMonsterSpawnPacketEiiss
// DEMANGLED: CBattle_Field::BloodMonsterSpawnPacket(int,int,short,short)
void CBattle_Field::BloodMonsterSpawnPacket(int monster_count,
                                             int phase_time_msec,
                                             short small_round,
                                             short phase) {
    (void)phase;
    int emitted_count = 0;
    PacketGuard packet;
    packet.put_header(0, 210);
    packet.put_short(monster_count);

    // The unoptimized 0627 body constructs this local value before walking
    // MapInfo::monsters_, although only the iterator is subsequently read.
    map_monster scratch_monster;
    (void)scratch_monster;
    MapInfo* const map_info = GetCurrentMapInfo();
    for (std::map<int, map_monster>::iterator monster =
             map_info->monsters_.begin();
         monster != map_info->monsters_.end() &&
             emitted_count != monster_count;
         ++monster) {
        map_monster& value = monster->second;
        if (static_cast<short>(value.blood_small_round_order_) !=
                small_round ||
            value.blood_runtime_flag_ == 1u) {
            continue;
        }

        packet.put_byte(static_cast<signed char>(value.blood_variant_));
        packet.put_short(static_cast<short>(value.map_monster_unique_id_));
        packet.put_int(value.monster_id_);
        packet.put_byte(phase_time_msec == -1 ? 3 : 0);
        packet.put_byte(
            static_cast<unsigned char>(dungeon_->get_blood_dungeon_type()) ==
                    2u
                ? 70
                : 60);
        packet.put_short(value.blood_spawn_rate_per_mille_);
        packet.put_short(value.blood_system_value_08_);
        packet.put_short(value.blood_system_value_0c_);
        packet.put_short(value.blood_system_value_10_);
        value.blood_runtime_flag_ = 1;
        ++emitted_count;
    }

    if (emitted_count == monster_count) {
        packet.put_short(phase_time_msec);
        packet.finalize(1);
        party_->send_to_party(packet);
    } else {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::BloodMonsterSpawnPacket(int, int, short int, short int)",
            0xE43,
            "BLOOD_LOG : EROOR BLOOD SPAWN MONSTER COUNT!!\n");
    }
}

// IDA_EA: 0x8305D0C
// IDA_NAME: _ZN13CBattle_Field28BloodCrazyMonsterSpawnPacketEv
// DEMANGLED: CBattle_Field::BloodCrazyMonsterSpawnPacket(void)
void CBattle_Field::BloodCrazyMonsterSpawnPacket() {
    PacketGuard packet;
    packet.put_header(0, 213);
    packet.put_int(blood_spawn_runtime_.selected_system_monster_id);
    packet.put_byte(
        static_cast<signed char>(blood_spawn_runtime_.selected_variant));
    packet.finalize(1);
    party_->send_to_party(packet);
}

// IDA_EA: 0x8305E9C
// IDA_NAME: _ZN13CBattle_Field24BloodRoundIntervalPacketEsi
// DEMANGLED: CBattle_Field::BloodRoundIntervalPacket(short,int)
void CBattle_Field::BloodRoundIntervalPacket(short blood_round,
                                              int interval_msec) {
    PacketGuard packet;
    packet.put_header(0, 215);
    packet.put_byte(static_cast<int>(blood_round) + 1);
    packet.put_int(interval_msec);
    packet.finalize(1);
    party_->send_to_party(packet);
}

// IDA_EA: 0x8305F50
// IDA_NAME: _ZN13CBattle_Field24forceValuesToChangeRoundEssi
// DEMANGLED: CBattle_Field::forceValuesToChangeRound(short,short,int)
bool CBattle_Field::forceValuesToChangeRound(
    short blood_round,
    short small_round,
    int next_round_difficulty) {
    const short next_small_round = static_cast<short>(
        getNextSamllRoundOrder(blood_round, next_round_difficulty));
    if (!next_small_round) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "bool CBattle_Field::forceValuesToChangeRound(short int, short int, int)",
            0xE7E,
            "ULTIMATE_LOG : blood_round_order (%d) forceValuesToChangeRound ERROR!!\n",
            blood_round);
        return false;
    }

    if (next_small_round > small_round) {
        for (int index = 0; index < next_small_round; ++index) {
            blood_flags_[index] = true;
        }
        MapInfo* const map_info = GetCurrentMapInfo();
        while (!map_info->monsters_.empty()) {
            std::map<int, map_monster>::iterator monster =
                map_info->monsters_.begin();
            const short monster_round =
                static_cast<short>(monster->second.blood_round_);
            const int monster_difficulty =
                static_cast<signed char>(
                    monster->second.blood_round_difficulty_);
            if (monster_round > blood_round ||
                (monster_round == blood_round &&
                 monster_difficulty == next_round_difficulty)) {
                break;
            }
            map_info->monsters_.erase(monster);
        }
    }
    blood_round_.blood_small_round_order =
        static_cast<uint16_t>(next_small_round);
    return true;
}

// IDA_EA: 0x83061B6
// IDA_NAME: _ZN13CBattle_Field22getNextRoundDifficultyEs
// DEMANGLED: CBattle_Field::getNextRoundDifficulty(short)
int CBattle_Field::getNextRoundDifficulty(short blood_round) {
    const int ultimate_difficulty =
        static_cast<signed char>(getUltimateDifficylty());
    if (blood_round == getBloodMaxRound() - 1 || !blood_round) {
        return 1;
    }
    return ultimate_difficulty;
}

// IDA_EA: 0x8306212
// IDA_NAME: _ZN13CBattle_Field22getNextSamllRoundOrderEsi
// DEMANGLED: CBattle_Field::getNextSamllRoundOrder(short,int)
int CBattle_Field::getNextSamllRoundOrder(
    short blood_round,
    int next_round_difficulty) {
    MapInfo* const map_info = GetCurrentMapInfo();
    for (std::map<int, map_monster>::iterator monster =
             map_info->monsters_.begin();
         monster != map_info->monsters_.end();
         ++monster) {
        if (static_cast<short>(monster->second.blood_round_) == blood_round &&
            static_cast<signed char>(
                monster->second.blood_round_difficulty_) ==
                next_round_difficulty) {
            return monster->second.blood_small_round_order_;
        }
    }
    return 0;
}

// IDA_EA: 0x830637C
// IDA_NAME: _ZN13CBattle_Field28sendUltimateRandomDifficultyEiii
// DEMANGLED: CBattle_Field::sendUltimateRandomDifficulty(int,int,int)
void CBattle_Field::sendUltimateRandomDifficulty(
    int difficulty,
    int blood_round,
    int delay_seconds) {
    PacketGuard packet;
    packet.put_header(0, 294);
    packet.put_byte(difficulty);
    packet.put_byte(blood_round);
    packet.put_short(delay_seconds);
    packet.finalize(1);
    party_->send_to_party(packet);
}

// IDA_EA: 0x8306436
// IDA_NAME: _ZN13CBattle_Field28sendUltimateRandomDifficultyEii
// DEMANGLED: CBattle_Field::sendUltimateRandomDifficulty(int,int)
void CBattle_Field::sendUltimateRandomDifficulty(int difficulty,
                                                  int blood_round) {
    PacketGuard packet;
    packet.put_header(0, 294);
    packet.put_byte(difficulty);
    packet.put_byte(blood_round);
    packet.finalize(1);
    party_->send_to_party(packet);
}

// IDA_EA: 0x83064DE
// IDA_NAME: _ZN13CBattle_Field18startUltimateRoundEv
// DEMANGLED: CBattle_Field::startUltimateRound(void)
char CBattle_Field::startUltimateRound() {
    const short blood_round = static_cast<short>(
        blood_round_.getBloodRoundOrder());
    const short small_round = static_cast<short>(
        blood_round_.getBloodSmallRoundOrder());
    (void)blood_round_.getBloodLastSmallRoundOrder();

    const int next_difficulty = getNextRoundDifficulty(blood_round);
    if (next_difficulty == 1) {
        return BloodSpawnTimerManager(true, false);
    }
    if (!next_difficulty) {
        LogManager::logFormat(
            1,
            "battle_field.cpp",
            "void CBattle_Field::startUltimateRound()",
            0xF04,
            "ULTIMATE_LOG : ULITMATE ROUND (%d) startUltimateRound() ERROR!!\n",
            blood_round);
        return 0;
    }
    if (forceValuesToChangeRound(
            blood_round, small_round, next_difficulty)) {
        return BloodSpawnTimerManager(true, false);
    }
    LogManager::logFormat(
        1,
        "battle_field.cpp",
        "void CBattle_Field::startUltimateRound()",
        0xF0A,
        "ULTIMATE_LOG : startUltimateRound() ERROR!!\n",
        blood_round);
    return 0;
}

// IDA_EA: 0x830EC12
// IDA_NAME: _ZN13CBattle_Field16getBloodMaxRoundEv
// DEMANGLED: CBattle_Field::getBloodMaxRound(void)
int CBattle_Field::getBloodMaxRound() {
    if (!dungeon_) {
        return 0;
    }
    return dungeon_->blood_max_round_;
}
