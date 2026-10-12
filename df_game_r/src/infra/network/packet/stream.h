#ifndef OLDSRV_PROTOCOL_STREAM_H_
#define OLDSRV_PROTOCOL_STREAM_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/account/login_payloads.h"
#include "old_data/db/character/create_charac_payload.h"
#include "old_data/db/creature/creature_item_payload.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_data/db/quest/expert_job_payload.h"
#include "old_data/db/statistics_log/statistics_log_payloads.h"

// IDA_EA: DB_MailBox_Req_List::dispatch @0x841DE4A and
// WongWork::CMailBoxHelper::SetMailBoxInfo @0x85525A6.  Signal 127 uses the
// same fixed-size frame in both directions: the first 0x64 bytes are the DB
// request. The 0627 snapshot ends at 0x338A; the rebuilt 0828 pair appends
// twenty Chronicle values without shifting any legacy field.
// Every field below is evidenced by one of those two original functions; the
// packed declaration is the wire ABI, not an in-memory convenience layout.
#pragma pack(push, 1)
struct SIG_LETTER_INFO {
    ~SIG_LETTER_INFO();

    uint32_t letter_id;          // +0x000
    int32_t send_charac_no;      // +0x004
    char send_charac_name[29];   // +0x008
    char letter_text[256];       // +0x025
    int32_t reg_time;            // +0x125
    int16_t stat;                // +0x129
};

struct SIG_MAILBOX_RANDOM_OPTION_FIELD {
    uint8_t option_index;        // +0x00
    uint8_t first_value;         // +0x01
    uint8_t second_value;        // +0x02
};

struct SIG_MAILBOX_RANDOM_OPTION {
    SIG_MAILBOX_RANDOM_OPTION_FIELD option_fields[3]; // +0x00
    uint8_t initial_seed;                                // +0x09
    SIG_MAILBOX_RANDOM_OPTION_FIELD changed_option;       // +0x0a
    uint8_t changed_option_seed;                          // +0x0d
};

struct SIG_MAILBOX_MAIL_ENTRY {
    uint32_t postal_id;          // +0x00
    int32_t send_charac_no;      // +0x04
    char send_charac_name[29];   // +0x08
    int32_t gold;                // +0x25
    uint8_t seal_flag;           // +0x29
    uint32_t item_id;            // +0x2a
    int32_t add_info;            // +0x2e
    uint8_t upgrade;             // +0x32
    uint16_t endurance;          // +0x33
    uint32_t extend_info;        // +0x35
    uint8_t item_guid[10];       // +0x39
    uint8_t amplify_option;      // +0x43
    uint16_t amplify_value;      // +0x44
    SIG_MAILBOX_RANDOM_OPTION random_option; // +0x46
    uint8_t separate_upgrade;    // +0x54
    int32_t expire_time;         // +0x55 (postal occ_time + 15 days)
    uint8_t avatar_flag;         // +0x59
    uint8_t creature_flag;       // +0x5a
    uint8_t unlimit_flag;        // +0x5b
    uint32_t letter_id;          // +0x5c
    uint8_t type;                // +0x60
};

struct SIG_MAILBOX_AVATAR_EMBLEM_SOCKET {
    uint16_t socket_type;        // +0x00
    uint32_t emblem_item_index;  // +0x02
};

struct SIG_MAILBOX_AVATAR_EMBLEM_INFO {
    SIG_MAILBOX_AVATAR_EMBLEM_SOCKET sockets[5];
};

struct SIG_MAILBOX_AVATAR_ENTRY {
    int32_t ui_id;               // +0x00
    int32_t expire_time;         // +0x04
    uint32_t reg_time;           // +0x08
    char ipg_agency_no[25];      // +0x0c
    SIG_MAILBOX_AVATAR_EMBLEM_INFO emblem_info; // +0x25
    uint16_t avatar_expansion[2]; // +0x43
};

struct SIG_MAILBOX_CREATURE_ENTRY {
    int32_t ui_id;               // +0x00
    int32_t expire_time;         // +0x04
    int32_t reg_time;            // +0x08
    // The original uses a 37-byte record stride.  The remaining fixed
    // agency field is retained by the wire format; 0x841DC84 deliberately
    // leaves it zero because its SELECT projects only the three fields above.
    char ipg_agency_no[25];      // +0x0c
};

struct SIG_MAILBOX_LIST_REQUEST {
    int32_t charac_no;           // +0x00
    int32_t last_postal_id;      // +0x04
    int32_t remain_count;        // +0x08
    int32_t last_letter_id;      // +0x0c
    int32_t mail_load_count;     // +0x10
    uint32_t package_load_lack[20]; // +0x14
};

