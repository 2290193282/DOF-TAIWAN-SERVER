#include "old_game/activity/schedule/freely_give_item.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/checksum.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_freely_give_item.h"
#include "old_data/db/inventory/db_save_inven.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

using freely_give_item_db::ACTION_CLAIM_BOX;
using freely_give_item_db::ACTION_CLAIM_COUPON_LOCAL;
using freely_give_item_db::ACTION_CLAIM_COUPON_MAIL;
using freely_give_item_db::ACTION_CONVERT_UNLIMITED;
using freely_give_item_db::ACTION_NONE;
using freely_give_item_db::ACTION_RESET;
using freely_give_item_db::ACTION_SELECT_WEAPON;
using freely_give_item_db::ACTION_UPGRADE_REWARD;
using freely_give_item_db::ACTION_USE_COUPON;
using freely_give_item_db::Message;

enum LocalError {
    LOCAL_ERROR_NONE = 0,
    LOCAL_ERROR_GENERAL = 1,
    LOCAL_ERROR_INVENTORY = 4,
    LOCAL_ERROR_DIFFERENT_SERVER = 7,
    LOCAL_ERROR_FAILED = 16,
    LOCAL_ERROR_NO_RESET_WEAPON = 17,
    LOCAL_ERROR_ALREADY_CLAIMED = 18,
    LOCAL_ERROR_PENDING = 19,
    LOCAL_ERROR_CONDITION = 21,
    LOCAL_ERROR_NO_WEAPON_OR_MONEY = 22,
    LOCAL_ERROR_SCRIPT = 95,
    LOCAL_ERROR_EVENT_OR_THRESHOLD = 178,
};

struct RewardRow {
    uint32_t item[6];
};

struct FreelyGiveConfig {
    FreelyGiveConfig()
        : gift_box(0u), chicken_max(0u), coupon(0u), connect_seconds(0u),
          offline_seconds(0u), total_extension_days(0u),
          unlimited_threshold_days(0u), total_extension_reward(0u),
          base_extension_days(0u), normal_lower(0u), normal_upper(0u),
          separate_lower(0u), separate_upper(0u), each_time_unit(0u),
          fatigue_unit(0u), reset_money(0u), rows() {}

    uint32_t gift_box;
    uint8_t chicken_max;
    uint32_t coupon;
    uint32_t connect_seconds;
    uint32_t offline_seconds;
    uint8_t total_extension_days;
    uint8_t unlimited_threshold_days;
    uint32_t total_extension_reward;
    uint8_t base_extension_days;
    uint8_t normal_lower;
    uint8_t normal_upper;
    uint8_t separate_lower;
    uint8_t separate_upper;
    uint8_t each_time_unit;
    uint32_t fatigue_unit;
    uint32_t reset_money;
    std::vector<RewardRow> rows;

    const RewardRow* FindPeriodic(uint32_t item_id) const {
        for (std::vector<RewardRow>::const_iterator row = rows.begin();
             row != rows.end(); ++row) {
            if (row->item[0] == item_id) return &*row;
        }
        return 0;
    }

    const RewardRow* FindPermanent(uint32_t item_id) const {
        for (std::vector<RewardRow>::const_iterator row = rows.begin();
             row != rows.end(); ++row) {
            if (row->item[5] == item_id) return &*row;
        }
        return 0;
    }
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool ReadPositive(uint32_t& destination) {
    bool success = false;
    const int value = ScanInt(&success);
    if (!success || value <= 0) return false;
    destination = static_cast<uint32_t>(value);
    return true;
}

bool ReadByte(uint8_t& destination) {
    uint32_t value = 0u;
    if (!ReadPositive(value) || value > 255u) return false;
    destination = static_cast<uint8_t>(value);
    return true;
}

bool ReadRange(uint8_t& lower, uint8_t& upper) {
    return ReadByte(lower) && ReadByte(upper) && lower <= upper;
}

bool ReadRewardRows(std::vector<RewardRow>& rows) {
    bool is_type = false;
    bool success = false;
    std::string token;
    std::vector<uint32_t> values;
    for (;;) {
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            if (token != "[/freely give item event reward list]") {
                return false;
            }
            break;
        }
        if (!success || value <= 0 || values.size() >= 168u) return false;
        values.push_back(static_cast<uint32_t>(value));
    }
    if (values.size() != 168u) return false;
    for (unsigned int index = 0u; index < values.size(); index += 6u) {
        RewardRow row;
        for (unsigned int column = 0u; column < 6u; ++column) {
            row.item[column] = values[index + column];
        }
        rows.push_back(row);
    }
    return rows.size() == 28u;
}

bool IsEquipmentWeapon(uint32_t item_id) {
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    return prototype && prototype->vtable_ &&
           prototype->vtable_->get_item_type(prototype) ==
               oldsrv::config::kEquipmentTypeWeapon;
}

bool IsStackable(uint32_t item_id) {
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    return prototype && prototype->vtable_ && prototype->is_stackable() == 1;
}

class ConfigCache {
public:
    ConfigCache() : attempted_(false), valid_(false), config_() {}

