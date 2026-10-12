# C-owned old-server gameplay runtime manifest.
#
# Only canonical gameplay sources belong here.  Numbered legacy directories
# are deliberately absent: after migration they remain callers or migration
# markers and must not provide a second IDA business body.
# 140828 altar/tournament consumers remain in the canonical dungeon, party,
# and battlefield entries below; the focused B/C closure gate is
# tools/tests/verify_blood_tournament_pvf_0828.py.
# C-USERINFO-ASM-BATCH-022 checkpoint: 4x50, exact-200 NM and clean owner L1 PASS; A/B consumer handoffs pending.
# C-GAMEPLAY-ASM-BATCH-023 checkpoint: slice 1/4 exact-50 focused G++4.9/NM PASS; slice 2 is active.
# C-GAMEPLAY-ASM-BATCH-025 IN_PROGRESS: the frozen exact-1000 ledger is
# static/NM clean.  Remote G++4.9 compiled all 161 C sources at that frozen
# checkpoint.
# C-GAMEPLAY-BUSINESS-BATCH-026 IN_PROGRESS: the first exact 200 of the next
# 1,000-function business-closure delivery are frozen and focused G++4.9/NM
# clean.  The canonical manifest currently has 167 unique, existing sources;
# after removal of one provably unused stale include, the full 167-object
# G++4.9 owner probe and exact-200 raw-NM gate both pass.
# Batch-025 raw NM remains missing 0/duplicate 0 for its exact ledger.  C-owned
# pseudo providers
# have been removed or restored as true members; L2/L3 are not claimed.

C_CREATURE_AVATAR_SRC := \
	src/old_game/appearance/avatar/avatar_manager_closure_batch027.cpp \
	src/old_game/appearance/avatar/cavatar_item_mgr_assignment_runtime.cpp \
	src/old_game/appearance/avatar/avatar_recharge_endurance_save.cpp \
	src/old_game/gm/item/avatar_recharge_emblem_gm_runtime.cpp \
	src/old_game/appearance/avatar/avatar_recharge_fatigue_runtime.cpp \
	src/old_game/appearance/avatar/avatar_recharge_move_item_runtime.cpp \
	src/old_game/appearance/avatar/avatar_fixed_hidden_option_server.cpp \
	src/old_game/appearance/avatar/arad_avatar_script_services.cpp \
	src/old_game/appearance/avatar/arad_internal_return_avatar_hidden_option_change.cpp \
	src/old_game/appearance/avatar/arad_internal_return_avatar_roulette.cpp \
	src/old_game/appearance/avatar/arad_internal_return_avatar_recharge_compound_convert.cpp \
	src/old_game/appearance/avatar/emblem_compound_server_process.cpp \
	src/old_game/appearance/avatar/avatar_compound_convert_conditions.cpp \
	src/old_game/appearance/avatar/avatar_convert_server_transaction.cpp \
	src/old_game/appearance/avatar/avatar_convert_emblem_statistic.cpp \
	src/old_game/appearance/avatar/compound_emblem_statistic.cpp \
	src/old_game/appearance/avatar/packet_dispatcher_compound_emblem.cpp \
	src/old_game/appearance/avatar/arad_avatar_packet_signals.cpp \
	src/old_game/appearance/pet/avatar_creature_runtime_accessors.cpp \
	src/old_game/appearance/avatar/cavatar_item_mgr_erase_runtime.cpp \
	src/old_game/appearance/avatar/cavatar_item_mgr_reset.cpp \
	src/old_game/appearance/pet/creature_avatar_value_state_batch026_runtime.cpp \
	src/old_game/appearance/pet/creature_item_converter.cpp \
	src/old_game/appearance/pet/creature_item_factory_runtime.cpp \
	src/old_game/appearance/pet/creature_item_list_packets.cpp \
	src/old_game/appearance/pet/ccreature_mgr_item_result.cpp \
	src/old_game/appearance/pet/inter_creature_item_returns.cpp \
	src/old_game/appearance/avatar/cavatar_item_mgr_db_load_runtime.cpp \
	src/old_game/appearance/avatar/inter_avatar_db_returns.cpp \
	src/old_game/appearance/pet/ccreature_mgr_rename.cpp \
	src/old_game/appearance/pet/ccreature_hatch_runtime.cpp \
	src/old_game/appearance/pet/ccreature_response_runtime.cpp \
	src/old_game/appearance/pet/creature_state_closure_batch027.cpp \
	src/old_game/appearance/pet/ccreature_mgr_is_save_creature_item.cpp \
	src/old_game/appearance/pet/ccreature_mgr_get_creature_item.cpp \
	src/old_game/appearance/pet/ccreature_mgr_keep_right_item.cpp \
	src/old_game/appearance/pet/ccreature_mgr_reset.cpp \
	src/old_game/appearance/pet/ccreature_mgr_save_creature_item.cpp \
	src/old_game/appearance/pet/ccreature_cmd372_preabi.cpp \
	src/old_game/appearance/pet/ccreature_die_runtime.cpp \
	src/old_game/appearance/pet/ccreature_item_evolute_runtime.cpp \
	src/old_game/appearance/pet/ccreature_save_creature_item.cpp \
	src/old_game/appearance/pet/ccreature_set_state.cpp \
	src/old_game/appearance/pet/creature_fillpoint_runtime.cpp \
	src/old_game/appearance/pet/creature_fillpoint_business_runtime.cpp \
	src/old_game/appearance/pet/packet_dispatcher_compound_creature.cpp \
	src/old_game/appearance/pet/packet_dispatcher_creature_fillpoint.cpp \
	src/old_game/appearance/avatar/cdb_msg_sender.cpp \
	src/old_game/appearance/avatar/compound_avatar_lifecycle_runtime.cpp \
	src/old_game/appearance/avatar/compound_avatar_runtime.cpp \
	src/old_game/appearance/avatar/compound_avatar_value_types.cpp \
	src/old_game/appearance/avatar/packet_dispatcher_compound_avatar.cpp \
	src/old_game/appearance/avatar/packet_dispatcher_disjoint_avatar.cpp

# ARAD internal packet codes 44--46 are allocated by B's
# Arad_InternalDispatcher.  Their concrete C-owned handlers (including the
# individually-addressable C2/D2/D0 trios) must participate in the link; the
# B owner only owns the map and registration order.
C_CREATURE_AVATAR_SRC += src/old_game/player/session/arad_internal_return_jumping_character.cpp

