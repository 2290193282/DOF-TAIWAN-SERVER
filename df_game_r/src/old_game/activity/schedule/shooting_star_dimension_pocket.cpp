#include "old_game/activity/schedule/shooting_star_dimension_pocket.h"

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
#include "old_data/db/event/db_shooting_star_dimension_pocket.h"
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

using shooting_star_dimension_pocket_db::MailReward;
using shooting_star_dimension_pocket_db::Message;

enum ShootingError {
    SHOOTING_ERROR_PAYMENT = 7,
    SHOOTING_ERROR_PENDING = 19,
    SHOOTING_ERROR_CLOSED_OR_CLAIMED = 95,
};

enum DimensionError {
    DIMENSION_ERROR_ITEM = 17,
    DIMENSION_ERROR_CLOSED = 178,
    DIMENSION_ERROR_STATE = 216,
};

struct ShootingReward {
    ShootingReward()
        : index(0u), item_id(0u), count(0u), price(0u),
          account_once(false), character_daily(false), mail_title(),
          mail_content() {}
    uint8_t index;
    uint32_t item_id;
    uint32_t count;
    uint32_t price;
    bool account_once;
    bool character_daily;
    std::string mail_title;
    std::string mail_content;
};

struct ShootingConfig {
    ShootingConfig()
        : start_hour(0), end_hour(0), base_reward_hour(0), payment_item(0u),
          rewards() {}
    int start_hour;
    int end_hour;
    int base_reward_hour;
    uint32_t payment_item;
    std::vector<ShootingReward> rewards;
};

struct DimensionConfig {
    DimensionConfig()
        : maximum_clear_count(0u), material_items(), reward_booster(0u),
          mail_title(), mail_content() {}
    uint8_t maximum_clear_count;
    std::vector<uint32_t> material_items;
    uint32_t reward_booster;
    std::string mail_title;
    std::string mail_content;
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
    int payment_item = 0;
    if (!ReadTaggedInteger("[shooting start hour]", parsed.start_hour) ||
        !ReadTaggedInteger("[shooting end hour]", parsed.end_hour) ||
        !ReadTaggedInteger("[base reward hour]", parsed.base_reward_hour) ||
        !ReadTaggedInteger("[payment item]", payment_item) ||
        payment_item <= 0) {
        return false;
    }
    parsed.payment_item = static_cast<uint32_t>(payment_item);
    for (int row = 0; row < 9; ++row) {
        int index = 0;
        int item_id = 0;
        int count = 0;
        int price = 0;
        int account_once = 0;
        int character_once = 0;
        int daily = 0;
        std::string reward_text;
        ShootingReward reward;
        if (!ExpectTag("[reward]") ||
            !ReadTaggedInteger("[idx]", index) ||
            !ReadTaggedInteger("[item]", item_id) ||
            !ReadTaggedInteger("[count]", count) ||
            !ReadTaggedInteger("[price]", price) ||
            !ReadTaggedInteger("[condition account]", account_once) ||
            !ReadTaggedInteger("[condition charac]", character_once) ||
            !ReadTaggedInteger("[condition daily]", daily) ||
            !ReadTaggedString("[reward str]", reward_text) ||
            !ReadTaggedString("[mail title]", reward.mail_title) ||
            !ReadTaggedString("[mail content]", reward.mail_content) ||
            !ExpectTag("[/reward]") || index <= 0 || item_id <= 0 ||
            count <= 0 || price < 0) {
            return false;
        }
        reward.index = static_cast<uint8_t>(index);
        reward.item_id = static_cast<uint32_t>(item_id);
        reward.count = static_cast<uint32_t>(count);
        reward.price = static_cast<uint32_t>(price);
        reward.account_once = account_once == 1 && character_once == 0 &&
                              daily == 0;
        reward.character_daily = account_once == 0 && character_once == 1 &&
                                 daily == 1;
        if (!reward.account_once && !reward.character_daily) return false;
        parsed.rewards.push_back(reward);
    }
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;

