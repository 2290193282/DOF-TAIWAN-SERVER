#include "old_game/activity/schedule/sea_bottom_event.h"

#include "common/config.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/checksum.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_sea_bottom_event.h"
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

using sea_bottom_event_db::MailReward;
using sea_bottom_event_db::Message;

enum SeaBottomError {
    SEA_ERROR_DATABASE = 18,
    SEA_ERROR_INVALID_ITEM = 23,
    SEA_ERROR_INVALID_DEPTH = 57,
    SEA_ERROR_EVENT_CLOSED = 178,
};

struct BoxRule {
    BoxRule() : item_id(0u), weight(0) {}
    uint32_t item_id;
    int16_t weight;
};

struct RewardRule {
    RewardRule() : key(0u), item_id(0u), count(0u) {}
    uint32_t key;      // normal: weight; specified: exact depth
    uint32_t item_id;
    uint32_t count;
};

struct SeaBottomConfig {
    SeaBottomConfig()
        : maximum_depth(0u), normal_total(0u), boxes(), normal_rewards(),
          specified_rewards(), mail_title_key(), mail_content_key() {}
    uint32_t maximum_depth;
    uint32_t normal_total;
    std::vector<BoxRule> boxes;
    std::vector<RewardRule> normal_rewards;
    std::vector<RewardRule> specified_rewards;
    std::string mail_title_key;
    std::string mail_content_key;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool ExpectTag(const char* expected) {
    std::string tag;
    return expected && ScanType(tag, true) == 1 && tag == expected;
}

bool ReadInteger(int& value) {
    bool success = false;
    value = ScanInt(&success);
    return success;
}

bool ReadTaggedInteger(const char* tag, int& value) {
    return ExpectTag(tag) && ReadInteger(value);
}

bool ReadTaggedString(const char* tag, std::string& value) {
    return ExpectTag(tag) && ScanStr(&value) == 1;
}

bool LoadSeaBottomConfig(SeaBottomConfig& parsed) {
    if (!loadRDARScriptFile("Event/tw_seabottom2000m/",
                            "theseabottom2000m.evt")) {
        return false;
    }
    RdarUnloadGuard unload;
    int maximum_depth = 0;
    if (!ReadTaggedInteger("[max depth]", maximum_depth) ||
        maximum_depth != sea_bottom_event_db::kMaximumDepth ||
        !ExpectTag("[box item list]")) {
        return false;
    }
    parsed.maximum_depth = static_cast<uint32_t>(maximum_depth);
    for (int index = 0; index < 15; ++index) {
        int item_id = 0;
        int weight = 0;
        BoxRule box;
        if (!ReadInteger(item_id) || !ReadInteger(weight) || item_id <= 0 ||
            weight < -10 || weight > 45 || weight == 0) {
            return false;
        }
        box.item_id = static_cast<uint32_t>(item_id);
        box.weight = static_cast<int16_t>(weight);
        parsed.boxes.push_back(box);
    }
    int normal_total = 0;
    if (!ExpectTag("[/box item list]") ||
        !ExpectTag("[normal reward list]") ||
        !ReadInteger(normal_total) || normal_total != 10000) {
        return false;
    }
    parsed.normal_total = static_cast<uint32_t>(normal_total);
    uint32_t normal_sum = 0u;
    for (int index = 0; index < 9; ++index) {
        int weight = 0;
        int item_id = 0;
        int count = 0;
        RewardRule reward;
        if (!ReadInteger(weight) || !ReadInteger(item_id) ||
            !ReadInteger(count) || weight <= 0 || item_id <= 0 ||
            count <= 0) {
            return false;
        }
        reward.key = static_cast<uint32_t>(weight);
        reward.item_id = static_cast<uint32_t>(item_id);
        reward.count = static_cast<uint32_t>(count);
        normal_sum += reward.key;
        parsed.normal_rewards.push_back(reward);
    }
    if (normal_sum != parsed.normal_total ||
        !ExpectTag("[/normal reward list]") ||
        !ExpectTag("[specify reward list]")) {
        return false;
    }
    for (int index = 0; index < sea_bottom_event_db::kSpecifiedRewardCount;
         ++index) {
        int depth = 0;
        int item_id = 0;
        int count = 0;
        RewardRule reward;
        if (!ReadInteger(depth) || !ReadInteger(item_id) ||
            !ReadInteger(count) || depth != (index + 1) * 100 ||
            item_id <= 0 || count <= 0) {
            return false;
        }
        reward.key = static_cast<uint32_t>(depth);
        reward.item_id = static_cast<uint32_t>(item_id);
        reward.count = static_cast<uint32_t>(count);
        parsed.specified_rewards.push_back(reward);
    }
    std::string specified_title;
    std::string specified_content;
    if (!ExpectTag("[/specify reward list]") ||
        !ReadTaggedString("[normal mail title]", parsed.mail_title_key) ||
        !ReadTaggedString("[normal mail content]", parsed.mail_content_key) ||
        !ReadTaggedString("[specify mail title]", specified_title) ||
        !ReadTaggedString("[specify mail content]", specified_content) ||
        parsed.mail_title_key != specified_title ||
        parsed.mail_content_key != specified_content ||
        parsed.mail_title_key != "twdf_under2000M_mail_title01" ||
        parsed.mail_content_key != "twdf_under2000M_mail_contents01") {
        return false;
    }
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;

    static const int32_t expected_boxes[15][2] = {
        {690000854, 1},  {690000855, 2},  {690000856, 4},
        {690000857, 5},  {690000858, 10}, {690000859, 11},
        {690000860, 12}, {690000861, 14}, {690000862, 15},
        {690000863, 20}, {690000864, 30}, {690000865, 45},
        {690000866, -1}, {690000867, -5}, {690000868, -10},
    };
    static const uint32_t expected_normal[9][3] = {
        {1750u, 3108u, 1u},    {499u, 7249u, 1u},
        {499u, 7248u, 1u},     {1750u, 3296u, 1u},
        {1500u, 7252u, 1u},    {1500u, 7251u, 1u},
        {1000u, 2600032u, 1u}, {1500u, 3297u, 1u},
        {2u, 2651113u, 1u},
    };
    static const uint32_t expected_specified[20][2] = {
        {690001042u, 1u}, {15u, 2u},        {690001046u, 1u},
        {15u, 1u},        {10u, 15u},       {21u, 1u},
        {690001047u, 1u}, {15u, 3u},        {690001041u, 1u},
        {7456u, 1u},      {690001044u, 1u}, {690001045u, 1u},
        {900u, 2u},       {690001054u, 1u}, {690000301u, 1u},
        {690000651u, 1u}, {690001054u, 2u}, {690001043u, 1u},
        {21u, 2u},        {690000650u, 1u},
    };
    if (parsed.boxes.size() != 15u || parsed.normal_rewards.size() != 9u ||
        parsed.specified_rewards.size() != 20u) {
        return false;
    }
    for (unsigned int index = 0u; index < 15u; ++index) {
        if (parsed.boxes[index].item_id !=
                static_cast<uint32_t>(expected_boxes[index][0]) ||
            parsed.boxes[index].weight != expected_boxes[index][1]) {
            return false;
        }
    }
    for (unsigned int index = 0u; index < 9u; ++index) {
        const RewardRule& reward = parsed.normal_rewards[index];
        if (reward.key != expected_normal[index][0] ||
            reward.item_id != expected_normal[index][1] ||
            reward.count != expected_normal[index][2]) {
            return false;
        }
    }
    for (unsigned int index = 0u; index < 20u; ++index) {
        const RewardRule& reward = parsed.specified_rewards[index];
        if (reward.item_id != expected_specified[index][0] ||
            reward.count != expected_specified[index][1]) {
            return false;
        }
    }
    return true;
}

class ConfigCache {
public:
    ConfigCache() : attempted_(false), valid_(false), config_() {}

