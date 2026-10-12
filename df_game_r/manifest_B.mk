# B-owned canonical provider manifest.  A's root build consumes this list
# without reordering or duplicating B business sources.
# 140828 altar/tournament PVF import closure remains owned by the canonical
# map_runtime.cpp entry below; tools/tests/verify_blood_tournament_pvf_0828.py
# attests its strict nested-card parser and 0627 server-only entry limits.

B_OLD_DATA_DB_CORE_SRC := \
	src/old_data/db/core/db_dispatcher.cpp \
	src/old_data/db/core/i_dispatcher.cpp \
	src/old_data/db/core/db_mgr.cpp \
	src/old_data/db/core/db_task.cpp \
	src/old_data/db/core/db_legacy_character_snapshot.cpp \
	src/old_data/db/core/db_legacy_feature_fence.cpp \
	src/old_data/db/core/feature_db_task.cpp \
	src/old_data/db/core/feature_db_thread.cpp \
	src/old_data/db/core/mysql.cpp \
	src/old_data/db/core/taiwan_interface_db_dispatcher.cpp \
	src/old_data/db/taiwan/taiwan_db_dispatcher.cpp \
	src/old_data/db/taiwan/taiwan_runtime_dispatch.cpp

# DBTask constructs these existing, canonical dispatcher providers directly.
# Their exact C2 constructors only invoke DBDispatcher's base constructor;
# omitting the sources left the otherwise restored dispatchers unresolved.
B_OLD_DATA_DB_DISPATCHER_GAP_SRC := \
	src/old_data/db/character/db_update_charac_view.cpp \
	src/old_data/db/avatar_aura/db_avatar_change_stat.cpp \
	src/old_data/db/mail/db_mailbox_req_check_receiver.cpp \
	src/old_data/db/mail/db_mailbox_req_delete.cpp \
	src/old_data/db/creature/db_update_creature_item_list.cpp \
	src/old_data/db/auction_cashshop/db_save_gold_zero_trade_log.cpp \
	src/old_data/db/character/db_save_charac_slot_extend.cpp \
	src/old_data/db/character/db_save_character_option.cpp \
	src/old_data/db/auction_cashshop/db_auction_sub_money.cpp \
	src/old_data/db/auction_cashshop/arad_item_query.cpp

# Exact residual DBTask slots.  Source ownership remains one provider per
# functional DB directory; DBTask itself only performs construction.
B_OLD_DATA_DB_REMAINING_SLOT_RESTORE_SRC := \
	src/old_data/db/anti_cheat/db_report_mannerless_user.cpp \
	src/old_data/db/event/db_event_item_and_levelup_dispatchers.cpp \
	src/old_data/db/mail/db_multi_mail_dispatchers.cpp \
	src/old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.cpp \
	src/old_data/db/statistics_log/db_timer_statistics_log.cpp

B_OLD_DATA_DB_REMAINING_SLOT_RESTORE_HEADERS := \
	src/old_data/db/anti_cheat/db_report_mannerless_user.h \
	src/old_data/db/event/db_event_item_and_levelup_dispatchers.h \
	src/old_data/db/mail/db_multi_mail_dispatchers.h \
	src/old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h \
	src/old_data/db/statistics_log/db_timer_statistics_log.h

B_OLD_DATA_DB_CORE_HEADERS := \
	src/old_data/db/core/db_dispatcher.h \
	src/old_data/db/core/i_dispatcher.h \
	src/old_data/db/core/db_mgr.h \
	src/old_data/db/core/db_types.h \
	src/old_data/db/core/db_runtime_payloads.h \
	src/old_data/db/core/db_task.h \
	src/old_data/db/core/db_legacy_character_snapshot.h \
	src/old_data/db/core/db_legacy_feature_fence.h \
	src/old_data/db/core/feature_db_task.h \
	src/old_data/db/core/feature_db_thread.h \
	src/old_data/db/core/mysql.h \
	src/old_data/db/core/taiwan_interface_db_dispatcher.h \
	src/old_data/db/taiwan/taiwan_db_dispatcher.h \
	src/old_data/db/taiwan/taiwan_runtime_dispatch.h

# Complete account game-option and character persistent-state dispatch closure.
B_OLD_DATA_DB_GAME_OPTION_SRC := \
	src/old_data/db/game_option/db_load_game_option.cpp \
	src/old_data/db/game_option/db_save_game_option_1.cpp \
	src/old_data/db/game_option/db_save_game_option_2.cpp \
	src/old_data/db/game_option/db_save_game_option_quick_chatting.cpp \
	src/old_data/db/game_option/db_save_game_option_chatting_emoticon.cpp \
	src/old_data/db/game_option/db_update_cool_time_item.cpp \
	src/old_data/db/game_option/db_update_effect_item.cpp \
	src/old_data/db/game_option/db_update_dimension.cpp

B_OLD_DATA_DB_GAME_OPTION_HEADERS := \
	src/old_data/db/game_option/db_load_game_option.h \
	src/old_data/db/game_option/db_save_game_option_1.h \
	src/old_data/db/game_option/db_save_game_option_2.h \
	src/old_data/db/game_option/db_save_game_option_quick_chatting.h \
	src/old_data/db/game_option/db_save_game_option_chatting_emoticon.h \
	src/old_data/db/game_option/db_update_cool_time_item.h \
	src/old_data/db/game_option/db_update_effect_item.h \
	src/old_data/db/game_option/db_update_dimension.h

# Batch-048 complete item-making-skill DB save closure.
B_OLD_DATA_DB_ITEM_MAKING_SKILL_SRC := \
	src/old_data/db/skill/db_update_item_making_skill.cpp \
	src/old_data/db/skill/item_making_skill_payload.cpp

B_OLD_DATA_DB_ITEM_MAKING_SKILL_HEADERS := \
	src/old_data/db/skill/db_update_item_making_skill.h \
	src/old_data/db/skill/item_making_skill_payload.h

B_OLD_DATA_DB_CHANNEL_SRC := \
	src/old_data/db/channel/db_delete_channel.cpp \
	src/old_data/db/channel/db_insert_channel.cpp \
	src/old_data/db/channel/db_save_channel_user_level.cpp \
	src/old_data/db/channel/db_save_gamechannel.cpp \
	src/old_data/db/channel/db_update_channel.cpp \
	src/old_data/db/channel/db_update_detail_channel.cpp

B_OLD_DATA_DB_CHANNEL_HEADERS := \
	src/old_data/db/channel/db_delete_channel.h \
	src/old_data/db/channel/db_insert_channel.h \
	src/old_data/db/channel/db_save_channel_user_level.h \
	src/old_data/db/channel/db_save_gamechannel.h \
	src/old_data/db/channel/db_update_channel.h \
	src/old_data/db/channel/db_update_detail_channel.h

B_OLD_DATA_DB_STATISTICS_LOG_SRC := \
	src/old_data/db/statistics_log/db_insert_circulation_statistic.cpp \
	src/old_data/db/statistics_log/db_insert_party_type.cpp \
	src/old_data/db/statistics_log/db_insert_pvp_map.cpp \
	src/old_data/db/statistics_log/db_insert_pvp_player_rate.cpp \
	src/old_data/db/statistics_log/db_insert_query_counter_db.cpp \
	src/old_data/db/statistics_log/db_insert_query_counter_logdb.cpp \
	src/old_data/db/statistics_log/db_insert_queue_size_log.cpp \
	src/old_data/db/statistics_log/db_insert_response_time.cpp \
	src/old_data/db/statistics_log/db_insert_server_load_statistic.cpp \
	src/old_data/db/statistics_log/db_insert_user_play_time.cpp \
	src/old_data/db/statistics_log/db_insert_value_statistic.cpp \
	src/old_data/db/statistics_log/db_update_query_count_db_logdb.cpp \
	src/old_data/db/statistics_log/db_update_query_count_timer_db.cpp \
	src/old_data/db/statistics_log/db_update_query_count_timer_logdb.cpp \
	src/old_data/db/statistics_log/db_update_vending_machine_statistic.cpp

B_OLD_DATA_DB_STATISTICS_LOG_HEADERS := \
	src/old_data/db/statistics_log/db_insert_circulation_statistic.h \
	src/old_data/db/statistics_log/db_insert_party_type.h \
	src/old_data/db/statistics_log/db_insert_pvp_map.h \
	src/old_data/db/statistics_log/db_insert_pvp_player_rate.h \
	src/old_data/db/statistics_log/db_insert_query_counter_db.h \
	src/old_data/db/statistics_log/db_insert_query_counter_logdb.h \
	src/old_data/db/statistics_log/db_insert_queue_size_log.h \
	src/old_data/db/statistics_log/db_insert_response_time.h \
	src/old_data/db/statistics_log/db_insert_server_load_statistic.h \
	src/old_data/db/statistics_log/db_insert_user_play_time.h \
	src/old_data/db/statistics_log/db_insert_value_statistic.h \
	src/old_data/db/statistics_log/db_update_query_count_db_logdb.h \
	src/old_data/db/statistics_log/db_update_query_count_timer_db.h \
	src/old_data/db/statistics_log/db_update_query_count_timer_logdb.h \
	src/old_data/db/statistics_log/db_update_vending_machine_statistic.h \
	src/old_data/db/statistics_log/statistics_log_payloads.h

B_OLD_DATA_DB_PRIVATE_STORE_SRC := \
	src/old_data/db/private_store/db_save_private_store.cpp \
	src/old_data/db/private_store/db_load_private_store.cpp

B_OLD_DATA_DB_MERCENARY_SRC := \
	src/old_data/db/mercenary/db_save_mercenary_data.cpp

B_OLD_DATA_DB_EVENT_SRC := \
	src/old_data/db/event/db_glass_bottle_event.cpp \
	src/old_data/db/inventory/db_inventory_mutation_barrier.cpp \
	src/old_data/db/event/db_shooting_star_dimension_pocket.cpp \
	src/old_data/db/event/db_sea_bottom_event.cpp \
	src/old_data/db/event/db_blue_horse_event.cpp \
	src/old_data/db/event/db_sugar_time.cpp \
	src/old_data/db/event/db_zombie_mode_result.cpp \
	src/old_data/db/event/db_dual_raid_reward.cpp \
	src/old_data/db/event/db_racing_dungeon.cpp \
	src/old_data/db/event/db_golden_commerce.cpp \
	src/old_data/db/event/db_exp_keeping_event.cpp \
	src/old_data/db/event/db_freely_give_item.cpp \
	src/old_data/db/event/db_event_all_user_gift.cpp \
	src/old_data/db/event/db_king_of_pick.cpp \
	src/old_data/db/event/db_take_magic_lamp.cpp \
	src/old_data/db/event/db_facebook_like.cpp \
	src/old_data/db/event/db_event_attendance.cpp \
	src/old_data/db/event/db_dragon_timeattack_rank.cpp \
	src/old_data/db/event/db_skill_damage_rank_event.cpp \
	src/old_data/db/event/db_event_account_fatigue_stat.cpp \
	src/old_data/db/event/db_levelup_support_3rd.cpp \
	src/old_data/db/event/db_decide_levelup_event.cpp \
	src/old_data/db/event/db_fatigue_acceleration.cpp \
	src/old_data/db/event/db_satisfaction_survey.cpp \
	src/old_data/db/event/db_save_object_bring_up.cpp \
	src/old_data/db/event/db_statistic_advance_altar.cpp \
	src/old_data/db/event/db_req_save_crazy_levelup_event.cpp

B_OLD_DATA_DB_PVP_PARTY_SRC += \
	src/old_data/db/pvp_party/db_matching_channel_handoff.cpp \
	src/old_data/db/pvp_party/db_pvp_tournament.cpp \
	src/old_data/db/pvp_party/db_pvp_practice_combo_score.cpp

B_OLD_DATA_DB_ANTI_CHEAT_SRC := \
	src/old_data/db/anti_cheat/db_req_black_count.cpp \
	src/old_data/db/anti_cheat/db_report_mannerless_user_upgrade.cpp

B_OLD_DATA_DB_ANTI_CHEAT_HEADERS := \
	src/old_data/db/anti_cheat/db_req_black_count.h

B_OLD_DATA_LOGIN_ACCOUNT_CARGO_SRC := \
	src/old_data/Login/account_cargo/account_cargo_script.cpp \
	src/old_data/Login/account_cargo/account_cargo_inter_receivers.cpp

