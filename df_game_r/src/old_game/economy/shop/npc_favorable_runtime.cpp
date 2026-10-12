#include "old_data/PVF/npc/npc_script.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <algorithm>

namespace {

typedef stFavorableRelationShip_t::stGiftItemInfo_t GiftItemInfo;
typedef std::map<uint32_t, GiftItemInfo> GiftItemMap;

const GiftItemInfo* FindGiftItemInfo(const GiftItemMap& entries,
                                    uint32_t item_index) {
    GiftItemMap::const_iterator found = entries.find(item_index);
    return found == entries.end() ? 0 : &found->second;
}

}  // namespace

// IDA_EA: 0x85804B2
// IDA_NAME: _ZN15CNPCDynamicInfo10changeMoodEb
// DEMANGLED: CNPCDynamicInfo::changeMood(bool)
int CNPCDynamicInfo::changeMood(bool force_change) {
    if (force_change ||
        next_change_second <= GlobalData::s_systemTime_.getCurSec()) {
        ENUM_NPC_MOOD next_mood = static_cast<ENUM_NPC_MOOD>(mood);
        unsigned int next_position = static_cast<unsigned int>(mood_position);
        next_change_second = script->getNextMood(next_mood, next_position);
        mood = static_cast<int>(next_mood);
        mood_position = static_cast<int32_t>(next_position);
        next_change_second += GlobalData::s_systemTime_.getCurSec();
    }
    return getMood();
}

// IDA_EA: 0x858053E
// IDA_NAME: _ZN15CNPCDynamicInfo15forceChangeMoodE13ENUM_NPC_MOOD
// DEMANGLED: CNPCDynamicInfo::forceChangeMood(ENUM_NPC_MOOD)
int CNPCDynamicInfo::forceChangeMood(ENUM_NPC_MOOD target_mood) {
    if (getMood() == static_cast<int>(target_mood)) {
        return 0;
    }
    while (changeMood(true) != static_cast<int>(target_mood)) {
    }
    return 1;
}

// IDA_EA: 0x8580584
// IDA_NAME: _ZNK15CNPCDynamicInfo9isKeyItemEmj
// DEMANGLED: CNPCDynamicInfo::isKeyItem(unsigned long,unsigned int)const
int CNPCDynamicInfo::isKeyItem(unsigned long item_index,
                               unsigned int item_count) const {
    return script->isKeyItem(item_index, item_count);
}

// IDA_EA: 0x85805A8
// IDA_NAME: _ZNK15CNPCDynamicInfo12giveGiftItemEjmjPK5CItemR32ENUM_NPC_FAVOR_GIFT_RESULT_CONST
// DEMANGLED: CNPCDynamicInfo::giveGiftItem(unsigned int,unsigned long,unsigned int,CItem const*,ENUM_NPC_FAVOR_GIFT_RESULT_CONST &)const
int CNPCDynamicInfo::giveGiftItem(
    unsigned int mood,
    unsigned long item_index,
    unsigned int item_count,
    const CItem* item,
    ENUM_NPC_FAVOR_GIFT_RESULT_CONST& result_const) const {
    // IDA 0x85805D7 passes mood, item and count to CNPCScript. The middle
    // unsigned-long argument is present in the ABI but deliberately unused.
    (void)item_index;
    int16_t favor_value = static_cast<int16_t>(
        script->giveGiftItem(mood, item, item_count, result_const));
    if (favor_value > 0) {
        favor_value = static_cast<int16_t>(
            script->getFavorRatePerMood(
                static_cast<ENUM_NPC_MOOD>(getMood())) *
            static_cast<float>(favor_value));
    }
    return static_cast<uint16_t>(favor_value);
}

// IDA_EA: 0x8580624
// IDA_NAME: _ZNK15CNPCDynamicInfo21makeNotiPacketCurMoodER11PacketGuard
// DEMANGLED: CNPCDynamicInfo::makeNotiPacketCurMood(PacketGuard &)const
int CNPCDynamicInfo::makeNotiPacketCurMood(PacketGuard& packet) const {
    packet.put_short(static_cast<int>(script->getIndex()));
    return packet.put_byte(getMood());
}