    const SeaBottomConfig* Get() {
        Ensure();
        return valid_ ? &config_ : 0;
    }

private:
    bool IsPrototype(uint32_t item_id, bool stackable) const {
        CItem* const item = G_CDataManager()
            ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
        return item && item->vtable_ &&
               (!stackable || item->is_stackable() == 1);
    }

    void Ensure() {
        if (attempted_) return;
        attempted_ = true;
        valid_ = LoadSeaBottomConfig(config_);
        for (std::vector<BoxRule>::const_iterator box = config_.boxes.begin();
             valid_ && box != config_.boxes.end(); ++box) {
            valid_ = IsPrototype(box->item_id, true);
        }
        for (std::vector<RewardRule>::const_iterator reward =
                 config_.normal_rewards.begin();
             valid_ && reward != config_.normal_rewards.end(); ++reward) {
            valid_ = IsPrototype(reward->item_id, false);
        }
        for (std::vector<RewardRule>::const_iterator reward =
                 config_.specified_rewards.begin();
             valid_ && reward != config_.specified_rewards.end(); ++reward) {
            valid_ = IsPrototype(reward->item_id, false);
        }
    }

    bool attempted_;
    bool valid_;
    SeaBottomConfig config_;
};

ConfigCache& Config() {
    static ConfigCache cache;
    return cache;
}

const BoxRule* FindBox(const SeaBottomConfig& config, uint32_t item_id) {
    for (std::vector<BoxRule>::const_iterator box = config.boxes.begin();
         box != config.boxes.end(); ++box) {
        if (box->item_id == item_id) return &*box;
    }
    return 0;
}

bool SelectRequestReward(const SeaBottomConfig& config,
                         uint32_t target_depth,
                         uint32_t claimed_mask,
                         RewardRule& selected,
                         uint32_t& claimed_mask_after) {
    selected = RewardRule();
    claimed_mask_after = claimed_mask;
    if (target_depth != 0u && target_depth % 100u == 0u) {
        const uint32_t index = target_depth / 100u - 1u;
        if (index < config.specified_rewards.size() &&
            (claimed_mask & (1u << index)) == 0u) {
            selected = config.specified_rewards[index];
            claimed_mask_after |= 1u << index;
            return true;
        }
    }
    // Normal rewards are deliberately unresolved in the game request.  The
    // DB owner locks the character state, performs the official 10000-weight
    // roll, and persists the selected reward in the immutable move journal.
    return config.normal_total == 10000u &&
           config.normal_rewards.size() == 9u;
}

struct Session {
    Session()
        : account_id(0u), charac_no(0u), session_token(0u),
          next_request_token(1u), campaign_key(0u), revision(0u),
          current_depth(0u), claimed_mask(0u), server_group(-1),
          loaded(false), active(false), db_pending(false),
          local_mutation(false), retry_ready(false), recovery_blocked(false),
          db_retry_count(0u), pending_started_epoch(0u),
          retry_after_epoch(0u), pending() {
        std::memset(&pending, 0, sizeof(pending));
    }
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t campaign_key;
    uint32_t revision;
    uint32_t current_depth;
    uint32_t claimed_mask;
    int server_group;
    bool loaded;
    bool active;
    bool db_pending;
    bool local_mutation;
    bool retry_ready;
    bool recovery_blocked;
    uint8_t db_retry_count;
    uint32_t pending_started_epoch;
    uint32_t retry_after_epoch;
    Message pending;
};

Mutex g_mutex;
std::map<CUser*, Session> g_sessions;

uint32_t InitialSessionToken() {
    const uint32_t epoch = static_cast<uint32_t>(std::time(0));
    const uint32_t process = static_cast<uint32_t>(::getpid());
    const uint32_t seeded = epoch ^ (process << 16u) ^ 0xD8320727u;
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

bool HasIdentity(CUser* user) {
    return user && user->get_acc_id() > 0 && CurrentCharacterNo(user) != 0u &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               sea_bottom_event_db::kMaximumServerGroupCount;
}

bool IsTownReady(CUser* user) {
    return HasIdentity(user) && user->get_state() == 3;
}

void TraceFailure(const char* text, CUser* user) {
    cMyTrace trace("CMD727 native Sea Bottom 0828", __LINE__, 5);
    trace("[0828 CMD727] %s account(%u) charac(%u)",
          text ? text : "unknown",
          static_cast<unsigned int>(user && user->get_acc_id() > 0
                                        ? user->get_acc_id() : 0),
          static_cast<unsigned int>(CurrentCharacterNo(user)));
}

bool FillMailReward(uint32_t item_id,
                    uint32_t count,
                    MailReward& destination) {
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
    std::memcpy(destination.item_guid, record.item_guid,
                sizeof(destination.item_guid));
    std::memcpy(destination.random_option, record.random_option,
                sizeof(destination.random_option));
    return destination.item_id == item_id && destination.add_info == count;
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

bool SnapshotSession(CUser* user, Session& snapshot) {
    MutexGuard lock(&g_mutex);
    const std::map<CUser*, Session>::const_iterator found =
        g_sessions.find(user);
    if (found == g_sessions.end()) return false;
    snapshot = found->second;
    return true;
}

bool FillBaseRequest(CUser* user, uint8_t operation, Message& message) {
    if (!HasIdentity(user)) return false;
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
    message.campaign_key = session.campaign_key;
    message.revision = session.revision;
    message.current_depth = session.current_depth;
    message.claimed_mask = session.claimed_mask;
    message.server_group = session.server_group;
    message.operation = operation;
    return true;
}

bool QueueTracked(CUser* user, const Message& message, bool durable_retry) {
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            found->second.session_token != message.session_token) {
            return false;
        }
        found->second.db_pending = true;
        found->second.retry_ready = false;
        found->second.pending_started_epoch =
            static_cast<uint32_t>(std::time(0));
        found->second.pending = message;
    }
    if (sea_bottom_event_db::DBSeaBottomEvent::QueueRequest(
            user->GetUID(), message)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == message.session_token &&
        found->second.pending.request_token == message.request_token) {
        found->second.db_pending = false;
        found->second.retry_ready = durable_retry;
        found->second.pending_started_epoch = 0u;
        found->second.retry_after_epoch =
            static_cast<uint32_t>(std::time(0)) + 60u;
    }
    return false;
}

bool RetryDurable(CUser* user) {
    Message request;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            !found->second.retry_ready) {
            return false;
        }
        request = found->second.pending;
    }
    // SaveInventory enters DB_Q and may take user/inventory locks.  Never call
    // it while holding the Sea Bottom session mutex.
    if ((request.operation ==
             sea_bottom_event_db::OPERATION_PREPARE_MAIL ||
         request.operation == sea_bottom_event_db::OPERATION_COMMIT) &&
        request.local_mutation_done != 0u &&
        (!user || user->SaveInventory() != 1)) {
        return false;
    }
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            !found->second.retry_ready ||
            found->second.session_token != request.session_token ||
            found->second.pending.request_token != request.request_token) {
            return false;
        }
        found->second.db_pending = true;
        found->second.retry_ready = false;
        found->second.pending_started_epoch =
            static_cast<uint32_t>(std::time(0));
    }
    if (sea_bottom_event_db::DBSeaBottomEvent::QueueRequest(
            user->GetUID(), request)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == request.session_token &&
        found->second.pending.request_token == request.request_token) {
        found->second.db_pending = false;
        found->second.retry_ready = true;
        found->second.pending_started_epoch = 0u;
        found->second.retry_after_epoch =
            static_cast<uint32_t>(std::time(0)) + 60u;
    }
    return false;
}