B_OLD_DATA_LOGIN_ACCOUNT_CARGO_HEADERS := \
	src/old_data/Login/account_cargo/account_cargo_script.h \
	src/old_data/Login/account_cargo/account_cargo_inter_receivers.h

B_OLD_DATA_LOGIN_CORE_SRC := \
	src/old_data/Login/quest_authen_data.cpp \
	src/old_data/Login/garena/auth_garena_token.cpp \
	src/old_data/Login/arad/login_punish.cpp \
	src/old_data/Login/arad/arad_white_account.cpp \
	src/old_data/Login/arad/arad_login_dispatcher.cpp \
	src/old_data/db/account/arad_login_query.cpp \
	src/old_data/db/auction_cashshop/arad_buy_item_dispatcher.cpp

B_OLD_DATA_LOGIN_CORE_HEADERS := \
	src/old_data/Login/quest_authen_data.h \
	src/old_data/Login/garena/auth_garena_token.h \
	src/old_data/Login/garena/openssl_rsa.h \
	src/old_data/Login/arad/login_punish.h \
	src/old_data/Login/arad/arad_white_account.h \
	src/old_data/Login/arad/arad_login_dispatcher.h \
	src/old_data/db/account/arad_login_query.h \
	src/old_data/db/auction_cashshop/arad_buy_item_dispatcher.h

B_OLD_DATA_LOGIN_INTER_RETURNS_SRC := \
	src/old_data/Login/inter_returns/login_inter_receivers.cpp \
	src/old_data/Login/inter_returns/inter_action_point_returns.cpp \
	src/old_data/Login/inter_returns/inter_blood_dungeon_best_record_return.cpp \
	src/old_data/Login/inter_returns/inter_breakaway_return_dispatcher.cpp \
	src/old_data/Login/inter_returns/inter_cutoff_userinfo_return.cpp \
	src/old_data/Login/inter_returns/inter_disconnect.cpp \
	src/old_data/Login/inter_returns/inter_exact_leaf_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_event_trigger.cpp \
	src/old_data/Login/inter_returns/inter_guild_notification_return_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_guild_name_return_dispatchers.cpp \
	src/old_game/player/core/cuser_charac_name_runtime.cpp \
	src/old_game/player/core/cuser_solo_play_mode_runtime.cpp \
	src/old_game/player/session/cuser_guild_inter_return_runtime.cpp \
	src/common/manager/character_name_change_proxy_runtime.cpp \
	src/old_game/player/session/inter_change_charac_name_return_dispatcher.cpp \
	src/old_game/social/guild/guild_member_packet_runtime.cpp \
	src/old_game/social/guild/inter_guild_member_return_dispatchers.cpp \
	src/old_game/social/guild/inter_guild_pending_return_dispatchers.cpp \
	src/old_game/pvp/battlefield/inter_power_war_dialogue2_returns.cpp \
	src/old_game/economy/mall/auction_inter_return_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_death_tower_return_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_monitor_return_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_social_return_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_monitor_notice_control.cpp \
	src/old_data/Login/inter_returns/inter_monitor_user_getout.cpp \
	src/old_data/Login/inter_returns/inter_task4_basic_dispatchers.cpp \
	src/old_data/Login/inter_returns/inter_tcp_server_connect_return.cpp \
	src/old_data/Login/inter_returns/inter_login_info.cpp \
	src/old_data/Login/inter_returns/inter_load_game_option.cpp \
	src/old_data/Login/inter_returns/inter_load_skill.cpp \
	src/old_data/Login/inter_returns/inter_load_ticket.cpp \
	src/old_data/Login/inter_returns/inter_last_msg_response.cpp \
	src/old_data/Login/inter_returns/inter_notice.cpp \
	src/old_data/Login/inter_returns/inter_ready_to_terminate.cpp \
	src/old_data/Login/inter_returns/inter_update_logout.cpp \
	src/old_data/Login/inter_returns/login_ingame_event_history.cpp \
	src/old_data/Login/inter_returns/login_growth_event_records.cpp \
	src/old_data/Login/inter_returns/auto_punish_inter_receivers.cpp \
	src/old_data/Login/inter_returns/break_away_prevent_runtime.cpp \
	src/old_data/Login/inter_returns/inter_load_etc_dispatch.cpp \
	src/old_data/Login/inter_returns/inter_load_etc_break_away_reward.cpp \
	src/old_data/Login/inter_returns/inter_load_break_away_prevent_system.cpp \
	src/old_data/Login/inter_returns/inter_load_hero_mission_event_data.cpp \
	src/old_data/Login/inter_returns/inter_select_mobile_auth_reward.cpp \
	src/old_data/Login/inter_returns/statistic_manager_runtime.cpp \
	src/infra/network/dispatcher/inter_dispatcher_arad_only.cpp

B_OLD_DATA_LOGIN_INTER_RETURNS_HEADERS := \
	src/old_data/Login/inter_returns/login_inter_receivers.h \
	src/old_data/Login/inter_returns/inter_action_point_returns.h \
	src/old_data/Login/inter_returns/inter_blood_dungeon_best_record_return.h \
	src/old_data/Login/inter_returns/inter_breakaway_return_dispatcher.h \
	src/old_data/Login/inter_returns/inter_cutoff_userinfo_return.h \
	src/old_data/Login/inter_returns/inter_guild_notification_return_dispatchers.h \
	src/old_data/Login/inter_returns/inter_guild_name_return_dispatchers.h \
	src/old_game/player/session/inter_change_charac_name_return_dispatcher.h \
	src/old_game/social/guild/guild_member_packet_runtime.h \
	src/old_game/social/guild/inter_guild_member_return_dispatchers.h \
	src/old_game/social/guild/inter_guild_pending_return_dispatchers.h \
	src/old_game/pvp/battlefield/inter_power_war_dialogue2_returns.h \
	src/old_game/world/region/power_manager_dialogue2_state.h \
	src/old_game/economy/mall/auction_inter_return_dispatchers.h \
	src/old_game/economy/mall/auction_regist_item_return_abi.h \
	src/old_data/Login/inter_returns/inter_death_tower_return_dispatchers.h \
	src/old_data/Login/inter_returns/inter_monitor_return_dispatchers.h \
	src/old_data/Login/inter_returns/inter_social_return_dispatchers.h \
	src/old_data/Login/inter_returns/inter_monitor_notice_control.h \
	src/old_data/Login/inter_returns/inter_monitor_user_getout.h \
	src/old_data/Login/inter_returns/inter_event_trigger.h \
	src/old_data/Login/inter_returns/inter_task4_basic_dispatchers.h \
	src/old_data/Login/inter_returns/inter_tcp_server_connect_return.h \
	src/old_data/Login/inter_returns/inter_login_info.h \
	src/old_data/Login/inter_returns/inter_load_game_option.h \
	src/old_data/Login/inter_returns/inter_load_skill.h \
	src/old_data/Login/inter_returns/inter_last_msg_response.h \
	src/old_data/Login/inter_returns/inter_update_logout.h \
	src/old_data/Login/inter_returns/auto_punish_inter_receivers.h \
	src/old_data/Login/inter_returns/break_away_prevent_runtime.h \
	src/old_data/Login/inter_returns/inter_load_etc_break_away_reward.h \
	src/old_data/Login/inter_returns/inter_load_break_away_prevent_system.h \
	src/old_data/Login/inter_returns/inter_load_hero_mission_event_data.h \
	src/old_data/Login/inter_returns/inter_select_mobile_auth_reward.h \
	src/old_data/Login/inter_returns/statistic_manager_runtime.h \
	src/infra/network/dispatcher/inter_dispatcher_arad_only.h

B_OLD_DATA_LOGIN_SECURITY_SRC := \
	src/common/manager/game_manager_cutoff_runtime.cpp \
	src/old_data/Login/security/cleanpad_point_table_runtime.cpp \
	src/old_data/Login/security/inter_account_security_return_dispatchers.cpp \
	src/old_data/Login/security/inter_report_mannerless_return_dispatcher.cpp \
	src/old_data/Login/security/inter_slang_cleanpad_return_dispatchers.cpp \
	src/old_data/Login/security/security_card_center.cpp \
	src/old_data/Login/security/security_card.cpp \
	src/old_data/Login/security/secu_account_hacking_runtime.cpp \
	src/old_data/Login/security/auto_punish_black_ip_manager.cpp \
	src/old_data/Login/security/auto_punish_hack_type_manager.cpp \
	src/old_data/Login/security/auto_punish_hack_rule_check.cpp

B_OLD_DATA_LOGIN_SECURITY_HEADERS := \
	src/old_data/Login/security/inter_account_security_return_dispatchers.h \
	src/old_data/Login/security/inter_report_mannerless_return_dispatcher.h \
	src/old_data/Login/security/inter_slang_cleanpad_return_dispatchers.h \
	src/old_data/Login/security/security_card_center.h \
	src/old_data/Login/security/security_card.h \
	src/old_data/Login/security/auto_punish_black_ip_manager.h \
	src/old_data/Login/security/auto_punish_counter_abi.h \
	src/old_data/Login/security/auto_punish_hack_type_abi.h \
	src/old_data/Login/security/auto_punish_hack_type_manager.h

B_OLD_DATA_LOGIN_PACKETDISPATCH_SRC := \
	src/old_data/Login/packet_dispatch/security_card_dispatchers.cpp \
	src/old_data/Login/packet_dispatch/mouse_register_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/cera_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/change_skill_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/guild_attendance_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/donate_guild_fund_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/approve_join_guild_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/request_join_guild_dispatcher.cpp \
	src/old_data/db/legacy_dispatch/db_request_join_guild_make_request.cpp \
	src/old_data/Login/packet_dispatch/blacklist_register_delete_dispatchers.cpp \
	src/old_data/Login/packet_dispatch/guild_board_dispatchers.cpp \
	src/old_data/Login/packet_dispatch/skill_init_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/combo_skill_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/purify_item_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/select_striker_types.cpp \
	src/old_data/Login/packet_dispatch/select_striker_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/cera_package_open_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/collect_items_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/use_dye_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/break_away_quest_check_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/select_item_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/use_creature_evolution_item_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/select_item_growth_power_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/precheck_solo_teleport_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/use_right_of_change_grow_type_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/cuser_change_grow_type_item_runtime.cpp \
	src/old_data/Login/packet_dispatch/use_skill_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/guild_cargo_history_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/compound_equipment_upgrade_card_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/avatar_option_change_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/add_socket_to_avatar_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/use_stackable_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/teleport_dispatcher.cpp \
	src/old_data/Login/packet_dispatch/seria_room_ani_deco_dispatcher.cpp \
	src/old_game/activity/schedule/seria_room_ani_deco_event.cpp \
	src/old_data/Login/packet_dispatch/login_dispatcher.cpp

B_OLD_DATA_LOGIN_PACKETDISPATCH_HEADERS := \
	src/old_data/Login/packet_dispatch/security_card_dispatchers.h \
	src/old_data/Login/packet_dispatch/mouse_register_dispatcher.h \
	src/old_data/Login/packet_dispatch/cera_dispatcher.h \
	src/old_data/Login/packet_dispatch/change_skill_dispatcher.h \
	src/old_data/Login/packet_dispatch/power_war_process_info_dispatcher.h \
	src/old_data/Login/packet_dispatch/guild_attendance_dispatcher.h \
	src/old_data/Login/packet_dispatch/donate_guild_fund_dispatcher.h \
	src/old_data/Login/packet_dispatch/request_seria_buff_dispatcher.h \
	src/old_data/Login/packet_dispatch/approve_join_guild_dispatcher.h \
	src/old_data/Login/packet_dispatch/request_join_guild_dispatcher.h \
	src/old_data/Login/packet_dispatch/blacklist_register_delete_dispatchers.h \
	src/old_data/Login/packet_dispatch/guild_board_dispatchers.h \
	src/old_data/Login/packet_dispatch/skill_init_dispatcher.h \
	src/old_data/Login/packet_dispatch/combo_skill_dispatcher.h \
	src/old_data/Login/packet_dispatch/purify_item_dispatcher.h \
	src/old_data/Login/packet_dispatch/select_striker_types.h \
	src/old_data/Login/packet_dispatch/select_striker_dispatcher.h \
	src/old_data/Login/packet_dispatch/cera_package_open_dispatcher.h \
	src/old_data/Login/packet_dispatch/collect_items_dispatcher.h \
	src/old_data/Login/packet_dispatch/use_dye_dispatcher.h \
	src/old_data/Login/packet_dispatch/break_away_quest_check_dispatcher.h \
	src/old_data/Login/packet_dispatch/select_item_dispatcher.h \
	src/old_data/Login/packet_dispatch/use_creature_evolution_item_dispatcher.h \
	src/old_data/Login/packet_dispatch/select_item_growth_power_dispatcher.h \
	src/old_data/Login/packet_dispatch/precheck_solo_teleport_dispatcher.h \
	src/old_data/Login/packet_dispatch/use_right_of_change_grow_type_dispatcher.h \
	src/old_data/Login/packet_dispatch/use_skill_dispatcher.h \
	src/old_data/Login/packet_dispatch/guild_cargo_history_dispatcher.h \
	src/old_data/Login/packet_dispatch/compound_equipment_upgrade_card_dispatcher.h \
	src/old_data/Login/packet_dispatch/avatar_option_change_dispatcher.h \
	src/old_data/Login/packet_dispatch/add_socket_to_avatar_dispatcher.h \
	src/old_data/Login/packet_dispatch/use_stackable_dispatcher.h \
	src/old_data/Login/packet_dispatch/teleport_dispatcher.h \
	src/old_game/activity/schedule/seria_room_ani_deco_event.h \
	src/old_data/Login/packet_dispatch/login_dispatcher.h

