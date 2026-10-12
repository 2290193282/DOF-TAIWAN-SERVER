#include "old_game/activity/schedule/glass_bottle_event.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_glass_bottle_event.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <ctime>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

using glass_bottle_event_db::MailReward;
using glass_bottle_event_db::Message;

enum CommandError {
    ERROR_DATABASE = 3,
    ERROR_GOLD = 10,
    ERROR_ITEM = 14,
    ERROR_PENDING = 19,
    ERROR_DAILY_LIMIT = 22,
    ERROR_LEVEL = 98,
    ERROR_EVENT_CLOSED = 178,
};

struct ShootingReward {
    ShootingReward()
        : index(0), item_id(0), count(0), price(0), account_once(0),
          character_once(0), daily(0), reward_text(), mail_title(),
          mail_content() {}
    int index;
    int item_id;
    int count;
    int price;
    int account_once;
    int character_once;
    int daily;
    std::string reward_text;
    std::string mail_title;
    std::string mail_content;
};

struct ShootingConfig {
    ShootingConfig()
        : start_hour(0), end_hour(0), base_reward_hour(0), payment_item(0),
          rewards() {}
    int start_hour;
    int end_hour;
    int base_reward_hour;
    int payment_item;
    std::vector<ShootingReward> rewards;
};

struct DimensionConfig {
    DimensionConfig()
        : maximum_clear_count(0), material_item(), material_count(),
          reward_booster(0), mail_title(), mail_content() {}
    int maximum_clear_count;
    std::vector<int> material_item;
    std::vector<int> material_count;
    int reward_booster;
    std::string mail_title;
    std::string mail_content;
};

struct BottleReward {
    BottleReward() : count(0), item_id(0), item_count(0) {}
    int count;
    int item_id;
    int item_count;
};

