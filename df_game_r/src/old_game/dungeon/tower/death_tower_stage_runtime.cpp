#include "old_game/pvp/battlefield/battle_field.h"

#include <cstring>
#include <map>

#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/tower/death_tower_packet_handler_runtime.h"
#include "old_game/dungeon/tower/death_tower_value_statistic_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/log/format/trace.h"
#include "common/manager/system_time.h"

// IDA preserves this as a global fixed-underlying-type enum.  Its enumerator
// table is outside this recovered slice, but throw sites and packet builders
// require the actual enum identity rather than an integer substitute.
enum ENUM_ERROR : int {};

extern int CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT");
extern int CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT");

namespace WongWork {

// IDA_EA: 0x846966C
// IDA_NAME: _ZN8WongWork11CDeathTower23deathTowerCardStatisticEiP5CUserPK10Inven_Item
// DEMANGLED: WongWork::CDeathTower::deathTowerCardStatistic(
//     int,CUser *,Inven_Item const*)
bool CDeathTower::deathTowerCardStatistic(
    int reward_item_count,
    CUser* user,
    const Inven_Item* reward_items) {
    const bool non_tutorial_dungeon =
        dungeon_manager_.getDungeon()->get_index() != 11000;
    const int charac_level = user->get_charac_level();

    for (int item_index = 0;
         item_index < reward_item_count;
         ++item_index) {
        const Inven_Item& reward_item = reward_items[item_index];

        if (!reward_item.item_id_) {
            const unsigned int amount =
                static_cast<unsigned int>(reward_item.get_add_info());
            GetInstanceValueStatistic()->AddValueStatistic(
                static_cast<VALUE_STATISTIC_FIELD>(11), user, amount);
            GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
                static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(8),
                static_cast<char>(non_tutorial_dungeon),
                static_cast<char>(charac_level),
                amount);
            continue;
        }

        // Reward generation keeps an unfilled card as the official -1
        // sentinel.  The grant path already ignores that sentinel; the
        // statistic path must do the same.  Otherwise an empty random-card
        // result reaches GetSellItemPrice with a null prototype and crashes
        // the channel while the Illusion Tower clear panel is being built.
        if (reward_item.item_id_ == -1) {
            continue;
        }

        CItem* const item = G_CDataManager()->find_item(reward_item.item_id_);
        if (!item) {
            LogManager::logFormat(
                1,
                "DeathTower.cpp",
                "bool WongWork::CDeathTower::deathTowerCardStatistic(int, CUser*, Inven_Item const*)",
                0x3f2,
                "[DEATH_TOWER][REWARD_ITEM_MISSING] dungeon=%d charac_no=%u item_id=%d card=%d count=%d\n",
                dungeon_manager_.getDungeon()->get_index(),
                static_cast<unsigned int>(user->getCurCharacNo()),
                reward_item.item_id_,
                item_index,
                reward_item_count);
            continue;
        }

        int sell_price = 0;

        // The old function's const input is passed to the non-const Store
        // ABI.  It does not write this reward record on the original path;
        // retain the call signature without manufacturing a copied item.
        G_Store()->GetSellItemPrice(
            const_cast<Inven_Item&>(reward_item),
            item,
            static_cast<short>(reward_item_count),
            false,
            sell_price);

        GetInstanceValueStatistic()->AddValueStatistic(
            static_cast<VALUE_STATISTIC_FIELD>(12),
            user,
            static_cast<unsigned int>(sell_price));

        if (item->GetAttachType() == 3) {
            GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
                static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(2),
                static_cast<char>(non_tutorial_dungeon),
                static_cast<char>(charac_level),
                1);
        } else {
            GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
                static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(10),
                static_cast<char>(non_tutorial_dungeon),
                static_cast<char>(charac_level),
                static_cast<unsigned int>(item->get_rarity()));
        }

        GetInstanceDeathTowerValueStatistic()->SendDeathTowerValueStatistic(
            static_cast<DEATHTOWER_VALUE_STATISTIC_FIELD>(7),
            static_cast<char>(non_tutorial_dungeon),
            static_cast<char>(charac_level),
            static_cast<unsigned int>(sell_price));
    }

    // This is the final loop predicate in the original EAX/AL return path.
    return false;
}

