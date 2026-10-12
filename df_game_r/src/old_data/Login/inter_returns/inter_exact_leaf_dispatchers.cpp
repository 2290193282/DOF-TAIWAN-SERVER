#include "old_data/Login/inter_returns/inter_exact_leaf_dispatchers.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stddef.h>
#include <stdint.h>

#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "common/manager/ipg_runtime.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_game/economy/trade/trade_limit_item_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/packet_dispatch/guild_cargo_history_dispatcher.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/account/db_find_factory_hub_user.h"
#include "old_data/db/character/db_load_no_cache_data.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/progression/career/character_expert_job.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

enum ENUM_ERROR : int {};

namespace {

const uint8_t kGuildCargoSuccess = 0xc1u;

#pragma pack(push, 1)

// The Inter wire prefix is owned by the legacy dispatcher.  Its fields are
// intentionally unnamed because none are consumed by this receiver.
struct InterUserInfoMinimumForGuildPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
};

struct InterItemLimitEditionSellStartPayload {
    uint8_t dispatcher_prefix[10];
    uint8_t reset_existing;
    uint32_t item_count;
    stItemLimitEditionItemInfo_t items[1];
};

struct InterItemLimitEditionSellEndPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t item_count;
    uint32_t ipg_no[1];
};

struct InterItemLimitEditionUpdateRow {
    uint32_t ipg_no;
    uint32_t current_sell_count;
    uint8_t raw_08;
};

struct InterItemLimitEditionUpdatePayload {
    uint8_t dispatcher_prefix[14];
    uint32_t item_count;
    InterItemLimitEditionUpdateRow items[1];
};

struct InterItemLimitEditionReplyOption {
    uint32_t item_id;
    uint8_t selected;
};

struct InterItemLimitEditionReplyAdditionalItem {
    uint32_t item_id;
    uint16_t first_option;
    uint16_t second_option;
};

struct InterItemLimitEditionReplyPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t ipg_no;
    uint32_t current_sell_count;
    uint8_t result_code;
    uint8_t select_type;
    uint8_t option_count;
    InterItemLimitEditionReplyOption options[35];
    uint8_t additional_item_count;
    InterItemLimitEditionReplyAdditionalItem additional_items[35];
};

// IDA receiver 0x84DC97A consumes this return record at byte zero, unlike
// the other Inter records in this file.
struct InterFindFactoryHubPayload {
    uint32_t account_id;
    uint8_t fatigue_check;
    char charac_name[29];
    uint8_t reserved[2];
};

struct PacketMonitorFindFactoryHubUser : PacketHeader {
    PacketMonitorFindFactoryHubUser()
        : PacketHeader(0x100eu, 0x0033u),
          charac_no(0),
          charac_name_length(0),
          etc_code(0),
          account_id(0) {
        std::memset(charac_name, 0, sizeof(charac_name));
    }

    uint32_t charac_no;
    uint8_t charac_name_length;
    char charac_name[30];
    uint16_t etc_code;
    uint32_t account_id;
};

// Inter_ApplyTempPowerSide @0x84DCCDE consumes only the two documented
// values in this legacy reply.  The intervening bytes remain source-owned.
struct InterApplyTempPowerSidePayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t raw_12_15[4];
    uint8_t power_side;
};

// Unlike the usual Inter replies, the Power War data stream begins at byte
// zero; the old receiver reads its two values and then the winner byte.
struct InterLoadPowerWarInfoPayload {
    int32_t power_a;
    int32_t power_b;
    uint8_t winner_side;
};

struct InterSetFactoryHubUserPayload {
    uint8_t dispatcher_prefix[15];
    char charac_name[29];
    uint8_t raw_2c;
    uint8_t should_add_hack;
    uint16_t hack_type;
    int32_t etc_value;
};

struct InterSetFactoryHubUserNotConnPayload {
    // IDA 0x84DD0C4 passes this raw dword as CHackAnalyzer::this.  The
    // ReqDBSaveHackInfo body serializes that receiver value as account_id;
    // it is not the connected user's embedded CHackAnalyzer instance.
    WongWork::CHackAnalyzer* account_id_as_hack_analyzer_this;
    uint32_t charac_no;
    uint16_t hack_type;
};

struct InterSetCleanPadPointPayload {
    uint8_t dispatcher_prefix[14];
    uint16_t point;
};

// The guild service returns the opaque 53-byte cargo slots verbatim.  Only
// the dword at +1 is inspected by the original receiver before the entire
// record is serialized back to the client.
struct GuildCargoEntry {
    uint8_t raw_00;
    uint32_t item_key;
    uint8_t raw_05[48];
};

struct InterGuildCargoPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    GuildCargoEntry entries[120];
    int32_t capacity;
    uint8_t result;
};

// Guild-cargo history records are likewise forwarded without field decoding;
// only their first dword is a presence predicate in the old loop.
struct GuildCargoHistoryEntry {
    uint32_t entry_key;
    uint8_t raw_04[44];
};

struct InterGuildCargoHistoryPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    GuildCargoHistoryEntry entries[50];
};

// CMD251's final reply carries the complete packed DnfItemInfo layout.  Its
// first byte is seal/state; CItem::make_item supplies the local Inven_Item
// state, so the original receiver does not consume that byte.
struct InterGuildCargoPushItemWire {
    uint8_t seal_or_state;
    uint32_t item_id;
    uint8_t item_attr;
    uint32_t add_info;
    uint16_t avatar_select;
    uint32_t extend_info;
    uint8_t amplify_type;
    uint16_t amplify_value;
    uint8_t globally_unique_id[10];
    uint8_t random_option[14];
    uint8_t upgrade_separate;
    uint8_t raw_2c_34[9];
};

// CMD252 does not return DnfItemInfo's initial seal/state byte.  Its item
// record starts directly with the item id, hence this is one byte shorter
// than the CMD251 return record.
struct InterGuildCargoPopItemWire {
    uint32_t item_id;
    uint8_t item_attr;
    uint32_t add_info;
    uint16_t avatar_select;
    uint32_t extend_info;
    uint8_t amplify_type;
    uint16_t amplify_value;
    uint8_t globally_unique_id[10];
    uint8_t random_option[14];
    uint8_t upgrade_separate;
    uint8_t raw_2b_33[9];
};

// CMD251's guild-service reply.  The receiver consumes every field which
// participated in the original inventory deletion and 0x70d reconstruction.
struct InterGuildCargoCheckPushItemPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t result;
    uint32_t item_count;
    uint32_t item_id;
    uint32_t add_info;
    uint32_t stackable_limit;
    uint8_t is_stackable;
    uint8_t item_space;
    uint16_t slot;
};

// CMD251's final guild-service reply.  The 14 random-option bytes are
// directly copied to Inven_Item::chronicle_random_option_.random_option_.
struct InterGuildCargoPushItemPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t result;
    uint32_t cargo_slot;
    InterGuildCargoPushItemWire item;
    uint8_t reply_item_space;
    uint16_t reply_slot;
};

// CMD252's guild-service reply.  Unlike the push return, the terminal
// result and cargo slot are at +0x4b/+0x4c, while the byte returned to the
// client is the low byte at +0x12.
struct InterGuildCargoPopItemPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t reply_item_count;
    uint8_t raw_13_16[4];
    InterGuildCargoPopItemWire item;
    uint8_t result;
    uint32_t cargo_slot;
};

struct InterGuildCargoMoveItemPayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t result;
    uint32_t from_slot;
    uint32_t to_slot;
};

struct InterGuildCargoUpgradePayload {
    uint8_t dispatcher_prefix[14];
    uint32_t charac_no;
    uint8_t result;
};

#pragma pack(pop)

static_assert(offsetof(InterUserInfoMinimumForGuildPayload, charac_no) == 14u,
              "Inter guild-user-info character offset must remain +0x0e");
static_assert(offsetof(InterItemLimitEditionSellStartPayload, item_count) == 11u &&
                  offsetof(InterItemLimitEditionSellStartPayload, items) == 15u,
              "Inter limit-edition sell-start payload offsets changed");
static_assert(offsetof(InterItemLimitEditionSellEndPayload, item_count) == 14u &&
                  offsetof(InterItemLimitEditionSellEndPayload, ipg_no) == 18u,
              "Inter limit-edition sell-end payload offsets changed");
static_assert(sizeof(InterItemLimitEditionUpdateRow) == 9u &&
                  offsetof(InterItemLimitEditionUpdatePayload, items) == 18u,
              "Inter limit-edition update payload offsets changed");
