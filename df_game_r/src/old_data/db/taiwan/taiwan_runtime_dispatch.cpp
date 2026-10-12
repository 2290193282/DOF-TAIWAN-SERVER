#include "old_data/db/taiwan/taiwan_runtime_dispatch.h"
#include "infra/concurrent/thread/accept_thread.h"
#include "infra/concurrent/thread/monitor_tcp_thread.h"
#include "infra/concurrent/thread/network_thread.h"
#include "infra/concurrent/thread/udp_thread.h"

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/black_ip_inter_receivers.h"
#include "old_game/social/friend/buddy_monitor_dispatchers_a.h"
#include "old_game/social/guild/guild_double_check_inter_receivers.h"
#include "old_game/social/guild/guild_join_inter_receivers.h"
#include "old_game/social/guild/guild_monitor_notice_dispatchers_a.h"
#include "old_game/social/guild/guild_monitor_query_dispatchers_a.h"
#include "old_game/social/guild/guild_monitor_state_dispatchers_a.h"
#include "infra/security/policy/hack_log_inter_receiver.h"
#include "common/manager/in_game_advertisement_inter_receivers.h"
#include "infra/security/policy/inter_hades_disconnect_dispatcher.h"
#include "common/manager/inter_cera_ipg_return_dispatchers_a.h"
#include "infra/network/transport/inter_game_server_registration_receivers.h"
#include "common/manager/p2p_hole_inter_receiver.h"
#include "common/manager/shared_server_message_inter_receiver.h"
#include "old_data/Login/account_cargo/account_cargo_inter_receivers.h"
#include "old_data/Login/inter_returns/inter_action_point_returns.h"
#include "old_data/Login/inter_returns/inter_blood_dungeon_best_record_return.h"
#include "old_data/Login/inter_returns/inter_breakaway_return_dispatcher.h"
#include "old_data/Login/inter_returns/inter_cutoff_userinfo_return.h"
#include "old_data/Login/inter_returns/inter_monitor_call_userinfo_return.h"
#include "old_data/Login/inter_returns/inter_login_info.h"
#include "old_data/Login/inter_returns/inter_exact_leaf_dispatchers.h"
#include "old_data/Login/inter_returns/inter_event_trigger.h"
#include "old_data/Login/inter_returns/inter_guild_notification_return_dispatchers.h"
#include "old_data/Login/inter_returns/inter_guild_name_return_dispatchers.h"
#include "old_data/Login/inter_returns/inter_death_tower_return_dispatchers.h"
#include "old_data/Login/inter_returns/inter_monitor_return_dispatchers.h"
#include "old_data/Login/inter_returns/inter_social_return_dispatchers.h"
#include "old_data/Login/inter_returns/inter_monitor_notice_control.h"
#include "old_data/Login/inter_returns/inter_monitor_user_getout.h"
#include "old_data/Login/inter_returns/inter_task4_basic_dispatchers.h"
#include "old_data/Login/inter_returns/inter_tcp_server_connect_return.h"
#include "old_game/player/session/inter_online_preliminary_dispatchers.h"
#include "old_game/player/session/inter_character_return_dispatchers.h"
#include "old_game/player/core/inter_pad_reward_dispatcher.h"
#include "old_game/player/session/inter_change_charac_name_return_dispatcher.h"
#include "old_game/player/core/inter_regdate_dispatcher.h"
#include "old_game/player/core/inter_gold_control_dispatcher.h"
#include "old_game/activity/schedule/inter_levelup_support_return_dispatcher.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/activity/schedule/levelup_support_3rd.h"
#include "old_game/economy/equipment/equipment_growth_runtime.h"
#include "old_game/economy/equipment/inter_clone_title.h"
#include "old_game/player/core/inter_charac_link.h"
#include "old_game/activity/schedule/inter_collect_box.h"
#include "old_data/db/character/db_charac_link.h"
#include "old_data/db/character/db_clone_title.h"
#include "old_data/db/event/db_collect_box.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_game/activity/schedule/packet_dispatcher_event_account_fatigue_stat.h"
#include "old_game/activity/schedule/packet_dispatcher_event_all_user_gift.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/packet_dispatcher_skill_damage_rank_event.h"
#include "old_game/activity/schedule/packet_dispatcher_take_magic_lamp.h"
#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"
#include "old_game/activity/schedule/exp_keeping_event.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/dungeon/raid/dual_raid.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/activity/schedule/glass_bottle_event.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/sugar_time_event.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/activity/schedule/sea_bottom_event.h"
#include "old_game/pvp/battlefield/packet_dispatcher_pvp_practice_combo_score.h"
#include "old_game/pvp/match/matching_channel_handoff.h"
#include "old_game/pvp/tournament/pvp_tournament.h"
#include "runtime/exchange_server/exchange_server_runtime.h"
#include "old_data/db/event/db_dragon_timeattack_rank.h"
#include "old_game/dungeon/progress/dragon_timeattack_rank_runtime.h"
#include "old_game/activity/reward/inter_collect_items_reward_returns.h"
#include "old_game/activity/schedule/inter_event_item_return_dispatchers_a.h"
#include "old_game/activity/schedule/inter_server_event_start.h"
#include "old_game/activity/schedule/inter_advance_altar_load.h"
#include "old_game/economy/inventory/inter_load_inven.h"
#include "old_game/appearance/pet/inter_creature_item_returns.h"
#include "old_game/appearance/avatar/inter_avatar_db_returns.h"
#include "old_game/economy/shop/inter_npc_limit_buy_item_info_return.h"
#include "old_game/economy/shop/inter_npc_limit_buy_info_returns.h"
#include "old_game/economy/mall/inter_cerashop_purchase_count_return.h"
#include "old_game/world/region/inter_world_return_dispatchers.h"
#include "old_game/world/region/inter_village_attacked_runtime.h"
#include "old_game/pvp/battlefield/inter_power_war_dialogue2_returns.h"
#include "old_game/dungeon/tower/inter_tower_of_despair_dispatchers.h"
#include "old_data/Login/inter_returns/inter_load_game_option.h"
#include "old_data/Login/inter_returns/inter_load_etc_break_away_reward.h"
#include "old_data/Login/inter_returns/inter_load_skill.h"
#include "old_data/Login/inter_returns/inter_load_ticket.h"
#include "old_data/Login/inter_returns/inter_load_hero_mission_event_data.h"
#include "old_data/Login/inter_returns/inter_last_msg_response.h"
#include "old_data/Login/inter_returns/inter_notice.h"
#include "old_data/Login/inter_returns/inter_ready_to_terminate.h"
#include "old_data/Login/inter_returns/inter_select_mobile_auth_reward.h"
#include "old_data/Login/inter_returns/inter_update_logout.h"
#include "old_data/Login/inter_returns/login_inter_receivers.h"
#include "old_data/Login/inter_returns/auto_punish_inter_receivers.h"
#include "old_data/Login/inter_returns/inter_load_break_away_prevent_system.h"
#include "old_data/Login/packet_dispatch/blacklist_register_delete_dispatchers.h"
#include "old_data/Login/packet_dispatch/guild_board_dispatchers.h"
#include "old_data/Login/security/inter_account_security_return_dispatchers.h"
#include "old_data/Login/security/inter_report_mannerless_return_dispatcher.h"
#include "old_data/Login/security/inter_slang_cleanpad_return_dispatchers.h"
#include "infra/network/packet/PacketHeader.h"
#include "old_data/channel/inter_returns/channel_inter_receivers.h"
#include "infra/security/policy/restrict_geolocation_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/taiwan/taiwan_db_payloads.h"
#include "old_data/db/event/db_event_account_fatigue_stat.h"
#include "old_data/db/event/db_event_all_user_gift.h"
#include "old_data/db/event/db_event_attendance.h"
#include "old_data/db/event/db_king_of_pick.h"
#include "old_data/db/event/db_decide_levelup_event.h"
#include "old_data/db/event/db_fatigue_acceleration.h"
#include "old_data/db/event/db_levelup_support_3rd.h"
#include "old_data/db/event/db_skill_damage_rank_event.h"
#include "old_data/db/event/db_take_magic_lamp.h"
#include "old_data/db/event/db_facebook_like.h"
#include "old_data/db/event/db_freely_give_item.h"
#include "old_data/db/pvp_party/db_integrated_match_pvp_score.h"
#include "old_data/db/pvp_party/db_pvp_practice_combo_score.h"
#include "old_data/db/taiwan/inter_slots_331_353.h"
#include "old_data/db/taiwan/inter_slots_357_382.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/pvp/battlefield/inter_pvp_buddy_runtime.h"
#include "old_game/pvp/battlefield/inter_integrated_match_pvp_score.h"
#include "old_game/pvp/battlefield/inter_pvp_live_event_data_runtime.h"
#include "old_game/social/friend/friend_dispatcher.h"
#include "old_game/social/guild/inter_guild_exp_book_delete_return.h"
#include "old_game/social/guild/inter_guild_member_return_dispatchers.h"
#include "old_game/social/guild/inter_guild_pending_return_dispatchers.h"
#include "old_game/social/party/inter_hyperlink_chat_dispatchers.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/reward/inter_taiwan_event_reward_dispatchers.h"
#include "old_game/activity/reward/inter_ontime_reward_runtime.h"
#include "old_game/economy/mall/auction_runtime.h"
#include "old_game/economy/mall/auction_inter_return_dispatchers.h"
#include "old_game/economy/inventory/inter_owner_is_vip.h"
#include "old_game/economy/mail/inter_mailbox_check_receiver.h"
#include "old_game/economy/mail/inter_mailbox_req_list.h"
#include "old_game/economy/mail/inter_mailbox_req_send.h"
#include "old_game/economy/mail/inter_monitor_notify_new_mail.h"
#include "old_game/economy/mail/inter_multimailbox_send_return.h"
#include "old_game/economy/trade/private_store_inter_load_runtime.h"
#include "old_game/economy/mall/cerashop_add_restrict_inter_dispatcher.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/character_dispatchers.h"
#include "old_game/player/core/inter_set_secoun_user_data.h"
#include "old_game/player/core/userinfo_dispatcher_reads.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/dispatcher/inter_dispatcher_arad_only.h"
#include "old_data/db/core/db_shutdown_quiescence.h"
#include "old_data/db/core/db_thread.h"
#include "old_data/db/core/feature_db_thread.h"
#include "infra/concurrent/task_queue/message_counter.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/concurrent/timer/timer_dispatcher_abi.h"
#include "infra/security/policy/packet_dispatcher.h"
#include "infra/security/policy/inter_antibot_return_dispatchers.h"
#include "infra/security/policy/inter_security_return_dispatchers.h"
#include "common/manager/os_api.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <unistd.h>