    static const uint32_t expected[9][4] = {
        {193u, 1u, 1000u, 1u},
        {3330u, 30u, 300u, 0u},
        {690000776u, 1u, 2000u, 1u},
        {26503u, 1u, 2000u, 1u},
        {8u, 2u, 0u, 0u},
        {10000541u, 2u, 150u, 0u},
        {8082u, 2u, 75u, 0u},
        {16u, 1u, 450u, 1u},
        {690000772u, 1u, 1000u, 0u},
    };
    if (parsed.start_hour != 21 || parsed.end_hour != 23 ||
        parsed.base_reward_hour != 6 || parsed.payment_item != 690000771u ||
        parsed.rewards.size() != 9u) {
        return false;
    }
    for (unsigned int index = 0u; index < 9u; ++index) {
        const ShootingReward& reward = parsed.rewards[index];
        if (reward.index != index + 1u ||
            reward.item_id != expected[index][0] ||
            reward.count != expected[index][1] ||
            reward.price != expected[index][2] ||
            reward.account_once != (expected[index][3] != 0u) ||
            reward.character_daily == (expected[index][3] != 0u) ||
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
    int maximum = 0;
    if (!ReadTaggedInteger("[max proper dungeon clear count]", maximum) ||
        maximum <= 0 || !ExpectTag("[material item info list]")) {
        return false;
    }
    parsed.maximum_clear_count = static_cast<uint8_t>(maximum);
    for (int index = 0; index < 7; ++index) {
        int item_id = 0;
        int count = 0;
        if (!ReadInteger(item_id) || !ReadInteger(count) || item_id <= 0 ||
            count != 20) {
            return false;
        }
        parsed.material_items.push_back(static_cast<uint32_t>(item_id));
    }
    int booster = 0;
    if (!ExpectTag("[/material item info list]") ||
        !ReadTaggedInteger("[reward booster item index]", booster) ||
        booster <= 0 ||
        !ReadTaggedString("[mail title]", parsed.mail_title) ||
        !ReadTaggedString("[mail content]", parsed.mail_content)) {
        return false;
    }
    parsed.reward_booster = static_cast<uint32_t>(booster);
    std::string trailing;
    if (ScanType(trailing, true) == 1) return false;
    static const uint32_t expected[7] = {
        3037u, 3030u, 3027u, 3151u, 3156u, 3142u, 3028u,
    };
    if (parsed.maximum_clear_count != 5u ||
        parsed.reward_booster != 690000773u ||
        parsed.mail_title != "twdf_MysteyMarble_Result_title" ||
        parsed.mail_content != "twdf_MysteyMarble_Result_text" ||
        parsed.material_items.size() != 7u) {
        return false;
    }
    for (unsigned int index = 0u; index < 7u; ++index) {
        if (parsed.material_items[index] != expected[index]) return false;
    }
    return true;
}

class ConfigCache {
public:
    ConfigCache()
        : attempted_(false), shooting_valid_(false), dimension_valid_(false),
          shooting_(), dimension_() {}

    const ShootingConfig* shooting() {
        Ensure();
        return shooting_valid_ ? &shooting_ : 0;
    }
    const DimensionConfig* dimension() {
        Ensure();
        return dimension_valid_ ? &dimension_ : 0;
    }

private:
    bool IsPrototype(uint32_t item_id, bool require_stackable) const {
        CItem* const item = G_CDataManager()
            ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
        return item && item->vtable_ &&
               (!require_stackable || item->is_stackable() == 1);
    }

    void Ensure() {
        if (attempted_) return;
        attempted_ = true;
        shooting_valid_ = LoadShootingConfig(shooting_);
        dimension_valid_ = LoadDimensionConfig(dimension_);
        if (shooting_valid_) {
            shooting_valid_ = IsPrototype(shooting_.payment_item, true);
            for (std::vector<ShootingReward>::const_iterator reward =
                     shooting_.rewards.begin();
                 shooting_valid_ && reward != shooting_.rewards.end();
                 ++reward) {
                shooting_valid_ = IsPrototype(reward->item_id, false);
            }
        }
        if (dimension_valid_) {
            dimension_valid_ = IsPrototype(dimension_.reward_booster, true);
            for (std::vector<uint32_t>::const_iterator item =
                     dimension_.material_items.begin();
                 dimension_valid_ && item != dimension_.material_items.end();
                 ++item) {
                dimension_valid_ = IsPrototype(*item, true);
            }
        }
    }

    bool attempted_;
    bool shooting_valid_;
    bool dimension_valid_;
    ShootingConfig shooting_;
    DimensionConfig dimension_;
};

ConfigCache& Config() {
    static ConfigCache cache;
    return cache;
}

const ShootingReward* FindShootingReward(uint8_t reward_index) {
    const ShootingConfig* const config = Config().shooting();
    if (!config || reward_index < 1u ||
        reward_index > config->rewards.size()) {
        return 0;
    }
    const ShootingReward& reward = config->rewards[reward_index - 1u];
    return reward.index == reward_index ? &reward : 0;
}

bool IsDimensionMaterial(uint32_t item_id) {
    const DimensionConfig* const config = Config().dimension();
    if (!config) return false;
    for (std::vector<uint32_t>::const_iterator item =
             config->material_items.begin();
         item != config->material_items.end(); ++item) {
        if (*item == item_id) return true;
    }
    return false;
}

struct Session {
    Session()
        : account_id(0u), charac_no(0u), session_token(0u),
          next_request_token(1u), source_epoch(0u),
          dimension_campaign_key(0u), server_group(-1),
          reset_progress_pending(false), loaded(false),
          db_pending(false), db_pending_recovery(false),
          claim_recovery_required(false), local_mutation(false),
          db_retry_count(0u),
          pending_started_epoch(0u), retry_after_epoch(0u),
          shooting_active(0u), dimension_active(0u), dimension_state(0u),
          dimension_clear_count(0u), shooting_available_mask(0u), pending() {
        std::memset(&pending, 0, sizeof(pending));
    }
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t source_epoch;
    uint32_t dimension_campaign_key;
    int server_group;
    bool reset_progress_pending;
    bool loaded;
    bool db_pending;
    bool db_pending_recovery;
    bool claim_recovery_required;
    bool local_mutation;
    uint8_t db_retry_count;
    uint32_t pending_started_epoch;
    uint32_t retry_after_epoch;
    uint8_t shooting_active;
    uint8_t dimension_active;
    uint8_t dimension_state;
    uint8_t dimension_clear_count;
    uint16_t shooting_available_mask;
    Message pending;
};

Mutex g_mutex;
std::map<CUser*, Session> g_sessions;
typedef std::map<uint64_t, uint32_t> SourceEpochMap;
SourceEpochMap g_source_epoch_overrides;

uint32_t InitialSessionToken() {
    const uint32_t epoch = static_cast<uint32_t>(std::time(0));
    const uint32_t process = static_cast<uint32_t>(::getpid());
    const uint32_t seeded = epoch ^ (process << 16u) ^ 0xD8310721u;
    return seeded == 0u ? 1u : seeded;
}

uint32_t g_session_token = InitialSessionToken();

uint32_t ReadFreshSourceEpoch() {
    const std::time_t now = std::time(0);
    if (now <= 0 || static_cast<unsigned long long>(now) > 0xffffffffULL)
        return 0u;
    return static_cast<uint32_t>(now);
}

uint32_t g_unloaded_source_epoch = ReadFreshSourceEpoch();

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

uint64_t CharacterGenerationKey(CUser* user) {
    return user && user->get_acc_id() > 0 && CurrentCharacterNo(user) != 0u
               ? (static_cast<uint64_t>(
                      static_cast<uint32_t>(user->get_acc_id())) << 32u) |
                     CurrentCharacterNo(user)
               : 0ULL;
}

bool HasIdentity(CUser* user) {
    return user && user->get_acc_id() > 0 && CurrentCharacterNo(user) != 0u &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               shooting_star_dimension_pocket_db::kMaximumServerGroupCount;
}

bool IsTownReady(CUser* user) {
    return HasIdentity(user) && user->get_state() == 3;
}

uint32_t ReadProperClearSerial(CUser* user) {
    const int value = user ? user->GetProperDungeonClearCount() : 0;
    return static_cast<uint32_t>(value < 0 ? 0 :
                                 (value > 0xffff ? 0xffff : value));
}

bool IsClaimOperation(uint8_t operation) {
    return (operation >=
                shooting_star_dimension_pocket_db::OPERATION_SHOOTING_RESERVE &&
            operation <=
                shooting_star_dimension_pocket_db::OPERATION_SHOOTING_CANCEL) ||
           (operation >=
                shooting_star_dimension_pocket_db::
                    OPERATION_DIMENSION_RESERVE_MATERIAL &&
            operation <=
                shooting_star_dimension_pocket_db::
                    OPERATION_DIMENSION_COMMIT_REWARD);
}

void TraceFailure(const char* text, CUser* user) {
    cMyTrace trace("CMD720/721 native Taiwan event 0828", __LINE__, 5);
    trace("[0828 CMD720/721] %s account(%u) charac(%u)",
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

bool FillRequest(CUser* user, uint8_t operation, Message& message) {
    if (!HasIdentity(user) || !G_CEnvironment()) return false;
    std::memset(&message, 0, sizeof(message));
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        found->second.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        found->second.charac_no != CurrentCharacterNo(user) ||
        found->second.server_group != user->GetServerGroup()) {
        return false;
    }
    const int reset_hour = G_CEnvironment()->get_fatigue_time();
    // Both recovered event families are keyed to the official 06:00 day.
    // A server configured to another fatigue boundary is not allowed to
    // silently create a different once/daily ledger.
    if (reset_hour != 6) return false;
    Session& session = found->second;
    message.account_id = session.account_id;
    message.charac_no = session.charac_no;
    message.session_token = session.session_token;
    message.request_token = NextNonZero(session.next_request_token);
    message.server_group = session.server_group;
    message.operation = operation;
    message.daily_reset_hour = static_cast<uint8_t>(reset_hour);
    if (session.source_epoch == 0u) return false;
    // The DB derives the source activity-day from this session-fixed epoch.
    // It must not sample wall clock here: 06:00 can arrive before the main
    // thread executes the authoritative proper-clear reset hook.
    message.activity_day = session.source_epoch;
    if (operation >=
            shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CLEAR &&
        operation <=
            shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_REWARD) {
        message.campaign_key = session.dimension_campaign_key;
    }
    message.source_serial = ReadProperClearSerial(user);
    return true;
}

bool QueueTracked(CUser* user,
                  const Message& message,
                  bool recovery = false) {
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() || found->second.db_pending ||
            found->second.session_token != message.session_token) {
            return false;
        }
        found->second.db_pending = true;
        found->second.db_pending_recovery = recovery;
        found->second.pending_started_epoch =
            static_cast<uint32_t>(std::time(0));
        found->second.pending = message;
    }
    if (shooting_star_dimension_pocket_db::DBShootingStarDimensionPocket::QueueRequest(
            user->GetUID(), message)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == message.session_token &&
        found->second.pending.request_token == message.request_token) {
        found->second.db_pending = false;
        found->second.db_pending_recovery = false;
        found->second.pending_started_epoch = 0u;
        found->second.retry_after_epoch =
            static_cast<uint32_t>(std::time(0)) + 60u;
    }
    return false;
}

bool MatchResponse(CUser* user,
                   const Message& response,
                   Message& request,
                   uint8_t& retry_count,
                   bool& recovery) {
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
    recovery = found->second.db_pending_recovery;
    found->second.db_pending = false;
    found->second.db_pending_recovery = false;
    found->second.pending_started_epoch = 0u;
    if (response.result ==
        shooting_star_dimension_pocket_db::RESULT_DATABASE_FAILURE) {
        retry_count = found->second.db_retry_count;
        if (found->second.db_retry_count < 3u) {
            ++found->second.db_retry_count;
        }
    } else {
        found->second.db_retry_count = 0u;
        retry_count = 0u;
        if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_LOAD ||
            response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_REFRESH) {
            found->second.shooting_active = response.shooting_active;
            found->second.dimension_active = response.dimension_active;
            found->second.shooting_available_mask =
                response.shooting_available_mask;
        } else if (response.operation ==
                   shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT) {
            found->second.shooting_available_mask =
                response.shooting_available_mask;
        } else if (response.operation >=
                       shooting_star_dimension_pocket_db::
                           OPERATION_SHOOTING_RESERVE &&
                   response.operation <=
                       shooting_star_dimension_pocket_db::
                           OPERATION_SHOOTING_CANCEL &&
                   response.result ==
                       shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED) {
            found->second.shooting_active = 0u;
            found->second.shooting_available_mask = 0u;
        }
        if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_LOAD ||
            response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_REFRESH ||
            response.operation >=
                shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CLEAR) {
            found->second.dimension_state = response.dimension_state;
            found->second.dimension_clear_count =
                response.dimension_clear_count;
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED) {
                found->second.dimension_active = 0u;
            }
        }
        if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_LOAD ||
            response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_REFRESH) {
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
                found->second.claim_recovery_required =
                    response.claim_kind !=
                    shooting_star_dimension_pocket_db::CLAIM_NONE;
            }
            if (response.result ==
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS &&
                response.dimension_active == 0u) {
                found->second.dimension_campaign_key = 0u;
                found->second.reset_progress_pending = false;
            } else if (response.result ==
                           shooting_star_dimension_pocket_db::RESULT_SUCCESS &&
                       response.claim_kind !=
                           shooting_star_dimension_pocket_db::
                               CLAIM_SHOOTING_REWARD &&
                       response.campaign_key != 0u) {
                if (found->second.dimension_campaign_key !=
                    response.campaign_key) {
                    // A different campaign is a new baseline.  A reset latch
                    // from the prior run must never backfill the new run.
                    found->second.reset_progress_pending = false;
                }
                found->second.dimension_campaign_key =
                    response.campaign_key;
            }
        } else if (response.operation >=
                   shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CLEAR) {
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED) {
                found->second.dimension_campaign_key = 0u;
            } else if (response.campaign_key != 0u) {
                found->second.dimension_campaign_key = response.campaign_key;
            }
        }
        if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_DIMENSION_SEED &&
            (response.result ==
                 shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
             response.result ==
                 shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED)) {
            found->second.reset_progress_pending = false;
        }
    }
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