// IDA_EA: 0x8469A8C
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandlerC2Ev
CDeathTower::CPacketHandler::CPacketHandler()
    : PacketGuard() {
}

// IDA_EA: 0x8469AA0
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandlerD2Ev
CDeathTower::CPacketHandler::~CPacketHandler() {
}

// IDA_EA: 0x845FE4C
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler15makeCommonErrorE14ENUM_CMDPACKET10ENUM_ERROR
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeCommonError(
//     ENUM_CMDPACKET,ENUM_ERROR)
int CDeathTower::CPacketHandler::makeCommonError(ENUM_CMDPACKET command,
                                                 ENUM_ERROR error) {
    put_header(1, static_cast<int>(command));
    put_byte(0);
    put_byte(static_cast<int>(error));
    return finalize(true);
}

// IDA_EA: 0x8460176
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler12makeDieStateEt
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDieState(unsigned short)
int CDeathTower::CPacketHandler::makeDieState(unsigned short unique_id) {
    put_header(0, 32);
    put_short(unique_id);
    put_byte(0);
    return finalize(true);
}

// IDA_EA: 0x8469AB4
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr11getEndStageEv
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getEndStage(void)
int CDeathTower::CDungeonMgr::getEndStage() {
    return end_stage_;
}

// IDA_EA: 0x84605EE
// IDA_NAME: _ZN8WongWork11CDeathTower11CDungeonMgr11getStageMapEj
// DEMANGLED: WongWork::CDeathTower::CDungeonMgr::getStageMap(unsigned int)
CMap* CDeathTower::CDungeonMgr::getStageMap(unsigned int current_stage) {
    const int map_index = dungeon_->getDeathTowerMapIndex(current_stage);
    return G_CDataManager()->find_map(map_index);
}

// IDA_EA: 0x84662CC
// IDA_NAME: _ZN8WongWork11CDeathTower18handleUseStackableEP5CUser14ENUM_ITEMSPACEt
// DEMANGLED: WongWork::CDeathTower::handleUseStackable(CUser *,
//             ENUM_ITEMSPACE,unsigned short)
int CDeathTower::handleUseStackable(CUser* user,
                                    ENUM_ITEMSPACE item_space,
                                    unsigned short slot) {
    const int inventory_type = GetInvenTypeFromItemSpace(item_space);
    const Inven_Item* const item = user->getCurCharacInvenR()->GetInvenRef(
        inventory_type, static_cast<int>(slot));
    if (!item) {
        return 0;
    }

    const CDungeon* const dungeon = getCDungeonMGr()->getDungeon();
    const unsigned int item_id = static_cast<unsigned int>(item->item_id_);
    if (dungeon->limitOfStackableItemInTower() == 1 &&
        (item_id <= 0x1963u || item_id > 0x1b57u) &&
        item_id != 10u && item_id != 24u && item_id != 51u &&
        item_id != 2660234u) {
        return 0;
    }
    return 1;
}

// IDA_EA: 0x8469B02
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage15getCurrentStageEv
// DEMANGLED: WongWork::CDeathTower::CStage::getCurrentStage(void)
int CDeathTower::CStage::getCurrentStage() {
    return current_stage_;
}

// IDA_EA: 0x8469B0C
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage16getStageMapIndexEv
// DEMANGLED: WongWork::CDeathTower::CStage::getStageMapIndex(void)
int CDeathTower::CStage::getStageMapIndex() {
    return map_info_.map_index_;
}

// IDA_EA: 0x8469B18
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage8getStateEv
// DEMANGLED: WongWork::CDeathTower::CStage::getState(void)
int CDeathTower::CStage::getState() {
    return static_cast<int>(state_);
}

// IDA_EA: 0x8461C26
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage12onStartStageEv
// DEMANGLED: WongWork::CDeathTower::CStage::onStartStage(void)
int CDeathTower::CStage::onStartStage() {
    stage_start_tick_ = GlobalData::s_systemTime_.getCurTickCount();
    return stage_start_tick_;
}

// IDA_EA: 0x8461C40
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage13onFinishStageEv
// DEMANGLED: WongWork::CDeathTower::CStage::onFinishStage(void)
int CDeathTower::CStage::onFinishStage() {
    stage_end_tick_ = GlobalData::s_systemTime_.getCurTickCount();
    return stage_end_tick_;
}

// IDA_EA: 0x8461B3E
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage15checkClearStageEv
// DEMANGLED: WongWork::CDeathTower::CStage::checkClearStage(void)
int CDeathTower::CStage::checkClearStage() {
    // The old body constructs this local once, reuses it for every map entry,
    // and destroys it at the shared return point.
    map_monster monster;
    for (std::map<int, map_monster>::iterator it = map_info_.monsters_.begin();
         it != map_info_.monsters_.end();
         ++it) {
        monster = it->second;
        if (monster.assigned_team_ == 100 && !monster.packet_value_13_) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x84617DA
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage13moveNextStageEj
// DEMANGLED: WongWork::CDeathTower::CStage::moveNextStage(unsigned int)
int CDeathTower::CStage::moveNextStage(unsigned int end_stage) {
    if (checkClearStage() != 1 ||
        static_cast<unsigned int>(current_stage_) >= end_stage) {
        return 0;
    }
    ++current_stage_;
    return 1;
}

// IDA_EA: 0x8461C5A
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage8setStateENS0_16ENUM_STAGE_STATEE
// DEMANGLED: WongWork::CDeathTower::CStage::setState(
//     WongWork::CDeathTower::ENUM_STAGE_STATE)
int CDeathTower::CStage::setState(ENUM_STAGE_STATE state) {
    if (static_cast<int>(state) == 1) {
        if (static_cast<int>(state_) != 0) {
            return 0;
        }
    } else {
        if (static_cast<int>(state) != 2 ||
            static_cast<int>(state_) != 1) {
            return 0;
        }
    }
    state_ = state;
    return 1;
}

// IDA_EA: 0x8461828
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage15makeStagePacketER11PacketGuard
// DEMANGLED: WongWork::CDeathTower::CStage::makeStagePacket(PacketGuard &)
bool CDeathTower::CStage::makeStagePacket(PacketGuard& packet) {
    packet.put_short(getCurrentStage());
    packet.put_int(static_cast<int>(get_rand_int(0)));
    packet.put_short(getStageMapIndex());

    packet.put_byte(static_cast<int>(map_info_.monsters_.size()));
    for (std::map<int, map_monster>::iterator it = map_info_.monsters_.begin();
         it != map_info_.monsters_.end();
         ++it) {
        // IDA uses map_monster C2/D2 around each value copy before packet
        // emission; retain that real value lifetime.
        map_monster monster(it->second);
        packet.put_int(monster.spawn_sequence_);
        packet.put_short(monster.map_monster_unique_id_);
        packet.put_int(monster.monster_id_);
        packet.put_byte(monster.monster_level_);
        packet.put_byte(monster.monster_kind_);
        packet.put_byte(monster.packet_value_13_);
        packet.put_byte(monster.stage_monster_type_);
    }

    packet.put_byte(static_cast<int>(map_info_.items_.size()));
    for (std::map<int, map_item>::iterator it = map_info_.items_.begin();
         it != map_info_.items_.end();
         ++it) {
        // The original copies this 0x54-byte map_item record to its stack.
        // map_item has the same real value representation in this owner.
        const map_item item = it->second;
        packet.put_byte(item.packet_value_01_);
        packet.put_short(item.unique_id_);
        packet.put_int(item.item.item_id_);
        packet.put_int(item.item.get_add_info());
        packet.put_short(item.item.avatar_select_);
        packet.put_byte(item.item.amplify_option_.getAbilityType());
        packet.put_short(item.item.amplify_option_.getAbilityValue());
    }
    return false;
}

// IDA_EA: 0x8461EE6
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData16resetMemberReadyEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::resetMemberReady(void)
bool CDeathTower::CPlayData::resetMemberReady() {
    all_members_ready_ = 0;
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        member_ready_[member_slot] = 0;
    }
    return false;
}

// IDA_EA: 0x8469B76
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData13isMemberAliveEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::isMemberAlive(int)
int CDeathTower::CPlayData::isMemberAlive(int member_slot) {
    return member_alive_[member_slot];
}

// IDA_EA: 0x8469B5A
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData14setMemberAliveEib
// DEMANGLED: WongWork::CDeathTower::CPlayData::setMemberAlive(int,bool)
int CDeathTower::CPlayData::setMemberAlive(int member_slot, bool alive) {
    member_alive_[member_slot] = alive;
    return member_slot;
}

// IDA_EA: 0x8469B86
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData14setMemberReadyEib
// DEMANGLED: WongWork::CDeathTower::CPlayData::setMemberReady(int,bool)
int CDeathTower::CPlayData::setMemberReady(int member_slot, bool ready) {
    member_ready_[member_slot] = ready;
    return member_slot;
}

// IDA_EA: 0x8469BA2
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData13isMemberReadyEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::isMemberReady(int)
int CDeathTower::CPlayData::isMemberReady(int member_slot) {
    return member_ready_[member_slot];
}

// IDA_EA: 0x8469BB2
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData11addPlayTimeEj
// DEMANGLED: WongWork::CDeathTower::CPlayData::addPlayTime(unsigned int)
CDeathTower::CPlayData*
CDeathTower::CPlayData::addPlayTime(uint32_t stage_clear_time) {
    last_play_time_ = play_time_;
    play_time_ += stage_clear_time;
    return this;
}

// IDA_EA: 0x8469BD4
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData11getPlayTimeEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::getPlayTime(void)
int CDeathTower::CPlayData::getPlayTime() {
    return play_time_;
}

// IDA_EA: 0x8469BE0
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData15getLastPlayTimeEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::getLastPlayTime(void)
int CDeathTower::CPlayData::getLastPlayTime() {
    return last_play_time_;
}

// IDA_EA: 0x8461E4A
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData18checkEnterStartMapEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::checkEnterStartMap(int)
int CDeathTower::CPlayData::checkEnterStartMap(int member_slot) {
    if (enter_start_map_[member_slot] == 1) {
        return 0;
    }
    enter_start_map_[member_slot] = 1;
    return 1;
}

// IDA_EA: 0x8461E7E
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData19checkAllMemberReadyEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::checkAllMemberReady(int)
int CDeathTower::CPlayData::checkAllMemberReady(int member_count) {
    if (all_members_ready_) {
        return 1;
    }

    int ready_count = 0;
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (member_ready_[member_slot] && ++ready_count == member_count) {
            all_members_ready_ = 1;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8461D02
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData19makeStartMemberInfoEP6CParty
// DEMANGLED: WongWork::CDeathTower::CPlayData::makeStartMemberInfo(CParty *)
CDeathTower::CPlayData*
CDeathTower::CPlayData::makeStartMemberInfo(CParty* party) {
    std::memset(start_member_info_, 0, sizeof(start_member_info_));

    int output_slot = 0;
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        CUser* const user = party->get_user(member_slot);
        if (!user || party->checkValidUser(member_slot) != 1) {
            continue;
        }

        stStartMemberInfo_t& entry = start_member_info_[output_slot++];
        std::strcpy(entry.character_name_, user->getCurCharacName());
        entry.character_level_ = user->get_charac_level();
        entry.character_job_ = user->get_charac_job();
        entry.grow_type_ = user->getCurCharacGrowType();
    }

    start_member_count_ = party->get_member_count();
    return this;
}

// IDA_EA: 0x8469B34
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData18getStartMemberInfoEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::getStartMemberInfo(void)
CDeathTower::CPlayData::stStartMemberInfo_t*
CDeathTower::CPlayData::getStartMemberInfo() {
    return start_member_info_;
}

// IDA_EA: 0x8469B26
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData17getStartMemberCntEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::getStartMemberCnt(void)
int CDeathTower::CPlayData::getStartMemberCnt() {
    return start_member_count_;
}

// IDA_EA: 0x8469B42
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData17getLastRoutedTurnEv
// DEMANGLED: WongWork::CDeathTower::CPlayData::getLastRoutedTurn(void)
int CDeathTower::CPlayData::getLastRoutedTurn() {
    return last_routed_turn_;
}

// IDA_EA: 0x8469B4C
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData17setLastRoutedTurnEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::setLastRoutedTurn(int)
CDeathTower::CPlayData*
CDeathTower::CPlayData::setLastRoutedTurn(int member_slot) {
    last_routed_turn_ = member_slot;
    return this;
}

// IDA_EA: 0x8469BEC
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData18getRewardItemCountEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::getRewardItemCount(int)
int CDeathTower::CPlayData::getRewardItemCount(int member_slot) {
    return reward_items_[member_slot].item_count_;
}

// IDA_EA: 0x8469C06
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData13getRewardItemEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::getRewardItem(int)
Inven_Item* CDeathTower::CPlayData::getRewardItem(int member_slot) {
    return reward_items_[member_slot].items_;
}

// IDA_EA: 0x8469C1E
// IDA_NAME: _ZN8WongWork11CDeathTower9CPlayData12getRewardExpEi
// DEMANGLED: WongWork::CDeathTower::CPlayData::getRewardExp(int)
int CDeathTower::CPlayData::getRewardExp(int member_slot) {
    return reward_exp_[member_slot];
}

// IDA_EA: 0x8467188
// IDA_NAME: _ZN8WongWork11CDeathTower15_checkMemberDieEv
// DEMANGLED: WongWork::CDeathTower::_checkMemberDie(void)
int CDeathTower::_checkMemberDie() {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (party_->checkValidUser(member_slot) &&
            play_data_.isMemberAlive(member_slot)) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x846595A
// IDA_NAME: _ZN8WongWork11CDeathTower13handleDieUserEP5CUser
// DEMANGLED: WongWork::CDeathTower::handleDieUser(CUser *)
int CDeathTower::handleDieUser(CUser* user) {
    for (int member_slot = 0; member_slot <= 3; ++member_slot) {
        if (party_->checkValidUser(member_slot) != 1 ||
            party_->get_user(member_slot) != user) {
            continue;
        }

        if (play_data_.isMemberAlive(member_slot) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_DIE_CHARACTER, 18);
            return 0;
        }

        play_data_.setMemberAlive(member_slot, false);
        CPacketHandler packet;
        packet.makeDieState(user->get_unique_id());
        party_->send_to_party(packet);
        break;
    }

    if (_checkMemberDie()) {
        _onFinishStage();
        _onPrepareFinishTower();
    }
    return 1;
}

// IDA_EA: 0x84670F8
// IDA_NAME: _ZN8WongWork11CDeathTower17_checkMemberReadyEv
// DEMANGLED: WongWork::CDeathTower::_checkMemberReady(void)
int CDeathTower::_checkMemberReady() {
    play_data_.resetMemberReady();
    const int timeout = (party_->get_member_count() == 1)
                            ? CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT
                            : CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT;
    const int timer_key = _genTimerKey(static_cast<TIMER_MESSAGE>(73));
    return G_TimerQueue()->InsertTimer(6, static_cast<int>(index_), 73,
                                       timeout,
                                       static_cast<unsigned int>(timer_key),
                                       73);
}

// IDA_EA: 0x8465C94
// IDA_NAME: _ZN8WongWork11CDeathTower13handleMoveMapEv
// DEMANGLED: WongWork::CDeathTower::handleMoveMap(void)
int CDeathTower::handleMoveMap() {
    if (_checkMemberDie()) {
        CPacketHandler packet;
        packet.makeCommonError(ENUM_CMDPACKET_MOVE_MAP,
                               static_cast<ENUM_ERROR>(22));
        party_->send_to_party(packet);
        return 0;
    }

    if (stage_.getState() <= 1) {
        CPacketHandler packet;
        packet.makeCommonError(ENUM_CMDPACKET_MOVE_MAP,
                               static_cast<ENUM_ERROR>(19));
        party_->send_to_party(packet);
        return 0;
    }

    const int end_stage = dungeon_manager_.getEndStage();
    if (!stage_.moveNextStage(static_cast<unsigned int>(end_stage))) {
        CPacketHandler packet;
        packet.makeCommonError(ENUM_CMDPACKET_MOVE_MAP,
                               static_cast<ENUM_ERROR>(22));
        party_->send_to_party(packet);
        return 0;
    }

    try {
        const int current_stage = stage_.getCurrentStage();
        CMap* const map = dungeon_manager_.getStageMap(
            static_cast<unsigned int>(current_stage));
        if (!map) {
            throw static_cast<ENUM_ERROR>(21);
        }

        stage_.reset();
        if (stage_.consistMap(dungeon_manager_, map, this) != 1) {
            throw static_cast<ENUM_ERROR>(1);
        }
    } catch (ENUM_ERROR error) {
        CPacketHandler packet;
        packet.makeCommonError(ENUM_CMDPACKET_MOVE_MAP, error);
        party_->send_to_party(packet);
        return 0;
    }

    CPacketHandler packet;
    packet.put_header(0, 143);
    stage_.makeStagePacket(packet);
    packet.finalize(true);
    party_->send_to_party(packet);
    _checkMemberReady();
    return 1;
}

// IDA_EA: 0x8466150
// IDA_NAME: _ZN8WongWork11CDeathTower18handleStageCommandEP5CUserh
// DEMANGLED: WongWork::CDeathTower::handleStageCommand(CUser *,unsigned char)
int CDeathTower::handleStageCommand(CUser* user, uint8_t command) {
    if (party_->getManager() != user) {
        return 0;
    }

    if (command == 1) {
        if (stage_.setState(static_cast<ENUM_STAGE_STATE>(1)) != 1) {
            return 1;
        }
        if (_checkMemberDie()) {
            return 1;
        }
        _onStartStage();
        return 1;
    }

    if (command != 2) {
        return 0;
    }
    if (stage_.setState(static_cast<ENUM_STAGE_STATE>(2)) != 1) {
        return 1;
    }
    return 1;
}

// IDA_EA: 0x8467CAC
// IDA_NAME: _ZN8WongWork11CDeathTower13_onStartStageEv
// DEMANGLED: WongWork::CDeathTower::_onStartStage(void)
int CDeathTower::_onStartStage() {
    return stage_.onStartStage();
}

// IDA_EA: 0x8467CC2
// IDA_NAME: _ZN8WongWork11CDeathTower14_onFinishStageEv
// DEMANGLED: WongWork::CDeathTower::_onFinishStage(void)
bool CDeathTower::_onFinishStage() {
    stage_.onFinishStage();
    const unsigned int stage_clear_time = stage_.getStageClearTime();
    play_data_.addPlayTime(stage_clear_time);

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }

        if (stage_.getStageClearTime() <= 0x176Fu) {
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<ENUM_HACKTYPE>(0x19A), 1, 0, 0);
        }

        user->getCurCharacQuestW()->set_authen_data(
            static_cast<QUEST_CONDITION>(1), stage_.getStageMapIndex(), 0);
    }

    // The original loop stores `seat <= 3` into AL before terminating at
    // seat 4, so its observable bool return is false.
    return false;
}