struct SIG_MAILBOX_LIST_RESPONSE {
    SIG_MAILBOX_MAIL_ENTRY mails[20];         // +0x0000
    int32_t mail_count;                        // +0x0794
    int32_t not_loaded_mail_count;             // +0x0798
    uint32_t last_postal_id;                   // +0x079c
    int32_t avatar_count;                      // +0x07a0
    SIG_MAILBOX_AVATAR_ENTRY avatars[20];      // +0x07a4
    int32_t creature_count;                    // +0x0d30
    SIG_MAILBOX_CREATURE_ENTRY creatures[20];  // +0x0d34
    int32_t letter_count;                      // +0x1018
    SIG_LETTER_INFO letters[30];               // +0x101c
    int32_t requested_mail_count;              // +0x3326
    int32_t loaded_letter_adjusted;            // +0x332a
    int32_t letter_carryover_count;            // +0x332e
    int32_t charac_no;                         // +0x3332
    int32_t mail_load_count;                   // +0x3336
    uint32_t changed_package_ids[20];          // +0x333a
    // 140828 native extension indexed exactly like mails[].
    uint32_t chronicle_values[20];             // +0x338a
};

struct SIG_MAILBOX_LIST {
    union {
        SIG_MAILBOX_LIST_REQUEST request;
        SIG_MAILBOX_LIST_RESPONSE response;
    };
    // The wire union contains SIG_LETTER_INFO, whose original D1/D2 is an
    // exported ABI symbol. Define the union lifetime explicitly without
    // changing any byte in the DB frame.
    SIG_MAILBOX_LIST() {}
    ~SIG_MAILBOX_LIST() {}
};
#pragma pack(pop)

static_assert(sizeof(SIG_LETTER_INFO) == 0x12bu,
              "SIG_LETTER_INFO IDA ABI size must stay 0x12B");
static_assert(sizeof(SIG_MAILBOX_RANDOM_OPTION) == 0x0eu,
              "mail random-option wire value must stay 14 bytes");
static_assert(sizeof(SIG_MAILBOX_MAIL_ENTRY) == 0x61u,
              "mail entry wire layout must stay 97 bytes");
static_assert(offsetof(SIG_MAILBOX_MAIL_ENTRY, gold) == 0x25u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, item_id) == 0x2au &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, add_info) == 0x2eu &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, endurance) == 0x33u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, extend_info) == 0x35u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, item_guid) == 0x39u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, random_option) == 0x46u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, separate_upgrade) == 0x54u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, expire_time) == 0x55u &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, letter_id) == 0x5cu &&
                  offsetof(SIG_MAILBOX_MAIL_ENTRY, type) == 0x60u,
              "mail entry fields must retain the IDA wire offsets");
static_assert(sizeof(SIG_MAILBOX_AVATAR_ENTRY) == 0x47u,
              "avatar mail entry wire layout must stay 71 bytes");
static_assert(offsetof(SIG_MAILBOX_AVATAR_ENTRY, ipg_agency_no) == 0x0cu &&
                  offsetof(SIG_MAILBOX_AVATAR_ENTRY, emblem_info) == 0x25u &&
                  offsetof(SIG_MAILBOX_AVATAR_ENTRY, avatar_expansion) == 0x43u,
              "avatar mail entry fields must retain the IDA wire offsets");
static_assert(sizeof(SIG_MAILBOX_CREATURE_ENTRY) == 0x25u,
              "creature mail entry wire layout must stay 37 bytes");
static_assert(offsetof(SIG_MAILBOX_CREATURE_ENTRY, ipg_agency_no) == 0x0cu,
              "creature mail entry agency field must retain the wire offset");
static_assert(sizeof(SIG_MAILBOX_LIST_REQUEST) == 0x64u,
              "mail-list request wire layout must stay 100 bytes");
static_assert(offsetof(SIG_MAILBOX_LIST_REQUEST, package_load_lack) == 0x14u,
              "mail-list request package-lack list must stay at +0x14");
static_assert(offsetof(SIG_MAILBOX_LIST_RESPONSE, mail_count) == 0x0794u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, avatars) == 0x07a4u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, creature_count) == 0x0d30u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, creatures) == 0x0d34u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, letter_count) == 0x1018u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, letters) == 0x101cu &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, requested_mail_count) == 0x3326u &&
                  offsetof(SIG_MAILBOX_LIST_RESPONSE, changed_package_ids) == 0x333au,
              "mail-list reply sections must retain the IDA wire offsets");
static_assert(offsetof(SIG_MAILBOX_LIST_RESPONSE, charac_no) == 0x3332u,
              "mail-list reply character offset must stay 0x3332");
static_assert(offsetof(SIG_MAILBOX_LIST_RESPONSE, mail_load_count) == 0x3336u,
              "mail-list reply load-count offset must stay 0x3336");
static_assert(offsetof(SIG_MAILBOX_LIST_RESPONSE, chronicle_values) == 0x338au,
              "0828 Chronicle array must follow the legacy mail frame");
static_assert(sizeof(SIG_MAILBOX_LIST_RESPONSE) == 0x33dau,
              "0828 mail-list response must include Chronicle values");
static_assert(sizeof(SIG_MAILBOX_LIST) == 0x33dau,
              "0828 mail-list signal must include Chronicle values");

