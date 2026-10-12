#include "old_game/pvp/battlefield/battle_field.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/dungeon/tower/tower_rank_blood_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/channel/channel_script_t.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/log/format/trace.h"
#include "infra/log/format/history_trace.h"

// IDA_EA: 0x80CA15C
// CALLEE_IDA_NAME: _Z12get_rand_inti
// DEMANGLED: get_rand_int(int)
unsigned int get_rand_int(int limit);

namespace WongWork {

// IDA_EA: 0x81519B6
// IDA_NAME: _ZN8WongWork9CBossPlay14stRewardItem_tC2Ev
// DEMANGLED: WongWork::CBossPlay::stRewardItem_t::stRewardItem_t(void)
CBossPlay::stRewardItem_t::stRewardItem_t() {}

// IDA_EA: 0x8149120
// IDA_NAME: _ZN8WongWork9CBossPlay5resetEv
// DEMANGLED: WongWork::CBossPlay::reset(void)
CBossPlay* CBossPlay::reset() {
    party_ = 0;
    state_08_ = 0;
    all_died_ = 0;
    std::memset(member_alive_, 0, sizeof(member_alive_));
    std::memset(member_ready_, 0, sizeof(member_ready_));
    std::memset(values_, 0, sizeof(values_));

    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        stRewardItem_t& reward = rewards_[member_slot];
        reward.reward_value_ = 0;
        for (int item_slot = 0; item_slot != 10; ++item_slot) {
            Inven_Item& item = reward.items_[item_slot];
            item.reset();
            item.item_id_ = -1;
        }
    }
    std::memset(tail_9c8_, 0, sizeof(tail_9c8_));
    std::memset(tail_9d8_, 0, sizeof(tail_9d8_));
    return this;
}

// IDA_EA: 0x8145682
// IDA_NAME: _ZN8WongWork9CBossPlay13isMemberAliveEi
// DEMANGLED: WongWork::CBossPlay::isMemberAlive(int)
int CBossPlay::isMemberAlive(int member_slot) {
    return member_alive_[member_slot];
}

// IDA_EA: 0x8145692
// IDA_NAME: _ZN8WongWork9CBossPlay14setMemberReadyEib
// DEMANGLED: WongWork::CBossPlay::setMemberReady(int,bool)
int CBossPlay::setMemberReady(int member_slot, bool ready) {
    member_ready_[member_slot] = ready;
    return member_slot;
}

// IDA_EA: 0x814A7E4
// IDA_NAME: _ZN8WongWork9CBossPlay14setMemberAliveEib
// DEMANGLED: WongWork::CBossPlay::setMemberAlive(int,bool)
int CBossPlay::setMemberAlive(int member_slot, bool alive) {
    member_alive_[member_slot] = alive;
    return member_slot;
}