bool SetLoaded(CUser* user, bool loaded) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end()) return false;
    found->second.loaded = loaded;
    found->second.retry_after_epoch =
        loaded ? 0u : static_cast<uint32_t>(std::time(0)) + 60u;
    return true;
}

void RequireClaimRecovery(CUser* user) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end()) return;
    found->second.claim_recovery_required = true;
    found->second.loaded = false;
    found->second.local_mutation = false;
    found->second.retry_after_epoch =
        static_cast<uint32_t>(std::time(0)) + 60u;
}

void SetLocalMutation(CUser* user, bool active) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end()) found->second.local_mutation = active;
}

void SendShootingResult(CUser* user,
                        bool success,
                        uint8_t error,
                        uint8_t reward_index,
                        uint32_t item_id,
                        uint8_t count) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SHOOTINGSTAR_BUY_ITEM);
    packet.put_byte(success ? 1 : 0);
    if (success) {
        packet.put_byte(reward_index);
        packet.put_byte(1);
        packet.put_int(static_cast<int>(item_id));
        packet.put_byte(count);
    } else {
        packet.put_byte(error);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendDimensionResult(CUser* user,
                         bool success,
                         uint8_t error,
                         uint8_t mode) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_DIMENSION_POCKET_ITEM);
    packet.put_byte(success ? 1 : 0);
    if (success) packet.put_byte(mode);
    else packet.put_byte(error);
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapShootingError(uint8_t result) {
    using namespace shooting_star_dimension_pocket_db;
    if (result == RESULT_INVALID_ITEM) return SHOOTING_ERROR_PAYMENT;
    if (result == RESULT_EVENT_CLOSED || result == RESULT_ALREADY_CLAIMED)
        return SHOOTING_ERROR_CLOSED_OR_CLAIMED;
    return SHOOTING_ERROR_PENDING;
}

uint8_t MapDimensionError(uint8_t result) {
    using namespace shooting_star_dimension_pocket_db;
    if (result == RESULT_INVALID_ITEM) return DIMENSION_ERROR_ITEM;
    if (result == RESULT_EVENT_CLOSED) return DIMENSION_ERROR_CLOSED;
    return DIMENSION_ERROR_STATE;
}

bool FindPaymentSlot(CUser* user,
                     uint32_t item_id,
                     uint32_t count,
                     uint16_t& slot,
                     uint32_t& source_before_count) {
    slot = 0xffffu;
    source_before_count = 0u;
    if (count == 0u) return true;
    CInventory* const inventory = user ? user->getCurCharacInvenR() : 0;
    if (!inventory) return false;
    for (int index = 0; index < oldsrv::config::kMainInventoryItemCount;
         ++index) {
        const Inven_Item* const item = inventory->GetInvenRef(1, index);
        if (item && item->item_id_ == static_cast<int32_t>(item_id) &&
            item->get_add_info() >= static_cast<int>(count) &&
            !user->CheckItemLock(1, index)) {
            slot = static_cast<uint16_t>(index);
            source_before_count =
                static_cast<uint32_t>(item->get_add_info());
            return true;
        }
    }
    return false;
}

bool ValidateExactStack(CUser* user,
                        uint16_t slot,
                        uint32_t item_id,
                        uint32_t count,
                        uint32_t expected_before_count) {
    if (!user || slot >= oldsrv::config::kMainInventoryItemCount ||
        count == 0u || expected_before_count < count ||
        user->CheckItemLock(1, static_cast<int>(slot))) {
        return false;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    const Inven_Item* const item = inventory
        ? inventory->GetInvenRef(1, static_cast<int>(slot)) : 0;
    return item && item->item_id_ == static_cast<int32_t>(item_id) &&
           item->get_add_info() == static_cast<int>(expected_before_count);
}

bool DeleteExactStack(CUser* user,
                      uint16_t slot,
                      uint32_t item_id,
                      uint32_t count,
                      uint32_t expected_before_count,
                      bool& inventory_save_queued) {
    inventory_save_queued = count == 0u && expected_before_count == 0u;
    if (count == 0u) return inventory_save_queued;
    if (!ValidateExactStack(user, slot, item_id, count,
                            expected_before_count)) {
        return false;
    }
    const CInventory* const readable = user->getCurCharacInvenR();
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!readable || !shadow.setCopy(*readable) ||
        shadow.delete_item(static_cast<INVEN_TYPE>(1),
                           static_cast<int>(slot),
                           static_cast<int>(count),
                           static_cast<eItemDelReason>(54), false) != 1) {
        return false;
    }
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) return false;
    SetLocalMutation(user, true);
    const bool deleted = inventory->delete_item(
        static_cast<INVEN_TYPE>(1), static_cast<int>(slot),
        static_cast<int>(count), static_cast<eItemDelReason>(54), true) == 1;
    SetLocalMutation(user, false);
    if (deleted) {
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             static_cast<int>(slot));
        inventory_save_queued = user->SaveInventory() == 1;
    }
    return deleted;
}

