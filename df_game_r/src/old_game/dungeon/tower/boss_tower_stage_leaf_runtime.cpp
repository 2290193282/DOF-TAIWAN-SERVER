#include "old_game/pvp/battlefield/battle_field.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/server_utility.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x88F8AE6
// IDA_NAME: _ZN10RandomListD2Ev
// DEMANGLED: RandomList::~RandomList(void)
RandomList::~RandomList() {
}

namespace WongWork {

// IDA_EA: 0x8149466
// IDA_NAME: _ZN8WongWork9CBossPlay18getLiveMemberCountEv
// DEMANGLED: WongWork::CBossPlay::getLiveMemberCount(void)
int CBossPlay::getLiveMemberCount() {
    int count = 0;
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        if (member_alive_[member_slot]) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x814D1B6
// IDA_NAME: _ZN8WongWork10CBossStage13moveNextStageEv
// DEMANGLED: WongWork::CBossStage::moveNextStage(void)
int CBossStage::moveNextStage() {
    if (!map_info_.monsters_.empty()) {
        return 0;
    }
    if (current_stage_ >= maximum_stage_) {
        return 0;
    }
    ++current_stage_;
    return 1;
}

// IDA_EA: 0x814D214
// IDA_NAME: _ZN8WongWork10CBossStage16killMonsterEraseEtR11map_monsterRi
// DEMANGLED: WongWork::CBossStage::killMonsterErase(unsigned short,map_monster &,int &)
int CBossStage::killMonsterErase(uint16_t monster_id,
                                 map_monster& monster,
                                 int& monster_kind) {
    std::map<int, map_monster>::iterator found =
        map_info_.monsters_.find(static_cast<int>(monster_id));
    if (found == map_info_.monsters_.end()) {
        return 0;
    }

    monster = found->second;
    monster_kind = static_cast<int>(found->second.monster_level_);
    map_info_.monsters_.erase(static_cast<int>(monster_id));
    return 1;
}

// IDA_EA: 0x814F4FC
// IDA_NAME: _ZN8WongWork10CBossStage14randomMapIndexERi
// DEMANGLED: WongWork::CBossStage::randomMapIndex(int &)
int CBossStage::randomMapIndex(int& map_index) {
    const RandomTowerMapIndexTable& indexes =
        dungeon_->getRandomTowerMapIndexes();
    RandomTowerMapIndexTable::const_iterator found =
        indexes.find(static_cast<int>(current_stage_));
    if (found == indexes.end()) {
        return 0;
    }

    const std::vector<RandomList> candidates = found->second;
    const int random_value = static_cast<int>(get_rand_int(10000));
    int accumulated_weight = 0;
    for (size_t index = 0; index < candidates.size(); ++index) {
        accumulated_weight += candidates[index].weight;
        if (random_value < accumulated_weight) {
            map_index = candidates[index].map_index;
            break;
        }
    }
    return 1;
}

// IDA_EA: 0x814CE12
// IDA_NAME: _ZN8WongWork10CBossStage17SendMapInfoPacketEv
// DEMANGLED: WongWork::CBossStage::SendMapInfoPacket(void)
int CBossStage::SendMapInfoPacket() {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_START_BOSS_TOWER_MAP);
    packet.put_short(static_cast<int>(current_stage_));
    packet.put_int(map_info_.random_map_seed_);
    packet.put_short(map_info_.map_index_);

    packet.put_byte(static_cast<int>(map_info_.monsters_.size()));
    for (std::map<int, map_monster>::const_iterator it =
             map_info_.monsters_.begin();
         it != map_info_.monsters_.end(); ++it) {
        // IDA invokes map_monster copy C2/D2 for each map entry before it
        // serializes the exact fields below.
        map_monster monster(it->second);
        packet.put_int(monster.spawn_sequence_);
        packet.put_short(monster.map_monster_unique_id_);
        packet.put_short(monster.monster_id_);
        packet.put_byte(monster.monster_level_);
        packet.put_byte(monster.monster_kind_);
        packet.put_byte(monster.packet_value_13_);
        packet.put_byte(monster.stage_monster_type_);
    }

    packet.put_byte(static_cast<int>(map_info_.items_.size()));
    for (std::map<int, map_item>::const_iterator it =
             map_info_.items_.begin();
         it != map_info_.items_.end(); ++it) {
        const map_item& map_item_value = it->second;
        const Inven_Item& item = map_item_value.item;

        packet.put_byte(map_item_value.packet_value_01_);
        packet.put_short(map_item_value.unique_id_);
        packet.put_int(item.item_id_);
        packet.put_int(item.get_add_info());
        packet.put_short(item.avatar_select_);
        packet.put_byte(item.amplify_option_.getAbilityType());
        packet.put_short(item.amplify_option_.getAbilityValue());
    }

    packet.finalize(true);
    party_->send_to_party(packet);
    return 1;
}

}  // namespace WongWork