// IDA_EA: 0x8149914
// IDA_NAME: _ZN8WongWork9CBossPlay18generateRewardItemEiij23ENUM_DUNGEON_DIFFICULTYjPK8CDungeon
// DEMANGLED: WongWork::CBossPlay::generateRewardItem(int,int,unsigned int,ENUM_DUNGEON_DIFFICULTY,unsigned int,CDungeon const*)
int CBossPlay::generateRewardItem(int member_slot,
                                  int item_count,
                                  unsigned int character_level,
                                  ENUM_DUNGEON_DIFFICULTY difficulty,
                                  unsigned int context,
                                  const CDungeon* dungeon) {
    stRewardItem_t& reward = rewards_[member_slot];
    reward.reward_value_ = static_cast<uint32_t>(item_count);

    stDeathTower_t* const tower_script =
        G_CDataManager()->GetTowerScript(dungeon->get_index());
    const int random_item_threshold = tower_script->reward_item_prob_cumulative[0];
    const int gold_item_threshold = tower_script->reward_item_prob_cumulative[1];
    const float gold_scale = tower_script->reward_gold_weight;
    CItemGenerator* const generator =
        G_CDataManager()->getItemGenerator()->getGenerator(
            CItemGeneratorMgr::kClearReward);

    for (int item_slot = 0; item_slot < item_count; ++item_slot) {
        stGenerateRefData_t reference;
        stGenerateResult_t result;
        const unsigned int random_value = get_rand_int(1000);

        if (static_cast<unsigned int>(context * random_item_threshold) <=
            random_value) {
            if (static_cast<unsigned int>(context * gold_item_threshold) <=
                random_value) {
                continue;
            }

            reference.generator_data.clear_reward
                .character_level_or_world_area =
                    static_cast<uint8_t>(character_level);
            reference.generator_data.clear_reward
                .dungeon_difficulty_or_world_index =
                    static_cast<uint8_t>(difficulty);
            reference.generator_data.clear_reward.item_probability_scale =
                1.0f;
            reference.generator_data.clear_reward.clear_reward_drop_probability = 0;
            reference.generator_data.clear_reward.dungeon =
                const_cast<CDungeon*>(dungeon);
            generator->generateRandomItem(reference, result);
            if (result.items.empty()) continue;

            reward.items_[item_slot].setCopy(result.items.front());
            CItem* const item = G_CDataManager()->find_item(
                reward.items_[item_slot].getKey());
            if (item) {
                CLuckPoint::UseLuckPoint(
                    party_->get_user(member_slot),
                    static_cast<int>(reward.items_[item_slot].item_category_),
                    item->get_rarity(), item->get_grade());
            }
            continue;
        }

        reference.generator_data.gold_reward.character_level =
            static_cast<uint8_t>(character_level);
        reference.generator_data.gold_reward.dungeon_difficulty =
            static_cast<uint8_t>(difficulty);
        reference.generator_data.gold_reward.party_member_count =
            static_cast<uint8_t>(party_->get_member_count());
        reference.generator_data.gold_reward.exp_bonus_context = 1;
        reference.generator_data.gold_reward.gold_weight_08 = 1;
        reference.generator_data.gold_reward.gold_weight_0a = 1;
        reference.generator_data.gold_reward.gold_reserved_0c = 1;
        reference.generator_data.gold_reward.gold_weight_0e = 1;
        generator->generateSpecificItem(reference, result);
        if (!result.items.empty()) {
            reward.items_[item_slot].setCopy(result.items.front());
            reward.items_[item_slot].set_add_info(static_cast<int>(
                static_cast<long double>(reward.items_[item_slot].get_add_info()) *
                gold_scale));
        }
    }
    return 1;
}

