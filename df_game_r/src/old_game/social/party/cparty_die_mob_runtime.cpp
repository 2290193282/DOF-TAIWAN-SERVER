#include "old_game/social/party/party.h"
#include "old_game/dungeon/instance/instant_reinforce.h"
#include "old_game/dungeon/instance/time_spiral_runtime.h"
#include "old_data/PVF/etc/time_spiral_script.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/escort/golden_commerce.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/blue_marble_runtime.h"
#include "old_game/activity/schedule/exp_keeping_event.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/system_time.h"
#include "infra/log/format/log_manager.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/social/party/hellparty_characlink_abi.h"
#include "old_game/economy/inventory/item_generator_mgr_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/dungeon/ai/kill_monster_info_runtime.h"
#include "old_game/dungeon/instance/tod_layer.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/equipment_growth_runtime.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

void FillKillRewardContext(CParty& party, STKillMonsterData& data) {
    int money_ratio_total = 0;
    int money_ratio_count = 0;
    int minimum_drop_rate = 100;
    ServiceRestrictManager* const restrictions =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    const bool restricted = restrictions &&
        restrictions->isRestricted(RestrictCategory::Security, 1) != 0;

    for (int seat = 0; seat <= 3; ++seat) {
        if (party.checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = party.get_user(seat);
        if (!member) {
            continue;
        }

        const int money_limit = G_CDataManager()->GetMoneyLimitPerLevel(
            member->get_charac_level(), member->get_acc_name());
        if (money_limit > 0) {
            money_ratio_total +=
                100 * member->getCurCharacMoney() / money_limit;
            ++money_ratio_count;
        }
        if (restricted) {
            minimum_drop_rate = std::min(minimum_drop_rate,
                                         member->getStdDropRate());
        }
    }

    data.money_ratio = money_ratio_count
                           ? money_ratio_total / money_ratio_count
                           : 0;
    data.standard_drop_rate = minimum_drop_rate;
}

float GetMonsterExpPartyWeight(CParty& party, int member_count) {
    // CParty::kill_monster reads the same member-count table as its clear
    // reward path, then adds the type-24 repeat-event rate.  Keeping this in
    // the typed ServerParameterScript owner avoids recreating the original
    // CDataManager byte-offset expression in CMD042.
    float weight = 1.0f;
    ServerParameterScript* const parameters =
        G_CDataManager()->get_server_parameter_script();
    if (parameters) {
        weight = parameters->GetClearDungeonPartyExpWeight(member_count);
    }
    CEventBase* const event = GlobalData::s_event_manager
                                  ? GlobalData::s_event_manager->GetRepeatEvent(24)
                                  : 0;
    if (event) {
        weight += static_cast<float>(
            static_cast<CPartyExpBonusEvent*>(event)->getBonusRate(
                member_count, party.isHelpAbuseParty() != 0));
    }
    return weight;
}

}  // namespace

// IDA_EA: 0x85BA762
// IDA_NAME: _ZN6CParty18setMonsterMoveHackEv
bool CParty::setMonsterMoveHack() {
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = get_user(seat);
        if (member) {
            member->getHackAnalyzer()->addServerHackCnt(
                member,
                static_cast<WongWork::ENUM_HACKTYPE>(0xd3),
                1u,
                0u,
                0u);
        }
    }
    return true;
}

// IDA_EA: DisPatcher_DieMob::dispatch_sig @ 0x81EB0C4
void CParty::recordDieMobReport(unsigned short monster_unique_id) {
    // The original deliberately stops growing this abuse-observation map
    // after 200 keys.  Existing keys are still counted while the map is at
    // that bound, matching the find/increment-or-insert sequence.
    if (map_int_int_.size() > 0xc7u) {
        return;
    }

    std::map<int, int>::iterator report =
        map_int_int_.find(static_cast<int>(monster_unique_id));
    if (report != map_int_int_.end()) {
        ++report->second;
        return;
    }
    map_int_int_.insert(std::make_pair(static_cast<int>(monster_unique_id), 1));
}

// IDA_EA: 0x85B6540
// IDA_NAME: _ZN6CParty20DestroyPassiveObjectEi
int CParty::DestroyPassiveObject(int object_unique_id) {
    const bool trace_time_spiral =
        object_unique_id == time_spiral::GetTrapPassiveObjectId() ||
        battle_field_.get_dungeon_index() == time_spiral::kDungeonIndex;
    const int dungeon_index = battle_field_.get_dungeon_index();
    const int map_index = battle_field_.GetCurrentMapIndex();
    const int pos_x = battle_field_.GetCurPosX();
    const int pos_y = battle_field_.GetCurPosY();
    const int destroyed = battle_field_.DestroyPassiveObject(object_unique_id);
    int clear_condition = -1;
    int armed = -1;
    if (destroyed) {
        clear_condition =
            battle_field_.ClearCondition(0, object_unique_id) ? 1 : 0;
        if (clear_condition != 0) {
            ClearDungeon();
        } else {
            // Arm only after the inherited current-map membership authority
            // accepted the static script id.  GET_ITEM is not an owner.
            armed = time_spiral::ArmAfterTrapDestroyed(
                        this, object_unique_id) ? 1 : 0;
        }
    }
    if (trace_time_spiral) {
        LogManager::logFormat(
            1,
            "time_spiral_diag.cpp",
            "int CParty::DestroyPassiveObject(int)",
            0,
            "[TIME_SPIRAL][DESTROY] party=%d dungeon=%d map=%d pos=%d,%d object=%d accepted=%d clear_condition=%d armed=%d",
            GetPartyIndex(),
            dungeon_index,
            map_index,
            pos_x,
            pos_y,
            object_unique_id,
            destroyed,
            clear_condition,
            armed);
    }
    return destroyed;
}