struct BottleConfig {
    BottleConfig()
        : minimum_sender_level(0), receiver_level_min(0),
          receiver_level_max(0), maximum_send_count(0), send_gold(0),
          send_item_id(0), send_item_count(0), sender_rewards(),
          receiver_item_id(0), receiver_item_count(0), sender_mail_title(),
          sender_mail_content(), receiver_mail_content() {}
    int minimum_sender_level;
    int receiver_level_min;
    int receiver_level_max;
    int maximum_send_count;
    int send_gold;
    int send_item_id;
    int send_item_count;
    std::vector<BottleReward> sender_rewards;
    int receiver_item_id;
    int receiver_item_count;
    std::string sender_mail_title;
    std::string sender_mail_content;
    std::string receiver_mail_content;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool ReadInteger(int& value) {
    bool success = false;
    value = ScanInt(&success);
    return success;
}

bool ExpectTag(const char* expected) {
    std::string tag;
    return expected && ScanType(tag, true) == 1 && tag == expected;
}

bool ReadTaggedInteger(const char* tag, int& value) {
    return ExpectTag(tag) && ReadInteger(value);
}

bool ReadTaggedString(const char* tag, std::string& value) {
    return ExpectTag(tag) && ScanStr(&value) == 1;
}

bool LoadShootingConfig(ShootingConfig& parsed) {
    if (!loadRDARScriptFile("Event/", "tw_shootingstar.evt")) return false;
    RdarUnloadGuard unload;
    if (!ReadTaggedInteger("[shooting start hour]", parsed.start_hour) ||
        !ReadTaggedInteger("[shooting end hour]", parsed.end_hour) ||
        !ReadTaggedInteger("[base reward hour]", parsed.base_reward_hour) ||
        !ReadTaggedInteger("[payment item]", parsed.payment_item)) {
        return false;
    }
    for (int row = 0; row < 9; ++row) {
        ShootingReward reward;
        if (!ExpectTag("[reward]") ||
            !ReadTaggedInteger("[idx]", reward.index) ||
            !ReadTaggedInteger("[item]", reward.item_id) ||
            !ReadTaggedInteger("[count]", reward.count) ||
            !ReadTaggedInteger("[price]", reward.price) ||
            !ReadTaggedInteger("[condition account]", reward.account_once) ||
            !ReadTaggedInteger("[condition charac]", reward.character_once) ||
            !ReadTaggedInteger("[condition daily]", reward.daily) ||
            !ReadTaggedString("[reward str]", reward.reward_text) ||
            !ReadTaggedString("[mail title]", reward.mail_title) ||
            !ReadTaggedString("[mail content]", reward.mail_content) ||
            !ExpectTag("[/reward]")) {
            return false;
        }
        parsed.rewards.push_back(reward);
    }
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;
    static const int expected[9][7] = {
        {1,193,1,1000,1,0,0}, {2,3330,30,300,0,1,1},
        {3,690000776,1,2000,1,0,0}, {4,26503,1,2000,1,0,0},
        {5,8,2,0,0,1,1}, {6,10000541,2,150,0,1,1},
        {7,8082,2,75,0,1,1}, {8,16,1,450,1,0,0},
        {9,690000772,1,1000,0,1,1},
    };
    if (parsed.start_hour != 21 || parsed.end_hour != 23 ||
        parsed.base_reward_hour != 6 || parsed.payment_item != 690000771 ||
        parsed.rewards.size() != 9u) {
        return false;
    }
    for (unsigned int index = 0u; index < 9u; ++index) {
        const ShootingReward& reward = parsed.rewards[index];
        if (reward.index != expected[index][0] ||
            reward.item_id != expected[index][1] ||
            reward.count != expected[index][2] ||
            reward.price != expected[index][3] ||
            reward.account_once != expected[index][4] ||
            reward.character_once != expected[index][5] ||
            reward.daily != expected[index][6] ||
            reward.mail_title != "twdf_shootingstar_mail_title_01" ||
            reward.mail_content != "twdf_shootingstar_mail_text_01") {
            return false;
        }
    }
    return true;
}

bool LoadDimensionConfig(DimensionConfig& parsed) {
    if (!loadRDARScriptFile("Event/", "tw_dimensionpocket.evt")) return false;
    RdarUnloadGuard unload;
    if (!ReadTaggedInteger("[max proper dungeon clear count]",
                           parsed.maximum_clear_count) ||
        !ExpectTag("[material item info list]")) {
        return false;
    }
    for (int index = 0; index < 7; ++index) {
        int item_id = 0;
        int count = 0;
        if (!ReadInteger(item_id) || !ReadInteger(count)) return false;
        parsed.material_item.push_back(item_id);
        parsed.material_count.push_back(count);
    }
    if (!ExpectTag("[/material item info list]") ||
        !ReadTaggedInteger("[reward booster item index]",
                           parsed.reward_booster) ||
        !ReadTaggedString("[mail title]", parsed.mail_title) ||
        !ReadTaggedString("[mail content]", parsed.mail_content)) {
        return false;
    }
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;
    static const int expected_items[7] = {
        3037, 3030, 3027, 3151, 3156, 3142, 3028,
    };
    if (parsed.maximum_clear_count != 5 ||
        parsed.reward_booster != 690000773 ||
        parsed.mail_title != "twdf_MysteyMarble_Result_title" ||
        parsed.mail_content != "twdf_MysteyMarble_Result_text" ||
        parsed.material_item.size() != 7u ||
        parsed.material_count.size() != 7u) {
        return false;
    }
    for (unsigned int index = 0u; index < 7u; ++index) {
        if (parsed.material_item[index] != expected_items[index] ||
            parsed.material_count[index] != 20) {
            return false;
        }
    }
    return true;
}

bool LoadBottleConfig(BottleConfig& parsed) {
    if (!loadRDARScriptFile("Event/", "tw_glassbottleletter.evt")) {
        return false;
    }
    RdarUnloadGuard unload;
    if (!ReadTaggedInteger("[minimum sender level]",
                           parsed.minimum_sender_level) ||
        !ExpectTag("[receiver level range]") ||
        !ReadInteger(parsed.receiver_level_min) ||
        !ReadInteger(parsed.receiver_level_max) ||
        !ReadTaggedInteger("[max send count]", parsed.maximum_send_count) ||
        !ReadTaggedInteger("[send gold]", parsed.send_gold) ||
        !ExpectTag("[send item info]") ||
        !ReadInteger(parsed.send_item_id) ||
        !ReadInteger(parsed.send_item_count) ||
        !ExpectTag("[sender reward item info by count]")) {
        return false;
    }
    bool is_type = false;
    bool success = false;
    std::string token;
    std::vector<int> values;
    for (;;) {
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            if (token != "[/sender reward item info by count]") return false;
            break;
        }
        if (!success || value <= 0 || values.size() >= 42u) return false;
        values.push_back(value);
    }
    if (values.size() != 42u) return false;
    for (unsigned int index = 0u; index < values.size(); index += 3u) {
        BottleReward reward;
        reward.count = values[index];
        reward.item_id = values[index + 1u];
        reward.item_count = values[index + 2u];
        parsed.sender_rewards.push_back(reward);
    }
    if (!ExpectTag("[receiver reward item info]") ||
        !ReadInteger(parsed.receiver_item_id) ||
        !ReadInteger(parsed.receiver_item_count) ||
        !ReadTaggedString("[sender mail title]", parsed.sender_mail_title) ||
        !ReadTaggedString("[sender mail content]",
                          parsed.sender_mail_content) ||
        !ReadTaggedString("[receiver mail content]",
                          parsed.receiver_mail_content)) {
        return false;
    }
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;
    if (parsed.minimum_sender_level != 50 ||
        parsed.receiver_level_min != 20 ||
        parsed.receiver_level_max != 49 ||
        parsed.maximum_send_count != 10 || parsed.send_gold != 10000 ||
        parsed.send_item_id != 690001533 || parsed.send_item_count != 1 ||
        parsed.receiver_item_id != 690001534 ||
        parsed.receiver_item_count != 1 ||
        parsed.sender_mail_title != "twdf_bottleletter_mail_title" ||
        parsed.sender_mail_content != "twdf_bottleletter_mail_text1" ||
        parsed.receiver_mail_content != "twdf_bottleletter_mail_text2" ||
        parsed.sender_rewards.size() != 14u) {
        return false;
    }
    for (unsigned int index = 0u; index < 10u; ++index) {
        const BottleReward& reward = parsed.sender_rewards[index];
        if (reward.count != static_cast<int>(index + 1u) ||
            reward.item_id != 1248 || reward.item_count != 1) {
            return false;
        }
    }
    static const int tenth_items[4] = {
        690001537, 3290, 3311, 690000835,
    };
    for (unsigned int index = 0u; index < 4u; ++index) {
        const BottleReward& reward = parsed.sender_rewards[index + 10u];
        if (reward.count != 10 || reward.item_id != tenth_items[index] ||
            reward.item_count != 1) {
            return false;
        }
    }
    return true;
}