bool MatchResponse(CUser* user,
                   const Message& response,
                   Message& request,
                   uint8_t& retries) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() || !found->second.db_pending ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.session_token != response.session_token ||
        found->second.server_group != response.server_group ||
        found->second.pending.request_token != response.request_token ||
        response.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no != CurrentCharacterNo(user) ||
        response.server_group != user->GetServerGroup()) {
        return false;
    }
    request = found->second.pending;
    found->second.db_pending = false;
    found->second.pending_started_epoch = 0u;
    retries = found->second.db_retry_count;
    if (response.result == sea_bottom_event_db::RESULT_DATABASE_FAILURE) {
        if (found->second.db_retry_count != 0xffu)
            ++found->second.db_retry_count;
    } else {
        found->second.db_retry_count = 0u;
    }
    return true;
}

void ApplySnapshot(CUser* user, const Message& response, bool loaded) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end()) return;
    found->second.loaded = loaded;
    found->second.active = response.active != 0u;
    found->second.campaign_key = response.campaign_key;
    found->second.revision = response.revision;
    found->second.current_depth = response.current_depth;
    found->second.claimed_mask = response.claimed_mask;
    if (loaded) {
        found->second.recovery_blocked = false;
        found->second.retry_after_epoch = 0u;
    } else {
        found->second.retry_after_epoch =
            static_cast<uint32_t>(std::time(0)) + 60u;
    }
}

