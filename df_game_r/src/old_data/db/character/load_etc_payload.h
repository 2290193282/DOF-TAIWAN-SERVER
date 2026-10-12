#ifndef OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_ETC_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_ETC_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>

#include <bitset>
#include <type_traits>

#include "old_game/player/setting/character_option.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_data/db/dungeon/best_clear_time_payload.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_data/db/skill/item_making_skill_payload.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/dungeon/instance/tod_layer.h"
#include "old_game/activity/schedule/event_messages.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/redeem_item_runtime.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/skill/combo_skill.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/player/core/title_book_runtime.h"
#include "old_data/db/inventory/achievement_payload.h"
#include "infra/network/packet/stream.h"

// The following records are the named constituents selected by
// Stream::GetOutBuffer<SIG_LOAD_ETC> @0x845063C.  Their order and widths are
// fixed by SIG_LOAD_ETC::SIG_LOAD_ETC @0x8491F86 and by the DB_LoadEtc
// readers at 0x8403BEE..0x844A704.

#pragma pack(push, 1)

struct LimitItemUsagePayload {
    // CUser::GetLimitItemUsage @0x8688DC4 writes this byte from
    // LimitItemUsage+0x10.  The record is consumed again by
    // CUser::SetLimitInfo @0x86888F8.
    uint8_t limit_type;       // +0x00
    uint8_t ida_off_001;      // +0x01, not consumed by the old DB path
    uint8_t ida_off_002;      // +0x02, not consumed by the old DB path
    uint8_t ida_off_003;      // +0x03, not consumed by the old DB path
    // LimitItemUsage's CommonTime::current_time (+0x0c), not a begin-time
    // interval.  The old loader restores it through SetCreateTime.
    uint32_t last_usage_time; // +0x04
    uint32_t item_id;         // +0x08
    uint32_t used_count;      // +0x0c
};

// IDA unit: 99_misc_global/SIG_LIMIT_ITEM_INFO.  Inter_LoadEtc passes the
// address of SIG_LOAD_ETC+0x2638 directly to CUser::SetLimitInfo, so this
// prefix is a real typed view of that contiguous payload segment.
struct SIG_LIMIT_ITEM_INFO {
    uint32_t count;                           // +0x00
    LimitItemUsagePayload entries[60];        // +0x04
};

// IDA_EA: 0x848FF7C.  CUser copies all 0x199 bytes at 0x868148C; the DB
// loader fills 204 two-byte mastered-skill records and the trailing state.
struct SIG_LOAD_PREMIUM_LETHE {
    SIG_LOAD_PREMIUM_LETHE();

    _Mastered_skill mastered_skills[204];
    uint8_t lethe_state;
};

// IDA_EA: 0x8491EE0.  The two 204-entry trees and 50 encoded commands account
// for the complete 0x414-byte Stream payload proven at 0x8450B84.
struct SIG_LOAD_SKILL {
    SIG_LOAD_SKILL();

    uint32_t charac_no;                   // +0x000
    uint32_t result;                      // +0x004
    uint32_t remain_sp;                   // +0x008
    _Mastered_skill first_tree[204];      // +0x00c
    uint32_t remain_sp_second;            // +0x1a4
    _Mastered_skill second_tree[204];     // +0x1a8
    uint32_t remain_sfp_first;            // +0x340
    uint32_t remain_sfp_second;           // +0x344
    STSkillCommandData::STCommand skill_commands[50]; // +0x348
    uint8_t skill_table_index;            // +0x410
    uint8_t script_version;               // +0x411
    uint8_t ida_off_412;                  // +0x412
    uint8_t ida_off_413;                  // +0x413
};

// IDA_EA: 0x832AEDE/0x848F8D4.
struct MissionInfo {
    MissionInfo();
    MissionInfo* clear();

    uint16_t mission_kind;
    uint16_t mission_index;
    uint16_t current_value;
    uint16_t target_value;
};

struct SIG_REQUEST_PVP_MISSION_LIST {
    SIG_REQUEST_PVP_MISSION_LIST();

    uint32_t charac_no;
    MissionInfo missions[36];
    uint16_t mission_state[16];
    uint16_t issue_mode_win;
    uint16_t issue_mode_loss;
    uint16_t issue_all;
};

// IDA_EA: 0x8491F68.
struct SIG_REQUEST_TOD_USER_STATE {
    SIG_REQUEST_TOD_USER_STATE();

    uint32_t charac_no;
    int32_t first_layer_start_time;
    int32_t today_enter_count;
    TOD_Layer last_clear_layer;
    int16_t enter_count_by_week;
    int32_t modified_time;
    int32_t last_clear_time;
};

// IDA_EA: 0x848FAB2.
struct SIG_ITEM_DICTIONARY_FLAG {
    SIG_ITEM_DICTIONARY_FLAG();

    uint32_t charac_no;
    uint8_t flags[5000];
};

// IDA_EA: 0x848FB4C.
struct SIG_BOOSTER_GAGE_DATA {
    SIG_BOOSTER_GAGE_DATA();

