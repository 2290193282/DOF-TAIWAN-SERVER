#ifndef OLDSRV_OLD_DATA_DB_ACCOUNT_LOGIN_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_ACCOUNT_LOGIN_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "common/config.h"
#include "infra/security/policy/secu_gold_control_abi.h"
#include "old_game/activity/schedule/character_event_records.h"

class CUser;
struct Inven_Item;
struct competition_reward_result;

#pragma pack(push, 1)

// IDA_EA: DB_Login::UpdateLoginStat @0x8414D50 and
// DB_UpdateLogin::dispatch @0x841826A.  The A-owned StreamGuard provider
// exports GetInBuffer<SIG_UPDATE_LOGIN> @0x8450A1E for this exact type.
struct SIG_UPDATE_LOGIN {
    uint32_t m_id;             // +0x00
    int32_t channel_no;        // +0x04
    int32_t server_group;      // +0x08
    char login_ip[0x14];       // +0x0C
};

static_assert(sizeof(SIG_UPDATE_LOGIN) == 0x20u,
              "SIG_UPDATE_LOGIN must preserve the original 32-byte wire ABI");

// IDA_EA: CUser::set_ingame_event_history @0x868D664 consumes the contiguous
// 30-record span at original SIG_LOGIN_DATA +0x3FAC. The expanded character
// view moves the same typed span to +0x40CC; it remains a view of the DB
// payload, not an independently allocated message.
struct SIG_INGAME_EVENT_HISTORY {
    struct Entry {
        int32_t type;
        int32_t first;
        int32_t second;
    } entries[30];
};

// IDA unit: CUser::setCharacLevelUpGift_AccountOnce @0x8691DCA.
// Inter_LoginInfo passes original SIG_LOGIN_DATA+0x4338 to this distinct
// consumer view. The expanded character view moves it to +0x4458. The
// upgraded transport payload is 0x4478 after one eight-byte visible
// equipment record was added to each of the 36 character-view entries. The DB
// loader fills
// only its four in-range records; the old consumer nevertheless traverses at
// most eleven entries and stops at the first zero level.  Keep this view
// separate so callers do not silently change the on-wire SIG_LOGIN_DATA ABI.
struct SIG_LEVELUP_GIFT_ACCOUNT_ONCE {
    struct Entry {
        int32_t job_type;
        int32_t level;
    } entries[11];
};

static_assert(sizeof(SIG_LEVELUP_GIFT_ACCOUNT_ONCE::Entry) == 0x08u,
              "SIG_LEVELUP_GIFT_ACCOUNT_ONCE entry ABI must stay 0x08");
static_assert(sizeof(SIG_LEVELUP_GIFT_ACCOUNT_ONCE) == 0x58u,
              "SIG_LEVELUP_GIFT_ACCOUNT_ONCE consumer view must stay 0x58");

static_assert(sizeof(SIG_INGAME_EVENT_HISTORY::Entry) == 0x0cu,
              "in-game event-history record must retain its 12-byte DB ABI");
static_assert(sizeof(SIG_INGAME_EVENT_HISTORY) == 0x168u,
              "in-game event-history span must retain 30 old DB records");

// IDA_EA: CUser::SetGrowthWeaponEventInfo @0x8691BAE receives this exact
// 20-byte view at original SIG_LOGIN_DATA +0x4140. The expanded character
// view moves it to +0x4260.
struct SIG_GROWTH_WEAPON_EVENT {
    uint8_t enabled;
    uint8_t alignment_01[3];
    int32_t charac_no;
    int32_t weapon_id;
    int32_t infinity_weapon_id;
    int16_t timepiece;
    int16_t server_id;
};

static_assert(sizeof(SIG_GROWTH_WEAPON_EVENT) == 0x14u,
              "growth-weapon login-event view must retain its old 20-byte ABI");