void SetLocalMutation(CUser* user, bool active) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end()) found->second.local_mutation = active;
}

void BlockRecovery(CUser* user) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end()) {
        found->second.loaded = false;
        found->second.recovery_blocked = true;
    }
}

void SendMoveResult(CUser* user,
                    bool success,
                    uint8_t error,
                    uint16_t target_depth,
                    uint16_t reward_count,
                    uint32_t reward_item_id) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_THE_SEA_BOTTOM2000_USE);
    packet.put_byte(success ? 1 : 0);
    if (success) {
        packet.put_short(target_depth);
        packet.put_short(reward_count);
        packet.put_int(static_cast<int>(reward_item_id));
        packet.put_int(0);  // official client stores but does not consume aux
    } else {
        packet.put_byte(error);
    }
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapError(uint8_t result) {
    if (result == sea_bottom_event_db::RESULT_INVALID_ITEM)
        return SEA_ERROR_INVALID_ITEM;
    if (result == sea_bottom_event_db::RESULT_INVALID_DEPTH)
        return SEA_ERROR_INVALID_DEPTH;
    if (result == sea_bottom_event_db::RESULT_EVENT_CLOSED)
        return SEA_ERROR_EVENT_CLOSED;
    return SEA_ERROR_DATABASE;
}

bool ReadStackAndTotalState(CUser* user,
                            uint16_t slot,
                            uint32_t item_id,
                            bool& source_empty,
                            uint32_t& source_count,
                            uint32_t& total_count) {
    source_empty = false;
    source_count = 0u;
    total_count = 0u;
    if (!user || slot >= oldsrv::config::kMainInventoryItemCount ||
        item_id == 0u || item_id > 0x7fffffffu ||
        user->CheckItemLock(1, static_cast<int>(slot))) {
        return false;
    }
    const CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory) return false;
    for (int index = 0; index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        const Inven_Item* const item = inventory->GetInvenRef(1, index);
        if (!item) return false;
        const int raw_count = item->get_add_info();
        if (item->item_id_ == static_cast<int32_t>(item_id)) {
            if (raw_count <= 0 ||
                total_count > 0x7fffffffu -
                                  static_cast<uint32_t>(raw_count)) {
                return false;
            }
            total_count += static_cast<uint32_t>(raw_count);
        }
        if (index != static_cast<int>(slot)) continue;
        if (item->item_id_ == 0 && raw_count == 0) {
            source_empty = true;
        } else if (item->item_id_ == static_cast<int32_t>(item_id) &&
                   raw_count > 0) {
            source_count = static_cast<uint32_t>(raw_count);
        }
    }
    return true;
}

bool HashInventory(const CInventory* inventory, uint8_t digest[32]) {
    if (!inventory || !digest) return false;
    std::vector<unsigned char> raw(
        static_cast<size_t>(oldsrv::config::kMainInventoryItemCount) *
        sizeof(Inven_Item));
    for (int index = 0; index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        const Inven_Item* const item = inventory->GetInvenRef(1, index);
        if (!item) return false;
        std::memcpy(&raw[static_cast<size_t>(index) * sizeof(Inven_Item)],
                    item, sizeof(Inven_Item));
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               &raw[0], static_cast<int>(raw.size()), digest, 32) ==
           CHECKSUM_RESULT_OK;
}

bool BuildInventoryMutationProof(CUser* user,
                                 uint16_t slot,
                                 uint8_t pre_digest[32],
                                 uint8_t post_digest[32]) {
    const CInventory* const readable = user ? user->getCurCharacInvenR() : 0;
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    return readable && slot < oldsrv::config::kMainInventoryItemCount &&
           HashInventory(readable, pre_digest) && shadow.setCopy(*readable) &&
           shadow.delete_item(static_cast<INVEN_TYPE>(1),
                              static_cast<int>(slot), 1,
                              static_cast<eItemDelReason>(54), false) == 1 &&
           HashInventory(&shadow, post_digest) &&
           std::memcmp(pre_digest, post_digest, 32u) != 0;
}

bool ExactStackAndTotalState(CUser* user,
                             uint16_t slot,
                             uint32_t item_id,
                             uint32_t& source_count,
                             uint32_t& total_count) {
    bool source_empty = false;
    return ReadStackAndTotalState(user, slot, item_id, source_empty,
                                  source_count, total_count) &&
           !source_empty && source_count != 0u &&
           total_count >= source_count;
}

bool IsObservedConsumed(CUser* user, const Message& claim, bool& definite) {
    definite = false;
    if (!user || claim.box_count_before == 0u ||
        claim.box_total_before < claim.box_count_before ||
        claim.source_slot >= oldsrv::config::kMainInventoryItemCount) {
        return false;
    }
    uint8_t current_digest[32] = {};
    if (!HashInventory(user->getCurCharacInvenR(), current_digest)) {
        return false;
    }
    if (std::memcmp(current_digest, claim.inventory_post_sha256, 32u) == 0) {
        definite = true;
        return true;
    }
    if (std::memcmp(current_digest, claim.inventory_pre_sha256, 32u) == 0) {
        definite = true;
        return false;
    }
    return false;
}