class EventConfigCache {
public:
    EventConfigCache()
        : attempted_(false), shooting_valid_(false),
          dimension_valid_(false), bottle_valid_(false), shooting_(),
          dimension_(), bottle_() {}

    void Ensure() {
        if (attempted_) return;
        attempted_ = true;
        shooting_valid_ = LoadShootingConfig(shooting_);
        dimension_valid_ = LoadDimensionConfig(dimension_);
        bottle_valid_ = LoadBottleConfig(bottle_) && ValidateBottleItems();
    }

    bool shooting_valid() { Ensure(); return shooting_valid_; }
    bool dimension_valid() { Ensure(); return dimension_valid_; }
    bool bottle_valid() { Ensure(); return bottle_valid_; }
    const BottleConfig* bottle() {
        Ensure();
        return bottle_valid_ ? &bottle_ : 0;
    }

private:
    bool IsStackable(int item_id) const {
        CItem* const item = G_CDataManager()
            ? G_CDataManager()->find_item(item_id) : 0;
        return item && item->vtable_ && item->is_stackable() == 1;
    }

    bool ValidateBottleItems() const {
        static const int items[] = {
            690001533, 690001534, 1248, 690001537,
            3290, 3311, 690000835,
        };
        for (unsigned int index = 0u;
             index < sizeof(items) / sizeof(items[0]);
             ++index) {
            if (!IsStackable(items[index])) return false;
        }
        return true;
    }