B_OLD_DATA_LOGIN_PACKETS_SRC := \
	src/old_data/Login/packets/hades_login_packet.cpp \
	src/old_data/Login/packets/login_packets.cpp

B_OLD_DATA_LOGIN_PACKETS_HEADERS := \
	src/old_data/Login/packets/hades_login_packet.h \
	src/old_data/Login/packets/login_packets.h

B_OLD_DATA_AUTH_SECURITY_PAYLOAD_HEADERS := \
	src/old_data/db/auth_security/save_proxy_log_payload.h

B_OLD_DATA_PVF_COMMON_SRC := \
	src/old_data/PVF/common/arad_script.cpp \
	src/old_data/PVF/common/arad_jumping_script.cpp \
	src/old_data/PVF/common/disjoint_script_import.cpp \
	src/old_data/PVF/common/dnf_lex.cpp \
	src/old_data/PVF/common/legacy_string_abi.cpp \
	src/old_data/PVF/common/pvf_pack.cpp \
	src/old_data/PVF/common/rdar_script.cpp \
	src/old_data/PVF/common/script_initializers.cpp \
	src/old_data/PVF/common/script_file_list.cpp \
	src/old_data/PVF/common/script_string_manager.cpp

B_OLD_DATA_PVF_COMMON_HEADERS := \
	src/old_data/PVF/common/arad_script.h \
	src/old_data/PVF/common/arad_script_data.h \
	src/old_data/PVF/common/dnf_lex.h \
	src/old_data/PVF/common/legacy_string_abi.h \
	src/old_data/PVF/common/pvf_pack.h \
	src/old_data/PVF/common/rdar_script.h \
	src/old_data/PVF/common/script_initializers.h \
	src/old_data/PVF/common/script_file_list.h \
	src/old_data/PVF/common/script_string_manager.h

B_OLD_DATA_PVF_AICHARACTER_SRC := \
	src/old_data/PVF/aicharacter/st_ai_info_script.cpp

B_OLD_DATA_PVF_AICHARACTER_HEADERS := \
	src/old_data/PVF/aicharacter/st_ai_info_script.h

B_OLD_DATA_PVF_LOCALIZATION_SRC := \
	src/old_data/PVF/localization/collect_box_script.cpp \
	src/old_data/PVF/localization/collect_box_time_policy.cpp \
	src/old_data/PVF/localization/invalid_name_script.cpp \
	src/old_data/PVF/localization/slang_list_script.cpp \
	src/old_data/PVF/localization/slang_name_script.cpp

B_OLD_DATA_PVF_LOCALIZATION_HEADERS := \
	src/old_data/PVF/localization/collect_box_script.h \
	src/old_data/PVF/localization/collect_box_time_policy.h \
	src/old_data/PVF/localization/invalid_name_script.h \
	src/old_data/PVF/localization/slang_list_script.h \
	src/old_data/PVF/localization/slang_name_script.h

B_OLD_DATA_PVF_CREATURE_SRC := \
	src/old_data/PVF/creature/active_status_sync_server.cpp \
	src/old_data/PVF/creature/active_status_sync_common_runtime.cpp \
	src/old_data/PVF/creature/creature_script_runtime.cpp

B_OLD_DATA_PVF_CREATURE_HEADERS := \
	src/old_data/PVF/creature/active_status_sync_server.h \
	src/old_data/PVF/creature/creature_script.h

B_OLD_DATA_PVF_CHARACTER_SRC := \
	src/old_data/PVF/character/avatar_color_variation.cpp \
	src/old_data/PVF/character/character_create_creature_gift_script.cpp \
	src/old_data/PVF/character/expert_job_helpers.cpp \
	src/old_data/PVF/character/expert_job_import.cpp \
	src/old_data/PVF/character/expert_job_script_list.cpp \
	src/old_data/PVF/character/use_character_job_fatigue_script.cpp

B_OLD_DATA_PVF_CHARACTER_HEADERS := \
	src/old_data/PVF/character/avatar_color_variation.h \
	src/old_data/PVF/character/character_create_creature_gift_script.h \
	src/old_data/PVF/character/expert_job_helpers.h \
	src/old_data/PVF/character/expert_job_script_list.h \
	src/old_data/PVF/character/use_character_job_fatigue_script.h

B_OLD_DATA_PVF_NPC_SRC := \
	src/old_data/PVF/npc/interactive_dialog_script.cpp \
	src/old_data/PVF/npc/need_material_data_manager.cpp \
	src/old_data/PVF/npc/npc_common_data.cpp

B_OLD_DATA_PVF_NPC_HEADERS := \
	src/old_data/PVF/npc/interactive_dialog_script.h \
	src/old_data/PVF/npc/need_material_data_manager.h \
	src/old_data/PVF/npc/npc_common_data.h

B_OLD_DATA_PVF_TOWN_SRC := \
	src/old_data/PVF/town/town_script.cpp

B_OLD_DATA_PVF_TOWN_HEADERS := \
	src/old_data/PVF/town/town_script.h

B_OLD_DATA_PVF_STACKABLE_SRC := \
	src/old_data/PVF/stackable/arad_gift_item_set.cpp \
	src/old_data/PVF/stackable/arad_st_stackable_script.cpp \
	src/old_data/PVF/stackable/disguise_stackable_registry.cpp \
	src/old_data/PVF/stackable/stackable_script_types.cpp \
	src/old_data/PVF/stackable/stackable_script.cpp \
	src/old_data/PVF/stackable/stackable_script_init.cpp

B_OLD_DATA_PVF_STACKABLE_HEADERS := \
	src/old_data/PVF/stackable/arad_gift_item_set.h \
	src/old_data/PVF/stackable/arad_st_stackable_script.h \
	src/old_data/PVF/stackable/disguise_stackable_registry.h \
	src/old_data/PVF/stackable/stackable_script_types.h \
	src/old_data/PVF/stackable/stackable_script.h

B_OLD_DATA_PVF_EVENT_SRC := \
	src/old_data/PVF/event/upgrade_room.cpp \
	src/old_data/PVF/event/action_send_mail.cpp \
	src/old_data/PVF/event/action_mail_script.cpp \
	src/old_data/PVF/event/arad_event_period_script.cpp \
	src/old_data/PVF/event/bingo_script.cpp \
	src/old_data/PVF/event/blue_marble_script.cpp \
	src/old_data/PVF/event/condition_event_script.cpp \
	src/old_data/PVF/event/event_avatar_list_script.cpp \
	src/old_data/PVF/event/event_etc_script.cpp \
	src/old_data/PVF/event/event_upgrade_jar_script.cpp \
	src/old_data/PVF/event/event_reward_script.cpp \
	src/old_data/PVF/event/in_game_event_info_import.cpp \
	src/old_data/PVF/event/in_game_event_pvf_scripts.cpp \
	src/old_data/PVF/event/in_game_event_script_import.cpp \
	src/old_data/PVF/event/growth_capsule_script.cpp \
	src/old_data/PVF/event/level_reward_value_types.cpp \
	src/old_data/PVF/event/level_up_reward_item_script.cpp \
	src/old_data/PVF/event/new_account_level_up_to_job_script.cpp \
	src/old_data/PVF/event/object_bring_up_rule.cpp \
	src/old_data/PVF/event/present_response_script.cpp \
	src/old_data/PVF/event/present_response_script_exception.cpp \
	src/old_data/PVF/event/present_response_script_event.cpp \
	src/old_data/PVF/event/return_user_reward_script.cpp \
	src/old_data/PVF/event/seria_blessing_script.cpp \
	src/old_data/PVF/event/seria_room_deco_event_script.cpp \
	src/old_data/PVF/event/seria_room_deco_value_types.cpp \
	src/old_data/PVF/event/unlimit_challenge_script.cpp

B_OLD_DATA_PVF_EVENT_HEADERS := \
	src/old_data/PVF/event/action_send_mail.h \
	src/old_data/PVF/event/action_mail_script.h \
	src/old_data/PVF/event/bingo_script.h \
	src/old_data/PVF/event/blue_marble_script.h \
	src/old_data/PVF/event/condition_event_script.h \
	src/old_data/PVF/event/event_avatar_list_script.h \
	src/old_data/PVF/event/event_etc_script.h \
	src/old_data/PVF/event/event_upgrade_jar_script.h \
	src/old_data/PVF/event/event_reward_script.h \
	src/old_data/PVF/event/in_game_event_info_import.h \
	src/old_data/PVF/event/in_game_event_pvf_scripts.h \
	src/old_data/PVF/event/in_game_event_script_import.h \
	src/old_data/PVF/event/level_reward_scripts.h \
	src/old_data/PVF/event/object_bring_up_rule.h \
	src/old_data/PVF/event/present_response_script.h \
	src/old_data/PVF/event/present_response_script_exception.h \
	src/old_data/PVF/event/present_response_script_event.h \
	src/old_data/PVF/event/seria_blessing_script.h \
	src/old_data/PVF/event/seria_room_deco_event_script.h \
	src/old_data/PVF/event/seria_room_deco_value_types.h \
	src/old_data/PVF/event/unlimit_challenge_script.h

B_OLD_DATA_PVF_ETC_SRC := \
	src/old_data/PVF/etc/additional_effect_list_script.cpp \
	src/old_data/PVF/etc/arad_avatar_script_data.cpp \
	src/old_data/PVF/etc/assault_parameter_types.cpp \
	src/old_data/PVF/etc/avatar_roulette_script.cpp \
	src/old_data/PVF/etc/blood_war_script.cpp \
	src/old_data/PVF/etc/blood_dungeon_reward_info.cpp \
	src/old_data/PVF/etc/crane_minigame_script.cpp \
	src/old_data/PVF/etc/cargo_transport_stone_script.cpp \
	src/old_data/PVF/etc/compound_creature_script.cpp \
	src/old_data/PVF/etc/datamanager_etc_drivers_0828.cpp \
	src/old_data/PVF/etc/datamanager_optional_features_0828.cpp \
	src/old_data/PVF/etc/equipment_growth.cpp \
	src/old_data/PVF/etc/instant_reinforce.cpp \
	src/old_data/PVF/etc/time_spiral_script.cpp \
	src/old_data/PVF/etc/delete_invalid_item_data.cpp \
	src/old_data/PVF/etc/emotion_type_tags.cpp \
	src/old_data/PVF/etc/etc_parameter_script.cpp \
	src/old_data/PVF/etc/equipment_grade_sample.cpp \
	src/old_data/PVF/etc/global_script_tutorial.cpp \
	src/old_data/PVF/etc/guild_parameter_script.cpp \
	src/old_data/PVF/etc/guild_parameter_types.cpp \
	src/old_data/PVF/etc/independent_drop_parameter_script.cpp \
	src/old_data/PVF/etc/item_making_skill_script.cpp \
	src/old_data/PVF/etc/mercenary_system_script.cpp \
	src/old_data/PVF/etc/novice_command_script.cpp \
	src/old_data/PVF/etc/pcbang_script.cpp \
	src/old_data/PVF/etc/recommend_newbie_option_script.cpp \
	src/old_data/PVF/etc/pcbang_dimension_script.cpp \
	src/old_data/PVF/etc/power_parameter_script.cpp \
	src/old_data/PVF/etc/quick_party_script.cpp \
	src/old_data/PVF/etc/striker_script.cpp \
	src/old_data/PVF/etc/vending_machine_script.cpp