// IDA_EA: 0x8581910
// IDA_NAME: _ZNK15CNPCDynamicInfo7getMoodEv
// DEMANGLED: CNPCDynamicInfo::getMood(void)const
int CNPCDynamicInfo::getMood() const {
    return mood;
}

// IDA_EA: 0x8580722
// IDA_NAME: _ZN22CNPCDynamicInfoManager7onTimerEv
// DEMANGLED: CNPCDynamicInfoManager::onTimer(void)
void CNPCDynamicInfoManager::onTimer() {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_NPC_MOOD);
    int packet_index = packet.get_index();
    int16_t changed_count = 0;
    packet.put_short(0);

    for (InfoMap::iterator it = infos_.begin(); it != infos_.end(); ++it) {
        CNPCDynamicInfo* const info = it->second;
        const int previous_mood = info->getMood();
        if (previous_mood != info->changeMood(false)) {
            info->makeNotiPacketCurMood(packet);
            ++changed_count;
        }
    }

    packet.put_short(packet_index, changed_count);
    packet.finalize(true);
    if (changed_count) {
        G_GameWorld()->send_all(packet);
    }
}

// IDA_EA: 0x85808A6
// IDA_NAME: _ZNK22CNPCDynamicInfoManager21makeNotiPacketNPCMoodER11PacketGuard
// DEMANGLED: CNPCDynamicInfoManager::makeNotiPacketNPCMood(PacketGuard &)const
int CNPCDynamicInfoManager::makeNotiPacketNPCMood(PacketGuard& packet) const {
    packet.put_header(0, ENUM_NOTIPACKET_NPC_MOOD);
    int packet_index = packet.get_index();
    int16_t mood_count = 0;
    packet.put_short(0);

    for (InfoMap::const_iterator it = infos_.begin(); it != infos_.end();
         ++it) {
        it->second->makeNotiPacketCurMood(packet);
        ++mood_count;
    }

    packet.put_short(packet_index, mood_count);
    return packet.finalize(true);
}

// IDA_EA: 0x85809E4
// IDA_NAME: _ZNK10CNPCScript12giveGiftItemEjPK5CItemjR32ENUM_NPC_FAVOR_GIFT_RESULT_CONST
// DEMANGLED: CNPCScript::giveGiftItem(unsigned int,CItem const*,unsigned int,ENUM_NPC_FAVOR_GIFT_RESULT_CONST &)const
int CNPCScript::giveGiftItem(
    unsigned int mood,
    const CItem* item,
    unsigned int item_count,
    ENUM_NPC_FAVOR_GIFT_RESULT_CONST& result_const) const {
    uint16_t favor_value = 0;
    const uint32_t item_id = static_cast<uint32_t>(item->get_index());
    const uint32_t item_group =
        static_cast<uint32_t>(item->getItemGroupName());

    const GiftItemInfo* info = FindGiftItemInfo(
        script.favorable_relation.prefer_items[mood], item_id);
    if (info) {
        if (info->required_count > item_count) {
            result_const = static_cast<ENUM_NPC_FAVOR_GIFT_RESULT_CONST>(1);
        } else {
            favor_value = static_cast<uint16_t>(info->favor_value);
        }
    }

    if (item_group) {
        info = FindGiftItemInfo(
            script.favorable_relation.prefer_item_groups[mood], item_group);
        if (info) {
            if (info->required_count > item_count) {
                result_const =
                    static_cast<ENUM_NPC_FAVOR_GIFT_RESULT_CONST>(1);
            } else if (info->favor_value >
                       static_cast<int16_t>(favor_value)) {
                favor_value = static_cast<uint16_t>(info->favor_value);
            }
        }
    }

    if (!favor_value) {
        info = FindGiftItemInfo(
            script.favorable_relation.unprefer_items[mood], item_id);
        if (info) {
            favor_value = static_cast<uint16_t>(info->favor_value);
        }

        if (item_group) {
            info = FindGiftItemInfo(
                script.favorable_relation.unprefer_item_groups[mood],
                item_group);
            if (info) {
                if (favor_value) {
                    const int16_t current = static_cast<int16_t>(favor_value);
                    favor_value = static_cast<uint16_t>(std::max(
                        current,
                        info->favor_value));
                } else {
                    favor_value = static_cast<uint16_t>(info->favor_value);
                }
            }
        }
    }

    return favor_value;
}

