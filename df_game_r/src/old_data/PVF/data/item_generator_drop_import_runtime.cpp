#include <string>
#include <vector>

#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/item_generator_drop_types.h"

extern "C++" int roundFloat(float value);

namespace WongWork {

// IDA_EA: 0x853A56A
// IDA_NAME: _ZN8WongWork17CMonsterDrop_HellC2Ei
// DEMANGLED: WongWork::CMonsterDrop_Hell::CMonsterDrop_Hell(int)
CMonsterDrop_Hell::CMonsterDrop_Hell(int random_stream_count)
    : CItemGenerator(random_stream_count),
      probabilities_(),
      scaled_probabilities_(),
      item_rate_table_() {}

// IDA_EA: 0x8535344
// IDA_NAME: _ZN8WongWork17CMonsterDrop_Hell10loadScriptEPKc
// DEMANGLED: WongWork::CMonsterDrop_Hell::loadScript(char const*)
int CMonsterDrop_Hell::loadScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 770;
    }

    bool success = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[basis of rarity dicision]") {
            const int count = ScanInt(&success);
            if (!success) return 785;
            for (int drop_type = 1; drop_type <= count; ++drop_type) {
                for (int rarity = 0; rarity <= 5; ++rarity) {
                    rarity_decision_[drop_type][rarity] = ScanInt(&success);
                    if (!success) return 793;
                }
            }
        } else if (tag == "[drop prob count]") {
            const int count = ScanInt(&success);
            probabilities_.resize(static_cast<size_t>(count),
                                  stGenItemProb_t());
            if (!success) return 800;
        } else if (tag == "[dungeon difficulty drop prob]") {
            for (size_t index = 0; index < probabilities_.size(); ++index) {
                stGenItemProb_t& probability = probabilities_[index];
                probability.first_level = ScanInt(&success);
                if (!success) return 808;
                probability.last_level = ScanInt(&success);
                if (!success) return 810;
                for (int rarity = 0; rarity <= 4; ++rarity) {
                    probability.weight[rarity] = ScanInt(&success);
                    if (!success) return 818;
                }
            }
        } else if (tag == "[item drop ref table]") {
            item_ref_range_[0][0] = 0;
            item_ref_range_[0][1] = 0;
            for (int row = 1; row <= 200; ++row) {
                const int level = ScanInt(&success);
                if (!success) return 831;
                item_ref_range_[level][0] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 835;
                item_ref_range_[level][1] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 839;
            }
        } else {
            return 843;
        }
    }

    item_rate_table_.generateTable(item_ref_range_, true);
    return 0;
}

// IDA_EA: 0x853A426
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDropC2Ei
// DEMANGLED: WongWork::CSpecialMonsterDrop::CSpecialMonsterDrop(int)
CSpecialMonsterDrop::CSpecialMonsterDrop(int random_stream_count)
    : CItemGenerator(random_stream_count),
      probabilities_(),
      scaled_probabilities_(),
      item_rate_table_() {}

// IDA_EA: 0x853A4CA
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDropD2Ev
// DEMANGLED: WongWork::CSpecialMonsterDrop::~CSpecialMonsterDrop()
CSpecialMonsterDrop::~CSpecialMonsterDrop() {}

// IDA_EA: 0x85359E8
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDrop12setDropRatioEf
// DEMANGLED: WongWork::CSpecialMonsterDrop::setDropRatio(float)
void CSpecialMonsterDrop::setDropRatio(float ratio) {
    scaled_probabilities_ = probabilities_;
    for (std::vector<stGenItemProb_t>::iterator probability =
             scaled_probabilities_.begin();
         probability != scaled_probabilities_.end();
         ++probability) {
        for (int rarity = 0; rarity <= 4; ++rarity) {
            probability->weight[rarity] = roundFloat(static_cast<float>(
                static_cast<long double>(probability->weight[rarity]) * ratio));
        }
    }
}