// IDA_EA: Stream::GetInBuffer<SIG_AUTO_MARKET_CONDITIONS_CONTROL>
// @0x8456B06 and CStreamGuard::GetInBuffer<...> @0x845305E use the old
// 0x6C DB-load payload. The DB update companion is 0x30 bytes at
// Stream::GetOutBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL> @0x8453074
// and Stream::GetInBuffer<SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL>
// @0x8456B70.
#pragma pack(push, 1)
struct SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY {
    uint64_t total_gold;            // +0x00
    uint64_t auction_gold;          // +0x08
    uint64_t over_gold;             // +0x10
    uint64_t optimum_gold_supply;   // +0x18
    int32_t gold_phase;             // +0x20
    int32_t item_phase;             // +0x24
    int32_t durability_phase;       // +0x28
};

struct SIG_AUTO_MARKET_CONDITIONS_CONTROL {
    uint64_t control_over_gold;             // +0x00
    uint64_t control_optimum_gold_supply;   // +0x08
    SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY previous_day; // +0x10
    SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY current_day;  // +0x3C
    uint8_t request_flag;                   // +0x68
    uint8_t raw_69[3];                      // +0x69
};

struct SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL {
    uint64_t total_gold;            // +0x00
    uint64_t auction_gold;          // +0x08
    uint64_t over_gold;             // +0x10
    uint64_t optimum_gold_supply;   // +0x18
    int32_t gold_phase;             // +0x20
    int32_t item_phase;             // +0x24
    int32_t durability_phase;       // +0x28
    uint8_t raw_2c[4];              // +0x2C
};

// IDA_EA: ItemVendingMachine::ProcessBuyRequestEx @0x8177D59..0x8177FED.
// This is an in-process Stream record, not a packed wire packet.  Its
// naturally aligned option records are eight bytes wide: the original writes
// the item at +0 and selection byte at +4 in every record.
#pragma pack(pop)
// ProcessBuyRequestEx uses the old i386 four-byte aggregate alignment here.
// Keep it independent of an includer's active packing state.
#pragma pack(push, 4)
struct stCeraShopIPGRequestOption_t {
    uint32_t item_id;                // +0x00 in an option record
    uint8_t selected;                // +0x04 in an option record
};

struct stCeraShopIPGRequestAdditionalItem_t {
    uint32_t item_id;                // +0x00 in an additional-item record
    uint16_t first_option;           // +0x04 in an additional-item record
    uint16_t second_option;          // +0x06 in an additional-item record
};

// IDA_EA: CStreamGuard::GetInBuffer<stCeraShopIPGRequest_t> @0x817A50C,
// Stream::GetOutBuffer<stCeraShopIPGRequest_t> @0x817A522, and
// Stream::GetInBuffer<stCeraShopIPGRequest_t> @0x817A9AA.  The full layout
// is established by ProcessBuyRequestEx @0x8177D74..0x8177FED.
struct stCeraShopIPGRequest_t {
    uint32_t goods_no;                                      // +0x000
    uint32_t group_index;                                   // +0x004
    uint32_t item_index;                                    // +0x008
    uint16_t stack_count;                                   // +0x00C
    int32_t cera_price;                                     // +0x010
    int32_t win_point;                                      // +0x014
    int32_t gold_price;                                     // +0x018
    int32_t avatar_selection_value;                         // +0x01C
    uint32_t item_count;                                    // +0x020
    uint32_t item_order;                                    // +0x024
    uint32_t featured_index;                                // +0x028
    uint8_t select_type;                                    // +0x02C
    uint8_t avatar_select;                                  // +0x02D
    int32_t current_seconds;                                // +0x030
    uint8_t gift;                                           // +0x034
    // The source message word immediately after its recipient text is copied
    // verbatim by 0x8177E80.  Its business meaning is not inferred here.
    uint32_t post_recipient_wire_word;                      // +0x038
    uint16_t buy_type;                                      // +0x03C
    // Original request bytes +0x03e..+0x03f are reserved.  The recipient
    // suffix is read by ProcessBuyRequestEx from +0x040.
    uint8_t buy_type_reserved[2];                           // +0x03E
    char recipient_name_suffix[4];                          // +0x040
    uint8_t option_count;                                   // +0x044
    stCeraShopIPGRequestOption_t options[35];               // +0x048
    uint8_t additional_item_count;                          // +0x160
    stCeraShopIPGRequestAdditionalItem_t additional_items[35]; // +0x164
};
#pragma pack(pop)
#pragma pack(push, 1)

// IDA_EA: Stream::GetInBuffer<SIG_SWAP_CREATURE_ITEM> @0x8344FFC,
// Stream::GetOutBuffer<SIG_SWAP_CREATURE_ITEM> @0x8451AFA, and
// CStreamGuard::GetInBuffer<SIG_SWAP_CREATURE_ITEM> @0x8343AC8.
struct SIG_SWAP_CREATURE_ITEM {
    uint32_t first_ui_id;           // +0x00
    uint32_t second_slot;           // +0x04
    uint8_t first_empty;            // +0x08
    uint8_t raw_09[3];              // +0x09
    uint32_t second_ui_id;          // +0x0C
    uint32_t first_slot;            // +0x10
    uint8_t second_empty;           // +0x14
    uint8_t raw_15[3];              // +0x15
};

