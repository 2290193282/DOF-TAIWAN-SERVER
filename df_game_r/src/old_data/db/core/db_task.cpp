#include "old_data/db/core/db_task.h"

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/core/db_legacy_character_snapshot.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/db_shutdown_quiescence.h"
#include "old_data/db/anti_cheat/db_report_mannerless_user.h"
#include "old_data/db/anti_cheat/db_report_mannerless_user_upgrade.h"
#include "old_data/db/anti_cheat/db_req_black_count.h"
#include "old_data/db/auction_cashshop/db_load_cerashop_add_restrict_type.h"
#include "old_data/db/auction_cashshop/db_save_cerashop_add_restrict_type.h"
#include "old_data/db/auth_security/db_check_3rd_party_consent.h"
#include "old_data/db/event/advance_altar_db.h"
#include "old_data/db/event/db_event_item_and_levelup_dispatchers.h"
#include "old_data/db/event/db_req_save_crazy_levelup_event.h"
#include "old_data/db/event/db_statistic_advance_altar.h"
#include "old_data/db/legacy_dispatch/db_check_add_friend.h"
#include "old_data/db/legacy_dispatch/db_crashdown_advertisement_dispatch.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_data/db/legacy_dispatch/db_load_server_message.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h"
#include "old_data/db/legacy_dispatch/db_update_server_message.h"
#include "old_data/db/pvp_party/db_online_preliminary_noop_dispatchers.h"
#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"
#include "old_data/db/legacy_dispatch/db_action_point_load.h"
#include "old_data/db/legacy_dispatch/db_action_point_reward_medal.h"
#include "old_data/db/legacy_dispatch/db_action_point_update.h"
#include "old_data/db/anti_cheat/db_account_hack_check_my_account.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/account/db_arrange_user_login_info.h"
#include "old_data/db/avatar_aura/db_avatar_change_owner.h"
#include "old_data/db/avatar_aura/db_avatar_change_stat.h"
#include "old_data/db/avatar_aura/db_avatar_color_change.h"
#include "old_data/db/avatar_aura/db_avatar_extend_period.h"
#include "old_data/db/avatar_aura/db_avatar_item_del.h"
#include "old_data/db/avatar_aura/db_avatar_item_insert.h"
#include "old_data/db/avatar_aura/db_avatar_item_move.h"
#include "old_data/db/avatar_aura/db_avatar_option_change.h"
#include "old_data/db/avatar_aura/db_update_avatar_history.h"
#include "old_data/db/avatar_aura/db_update_avatar_jewel_slot.h"
#include "old_data/db/auction_cashshop/db_auction_sub_money.h"
#include "old_data/db/anti_cheat/db_auto_punished_second_user_data.h"
#include "old_data/db/auth_security/db_cancel_resting_user_restrict.h"
#include "old_data/db/mail/db_change_letter_stat.h"
#include "old_data/db/mail/db_multi_mail_dispatchers.h"
#include "old_data/db/auth_security/db_check_secu_reward.h"
#include "old_data/db/account/clocalchina_db_break_away_reset.h"
#include "old_data/db/account/clocalchina_db_last_day_break_away.h"
#include "old_data/db/inventory/db_create_account_cargo.h"
#include "old_data/db/character/db_check_charac_name.h"
#include "old_data/db/auth_security/db_count_of_human_certify.h"
#include "old_data/db/character/db_create_charac.h"
#include "old_data/db/channel/db_delete_channel.h"
#include "old_data/db/account/db_delete_account_cargo.h"
#include "old_data/db/character/db_delete_charac.h"
#include "old_data/db/event/db_event_account_character_level_up_once_gift.h"
#include "old_data/db/event/db_event_account_fatigue_stat.h"
#include "old_data/db/event/db_event_all_user_gift.h"
#include "old_data/db/event/db_event_attendance.h"
#include "old_data/db/event/db_king_of_pick.h"
#include "old_data/db/event/db_dragon_timeattack_rank.h"
#include "old_data/db/event/db_fatigue_acceleration.h"
#include "old_data/db/event/db_levelup_support_3rd.h"
#include "old_data/db/event/db_satisfaction_survey.h"
#include "old_data/db/event/db_skill_damage_rank_event.h"
#include "old_data/db/event/db_take_magic_lamp.h"
#include "old_data/db/event/db_facebook_like.h"
#include "old_data/db/pvp_party/db_pvp_practice_combo_score.h"
#include "old_data/db/event/db_event_growth_weapon_reset.h"
#include "old_data/db/event/db_event_save_growth_weapon.h"
#include "old_data/db/event/db_event_save_give_grow_creature.h"
#include "old_data/db/event/db_event_update_give_grow_creature.h"
#include "old_data/db/event/db_event_update_growth_weapon_charac.h"
#include "old_data/db/event/db_event_update_growth_weapon_timepiece.h"
#include "old_data/db/legacy_dispatch/db_fatigue_reset_timer.h"
#include "old_data/db/account/db_find_factory_hub.h"
#include "old_data/db/account/db_find_factory_hub_user.h"
#include "old_data/db/anti_cheat/db_forbid_user_to_play.h"
#include "old_data/db/account/db_force_save_simple_sso.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_data/db/dungeon/db_tod_update_user_state.h"
#include "old_data/db/dungeon/db_update_fair_pvp_score.h"
#include "old_data/db/mail/db_mailbox_send_new_mail_with_dbwork.h"
#include "old_data/db/auction_cashshop/db_get_premium_selected_cube_info.h"
#include "old_data/db/character/db_get_user_regdate.h"
#include "old_data/db/anti_cheat/db_hades_punish.h"
#include "old_data/db/anti_cheat/db_hack_ip_save.h"
#include "old_data/db/inventory/db_insert_random_option_item_inform.h"
#include "old_data/db/account/db_insert_account_first_login_daily_flag.h"
#include "old_data/db/account/db_insert_account_first_login_flag.h"
#include "old_data/db/channel/db_insert_channel.h"
#include "old_data/db/anti_cheat/db_insert_auto_punish_first_user.h"
#include "old_data/db/anti_cheat/db_insert_auto_punish_second_user.h"
#include "old_data/db/avatar_aura/db_insert_cera_item_history.h"
#include "old_data/db/creature/db_insert_creature_item.h"
#include "old_data/db/event/db_insert_attendance_event.h"
#include "old_data/db/statistics_log/db_insert_ontime_reward_user_log.h"
#include "old_data/db/statistics_log/db_insert_circulation_statistic.h"
#include "old_data/db/statistics_log/db_insert_query_counter_db.h"
#include "old_data/db/statistics_log/db_insert_query_counter_logdb.h"
#include "old_data/db/statistics_log/db_insert_party_type.h"
#include "old_data/db/statistics_log/db_insert_pvp_map.h"
#include "old_data/db/statistics_log/db_insert_pvp_player_rate.h"
#include "old_data/db/statistics_log/db_insert_queue_size_log.h"
#include "old_data/db/statistics_log/db_insert_response_time.h"
#include "old_data/db/statistics_log/db_insert_server_load_statistic.h"
#include "old_data/db/inventory/db_insert_ticket.h"
#include "old_data/db/statistics_log/db_insert_unlimit_support_log.h"
#include "old_data/db/statistics_log/db_insert_user_play_time.h"
#include "old_data/db/statistics_log/db_insert_value_statistic.h"
#include "old_data/db/legacy_dispatch/db_last_msg_db.h"
#include "old_data/db/legacy_dispatch/db_last_msg_log_db.h"
#include "old_data/db/statistics_log/db_log_buy_item.h"
#include "old_data/db/statistics_log/db_timer_statistics_log.h"
#include "old_data/db/statistics_log/db_log_ingame_advertisement.h"
#include "old_data/db/inventory/db_load_account_cargo.h"
#include "old_data/db/auction_cashshop/db_load_auto_market_conditions_control.h"
#include "old_data/db/auction_cashshop/db_billing_record.h"
#include "old_data/db/account/db_load_break_away_prevent_data.h"
#include "old_data/db/account/db_load_break_away_reward_data.h"
#include "old_data/db/dungeon/db_load_blood_best_record.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"
#include "old_data/db/character/db_load_charac.h"
#include "old_data/db/anti_cheat/db_load_clean_pad_point.h"
#include "old_data/db/dungeon/db_load_dimension.h"
#include "old_data/db/character/db_load_etc.h"
#include "old_data/db/quest/db_load_expert_job.h"
#include "old_data/db/game_option/db_load_game_option.h"
#include "old_data/db/inventory/db_load_inventory.h"
#include "old_data/db/dungeon/db_load_kill_monster_info.h"
#include "old_data/db/auction_cashshop/db_load_mileage.h"
#include "old_data/db/character/db_load_no_cache_data.h"
#include "old_data/db/dungeon/db_load_dungeon_clear.h"
#include "old_data/db/private_store/db_load_private_store.h"
#include "old_data/db/account/db_load_pu_user_check.h"
#include "old_data/db/quest/db_load_quest.h"
#include "old_data/db/inventory/db_load_ticket.h"
#include "old_data/db/account/db_login.h"
#include "old_data/db/mail/db_mailbox_req_check_receiver.h"
#include "old_data/db/mail/db_mailbox_req_delete.h"
#include "old_data/db/mail/db_mailbox_req_list.h"
#include "old_data/db/mail/db_mailbox_req_send.h"
#include "old_data/db/mercenary/db_mercenary_log_info.h"
#include "old_data/db/auth_security/db_mouse_register.h"
#include "old_data/db/auth_security/db_passpad_update_cancel_cnt.h"
#include "old_data/db/auth_security/db_passpad_update_fail_cnt.h"
#include "old_data/db/anti_cheat/db_req_save_hack_user_punish.h"
#include "old_data/db/equipment_upgrade/db_req_item_upgrade_log.h"
#include "old_data/db/anti_cheat/db_reload_auto_punish_rule_back_ip.h"
#include "old_data/db/anti_cheat/db_reload_auto_punish_rule_hack_type.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_data/db/character/db_request_tag_charac_info.h"
#include "old_data/db/character/db_req_change_charac_name.h"
#include "old_data/db/character/db_req_query_charac_info.h"
#include "old_data/db/mail/db_req_query_charac_info_mailbox.h"
#include "old_data/db/auction_cashshop/db_remove_featured_goods_idx.h"
#include "old_data/db/inventory/db_save_account_cargo.h"
#include "old_data/db/auction_cashshop/db_save_featured_goods_idx.h"
#include "old_data/db/anti_cheat/db_save_cleanpad_point.h"
#include "old_data/db/dungeon/db_save_pvp_mission_list.h"
#include "old_data/db/account/db_save_pu_user_check.h"
#include "old_data/db/pvp_party/db_save_pvp.h"
#include "old_data/db/channel/db_save_channel_user_level.h"
#include "old_data/db/account/db_save_account_info.h"
#include "old_data/db/dungeon/db_save_best_clear_time.h"
#include "old_data/db/dungeon/db_save_blood_dungeon.h"
#include "old_data/db/event/db_save_pc_room_play_time_event.h"
#include "old_data/db/character/db_save_charac_info4guild.h"
#include "old_data/db/character/db_save_charac_inform_notice.h"
#include "old_data/db/character/db_save_character_option.h"
#include "old_data/db/character/db_save_charac.h"
#include "old_data/db/anti_cheat/db_save_client_hack_log.h"
#include "old_data/db/dungeon/db_save_dungeon_clear.h"
#include "old_data/db/inventory/db_save_expand_equipslot.h"
#include "old_data/db/quest/db_save_expert_job.h"
#include "old_data/db/channel/db_save_gamechannel.h"
#include "old_data/db/auction_cashshop/db_save_gold_zero_trade_log.h"
#include "old_data/db/game_option/db_save_game_option_1.h"
#include "old_data/db/game_option/db_save_game_option_2.h"
#include "old_data/db/game_option/db_save_game_option_chatting_emoticon.h"
#include "old_data/db/game_option/db_save_game_option_quick_chatting.h"
#include "old_data/db/inventory/db_save_inven.h"
#include "old_data/db/inventory/db_save_item_dictionary_flag.h"
#include "old_data/db/dungeon/db_save_kill_monster_info.h"
#include "old_data/db/skill/db_save_skill.h"
#include "old_data/db/pvp_party/db_save_user_handicap.h"
#include "old_data/db/legacy_dispatch/db_legacy_stat_dispatchers.h"
#include "old_data/db/pvp_party/db_pvp_stat_dispatchers.h"
#include "old_data/db/auction_cashshop/db_save_mileage.h"
#include "old_data/db/auction_cashshop/db_save_premium_info.h"
#include "old_data/db/private_store/db_save_private_store.h"
#include "old_data/db/pvp_party/db_save_rank.h"
#include "old_data/db/inventory/db_save_redeem_item_list.h"
#include "old_data/db/anti_cheat/db_save_same_mid_other_char_login_hack.h"
#include "old_data/db/auth_security/db_save_proxy_log.h"
#include "old_data/db/inventory/db_save_title_book.h"
#include "old_data/db/inventory/db_save_achievement.h"
#include "old_data/db/character/db_save_charac_slot_extend.h"
#include "old_data/db/event/db_save_give_avenger_title_flag.h"
#include "old_data/db/event/db_save_event_levelup_support.h"
#include "old_data/db/mercenary/db_save_mercenary_data.h"
#include "old_data/db/event/db_save_object_bring_up.h"
#include "old_data/db/auth_security/db_secu_service_reward.h"
#include "old_data/db/auth_security/db_secu_service_update_validity_time.h"
#include "old_data/db/auth_security/db_security_card_add_cert_cnt.h"
#include "old_data/db/auth_security/db_security_card_activate.h"
#include "old_data/db/auth_security/db_security_card_disuse.h"
#include "old_data/db/auth_security/db_security_card_issue.h"
#include "old_data/db/auth_security/db_security_card_retransfer.h"
#include "old_data/db/auth_security/db_security_card_update_cancel_cnt.h"
#include "old_data/db/auth_security/db_security_card_update_fail_cnt.h"
#include "old_data/db/auth_security/db_security_card_update_validity_time.h"
#include "old_data/db/auth_security/db_select_allow_proxy_user.h"
#include "old_data/db/character/db_select_charac_mid.h"
#include "old_data/db/event/db_select_level_up_reward.h"
#include "old_data/db/core/db_suicide_when_queue_is_empty.h"
#include "old_data/db/core/db_terminate_server.h"
#include "old_data/db/channel/db_update_channel.h"
#include "old_data/db/avatar_aura/db_update_aura_avatar_option.h"
#include "old_data/db/auction_cashshop/db_update_auto_market_conditions_control.h"
#include "old_data/db/inventory/db_update_avatar_item_lock.h"
#include "old_data/db/character/db_update_booster_gage_data.h"
#include "old_data/db/event/db_update_bingo_event.h"
#include "old_data/db/dungeon/db_update_blood_best_record.h"
#include "old_data/db/event/db_update_blue_marble_enter_count.h"
#include "old_data/db/dungeon/db_update_break_away_clear_add_info.h"
#include "old_data/db/dungeon/db_update_break_away_dungeon_clear_cnt.h"
#include "old_data/db/account/db_update_break_away_ruck_point.h"
#include "old_data/db/account/db_update_break_away_uv_list.h"
#include "old_data/db/character/db_update_charac_view.h"
#include "old_data/db/character/db_update_character_day.h"
#include "old_data/db/event/db_update_chuseok_package_purchase.h"
#include "old_data/db/character/db_update_charac_play_time.h"
#include "old_data/db/skill/db_update_combo_skill.h"
#include "old_data/db/creature/db_update_creature_item.h"
#include "old_data/db/creature/db_update_creature_item_list.h"
#include "old_data/db/inventory/db_update_creature_item_lock.h"
#include "old_data/db/channel/db_update_detail_channel.h"
#include "old_data/db/game_option/db_update_dimension.h"
#include "old_data/db/game_option/db_update_cool_time_item.h"
#include "old_data/db/game_option/db_update_effect_item.h"
#include "old_data/db/skill/db_update_expert_job_statistic.h"
#include "old_data/db/character/db_update_grow_type.h"
#include "old_data/db/event/db_update_happy_bean_info.h"
#include "old_data/db/event/db_update_ingame_event_history.h"
#include "old_data/db/character/db_update_gm_data.h"
#include "old_data/db/inventory/db_update_inventory_capacity.h"
#include "old_data/db/skill/db_update_item_making_skill.h"
#include "old_data/db/event/db_update_add_item_count_value.h"
#include "old_data/db/event/db_update_item_count_value.h"
#include "old_data/db/inventory/db_update_item_lock.h"
#include "old_data/db/statistics_log/db_update_item_drop_statistics.h"
#include "old_data/db/inventory/db_update_limit_item_usage.h"
#include "old_data/db/legacy_dispatch/db_update_login.h"
#include "old_data/db/account/db_update_logout.h"
#include "old_data/db/legacy_dispatch/db_update_nat_type.h"
#include "old_data/db/legacy_dispatch/db_update_nat_type_user.h"
#include "old_data/db/npc/db_update_npc_relationship.h"
#include "old_data/db/skill/db_update_one_day_lethe_skill.h"
#include "old_data/db/auction_cashshop/db_update_purchase_cash_item_bonus.h"
#include "old_data/db/pvp_party/db_update_pvp_grade.h"
#include "old_data/db/auction_cashshop/db_update_cerashop_purchase_count.h"
#include "old_data/db/npc/db_update_quest_shop_buying_status_list.h"
#include "old_data/db/auction_cashshop/db_update_gift_count.h"
#include "old_data/db/account/db_update_user_fatigue_quantity.h"
#include "old_data/db/account/db_update_user_gift_fatigue_quantity.h"
#include "old_data/db/statistics_log/db_update_vending_machine_statistic.h"
#include "old_data/db/inventory/db_upgrade_account_cargo.h"
#include "old_data/db/statistics_log/db_update_query_count_db_logdb.h"
#include "old_data/db/statistics_log/db_update_query_count_timer_db.h"
#include "old_data/db/statistics_log/db_update_query_count_timer_logdb.h"
#include "old_data/db/event/db_update_ontime_last_recv_idx.h"
#include "old_data/db/dungeon/db_update_revenge_data.h"
#include "old_data/db/quest/db_update_quest.h"
#include "old_data/db/account/db_update_return_user_first_login.h"
#include "old_data/db/event/db_update_seria_room_animation_deco.h"
#include "old_data/db/event/db_update_school_point.h"
#include "old_data/db/creature/db_swap_creature_item.h"
#include "old_data/db/pvp_party/db_guild_exp_book_delete.h"
#include "old_data/db/auth_security/db_write_geo_reject.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/taiwan/taiwan_db_dispatcher.h"
#include "common/manager/global_data.h"
#include "common/manager/os_api.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_pool.h"