static_assert(offsetof(InterItemLimitEditionReplyPayload, ipg_no) == 14u &&
                  offsetof(InterItemLimitEditionReplyPayload, result_code) == 22u &&
                  offsetof(InterItemLimitEditionReplyPayload, options) == 25u &&
                  offsetof(InterItemLimitEditionReplyPayload,
                           additional_item_count) == 200u &&
                  offsetof(InterItemLimitEditionReplyPayload,
                           additional_items) == 201u &&
                  sizeof(InterItemLimitEditionReplyPayload) == 0x1e1u,
              "Inter limit-edition reply payload ABI changed");
static_assert(sizeof(InterFindFactoryHubPayload) == 0x24u,
              "Inter factory-hub payload ABI must stay 0x24");
static_assert(sizeof(PacketMonitorFindFactoryHubUser) == 0x33u,
              "factory-hub monitor packet ABI must stay 0x33");
static_assert(offsetof(InterApplyTempPowerSidePayload, charac_no) == 0x0eu &&
                  offsetof(InterApplyTempPowerSidePayload, power_side) ==
                      0x16u,
              "temporary power-side Inter payload offsets changed");
static_assert(offsetof(InterLoadPowerWarInfoPayload, winner_side) == 0x08u,
              "Power War Inter payload winner offset changed");
static_assert(offsetof(InterSetFactoryHubUserPayload, charac_name) == 0x0fu &&
                  offsetof(InterSetFactoryHubUserPayload, should_add_hack) ==
                      0x2du &&
                  offsetof(InterSetFactoryHubUserPayload, hack_type) == 0x2eu &&
                  offsetof(InterSetFactoryHubUserPayload, etc_value) == 0x30u,
              "factory-hub user Inter payload offsets changed");
static_assert(sizeof(InterSetFactoryHubUserNotConnPayload) == 0x0au &&
                  offsetof(InterSetFactoryHubUserNotConnPayload,
                           account_id_as_hack_analyzer_this) == 0x00u &&
                  offsetof(InterSetFactoryHubUserNotConnPayload, hack_type) ==
                      0x08u,
              "factory-hub disconnected Inter payload offsets changed");
static_assert(offsetof(InterSetCleanPadPointPayload, point) == 0x0eu,
              "clean-pad point Inter payload offset changed");
static_assert(sizeof(GuildCargoEntry) == 53u &&
                  offsetof(GuildCargoEntry, item_key) == 1u &&
                  offsetof(InterGuildCargoPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoPayload, entries) == 18u &&
                  offsetof(InterGuildCargoPayload, capacity) == 6378u &&
                  offsetof(InterGuildCargoPayload, result) == 6382u,
              "guild-cargo Inter reply payload offsets changed");
static_assert(sizeof(GuildCargoHistoryEntry) == 48u &&
                  offsetof(InterGuildCargoHistoryPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoHistoryPayload, entries) == 18u,
              "guild-cargo history Inter reply payload offsets changed");
static_assert(sizeof(InterGuildCargoPushItemWire) == 0x35u &&
                  offsetof(InterGuildCargoPushItemWire, item_id) == 0x01u &&
                  offsetof(InterGuildCargoPushItemWire, item_attr) == 0x05u &&
                  offsetof(InterGuildCargoPushItemWire, add_info) == 0x06u &&
                  offsetof(InterGuildCargoPushItemWire, avatar_select) ==
                      0x0au &&
                  offsetof(InterGuildCargoPushItemWire, extend_info) ==
                      0x0cu &&
                  offsetof(InterGuildCargoPushItemWire, amplify_type) ==
                      0x10u &&
                  offsetof(InterGuildCargoPushItemWire, amplify_value) ==
                      0x11u &&
                  offsetof(InterGuildCargoPushItemWire, globally_unique_id) ==
                      0x13u &&
                  offsetof(InterGuildCargoPushItemWire, random_option) ==
                      0x1du &&
                  offsetof(InterGuildCargoPushItemWire, upgrade_separate) ==
                      0x2bu,
              "guild-cargo push-item reply wire ABI changed");
static_assert(sizeof(InterGuildCargoPopItemWire) == 0x34u &&
                  offsetof(InterGuildCargoPopItemWire, item_id) == 0x00u &&
                  offsetof(InterGuildCargoPopItemWire, item_attr) == 0x04u &&
                  offsetof(InterGuildCargoPopItemWire, add_info) == 0x05u &&
                  offsetof(InterGuildCargoPopItemWire, avatar_select) ==
                      0x09u &&
                  offsetof(InterGuildCargoPopItemWire, extend_info) ==
                      0x0bu &&
                  offsetof(InterGuildCargoPopItemWire, amplify_type) ==
                      0x0fu &&
                  offsetof(InterGuildCargoPopItemWire, amplify_value) ==
                      0x10u &&
                  offsetof(InterGuildCargoPopItemWire, globally_unique_id) ==
                      0x12u &&
                  offsetof(InterGuildCargoPopItemWire, random_option) ==
                      0x1cu &&
                  offsetof(InterGuildCargoPopItemWire, upgrade_separate) ==
                      0x2au,
              "guild-cargo pop-item reply wire ABI changed");
static_assert(offsetof(InterGuildCargoCheckPushItemPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, result) == 18u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, item_count) == 19u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, item_id) == 23u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, add_info) == 27u &&
                  offsetof(InterGuildCargoCheckPushItemPayload,
                           stackable_limit) == 31u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, is_stackable) ==
                      35u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, item_space) ==
                      36u &&
                  offsetof(InterGuildCargoCheckPushItemPayload, slot) == 37u,
              "guild-cargo check-push Inter payload offsets changed");
static_assert(offsetof(InterGuildCargoPushItemPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoPushItemPayload, result) == 18u &&
                  offsetof(InterGuildCargoPushItemPayload, cargo_slot) == 19u &&
                  offsetof(InterGuildCargoPushItemPayload, item) == 23u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, item_id) == 24u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, item_attr) == 28u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, add_info) == 29u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, avatar_select) ==
                      33u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, extend_info) == 35u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, amplify_type) == 39u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, amplify_value) ==
                      40u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, random_option) ==
                      52u &&
                  offsetof(InterGuildCargoPushItemPayload, item) +
                          offsetof(InterGuildCargoPushItemWire, upgrade_separate) ==
                      66u &&
                  offsetof(InterGuildCargoPushItemPayload, reply_item_space) ==
                      76u &&
                  offsetof(InterGuildCargoPushItemPayload, reply_slot) == 77u,
              "guild-cargo push Inter payload offsets changed");
static_assert(offsetof(InterGuildCargoPopItemPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoPopItemPayload, reply_item_count) ==
                      18u &&
                  offsetof(InterGuildCargoPopItemPayload, item) == 23u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, item_id) == 23u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, item_attr) == 27u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, add_info) == 28u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, avatar_select) ==
                      32u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, extend_info) == 34u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, amplify_type) == 38u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, amplify_value) ==
                      39u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, random_option) ==
                      51u &&
                  offsetof(InterGuildCargoPopItemPayload, item) +
                          offsetof(InterGuildCargoPopItemWire, upgrade_separate) ==
                      65u &&
                  offsetof(InterGuildCargoPopItemPayload, result) == 75u &&
                  offsetof(InterGuildCargoPopItemPayload, cargo_slot) == 76u,
              "guild-cargo pop Inter payload offsets changed");
static_assert(offsetof(InterGuildCargoMoveItemPayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoMoveItemPayload, result) == 18u &&
                  offsetof(InterGuildCargoMoveItemPayload, from_slot) == 19u &&
                  offsetof(InterGuildCargoMoveItemPayload, to_slot) == 23u,
              "guild-cargo move Inter reply payload offsets changed");
static_assert(offsetof(InterGuildCargoUpgradePayload, charac_no) == 14u &&
                  offsetof(InterGuildCargoUpgradePayload, result) == 18u,
              "guild-cargo upgrade Inter reply payload offsets changed");