// IDA unit: 04_character_select_create/CHARAC_LOAD_MERCENARY.
// DB_Login::GetMercenarySystemInfo @0x8415618 writes these records at
// SIG_LOGIN_DATA +0x15B4 with a 0x54 stride.  The eight item identifiers at
// +0x34 are also consumed by the character-select runtime after DB loading.
struct CHARAC_LOAD_MERCENARY {
    CHARAC_LOAD_MERCENARY();
    ~CHARAC_LOAD_MERCENARY();

    CHARAC_LOAD_MERCENARY* Reset();
    void SendMessageToCUser(CUser* user, char* message) const;
    CHARAC_LOAD_MERCENARY* alter();
    CHARAC_LOAD_MERCENARY* save_complete();

    bool CheckCompetitionAreaIndex(char area_index,
                                   int charac_level) const;
    bool CheckCompetitionPeriodIndex(char period_index) const;
    float GetPeriodBonus() const;
    float GetAvatarBonus() const;
    float GetRewardCritical(int reward_dice) const;
    int CalcItemDropRate(CUser* user) const;
    int GetCompetitionState() const;
    int GetCompetitionPeriodSecond(char period_index) const;
    int GetCompetitionPeriodTerm(bool* partial_period) const;
    int GetRewardGold(CUser* user,
                      competition_reward_result& result) const;
    int GetRewardItem(CUser* user,
                      competition_reward_result& result,
                      Inven_Item& item,
                      std::string& server_message);

    // IDA_EA: 0x81B3814
    // IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY11CompetitionEP5CUsercc
    // DEMANGLED: CHARAC_LOAD_MERCENARY::Competition(CUser *,char,char)
    int Competition(CUser* user,
                    char competition_type,
                    char competition_count);

    // IDA_EA: 0x81B3B68
    // IDA_NAME: _ZN21CHARAC_LOAD_MERCENARY6RewardEP5CUserR25competition_reward_result
    // DEMANGLED: CHARAC_LOAD_MERCENARY::Reward(
    //             CUser *,competition_reward_result &)
    int Reward(CUser* user, competition_reward_result& result);

    uint32_t charac_no;             // +0x00
    uint16_t level;                 // +0x04
    char name[0x1e];                // +0x06
    uint8_t save_flag;              // +0x24
    uint8_t alignment_25[3];        // +0x25
    uint32_t start_time;            // +0x28
    uint32_t end_time;              // +0x2C
    uint8_t competition_area;       // +0x30
    uint8_t competition_period;     // +0x31
    uint8_t avatar_bonus_index;     // +0x32
    uint8_t alignment_33;           // +0x33
    int32_t avatar_item_id[8];      // +0x34
};

// IDA_EA: Stream::GetOutBuffer<SIG_LOGIN_DATA> @0x818FBB4,
// Stream::GetInBuffer<SIG_LOGIN_DATA> @0x823AE18, and
// CStreamGuard::GetInBuffer<SIG_LOGIN_DATA> @0x8237712.
// The DB/login producers establish the complete 0x4478 wire layout below.
struct SIG_LOGIN_DATA {
    struct CharacViewEntry {
        struct IdentityFields {
            char character_name[0x14]; // +0x04 from enclosing record
            uint8_t job;               // +0x18
            uint8_t grow_type;         // +0x19
            int16_t level;             // +0x1A
            uint8_t state_flags;       // +0x1C
            uint8_t tag_skill;         // +0x1D
            uint32_t tag_charac_no;     // +0x1E
        };

        struct VisibleItem {
            uint32_t item_id;
            uint16_t option;
            uint16_t color;
        };

        uint32_t charac_no;             // +0x00
        IdentityFields identity;         // +0x04
        uint8_t request_rename_state;    // +0x22
        uint8_t visible_header[5];       // +0x23
        VisibleItem visible_items[
            oldsrv::config::kCharacterViewEquipmentTypeCount]; // +0x28
        uint32_t create_time;            // +0x90
        uint32_t rename_expire_time;     // +0x94
        uint32_t visible_state;          // +0x98
    };