# PvP Buddy CMD290--292 owns its complete provider surface in
# packet_dispatcher_pvp_buddy.cpp; the aggregate dispatcher does not provide it.
C_DUNGEON_PVP_SRC := \
	src/old_game/pvp/match/matching_channel_handoff.cpp \
	src/old_game/pvp/match/channel_zombie_matching_producer.cpp \
	src/old_game/pvp/match/channel_zombie_matching_authority.cpp \
	src/old_game/dungeon/raid/dual_raid.cpp \
	src/old_game/dungeon/racing/racing_dungeon.cpp \
	src/old_game/dungeon/escort/golden_commerce.cpp \
	src/old_game/pvp/tournament/pvp_tournament.cpp \
	src/old_game/pvp/battlefield/battle_field_accessors.cpp \
	src/old_game/pvp/battlefield/battle_field_blood_clear_reward_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_blood_monster_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_blood_round_accessors_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_blood_round_runtime.cpp \
	src/old_game/pvp/battlefield/timer_blood_round_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_ultimate_round_runtime.cpp \
	src/old_game/pvp/battlefield/timer_ultimate_next_round_runtime.cpp \
	src/old_game/dungeon/instance/dispatcher_blood_round_ui_prepare_finish.cpp \
	src/old_game/pvp/battlefield/battle_field_consist_map_runtime.cpp \
	src/old_game/pvp/battlefield/battle_map_dependencies_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_front_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_sdc_damage_runtime.cpp \
	src/old_game/dungeon/instance/sdc_damage_compute.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_sdc_damage_check.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_sdc_active_status_damage_check.cpp \
	src/old_game/pvp/battlefield/battle_field_clear_map_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_grid_clear_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_item_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_finish_loading_runtime.cpp \
	src/old_game/dungeon/instance/timer_elevator_dungeon_runtime.cpp \
	src/old_game/dungeon/tower/timer_boss_tower_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_drop_item_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_die_mob_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_move_helpers_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_move_select_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_select_helpers_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_set_grid_path_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_pool_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_eplp_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_map_info_runtime.cpp \
	src/old_game/pvp/battlefield/battle_field_packet_runtime.cpp \
	src/old_game/dungeon/champion_break/champion_break_runtime.cpp \
	src/old_game/pvp/battlefield/packet_dispatcher_pvp_practice_combo_score.cpp \
	src/old_game/dungeon/mission/dungeon_mission.cpp \
	src/old_game/dungeon/mission/packet_dispatcher_dungeon_mission.cpp \
	src/old_game/pvp/battlefield/battle_field_state_machine_batch028.cpp \
	src/old_game/social/party/battle_field_quick_party_runtime.cpp \
	src/old_game/dungeon/progress/best_clear_time_runtime.cpp \
	src/old_game/dungeon/progress/dragon_timeattack_rank_runtime.cpp \
	src/old_game/dungeon/instance/bind_cube_lv_bonus_script_data.cpp \
	src/old_game/dungeon/instance/bind_cube_script_data.cpp \
	src/old_game/dungeon/tower/boss_tower_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_finish_loading_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_leaf_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_consist_map_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_gold_pickup_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_drop_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_kill_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_move_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_stage_pickup_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_die_mob_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_pool_accessors_runtime.cpp \
	src/old_game/dungeon/tower/boss_tower_typed_lifecycle_runtime.cpp \
	src/old_game/dungeon/instance/compound_extream_dun_item_runtime.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_compound_extream_item.cpp \
	src/old_game/dungeon/room/pc_room_public_abi.cpp \
	src/old_game/dungeon/room/inter_pcroom_player_response.cpp \
	src/old_game/dungeon/room/inter_pcroom_response.cpp \
	src/old_game/dungeon/instance/nexon_msg_table_runtime.cpp \
	src/old_game/dungeon/room/pc_room_server_proxy_runtime.cpp \
	src/old_game/dungeon/room/rental_machine_pc_room_runtime.cpp \
	src/old_game/dungeon/room/packet_dispatcher_pc_room_rent.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_use_coin.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_give_up_game.cpp \
	src/old_game/dungeon/drop/packet_dispatcher_drop_item.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_die_mob.cpp \
	src/old_game/dungeon/instance/zombie_itembox_runtime.cpp \
	src/old_game/dungeon/instance/zombie_map_runtime.cpp \
	src/old_game/dungeon/instance/zombie_mode_result.cpp \
	src/old_game/dungeon/instance/monster_move_system.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_finish_load.cpp \
	src/old_game/dungeon/room/packet_dispatcher_seria_room_deco.cpp \
	src/old_game/dungeon/instance/character_blood_dungeon_runtime.cpp \
	src/old_game/dungeon/progress/cdungeon_clear_to_string.cpp \
	src/old_game/dungeon/progress/dungeon_clear_enum_runtime.cpp \
	src/old_game/dungeon/progress/dungeon_clear_runtime.cpp \
	src/old_game/dungeon/instance/condition_level_chk_dungeon_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_accessors.cpp \
	src/old_game/dungeon/tower/death_tower_start_attach_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_consist_map_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_die_mob_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_clear_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_stage_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_ranking_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_value_statistic_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_playdata_statistic_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_entrance_log_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_pvp_dispatchers.cpp \
	src/old_game/pvp/arena/packet_dispatcher_make_war_room_temp.cpp \
	src/old_game/dungeon/tower/packet_dispatcher_death_tower_stage_command.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_derived_batch040.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_report_bad_p2p_user.cpp \
	src/old_game/dungeon/instance/luck_point_runtime.cpp \
	src/old_game/dungeon/instance/ridable_object_state_runtime.cpp \
	src/old_game/pvp/battlefield/packet_dispatcher_pvp_assault.cpp \
	src/old_game/pvp/battlefield/packet_dispatcher_pvp_buddy.cpp \
	src/old_game/social/party/packet_dispatcher_set_party_info.cpp \
	src/old_game/pvp/battlefield/dispatcher_request_pvp_exp_of_week.cpp \
	src/old_game/social/party/packet_dispatcher_call_party_member_realtime_info.cpp \
	src/old_game/pvp/season/packet_dispatcher_res_pvp_rank.cpp \
	src/old_game/pvp/battlefield/integrated_match_pvp_score_runtime.cpp \
	src/old_game/pvp/battlefield/dispatcher_fair_pvp_score.cpp \
	src/old_game/dungeon/instance/dungeon_pvp_packets.cpp \
	src/old_game/dungeon/instance/dungeon_pvp_value_types.cpp \
	src/old_game/dungeon/instance/dungeon_statistic_database_runtime.cpp \
	src/old_game/pvp/battlefield/statistics_pvp_runtime.cpp \
	src/old_game/social/party/dungeon_stat_party_batch027.cpp \
	src/old_game/dungeon/ai/dungeon_stat_monster_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_pvp_leave_leaf_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_persistence_value_states.cpp \
	src/old_game/pvp/battlefield/fair_pvp_score_runtime.cpp \
	src/old_game/pvp/battlefield/fair_pvp_team_info_runtime.cpp \
	src/old_game/pvp/match/fair_pvp_match_runtime.cpp \
	src/old_game/pvp/arena/fair_pvp_waiting_room_runtime.cpp \
	src/old_game/dungeon/ai/kill_monster_info_runtime.cpp \
	src/old_game/pvp/match/league_match_leaf_runtime.cpp \
	src/old_game/pvp/match/league_match_room_list_runtime.cpp \
	src/old_game/pvp/match/league_match_core_runtime.cpp \
	src/old_game/pvp/match/league_match_room_handoff_runtime.cpp \
	src/old_game/pvp/match/league_match_static_state_runtime.cpp \
	src/old_game/dungeon/instance/mandatory_member_migrations.cpp \
	src/old_game/pvp/battlefield/pvp_assault_assaulter_batch027.cpp \
	src/old_game/pvp/battlefield/pvp_assault_forced_move_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_geometry_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_lifecycle_packets.cpp \
	src/old_game/pvp/battlefield/pvp_assault_pool_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_power_war_state.cpp \
	src/old_game/pvp/battlefield/pvp_assault_setup_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_die_user_runtime.cpp \
	src/old_game/pvp/battlefield/packet_dispatcher_die_assault_player.cpp \
	src/old_game/pvp/battlefield/pvp_assault_state_callbacks.cpp \
	src/old_game/pvp/battlefield/pvp_assault_timeout_callbacks.cpp \
	src/old_game/pvp/arena/pvp_room_disconnect_runtime.cpp \
	src/old_game/player/core/cuser_disconnect_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_positioning_batch027.cpp \
	src/old_game/pvp/battlefield/pvp_live_event_data_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_result_runtime.cpp \
	src/old_game/pvp/match/online_preliminary_match_runtime.cpp \
	src/old_game/pvp/match/online_preliminary_match_types.cpp \
	src/old_game/pvp/match/pvp_matching_system_runtime.cpp \
	src/old_game/pvp/match/integrated_matching_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_battle_reset_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_check_winner_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_create_runtime.cpp \
	src/old_game/pvp/match/pvp_room_death_match_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_die_charac_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_reset_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_revenge_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_seat_state_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_start_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_timeout_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_lifecycle_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_vote_lack_user_runtime.cpp \
	src/old_game/pvp/arena/war_room_die_mob_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_war_static_pool_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_state_batch026.cpp \
	src/old_game/pvp/arena/pvp_room_state_machine_batch028.cpp \
	src/old_game/pvp/battlefield/pvp_user_table_runtime.cpp \
	src/old_game/social/party/quick_party_runtime.cpp \
	src/old_game/social/party/quick_party_static_pool_runtime.cpp \
	src/old_game/social/party/quick_party_reward_manager_runtime.cpp \
	src/old_game/social/party/quick_party_dimension_inout_reward_runtime.cpp \
	src/old_game/social/party/quick_party_system_manager_runtime.cpp \
	src/old_game/social/party/quick_party_waiting_timer_runtime.cpp \
	src/old_game/pvp/battlefield/relay_battle_mgr_runtime.cpp \
	src/old_game/dungeon/instance/revenge_dungeon_runtime.cpp \
	src/old_game/dungeon/instance/tod_layer.cpp \
	src/old_game/dungeon/instance/tod_user_state_runtime.cpp \
	src/old_game/dungeon/instance/sending_apc_info_runtime.cpp \
	src/old_game/dungeon/tower/inter_tower_of_despair_dispatchers.cpp \
	src/old_game/dungeon/tower/tower_of_despair_apc_mgr_runtime.cpp \
	src/old_game/dungeon/tower/tower_of_despair_mgr_runtime.cpp \
	src/old_game/pvp/arena/war_room_accessors.cpp \
	src/old_game/pvp/arena/war_room_startup_allocation_runtime.cpp \
	src/old_game/pvp/arena/war_room_prepare_runtime.cpp \
	src/old_game/pvp/arena/war_room_seat_state_runtime.cpp \
	src/old_game/pvp/arena/packet_dispatcher_war_room.cpp \
	src/old_game/pvp/arena/war_room_die_character_runtime.cpp \
	src/old_game/pvp/arena/war_room_lifecycle_runtime.cpp \
	src/old_game/pvp/battlefield/war_field_consist_map_runtime.cpp \
	src/old_game/pvp/battlefield/war_field_item_runtime.cpp \
	src/old_game/pvp/arena/war_room_state_front_runtime.cpp \
	src/old_game/pvp/arena/war_room_state_batch026.cpp \
	src/old_game/pvp/arena/war_room_timer_state_completion_runtime.cpp \
	src/old_game/pvp/arena/war_room_use_coin_runtime.cpp \
	src/old_game/pvp/arena/war_room_host_change_runtime.cpp \
	src/old_game/pvp/arena/timer_war_room_change_host.cpp \
	src/old_game/pvp/arena/timer_war_room_state_callbacks_runtime.cpp \
	src/old_game/pvp/arena/war_room_calc_guild_fp_runtime.cpp \
	src/old_game/pvp/arena/war_room_guild_points_runtime.cpp \
	src/old_game/pvp/arena/war_room_item_drop_runtime.cpp \
	src/old_game/pvp/battlefield/join_power_war_dispatcher.cpp

C_EVENT_SRC := \
	src/old_game/activity/schedule/glass_bottle_event.cpp \
	src/old_game/activity/schedule/shooting_star_dimension_pocket.cpp \
	src/old_game/activity/schedule/sea_bottom_event.cpp \
	src/old_game/activity/schedule/blue_horse_event.cpp \
	src/old_game/activity/schedule/sugar_time_event.cpp \
	src/old_game/activity/schedule/exp_keeping_event.cpp \
	src/old_game/activity/schedule/freely_give_item.cpp \
	src/old_game/activity/schedule/packet_dispatcher_event_all_user_gift.cpp \
	src/old_game/activity/schedule/king_of_pick.cpp \
	src/old_game/activity/schedule/event_shop_attendance.cpp \
	src/old_game/activity/schedule/decide_levelup_event.cpp \
	src/old_game/activity/schedule/levelup_support_3rd.cpp \
	src/old_game/activity/schedule/packet_dispatcher_take_magic_lamp.cpp \
	src/old_game/activity/schedule/packet_dispatcher_facebook_like.cpp \
	src/old_game/activity/schedule/fatigue_acceleration_runtime.cpp \
	src/old_game/activity/schedule/packet_dispatcher_event_account_fatigue_stat.cpp \
	src/old_game/activity/schedule/packet_dispatcher_satisfaction_survey.cpp \
	src/old_game/activity/schedule/packet_dispatcher_skill_damage_rank_event.cpp \
	src/old_game/activity/schedule/advance_altar_db_exchange_runtime.cpp \
	src/old_game/activity/schedule/inter_advance_altar_load.cpp \
	src/old_game/activity/reward/inter_ontime_event_ack_reward.cpp \
	src/old_game/activity/schedule/stackable_package_event_process_manager.cpp \
	src/old_game/activity/schedule/character_expand_runtime_data.cpp \
	src/old_game/activity/schedule/advance_altar_stage_control_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_die_mob_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_timer_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_stage_pool_runtime.cpp \
	src/old_game/activity/reward/advance_altar_achievement_reward_runtime.cpp \
	src/old_game/activity/reward/advance_altar_stage_reward_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_dispatchers.cpp \
	src/old_game/activity/schedule/cbingo_runtime.cpp \
	src/old_game/activity/schedule/cbingo_test_runtime.cpp \
	src/old_game/activity/schedule/bingo_event_core_runtime.cpp \
	src/old_game/activity/schedule/bingo_event_mark_runtime.cpp \
	src/old_game/activity/schedule/bingo_event_user_broadcast_runtime.cpp \
	src/old_game/activity/schedule/bingo_fatigue_packet_dispatchers.cpp \
	src/old_game/activity/schedule/blue_marble_packet_dispatchers.cpp \
	src/old_game/activity/schedule/blue_marble_pool_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_startup_allocation_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_runtime_leaf.cpp \
	src/old_game/activity/schedule/blue_marble_board_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_flow_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_packet_runtime.cpp \
	src/old_game/activity/reward/blue_marble_reward_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_session_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_state_runtime.cpp \
	src/old_game/activity/schedule/blue_marble_user_info_runtime.cpp \
	src/old_game/activity/schedule/collect_box_dispatcher.cpp \
	src/old_game/activity/schedule/inter_collect_box.cpp \
	src/old_game/activity/schedule/kiri_cargo_packet_dispatchers.cpp \
	src/old_game/activity/schedule/rainbow_snake_packet_dispatchers.cpp \
	src/old_game/activity/condition/condition_event_manager_runtime.cpp \
	src/old_game/activity/schedule/event_classify_script_manager_runtime.cpp \
	src/old_game/activity/schedule/event_character_handler.cpp \
	src/old_game/activity/schedule/event_character_parameter_script.cpp \
	src/old_game/activity/schedule/event_delivery_dispatch_runtime.cpp \
	src/old_game/activity/schedule/event_login_item_runtime.cpp \
	src/old_game/activity/schedule/event_messages.cpp \
	src/old_game/activity/schedule/event_packets.cpp \
	src/old_game/activity/reward/packet_dispatcher_event_rewards.cpp \
	src/old_game/activity/schedule/packet_dispatcher_object_bring_up.cpp \
	src/old_game/activity/reward/event_reward_slot_bound.cpp \
	src/old_game/activity/schedule/inter_server_event_start.cpp \
	src/old_game/activity/schedule/timer_event_christmas.cpp \
	src/old_game/activity/schedule/event_value_types.cpp \
	src/old_game/activity/schedule/levelup_support_script_services.cpp \
	src/old_game/activity/schedule/inter_levelup_support_return_dispatcher.cpp \
	src/old_game/activity/reward/inter_collect_items_reward_returns.cpp \
	src/old_game/activity/schedule/hero_mission_lifecycle_runtime.cpp \
	src/old_game/activity/condition/hero_mission_conditions_runtime.cpp \
	src/old_game/activity/schedule/hero_mission_set_value_runtime.cpp \
	src/old_game/gm/core/hero_mission_gm_controls_runtime.cpp \
	src/old_game/activity/schedule/hero_mission_script_import_runtime.cpp \
	src/old_game/activity/schedule/hero_mission_event_save_event_data.cpp \
	src/old_game/activity/schedule/hero_mission_event_load_event_data.cpp \
	src/old_game/activity/reward/cerashop_reward_point_event.cpp \
	src/old_game/activity/schedule/growth_creature_event.cpp \
	src/old_game/activity/reward/reward_key.cpp \
	src/old_game/activity/schedule/packet_dispatcher_sr08_event_unclaimed.cpp \
	src/old_game/activity/schedule/new_account_level_up_event_to_job.cpp \
	src/old_game/activity/schedule/one_a_day_item_shop_event.cpp