// IDA_EA: Stream::GetInBuffer<SIG_CREATURE_ITEM_LIST> @0x834546E,
// Stream::GetOutBuffer<SIG_CREATURE_ITEM_LIST> @0x8451AA6, and
// CStreamGuard::GetInBuffer<SIG_CREATURE_ITEM_LIST> @0x8344058.
struct SIG_CREATURE_ITEM_LIST {
    uint32_t count;                              // +0x0000
    Inven_Item inventory_items[141];             // +0x0004
    uint8_t inventory_alignment[3];              // +0x219D
    SIG_CREATURE_ITEM_DETAIL_INFO details[141];  // +0x21A0
};

static_assert(offsetof(SIG_CREATURE_ITEM_LIST, inventory_items) == 0x0004u &&
                  offsetof(SIG_CREATURE_ITEM_LIST, details) == 0x21A0u &&
                  sizeof(SIG_CREATURE_ITEM_LIST) == 0x4078u,
              "SIG_CREATURE_ITEM_LIST must retain the 0627 save-list ABI");

// IDA_EA: CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_AG> @0x847DDD6.
// Consumed by cUserHistoryLog::Auction @0x8685452.
struct SIG_AUCTION_LOG_MESSAGE_AG {
    uint8_t kind;                  // +0x00
    uint64_t auction_id;           // +0x01
    int32_t item_id;               // +0x09
    int32_t bid_price;             // +0x0D
    uint32_t item_count;           // +0x11
    uint8_t item_grade;            // +0x15
    int32_t register_price;        // +0x16
    uint16_t register_period;      // +0x1A
    uint32_t fee;                  // +0x1C
    uint8_t fail_reason;           // +0x20
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_AUCTION_LOG_MESSAGE_PG> @0x847DE02.
// Consumed by cUserHistoryLog::AuctionPoint @0x8685654.
struct SIG_AUCTION_LOG_MESSAGE_PG {
    uint8_t kind;                  // +0x00
    uint64_t auction_id;           // +0x01
    int32_t item_id;               // +0x09
    int32_t bid_price;             // +0x0D
    uint32_t item_count;           // +0x11
    uint8_t item_grade;            // +0x15
    int32_t register_price;        // +0x16
    uint16_t register_period;      // +0x1A
    uint32_t fee;                  // +0x1C
};

// IDA_EA: CStreamGuard::GetInBuffer<SIG_AUCTION_MONEY_SUB> @0x8237B96,
// Stream::GetInBuffer<SIG_AUCTION_MONEY_SUB> @0x823B368, and
// Stream::GetOutBuffer<SIG_AUCTION_MONEY_SUB> @0x8454626.
// The dispatcher fills money and charac_name; DB_AuctionSubMoney fills the
// account id on the return-side record.
struct SIG_AUCTION_MONEY_SUB {
    uint32_t account_id;           // +0x00
    uint32_t money;                // +0x04
    char charac_name[13];          // +0x08
    uint8_t raw_15[19];            // +0x15
};

#pragma pack(pop)

static_assert(sizeof(SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY) == 0x2Cu,
              "SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY IDA ABI size must stay 0x2C");
static_assert(sizeof(SIG_AUTO_MARKET_CONDITIONS_CONTROL) == 0x6Cu,
              "SIG_AUTO_MARKET_CONDITIONS_CONTROL IDA ABI size must stay 0x6C");
static_assert(sizeof(SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL) == 0x30u,
              "SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL IDA ABI size must stay 0x30");
static_assert(sizeof(stCeraShopIPGRequest_t) == 0x27Cu,
              "stCeraShopIPGRequest_t IDA ABI size must stay 0x27C");
static_assert(sizeof(stCeraShopIPGRequestOption_t) == 0x08u,
              "stCeraShopIPGRequestOption_t IDA ABI size must stay 0x08");
static_assert(sizeof(stCeraShopIPGRequestAdditionalItem_t) == 0x08u,
              "stCeraShopIPGRequestAdditionalItem_t IDA ABI size must stay 0x08");
static_assert(offsetof(SIG_AUTO_MARKET_CONDITIONS_CONTROL, previous_day) ==
                  0x10u,
              "SIG_AUTO_MARKET_CONDITIONS_CONTROL previous_day offset must stay 0x10");
static_assert(offsetof(SIG_AUTO_MARKET_CONDITIONS_CONTROL, current_day) ==
                  0x3Cu,
              "SIG_AUTO_MARKET_CONDITIONS_CONTROL current_day offset must stay 0x3C");
static_assert(offsetof(SIG_AUTO_MARKET_CONDITIONS_CONTROL, request_flag) ==
                  0x68u,
              "SIG_AUTO_MARKET_CONDITIONS_CONTROL request_flag offset must stay 0x68");
static_assert(
    offsetof(SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL, gold_phase) == 0x20u,
    "SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL gold_phase offset must stay 0x20");
static_assert(offsetof(stCeraShopIPGRequest_t, item_index) == 0x08u,
              "stCeraShopIPGRequest_t item_index offset must stay 0x08");
static_assert(offsetof(stCeraShopIPGRequest_t, cera_price) == 0x10u &&
                  offsetof(stCeraShopIPGRequest_t, select_type) == 0x2cu &&
                  offsetof(stCeraShopIPGRequest_t, current_seconds) == 0x30u &&
                  offsetof(stCeraShopIPGRequest_t, post_recipient_wire_word) ==
                      0x38u &&
                  offsetof(stCeraShopIPGRequest_t, recipient_name_suffix) ==
                      0x40u &&
                  offsetof(stCeraShopIPGRequest_t, options) == 0x48u &&
                  offsetof(stCeraShopIPGRequest_t, additional_items) ==
                      0x164u,
              "stCeraShopIPGRequest_t ProcessBuyRequestEx offsets changed");
static_assert(sizeof(SIG_SWAP_CREATURE_ITEM) == 0x18u,
              "SIG_SWAP_CREATURE_ITEM IDA ABI size must stay 0x18");
static_assert(sizeof(SIG_CREATURE_ITEM_LIST) == 0x4078u,
              "SIG_CREATURE_ITEM_LIST IDA ABI size must stay 0x4078");
static_assert(offsetof(SIG_SWAP_CREATURE_ITEM, second_ui_id) == 0x0Cu,
              "SIG_SWAP_CREATURE_ITEM second_ui_id offset must stay 0x0C");
static_assert(offsetof(SIG_SWAP_CREATURE_ITEM, first_slot) == 0x10u,
              "SIG_SWAP_CREATURE_ITEM first_slot offset must stay 0x10");
static_assert(offsetof(SIG_CREATURE_ITEM_LIST, details) == 0x21A0u,
              "SIG_CREATURE_ITEM_LIST details offset must stay 0x21A0");
static_assert(sizeof(SIG_AUCTION_LOG_MESSAGE_AG) == 0x21u,
              "SIG_AUCTION_LOG_MESSAGE_AG IDA ABI size must stay 0x21");
static_assert(sizeof(SIG_AUCTION_LOG_MESSAGE_PG) == 0x20u,
              "SIG_AUCTION_LOG_MESSAGE_PG IDA ABI size must stay 0x20");
static_assert(sizeof(SIG_AUCTION_MONEY_SUB) == 0x28u,
              "SIG_AUCTION_MONEY_SUB IDA ABI size must stay 0x28");
static_assert(offsetof(SIG_AUCTION_LOG_MESSAGE_AG, auction_id) == 0x01u,
              "SIG_AUCTION_LOG_MESSAGE_AG auction_id offset must stay 0x01");
static_assert(offsetof(SIG_AUCTION_LOG_MESSAGE_AG, bid_price) == 0x0du,
              "SIG_AUCTION_LOG_MESSAGE_AG bid_price offset must stay 0x0D");
static_assert(offsetof(SIG_AUCTION_LOG_MESSAGE_AG, fail_reason) == 0x20u,
              "SIG_AUCTION_LOG_MESSAGE_AG fail_reason offset must stay 0x20");
static_assert(offsetof(SIG_AUCTION_LOG_MESSAGE_PG, fee) == 0x1cu,
              "SIG_AUCTION_LOG_MESSAGE_PG fee offset must stay 0x1C");

// The following wire records are shared by the original Stream and
// CStreamGuard template specializations.  Field positions come from the
// corresponding old-server DB/game callers rather than from their former
// byte-count-only wrappers.
#pragma pack(push, 1)

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x80E2938 and
// Stream::GetInBuffer<...> @0x80E35C2.  DB_LoadCerashopAddRestrictType
// writes the two-word header and at most ten (ipg_no,count,next_date) rows.
struct SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE {
    struct RestrictEntry {
        uint32_t ipg_no;           // +0x00 from the entry
        uint32_t count;            // +0x04 from the entry
        uint32_t next_date;        // +0x08 from the entry
    };