// IDA_EA: 0x8580D30
// IDA_NAME: _ZNK10CNPCScript17getGiftRewardItemE13ENUM_NPC_MOOD27ENUM_NPC_FAVOR_CHANGE_CONSTj
// DEMANGLED: CNPCScript::getGiftRewardItem(ENUM_NPC_MOOD,ENUM_NPC_FAVOR_CHANGE_CONST,unsigned int)const
Inven_Item CNPCScript::getGiftRewardItem(
    ENUM_NPC_MOOD mood,
    ENUM_NPC_FAVOR_CHANGE_CONST favor_change,
    unsigned int favor_level) const {
    Inven_Item item;
    const stFavorableRelationShip_t::stRewardItemInfo_t& reward =
        script.favorable_relation.reward_items[favor_level]
                                               [static_cast<int>(mood)];
    const int favor_index = static_cast<int>(favor_change);
    const unsigned int random_value =
        get_rand_int(reward.total_rates[favor_index]);
    const std::vector<stFavorableRelationShip_t::stRewardItemElement_t>&
        candidates = reward.reward_items[favor_index];

    for (unsigned int index = 0;
         index < static_cast<unsigned int>(candidates.size()); ++index) {
        const stFavorableRelationShip_t::stRewardItemElement_t& candidate =
            candidates.at(index);
        if (static_cast<uint32_t>(candidate.accumulated_rate) >
            random_value) {
            if (candidate.item_index || candidate.item_count) {
                item.item_id_ = candidate.item_index;
                G_CDataManager()->get_item_list()->create_item(
                    ENUM_ITEM_CREATE_TYPE_DEFAULT,
                    item,
                    candidate.item_count);
            }
            return item;
        }
    }
    return item;
}

// IDA_EA: 0x8580E6A
// IDA_NAME: _ZNK10CNPCScript9isKeyItemEmj
// DEMANGLED: CNPCScript::isKeyItem(unsigned long,unsigned int)const
int CNPCScript::isKeyItem(unsigned long item_index,
                          unsigned int item_count) const {
    const std::pair<uint32_t, uint32_t> expected(
        static_cast<uint32_t>(item_index), item_count);
    const std::vector<std::pair<uint32_t, uint32_t> >& items =
        script.favorable_relation.key_items;
    return std::find(items.begin(), items.end(), expected) != items.end();
}

// IDA_EA: 0x8580F28
// IDA_NAME: _ZNK10CNPCScript11getNextMoodER13ENUM_NPC_MOODRj
// DEMANGLED: CNPCScript::getNextMood(ENUM_NPC_MOOD &,unsigned int &)const
int CNPCScript::getNextMood(ENUM_NPC_MOOD& mood,
                            unsigned int& mood_position) const {
    const std::vector<std::pair<int, uint32_t> >& changes =
        script.favorable_relation.mood_changes;
    if (changes.empty()) {
        mood = static_cast<ENUM_NPC_MOOD>(1);
        mood_position = 0;
        return 0;
    }

    ++mood_position;
    if (mood_position >= changes.size()) {
        mood_position = 0;
    }
    const std::pair<int, uint32_t>& change = changes.at(mood_position);
    mood = static_cast<ENUM_NPC_MOOD>(change.first);
    return static_cast<int>(change.second);
}