// IDA_EA: 0x8467E00
// IDA_NAME: _ZN8WongWork11CDeathTower21_onPrepareFinishTowerEv
// DEMANGLED: WongWork::CDeathTower::_onPrepareFinishTower(void)
int CDeathTower::_onPrepareFinishTower() {
    const int timer_key = _genTimerKey(static_cast<TIMER_MESSAGE>(74));
    return G_TimerQueue()->InsertTimer(
        6, static_cast<int>(index_), 74, 5, timer_key, 74);
}

// IDA_EA: 0x8467E60
// IDA_NAME: _ZN8WongWork11CDeathTower8_onClearEb
// DEMANGLED: WongWork::CDeathTower::_onClear(bool)
bool CDeathTower::_onClear(bool cleared) {
    const int elapsed_msec =
        cleared ? play_data_.getPlayTime() : play_data_.getLastPlayTime();

    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (!user || party_->checkValidUser(seat) != 1) {
            continue;
        }

        user->setChattingMessageCount(0);
        user->history_log().DungeonClearInfo(
            static_cast<int>(cleared), static_cast<long>(elapsed_msec / 1000));
    }

    // Like _onFinishStage, the decompiled loop returns its final `seat <= 3`
    // test and therefore exposes false after the terminating fourth seat.
    return false;
}