    const FreelyGiveConfig* Get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = Load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool Load() {
        if (!loadRDARScriptFile("Event/", "freelygiveitemevent.evt")) {
            return false;
        }
        RdarUnloadGuard unload;
        FreelyGiveConfig parsed;
        uint32_t seen = 0u;
        bool valid = true;
        std::string tag;
        while (valid && ScanType(tag, true) == 1) {
            if (tag == "[gift box item]" && !(seen & (1u << 0))) {
                valid = ReadPositive(parsed.gift_box); seen |= 1u << 0;
            } else if (tag == "[chicken gauge max]" &&
                       !(seen & (1u << 1))) {
                valid = ReadByte(parsed.chicken_max); seen |= 1u << 1;
            } else if (tag == "[coupon item index]" &&
                       !(seen & (1u << 2))) {
                valid = ReadPositive(parsed.coupon); seen |= 1u << 2;
            } else if (tag == "[connect gauge fill time]" &&
                       !(seen & (1u << 3))) {
                valid = ReadPositive(parsed.connect_seconds); seen |= 1u << 3;
            } else if (tag == "[unconnect gauge fill time]" &&
                       !(seen & (1u << 4))) {
                valid = ReadPositive(parsed.offline_seconds); seen |= 1u << 4;
            } else if (tag == "[total extension day]" &&
                       !(seen & (1u << 5))) {
                valid = ReadByte(parsed.total_extension_days); seen |= 1u << 5;
            } else if (tag == "[change unlimit base day]" &&
                       !(seen & (1u << 6))) {
                valid = ReadByte(parsed.unlimited_threshold_days);
                seen |= 1u << 6;
            } else if (tag == "[total extension reward]" &&
                       !(seen & (1u << 7))) {
                valid = ReadPositive(parsed.total_extension_reward);
                seen |= 1u << 7;
            } else if (tag == "[base extension day]" &&
                       !(seen & (1u << 8))) {
                valid = ReadByte(parsed.base_extension_days); seen |= 1u << 8;
            } else if (tag == "[upgrade success lower upper]" &&
                       !(seen & (1u << 9))) {
                valid = ReadRange(parsed.normal_lower, parsed.normal_upper);
                seen |= 1u << 9;
            } else if (tag == "[upgrade disjoint lower upper]" &&
                       !(seen & (1u << 10))) {
                valid = ReadRange(parsed.separate_lower,
                                  parsed.separate_upper);
                seen |= 1u << 10;
            } else if (tag == "[each time unit]" &&
                       !(seen & (1u << 11))) {
                valid = ReadByte(parsed.each_time_unit); seen |= 1u << 11;
            } else if (tag == "[each fatigue unit]" &&
                       !(seen & (1u << 12))) {
                valid = ReadPositive(parsed.fatigue_unit); seen |= 1u << 12;
            } else if (tag == "[need money for init event]" &&
                       !(seen & (1u << 13))) {
                valid = ReadPositive(parsed.reset_money); seen |= 1u << 13;
            } else if (tag == "[freely give item event reward list]" &&
                       !(seen & (1u << 14))) {
                valid = ReadRewardRows(parsed.rows); seen |= 1u << 14;
            } else {
                valid = false;
            }
        }

        if (!valid || seen != 0x7fffu || parsed.gift_box != 690001061u ||
            parsed.chicken_max != 9u || parsed.coupon != 690001062u ||
            parsed.connect_seconds != 1200u ||
            parsed.offline_seconds != 3600u ||
            parsed.total_extension_days != 120u ||
            parsed.unlimited_threshold_days != 60u ||
            parsed.total_extension_reward != 690001063u ||
            parsed.base_extension_days != 30u ||
            parsed.normal_lower != 4u || parsed.normal_upper != 8u ||
            parsed.separate_lower != 10u || parsed.separate_upper != 14u ||
            parsed.each_time_unit != 1u || parsed.fatigue_unit != 40u ||
            parsed.reset_money != 100000u || !G_CDataManager() ||
            !IsStackable(parsed.gift_box) || !IsStackable(parsed.coupon) ||
            !IsStackable(parsed.total_extension_reward)) {
            return false;
        }
        for (std::vector<RewardRow>::const_iterator row = parsed.rows.begin();
             row != parsed.rows.end(); ++row) {
            if (!IsEquipmentWeapon(row->item[0]) ||
                !IsEquipmentWeapon(row->item[5])) {
                return false;
            }
            for (unsigned int column = 1u; column < 5u; ++column) {
                if (!IsStackable(row->item[column])) return false;
            }
        }
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    FreelyGiveConfig config_;
};

ConfigCache& Config() {
    static ConfigCache cache;
    return cache;
}

struct Session {
    Session()
        : account_id(0u), charac_no(0u), session_token(0u),
          next_request_token(1u), server_group(-1), revision(0u),
          owner_charac_no(0u), current_item_id(0u), generation(0u),
          pending_token(0u), pending_charac_no(0u),
          pending_server_group(-1), pending_item_id(0u), pending_aux(0u),
          pending_slot(-2), phase(0u), failed(0u), reset_count(0u),
          chicken_count(0u), extension_days(0u), normal_mask(0u),
          disjoint_mask(0u), total_reward_claimed(0u), owner_match(false),
          pending_action(ACTION_NONE), loaded(false), db_pending(false),
          local_mutation(false), pending_command(0), local_error(0u),
          db_retry_count(0u), deferred_fatigue(0u), deferred_level(0u),
          deferred_timed_key(0u),
          deferred_upgrade_item(0u), deferred_upgrade_mode(0u),
          deferred_upgrade_bit(0u), deferred_failed_item(0u),
          proof_kind(0u), saga_state(0u), pre_money(0u), post_money(0u),
          pending_request(), owner_name() {
        std::memset(&pending_request, 0, sizeof(pending_request));
        std::memset(pre_inventory_sha256, 0,
                    sizeof(pre_inventory_sha256));
        std::memset(post_inventory_sha256, 0,
                    sizeof(post_inventory_sha256));
        std::memset(owner_name, 0, sizeof(owner_name));
    }

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    int server_group;
    uint32_t revision;
    uint32_t owner_charac_no;
    uint32_t current_item_id;
    uint32_t generation;
    uint32_t pending_token;
    uint32_t pending_charac_no;
    int pending_server_group;
    uint32_t pending_item_id;
    uint32_t pending_aux;
    int32_t pending_slot;
    uint8_t phase;
    uint8_t failed;
    uint8_t reset_count;
    uint8_t chicken_count;
    uint8_t extension_days;
    uint8_t normal_mask;
    uint8_t disjoint_mask;
    uint8_t total_reward_claimed;
    bool owner_match;
    uint8_t pending_action;
    bool loaded;
    bool db_pending;
    bool local_mutation;
    int pending_command;
    uint8_t local_error;
    uint8_t db_retry_count;
    uint32_t deferred_fatigue;
    uint32_t deferred_level;
    uint32_t deferred_timed_key;
    uint32_t deferred_upgrade_item;
    uint8_t deferred_upgrade_mode;
    uint8_t deferred_upgrade_bit;
    uint32_t deferred_failed_item;
    uint8_t proof_kind;
    uint8_t saga_state;
    uint32_t pre_money;
    uint32_t post_money;
    uint8_t pre_inventory_sha256[32];
    uint8_t post_inventory_sha256[32];
    Message pending_request;
    char owner_name[freely_give_item_db::kCharacterNameBytes];
};

Mutex g_mutex;
std::map<CUser*, Session> g_sessions;
uint32_t g_session_token = 1u;

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

uint32_t CurrentCharacterNo(CUser* user) {
    return user && user->getCurCharacR() && user->getCurCharacNo() > 0
               ? static_cast<uint32_t>(user->getCurCharacNo()) : 0u;
}

bool IsReady(CUser* user) {
    return user && user->get_state() == 3 && user->get_acc_id() > 0 &&
           CurrentCharacterNo(user) != 0u && user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               freely_give_item_db::kMaximumServerGroupCount;
}

bool IsEventActive(CUser* user) {
    CEventBase* const event = GlobalData::s_event_manager
        ? GlobalData::s_event_manager->GetRepeatEvent(
              freely_give_item_db::kEventId) : 0;
    return event && event->IsEventing(user) == 1;
}

uint32_t NowEpoch() {
    const std::time_t now = std::time(0);
    return now > 0 && static_cast<uint64_t>(now) <= 0xffffffffULL
               ? static_cast<uint32_t>(now) : 0u;
}

void TraceFailure(const char* text, CUser* user) {
    cMyTrace trace("CMD530-534 Phoenix 0828", __LINE__, 5);
    trace("[0828 Phoenix] %s account(%u) charac(%u)",
          text ? text : "unknown",
          static_cast<unsigned int>(user && user->get_acc_id() > 0
                                        ? user->get_acc_id() : 0),
          static_cast<unsigned int>(CurrentCharacterNo(user)));
}

void CopyName(char* destination, unsigned int capacity, const char* source) {
    if (!destination || capacity == 0u) return;
    std::memset(destination, 0, capacity);
    if (source) std::strncpy(destination, source, capacity - 1u);
}

bool BuildInventoryMoneyProof(CUser* user,
                              uint8_t digest[32],
                              uint32_t& money) {
    if (!user || !digest) return false;
    unsigned char snapshot_bytes[sizeof(SIG_SAVE_INVENTORY)];
    std::memset(snapshot_bytes, 0, sizeof(snapshot_bytes));
    SIG_SAVE_INVENTORY* const snapshot =
        reinterpret_cast<SIG_SAVE_INVENTORY*>(snapshot_bytes);
    const CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory || inventory->get_money() < 0) return false;
    snapshot->account_id = static_cast<uint32_t>(user->get_acc_id());
    snapshot->character_no = CurrentCharacterNo(user);
    snapshot->money = static_cast<uint32_t>(inventory->get_money());

    Inven_Item equipment_snapshot[
        oldsrv::config::kEquipmentTransferItemCount];
    for (int slot = 0; slot < oldsrv::config::kEquipmentTransferItemCount;
         ++slot) {
        const Inven_Item* const item = inventory->GetInvenRef(0, slot);
        if (!item) return false;
        equipment_snapshot[slot] = *item;
    }
    std::memcpy(snapshot->stackable_items, inventory->GetInvenStart(1),
                sizeof(snapshot->stackable_items));
    CInventory::RemoveKCItem(equipment_snapshot,
                             snapshot->stackable_items);
    if (user->get_state() == 10) {
        CInventory::RemoveFPItem(equipment_snapshot,
                                 snapshot->stackable_items);
    }
    std::memcpy(snapshot->equipment_items, equipment_snapshot,
                sizeof(snapshot->equipment_items));
    (void)user->removeRentalItemInfoFromInvenBuf(snapshot, 1);
    (void)user->removeRentalItemInfoFromInvenBuf(snapshot, 2);

    const size_t equipment_bytes =
        static_cast<size_t>(oldsrv::config::kOrdinaryEquipmentTypeCount) *
        sizeof(Inven_Item);
    const size_t inventory_bytes = sizeof(snapshot->stackable_items);
    std::vector<unsigned char> image(equipment_bytes + inventory_bytes);
    std::memcpy(&image[0],
                snapshot->equipment_items +
                    oldsrv::config::kOrdinaryEquipmentTypeFirst,
                equipment_bytes);
    std::memcpy(&image[equipment_bytes], snapshot->stackable_items,
                inventory_bytes);
    CNChecksumSha256 checksum;
    money = snapshot->money;
    return checksum.MakeChecksumFromMemory(
               &image[0], static_cast<int>(image.size()), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

bool ProofMatchesCurrent(CUser* user,
                         const uint8_t expected[32],
                         uint32_t expected_money) {
    uint8_t actual[32] = {};
    uint32_t money = 0u;
    return BuildInventoryMoneyProof(user, actual, money) &&
           money == expected_money &&
           std::memcmp(actual, expected, 32u) == 0;
}

bool IsLocalMutationAction(uint8_t action) {
    return action == ACTION_CLAIM_BOX ||
           action == ACTION_CLAIM_COUPON_LOCAL ||
           action == ACTION_USE_COUPON ||
           action == ACTION_CONVERT_UNLIMITED || action == ACTION_RESET ||
           action == ACTION_SELECT_WEAPON;
}

bool FillMailReward(uint32_t item_id,
                    uint32_t count,
                    freely_give_item_db::MailReward& destination) {
    std::memset(&destination, 0, sizeof(destination));
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    if (!prototype || !prototype->vtable_ || count == 0u || count > 999u) {
        return false;
    }
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    item.set_add_info(static_cast<int>(count));
    InvenItemMailRecord record;
    std::memset(&record, 0, sizeof(record));
    item.ExportMailRecord(record);
    destination.item_id = record.item_id;
    destination.add_info = record.add_info;
    destination.extend_info = record.extend_info;
    destination.chronicle_value = record.chronicle_value;
    destination.endurance = record.endurance;
    destination.amplify_value = record.amplify_value;
    destination.seal_flag = record.seal_flag;
    destination.item_attr = record.item_attr;
    destination.upgrade = static_cast<uint8_t>(item.GetUpgrade());
    destination.amplify_option = record.amplify_option;
    destination.separate_upgrade = record.separate_upgrade;
    destination.avatar_flag = record.avatar_flag ? 1u : 0u;
    destination.creature_flag = record.creature_flag ? 1u : 0u;
    std::memcpy(destination.item_guid,
                record.item_guid,
                sizeof(destination.item_guid));
    std::memcpy(destination.random_option,
                record.random_option,
                sizeof(destination.random_option));
    return destination.item_id == item_id && destination.add_info == count;
}

bool FillRequest(CUser* user, uint8_t operation, Message& message) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!config || !IsReady(user)) return false;
    std::memset(&message, 0, sizeof(message));
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        found->second.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        found->second.charac_no != CurrentCharacterNo(user) ||
        found->second.server_group != user->GetServerGroup()) {
        return false;
    }
    Session& session = found->second;
    message.account_id = session.account_id;
    message.charac_no = session.charac_no;
    message.session_token = session.session_token;
    message.request_token = NextNonZero(session.next_request_token);
    message.observed_epoch = NowEpoch();
    message.current_level = static_cast<uint32_t>(user->get_charac_level());
    message.server_group = session.server_group;
    CopyName(message.current_name,
             sizeof(message.current_name),
             user->getCurCharacName());
    message.operation = operation;
    message.event_active = IsEventActive(user) ? 1u : 0u;
    message.reply_required = 1u;
    message.chicken_max = config->chicken_max;
    message.base_extension_days = config->base_extension_days;
    message.unlimited_threshold_days = config->unlimited_threshold_days;
    message.total_extension_days = config->total_extension_days;
    message.each_time_unit = config->each_time_unit;
    message.connect_seconds = config->connect_seconds;
    message.offline_seconds = config->offline_seconds;
    message.fatigue_unit = config->fatigue_unit;
    message.reset_money = config->reset_money;
    message.action_slot = -2;
    return message.account_id != 0u && message.charac_no != 0u &&
           message.observed_epoch != 0u && message.current_level != 0u;
}