bool DeleteExactOne(CUser* user,
                    const Message& claim,
                    bool& inventory_save_queued) {
    inventory_save_queued = false;
    uint32_t source_count = 0u;
    uint32_t total_count = 0u;
    if (!ExactStackAndTotalState(user, claim.source_slot,
                                 claim.box_item_id, source_count,
                                 total_count) ||
        source_count != claim.box_count_before ||
        total_count != claim.box_total_before) {
        return false;
    }
    const CInventory* const readable = user->getCurCharacInvenR();
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    uint8_t current_digest[32] = {};
    uint8_t shadow_digest[32] = {};
    if (!readable || !shadow.setCopy(*readable) ||
        shadow.delete_item(static_cast<INVEN_TYPE>(1),
                           static_cast<int>(claim.source_slot), 1,
                           static_cast<eItemDelReason>(54), false) != 1 ||
        !HashInventory(readable, current_digest) ||
        !HashInventory(&shadow, shadow_digest) ||
        std::memcmp(current_digest, claim.inventory_pre_sha256, 32u) != 0 ||
        std::memcmp(shadow_digest, claim.inventory_post_sha256, 32u) != 0) {
        return false;
    }
    CInventory* const writable = user->getCurCharacInvenW();
    if (!writable) return false;
    SetLocalMutation(user, true);
    const bool deleted = writable->delete_item(
        static_cast<INVEN_TYPE>(1), static_cast<int>(claim.source_slot), 1,
        static_cast<eItemDelReason>(54), true) == 1;
    SetLocalMutation(user, false);
    if (deleted) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             static_cast<int>(claim.source_slot));
        // SIG_SAVE_INVENTORY (35) is submitted before the inner-32 inventory
        // commit wrapper. Signal 823 consumes that lower bound on legacy DB_Q
        // and only then forwards PREPARE_MAIL to the dedicated native queue.
        inventory_save_queued = user->SaveInventory() == 1;
    }
    return deleted;
}

bool FillFollowup(CUser* user,
                  const Message& claim,
                  uint8_t operation,
                  bool consumed,
                  uint8_t recovery_action,
                  Message& request) {
    if (!FillBaseRequest(user, operation, request)) return false;
    request.campaign_key = claim.campaign_key;
    request.revision = claim.revision;
    request.current_depth = claim.current_depth;
    request.target_depth = claim.target_depth;
    request.claimed_mask = claim.claimed_mask;
    request.claimed_mask_after = claim.claimed_mask_after;
    request.box_item_id = claim.box_item_id;
    request.box_count_before = claim.box_count_before;
    request.box_total_before = claim.box_total_before;
    request.reward_item_id = claim.reward_item_id;
    request.reward_count = claim.reward_count;
    request.move_id = claim.move_id;
    request.source_slot = claim.source_slot;
    request.box_weight = claim.box_weight;
    request.local_mutation_done = consumed ? 1u : 0u;
    request.rollover = claim.rollover;
    request.client_reply_expected = claim.client_reply_expected;
    request.recovery_action = recovery_action;
    std::memcpy(request.inventory_pre_sha256, claim.inventory_pre_sha256,
                sizeof(request.inventory_pre_sha256));
    std::memcpy(request.inventory_post_sha256, claim.inventory_post_sha256,
                sizeof(request.inventory_post_sha256));
    if (operation == sea_bottom_event_db::OPERATION_PREPARE_MAIL) {
        const SeaBottomConfig* const config = Config().Get();
        return consumed &&
               config &&
               FillMailReward(claim.reward_item_id, claim.reward_count,
                              request.mail_reward) &&
               CopyResolvedText(request.mail_title,
                                sizeof(request.mail_title),
                                config->mail_title_key) &&
               CopyResolvedText(request.mail_content,
                                sizeof(request.mail_content),
                                config->mail_content_key);
    }
    if (operation == sea_bottom_event_db::OPERATION_COMMIT) {
        if (!consumed || claim.mail_reward.item_id != claim.reward_item_id ||
            claim.mail_reward.add_info != claim.reward_count ||
            claim.mail_title[0] == 0 || claim.mail_content[0] == 0) {
            return false;
        }
        request.mail_reward = claim.mail_reward;
        std::memcpy(request.mail_title, claim.mail_title,
                    sizeof(request.mail_title));
        std::memcpy(request.mail_content, claim.mail_content,
                    sizeof(request.mail_content));
        return true;
    }
    return operation == sea_bottom_event_db::OPERATION_CANCEL && !consumed;
}

bool QueueSnapshot(CUser* user, uint8_t operation) {
    Message request;
    return FillBaseRequest(user, operation, request) &&
           QueueTracked(user, request, false);
}

bool QueueFollowup(CUser* user,
                   const Message& claim,
                   uint8_t operation,
                   bool consumed,
                   uint8_t recovery_action) {
    Message request;
    if (!FillFollowup(user, claim, operation, consumed, recovery_action,
                      request)) {
        return false;
    }
    return QueueTracked(user, request, true);
}

}  // namespace

namespace sea_bottom_event {

void BeginSession(CUser* user) {
    if (!IsTownReady(user) || !Config().Get()) return;
    Session session;
    session.account_id = static_cast<uint32_t>(user->get_acc_id());
    session.charac_no = CurrentCharacterNo(user);
    session.server_group = user->GetServerGroup();
    session.session_token = NextNonZero(g_session_token);
    {
        MutexGuard lock(&g_mutex);
        g_sessions[user] = session;
    }
    if (!QueueSnapshot(user, sea_bottom_event_db::OPERATION_LOAD)) {
        TraceFailure("initial native DB queue load submission failed", user);
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found != g_sessions.end()) {
            found->second.loaded = false;
            found->second.retry_after_epoch =
                static_cast<uint32_t>(std::time(0)) + 60u;
        }
    }
}