    uint32_t row_count;            // +0x00
    uint32_t restrict_scope;       // +0x04: account=0, character=1
    RestrictEntry entries[10];     // +0x08
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x80F9E44 and
// Stream::GetInBuffer<...> @0x80FA8DC.  DB_SaveClientHackLog consumes the
// unaligned 32-bit fields at +0x0A and +0x0E.
struct SIG_SAVE_CLIENT_HACK_LOG {
    uint32_t account_id;           // +0x00
    uint32_t charac_no;            // +0x04
    uint16_t hack_type;            // +0x08
    int32_t etc;                   // +0x0A
    int32_t hack_count;            // +0x0E
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x81575F0 and
// Stream::GetInBuffer<...> @0x8157E38.  CBossDungeonEntranceLog increments
// the first three counters and accumulates stage elapsed time in the fourth.
struct STBossDungeonTowerLog {
    uint32_t stage_start_count;    // +0x00
    uint32_t stage_end_count;      // +0x04
    uint32_t picked_item_count;    // +0x08
    uint32_t elapsed_time;         // +0x0C
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x8236E34 and
// Stream::GetInBuffer<...> @0x8239AEC.  DB_FindFactoryHub reads this exact
// request shape and emits its fatigue-check companion.
struct SIG_FIND_FACTORY_HUB {
    uint32_t account_id;           // +0x00
    char charac_name[29];          // +0x04
    uint8_t fatigue_check;         // +0x21
    uint8_t reserved[2];           // +0x22
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x8237932 and
// Stream::GetInBuffer<...> @0x823AFF0. Dispatcher_GatheringPartyStatus
// fills this full record; DB_GatheringParyStatus consumes both subrecords.
struct SIG_PARTY_STATUS {
    struct PartyResult {
        uint8_t party_result;      // +0x00 from the entry
        uint8_t peer_nat_type;     // +0x01 from the entry
        uint8_t peer_ip[4];        // +0x02 from the entry
        uint32_t established_time; // +0x06 from the entry
    };