// IDA_EA: 0x8580FD0
// IDA_NAME: _ZNK10CNPCScript13isRewardLevelEt
// DEMANGLED: CNPCScript::isRewardLevel(unsigned short)const
int CNPCScript::isRewardLevel(unsigned short favor_level) const {
    const std::vector<stFavorableRelationShip_t::stLevelRewardInfo>& rewards =
        script.favorable_relation.level_rewards;
    for (std::vector<stFavorableRelationShip_t::stLevelRewardInfo>::
             const_iterator it = rewards.begin();
         it != rewards.end(); ++it) {
        if (it->favor_level == favor_level) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8581076
// IDA_NAME: _ZNK10CNPCScript18getLevelRewardInfoEt
// DEMANGLED: CNPCScript::getLevelRewardInfo(unsigned short)const
stFavorableRelationShip_t::stLevelRewardInfo
CNPCScript::getLevelRewardInfo(unsigned short favor_level) const {
    stFavorableRelationShip_t::stLevelRewardInfo result;
    // The original C2 zeroes these fields but leaves the reserved halfword
    // untouched before either returning it or assigning the matching record.
    result.favor_level = 0;
    result.item_index = 0;
    result.item_count = 0;

    const std::vector<stFavorableRelationShip_t::stLevelRewardInfo>& rewards =
        script.favorable_relation.level_rewards;
    for (std::vector<stFavorableRelationShip_t::stLevelRewardInfo>::
             const_iterator it = rewards.begin();
         it != rewards.end(); ++it) {
        if (it->favor_level == favor_level) {
            return *it;
        }
    }
    return result;
}

// IDA_EA: 0x85811A4
// IDA_NAME: _ZNK10CNPCScript19getPeriodRewardTypeEv
// DEMANGLED: CNPCScript::getPeriodRewardType(void)const
int CNPCScript::getPeriodRewardType() const {
    const std::string& item_or_quest =
        script.favorable_relation.period_reward.item_or_quest;
    if (item_or_quest == "item") {
        return 1;
    }
    if (item_or_quest == "quest") {
        return 2;
    }
    return 0;
}

// IDA_EA: 0x85811F8
// IDA_NAME: _ZNK10CNPCScript17getBuffRewardInfoEi
// DEMANGLED: CNPCScript::getBuffRewardInfo(int)const
stFavorableRelationShip_t::stBuffRewardInfo
CNPCScript::getBuffRewardInfo(int favor_value) const {
    stFavorableRelationShip_t::stBuffRewardInfo result;
    const unsigned int random_value = get_rand_int(1000);
    int accumulated_probability = 0;
    result.buff_type = 0;

    const std::vector<stFavorableRelationShip_t::stBuffRewardInfo>& rewards =
        script.favorable_relation.buff_rewards;
    for (std::vector<stFavorableRelationShip_t::stBuffRewardInfo>::
             const_iterator it = rewards.begin();
         it != rewards.end(); ++it) {
        if (it->favor_level < favor_value) {
            accumulated_probability += it->probability;
            if (accumulated_probability > static_cast<int>(random_value)) {
                return *it;
            }
        }
    }
    return result;
}

// IDA_EA: 0x85819C6
// IDA_NAME: _ZNK10CNPCScript19getFavorRatePerMoodE13ENUM_NPC_MOOD
// DEMANGLED: CNPCScript::getFavorRatePerMood(ENUM_NPC_MOOD)const
float CNPCScript::getFavorRatePerMood(ENUM_NPC_MOOD mood) const {
    return script.favorable_relation
        .favor_rate_per_mood[static_cast<int>(mood)];
}

// IDA_EA: 0x86D2736
// IDA_NAME: _ZNK10CNPCScript11isExistRoleE13ENUM_NPC_ROLE
// DEMANGLED: CNPCScript::isExistRole(ENUM_NPC_ROLE)const
int CNPCScript::isExistRole(ENUM_NPC_ROLE role) const {
    return script.roles.find(static_cast<int>(role)) != script.roles.end();
}