B_OLD_DATA_PVF_ETC_HEADERS := \
	src/old_data/PVF/etc/additional_effect_list_script.h \
	src/old_data/PVF/etc/assault_parameter_types.h \
	src/old_data/PVF/etc/avatar_roulette_script.h \
	src/old_data/PVF/etc/blood_dungeon_reward_info.h \
	src/old_data/PVF/etc/blood_war_script.h \
	src/old_data/PVF/etc/crane_minigame_script.h \
	src/old_data/PVF/etc/datamanager_etc_drivers_0828.h \
	src/old_data/PVF/etc/datamanager_optional_features_0828.h \
	src/old_data/PVF/etc/party_matching_info.h \
	src/old_data/PVF/etc/quest_movie_info.h \
	src/old_data/PVF/etc/equipment_growth.h \
	src/old_data/PVF/etc/instant_reinforce.h \
	src/old_data/PVF/etc/time_spiral_script.h \
	src/old_data/PVF/etc/delete_invalid_item_data.h \
	src/old_data/PVF/etc/emotion_type_tags.h \
	src/old_data/PVF/etc/etc_parameter_script.h \
	src/old_data/PVF/etc/equipment_grade_sample.h \
	src/old_data/PVF/etc/global_script_tutorial.h \
	src/old_data/PVF/etc/guild_parameter_script.h \
	src/old_data/PVF/etc/guild_parameter_types.h \
	src/old_data/PVF/etc/independent_drop_parameter_script.h \
	src/old_data/PVF/etc/independent_drop_types.h \
	src/old_data/PVF/etc/item_making_skill_script.h \
	src/old_data/PVF/etc/mercenary_system_script.h \
	src/old_data/PVF/etc/novice_command_script.h \
	src/old_data/PVF/etc/pcbang_script.h \
	src/old_data/PVF/etc/recommend_newbie_option_script.h \
	src/old_data/PVF/etc/pcbang_dimension_script.h \
	src/old_data/PVF/etc/power_parameter_script.h \
	src/old_data/PVF/etc/quick_party_script.h \
	src/old_data/PVF/etc/striker_script.h \
	src/old_data/PVF/etc/vending_machine_script.h

B_OLD_DATA_PVF_DUNGEON_SRC := \
	src/old_data/PVF/dungeon/advance_altar_dungeon_script.cpp \
	src/old_data/PVF/dungeon/dead_tower_dialog_script.cpp \
	src/old_data/PVF/dungeon/dungeon_script_value_types.cpp \
	src/old_data/PVF/dungeon/grid_script.cpp \
	src/old_data/PVF/dungeon/maze_script.cpp

B_OLD_DATA_PVF_DUNGEON_HEADERS := \
	src/old_data/PVF/dungeon/advance_altar_dungeon_script.h \
	src/old_data/PVF/dungeon/dead_tower_dialog_script.h \
	src/old_data/PVF/dungeon/dungeon_script_value_types.h \
	src/old_data/PVF/dungeon/grid_script.h \
	src/old_data/PVF/dungeon/maze_script.h

B_OLD_DATA_PVF_MAP_SRC := \
	src/old_data/PVF/map/back_ani_script.cpp

B_OLD_DATA_PVF_MAP_HEADERS := \
	src/old_data/PVF/map/back_ani_script.h

B_OLD_DATA_PVF_N_QUEST_SRC := \
	src/old_data/PVF/n_quest/episode_script.cpp \
	src/old_data/PVF/n_quest/epic_quest_script.cpp \
	src/old_data/PVF/n_quest/qp_guide_recommend_data.cpp \
	src/old_data/PVF/n_quest/qp_guide_script.cpp \
	src/old_data/PVF/n_quest/quest_parameter_script.cpp \
	src/old_data/PVF/n_quest/quest_script.cpp

B_OLD_DATA_PVF_N_QUEST_HEADERS := \
	src/old_data/PVF/n_quest/episode_script.h \
	src/old_data/PVF/n_quest/qp_guide_recommend_data.h \
	src/old_data/PVF/n_quest/quest_parameter_script.h \
	src/old_data/PVF/n_quest/quest_skill_script.h

B_OLD_DATA_PVF_PVP_MISSION_SRC := \
	src/old_data/PVF/pvp_mission/combo_script_data.cpp \
	src/old_data/PVF/pvp_mission/combo_script_key.cpp \
	src/old_data/PVF/pvp_mission/fair_pvp_equipment_script.cpp \
	src/old_data/PVF/pvp_mission/fair_pvp_stat_script.cpp \
	src/old_data/PVF/pvp_mission/pvp_reference_tables.cpp \
	src/old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.cpp \
	src/old_data/PVF/pvp_mission/pvp_channel_list.cpp

B_OLD_DATA_PVF_PVP_MISSION_HEADERS := \
	src/old_data/PVF/pvp_mission/combo_script_data.h \
	src/old_data/PVF/pvp_mission/combo_script_key.h \
	src/old_data/PVF/pvp_mission/fair_pvp_equipment_script.h \
	src/old_data/PVF/pvp_mission/fair_pvp_stat_script.h \
	src/old_data/PVF/pvp_mission/pvp_reference_tables.h \
	src/old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h \
	src/old_data/PVF/pvp_mission/pvp_channel_list.h

B_OLD_DATA_CHANNEL_SRC := \
	src/old_data/channel/arad_event_notification_runtime.cpp \
	src/old_data/channel/arad_server_state_message.cpp \
	src/old_data/channel/packet_dispatch/channel_packets.cpp \
	src/old_data/channel/channel_dungeon_info_t.cpp \
	src/old_data/channel/channel_info_t.cpp \
	src/old_data/channel/channel_script_t.cpp \
	src/old_data/channel/channel_timers.cpp \
	src/old_data/channel/log_game_channel.cpp \
	src/old_data/channel/pvp_channel_info_t.cpp \
	src/old_data/channel/inter_returns/channel_inter_receivers.cpp

B_OLD_DATA_CHANNEL_HEADERS := \
	src/old_data/channel/arad_event_notification_runtime.h \
	src/old_data/channel/arad_server_state_message.h \
	src/old_data/channel/packet_dispatch/channel_packets.h \
	src/old_data/channel/legacy_runtime/channel_proxy.h \
	src/old_data/channel/channel_db_payloads.h \
	src/old_data/channel/channel_dungeon_info_t.h \
	src/old_data/channel/channel_info_t.h \
	src/old_data/channel/channel_script_t.h \
	src/old_data/channel/channel_timers.h \
	src/old_data/channel/log_game_channel.h \
	src/old_data/channel/legacy_runtime/pvp_channel.h \
	src/old_data/channel/pvp_channel_info_t.h \
	src/old_data/channel/inter_returns/channel_inter_receivers.h

# SR08 CMD198 owns the typed channel packet and its state-0 dispatcher once.
B_OLD_DATA_CHANNEL_PACKETDISPATCH_SRC := \
	src/old_data/channel/packet_dispatch/pvp_channel_info_dispatcher.cpp

B_OLD_DATA_CHANNEL_PACKETDISPATCH_HEADERS := \
	src/old_data/channel/packet_dispatch/pvp_channel_info_dispatcher.h

B_OLD_DATA_PVF_DATA_SRC := \
	src/old_data/PVF/data/arad_data_manager.cpp \
	src/old_data/PVF/data/arad_item.cpp \
	src/old_data/PVF/data/arad_one_plus_card_event_item_list.cpp \
	src/old_data/PVF/data/arad_statistics_event_item_list.cpp \
	src/old_data/PVF/data/cnrd_script_data.cpp \
	src/old_data/PVF/data/data_manager_action_point.cpp \
	src/old_data/PVF/data/data_manager_core_queries.cpp \
	src/old_data/PVF/data/data_manager_power_war_access.cpp \
	src/old_data/PVF/data/data_manager_full.cpp \
	src/old_data/PVF/data/data_manager_death_tower_access.cpp \
	src/old_data/PVF/data/data_manager_name_filter.cpp \
	src/old_data/PVF/data/data_manager_verify_runtime.cpp \
	src/old_data/PVF/data/data_manager_find_dungeon.cpp \
	src/old_data/PVF/data/data_manager_find_item.cpp \
	src/old_data/PVF/data/data_manager_ingame_advertisement_access.cpp \
	src/old_data/PVF/data/script_hash_param.cpp \
	src/old_data/PVF/data/levelup_support_item_info.cpp \
	src/old_data/PVF/data/premium_item_data.cpp \
	src/old_data/PVF/data/emoticon_script.cpp \
	src/old_data/PVF/data/data_manager_item_upgrade.cpp \
	src/old_data/PVF/data/data_manager_growth_power_access.cpp \
	src/old_data/PVF/data/item_list_find_item.cpp \
	src/old_data/PVF/data/item_list_cerashop_bonus_runtime.cpp \
	src/old_data/PVF/data/item_list_pvf_runtime.cpp \
	src/old_data/PVF/data/item_prototype_lifecycle.cpp \
	src/old_data/PVF/data/item_list_pvf_import.cpp \
	src/old_data/PVF/data/item_list_pvf_lifecycle.cpp

B_OLD_DATA_PVF_DATA_HEADERS := \
	src/old_data/PVF/data/arad_data_manager.h \
	src/old_data/PVF/data/arad_item.h \
	src/old_data/PVF/data/arad_one_plus_card_event_item_list.h \
	src/old_data/PVF/data/arad_statistics_event_item_list.h \
	src/old_data/PVF/data/cnrd_script_data.h \
	src/old_data/PVF/data/script_hash_param.h \
	src/old_data/PVF/data/levelup_support_item_info.h \
	src/old_data/PVF/data/premium_item_data.h \
	src/old_data/PVF/data/data_manager.h \
	src/old_data/PVF/data/emoticon_script.h

B_OLD_DATA_PVF_SYNC_SRC := \
	src/old_data/PVF/data/sync_script.cpp \
	src/old_data/PVF/data/sync_script_csync_script.cpp

B_OLD_DATA_PVF_SYNC_HEADERS := \
	src/old_data/PVF/data/field_length_checker.h \
	src/old_data/PVF/data/sync_script.h

B_OLD_DATA_PVF_EQUIPMENT_SRC := \
	src/old_data/PVF/equipment/equipment_script.cpp \
	src/old_data/PVF/equipment/equipment_part_set_import.cpp \
	src/old_data/PVF/amplify_item_script.cpp \
	src/old_data/PVF/equipment/item_amplifier.cpp \
	src/old_data/PVF/equipment/item_upgrade.cpp \
	src/old_data/PVF/equipment/item_upgrade_condition.cpp \
	src/old_data/PVF/equipment/item_upgrade_leaves.cpp \
	src/old_data/PVF/equipment/item_upgrade_amplify_random.cpp \
	src/old_data/PVF/equipment/item_upgrade_chronicle.cpp \
	src/old_data/PVF/equipment/item_upgrade_main_proc.cpp \
	src/old_data/PVF/equipment/item_upgrade_3rd_chronicle.cpp \
	src/old_data/PVF/equipment/item_upgrade_message.cpp \
	src/old_data/PVF/equipment/item_upgrade_penalty.cpp \
	src/old_data/PVF/equipment/item_upgrade_proc.cpp \
	src/old_data/PVF/equipment/item_upgrade_separate.cpp \
	src/infra/security/policy/packet_integrity_cmd.cpp \
	src/infra/security/policy/packet_dispatcher.cpp \
	src/old_data/PVF/equipment/random_option_script.cpp \
	src/old_data/PVF/equipment/reseal_table_script.cpp \
	src/old_data/PVF/equipment/upgrade_table.cpp

B_OLD_DATA_PVF_EQUIPMENT_HEADERS := \
	src/old_data/PVF/equipment/equipment_script.h \
	src/old_data/PVF/equipment/item_upgrade.h \
	src/old_data/PVF/equipment/random_option_script.h \
	src/old_data/PVF/equipment/reseal_table_script.h \
	src/old_data/PVF/equipment/upgrade_table.h

B_OLD_DATA_PVF_ITEMSHOP_SRC := \
	src/old_data/PVF/itemshop/secret_shop_script.cpp

