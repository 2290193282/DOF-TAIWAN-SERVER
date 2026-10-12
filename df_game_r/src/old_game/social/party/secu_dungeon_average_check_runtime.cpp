#include "old_game/social/party/party.h"

#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>

// IDA_EA: 0x827A488
// IDA_NAME: _ZN18SecKillMonsterInfoD2Ev
// DEMANGLED: SecKillMonsterInfo::~SecKillMonsterInfo()
SecKillMonsterInfo::~SecKillMonsterInfo() {}

// IDA_EA: 0x82794D6
// IDA_NAME: _ZN24Secu_DungeonAverageCheck18setKillMonsterInfoEPK15MSG_MONSTER_DIEP11map_monster
// DEMANGLED: Secu_DungeonAverageCheck::setKillMonsterInfo(
//     MSG_MONSTER_DIE const*,map_monster *)
void Secu_DungeonAverageCheck::setKillMonsterInfo(
    const MSG_MONSTER_DIE* message,
    map_monster* monster) {
    if (!party_) {
        return;
    }
    if (monster->isNamedMonster() || monster->monster_kind_ != 0) {
        return;
    }

    std::vector<SecKillMonsterInfo>::iterator current =
        findKillMonsterInfo(static_cast<unsigned int>(monster->monster_id_));
    if (current == kill_monsters_.end()) {
        kill_monsters_.push_back(SecKillMonsterInfo(
            static_cast<unsigned int>(monster->monster_id_), 1u,
            message->field_a1e_, message->field_a1a_));
        return;
    }

    if (current->kill_count_ > 4u) {
        const float hp_average =
            static_cast<float>(current->hp_total_) /
            static_cast<long double>(current->kill_count_);
        const float exp_average =
            static_cast<float>(current->exp_total_) /
            static_cast<long double>(current->kill_count_);
        const float hp_margin = hp_average > 9.0f ? hp_average * 0.1f : 1.0f;
        const unsigned int hp_upper =
            static_cast<unsigned int>(hp_average + hp_margin + 0.5f);
        const unsigned int hp_lower =
            static_cast<unsigned int>(hp_average - hp_margin);
        const float exp_margin = exp_average * 0.050000001f;
        const unsigned int exp_upper =
            static_cast<unsigned int>(exp_average + exp_margin + 0.5f);
        const unsigned int exp_lower =
            static_cast<unsigned int>(exp_average - exp_margin);

        if (party_->map_hit_statistics_.map_count_ > 0 &&
            message->field_a1e_ >= hp_lower && message->field_a1e_ <= hp_upper) {
            ++current->hp_outlier_count_;
            for (int seat = 0; seat <= 3; ++seat) {
                if (party_->checkValidUser(seat)) {
                    CUser* const user = party_->get_user(seat);
                    user->getHackAnalyzer()->addServerHackCnt(
                        user, static_cast<WongWork::ENUM_HACKTYPE>(0xE6), 1u,
                        0u, 0u);
                }
            }
        }

        if (party_->map_hit_statistics_.map_count_ > 0 &&
            message->field_a1a_ >= exp_lower && message->field_a1a_ <= exp_upper) {
            ++current->exp_outlier_count_;
            for (int seat = 0; seat <= 3; ++seat) {
                if (party_->checkValidUser(seat)) {
                    CUser* const user = party_->get_user(seat);
                    user->getHackAnalyzer()->addServerHackCnt(
                        user, static_cast<WongWork::ENUM_HACKTYPE>(0xE7), 1u,
                        0u, 0u);
                }
            }
        }
    }

    ++current->kill_count_;
    current->hp_total_ += message->field_a1e_;
    current->exp_total_ += message->field_a1a_;
}

// IDA_EA: 0x827993C
// IDA_NAME: _ZN24Secu_DungeonAverageCheck19findKillMonsterInfoEj
// DEMANGLED: Secu_DungeonAverageCheck::findKillMonsterInfo(unsigned int)
std::vector<SecKillMonsterInfo>::iterator
Secu_DungeonAverageCheck::findKillMonsterInfo(unsigned int monster_id) {
    std::vector<SecKillMonsterInfo>::iterator current = kill_monsters_.begin();
    while (current != kill_monsters_.end()) {
        if (current->monster_id_ == monster_id) {
            return current;
        }
        ++current;
    }
    return kill_monsters_.end();
}