enum MutationImage {
    MUTATION_IMAGE_AMBIGUOUS = 0,
    MUTATION_IMAGE_BEFORE = 1,
    MUTATION_IMAGE_AFTER = 2,
};

MutationImage ObserveMutationImage(CUser* user,
                                   uint16_t slot,
                                   uint32_t item_id,
                                   uint32_t removed_count,
                                   uint32_t source_before_count) {
    if (removed_count == 0u && source_before_count == 0u)
        return MUTATION_IMAGE_AFTER;
    if (!user || slot >= oldsrv::config::kMainInventoryItemCount ||
        removed_count == 0u || source_before_count < removed_count) {
        return MUTATION_IMAGE_AMBIGUOUS;
    }
    const CInventory* const inventory = user->getCurCharacInvenR();
    const Inven_Item* const item = inventory
        ? inventory->GetInvenRef(1, static_cast<int>(slot)) : 0;
    if (item && item->item_id_ == static_cast<int32_t>(item_id)) {
        const int raw_count = item->get_add_info();
        if (raw_count < 0) return MUTATION_IMAGE_AMBIGUOUS;
        const uint32_t current = static_cast<uint32_t>(raw_count);
        if (current == source_before_count) return MUTATION_IMAGE_BEFORE;
        if (source_before_count > removed_count &&
            current == source_before_count - removed_count) {
            return MUTATION_IMAGE_AFTER;
        }
        return MUTATION_IMAGE_AMBIGUOUS;
    }
    return source_before_count == removed_count &&
                   (!item || item->item_id_ == 0)
               ? MUTATION_IMAGE_AFTER : MUTATION_IMAGE_AMBIGUOUS;
}

bool FillShootingClaimMessage(CUser* user,
                              const Message& claim,
                              uint8_t operation,
                              bool local_mutation_done,
                              Message& request) {
    const ShootingReward* const reward =
        FindShootingReward(claim.reward_index);
    if (!reward || !FillRequest(user, operation, request)) return false;
    request.claim_id = claim.claim_id;
    request.campaign_key = claim.campaign_key;
    request.claim_kind =
        shooting_star_dimension_pocket_db::CLAIM_SHOOTING_REWARD;
    request.reward_index = claim.reward_index;
    request.item_id = reward->item_id;
    request.item_count = reward->count;
    request.payment_count = reward->price;
    request.source_before_count = claim.source_before_count;
    request.source_slot = claim.source_slot;
    request.local_mutation_done = local_mutation_done ? 1u : 0u;
    if (operation ==
        shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT) {
        return FillMailReward(reward->item_id, reward->count,
                              request.mail_reward) &&
               CopyResolvedText(request.mail_title,
                                sizeof(request.mail_title),
                                reward->mail_title) &&
               CopyResolvedText(request.mail_content,
                                sizeof(request.mail_content),
                                reward->mail_content);
    }
    return true;
}

bool FillDimensionClaimMessage(CUser* user,
                               const Message& claim,
                               uint8_t operation,
                               bool local_mutation_done,
                               Message& request) {
    const DimensionConfig* const config = Config().dimension();
    if (!config || !FillRequest(user, operation, request)) return false;
    request.claim_id = claim.claim_id;
    request.campaign_key = claim.campaign_key;
    request.claim_kind = claim.claim_kind;
    request.item_id = claim.item_id;
    request.item_count = claim.item_count;
    request.source_before_count = claim.source_before_count;
    request.source_slot = claim.source_slot;
    request.local_mutation_done = local_mutation_done ? 1u : 0u;
    if (operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_REWARD) {
        return claim.item_id == config->reward_booster &&
               claim.item_count == 1u &&
               FillMailReward(config->reward_booster, 1u,
                              request.mail_reward) &&
               CopyResolvedText(request.mail_title,
                                sizeof(request.mail_title),
                                config->mail_title) &&
               CopyResolvedText(request.mail_content,
                                sizeof(request.mail_content),
                                config->mail_content);
    }
    return true;
}

bool QueueRecoveredClaim(CUser* user, const Message& claim) {
    using namespace shooting_star_dimension_pocket_db;
    Message commit;
    if (claim.claim_kind == CLAIM_SHOOTING_REWARD) {
        const ShootingConfig* const config = Config().shooting();
        const ShootingReward* const reward =
            FindShootingReward(claim.reward_index);
        if (!config || !reward || reward->price != claim.payment_count ||
            reward->item_id != claim.item_id ||
            reward->count != claim.item_count) {
            TraceFailure("pending shooting claim/config mismatch", user);
            (void)SetLoaded(user, false);
            return false;
        }
        MutationImage image = ObserveMutationImage(
            user, claim.source_slot, config->payment_item, reward->price,
            claim.source_before_count);
        if (image == MUTATION_IMAGE_BEFORE) {
            if (claim.physical_delivery_present != 0u) {
                bool save_queued = false;
                const bool paid = DeleteExactStack(
                    user, claim.source_slot, config->payment_item,
                    reward->price, claim.source_before_count, save_queued);
                if (!paid || !save_queued ||
                    !FillShootingClaimMessage(
                        user, claim, OPERATION_SHOOTING_COMMIT,
                        reward->price != 0u, commit) ||
                    !QueueTracked(user, commit, true)) {
                    TraceFailure(
                        "delivered shooting recovery payment/commit failed",
                        user);
                    (void)SetLoaded(user, false);
                    return false;
                }
                return true;
            }
            Message cancel;
            if (!FillShootingClaimMessage(
                    user, claim, OPERATION_SHOOTING_CANCEL, false, cancel) ||
                !QueueTracked(user, cancel, true)) {
                TraceFailure("pending shooting exact cancel queue failed",
                             user);
                (void)SetLoaded(user, false);
                return false;
            }
            return true;
        }
        if (image != MUTATION_IMAGE_AFTER ||
            (reward->price != 0u && user->SaveInventory() != 1) ||
            !FillShootingClaimMessage(
                user, claim, OPERATION_SHOOTING_COMMIT,
                reward->price != 0u, commit) ||
            !QueueTracked(user, commit, true)) {
            TraceFailure("pending shooting image ambiguous/commit queue failed",
                         user);
            (void)SetLoaded(user, false);
            return false;
        }
        return true;
    }
    if (claim.claim_kind == CLAIM_DIMENSION_MATERIAL) {
        MutationImage image = ObserveMutationImage(
            user, claim.source_slot, claim.item_id, claim.item_count,
            claim.source_before_count);
        if (image == MUTATION_IMAGE_BEFORE) {
            Message cancel;
            if (!FillDimensionClaimMessage(
                    user, claim, OPERATION_DIMENSION_CANCEL_MATERIAL, false,
                    cancel) ||
                !QueueTracked(user, cancel, true)) {
                TraceFailure(
                    "pending Dimension exact cancel queue failed", user);
                (void)SetLoaded(user, false);
                return false;
            }
            return true;
        }
        if (image != MUTATION_IMAGE_AFTER ||
            user->SaveInventory() != 1 ||
            !FillDimensionClaimMessage(
                user, claim, OPERATION_DIMENSION_COMMIT_MATERIAL, true,
                commit) ||
            !QueueTracked(user, commit, true)) {
            TraceFailure(
                "pending Dimension material image ambiguous/queue failed",
                user);
            (void)SetLoaded(user, false);
            return false;
        }
        return true;
    }
    if (claim.claim_kind == CLAIM_DIMENSION_REWARD &&
        FillDimensionClaimMessage(
            user, claim, OPERATION_DIMENSION_COMMIT_REWARD, false, commit) &&
        QueueTracked(user, commit, true)) {
        return true;
    }
    TraceFailure("pending Dimension reward replay queue failed", user);
    (void)SetLoaded(user, false);
    return false;
}