namespace {

bool g_taiwan_runtime_started = false;
std::atomic<bool> g_dispatch_stop_after_drain(false);

struct DatabaseClosureDomains {
    bool dispatch;
    bool db;
    bool log_db;
    bool native_db;
};

struct DatabaseClosureSnapshot {
    uint32_t epoch[db_shutdown_quiescence::kActivityDomainCount];
};

DatabaseClosureSnapshot CaptureDatabaseClosureSnapshot() {
    DatabaseClosureSnapshot snapshot;
    for (int index = 0;
         index < db_shutdown_quiescence::kActivityDomainCount;
         ++index) {
        snapshot.epoch[index] =
            db_shutdown_quiescence::ActivityEpoch(
                static_cast<db_shutdown_quiescence::ActivityDomain>(
                    index));
    }
    return snapshot;
}

bool SameDatabaseClosureSnapshot(const DatabaseClosureSnapshot& lhs,
                                 const DatabaseClosureSnapshot& rhs,
                                 const DatabaseClosureDomains& domains) {
    const bool enabled[db_shutdown_quiescence::kActivityDomainCount] = {
        domains.dispatch,
        domains.db,
        domains.log_db,
        domains.native_db,
    };
    for (int index = 0;
         index < db_shutdown_quiescence::kActivityDomainCount;
         ++index) {
        if (enabled[index] && lhs.epoch[index] != rhs.epoch[index]) {
            return false;
        }
    }
    return true;
}

bool DatabaseClosureIdle(const DatabaseClosureDomains& domains) {
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!queues) {
        return false;
    }

    if (domains.dispatch &&
        (!queues->empty(MsgQueueMgr::DISPATCH_Q) ||
         db_shutdown_quiescence::ActivityInFlight(
             db_shutdown_quiescence::kActivityDispatch))) {
        return false;
    }
    if (domains.db &&
        (!queues->empty(MsgQueueMgr::DB_Q) ||
         db_shutdown_quiescence::ActivityInFlight(
             db_shutdown_quiescence::kActivityLegacyDb))) {
        return false;
    }
    if (domains.log_db &&
        (!queues->empty(MsgQueueMgr::LOGDB_Q) ||
         db_shutdown_quiescence::ActivityInFlight(
             db_shutdown_quiescence::kActivityLegacyLogDb))) {
        return false;
    }
    if (domains.native_db &&
        (!queues->empty(MsgQueueMgr::FEATURE_DB_Q) ||
         db_shutdown_quiescence::ActivityInFlight(
             db_shutdown_quiescence::kActivityNativeDb))) {
        return false;
    }
    return true;
}

bool WaitForBarrierAck(uint32_t generation) {
    // Bound the wait so a damaged pool/queue cannot deadlock process teardown.
    // SQL handlers are allowed up to two minutes to finish their current unit.
    const unsigned int kMaximumPolls = 1200000u;
    for (unsigned int poll = 0u; poll < kMaximumPolls; ++poll) {
        const int status =
            db_shutdown_quiescence::GetRoundStatus(generation);
        if (status != 0) {
            return status > 0;
        }
        OS_API::uSleep(100u);
    }
    return false;
}

bool WaitForDatabaseClosure(const DatabaseClosureDomains& domains) {
    // The full graph uses a causal FIFO round trip.  Epoch and in-flight
    // checks include DISPATCH_Q/LOGDB_Q so an ACK cannot overtake a response
    // which starts another DB operation.  Two consecutive clean generations
    // close the observation race at the final ACK boundary.
    if (domains.db && domains.native_db) {
        unsigned int stable_rounds = 0u;
        const unsigned int kMaximumRounds = 1024u;
        for (unsigned int round = 0u; round < kMaximumRounds; ++round) {
            const DatabaseClosureSnapshot before =
                CaptureDatabaseClosureSnapshot();
            const uint32_t generation =
                db_shutdown_quiescence::AllocateGeneration();
            if (!db_shutdown_quiescence::QueueBegin(generation) ||
                !WaitForBarrierAck(generation)) {
                return false;
            }
            const DatabaseClosureSnapshot after =
                CaptureDatabaseClosureSnapshot();
            if (SameDatabaseClosureSnapshot(before, after, domains) &&
                DatabaseClosureIdle(domains)) {
                ++stable_rounds;
                if (stable_rounds == 2u) {
                    return true;
                }
            } else {
                stable_rounds = 0u;
            }
        }
        return false;
    }

    // Early bootstrap failure can leave only a prefix of the worker DAG
    // alive.  There is no q6<->DB cycle in that case, so two unchanged idle
    // observations are the equivalent partial fixed point.
    unsigned int stable_rounds = 0u;
    const unsigned int kMaximumPolls = 120000u;
    for (unsigned int poll = 0u; poll < kMaximumPolls; ++poll) {
        const DatabaseClosureSnapshot before =
            CaptureDatabaseClosureSnapshot();
        if (!DatabaseClosureIdle(domains)) {
            stable_rounds = 0u;
            OS_API::uSleep(1000u);
            continue;
        }
        OS_API::uSleep(1000u);
        const DatabaseClosureSnapshot after =
            CaptureDatabaseClosureSnapshot();
        if (SameDatabaseClosureSnapshot(before, after, domains) &&
            DatabaseClosureIdle(domains)) {
            ++stable_rounds;
            if (stable_rounds == 2u) {
                return true;
            }
        } else {
            stable_rounds = 0u;
        }
    }
    return false;
}

void FailClosedDatabaseShutdown(const char* phase) {
    LogManager::logFormat(
        1,
        "taiwan_runtime_dispatch.cpp",
        "FailClosedDatabaseShutdown",
        __LINE__,
        "database shutdown quiescence failed: phase(%s)",
        phase ? phase : "unknown");
    // Never enter GlobalData::Fini with a live queue consumer or an
    // unacknowledged q6->DB_Q continuation.  A bounded barrier failure is a
    // process-fatal integrity error, not permission to tear shared state down.
    OS_API::Exit(1);
}

template <typename T>
bool AddInterDispatcher(TaiwanInternalDispatcher* owner) {
    T* const dispatcher = new T();
    if (dispatcher) {
        return owner->addDispatcher(
            static_cast<TaiwanInternalPack::T>(dispatcher->getCode()),
            dispatcher);
    }
    return true;
}

void FreeDispatchStream(Stream* stream) {
    if (!stream) {
        return;
    }
    switch (stream->pool_kind()) {
        case 1:
            if (GlobalData::small_stream_pool()) {
                GlobalData::small_stream_pool()->Free(stream);
            }
            break;
        case 2:
            if (GlobalData::big_stream_pool()) {
                GlobalData::big_stream_pool()->Free(stream);
            }
            break;
        default:
            if (GlobalData::stream_pool()) {
                GlobalData::stream_pool()->Free(stream);
            }
            break;
    }
}

}  // namespace