    bool attempted_;
    bool shooting_valid_;
    bool dimension_valid_;
    bool bottle_valid_;
    ShootingConfig shooting_;
    DimensionConfig dimension_;
    BottleConfig bottle_;
};

EventConfigCache& Config() {
    static EventConfigCache cache;
    return cache;
}

struct Session {
    Session()
        : account_id(0u), charac_no(0u), session_token(0u),
          next_request_token(1u), server_group(-1), loaded(false),
          db_pending(false), local_mutation(false), retry_count(0u),
          local_error(0u), pending() {
        std::memset(&pending, 0, sizeof(pending));
    }
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    int server_group;
    bool loaded;
    bool db_pending;
    bool local_mutation;
    uint8_t retry_count;
    uint8_t local_error;
    Message pending;
};

Mutex g_mutex;
std::map<CUser*, Session> g_sessions;

uint32_t InitialSessionToken() {
    const uint32_t epoch = static_cast<uint32_t>(std::time(0));
    const uint32_t process = static_cast<uint32_t>(::getpid());
    const uint32_t seeded = epoch ^ (process << 16u) ^ 0xD8280724u;
    return seeded == 0u ? 1u : seeded;
}

uint32_t g_session_token = InitialSessionToken();

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
           CurrentCharacterNo(user) != 0u &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               glass_bottle_event_db::kMaximumServerGroupCount;
}

void TraceFailure(const char* text, CUser* user) {
    cMyTrace trace("CMD720/721/724 Taiwan event 0828", __LINE__, 5);
    trace("[0828 Taiwan event] %s account(%u) charac(%u)",
          text ? text : "unknown",
          static_cast<unsigned int>(user && user->get_acc_id() > 0
                                        ? user->get_acc_id() : 0),
          static_cast<unsigned int>(CurrentCharacterNo(user)));
}

void SendCommandResult(CUser* user, bool success, uint8_t error) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SEND_GLASS_BOTTLE_LETTER);
    packet.put_byte(success ? 1 : 0);
    if (!success) packet.put_byte(error);
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapResult(uint8_t result) {
    switch (result) {
        case glass_bottle_event_db::RESULT_LEVEL: return ERROR_LEVEL;
        case glass_bottle_event_db::RESULT_DAILY_LIMIT: return ERROR_DAILY_LIMIT;
        case glass_bottle_event_db::RESULT_PENDING: return ERROR_PENDING;
        case glass_bottle_event_db::RESULT_EVENT_CLOSED:
            return ERROR_EVENT_CLOSED;
        case glass_bottle_event_db::RESULT_INVALID_STATE: return ERROR_ITEM;
        default: return ERROR_DATABASE;
    }
}

bool FillMailReward(uint32_t item_id, MailReward& destination) {
    std::memset(&destination, 0, sizeof(destination));
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    if (!prototype || !prototype->vtable_ ||
        prototype->is_stackable() != 1) {
        return false;
    }
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    item.set_add_info(1);
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
    std::memcpy(destination.item_guid, record.item_guid,
                sizeof(destination.item_guid));
    std::memcpy(destination.random_option, record.random_option,
                sizeof(destination.random_option));
    return destination.item_id == item_id && destination.add_info == 1u;
}

bool CopyResolvedText(char* destination,
                      unsigned int capacity,
                      const std::string& key) {
    if (!destination || capacity < 2u || key.empty()) return false;
    const char* const resolved =
        g_scriptStringManager_.findString(4, key.c_str(), 0);
    if (!resolved || resolved[0] == 0 ||
        std::strlen(resolved) >= capacity) {
        return false;
    }
    std::memset(destination, 0, capacity);
    std::memcpy(destination, resolved, std::strlen(resolved));
    return true;
}