bool QueueSnapshot(CUser* user, uint8_t operation) {
    Message request;
    return FillRequest(user, operation, request) &&
           QueueTracked(user, request);
}

bool QueueShootingCancel(CUser* user,
                         const Message& claim,
                         bool recovery = false) {
    Message cancel;
    return FillShootingClaimMessage(
               user, claim,
               shooting_star_dimension_pocket_db::OPERATION_SHOOTING_CANCEL,
               false, cancel) &&
           QueueTracked(user, cancel, recovery);
}

bool QueueDimensionCancel(CUser* user,
                          const Message& claim,
                          bool recovery = false) {
    Message cancel;
    return FillDimensionClaimMessage(
               user, claim,
               shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CANCEL_MATERIAL,
               false, cancel) &&
           QueueTracked(user, cancel, recovery);
}

void SendSnapshots(CUser* user, const Message& response) {
    shooting_star_dimension_pocket::SendShootingStarState(
        user, response.shooting_active ? 1u : 0u);
    shooting_star_dimension_pocket::SendShootingStarWishList(
        user, response.shooting_active
                  ? response.shooting_available_mask : 0u);
    shooting_star_dimension_pocket::SendDimensionPocketInfo(
        user, response.dimension_active ? response.dimension_state : 0u,
        response.dimension_clear_count);
}

bool RetryCommit(CUser* user,
                 const Message& request,
                 uint8_t retry_count,
                 bool recovery) {
    if (retry_count >= 3u) return false;
    switch (request.operation) {
        case shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT:
        case shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_MATERIAL:
            if (request.local_mutation_done != 0u &&
                (!user || user->SaveInventory() != 1)) {
                return false;
            }
            return QueueTracked(user, request, recovery);
        case shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_REWARD:
            return QueueTracked(user, request, recovery);
        default:
            return false;
    }
}

}  // namespace

namespace shooting_star_dimension_pocket {

void BeginSession(CUser* user) {
    if (!IsTownReady(user)) return;
    Session session;
    session.account_id = static_cast<uint32_t>(user->get_acc_id());
    session.charac_no = CurrentCharacterNo(user);
    session.server_group = user->GetServerGroup();
    {
        MutexGuard lock(&g_mutex);
        session.session_token = NextNonZero(g_session_token);
        const uint64_t generation_key = CharacterGenerationKey(user);
        SourceEpochMap::iterator override_epoch =
            g_source_epoch_overrides.find(generation_key);
        if (override_epoch != g_source_epoch_overrides.end()) {
            session.source_epoch = override_epoch->second;
            g_source_epoch_overrides.erase(override_epoch);
        } else {
            if (g_unloaded_source_epoch == 0u)
                g_unloaded_source_epoch = ReadFreshSourceEpoch();
            session.source_epoch = g_unloaded_source_epoch;
        }
        if (session.source_epoch == 0u) return;
        g_sessions[user] = session;
    }
    if (!QueueSnapshot(user,
                       shooting_star_dimension_pocket_db::OPERATION_LOAD)) {
        TraceFailure("initial native DB queue load submission failed", user);
        (void)SetLoaded(user, false);
    }
}

void RefreshSession(CUser* user) {
    Session session;
    if (!IsTownReady(user) || !SnapshotSession(user, session) ||
        !session.loaded || session.db_pending || session.local_mutation) {
        return;
    }
    if (!QueueSnapshot(user,
                       shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
        TraceFailure("campaign refresh native DB queue submission failed", user);
    }
}

void OnGlobalMinuteTick() {
    const uint32_t now = static_cast<uint32_t>(std::time(0));
    std::vector<CUser*> refresh_users;
    std::vector<CUser*> reload_users;
    std::vector<CUser*> reset_users;
    {
        MutexGuard lock(&g_mutex);
        for (std::map<CUser*, Session>::iterator entry =
                 g_sessions.begin(); entry != g_sessions.end(); ++entry) {
            Session& session = entry->second;
            if (!entry->first) continue;
            if (session.db_pending && session.pending_started_epoch != 0u &&
                now >= session.pending_started_epoch &&
                now - session.pending_started_epoch >= 120u) {
                // Queue acceptance is not a durable game-side completion.
                // Rotate the generation so a late DISPATCH_Q response cannot
                // bind, then recover every claim/high-water mark from DB.
                session.claim_recovery_required =
                    session.claim_recovery_required ||
                    IsClaimOperation(session.pending.operation);
                session.session_token = NextNonZero(g_session_token);
                session.next_request_token = 1u;
                session.loaded = false;
                session.db_pending = false;
                session.db_pending_recovery = false;
                session.local_mutation = false;
                session.db_retry_count = 0u;
                session.pending_started_epoch = 0u;
                session.retry_after_epoch = 0u;
                std::memset(&session.pending, 0, sizeof(session.pending));
            }
            if (session.db_pending || session.local_mutation) continue;
            if (session.loaded) {
                if (session.reset_progress_pending &&
                    session.dimension_active) {
                    reset_users.push_back(entry->first);
                } else {
                    refresh_users.push_back(entry->first);
                }
            } else if (session.retry_after_epoch == 0u ||
                       now >= session.retry_after_epoch) {
                // Reserve the next retry window before dropping the mutex so
                // the minute hook cannot enqueue duplicate recovery requests.
                session.retry_after_epoch = now + 60u;
                if (session.reset_progress_pending &&
                    session.dimension_active &&
                    !session.claim_recovery_required) {
                    // A claim must be reconstructed before SEED because DB2
                    // rejects progress while pending_claim_id is non-zero.
                    // Non-claim loss can safely replay the reset latch first.
                    reset_users.push_back(entry->first);
                } else {
                    reload_users.push_back(entry->first);
                }
            }
        }
    }
    for (std::vector<CUser*>::const_iterator user = reset_users.begin();
         user != reset_users.end(); ++user) {
        Message request;
        if (!FillRequest(
                *user,
                shooting_star_dimension_pocket_db::OPERATION_DIMENSION_SEED,
                request) || !QueueTracked(*user, request)) {
            TraceFailure("proper-clear reset retry native DB submission failed",
                         *user);
        }
    }
    for (std::vector<CUser*>::const_iterator user = refresh_users.begin();
         user != refresh_users.end(); ++user) {
        RefreshSession(*user);
    }
    for (std::vector<CUser*>::const_iterator user = reload_users.begin();
         user != reload_users.end(); ++user) {
        if (!HasIdentity(*user)) {
            ResetSession(*user);
        } else if (!QueueSnapshot(
                       *user,
                       shooting_star_dimension_pocket_db::OPERATION_LOAD)) {
            TraceFailure("lease recovery LOAD submission failed", *user);
            (void)SetLoaded(*user, false);
        }
    }
}

void ResetSession(CUser* user) {
    MutexGuard lock(&g_mutex);
    g_sessions.erase(user);
}

void FlushAndErase(CUser* user) {
    // Accepted native-queue commits carry deterministic request/delivery keys.
    // Teardown never invents a success or guesses whether a local mutation
    // crossed the non-XA Game/DB boundary.
    ResetSession(user);
}

void OnProperDungeonClear(CUser* user) {
    Session session;
    if (!user || user->get_state() != 5 || !SnapshotSession(user, session) ||
        !session.loaded || !session.dimension_active || session.db_pending ||
        session.local_mutation || session.dimension_state != 1u) {
        return;
    }
    Message request;
    if (!FillRequest(user,
                     session.reset_progress_pending
                         ? shooting_star_dimension_pocket_db::
                               OPERATION_DIMENSION_SEED
                         : shooting_star_dimension_pocket_db::
                               OPERATION_DIMENSION_CLEAR,
                     request) ||
        !QueueTracked(user, request)) {
        TraceFailure("proper-clear native DB queue submission failed", user);
    }
}

void OnProperClearGlobalReset() {
    const uint32_t source_epoch = ReadFreshSourceEpoch();
    if (source_epoch == 0u) return;
    MutexGuard lock(&g_mutex);
    g_unloaded_source_epoch = source_epoch;
    // Overrides are only a bridge from an authoritative pre-session reset to
    // BeginSession.  A newer process-wide reset supersedes every stale bridge.
    g_source_epoch_overrides.clear();
}

void OnProperClearSerialReset(CUser* user) {
    if (!HasIdentity(user)) return;
    const uint32_t source_epoch = ReadFreshSourceEpoch();
    const uint64_t generation_key = CharacterGenerationKey(user);
    if (source_epoch == 0u || generation_key == 0ULL) return;
    bool queue_now = false;
    {
        MutexGuard lock(&g_mutex);
        // This latch advances only after the canonical clear counter reset.
        // New/unloaded sessions therefore cannot combine a new wall-clock day
        // with the previous generation's serial.
        g_unloaded_source_epoch = source_epoch;
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end()) {
            g_source_epoch_overrides[generation_key] = source_epoch;
            return;
        }
        Session& session = found->second;
        if (session.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
            session.charac_no != CurrentCharacterNo(user)) {
            return;
        }
        session.source_epoch = source_epoch;
        // Only a campaign already known active at the authoritative reset may
        // recover the post-reset serial prefix.  An inactive/new campaign is
        // baselined by its next successful LOAD/REFRESH instead.
        session.reset_progress_pending = session.dimension_active != 0u;
        queue_now = session.loaded && session.dimension_active &&
                    !session.db_pending && !session.local_mutation;
    }
    if (!queue_now) {
        return;
    }
    Message request;
    if (!FillRequest(user,
                     shooting_star_dimension_pocket_db::OPERATION_DIMENSION_SEED,
                     request) ||
        !QueueTracked(user, request)) {
        TraceFailure("proper-clear reset seed native DB submission failed", user);
    }
}

void NotifyShootingStarReceived(CUser* user, uint8_t received_count) {
    Session session;
    if (received_count == 0u || !SnapshotSession(user, session) ||
        !session.loaded || !session.shooting_active) {
        return;
    }
    SendShootingStarReceived(user, received_count);
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

void SendShootingStarWishList(CUser* user, uint16_t available_mask) {
    if (!user || (available_mask & ~static_cast<uint16_t>(0x01ffu)) != 0u)
        return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SHOOTINGSTAR_WISH_LIST_STATE);
    packet.put_byte(9);
    for (uint8_t index = 1u; index <= 9u; ++index) {
        packet.put_byte(index);
        packet.put_byte((available_mask &
                         static_cast<uint16_t>(1u << (index - 1u)))
                            ? 1 : 0);
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendShootingStarReceived(CUser* user, uint8_t received_count) {
    if (!user || received_count == 0u) return;
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
        (state == 1u && proper_clear_count > 5u)) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_DIMENSION_POCKET_EVENT_INFO);
    packet.put_byte(state);
    if (state == 1u) packet.put_byte(proper_clear_count);
    packet.finalize(true);
    user->Send(packet);
}

bool HasNativeClosure() {
    return true;
}

}  // namespace tw_event_720_721_native_0828