// IDA_EA: 0x84BBAFA
// IDA_NAME: _ZN15InterDispatcherC2Ev
InterDispatcher::InterDispatcher() : dispatchers_() {
    std::memset(dispatchers_, 0, sizeof(dispatchers_));
    // Preserve the original one-based outer signal table.  Every entry below
    // has a concrete, compiled dispatch_sig provider; entries absent here are
    // deliberately left null until their full original provider is restored.
    register_dispatcher(1, new Inter_Disconnect());
    register_dispatcher(2, new Inter_SessionClose());
    register_dispatcher(3, new Inter_LoginInfo());
    register_dispatcher(4, new Inter_CreateCharac());
    register_dispatcher(5, new Inter_DeleteCharac());
    register_dispatcher(7, new Inter_LoadCharac());
    register_dispatcher(8, new Inter_LoadInven());
    register_dispatcher(9, new Inter_LoadSkill());
    register_dispatcher(10, new Inter_LoadQuest());
    register_dispatcher(11, new Inter_LoadTicket());
    register_dispatcher(12, new Inter_LoadDungeonClear());
    register_dispatcher(17, new APSystem::Inter_LoadActionPoint());
    register_dispatcher(18, new APSystem::Inter_EmptyActionPoint());
    register_dispatcher(20, new APSystem::Inter_LoadRewardMedal());
    register_dispatcher(25, new advancealtar::Inter_Load());
    register_dispatcher(32, new Inter_LoadEtc());
    register_dispatcher(44, new Inter_UpdateLogout());
    register_dispatcher(47, new Inter_DeleteChannel());
    register_dispatcher(49, new Inter_AvatarInsert());
    register_dispatcher(54, new Inter_AvatarChangeStat());
    register_dispatcher(61, new Inter_Notice());
    register_dispatcher(62, new Inter_ReadyToTerminate());
    register_dispatcher(63, new Inter_Terminate());
    register_dispatcher(65, new Inter_MonitorLoginOK());
    register_dispatcher(66, new Inter_MonitorCallUserInfo());
    register_dispatcher(67, new Inter_MonitorUserGetout());
    register_dispatcher(68, new Inter_MonitorBuddyInOut());
    register_dispatcher(70, new Inter_MonitorReplyBuddyList());
    register_dispatcher(71, new Inter_MonitorCeraUpdate());
    register_dispatcher(72, new Inter_IPGResult());
    register_dispatcher(73, new Inter_GenCeraTicket());
    register_dispatcher(74, new Inter_MonitorEventItemUpdate());
    register_dispatcher(75, new Inter_LoadEventItem());
    register_dispatcher(79, new Inter_FatigueResetDBReply());
    register_dispatcher(81, new Inter_SuicideWhenQueueIsEmpty());
    register_dispatcher(85, new Inter_MonitorServerEventStart());
    register_dispatcher(86, new Inter_MonitorNoticeGuildEnter());
    register_dispatcher(87, new Inter_MonitorNoticeGuildSecede());
    register_dispatcher(88, new Inter_MonitorNoticeGuildMarkChange());
    register_dispatcher(89, new Inter_MonitorNoticeGuildDismiss());
    register_dispatcher(90, new Inter_MonitorNoticeGuildChatMsg());
    register_dispatcher(91, new Inter_MonitorCallGuildMembers());
    register_dispatcher(92, new Inter_MonitorNoticeGuildNames());
    register_dispatcher(93, new Inter_MonitorNoticeGuildLoginOut());
    register_dispatcher(94, new Inter_MonitorNoticeOtherChannelChat());
    register_dispatcher(95, new Inter_MonitorCallGuildInfo());
    register_dispatcher(96, new Inter_MonitorNoticeGuildInfo());
    register_dispatcher(97, new Inter_MonitorSetGuildKey());
    register_dispatcher(98, new Inter_MonitorNoticeGuildCreate());
    register_dispatcher(100, new Inter_MonitorGuildMemberNamingReply());
    register_dispatcher(101, new Inter_MonitorSetSubGuildMasterReply());
    register_dispatcher(102, new Inter_MonitorNoticeGuildMemberInfo());
    register_dispatcher(108, new Inter_MonitorNoticeMemberLoginOut());
    register_dispatcher(109, new Inter_MonitorNoticeRequestMemberEnterToRequester());
    register_dispatcher(110, new Inter_MonitorNoticeRequestMemberEnterToResponser());
    register_dispatcher(111, new Inter_MonitorNoticeMemberEnterOk());
    register_dispatcher(112, new Inter_MonitorNoticeMemberEnterReply());
    register_dispatcher(113, new Inter_MonitorNoticeMemberSecede());
    register_dispatcher(114, new Inter_MonitorNoticeMemberSecedeToSeceder());
    register_dispatcher(115, new Inter_MonitorNoticeDeleteMemberId());
    register_dispatcher(116, new Inter_MonitorNoticeMemberChatMsg());
    register_dispatcher(117, new Inter_MonitorCallMemberList());
    register_dispatcher(118, new Inter_MonitorPayTaxToUpper());
    register_dispatcher(120, new Inter_LoadPrivateStore());
    register_dispatcher(122, new Inter_UserRepel());
    register_dispatcher(126, new Inter_MailBox_Req_CheckReceiver());
    register_dispatcher(127, new Inter_MailBox_Req_List());
    register_dispatcher(128, new Inter_MailBox_Req_Send());
    register_dispatcher(130, new Inter_Monitor_Notify_New_Mail());
    register_dispatcher(133, new Inter_TriggerEventStart());
    register_dispatcher(134, new Inter_TriggerEventEnd());
    register_dispatcher(136, new Inter_InsertCreatureItemResult());
    register_dispatcher(139, new Inter_UpdateCreatureItemResult());
    register_dispatcher(146, new Inter_MonitorNoticeMessage());
    register_dispatcher(148, new Inter_MonitorForbidChat());
    register_dispatcher(151, new Inter_LastMsgResponse());
    register_dispatcher(152, new Inter_MonitorGuildPointDel());
    register_dispatcher(153, new Inter_MonitorGuildWarInfo());
    register_dispatcher(154, new Inter_SaveClientHackLog());
    register_dispatcher(159, new Inter_CodeChecksumRange());
    register_dispatcher(163, new Inter_MouseRegister());
    register_dispatcher(170, new Inter_Monitor_MegaphoneMsg());
    register_dispatcher(171, new Inter_Monitor_RequestBlackListResult());
    register_dispatcher(172, new Inter_Monitor_RegisterBlackListResult());
    register_dispatcher(173, new Inter_Monitor_DeleteBlackListResult());
    register_dispatcher(174, new Inter_MonitorNoticeBlackList());
    register_dispatcher(179, new Inter_DB_RequestBlackCount());
    register_dispatcher(183, new Inter_WebNoticeSingle());
    register_dispatcher(186, new Inter_MemberExpLevelUp());
    register_dispatcher(188, new Inter_MonitorAddBuddy());
    register_dispatcher(189, new Inter_MonitorDelBuddy());
    register_dispatcher(194, new Inter_ChangeLetterStat());
    register_dispatcher(195, new Inter_ReqChangeCharacName());
    register_dispatcher(196, new Inter_ReqQueryCharacInfo());
    register_dispatcher(198, new Inter_MonitorCallGuildMembersNext());
    register_dispatcher(199, new Inter_MonitorCallGuildAllMembers());
    register_dispatcher(200, new Inter_MonitorCallGuildAllMembersNext());
    register_dispatcher(202, new Inter_ReportMannerless());
    register_dispatcher(206, new Inter_ChagneUserHandicap());
    register_dispatcher(211, new Inter_GMRequestMid());
    register_dispatcher(214, new Inter_UDP_Incoming_Capcha_Data());
    register_dispatcher(215, new Inter_MonitorNoticeChangeGuildMemberGrade());
    register_dispatcher(216, new Inter_GuildNoticeGuildMessage());
    register_dispatcher(218, new Inter_ReplyGuildLetter());
    register_dispatcher(219, new Inter_CallGuildInviteToCaller());
    register_dispatcher(220, new Inter_CallGuildInviteToInvited());
    register_dispatcher(221, new Inter_ReplyGuildInviteToInvited());
    register_dispatcher(222, new Inter_ReplyGuildInviteToCaller());
    register_dispatcher(226, new Inter_LoadDeathTowerBestRecord());
    register_dispatcher(228, new Inter_DeathTowerLoadTopRanker());
    register_dispatcher(229, new Inter_MonitorDeathTowerRanking());
    register_dispatcher(230, new Inter_MonitorTowerReloadTopRanker());
    register_dispatcher(231, new Inter_NoticeGuildMasterDelegate());
    register_dispatcher(232, new Inter_GuildReplyGuildMasterDelegate());
    register_dispatcher(233, new Inter_NoticeHasBeenGuildMaster());
    register_dispatcher(234, new Inter_ReplyGuildSecede());
    register_dispatcher(235, new Inter_DBCheckDoubleGuildName());
    register_dispatcher(236, new Inter_DBCheckDoubleGuildAddress());
    register_dispatcher(237, new Inter_ReplyGuildCreate());
    register_dispatcher(239, new Inter_NotifyAuctionMail());
    register_dispatcher(240, new Inter_AuctionResultAskPrice());
    register_dispatcher(241, new Inter_AuctionResultAskRegistedItemNum());
    register_dispatcher(242, new Inter_AuctionResultRegist());
    register_dispatcher(243, new Inter_AuctionResultBidding());
    register_dispatcher(244, new Inter_AuctionResultRegistCancel());
    register_dispatcher(245, new Inter_AuctionResultItemList());
    register_dispatcher(246, new Inter_AuctionResultMyRegistedItems());
    register_dispatcher(247, new Inter_AuctionResultMyBidding());
    register_dispatcher(248, new Inter_AuctionLogMessage());
    register_dispatcher(249, new Inter_AuctionNotifyAuctionService());
    register_dispatcher(250, new Inter_PointAuctionLogMessage());
    register_dispatcher(251, new Inter_PointAuctionChargeCeraPoint());
    register_dispatcher(253, new Inter_SecurityCardIssue());
    register_dispatcher(255, new Inter_SecurityCardActivate());
    register_dispatcher(262, new Inter_LoadDnfHackLog());
    register_dispatcher(264, new Inter_AutoPunishRuleBlackIP());
    register_dispatcher(265, new Inter_AutoPunishRuleHackType());
    register_dispatcher(271, new Inter_ReplyPowerWarScore());
    register_dispatcher(272, new Inter_NoticePowerWarProlong());
    register_dispatcher(284, new Inter_ReplyJoinPower());
    register_dispatcher(285, new Inter_SecedePower());
    register_dispatcher(286, new Inter_NoticePowerWarStartTime());
    register_dispatcher(287, new Inter_NoticePowerWarEndTime());
    register_dispatcher(288, new Inter_NoticePowerWarInfo());
    register_dispatcher(289, new Inter_TcpServerConnect());
    register_dispatcher(291, new Inter_ChangeGuildName());
    register_dispatcher(292, new Inter_NoticeChangeGuildNameToMembers());
    register_dispatcher(293, new Inter_Add_Slang());
    register_dispatcher(294, new Inter_LoadCleanPadPointReq());
    register_dispatcher(295, new Inter_LoadCleanPadPoint());
    register_dispatcher(298, new Inter_NoticeGuildNameOnGuildCreate());
    register_dispatcher(301, new Inter_CutOffCallUserInfo());
    register_dispatcher(303, new Inter_LoadGameOption());
    register_dispatcher(307, new Inter_PvPChannelInfo());
    register_dispatcher(308, new Inter_PvPChannelUserCount());
    register_dispatcher(312, new Inter_SecurityCardRetransfer());
    register_dispatcher(319, new Inter_LoadBloodDungeonBestRecord());
    // IDA InterDispatcher C2 places the global BreakAway DB return consumer
    // at outer signal 321.  DB_LoadBreakAwayPreventData emits this code with
    // user_idx -1, so it must not be routed through the Taiwan nested map.
    register_dispatcher(321, new Inter_LoadBreakAwayPreventSystem());
    register_dispatcher(323, new Inter_LoadBreakAwayPreventMessage());
    register_dispatcher(331, new Inter_LoadCharacLinkBonus());
    register_dispatcher(334, new Inter_LoadDimensionInout());
    register_dispatcher(339, new Inter_BlackIPMonitorReq());
    register_dispatcher(341, new Inter_BlackIPMonitorPartLoadReq());
    register_dispatcher(342, new Inter_BlackIPMonitor());
    register_dispatcher(343, new Inter_BlackIPMonitorDeleteIP());
    register_dispatcher(346, new Inter_LoadAutoMarketConditionsControl());
    register_dispatcher(348, new Inter_UpdateOneDayLethe());
    register_dispatcher(351, new Inter_DisconnectMonitorServer());
    register_dispatcher(352, new Inter_ResposncePCRoomPlyerList());
    register_dispatcher(353, new Inter_ResposncePCRoomPlyerCount());
    register_dispatcher(354, new Inter_NoticePCRoomPlayerLoginOut());
    register_dispatcher(357, new Inter_LoadPowerWarStatueRanker());
    register_dispatcher(358, new Inter_LoadPowerWarStatueInfo());
    register_dispatcher(364, new Inter_UpdateServerMessage());
    register_dispatcher(365, new Inter_LoadServerMessage());
    register_dispatcher(368, new Inter_NoticeServerMessageInfo());
    register_dispatcher(369, new Inter_NoticeCreateGuildAgit());
    register_dispatcher(370, new Inter_NoticeDeleteGuildAgit());
    register_dispatcher(371, new Inter_NoticeLoadGuildAgit());
    register_dispatcher(372, new Inter_NoticePowerWarRank());
    register_dispatcher(374, new Inter_NoticeGuildMemo());
    register_dispatcher(378, new Inter_PowerWarProcessInfo());
    register_dispatcher(379, new Inter_PowerWarEndInfo());
    register_dispatcher(381, new Inter_MonitorPunishUserReq());
    register_dispatcher(382, new Inter_PcroomResponse());
    register_dispatcher(384, new Inter_LoadExpertJob());
    register_dispatcher(388, new Inter_SendAllUserInfoMiniumForGuildSystem());
    register_dispatcher(389, new Inter_ItemLimitEdition_Sell_Start());
    register_dispatcher(390, new Inter_ItemLimitEdition_Sell_End());
    register_dispatcher(391, new Inter_ItemLimitEdition_Update());
    register_dispatcher(392, new Inter_ItemLimitEdition_Reply());
    register_dispatcher(394, new Inter_FindFactoryHub());
    register_dispatcher(396, new Inter_Hack_CheckMyAccount());
    register_dispatcher(398, new Inter_GuildExpLimit());
    register_dispatcher(402, new Inter_SetIPCount());
    register_dispatcher(404, new Inter_SetFullIPCount());
    register_dispatcher(405, new Inter_CancelRestingUser());
    register_dispatcher(408, new Inter_ApplyTempPowerSide());
    register_dispatcher(409, new Inter_ReloadPowerWarRanker());
    register_dispatcher(411, new Inter_LoadPowerWarInfo());
    register_dispatcher(416, new Inter_SetFactoryHubUser());
    register_dispatcher(417, new Inter_SetFactoryHubUserNotConn());
    register_dispatcher(418, new Inter_Set_CleanPad_Point());
    register_dispatcher(423, new Inter_LoadNoCacheData());
    register_dispatcher(426, new Inter_GuildCargo());
    register_dispatcher(427, new Inter_NoticeGuildCargo());
    register_dispatcher(428, new Inter_GuildCargoHistory());
    register_dispatcher(429, new Inter_GuildCargoCheckPushItem());
    register_dispatcher(430, new Inter_GuildCargoPushItem());
    register_dispatcher(431, new Inter_GuildCargoPopItem());
    register_dispatcher(432, new Inter_GuildCargoMoveItem());
    register_dispatcher(434, new Inter_GuildCargoUpgrade());
    register_dispatcher(435, new Inter_GuildBoardData());
    register_dispatcher(436, new Inter_GuildBoardDelete());
    register_dispatcher(437, new Inter_TakeScreenShot());
    register_dispatcher(441, new Inter_VillageAttackedStart());
    register_dispatcher(442, new Inter_VillageAttackedEnd());
    register_dispatcher(443, new Inter_VillageAttackedCountdown());
    register_dispatcher(444, new Inter_VillageAttackedUpdate());
    register_dispatcher(445, new Inter_VillageAttackedScore());
    register_dispatcher(446, new Inter_VillageAttackedReward());
    register_dispatcher(447, new Inter_VillageAttackedRewardServer());
    register_dispatcher(448, new Inter_FullLevelBroadCast());
    register_dispatcher(458, new Inter_SetSecounUserData());
    register_dispatcher(461, new Inter_SetARSInfo());
    register_dispatcher(463, new Inter_OnTimeEventRewardStart());
    register_dispatcher(465, new Inter_OnTimeEventRewardEnd());
    register_dispatcher(469, new Inter_LoadPvPBuddy());
    register_dispatcher(470, new Inter_PvPBuddyConnList());
    register_dispatcher(471, new Inter_ReqAddPvPBuddy());
    register_dispatcher(472, new Inter_AddPvPBuddyResult());
    register_dispatcher(473, new Inter_RemovePvPBuddyResult());
    register_dispatcher(474, new Inter_NoticePvPBuddyInOut());
    register_dispatcher(480, new Inter_LoadPvPLiveEventData());
    register_dispatcher(486, new Inter_LoadOnlinePreliminaryMember());
    register_dispatcher(488, new Inter_LoadOnlinePreliminaryTeam());
    register_dispatcher(490, new Inter_LoadOnlinePreliminaryMemberMatchList());
    register_dispatcher(492, new Inter_LoadOnlinePreliminaryTeamMatchList());
    register_dispatcher(494, new Inter_CreateAccountCargo());
    register_dispatcher(495, new Inter_UpgradeAccountCargo());
    register_dispatcher(498, new Inter_LoadAccountCargo());
    register_dispatcher(501, new Inter_MultiMailBoxReqSend());
    register_dispatcher(502, new Inter_ReqQueryCharacInfoMailbox());
    register_dispatcher(503, new Inter_ReqGameServerInfo());
    register_dispatcher(504, new Inter_MonitorRegistGameServerAck());
    register_dispatcher(509, new Inter_LoadMileage());
    register_dispatcher(521, new Inter_GuildExpBookDelete());
    register_dispatcher(522, new Inter_NoCache());
    register_dispatcher(523, new Inter_Send_Charac_no_from_monitor());
    register_dispatcher(525, new Inter_AuctionResultBuyItemApiece());
    register_dispatcher(528, new Inter_SendPeriodicMessage());
    register_dispatcher(529, new Inter_SendTimeSync());
    register_dispatcher(530, new Inter_SendTimeSyncLogin());
    register_dispatcher(532, new Inter_AddGuildFundResult());
    register_dispatcher(533, new Inter_TodayGuildMember());
    register_dispatcher(534, new Inter_CheckJoinGuild());
    register_dispatcher(535, new Inter_RequestJoinGuild());
    register_dispatcher(537, new Inter_JoinGuildInfo());
    register_dispatcher(538, new Inter_GuildJoinList());
    register_dispatcher(539, new Inter_ApproveJoinGuild());
    register_dispatcher(541, new Inter_GuildAttendanceInfo());
    register_dispatcher(542, new Inter_AchieveGuildAttendance());
    register_dispatcher(570, new Inter_HadesDisConnect());
    register_dispatcher(572, new Inter_UpdatePurchaseCashItemBonus());
    register_dispatcher(581, new Inter_GoblinPadChagePwdReward());
    register_dispatcher(582, new Inter_GoblinPadSamePwdCheck());
    register_dispatcher(594, new Inter_Antibot_SendBufferClient());
    register_dispatcher(595, new Inter_Antibot_PunishClient());
    register_dispatcher(596, new Inter_Antibot_ProcessProtectMsg());
    register_dispatcher(603, new Inter_PunishCancel());
    register_dispatcher(608, new Inter_BroadcastMsg());
    register_dispatcher(628, new Inter_UpdateSecuServiceState());
    register_dispatcher(630, new Inter_TOD_ReturnToMonitor_APCSettingComplete());
    register_dispatcher(631, new Inter_TOD_Reload_APC());
    register_dispatcher(642, new Inter_LoadKillMonsterInfo());
    register_dispatcher(654, new Inter_LevelUpReward());
    register_dispatcher(673, new Inter_GetUserRegdate());
    register_dispatcher(675, new Inter_GoldControlMoneyActionCancel());
    register_dispatcher(677, new Inter_MonitorNoticeGuildChatMsgHyperLink());
    register_dispatcher(683, new Inter_SendSecuReward());
    register_dispatcher(678, new Inter_MonitorNoticeOtherChannelChatHyperLink());
    register_dispatcher(679, new Inter_MonitorNoticeMemberChatMsgHyperLink());
    register_dispatcher(680, new Inter_Monitor_MegaphoneMsgHyperLink());
    register_dispatcher(694, new Inter_HadesAction());
    register_dispatcher(695, new Inter_Check3rdPartyConcent());
    register_dispatcher(697, new Inter_SelectCharacMid());
    register_dispatcher(709, new Inter_NpcLimitBuyItemInfo());
    register_dispatcher(710, new Inter_NpcLimitBuyItemInfoAll());
    register_dispatcher(711, new Inter_NpcLimitBuyItemChangeInfo());
    register_dispatcher(712, new Inter_NpcLimitBuyItemInfoRenew());
    register_dispatcher(718, new Inter_MonitorReplyCharacInfo());
    register_dispatcher(729, new Inter_CeraShopPurchaseCount());
    register_dispatcher(730, new Inter_OwnerisVIP());
    register_dispatcher(732, new Inter_DispatcherAradOnly());
    register_dispatcher(737, new Inter_LoadPuUserCheck());
    register_dispatcher(759, new Inter_WebNoticeIngameAdvertisement());
    register_dispatcher(761, new Inter_ReloadIngameAdvertisement());
    register_dispatcher(762, new Inter_CollectItemsResult());
    register_dispatcher(763, new Inter_CollectItemsReward());
    register_dispatcher(764, new Inter_CollectItemsRewardBroadcast());
    register_dispatcher(765, new Inter_RequestCharacSkillInfo());
    register_dispatcher(766, new Inter_RequestTagCharacInfo());
    register_dispatcher(770, new Inter_EmergencyPatchMessage());
    register_dispatcher(777, new Inter_AuctionSubMoney());
    register_dispatcher(778, new Inter_UpdateServerSnapShot());
    register_dispatcher(800, new Inter_LoadCharacTypeHotKey());
    register_dispatcher(815, new Inter_UpdateMiniCraneSeed());
    register_dispatcher(818, new Inter_GetPremiumSelectedCubeInfo());
    register_dispatcher(integrated_match_pvp_score_db::kDbSignal,
                        new Inter_IntegratedMatchPvpScore());
    register_dispatcher(830, new Inter_CheckAddFriend());
    register_dispatcher(833, new Inter_LoadCerashopAddRestrictType());
    register_dispatcher(event_all_user_gift::kDbSignal,
                        new Inter_EventAllUserGift());
    register_dispatcher(king_of_pick_db::kDbSignal,
                        new Inter_KingOfPick());
    register_dispatcher(magic_lamp::kDbSignal,
                        new Inter_TakeMagicLamp());
    register_dispatcher(facebook_like_db::kDbSignal,
                        new Inter_FacebookLike());
    register_dispatcher(event_attendance_db::kDbSignal,
                        new Inter_EventAttendance());
    register_dispatcher(pvp_practice_combo_score::kDbSignal,
                        new Inter_PvpPracticeComboScore());
    register_dispatcher(dragon_timeattack_rank::kDbSignal,
                        new Inter_DragonTimeAttackRank());
    register_dispatcher(skill_damage_rank_event::kDbSignal,
                        new Inter_SkillDamageRankEvent());
    register_dispatcher(848, new Inter_LoadEventLevelupSupport());
    register_dispatcher(decide_levelup_db::kDbSignal,
                        new Inter_DecideLevelUp());
    register_dispatcher(item_growth_db::kDbSignal,
                        new Inter_ItemGrowthState());
    register_dispatcher(fatigue_acceleration_db::kDbSignal,
                        new Inter_FatigueAcceleration());
    register_dispatcher(850, new Inter_SelectAllowProxyUser());
    register_dispatcher(event_account_fatigue_stat::kDbSignal,
                        new Inter_EventAccountFatigueStat());
    register_dispatcher(levelup_support_3rd_db::kDbSignal,
                        new Inter_LevelupSupport3rd());
    register_dispatcher(clone_title_db::kDbSignal,
                        new Inter_CloneTitle());
    register_dispatcher(charac_link_db::kDbSignal,
                        new Inter_CharacLink());
    register_dispatcher(collect_box_db::kDbSignal,
                        new Inter_CollectBox());
    // IDA constructor writes Inter_DispatcherTaiwan at table slot 846.
    register_dispatcher(kTaiwanDispatcherSlot, new Inter_DispatcherTaiwan());
}