    struct PremiumEntry {
        int32_t premium_type;
        int32_t service_start;
        int32_t service_end;
        int32_t event_id;
    };

    struct PunishRow {
        int32_t punish_type;
        uint32_t occurrence_time;
        int32_t punish_value;
        int32_t apply_flag;
        uint32_t start_time;
        uint32_t end_time;
    };

    struct EventHistoryEntry {
        int32_t type;
        int32_t history_1;
        int32_t history_2;
    };

    struct AccountLevelGiftEntry {
        int32_t job_type;
        int32_t level;
    };

    char login_user_id[0x18];                // +0x0000
    char login_password[0x84];               // +0x0018
    uint32_t game_server_group;              // +0x009C
    uint8_t login_state;                     // +0x00A0
    char login_ip[0x14];                     // +0x00A1
    uint8_t ip_permission_granted;            // +0x00B5
    uint8_t alignment_00b6[2];               // +0x00B6
    uint32_t fail_code;                      // +0x00B8
    uint32_t login_result;                   // +0x00BC
    uint32_t member_id;                      // +0x00C0
    uint32_t account_age_flags;              // +0x00C4
    uint32_t birth_year;                     // +0x00C8
    uint32_t register_day;                   // +0x00CC
    uint32_t register_date_unix;             // +0x00D0
    uint32_t register_time;                  // +0x00D4
    uint32_t punish_duration;                // +0x00D8
    uint32_t charac_view_count;              // +0x00DC
    union {
        CharacViewEntry charac_view[36];      // +0x00E0
        char charac_view_blob[36 * sizeof(CharacViewEntry)];
                                               // compressed DB blob at +0x00E0
    };

    uint32_t mercenary_count;                // +0x16D0
    CHARAC_LOAD_MERCENARY mercenaries[36];   // +0x16D4
    char user_name[0x80];                    // +0x22A4
    char email[0x80];                        // +0x2324
    char member_identity[0x10];              // +0x23A4
    uint32_t premium_count;                  // +0x23B4
    PremiumEntry premium_entries[106];       // +0x23B8
    uint16_t premium_alignment;              // +0x2A58
    uint16_t premium_event_count;            // +0x2A5A
    int32_t premium_event_ids[1000];         // +0x2A5C
    uint8_t premium_alignment_38dc;          // +0x39FC
    uint8_t hangame_flag;                    // +0x39FD

    // Names containing an old offset retain their IDA-derived spelling; the
    // comments below record offsets in the expanded transport layout.
    uint16_t punish_second_code;             // +0x39FE
    bool passpad_enabled;                    // +0x3A00
    char passpad_mousepass[0x24];            // +0x3A01
    char encrypted_password[7];              // +0x3A25
    char passpad_result;                     // +0x3A2C
    bool advice_change_password;             // +0x3A2D
    bool passpad_converted;                  // +0x3A2E
    uint8_t passpad_alignment;               // +0x3A2F
    long passpad_validity_time;              // +0x3A30 (32-bit old ABI)
    int32_t passpad_fail_count;               // +0x3A34
    int32_t passpad_cancel_count;             // +0x3A38
    uint8_t passpad_service_state[0x30];      // +0x3A3C