C_GROUP_SRC := \
	src/old_game/social/party/cparty_accessors_runtime.cpp \
	src/old_game/social/party/cparty_die_mob_runtime.cpp \
	src/old_game/social/party/cparty_quest_apc_runtime.cpp \
	src/old_game/social/party/cparty_blood_clear_reward_packet_runtime.cpp \
	src/old_game/social/party/cparty_blood_round_history_runtime.cpp \
	src/old_game/social/party/cparty_blood_clear_reward_runtime.cpp \
	src/old_game/social/party/cparty_drop_and_hack_checks.cpp \
	src/old_game/social/party/secu_dungeon_average_check_runtime.cpp \
	src/old_game/social/party/secu_party_path_hack_runtime.cpp \
	src/old_game/social/party/cparty_dungeon_admission.cpp \
	src/old_game/social/party/cparty_dungeon_start_runtime.cpp \
	src/old_game/social/party/cparty_dungeon_clear.cpp \
	src/old_game/social/party/cparty_dungeon_statistics_runtime.cpp \
	src/old_game/social/party/cparty_dungeon_inout_conditions.cpp \
	src/old_game/social/party/cparty_dungeon_inout_condition_command.cpp \
	src/old_game/social/party/cparty_exp_bonus_event_runtime.cpp \
	src/old_game/social/party/cparty_gained_each_exp_data_runtime.cpp \
	src/old_game/social/party/cparty_sum_gained_each_exp_runtime.cpp \
	src/old_game/social/party/cparty_free_revival_condition.cpp \
	src/old_game/social/party/cparty_use_coin_runtime.cpp \
	src/old_game/social/party/cparty_drop_item_runtime.cpp \
	src/old_game/social/party/cparty_after_dungeon_start_runtime.cpp \
	src/old_game/social/party/cparty_finish_loading_runtime.cpp \
	src/old_game/social/party/cparty_giveup_game.cpp \
	src/old_game/social/party/cparty_giveup_state_leaves.cpp \
	src/old_game/social/party/cparty_death_runtime.cpp \
	src/old_game/social/party/cparty_bluemarble_invite_runtime.cpp \
	src/old_game/social/party/cparty_pvp_invite_runtime.cpp \
	src/old_game/social/party/cparty_member_state_batch026.cpp \
	src/old_game/social/party/cparty_boss_tower_reward_exp_runtime.cpp \
	src/old_game/social/party/cparty_assault_timeout.cpp \
	src/old_game/social/party/cparty_result_flag_member_runtime.cpp \
	src/old_game/social/party/cparty_runtime_field_batch026.cpp \
	src/old_game/social/party/cparty_quick_party_basic_reward_runtime.cpp \
	src/old_game/social/party/cparty_quick_party_timer_runtime.cpp \
	src/old_game/social/party/cparty_card_selection_runtime.cpp \
	src/old_game/social/party/cparty_dungeon_clear_rewards.cpp \
	src/old_game/social/party/cparty_state_machine_batch028.cpp \
	src/old_game/social/party/cparty_vote_lack_user_runtime.cpp \
	src/old_game/social/party/cparty_ancient_ticket_runtime.cpp \
	src/old_game/social/party/cparty_static_pool_runtime.cpp \
	src/old_game/social/party/cparty_typed_lifecycle_preabi.cpp \
	src/old_game/social/party/cparty_walkout_runtime.cpp \
	src/old_game/social/party/packet_dispatcher_group_business_043.cpp \
	src/old_game/social/guild/packet_dispatcher_guild_business.cpp \
	src/old_game/social/guild/packet_dispatcher_guild_member_naming.cpp \
	src/old_game/social/guild/guild_double_check_dispatchers.cpp \
	src/old_game/social/guild/guild_create_window_dispatcher.cpp \
	src/old_game/social/guild/packet_dispatcher_guild_join_list.cpp \
	src/old_game/social/guild/packet_dispatcher_guild_join_request_db.cpp \
	src/old_game/social/guild/packet_dispatcher_refresh_guild_info.cpp \
	src/old_game/social/party/world_quick_party_pool.cpp \
	src/old_game/social/friend/friend_dispatcher.cpp \
	src/old_game/social/party/inter_hyperlink_chat_dispatchers.cpp \
	src/old_game/social/guild/inter_guild_exp_book_delete_return.cpp \
	src/old_game/social/guild/inter_guild_agit_receivers.cpp \
	src/old_game/progression/quest/packet_dispatcher_request_blacklist.cpp

C_INVENTORY_SRC := \
	src/old_game/economy/equipment/upgrade_room.cpp \
	src/old_game/economy/equipment/packet_dispatcher_upgrade_room.cpp \
	src/old_game/economy/inventory/arad_item_info.cpp \
	src/old_game/economy/inventory/avatar_item_access_runtime.cpp \
	src/old_game/economy/inventory/caccount_cargo_is_alter.cpp \
	src/old_game/economy/inventory/caccount_cargo_item_list.cpp \
	src/old_game/economy/inventory/ccargo_item_list.cpp \
	src/old_game/economy/inventory/ccargo_runtime.cpp \
	src/old_game/economy/equipment/cequip_item_closure_batch027.cpp \
	src/old_game/economy/equipment/cequip_item_upgrade_level.cpp \
	src/old_game/economy/equipment/clone_title_runtime.cpp \
	src/old_game/economy/inventory/cinventory_check_n_fix.cpp \
	src/old_game/economy/inventory/cinventory_checksum_runtime.cpp \
	src/old_game/economy/inventory/cinventory_check_to_fix.cpp \
	src/old_game/economy/inventory/cinventory_bulk_data_runtime.cpp \
	src/old_game/economy/inventory/cinventory_creature_accessors.cpp \
	src/old_game/economy/inventory/cinventory_closure_batch027.cpp \
	src/old_game/economy/inventory/cinventory_delete_item.cpp \
	src/old_game/economy/inventory/cinventory_front_runtime.cpp \
	src/old_game/economy/inventory/cinventory_move_item_runtime.cpp \
	src/old_game/economy/inventory/inventory_remain_capacity.cpp \
	src/old_game/economy/inventory/cinventory_item_list.cpp \
	src/old_game/economy/inventory/cinventory_make_item_packet.cpp \
	src/old_game/economy/inventory/cinventory_remove_fp_item.cpp \
	src/old_game/economy/inventory/cinventory_remove_kc_item.cpp \
	src/old_game/economy/inventory/cinventory_save_accessors.cpp \
	src/old_game/economy/inventory/cinventory_vilmark_dungeon_item.cpp \
	src/old_game/economy/inventory/cstackable_item_closure_batch027.cpp \
	src/old_game/economy/inventory/cstackable_item_verify_usable_place.cpp \
	src/old_game/economy/inventory/cstackable_item_verify_usable_server_type.cpp \
	src/old_game/economy/equipment/equip_item_accessors.cpp \
	src/old_game/economy/equipment/equip_item_endurance_runtime.cpp \
	src/old_game/economy/equipment/expand_equipslot_runtime.cpp \
	src/old_game/economy/inventory/gen_unique_no.cpp \
	src/old_game/economy/inventory/inventory_event_items.cpp \
	src/old_game/economy/inventory/inventory_compound_transaction.cpp \
	src/old_game/economy/equipment/inventory_equip_list.cpp \
	src/old_game/economy/equipment/packet_dispatcher_set_clone_title.cpp \
	src/old_game/economy/inventory/inventory_cargo_slot_accessors.cpp \
	src/old_game/economy/inventory/integrated_pvp_item_attr.cpp \
	src/old_game/economy/inventory/item_dictionary_runtime.cpp \
	src/old_game/economy/inventory/item_lock_dispatchers.cpp \
	src/old_game/economy/inventory/item_lock_lookup_runtime.cpp \
	src/old_game/economy/inventory/item_lock_runtime.cpp \
	src/old_game/economy/inventory/item_lock_timer.cpp \
	src/old_game/economy/inventory/item_space_runtime.cpp \
	src/old_game/economy/inventory/item_timeout_runtime.cpp \
	src/old_game/economy/inventory/inventory_memory.cpp \
	src/old_game/economy/inventory/cinventory_empty_slot_runtime.cpp \
	src/old_game/economy/inventory/citem_category_runtime.cpp \
	src/old_game/economy/inventory/citem_cerashop_package_runtime.cpp \
	src/old_game/economy/inventory/inventory_slot_planning_runtime.cpp \
	src/old_game/economy/inventory/inventory_value_state_batch026_runtime.cpp \
	src/old_game/economy/inventory/inventory_load_verification.cpp \
	src/old_game/economy/inventory/account_cargo_delete_transaction.cpp \
	src/old_game/economy/inventory/inven_item_runtime.cpp \
	src/old_game/economy/inventory/inven_item_mail_record.cpp \
	src/old_game/economy/inventory/packet_dispatcher_item_actions.cpp \
	src/old_game/economy/inventory/packet_dispatcher_random_option_modify.cpp \
	src/old_game/economy/inventory/packet_dispatcher_regeneration_random_option.cpp \
	src/old_game/economy/inventory/packet_dispatcher_sort_item.cpp \
	src/old_game/economy/equipment/packet_dispatcher_upgrade_item.cpp \
	src/old_game/economy/inventory/packet_dispatcher_unseal_random_option.cpp \
	src/old_game/economy/inventory/packet_dispatcher_use_chestnut_stone.cpp \
	src/old_game/economy/inventory/packet_dispatcher_use_limit_cube.cpp \
	src/old_game/economy/inventory/random_item_table.cpp \
	src/old_game/economy/inventory/recipe_info.cpp \
	src/old_game/economy/inventory/redeem_item_runtime.cpp \
	src/old_game/economy/inventory/slot_bound_checker.cpp \
	src/old_game/economy/equipment/store_repair_equip.cpp \
	src/old_game/economy/inventory/use_jewel_verify_gold_dispatchers.cpp

