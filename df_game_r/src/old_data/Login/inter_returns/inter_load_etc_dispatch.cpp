#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"

#include <cstring>
#include <string>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/pvp/battlefield/pvp_skill_tree_runtime.h"
#include "old_game/activity/reward/reward_item_runtime.h"
#include "common/manager/shared_server_message_abi.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"
#include "old_data/channel/arad_event_notification_runtime.h"
#include "old_data/channel/arad_server_state.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/dungeon/db_load_blood_best_record.h"
#include "old_data/db/dungeon/db_load_dimension.h"
#include "old_data/db/event/db_event_item_and_levelup_dispatchers.h"
#include "old_data/db/event/db_select_mobile_auth_reward_tw.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/activity/schedule/inter_server_event_start.h"
#include "old_game/activity/schedule/kiri_cargo_packet_dispatchers.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"
#include "old_game/economy/mall/crane_minigame_runtime.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_tree_param_runtime.h"
#include "old_game/player/core/achievement_runtime.h"
#include "old_game/player/core/cbooster_gage_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/charac_expand_cdata.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace {

struct LoadedSkillTreeSummary {
    unsigned int indexed_slots;
    unsigned int positive_levels;
    unsigned int level_sum;
};

LoadedSkillTreeSummary SummarizeLoadedSkillTree(
    const _Mastered_skill* tree,
    unsigned int count) {
    LoadedSkillTreeSummary result = {0u, 0u, 0u};
    if (!tree) return result;
    for (unsigned int index = 0; index < count; ++index) {
        if (!tree[index].skill_index) continue;
        ++result.indexed_slots;
        const unsigned int level =
            static_cast<unsigned char>(tree[index].skill_level);
        if (level) ++result.positive_levels;
        result.level_sum += level;
    }
    return result;
}

void LogSkillLoadSummary(const CUser* user,
                         const char* phase,
                         const _Mastered_skill* first_tree,
                         const _Mastered_skill* second_tree,
                         const SIG_LOAD_SKILL& skill) {
    if (!user) return;
    const LoadedSkillTreeSummary first =
        SummarizeLoadedSkillTree(first_tree, 204u);
    const LoadedSkillTreeSummary second =
        SummarizeLoadedSkillTree(second_tree, 204u);
    LogManager::logFormat(
        1, "skill_persistence_diag.cpp", "Inter_LoadEtc::dispatch_sig", 0,
        "[SKILL_PERSIST][LOAD] phase=%s job=%d raw_grow=%d first_grow=%d "
        "second_grow=%d selected_tree=%d pvp_tree_channel=%d table=%u "
        "script_version=%u tree0_slots=%u tree0_positive=%u tree0_levels=%u "
        "tree1_slots=%u tree1_positive=%u tree1_levels=%u "
        "sp0=%u sp1=%u sfp0=%u sfp1=%u",
        phase ? phase : "unknown", user->get_charac_job(),
        user->getCurCharacGrowType(), user->getCurCharFirstGrowType(),
        static_cast<int>(user->getCurCharSecondGrowType()),
        user->GetCurCharacSkillTreeIndex(),
        G_GameWorld()->IsPvPSkilTreeChannel() ? 1 : 0,
        static_cast<unsigned int>(skill.skill_table_index),
        static_cast<unsigned int>(skill.script_version),
        first.indexed_slots, first.positive_levels, first.level_sum,
        second.indexed_slots, second.positive_levels, second.level_sum,
        skill.remain_sp, skill.remain_sp_second,
        skill.remain_sfp_first, skill.remain_sfp_second);
}

void LogLiveSkillLoadSummary(const CUser* user,
                             const char* phase,
                             const SIG_LOAD_SKILL& skill) {
    if (!user || !user->getCurCharacSkillR()) return;
    const SkillSlot* const slots = user->getCurCharacSkillR();
    LogSkillLoadSummary(
        user, phase,
        slots->get_skillslot_buf(ENUM_SKILL_TREE_0),
        slots->get_skillslot_buf(ENUM_SKILL_TREE_1), skill);
}

}  // namespace