// IDA_EA: 0x8149E20
// IDA_NAME: _ZN8WongWork9CBossPlay17generateRewardExpEijjPK8CDungeon
// DEMANGLED: WongWork::CBossPlay::generateRewardExp(int,unsigned int,unsigned int,CDungeon const*)
int CBossPlay::generateRewardExp(int member_slot,
                                 unsigned int stage,
                                 unsigned int multiplier,
                                 const CDungeon* dungeon) {
    CUser* const user = party_->get_user(member_slot);
    if (!user) {
        return 0;
    }

    const float stage_exp = static_cast<float>(multiplier * stage);
    const float exp_weight = G_CDataManager()
                                 ->GetTowerScript(dungeon->get_index())
                                 ->reward_exp_weight[stage];
    unsigned int base_exp = static_cast<unsigned int>(
        static_cast<long double>(exp_weight) * stage_exp);

    stClearRewardBonusExp_t bonus_exp;
    stClearRewardExpParameter_t parameter;

    // IDA 0x8149E20 constructs parameter but never writes to it.  The
    // following are independent locals retained for the old overflow log.
    const float avatar_bonus = static_cast<float>(party_->getAvatarBonus());
    const float creature_bonus =
        static_cast<float>(party_->getCreatureBonus(user));

    float pcroom_bonus;
    CEventBase* const pcroom_event =
        GlobalData::s_event_manager->GetRepeatEvent(8);
    if (pcroom_event->IsEventing(user)) {
        CPCRoomBurningEvent* const event =
            static_cast<CPCRoomBurningEvent*>(pcroom_event);
        pcroom_bonus = static_cast<float>(event->GetExpFactor() / 100);
    }

    float double_exp_bonus;
    CEventBase* const double_exp_event =
        GlobalData::s_event_manager->GetRepeatEvent(3);
    if (double_exp_event->IsEventing(0)) {
        CExpDoubleEvent* const event =
            static_cast<CExpDoubleEvent*>(double_exp_event);
        double_exp_bonus = static_cast<float>(event->GetExpFactor()) / 100.0f;
    }

    int burning_fatigue_bonus;
    CEventBase* const burning_fatigue_event =
        GlobalData::s_event_manager->GetRepeatEvent(15);
    if (burning_fatigue_event->IsEventing(0)) {
        CBurningFatigueEvent* const event =
            static_cast<CBurningFatigueEvent*>(burning_fatigue_event);
        burning_fatigue_bonus = event->getBonusExpRate(
            static_cast<uint16_t>(user->getCurCharacUsedFatigue()));
    }

    channel_script_t* const channel_script =
        G_CDataManager()->GetChannelScript();
    const float channel_bonus = static_cast<float>(channel_script->getBonusRate(
        static_cast<uint8_t>(G_CEnvironment()->get_server_group()),
        static_cast<uint32_t>(G_CEnvironment()->get_channel_no()),
        dungeon->get_index()));

    float premium_bonus;
    if (user->IsHavePremiumAdvantage()) {
        premium_bonus =
            static_cast<float>(user->GetPremiumInfo()->GetAdvantageExpRate()) /
            100.0f;
    }
    const float server_event_bonus =
        static_cast<float>(CServerEvent::GetExpRate()) / 100.0f;

    unsigned int clear_reward_exp = party_->getClearRewardBonusExp(
        user, base_exp, parameter, bonus_exp);
    if (clear_reward_exp > 0x10C8E0u) {
        LogManager::logFormat(
            1, "localglobal/global_PlayUser.cpp",
            "virtual bool WongWork::CBossPlay::generateRewardExp(int, unsigned int, unsigned int, const CDungeon*)",
            0x159, "BOSS_LOG : EROOR BOSSTOWER CLEAR REWARD EXP(%u)\n",
            clear_reward_exp);
        LogManager::logFormat(
            1, "localglobal/global_PlayUser.cpp",
            "virtual bool WongWork::CBossPlay::generateRewardExp(int, unsigned int, unsigned int, const CDungeon*)",
            0x163,
            "BOSS_LOG : BOSSTOWER EXP PARAMETER : %f, %f, %f, %f, %f, %f, %f, %d\n",
            static_cast<double>(avatar_bonus), static_cast<double>(creature_bonus),
            static_cast<double>(pcroom_bonus),
            static_cast<double>(double_exp_bonus),
            static_cast<double>(channel_bonus), static_cast<double>(premium_bonus),
            static_cast<double>(server_event_bonus), burning_fatigue_bonus);
        clear_reward_exp = 1100000u;
    }
    values_[member_slot] = clear_reward_exp;
    return 1;
}

// IDA_EA: 0x8149382
// IDA_NAME: _ZN8WongWork9CBossPlay15_checkMemberDieEv
// DEMANGLED: WongWork::CBossPlay::_checkMemberDie(void)
int CBossPlay::_checkMemberDie() {
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        if (party_->checkValidUser(member_slot) &&
            isMemberAlive(member_slot)) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x81493FE
// IDA_NAME: _ZN8WongWork9CBossPlay19checkAllMemberReadyEi
// DEMANGLED: WongWork::CBossPlay::checkAllMemberReady(int)
int CBossPlay::checkAllMemberReady(int member_count) {
    if (all_died_) return 1;

    int ready_count = 0;
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        if (member_ready_[member_slot] && ++ready_count == member_count) {
            all_died_ = 1;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x81494A4
// IDA_NAME: _ZN8WongWork9CBossPlay13handleDieUserEP5CUser
// DEMANGLED: WongWork::CBossPlay::handleDieUser(CUser *)
int CBossPlay::handleDieUser(CUser* user) {
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        if (!party_->checkValidUser(member_slot) ||
            party_->get_user(member_slot) != user) {
            continue;
        }

        if (!isMemberAlive(member_slot)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_DIE_CHARACTER, 18);
            return 0;
        }

        setMemberAlive(member_slot, false);
        {
            PacketGuard packet;
            packet.put_header(0, 32);
            packet.put_short(user->get_unique_id());
            packet.put_byte(0);
            packet.finalize(true);
            party_->send_to_party(packet);
        }

        if (user->GetRevivalTime() > GlobalData::s_systemTime_.getCurSec()) {
            user->SetRevivalTime(0);
            user->SetUseAPCPotionTime(0);
            setMemberAlive(member_slot, true);

            PacketGuard packet;
            packet.put_header(0, 32);
            packet.put_short(user->get_unique_id());
            packet.put_byte(2);
            packet.finalize(true);
            party_->send_to_party(packet);
        }
        return 1;
    }
    return 1;
}

// IDA_EA: 0x8149724
// IDA_NAME: _ZN8WongWork9CBossPlay15handleLeaveUserEP5CUser
// DEMANGLED: WongWork::CBossPlay::handleLeaveUser(CUser *)
int CBossPlay::handleLeaveUser(CUser* user) {
    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        if (!party_->checkValidUser(member_slot) ||
            party_->get_user(member_slot) != user) {
            continue;
        }

        if (values_[member_slot] == 0 && user->get_state() == 12) {
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<ENUM_HACKTYPE>(0x13a), 1, 0, 0);
        }
        party_->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
        setMemberAlive(member_slot, false);
    }

    G_GameWorld()->out_from_dungeon(user);
    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    return 1;
}