# CTradeSpace lifecycle/readiness and the canonical 0x48-byte TradeSlot ABI
# are intentionally built with the C-owned mail/trade/cash provider set.
C_MAIL_TRADE_CASH_SRC := \
	src/old_game/gm/core/game_master_billing_recharge.cpp \
	src/old_game/gm/core/game_master_mail_trade_commands.cpp \
	src/old_game/economy/mall/auction_server_packet_bridge.cpp \
	src/old_game/economy/mall/auction_runtime.cpp \
	src/old_game/economy/mail/arad_mail_runtime.cpp \
	src/old_game/economy/mall/cash_billing_runtime.cpp \
	src/old_game/economy/mall/cash_service_runtime.cpp \
	src/old_game/economy/mall/cash_transaction_runtime.cpp \
	src/old_game/economy/mall/crane_minigame_runtime.cpp \
	src/old_game/economy/inventory/commerce_inter_runtime.cpp \
	src/old_game/economy/inventory/commerce_packet_dispatch_runtime.cpp \
	src/old_game/gm/core/commerce_timer_gm_runtime.cpp \
	src/old_game/economy/mail/timer_check_mail_timeout.cpp \
	src/old_game/economy/mall/delivery_runtime.cpp \
	src/old_game/economy/mail/mailbox_avatar_mail_runtime.cpp \
	src/old_game/economy/mail/mailbox_helper_runtime.cpp \
	src/old_game/economy/mail/inter_mailbox_check_receiver.cpp \
	src/old_game/economy/mail/inter_mailbox_req_list.cpp \
	src/old_game/economy/mail/inter_mailbox_req_send.cpp \
	src/old_game/economy/mail/inter_monitor_notify_new_mail.cpp \
	src/old_game/economy/mail/inter_multimailbox_send_return.cpp \
	src/old_game/economy/mail/mailbox_package_runtime.cpp \
	src/old_game/economy/mail/mailbox_send_mail_runtime.cpp \
	src/old_game/economy/mail/mailbox_system_multi_mail_runtime.cpp \
	src/old_game/economy/mail/packet_dispatcher_change_letter_stat.cpp \
	src/old_game/economy/mail/packet_dispatcher_mailbox_send.cpp \
	src/old_game/economy/mail/mailbox_load_state_runtime.cpp \
	src/old_game/economy/mail/mailbox_outer_pool_runtime.cpp \
	src/old_game/economy/mail/mailbox_param_runtime.cpp \
	src/old_game/economy/mail/mailbox_pool_registry_runtime.cpp \
	src/old_game/economy/mail/mailbox_record_runtime.cpp \
	src/old_game/economy/mail/mailbox_runtime_init.cpp \
	src/old_game/economy/mail/mail_trade_params.cpp \
	src/old_game/economy/mail/mail_trade_packets.cpp \
	src/old_game/economy/trade/private_store_item_runtime.cpp \
	src/old_game/economy/trade/private_store_layout_runtime.cpp \
	src/old_game/economy/trade/private_store_packet_buf_runtime.cpp \
	src/old_game/economy/trade/private_store_verify_runtime.cpp \
	src/old_game/economy/trade/private_store_buy_runtime.cpp \
	src/old_game/economy/trade/private_store_static_pool_runtime.cpp \
	src/old_game/economy/trade/private_store_trade_validation_runtime.cpp \
	src/old_game/economy/trade/private_store_transaction_helpers_runtime.cpp \
	src/old_game/economy/trade/private_store_item_transition_runtime.cpp \
	src/old_game/economy/trade/private_store_persistence_runtime.cpp \
	src/old_game/economy/trade/private_store_recall_remove_runtime.cpp \
	src/old_game/economy/trade/private_store_lifecycle_runtime.cpp \
	src/old_game/economy/trade/private_store_virtual_runtime.cpp \
	src/old_game/economy/trade/private_store_world_helpers_runtime.cpp \
	src/old_game/economy/trade/private_store_mgr_runtime.cpp \
	src/old_game/economy/trade/private_store_world_entry.cpp \
	src/old_game/economy/trade/private_store_inter_load_runtime.cpp \
	src/old_game/economy/mall/purchase_cash_item_bonus_runtime.cpp \
	src/old_game/economy/mail/mail_trade_value_types.cpp \
	src/old_game/economy/mail/arad_mail_value_types.cpp \
	src/old_game/economy/trade/trade_slot_runtime.cpp \
	src/old_game/economy/trade/trade_space_cancel_runtime.cpp \
	src/old_game/economy/trade/trade_space_lifecycle_runtime.cpp \
	src/old_game/economy/trade/trade_space_set_traders_runtime.cpp \
	src/old_game/economy/trade/trade_space_pool_runtime.cpp \
	src/old_game/economy/trade/trade_space_state_runtime.cpp \
	src/old_game/economy/trade/trade_state_dispatcher.cpp \
	src/old_game/economy/inventory/inter_owner_is_vip.cpp \
	src/old_game/economy/inventory/taiwan_billing_runtime.cpp

C_NPC_SRC := \
	src/old_game/economy/mall/cerashop_add_restrict_inter_dispatcher.cpp \
	src/old_game/economy/shop/inter_npc_limit_buy_item_info_return.cpp \
	src/old_game/economy/shop/inter_npc_limit_buy_info_returns.cpp \
	src/old_game/economy/mall/cerashop_add_restrict_runtime.cpp \
	src/old_game/economy/trade/cerashop_vending_closure_batch027.cpp \
	src/old_game/economy/trade/cerashop_vending_preconditions_runtime.cpp \
	src/old_game/economy/mall/cerashop_transactions_runtime.cpp \
	src/old_game/economy/trade/item_vending_machine_ipg_runtime.cpp \
	src/old_game/economy/inventory/arad_mileage_process_runtime.cpp \
	src/old_game/economy/mall/cerashop_purchase_bonus_runtime.cpp \
	src/old_game/economy/mall/inter_cerashop_purchase_count_return.cpp \
	src/old_game/economy/trade/item_vending_machine_auction_runtime.cpp \
	src/old_game/economy/shop/npc_dialog_value_types.cpp \
	src/old_game/economy/mall/cerashop_item_value_runtime.cpp \
	src/old_game/world/region/npc_relation_event_manager.cpp \
	src/old_game/economy/shop/buy_item_param.cpp \
	src/old_game/economy/shop/packet_dispatcher_buy_item.cpp \
	src/old_game/economy/mall/npc_shop_params.cpp \
	src/old_game/economy/mall/quest_shop_runtime.cpp \
	src/old_game/economy/mall/secret_shop_data_runtime.cpp \
	src/old_game/economy/mall/secret_shop_info_runtime.cpp \
	src/old_game/economy/trade/store_sell_price_closure_batch027.cpp \
	src/old_game/world/region/timer_npc_mood_change_runtime.cpp \
	src/old_game/economy/mall/npc_shop_timer_runtime.cpp

C_QUEST_SKILL_SRC := \
	src/old_game/progression/career/calchemist_item_upgrade_success_rate.cpp \
	src/old_game/progression/career/calchemist_assault_prevent.cpp \
	src/old_game/progression/career/calchemist_stackable_runtime.cpp \
	src/old_game/progression/career/character_expert_job.cpp \
	src/old_game/progression/quest/character_mission_list.cpp \
	src/old_game/progression/skill/cskill_runtime.cpp \
	src/old_game/progression/skill/combo_skill.cpp \
	src/old_game/progression/career/expert_job_statistic.cpp \
	src/old_game/progression/career/expert_job_factory_runtime.cpp \
	src/old_game/progression/career/expert_job_disjointer_runtime.cpp \
	src/old_game/progression/career/expert_job_enchanter_runtime.cpp \
	src/old_game/progression/career/expert_job_runtime.cpp \
	src/old_game/progression/quest/cuser_quest_point.cpp \
	src/old_game/progression/skill/guild_skill_runtime.cpp \
	src/old_game/progression/skill/item_making_skill.cpp \
	src/old_game/progression/quest/quest_skill_value_types.cpp \
	src/old_game/progression/quest/quest_skill_standalone_value_types.cpp \
	src/old_game/progression/quest/quest_skill_expert_job.cpp \
	src/old_game/progression/quest/quest_skill_messages.cpp \
	src/old_game/progression/quest/quest_skill_packets.cpp \
	src/old_game/progression/quest/quest_skill_params.cpp \
	src/old_game/progression/quest/quest_runtime.cpp \
	src/old_game/progression/quest/quest_state_runtime.cpp \
	src/old_game/progression/skill/skill_checksum_runtime.cpp \
	src/old_game/progression/quest/userquest_accept_runtime.cpp \
	src/old_game/progression/quest/userquest_leaves_runtime.cpp \
	src/old_game/progression/quest/userquest_trigger_conditions_runtime.cpp \
	src/old_game/progression/skill/skill_slot.cpp \
	src/old_game/progression/skill/skill_slot_buy_runtime.cpp \
	src/old_game/progression/career/packet_dispatcher_expert_job.cpp \
	src/old_game/progression/career/packet_dispatcher_enchant_by_bead.cpp \
	src/old_game/progression/quest/packet_dispatcher_sr08_quest_skill_unclaimed.cpp \
	src/old_game/progression/skill/premium_lethe_skill_runtime.cpp \
	src/old_game/progression/career/inter_update_one_day_lethe.cpp \
	src/old_game/progression/skill/skill_changer_condition_runtime.cpp