void RefreshSession(CUser* user) {
    Session session;
    if (!IsTownReady(user) || !SnapshotSession(user, session) ||
        !session.loaded || session.db_pending || session.local_mutation ||
        session.retry_ready || session.recovery_blocked) {
        return;
    }
    if (!QueueSnapshot(user, sea_bottom_event_db::OPERATION_REFRESH)) {
        TraceFailure("campaign refresh native DB queue submission failed", user);
    }
}

void OnGlobalMinuteTick() {
    const uint32_t now = static_cast<uint32_t>(std::time(0));
    std::vector<CUser*> retry;
    std::vector<CUser*> refresh;
    std::vector<CUser*> reload;
    {
        MutexGuard lock(&g_mutex);
        for (std::map<CUser*, Session>::iterator entry =
                 g_sessions.begin(); entry != g_sessions.end(); ++entry) {
            Session& session = entry->second;
            if (!entry->first) continue;
            if (session.db_pending && session.pending_started_epoch != 0u &&
                now >= session.pending_started_epoch &&
                now - session.pending_started_epoch >= 120u) {
                session.session_token = NextNonZero(g_session_token);
                session.next_request_token = 1u;
                session.loaded = false;
                session.db_pending = false;
                session.local_mutation = false;
                session.retry_ready = false;
                session.recovery_blocked = false;
                session.db_retry_count = 0u;
                session.pending_started_epoch = 0u;
                session.retry_after_epoch = 0u;
                std::memset(&session.pending, 0, sizeof(session.pending));
            }
            if (session.db_pending || session.local_mutation ||
                session.recovery_blocked)
                continue;
            if (session.retry_ready) retry.push_back(entry->first);
            else if (session.loaded) refresh.push_back(entry->first);
            else if (session.retry_after_epoch == 0u ||
                     now >= session.retry_after_epoch) {
                session.retry_after_epoch = now + 60u;
                reload.push_back(entry->first);
            }
        }
    }
    for (std::vector<CUser*>::const_iterator user = retry.begin();
         user != retry.end(); ++user) {
        if (!RetryDurable(*user))
            TraceFailure("durable prepare/commit/cancel retry failed", *user);
    }
    for (std::vector<CUser*>::const_iterator user = refresh.begin();
         user != refresh.end(); ++user) {
        RefreshSession(*user);
    }
    for (std::vector<CUser*>::const_iterator user = reload.begin();
         user != reload.end(); ++user) {
        if (!HasIdentity(*user)) {
            ResetSession(*user);
        } else if (!QueueSnapshot(*user,
                                  sea_bottom_event_db::OPERATION_LOAD)) {
            TraceFailure("lease recovery LOAD submission failed", *user);
        }
    }
}

void ResetSession(CUser* user) {
    MutexGuard lock(&g_mutex);
    g_sessions.erase(user);
}

void FlushAndErase(CUser* user) {
    // The reserved row owns SHA-256 commitments for the complete 352-slot
    // pre/post images.  Reconnect accepts only one of those two images; slot
    // moves or unrelated inventory mutations remain fail-closed.
    ResetSession(user);
}

void SendSeaBottomInfo(CUser* user,
                       uint32_t current_depth,
                       uint32_t claimed_mask) {
    if (!user || current_depth > sea_bottom_event_db::kMaximumDepth ||
        (claimed_mask & ~static_cast<uint32_t>(
                            sea_bottom_event_db::kClaimedMask)) != 0u) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_THE_SEA_BOTTOM2000_INFO);
    packet.put_int(static_cast<int>(current_depth));
    uint32_t count = 0u;
    for (uint32_t bit = 0u;
         bit < sea_bottom_event_db::kSpecifiedRewardCount; ++bit) {
        if ((claimed_mask & (1u << bit)) != 0u) ++count;
    }
    packet.put_int(static_cast<int>(count));
    for (uint32_t bit = 0u;
         bit < sea_bottom_event_db::kSpecifiedRewardCount; ++bit) {
        if ((claimed_mask & (1u << bit)) != 0u)
            packet.put_int(static_cast<int>((bit + 1u) * 100u));
    }
    packet.finalize(true);
    user->Send(packet);
}

bool HasNativeClosure() {
    return true;
}

}  // namespace tw_seabottom_native_0828

Dispatcher_TheSeaBottom2000Use::
    Dispatcher_TheSeaBottom2000Use()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_TheSeaBottom2000Use::
    ~Dispatcher_TheSeaBottom2000Use() {}

