#include "old_data/PVF/data/item_generator_drop_types.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "common/config.h"
#include "old_data/PVF/aicharacter/datamanager/ai_character_runtime.h"
#include "old_data/PVF/aicharacter/datamanager/monster_manager.h"
#include "old_data/PVF/aicharacter/datamanager/monster_script.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_data/PVF/map/datamanager/world_drop_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "common/manager/server_utility.h"
#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/gm/player/game_master_character.h"

extern "C++" int roundFloat(float value);
extern "C++" int GetRandomRange(int minimum, int maximum);

namespace WongWork {
namespace {

bool AppendGeneratedItem(stGenerateResult_t& result,
                         int item_id,
                         int add_info) {
    CItemList* const item_list =
        G_CDataManager()->get_arad_jumping_item_list();
    Inven_Item item;
    item.item_id_ = item_id;
    item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT, item, add_info);
    result.items.push_back(item);
    return true;
}

bool AppendKnownGeneratedItem(stGenerateResult_t& result,
                              int item_id,
                              int add_info) {
    CItemList* const item_list =
        G_CDataManager()->get_arad_jumping_item_list();
    if (!item_list->find_item(item_id)) return false;
    Inven_Item item;
    item.item_id_ = item_id;
    item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT, item, add_info);
    result.items.push_back(item);
    return true;
}

int ClearRewardDropType(const std::string& name) {
    if (name == "default") return 0;
    if (name == "event") return 1;
    if (name == "event2") return 2;
    if (name == "pcroom default") return 3;
    if (name == "pcroom event") return 4;
    if (name == "pcroom bonus") return 5;
    return -1;
}

// IDA 0x8535FF5 / 0x8536497 / 0x8536AD2 changes the x87 control word so the
// weighted probability is stored by truncating toward zero, before the
// standard-drop-rate integer scaling.
int TruncateDropProbability(long double value) {
    return static_cast<int>(value);
}

}  // namespace

// IDA_EA: 0x814A706
// IDA_NAME: _ZN8WongWork19stGenerateRefData_tC2Ev
stGenerateRefData_t::stGenerateRefData_t()
    : standard_drop_rate(100),
      clear_reward_scale(0.0f),
      flags(),
      monster_random_drop_scale(1.0f),
      premium_independent_drop_rate(0),
      party_member_force_drop(0),
      blue_marble_context(0),
      reserved_26(0),
      blue_marble_character_job(oldsrv::config::kCharacterJobInvalid),
      blue_marble_character_level(0),
      blue_marble_buff_value(0) {
    flags.push_back(false);
    flags.push_back(false);
}

// IDA_EA: 0x814AAF0
// IDA_NAME: _ZN8WongWork19stGenerateRefData_tD2Ev
stGenerateRefData_t::~stGenerateRefData_t() {}

// IDA_EA: 0x814AB06
// IDA_NAME: _ZN8WongWork18stGenerateResult_tC2Ev
stGenerateResult_t::stGenerateResult_t() : items(), luck_point() {}

// IDA_EA: 0x814AB4C
// IDA_NAME: _ZN8WongWork18stGenerateResult_tD2Ev
stGenerateResult_t::~stGenerateResult_t() {}

}  // namespace WongWork

// IDA_EA: 0x8550884 / 0x85508EC
CLuckPoint::CLuckPoint() : users_(), user_gain_item_index_() {}
CLuckPoint::~CLuckPoint() {}

// IDA_EA: 0x8550932 / 0x8550954 / 0x85509DA
CUser* CLuckPoint::AddUser(CUser* user) {
    if (user) users_.push_back(user);
    return user;
}

// IDA_EA: 0x8550954
// IDA_NAME: _ZN10CLuckPoint25_GetMaxLuckPointUserIndexEv
int CLuckPoint::_GetMaxLuckPointUserIndex() {
    int max_luck_point = 0;
    int max_index = 0;
    for (size_t index = 0; index < users_.size(); ++index) {
        const int luck_point = users_[index]->GetCurCharacLuckPoint();
        if (luck_point > max_luck_point) {
            max_luck_point = luck_point;
            max_index = static_cast<int>(index);
        }
    }
    return max_index;
}

// IDA_EA: 0x85509DA
// IDA_NAME: _ZN10CLuckPoint12GetLuckPointEv
unsigned int CLuckPoint::GetLuckPoint() {
    if (users_.empty()) return 0;
    unsigned int total = 0;
    for (size_t index = 0; index < users_.size(); ++index) {
        if (users_[index]) {
            total += static_cast<unsigned int>(
                users_[index]->GetCurCharacLuckPoint());
        }
    }
    return total / static_cast<unsigned int>(users_.size());
}

// IDA_EA: 0x8550B14
// IDA_NAME: _ZN10CLuckPoint12UseLuckPointEiiii
int CLuckPoint::UseLuckPoint(int use_luck_point,
                             int rarity,
                             int item_grade,
                             int item_index) {
    if (users_.empty()) return 0;
    const int user_index = _GetMaxLuckPointUserIndex();
    if (user_index < 0 || static_cast<size_t>(user_index) >= users_.size()) {
        return 0;
    }
    CUser* const user = users_[user_index];
    if (!user ||
        CLuckPoint::UseLuckPoint(user, use_luck_point, rarity, item_grade) !=
            1) {
        return 0;
    }
    user_gain_item_index_[item_index] = user_index;
    return 1;
}