// IDA_EA: 0x8279490
// IDA_NAME: _ZN24Secu_DungeonAverageCheck5resetEv
// DEMANGLED: Secu_DungeonAverageCheck::reset(void)
void Secu_DungeonAverageCheck::reset() {
    kill_monsters_.clear();
}

// IDA_EA: 0x8279A12
// IDA_NAME: _ZN24Secu_DungeonAverageCheck23CheckHackTypeEndDungeonEv
// DEMANGLED: Secu_DungeonAverageCheck::CheckHackTypeEndDungeon(void)
void Secu_DungeonAverageCheck::CheckHackTypeEndDungeon() {
    if (!party_) {
        return;
    }

    for (std::vector<SecKillMonsterInfo>::iterator it = kill_monsters_.begin();
         it != kill_monsters_.end();
         ++it) {
        const int allowance = static_cast<int>(
                                  static_cast<long double>(it->kill_count_) *
                                      0.7 +
                                  0.5) -
                              4;
        const int minimum_allowance = std::max(allowance, 0);
        const int hp_excess =
            static_cast<int>(it->hp_outlier_count_) - minimum_allowance;
        const int exp_excess =
            static_cast<int>(it->exp_outlier_count_) - minimum_allowance;
        if (hp_excess <= 0 && exp_excess <= 0) {
            continue;
        }

        for (int seat = 0; seat <= 3; ++seat) {
            if (!party_->checkValidUser(seat)) {
                continue;
            }

            CUser* const user = party_->get_user(seat);
            const int grow_type = user->get_charac_job() * 100 +
                                  user->getCurCharFirstGrowType() * 10 +
                                  user->getCurCharSecondGrowType();
            if (hp_excess > 0) {
                const unsigned int saved_grow_type =
                    static_cast<unsigned int>(user->SetETC(232, grow_type));
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0xE8),
                    static_cast<unsigned int>(hp_excess), saved_grow_type, 0);
            }
            if (exp_excess > 0) {
                const unsigned int saved_grow_type =
                    static_cast<unsigned int>(user->SetETC(233, grow_type));
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(0xE9),
                    static_cast<unsigned int>(exp_excess), saved_grow_type, 0);
            }
        }
    }
}

// IDA_EA: 0x82799D6
// IDA_NAME: _ZN24Secu_DungeonAverageCheck12dungeonClearEv
// DEMANGLED: Secu_DungeonAverageCheck::dungeonClear(void)
void Secu_DungeonAverageCheck::dungeonClear() {
    CheckHackTypeEndDungeon();
    reset();
}

// IDA_EA: 0x82799F4
// IDA_NAME: _ZN24Secu_DungeonAverageCheck11dungeonFailEv
// DEMANGLED: Secu_DungeonAverageCheck::dungeonFail(void)
void Secu_DungeonAverageCheck::dungeonFail() {
    CheckHackTypeEndDungeon();
    reset();
}

// IDA_EA: 0x827AACA
// IDA_NAME: _ZN24Secu_HackLogCheckByParty11dungeonFailEv
// DEMANGLED: Secu_HackLogCheckByParty::dungeonFail(void)
void Secu_HackLogCheckByParty::dungeonFail() {
    Secu_DungeonAverageCheck::dungeonFail();
}


// IDA_EA: 0x827AAB6
// IDA_NAME: _ZN24Secu_HackLogCheckByParty12dungeonClearEv
// DEMANGLED: Secu_HackLogCheckByParty::dungeonClear(void)
void Secu_HackLogCheckByParty::dungeonClear() {
    Secu_DungeonAverageCheck::dungeonClear();
}

// IDA_EA: 0x827A940
// IDA_NAME: _ZN24Secu_HackLogCheckByParty6DieMobEPK15MSG_MONSTER_DIEP11map_monster
// DEMANGLED: Secu_HackLogCheckByParty::DieMob(MSG_MONSTER_DIE const*,map_monster *)
void Secu_HackLogCheckByParty::DieMob(const MSG_MONSTER_DIE* message,
                                      map_monster* monster) {
    if (message->monster_type_ != 0xffffu) {
        Secu_DungeonAverageCheck::setKillMonsterInfo(message, monster);
    }
}