// IDA_EA: 0x8143996
// IDA_NAME: _ZN8WongWork10CBossTower13handleMoveMapEPt
// DEMANGLED: WongWork::CBossTower::handleMoveMap(unsigned short *)
int CBossTower::handleMoveMap(unsigned short* map_path) {
    if (!CBossStage::handleMoveMap(map_path)) return 0;

    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (user && isMemberAlive(member_slot)) {
            entrance_log_.IncrementBossStageStart(
                *user, static_cast<uint8_t>(current_stage_),
                static_cast<uint8_t>(member_count_class_));
        }
    }
    return 1;
}

// IDA_EA: 0x8143A62
// IDA_NAME: _ZN8WongWork10CBossTower13handleDieUserEP5CUser
// DEMANGLED: WongWork::CBossTower::handleDieUser(CUser *)
int CBossTower::handleDieUser(CUser* user) {
    if (!user || !CBossPlay::handleDieUser(user)) return 0;

    if (_checkMemberDie()) {
        onFinishStage();
        if (!prepare_finish_) _onPrepareFinishTower(false);
    }
    return 1;
}

// IDA_EA: 0x81454B8
// IDA_NAME: _ZN8WongWork10CBossTower13handleUseItemEP5CUsermh
// DEMANGLED: WongWork::CBossTower::handleUseItem(CUser *,unsigned long,unsigned char)
int CBossTower::handleUseItem(CUser* user,
                              unsigned long item_id,
                              unsigned char item_category) {
    const int member_slot = party_->GetMemberSlotNo(user);
    if (member_slot == -1) {
        return 0;
    }
    if (item_id == 3037u) {
        ++tail_9c8_[member_slot];
    }
    if (item_category == 2u) {
        ++tail_9d8_[member_slot];
    }
    return 1;
}

// IDA_EA: 0x8143D2C
// IDA_NAME: _ZN8WongWork10CBossTower16handlePickupItemEP5CUseribc
// DEMANGLED: WongWork::CBossTower::handlePickupItem(CUser *,int,bool,char)
int CBossTower::handlePickupItem(CUser* user,
                                 int unique_id,
                                 bool client_flag,
                                 char item_rarity) {
    if (!party_ || party_->IsRoutingState()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GET_ITEM, 216);
        return 0;
    }

    char server_item_rarity = 0;
    if (!CBossStage::handlePickupItem(user, unique_id, client_flag,
                                      server_item_rarity)) {
        return 0;
    }
    if (item_rarity == 3) {
        entrance_log_.IncrementBossPickUpItem(
            *user, static_cast<uint8_t>(current_stage_),
            static_cast<uint8_t>(member_count_class_));
    }
    return 1;
}

// IDA_EA: 0x8143E5E
// IDA_NAME: _ZN8WongWork10CBossTower14handleDropItemEP5CUseriicii
// DEMANGLED: WongWork::CBossTower::handleDropItem(CUser *,int,int,char,int,int)
int CBossTower::handleDropItem(CUser* user,
                               int item_id,
                               int item_count,
                               char rarity,
                               int x,
                               int y) {
    return CBossStage::handleDropItem(user, item_id, item_count, rarity, x, y) ==
           1;
}