#include <cstring>
#include <unistd.h>


typedef char DBTask_Size_Must_Be_0xD54[(sizeof(DBTask) == 0xD54) ? 1 : -1];

// IDA_EA: 0x83FBFC6
// IDA_NAME: _ZN6DBTaskC2Ev
// DEMANGLED: DBTask::DBTask(void)
DBTask::DBTask() : queue_idx_(MsgQueueMgr::DB_Q), dispatchers_() {
    RegistDispatcher();
}

// IDA_EA: 0x83FBFDA
// IDA_NAME: _ZN6DBTaskD2Ev
// DEMANGLED: DBTask::~DBTask()
DBTask::~DBTask() {
    UnregistDispatcher();
}

// IDA_EA: 0x83FBFEE
// IDA_NAME: _ZN6DBTask16RegistDispatcherEv
// DEMANGLED: DBTask::RegistDispatcher(void)
int DBTask::RegistDispatcher() {
    for (int index = 0; index <= 851; ++index) {
        dispatchers_[index] = 0;
    }

    dispatchers_[3] = new DB_Login();
    dispatchers_[4] = new DB_CreateCharac();
    dispatchers_[5] = new DB_DeleteCharac();
    dispatchers_[6] = new DB_UpdateCharacView();
    dispatchers_[7] = new DB_LoadCharac();
    dispatchers_[8] = new DB_LoadInventory();
    dispatchers_[10] = new DB_LoadQuest();
    dispatchers_[17] = new APSystem::DB_LoadActionPoint();
    dispatchers_[19] = new APSystem::DB_UpdateActionPoint();
    dispatchers_[20] = new APSystem::DB_LoadRewardMedal();
    dispatchers_[21] = new APSystem::DB_UpdateRewardMedal();
    dispatchers_[793] = new DB_EventAccountCharacterLevelUpOnceGift();
    dispatchers_[32] = new DB_LoadEtc();
    dispatchers_[40] = new DB_InsertTicket();
    dispatchers_[11] = new DB_LoadTicket();
    dispatchers_[12] = new DB_LoadDungeonClear();
    dispatchers_[34] = new DB_SaveCharac();
    dispatchers_[35] = new DB_SaveInven();
    dispatchers_[36] = new DB_SaveSkill();
    dispatchers_[37] = new DB_SavePvp();
    dispatchers_[38] = new DB_SaveRank();
    dispatchers_[39] = new DB_SaveDungeonClear();
    dispatchers_[41] = new DB_UpdateQuest();
    dispatchers_[42] = new DB_UpdateGrowType();
    dispatchers_[43] = new DB_UpdateLogin();
    dispatchers_[44] = new DB_UpdateLogout();
    dispatchers_[45] = new DB_InsertChannel();
    dispatchers_[46] = new DB_UpdateChannel();
    dispatchers_[47] = new DB_DeleteChannel();
    dispatchers_[49] = new DB_AvatarItemInsert();
    dispatchers_[50] = new DB_AvatarItemMove();
    dispatchers_[51] = new DB_AvatarItemDel();
    dispatchers_[52] = new DB_AvatarExtendPeriod();
    dispatchers_[53] = new DB_AvatarChangeOwner();
    dispatchers_[54] = new DB_AvatarChangeStat();
    dispatchers_[55] = new DB_StatBreakJar();
    dispatchers_[56] = new DB_StatComposition();
    dispatchers_[57] = new DB_StatCoinUse();
    dispatchers_[58] = new DB_StatPvpPlay();
    dispatchers_[59] = new DB_StatGameRank();
    dispatchers_[60] = new DB_StatDungeonClear();
    dispatchers_[312] = new DB_SecurityCardRetransfer();
    dispatchers_[572] = new DB_UpdatePurchaseCashItemBonus();
    dispatchers_[63] = new DB_TerminateServer();
    dispatchers_[576] = new DB_UpdateItemCountValue();
    dispatchers_[321] = new DB_LoadBreakAwayPreventData();
    dispatchers_[322] = new DB_LoadBreakAwayRewardData();
    dispatchers_[325] = new DB_UpdateBreakAwayDungeonClearCnt();
    dispatchers_[326] = new DB_UpdateBreakAwayAccruedCera();
    dispatchers_[580] = new DB_UpdateRevengeData();
    dispatchers_[330] = new DB_UpdateGMData();
    dispatchers_[78] = new DB_FatigueResetTimer();
    dispatchers_[80] = new DB_CrashDownReport();
    dispatchers_[593] = new DB_LogBuyItem();
    dispatchers_[82] = new DB_SaveFeaturedGoodsIdx();
    dispatchers_[83] = new DB_RemoveFeaturedGoodsIdx();
    dispatchers_[84] = new DB_SavePremiumInfo();
    dispatchers_[346] = new DB_LoadAutoMarketConditionsControl();
    dispatchers_[347] = new DB_UpdateAutoMarketConditionsControl();
    dispatchers_[99] = new DB_SaveCharacInfo4Guild();
    dispatchers_[105] = new DB_StatUseShusiaService();
    dispatchers_[106] = new DB_UpdateDetailChannel();
    dispatchers_[107] = new DB_SaveChannelUserLevel();
    dispatchers_[119] = new DB_SavePrivateStore();
    dispatchers_[120] = new DB_LoadPrivateStore();
    dispatchers_[121] = new DB_SaveGamechannel();
    dispatchers_[123] = new DB_UpdateQueryCountTimerLogdb();
    dispatchers_[124] = new DB_UpdateQueryCountTimerDb();
    dispatchers_[125] = new DB_UpdateQueryCountDbLogdb();
    dispatchers_[126] = new DB_MailBox_Req_CheckReceiver();
    dispatchers_[127] = new DB_MailBox_Req_List();
    dispatchers_[128] = new DB_MailBox_Req_Send();
    dispatchers_[129] = new DB_MailBox_Req_Delete();
    dispatchers_[131] = new DB_UpdateNatType();
    dispatchers_[132] = new DB_UpdateNatTypeUser();
    dispatchers_[135] = new DB_InsertCreatureItem();
    dispatchers_[137] = new DB_UpdateCreatureItemList();
    dispatchers_[138] = new DB_UpdateCreatureItem();
    dispatchers_[140] = new DB_SwapCreatureItem();
    dispatchers_[147] = new DB_ForbidUserToPlay();
    dispatchers_[149] = new DB_LastMsgDb();
    dispatchers_[150] = new DB_LastMsgLogDb();
    dispatchers_[663] = new DB_UpdateOntimeLastRecvIdx();
    dispatchers_[665] = new DB_InsertRandomOptionItemInform();
    dispatchers_[154] = new DB_SaveClientHackLog();
    dispatchers_[155] = new DB_InsertQueueSizeLog();
    dispatchers_[157] = new DB_SaveSameMIDOtherCharLoginHack();
    dispatchers_[158] = new DB_SaveProxyLog();
    dispatchers_[161] = new DB_SaveGoldZeroTradeLog();
    dispatchers_[163] = new DB_MouseRegister();
    dispatchers_[164] = new DB_PassPadUpdateFailCnt();
    dispatchers_[165] = new DB_PassPadUpdateCancelCnt();
    dispatchers_[176] = new DB_BillingRecord();
    dispatchers_[683] = new DB_CheckSecuReward();
    dispatchers_[181] = new DB_ReqSaveHackUserPunish();
    dispatchers_[182] = new DB_InsertUserPlayTime();
    dispatchers_[187] = new DB_UpdateHappyBeanInfo();
    dispatchers_[190] = new DB_InsertQueryCounterDb();
    dispatchers_[191] = new DB_InsertQueryCounterLogdb();
    dispatchers_[192] = new DB_ReqItemUpgradeLog();
    dispatchers_[194] = new DB_ChangeLetterStat();
    dispatchers_[195] = new DB_ReqChangeCharacName();
    dispatchers_[196] = new DB_ReqQueryCharacInfo();
    dispatchers_[197] = new DB_MailBoxSendNewMailWithDBWork();
    dispatchers_[206] = new DB_SaveUserHandicap();
    dispatchers_[207] = new DB_ForceSaveSimpleSSO();
    dispatchers_[209] = new DB_UpdateGiftCount();
    dispatchers_[210] = new DB_UpdateItemDropStatistics();
    dispatchers_[212] = new DB_UpdatePvPGrade();
    dispatchers_[728] = new DB_UpdateBoosterGageData();
    dispatchers_[213] = new DB_InsertValueStatistic();
    dispatchers_[733] = new CLocalChina_DB_BreakAwayReset();
    dispatchers_[734] = new CLocalChina_DB_LastDay_BreakAway();
    dispatchers_[226] = new DB_GetDeathTowerBestRecord();
    dispatchers_[238] = new DB_ArrangeUserLoginInfo();
    dispatchers_[253] = new DB_SecurityCardIssue();
    dispatchers_[254] = new DB_SecurityCardDisuse();
    dispatchers_[255] = new DB_SecurityCardActivate();
    dispatchers_[256] = new DB_SecurityCardUpdateValidityTime();
    dispatchers_[257] = new DB_SecurityCardUpdateFailCnt();
    dispatchers_[258] = new DB_SecurityCardUpdateCancelCnt();
    dispatchers_[259] = new DB_InsertPartyType();
    dispatchers_[260] = new DB_InsertPVPPlayerRate();
    dispatchers_[261] = new DB_InsertPVPMap();
    dispatchers_[264] = new DB_ReloadAutoPunishRuleBackIP();
    dispatchers_[265] = new DB_ReloadAutoPunishRuleHackType();
    dispatchers_[266] = new DB_InsertAutoPunishFirstUser();
    dispatchers_[267] = new DB_InsertAutoPunishSecondUser();
    dispatchers_[283] = new DB_WriteGeoReject();
    dispatchers_[290] = new DB_InsertUnlimitSupportLog();
    dispatchers_[295] = new DB_LoadCleanPadPoint();
    dispatchers_[340] = new DB_SaveCleanpadPoint();
    dispatchers_[297] = new DB_UpdateSchoolPoint();
    dispatchers_[273] = new DB_CountOfHumanCertify();
    dispatchers_[302] = new DB_LoadGameOption();
    dispatchers_[304] = new DB_SaveGameOption_1();
    dispatchers_[305] = new DB_SaveGameOption_2();
    dispatchers_[306] = new DB_UpdateCoolTimeItem();
    dispatchers_[311] = new DB_UpdateEffectItem();
    dispatchers_[313] = new DB_UpdateAvatarJewelSlot();
    dispatchers_[319] = new DB_LoadBloodBestRecord();
    dispatchers_[320] = new DB_UpdateBloodBestRecord();
    dispatchers_[324] = new DB_UpdateBreakAwayClearAddInfo();
    dispatchers_[327] = new DB_UpdateBreakAwayUvList();
    dispatchers_[329] = new DB_UpdateBreakAwayRuckPoint();
    dispatchers_[334] = new DB_LoadDimension();
    dispatchers_[335] = new DB_UpdateDimension();
    dispatchers_[336] = new DB_SaveAccountInfo();
    dispatchers_[337] = new DB_InsertCirculationStatistic();
    dispatchers_[348] = new DB_UpdateOneDayLetheSkill();
    dispatchers_[355] = new DB_UpdateVendingMachineStatistic();
    dispatchers_[366] = new DB_UpdateCharacterDay();
    dispatchers_[367] = new DB_UpdateNPCRelationship();
    dispatchers_[384] = new DB_LoadExpertJob();
    dispatchers_[385] = new DB_SaveExpertJob();
    dispatchers_[386] = new DB_UpdateExpertJobStatistic();
    dispatchers_[387] = new DB_InsertServerLoadStatistic();
    dispatchers_[394] = new DB_FindFactoryHub();
    dispatchers_[380] = new DB_SecurityCardAddCertCnt();
    dispatchers_[395] = new DB_AccountHack_CheckMyAccount();
    dispatchers_[400] = new DB_HackIPSave();
    dispatchers_[407] = new DB_SaveBloodDungeon();
    dispatchers_[405] = new DB_CancelRestingUserRestrict();
    dispatchers_[413] = new DB_SecuServiceReward();
    dispatchers_[416] = new DB_Find_Factory_Hub_User();
    dispatchers_[423] = new DB_LoadNoCacheData();
    dispatchers_[440] = new DB_UpdateItemLock();
    dispatchers_[449] = new DB_UpdateCreatureItemLock();
    dispatchers_[450] = new DB_UpdateAvatarItemLock();
    dispatchers_[457] = new DB_SecuServiceUpdateValidityTime();
    dispatchers_[458] = new DB_AutoPunishedSecondUserData();
    dispatchers_[500] = new DB_SaveRedeemItemList();
    dispatchers_[494] = new DB_CreateAccountCargo();
    dispatchers_[495] = new DB_UpgradeAccountCargo();
    dispatchers_[496] = new DB_DeleteAccountCargo();
    dispatchers_[497] = new DB_SaveAccountCargo();
    dispatchers_[498] = new DB_LoadAccountCargo();
    dispatchers_[508] = new DB_SaveMileage();
    dispatchers_[509] = new DB_LoadMileage();
    dispatchers_[521] = new DB_GuildExpBookDelete();
    dispatchers_[531] = new DB_UpdateChuseokPackagePurchase();
    dispatchers_[543] = new DB_UpdateReturnUserFirstLogin();
    dispatchers_[569] = new DB_SaveGameOption_QuickChatting();
    dispatchers_[577] = new DB_UpdateLimitItemUsage();
    dispatchers_[590] = new DB_UpdateCharacPlayTime();
    dispatchers_[606] = new DB_InsertAccountFirstLoginFlag();
    dispatchers_[609] = new DB_UpdateFairPvPScore();
    dispatchers_[611] = new DB_SavePvPMissionList();
    dispatchers_[620] = new DB_InsertResponseTime();
    dispatchers_[621] = new DB_SaveExpandEquipslot();
    dispatchers_[622] = new DB_UpdateItemMakingSkill();
    dispatchers_[632] = new DB_TOD_Update_UserState();
    dispatchers_[633] = new DB_UpdateQuestShopBuyingStatusList();
    dispatchers_[640] = new DB_UpdateAddItemCountValue();
    dispatchers_[641] = new DB_SaveKillMonsterInfo();
    dispatchers_[642] = new DB_LoadKillMonsterInfo();
    dispatchers_[645] = new DB_SaveTitleBook();
    dispatchers_[646] = new DB_SaveAchievement();
    dispatchers_[647] = new DB_UpdateUserFatigueQuantity();
    dispatchers_[648] = new DB_UpdateUserGiftFatigueQuantity();
    dispatchers_[650] = new DB_SaveCharacSlotExtend();
    dispatchers_[653] = new DB_SaveGiveAvengerTitleFlag();
    dispatchers_[654] = new DB_SelectLevelUpReward();
    dispatchers_[664] = new DB_InsertOnTimeRewardUserLog();
    dispatchers_[685] = new DB_UpdateInGameEventHistory();
    dispatchers_[674] = new DB_CheckCharacName();
    dispatchers_[673] = new DB_GetUserRegdate();
    dispatchers_[686] = new DB_UpdateInventoryCapacity();
    dispatchers_[687] = new DB_UpdateAuraAvatarOption();
    dispatchers_[688] = new DB_UpdateAvatarHistory();
    dispatchers_[689] = new DB_InsertCeraItemHistory();
    dispatchers_[690] = new DB_SaveItemDictionaryFlag();
    // IDA raw table slot 692 includes DBTask's queue-index word at slot zero.
    dispatchers_[691] = new DB_SaveMercenaryData();
    dispatchers_[693] = new DB_SaveCharacterOption();
    dispatchers_[694] = new DB_HadesPunish();
    dispatchers_[696] = new DB_MercenaryLogInfo();
    dispatchers_[697] = new DB_SelectCharacMid();
    dispatchers_[707] = new DB_AvatarOptionChange();
    // IDA raw table slot 715 includes DBTask's queue-index word at slot zero.
    dispatchers_[714] = new DB_SaveObjectBringUp();
    dispatchers_[727] = new DB_SaveGameOption_ChattingEmoticon();
    dispatchers_[729] = new DB_UpdateCeraShopPurchaseCount();
    dispatchers_[732] = new DB_AradDatabaseDispatcher();
    dispatchers_[737] = new DB_LoadPuUserCheck();
    dispatchers_[738] = new DB_SavePuUserCheck();
    dispatchers_[765] = new DB_RequestCharacSkillInfo();
    dispatchers_[766] = new DB_RequestTagCharacInfo();
    dispatchers_[767] = new DB_SavePcRoomPlayTimeEvent();
    dispatchers_[768] = new DB_SaveBestClearTime();
    dispatchers_[777] = new DB_AuctionSubMoney();
    dispatchers_[780] = new DB_UpdateBlueMarbleEnterCount();
    dispatchers_[782] = new DB_InsertAttendanceEvent();
    dispatchers_[502] = new DB_ReqQueryCharacInfoMailbox();
    dispatchers_[785] = new DB_SaveCharacInformNotice();
    dispatchers_[787] = new DB_EventSaveGrowthWeapon();
    dispatchers_[788] = new DB_EventUpdateGrowthWeaponCharac();
    dispatchers_[789] = new DB_EventUpdateGrowthWeaponTimepiece();
    dispatchers_[790] = new DB_EventGrowthWeaponReset();
    dispatchers_[791] = new DB_EventSaveGiveGrowCreature();
    dispatchers_[792] = new DB_EventUpdateGiveGrowCreature();
    dispatchers_[794] = new DB_AvatarColorChange();
    dispatchers_[797] = new DB_UpdateComboSkill();
    dispatchers_[798] = new DB_UpdateSeriaRoomAnimationDeco();
    dispatchers_[799] = new DB_UpdateBingoEvent();
    dispatchers_[800] = new DB_LoadCharacHotKeyOption();
    dispatchers_[812] = new DB_LogIngameAdvertisement();
    dispatchers_[834] = new DB_InsertAccountFirstLoginDailyFlag();
    // IDA 0x83FEFEF stores DB_DispatcherTaiwan at this + 0xD3C:
    // DBTask's dispatcher array begins at +0x04, therefore the canonical
    // outer Taiwan route is slot 846 (0x34e), not the unrelated slot 78.
    dispatchers_[846] = new DB_DispatcherTaiwan();
    dispatchers_[847] = new DB_SaveEventLevelupSupport();
    dispatchers_[850] = new DB_SelectAllowProxyUser();

    // The following registrations are the original DBTask slots recovered
    // from IDA 0x83FBFEE.  Their concrete providers already exist in this
    // tree; keeping these routes absent causes valid queued work to be
    // silently dropped by DBProcess.
    dispatchers_[25] = new advancealtar::DB_Load();
    dispatchers_[26] = new advancealtar::DB_Update();
    dispatchers_[73] = new DB_GenCeraTicket();
    dispatchers_[81] = new DB_SuicideWhenQueueIsEmpty();
    dispatchers_[175] = new DB_UpdateDungeonDataStatisticLogdb();
    dispatchers_[179] = new DB_ReqBlackCount();
    dispatchers_[180] = new DB_ReqSaveCrazyLevelupEvent();
    dispatchers_[202] = new DB_ReportMannerlessUser_Upgrade();
    dispatchers_[204] = new DB_UpdateClientCrashDownLog();
    dispatchers_[208] = new DB_UpdateMemberDoubtTrade();
    dispatchers_[224] = new DB_InsertDungeonEntrance();
    dispatchers_[225] = new DB_InsertDungeonEntrance_hour();
    dispatchers_[235] = new DB_CheckDoubleGuildName();
    dispatchers_[236] = new DB_CheckDoubleGuildAddress();
    dispatchers_[262] = new DB_LoadDnfHackLog();
    dispatchers_[263] = new DB_InsertLevelDungeonPlay();
    dispatchers_[274] = new DB_Insert0712WinterEventCoupon();
    dispatchers_[275] = new DB_Insert0712WinterEventCoupon();
    dispatchers_[278] = new DB_InsertDungeonExp();
    dispatchers_[279] = new DB_InsertArchieveEventLog();
    dispatchers_[299] = new DB_GoblinPadUpdateReward();
    dispatchers_[300] = new DB_GoblinPadUpdateValidityTime();
    dispatchers_[338] = new DB_BlackIPMonitor();
    dispatchers_[341] = new DB_BlackIPMonitorPartLoad();
    dispatchers_[356] = new DB_BlackipPrivateSecurityValuation();
    dispatchers_[357] = new DB_LoadPowerWarStatueRanker();
    dispatchers_[358] = new DB_LoadPowerWarStatueInfo();
    dispatchers_[364] = new DB_UpdateServerMessage();
    dispatchers_[365] = new DB_LoadServerMessage();
    dispatchers_[411] = new DB_LoadPowerWarInfo();
    dispatchers_[415] = new DB_SaveConditionEventInfo();
    dispatchers_[421] = new DB_DeletePowerWarStatueMessage();
    dispatchers_[451] = new DB_UpdateMemberPlayInfoPcRoom();
    dispatchers_[452] = new DB_UpdateCompoundEmblemStatics();
    dispatchers_[468] = new DB_SaveProperDungeonClearCount();
    dispatchers_[469] = new DB_LoadPvPBuddy();
    dispatchers_[475] = new DB_InsertPvPBuddy();
    dispatchers_[476] = new DB_DeletePvPBuddy();
    dispatchers_[477] = new DB_UpdatePvPBuddy();
    dispatchers_[479] = new DB_DeleteGuild();
    dispatchers_[480] = new DB_LoadPvPLiveEventData();
    dispatchers_[481] = new DB_UpdatePvPLiveEventData();
    dispatchers_[486] = new DB_LoadOnlinePreliminaryMember();
    dispatchers_[487] = new DB_UpdateOnlinePreliminaryMember();
    dispatchers_[488] = new DB_LoadOnlinePreliminaryTeam();
    dispatchers_[489] = new DB_UpdateOnlinePreliminaryTeam();
    dispatchers_[491] = new DB_UpdateOnlinePreliminaryMemberMatchList();
    dispatchers_[492] = new DB_LoadOnlinePreliminaryTeamMatchList();
    dispatchers_[493] = new DB_UpdateOnlinePreliminaryTeamMatchList();
    dispatchers_[524] = new DB_InsertPoliceSaveChatting();
    dispatchers_[534] = new DB_CheckJoinGuild();
    dispatchers_[535] = new DB_RequestJoinGuild();
    dispatchers_[536] = new DB_CancelJoinGuild();
    dispatchers_[537] = new DB_JoinGuildInfo();
    dispatchers_[538] = new DB_GuildJoinList();
    dispatchers_[540] = new DB_DenyJoinGuild();
    dispatchers_[583] = new DB_UpdateGoblinPadAdvicePwdRewardTime();
    dispatchers_[629] = new DB_TOD_Random_Select_UserAPC();
    dispatchers_[631] = new DB_TOD_Reset_UserAPC();
    dispatchers_[668] = new DB_StatisticCeraShopGiftLog();
    dispatchers_[684] = new DB_UpdateFatigueAttendance();
    dispatchers_[695] = new DB_Check3rdPartyConcent();
    dispatchers_[757] = new DB_SaveInformNotice();
    dispatchers_[760] = new DB_NoticeIngameAdvertisement();
    dispatchers_[796] = new DB_TestTradeRestrict();
    dispatchers_[814] = new DB_LogCeraShopError();
    dispatchers_[818] = new DB_GetPremiumSelectedCubeInfo();
    dispatchers_[827] = new advancealtar::DB_StatisticAdvanceAltar();
    dispatchers_[830] = new DB_CheckAddFriend();
    dispatchers_[832] = new DB_SaveCerashopAddRestrictType();
    dispatchers_[833] = new DB_LoadCerashopAddRestrictType();

    // The last raw DBTask registrations previously had no canonical provider.
    // Keep the implementation in the corresponding DB feature directory;
    // these statements are only the original slot-to-class binding.
    dispatchers_[76] = new DB_SaveEventItem();
    dispatchers_[77] = new DB_RemoveEventItem();
    dispatchers_[103] = new DB_StatDungeonFail();
    dispatchers_[104] = new DB_StatPVPPlay2();
    dispatchers_[142] = new DB_GatheringParyStatus();
    dispatchers_[203] = new DB_ReportMannerlessUser();
    dispatchers_[205] = new DB_TimerStatisticsLog();
    dispatchers_[227] = new DB_UpdateDeathTowerBestRecord();
    dispatchers_[228] = new DB_DeathTowerLoadTopRanker();
    dispatchers_[309] = new DB_UpdatePvPResult();
    dispatchers_[310] = new DB_UpdatePvPLastPlayTime();
    dispatchers_[501] = new DB_Multi_MailBox_Req_Send();
    dispatchers_[676] = new DB_MailBox_Req_System_Multi_Mail();
    dispatchers_[736] = new CLocalGlobal_DB_STATICS_BOSSTOWER();
    dispatchers_[819] = new DB_InsertEventStageLog();
    dispatchers_[legacy_character_snapshot::kDbSignal] =
        new legacy_character_snapshot::
            DB_LegacyCharacterSnapshot();
    dispatchers_[legacy_feature_fence::kDbSignal] =
        new legacy_feature_fence::DB_LegacyFeatureFence();
    dispatchers_[848] = new DB_LoadEventLevelupSupport();
    return 1;
}