void BroadcastItemLimitEditionInfo(CItemLimitEditionMgr* manager) {
    PacketGuard packet;
    packet.put_header(0, 225);
    packet.put_byte(0);
    manager->makeItemLimitEditionInfo(packet);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

void BroadcastItemLimitEditionRemoveInfo(CItemLimitEditionMgr* manager) {
    PacketGuard packet;
    packet.put_header(0, 225);
    packet.put_byte(1);
    manager->makeItemLimitEditionRemoveInfo(packet);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

void ThrowInterItemLimitEditionError(int error) {
    throw static_cast<ENUM_ERROR>(error);
}

void MakeItemLimitEditionDescription(const CItemLimitEdition* edition,
                                     const CItem* item,
                                     char* description,
                                     size_t description_size) {
    if (edition->getRelateditem() == 1) {
        // IDA's format__12 is the same Continue Coin label used by the
        // CeraShop item-string provider for item index 1.
        std::snprintf(description, description_size, "Continue Coin %d",
                      edition->getRelatedItemCnt());
    } else if (item->is_stackable() && edition->getRelatedItemCnt() > 1) {
        std::snprintf(description, description_size, "%s Count %d",
                      item->GetItemName(), edition->getRelatedItemCnt());
    } else {
        std::strcpy(description, item->GetItemName());
    }
}

}  // namespace

// IDA_EA: 0x84C7DEE
// IDA_NAME: _ZN15Inter_Terminate12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Terminate::dispatch_sig(CUser *,char *,int)
int Inter_Terminate::dispatch_sig(CUser* /*user*/,
                                  char* /*payload*/,
                                  int /*payload_size*/) {
    std::puts("Game Server is Terminated");
    std::exit(0);
}

// IDA_EA: 0x84EA144 / 0x84EA160 / 0x84EA190
// IDA_NAME: _ZN15Inter_TerminateC2Ev / D2Ev / D0Ev
Inter_Terminate::Inter_Terminate() : ISigDispatcher() {}
Inter_Terminate::~Inter_Terminate() {}

// IDA_EA: 0x84CF546
// IDA_NAME: _ZN25Inter_FatigueResetDBReply12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_FatigueResetDBReply::dispatch_sig(CUser *,char *,int)
int Inter_FatigueResetDBReply::dispatch_sig(CUser* /*user*/,
                                            char* /*payload*/,
                                            int /*payload_size*/) {
    // Original body is exactly `xor eax,eax; ret`.
    return 0;
}

// IDA_EA: 0x84EB46A / 0x84EB486 / 0x84EB4B6
// IDA_NAME: _ZN25Inter_FatigueResetDBReplyC2Ev / D2Ev / D0Ev
Inter_FatigueResetDBReply::Inter_FatigueResetDBReply() : ISigDispatcher() {}
Inter_FatigueResetDBReply::~Inter_FatigueResetDBReply() {}

// IDA_EA: 0x84CF550
// IDA_NAME: _ZN29Inter_SuicideWhenQueueIsEmpty12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SuicideWhenQueueIsEmpty::dispatch_sig(CUser *,char *,int)
int Inter_SuicideWhenQueueIsEmpty::dispatch_sig(CUser* /*user*/,
                                                 char* /*payload*/,
                                                 int /*payload_size*/) {
    // Original body is exactly `xor eax,eax; ret`.
    return 0;
}

// IDA_EA: 0x84ED508 / 0x84F9E40 / 0x84F9E70
// IDA_NAME: _ZN29Inter_SuicideWhenQueueIsEmptyC2Ev / D2Ev / D0Ev
Inter_SuicideWhenQueueIsEmpty::Inter_SuicideWhenQueueIsEmpty()
    : ISigDispatcher() {}
Inter_SuicideWhenQueueIsEmpty::~Inter_SuicideWhenQueueIsEmpty() {}

// IDA_EA: 0x84CE5C4
// IDA_NAME: _ZN15Inter_UserRepel12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UserRepel::dispatch_sig(CUser *,char *,int)
int Inter_UserRepel::dispatch_sig(CUser* user,
                                  char* /*payload*/,
                                  int /*payload_size*/) {
    user->DisConnSig(static_cast<DISCONN_SIG>(20), true, 0);
    return 0;
}

// IDA_EA: 0x84EB096 / 0x84EB0B2 / 0x84EB0E2
// IDA_NAME: _ZN15Inter_UserRepelC2Ev / D2Ev / D0Ev
Inter_UserRepel::Inter_UserRepel() : ISigDispatcher() {}
Inter_UserRepel::~Inter_UserRepel() {}

// IDA_EA: 0x84D28C0
// IDA_NAME: _ZN30Inter_UDP_Incoming_Capcha_Data12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UDP_Incoming_Capcha_Data::dispatch_sig(CUser *,char *,int)
int Inter_UDP_Incoming_Capcha_Data::dispatch_sig(CUser* /*user*/,
                                                  char* /*payload*/,
                                                  int /*payload_size*/) {
    // Original body is exactly `xor eax,eax; ret`.
    return 0;
}

// IDA_EA: 0x84ED914
// IDA_NAME: _ZN30Inter_UDP_Incoming_Capcha_DataC2Ev
// DEMANGLED: Inter_UDP_Incoming_Capcha_Data::Inter_UDP_Incoming_Capcha_Data(void)
Inter_UDP_Incoming_Capcha_Data::Inter_UDP_Incoming_Capcha_Data()
    : ISigDispatcher() {}

// IDA_EA: 0x84F9480
// IDA_NAME: _ZN30Inter_UDP_Incoming_Capcha_DataD2Ev
// DEMANGLED: Inter_UDP_Incoming_Capcha_Data::~Inter_UDP_Incoming_Capcha_Data()
// IDA_EA: 0x84F94B0
// IDA_NAME: _ZN30Inter_UDP_Incoming_Capcha_DataD0Ev
// DEMANGLED: Inter_UDP_Incoming_Capcha_Data::~Inter_UDP_Incoming_Capcha_Data()
Inter_UDP_Incoming_Capcha_Data::~Inter_UDP_Incoming_Capcha_Data() {}

// IDA_EA: 0x84DB200
// IDA_NAME: _ZN19Inter_LoadExpertJob12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadExpertJob::dispatch_sig(CUser *,char *,int)
int Inter_LoadExpertJob::dispatch_sig(CUser* user,
                                      char* payload,
                                      int /*payload_size*/) {
    if (user->get_state() <= 1) {
        return 0;
    }
    if (user->getCurCharacR()) {
        expert_job::CCharacExpertJob* const expert_job_data =
            static_cast<expert_job::CCharacExpertJob*>(
                user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(0)));
        expert_job_data->loadData(user, payload);
        expert_job_data->SendExpertJobInfo(user);
    }
    return 0;
}

// IDA_EA: 0x84EE1D4 / 0x84F7D58 / 0x84F7D88
// IDA_NAME: _ZN19Inter_LoadExpertJobC2Ev / D2Ev / D0Ev
Inter_LoadExpertJob::Inter_LoadExpertJob() : ISigDispatcher() {}
Inter_LoadExpertJob::~Inter_LoadExpertJob() {}

// IDA_EA: 0x84D533A
// IDA_NAME: _ZN41Inter_SendAllUserInfoMiniumForGuildSystem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SendAllUserInfoMiniumForGuildSystem::dispatch_sig(CUser *,char *,int)
int Inter_SendAllUserInfoMiniumForGuildSystem::dispatch_sig(
    CUser* user,
    char* payload,
    int /*payload_size*/) {
    const InterUserInfoMinimumForGuildPayload& input =
        *reinterpret_cast<const InterUserInfoMinimumForGuildPayload*>(payload);
    if (user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        PacketGuard packet;
        packet.clear();
        packet.put_header(0, 2);
        packet.put_byte(0);
        packet.put_short(1);
        user->make_basic_info(packet, 0);
        packet.finalize(true);
        G_GameWorld()->send_all(packet, user);
    } else {
        LogManager::logFormat(
            1,
            "InterDispatcher.cpp",
            "virtual int Inter_SendAllUserInfoMiniumForGuildSystem::dispatch_sig(CUser*, char*, int)",
            0x36e3,
            "Inter_SendAllUserInfoMiniumForGuildSystem::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharId(%d)",
            user->get_charac_no(-1),
            static_cast<int>(input.charac_no));
    }
    return 0;
}

// IDA_EA: 0x84EDCCC / 0x84F8B5C / 0x84F8B8C
// IDA_NAME: _ZN41Inter_SendAllUserInfoMiniumForGuildSystemC2Ev / D2Ev / D0Ev
Inter_SendAllUserInfoMiniumForGuildSystem::
    Inter_SendAllUserInfoMiniumForGuildSystem()
    : ISigDispatcher() {}
Inter_SendAllUserInfoMiniumForGuildSystem::
    ~Inter_SendAllUserInfoMiniumForGuildSystem() {}