// IDA_EA: 0x84BE89E
// IDA_NAME: _ZN15InterDispatcherD2Ev
InterDispatcher::~InterDispatcher() {
    for (int index = 0; index < kDispatcherCount; ++index) {
        delete dispatchers_[index];
        dispatchers_[index] = 0;
    }
}

// IDA_EA: 0x84BE906
// IDA_NAME: _ZN15InterDispatcher14get_dispatcherEi
ISigDispatcher* InterDispatcher::get_dispatcher(int code) {
    if (code >= 0 && code < kDispatcherCount && dispatchers_[code]) {
        return dispatchers_[code];
    }
    LogManager::logFormat(1,
                          "InterDispatcher.cpp",
                          "ISigDispatcher* InterDispatcher::get_dispatcher(int)",
                          0x5E5,
                          "cmd_map.find( code %d )",
                          code);
    return 0;
}

// IDA_EA: 0x84BE95E
// IDA_NAME: _ZN15InterDispatcher19register_dispatcherEiP14ISigDispatcher
InterDispatcher* InterDispatcher::register_dispatcher(int code,
                                                       ISigDispatcher* dispatcher) {
    if (code >= 0 && code < kDispatcherCount) {
        dispatchers_[code] = dispatcher;
    }
    return this;
}

// IDA_EA: 0x84BE970
// IDA_NAME: _ZN15InterDispatcher8dispatchEiiPci
int InterDispatcher::dispatch(int code,
                              int user_idx,
                              char* payload,
                              int payload_size) {
    if (code == 65) {
        LogManager::logFormat(
            1, "monitor_login_flow.cpp",
            "int InterDispatcher::dispatch(int, int, char*, int)", 0,
            "[MONITOR][INTER65_ENTER] uid=%d payload_size=%d",
            user_idx, payload_size);
    }
    if (code == 3) {
        LogManager::logFormat(1, "login_flow_diag.cpp",
                              "int InterDispatcher::dispatch(int, int, char*, int)",
                              0,
                              "[LOGIN_FLOW][INTER_LOGIN_REPLY] uid=%d payload_size=%d",
                              user_idx, payload_size);
    }
    if (code == 303) {
        LogManager::logFormat(1, "login_flow_diag.cpp",
                              "int InterDispatcher::dispatch(int, int, char*, int)",
                              0,
                              "[LOGIN_FLOW][GAME_OPTION_INTER_ENTER] uid=%d payload_size=%d",
                              user_idx, payload_size);
    }
    CUser* user = 0;
    if (user_idx != -1) {
        user = G_CGameManager()->getUser(user_idx, 0);
        if (!user) {
            if (code == 65) {
                LogManager::logFormat(
                    1, "monitor_login_flow.cpp",
                    "int InterDispatcher::dispatch(int, int, char*, int)", 0,
                    "[MONITOR][INTER65_USER_MISSING] uid=%d", user_idx);
            }
            if (code == 303) {
                LogManager::logFormat(
                    1, "login_flow_diag.cpp",
                    "int InterDispatcher::dispatch(int, int, char*, int)", 0,
                    "[LOGIN_FLOW][GAME_OPTION_INTER_USER_MISSING] uid=%d",
                    user_idx);
            }
            return 0;
        }
    }

    ISigDispatcher* const dispatcher = get_dispatcher(code);
    if (!dispatcher) {
        if (code == 65) {
            LogManager::logFormat(
                1, "monitor_login_flow.cpp",
                "int InterDispatcher::dispatch(int, int, char*, int)", 0,
                "[MONITOR][INTER65_DISPATCHER_MISSING] uid=%d", user_idx);
        }
        if (code == 3) {
            LogManager::logFormat(1, "login_flow_diag.cpp",
                                  "int InterDispatcher::dispatch(int, int, char*, int)",
                                  0,
                                  "[LOGIN_FLOW][INTER_LOGIN_MISSING] slot=3");
        }
        if (code == 303) {
            LogManager::logFormat(
                1, "login_flow_diag.cpp",
                "int InterDispatcher::dispatch(int, int, char*, int)", 0,
                "[LOGIN_FLOW][GAME_OPTION_INTER_MISSING] slot=303 uid=%d",
                user_idx);
        }
        LogManager::logFormat(1,
                              "InterDispatcher.cpp",
                              "bool InterDispatcher::dispatch(int, int, char*, int)",
                              0x602,
                              "get_dispatcher(%d)",
                              code);
        return 0;
    }

    const int result = dispatcher->dispatch_sig(user, payload, payload_size);
    if (code == 65) {
        LogManager::logFormat(
            1, "monitor_login_flow.cpp",
            "int InterDispatcher::dispatch(int, int, char*, int)", 0,
            "[MONITOR][INTER65_DONE] uid=%d state=%d result=%d",
            user_idx, user ? user->get_state() : -1, result);
    }
    if (code == 3) {
        LogManager::logFormat(1, "login_flow_diag.cpp",
                              "int InterDispatcher::dispatch(int, int, char*, int)",
                              0,
                              "[LOGIN_FLOW][INTER_LOGIN_DONE] uid=%d result=%d",
                              user_idx, result);
    }
    if (code == 303) {
        LogManager::logFormat(1, "login_flow_diag.cpp",
                              "int InterDispatcher::dispatch(int, int, char*, int)",
                              0,
                              "[LOGIN_FLOW][GAME_OPTION_INTER_DONE] uid=%d state=%d result=%d",
                              user_idx,
                              user ? user->get_state() : -1,
                              result);
    }
    if (result && user) {
        user->DisConnSig(static_cast<DISCONN_SIG>(2), true, result + 1000000);
    }
    return 1;
}