    struct PartyPing {
        uint8_t nat_type;          // +0x00 from the entry
        uint32_t ping;             // +0x01 from the entry
        uint32_t mtu;              // +0x05 from the entry
        uint8_t ip[4];             // +0x09 from the entry
    };

    char source_ip[16];            // +0x000
    uint32_t member_id;            // +0x010
    uint32_t nat_type;             // +0x014
    uint32_t map_id;               // +0x018
    uint32_t dungeon_id;           // +0x01C
    uint8_t end_reason;            // +0x020
    uint8_t is_connected_to_relay; // +0x021
    uint32_t connected_time_to_relay; // +0x022, intentionally unaligned
    uint16_t try_count;            // +0x026
    PartyResult results[31];       // +0x028; input count is bounded to 30
    uint16_t party_count;          // +0x15E
    PartyPing pings[8];            // +0x160
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x82378D8 and
// Stream::GetInBuffer<...> @0x823AF1C.  DB_ForbidUserToPlay reads the
// duration from the unaligned +0x05 field.
struct SIG_FORBID_USER_TO_PLAY {
    uint32_t charac_no;            // +0x00
    uint8_t forbid;                // +0x04
    int32_t duration_days;         // +0x05
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x82B2CA2 and
// Stream::GetInBuffer<...> @0x82BC896.  CGameManager writes all three words.
struct SIG_UPDATE_LINK_CHARAC_CONNECT_STATE {
    uint32_t first;                // +0x00
    uint32_t second;               // +0x04
    uint32_t third;                // +0x08
};

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x8450256 and
// Stream::GetInBuffer<...> @0x8454B1C.  EventCommon emits up to five rows.
struct SIG_LOAD_EVENTITEM {
    struct EventItem {
        uint32_t id;               // +0x00 from the entry
        uint32_t item_id;          // +0x04 from the entry
        uint32_t stack_count;      // +0x08 from the entry
        uint32_t event_code;       // +0x0C from the entry
    };