// IDA_EA: 0x84DB878
// IDA_NAME: _ZN33Inter_ItemLimitEdition_Sell_Start12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ItemLimitEdition_Sell_Start::dispatch_sig(CUser *,char *,int)
int Inter_ItemLimitEdition_Sell_Start::dispatch_sig(CUser* /*user*/,
                                                     char* payload,
                                                     int /*payload_size*/) {
    const InterItemLimitEditionSellStartPayload& input =
        *reinterpret_cast<const InterItemLimitEditionSellStartPayload*>(payload);
    CItemLimitEditionMgr* const manager =
        G_CDataManager()->getItemLimitEditionMgr();

    if (input.reset_existing) {
        if (!manager->empty()) {
            BroadcastItemLimitEditionRemoveInfo(manager);
        }
        manager->clear();
    }

    for (uint32_t index = 0; index < input.item_count; ++index) {
        const stItemLimitEditionItemInfo_t& item_info = input.items[index];
        CItem* const related_item = G_CDataManager()->find_item(
            static_cast<int>(item_info.related_item));
        if (related_item) {
            if (related_item->vtable_->is_avatar_item(related_item)) {
                continue;
            }
        } else if (item_info.related_item != 1u) {
            continue;
        }
        manager->registItem(item_info);
    }

    if (input.item_count) {
        BroadcastItemLimitEditionInfo(manager);
    }
    return 0;
}

// IDA_EA: 0x84EE228 / 0x84F7C6E / 0x84F7C9E
// IDA_NAME: _ZN33Inter_ItemLimitEdition_Sell_StartC2Ev / D2Ev / D0Ev
Inter_ItemLimitEdition_Sell_Start::Inter_ItemLimitEdition_Sell_Start()
    : ISigDispatcher() {}
Inter_ItemLimitEdition_Sell_Start::~Inter_ItemLimitEdition_Sell_Start() {}

// IDA_EA: 0x84DBBB2
// IDA_NAME: _ZN31Inter_ItemLimitEdition_Sell_End12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ItemLimitEdition_Sell_End::dispatch_sig(CUser *,char *,int)
int Inter_ItemLimitEdition_Sell_End::dispatch_sig(CUser* /*user*/,
                                                   char* payload,
                                                   int /*payload_size*/) {
    const InterItemLimitEditionSellEndPayload& input =
        *reinterpret_cast<const InterItemLimitEditionSellEndPayload*>(payload);
    PacketGuard packet;
    packet.put_header(0, 225);
    packet.put_byte(1);
    packet.put_byte(static_cast<int>(input.item_count));
    for (uint32_t index = 0; index < input.item_count; ++index) {
        const uint32_t ipg_no = input.ipg_no[index];
        G_CDataManager()->getItemLimitEditionMgr()->removeItem(ipg_no);
        packet.put_int(static_cast<int>(ipg_no));
    }
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84EE244 / 0x84F7C20 / 0x84F7C50
// IDA_NAME: _ZN31Inter_ItemLimitEdition_Sell_EndC2Ev / D2Ev / D0Ev
Inter_ItemLimitEdition_Sell_End::Inter_ItemLimitEdition_Sell_End()
    : ISigDispatcher() {}
Inter_ItemLimitEdition_Sell_End::~Inter_ItemLimitEdition_Sell_End() {}

// IDA_EA: 0x84DBD16
// IDA_NAME: _ZN29Inter_ItemLimitEdition_Update12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ItemLimitEdition_Update::dispatch_sig(CUser *,char *,int)
int Inter_ItemLimitEdition_Update::dispatch_sig(CUser* /*user*/,
                                                 char* payload,
                                                 int /*payload_size*/) {
    const InterItemLimitEditionUpdatePayload& input =
        *reinterpret_cast<const InterItemLimitEditionUpdatePayload*>(payload);
    PacketGuard packet;
    packet.put_header(0, 225);
    packet.put_byte(0);
    int packet_count_index = packet.get_index();
    int changed_count = 0;
    packet.put_byte(0);

    CItemLimitEditionMgr* const manager =
        G_CDataManager()->getItemLimitEditionMgr();
    for (uint32_t index = 0; index < input.item_count; ++index) {
        const InterItemLimitEditionUpdateRow& item = input.items[index];
        CItemLimitEdition* const edition = manager->getItemInfo(item.ipg_no);
        if (edition && !edition->isSellComplete() &&
            manager->updateItem(item.ipg_no, item.current_sell_count) == 1) {
            edition->makeInfoPacket(packet);
            ++changed_count;
        }
    }
    if (changed_count) {
        packet.put_byte(packet_count_index, changed_count);
        packet.finalize(true);
        G_GameWorld()->send_all(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE260 / 0x84F7BD2 / 0x84F7C02
// IDA_NAME: _ZN29Inter_ItemLimitEdition_UpdateC2Ev / D2Ev / D0Ev
Inter_ItemLimitEdition_Update::Inter_ItemLimitEdition_Update()
    : ISigDispatcher() {}
Inter_ItemLimitEdition_Update::~Inter_ItemLimitEdition_Update() {}

// IDA_EA: 0x84DBEF0
// IDA_NAME: _ZN28Inter_ItemLimitEdition_Reply12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ItemLimitEdition_Reply::dispatch_sig(CUser *,char *,int)
int Inter_ItemLimitEdition_Reply::dispatch_sig(CUser* user,
                                                char* payload,
                                                int /*payload_size*/) {
    if (user->get_state() > 2) {
        const InterItemLimitEditionReplyPayload& input =
            *reinterpret_cast<const InterItemLimitEditionReplyPayload*>(payload);
        CItemLimitEditionMgr* const manager =
            G_CDataManager()->getItemLimitEditionMgr();
        CItemLimitEdition* edition = 0;
        CItem* item = 0;

        if (manager->updateItem(input.ipg_no, input.current_sell_count) != 1) {
            ThrowInterItemLimitEditionError(22);
        }
        if (input.result_code == 1) {
            ThrowInterItemLimitEditionError(22);
        }
        if (input.result_code == 2) {
            ThrowInterItemLimitEditionError(95);
        }
        if (input.result_code) {
            ThrowInterItemLimitEditionError(1);
        }

        edition = manager->getItemInfo(input.ipg_no);
        if (!edition) {
            ThrowInterItemLimitEditionError(17);
        }
        if (edition->isSellComplete()) {
            PacketGuard packet;
            packet.put_header(0, 225);
            packet.put_byte(0);
            packet.put_byte(1);
            edition->makeInfoPacket(packet);
            packet.finalize(true);
            G_GameWorld()->send_all(packet);
        }

        if (user->GetCera() < edition->getPriceCera()) {
            ThrowInterItemLimitEditionError(11);
        }
        if (user->getCurCharacInvenR()->get_money() < edition->getPriceGold()) {
            ThrowInterItemLimitEditionError(10);
        }
        if (user->GetWinPoint() < edition->getPriceWinPoint()) {
            ThrowInterItemLimitEditionError(12);
        }

        if (edition->getRelateditem() != 1) {
            item = G_CDataManager()->find_item(edition->getRelateditem());
            if (!item) {
                ThrowInterItemLimitEditionError(17);
            }
        }

        int special_type = 0;
        int special_value = 0;
        bool special_unique = false;
        int featured_index = edition->getFeaturedIdx();
        if (!featured_index &&
            GlobalData::s_pCeraShop->IsSpecialItem(
                edition->getRelateditem(), special_type, special_value,
                special_unique)) {
            featured_index = G_CDataManager()->get_item_list()->GetRestrictCode(
                edition->getRelateditem());
        }

        user->resetUsedCera();
        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("InterDispatcher.cpp", 18137),
            false);
        stCeraShopIPGRequest_t* const request =
            guard.GetInBuffer<stCeraShopIPGRequest_t>();
        request->goods_no = input.ipg_no;
        request->group_index = 10;
        request->cera_price = edition->getPriceCera();
        request->gold_price = edition->getPriceGold();
        request->win_point = edition->getPriceWinPoint();
        request->item_index = static_cast<uint32_t>(edition->getRelateditem());
        request->stack_count =
            static_cast<uint16_t>(edition->getRelatedItemCnt());
        request->select_type = input.select_type;
        request->avatar_select =
            static_cast<uint8_t>(edition->getAvatarTypeSelect());
        request->avatar_selection_value = 0;
        request->featured_index = static_cast<uint32_t>(featured_index);
        request->item_count = 1;
        request->item_order = 1;
        request->current_seconds = GlobalData::s_systemTime_.getCurSec();
        request->gift = 0;
        // These names describe the common CeraShop request layout.  This
        // Inter receiver intentionally writes its original character/account
        // dwords into the same two byte ranges at +0x38/+0x40.
        request->post_recipient_wire_word =
            static_cast<uint32_t>(user->getCurCharacNo());
        const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
        std::memcpy(request->recipient_name_suffix, &account_id,
                    sizeof(account_id));
        request->option_count = input.option_count;
        for (unsigned int option = 0; option < input.option_count; ++option) {
            request->options[option].item_id = input.options[option].item_id;
            request->options[option].selected = input.options[option].selected;
        }
        request->additional_item_count = input.additional_item_count;
        for (unsigned int additional = 0;
             additional < input.additional_item_count;
             ++additional) {
            request->additional_items[additional].item_id =
                input.additional_items[additional].item_id;
            request->additional_items[additional].first_option =
                input.additional_items[additional].first_option;
            request->additional_items[additional].second_option =
                input.additional_items[additional].second_option;
        }

        char price[36];
        char order_id[8];
        char description[100];
        char unique_no[21];
        std::memset(price, 0, sizeof(price));
        std::memset(unique_no, 0, sizeof(unique_no));
        std::snprintf(price, sizeof(price), "%d", edition->getPriceCera());
        std::snprintf(order_id, sizeof(order_id), "%d", input.ipg_no);
        MakeItemLimitEditionDescription(edition, item, description,
                                        sizeof(description));
        WongWork::CGenUniqueNo::GenUniqueNo(unique_no);

        if (edition->getPriceCera()) {
            GlobalData::s_pIPGHelper->IPGOutput(
                user, order_id, price, unique_no, description,
                user->getCurCharacName(), guard.Get());
        } else {
            GlobalData::s_pIPGHelper->IPGOutputSimulation(
                user, order_id, price, unique_no, description,
                user->getCurCharacName(), guard.Get());
        }

        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM);
        packet.put_byte(1);
        packet.put_int(edition->getIPGNO());
        packet.put_int(edition->getCurSellNum());
        packet.put_int(edition->getTotalSellNum());
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE27C / 0x84F7B84 / 0x84F7BB4
// IDA_NAME: _ZN28Inter_ItemLimitEdition_ReplyC2Ev / D2Ev / D0Ev
Inter_ItemLimitEdition_Reply::Inter_ItemLimitEdition_Reply()
    : ISigDispatcher() {}
Inter_ItemLimitEdition_Reply::~Inter_ItemLimitEdition_Reply() {}

// IDA_EA: 0x84DC97A
// IDA_NAME: _ZN20Inter_FindFactoryHub12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_FindFactoryHub::dispatch_sig(CUser *,char *,int)
int Inter_FindFactoryHub::dispatch_sig(CUser* user,
                                       char* payload,
                                       int /*payload_size*/) {
    const InterFindFactoryHubPayload& input =
        *reinterpret_cast<const InterFindFactoryHubPayload*>(payload);
    if (user) {
        const unsigned int prior_704 = static_cast<unsigned int>(
            user->SetETC(704, static_cast<int>(input.account_id)));
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x2c0u), 1,
            prior_704, 0);
        if (input.fatigue_check) {
            const unsigned int prior_701 = static_cast<unsigned int>(
                user->SetETC(701, static_cast<int>(input.account_id)));
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<WongWork::ENUM_HACKTYPE>(0x2bdu), 1,
                prior_701, 0);
        }

        PacketMonitorFindFactoryHubUser packet;
        packet.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
        packet.charac_name_length =
            static_cast<uint8_t>(std::strlen(input.charac_name));
        std::strncpy(packet.charac_name, input.charac_name,
                     sizeof(packet.charac_name));
        packet.etc_code = 705;
        packet.account_id = static_cast<uint32_t>(user->get_acc_id());
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))->SendPacket(
                reinterpret_cast<char*>(&packet), packet.size);
    }
    return 0;
}