bool QueueTracked(CUser* user, const Message& message) {
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            found->second.session_token != message.session_token) {
            return false;
        }
        if (found->second.pending_request.request_token !=
            message.request_token) {
            found->second.db_retry_count = 0u;
        }
        found->second.db_pending = true;
        found->second.pending_request = message;
    }
    if (freely_give_item_db::DBFreelyGiveItem::QueueRequest(
            user->GetUID(), message)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == message.session_token &&
        found->second.pending_request.request_token == message.request_token) {
        found->second.db_pending = false;
    }
    return false;
}

void UpdateSessionFromResponse(Session& session, const Message& response) {
    session.revision = response.revision;
    session.owner_charac_no = response.owner_charac_no;
    session.current_item_id = response.current_item_id;
    session.generation = response.generation;
    session.pending_token = response.pending_token;
    session.pending_charac_no = response.pending_charac_no;
    session.pending_server_group = response.pending_server_group;
    session.pending_item_id = response.pending_item_id;
    session.pending_aux = response.pending_aux;
    session.pending_slot = response.action_slot;
    session.phase = response.phase;
    session.failed = response.failed;
    session.reset_count = response.reset_count;
    session.chicken_count = response.chicken_count;
    session.extension_days = response.extension_days;
    session.normal_mask = response.normal_reward_mask;
    session.disjoint_mask = response.disjoint_reward_mask;
    session.total_reward_claimed = response.total_reward_claimed;
    session.owner_match = response.owner_match != 0u;
    session.pending_action = response.pending_action;
    session.proof_kind = response.proof_kind;
    session.saga_state = response.saga_state;
    session.pre_money = response.pre_money;
    session.post_money = response.post_money;
    std::memcpy(session.pre_inventory_sha256,
                response.pre_inventory_sha256, 32u);
    std::memcpy(session.post_inventory_sha256,
                response.post_inventory_sha256, 32u);
    CopyName(session.owner_name,
             sizeof(session.owner_name),
             response.owner_name);
}

bool ResponseMatches(CUser* user, const Message& response) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() || !found->second.db_pending ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.session_token != response.session_token ||
        found->second.server_group != response.server_group ||
        found->second.pending_request.request_token != response.request_token ||
        response.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no != CurrentCharacterNo(user) ||
        response.server_group != user->GetServerGroup()) {
        return false;
    }
    found->second.db_pending = false;
    if (response.result !=
            freely_give_item_db::RESULT_DATABASE_FAILURE) {
        found->second.db_retry_count = 0u;
        UpdateSessionFromResponse(found->second, response);
    }
    return true;
}

Session* FindSessionUnlocked(CUser* user) {
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    return found == g_sessions.end() ? 0 : &found->second;
}

bool CanAccrue(const Session& session) {
    return session.loaded && !session.failed &&
           session.phase == freely_give_item_db::PHASE_PERIODIC_WEAPON;
}

void SendCommandResult(CUser* user, int command, bool success, uint8_t error) {
    if (!user || command < 530 || command > 534) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, command);
    packet.put_byte(success ? 1 : 0);
    if (!success) packet.put_byte(error);
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapError(int command, uint8_t result) {
    using namespace freely_give_item_db;
    if (command == ENUM_CMDPACKET_REQUEST_FREELY_GIVE_ITEM_BOX) {
        if (result == RESULT_INVENTORY) return LOCAL_ERROR_INVENTORY;
        if (result == RESULT_ALREADY_CLAIMED) return LOCAL_ERROR_ALREADY_CLAIMED;
        return LOCAL_ERROR_PENDING;
    }
    if (command == ENUM_CMDPACKET_REQUEST_COUPON_CHANGE) {
        if (result == RESULT_INVENTORY) return LOCAL_ERROR_INVENTORY;
        if (result == RESULT_PENDING) return LOCAL_ERROR_PENDING;
        if (result == RESULT_DIFFERENT_SERVER) return LOCAL_ERROR_DIFFERENT_SERVER;
        if (result == RESULT_NOT_ENOUGH_CHICKEN) return LOCAL_ERROR_CONDITION;
        if (result == RESULT_NO_WEAPON || result == RESULT_NOT_OWNER ||
            result == RESULT_FAILED || result == RESULT_STATE)
            return LOCAL_ERROR_NO_WEAPON_OR_MONEY;
        return LOCAL_ERROR_GENERAL;
    }
    if (command == ENUM_CMDPACKET_USE_FREE_GIVE_ITEM_COUPON) {
        if (result == RESULT_INVENTORY) return LOCAL_ERROR_INVENTORY;
        if (result == RESULT_PENDING) return LOCAL_ERROR_PENDING;
        if (result == RESULT_COUPON_SHORTAGE) return LOCAL_ERROR_CONDITION;
        if (result == RESULT_NO_WEAPON || result == RESULT_NOT_OWNER ||
            result == RESULT_FAILED || result == RESULT_STATE)
            return LOCAL_ERROR_NO_WEAPON_OR_MONEY;
        return LOCAL_ERROR_PENDING;
    }
    if (command == ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM) {
        if (result == RESULT_PENDING) return LOCAL_ERROR_PENDING;
        if (result == RESULT_NO_WEAPON || result == RESULT_NOT_OWNER ||
            result == RESULT_FAILED || result == RESULT_STATE)
            return LOCAL_ERROR_NO_WEAPON_OR_MONEY;
        if (result == RESULT_SCRIPT) return LOCAL_ERROR_SCRIPT;
        if (result == RESULT_BELOW_THRESHOLD)
            return LOCAL_ERROR_EVENT_OR_THRESHOLD;
        return LOCAL_ERROR_GENERAL;
    }
    if (command == ENUM_CMDPACKET_INIT_FREE_GIVE_ITEM_EVENT) {
        if (result == RESULT_FAILED) return LOCAL_ERROR_FAILED;
        if (result == RESULT_NO_WEAPON) return LOCAL_ERROR_NO_RESET_WEAPON;
        if (result == RESULT_ALREADY_RESET || result == RESULT_PENDING)
            return LOCAL_ERROR_PENDING;
        if (result == RESULT_EVENT_CLOSED)
            return LOCAL_ERROR_EVENT_OR_THRESHOLD;
        if (result == RESULT_NOT_OWNER || result == RESULT_STATE)
            return LOCAL_ERROR_PENDING;
        return LOCAL_ERROR_PENDING;
    }
    return LOCAL_ERROR_GENERAL;
}

uint8_t DefaultCommandError(int command) {
    if (command == ENUM_CMDPACKET_REQUEST_COUPON_CHANGE ||
        command == ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM) {
        return LOCAL_ERROR_GENERAL;
    }
    return LOCAL_ERROR_PENDING;
}

struct LocatedItem {
    LocatedItem() : item(0), inventory_type(1), item_space(0), slot(-1) {}
    Inven_Item* item;
    int inventory_type;
    int item_space;
    int slot;
};

bool FindItem(CUser* user,
              uint32_t item_id,
              bool include_equipment,
              LocatedItem& located) {
    CInventory* const inventory = user ? user->getCurCharacInvenW() : 0;
    if (!inventory || item_id == 0u) return false;
    if (include_equipment) {
        for (int slot = 0; slot < oldsrv::config::kEquipmentTypeCount; ++slot) {
            Inven_Item* const item = inventory->GetInvenRef(0, slot);
            if (item && item->item_id_ == static_cast<int32_t>(item_id)) {
                located.item = item; located.inventory_type = 0;
                located.item_space = 3; located.slot = slot; return true;
            }
        }
    }
    for (int slot = 0; slot < oldsrv::config::kMainInventoryItemCount; ++slot) {
        Inven_Item* const item = inventory->GetInvenRef(1, slot);
        if (item && item->item_id_ == static_cast<int32_t>(item_id)) {
            located.item = item; located.inventory_type = 1;
            located.item_space = 0; located.slot = slot; return true;
        }
    }
    return false;
}

bool FindAnyPeriodic(CUser* user, LocatedItem& located) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!config) return false;
    for (std::vector<RewardRow>::const_iterator row = config->rows.begin();
         row != config->rows.end(); ++row) {
        if (FindItem(user, row->item[0], true, located)) return true;
    }
    return false;
}

uint32_t CountItem(CUser* user, uint32_t item_id) {
    CInventory* const inventory = user ? user->getCurCharacInvenR() : 0;
    if (!inventory || item_id == 0u) return 0u;
    uint64_t count = 0u;
    for (int slot = 0; slot < oldsrv::config::kMainInventoryItemCount; ++slot) {
        const Inven_Item* const item = inventory->GetInvenRef(1, slot);
        if (!item || item->item_id_ != static_cast<int32_t>(item_id)) continue;
        CItem* const prototype = G_CDataManager()->find_item(item->item_id_);
        count += prototype && prototype->is_stackable()
                     ? static_cast<uint32_t>(std::max(0, item->get_add_info()))
                     : 1u;
    }
    return static_cast<uint32_t>(std::min<uint64_t>(count, 0xffffffffULL));
}