    uint32_t member_id;            // +0x00
    uint32_t charac_no;            // +0x04
    uint32_t item_count;           // +0x08
    EventItem items[5];            // +0x0C
};

#pragma pack(pop)

// IDA_EA: CStreamGuard::GetInBuffer<...> @0x82370A0 and
// Stream::GetInBuffer<...> @0x823A2F2.  The original aggregate has two
// trailing alignment bytes after the two one-byte slot values.
struct SIG_EXTEND_CHARAC_SLOT {
    uint32_t account_id;           // +0x00
    uint8_t slot_effect_count;     // +0x04
    uint8_t charac_slot_limit;     // +0x05
};

static_assert(sizeof(SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE) == 0x80u,
              "SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE must stay 128 bytes");
static_assert(offsetof(SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE, entries) == 0x08u,
              "SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE entries must stay at +0x08");
static_assert(sizeof(SIG_SAVE_CLIENT_HACK_LOG) == 0x12u,
              "SIG_SAVE_CLIENT_HACK_LOG must stay 18 bytes");
static_assert(offsetof(SIG_SAVE_CLIENT_HACK_LOG, etc) == 0x0Au,
              "SIG_SAVE_CLIENT_HACK_LOG etc must stay at +0x0A");
static_assert(sizeof(STBossDungeonTowerLog) == 0x10u,
              "STBossDungeonTowerLog must stay 16 bytes");
static_assert(sizeof(SIG_FIND_FACTORY_HUB) == 0x24u,
              "SIG_FIND_FACTORY_HUB must stay 36 bytes");
static_assert(offsetof(SIG_FIND_FACTORY_HUB, fatigue_check) == 0x21u,
              "SIG_FIND_FACTORY_HUB fatigue_check must stay at +0x21");
static_assert(sizeof(SIG_PARTY_STATUS::PartyResult) == 0x0Au,
              "SIG_PARTY_STATUS result record must stay 10 bytes");
static_assert(sizeof(SIG_PARTY_STATUS::PartyPing) == 0x0Du,
              "SIG_PARTY_STATUS ping record must stay 13 bytes");
static_assert(offsetof(SIG_PARTY_STATUS, results) == 0x28u,
              "SIG_PARTY_STATUS results must begin at +0x28");
static_assert(offsetof(SIG_PARTY_STATUS, party_count) == 0x15Eu,
              "SIG_PARTY_STATUS party_count must stay at +0x15E");
static_assert(offsetof(SIG_PARTY_STATUS, pings) == 0x160u,
              "SIG_PARTY_STATUS pings must begin at +0x160");
static_assert(sizeof(SIG_PARTY_STATUS) == 0x1C8u,
              "SIG_PARTY_STATUS must stay 456 bytes");
static_assert(sizeof(SIG_FORBID_USER_TO_PLAY) == 0x09u,
              "SIG_FORBID_USER_TO_PLAY must stay 9 bytes");
static_assert(offsetof(SIG_FORBID_USER_TO_PLAY, charac_no) == 0x00u &&
                  offsetof(SIG_FORBID_USER_TO_PLAY, forbid) == 0x04u &&
                  offsetof(SIG_FORBID_USER_TO_PLAY, duration_days) == 0x05u,
              "SIG_FORBID_USER_TO_PLAY duration_days must stay at +0x05");
static_assert(sizeof(SIG_UPDATE_LINK_CHARAC_CONNECT_STATE) == 0x0Cu,
              "SIG_UPDATE_LINK_CHARAC_CONNECT_STATE must stay 12 bytes");
static_assert(sizeof(SIG_LOAD_EVENTITEM) == 0x5Cu,
              "SIG_LOAD_EVENTITEM must stay 92 bytes");
static_assert(offsetof(SIG_LOAD_EVENTITEM, items) == 0x0Cu,
              "SIG_LOAD_EVENTITEM items must stay at +0x0C");
static_assert(sizeof(SIG_EXTEND_CHARAC_SLOT) == 0x08u,
              "SIG_EXTEND_CHARAC_SLOT must stay 8 bytes");
static_assert(offsetof(SIG_EXTEND_CHARAC_SLOT, account_id) == 0x00u &&
                  offsetof(SIG_EXTEND_CHARAC_SLOT, slot_effect_count) ==
                      0x04u &&
                  offsetof(SIG_EXTEND_CHARAC_SLOT, charac_slot_limit) ==
                      0x05u,
              "SIG_EXTEND_CHARAC_SLOT slot_effect_count must stay at +0x04");

using ::SIG_MAILBOX_LIST;
using ::SIG_AUTO_MARKET_CONDITIONS_CONTROL;
using ::SIG_AUTO_MARKET_CONDITIONS_CONTROL_DAILY;
using ::SIG_CREATURE_ITEM;
using ::SIG_CREATURE_ITEM_LIST;
using ::SIG_LOGIN_DATA;
using ::SIG_SWAP_CREATURE_ITEM;
using ::SIG_UPDATE_AUTO_MARKET_CONDITIONS_CONTROL;
using ::stCeraShopIPGRequest_t;

#pragma pack(push, 1)
// IDA_EA: stMonsterInfo_t::stMonsterInfo_t @0x848FC78 and
// SIG_LOAD_KILL_MONSTER_INFO::SIG_LOAD_KILL_MONSTER_INFO @0x848FC8E.
struct stMonsterInfo_t {
    uint32_t value;                 // +0x00
    uint8_t flag;                   // +0x04

    stMonsterInfo_t();
};

// IDA_EA: Stream::GetOutBuffer<SIG_LOAD_KILL_MONSTER_INFO> @0x8453D76,
// Stream::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO> @0x82399AE, and
// CStreamGuard::GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO> @0x8236DB4.
struct SIG_LOAD_KILL_MONSTER_INFO {
    stMonsterInfo_t boss_info[1500];      // +0x0000
    stMonsterInfo_t named_info[6000];     // +0x1D4C
    stMonsterInfo_t apc_boss_info[1500];  // +0x927C
    uint32_t charac_no;              // +0xAFC8

    SIG_LOAD_KILL_MONSTER_INFO();
};
#pragma pack(pop)

static_assert(sizeof(stMonsterInfo_t) == 0x05u,
              "stMonsterInfo_t IDA ABI size must stay 0x05");
static_assert(sizeof(SIG_LOAD_KILL_MONSTER_INFO) == 0xAFCCu,
              "SIG_LOAD_KILL_MONSTER_INFO IDA ABI size must stay 0xAFCC");
static_assert(sizeof(((SIG_LOAD_KILL_MONSTER_INFO*)0)->boss_info) == 0x1D4Cu,
              "kill-monster boss array size must stay 0x1D4C");
static_assert(sizeof(((SIG_LOAD_KILL_MONSTER_INFO*)0)->named_info) == 0x7530u,
              "kill-monster named array size must stay 0x7530");
static_assert(offsetof(SIG_LOAD_KILL_MONSTER_INFO, named_info) == 0x1D4Cu,
              "SIG_LOAD_KILL_MONSTER_INFO named_info offset must stay 0x1D4C");
static_assert(offsetof(SIG_LOAD_KILL_MONSTER_INFO, apc_boss_info) == 0x927Cu,
              "SIG_LOAD_KILL_MONSTER_INFO apc_boss_info offset must stay 0x927C");
static_assert(offsetof(SIG_LOAD_KILL_MONSTER_INFO, charac_no) == 0xAFC8u,
              "SIG_LOAD_KILL_MONSTER_INFO charac_no offset must stay 0xAFC8");

// IDA_EA: CONTINUOUS_ITEM_INFO::CONTINUOUS_ITEM_INFO @0x848FA0E.
struct CONTINUOUS_ITEM_INFO {
    uint8_t count;                  // +0x00
    uint8_t raw_01[3];              // +0x01
    struct Entry {
        uint32_t item_id;           // +0x00
        uint32_t remain_time;       // +0x04
    } entries[50];                  // +0x04