    uint32_t daily_clean_pad_point;           // +0x3A6C
    int32_t account_fail;                     // +0x3A70
    int32_t total_account_fail;               // +0x3A74
    int32_t last_play_time;                   // +0x3A78
    // DB_Login fills this value from member_login.dungeon_gain_gold; the
    // login receiver passes it directly to CDungeonGainedGold::setGainedGold.
    uint32_t dungeon_gain_gold;               // +0x3A7C
    int32_t report_count;                     // +0x3A80
    uint8_t reliable_flag;                    // +0x3A84
    uint8_t sso_alignment_3965[3];            // +0x3A85
    uint32_t trade_gold_daily;                // +0x3A88
    uint32_t school_id;                       // +0x3A8C
    float rating;                             // +0x3A90
    char school_name[0x28];                   // +0x3A94
    int16_t school_badge[3];                  // +0x3ABC
    uint8_t tutorial_skipable;                // +0x3AC2
    uint8_t school_alignment_39a3[5];         // +0x3AC3
    uint32_t school_broadcast_server_group;   // +0x3AC8
    uint32_t current_charac_no;               // +0x3ACC
    char current_charac_name[0x20];           // +0x3AD0
    uint32_t online_preliminary_server_group; // +0x3AF0
    uint32_t online_preliminary_charac_no;    // +0x3AF4
    uint32_t pvp_mode_mask;                   // +0x3AF8
    uint8_t security_card_service;            // +0x3AFC
    uint8_t security_alignment_39dd[3];       // +0x3AFD
    uint8_t security_card_loaded;             // +0x3B00
    uint8_t security_card_valid;              // +0x3B01
    uint8_t security_alignment_39e2[2];       // +0x3B02
    int32_t security_card_fail_count;         // +0x3B04
    uint32_t security_card_cancel_count;      // +0x3B08
    char security_card_cert_flag;             // +0x3B0C
    char security_card[0x27];                 // +0x3B0D
    int16_t security_card_q_pos;              // +0x3B34
    uint8_t security_alignment_3a16[2];       // +0x3B36
    char security_card_q_pos_text[5];         // +0x3B38
    uint8_t security_alignment_3a1d[7];       // +0x3B3D
    uint32_t punish_second_trade_count;       // +0x3B44
    unsigned long long punish_second_trade_gold; // +0x3B48
    uint32_t token_timestamp;                 // +0x3B50
    uint32_t token_state;                     // +0x3B54
    int16_t server_group;                     // +0x3B58
    char sso_login_ip[0x10];                  // +0x3B5A
    uint8_t school_point_joined;               // +0x3B6A
    uint8_t school_point_alignment;           // +0x3B6B
    uint32_t daily_clean_pad_count;           // +0x3B6C
    uint32_t punish_count;                    // +0x3B70
    PunishRow punish_rows[50];                // +0x3B74

    uint8_t return_user_loaded;               // +0x4024
    uint8_t return_user_first_login;          // +0x4025
    uint8_t return_user_alignment[2];         // +0x4026
    int32_t return_user_value;                // +0x4028
    uint8_t punish_user_marker;               // +0x402C
    uint8_t reward_security_flags;            // +0x402D
    uint8_t reward_daily_reset;               // +0x402E
    uint8_t reward_state;                     // +0x402F
    uint8_t event_charac_flag;                // +0x4030
    uint8_t event_charac_alignment[3];        // +0x4031
    // IDA_EA: Inter_LoginInfo::dispatch_sig @0x84C7675 reads original
    // SIG_LOGIN_DATA+0x3F14. The expanded payload stores the same field at
    // +0x4034 and passes it to CUser::SetMileage.
    int32_t mileage;                          // +0x4034
    int32_t break_away_accrue_cera;           // +0x4038
    char break_away_play_info[0x20];          // +0x403C
    uint32_t break_away_last_reward_time;     // +0x405C
    uint32_t break_away_first_reward_time;    // +0x4060
    uint32_t break_away_second_reward_time;   // +0x4064
    int32_t break_away_add_info;              // +0x4068
    uint32_t break_away_quest_time;           // +0x406C
    uint32_t break_away_last_update_time;     // +0x4070
    int32_t break_away_luck_point;            // +0x4074
    uint8_t break_away_member;                // +0x4078
    uint8_t break_away_alignment[3];          // +0x4079
    char charac_link_message;                 // +0x407C
    uint8_t charac_link_alignment;            // +0x407D
    uint16_t return_user_bonus_fatigue;       // +0x407E
    uint32_t return_user_update_time;         // +0x4080
    // The old input buffer holds the complete Secu_GoldControl source record
    // here.  Inter_LoginInfo passes this object to SetInfo; it is not generic
    // return-user padding.
    user_gold_info gold_control_info;         // +0x4084
    uint8_t charac_slot_effect_count;         // +0x40C0
    uint8_t charac_slot_limit;                // +0x40C1
    uint8_t event_counter_alignment[2];       // +0x40C2
    int32_t ontime_user_recv_index;           // +0x40C4
    uint8_t random_option_check_count;        // +0x40C8
    uint8_t fatigue_occurrence_day;           // +0x40C9
    uint16_t fatigue_count;                   // +0x40CA
    EventHistoryEntry in_game_event_history[30]; // +0x40CC
    char previous_login_ip[0x10];             // +0x4234
    uint8_t login_ip_changed;                 // +0x4244
    uint8_t login_event_state[0x13];          // +0x4245
    char bingo_reward;                        // +0x4258
    uint8_t bingo_alignment[3];               // +0x4259
    int32_t bingo_board;                      // +0x425C
    uint8_t growth_weapon_active;             // +0x4260
    uint8_t growth_weapon_alignment[3];       // +0x4261
    int32_t growth_weapon_charac_no;          // +0x4264
    int32_t growth_weapon_id;                 // +0x4268
    int32_t growth_weapon_infinity_id;        // +0x426C
    int16_t growth_weapon_timepiece;          // +0x4270
    int16_t growth_weapon_server_id;          // +0x4272
    SIG_GIVE_GROWTH_CREATURE_EVENT growth_creature_event; // +0x4274
    AccountLevelGiftEntry account_level_gifts[4];    // +0x4458
};