// IDA_EA: 0x8161EC4 / 0x8160796
InterLoadServiceRestrictPolicy::InterLoadServiceRestrictPolicy()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::LOAD_SERVICE_RESTRICT_POLICY) {
}

InterLoadServiceRestrictPolicy::~InterLoadServiceRestrictPolicy() {
}

int InterLoadServiceRestrictPolicy::dispatch_sig(CUser*,
                                                  char* payload,
                                                  int) {
    if (!payload) {
        cMyTrace trace(
            "virtual int InterLoadServiceRestrictPolicy::dispatch_sig(CUser*, char*, int)",
            148,
            5);
        trace("[Taiwan, Restrict] SigLoadRestrictPolicy is null.");
        return 149;
    }

    Taiwan::SigLoadRestrictPolicy* const policy =
        reinterpret_cast<Taiwan::SigLoadRestrictPolicy*>(payload);
    // IDA builds std::string from payload + 0x119d, the first data byte after
    // values[0], then passes payload->code as the category.
    ARAD::Singleton<ServiceRestrictManager>::Get()->beginPolicy(
        static_cast<RestrictCategory::Enum>(policy->code),
        std::string(policy->values + 1));
    return 0;
}


// IDA_EA: 0x8161F36 / 0x816088A
InterLoadGeolocation::InterLoadGeolocation()
    : TaiwanInterfaceInterDispatcher(TaiwanInternalPack::LOAD_GEOLOCATION) {
}