bool FillRewards(Message& message) {
    const BottleConfig* const config = Config().bottle();
    if (!config) return false;
    static const uint32_t item_ids[
        glass_bottle_event_db::kMaximumMailRewardCount] = {
        690001534u, 1248u, 690001537u, 3290u, 3311u, 690000835u,
    };
    for (unsigned int index = 0u;
         index < glass_bottle_event_db::kMaximumMailRewardCount;
         ++index) {
        if (!FillMailReward(item_ids[index], message.rewards[index])) {
            return false;
        }
    }
    message.reward_count = glass_bottle_event_db::kMaximumMailRewardCount;
    return CopyResolvedText(message.mail_title,
                            sizeof(message.mail_title),
                            config->sender_mail_title) &&
           CopyResolvedText(message.sender_mail_content,
                            sizeof(message.sender_mail_content),
                            config->sender_mail_content) &&
           CopyResolvedText(message.receiver_mail_content,
                            sizeof(message.receiver_mail_content),
                            config->receiver_mail_content);
}

bool FillRequest(CUser* user, uint8_t operation, Message& message) {
    if (!IsReady(user) || !G_CEnvironment()) return false;
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
    message.server_group = session.server_group;
    message.operation = operation;
    message.event_active = Config().bottle_valid() ? 1u : 0u;
    const int reset_hour = G_CEnvironment()->get_fatigue_time();
    if (reset_hour < 0 || reset_hour > 23) return false;
    message.daily_reset_hour = static_cast<uint8_t>(reset_hour);
    return true;
}

bool QueueTracked(CUser* user, const Message& message) {
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            found->second.session_token != message.session_token) {
            return false;
        }
        found->second.db_pending = true;
        found->second.pending = message;
    }
    if (glass_bottle_event_db::DBGlassBottleEvent::QueueRequest(
            user->GetUID(), message)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == message.session_token &&
        found->second.pending.request_token == message.request_token) {
        found->second.db_pending = false;
    }
    return false;
}

bool ResponseMatches(CUser* user, const Message& response) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() || !found->second.db_pending ||
        found->second.session_token != response.session_token ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.server_group != response.server_group ||
        found->second.pending.request_token != response.request_token ||
        response.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no != CurrentCharacterNo(user) ||
        response.server_group != user->GetServerGroup()) {
        return false;
    }
    found->second.db_pending = false;
    return true;
}

bool GetSession(CUser* user, Session& destination) {
    MutexGuard lock(&g_mutex);
    const std::map<CUser*, Session>::const_iterator found =
        g_sessions.find(user);
    if (found == g_sessions.end()) return false;
    destination = found->second;
    return true;
}

bool ValidateSource(CUser* user,
                    uint32_t source_slot,
                    const BottleConfig& config,
                    CInventory*& inventory) {
    inventory = user ? user->getCurCharacInvenW() : 0;
    if (!inventory || source_slot >=
                          static_cast<uint32_t>(
                              oldsrv::config::kMainInventoryItemCount)) {
        return false;
    }
    Inven_Item* const item = inventory->GetInvenRef(
        1, static_cast<int>(source_slot));
    return item && item->item_id_ == config.send_item_id &&
           item->get_add_info() >= config.send_item_count &&
           !user->CheckItemLock(1, static_cast<int>(source_slot));
}