// IDA_EA: 0x8550BE4
// IDA_NAME: _ZN10CLuckPoint13GetItemRarityEPiii
int CLuckPoint::GetItemRarity(int* rarity_weight,
                              int random_number,
                              int drop_type) {
    int modified_weight[6] = {0, 0, 0, 0, 0, 0};
    if (drop_type == 2 && !users_.empty()) {
        int luck_point_rate = 0;
        G_CDataManager()->get_server_parameter_script()->getLuckPoint(
            static_cast<int>(GetLuckPoint()), luck_point_rate);

        const int rare_range = rarity_weight[2] - rarity_weight[1];
        int rare_modifier =
            -GetTenThousandPercentage(rare_range, luck_point_rate);
        int unique_party_ratio = 0;
        int rare_party_ratio = 0;
        for (size_t index = 0; index < users_.size(); ++index) {
            if (!users_[index]) continue;
            CParty* const party = users_[index]->GetParty();
            if (party) {
                unique_party_ratio =
                    party->Get_party_overlapped_drop_ratio_unique();
                rare_party_ratio =
                    party->Get_party_overlapped_drop_ratio_rare();
            }
            break;
        }
        if (unique_party_ratio) {
            modified_weight[3] -= static_cast<int>(
                static_cast<long double>(rarity_weight[3] - rarity_weight[2]) *
                (static_cast<long double>(unique_party_ratio) * 0.01L));
            if (rarity_weight[3] > 999999) modified_weight[3] = 0;
        }
        if (rare_party_ratio) {
            rare_modifier -= static_cast<int>(
                static_cast<long double>(rare_range) *
                (static_cast<long double>(rare_party_ratio) * 0.01L));
        }
        if (rarity_weight[1]) {
            modified_weight[0] = rare_modifier * rarity_weight[0] /
                                 rarity_weight[1];
        }
        modified_weight[1] = rare_modifier;
    }
    for (int rarity = 0; rarity <= 5; ++rarity) {
        if (rarity_weight[rarity] + modified_weight[rarity] >=
            random_number) {
            return rarity;
        }
    }
    return 0;
}

// IDA_EA: 0x8550E50
// IDA_NAME: _ZN10CLuckPoint15GetUserGainItemEi
CUser* CLuckPoint::GetUserGainItem(int item_index) {
    if (item_index < 0 ||
        static_cast<size_t>(item_index) >= user_gain_item_index_.size()) {
        return 0;
    }
    const int user_index = user_gain_item_index_[item_index];
    if (user_index < 0 || static_cast<size_t>(user_index) >= users_.size()) {
        return 0;
    }
    return users_[user_index];
}

// IDA_EA: 0x8550EF0
// IDA_NAME: _ZN10CLuckPoint16sendItemDropRateEPKc
bool CLuckPoint::sendItemDropRate(const char* message) {
    bool result = false;
    for (size_t index = 0; index < users_.size(); ++index) {
        result = users_.size() > index;
        CUser* const user = users_[index];
        if (user && user->getCurCharacR()) {
            user->GetGameMasterCharacter()->sendServerState(user, message);
        }
    }
    return result;
}