// IDA_EA: 0x85B5A4C
// IDA_NAME: _ZN6CParty15OnKilledMonsterEP5CUseri11map_monsterRK15MSG_MONSTER_DIE
// DEMANGLED: CParty::OnKilledMonster(CUser *,int,map_monster,
//                                    MSG_MONSTER_DIE const&)
int CParty::OnKilledMonster(CUser* user,
                            int monster_id,
                            map_monster monster,
                            const MSG_MONSTER_DIE& message) {
    const bool ordinary_monster = monster.monster_kind_ <= 3;
    const bool map_not_yet_complete = battle_field_.IsClearDungeon() != 1;

    if (map_not_yet_complete && message.field_a22_ == 0 &&
        trace_mob_die_hack_.addMonsterDamage(
            static_cast<int>(message.field_a1a_), message.monster_type_,
            monster_id) > 2 && user) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0xc9), 1, 0, 0);
    }

    const int dungeon_index = battle_field_.get_dungeon_index();
    if (user && ordinary_monster && message.field_a22_ == 0 &&
        static_cast<uint32_t>(monster.active_static_info_.getHpMax()) >
            message.field_a1a_ &&
        monster_id != 10 && monster_id != 50001 && monster_id != 1034 &&
        dungeon_index != 62 && dungeon_index != 63 && dungeon_index != 64) {
        CParty* const killer_party = user->GetParty();
        for (int seat = 0; seat <= 3; ++seat) {
            if (checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = get_user(seat);
            CParty* const member_party = member ? member->GetParty() : 0;
            if (!member || !member_party || !killer_party) {
                continue;
            }

            CDungeon* const member_dungeon =
                member_party->battle_field().GetDungeon();
            CDungeon* const killer_dungeon =
                killer_party->battle_field().GetDungeon();
            const bool special_dungeon =
                (member_dungeon && member_dungeon->get_index() == 11006) ||
                (killer_dungeon && killer_dungeon->get_index() == 11006);
            const WongWork::ENUM_HACKTYPE hack_type =
                member == user ? static_cast<WongWork::ENUM_HACKTYPE>(0x70)
                               : static_cast<WongWork::ENUM_HACKTYPE>(0x71);
            member->getHackAnalyzer()->addServerHackCnt(
                member, hack_type, 1, special_dungeon ? 2u : 0u, 0u);
        }
    }

    MapInfo* const current_map = battle_field_.GetCurrentMapInfo();
    if (user && ordinary_monster && message.field_a22_ == 0 && current_map &&
        current_map->normal_kill_check_.getUseSkillMaterialTime() == 0) {
        const uint32_t damage = message.field_a1a_;
        const uint32_t hp = static_cast<uint32_t>(
            monster.active_static_info_.getHpMax());
        int severity = 0;
        if (4u * hp < damage) {
            severity = 3;
        } else if (3u * hp < damage) {
            severity = 2;
        } else if (2u * hp < damage) {
            severity = 1;
        }
        if (severity > 0) {
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<WongWork::ENUM_HACKTYPE>(0x86), severity,
                0, 0);
        }
    }

    if (battle_field_.check_grid_clear() == 1) {
        battle_field_.onClearMap(true);
        dual_raid::OnPartyMapClear(
            this,
            static_cast<unsigned int>(battle_data_.map_move_count()),
            true);
    }
    return 1;
}

// IDA_EA: 0x85BAE6E
// IDA_NAME: _ZN6CParty25MemberRegisterKillMonsterER11map_monsterh
// DEMANGLED: CParty::MemberRegisterKillMonster(map_monster &,unsigned char)
char CParty::MemberRegisterKillMonster(map_monster& monster,
                                       unsigned char dungeon_difficulty) {
    // The old implementation deliberately skips seat zero: its progress
    // belongs to the direct killer path.  This party-side pass persists the
    // shared boss/APC/named-monster history for the remaining three members.
    if (monster.monster_kind_ == 3) {
        for (int seat = 1; seat <= 3; ++seat) {
            if (checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = get_user(seat);
            CKillMonsterInfo* const kill_info =
                static_cast<CKillMonsterInfo*>(member->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(12)));
            if (!kill_info->CheckBossKill(monster.monster_id_,
                                          dungeon_difficulty)) {
                kill_info->RegisterBossKill(monster.monster_id_,
                                            dungeon_difficulty);
            }
        }
        return 1;
    }

    if (monster.monster_kind_ == 8) {
        for (int seat = 1; seat <= 3; ++seat) {
            if (checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = get_user(seat);
            CKillMonsterInfo* const kill_info =
                static_cast<CKillMonsterInfo*>(member->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(12)));
            if (!kill_info->CheckAPCBossKill(monster.monster_id_,
                                             dungeon_difficulty)) {
                kill_info->RegisterAPCBossKill(monster.monster_id_,
                                               dungeon_difficulty);
            }
        }
        return 1;
    }

    if (!monster.isNamedMonster()) {
        return 0;
    }

    for (int seat = 1; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = get_user(seat);
        CKillMonsterInfo* const kill_info =
            static_cast<CKillMonsterInfo*>(member->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(12)));
        if (!kill_info->CheckNamedKill(monster.monster_id_,
                                       dungeon_difficulty)) {
            kill_info->RegisterNamedKill(monster.monster_id_,
                                         dungeon_difficulty);
        }
    }
    return 1;
}