int InsertItem(CUser* user, uint32_t item_id, uint32_t count) {
    CInventory* const inventory = user ? user->getCurCharacInvenW() : 0;
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    if (!inventory || !prototype || !prototype->vtable_ || count == 0u ||
        count > 999u) return -1;
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    if (prototype->is_stackable()) item.set_add_info(static_cast<int>(count));
    return inventory->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(13), true, false);
}

bool DeleteOneStackable(CUser* user, uint32_t item_id) {
    CInventory* const inventory = user ? user->getCurCharacInvenW() : 0;
    if (!inventory) return false;
    for (int slot = 0; slot < oldsrv::config::kMainInventoryItemCount; ++slot) {
        Inven_Item* const item = inventory->GetInvenRef(1, slot);
        if (item && item->item_id_ == static_cast<int32_t>(item_id) &&
            item->get_add_info() > 0) {
            if (inventory->delete_item(static_cast<INVEN_TYPE>(1), slot, 1,
                                       static_cast<eItemDelReason>(3), true) != 1)
                return false;
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), slot);
            return true;
        }
    }
    return false;
}

bool ResolveCommandSlot(CUser* user, int source_slot, LocatedItem& located) {
    CInventory* const inventory = user ? user->getCurCharacInvenW() : 0;
    if (!inventory) return false;
    if (source_slot == -1) {
        located.inventory_type = 0;
        located.item_space = 3;
        located.slot = oldsrv::config::kEquipmentTypeWeapon;
    } else if (source_slot >= 0 &&
               source_slot < oldsrv::config::kMainInventoryItemCount) {
        located.inventory_type = 1;
        located.item_space = 0;
        located.slot = source_slot;
    } else {
        return false;
    }
    located.item = inventory->GetInvenRef(located.inventory_type,
                                          located.slot);
    return located.item && !located.item->isEmpty();
}

uint16_t ExpiryDayAfter(uint32_t days) {
    const uint64_t now = NowEpoch();
    if (now == 0u) return 0u;
    const uint64_t expiry = now + static_cast<uint64_t>(days) * 86400ULL;
    if (expiry <= 1151683200ULL) return 0u;
    const uint64_t encoded = (expiry - 1151683200ULL) / 86400ULL;
    return encoded <= 65535u ? static_cast<uint16_t>(encoded) : 0u;
}

bool ReplaceWithPermanent(CUser* user,
                          LocatedItem& source_location,
                          uint32_t permanent_item_id) {
    if (!user || !source_location.item) return false;
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(permanent_item_id)) : 0;
    if (!prototype || !prototype->vtable_ ||
        prototype->vtable_->get_item_type(prototype) !=
            oldsrv::config::kEquipmentTypeWeapon) return false;

    const Inven_Item source = *source_location.item;
    Inven_Item replacement;
    prototype->vtable_->make_item(prototype, replacement);
    replacement.item_id_ = static_cast<int32_t>(permanent_item_id);
    replacement.SetUpgrade(static_cast<uint8_t>(source.GetUpgrade()));
    replacement.set_add_info(source.get_add_info());
    replacement.extend_info_ = source.extend_info_;
    replacement.amplify_option_ = source.amplify_option_;
    replacement.upgrade_separate_.SetUpgradeSeparate(
        static_cast<uint8_t>(
            source.upgrade_separate_.GetUpgradeSeparate()));

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) return false;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (session) session->local_mutation = true;
    }
    const int deleted = inventory->delete_item(
        static_cast<INVEN_TYPE>(source_location.inventory_type),
        source_location.slot, 1, static_cast<eItemDelReason>(54), true);
    if (deleted == 1) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(
                                 source_location.item_space),
                             source_location.slot);
        inventory->update_item(
            static_cast<INVEN_TYPE>(source_location.inventory_type),
            source_location.slot, replacement);
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(
                                 source_location.item_space),
                             source_location.slot);
        user->enableSaveInven();
    }
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (session) session->local_mutation = false;
    }
    return deleted == 1;
}

void SendIncrease(CUser* user,
                  uint8_t chicken_count,
                  uint8_t reason,
                  uint8_t award_count,
                  uint8_t extension_days,
                  uint16_t countdown) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_FREELY_GIVEITEM_EVENT_INCREASE_CNT);
    packet.put_byte(chicken_count);
    packet.put_byte(reason);
    packet.put_byte(award_count);
    packet.put_byte(extension_days);
    packet.put_short(countdown);
    packet.finalize(true);
    user->Send(packet);
}

void SendNotices(CUser* user, const Message& response) {
    for (uint8_t index = 0u; index < response.notice_count; ++index) {
        SendIncrease(user,
                     response.chicken_count,
                     response.notices[index].reason,
                     response.notices[index].award_count,
                     response.extension_days,
                     response.countdown_seconds);
    }
}

void SendInfoSnapshot(CUser* user, const Session& session) {
    if (!user) return;
    bool inventory_flag = false;
    bool equipped_flag = false;
    if (session.current_item_id != 0u) {
        LocatedItem item;
        if (FindItem(user, session.current_item_id, true, item)) {
            equipped_flag = item.inventory_type == 0;
            inventory_flag = item.inventory_type == 1;
        }
    }
    const size_t raw_length = std::min<size_t>(
        std::strlen(session.owner_name),
        freely_give_item_db::kCharacterNameBytes - 1u);
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_FREELY_GIVEITEM_EVENT_INFO);
    packet.put_byte(IsEventActive(user) ? 1 : 0);
    packet.put_int(static_cast<int>(raw_length));
    if (raw_length != 0u) {
        packet.put_binary(session.owner_name,
                          static_cast<int>(raw_length));
    }
    packet.put_byte(session.owner_match ? 1 : 0);
    uint8_t client_stage = 0u;
    if (session.phase == freely_give_item_db::PHASE_PERIODIC_WEAPON) {
        client_stage = session.failed ? 2u : 1u;
    } else if (session.phase ==
               freely_give_item_db::PHASE_COMPLETED) {
        client_stage = 3u;
    }
    const uint32_t client_item =
        session.phase == freely_give_item_db::PHASE_NONE ||
        session.phase == freely_give_item_db::PHASE_BOX_CLAIMED
            ? 0x7fffffffu : session.current_item_id;
    packet.put_byte(client_stage);
    packet.put_int(static_cast<int>(client_item));
    packet.put_byte(session.reset_count ? 1 : 0);
    packet.put_byte(inventory_flag ? 1 : 0);
    packet.put_byte(equipped_flag ? 1 : 0);
    packet.put_byte(session.phase ==
                            freely_give_item_db::PHASE_BOX_CLAIMED ||
                        session.phase ==
                            freely_give_item_db::PHASE_COMPLETED
                        ? 1 : 0);
    packet.finalize(true);
    user->Send(packet);
}

void SendInfoFromSession(CUser* user) {
    Session snapshot;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded) return;
        snapshot = *session;
    }
    SendInfoSnapshot(user, snapshot);
}

bool QueueSimple(CUser* user, uint8_t operation, uint32_t source_value) {
    Message request;
    if (!FillRequest(user, operation, request)) return false;
    request.source_value = source_value;
    return QueueTracked(user, request);
}

bool QueueReserve(CUser* user,
                  uint8_t action,
                  uint32_t item_id,
                  uint32_t aux,
                  int slot,
                  int command,
                  bool with_mail) {
    Message request;
    if (!FillRequest(user,
                     freely_give_item_db::OPERATION_RESERVE_ACTION,
                     request)) return false;
    request.action = action;
    request.action_item_id = item_id;
    request.action_aux = aux;
    request.action_slot = slot;
    if (IsLocalMutationAction(action)) {
        request.proof_kind = freely_give_item_db::
            PERSISTENCE_PROOF_INVENTORY_MONEY;
        if (!BuildInventoryMoneyProof(user,
                                      request.pre_inventory_sha256,
                                      request.pre_money)) {
            return false;
        }
    }
    if (with_mail && !FillMailReward(item_id, 1u, request.mail_reward)) {
        return false;
    }
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded || session->db_pending ||
            session->pending_action != ACTION_NONE) return false;
        session->pending_command = command;
        session->local_error = 0u;
    }
    if (QueueTracked(user, request)) return true;
    MutexGuard lock(&g_mutex);
    Session* const session = FindSessionUnlocked(user);
    if (session) session->pending_command = 0;
    return false;
}