Dispatcher_ShootingStarBuyItem::Dispatcher_ShootingStarBuyItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_ShootingStarBuyItem::~Dispatcher_ShootingStarBuyItem() {}

int Dispatcher_ShootingStarBuyItem::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    unsigned char reward_index = 0u;
    if (packet.get_byte(reward_index) != 1) {
        SendShootingResult(user, false, SHOOTING_ERROR_PAYMENT, 0u, 0u, 0u);
        return 0;
    }
    const ShootingConfig* const config = Config().shooting();
    const ShootingReward* const reward = FindShootingReward(reward_index);
    Session session;
    if (!IsTownReady(user) || !config || !reward ||
        !SnapshotSession(user, session)) {
        SendShootingResult(user, false, SHOOTING_ERROR_CLOSED_OR_CLAIMED,
                           0u, 0u, 0u);
        return 0;
    }
    if (!session.loaded || session.db_pending || session.local_mutation) {
        SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                           0u, 0u, 0u);
        return 0;
    }
    if (!session.shooting_active ||
        (session.shooting_available_mask &
         static_cast<uint16_t>(1u << (reward_index - 1u))) == 0u) {
        SendShootingResult(user, false, SHOOTING_ERROR_CLOSED_OR_CLAIMED,
                           0u, 0u, 0u);
        return 0;
    }
    MailReward delivery_probe;
    char title_probe[shooting_star_dimension_pocket_db::kMailTitleBytes + 1] = {};
    char content_probe[shooting_star_dimension_pocket_db::kMailContentBytes + 1] = {};
    if (!FillMailReward(reward->item_id, reward->count, delivery_probe) ||
        !CopyResolvedText(title_probe, sizeof(title_probe),
                          reward->mail_title) ||
        !CopyResolvedText(content_probe, sizeof(content_probe),
                          reward->mail_content)) {
        SendShootingResult(user, false, SHOOTING_ERROR_CLOSED_OR_CLAIMED,
                           0u, 0u, 0u);
        return 0;
    }
    uint16_t source_slot = 0xffffu;
    uint32_t source_before_count = 0u;
    if (!FindPaymentSlot(user, config->payment_item, reward->price,
                         source_slot, source_before_count)) {
        SendShootingResult(user, false, SHOOTING_ERROR_PAYMENT,
                           0u, 0u, 0u);
        return 0;
    }
    Message request;
    if (!FillRequest(user,
                     shooting_star_dimension_pocket_db::OPERATION_SHOOTING_RESERVE,
                     request)) {
        SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                           0u, 0u, 0u);
        return 0;
    }
    request.claim_kind =
        shooting_star_dimension_pocket_db::CLAIM_SHOOTING_REWARD;
    request.reward_index = reward_index;
    request.item_id = reward->item_id;
    request.item_count = reward->count;
    request.payment_count = reward->price;
    request.source_before_count = source_before_count;
    request.source_slot = source_slot;
    if (!QueueTracked(user, request)) {
        SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                           0u, 0u, 0u);
    }
    return 0;
}

int Dispatcher_ShootingStarBuyItem::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_ShootingStarBuyItem::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_DimensionPocketItem::Dispatcher_DimensionPocketItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_DimensionPocketItem::~Dispatcher_DimensionPocketItem() {}