C_USERINFO_SRC := \
	src/old_game/player/core/achievement_runtime.cpp \
	src/old_game/player/core/achievement_title_section.cpp \
	src/old_game/player/core/cerashop_add_restrict_user_info.cpp \
	src/old_game/player/core/charac_expand_cdata.cpp \
	src/old_game/player/core/charac_visible_flag.cpp \
	src/old_game/player/core/charac_expand_cdata_mgr_lifecycle.cpp \
	src/old_game/player/core/charac_expand_cdata_mgr_update.cpp \
	src/old_game/player/setting/character_option.cpp \
	src/old_game/player/core/character_restrict_policy_runtime.cpp \
	src/old_game/player/core/character_tag_data.cpp \
	src/old_game/player/core/cbooster_gage_runtime.cpp \
	src/old_game/player/core/cbooster_gage_reward_runtime.cpp \
	src/old_game/player/core/ctitle_book_runtime.cpp \
	src/old_game/player/core/creature_guild_params.cpp \
	src/old_game/player/core/userinfo_messages.cpp \
	src/old_game/player/core/cuser_avatar_socket_action.cpp \
	src/old_game/player/core/cuser_delete_invalid_item_reward.cpp \
	src/old_game/player/core/cuser_delete_specific_item_transaction.cpp \
	src/old_game/player/core/cuser_delete_guild_exp_book.cpp \
	src/old_game/player/core/cuser_basic_info_runtime.cpp \
	src/old_game/player/core/cuser_bingo_runtime.cpp \
	src/old_game/player/core/cuser_charac_link_runtime.cpp \
	src/old_game/player/core/inter_charac_link.cpp \
	src/old_game/player/core/packet_dispatcher_link_character.cpp \
	src/old_game/player/core/cuser_charac_reset_runtime.cpp \
	src/old_game/player/core/cuser_charac_reset_support_runtime.cpp \
	src/old_game/player/core/cuser_reset_full_runtime.cpp \
	src/old_game/player/core/cuser_quest_action_runtime.cpp \
	src/old_game/player/core/cuser_quest_clear_event_runtime.cpp \
	src/old_game/player/core/cuser_quest_stat_runtime.cpp \
	src/old_game/player/core/cuser_character_info_runtime.cpp \
	src/old_game/player/core/cuser_fatigue_runtime.cpp \
	src/old_game/player/core/cuser_human_certify_runtime.cpp \
	src/old_game/player/core/packet_dispatcher_human_certify.cpp \
	src/old_game/player/core/packet_dispatcher_exit.cpp \
	src/old_game/player/core/cuser_die_cur_charac_runtime.cpp \
	src/old_game/player/core/cuser_typed_lifecycle_preabi.cpp \
	src/old_game/player/core/cuser_accept_runtime.cpp \
	src/old_game/gm/player/game_master_character.cpp \
	src/old_game/player/core/cuser_global_info_handle.cpp \
	src/old_game/player/session/cuser_character_select_state.cpp \
	src/old_game/player/core/cuser_achievement_history.cpp \
	src/old_game/player/core/cuser_disable_save_inform_notice_flag.cpp \
	src/old_game/player/core/cuser_get_account_cargo.cpp \
	src/old_game/player/core/cuser_get_changed_gift_fatigue_quantity.cpp \
	src/old_game/player/core/cuser_get_etc.cpp \
	src/old_game/player/core/cuser_get_inform_notice_flag.cpp \
	src/old_game/player/core/cuser_get_skill_command_buf.cpp \
	src/old_game/player/core/cuser_get_used_fatigue_quantity.cpp \
	src/old_game/player/core/cuser_giveup_penalty.cpp \
	src/old_game/player/core/guild_db_info_runtime.cpp \
	src/old_game/player/core/cuser_guild_agit_db_info.cpp \
	src/old_game/player/core/local_china_user.cpp \
	src/old_game/player/core/cuser_history_log_cargo.cpp \
	src/old_game/player/session/cuser_history_log_login_runtime.cpp \
	src/old_game/player/core/cuser_history_log_private_store.cpp \
	src/old_game/player/core/cuser_history_log_item_making_skill.cpp \
	src/old_game/player/core/cuser_history_log_skill.cpp \
	src/old_game/player/core/cuser_history_log_pay_coin.cpp \
	src/old_game/player/core/cuser_history_log_pvp_mission.cpp \
	src/old_game/player/setting/cuser_history_log_random_option.cpp \
	src/old_game/player/core/cuser_add_item_runtime.cpp \
	src/old_game/player/core/cuser_pending_creature_exp_runtime.cpp \
	src/old_game/player/core/cuser_monster_kill_check_runtime.cpp \
	src/old_game/player/core/cuser_history_log_kill_monster_runtime.cpp \
	src/old_game/player/core/cuser_exp_bonus_leaf_runtime.cpp \
	src/old_game/player/core/cuser_exp_sp_leaf_runtime.cpp \
	src/old_game/player/core/cuser_update_guild_runtime.cpp \
	src/old_game/player/core/cuser_avenger_title_runtime.cpp \
	src/old_game/player/core/cuser_mercenary_level_runtime.cpp \
	src/old_game/player/core/cuser_quest_drop_leaf_runtime.cpp \
	src/old_game/player/core/cuser_check_quest_monster_runtime.cpp \
	src/old_game/player/core/cuser_send_noti_packet_runtime.cpp \
	src/old_game/player/core/cuser_history_context_foundation.cpp \
	src/old_game/player/core/cuser_separate_upgrade_runtime.cpp \
	src/old_game/player/core/cuser_item_upgrade_history.cpp \
	src/old_game/player/core/cuser_item_upgrade_guild_message.cpp \
	src/old_game/player/core/cuser_history_log_secu_service_runtime.cpp \
	src/old_game/player/core/cuser_history_log_antibot.cpp \
	src/old_game/player/core/cuser_history_log_blood_round.cpp \
	src/old_game/player/core/cuser_history_log_hack_detected.cpp \
	src/old_game/player/core/cuser_history_log_ip_counter.cpp \
	src/old_game/player/core/cuser_set_hack_user_runtime.cpp \
	src/old_game/player/core/cuser_recover_stamina_types.cpp \
	src/old_game/player/core/cuser_item_policy_foundation.cpp \
	src/old_game/player/core/cuser_item_lock_runtime.cpp \
	src/old_game/player/core/cuser_log_history_runtime.cpp \
	src/old_game/player/core/cuser_identity_command_runtime.cpp \
	src/old_game/player/core/cuser_is_break_away_dungeon_clear.cpp \
	src/old_game/player/core/cuser_is_exist_account_cargo.cpp \
	src/old_game/player/core/cuser_is_save_inform_notice_flag.cpp \
	src/old_game/player/core/cuser_is_used_break_away_luck_point.cpp \
	src/old_game/player/session/cuser_logout_cached_character.cpp \
	src/old_game/player/session/nexon_packet_login_runtime.cpp \
	src/old_game/player/session/cuser_pcroom_logout_runtime.cpp \
	src/old_game/player/session/cuser_pcroom_login_runtime.cpp \
	src/old_game/player/session/cuser_logout_persistent_state_runtime.cpp \
	src/old_game/player/core/cuser_log_out_runtime.cpp \
	src/old_game/player/session/cuser_update_logout_runtime.cpp \
	src/old_game/player/core/cuser_check_money.cpp \
	src/old_game/player/core/cuser_money_log_runtime.cpp \
	src/old_game/player/session/online_preliminary_match_list_runtime.cpp \
	src/old_game/player/session/online_preliminary_member_runtime.cpp \
	src/old_game/player/session/inter_online_preliminary_dispatchers.cpp \
	src/old_game/player/session/inter_character_return_dispatchers.cpp \
	src/old_game/player/core/cuser_tag_charac_inventory_runtime.cpp \
	src/old_game/player/session/online_preliminary_runtime.cpp \
	src/old_game/player/session/online_preliminary_team_runtime.cpp \
	src/old_game/player/session/online_preliminary_team_pool_runtime.cpp \
	src/old_game/player/session/timer_update_online_preliminary_team.cpp \
	src/old_game/player/session/cuser_account_session_state_runtime.cpp \
	src/old_game/player/core/cuser_account_progress_flags_runtime.cpp \
	src/old_game/player/core/cuser_account_security_guild_leaf_runtime.cpp \
	src/old_game/player/core/cuser_business_closure_batch026_next100.cpp \
	src/old_game/player/core/cuser_money_mileage.cpp \
	src/old_game/player/core/cuser_ontime_event_reward.cpp \
	src/old_game/player/core/cuser_inventory_item_runtime.cpp \
	src/old_game/player/core/cuser_secu_reward_item.cpp \
	src/old_game/player/core/cuser_tutorial_reward_runtime.cpp \
	src/old_game/player/core/cuser_ontime_reward.cpp \
	src/old_game/player/core/cuser_packet_action_runtime.cpp \
	src/old_game/player/core/cuser_progress_security_leaf_batch026.cpp \
	src/old_game/player/core/cuser_rental_inventory.cpp \
	src/old_game/player/core/cuser_rental_export_runtime.cpp \
	src/old_game/player/core/cuser_money_log_send_runtime.cpp \
	src/old_game/player/core/cuser_pvp_index_runtime.cpp \
	src/old_game/player/core/cuser_pvp_assault_team_runtime.cpp \
	src/old_game/player/session/cuser_return_cleanup_accessors.cpp \
	src/old_game/player/session/cuser_return_user_runtime.cpp \
	src/old_game/player/core/cuser_runtime_state_accessors.cpp \
	src/old_game/player/core/cuser_static_pool_runtime.cpp \
	src/old_game/player/core/cuser_reset_time_gate_quest.cpp \
	src/old_game/player/core/cuser_account_slot_runtime.cpp \
	src/old_game/player/core/cuser_inform_notice_runtime.cpp \
	src/old_game/player/core/user_mercenary_info_mgr_runtime.cpp \
	src/old_game/player/core/cuser_blue_marble_runtime.cpp \
	src/old_game/player/core/change_emotion_packet_dispatcher.cpp \
	src/old_game/player/core/cuser_save_charac_data.cpp \
	src/old_game/player/core/cuser_save_condition_event_info.cpp \
	src/old_game/player/core/cuser_save_inventory.cpp \
	src/old_game/player/core/cuser_save_inventory_base.cpp \
	src/old_game/player/core/cuser_save_inventory_only.cpp \
	src/old_game/player/core/cuser_save_mercenary_data.cpp \
	src/old_game/player/core/cuser_save_proper_dungeon_clear_count.cpp \
	src/old_game/player/core/cuser_dungeon_leaf_state_runtime.cpp \
	src/old_game/player/core/cuser_simple_state_runtime.cpp \
	src/old_game/player/core/cuser_chat_runtime.cpp \
	src/old_game/player/core/cuser_hack_count_runtime.cpp \
	src/old_game/player/core/cuser_update_bingo_data.cpp \
	src/old_game/player/core/cuser_update_blood_record.cpp \
	src/old_game/player/core/cuser_update_blue_marble_enter_count.cpp \
	src/old_game/player/core/cuser_update_combo_skill.cpp \
	src/old_game/player/core/cuser_update_data.cpp \
	src/old_game/player/core/cuser_itemspace.cpp \
	src/old_game/player/core/cuser_update_data_link_breakaway.cpp \
	src/old_game/player/core/cuser_update_data_state_accessors.cpp \
	src/old_game/player/core/cuser_update_dimension_inout.cpp \
	src/old_game/player/core/cuser_dimension_inout_update_runtime.cpp \
	src/old_game/player/core/cuser_update_npc_relationship.cpp \
	src/old_game/player/core/cuser_update_pc_room_play_time.cpp \
	src/old_game/player/setting/cuser_update_save_option.cpp \
	src/old_game/player/core/cuser_userinfo_dispatcher_runtime.cpp \
	src/old_game/player/core/cuser_work_periodic.cpp \
	src/old_game/player/core/cuser_world_identity.cpp \
	src/old_game/player/core/wongwork_chandle_premium_set_user_premium.cpp \
	src/old_game/player/core/wongwork_cuser_premium_get_premium_info_list.cpp \
	src/old_game/player/core/wongwork_cuser_premium_is_affected_unlimit_fatigue.cpp

# InterDispatcher dialogue-3 C-owned receiver and premium provider closure.
C_USERINFO_SRC += src/old_game/player/core/cuser_charac_link_bonus_exp.cpp src/old_game/player/core/inter_load_dimension_inout.cpp src/old_game/player/core/cuser_save_backup_skill_info.cpp src/old_game/player/core/wongwork_chandle_premium_one_day_lethe.cpp src/old_game/player/core/wongwork_cuser_premium_advantage_items.cpp src/old_game/player/core/inter_power_war_user_receivers.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_cerashop_ipg_history_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/timer_premium_service_runtime.cpp

# Batch-026 CUser/Premium exact-100: 96+4 canonical providers; focused remote
# G++4.9/raw-NM PASS, checkpoint-200/post-663 overlap 0.
C_USERINFO_SRC += src/old_game/player/core/cuser_business_closure_batch026_exact96.cpp src/old_game/player/core/cuser_guild_level_up.cpp src/old_game/player/core/wongwork_cuser_premium_mutation_batch026.cpp src/old_game/player/core/wongwork_cuser_premium_gold_bonus.cpp src/old_game/player/core/cuser_history_log_level_up_runtime.cpp src/old_game/player/core/cuser_cache_character_memory.cpp src/old_game/player/core/cuser_get_charac_info.cpp src/old_game/player/core/cuser_get_charac_info_detail.cpp src/old_game/player/core/cuser_inventory_snapshot.cpp src/old_game/player/core/cuser_process_npc_gift_level_up_runtime.cpp src/old_game/player/core/character_inform_notice.cpp src/old_game/player/core/cuser_validate_striker_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_ticket_runtime.cpp src/old_game/player/core/cuser_inventory_load_runtime.cpp