bool QueueCommitOrCancel(CUser* user,
                         const Message& reserve,
                         bool commit,
                         uint8_t local_error) {
    Message request;
    if (!FillRequest(
            user,
            commit ? freely_give_item_db::OPERATION_COMMIT_ACTION
                   : freely_give_item_db::OPERATION_CANCEL_ACTION,
            request)) return false;
    request.action = reserve.pending_action;
    request.action_token = reserve.pending_token;
    request.action_item_id = reserve.pending_item_id;
    request.action_aux = reserve.pending_aux;
    request.action_slot = reserve.action_slot;
    request.pending_token = reserve.pending_token;
    request.pending_action = reserve.pending_action;
    request.pending_server_group = reserve.pending_server_group;
    request.pending_charac_no = reserve.pending_charac_no;
    request.pending_item_id = reserve.pending_item_id;
    request.pending_aux = reserve.pending_aux;
    request.proof_kind = reserve.proof_kind;
    request.saga_state = reserve.saga_state;
    request.pre_money = reserve.pre_money;
    request.post_money = reserve.post_money;
    std::memcpy(request.pre_inventory_sha256,
                reserve.pre_inventory_sha256, 32u);
    std::memcpy(request.post_inventory_sha256,
                reserve.post_inventory_sha256, 32u);
    const FreelyGiveConfig* const config = Config().Get();
    if (commit && config &&
        reserve.pending_action == ACTION_CLAIM_COUPON_MAIL) {
        if (!FillMailReward(config->coupon, 1u, request.mail_reward))
            return false;
    }
    if (commit && config && reserve.pending_action == ACTION_USE_COUPON) {
        if (!FillMailReward(config->total_extension_reward,
                            1u,
                            request.mail_reward)) return false;
    }
    if (commit && reserve.pending_action == ACTION_UPGRADE_REWARD) {
        if (!FillMailReward(reserve.pending_item_id,
                            1u,
                            request.mail_reward)) return false;
    }
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session) return false;
        session->local_error = local_error;
    }
    if (IsLocalMutationAction(reserve.pending_action) &&
        user->SaveInventory() != 1) {
        return false;
    }
    return QueueTracked(user, request);
}

bool QueueStage(CUser* user, const Message& reserve) {
    Message request;
    if (!FillRequest(user,
                     freely_give_item_db::OPERATION_STAGE_ACTION,
                     request)) {
        return false;
    }
    request.action = reserve.pending_action;
    request.action_token = reserve.pending_token;
    request.action_item_id = reserve.pending_item_id;
    request.action_aux = reserve.pending_aux;
    request.action_slot = reserve.action_slot;
    request.pending_token = reserve.pending_token;
    request.pending_action = reserve.pending_action;
    request.pending_server_group = reserve.pending_server_group;
    request.pending_charac_no = reserve.pending_charac_no;
    request.pending_item_id = reserve.pending_item_id;
    request.pending_aux = reserve.pending_aux;
    request.proof_kind = reserve.proof_kind;
    request.saga_state = reserve.saga_state;
    request.pre_money = reserve.pre_money;
    std::memcpy(request.pre_inventory_sha256,
                reserve.pre_inventory_sha256, 32u);
    if (!BuildInventoryMoneyProof(user,
                                  request.post_inventory_sha256,
                                  request.post_money) ||
        (request.pre_money == request.post_money &&
         std::memcmp(request.pre_inventory_sha256,
                     request.post_inventory_sha256, 32u) == 0 &&
         request.action != ACTION_SELECT_WEAPON)) {
        return false;
    }
    return QueueTracked(user, request);
}

bool ApplyReserved(CUser* user, const Message& reserve) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!user || !config || reserve.pending_action == ACTION_NONE ||
        reserve.pending_token == 0u) return false;
    const bool local_action = IsLocalMutationAction(reserve.pending_action);
    if (local_action) {
        if (reserve.proof_kind != freely_give_item_db::
                                      PERSISTENCE_PROOF_INVENTORY_MONEY ||
            (reserve.saga_state !=
                 freely_give_item_db::SAGA_STATE_RESERVED &&
             reserve.saga_state !=
                 freely_give_item_db::SAGA_STATE_STAGED)) {
            TraceFailure("invalid durable action proof", user);
            return false;
        }
        if (reserve.saga_state ==
                freely_give_item_db::SAGA_STATE_STAGED &&
            ProofMatchesCurrent(user, reserve.post_inventory_sha256,
                                 reserve.post_money)) {
            // Refresh the durable stage correlation after reconnect before
            // issuing a kind-7 finalization.  The native owner accepts this
            // only when DB3 is still the exact staged post-image (or the
            // original pre-image when signal 35 never ran).
            return QueueStage(user, reserve);
        }
        if (!ProofMatchesCurrent(user, reserve.pre_inventory_sha256,
                                 reserve.pre_money)) {
            TraceFailure("pending action image is neither exact pre nor post",
                         user);
            return false;
        }
    }
    uint8_t error = LOCAL_ERROR_GENERAL;
    bool success = false;
    switch (reserve.pending_action) {
        case ACTION_CLAIM_BOX: {
            const int slot = InsertItem(user, reserve.pending_item_id, 1u);
            success = slot >= 0;
            if (success) {
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0), slot);
            }
            error = LOCAL_ERROR_INVENTORY;
            break;
        }
        case ACTION_CLAIM_COUPON_LOCAL: {
            const int slot = InsertItem(user, reserve.pending_item_id, 1u);
            success = slot >= 0;
            if (success) {
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0), slot);
            }
            error = LOCAL_ERROR_INVENTORY;
            break;
        }
        case ACTION_CLAIM_COUPON_MAIL:
            success = true;
            break;
        case ACTION_USE_COUPON: {
            LocatedItem weapon;
            const uint32_t coupon_count = CountItem(user, config->coupon);
            const uint32_t baseline = reserve.action_slot >= 0
                ? static_cast<uint32_t>(reserve.action_slot) : 0u;
            if (reserve.pending_aux > 65535u || baseline == 0u ||
                coupon_count != baseline) {
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            if (!FindItem(user, reserve.current_item_id, true, weapon) ||
                !weapon.item) {
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            if (!DeleteOneStackable(user, config->coupon)) {
                error = LOCAL_ERROR_CONDITION;
                break;
            }
            if (!FindItem(user, reserve.current_item_id, true, weapon) ||
                !weapon.item) {
                (void)InsertItem(user, config->coupon, 1u);
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            if (weapon.item->avatar_select_ + config->each_time_unit !=
                    reserve.pending_aux) {
                (void)InsertItem(user, config->coupon, 1u);
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            weapon.item->avatar_select_ =
                static_cast<uint16_t>(reserve.pending_aux);
            user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(
                                     weapon.item_space),
                                 weapon.slot);
            user->enableSaveInven();
            success = true;
            error = LOCAL_ERROR_CONDITION;
            break;
        }
        case ACTION_CONVERT_UNLIMITED: {
            LocatedItem source;
            if (ResolveCommandSlot(user, reserve.action_slot, source) &&
                source.item->item_id_ ==
                    static_cast<int32_t>(reserve.current_item_id)) {
                success = ReplaceWithPermanent(
                    user, source, reserve.pending_item_id);
            }
            error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
            break;
        }
        case ACTION_RESET: {
            LocatedItem weapon;
            CInventory* const inventory = user->getCurCharacInvenW();
            if (!inventory ||
                !ResolveCommandSlot(user, reserve.action_slot, weapon) ||
                !weapon.item || weapon.item->item_id_ !=
                    static_cast<int32_t>(reserve.current_item_id)) {
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            const uint32_t money = inventory->get_money() >= 0
                ? static_cast<uint32_t>(inventory->get_money()) : 0u;
            if (money != reserve.pending_aux || money < config->reset_money ||
                inventory->use_money(
                    static_cast<int>(config->reset_money),
                    static_cast<eMoneySubReason>(0), true) != 1) {
                error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
                break;
            }
            {
                MutexGuard lock(&g_mutex);
                Session* const session = FindSessionUnlocked(user);
                if (session) session->local_mutation = true;
            }
            success = inventory->delete_item(
                          static_cast<INVEN_TYPE>(weapon.inventory_type),
                          weapon.slot, 1,
                          static_cast<eItemDelReason>(54), true) == 1;
            {
                MutexGuard lock(&g_mutex);
                Session* const session = FindSessionUnlocked(user);
                if (session) session->local_mutation = false;
            }
            if (success) {
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(
                                         weapon.item_space),
                                     weapon.slot);
            } else {
                (void)inventory->gain_money(
                    static_cast<int>(config->reset_money),
                    static_cast<eMoneyAddReason>(0), true, 0);
            }
            error = money < config->reset_money
                        ? LOCAL_ERROR_NO_WEAPON_OR_MONEY
                        : LOCAL_ERROR_NO_RESET_WEAPON;
            break;
        }
        case ACTION_UPGRADE_REWARD:
            // Delivery is the deterministic DB-side postal outbox.  The
            // local phase therefore has no mutation to replay.
            success = true;
            break;
        case ACTION_SELECT_WEAPON: {
            LocatedItem weapon;
            if (reserve.pending_aux != 0u &&
                reserve.pending_aux <= 65535u &&
                ResolveCommandSlot(user, reserve.action_slot, weapon) &&
                weapon.item && weapon.item->item_id_ ==
                    static_cast<int32_t>(reserve.pending_item_id)) {
                weapon.item->avatar_select_ =
                    static_cast<uint16_t>(reserve.pending_aux);
                user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(
                                         weapon.item_space),
                                     weapon.slot);
                user->enableSaveInven();
                success = true;
            }
            error = LOCAL_ERROR_NO_WEAPON_OR_MONEY;
            break;
        }
        default:
            break;
    }
    if (success && local_action) {
        if (!QueueStage(user, reserve)) {
            TraceFailure("post-image stage queue failed; pending retained",
                         user);
            return false;
        }
        return true;
    }
    if (!success && local_action) {
        Message cancelled = reserve;
        if (!BuildInventoryMoneyProof(user,
                                      cancelled.post_inventory_sha256,
                                      cancelled.post_money) ||
            cancelled.post_money != cancelled.pre_money ||
            std::memcmp(cancelled.post_inventory_sha256,
                        cancelled.pre_inventory_sha256, 32u) != 0) {
            TraceFailure("compensation not exact; pending retained", user);
            return false;
        }
        if (!QueueCommitOrCancel(user, cancelled, false, error)) {
            TraceFailure("durable cancel queue failed", user);
            return false;
        }
        return false;
    }
    if (!QueueCommitOrCancel(user, reserve, success, error)) {
        TraceFailure("mail action commit/cancel queue failed", user);
        return false;
    }
    return success;
}