// IDA_EA: 0x85A27E8
// IDA_NAME: _ZN6CParty12kill_monsterEP5CUseritRK15MSG_MONSTER_DIEPi
int CParty::kill_monster(CUser* user,
                         int monster_unique_id,
                         unsigned short monster_type,
                         const MSG_MONSTER_DIE& message,
                         int* result) {
    const int member_count = std::max(1, get_member_count());
    STKillMonsterData kill_data;
    kill_data.monster_unique_id =
        static_cast<unsigned short>(monster_unique_id);
    kill_data.monster_type = monster_type;
    kill_data.party_member_count =
        static_cast<uint8_t>(member_count);
    kill_data.reserved_05_07[0] = 0;
    kill_data.reserved_05_07[1] = 0;
    kill_data.reserved_05_07[2] = 0;
    kill_data.money_ratio = 0;
    kill_data.standard_drop_rate = 100;
    kill_data.suppress_random_drop = message.field_a22_;
    kill_data.mark_monster_completed = message.field_a23_;
    kill_data.reserved_12_13[0] = 0;
    kill_data.reserved_12_13[1] = 0;
    kill_data.random_drop_scale = 1.0f;

    // The original calculates a party-wide money ratio and, only under the
    // security restriction, chooses the lowest member drop rate before the
    // battlefield generates map items.
    FillKillRewardContext(*this, kill_data);
    QuickParty::RewardData_Param drop_reward_parameter(this);
    QuickParty::RewardData_Result drop_reward;
    set_random_reward(static_cast<QuickParty::RandomBuffType>(3),
                      drop_reward_parameter,
                      drop_reward);
    kill_data.random_drop_scale = drop_reward.raw_14;

    PacketGuard packet;
    map_monster killed_monster;
    unsigned int gained_exp = 0;
    if (battle_field_.kill_monster(packet,
                                   killed_monster,
                                   gained_exp,
                                   kill_data) != 1) {
        return 0;
    }

    if (result) {
        *result = killed_monster.monster_id_;
    }

    OnKilledMonster(user, killed_monster.monster_id_, killed_monster, message);
    champion_break::OnMonsterKilled(
        this, static_cast<int>(killed_monster.monster_kind_));

    // IDA 0x85A2D4A..0x85A2DFD: successful kills are classified before the
    // trace/reward paths.  These exact BattleData dwords are later consumed
    // by clear EXP and card-gold generation.
    const int converted_monster_role =
        battle_field_.convertAPCRoleType2MonsterRoleType(
            static_cast<char>(killed_monster.monster_kind_));
    if (converted_monster_role == 1) {
        ++battle_data_.monster_role_1_kill_count_;
    } else if (converted_monster_role == 3) {
        ++battle_data_.monster_role_3_kill_count_;
    } else {
        ++battle_data_.monster_role_other_kill_count_;
        ++battle_data_.monster_drop_limit_candidate_count_;

        int maximum_monster_count =
            G_CDataManager()->get_server_parameter_script()
                ->get_max_monster_drop_per_map();
        CMap* const current_map = battle_field_.GetCurrentMap();
        if (current_map && current_map->max_monster_count_exception_ != 0) {
            maximum_monster_count = current_map->max_monster_count_exception_;
        }
        if (maximum_monster_count == 0 ||
            battle_data_.monster_drop_limit_candidate_count_ <=
                maximum_monster_count) {
            ++battle_data_.monster_drop_limit_accepted_count_;
        }
    }

    // The original trace is deliberately owned by CParty: it samples the
    // manager when a grid becomes empty, then asks every valid member to
    // refresh the human-certification gate.
    CUser* const manager = getManager();
    if (manager) {
        trace_mob_die_hack_.setMobKiller(static_cast<int>(monster_type));
        if (battle_field_.check_grid_clear() == 1) {
            CUser* trace_users[4] = {manager, 0, 0, 0};
            trace_mob_die_hack_.traceEnd(manager, trace_users);
            for (int seat = 0; seat <= 3; ++seat) {
                if (checkValidUser(seat) == 1) {
                    CUser* const member = get_user(seat);
                    if (member) {
                        member->reqHumanCertify4ClearMap(true);
                    }
                }
            }
        }
    }

    if (converted_monster_role == 3) {
        battle_field_.dungeon_clear_tracer().IncrementKilledBossCount();
    }
    battle_field_.dungeon_clear_tracer().IncrementKilledMonsterCount();

    const int dungeon_index = battle_field_.get_dungeon_index();
    const int dungeon_difficulty = battle_field_.get_dungeon_diff();
    CDungeon* const dungeon = battle_field_.GetDungeon();
    const int circulation_class = dungeon->circulation_statistic_class_;
    const bool blood_dungeon = dungeon->blood_dungeon_type_ != 0;
    const uint16_t village_monster_uid =
        battle_field_.village_monster_timer_token();
    const int member_level_gap = getMemberLevelGap();
    const float party_exp_weight =
        GetMonsterExpPartyWeight(*this, member_count);
    const float dungeon_exp_weight =
        static_cast<float>(dungeon->get_exp_weight());
    const unsigned int monster_total_exp = static_cast<unsigned int>(
        getMonsterTotalExp(gained_exp,
                           static_cast<char>(killed_monster.monster_kind_),
                           dungeon_difficulty,
                           dungeon_exp_weight,
                           party_exp_weight,
                           killed_monster.isNamedMonster()));

    LogManager::logFormat(
        1,
        "dungeon42_diag.cpp",
        "bool CParty::kill_monster(CUser*, int, short unsigned int, const MSG_MONSTER_DIE&, int*)",
        0,
        "[DUNGEON42][EXP_INPUT] dungeon=%d difficulty=%d monster_uid=%d monster_index=%d monster_level=%u monster_kind=%d gained_base=%u monster_total=%u party_weight=%.6f dungeon_weight=%.6f member_count=%d circulation=%d village_uid=%u blood=%u",
        dungeon_index,
        dungeon_difficulty,
        monster_unique_id,
        killed_monster.monster_id_,
        static_cast<unsigned int>(killed_monster.monster_level_),
        static_cast<int>(killed_monster.monster_kind_),
        gained_exp,
        monster_total_exp,
        static_cast<double>(party_exp_weight),
        static_cast<double>(dungeon_exp_weight),
        member_count,
        circulation_class,
        static_cast<unsigned int>(village_monster_uid),
        blood_dungeon ? 1u : 0u);

    // IDA 0x85A2F8D..0x85A2FEB: quest progress is limited to members who
    // actually entered this map and is disabled for instance-dungeon mode.
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1 ||
            battle_data_.member_enter_map(seat) != 1u ||
            battle_field_.instance_dungeon_mode() == 1) {
            continue;
        }

        CUser* const member = get_user(seat);
        if (!member) {
            continue;
        }

        const ENUM_QUEST_ENEMY_TYPE enemy_type =
            killed_monster.monster_kind_ > 4 &&
                    killed_monster.monster_kind_ <= 8
                ? static_cast<ENUM_QUEST_ENEMY_TYPE>(2)
                : static_cast<ENUM_QUEST_ENEMY_TYPE>(1);
        member->CheckQuestMonster(dungeon_index,
                                  killed_monster.monster_id_,
                                  enemy_type);
    }

    // RECONSTRUCTED_0828: equipment_grow.etc names `death` as its progress
    // source.  Award only the same legitimate, non-instance monster deaths
    // used by the ordinary EXP path, and only to members present in this map.
    if (message.field_a22_ == 0 &&
        battle_field_.instance_dungeon_mode() != 1) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (checkValidUser(seat) != 1 ||
                battle_data_.member_enter_map(seat) != 1u) {
                continue;
            }
            CUser* const member = get_user(seat);
            if (member) {
                equipment_growth_runtime::AwardDeathProgress(
                    member,
                    killed_monster.monster_kind_,
                    killed_monster.monster_level_,
                    dungeon_difficulty);
            }
        }
    }

    // IDA 0x85A300C: this client report byte gates the whole experience loop.
    // Its drop-side test remains the stricter `== 1` in CBattle_Field.
    for (int seat = 0; seat <= 3 && message.field_a22_ == 0; ++seat) {
        if (checkValidUser(seat) != 1 ||
            battle_field_.instance_dungeon_mode() == 1) {
            continue;
        }

        CUser* const member = get_user(seat);
        if (!member) {
            continue;
        }
        if (battle_data_.member_enter_map(seat) != 1u && !blood_dungeon) {
            member->SendZeroExpPacket();
            continue;
        }

        unsigned int member_exp = 0;
        unsigned int member_base_exp = gained_exp;
        const char* exp_branch = "ordinary";
        if (circulation_class != 0 || village_monster_uid != 0) {
            G_CDataManager()->get_mob_reward(member->get_charac_level(),
                                             member_base_exp);
            member_exp = static_cast<unsigned int>(
                getMonsterTotalExp(
                    member_base_exp,
                    static_cast<char>(killed_monster.monster_kind_),
                    dungeon_difficulty,
                    dungeon_exp_weight,
                    party_exp_weight,
                    false) /
                member_count);
            exp_branch = "circulation_or_village";
        } else if (dungeon_index == 100) {
            G_CDataManager()->get_mob_reward(member->get_charac_level(),
                                             member_base_exp);
            member_exp = static_cast<unsigned int>(
                getMonsterTotalExp(
                    member_base_exp,
                    static_cast<char>(killed_monster.monster_kind_),
                    dungeon_difficulty,
                    dungeon_exp_weight,
                    party_exp_weight,
                    false) /
                member_count);
            exp_branch = "dungeon_100";
        } else {
            // Risk/Requiem dungeons intentionally use a basis level above
            // their entry range.  The clear-reward path already bypasses
            // BaseExpPenalty for this mode; apply the same rule per monster.
            const bool risk_dungeon = dungeon->isRiskDungeon();
            const long double level_penalty = risk_dungeon
                ? 1.0L
                : G_CDataManager()->BaseExpPenalty(
                      member->get_charac_level(), dungeon->get_standard_level());
            member_exp = static_cast<unsigned int>(
                static_cast<long double>(MemberPenaltyRate()) *
                level_penalty *
                static_cast<long double>(monster_total_exp) /
                static_cast<long double>(member_count));
            if (risk_dungeon) {
                exp_branch = "risk_dungeon";
            }
        }

        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "bool CParty::kill_monster(CUser*, int, short unsigned int, const MSG_MONSTER_DIE&, int*)",
            0,
            "[DUNGEON42][EXP_MEMBER] uid=%d seat=%d level=%d branch=%s member_base=%u member_exp=%u entered=%u",
            member->GetUID(),
            seat,
            member->get_charac_level(),
            exp_branch,
            member_base_exp,
            member_exp,
            static_cast<unsigned int>(battle_data_.member_enter_map(seat)));

        QuickParty::RewardData_Param exp_reward_parameter(this);
        QuickParty::RewardData_Result exp_reward;
        exp_reward.raw_00 = static_cast<int>(member_exp);
        set_random_reward(static_cast<QuickParty::RandomBuffType>(0),
                          exp_reward_parameter,
                          exp_reward);
        member_exp = static_cast<unsigned int>(exp_reward.raw_00);
        member_exp = dual_raid::ScaleMonsterExperience(
            this, member, member_exp);
        member_exp = golden_commerce::ScaleMonsterExperience(
            this, member, member_exp);
        STGainedEachExpData gained_each_exp;
        gained_each_exp.gained_exp = member_exp;
        gained_each_exp.exp_scale = party_exp_weight;
        gained_each_exp.member_level_gap = member_level_gap;
        member_exp = static_cast<unsigned int>(
            sumGainedEachExp(member, gained_each_exp));

        if (circulation_class != 1 && village_monster_uid == 0) {
            member->calcHelpAbuseRatio(member_exp,
                                       isHelpAbuseParty() != 0);
        }

        if (blood_dungeon) {
            continue;
        }

        if (member->getMoveSpace() == 1) {
            BlueMarble* const blue_marble = member->getBlueMarble();
            if (blue_marble) {
                const int exp_buff = blue_marble->getBuff(
                    member, BlueMarbleBuffType::kExperience);
                if (exp_buff > 1) {
                    member_exp *= static_cast<unsigned int>(exp_buff);
                }
            }
        }

        int gained_sp = 0;
        int gained_sfp = 0;
        const uint32_t exp_keeping_source_level =
            static_cast<uint32_t>(member->get_charac_level());
        if (member->gain_exp_sp(
                static_cast<int>(member_exp),
                gained_sp,
                gained_sfp,
                static_cast<eExpAddReason>(1),
                killed_monster.monster_id_,
                false)) {
            member->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                                   static_cast<ENUM_NOTIPACKET>(2),
                                   1);
        }

        golden_commerce::RecordMonsterExperienceAwarded(this, member, member_exp);
        exp_keeping::OnMonsterExperienceAwarded(
            member,
            member_exp,
            static_cast<uint32_t>(dungeon_index),
            exp_keeping_source_level,
            exp_keeping::MakeMonsterSourceToken(
                static_cast<uint32_t>(GetPartyIndex()),
                battle_data_.map_start_time(),
                static_cast<uint32_t>(battle_field_.GetCurrentMapIndex()),
                static_cast<uint32_t>(monster_unique_id),
                static_cast<uint32_t>(member->getCurCharacNo())));

        LogManager::logFormat(
            1,
            "dungeon42_diag.cpp",
            "bool CParty::kill_monster(CUser*, int, short unsigned int, const MSG_MONSTER_DIE&, int*)",
            0,
            "[DUNGEON42][EXP_GRANTED] uid=%d seat=%d final_exp=%u gained_sp=%d gained_sfp=%d",
            member->GetUID(),
            seat,
            member_exp,
            gained_sp,
            gained_sfp);
    }

    if (user) {
        user->history_log().KillMonster(
            killed_monster.monster_id_,
            message.field_a1e_,
            message.field_a1a_,
            message.field_a24_,
            message.field_a26_,
            message,
            static_cast<char>(killed_monster.monster_kind_),
            killed_monster.monster_kind_ == 5,
            user->getCurCharacExp(),
            static_cast<int>(monster_total_exp),
            static_cast<eExpAddReason>(1));
    }

    secu_.DieMob(&message, &killed_monster);
    send_to_party(packet);

    // IDA 0x85A347A..0x85A34E1 sends the same CBattle_Field+0x1a8 value
    // through NOTI50 whenever converted monster role 1 dies.  Together with
    // CLIENT_IDA_EVIDENCE sub_590BE0 @ 0x590BE0, this closes the NOTI38 tail
    // byte as KEY_COUNT and preserves the original synchronization path.
    if (converted_monster_role == 1) {
        PacketGuard key_count_packet;
        key_count_packet.put_header(0, ENUM_NOTIPACKET_KEY_COUNT);
        key_count_packet.put_byte(battle_field_.key_count_);
        key_count_packet.finalize(true);
        send_to_party(key_count_packet);
    }

    prepare_dungeon_clear();
    int clear_condition_type = 2;
    if (killed_monster.monster_kind_ == 3) {
        clear_condition_type = 4;
    } else if (killed_monster.monster_kind_ >= 5 &&
               killed_monster.monster_kind_ <= 8) {
        clear_condition_type = 3;
    }
    if (battle_field_.ClearCondition(clear_condition_type,
                                     killed_monster.monster_id_)) {
        ClearDungeon();
    }
    return 1;
}