// IDA_EA: 0x8143EBC
// IDA_NAME: _ZN8WongWork10CBossTower17handleKillMonsterEP5CUserttRK15MSG_MONSTER_DIEPi
// DEMANGLED: WongWork::CBossTower::handleKillMonster(CUser *,unsigned short,unsigned short,MSG_MONSTER_DIE const&,int *)
int CBossTower::handleKillMonster(CUser* user,
                                  unsigned short monster_id,
                                  unsigned short monster_type,
                                  const MSG_MONSTER_DIE& message,
                                  int* result) {
    if (!CBossStage::handleKillMonster(user, monster_id, monster_type, message,
                                       result)) {
        return 0;
    }
    if (map_info_.monsters_.empty()) {
        onFinishStage();
        if (current_stage_ >= maximum_stage_) _onPrepareFinishTower(true);
    }
    return 1;
}

// IDA_EA: 0x81519EC
// IDA_NAME: _ZN8WongWork9CBossPlayC2Ev
// DEMANGLED: WongWork::CBossPlay::CBossPlay(void)
CBossPlay::CBossPlay() : rewards_() {
    reset();
}

// IDA_EA: 0x8151A3A
// IDA_NAME: _ZN8WongWork9CBossPlayD2Ev
// DEMANGLED: WongWork::CBossPlay::~CBossPlay()
CBossPlay::~CBossPlay() {
    reset();
}

// IDA_EA: 0x814BD0C
// IDA_NAME: _ZN8WongWork10CBossStageC2Ev
// DEMANGLED: WongWork::CBossStage::CBossStage(void)
CBossStage::CBossStage() : CBossPlay(), map_info_() {
    current_stage_ = 1;
    dungeon_ = 0;
    member_count_class_ = 0;
    map_info_.Clear();
    map_info_.next_monster_unique_id_ = static_cast<int>(get_rand_int(60000));
    map_info_.next_drop_item_unique_id_ =
        static_cast<int>(get_rand_int(60000));
}

// IDA_EA: 0x814BDD6
// IDA_NAME: _ZN8WongWork10CBossStageD2Ev
// DEMANGLED: WongWork::CBossStage::~CBossStage()
CBossStage::~CBossStage() {
    current_stage_ = 1;
    dungeon_ = 0;
    member_count_class_ = 0;
    map_info_.Clear();
    map_info_.next_monster_unique_id_ = static_cast<int>(get_rand_int(60000));
    map_info_.next_drop_item_unique_id_ =
        static_cast<int>(get_rand_int(60000));
}

}  // namespace WongWork

// IDA_EA: 0x81456C2
// IDA_NAME: _ZN23CBossDungeonEntranceLogC2Ev
// DEMANGLED: CBossDungeonEntranceLog::CBossDungeonEntranceLog(void)
CBossDungeonEntranceLog::CBossDungeonEntranceLog()
    : std::map<uint32_t, STBossDungeonTowerLog>() {
    reset();
}

// IDA_EA: 0x8145704
// IDA_NAME: _ZN23CBossDungeonEntranceLogD2Ev
// DEMANGLED: CBossDungeonEntranceLog::~CBossDungeonEntranceLog()
CBossDungeonEntranceLog::~CBossDungeonEntranceLog() {
    reset();
}

// IDA_EA: 0x8145746
// IDA_NAME: _ZN23CBossDungeonEntranceLog5resetEv
// DEMANGLED: CBossDungeonEntranceLog::reset(void)
CBossDungeonEntranceLog* CBossDungeonEntranceLog::reset() {
    clear();
    return this;
}

namespace {

// IDA 0x81564F0/0x815662E/0x815678E materializes this four-byte key from
// the character level, grow type, low-nibble job, compact member-count class,
// and the boss stage.  Keeping the packing explicit preserves the map's
// per-character/per-stage aggregation without relying on a stack overlay.
uint32_t make_boss_entrance_log_key(const CUser& user,
                                    uint8_t stage,
                                    uint8_t member_count_class) {
    const uint32_t level = static_cast<uint8_t>(user.get_charac_level());
    const uint32_t grow_type =
        static_cast<uint8_t>(user.getCurCharacGrowType());
    const uint32_t job_and_member_count =
        (static_cast<uint32_t>(user.get_charac_job()) & 0x0fu) |
        ((static_cast<uint32_t>(member_count_class) & 0x0fu) << 4);

    return level |
           (grow_type << 8) |
           (job_and_member_count << 16) |
           (static_cast<uint32_t>(stage) << 24);
}

}  // namespace