InterLoadGeolocation::~InterLoadGeolocation() {
}

int InterLoadGeolocation::dispatch_sig(CUser*, char* payload, int) {
    if (!payload) {
        cMyTrace trace("virtual int InterLoadGeolocation::dispatch_sig(CUser*, char*, int)",
                       172,
                       5);
        trace("[Taiwan, GeoIP] SigLoadRestrictPolicy is null.");
        return 173;
    }

    Taiwan::SigLoadGeolocation* const countries =
        reinterpret_cast<Taiwan::SigLoadGeolocation*>(payload);
    if (countries->count > 45u) {
        cMyTrace trace("virtual int InterLoadGeolocation::dispatch_sig(CUser*, char*, int)",
                       178,
                       5);
        trace("[Taiwan, GeoIP] Invalid GeoIP country count. (%d)",
              countries->count);
        countries->count = 45u;
    }
    for (uint32_t index = 0; index < countries->count; ++index) {
        const std::string country(countries->country[index]);
        ARAD::Singleton<RestrictGeolocation>::Get()->addAllowCountry(country);
        cMyTrace trace("virtual int InterLoadGeolocation::dispatch_sig(CUser*, char*, int)",
                       185,
                       9,
                       true,
                       true);
        trace("GeoIP Allow Country Code : %s", countries->country[index]);
    }
    return 0;
}


// IDA_EA: 0x81601D0
// IDA_NAME: _ZN24TaiwanInternalDispatcherC2Ev
// DEMANGLED: TaiwanInternalDispatcher::TaiwanInternalDispatcher(void)
TaiwanInternalDispatcher::TaiwanInternalDispatcher() : dispatchers_() {
    dispatchers_.clear();
}

// IDA_EA: 0x8160212
// IDA_NAME: _ZN24TaiwanInternalDispatcherD2Ev
// DEMANGLED: TaiwanInternalDispatcher::~TaiwanInternalDispatcher()
TaiwanInternalDispatcher::~TaiwanInternalDispatcher() {
    std::for_each(dispatchers_.begin(), dispatchers_.end(), FnDelete());
}