#pragma pack(pop)

static_assert(sizeof(CHARAC_LOAD_MERCENARY) == 0x54u,
              "CHARAC_LOAD_MERCENARY IDA ABI size must stay 0x54");
static_assert(offsetof(CHARAC_LOAD_MERCENARY, avatar_item_id) == 0x34u,
              "CHARAC_LOAD_MERCENARY avatar item IDs must stay at +0x34");
static_assert(sizeof(SIG_LOGIN_DATA::CharacViewEntry) == 0x9Cu,
              "SIG_LOGIN_DATA character-view entry must stay 0x9C");
static_assert(sizeof(SIG_LOGIN_DATA::CharacViewEntry::IdentityFields) == 0x1eu,
              "character-view identity span must stay 0x1E");
static_assert(
    sizeof(((SIG_LOGIN_DATA::CharacViewEntry::IdentityFields*)0)
               ->character_name) == 0x14u,
    "character-view character name must stay 20 bytes");
static_assert(sizeof(SIG_LOGIN_DATA::PremiumEntry) == 0x10u,
              "SIG_LOGIN_DATA premium entry must stay 0x10");
static_assert(sizeof(SIG_LOGIN_DATA::PunishRow) == 0x18u,
              "SIG_LOGIN_DATA punish row must stay 0x18");
static_assert(sizeof(SIG_LOGIN_DATA::EventHistoryEntry) == 0x0cu,
              "SIG_LOGIN_DATA event history entry must stay 0x0C");
static_assert(sizeof(SIG_LOGIN_DATA) == 0x4478u,
              "SIG_LOGIN_DATA native ABI size must stay 0x4478");

static_assert(offsetof(SIG_LOGIN_DATA::CharacViewEntry, identity) == 0x04u,
              "character-view identity must stay at +0x04");
static_assert(
    offsetof(SIG_LOGIN_DATA::CharacViewEntry, identity) +
            offsetof(SIG_LOGIN_DATA::CharacViewEntry::IdentityFields,
                     character_name) ==
        0x04u,
    "character-view character name must stay at +0x04");
static_assert(
    offsetof(SIG_LOGIN_DATA::CharacViewEntry, identity) +
            offsetof(SIG_LOGIN_DATA::CharacViewEntry::IdentityFields,
                     grow_type) ==
        0x19u,
    "character-view growth type must stay at +0x19");
static_assert(
    offsetof(SIG_LOGIN_DATA::CharacViewEntry, identity) +
            offsetof(SIG_LOGIN_DATA::CharacViewEntry::IdentityFields, level) ==
        0x1au,
    "character-view level must stay at +0x1A");