bool RetryPendingAction(CUser* user) {
    Message reserve;
    std::memset(&reserve, 0, sizeof(reserve));
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded || session->db_pending ||
            session->pending_action == ACTION_NONE ||
            session->pending_token == 0u) return false;
        reserve.pending_action = session->pending_action;
        reserve.pending_token = session->pending_token;
        if (session->pending_charac_no != session->charac_no ||
            session->pending_server_group != session->server_group) {
            return false;
        }
        reserve.pending_charac_no = session->pending_charac_no;
        reserve.pending_server_group = session->pending_server_group;
        reserve.pending_item_id = session->pending_item_id;
        reserve.pending_aux = session->pending_aux;
        reserve.action_slot = session->pending_slot;
        reserve.current_item_id = session->current_item_id;
        reserve.owner_charac_no = session->owner_charac_no;
        reserve.phase = session->phase;
        reserve.failed = session->failed;
        reserve.proof_kind = session->proof_kind;
        reserve.saga_state = session->saga_state;
        reserve.pre_money = session->pre_money;
        reserve.post_money = session->post_money;
        std::memcpy(reserve.pre_inventory_sha256,
                    session->pre_inventory_sha256, 32u);
        std::memcpy(reserve.post_inventory_sha256,
                    session->post_inventory_sha256, 32u);
    }
    return ApplyReserved(user, reserve);
}

void ReconcileLoadedState(CUser* user, const Message& response) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!config || !user) return;
    if (response.pending_action != ACTION_NONE &&
        response.pending_token != 0u &&
        (response.pending_charac_no != CurrentCharacterNo(user) ||
         response.pending_server_group != user->GetServerGroup())) {
        return;
    }
    if (response.pending_action != ACTION_NONE) {
        (void)ApplyReserved(user, response);
        return;
    }
    if (response.phase == freely_give_item_db::PHASE_BOX_CLAIMED &&
        response.owner_match) {
        for (std::vector<RewardRow>::const_iterator row = config->rows.begin();
             row != config->rows.end(); ++row) {
            LocatedItem weapon;
            if (!FindItem(user, row->item[0], true, weapon)) continue;
            uint16_t expiry = weapon.item->avatar_select_;
            const uint16_t configured =
                ExpiryDayAfter(config->base_extension_days);
            if (configured != 0u && expiry < configured) expiry = configured;
            if (expiry != 0u) {
                const int action_slot = weapon.inventory_type == 0
                    ? -1 : weapon.slot;
                (void)QueueReserve(user, ACTION_SELECT_WEAPON, row->item[0],
                                   expiry, action_slot, 0, false);
            }
            return;
        }
    }
    if (response.phase == freely_give_item_db::PHASE_PERIODIC_WEAPON &&
        response.owner_match && !response.failed) {
        LocatedItem weapon;
        if (!FindItem(user, response.current_item_id, true, weapon)) {
            Message failed;
            if (FillRequest(user,
                            freely_give_item_db::OPERATION_MARK_FAILED,
                            failed)) {
                failed.source_value = response.current_item_id;
                (void)QueueTracked(user, failed);
            }
        }
    }
}

void FlushDeferred(CUser* user);

void CompleteCommand(CUser* user, bool success, uint8_t error) {
    int command = 0;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session) return;
        command = session->pending_command;
        session->pending_command = 0;
        session->local_error = 0u;
    }
    if (command) SendCommandResult(user, command, success, error);
}

void QueueUpgradeReward(CUser* user,
                        uint32_t item_id,
                        uint8_t mode,
                        uint8_t bit_index) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!config || bit_index >= 2u || (mode != 1u && mode != 2u)) return;
    const RewardRow* const row = config->FindPeriodic(item_id);
    if (!row) return;
    const uint32_t reward = mode == 1u
        ? row->item[1u + bit_index] : row->item[3u + bit_index];
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !CanAccrue(*session) || !session->owner_match ||
            session->current_item_id != item_id) return;
        const uint8_t mask = mode == 1u ? session->normal_mask
                                        : session->disjoint_mask;
        if (mask & static_cast<uint8_t>(1u << bit_index)) return;
    }
    if (!QueueReserve(user,
                      ACTION_UPGRADE_REWARD,
                      reward,
                      (static_cast<uint32_t>(mode) << 8u) | bit_index,
                      -2,
                      0,
                      true)) {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (session) {
            session->deferred_upgrade_item = item_id;
            session->deferred_upgrade_mode = mode;
            session->deferred_upgrade_bit = bit_index;
        }
    }
}

void FlushDeferred(CUser* user) {
    uint32_t fatigue = 0u;
    uint32_t level = 0u;
    uint32_t timed_key = 0u;
    uint32_t upgrade_item = 0u;
    uint8_t upgrade_mode = 0u;
    uint8_t upgrade_bit = 0u;
    uint32_t failed_item = 0u;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || session->db_pending || !session->loaded) return;
        if (session->deferred_fatigue) {
            fatigue = session->deferred_fatigue;
            session->deferred_fatigue = 0u;
        } else if (session->deferred_level) {
            level = session->deferred_level;
            session->deferred_level = 0u;
        } else if (session->deferred_timed_key) {
            timed_key = session->deferred_timed_key;
            session->deferred_timed_key = 0u;
        } else if (session->deferred_upgrade_item) {
            upgrade_item = session->deferred_upgrade_item;
            upgrade_mode = session->deferred_upgrade_mode;
            upgrade_bit = session->deferred_upgrade_bit;
            session->deferred_upgrade_item = 0u;
        } else if (session->deferred_failed_item) {
            failed_item = session->deferred_failed_item;
            session->deferred_failed_item = 0u;
        }
    }
    if (fatigue) {
        Message request;
        if (FillRequest(user,
                        freely_give_item_db::OPERATION_FATIGUE,
                        request)) {
            request.source_value = fatigue;
            if (!QueueTracked(user, request)) {
                MutexGuard lock(&g_mutex);
                Session* const session = FindSessionUnlocked(user);
                if (session) session->deferred_fatigue += fatigue;
            }
        }
    } else if (level) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_LEVEL_UP,
                         level)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_level = std::max<uint32_t>(
                session->deferred_level, level);
        }
    } else if (timed_key) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_TIMED,
                         timed_key)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_timed_key = timed_key;
        }
    } else if (upgrade_item) {
        QueueUpgradeReward(user, upgrade_item, upgrade_mode, upgrade_bit);
    } else if (failed_item) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_MARK_FAILED,
                         failed_item)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_failed_item = failed_item;
        }
    }
}

void DispatchEmptyCommand(CUser* user, PacketBuf& packet, int command) {
    if (!user || !IsReady(user)) {
        SendCommandResult(user, command, false,
                          DefaultCommandError(command));
        return;
    }
    const FreelyGiveConfig* const config = Config().Get();
    if (!config) {
        SendCommandResult(user, command, false,
                          DefaultCommandError(command));
        return;
    }
    Session snapshot;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded || session->db_pending ||
            session->pending_action != ACTION_NONE) {
            SendCommandResult(user, command, false, LOCAL_ERROR_PENDING);
            return;
        }
        snapshot = *session;
    }

    bool queued = false;
    if (command == ENUM_CMDPACKET_REQUEST_FREELY_GIVE_ITEM_BOX) {
        if (!IsEventActive(user)) {
            SendCommandResult(user, command, false, LOCAL_ERROR_PENDING);
            return;
        }
        if (snapshot.phase != freely_give_item_db::PHASE_NONE ||
            snapshot.owner_charac_no != 0u) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_ALREADY_CLAIMED);
            return;
        }
        queued = QueueReserve(user, ACTION_CLAIM_BOX, config->gift_box,
                              CountItem(user, config->gift_box), -2,
                              command, false);
    } else if (command == ENUM_CMDPACKET_REQUEST_COUPON_CHANGE) {
        if (snapshot.failed ||
            snapshot.phase !=
                freely_give_item_db::PHASE_PERIODIC_WEAPON) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_NO_WEAPON_OR_MONEY);
            return;
        }
        if (snapshot.chicken_count < config->chicken_max) {
            SendCommandResult(user, command, false, LOCAL_ERROR_CONDITION);
            return;
        }
        const uint8_t action = snapshot.owner_match
            ? ACTION_CLAIM_COUPON_LOCAL : ACTION_CLAIM_COUPON_MAIL;
        queued = QueueReserve(user, action, config->coupon,
                              snapshot.owner_match
                                  ? CountItem(user, config->coupon) : 0u,
                              -2, command, !snapshot.owner_match);
    } else if (command == ENUM_CMDPACKET_USE_FREE_GIVE_ITEM_COUPON) {
        LocatedItem weapon;
        if (!snapshot.owner_match || snapshot.failed ||
            snapshot.phase !=
                freely_give_item_db::PHASE_PERIODIC_WEAPON ||
            !FindItem(user, snapshot.current_item_id, true, weapon)) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_NO_WEAPON_OR_MONEY);
            return;
        }
        if (CountItem(user, config->coupon) == 0u) {
            SendCommandResult(user, command, false, LOCAL_ERROR_CONDITION);
            return;
        }
        if (weapon.item->avatar_select_ == 0u ||
            weapon.item->avatar_select_ == 65535u ||
            static_cast<uint32_t>(weapon.item->avatar_select_) +
                    config->each_time_unit > 65535u) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_NO_WEAPON_OR_MONEY);
            return;
        }
        queued = QueueReserve(
            user, ACTION_USE_COUPON, snapshot.current_item_id,
            static_cast<uint32_t>(weapon.item->avatar_select_) +
                config->each_time_unit,
            static_cast<int>(CountItem(user, config->coupon)),
            command, false);
    } else if (command == ENUM_CMDPACKET_INIT_FREE_GIVE_ITEM_EVENT) {
        if (!IsEventActive(user)) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_EVENT_OR_THRESHOLD);
            return;
        }
        if (snapshot.failed) {
            SendCommandResult(user, command, false, LOCAL_ERROR_FAILED);
            return;
        }
        if (snapshot.reset_count != 0u) {
            SendCommandResult(user, command, false, LOCAL_ERROR_PENDING);
            return;
        }
        LocatedItem weapon;
        if (snapshot.phase !=
                freely_give_item_db::PHASE_PERIODIC_WEAPON ||
            !snapshot.owner_match ||
            !FindItem(user, snapshot.current_item_id, false, weapon)) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_NO_RESET_WEAPON);
            return;
        }
        CInventory* const inventory = user->getCurCharacInvenR();
        if (!inventory || inventory->get_money() <
                              static_cast<int>(config->reset_money)) {
            SendCommandResult(user, command, false,
                              LOCAL_ERROR_NO_WEAPON_OR_MONEY);
            return;
        }
        queued = QueueReserve(
            user, ACTION_RESET, snapshot.current_item_id,
            static_cast<uint32_t>(inventory->get_money()), weapon.slot,
            command, false);
    }
    if (!queued) {
        SendCommandResult(user, command, false, LOCAL_ERROR_PENDING);
    }
}

}  // namespace