// IDA_EA: 0x8160294
// IDA_NAME: _ZN24TaiwanInternalDispatcher4initEv
// DEMANGLED: TaiwanInternalDispatcher::init(void)
bool TaiwanInternalDispatcher::init() {
    return AddInterDispatcher<InterLoadServiceRestrictPolicy>(this) &&
           AddInterDispatcher<InterLoadGeolocation>(this) &&
           AddInterDispatcher<InterCheckStayTimeEvent>(this) &&
           AddInterDispatcher<InterSelectCreateDnfEventInfo>(this) &&
           AddInterDispatcher<InterSelectCreateDnfUserInfo>(this) &&
           AddInterDispatcher<InterSelectMobileAuthReward>(this) &&
           AddInterDispatcher<InterSelectPcroomDailyReward>(this) &&
           AddInterDispatcher<InterUpdateP2pHolePunchingState>(this) &&
           AddInterDispatcher<InterInsertGiveBoxEventReward>(this) &&
           AddInterDispatcher<InterLoadHeroMissionEventData>(this) &&
           AddInterDispatcher<InterExpKeepingEvent>(this) &&
           AddInterDispatcher<InterFreelyGiveItem>(this) &&
           AddInterDispatcher<InterPvpTournament>(this) &&
           AddInterDispatcher<InterDualRaidReward>(this) &&
           AddInterDispatcher<InterRacingDungeon>(this) &&
           AddInterDispatcher<InterMatchingChannelHandoff>(this) &&
           AddInterDispatcher<InterZombieModeResult>(this) &&
           AddInterDispatcher<InterGoldenCommerceReward>(this) &&
           AddInterDispatcher<InterGlassBottleEvent>(this) &&
           AddInterDispatcher<InterBlueHorseEvent>(this) &&
           AddInterDispatcher<InterSugarTime>(this) &&
           AddInterDispatcher<InterShootingStarDimensionPocket>(this) &&
           AddInterDispatcher<InterSeaBottomEvent>(this);
}

// IDA_EA: 0x816069A
// IDA_NAME: _ZN24TaiwanInternalDispatcher13getDispatcherEN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanInternalDispatcher::getDispatcher(TaiwanInternalPack::T)
TaiwanInterfaceInterDispatcher* TaiwanInternalDispatcher::getDispatcher(
    TaiwanInternalPack::T code) {
    DispatcherMap::iterator it = dispatchers_.find(code);
    return it == dispatchers_.end() ? 0 : it->second;
}

// IDA_EA: 0x81606FE
// IDA_NAME: _ZN24TaiwanInternalDispatcher13addDispatcherEN18TaiwanInternalPack1TEP30TaiwanInterfaceInterDispatcher
// DEMANGLED: TaiwanInternalDispatcher::addDispatcher(TaiwanInternalPack::T,TaiwanInterfaceInterDispatcher *)
bool TaiwanInternalDispatcher::addDispatcher(
    TaiwanInternalPack::T code,
    TaiwanInterfaceInterDispatcher* dispatcher) {
    if (getDispatcher(code)) {
        return false;
    }
    dispatchers_[code] = dispatcher;
    return true;
}

// IDA_EA: 0x8160744
// IDA_NAME: _ZN24TaiwanInternalDispatcher13delDispatcherEN18TaiwanInternalPack1TE
// DEMANGLED: TaiwanInternalDispatcher::delDispatcher(TaiwanInternalPack::T)
bool TaiwanInternalDispatcher::delDispatcher(TaiwanInternalPack::T code) {
    if (!getDispatcher(code)) {
        return false;
    }
    dispatchers_.erase(dispatchers_.find(code));
    return true;
}

// IDA_EA: 0x8161E9A
// IDA_NAME: _ZN24TaiwanInternalDispatcher8FnDeleteclERSt4pairIKN18TaiwanInternalPack1TEP30TaiwanInterfaceInterDispatcherE
// DEMANGLED: TaiwanInternalDispatcher::FnDelete::operator()(std::pair<TaiwanInternalPack::T const,TaiwanInterfaceInterDispatcher *> &)
void TaiwanInternalDispatcher::FnDelete::operator()(
    std::pair<const TaiwanInternalPack::T,
              TaiwanInterfaceInterDispatcher*>& entry) {
    if (entry.second) {
        delete entry.second;
    }
}


// IDA_EA: 0x84E8BBA
// IDA_NAME: _ZN22Inter_DispatcherTaiwanC2Ev
// DEMANGLED: Inter_DispatcherTaiwan::Inter_DispatcherTaiwan(void)
Inter_DispatcherTaiwan::Inter_DispatcherTaiwan()
    : ISigDispatcher(), dispatcher_() {
    (void)dispatcher_.init();
}

// IDA_EA: 0x84F577A
// IDA_NAME: _ZN22Inter_DispatcherTaiwanD2Ev
// DEMANGLED: Inter_DispatcherTaiwan::~Inter_DispatcherTaiwan()
// IDA_EA: 0x84F57DC
// IDA_NAME: _ZN22Inter_DispatcherTaiwanD0Ev
// DEMANGLED: Inter_DispatcherTaiwan::~Inter_DispatcherTaiwan()
Inter_DispatcherTaiwan::~Inter_DispatcherTaiwan() {
}

// IDA_EA: 0x84E8C2E
// IDA_NAME: _ZN22Inter_DispatcherTaiwan12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DispatcherTaiwan::dispatch_sig(CUser *,char *,int)
int Inter_DispatcherTaiwan::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    int code = 0;
    std::memcpy(&code, payload, sizeof(code));
    TaiwanInterfaceInterDispatcher* const dispatcher =
        dispatcher_.getDispatcher(static_cast<TaiwanInternalPack::T>(code));
    if (dispatcher) {
        return dispatcher->dispatch_sig(user,
                                        payload + sizeof(code),
                                        payload_size - sizeof(code));
    }

    cMyTrace trace(
        "virtual int Inter_DispatcherTaiwan::dispatch_sig(CUser*, char*, int)",
        28106,
        5);
    trace("[TaiwanDispatcher] Invalid internal packet code. Code:%d", code);
    return 28107;
}


// IDA_EA: 0x847E268 / 0x847E2D2
DispatchThread::DispatchThread() : ThreadInterface() {
    g_dispatch_stop_after_drain.store(false, std::memory_order_release);
}

DispatchThread::~DispatchThread() {
}

void DispatchThread::requestStopAfterDrain() {
    g_dispatch_stop_after_drain.store(true, std::memory_order_release);
}

void DispatchThread::dispatch(void*) {
    TimerEntry timer_entry;
    message_counter::CMessageCounter message_counter(10000);
    char* const server_name = const_cast<char*>(
        G_CEnvironment()->get_file_name());
    static char kUserPacketCounterName[] = "UserPacketCounter";
    message_counter::CUserPacketCounter user_packet_counter(
        server_name, kUserPacketCounterName);
    PacketDispatcher::pUserPacketCounter_ = &user_packet_counter;
    message_counter::CTimeInterval* const interval =
        message_counter::CTimeInterval::Create();
    int last_log_second = GlobalData::system_time()->getCurSec();

    for (;;) {
        GlobalData::system_time()->update();
        // Timers are ingress.  Once global stop is published, Dispatch stays
        // alive only as the response/continuation consumer needed to close
        // DISPATCH_Q <-> DB_Q <-> queue6.
        if (!GlobalData::server_stop() &&
            G_TimerQueue()->GetTimerMess(&timer_entry)) {
            db_shutdown_quiescence::BeginActivity(
                db_shutdown_quiescence::kActivityDispatch,
                -1);
            interval->SetLastTime();
            G_TimerDispatcher()->dispatch(timer_entry);
            message_counter.IncreaseMessage(
                0, 0,
                static_cast<uint16_t>(timer_entry.message + 30000),
                interval->GetTimeInterval());
            db_shutdown_quiescence::EndActivity(
                db_shutdown_quiescence::kActivityDispatch);
        }

        Stream* const stream = GlobalData::msgq_mgr()->get(
            MsgQueueMgr::DISPATCH_Q);
        if (!stream) {
            if (g_dispatch_stop_after_drain.load(
                    std::memory_order_acquire)) {
                break;
            }
            usleep(100);
            continue;
        }

        db_shutdown_quiescence::BeginActivity(
            db_shutdown_quiescence::kActivityDispatch,
            -1);

        const int stream_length = stream->length();
        const int pool_kind = stream->pool_kind();
        unsigned int code = 0;
        int user_idx = 0;
        (*stream) >> code;
        (*stream) >> user_idx;
        const int current_second = GlobalData::system_time()->getCurSec();
        interval->SetLastTime();
        if (code) {
            G_InterDispatcher()->dispatch(code,
                                          user_idx,
                                          stream->get_buffer(),
                                          stream->length());
        } else {
            // CUser::Parsing enqueues only complete C2S frames.  Keep this
            // DispatchThread accounting/diagnostic reader tied to the same
            // packed CS_Header consumed by PacketDispatcher, rather than
            // duplicating byte 0/1 literals here.
            const CS_Header* const c2s_header =
                reinterpret_cast<const CS_Header*>(stream->get_buffer());
            const uint8_t packet_class = c2s_header->cmd;
            const uint16_t packet_id = c2s_header->type;
            if (packet_class == 1 && packet_id == 1) {
                LogManager::logFormat(
                    1, "dispatch_diag.cpp", "DispatchThread::dispatch", 0,
                    "[DIAG0627][DISPATCH_CONSUME] uid=%d class=%d id=%u payload=%d",
                    user_idx, static_cast<int>(packet_class),
                    static_cast<unsigned int>(packet_id), stream->length());
            }
            G_PacketDispatcher()->dispatch(user_idx,
                                            stream->get_buffer(),
                                            stream->length());
            message_counter.IncreaseMessage(0,
                                            packet_class,
                                            packet_id,
                                            interval->GetTimeInterval());
        }

        if (code) {
            message_counter.IncreaseMessage(code,
                                            0,
                                            0,
                                            interval->GetTimeInterval());
        }
        if (current_second - last_log_second > 600) {
            last_log_second = current_second;
            message_counter.WriteLog();
            user_packet_counter.WriteLog();
            cMyTrace trace("virtual void DispatchThread::dispatch(void*)",
                           143,
                           5);
            trace("timer_queue_count(%u)",
                  static_cast<unsigned int>(G_TimerQueue()->GetQueueSize()));
            G_TimerQueue()->write_timer_queue_log();
        }

        if (pool_kind == 1 && stream_length > 4095) {
            cMyTrace trace("virtual void DispatchThread::dispatch(void*)", 158, 5);
            trace("Small Stream Size over code(%d), idx(%d), size(%u)",
                  code, user_idx, static_cast<unsigned int>(stream_length));
        } else if (pool_kind == 2 && stream_length > 0x3ffff) {
            cMyTrace trace("virtual void DispatchThread::dispatch(void*)", 178, 5);
            trace("Big Stream Size over code(%d), idx(%d), size(%u)",
                  code, user_idx, static_cast<unsigned int>(stream_length));
        } else if (pool_kind == 0 && stream_length > 0x3fff) {
            cMyTrace trace("virtual void DispatchThread::dispatch(void*)", 168, 5);
            trace("Default Stream Size over code(%d), idx(%d), size(%u)",
                  code, user_idx, static_cast<unsigned int>(stream_length));
        }
        FreeDispatchStream(stream);
        db_shutdown_quiescence::EndActivity(
            db_shutdown_quiescence::kActivityDispatch);
    }
    message_counter::CTimeInterval::Destory(interval);
}