// IDA_EA: 0x84EE298 / 0x84F7B36 / 0x84F7B66
// IDA_NAME: _ZN20Inter_FindFactoryHubC2Ev / D2Ev / D0Ev
Inter_FindFactoryHub::Inter_FindFactoryHub() : ISigDispatcher() {}
Inter_FindFactoryHub::~Inter_FindFactoryHub() {}

// IDA_EA: 0x84DCAEE
// IDA_NAME: _ZN19Inter_GuildExpLimit12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildExpLimit::dispatch_sig(CUser *,char *,int)
int Inter_GuildExpLimit::dispatch_sig(CUser* /*user*/,
                                      char* /*payload*/,
                                      int /*payload_size*/) {
    GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(0))->SendGuildExpLimit();
    return 0;
}

// IDA_EA: 0x84EE2B4 / 0x84F7AE8 / 0x84F7B18
// IDA_NAME: _ZN19Inter_GuildExpLimitC2Ev / D2Ev / D0Ev
Inter_GuildExpLimit::Inter_GuildExpLimit() : ISigDispatcher() {}
Inter_GuildExpLimit::~Inter_GuildExpLimit() {}

// IDA_EA: 0x84DCCB8
// IDA_NAME: _ZN23Inter_CancelRestingUser12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CancelRestingUser::dispatch_sig(CUser *,char *,int)
int Inter_CancelRestingUser::dispatch_sig(CUser* user,
                                          char* /*payload*/,
                                          int /*payload_size*/) {
    if (user) {
        user->setRestingUserRestrict(false);
    }
    return 0;
}

// IDA_EA: 0x84EE308 / 0x84F79FE / 0x84F7A2E
// IDA_NAME: _ZN23Inter_CancelRestingUserC2Ev / D2Ev / D0Ev
Inter_CancelRestingUser::Inter_CancelRestingUser() : ISigDispatcher() {}
Inter_CancelRestingUser::~Inter_CancelRestingUser() {}

// IDA_EA: 0x84DCCDE
// IDA_NAME: _ZN24Inter_ApplyTempPowerSide12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ApplyTempPowerSide::dispatch_sig(CUser *,char *,int)
int Inter_ApplyTempPowerSide::dispatch_sig(CUser* user,
                                           char* payload,
                                           int /*payload_size*/) {
    const InterApplyTempPowerSidePayload& input =
        *reinterpret_cast<const InterApplyTempPowerSidePayload*>(payload);
    if (static_cast<int>(input.charac_no) == user->get_charac_no(-1)) {
        STGuildDBInfoOnly* const guild_info = user->GetGuildDBInfo();
        STGuildProxy proxy(guild_info->guild_name,
                           guild_info->guild_level,
                           input.power_side);
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->ModifyGuildInfoProxy(
                static_cast<unsigned int>(user->get_charac_guildkey()),
                proxy);

        PacketGuard broadcast;
        broadcast.put_header(0, 2);
        broadcast.put_byte(0);
        broadcast.put_short(1);
        user->make_basic_info(broadcast, 0);
        broadcast.finalize(true);
        G_GameWorld()->send_all(broadcast, user);

        PacketGuard notify;
        notify.put_header(1, ENUM_CMDPACKET_JOIN_POWER_WAR);
        notify.put_byte(1);
        notify.put_byte(1);
        notify.finalize(true);
        user->Send(notify);
    }
    return 0;
}

// IDA_EA: 0x84EE324 / 0x84F79B0 / 0x84F79E0
// IDA_NAME: _ZN24Inter_ApplyTempPowerSideC2Ev / D2Ev / D0Ev
Inter_ApplyTempPowerSide::Inter_ApplyTempPowerSide() : ISigDispatcher() {}
Inter_ApplyTempPowerSide::~Inter_ApplyTempPowerSide() {}

// IDA_EA: 0x84DCEEC
// IDA_NAME: _ZN26Inter_ReloadPowerWarRanker12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReloadPowerWarRanker::dispatch_sig(CUser *,char *,int)
int Inter_ReloadPowerWarRanker::dispatch_sig(CUser* /*user*/,
                                             char* /*payload*/,
                                             int /*payload_size*/) {
    cMyTrace trace(
        "virtual int Inter_ReloadPowerWarRanker::dispatch_sig(CUser*, char*, int)",
        18475, 0);
    trace("Receive Packet_Request_Reload_Power_War_Ranker");
    GlobalData::s_power_manager->ClearMVPInfo();
    GlobalData::s_power_manager->LoadRankerInfo();
    return 0;
}

// IDA_EA: 0x84EE340 / 0x84F7962 / 0x84F7992
// IDA_NAME: _ZN26Inter_ReloadPowerWarRankerC2Ev / D2Ev / D0Ev
Inter_ReloadPowerWarRanker::Inter_ReloadPowerWarRanker()
    : ISigDispatcher() {}
Inter_ReloadPowerWarRanker::~Inter_ReloadPowerWarRanker() {}

// IDA_EA: 0x84DCF4A
// IDA_NAME: _ZN22Inter_LoadPowerWarInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadPowerWarInfo::dispatch_sig(CUser *,char *,int)
int Inter_LoadPowerWarInfo::dispatch_sig(CUser* /*user*/,
                                         char* payload,
                                         int /*payload_size*/) {
    const InterLoadPowerWarInfoPayload& input =
        *reinterpret_cast<const InterLoadPowerWarInfoPayload*>(payload);
    cMyTrace trace(
        "virtual int Inter_LoadPowerWarInfo::dispatch_sig(CUser*, char*, int)",
        18490, 0);
    trace("Receive SIG_LOAD_POWER_WAR_INFO(Winner:%d, A:%d, B:%d)",
          input.winner_side, input.power_a, input.power_b);
    GlobalData::s_power_manager->SetPowerInfo(
        static_cast<char>(input.winner_side), input.power_a, input.power_b);
    return 0;
}