FreelyGiveItemEvent::FreelyGiveItemEvent() : CEventBase() {}
FreelyGiveItemEvent::~FreelyGiveItemEvent() {}

void FreelyGiveItemEvent::StartEvent() {
    SetEventFlag(true);
    StartAction();
}

void FreelyGiveItemEvent::StartEvent(Word_Param param) {
    (void)param;
    // Database/GM activation enters through the parameterized virtual.  Keep
    // configuration loading in the manager's following StartAction hook so a
    // single activation cannot enqueue the same account load twice.
    SetEventFlag(true);
}

void FreelyGiveItemEvent::EndEvent() {
    SetEventFlag(false);
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers([](CUser* user) {
            freely_give_item::SendEventInfo(user);
        });
    }
}

void FreelyGiveItemEvent::StartAction() {
    if (!Config().Get()) {
        // Keep the base event flag as the authoritative readiness signal used
        // by the startup loader.  A strict PVF parse failure must not leave an
        // advertised-but-unusable activity active.
        SetEventFlag(false);
        TraceFailure("strict freelygiveitemevent.evt parse failed", 0);
        return;
    }
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers([](CUser* user) {
            freely_give_item::RequestLoad(user);
        });
    }
}

namespace freely_give_item {

void RequestLoad(CUser* user) {
    if (!IsReady(user) || !Config().Get()) return;
    Message request;
    bool already_loaded = false;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found != g_sessions.end() &&
            found->second.account_id ==
                static_cast<uint32_t>(user->get_acc_id()) &&
            found->second.charac_no == CurrentCharacterNo(user) &&
            found->second.server_group == user->GetServerGroup() &&
            (found->second.loaded || found->second.db_pending)) {
            already_loaded = true;
        } else {
            Session& session = g_sessions[user];
            session = Session();
            session.account_id = static_cast<uint32_t>(user->get_acc_id());
            session.charac_no = CurrentCharacterNo(user);
            session.session_token = NextNonZero(g_session_token);
            session.server_group = user->GetServerGroup();
        }
    }
    if (already_loaded) {
        SendEventInfo(user);
        return;
    }
    if (!FillRequest(user,
                     freely_give_item_db::OPERATION_LOAD,
                     request) ||
        !QueueTracked(user, request)) {
        TraceFailure("load queue failed", user);
        ResetSession(user);
    }
}

void FlushAndErase(CUser* user) {
    if (!user) return;
    Message disconnect;
    if (FillRequest(user,
                    freely_give_item_db::OPERATION_DISCONNECT,
                    disconnect)) {
        disconnect.reply_required = 0u;
        (void)freely_give_item_db::DBFreelyGiveItem::QueueRequest(
            user->GetUID(), disconnect);
    }
    ResetSession(user);
}

void ResetSession(CUser* user) {
    MutexGuard lock(&g_mutex);
    g_sessions.erase(user);
}

void OnGlobalMinuteTick() {
    GameWorld* const world = G_GameWorld();
    if (!world) return;
    world->ProcessAllUsers([](CUser* user) {
        if (!IsReady(user)) return;
        bool available = false;
        bool has_pending_action = false;
        {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            available = session && session->loaded &&
                        !session->db_pending &&
                        session->pending_action == ACTION_NONE;
            has_pending_action = session && session->loaded &&
                                 !session->db_pending &&
                                 session->pending_action != ACTION_NONE;
        }
        if (available) {
            (void)QueueSimple(
                user,
                freely_give_item_db::OPERATION_ONLINE_CHECKPOINT,
                0u);
        } else if (has_pending_action) {
            (void)RetryPendingAction(user);
        } else {
            FlushDeferred(user);
        }
    });
}

void OnFatigueUsed(CUser* user, uint32_t fatigue_delta) {
    if (!user || fatigue_delta == 0u || !IsEventActive(user)) return;
    bool queue_now = false;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !CanAccrue(*session)) return;
        if (session->db_pending || session->pending_action != ACTION_NONE) {
            const uint64_t total =
                static_cast<uint64_t>(session->deferred_fatigue) +
                fatigue_delta;
            session->deferred_fatigue = static_cast<uint32_t>(
                std::min<uint64_t>(total, 0xffffffffULL));
        } else {
            queue_now = true;
        }
    }
    if (queue_now) {
        Message request;
        if (FillRequest(user,
                        freely_give_item_db::OPERATION_FATIGUE,
                        request)) {
            request.source_value = fatigue_delta;
            if (!QueueTracked(user, request)) {
                MutexGuard lock(&g_mutex);
                Session* const session = FindSessionUnlocked(user);
                if (session) session->deferred_fatigue += fatigue_delta;
            }
        }
    }
}

void OnLevelUp(CUser* user, uint32_t new_level) {
    if (!user || new_level == 0u || new_level > 85u ||
        !IsEventActive(user)) return;
    bool queue_now = false;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !CanAccrue(*session)) return;
        if (session->db_pending || session->pending_action != ACTION_NONE) {
            session->deferred_level =
                std::max<uint32_t>(session->deferred_level, new_level);
        } else queue_now = true;
    }
    if (queue_now) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_LEVEL_UP,
                         new_level)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_level = std::max<uint32_t>(
                session->deferred_level, new_level);
        }
    }
}

void OnTimedLoginAward(CUser* user, uint32_t occurrence_key) {
    if (!user || occurrence_key == 0u || !IsEventActive(user)) return;
    bool queue_now = false;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !CanAccrue(*session)) return;
        if (session->db_pending || session->pending_action != ACTION_NONE)
            session->deferred_timed_key = occurrence_key;
        else queue_now = true;
    }
    if (queue_now) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_TIMED,
                         occurrence_key)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_timed_key = occurrence_key;
        }
    }
}

void OnBoosterSelectionResult(CUser* user,
                              uint32_t source_item_id,
                              uint32_t result_item_id) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!user || !config || source_item_id != config->gift_box ||
        !config->FindPeriodic(result_item_id)) return;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded ||
            !((session->phase ==
                   freely_give_item_db::PHASE_BOX_CLAIMED &&
               session->owner_match) ||
              session->pending_action == ACTION_CLAIM_BOX)) return;
    }
    LocatedItem weapon;
    if (!FindItem(user, result_item_id, false, weapon)) return;
    const uint16_t expiry = ExpiryDayAfter(config->base_extension_days);
    if (expiry == 0u) return;
    if (!QueueReserve(user, ACTION_SELECT_WEAPON, result_item_id, expiry,
                      weapon.slot, 0, false)) {
        TraceFailure("booster selection reserve queue failed", user);
    }
}

void OnNormalUpgradeResult(CUser* user,
                           uint32_t item_id,
                           uint8_t upgrade,
                           bool succeeded) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!succeeded || !user || !config ||
        (upgrade != config->normal_lower &&
         upgrade != config->normal_upper) ||
        !config->FindPeriodic(item_id)) return;
    QueueUpgradeReward(user, item_id, 1u,
                       upgrade == config->normal_lower ? 0u : 1u);
}

void OnUpgradeDisjoint(CUser* user,
                       uint32_t item_id,
                       uint8_t source_upgrade,
                       bool destroyed) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!destroyed || !user || !config ||
        (source_upgrade != config->separate_lower &&
         source_upgrade != config->separate_upper) ||
        !config->FindPeriodic(item_id)) return;
    QueueUpgradeReward(
        user, item_id, 2u,
        source_upgrade == config->separate_lower ? 0u : 1u);
}

void OnEventWeaponRemoved(CUser* user, uint32_t item_id) {
    const FreelyGiveConfig* const config = Config().Get();
    if (!user || !config || !config->FindPeriodic(item_id)) return;
    bool queue_now = false;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session || !session->loaded || session->local_mutation ||
            session->phase !=
                freely_give_item_db::PHASE_PERIODIC_WEAPON ||
            session->current_item_id != item_id) return;
        if (session->db_pending) session->deferred_failed_item = item_id;
        else queue_now = true;
    }
    if (queue_now) {
        if (!QueueSimple(user,
                         freely_give_item_db::OPERATION_MARK_FAILED,
                         item_id)) {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) session->deferred_failed_item = item_id;
        }
    }
}