// IDA_EA: 0x81564F0
// IDA_NAME: _ZN23CBossDungeonEntranceLog23IncrementBossStageStartER5CUserhh
// DEMANGLED: CBossDungeonEntranceLog::IncrementBossStageStart(CUser &,unsigned char,unsigned char)
int CBossDungeonEntranceLog::IncrementBossStageStart(
    CUser& user,
    uint8_t stage,
    uint8_t member_count_class) {
    const uint32_t key = make_boss_entrance_log_key(
        user, stage, member_count_class);
    STBossDungeonTowerLog& entry = (*this)[key];
    ++entry.stage_start_count;
    return 1;
}

// IDA_EA: 0x815662E
// IDA_NAME: _ZN23CBossDungeonEntranceLog21IncrementBossStageEndER5CUserhjh
// DEMANGLED: CBossDungeonEntranceLog::IncrementBossStageEnd(CUser &,unsigned char,unsigned int,unsigned char)
int CBossDungeonEntranceLog::IncrementBossStageEnd(
    CUser& user,
    uint8_t stage,
    unsigned int elapsed_seconds,
    uint8_t member_count_class) {
    const uint32_t key = make_boss_entrance_log_key(
        user, stage, member_count_class);
    STBossDungeonTowerLog& entry = (*this)[key];
    ++entry.stage_end_count;
    entry.elapsed_time += elapsed_seconds;
    return 1;
}

// IDA_EA: 0x815678E
// IDA_NAME: _ZN23CBossDungeonEntranceLog23IncrementBossPickUpItemER5CUserhh
// DEMANGLED: CBossDungeonEntranceLog::IncrementBossPickUpItem(CUser &,unsigned char,unsigned char)
int CBossDungeonEntranceLog::IncrementBossPickUpItem(
    CUser& user,
    uint8_t stage,
    uint8_t member_count_class) {
    const uint32_t key = make_boss_entrance_log_key(
        user, stage, member_count_class);
    STBossDungeonTowerLog& entry = (*this)[key];
    ++entry.picked_item_count;
    return 1;
}

// IDA_EA: 0x81568CC
// IDA_NAME: _ZN23CBossDungeonEntranceLog20BossTowerLogDBUpdateEv
// DEMANGLED: CBossDungeonEntranceLog::BossTowerLogDBUpdate(void)
void CBossDungeonEntranceLog::BossTowerLogDBUpdate() {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire(
            "localglobal/global_Statistics.cpp", 105),
        true);
    const unsigned char entry_count = static_cast<unsigned char>(size());
    if (!entry_count) {
        return;
    }

    guard << 736;
    guard << -1;
    guard << entry_count;
    for (iterator it = begin(); it != end(); ++it) {
        guard << it->first;
        STBossDungeonTowerLog* const payload =
            guard.GetInBuffer<STBossDungeonTowerLog>();
        std::memcpy(payload, &it->second, sizeof(*payload));
        std::memset(&it->second, 0, sizeof(it->second));
    }
    clear();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
}