// IDA_EA: 0x83FF156
// IDA_NAME: _ZN6DBTask18UnregistDispatcherEv
// DEMANGLED: DBTask::UnregistDispatcher(void)
int DBTask::UnregistDispatcher() {
    for (int index = 0; index <= 851; ++index) {
        if (dispatchers_[index]) {
            delete dispatchers_[index];
            dispatchers_[index] = 0;
        }
    }
    return 1;
}

// IDA_EA: 0x83FF1C8
// IDA_NAME: _ZN6DBTask12HandleEventsEN11MsgQueueMgr9QUEUE_IDXERi
// DEMANGLED: DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX,int &)
int DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX queue_idx, int& signal) {
    Stream* stream = GlobalData::s_msgq_mgr->get(queue_idx);
    if (!stream) {
        OS_API::uSleep(100);
        return 1;
    }

    queue_idx_ = queue_idx;
    const int stream_size = stream->length();
    const int pool_kind = stream->pool_kind();
    int activity_signal = 0;
    if (stream_size >= static_cast<int>(sizeof(activity_signal))) {
        std::memcpy(&activity_signal,
                    stream->get_buffer(),
                    sizeof(activity_signal));
    }
    const db_shutdown_quiescence::ActivityDomain activity_domain =
        queue_idx == MsgQueueMgr::LOGDB_Q
            ? db_shutdown_quiescence::kActivityLegacyLogDb
            : db_shutdown_quiescence::kActivityLegacyDb;
    db_shutdown_quiescence::BeginActivity(activity_domain,
                                                activity_signal);
    DBProcess(stream, signal);
    db_shutdown_quiescence::EndActivity(activity_domain);
    if (signal == 63) {
        const char* queue_name = MsgQueueMgr::get_queue_name(queue_idx);
        cMyTrace trace("bool DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX, int&)",
                       2278, 0);
        trace("Terminate Server db queue: %s(%d)\n", queue_name,
              static_cast<int>(queue_idx));
        // Signal 63 is diagnostic compatibility only.  The explicit
        // stop-after-drain flag owns termination, and this stream must still
        // pass through the unified pool return below.
    }

    switch (pool_kind) {
        case 1:
            if (stream_size > 4095) {
                cMyTrace trace(
                    "bool DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX, int&)",
                    2290, 5);
                trace("Small Stream Size over code(%d), size(%u)", signal,
                      stream_size);
            }
            GlobalData::small_stream_pool()->Free(stream);
            break;
        case 2:
            if (stream_size > 0x3FFFF) {
                cMyTrace trace(
                    "bool DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX, int&)",
                    2312, 5);
                trace("Big Stream Size over code(%d), size(%u)", signal,
                      stream_size);
            }
            GlobalData::big_stream_pool()->Free(stream);
            break;
        case 0:
            if (stream_size > 0x3FFF) {
                cMyTrace trace(
                    "bool DBTask::HandleEvents(MsgQueueMgr::QUEUE_IDX, int&)",
                    2301, 5);
                trace("Default Stream Size over code(%d), size(%u)", signal,
                      stream_size);
            }
            GlobalData::stream_pool()->Free(stream);
            break;
        default:
            break;
    }
    return 1;
}