bool StartTaiwanDatabaseRuntime() {
    if (g_taiwan_runtime_started) {
        return true;
    }
    if (GlobalData::server_stop()) {
        return false;
    }
    if (!GlobalData::s_dispatch_thr || !GlobalData::s_db_thr ||
        !GlobalData::s_db_thr_for_logDB ||
        !GlobalData::s_feature_db_thread ||
        !GlobalData::s_feature_db_thread->ready() ||
        !GlobalData::s_accept_thr || !GlobalData::s_net_thr ||
        !GlobalData::s_udp_thr || !GlobalData::s_monitor_tcp_thr) {
        return false;
    }

    bool dispatch_started = false;
    bool udp_started = false;
    bool monitor_started = false;
    bool db_started = false;
    bool log_db_started = false;
    bool native_db_started = false;
    bool accept_started = false;
    bool network_started = false;

    // Preserve the old worker order and insert the native DB worker after the
    // two legacy DB owners.  A successful pthread_create is recorded before
    // packet-pool binding, so a bind failure also joins that worker.
    if (GlobalData::s_dispatch_thr->begin() != 1) {
        goto start_failed;
    }
    dispatch_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_DISPATCH),
            GlobalData::s_dispatch_thr->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_udp_thr->begin() != 1) {
        goto start_failed;
    }
    udp_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_UDP),
            GlobalData::s_udp_thr->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_monitor_tcp_thr->begin() != 1) {
        goto start_failed;
    }
    monitor_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_MONITOR),
            GlobalData::s_monitor_tcp_thr->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_db_thr->begin() != 1) {
        goto start_failed;
    }
    db_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_DB),
            GlobalData::s_db_thr->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_db_thr_for_logDB->begin() != 1) {
        goto start_failed;
    }
    log_db_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_LOGDB),
            GlobalData::s_db_thr_for_logDB->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_feature_db_thread->begin() != 1) {
        goto start_failed;
    }
    native_db_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_FEATURE_DB),
            GlobalData::s_feature_db_thread->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_accept_thr->begin() != 1) {
        goto start_failed;
    }
    accept_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_ACCEPT),
            GlobalData::s_accept_thr->getThreadID())) {
        goto start_failed;
    }

    if (GlobalData::s_net_thr->begin() != 1) {
        goto start_failed;
    }
    network_started = true;
    if (!GlobalData::setCurrentPacketPool(
            static_cast<THREAD_TYPE>(GlobalData::THREAD_NETWORK),
            GlobalData::s_net_thr->getThreadID())) {
        goto start_failed;
    }

    g_taiwan_runtime_started = true;
    return true;

start_failed:
    // Publish the ingress predicate, but keep Dispatch alive as the database
    // response consumer.  The same relative DAG is used for partial startup:
    // ingress -> Exchange -> database fixed point -> DB/LOG -> q6 -> Dispatch.
    GlobalData::set_server_stop(true);
    if (network_started) {
        GlobalData::s_net_thr->join();
    }
    if (accept_started) {
        GlobalData::s_accept_thr->join();
    }
    if (udp_started) {
        GlobalData::s_udp_thr->join();
    }
    if (monitor_started) {
        GlobalData::s_monitor_tcp_thr->join();
    }
    // Exchange begins after this routine on the normal app path, so this is
    // normally a no-op here.  Keeping it in the failure DAG also covers any
    // future caller that starts Exchange before completing this bootstrap.
    if (!exchange_server::StopExchangeServerThreadAfterDrain()) {
        FailClosedDatabaseShutdown("start_failed_exchange");
    }
    {
        DatabaseClosureDomains domains;
        domains.dispatch = dispatch_started;
        domains.db = db_started;
        domains.log_db = log_db_started;
        domains.native_db = native_db_started;
        if (!WaitForDatabaseClosure(domains)) {
            FailClosedDatabaseShutdown("start_failed_database_barrier");
        }
    }
    if (db_started) {
        GlobalData::s_db_thr->requestStopAfterDrain();
    }
    if (log_db_started) {
        GlobalData::s_db_thr_for_logDB->requestStopAfterDrain();
    }
    if (db_started) {
        GlobalData::s_db_thr->join();
    }
    if (log_db_started) {
        GlobalData::s_db_thr_for_logDB->join();
    }
    if (native_db_started) {
        GlobalData::s_feature_db_thread->requestStopAfterDrain();
        GlobalData::s_feature_db_thread->join();
    }
    if (dispatch_started) {
        GlobalData::s_dispatch_thr->requestStopAfterDrain();
        GlobalData::s_dispatch_thr->join();
    }
    return false;
}

void StopTaiwanDatabaseRuntime() {
    if (!g_taiwan_runtime_started) {
        return;
    }
    GlobalData::set_server_stop(true);
    // Strict shutdown DAG:
    // ingress -> queue-5 Exchange -> joint DB closure -> legacy DB/LOG
    // -> native queue 6 -> Dispatch.  Dispatch remains alive through the
    // complete q6->DB_Q->q6 ACK, so final client/inter-server continuations
    // cannot be stranded merely because global stop was published.
    GlobalData::s_net_thr->join();
    GlobalData::s_accept_thr->join();
    GlobalData::s_udp_thr->join();
    GlobalData::s_monitor_tcp_thr->join();
    if (!exchange_server::StopExchangeServerThreadAfterDrain()) {
        FailClosedDatabaseShutdown("normal_exchange");
    }
    {
        DatabaseClosureDomains domains;
        domains.dispatch = true;
        domains.db = true;
        domains.log_db = true;
        domains.native_db = true;
        if (!WaitForDatabaseClosure(domains)) {
            FailClosedDatabaseShutdown("normal_database_barrier");
        }
    }
    GlobalData::s_db_thr->requestStopAfterDrain();
    GlobalData::s_db_thr_for_logDB->requestStopAfterDrain();
    GlobalData::s_db_thr->join();
    GlobalData::s_db_thr_for_logDB->join();
    GlobalData::s_feature_db_thread->requestStopAfterDrain();
    GlobalData::s_feature_db_thread->join();
    GlobalData::s_dispatch_thr->requestStopAfterDrain();
    GlobalData::s_dispatch_thr->join();
    g_taiwan_runtime_started = false;
}