B_OLD_DATA_PVF_ITEMSHOP_HEADERS := \
	src/old_data/PVF/itemshop/secret_shop_script.h

# BEGIN B-PVF-BATCH-200-003
B_OLD_DATA_PVF_BATCH_200_003_SRC := \
	src/old_data/PVF/map/map_script_types.cpp \
	src/old_data/PVF/passiveobject/passive_object_script_types.cpp \
	src/old_data/PVF/passiveobject/passive_object_script.cpp \
	src/old_data/PVF/etc/title_book_info.cpp \
	src/old_data/PVF/common/arad_script_loader.cpp \
	src/old_data/PVF/etc/arad_special_use_item_script.cpp \
	src/old_data/PVF/skill/skill_script.cpp \
	src/old_data/PVF/itemshop/item_shop_script.cpp \
	src/old_data/PVF/n_quest/training_quest_script.cpp \
	src/old_data/PVF/itemshop/quest_shop_status.cpp \
	src/old_data/PVF/common/script_value.cpp \
	src/old_data/PVF/town/arad_village_attack_reward.cpp \
	src/old_data/PVF/equipment/growth_equipment_script.cpp

B_OLD_DATA_PVF_BATCH_200_003_HEADERS := \
	src/old_data/PVF/map/map_script_types.h \
	src/old_data/PVF/passiveobject/passive_object_script_types.h \
	src/old_data/PVF/etc/title_book_info.h \
	src/old_data/PVF/common/arad_script_loader.h \
	src/old_data/PVF/etc/arad_special_use_item_script.h \
	src/old_data/PVF/skill/skill_script.h \
	src/old_data/PVF/itemshop/item_shop_script.h \
	src/old_data/PVF/n_quest/training_quest_script.h \
	src/old_data/PVF/itemshop/quest_shop_status.h \
	src/old_data/PVF/common/script_value.h \
	src/old_data/PVF/town/arad_village_attack_reward.h \
	src/old_data/PVF/equipment/growth_equipment_script.h
# END B-PVF-BATCH-200-003

B_OLD_DATA_DB_BATCH_200_002_SRC := \
	src/old_data/db/account/clocalchina_db_break_away_reset.cpp \
	src/old_data/db/account/clocalchina_db_last_day_break_away.cpp \
	src/old_data/db/account/db_delete_account_cargo.cpp \
	src/old_data/db/account/db_load_break_away_prevent_data.cpp \
	src/old_data/db/account/db_update_break_away_uv_list.cpp \
	src/old_data/db/account/db_update_return_user_first_login.cpp \
	src/old_data/db/account/db_update_user_fatigue_quantity.cpp \
	src/old_data/db/auth_security/db_count_of_human_certify.cpp \
	src/old_data/db/auth_security/db_select_allow_proxy_user.cpp \
	src/old_data/db/anti_cheat/db_save_cleanpad_point.cpp \
	src/old_data/db/event/db_event_account_character_level_up_once_gift.cpp \
	src/old_data/db/event/db_event_growth_weapon_reset.cpp \
	src/old_data/db/event/db_event_save_growth_weapon.cpp \
	src/old_data/db/event/db_event_save_give_grow_creature.cpp \
	src/old_data/db/event/db_event_update_give_grow_creature.cpp \
	src/old_data/db/event/db_event_update_growth_weapon_charac.cpp \
	src/old_data/db/event/db_event_update_growth_weapon_timepiece.cpp \
	src/old_data/db/event/db_insert_attendance_event.cpp \
	src/old_data/db/event/db_save_give_avenger_title_flag.cpp \
	src/old_data/db/event/db_save_pc_room_play_time_event.cpp \
	src/old_data/db/event/db_update_bingo_event.cpp \
	src/old_data/db/event/db_update_blue_marble_enter_count.cpp \
	src/old_data/db/event/db_update_chuseok_package_purchase.cpp \
	src/old_data/db/event/db_update_happy_bean_info.cpp \
	src/old_data/db/event/db_update_ontime_last_recv_idx.cpp \
	src/old_data/db/avatar_aura/db_insert_cera_item_history.cpp \
	src/old_data/db/avatar_aura/db_update_aura_avatar_option.cpp \
	src/old_data/db/avatar_aura/db_update_avatar_history.cpp \
	src/old_data/db/avatar_aura/db_update_avatar_jewel_slot.cpp \
	src/old_data/db/auction_cashshop/db_load_auto_market_conditions_control.cpp \
	src/old_data/db/auction_cashshop/db_update_auto_market_conditions_control.cpp \
	src/old_data/db/auction_cashshop/db_update_gift_count.cpp \
	src/old_data/db/character/db_update_charac_play_time.cpp \
	src/old_data/db/inventory/db_insert_random_option_item_inform.cpp \
	src/old_data/db/inventory/db_save_item_dictionary_flag.cpp \
	src/old_data/db/inventory/db_save_redeem_item_list.cpp \
	src/old_data/db/inventory/db_update_inventory_capacity.cpp \
	src/old_data/db/inventory/db_update_avatar_item_lock.cpp \
	src/old_data/db/inventory/db_update_creature_item_lock.cpp \
	src/old_data/db/inventory/db_update_item_lock.cpp \
	src/old_data/db/dungeon/db_load_blood_best_record.cpp \
	src/old_data/db/dungeon/db_update_revenge_data.cpp \
	src/old_data/db/statistics_log/db_log_buy_item.cpp \
	src/old_data/db/equipment_upgrade/db_req_item_upgrade_log.cpp \
	src/old_data/db/mercenary/db_mercenary_log_info.cpp \
	src/old_data/db/account/db_save_pu_user_check.cpp

B_OLD_DATA_DB_BATCH_200_002_HEADERS := \
	src/old_data/db/account/break_away_payload.h \
	src/old_data/db/account/clocalchina_db_break_away_reset.h \
	src/old_data/db/account/clocalchina_db_last_day_break_away.h \
	src/old_data/db/account/db_delete_account_cargo.h \
	src/old_data/db/account/db_load_break_away_prevent_data.h \
	src/old_data/db/account/db_load_break_away_reward_data.h \
	src/old_data/db/account/db_save_pu_user_check.h \
	src/old_data/db/account/db_update_break_away_uv_list.h \
	src/old_data/db/account/db_update_return_user_first_login.h \
	src/old_data/db/account/db_update_user_fatigue_quantity.h \
	src/old_data/db/anti_cheat/db_save_cleanpad_point.h \
	src/old_data/db/auction_cashshop/db_load_auto_market_conditions_control.h \
	src/old_data/db/auction_cashshop/db_update_auto_market_conditions_control.h \
	src/old_data/db/auction_cashshop/db_update_gift_count.h \
	src/old_data/db/auth_security/db_count_of_human_certify.h \
	src/old_data/db/auth_security/db_select_allow_proxy_user.h \
	src/old_data/db/avatar_aura/avatar_item_payload.h \
	src/old_data/db/avatar_aura/db_insert_cera_item_history.h \
	src/old_data/db/avatar_aura/db_update_aura_avatar_option.h \
	src/old_data/db/avatar_aura/db_update_avatar_history.h \
	src/old_data/db/avatar_aura/db_update_avatar_jewel_slot.h \
	src/old_data/db/character/db_update_charac_play_time.h \
	src/old_data/db/dungeon/blood_best_record_payload.h \
	src/old_data/db/dungeon/db_load_blood_best_record.h \
	src/old_data/db/dungeon/db_update_revenge_data.h \
	src/old_data/db/equipment_upgrade/db_req_item_upgrade_log.h \
	src/old_data/db/event/db_event_account_character_level_up_once_gift.h \
	src/old_data/db/event/db_event_growth_weapon_reset.h \
	src/old_data/db/event/db_event_save_growth_weapon.h \
	src/old_data/db/event/db_event_save_give_grow_creature.h \
	src/old_data/db/event/db_event_update_give_grow_creature.h \
	src/old_data/db/event/db_event_update_growth_weapon_charac.h \
	src/old_data/db/event/db_event_update_growth_weapon_timepiece.h \
	src/old_data/db/event/db_insert_attendance_event.h \
	src/old_data/db/event/db_save_give_avenger_title_flag.h \
	src/old_data/db/event/db_save_pc_room_play_time_event.h \
	src/old_data/db/event/db_update_bingo_event.h \
	src/old_data/db/event/db_update_blue_marble_enter_count.h \
	src/old_data/db/event/db_update_chuseok_package_purchase.h \
	src/old_data/db/event/db_update_happy_bean_info.h \
	src/old_data/db/event/db_update_ontime_last_recv_idx.h \
	src/old_data/db/inventory/db_insert_random_option_item_inform.h \
	src/old_data/db/inventory/db_save_item_dictionary_flag.h \
	src/old_data/db/inventory/db_save_redeem_item_list.h \
	src/old_data/db/inventory/db_update_inventory_capacity.h \
	src/old_data/db/inventory/db_update_avatar_item_lock.h \
	src/old_data/db/inventory/db_update_creature_item_lock.h \
	src/old_data/db/inventory/db_update_item_lock.h \
	src/old_data/db/mercenary/db_mercenary_log_info.h \
	src/old_data/db/statistics_log/db_log_buy_item.h

# Root-owned narrow B route for SUBCLAIM-SR08-B-C-EVENT-DUNGEON-CONSENT-222.
B_OLD_DATA_DB_THIRD_PARTY_CONSENT_SRC := \
	src/old_data/db/auth_security/db_check_3rd_party_consent.cpp

B_OLD_DATA_DB_THIRD_PARTY_CONSENT_HEADERS := \
	src/old_data/db/auth_security/db_check_3rd_party_consent.h

# Root-owned narrow B route for SUBCLAIM-SR08-A-B-DB-LEAF-PROTOCOL-223.
B_OLD_DATA_DB_SR08_LEAF_PROTOCOL_SRC := \
	src/old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.cpp

B_OLD_DATA_DB_SR08_LEAF_PROTOCOL_HEADERS := \
	src/old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h \
	src/old_data/db/legacy_dispatch/db_sr08_leaf_dispatchers.h

# BEGIN B-DB-BATCH-200-004
B_OLD_DATA_DB_BATCH_200_004_SRC := \
	src/old_data/db/account/db_arrange_user_login_info.cpp \
	src/old_data/db/account/db_find_factory_hub_user.cpp \
	src/old_data/db/account/db_insert_account_first_login_daily_flag.cpp \
	src/old_data/db/account/db_insert_account_first_login_flag.cpp \
	src/old_data/db/account/db_login.cpp \
	src/old_data/db/account/db_load_pu_user_check.cpp \
	src/old_data/db/account/db_update_break_away_ruck_point.cpp \
	src/old_data/db/account/db_update_user_gift_fatigue_quantity.cpp \
	src/old_data/db/anti_cheat/db_hack_ip_save.cpp \
	src/old_data/db/anti_cheat/db_hades_punish.cpp \
	src/old_data/db/anti_cheat/db_load_clean_pad_point.cpp \
	src/old_data/db/anti_cheat/db_reload_auto_punish_rule_back_ip.cpp \
	src/old_data/db/anti_cheat/db_reload_auto_punish_rule_hack_type.cpp \
	src/old_data/db/anti_cheat/db_req_save_hack_user_punish.cpp \
	src/old_data/db/auction_cashshop/db_save_featured_goods_idx.cpp \
	src/old_data/db/auction_cashshop/db_billing_record.cpp \
	src/old_data/db/auction_cashshop/db_update_purchase_cash_item_bonus.cpp \
	src/old_data/db/auth_security/db_cancel_resting_user_restrict.cpp \
	src/old_data/db/auth_security/db_check_secu_reward.cpp \
	src/old_data/db/auth_security/db_passpad_update_cancel_cnt.cpp \
	src/old_data/db/auth_security/db_passpad_update_fail_cnt.cpp \
	src/old_data/db/auth_security/db_secu_service_reward.cpp \
	src/old_data/db/auth_security/db_secu_service_update_validity_time.cpp \
	src/old_data/db/auth_security/db_security_card_activate.cpp \
	src/old_data/db/auth_security/db_security_card_update_fail_cnt.cpp \
	src/old_data/db/auth_security/db_security_card_update_validity_time.cpp \
	src/old_data/db/auth_security/db_mouse_register.cpp \
	src/old_data/db/character/arad_create_query.cpp \
	src/old_data/db/character/db_check_charac_name.cpp \
	src/old_data/db/character/db_create_charac.cpp \
	src/old_data/db/character/db_get_user_regdate.cpp \
	src/old_data/db/character/db_load_charac.cpp \
	src/old_data/db/character/db_load_charac_hotkey_option.cpp \
	src/old_data/db/character/db_load_etc.cpp \
	src/old_data/db/character/load_etc_payload.cpp \
	src/old_data/db/character/load_etc_payload_lifecycle.cpp \
	src/old_data/db/character/load_etc_payload_value_types.cpp \
	src/old_data/db/character/db_load_no_cache_data.cpp \
	src/old_data/db/character/db_select_charac_mid.cpp \
	src/old_data/db/character/db_update_gm_data.cpp \
	src/old_data/db/creature/db_swap_creature_item.cpp \
	src/old_data/db/dungeon/db_get_death_tower_best_record.cpp \
	src/old_data/db/event/db_save_event_levelup_support.cpp \
	src/old_data/db/event/db_select_level_up_reward.cpp \
	src/old_data/db/event/db_update_add_item_count_value.cpp \
	src/old_data/db/event/db_update_ingame_event_history.cpp \
	src/old_data/db/event/db_update_seria_room_animation_deco.cpp \
	src/old_data/db/inventory/db_load_inventory.cpp \
	src/old_data/db/inventory/load_inventory_payload.cpp \
	src/old_data/db/pvp_party/db_guild_exp_book_delete.cpp \
	src/old_data/db/pvp_party/db_update_pvp_grade.cpp \
	src/old_data/db/quest/db_update_quest.cpp \
	src/old_data/db/skill/db_update_combo_skill.cpp \
	src/old_data/db/statistics_log/db_insert_ontime_reward_user_log.cpp \
	src/old_data/db/statistics_log/db_insert_unlimit_support_log.cpp \
	src/old_data/db/statistics_log/db_update_item_drop_statistics.cpp