# IDA 0x86787FC: CUser growth-type transition, skill-tree and proxy transaction.
C_USERINFO_SRC += src/old_game/player/session/wongwork_cuser_premium_return_item_rate.cpp src/old_game/player/core/wongwork_cuser_premium_over_equip_level.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_set_grow_type_runtime.cpp

# IDA 0x8657ADA/0x86657FC/0x868B20C: status-use, fatigue reset and EXP result flow.
C_USERINFO_SRC += src/old_game/player/core/cuser_increase_status_runtime.cpp src/old_game/player/core/cuser_recover_fatigue_runtime.cpp src/old_game/player/core/cuser_service_of_shusia.cpp src/old_game/player/core/cuser_reward_exp_status_runtime.cpp

# IDA 0x8668D1A: quest-item-driven character growth transition.
C_USERINFO_SRC += src/old_game/player/core/cuser_use_quest_item_runtime.cpp

# IDA 0x8658B2A..0x865EC0E: CUser continuous cooldown/effect-item persistence.
C_USERINFO_SRC += src/old_game/player/core/cuser_continuous_item_runtime.cpp

# IDA 0x865DEFC/0x865E0AE and 0x8196ED7: stackable-use transaction,
# level-70 buff broadcast, and secret-shop additional-item delivery.
C_USERINFO_SRC += src/old_game/player/core/cuser_use_stackable_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/stackable_action_runtime.cpp src/old_game/player/core/cuser_lottery_count_item_runtime.cpp src/old_game/player/core/packet_dispatcher_stackable_action_use.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_multibox_lottery_transaction.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_lottery_transaction.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_mailbox_package_helpers.cpp

# IDA 0x8676826/0x86945A2: random-box statistic packet and CUser sender.
C_USERINFO_SRC += src/old_game/player/core/cuser_random_box_statistic_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_history_log_booster_runtime.cpp src/old_game/player/core/cuser_event_jar_output_statistic.cpp

# IDA 0x8670E38..0x869304E: recipe compound transaction, avatar recipe
# transaction, equipment option inheritance, and composition notifications.
C_USERINFO_SRC += src/old_game/player/core/cuser_compound_item_runtime.cpp

# IDA 0x864DCF2/0x8654768/0x867794C/0x8677DD8/0x8688DC4: CUser ETC cache.
C_USERINFO_SRC += src/old_game/player/core/cuser_etc_info_runtime.cpp

# IDA 0x86645A4 / 0x866E3E2..0x866E914: quest reward leaf ABI.
C_USERINFO_SRC += src/old_game/player/core/cuser_quest_reward_runtime.cpp

# IDA 0x8689D74: level-up-item eligibility leaf used by TutorialLevelUp and
# CUser::increase_status; it preserves the original PVF exp calculation and
# both cMyTrace diagnostic paths.
C_USERINFO_SRC += src/old_game/player/core/cuser_level_up_item_state_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_arad_event_item_log_runtime.cpp

# IDA 0x8334844..0x8335A8A and 0x8681820/0x86818FC: condition-event
# service owner plus its two user notification packet leaves.  CMD263's
# dispatcher is still withheld until its derived-provider closure is restored.
C_USERINFO_SRC += src/old_game/player/core/cuser_condition_event_packets.cpp

# SR08 CMD196 and CMD214 through CMD217: C-owned state-0 derived dispatchers.
# They are intentionally absent from PacketDispatcher root registration.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_disguise_request.cpp src/old_game/player/core/packet_dispatcher_disguise_cancel.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_request_pc_room_player_list.cpp src/old_game/player/core/packet_dispatcher_request_pc_room_player_count.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_goblin_pad_status.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_goblin_pad_request_crypt_key.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_user_history_log.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_verify_creature_quest.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_mercenary_return.cpp src/old_game/player/core/packet_dispatcher_mercenary_info.cpp src/old_game/player/core/cuser_mercenary_info_packet.cpp
C_USERINFO_SRC += src/old_game/player/core/character_messages.cpp src/old_game/player/session/character_select_params.cpp
C_USERINFO_SRC += src/old_game/player/core/timer_revival_charac_runtime.cpp
# SR08 CMD4/CMD5/CMD6: recovered state-2 character-selection dispatcher
# derived objects and their unique IPacketDispatcher template providers.
C_USERINFO_SRC += src/old_game/player/core/character_dispatchers.cpp
C_USERINFO_SRC += src/old_game/player/core/character_runtime_dispatchers.cpp src/old_game/player/core/cuser_equipslot_flag.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_upgrade_guild_cargo.cpp
C_USERINFO_SRC += src/old_game/player/setting/character_option_dispatchers.cpp
C_USERINFO_SRC += src/old_game/player/core/inform_notice_2nd_dispatcher.cpp src/old_game/player/core/inform_notice_dispatcher.cpp src/old_game/player/core/cuser_handle_inform_notice_flag.cpp src/old_game/player/core/achievement_trigger_dispatcher.cpp

# SR08 CPad security ABI: the one canonical provider for the eight released
# CPad accessors used by B's security handlers.
C_USERINFO_SRC += src/old_game/player/core/cpad_security_runtime.cpp

# SR08 CMD200/CMD201/CMD369/CMD473: typed C/userinfo game-option save chain.
# The handlers are deliberately not inserted into the incomplete root table.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_sr08_userinfo_unclaimed.cpp src/old_game/player/setting/cgame_option_runtime.cpp src/old_game/player/setting/cuser_game_option_runtime.cpp

# CMD267/CMD268 Village Monster owns its declarations and complete provider
# surface in packet_dispatcher_village_monster.cpp, not the aggregate dispatcher.
C_WORLD_SRC := \
	src/old_game/world/region/area_runtime.cpp \
	src/old_game/economy/inventory/game_world_commercial_zone_runtime.cpp \
	src/old_game/world/region/game_world_lifecycle.cpp \
	src/old_game/world/region/game_world_leave_runtime.cpp \
	src/old_game/world/broadcast/game_world_chat_runtime.cpp \
	src/old_game/world/region/game_world_buddy_runtime.cpp \
	src/old_game/world/region/game_world_runtime.cpp \
	src/old_game/world/region/inter_world_return_dispatchers.cpp \
	src/old_game/world/region/game_world_state_machine_batch028.cpp \
	src/old_game/world/region/game_world_transition_runtime.cpp \
	src/old_game/world/region/game_world_weekend_event_runtime.cpp \
	src/old_game/world/region/game_world_one_a_day_item_shop.cpp \
	src/old_game/world/region/gameworld_party_info.cpp \
	src/old_game/world/region/invalid_area_move.cpp \
	src/old_game/world/region/collision_detection_runtime.cpp \
	src/old_game/world/region/map_area_runtime.cpp \
	src/old_game/world/region/power_manager_runtime.cpp \
	src/old_game/world/region/power_manager_statue_runtime.cpp \
	src/old_game/pvp/battlefield/power_war_log_runtime.cpp \
	src/old_game/world/region/village_attacked_runtime.cpp \
	src/old_game/world/region/village_object_runtime.cpp \
	src/old_game/world/region/village_runtime.cpp \
	src/old_game/world/region/world_area_allocators_runtime.cpp \
	src/old_game/world/region/world_dispatcher_runtime.cpp \
	src/old_game/world/region/world_entry_channel_runtime.cpp \
	src/old_game/world/region/world_school_area_manager_runtime.cpp \
	src/old_game/world/region/world_area_setup_batch027.cpp \
	src/old_game/world/region/world_user_info_runtime.cpp \
	src/old_game/world/region/world_value_types.cpp \
	src/old_game/world/region/packet_dispatcher_world_business_043.cpp \
	src/old_game/world/region/packet_dispatcher_village_monster.cpp \
	src/old_game/pvp/battlefield/inter_power_war_global_receivers.cpp \
	src/old_game/world/broadcast/inter_server_message_receivers.cpp \
	src/old_game/world/region/packet_dispatcher_world_session.cpp \
	src/old_game/world/region/packet_dispatcher_set_pos_runtime.cpp \
	src/old_game/world/portal/solo_teleport_dispatcher.cpp

C_SRC = \
	$(C_GLOBAL_SRC) \
	$(C_GAME_MASTER_SRC) \
	$(C_CREATURE_AVATAR_SRC) \
	$(C_DUNGEON_PVP_SRC) \
	$(C_EVENT_SRC) \
	$(C_GROUP_SRC) \
	$(C_INVENTORY_SRC) \
	$(C_MAIL_TRADE_CASH_SRC) \
	$(C_NPC_SRC) \
	$(C_QUEST_SKILL_SRC) \
	$(C_USERINFO_SRC) \
	$(C_WORLD_SRC)

C_GLOBAL_SRC := \
	src/old_game/economy/trade/item_private_store_accessors.cpp \
	src/old_game/economy/inventory/item_account_cargo_accessors.cpp \
	src/old_game/economy/inventory/check_get_item_packet.cpp \
	src/common/manager/game_manager_upper_userinfo_runtime.cpp \
	src/old_game/world/region/game_manager_world_lists_runtime.cpp \
	src/old_data/Login/inter_returns/inter_monitor_call_userinfo_return.cpp

# GameMaster's shared ABI is its own functional runtime.  It remains detached
# from CMD179 until every registered new-command and oldGmRoutine transaction
# is restored; CMD065 preserves only its original disabled debug front-end and
# CMD110 is limited to its recovered GM protocol cases; neither adds a root
# registration table.
C_GAME_MASTER_SRC := \
	src/old_game/gm/core/game_master_runtime_abi.cpp \
	src/old_game/gm/core/game_master_mgr_runtime.cpp \
	src/old_game/gm/player/game_master_character_select_commands.cpp \
	src/old_game/gm/player/game_master_delete_account_cargo.cpp \
	src/old_game/gm/core/game_master_npc_shop_commands.cpp \
	src/old_game/gm/core/dispatcher_gm_command.cpp \
	src/old_game/gm/core/packet_dispatcher_debug_command.cpp \
	src/old_game/gm/core/dispatcher_new_gmdebug_command.cpp

# SR11 full-tree closure: these are existing, single-provider C restorations
# whose IDA names are referenced by the current private C queue.  They are
# grouped by their canonical gameplay owner; no draft trade-state provider is
# admitted here.
C_CREATURE_AVATAR_SRC += \
	src/old_game/appearance/avatar/avatar_roulette.cpp \
	src/old_game/appearance/avatar/world_avatar_cube_types.cpp \
	src/old_game/appearance/avatar/packet_dispatcher_gold_take_increasing_amount.cpp