// IDA_EA: 0x84EE35C / 0x84F7914 / 0x84F7944
// IDA_NAME: _ZN22Inter_LoadPowerWarInfoC2Ev / D2Ev / D0Ev
Inter_LoadPowerWarInfo::Inter_LoadPowerWarInfo() : ISigDispatcher() {}
Inter_LoadPowerWarInfo::~Inter_LoadPowerWarInfo() {}

// IDA_EA: 0x84DCFEC
// IDA_NAME: _ZN23Inter_SetFactoryHubUser12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SetFactoryHubUser::dispatch_sig(CUser *,char *,int)
int Inter_SetFactoryHubUser::dispatch_sig(CUser* user,
                                          char* payload,
                                          int /*payload_size*/) {
    if (user) {
        const InterSetFactoryHubUserPayload& input =
            *reinterpret_cast<const InterSetFactoryHubUserPayload*>(payload);
        if (input.should_add_hack) {
            const unsigned int prior = static_cast<unsigned int>(user->SetETC(
                static_cast<short>(input.hack_type), input.etc_value));
            user->getHackAnalyzer()->addServerHackCnt(
                user, static_cast<WongWork::ENUM_HACKTYPE>(input.hack_type),
                1, prior, 0);
        } else {
            char charac_name[29];
            std::strncpy(charac_name, input.charac_name, sizeof(charac_name));
            DB_Find_Factory_Hub_User::makeRequest(
                static_cast<unsigned int>(user->GetUID()), charac_name,
                input.hack_type);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE378 / 0x84F78C6 / 0x84F78F6
// IDA_NAME: _ZN23Inter_SetFactoryHubUserC2Ev / D2Ev / D0Ev
Inter_SetFactoryHubUser::Inter_SetFactoryHubUser() : ISigDispatcher() {}
Inter_SetFactoryHubUser::~Inter_SetFactoryHubUser() {}

// IDA_EA: 0x84DD0C4
// IDA_NAME: _ZN30Inter_SetFactoryHubUserNotConn12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SetFactoryHubUserNotConn::dispatch_sig(CUser *,char *,int)
int Inter_SetFactoryHubUserNotConn::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int /*payload_size*/) {
    if (user) {
        const InterSetFactoryHubUserNotConnPayload& input =
            *reinterpret_cast<const InterSetFactoryHubUserNotConnPayload*>(
                payload);
        static_cast<void>(user->getHackAnalyzer());
        const int account_id = user->get_acc_id();
        input.account_id_as_hack_analyzer_this->ReqDBSaveHackInfo(
            input.charac_no, input.hack_type, account_id, 1);
    }
    return 0;
}

// IDA_EA: 0x84EE394 / 0x84F7878 / 0x84F78A8
// IDA_NAME: _ZN30Inter_SetFactoryHubUserNotConnC2Ev / D2Ev / D0Ev
Inter_SetFactoryHubUserNotConn::Inter_SetFactoryHubUserNotConn()
    : ISigDispatcher() {}
Inter_SetFactoryHubUserNotConn::~Inter_SetFactoryHubUserNotConn() {}

// IDA_EA: 0x84DD12E
// IDA_NAME: _ZN24Inter_Set_CleanPad_Point12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_Set_CleanPad_Point::dispatch_sig(CUser *,char *,int)
int Inter_Set_CleanPad_Point::dispatch_sig(CUser* user,
                                           char* payload,
                                           int /*payload_size*/) {
    if (user) {
        const InterSetCleanPadPointPayload& input =
            *reinterpret_cast<const InterSetCleanPadPointPayload*>(payload);
        user->getMCAPManagerW().setPoint(input.point);
        const unsigned int actual_point = user->getMCAPManagerW().getPoint();
        cMyTrace trace(
            "virtual int Inter_Set_CleanPad_Point::dispatch_sig(CUser*, char*, int)",
            18566, 0);
        trace("CLEANPAD_POINT_CONTROL / m_id : %s , Point : %d (getPorint : %d)",
              NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
              input.point, actual_point);
    }
    return 0;
}

// IDA_EA: 0x84EE3B0 / 0x84F782A / 0x84F785A
// IDA_NAME: _ZN24Inter_Set_CleanPad_PointC2Ev / D2Ev / D0Ev
Inter_Set_CleanPad_Point::Inter_Set_CleanPad_Point() : ISigDispatcher() {}
Inter_Set_CleanPad_Point::~Inter_Set_CleanPad_Point() {}

// IDA_EA: 0x84DD1EE
// IDA_NAME: _ZN21Inter_LoadNoCacheData12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadNoCacheData::dispatch_sig(CUser *,char *,int)
int Inter_LoadNoCacheData::dispatch_sig(CUser* user,
                                        char* payload,
                                        int /*payload_size*/) {
    const SIG_LOAD_NO_CACHE_DATA& input =
        *reinterpret_cast<const SIG_LOAD_NO_CACHE_DATA*>(payload);
    if (user && user->getCurCharacR() &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CCharacBloodDungeon* const data =
            static_cast<CCharacBloodDungeon*>(user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
        data->loadData(user, payload + 4);
    }
    return 0;
}

// IDA_EA: 0x84EE3CC / 0x84F77DC / 0x84F780C
// IDA_NAME: _ZN21Inter_LoadNoCacheDataC2Ev / D2Ev / D0Ev
Inter_LoadNoCacheData::Inter_LoadNoCacheData() : ISigDispatcher() {}
Inter_LoadNoCacheData::~Inter_LoadNoCacheData() {}

// IDA_EA: 0x84DD26C
// IDA_NAME: _ZN16Inter_GuildCargo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargo::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargo::dispatch_sig(CUser* user,
                                   char* payload,
                                   int /*payload_size*/) {
    const InterGuildCargoPayload& input =
        *reinterpret_cast<const InterGuildCargoPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CUser_history_log(user)->GuildCargoResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(1), input.result,
            input.capacity);
        if (input.result == kGuildCargoSuccess) {
            user->SetGuildCargoCapacity(static_cast<unsigned int>(input.capacity));
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_GUILD_CARGO);
            packet.put_byte(1);
            packet.put_int(input.capacity);
            int count_index = packet.get_index();
            int count = 0;
            packet.put_short(count);
            for (int index = 0; index <= 119; ++index) {
                const GuildCargoEntry& entry = input.entries[index];
                if (entry.item_key) {
                    packet.put_short(index);
                    packet.put_binary(reinterpret_cast<const char*>(&entry), 53);
                    ++count;
                }
            }
            packet.put_short(count_index, count);
            packet.finalize(true);
            user->Send(packet);
        } else {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO,
                                     input.result);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE3E8 / 0x84F778E / 0x84F77BE
// IDA_NAME: _ZN16Inter_GuildCargoC2Ev / D2Ev / D0Ev
Inter_GuildCargo::Inter_GuildCargo() : ISigDispatcher() {}
Inter_GuildCargo::~Inter_GuildCargo() {}

// IDA_EA: 0x84DD4B2
// IDA_NAME: _ZN22Inter_NoticeGuildCargo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeGuildCargo::dispatch_sig(CUser *,char *,int)
int Inter_NoticeGuildCargo::dispatch_sig(CUser* user,
                                         char* payload,
                                         int /*payload_size*/) {
    const InterGuildCargoPayload& input =
        *reinterpret_cast<const InterGuildCargoPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CUser_history_log(user)->GuildCargoResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(2), 0,
            input.capacity);
        user->SetGuildCargoCapacity(static_cast<unsigned int>(input.capacity));
        PacketGuard packet;
        packet.put_header(0, 237);
        packet.put_int(input.capacity);
        int count_index = packet.get_index();
        int count = 0;
        packet.put_short(count);
        for (int index = 0; index <= 119; ++index) {
            const GuildCargoEntry& entry = input.entries[index];
            if (entry.item_key) {
                packet.put_short(index);
                packet.put_binary(reinterpret_cast<const char*>(&entry), 53);
                ++count;
            }
        }
        packet.put_short(count_index, count);
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE404 / 0x84F7740 / 0x84F7770
// IDA_NAME: _ZN22Inter_NoticeGuildCargoC2Ev / D2Ev / D0Ev
Inter_NoticeGuildCargo::Inter_NoticeGuildCargo() : ISigDispatcher() {}
Inter_NoticeGuildCargo::~Inter_NoticeGuildCargo() {}

// IDA_EA: 0x84DD69E
// IDA_NAME: _ZN23Inter_GuildCargoHistory12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoHistory::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoHistory::dispatch_sig(CUser* user,
                                          char* payload,
                                          int /*payload_size*/) {
    const InterGuildCargoHistoryPayload& input =
        *reinterpret_cast<const InterGuildCargoHistoryPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_GUILD_CARGO_HISTORY);
        packet.put_byte(1);
        int count_index = packet.get_index();
        int count = 0;
        packet.put_int(count);
        for (int index = 0; index <= 49; ++index) {
            const GuildCargoHistoryEntry& entry = input.entries[index];
            if (entry.entry_key) {
                packet.put_binary(reinterpret_cast<const char*>(&entry), 48);
                ++count;
            }
        }
        packet.put_int(count_index, count);
        packet.finalize(true);
        user->Send(packet);
        CUser_history_log(user)->GuildCargoHistoryResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(4), count);
    }
    return 0;
}