// IDA_EA: 0x8467F6E
// IDA_NAME: _ZN8WongWork11CDeathTower19_onFinishDeathTowerEv
// DEMANGLED: WongWork::CDeathTower::_onFinishDeathTower(void)
void CDeathTower::_onFinishDeathTower() {
    for (int seat = 0; seat <= 3; ++seat) {
        CUser* const user = party_->get_user(seat);
        if (user && party_->checkValidUser(seat) == 1) {
            onLeaveUser(user);
        }
    }

    party_->ReturnToVillage();
    if (getTowerState() > 0) {
        _destroy();
    }
}

// IDA_EA: 0x8469C5C
// IDA_NAME: _ZN8WongWork11CDeathTower13getTowerStateEv
// DEMANGLED: WongWork::CDeathTower::getTowerState(void)
int CDeathTower::getTowerState() {
    return static_cast<int>(tower_state_);
}

// IDA_EA: 0x8469C68
// IDA_NAME: _ZN8WongWork11CDeathTower14_checkTimerKeyE13TIMER_MESSAGEj
// DEMANGLED: WongWork::CDeathTower::_checkTimerKey(TIMER_MESSAGE,unsigned int)
bool CDeathTower::_checkTimerKey(TIMER_MESSAGE message,
                                 unsigned int timer_key) {
    // IDA 0x8469C6E subtracts 0x48 before forming
    // this + 0x0c + 4 * (message - 0x48 + 0x2cc).  The physical key array
    // therefore begins at +0xb3c with message 72, exactly like
    // _genTimerKey @ 0x830EDEC.  This includes the six TimerDeathTower
    // message slots 73..78.
    return timer_keys_[static_cast<int>(message) - 72] == timer_key;
}