int Dispatcher_TheSeaBottom2000Use::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    unsigned short event_field = 0u;
    unsigned short source_slot = 0u;
    unsigned int item_id = 0u;
    if (packet.get_short(event_field) != 1 ||
        packet.get_short(source_slot) != 1 || packet.get_int(item_id) != 1 ||
        event_field != 0u) {
        SendMoveResult(user, false, SEA_ERROR_INVALID_ITEM, 0u, 0u, 0u);
        return 0;
    }
    const SeaBottomConfig* const config = Config().Get();
    const BoxRule* const box = config ? FindBox(*config, item_id) : 0;
    Session session;
    if (!IsTownReady(user) || !config || !box ||
        !SnapshotSession(user, session)) {
        SendMoveResult(user, false, SEA_ERROR_EVENT_CLOSED, 0u, 0u, 0u);
        return 0;
    }
    if (!session.loaded || session.db_pending || session.local_mutation ||
        session.retry_ready || session.recovery_blocked) {
        SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
        return 0;
    }
    if (!session.active) {
        SendMoveResult(user, false, SEA_ERROR_EVENT_CLOSED, 0u, 0u, 0u);
        return 0;
    }
    uint32_t box_count = 0u;
    uint32_t box_total = 0u;
    if (!ExactStackAndTotalState(user, source_slot, item_id, box_count,
                                 box_total)) {
        SendMoveResult(user, false, SEA_ERROR_INVALID_ITEM, 0u, 0u, 0u);
        return 0;
    }
    uint8_t inventory_pre_sha256[32] = {};
    uint8_t inventory_post_sha256[32] = {};
    if (!BuildInventoryMutationProof(user, source_slot,
                                     inventory_pre_sha256,
                                     inventory_post_sha256)) {
        SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
        return 0;
    }
    const int target = static_cast<int>(session.current_depth) + box->weight;
    if (target < 0 || target > sea_bottom_event_db::kMaximumDepth) {
        SendMoveResult(user, false, SEA_ERROR_INVALID_DEPTH, 0u, 0u, 0u);
        return 0;
    }
    RewardRule reward;
    uint32_t claimed_after = 0u;
    if (!SelectRequestReward(*config, static_cast<uint32_t>(target),
                             session.claimed_mask, reward, claimed_after)) {
        SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
        return 0;
    }
    Message request;
    if (!FillBaseRequest(user, sea_bottom_event_db::OPERATION_RESERVE,
                         request)) {
        SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
        return 0;
    }
    request.target_depth = static_cast<uint32_t>(target);
    request.claimed_mask_after = claimed_after;
    request.box_item_id = item_id;
    request.box_count_before = box_count;
    request.box_total_before = box_total;
    request.reward_item_id = reward.item_id;
    request.reward_count = reward.count;
    request.source_slot = source_slot;
    request.box_weight = box->weight;
    request.rollover = target == sea_bottom_event_db::kMaximumDepth ? 1u : 0u;
    request.client_reply_expected = 1u;
    std::memcpy(request.inventory_pre_sha256, inventory_pre_sha256,
                sizeof(request.inventory_pre_sha256));
    std::memcpy(request.inventory_post_sha256, inventory_post_sha256,
                sizeof(request.inventory_post_sha256));
    if (!QueueTracked(user, request, false)) {
        SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
    }
    return 0;
}

int Dispatcher_TheSeaBottom2000Use::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_TheSeaBottom2000Use::process(CUser* user,
                                                       MSG_BASE& message,
                                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

InterSeaBottomEvent::InterSeaBottomEvent()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              sea_bottom_event_db::kTaiwanInnerCode)) {}

InterSeaBottomEvent::~InterSeaBottomEvent() {}