// IDA_EA: 0x84EE420 / 0x84F76F2 / 0x84F7722
// IDA_NAME: _ZN23Inter_GuildCargoHistoryC2Ev / D2Ev / D0Ev
Inter_GuildCargoHistory::Inter_GuildCargoHistory() : ISigDispatcher() {}
Inter_GuildCargoHistory::~Inter_GuildCargoHistory() {}

// IDA_EA: 0x84DD856
// IDA_NAME: _ZN29Inter_GuildCargoCheckPushItem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoCheckPushItem::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoCheckPushItem::dispatch_sig(CUser* user,
                                                char* payload,
                                                int /*payload_size*/) {
    const InterGuildCargoCheckPushItemPayload& input =
        *reinterpret_cast<const InterGuildCargoCheckPushItemPayload*>(payload);
    if (user->get_state() <= 2 ||
        user->get_charac_no(-1) != static_cast<int>(input.charac_no)) {
        return 0;
    }

    if (input.result != kGuildCargoSuccess) {
        CUser_history_log(user)->GuildCargoCheckPushItemResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(6), input.result,
            static_cast<int>(input.item_count), static_cast<int>(input.item_id),
            static_cast<int>(input.add_info), input.item_space, input.slot);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, input.result);
        return 0;
    }

    CInventory* const inventory_r = user->getCurCharacInvenR();
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(input.item_space));
    Inven_Item inventory_item =
        inventory_r->GetInvenSlot(inventory_type, input.slot);
    if (!input.item_id ||
        inventory_item.item_id_ != static_cast<int32_t>(input.item_id)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, 197);
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(inventory_item.item_id_);
    if (!item || !item->isPackagable() || item->GetCashPrice() ||
        item->vtable_->is_impossible_charac_cargo(item) ||
        IsTradeLimitAttachTypeItem(inventory_item)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, 197);
        return 0;
    }

    if (item->getUsablePeriod() || item->getExpirationDate()) {
        const int expiration_sec =
            86400 * static_cast<int>(inventory_item.avatar_select_) + 1151683200;
        if (expiration_sec <= GlobalData::s_systemTime_.getCurSec()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, 197);
            return 0;
        }
    }

    if (item->is_stackable() &&
        inventory_item.add_info_ < static_cast<int32_t>(input.add_info)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, 198);
        return 0;
    }

    const int attach_type = item->GetAttachType();
    if (attach_type == 1 || attach_type == 2 || attach_type == 5 ||
        (attach_type == 3 && inventory_item.seal_or_state_ != 1) ||
        inventory_item.upgrade_separate_.IsTradeRestriction() ||
        user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(inventory_item.GetInventoryType()),
            input.slot, static_cast<int>(input.add_info),
            static_cast<eItemDelReason>(34), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM, 197);
        return 0;
    }

    user->SaveInventory();

    Packet_Guild_Guild_Cargo_Push_Item service_request;
    service_request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    service_request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    service_request.item_count = input.item_count;
    service_request.stackable_limit = input.stackable_limit;
    service_request.is_stackable = input.is_stackable;

    const uint8_t item_attr =
        static_cast<uint8_t>(inventory_item.GetItemAttr());
    // DnfItemInfo has a seal/state byte at +0x00.  The original C2 leaves
    // it reset and writes the local Inven_Item attribute at wire +0x05.
    service_request.item.item_attr_ = item_attr;
    service_request.item.item_id_ = inventory_item.item_id_;

    uint32_t transported_add_info =
        static_cast<uint32_t>(inventory_item.add_info_);
    if (item->is_stackable()) transported_add_info = input.add_info;
    service_request.item.add_info_ = transported_add_info;
    service_request.item.avatar_select_ = inventory_item.avatar_select_;
    service_request.item.extend_info_ = inventory_item.extend_info_;

    uint8_t amplify_type = 0;
    uint16_t amplify_value = 0;
    inventory_item.amplify_option_.getValues(amplify_type, amplify_value);
    service_request.item.amplify_type_ = amplify_type;
    service_request.item.amplify_value_ = amplify_value;
    service_request.item.globally_unique_id_ =
        inventory_item.globally_unique_id_;
    service_request.item.random_option_ =
        inventory_item.chronicle_random_option_.random_option_;
    service_request.item.upgrade_separate_.SetUpgradeSeparate(
        static_cast<uint8_t>(
            inventory_item.upgrade_separate_.GetUpgradeSeparate()));
    service_request.item_space = input.item_space;
    service_request.slot = input.slot;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&service_request),
                               service_request.size);
    CUser_history_log(user)->GuildCargoPushItem(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(7),
        static_cast<int>(input.item_count), inventory_item.item_id_,
        static_cast<int>(transported_add_info), inventory_item.avatar_select_,
        inventory_item.extend_info_, item_attr,
        static_cast<unsigned char>(
            service_request.item.upgrade_separate_.GetUpgradeSeparate()));
    return 0;
}

// IDA_EA: 0x84EE43C / 0x84F76A4 / 0x84F76D4
// IDA_NAME: _ZN29Inter_GuildCargoCheckPushItemC2Ev / D2Ev / D0Ev
Inter_GuildCargoCheckPushItem::Inter_GuildCargoCheckPushItem()
    : ISigDispatcher() {}
Inter_GuildCargoCheckPushItem::~Inter_GuildCargoCheckPushItem() {}

// IDA_EA: 0x84DDEA2
// IDA_NAME: _ZN24Inter_GuildCargoPushItem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoPushItem::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoPushItem::dispatch_sig(CUser* user,
                                           char* payload,
                                           int /*payload_size*/) {
    const InterGuildCargoPushItemPayload& input =
        *reinterpret_cast<const InterGuildCargoPushItemPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CUser_history_log(user)->GuildCargoPushItemResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(8), input.result,
            static_cast<int>(input.cargo_slot),
            static_cast<int>(input.item.item_id),
            static_cast<int>(input.item.add_info), input.item.avatar_select,
            static_cast<int>(input.item.extend_info));
        if (input.result == kGuildCargoSuccess) {
            PacketGuard packet;
            packet.put_header(1, ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM);
            packet.put_byte(1);
            packet.put_byte(input.reply_item_space);
            packet.put_short(input.reply_slot);
            packet.put_int(input.item.add_info);
            packet.put_short(static_cast<int>(input.cargo_slot));
            packet.finalize(true);
            user->Send(packet);
        } else {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM,
                                     input.result);
            CInventory* const inventory_w = user->getCurCharacInvenW();
            CItem* const item =
                G_CDataManager()->find_item(input.item.item_id);
            Inven_Item inventory_item;
            item->vtable_->make_item(item, inventory_item);
            inventory_item.item_id_ =
                static_cast<int32_t>(input.item.item_id);
            inventory_item.add_info_ =
                static_cast<int32_t>(input.item.add_info);
            inventory_item.avatar_select_ = input.item.avatar_select;
            inventory_item.extend_info_ =
                static_cast<int32_t>(input.item.extend_info);
            inventory_item.SetItemAttr(input.item.item_attr);
            inventory_item.amplify_option_.assign(input.item.amplify_type,
                                                  input.item.amplify_value);
            std::memcpy(
                &inventory_item.chronicle_random_option_.random_option_,
                input.item.random_option, sizeof(input.item.random_option));
            const unsigned char wire_upgrade_separate =
                reinterpret_cast<const UpgradeSeparateInfo*>(
                    &input.item.upgrade_separate)
                    ->GetUpgradeSeparate();
            inventory_item.upgrade_separate_.SetUpgradeSeparate(
                wire_upgrade_separate);
            if (inventory_w->insertItemIntoInventory(
                    inventory_item, static_cast<eItemAddReason>(39), true,
                    false) < 0) {
                const char* const title = g_scriptStringManager_.findString(
                    4, "game_server_msg_114", 0);
                WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                    title, inventory_item, 0, input.charac_no, 0, 0, 0,
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
                    false, false);
                CUser_history_log(user)->GuildCargoItemRecovery(
                    static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(9),
                    static_cast<int>(input.item.item_id),
                    static_cast<int>(input.item.add_info),
                    input.item.avatar_select,
                    static_cast<int>(input.item.extend_info),
                    input.item.item_attr, wire_upgrade_separate);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x84EE458 / 0x84F7656 / 0x84F7686