bool ApplyLocalMutation(CUser* user,
                        const Message& reserve,
                        uint8_t& error) {
    const BottleConfig* const config = Config().bottle();
    if (!config) { error = ERROR_EVENT_CLOSED; return false; }
    if (!user || user->get_charac_level() < config->minimum_sender_level) {
        error = ERROR_LEVEL;
        return false;
    }
    CInventory* inventory = 0;
    if (!ValidateSource(user, reserve.source_slot, *config, inventory)) {
        error = ERROR_ITEM;
        return false;
    }
    const int money_before = inventory->get_money();
    if (money_before < config->send_gold) {
        error = ERROR_GOLD;
        return false;
    }
    const CInventory* const readable = user->getCurCharacInvenR();
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!readable || !shadow.setCopy(*readable) ||
        shadow.delete_item(
            static_cast<INVEN_TYPE>(1),
            static_cast<int>(reserve.source_slot),
            config->send_item_count,
            static_cast<eItemDelReason>(54), false) != 1) {
        error = ERROR_ITEM;
        return false;
    }
    if (inventory->use_money(config->send_gold,
                             static_cast<eMoneySubReason>(0), true) != 1) {
        error = ERROR_GOLD;
        return false;
    }
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found != g_sessions.end()) found->second.local_mutation = true;
    }
    const bool deleted = inventory->delete_item(
        static_cast<INVEN_TYPE>(1), static_cast<int>(reserve.source_slot),
        config->send_item_count, static_cast<eItemDelReason>(54), true) == 1;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found != g_sessions.end()) found->second.local_mutation = false;
    }
    if (!deleted) {
        (void)inventory->gain_money(config->send_gold,
                                    static_cast<eMoneyAddReason>(0),
                                    true, 0);
        if (inventory->get_money() != money_before) {
            inventory->set_money(static_cast<unsigned int>(money_before));
            TraceFailure("glass-bottle gold rollback repaired", user);
        }
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), 0);
        error = ERROR_ITEM;
        return false;
    }
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0),
                         static_cast<int>(reserve.source_slot));
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0), 0);
    error = 0u;
    return true;
}

bool QueueCancel(CUser* user,
                 const Message& reserve,
                 uint8_t local_error) {
    Message cancel = reserve;
    cancel.operation = glass_bottle_event_db::OPERATION_CANCEL_GLASS_BOTTLE;
    cancel.local_mutation_done = 0u;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end()) return false;
        cancel.request_token = NextNonZero(found->second.next_request_token);
        found->second.local_error = local_error;
    }
    return QueueTracked(user, cancel);
}

}  // namespace

namespace glass_bottle_event {

void BeginSession(CUser* user) {
    if (!IsReady(user)) return;
    Session session;
    session.account_id = static_cast<uint32_t>(user->get_acc_id());
    session.charac_no = CurrentCharacterNo(user);
    session.server_group = user->GetServerGroup();
    session.session_token = NextNonZero(g_session_token);
    {
        MutexGuard lock(&g_mutex);
        g_sessions[user] = session;
    }
    Message request;
    if (!FillRequest(user,
                     glass_bottle_event_db::OPERATION_LOAD_GLASS_BOTTLE,
                     request) ||
        !QueueTracked(user, request)) {
        TraceFailure("glass-bottle load DB_Q submission failed", user);
        ResetSession(user);
    }
}

void ResetSession(CUser* user) {
    MutexGuard lock(&g_mutex);
    g_sessions.erase(user);
}

void FlushAndErase(CUser* user) {
    // DB commits already accepted by DB_Q are reconciled by deterministic
    // postal keys.  No new best-effort mutation is started during teardown.
    ResetSession(user);
}

void SendShootingStarState(CUser* user, uint8_t active) {
    if (!user || active > 1u) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SHOOTINGSTAR_STATE);
    packet.put_byte(active);
    packet.finalize(true);
    user->Send(packet);
}

void SendShootingStarWishList(CUser* user,
                              const ShootingStarWishState* states,
                              uint8_t count) {
    if (!user || count > 9u || (count != 0u && !states)) return;
    for (uint8_t index = 0u; index < count; ++index) {
        if (states[index].reward_index < 1u ||
            states[index].reward_index > 9u ||
            states[index].available > 1u) return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SHOOTINGSTAR_WISH_LIST_STATE);
    packet.put_byte(count);
    for (uint8_t index = 0u; index < count; ++index) {
        packet.put_byte(states[index].reward_index);
        packet.put_byte(states[index].available);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendShootingStarReceiveStar(CUser* user, uint8_t received_count) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SHOOTINGSTAR_RECEIVE_STAR);
    packet.put_byte(received_count);
    packet.finalize(true);
    user->Send(packet);
}