int Dispatcher_DimensionPocketItem::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    unsigned char mode = 0u;
    if (packet.get_byte(mode) != 1 || mode > 1u) {
        SendDimensionResult(user, false, DIMENSION_ERROR_ITEM, 0u);
        return 0;
    }
    unsigned short source_slot = 0u;
    if (mode == 0u && packet.get_short(source_slot) != 1) {
        SendDimensionResult(user, false, DIMENSION_ERROR_ITEM, mode);
        return 0;
    }
    const DimensionConfig* const config = Config().dimension();
    Session session;
    if (!IsTownReady(user) || !config || !SnapshotSession(user, session)) {
        SendDimensionResult(user, false, DIMENSION_ERROR_CLOSED, mode);
        return 0;
    }
    if (!session.loaded || session.db_pending || session.local_mutation) {
        SendDimensionResult(user, false, DIMENSION_ERROR_STATE, mode);
        return 0;
    }
    if (!session.dimension_active) {
        SendDimensionResult(user, false, DIMENSION_ERROR_CLOSED, mode);
        return 0;
    }
    Message request;
    if (mode == 0u) {
        if (session.dimension_state != 2u ||
            source_slot >= oldsrv::config::kMainInventoryItemCount) {
            SendDimensionResult(user, false, DIMENSION_ERROR_STATE, mode);
            return 0;
        }
        const CInventory* const inventory = user->getCurCharacInvenR();
        const Inven_Item* const item = inventory
            ? inventory->GetInvenRef(1, static_cast<int>(source_slot)) : 0;
        if (!item || item->item_id_ <= 0 ||
            !IsDimensionMaterial(static_cast<uint32_t>(item->item_id_)) ||
            item->get_add_info() < 20 ||
            user->CheckItemLock(1, static_cast<int>(source_slot)) ||
            !FillRequest(
                user,
                shooting_star_dimension_pocket_db::
                    OPERATION_DIMENSION_RESERVE_MATERIAL,
                request)) {
            SendDimensionResult(user, false, DIMENSION_ERROR_ITEM, mode);
            return 0;
        }
        request.claim_kind =
            shooting_star_dimension_pocket_db::CLAIM_DIMENSION_MATERIAL;
        request.item_id = static_cast<uint32_t>(item->item_id_);
        request.item_count = 20u;
        request.source_before_count =
            static_cast<uint32_t>(item->get_add_info());
        request.source_slot = source_slot;
    } else {
        if (session.dimension_state != 3u ||
            !FillRequest(
                user,
                shooting_star_dimension_pocket_db::
                    OPERATION_DIMENSION_RESERVE_REWARD,
                request)) {
            SendDimensionResult(user, false, DIMENSION_ERROR_STATE, mode);
            return 0;
        }
        MailReward delivery_probe;
        char title_probe[shooting_star_dimension_pocket_db::kMailTitleBytes + 1] = {};
        char content_probe[shooting_star_dimension_pocket_db::kMailContentBytes + 1] = {};
        if (!FillMailReward(config->reward_booster, 1u, delivery_probe) ||
            !CopyResolvedText(title_probe, sizeof(title_probe),
                              config->mail_title) ||
            !CopyResolvedText(content_probe, sizeof(content_probe),
                              config->mail_content)) {
            SendDimensionResult(user, false, DIMENSION_ERROR_CLOSED, mode);
            return 0;
        }
        request.claim_kind =
            shooting_star_dimension_pocket_db::CLAIM_DIMENSION_REWARD;
        request.item_id = config->reward_booster;
        request.item_count = 1u;
    }
    if (!QueueTracked(user, request)) {
        SendDimensionResult(user, false, DIMENSION_ERROR_STATE, mode);
    }
    return 0;
}

int Dispatcher_DimensionPocketItem::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DimensionPocketItem::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

InterShootingStarDimensionPocket::InterShootingStarDimensionPocket()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              shooting_star_dimension_pocket_db::kTaiwanInnerCode)) {}

InterShootingStarDimensionPocket::~InterShootingStarDimensionPocket() {}