// IDA_EA: 0x846801E
// IDA_NAME: _ZN8WongWork11CDeathTower20_checkRenewTopRecordEjjjj
// DEMANGLED: WongWork::CDeathTower::_checkRenewTopRecord(unsigned int,
//             unsigned int,unsigned int,unsigned int)
int CDeathTower::_checkRenewTopRecord(unsigned int dungeon_index,
                                      unsigned int member_count,
                                      unsigned int stage,
                                      unsigned int play_time) {
    const int tower_index =
        CDeathTowerRanking::makeDungeonIdx2TowerIdx(dungeon_index);
    CDeathTowerRanking* const rank_table =
        CDeathTowerRanking::getRankTable(member_count);
    return rank_table->checkRenewTopRecord(
        static_cast<unsigned int>(tower_index), stage, play_time);
}

// IDA_EA: 0x846805E
// IDA_NAME: _ZN8WongWork11CDeathTower19_checkRenewMyRecordEjjjjj
// DEMANGLED: WongWork::CDeathTower::_checkRenewMyRecord(unsigned int,
//             unsigned int,unsigned int,unsigned int,unsigned int)
int CDeathTower::_checkRenewMyRecord(unsigned int dungeon_index,
                                     unsigned int member_count,
                                     unsigned int charac_no,
                                     unsigned int stage,
                                     unsigned int play_time) {
    const int tower_index =
        CDeathTowerRanking::makeDungeonIdx2TowerIdx(dungeon_index);
    CDeathTowerRanking* const rank_table =
        CDeathTowerRanking::getRankTable(member_count);
    return rank_table->checkRenewMyRecord(
        static_cast<unsigned int>(tower_index),
        charac_no,
        stage,
        play_time);
}

}  // namespace WongWork