void SendDimensionPocketInfo(CUser* user,
                             uint8_t state,
                             uint8_t proper_clear_count) {
    if (!user || state > 3u ||
        (state == 1u && proper_clear_count > 5u)) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_DIMENSION_POCKET_EVENT_INFO);
    packet.put_byte(state);
    if (state == 1u) packet.put_byte(proper_clear_count);
    packet.finalize(true);
    user->Send(packet);
}

void SendGlassBottleEvent(CUser* user, uint16_t sent_count) {
    if (!user || sent_count > 10u) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GLASS_BOTTLE_LETTER_EVENT);
    packet.put_short(sent_count);
    packet.finalize(true);
    user->Send(packet);
}

bool HasNativeShootingStarClosure() {
    (void)Config().shooting_valid();
    return false;
}

bool HasNativeDimensionPocketClosure() {
    (void)Config().dimension_valid();
    return false;
}

bool HasNativeGlassBottleClosure() {
    // This gate describes implementation ownership, not today's PVF/event
    // activation.  The dispatcher still re-validates the typed PVF cache and
    // returns native error 178 when the required 0828 data is absent/malformed.
    return true;
}

}  // namespace tw_event_0828

Dispatcher_SendGlassBottleLetter::
    Dispatcher_SendGlassBottleLetter()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_SendGlassBottleLetter::~
    Dispatcher_SendGlassBottleLetter() {}

int Dispatcher_SendGlassBottleLetter::dispatch_sig(CUser* user,
                                                         PacketBuf& packet) {
    unsigned int source_slot = 0u;
    if (packet.get_int(source_slot) != 1) {
        SendCommandResult(user, false, ERROR_ITEM);
        return 0;
    }
    const BottleConfig* const config = Config().bottle();
    if (!config) {
        SendCommandResult(user, false, ERROR_EVENT_CLOSED);
        return 0;
    }
    if (!IsReady(user)) {
        SendCommandResult(user, false, ERROR_DATABASE);
        return 0;
    }
    if (user->get_charac_level() < config->minimum_sender_level) {
        SendCommandResult(user, false, ERROR_LEVEL);
        return 0;
    }
    CInventory* inventory = 0;
    if (!ValidateSource(user, source_slot, *config, inventory)) {
        SendCommandResult(user, false, ERROR_ITEM);
        return 0;
    }
    if (inventory->get_money() < config->send_gold) {
        SendCommandResult(user, false, ERROR_GOLD);
        return 0;
    }
    Session session;
    if (!GetSession(user, session) || !session.loaded ||
        session.db_pending) {
        SendCommandResult(user, false, ERROR_PENDING);
        return 0;
    }
    Message request;
    if (!FillRequest(user,
                     glass_bottle_event_db::OPERATION_RESERVE_GLASS_BOTTLE,
                     request) ||
        !FillRewards(request)) {
        SendCommandResult(user, false, ERROR_EVENT_CLOSED);
        return 0;
    }
    request.source_slot = source_slot;
    if (!QueueTracked(user, request)) {
        SendCommandResult(user, false, ERROR_DATABASE);
    }
    return 0;
}

int Dispatcher_SendGlassBottleLetter::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}

int Dispatcher_SendGlassBottleLetter::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

InterGlassBottleEvent::InterGlassBottleEvent()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              glass_bottle_event_db::kTaiwanInnerCode)) {}

InterGlassBottleEvent::~InterGlassBottleEvent() {}