void SendEventInfo(CUser* user) {
    (void)RetryPendingAction(user);
    SendInfoFromSession(user);
}

}  // namespace freely_give_item_0828

#define DEFINE_EMPTY_DISPATCHER(class_name, command_id)                    \
class_name::class_name()                                                   \
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>() {} \
class_name::~class_name() {}                                               \
int class_name::dispatch_sig(CUser* user, PacketBuf& packet) {             \
    DispatchEmptyCommand(user, packet, command_id);                        \
    return 0;                                                              \
}                                                                         \
int class_name::read(PacketBuf& packet, MSG_BASE& message) {               \
    (void)packet; (void)message; return 0;                                 \
}                                                                         \
int class_name::process(CUser* user, MSG_BASE& message, ParamBase& param) { \
    (void)user; (void)message; (void)param; return 0;                      \
}

DEFINE_EMPTY_DISPATCHER(
    Dispatcher_RequestFreelyGiveItemBox,
    ENUM_CMDPACKET_REQUEST_FREELY_GIVE_ITEM_BOX)
DEFINE_EMPTY_DISPATCHER(
    Dispatcher_RequestCouponChange,
    ENUM_CMDPACKET_REQUEST_COUPON_CHANGE)
DEFINE_EMPTY_DISPATCHER(
    Dispatcher_UseFreelyGiveItemCoupon,
    ENUM_CMDPACKET_USE_FREE_GIVE_ITEM_COUPON)
DEFINE_EMPTY_DISPATCHER(
    Dispatcher_InitFreelyGiveItemEvent,
    ENUM_CMDPACKET_INIT_FREE_GIVE_ITEM_EVENT)

#undef DEFINE_EMPTY_DISPATCHER

Dispatcher_ChangePeriodicToUnlimited::
    Dispatcher_ChangePeriodicToUnlimited()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>() {}
Dispatcher_ChangePeriodicToUnlimited::~
    Dispatcher_ChangePeriodicToUnlimited() {}

int Dispatcher_ChangePeriodicToUnlimited::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    int source_slot = 0;
    if (!user || packet.get_int(source_slot) != 1 || !IsReady(user)) {
        SendCommandResult(
            user, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
            false, LOCAL_ERROR_GENERAL);
        return 0;
    }
    const FreelyGiveConfig* const config = Config().Get();
    Session snapshot;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!config || !session || !session->loaded || session->db_pending ||
            session->pending_action != ACTION_NONE) {
            SendCommandResult(
                user, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
                false, LOCAL_ERROR_PENDING);
            return 0;
        }
        snapshot = *session;
    }
    LocatedItem source;
    const RewardRow* const row = config->FindPeriodic(snapshot.current_item_id);
    if (!snapshot.owner_match || snapshot.failed ||
        snapshot.phase != freely_give_item_db::PHASE_PERIODIC_WEAPON ||
        !row || !ResolveCommandSlot(user, source_slot, source) ||
        source.item->item_id_ !=
            static_cast<int32_t>(snapshot.current_item_id)) {
        SendCommandResult(
            user, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
            false, LOCAL_ERROR_NO_WEAPON_OR_MONEY);
        return 0;
    }
    if (snapshot.extension_days < config->unlimited_threshold_days) {
        SendCommandResult(
            user, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
            false, LOCAL_ERROR_EVENT_OR_THRESHOLD);
        return 0;
    }
    if (!QueueReserve(
            user, ACTION_CONVERT_UNLIMITED, row->item[5], 0u,
            source_slot, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
            false)) {
        SendCommandResult(
            user, ENUM_CMDPACKET_CHANGE_PERIODIC_TO_UNLIMIT_ITEM,
            false, LOCAL_ERROR_PENDING);
    }
    return 0;
}

int Dispatcher_ChangePeriodicToUnlimited::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}

int Dispatcher_ChangePeriodicToUnlimited::process(
    CUser* user, MSG_BASE& message, ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

InterFreelyGiveItem::InterFreelyGiveItem()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              freely_give_item_db::kTaiwanInnerCode)) {}
InterFreelyGiveItem::~InterFreelyGiveItem() {}

int InterFreelyGiveItem::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(Message))) return 0;
    const Message response = *reinterpret_cast<const Message*>(payload);
    if (response.operation < freely_give_item_db::OPERATION_LOAD ||
        response.operation >
            freely_give_item_db::OPERATION_STAGE_ACTION ||
        response.phase > freely_give_item_db::PHASE_COMPLETED ||
        response.chicken_count > 9u || response.extension_days > 120u ||
        response.pending_server_group < -1 ||
        response.pending_server_group >=
            freely_give_item_db::kMaximumServerGroupCount ||
        response.notice_count >
            freely_give_item_db::kMaximumNoticeCount ||
        !ResponseMatches(user, response)) {
        return 0;
    }

    int command = 0;
    uint8_t local_error = 0u;
    {
        MutexGuard lock(&g_mutex);
        Session* const session = FindSessionUnlocked(user);
        if (!session) return 0;
        command = session->pending_command;
        local_error = session->local_error;
        if (response.operation == freely_give_item_db::OPERATION_LOAD)
            session->loaded = response.result ==
                                  freely_give_item_db::RESULT_SUCCESS;
    }

    if (response.result == freely_give_item_db::
                               RESULT_DATABASE_FAILURE) {
        bool retry = false;
        {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session && session->db_retry_count < 3u) {
                ++session->db_retry_count;
                retry = true;
            }
        }
        if (retry && QueueTracked(user, response)) {
            return 0;
        }
        if (response.operation == freely_give_item_db::OPERATION_LOAD) {
            TraceFailure("load failed after DB retries", user);
            freely_give_item::ResetSession(user);
            return 0;
        }
        {
            MutexGuard lock(&g_mutex);
            Session* const session = FindSessionUnlocked(user);
            if (session) {
                if (response.operation ==
                    freely_give_item_db::OPERATION_FATIGUE) {
                    const uint64_t total =
                        static_cast<uint64_t>(session->deferred_fatigue) +
                        response.source_value;
                    session->deferred_fatigue = static_cast<uint32_t>(
                        std::min<uint64_t>(total, 0xffffffffULL));
                } else if (response.operation ==
                           freely_give_item_db::OPERATION_LEVEL_UP) {
                    session->deferred_level = std::max<uint32_t>(
                        session->deferred_level, response.source_value);
                } else if (response.operation ==
                           freely_give_item_db::OPERATION_TIMED) {
                    session->deferred_timed_key = response.source_value;
                } else if (response.operation ==
                           freely_give_item_db::OPERATION_MARK_FAILED) {
                    session->deferred_failed_item = response.source_value;
                }
            }
        }
        CompleteCommand(user, false,
                        command ? MapError(command, response.result)
                                : LOCAL_ERROR_GENERAL);
        FlushDeferred(user);
        return 0;
    }

    if (response.operation == freely_give_item_db::OPERATION_LOAD) {
        if (response.result == freely_give_item_db::RESULT_SUCCESS) {
            SendInfoFromSession(user);
            SendNotices(user, response);
            ReconcileLoadedState(user, response);
            if (response.pending_action == ACTION_NONE &&
                response.notice_count == 0u) {
                SendIncrease(user, response.chicken_count,
                             freely_give_item_db::REASON_SILENT_START,
                             0u, response.extension_days,
                             response.countdown_seconds);
            }
        }
        FlushDeferred(user);
        return 0;
    }

    if (response.operation ==
            freely_give_item_db::OPERATION_RESERVE_ACTION) {
        if (response.result != freely_give_item_db::RESULT_SUCCESS) {
            CompleteCommand(user, false,
                            command ? MapError(command, response.result)
                                    : LOCAL_ERROR_GENERAL);
            FlushDeferred(user);
            return 0;
        }
        (void)ApplyReserved(user, response);
        return 0;
    }

    if (response.operation ==
            freely_give_item_db::OPERATION_STAGE_ACTION) {
        if (response.result != freely_give_item_db::RESULT_SUCCESS ||
            response.saga_state !=
                freely_give_item_db::SAGA_STATE_STAGED ||
            !QueueCommitOrCancel(user, response, true,
                                 LOCAL_ERROR_NONE)) {
            TraceFailure("staged action not finalized; pending retained",
                         user);
        }
        return 0;
    }

    if (response.operation ==
            freely_give_item_db::OPERATION_COMMIT_ACTION) {
        if (response.result == freely_give_item_db::RESULT_SUCCESS) {
            CompleteCommand(user, true, 0u);
            SendInfoFromSession(user);
            if (response.action == ACTION_CLAIM_COUPON_LOCAL ||
                response.action == ACTION_CLAIM_COUPON_MAIL) {
                SendIncrease(user, response.chicken_count,
                             freely_give_item_db::REASON_SILENT_RESET,
                             0u, response.extension_days,
                             response.countdown_seconds);
            }
            if (response.action == ACTION_CLAIM_BOX) {
                ReconcileLoadedState(user, response);
            }
        } else {
            CompleteCommand(user, false,
                            command ? MapError(command, response.result)
                                    : LOCAL_ERROR_GENERAL);
        }
        FlushDeferred(user);
        return 0;
    }

    if (response.operation ==
            freely_give_item_db::OPERATION_CANCEL_ACTION) {
        CompleteCommand(user, false,
                        local_error ? local_error : LOCAL_ERROR_GENERAL);
        SendInfoFromSession(user);
        FlushDeferred(user);
        return 0;
    }

    if (response.result == freely_give_item_db::RESULT_SUCCESS) {
        SendInfoFromSession(user);
        SendNotices(user, response);
    }
    FlushDeferred(user);
    return 0;
}