// IDA_EA: 0x84C0264
// IDA_NAME: _ZN13Inter_LoadEtc12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadEtc::dispatch_sig(CUser *,char *,int)
//
// This is the slot-32 load-completion state machine.  Its DB payload is
// already a typed SIG_LOAD_ETC record at this dispatcher boundary; all fields
// below retain the original ordering of load, validation, world insertion,
// notification, and deferred DB work.
int Inter_LoadEtc::dispatch_sig(CUser* user,
                                char* raw_payload,
                                int /*payload_size*/) {
    if (!user->get_state()) {
        return 2683;
    }
    if (!user->getCurCharacR()) {
        return 2687;
    }

    SIG_LOAD_ETC* const payload = reinterpret_cast<SIG_LOAD_ETC*>(raw_payload);
    LogManager::logFormat(
        1,
        "Inter_LoadEtc.cpp",
        "Inter_LoadEtc::dispatch_sig",
        2700,
        "[CHARVIEW][ETC_ENTER] state=%d lock=%d section1=%d section11=%d result=%d",
        user->get_state(),
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0,
        payload->result);
    if (payload->result == 1) {
        return 2710;
    }
    if (!user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1))) {
        // The reconstructed signal-8 path crosses a native SQL006 queue after
        // DB_Q.  Refuse any stale/out-of-order ETC reply before it can insert
        // the user into the world or unlock saves over preview-only state.
        LogManager::logFormat(
            1,
            "Inter_LoadEtc.cpp",
            "virtual int Inter_LoadEtc::dispatch_sig(CUser*,char*,int)",
            2711,
            "[NATIVE_0828][ETC_BEFORE_INVENTORY] uid=%d charac_no=%u",
            user->GetUID(),
            static_cast<unsigned int>(payload->charac_no));
        return 2711;
    }

    CDataManager* const data_manager = G_CDataManager();
    if (data_manager->IsAvailablePvPChannel(payload->pvp_result.values[5]) !=
        1) {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_LoadEtc::dispatch_sig(CUser*, char*, int)",
            0xA9B,
            "Inter_LoadEtc::dispatch_sig pvp_grade = %d",
            payload->pvp_result.values[5]);
        return 2716;
    }

    PacketGuard packet;
    if (SetPvpRecord(user, payload) != 1) {
        return 2722;
    }
    if (SetRankRecord(user, payload) != 1) {
        return 2725;
    }

    cMyTrace trace("virtual int Inter_LoadEtc::dispatch_sig(CUser*, char*, int)",
                   2757,
                   0);
    trace("CharLastPlayTick char(%d) t(%d)",
          user->getCurCharacNo(),
          user->getCurCharacLastPlayTick());
    user->ProcPremiumFatigue();
    user->GetPremiumInfo()->RecalcAdditionalInfo(user);
    if (SetFatigue(user) != 1) {
        return 2767;
    }
    verifyCharacVillage(user);
    if (!npc_gift::Load(user, payload)) {
        // NPC favour is an optional 0828 sidecar.  A missing/invalid snapshot
        // must fail that feature closed, not turn a successfully loaded core
        // character into the fatal InterDispatcher return 2788.  Erase also
        // prevents a previous character's cached relationship state from
        // leaking across a character switch.
        npc_gift::Erase(user);
        LogManager::logFormat(
            1,
            "Inter_LoadEtc.cpp",
            "virtual int Inter_LoadEtc::dispatch_sig(CUser*,char*,int)",
            2788,
            "[NATIVE_0828][NPC_GIFT_LOAD_FAIL_CLOSED] uid=%d charac_no=%u",
            user->GetUID(),
            static_cast<unsigned int>(payload->charac_no));
    }
    if (SendResult(user) != 1) {
        return 2790;
    }

    user->SendRestrictedGoods(false, 0);
    user->LoadCoolTimeItemInfo(&payload->cooltime_items.item_info);
    user->LoadEffectItemInfo(&payload->effect_items.item_info);

    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(6))
        ->loadData(user, reinterpret_cast<char*>(&payload->redeem_items));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8))
        ->loadData(user, reinterpret_cast<char*>(&payload->pvp_missions));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13))
        ->loadData(user, reinterpret_cast<char*>(&payload->tod_user_state));

    const int tag_charac_index = user->get_charac_idx(payload->tag_charac_no);
    if (payload->tag_charac_no && tag_charac_index != -1 &&
        user->getTagCharacR()) {
        static_cast<SkillSlot*>(user->getTagCharacSkillW())->set_skill_slot(
            reinterpret_cast<char*>(payload->tag_charac_etc.skill_trees[0]),
            reinterpret_cast<char*>(payload->tag_charac_etc.skill_trees[1]),
            0x198);
    }
    if (tag_charac_index == -1) {
        payload->quest_shop.has_tagged = 0;
    }

    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11))
        ->loadData(user, reinterpret_cast<char*>(&payload->quest_shop));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(10))
        ->loadData(user, reinterpret_cast<char*>(&payload->item_making_skill));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(16))
        ->loadData(user, reinterpret_cast<char*>(&payload->item_dictionary));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(14))
        ->loadData(user, reinterpret_cast<char*>(&payload->title_book));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(15))
        ->loadData(user, reinterpret_cast<char*>(&payload->achievement));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(17))
        ->loadData(user, reinterpret_cast<char*>(&payload->booster_gage));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(18))
        ->loadData(user, reinterpret_cast<char*>(&payload->best_clear_time));
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(19))
        ->loadData(user, payload->charac_inform_notice);

    if (payload->achievement.last_update_time == 0) {
        static_cast<CAchievement*>(
            user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(15)))
            ->adjust();
    }

    // The original history call reads CUser's embedded CNetwork directly.
    // Inter_LoadEtc is a named CUser friend for that exact ABI access.
    char peer_ip[16];
    if (user->network_.get_ip()) {
        std::memset(peer_ip, 0, sizeof(peer_ip));
        user->history_log().Login(user->getWebAddress(),
                                  user->network_.GetPeerIP2(peer_ip,
                                                            sizeof(peer_ip)),
                                  user->network_.get_port(),
                                  G_CEnvironment()->get_channel_no(),
                                  user->GetLogInOutState(),
                                  user->GetServerGroup());
    } else {
        user->history_log().Login(user->getWebAddress(),
                                  user->network_.GetPeerIP2(peer_ip,
                                                            sizeof(peer_ip)),
                                  user->network_.get_port(),
                                  G_CEnvironment()->get_channel_no(),
                                  user->GetLogInOutState(),
                                  user->GetServerGroup());
    }

    user->SetLoginTick(GlobalData::s_systemTime_.getCurSec());
    user->SetCharacStartPlayTick(GlobalData::s_systemTime_.getCurSec());

    CSkillTreeParam skill_tree(user, payload, packet);
    LogSkillLoadSummary(user, "db_payload", payload->skill.first_tree,
                        payload->skill.second_tree, payload->skill);
    bool recreate_pvp_skill_tree = false;
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        PvPSkillTreeParameterScript* const pvp_skill_script =
            data_manager->get_pvp_skill_tree_parameter_script();
        if (payload->skill.script_version != pvp_skill_script->version &&
            user->get_pvp_grade() <
                pvp_skill_script->getInitSkillTreeGrade(
                    pvp_skill_script->version)) {
            std::memset(&payload->skill, 0, sizeof(payload->skill));
            recreate_pvp_skill_tree = true;
        }
    }

    if (user->SetSkill(&payload->skill, &payload->combo_skill) != 1) {
        user->disableSaveSkill();
        return 2961;
    }
    LogLiveSkillLoadSummary(user, "after_set_skill", payload->skill);
    user->disableSaveSkill();

    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        if (!payload->create_pvp_skill_row &&
            payload->fair_pvp.last_play_time != 0) {
            SetPvPSkillTree(user, payload->skill.first_tree, ENUM_SKILL_TREE_0);
            SetPvPSkillTree(user, payload->skill.second_tree,
                            ENUM_SKILL_TREE_1);
        } else {
            user->givePvPSkillTree(0, false, 3);
            user->givePvPSkillTree(0, true, 3);
            const int pvp_skill_point =
                data_manager->get_pvp_skill_tree_parameter_script()
                    ->getPvPSkillPoint(user->get_charac_job(),
                                       user->getCurCharFirstGrowType(),
                                       user->getCurCharSecondGrowType(),
                                       user->get_pvp_grade(),
                                       true);
            user->getCurCharacSkillW()->set_remain_sp_at_index(
                pvp_skill_point, ENUM_SKILL_TREE_0);
            user->getCurCharacSkillW()->set_remain_sp_at_index(
                pvp_skill_point, ENUM_SKILL_TREE_1);
        }
    }

    if (skill_tree.SetSkillTree(this) != 0) {
        return 3020;
    }
    LogLiveSkillLoadSummary(user, "after_skill_tree_reconcile",
                            payload->skill);

    item_lock::CItemLock* const item_lock =
        static_cast<item_lock::CItemLock*>(
            user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(2)));
    CTitleBook* const title_book = static_cast<CTitleBook*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(14)));
    title_book->getItemLockList(item_lock->GetItemLockRefVec());
    item_lock->VerifyItemLock(user);

    if (G_CGameManager()->insert_game_world(user) != 1) {
        return 3092;
    }

    user->sendCharacQp();
    // 角色已经加入世界后，沿用 CUser 的可见状态 owner 下发
    // 转职/觉醒外观显示开关（NOTI446 + NOTI356）。旧版
    // CQuestShop::sendCharacQuestPiece 的 quest_piece u32 不属于
    // 当前客户端的通知布局，不能在这里继续发送。
    user->send_charac_visible_data();
    static_cast<CBoosterGage*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(17)))
        ->send_data(user, 0);
    user->SendItemMakingSkillInfo();
    if (G_GameWorld()->IsPvPSkilTreeChannel() &&
        (payload->create_pvp_skill_row ||
         !payload->fair_pvp.last_play_time || recreate_pvp_skill_tree)) {
        packet.clear();
        packet.put_header(0, 340);
        packet.finalize(true);
        user->Send(packet);
    }

    user->CheckMaxLuckyLevel();
    user->SetCurConditionEventStep(
        static_cast<int16_t>(payload->condition_event_step));
    user->SetCurConditionEventRewardStep(
        static_cast<int16_t>(payload->condition_event_reward_step));
    user->SetCurCharacRevengeData(payload->village_attack_count,
                                  payload->revenge_dungeon_count);
    if (static_cast<int16_t>(payload->proper_dungeon_clear_count) < 0) {
        payload->proper_dungeon_clear_count = 0;
    }
    user->SetProperDungeonClearCount(
        static_cast<int16_t>(payload->proper_dungeon_clear_count));
    user->SetCurCharacUsedGiftFatigueQuantity(payload->used_gift_fatigue);

    if (user->getCurCharacR()) {
        user->SetFirstTimeLogin(false);
        int daily_reset_time = G_CEnvironment()->get_fatigue_time();
        CCoinRefillEvent* const coin_event = static_cast<CCoinRefillEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(13));
        if (coin_event->IsEventing(0)) {
            daily_reset_time = coin_event->getLastEventTime();
        }

        user->setBlueMarbleEnterCount(payload->blue_marble_enter_count);
        if (CheckDailyScheduleTime(
                daily_reset_time,
                user->getCurCharacLastPlayTick(),
                GlobalData::s_systemTime_.getCurSec())) {
            user->SetFirstTimeLogin(true);
            user->RecoverCoin(1);
            user->AddDailyItem();
            user->ClearCurConditionEventStep();
            user->ClearCurConditionEventRewardStep();
            user->ClearProperDungeonClearCount();
            king_of_pick::OnProperClearSerialReset(user);
            shooting_star_dimension_pocket::OnProperClearSerialReset(user);
            user->SetFirstLogin();

            if (user->IsFirstTimeLogin() && !user->getCurCharacGrowType()) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(417),
                    1,
                    0,
                    0);
            }

            user->ResetCurCharacUsedGiftFatigueQuantity();
            const int character_level = user->get_charac_level();
            if (user->IsFirstTimeLogin() &&
                !user->getCurCharSecondGrowType()) {
                unsigned int grant_count = 0;
                int grow_type = 0;
                if (character_level > 60) {
                    grant_count = 3;
                    grow_type = 12;
                } else if (character_level > 50) {
                    grant_count = 1;
                    grow_type = 11;
                }
                for (unsigned int index = 0; index < grant_count; ++index) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(419),
                        1,
                        static_cast<unsigned int>(grow_type),
                        0);
                }
            }

            if (user->IsFirstTimeLogin() &&
                !user->getCurCharFirstGrowType()) {
                unsigned int grant_count = 0;
                int grow_type = 0;
                if (character_level > 50) {
                    grant_count = 5;
                    grow_type = 4;
                } else if (character_level > 40) {
                    grant_count = 3;
                    grow_type = 3;
                } else if (character_level > 30) {
                    grant_count = 2;
                    grow_type = 2;
                } else if (character_level > 20) {
                    grant_count = 1;
                    grow_type = 1;
                }
                for (unsigned int index = 0; index < grant_count; ++index) {
                    user->getHackAnalyzer()->addServerHackCnt(
                        user,
                        static_cast<WongWork::ENUM_HACKTYPE>(419),
                        1,
                        static_cast<unsigned int>(grow_type),
                        0);
                }
            }
            user->resetBlueMarbleEnterCount();
        }

        user->sendBlueMarbleEnterCount();
        user->SendOpenflag();
        RewardItem(user,
                   user->getCurCharacLastPlayTick(),
                   1262818800,
                   1264028400,
                   0x27ADDDu,
                   "game_server_msg_53",
                   "game_server_msg_131",
                   0);
        if (user->getCurCharacLastPlayTick()) {
            user->initAllSkill();
        }
    }

    user->SendRevengeDungeon();
    user->SendConditionEventInfo();
    user->SendProperDungeonClearCount();
    // King-of-Pick must observe the authoritative etc payload (and the
    // optional daily clear immediately above) before seeding its durable
    // source watermark.  Inter_LoadCharac is too early: this counter is still
    // the CUser initialization default there.
    king_of_pick::RequestLoginState(user);
    // The source serial becomes authoritative only after SetProper... and the
    // optional 06:00 ClearProper... branch above.  Starting from the earlier
    // reach_game_world hook would seed zero and later backfill pre-login
    // clears into the native Dimension Pocket ledger.
    shooting_star_dimension_pocket::BeginSession(user);
    freely_give_item::RequestLoad(user);
    processSeriaRoomAniDecoEvent(payload->seria_room_ani_deco, *user);
    processCheckInvalidExpPerLevel(*user);
    WongWork::CHandlePremium::handlePremiumEffect(
        user, static_cast<ENUM_PREMIUM_TYPE>(9999));

    if (CServerEvent::IsEventing(1)) {
        Inter_MonitorServerEventStart::SendEventNotiPacket(
            true,
            1,
            static_cast<int16_t>(CServerEvent::GetEventInterval(1)),
            user);
    }

    packet.clear();
    packet.put_header(0, 188);
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        packet.put_byte(99);
    } else {
        packet.put_byte(GlobalData::s_power_manager->GetWinnerSide());
    }
    packet.finalize(true);
    user->Send(packet);
    if (GlobalData::s_power_manager->CheckCompleteDBLoadStatueInfo()) {
        GlobalData::s_power_manager->SendPowerWarUserStatueInfo(user);
    }

    packet.clear();
    GlobalData::s_event_manager->MakeNotiEventInfo(packet, user);
    user->Send(packet);
    skill_tree.SendSkillPointReturnInfo();
    user->SendDungeonClearList();
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(11));
    _checkUserDisadvantage(user);
    user->onSelectCharacter();

    if (user->NeedSecuReward() && user->getRewardType()) {
        user->AddSecuRewardItem(user->getRewardType());
        user->setRewardType(0);
    }
    if (user->getPad()->getChangedPwd()) {
        user->getPad()->SendChagePwdRewardMail(user);
    }
    user->discard_mouse_register();

    if (user->get_charac_level() > 39) {
        DB_LoadBloodBestRecord::makeRequest(user->GetUID(),
                                            user->getCurCharacNo());
    }
    dungeon_runtime::BeginInCountLoad(
        user, static_cast<uint32_t>(user->getCurCharacNo()));
    DB_LoadDimension::makeRequest(user->GetUID(), user->getCurCharacNo());
    if (user->isBreakAwayRewardData()) {
        giveBreakAwayReward(user);
    }
    if (user->getBreakAwayDungeonClearCnt() > 0) {
        const int remain_seconds = user->getBreakAwayQuestTime() -
                                   GlobalData::s_systemTime_.getCurSec();
        if (remain_seconds > 0) {
            packet.clear();
            packet.put_header(0, 310);
            packet.put_int(remain_seconds);
            packet.put_byte(3 - user->getBreakAwayDungeonClearCnt());
            packet.finalize(true);
            user->Send(packet);
        } else {
            user->setBreakAwayDungeonClearCnt(-1);
        }
    }

    if (user->get_charac_level() >=
        G_GameWorld()->getDungeonMinimumRequiredLevel(255)) {
        if (G_GameWorld()->IsPVPChannel() != 1) {
            const bool available =
                data_manager->get_server_parameter_script()->isDungeonOpen(255);
            G_GameWorld()->send_user_dungeon_inout_message(user, 11007,
                                                            available);
        }
    }
    data_manager->GetAutoMarketContitionsControl()->SendNotiSetDurability(user);
    if (GlobalData::s_globalEffectManager_->getEffectNum()) {
        packet.clear();
        GlobalData::s_globalEffectManager_->makeGlobalEffectInfo(packet);
        packet.finalize(true);
        user->Send(packet);
    }

    CSharedServerMessageManager* const shared_messages =
        G_CGameManager()->GetSharedServerMessageManager();
    if (shared_messages) {
        if (shared_messages->IsSetServerMessage()) {
            EVENT_SERVER_MESSAGE* const current =
                shared_messages->GetCurServerMessage();
            if (current) {
                payload->server_message_info.count = current->count;
                if (payload->server_message_info.count ==
                    current->messages.size()) {
                    for (size_t index = 0; index < current->messages.size();
                         ++index) {
                        const stSERVER_MESSAGE& source = current->messages[index];
                        EventServerMessagePayload& target =
                            payload->server_message_info.messages[index];
                        target.message_index = source.message_index;
                        target.kind = source.kind;
                        target.update_time = source.update_time;
                        std::strncpy(target.charac_name, source.charac_name,
                                     sizeof(target.charac_name) - 1);
                        target.message_length = source.message_len;
                        std::strncpy(target.message, source.message, 0x31u);
                        target.charac_name_length = source.charac_name_len;
                    }
                }
            }
        } else {
            payload->server_message_info.count = 0;
        }
        shared_messages->SendServerMessageInfo(user,
                                               &payload->server_message_info,
                                               0);
    }

    user->SetLimitInfo(&payload->limit_item_info);
    user->SendLoginToPCRoomServer();
    user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(7))
        ->loadData(user, reinterpret_cast<char*>(&payload->fair_pvp));

    if (user->GetCurCharacExpertJobType() ||
        user->GetCurCharacExpertJobExp() == -1) {
        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 3799),
                           true);
        *guard << 384;
        *guard << user->GetUID();
        *guard << user->getCurCharacNo();
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }

    CItemLimitEditionMgr* const item_limit_manager =
        data_manager->getItemLimitEditionMgr();
    if (!item_limit_manager->empty()) {
        packet.clear();
        packet.put_header(0, 225);
        packet.put_byte(0);
        item_limit_manager->makeItemLimitEditionInfo(packet);
        packet.finalize(true);
        user->Send(packet);
    }

    {
        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 3872),
                           true);
        *guard << 423;
        *guard << user->GetUID();
        *guard << user->getCurCharacNo();
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 3882),
                           true);
        *guard << 469;
        *guard << user->GetUID();
        *guard << static_cast<char>(user->GetServerGroup());
        *guard << user->getCurCharacNo();
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                               "InterDispatcher.cpp", 3894),
                           true);
        *guard << 480;
        *guard << user->GetUID();
        *guard << static_cast<char>(user->GetServerGroup());
        *guard << user->getCurCharacNo();
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    }

    if (G_GameWorld()->GetChannelType() == 14) {
        if (online_preliminary::COnlinePreliminary::GetPlayCountPerOneTeam() ==
            1) {
            CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                                   "InterDispatcher.cpp", 3908),
                               true);
            *guard << 486;
            *guard << user->GetUID();
            *guard << static_cast<char>(user->GetServerGroup());
            *guard << user->getCurCharacNo();
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
        } else {
            CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                                   "InterDispatcher.cpp", 3917),
                               true);
            *guard << 488;
            *guard << user->GetUID();
            *guard << static_cast<char>(user->GetServerGroup());
            *guard << user->getCurCharacNo();
            GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
        }
    }

    user->send_RedeemInfo();
    if (user) {
        GlobalData::s_DailyScheduleManager->CheckAndSendToOneUserSchedule(
            *user);
    }
    user->send_MissionList();
    user->processReturnUser();
    APSystem::CUserProc::SendActionListState(user);
    user->setFlagPresentAvengerTitle(payload->avenger_title_received != 0);
    user->VerifyPresentAvengerTitle();

    ARAD::Arad_EventPeriodDataManager* const event_periods =
        static_cast<ARAD::Arad_EventPeriodDataManager*>(
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()->findGameScript(
                std::string("Arad_EventPeriodDataManager")));
    if (event_periods->isApplied(static_cast<ENUM_REPEAT_EVENT_CODE>(142),
                                 GlobalData::s_systemTime_.getCurSec())) {
        AradServerStateMessage state;
        ARAD::notifyOpenMessageDialog(user, 0, 142, state);
    }

    RequestDBEventArad(user);
    EventClassify::CEventScriptMng* const event_script_manager =
        data_manager->get_event_script_mng();
    event_script_manager->process_account_first_login(
        145, user, payload->user_confirm_flags);
    event_script_manager->process_account_first_login(
        146, user, payload->user_confirm_flags);
    event_script_manager->process_account_first_login(
        156, user, payload->user_confirm_flags);

    user->SendOneADayItemShopIndex(
        data_manager->get_item_shop()->GetOneADayItemList());
    user->reset_is_end_lethe_at_login();
    event_script_manager->send_event_init_data(user, false);
    event_script_manager->send_event_init_record_data(user);
    KiriCargo::SendInfo(user);
    user->setPcRoomPlayTimeData(GlobalData::s_systemTime_.getCurSec(),
                                payload->pc_room_play_time.play_time,
                                payload->pc_room_play_time.reward_index);
    user->set_aura_avatar_option(payload->aura_avatar_option);
    user->send_aura_avatar_option();
    GlobalData::s_serverStateManager_->sendState(user);
    user->setCharacOption(payload->character_option);
    user->sendCharacOption();
    if (payload->tag_charac_no) {
        user->SendTagCharacInfo();
    }
    user->getCurCharacW()->fields_.tag_charac_info_loaded_ = 0;
    G_CGameManager()->GetCraneMinigameManager()->SendCraneItemNeedMaterial(
        user);
    processAttendAnceEvent(&payload->attendance, *user);
    user->SendMaxEquipLevel();
    processGrowthWeaponEvent(*user);
    processGrowthCreatureEvent(*user);
    user->sendBingoAddData();
    user->requestCubePremiumInfoIfContracted();
    CerashopAddRestrict::Manager::LoadRequestBuyRestrictItem(user);
    if (GlobalData::s_event_manager->GetRepeatEvent(158)->IsEventing(0)) {
        DB_LoadEventLevelupSupport::makeRequest(user->GetUID(),
                                                user->getCurCharacNo());
    }
    ARAD::Singleton<HeroMissionEvent>::Get()->loadEventData(user);
    if (Taiwan::GarenaCyberCafe(user->getGarenaAuthData()->getGcaType())) {
        Taiwan::GarenaAuthPCRoom(user);
    }
    if (user->getGarenaAuthData()->getMobileAuth()) {
        DBSelectMobileAuthRewardTw::makeRequest(user);
    }

    fatigue_acceleration::BeginSession(user);

    const int unlock_result = user->unlock4DataLoad();
    LogManager::logFormat(
        1,
        "Inter_LoadEtc.cpp",
        "Inter_LoadEtc::dispatch_sig",
        4408,
        "[CHARVIEW][ETC_EXIT] unlock=%d lock=%d section1=%d section11=%d",
        unlock_result,
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);
    return unlock_result == 1 ? 0 : 4409;
}