// IDA_EA: 0x83FF3F4
// IDA_NAME: _ZN6DBTask9DBProcessEP6StreamRi
// DEMANGLED: DBTask::DBProcess(Stream *,int &)
int DBTask::DBProcess(Stream* stream, int& signal) {
    int dispatcher_signal = 0;
    const int signal_read = *stream >> dispatcher_signal;
    signal = dispatcher_signal;
    if (signal_read != 1) {
        cMyTrace trace("bool DBTask::DBProcess(Stream*, int&)", 2381, 5);
        trace("[%s][%d]", "bool DBTask::DBProcess(Stream*, int&)", 2381);
        return 0;
    }

    int user_idx = 0;
    if ((*stream >> user_idx) != 1) {
        cMyTrace trace("bool DBTask::DBProcess(Stream*, int&)", 2399, 5);
        trace("[%s][%d]", "bool DBTask::DBProcess(Stream*, int&)", 2399);
        return 0;
    }

    DBDispatcher* dispatcher = 0;
    if (signal >= 0 && signal < kDispatcherCount) {
        dispatcher = dispatchers_[signal];
    }
    if (!dispatcher) {
        LogManager::logFormat(1,
                              "DBThread.cpp",
                              "bool DBTask::DBProcess(Stream*, int&)",
                              0x970,
                              "Can't find the db dispatcher : signal(%d)",
                              signal);
        return 0;
    }

    if (queue_idx_ == MsgQueueMgr::DB_Q) {
        GlobalData::s_db_mgr->setUserIdx(user_idx);
    }
    dispatcher->dispatch(signal, user_idx, stream);
    if (queue_idx_ == MsgQueueMgr::DB_Q) {
        GlobalData::s_db_mgr->setUserIdx(-1);
    }
    return 1;
}