namespace WongWork {

// IDA_EA: 0x85383B8
// IDA_NAME: _ZN8WongWork10CWorldDrop18generateRandomItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
void CWorldDrop::generateRandomItem(const stGenerateRefData_t& reference,
                                    stGenerateResult_t& result) {
    const uint8_t world_area = reference.generator_data.world_drop.world_area;
    const uint8_t world_index =
        reference.generator_data.world_drop.world_index;
    if (world_index > 199u) return;

    WorldDropInfo& info = world_area == 1u
        ? war_area_drop_[world_index] : normal_drop_[world_index];
    const int random_number = random_number_.generateNumber(0, 100000);
    const int drop_limit = reference.standard_drop_rate *
                           info.drop_rate_or_total_weight / 100;
    if (random_number > drop_limit) return;

    const int selected = random_number_.generateNumber(1, info.total_weight);
    int cumulative_weight = 0;
    for (std::vector<WorldDropItemChunck>::const_iterator it =
             info.item_chunks.begin();
         it != info.item_chunks.end(); ++it) {
        cumulative_weight += it->weight;
        if (selected < cumulative_weight) {
            AppendGeneratedItem(result, it->item_id, 1);
            return;
        }
    }
}

// IDA_EA: 0x853855C
// IDA_NAME: _ZN8WongWork10CWorldDrop10loadScriptEPKcS2_
int CWorldDrop::loadScript(const char* normal_filename,
                           const char* war_area_filename) {
    if (ImportWorldDrop(normal_drop_, normal_filename) != 1) return 1977;
    return ImportWorldDrop(war_area_drop_, war_area_filename) == 1 ? 0 : 1979;
}

// IDA_EA: 0x853A81C
// IDA_NAME: _ZN8WongWork10CWorldDrop10loadScriptEPKc
int CWorldDrop::loadScript(const char*) { return 0; }

// IDA_EA: 0x853A826
// IDA_NAME: _ZN8WongWork10CWorldDropC2Ei
CWorldDrop::CWorldDrop(int random_stream_count)
    : CItemGenerator(random_stream_count), normal_drop_(), war_area_drop_() {}

// IDA_EA: 0x8539C3E
// IDA_NAME: _ZN8WongWork12CDungeonDrop20generateSpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
void CDungeonDrop::generateSpecificItem(const stGenerateRefData_t& reference,
                                        stGenerateResult_t& result) {
    const std::vector<std::pair<int, int> >* const items =
        reference.generator_data.dungeon_drop.items;
    const int number = random_number_.generateNumber(1, 10000);
    int cumulative_weight = 0;
    for (std::vector<std::pair<int, int> >::const_iterator it = items->begin();
         it != items->end(); ++it) {
        cumulative_weight += it->second;
        if (number < cumulative_weight) {
            // The original constructs an Inven_Item, writes its +0x02 item
            // id, then calls CItemList::create_item with create-type 1.  Do
            // not route this through the generic generator helper: that
            // helper deliberately serves other generator families and uses
            // their create-type policy.
            Inven_Item item;
            item.item_id_ = it->first;
            G_CDataManager()->get_arad_jumping_item_list()->create_item(
                static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
            result.items.push_back(item);
            return;
        }
    }
}

// IDA_EA: 0x8538234
// IDA_NAME: _ZN8WongWork11CObjectDrop20generateSpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CObjectDrop::generateSpecificItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
//
// The CStage caller dispatches this through CItemGenerator's vptr + 0x0c and
// ignores EAX.  Its source-level return is therefore void, matching the base
// virtual slot and the calling ABI rather than Hex-Rays' incidental AL value.
void CObjectDrop::generateSpecificItem(const stGenerateRefData_t& reference,
                                       stGenerateResult_t& result) {
    const stGenerateRefObjectDrop_t& object_drop =
        reference.generator_data.object_drop;

    for (int index = 0; index < object_drop.item_count; ++index) {
        const int number = random_number_.generateNumber(2, 10000);
        int cumulative_weight = 0;
        for (std::list<std::pair<int, int> >::const_iterator it =
                 object_drop.items->begin();
             it != object_drop.items->end();
             ++it) {
            cumulative_weight += it->second;
            if (number < cumulative_weight) {
                // IDA 0x85382B6..0x85382E8: C2, direct +0x02 id write,
                // CItemList::create_item(1, item, 1), then vector::push_back.
                Inven_Item item;
                item.item_id_ = it->first;
                G_CDataManager()->get_arad_jumping_item_list()->create_item(
                    static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
                result.items.push_back(item);
                break;
            }
        }
    }
}

// IDA_EA: 0x853ABC2
// IDA_NAME: _ZN8WongWork12CDungeonDropC2Ei
CDungeonDrop::CDungeonDrop(int random_stream_count)
    : CItemGenerator(random_stream_count) {}

// IDA_EA: 0x85385B2 / 0x85385C8 / 0x8538694
// IDA_EA: 0x853F17E
// IDA_NAME: _ZN8WongWork12CClearReward25stGoldCardBlankItemInfo_tD2Ev
CClearReward::stGoldCardBlankItemInfo_t::~stGoldCardBlankItemInfo_t() {}

// IDA_EA: 0x85385B2
// IDA_NAME: _ZN8WongWork12CClearReward21getGoldCardCommissionEi
int CClearReward::getGoldCardCommission(int standard_level) {
    return gold_card_commission_[standard_level];
}

// IDA_EA: 0x85385C8
// IDA_NAME: _ZN8WongWork12CClearReward12setDropRatioEf
// DEMANGLED: WongWork::CClearReward::setDropRatio(float)
void CClearReward::setDropRatio(float ratio) {
    for (int kind = 0; kind <= 5; ++kind) {
        active_probability_[kind] = configured_probability_[kind];
        for (std::vector<stGenItemProb_t>::iterator it =
                 active_probability_[kind].begin();
             it != active_probability_[kind].end(); ++it) {
            it->probability = roundFloat(it->probability * ratio);
        }
    }
}

// IDA_EA: 0x8538694
// IDA_NAME: _ZN8WongWork12CClearReward18_getGenItemProbIdxEiNS_22eClearRewardDropProb_tE
int CClearReward::_getGenItemProbIdx(int character_level,
                                     int drop_probability) {
    if (drop_probability < 0 || drop_probability >= 6) return -1;
    const std::vector<stGenItemProb_t>& rows =
        active_probability_[drop_probability];
    for (size_t index = 0; index < rows.size(); ++index) {
        if (rows[index].first_level <= character_level &&
            rows[index].last_level >= character_level) return index;
    }
    return -1;
}

// IDA_EA: 0x85387C0
// IDA_EA: 0x85387C0
// IDA_NAME: _ZN8WongWork12CClearReward19_generateRandomItemERKNS_26stGenerateRefClearReward_tEiRNS_18stGenerateResult_tEf
int CClearReward::_generateRandomItem(
    const stGenerateRefClearReward_t& reference,
    int standard_drop_rate,
    stGenerateResult_t& result,
    float clear_reward_scale) {
    const int row_index = _getGenItemProbIdx(
        reference.character_level_or_world_area,
        reference.clear_reward_drop_probability);
    if (row_index < 0) return 0;
    const stGenItemProb_t& probability = active_probability_
        [reference.clear_reward_drop_probability][row_index];
    const int scaled_probability = TruncateDropProbability(
        static_cast<long double>(probability.probability) *
        static_cast<long double>(reference.item_probability_scale));
    const int weighted_probability =
        (standard_drop_rate / 100) *
        (scaled_probability +
         dungeon_difficulty_drop_bonus_
             [reference.dungeon_difficulty_or_world_index]);
    const int threshold = TruncateDropProbability(
        static_cast<long double>(weighted_probability) *
        (1.0L - static_cast<long double>(clear_reward_scale)));
    if (random_number_.generateNumber(0, 10000) >= threshold) return 0;

    int drop_type = 1;
    int cumulative_probability = 0;
    const int type_number = random_number_.generateNumber(2, 10000);
    for (int candidate = 1; candidate <= 4; ++candidate) {
        cumulative_probability += drop_item_type_probability_[candidate - 1];
        if (type_number < cumulative_probability) {
            drop_type = candidate;
            break;
        }
    }
    const int rarity = result.luck_point.GetItemRarity(
        rarity_decision_, random_number_.generateNumber(3, 1000000), drop_type);
    G_CDataManager()->getItemGenerator()->onTryGenerateRandomItem(
        static_cast<CItemGeneratorMgr::eGenerateType_t>(getGeneratorType()),
        drop_type, rarity);
    const int item_id = item_rate_table_.chooseItem(
        drop_type, rarity, reference.character_level_or_world_area);
    return item_id != -1 && AppendKnownGeneratedItem(result, item_id, 1);
}

// IDA_EA: 0x8538BF8
// IDA_EA: 0x8538BF8
// IDA_NAME: _ZN8WongWork12CClearReward21_generateSpecificItemERKNS_26stGenerateRefClearReward_tERNS_18stGenerateResult_tE
int CClearReward::_generateSpecificItem(
    const stGenerateRefClearReward_t& reference,
    stGenerateResult_t& result) {
    const int item_id = reference.dungeon->get_clear_item(
        random_number_.generateNumber(4, 100));
    return item_id != -2 && AppendGeneratedItem(result, item_id, 1);
}

// IDA_EA: 0x853873A
void CClearReward::generateRandomItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result) {
    if (random_number_.generateNumber(1, 10000) >= drop_kind_probability_[0]) {
        _generateSpecificItem(reference.generator_data.clear_reward, result);
        return;
    }
    _generateRandomItem(reference.generator_data.clear_reward,
                        reference.standard_drop_rate,
                        result,
                        reference.clear_reward_scale);
}

// IDA_EA: 0x8538C84
void CClearReward::generateSpecificItem(const stGenerateRefData_t& reference,
                                        stGenerateResult_t& result) {
    const stGenerateRefGoldReward_t& gold_reference =
        reference.generator_data.gold_reward;
    GoldGenerateRange* const gold = common_data_->getGoldGenTable(
        gold_reference.character_level);
    const float reward_multiplier = static_cast<float>(
        (G_CDataManager()->get_server_parameter_script()->GetExpBonusWeight(
             static_cast<unsigned int>(gold_reference.exp_bonus_context)) +
         1.0L) *
        static_cast<long double>(
            dungeon_difficulty_gold_bonus_[gold_reference.dungeon_difficulty]) *
        static_cast<long double>(
            party_member_drop_bonus_[gold_reference.party_member_count]));
    const int base_gold = 175 * gold->minimum / 1000;
    const int gold_weight = static_cast<int>(
        (static_cast<long double>(
             (base_gold * (gold_reference.gold_weight_08 +
                           2 * (2 * gold_reference.gold_weight_0e +
                                gold_reference.gold_weight_0a))) / 2) *
         reward_multiplier));
    const int party_count = gold_reference.party_member_count;
    for (int index = 0; index < party_count; ++index) {
        const int party_gold = static_cast<int>(
            (static_cast<long double>(gold_weight) / 2.0L /
             static_cast<long double>(party_count)) * reward_multiplier);
        Inven_Item item;
        item.item_category_ = 2u;
        item.set_add_info((GetRandomRange(-gold->maximum, gold->maximum) + 100) *
                          party_gold / 100);
        result.items.push_back(item);
    }
}

// IDA_EA: 0x8538A92
// IDA_NAME: _ZN8WongWork12CClearReward27generatePremiumGoldCardItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE11ENUM_RARITYj
// DEMANGLED: WongWork::CClearReward::generatePremiumGoldCardItem(WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &,ENUM_RARITY,unsigned int)
int CClearReward::generatePremiumGoldCardItem(
    const stGenerateRefData_t& reference,
    stGenerateResult_t& result,
    int rarity,
    unsigned int fallback_item_id) {
    int selected_rarity = 0;
    if (rarity >= 0) {
        selected_rarity = rarity;
    } else {
        const unsigned int random_number = random_number_.generateNumber(
            3, 10000);
        ServerParameterScript* const parameters =
            G_CDataManager()->get_server_parameter_script();
        for (int candidate = 0; candidate <= 5; ++candidate) {
            if (parameters->getPremiumGoldCardRarityDecision(candidate) >=
                static_cast<int>(random_number)) {
                selected_rarity = candidate;
                break;
            }
        }
    }
    G_CDataManager()->getItemGenerator()->onTryGenerateRandomItem(
        static_cast<CItemGeneratorMgr::eGenerateType_t>(getGeneratorType()),
        2,
        selected_rarity);
    const int item_id = item_rate_table_.chooseItem(
        2, selected_rarity,
        reference.generator_data.clear_reward.character_level_or_world_area);
    return AppendKnownGeneratedItem(result,
        item_id == -1 ? static_cast<int>(fallback_item_id) : item_id, 1);
}

// IDA_EA: 0x8538E64
// IDA_NAME: _ZN8WongWork12CClearReward35power_war_dungeon_gold_clear_rewardERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tEi
// DEMANGLED: WongWork::CClearReward::power_war_dungeon_gold_clear_reward(WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &,int)
int CClearReward::power_war_dungeon_gold_clear_reward(
    const stGenerateRefData_t& reference,
    stGenerateResult_t& result,
    int character_level) {
    const stGenerateRefGoldReward_t& gold_reference =
        reference.generator_data.gold_reward;
    GoldGenerateRange* const gold = common_data_->getGoldGenTable(
        static_cast<unsigned char>(character_level));
    const float reward_multiplier = static_cast<float>(
        (G_CDataManager()->get_server_parameter_script()->GetExpBonusWeight(
             static_cast<unsigned int>(gold_reference.exp_bonus_context)) +
         1.0L) *
        static_cast<long double>(
            dungeon_difficulty_gold_bonus_[gold_reference.dungeon_difficulty]) *
        static_cast<long double>(
            party_member_drop_bonus_[gold_reference.party_member_count]));
    const int base_gold = 175 * gold->minimum / 1000;
    const int gold_weight = static_cast<int>(
        static_cast<long double>(
            (base_gold * (gold_reference.gold_weight_08 +
                          2 * (2 * gold_reference.gold_weight_0e +
                               gold_reference.gold_weight_0a))) / 2) *
        reward_multiplier);
    const int party_gold = static_cast<int>(
        (static_cast<long double>(gold_weight) / 2.0L /
         static_cast<long double>(gold_reference.party_member_count)) *
        reward_multiplier);
    Inven_Item item;
    item.item_category_ = 2u;
    item.set_add_info((GetRandomRange(-gold->maximum, gold->maximum) + 100) *
                      party_gold / 100);
    result.items.push_back(item);
    return 1;
}

// IDA_EA: 0x853900C
// IDA_NAME: _ZN8WongWork12CClearReward27generatePcRoomCardBlankItemER10Inven_Item
// DEMANGLED: WongWork::CClearReward::generatePcRoomCardBlankItem(Inven_Item &)
int CClearReward::generatePcRoomCardBlankItem(Inven_Item& item) {
    if (pcroom_card_blank_items_.empty()) return 0;
    int number = get_rand_int(pcroom_card_blank_total_weight_);
    for (std::vector<stGoldCardBlankItemInfo_t>::const_iterator it =
             pcroom_card_blank_items_.begin();
         it != pcroom_card_blank_items_.end(); ++it) {
        number -= it->weight;
        if (number < 0 && it->item_id != -1) {
            item.item_id_ = it->item_id;
            G_CDataManager()->get_arad_jumping_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, item, it->add_info);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8539166
// IDA_NAME: _ZN8WongWork12CClearReward25generateGoldCardBlankItemER10Inven_Item
// DEMANGLED: WongWork::CClearReward::generateGoldCardBlankItem(Inven_Item &)
int CClearReward::generateGoldCardBlankItem(Inven_Item& item) {
    if (gold_card_blank_items_.empty()) return 0;
    int number = get_rand_int(gold_card_blank_total_weight_);
    for (std::vector<stGoldCardBlankItemInfo_t>::const_iterator it =
             gold_card_blank_items_.begin();
         it != gold_card_blank_items_.end(); ++it) {
        number -= it->weight;
        if (number < 0 && it->item_id != -1) {
            item.item_id_ = it->item_id;
            G_CDataManager()->get_arad_jumping_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT, item, it->add_info);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x85392C0
int CClearReward::loadScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) return 2371;
    bool success = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[drop prob count]") {
            const int count = ScanInt(&success);
            if (!success) return 2385;
            for (int kind = 0; kind <= 5; ++kind)
                configured_probability_[kind].resize(count);
        } else if (tag == "[drop prob]") {
            while (true) {
                std::string drop_kind;
                if (ScanStr(&drop_kind) != 1) break;
                const int kind = ClearRewardDropType(drop_kind);
                if (kind < 0) return 2409;
                for (size_t row = 0;
                     row < configured_probability_[kind].size();
                     ++row) {
                    stGenItemProb_t& probability =
                        configured_probability_[kind][row];
                    probability.first_level = ScanInt(&success);
                    if (!success) return 2415;
                    probability.last_level = ScanInt(&success);
                    if (!success) return 2417;
                    probability.probability = ScanInt(&success);
                    if (!success) return 2419;
                }
            }
        } else if (tag == "[drop kind prob]") {
            for (int index = 0; index < 2; ++index) {
                drop_kind_probability_[index] = ScanInt(&success);
                if (!success) return index ? 2428 : 2426;
            }
        } else if (tag == "[drop item type prob]") {
            for (int index = 0; index < 4; ++index) {
                drop_item_type_probability_[index] = ScanInt(&success);
                if (!success) return 2436;
            }
        } else if (tag == "[basis of rarity dicision]") {
            for (int index = 0; index < 6; ++index) {
                rarity_decision_[index] = ScanInt(&success);
                if (!success) return 2445;
            }
        } else if (tag == "[dungeon difficulty drop bonusrate]") {
            for (int index = 0; index < 5; ++index) {
                dungeon_difficulty_drop_bonus_[index] = ScanInt(&success);
                if (!success) return 2458;
            }
        } else if (tag == "[party member drop bonusrate]") {
            for (int index = 0; index < 4; ++index) {
                party_member_drop_bonus_[index] = ScanFloat(&success);
                if (!success) return 2467;
            }
        } else if (tag == "[dungeon difficulty gold drop bonusrate]") {
            for (int index = 0; index < 5; ++index) {
                dungeon_difficulty_gold_bonus_[index] = ScanFloat(&success);
                if (!success) return 2480;
            }
        } else if (tag == "[item drop ref table]") {
            item_ref_range_[0][0] = item_ref_range_[0][1] = 0;
            for (int row = 1; row <= 200; ++row) {
                const int level = ScanInt(&success);
                if (!success) return 2492;
                item_ref_range_[level][0] = static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 2496;
                item_ref_range_[level][1] = static_cast<unsigned char>(ScanInt(&success));
                if (!success) return 2500;
            }
        } else if (tag == "[gold card cost table]") {
            while (true) {
                const int level = ScanInt(&success);
                if (!success) break;
                gold_card_commission_[level] = ScanInt(&success);
                if (!success) return 2513;
            }
        } else if (tag == "[gold card create rate]") {
            gold_card_create_rate_ = ScanFloat(&success);
            if (!success) return 2520;
        } else if (tag == "[gold card blank item]" ||
                   tag == "[pcroom card blank item]") {
            std::vector<stGoldCardBlankItemInfo_t>& items =
                tag == "[gold card blank item]" ? gold_card_blank_items_
                                                  : pcroom_card_blank_items_;
            int& total_weight = tag == "[gold card blank item]"
                ? gold_card_blank_total_weight_ : pcroom_card_blank_total_weight_;
            total_weight = 0;
            while (true) {
                stGoldCardBlankItemInfo_t row;
                row.weight = ScanInt(&success);
                if (!success) break;
                row.item_id = ScanInt(&success);
                if (!success) return tag == "[gold card blank item]" ? 2568 : 2534;
                row.add_info = ScanInt(&success);
                if (!success) return tag == "[gold card blank item]" ? 2571 : 2537;
                items.push_back(row);
                total_weight += row.weight;
            }
        } else if (tag == "[reward item rate per map max count]") {
            while (true) {
                const int map_index = ScanInt(&success);
                if (!success) break;
                if (map_index > 9) return 2550;
                reward_item_rate_per_map_max_count_[map_index] = ScanInt(&success);
                if (!success) return 2553;
            }
        } else {
            return 2625;
        }
    }
    item_rate_table_.generateTable(item_ref_range_, false);
    return 0;
}

// IDA_EA: 0x853A968
// IDA_NAME: _ZN8WongWork12CClearRewardC2Ei
CClearReward::CClearReward(int random_stream_count)
    : CItemGenerator(random_stream_count),
      configured_probability_(), active_probability_(),
      drop_kind_probability_(), drop_kind_reserved_(0), drop_item_type_probability_(),
      dungeon_difficulty_drop_bonus_(), dungeon_difficulty_gold_bonus_(),
      party_member_drop_bonus_(), rarity_decision_(), item_ref_range_(),
      item_ref_padding_(), item_rate_table_(), gold_card_commission_(),
      gold_card_create_rate_(0.0f), gold_card_blank_total_weight_(0),
      gold_card_blank_items_(), reward_item_rate_per_map_max_count_(),
      pcroom_card_blank_total_weight_(0), pcroom_card_blank_items_() {
    item_rate_table_.set_disable_drop_random_option_item();
}

// IDA_EA: 0x85BF000
// IDA_NAME: _ZN8WongWork12CClearReward21getGoldCardCreateRateEv
long double CClearReward::getGoldCardCreateRate() {
    return static_cast<long double>(gold_card_create_rate_);
}

// IDA_EA: 0x85BF018
// IDA_NAME: _ZNK8WongWork12CClearReward31getRewardItemRatePerMapMaxCountEi
// DEMANGLED: WongWork::CClearReward::getRewardItemRatePerMapMaxCount(int) const
int CClearReward::getRewardItemRatePerMapMaxCount(int map_index) const {
    return map_index > 9 ? 0 : reward_item_rate_per_map_max_count_[map_index];
}

// IDA_EA: 0x85372EE
// IDA_NAME: _ZN8WongWork12CMonsterDrop18_getGenItemProbIdxEi
// DEMANGLED: WongWork::CMonsterDrop::_getGenItemProbIdx(int)
int CMonsterDrop::_getGenItemProbIdx(int standard_level) {
    for (size_t index = 0; index < scaled_probabilities_.size(); ++index) {
        const stGenItemProb_t& probability = scaled_probabilities_[index];
        if (probability.first_level <= standard_level &&
            probability.last_level >= standard_level) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x8535DC8
// IDA_NAME: _ZN8WongWork12CMonsterDrop16_getBossGoldDropEhRtS1_
// DEMANGLED: WongWork::CMonsterDrop::_getBossGoldDrop(unsigned char,
//            unsigned short &,unsigned short &)
int CMonsterDrop::_getBossGoldDrop(uint8_t party_member_index,
                                   uint16_t& item_count,
                                   uint16_t& gold_multiplier) {
    if (party_member_index > 3u) {
        return 0;
    }

    const uint16_t normal_rate =
        boss_gold_drop_.condition_rate[party_member_index][0];
    const uint16_t boss_rate =
        boss_gold_drop_.condition_rate[party_member_index][1];
    const bool boss_selection = boss_rate > static_cast<int>(
        random_number_.generateNumber(0, normal_rate + boss_rate));

    const uint16_t minimum =
        boss_gold_drop_.gold_quantity[boss_selection ? 1 : 0][0];
    const uint16_t maximum =
        boss_gold_drop_.gold_quantity[boss_selection ? 1 : 0][1];
    item_count = static_cast<uint16_t>(
        minimum + random_number_.generateNumber(0, maximum - minimum));
    gold_multiplier = boss_gold_drop_.gold_volume[boss_selection ? 1 : 0];
    return 1;
}

// IDA_EA: 0x853736E
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDrop18_getGenItemProbIdxEi
// DEMANGLED: WongWork::CSpecialMonsterDrop::_getGenItemProbIdx(int)
int CSpecialMonsterDrop::_getGenItemProbIdx(int standard_level) {
    for (size_t index = 0; index < scaled_probabilities_.size(); ++index) {
        const stGenItemProb_t& probability = scaled_probabilities_[index];
        if (probability.first_level <= standard_level &&
            probability.last_level >= standard_level) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x8535ED2
// IDA_NAME: _ZN8WongWork12CMonsterDrop19_generateRandomItemEhchhhiiRNS_18stGenerateResult_tERb
// DEMANGLED: WongWork::CMonsterDrop::_generateRandomItem(unsigned char,char,
//     unsigned char,unsigned char,unsigned char,int,int,
//     WongWork::stGenerateResult_t &,bool &)
void CMonsterDrop::_generateRandomItem(uint8_t character_level,
                                       int8_t monster_role,
                                       uint8_t dungeon_standard_level,
                                       uint8_t dungeon_difficulty,
                                       uint8_t party_member_count,
                                       int bourgeois_rate,
                                       int standard_drop_rate,
                                       stGenerateResult_t& result,
                                       bool& generated_special_item) {
    const int probability_index = _getGenItemProbIdx(dungeon_standard_level);
    if (probability_index < 0) return;

    // The original copies the selected seven-dword row to its stack before
    // selecting gold and item kinds, so later market adjustment never alters
    // the configured probability vector.
    const CMonsterDrop::stGenItemProb_t drop_probability =
        scaled_probabilities_[probability_index];
    const uint8_t party_member_index =
        static_cast<uint8_t>(party_member_count - 1u);

    int gold_drop_value = TruncateDropProbability(
        static_cast<long double>(drop_probability.weight[0]) *
        dungeon_difficulty_bonus_[0][dungeon_difficulty] *
        monster_type_bonus_[0][monster_role] *
        party_member_bonus_[0][party_member_index]);

    int bounded_bourgeois_rate = bourgeois_rate;
    if (bounded_bourgeois_rate < 0) bounded_bourgeois_rate = 0;
    if (bounded_bourgeois_rate > 100) bounded_bourgeois_rate = 100;
    if (bounded_bourgeois_rate > 50) {
        const float reduction = static_cast<float>(
            static_cast<long double>(bounded_bourgeois_rate - 50) *
                0.0020000001L +
            0.0099999998L);
        gold_drop_value = TruncateDropProbability(
            static_cast<long double>(gold_drop_value) * (1.0L - reduction));
    }
    gold_drop_value = standard_drop_rate * gold_drop_value / 100;

    if (static_cast<int>(random_number_.generateNumber(0, 10000)) <
        gold_drop_value) {
        GoldGenerateRange* const gold =
            common_data_->getGoldGenTable(character_level);
        int gold_amount = gold->minimum +
            GetRandomRange(-gold->maximum, gold->maximum) * gold->minimum /
                100;
        Inven_Item item;
        item.item_category_ = 2u;
        item.set_add_info(gold_amount > 1 ? gold_amount : 1);

        uint16_t item_count = 0;
        uint16_t gold_multiplier = 0;
        if (monster_role == 3 &&
            _getBossGoldDrop(party_member_index, item_count, gold_multiplier)) {
            gold_amount = gold_amount * gold_multiplier / 100;
            item.set_add_info(gold_amount > 1 ? gold_amount : 1);
            for (int index = 0; index < item_count; ++index) {
                result.items.push_back(item);
            }
        } else {
            result.items.push_back(item);
        }
    }

    if (generated_special_item) return;

    for (int drop_type = 1; drop_type <= 4; ++drop_type) {
        float item_drop_rate = 1.0f;
        int adjusted_rarity_decision[6] = {0, 0, 0, 0, 0, 0};
        int* rarity_decision = rarity_decision_[drop_type];
        if (drop_type == 2) {
            CAutoMarketConditionsControl* const market =
                G_CDataManager()->GetAutoMarketContitionsControl();
            item_drop_rate = market->GetItemDropRate(probability_index);
            double control_rarity = 0.0;
            if (item_drop_rate != 0.0f) {
                control_rarity = 1.0 / item_drop_rate - 1.0;
            }
            if (market->ChangeItemRarity(rarity_decision_[drop_type],
                                         adjusted_rarity_decision,
                                         6,
                                         control_rarity)) {
                rarity_decision = adjusted_rarity_decision;
            }
        }

        float probability =
            static_cast<float>(drop_probability.weight[drop_type]);
        if (drop_type == 2) {
            probability = static_cast<float>(
                static_cast<long double>(normal_drop_scale_) * probability);
            probability = static_cast<float>(
                static_cast<long double>(normal_drop_scale_rate_) * probability);
        }
        const int drop_value = standard_drop_rate * TruncateDropProbability(
            static_cast<long double>(probability) * item_drop_rate *
            dungeon_difficulty_bonus_[drop_type][dungeon_difficulty] *
            monster_type_bonus_[drop_type][monster_role] *
            party_member_bonus_[drop_type][party_member_index]) / 100;
        const int random_value = static_cast<int>(
            random_number_.generateNumber(0, 10000));
        if (drop_value < random_value) continue;

        const int rarity = result.luck_point.GetItemRarity(
            rarity_decision,
            static_cast<int>(random_number_.generateNumber(1, 1000000)),
            drop_type);
        G_CDataManager()->getItemGenerator()->onTryGenerateRandomItem(
            static_cast<CItemGeneratorMgr::eGenerateType_t>(getGeneratorType()),
            drop_type,
            rarity);
        const int item_id = item_rate_table_.chooseItem(
            drop_type, rarity, character_level);
        if (item_id == -1) continue;

        if (drop_type == 2) {
            char message[256] = {0};
            std::sprintf(
                message,
                "monster drop rate : item_id(%d), mon_lv(%d), role(%d), "
                "dungeon_lv(%d), dungeon_diff(%d), member_cnt(%d), "
                "bourgeois_rate(%d), drop_rate(%d), rand_value(%d), "
                "drop_value(%d)",
                item_id,
                character_level,
                monster_role,
                dungeon_standard_level,
                dungeon_difficulty,
                party_member_index,
                bounded_bourgeois_rate,
                standard_drop_rate,
                random_value,
                drop_value);
            result.luck_point.sendItemDropRate(message);
        }

        CItem* const prototype =
            G_CDataManager()->get_arad_jumping_item_list()->find_item(item_id);
        if (!prototype) continue;

        Inven_Item item;
        item.item_id_ = item_id;
        G_CDataManager()->get_arad_jumping_item_list()->create_item(
            static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
        if (drop_type == 2) {
            if (prototype->vtable_->is_creature_item(prototype) != 1 &&
                prototype->vtable_->is_avatar_item(prototype) != 1) {
                item.set_add_info(random_upgrade_.generateEquipGrade(
                    static_cast<int>(character_level) - prototype->get_grade()));
            }
            if (prototype->vtable_->check_item_upgrade_possible(prototype) &&
                !item.amplify_option_.hasAbility()) {
                const int upgrade = random_upgrade_.generateUpgrade(
                    static_cast<ENUM_RARITY>(prototype->get_rarity()),
                    static_cast<int>(character_level) - prototype->get_grade());
                if (upgrade > 0) {
                    item.SetUpgrade(static_cast<uint8_t>(upgrade));
                }
            }
        }

        const int item_index = static_cast<int>(result.items.size());
        result.luck_point.UseLuckPoint(
            static_cast<int>(item.item_category_),
            prototype->get_rarity(),
            prototype->get_grade(),
            item_index);
        result.items.push_back(item);
    }
}

// IDA_EA: 0x8536882
// IDA_NAME: _ZN8WongWork19CSpecialMonsterDrop19_generateRandomItemEhchhhihiRNS_18stGenerateResult_tERb
// DEMANGLED: WongWork::CSpecialMonsterDrop::_generateRandomItem(
//     unsigned char,char,unsigned char,unsigned char,unsigned char,int,
//     unsigned char,int,WongWork::stGenerateResult_t &,bool &)
void CSpecialMonsterDrop::_generateRandomItem(
    uint8_t character_level,
    int8_t monster_role,
    uint8_t dungeon_standard_level,
    uint8_t dungeon_difficulty,
    uint8_t party_member_count,
    int bourgeois_rate,
    uint8_t item_drop_dungeon_type,
    int standard_drop_rate,
    stGenerateResult_t& result,
    bool& generated_special_item) {
    const int probability_index = _getGenItemProbIdx(dungeon_standard_level);
    if (probability_index < 0) return;

    const CSpecialMonsterDrop::stGenItemProb_t drop_probability =
        scaled_probabilities_[probability_index];
    const uint8_t party_member_index =
        static_cast<uint8_t>(party_member_count - 1u);

    for (int drop_type = 1; drop_type <= 4; ++drop_type) {
        float item_drop_rate = 1.0f;
        int adjusted_rarity_decision[6] = {0, 0, 0, 0, 0, 0};
        int* rarity_decision = rarity_decision_[drop_type];
        if (drop_type == 2) {
            CAutoMarketConditionsControl* const market =
                G_CDataManager()->GetAutoMarketContitionsControl();
            item_drop_rate = market->GetItemDropRate(probability_index);
            double control_rarity = 0.0;
            if (item_drop_rate != 0.0f) {
                control_rarity = 1.0 / item_drop_rate - 1.0;
            }
            if (market->ChangeItemRarity(rarity_decision_[drop_type],
                                         adjusted_rarity_decision,
                                         6,
                                         control_rarity)) {
                rarity_decision = adjusted_rarity_decision;
            }
        }

        const int drop_value = standard_drop_rate * TruncateDropProbability(
            static_cast<long double>(drop_probability.weight[drop_type]) *
            item_drop_rate *
            dungeon_difficulty_bonus_[drop_type][dungeon_difficulty] *
            monster_type_bonus_[drop_type][monster_role] *
            party_member_bonus_[drop_type][party_member_index]) / 100;
        const int random_value = static_cast<int>(
            random_number_.generateNumber(0, 10000));
        if (drop_value < random_value) continue;

        const int rarity = result.luck_point.GetItemRarity(
            rarity_decision,
            static_cast<int>(random_number_.generateNumber(1, 1000000)),
            drop_type);
        G_CDataManager()->getItemGenerator()->onTryGenerateRandomItem(
            CItemGeneratorMgr::kMonsterDrop, drop_type, rarity);
        const int item_id = item_rate_table_.chooseItem(
            drop_type,
            rarity,
            character_level,
            dungeon_difficulty,
            item_drop_dungeon_type);
        if (item_id == -1) continue;

        CItem* const prototype =
            G_CDataManager()->get_arad_jumping_item_list()->find_item(item_id);
        if (!prototype) continue;

        if (drop_type == 2) {
            char message[256] = {0};
            std::sprintf(
                message,
                "special monster drop rate : item_id(%d), mon_lv(%d), "
                "role(%d), dungeon_lv(%d), dungeon_diff(%d), member_cnt(%d), "
                "bourgeois_rate(%d), drop_rate(%d), rand_value(%d), "
                "drop_value(%d)",
                item_id,
                character_level,
                monster_role,
                dungeon_standard_level,
                dungeon_difficulty,
                party_member_index,
                bourgeois_rate,
                standard_drop_rate,
                random_value,
                drop_value);
            result.luck_point.sendItemDropRate(message);
        }

        Inven_Item item;
        item.item_id_ = item_id;
        G_CDataManager()->get_arad_jumping_item_list()->create_item(
            static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
        const int item_index = static_cast<int>(result.items.size());
        result.luck_point.UseLuckPoint(
            static_cast<int>(item.item_category_),
            prototype->get_rarity(),
            prototype->get_grade(),
            item_index);
        result.items.push_back(item);
        generated_special_item = true;
    }
}

// IDA_EA: 0x8536DA8
// IDA_NAME: _ZN8WongWork12CMonsterDrop18generateRandomItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CMonsterDrop::generateRandomItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CMonsterDrop::generateRandomItem(const stGenerateRefData_t& reference,
                                      stGenerateResult_t& result) {
    const stGenerateRefMonsterRandomDrop_t& random =
        reference.generator_data.monster_random_drop;
    if (random.monster_role > 3) {
        return;
    }

    bool generated_special_item = false;
    if (!random.is_hell_party) {
        for (int index = 0; random.monster_drop_count > index; ++index) {
            special_drop_._generateRandomItem(
                random.character_level,
                random.monster_role,
                random.dungeon_standard_level,
                random.dungeon_difficulty,
                random.party_member_count,
                random.bourgeois_rate,
                random.item_drop_dungeon_type,
                reference.standard_drop_rate,
                result,
                generated_special_item);
        }
    }

    for (int index = 0; random.monster_drop_count > index; ++index) {
        if (reference.flags[0]) {
            normal_drop_scale_ = first_mob_hunting_weight_.first_boss_weight;
        } else if (reference.flags[1]) {
            normal_drop_scale_ = first_mob_hunting_weight_.first_named_mob_weight;
        }
        normal_drop_scale_rate_ = reference.monster_random_drop_scale;
        _generateRandomItem(random.character_level,
                            random.monster_role,
                            random.dungeon_standard_level,
                            random.dungeon_difficulty,
                            random.party_member_count,
                            random.bourgeois_rate,
                            reference.standard_drop_rate,
                            result,
                            generated_special_item);
        normal_drop_scale_ = 1;
        normal_drop_scale_rate_ = 1.0f;
    }
}

// IDA_EA: 0x8536FCE
// IDA_NAME: _ZN8WongWork12CMonsterDrop29_generateMonseterSpecificItemEPK8CMonstercRNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CMonsterDrop::_generateMonseterSpecificItem(
//     CMonster const*,char,WongWork::stGenerateResult_t &)
void CMonsterDrop::_generateMonseterSpecificItem(
    const CMonster* monster,
    char drop_kind,
    stGenerateResult_t& result) {
    const int random_value = static_cast<int>(
        random_number_.generateNumber(2, 10000));
    const int item_id = monster->drop_item_index(
        static_cast<unsigned int>(drop_kind), random_value);
    if (item_id == -2) {
        return;
    }

    Inven_Item item;
    item.item_id_ = item_id;
    G_CDataManager()->get_arad_jumping_item_list()->create_item(
        static_cast<ENUM_ITEM_CREATE_TYPE>(1), item, 1);
    result.items.push_back(item);
}

// IDA_EA: 0x8537070
// IDA_NAME: _ZN8WongWork12CMonsterDrop20generateSpecificItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CMonsterDrop::generateSpecificItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CMonsterDrop::generateSpecificItem(const stGenerateRefData_t& reference,
                                        stGenerateResult_t& result) {
    const stGenerateRefMonsterSpecificDrop_t& specific =
        reference.generator_data.monster_specific_drop;
    if (specific.hell_party_specific_path) {
        hell_drop_.hellPartySpecificItem(reference, result);
        return;
    }

    if (specific.monster_kind <= 4) {
        CMonster* const monster = G_CDataManager()->find_monster(
            static_cast<int>(specific.monster_id));
        if (!monster) {
            return;
        }

        if (!monster->getMonsterScript()->independent_drops.empty()) {
            monster->generateIndependentItem(reference, result.items);
        }
        monster->generateDropAvatarItem(specific.dungeon_difficulty,
                                        specific.avatar_drop_context,
                                        result.items);
        if (!monster->getMonsterScript()->catch_items.empty() &&
            specific.catch_monster_drop_enabled) {
            monster->generateCatchMonsterItem(result.items);
        }

        if (specific.suppress_normal_specific_drop_a != 1 &&
            specific.suppress_normal_specific_drop_b != 1) {
            for (int index = 0; specific.monster_drop_count > index;
                 ++index) {
                _generateMonseterSpecificItem(monster, 0, result);
            }
        }
        if (specific.monster_kind != 0) {
            _generateMonseterSpecificItem(
                monster, static_cast<char>(specific.monster_kind), result);
        }
        return;
    }

    for (int index = 0; specific.monster_drop_count > index; ++index) {
        hell_drop_.generateSpecificItem(reference, result);
    }

    CAICharacter* const ai_character = G_CDataManager()
        ->get_ai_character_list()
        ->get(specific.monster_id);
    if (!ai_character) {
        return;
    }
    ai_character->generateIndependentItem(reference, result.items);
    ai_character->generateDropAvatarItem(specific.dungeon_difficulty,
                                         specific.avatar_drop_context,
                                         result.items);
}

// IDA_EA: 0x853818A
// IDA_NAME: _ZN8WongWork11CObjectDrop18generateRandomItemERKNS_19stGenerateRefData_tERNS_18stGenerateResult_tE
// DEMANGLED: WongWork::CObjectDrop::generateRandomItem(
//     WongWork::stGenerateRefData_t const&,WongWork::stGenerateResult_t &)
void CObjectDrop::generateRandomItem(const stGenerateRefData_t& reference,
                                     stGenerateResult_t& result) {
    const stGenerateRefObjectRandomDrop_t& object =
        reference.generator_data.object_random_drop;
    bool generated_special_item = false;
    for (int index = 0; object.item_count > index; ++index) {
        _generateRandomItem(object.character_level,
                            0,
                            object.character_level,
                            object.dungeon_difficulty,
                            object.party_member_count,
                            0,
                            reference.standard_drop_rate,
                            result,
                            generated_special_item);
    }
}

}  // namespace WongWork