// IDA_EA: 0x85373EE
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDrop10loadScriptEPKc
// DEMANGLED: WongWork::CSpecialMonsterDrop::loadScript(char const*)
int CSpecialMonsterDrop::loadScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 1531;
    }

    bool success = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[drop prob count]") {
            const int count = ScanInt(&success);
            probabilities_.resize(static_cast<size_t>(count),
                                  stGenItemProb_t());
            if (!success) return 1545;
        } else if (tag == "[drop prob]") {
            for (size_t index = 0; index < probabilities_.size(); ++index) {
                stGenItemProb_t& probability = probabilities_[index];
                probability.first_level = ScanInt(&success);
                if (!success) return 1553;
                probability.last_level = ScanInt(&success);
                if (!success) return 1555;
                for (int rarity = 0; rarity <= 4; ++rarity) {
                    probability.weight[rarity] = ScanInt(&success);
                    if (!success) return 1559;
                }
            }
        } else if (tag == "[basis of rarity dicision]") {
            for (int drop_type = 1; drop_type <= 4; ++drop_type) {
                for (int rarity = 0; rarity <= 5; ++rarity) {
                    rarity_decision_[drop_type][rarity] = ScanInt(&success);
                    if (!success) return 1571;
                }
            }
        } else if (tag == "[party member drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int party_members = 0; party_members <= 3;
                     ++party_members) {
                    party_member_bonus_[drop_type][party_members] =
                        ScanFloat(&success);
                    if (!success) return 1582;
                }
            }
        } else if (tag == "[dungeon difficulty drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int difficulty = 0; difficulty <= 4; ++difficulty) {
                    dungeon_difficulty_bonus_[drop_type][difficulty] =
                        ScanFloat(&success);
                    if (!success) return 1597;
                }
            }
        } else if (tag == "[monster type drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int monster_type = 0; monster_type <= 3;
                     ++monster_type) {
                    monster_type_bonus_[drop_type][monster_type] =
                        ScanFloat(&success);
                    if (!success) return 1608;
                }
            }
        } else if (tag == "[item drop ref table]") {
            item_ref_range_[0][0] = 0;
            item_ref_range_[0][1] = 0;
            for (int row = 1; row <= 200; ++row) {
                const int level = ScanInt(&success);
                if (!success) return 1621;
                item_ref_range_[level][0] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 1625;
                item_ref_range_[level][1] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 1629;
            }
        } else {
            return 1633;
        }
    }

    item_rate_table_.generateTable(item_ref_range_);
    return 0;
}

// IDA_EA: 0x853A6AE
// IDA_NAME: _ZN8WongWork12CMonsterDrop25stFirstMobHuntingWeight_tC2Ev
CMonsterDrop::stFirstMobHuntingWeight_t::stFirstMobHuntingWeight_t()
    : first_boss_weight(1), first_named_mob_weight(1) {}

// IDA_EA: 0x853A6C8
// IDA_NAME: _ZN8WongWork12CMonsterDropC2Ei
// DEMANGLED: WongWork::CMonsterDrop::CMonsterDrop(int)
CMonsterDrop::CMonsterDrop(int random_stream_count)
    : CItemGenerator(random_stream_count),
      probabilities_(),
      scaled_probabilities_(),
      item_rate_table_(),
      hell_drop_(2),
      special_drop_(3),
      first_mob_hunting_weight_(),
      normal_drop_scale_(1),
      normal_drop_scale_rate_(1.0f) {}

// IDA_EA: 0x8535CF8
// IDA_NAME: _ZN8WongWork12CMonsterDrop12setDropRatioEf
// DEMANGLED: WongWork::CMonsterDrop::setDropRatio(float)
void CMonsterDrop::setDropRatio(float ratio) {
    scaled_probabilities_ = probabilities_;
    for (std::vector<stGenItemProb_t>::iterator probability =
             scaled_probabilities_.begin();
         probability != scaled_probabilities_.end();
         ++probability) {
        for (int rarity = 0; rarity <= 4; ++rarity) {
            probability->weight[rarity] = roundFloat(static_cast<float>(
                static_cast<long double>(probability->weight[rarity]) * ratio));
        }
    }
    special_drop_.setDropRatio(ratio);
    hell_drop_.setDropRatio(ratio);
}