// IDA_EA: 0x85A9330
// IDA_NAME: _ZN6CParty12ClearDungeonEv
// DEMANGLED: CParty::ClearDungeon(void)
void CParty::ClearDungeon() {
    time_spiral::ResetParty(this);
    instant_reinforce::ResetParty(this);
    if (tournament_victory_ != 0) {
        return;
    }

    const int member_count = get_member_count();
    if (battle_field_.instance_dungeon_mode() == 1) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (checkValidUser(seat) == 1) {
                CUser* const member = get_user(seat);
                if (member) {
                    OnSuccessedDungeonClear(member);
                }
            }
        }
        SetEPLPState(2);
        const unsigned int timer_key = static_cast<unsigned int>(
            gen_timer_key(TIMER_MESSAGE_EPLP_COMMAND_WAIT));
        G_TimerQueue()->InsertTimer(1,
                                    GetPartyIndex(),
                                    TIMER_MESSAGE_EPLP_COMMAND_WAIT,
                                    60,
                                    timer_key,
                                    0);
        set_state(1);
        PacketGuard instance_packet;
        instance_packet.put_header(0, 254);
        instance_packet.finalize(true);
        send_to_party(instance_packet);
        battle_field_.dungeon_clear_tracer().Trace(
            "void CParty::ClearDungeon()");
        tournament_victory_ = 1;
        return;
    }

    CDungeon* const dungeon = battle_field_.GetDungeon();
    if (!dungeon) {
        return;
    }
    CSystemTime* const system_time = GlobalData::system_time();
    const uint64_t current_tick = static_cast<uint32_t>(
        system_time ? system_time->getCurTickCount() : 0);
    battle_data_.dungeon_playing_time() =
        current_tick - battle_data_.dungeon_start_tick() -
        battle_data_.dungeon_pause_time();
    const uint64_t elapsed_ms = battle_data_.dungeon_playing_time();
    const bool blood_dungeon = dungeon->get_blood_dungeon_type() != 0;
    const bool tower_of_despair = dungeon->isTowerOfDespairDungeon() != 0;
    const bool tournament_dungeon = dungeon->isTournamentDungeon() != 0;

    // The original normal-dungeon completion clears the exact action-point
    // groups before deciding which clear-reward branch owns the packet.  This
    // is deliberately also performed for blood/tower/tournament completions.
    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = get_user(seat);
        if (!member) {
            continue;
        }
        if (dungeon->village_attack_revenge_dungeon_ != 0) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member, static_cast<APSystem::_ActionGroupIndex>(24), 0,
                ENUM_PACKETCLASS_NOTIPACKET);
        } else if (dungeon->get_index() == 100) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member, static_cast<APSystem::_ActionGroupIndex>(25), 0,
                ENUM_PACKETCLASS_NOTIPACKET);
        } else if (dungeon->ancient_dungeon_ != 0) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member, static_cast<APSystem::_ActionGroupIndex>(27), 0,
                ENUM_PACKETCLASS_NOTIPACKET);
        } else if (dungeon->get_dimension_possible() > 0) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member, static_cast<APSystem::_ActionGroupIndex>(28), 0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
        if (member->IsUsedCoinInDungeon() != 1) {
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member, static_cast<APSystem::_ActionGroupIndex>(15), 0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
    }

    PacketGuard clear_packet;
    if (!blood_dungeon && !tower_of_despair && !tournament_dungeon) {
        clear_packet.put_header(0, 31);
        clear_packet.finalize(true);
    } else if (blood_dungeon) {
        battle_field_.blood_clear_reward().onFinishBloodRound(
            true,
            static_cast<short>(battle_field_.blood_round().getBloodRoundOrder()),
            this,
            dungeon);
        const unsigned int timer_key = static_cast<unsigned int>(gen_timer_key(
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0));
        G_TimerQueue()->InsertTimer(
            1,
            GetPartyIndex(),
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0,
            5,
            timer_key,
            TIMER_MESSAGE_BLOOD_DUNGEON_CLEAR_REWARD_0);
    } else if (tower_of_despair) {
        // IDA 0x85A980A..0x85A9E9D: Tower-of-Despair completion is a
        // manager-owned single-character reward transaction, not the normal
        // dungeon-clear packet.  Keep its state, generated items and timer
        // with the CParty/CBattle_Field owners.
        tournament_victory_ = 1;
        CUser* const manager = getManager();
        TOD_UserState* const tod_state = manager
            ? static_cast<TOD_UserState*>(manager->GetCharacExpandData(
                  static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)))
            : 0;
        if (tod_state) {
            tod_state->ClearLayer();
            const TOD_Layer layer = tod_state->getLastEnterLayer();
            const unsigned int layer_number = layer.GetLayer();
            CBattle_Field::CTowerOfDespairRewardData& tower_reward =
                battle_field_.tower_reward();
            tower_reward.period_reward = std::make_pair(-1, 0);

            int reward_roll_count = 5;
            TowerOfDespairMgr* const tower_manager =
                GlobalData::s_pTowerOfDespairMgr;
            if (tower_manager &&
                tower_manager->Get_APCIndexByLayer(layer_number) != -1) {
                const char* const apc_name =
                    tower_manager->Get_APCNameByLayer(layer);
                if (apc_name && *apc_name) {
                    PacketGuard notice;
                    notice.put_header(0, 86);
                    notice.put_byte(8);
                    notice.put_short(static_cast<short>(layer_number));
                    notice.put_int(static_cast<int>(std::strlen(apc_name)));
                    notice.put_str(apc_name, std::strlen(apc_name));

                    const char* const manager_name =
                        manager->getCurCharacName();
                    if (manager_name) {
                        notice.put_int(
                            static_cast<int>(std::strlen(manager_name)));
                        notice.put_str(manager_name,
                                       std::strlen(manager_name));
                    } else {
                        // Preserve the original fallback length/payload pair.
                        notice.put_int(5);
                        notice.put_str("Neople", 6);
                    }
                    notice.finalize(true);
                    G_GameWorld()->send_all(notice);
                }
                reward_roll_count = 9;
                tower_reward.period_reward = std::make_pair(1252, 1);
            }
            if (layer_number == 100u) {
                tower_reward.period_reward = std::make_pair(3314, 1);
            }

            WongWork::stGenerateRefData_t reference;
            // 0627 IDA 0x85A9B5B..0x85A9B8D writes these values into the
            // clear-reward union at stGenerateRefData_t +0x34, not into the
            // two top-level rate fields.  In particular, setting the
            // top-level clear_reward_scale to 1.0 makes the generator's
            // (1.0 - scale) factor zero and leaves every TOD reward slot
            // empty.  Keep the constructor defaults for those top-level
            // fields and restore the original union inputs.
            reference.generator_data.clear_reward
                .character_level_or_world_area = 70;  // Original "F" byte.
            reference.generator_data.clear_reward
                .dungeon_difficulty_or_world_index = 0;
            reference.generator_data.clear_reward.item_probability_scale =
                1.0f;
            reference.generator_data.clear_reward
                .clear_reward_drop_probability = 0;
            reference.generator_data.clear_reward.dungeon = dungeon;
            WongWork::stGenerateResult_t generated;
            WongWork::CItemGenerator* const generator =
                G_CDataManager()->getItemGenerator()->getGenerator(
                    WongWork::CItemGeneratorMgr::kClearReward);
            if (generator) {
                for (int roll = 0; roll < reward_roll_count; ++roll) {
                    generator->generateRandomItem(reference, generated);
                }
            }

            std::vector<std::pair<int, int> > reward_items;
            reward_items.reserve(generated.items.size() + 1u);
            for (std::vector<Inven_Item>::const_iterator item =
                     generated.items.begin();
                 item != generated.items.end();
                 ++item) {
                CItem* const item_data =
                    G_CDataManager()->find_item(item->item_id_);
                const int item_count = item_data && item_data->is_stackable()
                    ? item->get_add_info()
                    : 1;
                reward_items.push_back(
                    std::make_pair(item->item_id_, item_count));
            }

            // The delayed TOD packet reads the generated set only; the
            // period item is appended only to the immediate inventory grant.
            tower_reward.today_reward_items = reward_items;
            if (tower_reward.period_reward.first != -1) {
                reward_items.push_back(tower_reward.period_reward);
            }

            std::vector<int> inserted_slots;
            const int inserted = manager->getCurCharacInvenW()->insert_event_items(
                reward_items,
                inserted_slots,
                static_cast<eMoneyAddReason>(31),
                static_cast<eItemAddReason>(55),
                true,
                "game_server_msg_191",
                "game_server_msg_192");
            if (inserted > 0 && inserted_slots.size() > 2u) {
                for (int item_index = 0, slot_index = 0;
                     item_index < inserted &&
                         static_cast<size_t>(slot_index) <
                             inserted_slots.size();
                     ++item_index, slot_index += 3) {
                    manager->SendUpdateItem(
                        static_cast<CUser::eSendTarget>(1),
                        static_cast<ENUM_ITEMSPACE>(0),
                        inserted_slots[slot_index]);
                }
            }

            const unsigned int timer_key = static_cast<unsigned int>(
                gen_timer_key(TIMER_MESSAGE_TOD_SEND_CLEAR_REWARD));
            G_TimerQueue()->InsertTimer(1,
                                        GetPartyIndex(),
                                        TIMER_MESSAGE_TOD_SEND_CLEAR_REWARD,
                                        2,
                                        timer_key,
                                        TIMER_MESSAGE_TOD_SEND_CLEAR_REWARD);

            Packet_TowerOfDespair_Statistic_GTS statistic;
            statistic.key.raw_00 = static_cast<int>(layer_number);
            statistic.byte_12 = 0;
            CStatisticServerProxy* const proxy =
                GlobalData::s_statistic_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(0));
            proxy->SendPacket(reinterpret_cast<char*>(&statistic),
                              sizeof(statistic));
        }
    } else if (tournament_dungeon) {
        // The raw order is observable: latch first, then reset/create the
        // reward state.  The common quest and quick-party tail still runs.
        tournament_victory_ = 1;
        CTournamentDungeonReward& reward = battle_field_.tournament_reward();
        reward.ResetReward();
        MakeTournamentReward(reward);
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = get_user(seat);
        if (!member) {
            continue;
        }
        UserQuest* const quest = member->getCurCharacQuestW();
        if (quest) {
            quest->set_authen_data(static_cast<QUEST_CONDITION>(4),
                                   member_count,
                                   0);
            quest->set_authen_data(static_cast<QUEST_CONDITION>(2),
                                   static_cast<int>(elapsed_ms / 1000u),
                                   0);
        }
        if (!blood_dungeon && !tower_of_despair && !tournament_dungeon) {
            member->Send(clear_packet);
        }
        if (secret_shop_data_.IsOpen() != 0 &&
            GlobalData::s_secret_shop != 0) {
            GlobalData::s_secret_shop->LotteryItems(
                secret_shop_data_.infos[seat].retailer,
                secret_shop_data_.state,
                dungeon->get_index(),
                dungeon->get_standard_level());
            SendSecretShopItemList(member);
            APSystem::CUserProc::ClearActionAndSendtoUser(
                member,
                static_cast<APSystem::_ActionGroupIndex>(13),
                0,
                ENUM_PACKETCLASS_NOTIPACKET);
        }
        if (member->getBreakAwayDungeonClearCnt() > 0u) {
            member->subBreakAwayDungeonClearCnt();
        }
    }

    const int completion_member_count = get_member_count();
    if (completion_member_count == 1) {
        stHellParty_t* const hell_party =
            G_CDataManager()->get_hellparty_script_values();
        if (hell_party) {
            battle_data_.SetHellPartyValueTotal(hell_party->base_value);
        }
    } else if (!battle_field_.skip_hell_party_clear_accumulation()) {
        cal_hellparty_value(battle_field_.get_dungeon_diff(),
                            get_rate_level_difference(),
                            completion_member_count - 1);
    }

    battle_field_.dungeon_clear_tracer().Trace("void CParty::ClearDungeon()");
    tournament_victory_ = 1;
    QuickParty::RewardData_Param reward_parameter(this);
    QuickParty::RewardData_Result reward;
    set_random_reward(static_cast<QuickParty::RandomBuffType>(10),
                      reward_parameter,
                      reward);
}