C_DUNGEON_PVP_SRC += \
	src/old_game/dungeon/instance/instant_reinforce.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_instant_reinforce.cpp \
	src/old_game/dungeon/instance/time_spiral_runtime.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_break_trap_result.cpp \
	src/old_game/pvp/battlefield/battle_field_typed_lifecycle_preabi.cpp \
	src/old_game/dungeon/instance/cdungeon_accessors_runtime.cpp \
	src/old_game/dungeon/instance/cdungeon_maze_accessors_runtime.cpp \
	src/old_game/dungeon/tower/death_tower_packet_handler_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_stat_node_runtime.cpp \
	src/old_game/dungeon/instance/dungeon_runtime_constants.cpp \
	src/old_game/pvp/match/fair_pvp_match_interface.cpp \
	src/old_game/pvp/battlefield/pvp_assault_runtime.cpp \
	src/old_game/pvp/battlefield/pvp_assault_request_price_runtime.cpp \
	src/old_game/dungeon/room/timer_check_paid_pcroom_term_runtime.cpp \
	src/old_game/dungeon/instance/timer_dungeon_data_statistic_runtime.cpp \
	src/old_game/dungeon/instance/timer_dungeon_statistic_entrance_hour_runtime.cpp \
	src/old_game/dungeon/instance/timer_dungeon_inout_runtime.cpp \
	src/old_game/pvp/match/timer_fair_pvp_rematch_runtime.cpp \
	src/old_game/pvp/battlefield/timer_connect_p2p_pvp_timeout_runtime.cpp \
	src/old_game/pvp/battlefield/timer_tournament_dungeon_clear_state_select_runtime.cpp \
	src/old_game/pvp/battlefield/timer_update_pvp_grade_runtime.cpp \
	src/old_game/pvp/battlefield/relay_battle_timer_runtime.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_req_peer.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_res_peer.cpp \
	src/old_game/dungeon/instance/packet_dispatcher_eplp_command.cpp \
	src/old_game/pvp/arena/war_room_peer_join_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_leave_penalty_runtime.cpp

C_EVENT_SRC += \
	src/old_game/activity/schedule/advance_altar_data.cpp \
	src/old_game/activity/schedule/advance_altar_daily_ticket_runtime.cpp \
	src/old_game/activity/schedule/advance_altar_statistics_log_runtime.cpp \
	src/old_game/activity/schedule/packet_dispatcher_advance_altar_start_game.cpp \
	src/old_game/activity/schedule/packet_dispatcher_advance_altar_buy_item.cpp \
	src/old_game/activity/schedule/packet_dispatcher_advance_altar_pause.cpp \
	src/old_game/activity/schedule/character_event_records.cpp \
	src/old_game/activity/condition/character_restrict.cpp \
	src/old_game/activity/schedule/event_character_info.cpp \
	src/old_game/activity/schedule/event_table.cpp \
	src/old_game/activity/schedule/event_mgr_coin_character_create.cpp

C_EVENT_SRC += \
	src/old_game/activity/schedule/hero_mission_value_token_runtime.cpp \
	src/old_game/activity/schedule/hero_mission_set_event_data.cpp \
	src/old_game/activity/reward/packet_dispatcher_tournament_reward_select_state.cpp \
	src/old_game/activity/reward/packet_dispatcher_tournament_reward_select.cpp

C_GROUP_SRC += \
 src/old_game/social/party/packet_dispatcher_item_hyperlink_message.cpp \
	src/old_game/social/party/cparty_ridable_object_state.cpp \
	src/old_game/social/party/cparty_event_character_reward.cpp \
	src/old_game/social/party/cparty_ipinfo_runtime.cpp \
	src/old_game/social/party/cparty_tournament_timer.cpp \
	src/old_game/social/party/cparty_item_lookup_runtime.cpp \
	src/old_game/social/party/cparty_get_item_runtime.cpp \
	src/old_game/social/party/packet_dispatcher_get_item.cpp \
	src/old_game/social/party/cparty_set_play_result_runtime.cpp \
	src/old_game/social/party/cparty_clear_reward_runtime.cpp \
	src/old_game/social/party/cparty_dungeon_fail_runtime.cpp \
	src/old_game/social/party/cparty_on_timeout_boss_die_runtime.cpp \
	src/old_game/social/party/packet_dispatcher_set_play_result.cpp \
	src/old_game/social/party/timer_blood_dungeon_clear_reward_runtime.cpp \
	src/old_game/social/party/timer_dungeon_clear_runtime.cpp \
	src/old_game/social/party/cparty_tod_clear_reward_runtime.cpp

C_INVENTORY_SRC += \
	src/old_game/economy/inventory/avatar_coin.cpp \
	src/old_game/economy/equipment/packet_dispatcher_invest_item_amplify_option.cpp \
	src/old_game/economy/inventory/packet_dispatcher_use_booster_item.cpp \
	src/old_game/economy/equipment/packet_dispatcher_enchant_3rd_chronicle.cpp \
	src/old_game/economy/equipment/packet_dispatcher_upgrade_chronicle.cpp \
	src/old_game/economy/inventory/packet_dispatcher_mod_item_attr.cpp \
	src/old_game/economy/inventory/avatar_coin_packet_sync.cpp \
	src/old_game/economy/equipment/avatar_equipment_value_types.cpp \
	src/old_game/economy/inventory/inventory_avatar_item_mgr_access.cpp \
	src/old_game/economy/inventory/inventory_item_lookup.cpp \
	src/old_game/economy/inventory/inter_load_inven.cpp \
	src/old_game/economy/inventory/inventory_params.cpp \
	src/old_game/economy/inventory/cinventory_over_stackable_item.cpp

C_NPC_SRC += \
	src/old_game/economy/shop/npc_limit_packets.cpp \
	src/old_game/world/region/npc_relation_values.cpp \
	src/old_game/economy/shop/packet_dispatcher_give_gift_to_npc.cpp

C_QUEST_SKILL_SRC += src/old_game/progression/career/expert_job_giveup.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/expert_extraction_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_expert_extraction.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_monstercard_bind.cpp
C_QUEST_SKILL_SRC += src/old_game/gm/core/game_master_hero_mission.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/expert_job_peer_request_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/expert_job_peer_response_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/quest_delivery_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/force_change_grow_type_quest_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/skill_slot_grow_type_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/skill_tree_param_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/skill_slot_over_level_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/skill_slot_calc_used_sp_runtime.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_close_disjoint_store.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_disjoint_machine.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_disjoint_store.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/packet_dispatcher_request_seria_buff.cpp


C_USERINFO_SRC += \
	src/old_game/player/core/cuser_set_charac_level_runtime.cpp \
	src/old_game/player/core/character_cache_data.cpp \
	src/old_game/player/core/exchange_server_character_cache.cpp \
	src/old_game/player/core/character_growth_tables.cpp \
	src/old_game/player/core/character_status_data.cpp \
	src/old_game/player/core/calc_charac_stat_runtime.cpp \
	src/old_game/player/session/cuser_character_select_business04_leaf.cpp \
	src/old_game/player/core/ccharac_list_init_runtime.cpp \
	src/old_game/player/core/character_load_mercenary.cpp \
	src/old_game/player/core/character_load_mercenary_competition_runtime.cpp \
	src/old_game/player/core/character_manage_script.cpp \
	src/old_game/player/core/character_misc_data.cpp \
	src/old_game/player/core/character_packets.cpp \
	src/old_game/player/core/character_script_growth_data.cpp \
	src/old_game/player/core/character_view_data.cpp \
	src/old_game/player/core/cuser_history_log_mercenary.cpp \
	src/old_game/player/core/cuser_history_log_ridable.cpp \
	src/old_game/player/core/cuser_code_checksum_runtime.cpp \
	src/old_game/player/core/cuser_inventory_ref_runtime.cpp \
	src/old_game/player/core/cuser_pcroom_daily_reward_bridge.cpp \
	src/old_game/player/core/cuser_pvp_result_write_runtime.cpp \
	src/old_game/player/core/cuser_reward_exp_runtime.cpp \
	src/old_game/player/core/cuser_quest_auto_clear_runtime.cpp \
	src/old_game/player/core/userinfo_dispatcher_reads.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_master_new_skills_runtime.cpp

C_USERINFO_SRC += \
	src/old_game/gm/progression/cuser_change_grow_type_gm_runtime.cpp \
	src/old_game/player/core/cuser_premium_grow_type_runtime.cpp \
	src/old_game/player/core/cuser_reset_tag_info_runtime.cpp \
	src/old_game/player/core/cuser_history_log_premium_cube.cpp \
	src/old_game/player/core/packet_dispatcher_update_contract_cube.cpp

# CMD179 named GM command: CGameMasterMgr key "bongfail".
C_USERINFO_SRC += \
	src/old_game/gm/core/game_master_set_multibox_fail_count.cpp \
	src/old_game/gm/core/game_master_recipe_success.cpp \
	src/old_game/gm/core/game_master_mobile_restrict.cpp \
	src/old_game/gm/progression/game_master_level_up.cpp

C_WORLD_SRC += \
 src/old_game/world/broadcast/game_world_hyperlink_chat_runtime.cpp \
	src/old_game/world/region/stage_map_data.cpp \
	src/old_game/world/region/world_packet_structs.cpp \
	src/old_game/world/region/world_params.cpp \
	src/old_game/world/region/world_stage_map_scripts.cpp \
	src/old_game/pvp/battlefield/cmaplist_pvp_map_info.cpp

# GameWorld 05_world_enter_village service-cluster closure.  Each provider
# remains in its functional world unit; none is routed through an aggregate
# packet-dispatcher source.
C_WORLD_SRC += \
	src/old_game/world/region/game_world_user_routing_runtime.cpp \
	src/old_game/world/region/game_world_daily_reset_runtime.cpp \
	src/old_game/world/region/game_world_daily_maintenance_runtime.cpp \
	src/old_game/world/region/game_world_statistics_runtime.cpp \
	src/old_game/pvp/battlefield/game_world_power_war_lifecycle.cpp \
	src/old_game/pvp/battlefield/power_manager_join_power_war_runtime.cpp \
	src/old_game/world/region/game_world_mini_crane_runtime.cpp \
	src/old_game/world/region/game_world_event_staytime_runtime.cpp

# SR08 CMD108: complete C-owned state-0 derived dispatcher.  It is not
# inserted into the frozen PacketDispatcher registration table.
C_DUNGEON_PVP_SRC += src/old_game/social/party/packet_dispatcher_gathering_party_status.cpp

# CMD179 named GM command: CGameMasterMgr key "pcroomrewardreset".
C_DUNGEON_PVP_SRC += src/old_game/gm/dungeon/game_master_pcroom_reward_reset.cpp

# Original CGameMasterMgr numeric commands 2, 52, and 120.  Their concrete
# provider is kept with the Dungeon/PvP domain rather than the generic manager.
C_DUNGEON_PVP_SRC += src/old_game/gm/dungeon/game_master_pvp_dungeon_commands.cpp

# SR08 CMD117: complete C-owned state-0 derived dispatcher.  It is not
# inserted into the frozen PacketDispatcher registration table.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_creature_send_message.cpp

# SR08 CMD125: equipped-creature script-chat derived dispatcher.  It is not
# inserted into the frozen PacketDispatcher registration table.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_creature_script_message.cpp

# 140828 CMD101: strict requester-scene-id parser and NOTI104 creature
# response owner.  The action is animation-only and has no DB mutation.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_response_creature.cpp

# SR08 CMD103: creature rename dispatcher plus its exact state-0 generic
# specialization.  It remains absent from the frozen root registration table.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_rename_creature.cpp

# SR08 CMD105: creature-hatching dispatcher and its exact state-3 generic
# specialization.  It remains absent from the frozen root registration table.
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_hatch_creature.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_recover_stamina.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_history_log_redeem_item.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_redeem_item.cpp
C_USERINFO_SRC += src/old_game/player/core/packet_dispatcher_redeem.cpp