    uint32_t account_id;
    int32_t gage;
};

// IDA_EA: 0x8491F86/0x84921F0; complete size proven by
// Stream::GetOutBuffer<SIG_LOAD_ETC> @0x845063C.
struct SIG_LOAD_ETC {
    SIG_LOAD_ETC();
    ~SIG_LOAD_ETC();

    uint32_t account_id;                         // +0x00000
    uint32_t charac_no;                          // +0x00004
    uint32_t result;                             // +0x00008
    uint32_t tag_charac_no;                      // +0x0000c
    TAG_CHARAC_ETC_DATA tag_charac_etc;           // +0x00010
    PvpResultType pvp_result;                    // +0x00340
    uint32_t battle_record_count;                // +0x003c4
    GameResultType battle_records[100];          // +0x003c8
    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO cooltime_items; // +0x02308
    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO effect_items;   // +0x024a0
    SIG_LIMIT_ITEM_INFO limit_item_info;          // +0x02638
    uint8_t one_day_lethe_state;                 // +0x029fc
    SIG_LOAD_PREMIUM_LETHE premium_lethe;        // +0x029fd
    uint8_t one_day_lethe_second_state;          // +0x02b96
    SIG_LOAD_PREMIUM_LETHE premium_lethe_second; // +0x02b97
    uint32_t server_info;                        // +0x02d30
    uint32_t channel_no;                         // +0x02d34
    SIG_EVENT_SERVER_MESSAGE server_message_info; // +0x02d38
    uint32_t npc_relationship_count;             // +0x03e6c
    stNPCRelationship_t npc_relationships[50];   // +0x03e70
    uint16_t condition_event_step;               // +0x040c8
    uint16_t condition_event_reward_step;        // +0x040ca
    uint16_t proper_dungeon_clear_count;         // +0x040cc
    SIG_REDEEMINFO_LIST_TO_DB redeem_items;      // +0x040ce
    stTitleBook title_book;                      // +0x0466a
    stAchievement_DBData achievement;            // +0x0b1a4
    uint8_t village_attack_count;                // +0x0c9ac
    uint8_t revenge_dungeon_count;               // +0x0c9ad
    uint8_t avenger_title_received;              // +0x0c9ae
    SIG_SAVE_PC_ROOM_PLAY_TIME pc_room_play_time; // +0x0c9af
    uint32_t used_gift_fatigue;                  // +0x0c9b8
    std::bitset<32> user_confirm_flags;           // +0x0c9bc
    SIG_LOAD_SKILL skill;                        // +0x0c9c0
    uint8_t create_pvp_skill_row;                // +0x0cdd4
    SIG_REQUEST_PVP_MISSION_LIST pvp_missions;   // +0x0cdd5
    SIG_FAIR_PVP_SCORE fair_pvp;                 // +0x0cf1f
    SIG_QUEST_SHOP_DATA quest_shop;              // +0x0d5a7
    SIG_ITEM_MAKING_SKILL item_making_skill;     // +0x0d6fc
    SIG_REQUEST_TOD_USER_STATE tod_user_state;   // +0x0d718
    stAuraAvatarOption aura_avatar_option;       // +0x0d730
    SIG_ITEM_DICTIONARY_FLAG item_dictionary;    // +0x0d73c
    CharacterOption character_option;            // +0x0eac8
    SIG_BOOSTER_GAGE_DATA booster_gage;          // +0x0ecc8
    SIG_BEST_CLEAR_TIME best_clear_time;         // +0x0ecd0
    uint8_t blue_marble_enter_count;             // +0x14a98
    SIG_ATTENDANCE_EVENT attendance;             // +0x14a99
    char charac_inform_notice[16];                // +0x14a9a
    SIG_COMBO_SKILL combo_skill;                 // +0x14aaa
    seriaRoom_AniDeco seria_room_ani_deco;       // +0x14b5c
};

#pragma pack(pop)

static_assert(sizeof(LimitItemUsagePayload) == 0x10u,
              "LimitItemUsagePayload must match IDA");
static_assert(sizeof(SIG_LIMIT_ITEM_INFO) == 0x3c4u,
              "SIG_LIMIT_ITEM_INFO must match IDA");
static_assert(offsetof(SIG_LIMIT_ITEM_INFO, entries) == 0x04u,
              "SIG_LIMIT_ITEM_INFO entries must match IDA");
static_assert(sizeof(SIG_LOAD_PREMIUM_LETHE) == 0x199u,
              "SIG_LOAD_PREMIUM_LETHE must match IDA");
static_assert(sizeof(Redeem_Item::Inven_Item_Expand_RedeemInfo) == 0x44u,
              "redeem item payload must match IDA");
static_assert(
    std::is_trivially_destructible<
        Redeem_Item::Inven_Item_Expand_RedeemInfo>::value,
    "redeem item payload must keep constructor-only Inven_Item lifetime");
static_assert(sizeof(SIG_REDEEMINFO_LIST_TO_DB) == 0x59cu,
              "SIG_REDEEMINFO_LIST_TO_DB must match IDA");