// IDA_EA: 0x85AA39A
// IDA_NAME: _ZN6CParty21prepare_dungeon_clearEv
// DEMANGLED: CParty::prepare_dungeon_clear(void)
void CParty::prepare_dungeon_clear() {
    // IDA performs these two trace writes before it tests grid completion;
    // failed/intermediate maps must remain observable by the clear tracer.
    battle_field_.dungeon_clear_tracer().SetClearedDungeon(
        tournament_victory_ != 0);
    battle_field_.dungeon_clear_tracer().SetEndPoint(
        battle_field_.check_end_point());

    if (battle_field_.check_grid_clear() != 1) {
        return;
    }

    if (battle_field_.instance_dungeon_mode() != 1) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (checkValidUser(seat) != 1) {
                continue;
            }
            CUser* const member = get_user(seat);
            if (!member) {
                continue;
            }
            UserQuest* const quest = member->getCurCharacQuestW();
            if (quest) {
                quest->set_authen_data(static_cast<QUEST_CONDITION>(1),
                                       battle_field_.get_cur_map(),
                                       0);
            }

            const int pending_creature_exp = member->takePendingCreatureExp();
            if (pending_creature_exp <= 0) {
                continue;
            }
            const user_creature::CCreatureMgr* const read_creatures =
                member->getCurCharacInvenR()->GetCreatureMgrR();
            if (read_creatures &&
                !read_creatures->IsGrowCreature_Equipped_Creature()) {
                user_creature::CCreatureMgr* const write_creatures =
                    member->getCurCharacInvenW()->GetCreatureMgrW();
                if (write_creatures) {
                    write_creatures->GainExp(pending_creature_exp);
                }
            }
        }
    }

    if (battle_field_.ClearCondition(1, battle_field_.GetCurrentMapIndex()) ||
        battle_field_.check_end_point()) {
        ClearDungeon();
    }
}