# SR08 CMD457: complete C-owned debug quest derived dispatcher.  No root
# PacketDispatcher registration is permitted until the entire tree closes.
C_QUEST_SKILL_SRC += src/old_game/progression/quest/packet_dispatcher_debug_clear_quest.cpp

# SR08 CMD454: complete C-owned urgent-quest derived dispatcher.  No root
# PacketDispatcher registration is permitted until the entire tree closes.
C_QUEST_SKILL_SRC += src/old_game/progression/quest/packet_dispatcher_req_urgent_quest.cpp

# SR08 CMD597: complete C-owned levelup-support derived dispatcher.  No root
# PacketDispatcher registration is permitted until the entire tree closes.
C_EVENT_SRC += src/old_game/activity/schedule/packet_dispatcher_levelup_support_request_item.cpp
C_EVENT_SRC += src/old_game/activity/schedule/packet_dispatcher_event_server_level_up.cpp

# SR08 CMD602: full hero-mission request/reward derived dispatcher and its
# owned typed event services.  The 606-entry root table remains untouched.
C_EVENT_SRC += src/old_game/activity/schedule/hero_mission_packet_services.cpp \
	src/old_game/activity/reward/packet_dispatcher_hero_mission_event_state_reward.cpp

# SR08 CMD459 prerequisite: the real embedded tournament reward object used
# by CParty owns card-selection state at CParty +0x17bc.
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/tournament_dungeon_reward_selection.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/tournament_dungeon_reward_runtime.cpp

# SR08 CMD508: BlueMarble confirmation dispatcher; no frozen-table mutation.
C_EVENT_SRC += src/old_game/activity/schedule/packet_dispatcher_blue_marble_confirm_info.cpp

# SR08 CMD506: growth-weapon material transaction; no frozen-table mutation.
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_growth_weapon_event_use_material.cpp
C_EVENT_SRC += src/old_game/activity/schedule/growth_equip_event.cpp
C_EVENT_SRC += src/old_game/activity/schedule/online_preliminary_event.cpp
C_EVENT_SRC += src/old_game/activity/schedule/premium_gold_card_event.cpp
C_EVENT_SRC += src/old_game/activity/schedule/pcroom_play_time_event.cpp

# SR08 CMD332: skill-command customization; no frozen-table mutation.
C_USERINFO_SRC += src/old_game/player/setting/cuser_customizing_skill_command.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/packet_dispatcher_skill_command_customizing.cpp

# SR08 CMD489: in-game advertisement selector; no frozen-table mutation.
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_ingame_advertisement.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_condition_event_reward.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/packet_dispatcher_buy_skill.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/packet_dispatcher_request_assault_price.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/skill/packet_dispatcher_refund_skill.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/career/packet_dispatcher_disjoint_item.cpp
C_QUEST_SKILL_SRC += src/old_game/progression/quest/packet_dispatcher_request_disjoint_item.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_guild_skill_message.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_delete_item_runtime.cpp
# IDA 0x867E734/0x8689010: world-entry daily stackable cleanup and
# reset-item-script selection.  These are CUser business bodies, not a
# PacketDispatcher registration change.
C_USERINFO_SRC += src/old_game/player/core/cuser_script_item_lifecycle_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_daily_reset_runtime.cpp
# IDA 0x8656CAA: daily stack restoration, script-item transaction, Advance
# Altar free-ticket refill, and the legacy date-gated item/mail branch.
C_USERINFO_SRC += src/old_game/player/core/cuser_daily_item_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_add_redeem_info_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_event_dungeon_clear_reward_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_dungeon_item_sell_count_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_security_auth_request_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_move_item_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_get_item_serial_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_get_item_distance_runtime.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/delete_item_param.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_delete_item.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_move_item.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/account_cargo_move_item_runtime.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/account_cargo_login_lifecycle.cpp
C_NPC_SRC += src/old_game/economy/shop/packet_dispatcher_sell_item.cpp
C_NPC_SRC += src/old_game/economy/trade/store_sell_item_runtime.cpp
C_NPC_SRC += src/old_game/economy/shop/npc_favorable_runtime.cpp

# Object/NM-verified SR08 PvP dispatcher providers used by the active
# PacketDispatcher slots 170 and 443.
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/packet_dispatcher_sr08_pvp_unclaimed.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/battle_field_item_lookup_runtime.cpp
C_DUNGEON_PVP_SRC += src/old_game/dungeon/tower/boss_tower_get_item_runtime.cpp
C_DUNGEON_PVP_SRC += src/old_game/dungeon/tower/death_tower_pickup_runtime.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/death_tower_reward_runtime.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/war_field_pickup_runtime.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/arena/war_room_get_item_runtime.cpp

# Canonical Dungeon/PvP providers found by the source-to-manifest audit.
# Keep map assembly, Death Tower drop policy, and the PC-room GM command in
# the C domain rather than leaving their IDA-marked bodies unlinked.
C_DUNGEON_PVP_SRC += \
	src/old_game/dungeon/tower/cdungeon_tower_of_despair_runtime.cpp \
	src/old_game/dungeon/tower/cdungeon_tower_item_drop_runtime.cpp \
	src/old_game/gm/dungeon/game_master_pcroom_premium_cmd.cpp

# PacketDispatcher 606-slot closure: restored C-owned gameplay dispatchers.
# These files already contain the IDA-derived bodies; this block only makes
# their canonical providers visible to the full-tree build.
C_CREATURE_AVATAR_SRC += src/old_game/appearance/avatar/arad_avatar_packet_dispatchers.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/arena/arad_request_pcroom_daily_reward_dispatcher.cpp
C_EVENT_SRC += src/old_game/activity/schedule/arad_event_create_dnf_request_dispatcher.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_sr08_inventory_unclaimed.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_use_title_change_item.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_exchange_item_from_npc.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/packet_dispatcher_cargo_transport_item.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/cinventory_identity_closure.cpp
C_INVENTORY_SRC += src/old_game/economy/inventory/cinventory_avatar_color_transition_runtime.cpp
C_MAIL_TRADE_CASH_SRC += src/old_game/economy/mall/packet_dispatcher_commerce_auction.cpp
C_NPC_SRC += src/old_game/economy/inventory/packet_dispatcher_clear_used_qp.cpp
C_QUEST_SKILL_SRC += \
	src/old_game/progression/skill/packet_dispatcher_change_another_skill_tree.cpp \
	src/old_game/progression/skill/packet_dispatcher_skill_command.cpp \
	src/old_game/progression/quest/packet_dispatcher_pvp_mission.cpp \
	src/old_game/progression/quest/pvp_mission_combo_clear_dispatcher.cpp
C_USERINFO_SRC += \
	src/old_game/player/core/title_book_packet_dispatcher.cpp \
	src/old_game/player/core/arad_db_charac_dispatcher.cpp \
	src/old_game/player/core/arad_jumping_character_packet_dispatcher.cpp \
	src/old_game/player/core/cuser_arad_jumping_access.cpp \
	src/old_game/player/core/inter_pad_reward_dispatcher.cpp \
	src/old_game/player/core/inter_regdate_dispatcher.cpp \
	src/old_game/player/core/inter_gold_control_dispatcher.cpp \
	src/old_game/player/core/cuser_load_etc_postload_packets.cpp \
	src/old_game/player/core/cuser_dungeon_clear_packet.cpp \
	src/old_game/player/session/cuser_select_character_mcap_runtime.cpp \
	src/old_game/player/core/cuser_one_a_day_item_shop_packet.cpp

# Direct IDA cUserHistoryLog leaves from 06_userinfo_stat_inventory.
C_USERINFO_SRC += src/old_game/player/core/cuser_history_log_identity.cpp
C_USERINFO_SRC += src/old_game/player/core/timer_avatar_expire_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/timer_creature_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/wongwork_cuser_premium_identity_closure.cpp

# Original CGameMasterMgr numeric commands 37/38/60/70.  Their concrete
# inventory and premium transactions belong to the USERINFO owner.
C_USERINFO_SRC += src/old_game/gm/item/game_master_inventory_clear_commands.cpp
C_USERINFO_SRC += src/old_game/gm/core/game_master_premium_delete_command.cpp
C_USERINFO_SRC += src/old_game/gm/item/game_master_get_cash_inven_command.cpp

# TimerDispatcher message 156: periodic event-script time refresh.
C_EVENT_SRC += src/old_game/activity/schedule/timer_event_management.cpp
C_EVENT_SRC += src/old_game/activity/schedule/event_runtime_db_sync.cpp
C_EVENT_SRC += src/old_game/activity/reward/inter_taiwan_event_reward_dispatchers.cpp

# Original CGameMasterMgr numeric registrations 46/47/102/125/127/128/130.
# The command implementations remain in their event owner unit.
C_EVENT_SRC += src/old_game/gm/core/game_master_event_commands.cpp

# 07 NPC/shop value-type lifecycle recovery.  This supplies the original
# NpcDialogue, SkillShopGuideTextData, and favorable-reward C2/D2/clear/assign
# providers without routing them through a packet dispatcher aggregate.
C_NPC_SRC += src/old_game/economy/mall/npc_shop_value_lifecycle_types.cpp

# PvP timer leaves and Fair PvP equipment validation.  The three canonical
# providers pass the 15-symbol CentOS6 i386 object/NM gate documented in
# C/probes/pvp_timer_frontier_object_nm_20260730.md.
C_DUNGEON_PVP_SRC += \
	src/old_game/pvp/battlefield/fair_pvp_equipment_validation_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_lack_user_timer_runtime.cpp \
	src/old_game/pvp/arena/pvp_room_timer_callbacks_runtime.cpp

# IDA 0x850CE40 / 0x865DA4E: equipment-state anti-cheat accounting and the
# independent expired-avatar batch provider.  The CParty consumer is already
# in C_GROUP_SRC; no dispatcher registration is inferred for SetEquipment.
C_INVENTORY_SRC += src/old_game/economy/equipment/cinventory_equipment_state_runtime.cpp
C_INVENTORY_SRC += src/old_game/economy/equipment/item_growth_state.cpp
C_INVENTORY_SRC += src/old_game/economy/equipment/equipment_growth_runtime.cpp
C_INVENTORY_SRC += src/old_game/economy/equipment/inter_clone_title.cpp
C_DUNGEON_PVP_SRC += src/old_game/pvp/battlefield/inter_integrated_match_pvp_score.cpp
C_USERINFO_SRC += src/old_game/player/core/cuser_equipment_load_runtime.cpp

# IDA 0x860CBA5/0x860CC38/0x860CD14: daily user-play statistic singleton
# and its fixed 20-record LOGDB stream serialization.
C_USERINFO_SRC += src/old_game/player/core/user_play_time_statistic_runtime.cpp
C_USERINFO_SRC += src/old_game/player/core/timer_insert_user_play_time_runtime.cpp

# Hell Party item-rarity statistic sender and its original local singleton.
C_GROUP_SRC += src/old_game/social/party/hell_party_item_statistic_runtime.cpp

# QuickParty quest-item drop-rate reward provider.  It remains a concrete
# reward-type body only; no constructor or CQuickPartyRewardManager slot is
# inferred because the old binary has no corresponding allocation xref.
C_QUEST_SKILL_SRC += src/old_game/progression/quest/quest_item_drop_rate_runtime.cpp