namespace WongWork {

// IDA_EA: 0x8142B36
// IDA_NAME: _ZN8WongWork10CBossTower8_destroyEv
// DEMANGLED: WongWork::CBossTower::_destroy(void)
int CBossTower::_destroy() {
    entrance_log_.BossTowerLogDBUpdate();
    entrance_log_.reset();
    G_CGameManager()->returnBossTower(this);
    return 1;
}

// IDA_EA: 0x8143AE2
// IDA_NAME: _ZN8WongWork10CBossTower15handleLeaveUserEP5CUser
// DEMANGLED: WongWork::CBossTower::handleLeaveUser(CUser *)
int CBossTower::handleLeaveUser(CUser* user) {
    if (!user || !CBossPlay::handleLeaveUser(user)) {
        return 0;
    }

    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    const int dimension_mode = party_->battle_field().instance_dungeon_mode();
    party_->getMemberNames(member_names);
    CUser_history_log(user)->LeaveDungeon(
        dungeon_->GetDungeonName(),
        static_cast<int>(member_count_class_),
        member_names,
        dimension_mode);

    cHistoryTrace history_trace = {};
    history_trace.source_code = static_cast<char>(dimension_mode);
    history_trace("member_count : %d", party_->get_member_count());

    if (party_->get_member_count() <= 0) {
        _destroy();
        return 0;
    }

    if (_checkMemberDie()) {
        onFinishStage();
        _onPrepareFinishTower(false);
    } else {
        const int member_count = party_->get_member_count();
        if (member_count == 1) {
            member_count_class_ = 0;
        } else if (member_count == 2) {
            member_count_class_ = 1;
        } else if (member_count == 3) {
            member_count_class_ = 2;
        } else if (member_count == 4) {
            member_count_class_ = 3;
        }
    }
    return 1;
}

// IDA_EA: 0x81440B2
// IDA_NAME: _ZN8WongWork10CBossTower13onFinishStageEv
// DEMANGLED: WongWork::CBossTower::onFinishStage(void)
int CBossTower::onFinishStage() {
    stage_end_tick_ = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    const uint32_t elapsed_seconds =
        (stage_end_tick_ - stage_start_tick_) / 1000u;

    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (user && party_->checkValidUser(member_slot) &&
            isMemberAlive(member_slot)) {
            entrance_log_.IncrementBossStageEnd(
                *user,
                static_cast<uint8_t>(current_stage_),
                elapsed_seconds,
                static_cast<uint8_t>(member_count_class_));
        }
    }
    return 1;
}

// IDA_EA: 0x8142D24
// IDA_NAME: _ZN8WongWork10CBossTower21_onPrepareFinishTowerEb
// DEMANGLED: WongWork::CBossTower::_onPrepareFinishTower(bool)
int CBossTower::_onPrepareFinishTower(bool completed) {
    prepare_finish_ = completed ? 1u : 0u;
    stage_finish_tick_ = static_cast<uint32_t>(
        GlobalData::s_systemTime_.getCurTickCount());
    const uint32_t elapsed_seconds =
        (stage_finish_tick_ - stage_prepare_tick_) / 1000u;

    // The original obtains this snapshot before walking the members.  It is
    // retained even though the history event itself carries only completion
    // and elapsed time; getMemberNames owns the canonical party snapshot.
    char member_names[256];
    std::memset(member_names, 0, sizeof(member_names));
    party_->getMemberNames(member_names);

    for (int member_slot = 0; member_slot != 4; ++member_slot) {
        CUser* const user = party_->get_user(member_slot);
        if (!user || !party_->checkValidUser(member_slot)) {
            continue;
        }
        if (tail_9d8_[member_slot] == 0) {
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<ENUM_HACKTYPE>(0x13a), 1, 0, 0);
        }
        CUser_history_log(user)->DungeonClearInfo(
            completed ? 1 : 0, static_cast<long>(elapsed_seconds));
    }

    return G_TimerQueue()->InsertTimer(
        7, static_cast<int>(pool_index_), 163, 5, 163, 163);
}

// IDA_EA: 0x81429B4
// IDA_NAME: _ZN8WongWork10CBossTowerC2Ev
// DEMANGLED: WongWork::CBossTower::CBossTower(void)
CBossTower::CBossTower() : CBossStage(), entrance_log_() {
    CBossStage::reset();
    pool_index_ = 0;
    state_08_ = 0;
    std::memset(member_alive_, 0, sizeof(member_alive_));
    dungeon_ = 0;
    party_ = 0;
}

// IDA_EA: 0x8142A74
// IDA_NAME: _ZN8WongWork10CBossTowerD2Ev
// DEMANGLED: WongWork::CBossTower::~CBossTower()
CBossTower::~CBossTower() {
    CBossStage::reset();
    pool_index_ = 0;
    state_08_ = 0;
    std::memset(member_alive_, 0, sizeof(member_alive_));
    dungeon_ = 0;
    party_ = 0;
}

}  // namespace WongWork