    CONTINUOUS_ITEM_INFO();
};

// IDA_EA: SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO::SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO
// @0x8491ECA, Stream::GetOutBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>
// @0x8452D66, Stream::GetInBuffer<SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO>
// @0x86A039C, and CStreamGuard::GetInBuffer<...> @0x869AEA4.
struct SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO {
    uint32_t charac_no;             // +0x00
    CONTINUOUS_ITEM_INFO item_info; // +0x04

    SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO();
};

static_assert(sizeof(CONTINUOUS_ITEM_INFO) == 0x194u,
              "CONTINUOUS_ITEM_INFO IDA ABI size must stay 0x194");
static_assert(sizeof(CONTINUOUS_ITEM_INFO::Entry) == 0x08u,
              "CONTINUOUS_ITEM_INFO::Entry IDA ABI size must stay 0x08");
static_assert(offsetof(CONTINUOUS_ITEM_INFO, entries) == 0x04u,
              "CONTINUOUS_ITEM_INFO entries offset must stay 0x04");
static_assert(sizeof(SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO) == 0x198u,
              "SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO IDA ABI size must stay 0x198");
static_assert(offsetof(SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO, item_info) ==
                  0x04u,
              "SIG_SAVE_AND_LOAD_CONTINUOUS_ITEM_INFO item_info offset must stay 0x04");

// IDA_EA: Stream core object @0x861BC08..0x861C8B0.
// The first fields follow the old 32-bit layout after the vptr:
//   +0x04 size, +0x08 buffer, +0x0C in, +0x10 out, +0x14 big-stream flag,
//   +0x18 acquire file, +0x1C acquire line, +0x20 pool kind, +0x24 pool-live.
class Stream {
public:
    Stream();
    explicit Stream(int size);
    explicit Stream(char* src);
    Stream(Stream& rhs);
    virtual ~Stream();

    int size() const;
    int length() const;
    int in_ptr() const;
    int in_ptr(int bytes);
    int out_ptr() const;
    int out_ptr(int bytes);
    Stream* reset();
    Stream* set_big_stream(bool enabled);
    int is_big_stream() const;
    int resize(int size);
    bool enable_in(int bytes) const;
    bool enable_out(int bytes) const;

    Stream& operator<<(bool value);
    Stream& operator<<(char value);
    Stream& operator<<(short value);
    Stream& operator<<(int value);
    Stream& operator<<(long value);
    Stream& operator<<(char* value);
    Stream& operator<<(const char* value);
    Stream& operator<<(unsigned char value);
    Stream& operator<<(unsigned short value);
    Stream& operator<<(unsigned int value);
    Stream& operator<<(unsigned long value);
    int put_binary(void* src, int bytes);
    void* GetInBufferUntyped(int bytes);

    template <typename T>
    T* GetOutBuffer();
    template <typename T>
    T* GetInBuffer();

    int operator>>(bool& value);
    int operator>>(char& value);
    int operator>>(short& value);
    int operator>>(int& value);
    int operator>>(long& value);
    int operator>>(char* value);
    int operator>>(unsigned char& value);
    int operator>>(unsigned short& value);
    int operator>>(unsigned int& value);
    int operator>>(unsigned long& value);
    int get_binary(void* dest, int bytes);
    int str_len(char* src) const;
    char* get_buffer();

    void set_acquire_site(const char* file, int line);
    void set_pool_kind(int kind);
    int pool_kind() const;
    void set_pool_live(bool live);
    bool pool_live() const;

protected:
    int size_;
    char* buffer_;
    int in_ptr_;
    int out_ptr_;
    bool big_stream_;
    char align_after_big_[3];
    const char* acquire_file_;
    int acquire_line_;
    int pool_kind_;
    bool pool_live_;
    char align_after_live_[3];
    uint32_t pool_dummy_;
    uint8_t pool_ref_count_;
};

class SmallStream : public Stream {
public:
    SmallStream();
    virtual ~SmallStream();
};

class BigStream : public Stream {
public:
    BigStream();
    virtual ~BigStream();
};


#endif  // OLDSRV_PROTOCOL_STREAM_H_