int InterGlassBottleEvent::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(Message))) return 0;
    const Message response = *reinterpret_cast<const Message*>(payload);
    if (response.operation <
            glass_bottle_event_db::OPERATION_LOAD_GLASS_BOTTLE ||
        response.operation >
            glass_bottle_event_db::OPERATION_CANCEL_GLASS_BOTTLE ||
        response.phase > glass_bottle_event_db::PHASE_CANCELLED ||
        response.send_count >
            glass_bottle_event_db::kMaximumGlassBottleSendCount ||
        !ResponseMatches(user, response)) {
        return 0;
    }

    if (response.result == glass_bottle_event_db::RESULT_DATABASE_FAILURE) {
        SendCommandResult(user, false, ERROR_DATABASE);
        TraceFailure("glass-bottle database operation failed", user);
        return 0;
    }
    if (response.operation ==
        glass_bottle_event_db::OPERATION_LOAD_GLASS_BOTTLE) {
        {
            MutexGuard lock(&g_mutex);
            std::map<CUser*, Session>::iterator found = g_sessions.find(user);
            if (found != g_sessions.end()) {
                found->second.loaded =
                    response.result == glass_bottle_event_db::RESULT_SUCCESS;
            }
        }
        if (response.result == glass_bottle_event_db::RESULT_SUCCESS) {
            glass_bottle_event::SendGlassBottleEvent(user, response.send_count);
        }
        return 0;
    }
    if (response.result != glass_bottle_event_db::RESULT_SUCCESS) {
        SendCommandResult(user, false, MapResult(response.result));
        return 0;
    }

    if (response.operation ==
        glass_bottle_event_db::OPERATION_RESERVE_GLASS_BOTTLE) {
        if (response.phase == glass_bottle_event_db::PHASE_DELIVERED) {
            SendCommandResult(user, true, 0u);
            glass_bottle_event::SendGlassBottleEvent(user, response.send_count);
            return 0;
        }
        if (response.phase != glass_bottle_event_db::PHASE_RESERVED ||
            response.claim_id == 0ULL ||
            response.receiver_charac_no == 0u) {
            SendCommandResult(user, false, ERROR_DATABASE);
            return 0;
        }
        uint8_t local_error = 0u;
        if (!ApplyLocalMutation(user, response, local_error)) {
            if (!QueueCancel(user, response, local_error)) {
                SendCommandResult(user, false, local_error);
            }
            return 0;
        }
        Message commit = response;
        commit.operation =
            glass_bottle_event_db::OPERATION_COMMIT_GLASS_BOTTLE;
        commit.local_mutation_done = 1u;
        {
            MutexGuard lock(&g_mutex);
            std::map<CUser*, Session>::iterator found = g_sessions.find(user);
            if (found == g_sessions.end()) return 0;
            commit.request_token = NextNonZero(found->second.next_request_token);
        }
        if (!QueueTracked(user, commit)) {
            // The exact commit frame remains recoverable from the durable
            // reservation, but queue unavailability is not a success ACK.
            SendCommandResult(user, false, ERROR_DATABASE);
            TraceFailure("glass-bottle commit DB_Q submission failed", user);
        }
        return 0;
    }
    if (response.operation ==
        glass_bottle_event_db::OPERATION_COMMIT_GLASS_BOTTLE) {
        if (response.phase != glass_bottle_event_db::PHASE_DELIVERED) {
            SendCommandResult(user, false, ERROR_DATABASE);
            return 0;
        }
        SendCommandResult(user, true, 0u);
        glass_bottle_event::SendGlassBottleEvent(user, response.send_count);
        return 0;
    }
    if (response.operation ==
        glass_bottle_event_db::OPERATION_CANCEL_GLASS_BOTTLE) {
        uint8_t local_error = ERROR_ITEM;
        {
            MutexGuard lock(&g_mutex);
            std::map<CUser*, Session>::iterator found = g_sessions.find(user);
            if (found != g_sessions.end() && found->second.local_error != 0u) {
                local_error = found->second.local_error;
                found->second.local_error = 0u;
            }
        }
        SendCommandResult(user, false, local_error);
    }
    return 0;
}