// IDA_NAME: _ZN24Inter_GuildCargoPushItemC2Ev / D2Ev / D0Ev
Inter_GuildCargoPushItem::Inter_GuildCargoPushItem() : ISigDispatcher() {}
Inter_GuildCargoPushItem::~Inter_GuildCargoPushItem() {}

// IDA_EA: 0x84DE330
// IDA_NAME: _ZN23Inter_GuildCargoPopItem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoPopItem::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoPopItem::dispatch_sig(CUser* user,
                                          char* payload,
                                          int /*payload_size*/) {
    const InterGuildCargoPopItemPayload& input =
        *reinterpret_cast<const InterGuildCargoPopItemPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CUser_history_log(user)->GuildCargoPopItemResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(11), input.result,
            static_cast<int>(input.cargo_slot),
            static_cast<int>(input.item.item_id),
            static_cast<int>(input.item.add_info), input.item.avatar_select,
            static_cast<int>(input.item.extend_info));
        if (input.result == kGuildCargoSuccess) {
            CInventory* const inventory_w = user->getCurCharacInvenW();
            CItem* const item =
                G_CDataManager()->find_item(input.item.item_id);
            Inven_Item inventory_item;
            if (item) {
                item->vtable_->make_item(item, inventory_item);
                inventory_item.item_id_ =
                    static_cast<int32_t>(input.item.item_id);
                inventory_item.add_info_ =
                    static_cast<int32_t>(input.item.add_info);
                inventory_item.avatar_select_ = input.item.avatar_select;
                inventory_item.extend_info_ =
                    static_cast<int32_t>(input.item.extend_info);
                inventory_item.SetItemAttr(input.item.item_attr);
                inventory_item.amplify_option_.assign(
                    input.item.amplify_type, input.item.amplify_value);
                std::memcpy(
                    &inventory_item.chronicle_random_option_.random_option_,
                    input.item.random_option,
                    sizeof(input.item.random_option));
                // EA 0x84DE330 performs the equivalent of
                // `HIBYTE(v22) = payload[+0x41]`: unlike CMD251, CMD252
                // restores the whole flags byte, including trade/restriction
                // bits, after CItem::make_item initialized the local item.
                inventory_item.upgrade_separate_
                    .SetUpgradeSeparateAndTradeRestriction(
                        input.item.upgrade_separate);

                int inserted = 0;
                if (item->vtable_->is_creature_item(item)) {
                    inserted = inventory_w->tryInsertItemIntoCreature(
                        inventory_item);
                } else if (item->vtable_->is_avatar_item(item)) {
                    inserted = inventory_w->tryInsertAvatarIntoInventory(
                        inventory_item);
                } else {
                    inserted = inventory_w->tryInsertItemIntoInventory(
                        inventory_item);
                }

                int insert_result = 0;
                if (inserted >= 0) {
                    if (item->vtable_->is_creature_item(item)) {
                        insert_result = inventory_w->insertItemIntoCreature(
                            inventory_item, static_cast<eItemAddReason>(40),
                            true, false);
                    } else if (item->vtable_->is_avatar_item(item)) {
                        insert_result = inventory_w->insertAvatarIntoInventory(
                            inventory_item,
                            static_cast<eAvatarItemAddReason>(18), true);
                    } else {
                        insert_result = inventory_w->insertItemIntoInventory(
                            inventory_item, static_cast<eItemAddReason>(40),
                            true, false);
                    }
                }

                if (inserted < 0 || insert_result < 0) {
                    const char* const title =
                        g_scriptStringManager_.findString(
                            4, "game_server_msg_114", 0);
                    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                        title, inventory_item, 0, input.charac_no, 0, 0, 0,
                        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
                        false, false);
                    const unsigned char wire_upgrade_separate =
                        reinterpret_cast<const UpgradeSeparateInfo*>(
                            &input.item.upgrade_separate)
                            ->GetUpgradeSeparate();
                    CUser_history_log(user)->GuildCargoItemRecovery(
                        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(12),
                        static_cast<int>(input.item.item_id),
                        static_cast<int>(input.item.add_info),
                        input.item.avatar_select,
                        static_cast<int>(input.item.extend_info),
                        input.item.item_attr, wire_upgrade_separate);
                }

                PacketGuard packet;
                packet.put_header(1, ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM);
                packet.put_byte(1);
                packet.put_short(static_cast<int>(input.cargo_slot));
                packet.put_byte(input.reply_item_count);
                packet.put_short(inserted);
                packet.put_int(input.item.add_info);
                packet.finalize(true);
                user->Send(packet);
            } else {
                const int charac_no = user->getCurCharacNo();
                cMyTrace trace(
                    "virtual int Inter_GuildCargoPopItem::dispatch_sig(CUser*, char*, int)",
                    19054, 0);
                trace(
                    "ERROR OCCURRED! GUILD CARGO POP ITEM - fail find item(%d), User(%d)",
                    static_cast<int>(input.item.item_id), charac_no);
                user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM,
                                         197);
            }
        } else {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM,
                                     input.result);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE474 / 0x84F7608 / 0x84F7638
// IDA_NAME: _ZN23Inter_GuildCargoPopItemC2Ev / D2Ev / D0Ev
Inter_GuildCargoPopItem::Inter_GuildCargoPopItem() : ISigDispatcher() {}
Inter_GuildCargoPopItem::~Inter_GuildCargoPopItem() {}

// IDA_EA: 0x84DEBB4
// IDA_NAME: _ZN24Inter_GuildCargoMoveItem12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoMoveItem::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoMoveItem::dispatch_sig(CUser* user,
                                           char* payload,
                                           int /*payload_size*/) {
    const InterGuildCargoMoveItemPayload& input =
        *reinterpret_cast<const InterGuildCargoMoveItemPayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        CUser_history_log(user)->GuildCargoMoveItemResult(
            static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(14), input.result,
            static_cast<int>(input.from_slot), static_cast<int>(input.to_slot));
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_GUILD_CARGO_MOVE_ITEM);
        if (input.result == kGuildCargoSuccess) {
            packet.put_byte(1);
        } else {
            packet.put_byte(0);
            packet.put_byte(input.result);
        }
        packet.put_short(static_cast<int>(input.from_slot));
        packet.put_short(static_cast<int>(input.to_slot));
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE490 / 0x84F75BA / 0x84F75EA
// IDA_NAME: _ZN24Inter_GuildCargoMoveItemC2Ev / D2Ev / D0Ev
Inter_GuildCargoMoveItem::Inter_GuildCargoMoveItem() : ISigDispatcher() {}
Inter_GuildCargoMoveItem::~Inter_GuildCargoMoveItem() {}

// IDA_EA: 0x84DED42
// IDA_NAME: _ZN23Inter_GuildCargoUpgrade12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildCargoUpgrade::dispatch_sig(CUser *,char *,int)
int Inter_GuildCargoUpgrade::dispatch_sig(CUser* user,
                                          char* payload,
                                          int /*payload_size*/) {
    const InterGuildCargoUpgradePayload& input =
        *reinterpret_cast<const InterGuildCargoUpgradePayload*>(payload);
    if (user->get_state() > 2 &&
        user->get_charac_no(-1) == static_cast<int>(input.charac_no)) {
        PacketGuard packet;
        packet.put_header(1, ENUM_CMDPACKET_UPGRADE_GUILD_CARGO);
        if (input.result == kGuildCargoSuccess) {
            packet.put_byte(1);
        } else {
            packet.put_byte(0);
            packet.put_byte(input.result);
        }
        packet.finalize(true);
        user->Send(packet);
    }
    return 0;
}

// IDA_EA: 0x84EE4AC / 0x84F756C / 0x84F759C
// IDA_NAME: _ZN23Inter_GuildCargoUpgradeC2Ev / D2Ev / D0Ev
Inter_GuildCargoUpgrade::Inter_GuildCargoUpgrade() : ISigDispatcher() {}
Inter_GuildCargoUpgrade::~Inter_GuildCargoUpgrade() {}