int InterShootingStarDimensionPocket::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    const Message response = *reinterpret_cast<const Message*>(payload);
    if (response.operation < shooting_star_dimension_pocket_db::OPERATION_LOAD ||
        response.operation >
            shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_REWARD ||
        response.result > shooting_star_dimension_pocket_db::RESULT_INVALID_ITEM ||
        response.phase > shooting_star_dimension_pocket_db::PHASE_DELIVERED ||
        response.claim_kind >
            shooting_star_dimension_pocket_db::CLAIM_DIMENSION_REWARD ||
        response.dimension_state > 3u ||
        response.dimension_clear_count > 5u ||
        response.physical_delivery_present > 1u ||
        (response.shooting_available_mask & 0xfe00u) != 0u) {
        return 0;
    }
    Message request;
    uint8_t retry_count = 0u;
    bool recovery = false;
    if (!MatchResponse(user, response, request, retry_count, recovery))
        return 0;

    if (response.result ==
        shooting_star_dimension_pocket_db::RESULT_DATABASE_FAILURE) {
        if (RetryCommit(user, request, retry_count, recovery)) return 0;
        TraceFailure("DB operation failed or retry exhausted", user);
        if (IsClaimOperation(request.operation)) {
            RequireClaimRecovery(user);
        } else if (recovery || request.local_mutation_done != 0u) {
            (void)SetLoaded(user, false);
        }
        if (request.operation ==
                shooting_star_dimension_pocket_db::OPERATION_SHOOTING_RESERVE ||
            request.operation ==
                shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT ||
            request.operation ==
                shooting_star_dimension_pocket_db::OPERATION_SHOOTING_CANCEL) {
            SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                               0u, 0u, 0u);
        } else if (request.operation >=
                       shooting_star_dimension_pocket_db::
                           OPERATION_DIMENSION_RESERVE_MATERIAL &&
                   request.operation <=
                       shooting_star_dimension_pocket_db::
                           OPERATION_DIMENSION_COMMIT_REWARD) {
            SendDimensionResult(user, false, DIMENSION_ERROR_STATE,
                                request.claim_kind ==
                                        shooting_star_dimension_pocket_db::
                                            CLAIM_DIMENSION_REWARD
                                    ? 1u : 0u);
        }
        return 0;
    }

    if (response.operation == shooting_star_dimension_pocket_db::OPERATION_LOAD ||
        response.operation == shooting_star_dimension_pocket_db::OPERATION_REFRESH) {
        if (response.result != shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
            if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_LOAD) {
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        (void)SetLoaded(user, true);
        SendSnapshots(user, response);
        if (response.claim_kind !=
                shooting_star_dimension_pocket_db::CLAIM_NONE &&
            response.phase == shooting_star_dimension_pocket_db::PHASE_RESERVED) {
            (void)QueueRecoveredClaim(user, response);
            return 0;
        }
        Session session;
        const bool have_session = SnapshotSession(user, session);
        const bool reset_pending =
            have_session && session.reset_progress_pending &&
            response.dimension_active != 0u;
        if (reset_pending ||
            (response.dimension_active && response.dimension_state == 1u)) {
            Message catch_up;
            if (!FillRequest(
                    user,
                    reset_pending
                        ? shooting_star_dimension_pocket_db::
                              OPERATION_DIMENSION_SEED
                        : shooting_star_dimension_pocket_db::
                              OPERATION_DIMENSION_CLEAR,
                    catch_up) ||
                !QueueTracked(user, catch_up)) {
                TraceFailure(reset_pending
                                 ? "proper-clear reset normalization queue failed"
                                 : "proper-clear high-water catch-up queue failed",
                             user);
            }
        }
        return 0;
    }

    if (response.operation ==
            shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CLEAR ||
        response.operation ==
            shooting_star_dimension_pocket_db::OPERATION_DIMENSION_SEED) {
        if (response.result == shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
            shooting_star_dimension_pocket::SendDimensionPocketInfo(
                user, response.dimension_state,
                response.dimension_clear_count);
        } else if (response.result ==
                   shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED) {
            shooting_star_dimension_pocket::SendDimensionPocketInfo(user, 0u,
                                                                  0u);
        }
        if (response.operation ==
                shooting_star_dimension_pocket_db::OPERATION_DIMENSION_SEED &&
            (response.result ==
                 shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
             response.result ==
                 shooting_star_dimension_pocket_db::RESULT_EVENT_CLOSED)) {
            Session session;
            if (SnapshotSession(user, session) && !session.loaded &&
                !QueueSnapshot(
                    user, shooting_star_dimension_pocket_db::OPERATION_LOAD)) {
                TraceFailure("post-reset recovery LOAD submission failed",
                             user);
                (void)SetLoaded(user, false);
            }
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_SHOOTING_RESERVE) {
        if (response.result != shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_PENDING) {
                RequireClaimRecovery(user);
            }
            SendShootingResult(user, false,
                               MapShootingError(response.result),
                               0u, 0u, 0u);
            return 0;
        }
        const ShootingConfig* const config = Config().shooting();
        const ShootingReward* const reward =
            FindShootingReward(response.reward_index);
        if (!config || !reward || response.phase !=
                shooting_star_dimension_pocket_db::PHASE_RESERVED) {
            RequireClaimRecovery(user);
            SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                               0u, 0u, 0u);
            return 0;
        }
        bool inventory_save_queued = false;
        const bool paid = DeleteExactStack(
            user, response.source_slot, config->payment_item, reward->price,
            response.source_before_count, inventory_save_queued);
        Message next;
        if (paid) {
            if (!inventory_save_queued || !FillShootingClaimMessage(
                    user, response,
                    shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT,
                    reward->price != 0u, next) ||
                !QueueTracked(user, next)) {
                TraceFailure("shooting commit queue failed after payment",
                             user);
                RequireClaimRecovery(user);
                SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                                   0u, 0u, 0u);
            }
        } else if (!QueueShootingCancel(user, response)) {
            TraceFailure("shooting cancel queue failed", user);
            RequireClaimRecovery(user);
            SendShootingResult(user, false, SHOOTING_ERROR_PENDING,
                               0u, 0u, 0u);
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_SHOOTING_COMMIT) {
        const ShootingReward* const reward =
            FindShootingReward(response.reward_index);
        if (recovery) {
            if (response.result !=
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
                response.phase !=
                    shooting_star_dimension_pocket_db::PHASE_DELIVERED ||
                !reward ||
                !QueueSnapshot(
                    user,
                    shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
                TraceFailure("shooting reconnect recovery did not close",
                             user);
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        if (response.result == shooting_star_dimension_pocket_db::RESULT_SUCCESS &&
            response.phase == shooting_star_dimension_pocket_db::PHASE_DELIVERED &&
            reward && reward->count <= 255u) {
            SendShootingResult(user, true, 0u, reward->index,
                               reward->item_id,
                               static_cast<uint8_t>(reward->count));
            shooting_star_dimension_pocket::SendShootingStarWishList(
                user, response.shooting_available_mask);
        } else {
            RequireClaimRecovery(user);
            SendShootingResult(user, false,
                               MapShootingError(response.result),
                               0u, 0u, 0u);
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_SHOOTING_CANCEL) {
        if (recovery) {
            if (response.result !=
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
                !QueueSnapshot(
                    user,
                    shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
                TraceFailure("shooting reconnect cancel did not close", user);
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        if (response.result !=
            shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
            RequireClaimRecovery(user);
        }
        SendShootingResult(user, false,
                           response.result ==
                                   shooting_star_dimension_pocket_db::RESULT_SUCCESS
                               ? SHOOTING_ERROR_PAYMENT
                               : SHOOTING_ERROR_PENDING,
                           0u, 0u, 0u);
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_RESERVE_MATERIAL) {
        if (response.result != shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
            response.phase != shooting_star_dimension_pocket_db::PHASE_RESERVED) {
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_PENDING) {
                RequireClaimRecovery(user);
            } else if (response.result ==
                       shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
                RequireClaimRecovery(user);
            }
            SendDimensionResult(user, false,
                                MapDimensionError(response.result), 0u);
            return 0;
        }
        bool inventory_save_queued = false;
        const bool consumed = DeleteExactStack(
            user, response.source_slot, response.item_id,
            response.item_count, response.source_before_count,
            inventory_save_queued);
        Message next;
        if (consumed) {
            if (!inventory_save_queued || !FillDimensionClaimMessage(
                    user, response,
                    shooting_star_dimension_pocket_db::
                        OPERATION_DIMENSION_COMMIT_MATERIAL,
                    true, next) ||
                !QueueTracked(user, next)) {
                TraceFailure("dimension material commit queue failed", user);
                RequireClaimRecovery(user);
                SendDimensionResult(user, false, DIMENSION_ERROR_STATE, 0u);
            }
        } else if (!QueueDimensionCancel(user, response)) {
            TraceFailure("dimension material cancel queue failed", user);
            RequireClaimRecovery(user);
            SendDimensionResult(user, false, DIMENSION_ERROR_STATE, 0u);
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_MATERIAL) {
        if (recovery) {
            if (response.result !=
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
                response.phase !=
                    shooting_star_dimension_pocket_db::PHASE_DELIVERED ||
                response.dimension_state != 3u ||
                !QueueSnapshot(
                    user,
                    shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
                TraceFailure(
                    "Dimension material reconnect recovery did not close",
                    user);
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        if (response.result == shooting_star_dimension_pocket_db::RESULT_SUCCESS &&
            response.phase == shooting_star_dimension_pocket_db::PHASE_DELIVERED &&
            response.dimension_state == 3u) {
            SendDimensionResult(user, true, 0u, 0u);
            shooting_star_dimension_pocket::SendDimensionPocketInfo(
                user, 3u, response.dimension_clear_count);
        } else {
            RequireClaimRecovery(user);
            SendDimensionResult(user, false,
                                MapDimensionError(response.result), 0u);
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_CANCEL_MATERIAL) {
        if (recovery) {
            if (response.result !=
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
                !QueueSnapshot(
                    user,
                    shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
                TraceFailure("Dimension reconnect cancel did not close",
                             user);
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        if (response.result !=
            shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
            RequireClaimRecovery(user);
        }
        SendDimensionResult(user, false,
                            response.result ==
                                    shooting_star_dimension_pocket_db::RESULT_SUCCESS
                                ? DIMENSION_ERROR_ITEM
                                : DIMENSION_ERROR_STATE,
                            0u);
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_RESERVE_REWARD) {
        if (response.result != shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
            response.phase != shooting_star_dimension_pocket_db::PHASE_RESERVED) {
            if (response.result ==
                shooting_star_dimension_pocket_db::RESULT_PENDING) {
                RequireClaimRecovery(user);
            } else if (response.result ==
                       shooting_star_dimension_pocket_db::RESULT_SUCCESS) {
                RequireClaimRecovery(user);
            }
            SendDimensionResult(user, false,
                                MapDimensionError(response.result), 1u);
            return 0;
        }
        Message commit;
        if (!FillDimensionClaimMessage(
                user, response,
                shooting_star_dimension_pocket_db::
                    OPERATION_DIMENSION_COMMIT_REWARD,
                false, commit) ||
            !QueueTracked(user, commit)) {
            TraceFailure("dimension reward commit queue failed", user);
            RequireClaimRecovery(user);
            SendDimensionResult(user, false, DIMENSION_ERROR_STATE, 1u);
        }
        return 0;
    }

    if (response.operation ==
        shooting_star_dimension_pocket_db::OPERATION_DIMENSION_COMMIT_REWARD) {
        if (recovery) {
            if (response.result !=
                    shooting_star_dimension_pocket_db::RESULT_SUCCESS ||
                response.phase !=
                    shooting_star_dimension_pocket_db::PHASE_DELIVERED ||
                response.dimension_state != 0u ||
                !QueueSnapshot(
                    user,
                    shooting_star_dimension_pocket_db::OPERATION_REFRESH)) {
                TraceFailure(
                    "Dimension reward reconnect recovery did not close",
                    user);
                (void)SetLoaded(user, false);
            }
            return 0;
        }
        if (response.result == shooting_star_dimension_pocket_db::RESULT_SUCCESS &&
            response.phase == shooting_star_dimension_pocket_db::PHASE_DELIVERED &&
            response.dimension_state == 0u) {
            SendDimensionResult(user, true, 0u, 1u);
            shooting_star_dimension_pocket::SendDimensionPocketInfo(user, 0u,
                                                                  0u);
        } else {
            RequireClaimRecovery(user);
            SendDimensionResult(user, false,
                                MapDimensionError(response.result), 1u);
        }
    }
    return 0;
}