B_OLD_DATA_DB_BATCH_200_004_HEADERS := \
	src/old_data/db/account/db_arrange_user_login_info.h \
	src/old_data/db/account/db_find_factory_hub_user.h \
	src/old_data/db/account/db_insert_account_first_login_daily_flag.h \
	src/old_data/db/account/db_insert_account_first_login_flag.h \
	src/old_data/db/account/db_login.h \
	src/old_data/db/account/login_payloads.h \
	src/old_data/db/account/db_load_pu_user_check.h \
	src/old_data/db/account/db_update_break_away_ruck_point.h \
	src/old_data/db/account/db_update_user_gift_fatigue_quantity.h \
	src/old_data/db/anti_cheat/db_hack_ip_save.h \
	src/old_data/db/anti_cheat/db_hades_punish.h \
	src/old_data/db/anti_cheat/db_load_clean_pad_point.h \
	src/old_data/db/anti_cheat/db_reload_auto_punish_rule_back_ip.h \
	src/old_data/db/anti_cheat/db_reload_auto_punish_rule_hack_type.h \
	src/old_data/db/anti_cheat/db_req_save_hack_user_punish.h \
	src/old_data/db/auction_cashshop/db_save_featured_goods_idx.h \
	src/old_data/db/auction_cashshop/db_billing_record.h \
	src/old_data/db/auction_cashshop/db_update_purchase_cash_item_bonus.h \
	src/old_data/db/auth_security/db_cancel_resting_user_restrict.h \
	src/old_data/db/auth_security/db_check_secu_reward.h \
	src/old_data/db/auth_security/db_passpad_update_cancel_cnt.h \
	src/old_data/db/auth_security/db_passpad_update_fail_cnt.h \
	src/old_data/db/auth_security/db_secu_service_reward.h \
	src/old_data/db/auth_security/db_secu_service_update_validity_time.h \
	src/old_data/db/auth_security/db_security_card_activate.h \
	src/old_data/db/auth_security/db_security_card_update_fail_cnt.h \
	src/old_data/db/auth_security/db_security_card_update_validity_time.h \
	src/old_data/db/auth_security/db_mouse_register.h \
	src/old_data/db/character/arad_create_query.h \
	src/old_data/db/character/create_charac_payload.h \
	src/old_data/db/character/db_check_charac_name.h \
	src/old_data/db/character/db_create_charac.h \
	src/old_data/db/character/db_get_user_regdate.h \
	src/old_data/db/character/db_load_charac.h \
	src/old_data/db/character/load_charac_payload.h \
	src/old_data/db/character/db_load_charac_hotkey_option.h \
	src/old_data/db/character/db_load_etc.h \
	src/old_data/db/character/load_etc_payload.h \
	src/old_data/db/character/db_load_no_cache_data.h \
	src/old_data/db/character/db_select_charac_mid.h \
	src/old_data/db/character/db_update_gm_data.h \
	src/old_data/db/creature/db_swap_creature_item.h \
	src/old_data/db/dungeon/db_get_death_tower_best_record.h \
	src/old_data/db/event/db_save_event_levelup_support.h \
	src/old_data/db/event/db_select_level_up_reward.h \
	src/old_data/db/event/db_update_add_item_count_value.h \
	src/old_data/db/event/db_update_ingame_event_history.h \
	src/old_data/db/event/db_update_seria_room_animation_deco.h \
	src/old_data/db/inventory/db_load_inventory.h \
	src/old_data/db/inventory/load_inventory_payload.h \
	src/old_data/db/pvp_party/db_guild_exp_book_delete.h \
	src/old_data/db/pvp_party/db_update_pvp_grade.h \
	src/old_data/db/quest/db_update_quest.h \
	src/old_data/db/skill/db_update_combo_skill.h \
	src/old_data/db/statistics_log/db_insert_ontime_reward_user_log.h \
	src/old_data/db/statistics_log/db_insert_unlimit_support_log.h \
	src/old_data/db/statistics_log/db_update_item_drop_statistics.h
# END B-DB-BATCH-200-004

# BEGIN B-DBLOGINCHANNEL-BATCH-200-005
# Only providers not already registered by an earlier B cluster are listed
# here.  The exact batch-wide 85-provider view is frozen in
# tools/batch_b_dbloginchannel_200_005_manifest.mk.
# ARAD inner return codes 41--43 (database load, monitor apply, monitor
# delete) are restored in the channel provider listed below.
B_OLD_DATA_DBLOGINCHANNEL_BATCH_200_005_SRC := \
	src/old_data/Login/garena/taiwan_garena_auth_data.cpp \
	src/old_data/Login/simple_sso.cpp \
	src/old_data/PVF/genuine_upgrade_table.cpp \
	src/old_data/channel/arad_server_state.cpp \
	src/old_data/channel/arad_internal_server_state_return_dispatchers.cpp \
	src/old_data/db/account/db_force_save_simple_sso.cpp \
	src/old_data/db/account/db_update_logout.cpp \
	src/old_data/db/anti_cheat/db_account_hack_check_my_account.cpp \
	src/old_data/db/anti_cheat/db_auto_punished_second_user_data.cpp \
	src/old_data/db/anti_cheat/db_insert_auto_punish_first_user.cpp \
	src/old_data/db/anti_cheat/db_insert_auto_punish_second_user.cpp \
	src/old_data/db/anti_cheat/db_save_client_hack_log.cpp \
	src/old_data/db/auction_cashshop/db_update_cerashop_purchase_count.cpp \
	src/old_data/db/auth_security/db_security_card_add_cert_cnt.cpp \
	src/old_data/db/auth_security/db_security_card_disuse.cpp \
	src/old_data/db/auth_security/db_security_card_issue.cpp \
	src/old_data/db/auth_security/db_security_card_retransfer.cpp \
	src/old_data/db/auth_security/db_security_card_update_cancel_cnt.cpp \
	src/old_data/db/avatar_aura/db_avatar_change_owner.cpp \
	src/old_data/db/avatar_aura/db_avatar_item_insert.cpp \
	src/old_data/db/character/db_delete_charac.cpp \
	src/old_data/db/character/db_req_change_charac_name.cpp \
	src/old_data/db/character/db_req_query_charac_info.cpp \
	src/old_data/db/character/db_request_tag_charac_info.cpp \
	src/old_data/db/character/db_save_charac_inform_notice.cpp \
	src/old_data/db/creature/db_insert_creature_item.cpp \
	src/old_data/db/creature/db_update_creature_item.cpp \
	src/old_data/db/dungeon/db_load_dimension.cpp \
	src/old_data/db/dungeon/db_load_dungeon_clear.cpp \
	src/old_data/db/dungeon/db_load_kill_monster_info.cpp \
	src/old_data/db/dungeon/db_save_best_clear_time.cpp \
	src/old_data/db/dungeon/db_save_blood_dungeon.cpp \
	src/old_data/db/dungeon/db_save_kill_monster_info.cpp \
	src/old_data/db/dungeon/db_save_dungeon_clear.cpp \
	src/old_data/db/dungeon/db_update_fair_pvp_score.cpp \
	src/old_data/db/dungeon/db_save_pvp_mission_list.cpp \
	src/old_data/db/dungeon/db_tod_update_user_state.cpp \
	src/old_data/db/event/db_create_dnf_user_info.cpp \
	src/old_data/db/event/db_hero_mission_event_data.cpp \
	src/old_data/db/event/db_insert_give_box_event_reward.cpp \
	src/old_data/db/event/db_pcroom_daily_reward.cpp \
	src/old_data/db/event/db_select_mobile_auth_reward_tw.cpp \
	src/old_data/db/event/db_select_create_dnf_event_info.cpp \
	src/old_data/db/event/taiwan_staytime_dispatchers.cpp \
	src/old_data/db/event/db_update_item_count_value.cpp \
	src/old_data/db/event/db_update_school_point.cpp \
	src/old_data/db/taiwan/taiwan_db_payloads.cpp \
	src/old_data/db/taiwan/taiwan_restrict_dispatchers.cpp \
	src/old_data/db/inventory/db_create_account_cargo.cpp \
	src/old_data/db/inventory/db_load_account_cargo.cpp \
	src/old_data/db/inventory/db_load_ticket.cpp \
	src/old_data/db/inventory/db_save_account_cargo.cpp \
	src/old_data/db/inventory/db_save_achievement.cpp \
	src/old_data/db/inventory/db_save_expand_equipslot.cpp \
	src/old_data/db/inventory/expand_equipslot_payload.cpp \
	src/old_data/db/inventory/db_save_title_book.cpp \
	src/old_data/db/inventory/db_upgrade_account_cargo.cpp \
	src/old_data/db/mail/db_change_letter_stat.cpp \
	src/old_data/db/mail/db_mailbox_req_list.cpp \
	src/old_data/db/mail/db_mailbox_req_send.cpp \
	src/old_data/db/mail/db_mailbox_send_new_mail_with_dbwork.cpp \
	src/old_data/db/mail/db_req_query_charac_info_mailbox.cpp \
	src/old_data/db/pvp_party/db_save_pvp.cpp \
	src/old_data/db/pvp_party/db_save_rank.cpp \
	src/old_data/db/pvp_party/db_save_user_handicap.cpp \
	src/old_data/db/quest/db_load_quest.cpp \
	src/old_data/db/skill/db_request_charac_skill_info.cpp \
	src/old_data/db/skill/db_update_one_day_lethe_skill.cpp

