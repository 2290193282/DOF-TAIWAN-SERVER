#include "old_game/player/core/cuser.h"

#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/play_info_public_abi.h"
#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/economy/equipment/equipment_growth_runtime.h"
#include "old_game/economy/inventory/packet_dispatcher_cargo_transport_item.h"
#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/activity/schedule/levelup_support_3rd.h"
#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/activity/schedule/king_of_pick.h"
#include "old_game/activity/schedule/exp_keeping_event.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/activity/schedule/collect_box_dispatcher.h"
#include "old_game/activity/schedule/glass_bottle_event.h"
#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"
#include "old_game/activity/schedule/blue_horse_event.h"
#include "old_game/activity/schedule/sea_bottom_event.h"
#include "old_game/pvp/tournament/pvp_tournament.h"
#include "old_game/dungeon/racing/racing_dungeon.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"

#include <cstring>


void UserMercenaryInfoMgr::ClearForUserReset() {
    mercenaries_.clear();
}

// IDA_EA: 0x8658030
// IDA_NAME: _ZN5CUser5resetEv
// DEMANGLED: CUser::reset(void)
CUser* CUser::reset() {
    equipment_growth_runtime::ResetUserPersistence(this);
    cargo_transport_item_runtime::ResetUserPersistence(this);
    npc_gift::Erase(this);
    freely_give_item::FlushAndErase(this);
    collect_box::ResetSession(this);
    decide_levelup_event::ResetSession(this);
    levelup_support_3rd::ResetSession(this);
    fatigue_acceleration::ResetSession(this);
    champion_break::ResetSession(this);
    facebook_like::ResetSession(this);
    king_of_pick::ResetSession(this);
    event_attendance::ResetSession(this);
    exp_keeping::ResetSession(this);
    pvp_tournament::ResetSession(this);
    racing_dungeon::OnUserReset(this);
    channel_zombie_matching_producer::ResetUser(this);
    glass_bottle_event::FlushAndErase(this);
    shooting_star_dimension_pocket::FlushAndErase(this);
    blue_horse_event::FlushAndErase(this);
    sea_bottom_event::FlushAndErase(this);
    secret_event_shop::CloseForUser(this);
    clone_title::ResetUser(this);
    dungeon_runtime::ResetInCountUser(this);
    ResetCurCharac();

    gm_boss_tower_stage_override_ = 0u;
    gm_boss_tower_map_index_override_ = 0u;
    antibot_packet_state_ = 0u;
    // Expanded reset writes CUser+585256 (0x8ee28), the buying-gold byte.
    buying_gold_ = 0u;

    user_premium_.InitPremium();
    characters_.clear();
    characters_.reserve(8u);
    ting_flag_ = 0u;
    character_view_.reset();

    hack_analyzer_.resetHackInfo();
    for (int hack_type = 101; hack_type <= 4000; ++hack_type) {
        hack_analyzer_.resetServerHackAccumulatedCnt(
            static_cast<WongWork::ENUM_HACKTYPE>(hack_type));
    }

    account_hacking_.resetInfo();
    WongWork::CMailBox* const mailbox = GetMailBox();
    if (mailbox) {
        WongWork::CMailBoxHelper::FreeMailBox(mailbox);
        mailbox_ = 0;
    }

    restricted_goods_.clear();
    InitFinishPoint();
    quest_reward_request_count_ = 0u;
    quest_reward_request_tick_ = 0u;
    set_guildwar_point_per_pvpplay(0);
    std::memset(&guild_info_, 0, sizeof(guild_info_));
    std::memset(&guild_agit_info_, 0, sizeof(guild_agit_info_));
    today_guild_member_ = 0u;

    code_hack_storage_.reset();
    SetReliablePerson(false);
    SetOverTradeGoldCount(0u);
    setUserEventCharacterFlag(0u);
    black_list_.clear();

    charac_info_load_lock_ = 0u;
    charac_info_delete_lock_ = 0u;
    charac_info_create_lock_ = 0u;
    character_statistic_.statistic_received_ = 0u;
    character_.PlayInfo::Reset();
    last_login_charac_no_ = 0;
    last_login_channel_no_ = 0;

    hack_user_timer_ = 0u;
    hack_user_flag_ = 0u;
    total_hack_user_trade_gold_ = 0u;
    total_hack_user_trade_count_ = 0u;

    setAntibotKey(-1);
    setClientVersion(0u);
    dungeon_gained_gold_.reset();
    joined_school_point_event_ = 0u;
    character_statistic_.rating_storage_ = 0u;

    current_area_ = -1;
    account_used_fatigue_ = 0u;

    levelup_support_events_.clear();
    charac_expand_data_.reset();
    login_out_state_ = 0u;
    limit_item_usage_.clear();
    village_attacked_reward_ = 0u;

    account_id_ = 0;
    return_user_ = 0u;
    account_cargo_.Clear();
    save_member_bonus_fatigue_ = 0u;
    account_member_bonus_fatigue_ = 0u;
    chatting_message_count_ = 0;
    duplicate_message_index_ = 0u;
    user_strings_[0].clear();
    user_strings_[1].clear();
    return_user_expire_time_ = 0;
    return_user_first_login_ = 0u;

    ResetUsedCoinInDungeon();
    DBUpdateDBLogItem();
    ResetDBLogItem();
    action_point_manager_.Reset();
    CerashopAddRestrict::Manager::ClearBuyRestrictItem(this);
    reset_server_fatigue_data();
    reset_ingame_event_history();
    reset_ingame_event_history_update();

    mercenary_info_.ClearForUserReset();
    network_lifecycle_state_.mercenary_reward_dice_override = 0u;
    network_lifecycle_state_.mercenary_selection_index = -1;
    resetMoneyLog();
    antibot_send_buffer_count_ = 0u;
    antibot_dproto_callback_count_ = 0u;
    received_request_type_ = -1;
    pu_user_ = 0u;

    quest_.reset();
    manage_level_ = 0;
    manage_point_ = 0;
    user_max_level_ = 0;
    save_rental_info_to_exchange_ = 1u;
    // Original reset writes CUser+454545 (0x6ef91), the load-from-exchange
    // state paired with save_rental_info_to_exchange_ at +0x6ef90.
    load_rental_info_from_exchange_ = 0u;
    bingo_data_.clear();
    levelup_gifts_.gifts_.clear();
    growth_creature_event_.event_characters.clear();
    game_option_.reset_load_hotkey_flag();
    allow_location_geo_state_ = 0u;
    mobile_auth_reward_state_ = 0u;
    pcroom_daily_reward_remain_ = 0;
    return this;
}