int InterSeaBottomEvent::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    const Message response = *reinterpret_cast<const Message*>(payload);
    if (response.operation < sea_bottom_event_db::OPERATION_LOAD ||
        response.operation >
            sea_bottom_event_db::OPERATION_PREPARE_MAIL ||
        response.result > sea_bottom_event_db::RESULT_RECOVERY_AMBIGUOUS ||
        response.phase > sea_bottom_event_db::PHASE_MAIL_PREPARED ||
        response.active > 1u || response.local_mutation_done > 1u ||
        response.rollover > 1u || response.client_reply_expected > 1u ||
        response.recovery_action > 2u ||
        response.current_depth > sea_bottom_event_db::kMaximumDepth ||
        response.target_depth > sea_bottom_event_db::kMaximumDepth ||
        (response.claimed_mask &
         ~static_cast<uint32_t>(sea_bottom_event_db::kClaimedMask)) != 0u ||
        (response.claimed_mask_after &
         ~static_cast<uint32_t>(sea_bottom_event_db::kClaimedMask)) != 0u) {
        return 0;
    }
    Message request;
    uint8_t retries = 0u;
    if (!MatchResponse(user, response, request, retries)) return 0;

    if (response.result == sea_bottom_event_db::RESULT_DATABASE_FAILURE) {
        if (request.operation ==
                sea_bottom_event_db::OPERATION_PREPARE_MAIL ||
            request.operation == sea_bottom_event_db::OPERATION_COMMIT ||
            request.operation == sea_bottom_event_db::OPERATION_CANCEL) {
            {
                MutexGuard lock(&g_mutex);
                std::map<CUser*, Session>::iterator found =
                    g_sessions.find(user);
                if (found != g_sessions.end()) found->second.retry_ready = true;
            }
            if (retries == 0u)
                TraceFailure("durable follow-up entered retry journal", user);
        } else if (request.operation ==
                   sea_bottom_event_db::OPERATION_RESERVE) {
            SendMoveResult(user, false, SEA_ERROR_DATABASE, 0u, 0u, 0u);
        }
        return 0;
    }

    if (response.operation == sea_bottom_event_db::OPERATION_LOAD ||
        response.operation == sea_bottom_event_db::OPERATION_REFRESH) {
        if (response.result != sea_bottom_event_db::RESULT_SUCCESS) {
            if (response.operation == sea_bottom_event_db::OPERATION_LOAD)
                ApplySnapshot(user, response, false);
            return 0;
        }
        ApplySnapshot(user, response, response.phase ==
                                         sea_bottom_event_db::PHASE_NONE);
        sea_bottom_event::SendSeaBottomInfo(
            user, response.current_depth, response.claimed_mask);
        if (response.phase == sea_bottom_event_db::PHASE_RESERVED) {
            bool definite = false;
            bool consumed = IsObservedConsumed(user, response, definite);
            if (!definite) {
                BlockRecovery(user);
                TraceFailure("ambiguous reserved move; manual evidence required",
                             user);
                return 0;
            }
            bool save_queued = true;
            if (!consumed) {
                // A durable reservation is accepted intent.  If the pre-image
                // survived a crash, re-deduct exactly once instead of
                // cancelling and letting a deliberate crash evade payment.
                consumed = DeleteExactOne(user, response, save_queued);
                if (!consumed || !save_queued) {
                    BlockRecovery(user);
                    TraceFailure(
                        consumed
                            ? "recovered deduction save queue failed"
                            : "recovered pre-image exact deduction failed",
                        user);
                    return 0;
                }
            }
            Message recovery = response;
            recovery.client_reply_expected = 0u;
            if (!QueueFollowup(user, recovery,
                               sea_bottom_event_db::OPERATION_PREPARE_MAIL,
                               true, 1u)) {
                TraceFailure("reserved move recovery queue failed", user);
            }
        } else if (response.phase ==
                   sea_bottom_event_db::PHASE_MAIL_PREPARED) {
            Message recovery = response;
            recovery.client_reply_expected = 0u;
            if (!QueueFollowup(user, recovery,
                               sea_bottom_event_db::OPERATION_COMMIT,
                               true, 1u)) {
                TraceFailure("prepared mail recovery queue failed", user);
            }
        }
        return 0;
    }

    if (response.operation == sea_bottom_event_db::OPERATION_RESERVE) {
        if (response.result != sea_bottom_event_db::RESULT_SUCCESS ||
            response.phase != sea_bottom_event_db::PHASE_RESERVED) {
            SendMoveResult(user, false, MapError(response.result), 0u, 0u, 0u);
            return 0;
        }
        bool save_queued = false;
        const bool consumed = DeleteExactOne(user, response, save_queued);
        if (consumed && !save_queued) {
            BlockRecovery(user);
            TraceFailure("consumed box inventory save queue failed", user);
            return 0;
        }
        if (!QueueFollowup(
                user, response,
                consumed ? sea_bottom_event_db::OPERATION_PREPARE_MAIL
                         : sea_bottom_event_db::OPERATION_CANCEL,
                consumed, 0u)) {
            TraceFailure(consumed ? "post-consume mail prepare journaled"
                                  : "pre-consume cancel queue journaled",
                         user);
        }
        return 0;
    }

    if (response.operation ==
        sea_bottom_event_db::OPERATION_PREPARE_MAIL) {
        if (response.result == sea_bottom_event_db::RESULT_SUCCESS &&
            response.phase ==
                sea_bottom_event_db::PHASE_MAIL_PREPARED) {
            if (!QueueFollowup(user, response,
                               sea_bottom_event_db::OPERATION_COMMIT,
                               true, request.recovery_action)) {
                TraceFailure("frozen mail commit queue journaled", user);
            }
        } else {
            BlockRecovery(user);
            TraceFailure("mail projection prepare failed invariant", user);
        }
        return 0;
    }

    if (response.operation == sea_bottom_event_db::OPERATION_COMMIT) {
        if (response.result == sea_bottom_event_db::RESULT_SUCCESS &&
            response.phase == sea_bottom_event_db::PHASE_DELIVERED &&
            response.reward_count <= 0xffffu &&
            response.target_depth <= 0xffffu) {
            if (request.recovery_action == 1u) {
                // The recovered reservation may belong to a previous
                // campaign.  Do not publish that stale campaign as the live
                // session; immediately reload the transaction-locked gate.
                ApplySnapshot(user, response, false);
                if (!QueueSnapshot(
                        user, sea_bottom_event_db::OPERATION_REFRESH)) {
                    TraceFailure(
                        "post-recovery campaign refresh submission failed",
                        user);
                }
                return 0;
            }
            ApplySnapshot(user, response, true);
            if (request.client_reply_expected != 0u) {
                SendMoveResult(user, true, 0u,
                               static_cast<uint16_t>(response.target_depth),
                               static_cast<uint16_t>(response.reward_count),
                               response.reward_item_id);
            }
            // The client queues state transitions during the treasure
            // animation.  Bottom therefore must be observed before rollover.
            sea_bottom_event::SendSeaBottomInfo(
                user, response.target_depth, response.claimed_mask_after);
            if (response.rollover != 0u) {
                sea_bottom_event::SendSeaBottomInfo(
                    user, 0u, response.claimed_mask_after);
            }
        } else {
            BlockRecovery(user);
            TraceFailure("consumed move failed invariant; kept fail-closed",
                         user);
        }
        return 0;
    }

    if (response.operation == sea_bottom_event_db::OPERATION_CANCEL) {
        if (response.result == sea_bottom_event_db::RESULT_SUCCESS &&
            response.phase == sea_bottom_event_db::PHASE_CANCELLED) {
            ApplySnapshot(user, response, true);
            if (request.client_reply_expected != 0u)
                SendMoveResult(user, false, SEA_ERROR_INVALID_ITEM,
                               0u, 0u, 0u);
        } else {
            BlockRecovery(user);
            TraceFailure("unconsumed move cancellation failed invariant", user);
        }
    }
    return 0;
}