B_OLD_DATA_DBLOGINCHANNEL_BATCH_200_005_HEADERS := \
	src/old_data/Login/garena/taiwan_garena_auth_data.h \
	src/old_data/Login/simple_sso.h \
	src/old_data/PVF/genuine_upgrade_table.h \
	src/old_data/channel/arad_server_state.h \
	src/old_data/db/account/db_force_save_simple_sso.h \
	src/old_data/db/account/db_update_logout.h \
	src/old_data/db/anti_cheat/db_account_hack_check_my_account.h \
	src/old_data/db/anti_cheat/db_auto_punished_second_user_data.h \
	src/old_data/db/anti_cheat/db_insert_auto_punish_first_user.h \
	src/old_data/db/anti_cheat/db_insert_auto_punish_second_user.h \
	src/old_data/db/anti_cheat/db_save_client_hack_log.h \
	src/old_data/db/auction_cashshop/db_update_cerashop_purchase_count.h \
	src/old_data/db/auth_security/db_security_card_add_cert_cnt.h \
	src/old_data/db/auth_security/db_security_card_disuse.h \
	src/old_data/db/auth_security/db_security_card_issue.h \
	src/old_data/db/auth_security/db_security_card_retransfer.h \
	src/old_data/db/auth_security/db_security_card_update_cancel_cnt.h \
	src/old_data/db/avatar_aura/db_avatar_change_owner.h \
	src/old_data/db/avatar_aura/db_avatar_item_insert.h \
	src/old_data/db/character/db_delete_charac.h \
	src/old_data/db/character/db_req_change_charac_name.h \
	src/old_data/db/character/db_req_query_charac_info.h \
	src/old_data/db/character/db_request_tag_charac_info.h \
	src/old_data/db/character/db_save_charac_inform_notice.h \
	src/old_data/db/character/charac_inform_notice_payload.h \
	src/old_data/db/character/query_charac_info_payload.h \
	src/old_data/db/character/tag_charac_info_payload.h \
	src/old_data/db/creature/creature_item_payload.h \
	src/old_data/db/creature/db_insert_creature_item.h \
	src/old_data/db/creature/db_update_creature_item.h \
	src/old_data/db/dungeon/db_load_dimension.h \
	src/old_data/db/dungeon/db_load_dungeon_clear.h \
	src/old_data/db/dungeon/db_load_kill_monster_info.h \
	src/old_data/db/dungeon/db_save_best_clear_time.h \
	src/old_data/db/dungeon/db_save_blood_dungeon.h \
	src/old_data/db/dungeon/db_save_kill_monster_info.h \
	src/old_data/db/dungeon/db_save_dungeon_clear.h \
	src/old_data/db/dungeon/db_update_fair_pvp_score.h \
	src/old_data/db/dungeon/db_save_pvp_mission_list.h \
	src/old_data/db/dungeon/db_tod_update_user_state.h \
	src/old_data/db/dungeon/blood_dungeon_payload.h \
	src/old_data/db/dungeon/dungeon_clear_payload.h \
	src/old_data/db/event/db_create_dnf_user_info.h \
	src/old_data/db/event/db_hero_mission_event_data.h \
	src/old_data/db/event/db_insert_give_box_event_reward.h \
	src/old_data/db/event/db_pcroom_daily_reward.h \
	src/old_data/db/event/db_select_mobile_auth_reward_tw.h \
	src/old_data/db/event/db_select_create_dnf_event_info.h \
	src/old_data/db/event/taiwan_staytime_dispatchers.h \
	src/old_data/db/event/db_update_item_count_value.h \
	src/old_data/db/event/db_update_school_point.h \
	src/old_data/db/taiwan/taiwan_db_payloads.h \
	src/old_data/db/taiwan/taiwan_restrict_dispatchers.h \
	src/old_data/db/inventory/db_create_account_cargo.h \
	src/old_data/db/inventory/db_load_account_cargo.h \
	src/old_data/db/inventory/db_load_ticket.h \
	src/old_data/db/inventory/db_save_account_cargo.h \
	src/old_data/db/inventory/db_save_expand_equipslot.h \
	src/old_data/db/inventory/expand_equipslot_payload.h \
	src/old_data/db/inventory/db_save_title_book.h \
	src/old_data/db/inventory/title_book_payload.h \
	src/old_data/db/inventory/db_upgrade_account_cargo.h \
	src/old_data/db/mail/db_change_letter_stat.h \
	src/old_data/db/mail/letter_stat_payload.h \
	src/old_data/db/mail/db_mailbox_req_list.h \
	src/old_data/db/mail/db_mailbox_req_send.h \
	src/old_data/db/mail/db_mailbox_send_new_mail_with_dbwork.h \
	src/old_data/db/mail/db_req_query_charac_info_mailbox.h \
	src/old_data/db/pvp_party/db_save_pvp.h \
	src/old_data/db/pvp_party/db_save_rank.h \
	src/old_data/db/pvp_party/db_save_user_handicap.h \
	src/old_data/db/quest/db_load_quest.h \
	src/old_data/db/skill/db_request_charac_skill_info.h \
	src/old_data/db/skill/db_update_one_day_lethe_skill.h
# END B-DBLOGINCHANNEL-BATCH-200-005

# Continuation providers migrated atomically from the legacy datamanager,
# database, and channel source roots.  Headers retain their public include
# paths while the single implementation provider lives under B's old_data
# ownership boundary.
B_OLD_DATA_CONTINUATION_LEGACY_MIGRATION_SRC := \
	src/old_data/channel/legacy_runtime/channel_proxy.cpp \
	src/old_data/channel/legacy_runtime/leading_channel_event.cpp \
	src/old_data/channel/legacy_runtime/pvp_channel.cpp \
	src/old_data/db/legacy_dispatch/arad_database_dispatcher.cpp \
	src/old_data/db/legacy_dispatch/arad_internal_dispatcher.cpp \
	src/old_data/db/legacy_dispatch/arad_internal_load_reward_event_item.cpp \
	src/old_data/db/legacy_dispatch/arad_dispatcher_request.cpp \
	src/old_data/db/legacy_dispatch/arad_internal_stream_jpn.cpp \
	src/old_data/db/legacy_dispatch/best_clear_time_payload.cpp \
	src/old_data/db/legacy_dispatch/db_action_point_load.cpp \
	src/old_data/db/legacy_dispatch/db_action_point_reward_medal.cpp \
	src/old_data/db/legacy_dispatch/db_action_point_update.cpp \
	src/old_data/db/legacy_dispatch/db_avatar_color_change.cpp \
	src/old_data/db/legacy_dispatch/db_avatar_extend_period.cpp \
	src/old_data/db/legacy_dispatch/db_avatar_item_del.cpp \
	src/old_data/db/legacy_dispatch/db_avatar_item_move.cpp \
	src/old_data/db/legacy_dispatch/db_avatar_option_change.cpp \
	src/old_data/db/legacy_dispatch/db_conn_info.cpp \
	src/old_data/db/legacy_dispatch/db_connector.cpp \
	src/old_data/db/legacy_dispatch/db_fatigue_reset_timer.cpp \
	src/old_data/db/legacy_dispatch/db_find_factory_hub.cpp \
	src/old_data/db/legacy_dispatch/db_forbid_user_to_play.cpp \
	src/old_data/db/legacy_dispatch/db_get_premium_selected_cube_info.cpp \
	src/old_data/db/legacy_dispatch/db_insert_ticket.cpp \
	src/old_data/db/legacy_dispatch/db_last_msg_db.cpp \
	src/old_data/db/legacy_dispatch/db_last_msg_log_db.cpp \
	src/old_data/db/legacy_dispatch/db_legacy_stat_dispatchers.cpp \
	src/old_data/db/legacy_dispatch/db_load_mileage.cpp \
	src/old_data/db/legacy_dispatch/db_log_ingame_advertisement.cpp \
	src/old_data/db/legacy_dispatch/db_pvp_stat_dispatchers.cpp \
	src/old_data/db/legacy_dispatch/db_remove_featured_goods_idx.cpp \
	src/old_data/db/legacy_dispatch/db_save_account_info.cpp \
	src/old_data/db/legacy_dispatch/db_save_charac_info4guild.cpp \
	src/old_data/db/legacy_dispatch/db_save_mileage.cpp \
	src/old_data/db/legacy_dispatch/db_save_premium_info.cpp \
	src/old_data/db/legacy_dispatch/db_save_proxy_log.cpp \
	src/old_data/db/legacy_dispatch/db_save_same_mid_other_char_login_hack.cpp \
	src/old_data/db/legacy_dispatch/db_suicide_when_queue_is_empty.cpp \
	src/old_data/db/legacy_dispatch/db_terminate_server.cpp \
	src/old_data/db/legacy_dispatch/db_update_blood_best_record.cpp \
	src/old_data/db/legacy_dispatch/db_update_booster_gage_data.cpp \
	src/old_data/db/legacy_dispatch/db_update_break_away_clear_add_info.cpp \
	src/old_data/db/legacy_dispatch/db_update_character_day.cpp \
	src/old_data/db/legacy_dispatch/db_update_expert_job_statistic.cpp \
	src/old_data/db/legacy_dispatch/db_update_grow_type.cpp \
	src/old_data/db/legacy_dispatch/db_update_limit_item_usage.cpp \
	src/old_data/db/legacy_dispatch/db_update_login.cpp \
	src/old_data/db/legacy_dispatch/db_update_nat_type.cpp \
	src/old_data/db/legacy_dispatch/db_update_nat_type_user.cpp \
	src/old_data/db/legacy_dispatch/db_update_npc_relationship.cpp \
	src/old_data/db/legacy_dispatch/db_update_quest_shop_buying_status_list.cpp \
	src/old_data/db/legacy_dispatch/db_write_geo_reject.cpp \
	src/old_data/db/legacy_dispatch/reward_event_request.cpp \
	src/old_data/PVF/legacy_datamanager/advance_altar_shop_import.cpp \
	src/old_data/PVF/legacy_datamanager/advance_altar_timeline_import.cpp \
	src/old_data/PVF/legacy_datamanager/ai_character_script_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/independent_drop_generator_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/independent_drop_owner_wrappers.cpp \
	src/old_data/PVF/legacy_datamanager/arad_event_period_data_manager.cpp \
	src/old_data/PVF/legacy_datamanager/arad_event_reward.cpp \
	src/old_data/PVF/legacy_datamanager/arad_reward_functor.cpp \
	src/old_data/PVF/legacy_datamanager/broadcast_item_script_data.cpp \
	src/old_data/PVF/legacy_datamanager/captcha_data_manager.cpp \
	src/old_data/PVF/legacy_datamanager/character_script_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/create_chrac_script.cpp \
	src/old_data/PVF/legacy_datamanager/daily_schedule_script.cpp \
	src/old_data/PVF/legacy_datamanager/data_manager_init.cpp \
	src/old_data/PVF/legacy_datamanager/data_manager_runtime_queries.cpp \
	src/old_data/PVF/legacy_datamanager/db_log_item.cpp \
	src/old_data/PVF/legacy_datamanager/equipment_parameter_condition_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/etc_parameter_script.cpp \
	src/old_data/PVF/legacy_datamanager/growth_booster_script.cpp \
	src/old_data/PVF/legacy_datamanager/image_communication_info.cpp \
	src/old_data/PVF/legacy_datamanager/in_game_advertisement_script.cpp \
	src/old_data/PVF/legacy_datamanager/levelup_support_2nd_script.cpp \
	src/old_data/PVF/legacy_datamanager/limit_item_usage_info.cpp \
	src/old_data/PVF/legacy_datamanager/cmap_path_gate.cpp \
	src/old_data/PVF/legacy_datamanager/dungeon_move_select_providers.cpp \
	src/old_data/PVF/legacy_datamanager/map_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/map_boss_consist_accessors.cpp \
	src/old_data/PVF/legacy_datamanager/monster_avatar_catch_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/monster_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/npc_relation_event_script.cpp \
	src/old_data/PVF/legacy_datamanager/npc_script_runtime.cpp \
	src/old_data/PVF/legacy_datamanager/pet_script.cpp \
	src/old_data/PVF/legacy_datamanager/pvp_mission_system.cpp \
	src/old_data/PVF/legacy_datamanager/quest_shop_script.cpp \
	src/old_data/PVF/legacy_datamanager/script_sync_packets.cpp \
	src/old_data/PVF/legacy_datamanager/script_tag.cpp \
	src/old_data/PVF/legacy_datamanager/seria_room_deco_animation_event_script.cpp \
	src/old_data/PVF/legacy_datamanager/server_parameter_script.cpp \
	src/old_data/PVF/legacy_datamanager/server_parameter_emblem_compound.cpp \
	src/old_data/PVF/legacy_datamanager/cdata_manager_server_parameter_access.cpp \
	src/old_data/PVF/legacy_datamanager/cdata_manager_emotion_type_tags_access.cpp \
	src/old_data/PVF/legacy_datamanager/cdata_manager_equipment_grade_sample_access.cpp \
	src/old_data/PVF/legacy_datamanager/cdata_manager_broadcast_item_access.cpp \
	src/old_data/PVF/legacy_datamanager/cdata_manager_arad_jumping_access.cpp \
	src/old_data/PVF/legacy_datamanager/skill_quest_runtime.cpp \
	src/old_game/progression/quest/quest_power_side_runtime.cpp \
	src/old_game/world/region/power_manager_winner_side.cpp \
	src/old_data/PVF/legacy_datamanager/skill_shop_script.cpp \
	src/old_data/PVF/legacy_datamanager/stage_map_script.cpp \
	src/old_data/PVF/legacy_datamanager/tile_script.cpp \
	src/old_data/PVF/legacy_datamanager/user_premium_load.cpp \
	src/old_data/PVF/legacy_datamanager/village_attacked_script.cpp \
	src/old_data/PVF/legacy_datamanager/war_area_counter_import.cpp \
	src/old_data/PVF/legacy_datamanager/world_drop_script.cpp \
	src/old_data/PVF/legacy_datamanager/world_map_script.cpp \
	src/old_data/PVF/legacy_datamanager/world_stage_map_runtime.cpp