// IDA_EA: 0x8537938
// IDA_NAME: _ZN8WongWork12CMonsterDrop10loadScriptEPKc
// DEMANGLED: WongWork::CMonsterDrop::loadScript(char const*)
int CMonsterDrop::loadScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 1643;
    }

    bool success = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[drop prob count]") {
            const int count = ScanInt(&success);
            probabilities_.resize(static_cast<size_t>(count),
                                  stGenItemProb_t());
            if (!success) return 1657;
        } else if (tag == "[drop prob]") {
            for (size_t index = 0; index < probabilities_.size(); ++index) {
                stGenItemProb_t& probability = probabilities_[index];
                probability.first_level = ScanInt(&success);
                if (!success) return 1665;
                probability.last_level = ScanInt(&success);
                if (!success) return 1667;
                for (int rarity = 0; rarity <= 4; ++rarity) {
                    probability.weight[rarity] = ScanInt(&success);
                    if (!success) return 1671;
                }
            }
        } else if (tag == "[basis of rarity dicision]") {
            for (int drop_type = 1; drop_type <= 4; ++drop_type) {
                for (int rarity = 0; rarity <= 5; ++rarity) {
                    rarity_decision_[drop_type][rarity] = ScanInt(&success);
                    if (!success) return 1683;
                }
            }
        } else if (tag == "[party member drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int party_members = 0; party_members <= 3;
                     ++party_members) {
                    party_member_bonus_[drop_type][party_members] =
                        ScanFloat(&success);
                    if (!success) return 1694;
                }
            }
        } else if (tag == "[dungeon difficulty drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int difficulty = 0; difficulty <= 4; ++difficulty) {
                    dungeon_difficulty_bonus_[drop_type][difficulty] =
                        ScanFloat(&success);
                    if (!success) return 1709;
                }
            }
        } else if (tag == "[monster type drop bonusrate]") {
            for (int drop_type = 0; drop_type <= 4; ++drop_type) {
                for (int monster_type = 0; monster_type <= 3;
                     ++monster_type) {
                    monster_type_bonus_[drop_type][monster_type] =
                        ScanFloat(&success);
                    if (!success) return 1720;
                }
            }
        } else if (tag == "[item drop ref table]") {
            item_ref_range_[0][0] = 0;
            item_ref_range_[0][1] = 0;
            for (int row = 1; row <= 200; ++row) {
                const int level = ScanInt(&success);
                if (!success) return 1733;
                item_ref_range_[level][0] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 1737;
                item_ref_range_[level][1] =
                    static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 1741;
            }
        } else if (tag == "[first boss/named mob hunting]") {
            first_mob_hunting_weight_.first_boss_weight = 1;
            first_mob_hunting_weight_.first_named_mob_weight = 1;
            first_mob_hunting_weight_.first_boss_weight = ScanInt(&success);
            if (!success) return 1751;
            first_mob_hunting_weight_.first_named_mob_weight = ScanInt(&success);
            if (!success) return 1755;
        } else if (tag == "[condition rate]") {
            for (int type = 0; type <= 3; ++type) {
                boss_gold_drop_.condition_rate[type][0] =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) return 1765;
                boss_gold_drop_.condition_rate[type][1] =
                    static_cast<uint16_t>(ScanInt(&success));
                if (!success) return 1769;
            }
        } else if (tag == "[gold quantity]") {
            for (int selection = 0; selection <= 1; ++selection) {
                for (int bound = 0; bound <= 1; ++bound) {
                    boss_gold_drop_.gold_quantity[selection][bound] =
                        static_cast<uint16_t>(ScanInt(&success));
                    if (!success) {
                        return 1776 + 4 * (2 * selection + bound);
                    }
                }
            }
        } else if (tag == "[gold volume]") {
            boss_gold_drop_.gold_volume[0] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) return 1794;
            boss_gold_drop_.gold_volume[1] =
                static_cast<uint16_t>(ScanInt(&success));
            if (!success) return 1798;
        } else {
            return 1802;
        }
    }

    item_rate_table_.generateTable(item_ref_range_, true);
    return 0;
}

// IDA_EA: 0x853A7F8
// IDA_NAME: _ZN8WongWork11CObjectDropC2Ei
// DEMANGLED: WongWork::CObjectDrop::CObjectDrop(int)
CObjectDrop::CObjectDrop(int random_stream_count)
    : CMonsterDrop(random_stream_count) {}

// IDA_EA: 0x8538352
// IDA_NAME: _ZN8WongWork11CObjectDrop10loadScriptEPKc
// DEMANGLED: WongWork::CObjectDrop::loadScript(char const*)
int CObjectDrop::loadScript(const char* filename) {
    for (int drop_type = 0; drop_type <= 4; ++drop_type) {
        for (int party_members = 0; party_members <= 3; ++party_members) {
            party_member_bonus_[drop_type][party_members] = 1.0f;
        }
    }
    return CMonsterDrop::loadScript(filename);
}

// IDA_EA: 0x85380FA
// IDA_NAME: _ZN8WongWork12CMonsterDrop10loadScriptEPKcS2_S2_
// DEMANGLED: WongWork::CMonsterDrop::loadScript(char const*,char const*,char const*)
int CMonsterDrop::loadScript(const char* normal_filename,
                             const char* hell_filename,
                             const char* extra_filename) {
    // IDA 0x8538117 calls vptr +0x04 here.  Keep virtual dispatch so an
    // object-drop instance first executes its own loader preamble before
    // entering the normal CMonsterDrop parser.
    if (loadScript(normal_filename) != 0) {
        return 1813;
    }
    const int hell_status = hell_drop_.loadScript(hell_filename);
    if (hell_status != 0) {
        return hell_status;
    }
    if (extra_filename != 0) {
        const int extra_status = special_drop_.loadScript(extra_filename);
        if (extra_status != 0) {
            return extra_status;
        }
    }
    return 0;
}

}  // namespace WongWork