static_assert(offsetof(SIG_LOGIN_DATA::CharacViewEntry, visible_items) == 0x28u,
              "character-view visible items must stay at +0x28");
static_assert(offsetof(SIG_LOGIN_DATA, member_id) == 0x00c0u,
              "SIG_LOGIN_DATA member ID must stay at +0xC0");
static_assert(offsetof(SIG_LOGIN_DATA, charac_view) == 0x00e0u,
              "SIG_LOGIN_DATA character view must stay at +0xE0");
static_assert(offsetof(SIG_LOGIN_DATA, mercenary_count) == 0x16d0u,
              "SIG_LOGIN_DATA mercenary count must stay at +0x16D0");
static_assert(offsetof(SIG_LOGIN_DATA, mercenaries) == 0x16d4u,
              "SIG_LOGIN_DATA mercenary records must stay at +0x16D4");
static_assert(offsetof(SIG_LOGIN_DATA, user_name) == 0x22a4u,
              "SIG_LOGIN_DATA user name must stay at +0x22A4");
static_assert(offsetof(SIG_LOGIN_DATA, premium_entries) == 0x23b8u,
              "SIG_LOGIN_DATA premium entries must stay at +0x23B8");
static_assert(offsetof(SIG_LOGIN_DATA, premium_event_ids) == 0x2a5cu,
              "SIG_LOGIN_DATA premium event IDs must stay at +0x2A5C");
static_assert(offsetof(SIG_LOGIN_DATA, passpad_enabled) == 0x3a00u,
              "SIG_LOGIN_DATA passpad flag must stay at +0x3A00");
static_assert(offsetof(SIG_LOGIN_DATA, last_play_time) == 0x3a78u,
              "SIG_LOGIN_DATA last-play time must stay at +0x3A78");
static_assert(offsetof(SIG_LOGIN_DATA, current_charac_name) == 0x3ad0u,
              "SIG_LOGIN_DATA current character name must stay at +0x3AD0");
static_assert(offsetof(SIG_LOGIN_DATA, punish_second_trade_count) == 0x3b44u,
              "SIG_LOGIN_DATA second-punish count must stay at +0x3B44");
static_assert(offsetof(SIG_LOGIN_DATA, punish_second_trade_gold) == 0x3b48u,
              "SIG_LOGIN_DATA second-punish trade gold must stay at +0x3B48");
static_assert(sizeof(unsigned long long) == 8u,
              "SIG_LOGIN_DATA second-punish trade gold must stay 64-bit");
static_assert(offsetof(SIG_LOGIN_DATA, server_group) == 0x3b58u,
              "SIG_LOGIN_DATA server group must stay at +0x3B58");
static_assert(offsetof(SIG_LOGIN_DATA, sso_login_ip) == 0x3b5au,
              "SIG_LOGIN_DATA SSO login IP must stay at +0x3B5A");
static_assert(offsetof(SIG_LOGIN_DATA, break_away_accrue_cera) == 0x4038u,
              "SIG_LOGIN_DATA break-away data must stay at +0x4038");
static_assert(offsetof(SIG_LOGIN_DATA, mileage) == 0x4034u,
              "SIG_LOGIN_DATA mileage must stay at +0x4034");
static_assert(offsetof(SIG_LOGIN_DATA, in_game_event_history) == 0x40ccu,
              "SIG_LOGIN_DATA event history must stay at +0x40CC");
static_assert(offsetof(SIG_LOGIN_DATA, growth_creature_event) == 0x4274u,
              "SIG_LOGIN_DATA growth-creature entries must stay at +0x4274");
static_assert(offsetof(SIG_LOGIN_DATA, account_level_gifts) == 0x4458u,
              "SIG_LOGIN_DATA account-level gifts must stay at +0x4458");

#endif  // OLDSRV_OLD_DATA_DB_ACCOUNT_LOGIN_PAYLOADS_H_
