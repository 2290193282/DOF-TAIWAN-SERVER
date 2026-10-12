#include "old_game/social/party/party.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/village_attacked_runtime.h"


namespace {

CInventory* CurrentInventoryR(CUser* user) {
    return static_cast<CInventory*>(user->getCurCharacInvenR());
}

int SnapshotItemKey(CUser* user, int slot) {
    Inven_Item item = CurrentInventoryR(user)->GetInvenSlot(0, slot);
    return item.getKey();
}

bool InInclusiveRange(int value, int first, int last) {
    return first <= value && value <= last;
}

}  // namespace

// IDA_EA: 0x85B916E
// IDA_NAME: _ZN6CParty33Reset_party_overlapped_drop_ratioEv
// DEMANGLED: CParty::Reset_party_overlapped_drop_ratio(void)
char CParty::Reset_party_overlapped_drop_ratio() {
    party_overlapped_drop_ratio_rare_ = 0;
    party_overlapped_drop_ratio_unique_ = 0;

    int item_24191_count = 0;
    int item_242xx_count = 0;
    int item_221xx_count = 0;
    int item_201xx_count = 0;
    bool all_three_special_slots = false;

    for (int seat = 0; seat <= 3; ++seat) {
        if (static_cast<unsigned char>(this->_checkValidUser(seat)) !=
                1u ||
            battle_data_.member_live_[seat] == 0u) {
            continue;
        }

        CUser* const user = members_[seat].user;
        int special_slot_count = 0;
        const int item_slot_19 = SnapshotItemKey(user, 19);
        const int item_slot_18 = SnapshotItemKey(user, 18);
        const int item_slot_17 = SnapshotItemKey(user, 17);

        if (InInclusiveRange(item_slot_19, 0x5e80, 0x5e87)) {
            ++item_242xx_count;
            ++special_slot_count;
        } else if (item_slot_19 == 24191) {
            ++item_24191_count;
        } else if (item_slot_19 == 24281) {
            ++item_242xx_count;
            ++special_slot_count;
        }

        if (InInclusiveRange(item_slot_18, 0x5693, 0x569a)) {
            ++item_221xx_count;
            ++special_slot_count;
        } else if (item_slot_19 == 24281) {
            ++item_221xx_count;
            ++special_slot_count;
        }

        if (InInclusiveRange(item_slot_17, 0x4ecb, 0x4ed2)) {
            ++item_201xx_count;
            ++special_slot_count;
        } else if (item_slot_19 == 24281) {
            ++item_201xx_count;
            ++special_slot_count;
        }

        if (special_slot_count == 3) {
            all_three_special_slots = true;
        }
    }

    if (item_24191_count) {
        party_overlapped_drop_ratio_rare_ += item_24191_count + 2;
    }
    if (item_242xx_count) {
        party_overlapped_drop_ratio_rare_ += 2 * (item_242xx_count + 2);
        party_overlapped_drop_ratio_unique_ += item_242xx_count + 2;
    }
    if (item_221xx_count) {
        party_overlapped_drop_ratio_rare_ += 2 * (item_221xx_count + 2);
        party_overlapped_drop_ratio_unique_ += item_221xx_count + 2;
    }
    if (item_201xx_count) {
        party_overlapped_drop_ratio_rare_ += 2 * (item_201xx_count + 2);
        party_overlapped_drop_ratio_unique_ += item_201xx_count + 2;
    }
    if (all_three_special_slots) {
        party_overlapped_drop_ratio_rare_ += 5;
        party_overlapped_drop_ratio_unique_ += 2;
    }
    return static_cast<char>(reinterpret_cast<uintptr_t>(this));
}

// IDA_EA: 0x85BB0AC
// IDA_NAME: _ZN6CParty24CheckHackAverageHitCountEv
// DEMANGLED: CParty::CheckHackAverageHitCount(void)
char CParty::CheckHackAverageHitCount() {
    if (static_cast<unsigned char>(
            reinterpret_cast<village_attacked::CVillageMonsterMgr*>(
                GlobalData::s_villageMonsterMgr)->OnEvent()) != 0u) {
        return 1;
    }

    int result = map_hit_statistics_.map_count_;
    if (result > 45) {
        return static_cast<char>(result);
    }

    const CDungeon* const dungeon = battle_field_.dungeon_;
    const bool excluded =
        (dungeon &&
         (dungeon->ancient_dungeon_ != 0u ||
          static_cast<signed char>(dungeon->get_dimension_possible()) >
              0)) ||
        result <= 1;
    if (excluded) {
        return 1;
    }

    const float map_count = static_cast<float>(result);
    const float hit_average =
        static_cast<float>(map_hit_statistics_.total_client_hit_count_) /
        map_count;
    const int hit_high = static_cast<int>(hit_average + hit_average * 0.1f +
                                           0.5f);
    const int hit_low = static_cast<int>(hit_average - hit_average * 0.1f);

    const float time_average =
        static_cast<float>(map_hit_statistics_.total_monster_field_a1a_) /
        map_count;
    const int time_high =
        static_cast<int>(time_average + time_average * 0.1f + 0.5f);
    const int time_low = static_cast<int>(time_average - time_average * 0.1f);

    const float third_average =
        static_cast<float>(map_hit_statistics_.total_monster_field_a1e_) /
        map_count;
    const int third_high =
        static_cast<int>(third_average + third_average * 0.1f + 0.5f);
    const int third_low =
        static_cast<int>(third_average - third_average * 0.1f);

    int hit_in_range = 0;
    int time_in_range = 0;
    int zero_time_count = 0;
    int third_in_range = 0;
    int zero_third_count = 0;
    for (int map_index = 0; map_index < result; ++map_index) {
        const int hit_value =
            map_hit_statistics_.client_hit_counts_[map_index];
        if (InInclusiveRange(hit_value, hit_low, hit_high)) {
            ++hit_in_range;
        }

        const int time_value =
            map_hit_statistics_.monster_field_a1a_[map_index];
        if (time_value) {
            if (InInclusiveRange(time_value, time_low, time_high)) {
                ++time_in_range;
            }
        } else {
            ++zero_time_count;
        }

        const int third_value =
            map_hit_statistics_.monster_field_a1e_[map_index];
        if (third_value) {
            if (InInclusiveRange(third_value, third_low, third_high)) {
                ++third_in_range;
            }
        } else {
            ++zero_third_count;
        }
    }

    const auto add_party_hack = [this](int hack_type) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (static_cast<unsigned char>(_checkValidUser(seat)) != 1u) {
                continue;
            }
            CUser* const user = members_[seat].user;
            reinterpret_cast<WongWork::CHackAnalyzer*>((user->getHackAnalyzer()))->addServerHackCnt(
                                                    user,
                                                    static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
                                                    1,
                                                    0,
                                                    0);
        }
    };

    if (result / 2 <= hit_in_range) {
        add_party_hack(0xd9);
    }

    const int nonzero_time_count = result - zero_time_count;
    if (nonzero_time_count > 0 && nonzero_time_count / 2 <= time_in_range) {
        add_party_hack(0xda);
    }

    const int nonzero_third_count = result - zero_third_count;
    if (nonzero_third_count > 0) {
        result = nonzero_third_count / 2;
        if (result <= third_in_range) {
            add_party_hack(0xdb);
        }
    } else {
        result = 0;
    }
    return static_cast<char>(result);
}