B_OLD_DATA_CONTINUATION_SEND_RESULT_SRC := \
	src/old_game/player/core/character_db_payloads.cpp \
	src/old_data/PVF/equipment/random_option_item_handle.cpp \
	src/old_data/PVF/data/item_generator_mgr_runtime.cpp \
	src/old_data/PVF/data/item_generator_types.cpp \
	src/old_data/PVF/data/item_generator_drop_types.cpp \
	src/old_data/PVF/data/item_generator_aux_drop_runtime.cpp \
	src/old_data/PVF/data/item_generator_hell_party_runtime.cpp \
	src/old_data/PVF/data/item_generator_primitives_runtime.cpp \
	src/old_data/PVF/data/item_generator_drop_import_runtime.cpp \
	src/old_data/PVF/data/data_manager_emoticon_runtime.cpp \
	src/old_data/db/legacy_dispatch/packet_dispatcher_send_mess_runtime.cpp \
	src/old_data/PVF/arad/arad_script_manager.cpp \
	src/old_data/db/auction_cashshop/db_load_cerashop_add_restrict_type.cpp \
	src/old_data/db/auction_cashshop/db_save_cerashop_add_restrict_type.cpp \
	src/old_data/PVF/common/dnf_flex_lexer.cpp \
	src/old_data/db/legacy_dispatch/db_final_refill_exact27_remainder.cpp \
	src/old_data/db/legacy_dispatch/db_check_add_friend.cpp \
	src/old_data/db/pvp_party/db_online_preliminary_noop_dispatchers.cpp \
	src/old_data/db/legacy_dispatch/db_crashdown_advertisement_dispatch.cpp \
	src/old_data/db/legacy_dispatch/db_load_server_message.cpp \
	src/old_data/db/legacy_dispatch/db_update_server_message.cpp \
	src/old_data/db/event/advance_altar_db.cpp \
	src/old_data/db/character/db_save_charac.cpp \
	src/old_data/db/inventory/db_save_inven.cpp \
	src/old_data/db/skill/db_save_skill.cpp

B_OLD_DATA_DB_EXPERT_JOB_SRC := \
	src/old_data/db/quest/db_load_expert_job.cpp \
	src/old_data/db/quest/db_save_expert_job.cpp

B_OLD_DATA_DB_EXPERT_JOB_HEADERS := \
	src/old_data/db/quest/expert_job_payload.h \
	src/old_data/db/quest/db_load_expert_job.h \
	src/old_data/db/quest/db_save_expert_job.h

# CMD20's dispatcher is B-owned, but its exact qsort providers belong to the
# existing inventory ABI.  Keep this one source in B so the provider has one
# object owner without changing C's manifest during the closure phase.
B_OLD_GAME_INVENTORY_SORT_SRC := \
	src/old_game/economy/inventory/cinventory_sort_runtime.cpp

B_OLD_DATA_SRC := \
	$(B_OLD_DATA_DB_DISPATCHER_GAP_SRC) \
	$(B_OLD_DATA_DB_REMAINING_SLOT_RESTORE_SRC) \
	$(B_OLD_DATA_DB_ITEM_MAKING_SKILL_SRC) \
	$(B_OLD_DATA_DB_EXPERT_JOB_SRC) \
	$(B_OLD_GAME_INVENTORY_SORT_SRC) \
	$(B_OLD_DATA_CONTINUATION_SEND_RESULT_SRC) \
	$(B_OLD_DATA_CONTINUATION_LEGACY_MIGRATION_SRC) \
	$(B_OLD_DATA_PVF_BATCH_200_003_SRC) \
	$(B_OLD_DATA_DB_BATCH_200_004_SRC) \
	$(B_OLD_DATA_DBLOGINCHANNEL_BATCH_200_005_SRC) \
	$(B_OLD_DATA_DB_BATCH_200_002_SRC) \
	$(B_OLD_DATA_DB_THIRD_PARTY_CONSENT_SRC) \
	$(B_OLD_DATA_DB_SR08_LEAF_PROTOCOL_SRC) \
	$(B_OLD_DATA_DB_CORE_SRC) \
	$(B_OLD_DATA_DB_GAME_OPTION_SRC) \
	$(B_OLD_DATA_DB_CHANNEL_SRC) \
	$(B_OLD_DATA_DB_STATISTICS_LOG_SRC) \
	$(B_OLD_DATA_DB_PRIVATE_STORE_SRC) \
	$(B_OLD_DATA_DB_MERCENARY_SRC) \
	$(B_OLD_DATA_DB_EVENT_SRC) \
	$(B_OLD_DATA_DB_PVP_PARTY_SRC) \
	$(B_OLD_DATA_DB_ANTI_CHEAT_SRC) \
	$(B_OLD_DATA_LOGIN_ACCOUNT_CARGO_SRC) \
	$(B_OLD_DATA_LOGIN_CORE_SRC) \
	$(B_OLD_DATA_LOGIN_INTER_RETURNS_SRC) \
	$(B_OLD_DATA_LOGIN_SECURITY_SRC) \
	$(B_OLD_DATA_LOGIN_PACKETDISPATCH_SRC) \
	$(B_OLD_DATA_LOGIN_PACKETS_SRC) \
	$(B_OLD_DATA_PVF_COMMON_SRC) \
	$(B_OLD_DATA_PVF_AICHARACTER_SRC) \
	$(B_OLD_DATA_PVF_LOCALIZATION_SRC) \
	$(B_OLD_DATA_PVF_CREATURE_SRC) \
	$(B_OLD_DATA_PVF_CHARACTER_SRC) \
	$(B_OLD_DATA_PVF_NPC_SRC) \
	$(B_OLD_DATA_PVF_TOWN_SRC) \
	$(B_OLD_DATA_PVF_STACKABLE_SRC) \
	$(B_OLD_DATA_PVF_EVENT_SRC) \
	$(B_OLD_DATA_PVF_ETC_SRC) \
	$(B_OLD_DATA_PVF_DUNGEON_SRC) \
	$(B_OLD_DATA_PVF_MAP_SRC) \
	$(B_OLD_DATA_PVF_N_QUEST_SRC) \
	$(B_OLD_DATA_PVF_PVP_MISSION_SRC) \
	$(B_OLD_DATA_CHANNEL_SRC) \
	$(B_OLD_DATA_CHANNEL_PACKETDISPATCH_SRC) \
	$(B_OLD_DATA_PVF_DATA_SRC) \
	$(B_OLD_DATA_PVF_SYNC_SRC) \
	$(B_OLD_DATA_PVF_EQUIPMENT_SRC) \
	$(B_OLD_DATA_PVF_ITEMSHOP_SRC)

B_OLD_DATA_HEADERS := \
	$(B_OLD_DATA_DB_ITEM_MAKING_SKILL_HEADERS) \
	$(B_OLD_DATA_DB_REMAINING_SLOT_RESTORE_HEADERS) \
	$(B_OLD_DATA_DB_EXPERT_JOB_HEADERS) \
	$(B_OLD_DATA_PVF_BATCH_200_003_HEADERS) \
	$(B_OLD_DATA_DB_BATCH_200_004_HEADERS) \
	$(B_OLD_DATA_DBLOGINCHANNEL_BATCH_200_005_HEADERS) \
	$(B_OLD_DATA_DB_BATCH_200_002_HEADERS) \
	$(B_OLD_DATA_DB_THIRD_PARTY_CONSENT_HEADERS) \
	$(B_OLD_DATA_DB_SR08_LEAF_PROTOCOL_HEADERS) \
	$(B_OLD_DATA_DB_CORE_HEADERS) \
	$(B_OLD_DATA_DB_GAME_OPTION_HEADERS) \
	$(B_OLD_DATA_DB_CHANNEL_HEADERS) \
	$(B_OLD_DATA_DB_STATISTICS_LOG_HEADERS) \
	src/old_data/db/private_store/db_save_private_store.h \
	src/old_data/db/private_store/db_load_private_store.h \
	src/old_data/db/mercenary/db_save_mercenary_data.h \
	src/old_data/db/event/db_save_object_bring_up.h \
	src/old_data/db/event/db_statistic_advance_altar.h \
	src/old_data/db/event/db_req_save_crazy_levelup_event.h \
	$(B_OLD_DATA_DB_ANTI_CHEAT_HEADERS) \
	$(B_OLD_DATA_LOGIN_CORE_HEADERS) \
	src/old_data/db/legacy_dispatch/db_check_add_friend.h \
	src/old_data/db/legacy_dispatch/geo_reject_payload.h \
	$(B_OLD_DATA_LOGIN_ACCOUNT_CARGO_HEADERS) \
	$(B_OLD_DATA_LOGIN_INTER_RETURNS_HEADERS) \
	$(B_OLD_DATA_LOGIN_SECURITY_HEADERS) \
	$(B_OLD_DATA_LOGIN_PACKETDISPATCH_HEADERS) \
	$(B_OLD_DATA_LOGIN_PACKETS_HEADERS) \
	$(B_OLD_DATA_AUTH_SECURITY_PAYLOAD_HEADERS) \
	$(B_OLD_DATA_PVF_COMMON_HEADERS) \
	$(B_OLD_DATA_PVF_AICHARACTER_HEADERS) \
	$(B_OLD_DATA_PVF_LOCALIZATION_HEADERS) \
	$(B_OLD_DATA_PVF_CREATURE_HEADERS) \
	$(B_OLD_DATA_PVF_CHARACTER_HEADERS) \
	$(B_OLD_DATA_PVF_NPC_HEADERS) \
	$(B_OLD_DATA_PVF_TOWN_HEADERS) \
	$(B_OLD_DATA_PVF_STACKABLE_HEADERS) \
	$(B_OLD_DATA_PVF_EVENT_HEADERS) \
	$(B_OLD_DATA_PVF_ETC_HEADERS) \
	$(B_OLD_DATA_PVF_DUNGEON_HEADERS) \
	$(B_OLD_DATA_PVF_MAP_HEADERS) \
	$(B_OLD_DATA_PVF_N_QUEST_HEADERS) \
	$(B_OLD_DATA_PVF_PVP_MISSION_HEADERS) \
	$(B_OLD_DATA_CHANNEL_HEADERS) \
	$(B_OLD_DATA_CHANNEL_PACKETDISPATCH_HEADERS) \
	$(B_OLD_DATA_PVF_DATA_HEADERS) \
	$(B_OLD_DATA_PVF_SYNC_HEADERS) \
	$(B_OLD_DATA_PVF_EQUIPMENT_HEADERS) \
	$(B_OLD_DATA_PVF_ITEMSHOP_HEADERS)

# Root Makefile consumes only these B-owned variables.
B_SRC := $(B_OLD_DATA_SRC)
B_HEADERS := $(B_OLD_DATA_HEADERS)

# PacketDispatcher 606-slot closure: restored B-owned account-cargo
# dispatchers.  The implementation remains under the DB owner directory.
B_SRC += src/old_data/db/account_cargo/packet_dispatcher_account_cargo.cpp
B_SRC += src/old_data/Login/account_cargo/account_cargo_actions.cpp
B_SRC += src/old_data/db/inventory/db_item_growth_state.cpp
B_HEADERS += src/old_data/db/inventory/db_item_growth_state.h
B_SRC += src/old_data/db/character/db_clone_title.cpp
B_HEADERS += src/old_data/db/character/db_clone_title.h
B_SRC += src/old_data/db/character/db_charac_link.cpp
B_HEADERS += src/old_data/db/character/db_charac_link.h
B_SRC += src/old_data/db/event/db_collect_box.cpp
B_HEADERS += src/old_data/db/event/db_collect_box.h
B_SRC += src/old_data/db/pvp_party/db_integrated_match_pvp_score.cpp
B_HEADERS += src/old_data/db/pvp_party/db_integrated_match_pvp_score.h