static_assert(std::is_trivially_destructible<
                  SIG_REDEEMINFO_LIST_TO_DB>::value,
              "redeem list must not synthesize an old-server D2");
static_assert(sizeof(stAchievement) == 0x0cu,
              "stAchievement must match IDA");
static_assert(sizeof(stAchievement_DBData) == 0x1808u,
              "stAchievement_DBData must match IDA");
static_assert(sizeof(SIG_LOAD_SKILL) == 0x414u,
              "SIG_LOAD_SKILL must match IDA");
static_assert(offsetof(SIG_LOAD_SKILL, first_tree) == 0x00cu &&
                  offsetof(SIG_LOAD_SKILL, second_tree) == 0x1a8u &&
                  offsetof(SIG_LOAD_SKILL, skill_commands) == 0x348u &&
                  offsetof(SIG_LOAD_SKILL, skill_table_index) == 0x410u &&
                  offsetof(SIG_LOAD_SKILL, script_version) == 0x411u,
              "SIG_LOAD_SKILL field offsets must match the old DB signal");
static_assert(sizeof(MissionInfo) == 0x08u,
              "MissionInfo must match IDA");
static_assert(sizeof(SIG_REQUEST_PVP_MISSION_LIST) == 0x14au,
              "SIG_REQUEST_PVP_MISSION_LIST must match IDA");
static_assert(sizeof(SIG_FAIR_PVP_SCORE) == 0x688u,
              "SIG_LOAD_ETC fair-PvP payload must match IDA");
static_assert(sizeof(SIG_ITEM_MAKING_SKILL) == 0x1cu,
              "SIG_ITEM_MAKING_SKILL must match IDA");
static_assert(sizeof(SIG_REQUEST_TOD_USER_STATE) == 0x18u,
              "SIG_REQUEST_TOD_USER_STATE must match IDA");
static_assert(sizeof(SIG_ITEM_DICTIONARY_FLAG) == 0x138cu,
              "SIG_ITEM_DICTIONARY_FLAG must match IDA");
static_assert(sizeof(SIG_BOOSTER_GAGE_DATA) == 0x08u,
              "SIG_BOOSTER_GAGE_DATA must match IDA");
static_assert(sizeof(std::bitset<32>) == 0x04u,
              "old i386 std::bitset<32> storage must stay four bytes");

static_assert(offsetof(SIG_LOAD_ETC, tag_charac_etc) == 0x10u,
              "SIG_LOAD_ETC tag data offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, pvp_result) == 0x340u,
              "SIG_LOAD_ETC pvp offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, battle_records) == 0x3c8u,
              "SIG_LOAD_ETC record offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, cooltime_items) == 0x2308u,
              "SIG_LOAD_ETC cooltime offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, limit_item_info) == 0x2638u,
              "SIG_LOAD_ETC limit-item payload offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, premium_lethe) == 0x29fdu,
              "SIG_LOAD_ETC lethe offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, server_message_info) == 0x2d38u &&
                  offsetof(SIG_EVENT_SERVER_MESSAGE, messages) == 0x04u,
              "SIG_LOAD_ETC server-message offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, npc_relationships) == 0x3e70u,
              "SIG_LOAD_ETC NPC offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, redeem_items) == 0x40ceu,
              "SIG_LOAD_ETC redeem offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, title_book) == 0x466au,
              "SIG_LOAD_ETC title-book offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, achievement) == 0xb1a4u,
              "SIG_LOAD_ETC achievement offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, user_confirm_flags) == 0xc9bcu,
              "SIG_LOAD_ETC event flags offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, skill) == 0xc9c0u,
              "SIG_LOAD_ETC skill offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, pvp_missions) == 0xcdd5u,
              "SIG_LOAD_ETC PvP mission offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, fair_pvp) == 0xcf1fu,
              "SIG_LOAD_ETC fair-PvP offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, quest_shop) == 0xd5a7u,
              "SIG_LOAD_ETC quest-shop offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, item_making_skill) == 0xd6fcu,
              "SIG_LOAD_ETC item-making offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, tod_user_state) == 0xd718u,
              "SIG_LOAD_ETC TOD offset after item-making must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, aura_avatar_option) == 0xd730u,
              "SIG_LOAD_ETC aura offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, character_option) == 0xeac8u,
              "SIG_LOAD_ETC character-option offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, best_clear_time) == 0xecd0u,
              "SIG_LOAD_ETC best-clear offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, attendance) == 0x14a99u,
              "SIG_LOAD_ETC attendance offset must match IDA");
static_assert(offsetof(SIG_LOAD_ETC, combo_skill) == 0x14aaau,
              "SIG_LOAD_ETC combo offset must match IDA");
static_assert(sizeof(SIG_LOAD_ETC) == 84832u,
              "SIG_LOAD_ETC must match Stream IDA size 84832");

#endif  // OLDSRV_OLD_DATA_DB_CHARACTER_LOAD_ETC_PAYLOAD_H_
