#include "old_game/activity/schedule/blue_horse_event.h"

#include "common/manager/global_data.h"
#include "core/config/env/environment.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_blue_horse_event.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <cstring>
#include <ctime>
#include <limits>
#include <map>
#include <string>
#include <unistd.h>

extern "C++" int GetRandomRange(int minimum, int maximum);

namespace {

using blue_horse_event_db::MailReward;
using blue_horse_event_db::Message;

enum {
    kBlueHorseEventId = 260,
    kBlueHorseTicketFatigue = 8,
    kBlueHorseMaximumDistance = 8,
    kBlueHorseSlotRateTotal = 10000,
};

struct Reward {
    Reward() : item_id(0u), count(0u) {}
    uint32_t item_id;
    uint32_t count;
};

struct BlueHorseConfig {
    BlueHorseConfig()
        : first_show_time(), fatigue_ticket(0u), maximum_distance(0u),
          weekday_reward(), weekend_reward(), combo_mail_title(),
          combo_mail_content(), complete_mail_title(),
          complete_mail_content() {
        for (unsigned int index = 0u; index < 4u; ++index) {
            slot_rate[index] = 0u;
            combo_distance[index] = 0u;
            for (unsigned int repeat = 0u; repeat < 4u; ++repeat) {
                combo_reward[index][repeat] = Reward();
            }
        }
    }

    std::string first_show_time;
    uint32_t fatigue_ticket;
    uint32_t maximum_distance;
    uint32_t combo_distance[4];
    uint32_t slot_rate[4];
    Reward combo_reward[4][4];  // [symbol - 1][repeat]
    Reward weekday_reward;
    Reward weekend_reward;
    std::string combo_mail_title;
    std::string combo_mail_content;
    std::string complete_mail_title;
    std::string complete_mail_content;
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

bool ReadInteger(uint32_t& destination) {
    bool success = false;
    const int value = ScanInt(&success);
    if (!success || value < 0) return false;
    destination = static_cast<uint32_t>(value);
    return true;
}

bool ReadTaggedInteger(const char* tag, uint32_t& destination) {
    return ExpectTag(tag) && ReadInteger(destination);
}

bool ReadTaggedString(const char* tag, std::string& destination) {
    return ExpectTag(tag) && ScanStr(&destination) == 1;
}

bool ReadPairList(const char* open_tag,
                  const char* close_tag,
                  uint32_t* values,
                  unsigned int pair_count) {
    if (!open_tag || !close_tag || !values || !ExpectTag(open_tag)) {
        return false;
    }
    for (unsigned int index = 0u; index < pair_count; ++index) {
        uint32_t key = 0u;
        uint32_t value = 0u;
        if (!ReadInteger(key) || !ReadInteger(value) ||
            key != index + 1u) {
            return false;
        }
        values[index] = value;
    }
    return ExpectTag(close_tag);
}

bool ReadComboRewards(BlueHorseConfig& parsed) {
    if (!ExpectTag("[combo reward info]")) return false;
    static const uint32_t kRepeatOrder[8] = {
        3u, 3u, 3u, 3u, 2u, 2u, 2u, 2u,
    };
    for (unsigned int row = 0u; row < 8u; ++row) {
        uint32_t symbol = 0u;
        uint32_t repeat = 0u;
        Reward reward;
        if (!ReadInteger(symbol) || !ReadInteger(repeat) ||
            !ReadInteger(reward.item_id) || !ReadInteger(reward.count) ||
            symbol != row % 4u + 1u || repeat != kRepeatOrder[row] ||
            reward.item_id == 0u || reward.count == 0u) {
            return false;
        }
        parsed.combo_reward[symbol - 1u][repeat] = reward;
    }
    return ExpectTag("[/combo reward info]");
}

bool ValidateConfig(const BlueHorseConfig& config) {
    static const uint32_t kRates[4] = {1000u, 2500u, 4500u, 1500u};
    static const uint32_t kTriple[4][2] = {
        {690001157u, 1u}, {900u, 2u}, {15u, 1u}, {42u, 10u},
    };
    static const uint32_t kPair[4][2] = {
        {690001155u, 1u}, {8u, 5u}, {42u, 1u}, {690001156u, 1u},
    };
    if (config.first_show_time != "2014-01-08 09:00:00" ||
        config.fatigue_ticket != kBlueHorseTicketFatigue ||
        config.maximum_distance != kBlueHorseMaximumDistance ||
        config.weekday_reward.item_id != 690001154u ||
        config.weekday_reward.count != 3u ||
        config.weekend_reward.item_id != 690001154u ||
        config.weekend_reward.count != 8u ||
        config.combo_mail_title != "tw_evt_combo_title" ||
        config.combo_mail_content != "tw_evt_combo_text" ||
        config.complete_mail_title != "tw_evt_weekend_title" ||
        config.complete_mail_content != "tw_evt_weekend_text") {
        return false;
    }
    uint32_t rate_total = 0u;
    for (unsigned int index = 0u; index < 4u; ++index) {
        if (config.slot_rate[index] != kRates[index] ||
            config.combo_distance[index] !=
                (index < 3u ? index + 1u : 0u) ||
            config.combo_reward[index][3].item_id != kTriple[index][0] ||
            config.combo_reward[index][3].count != kTriple[index][1] ||
            config.combo_reward[index][2].item_id != kPair[index][0] ||
            config.combo_reward[index][2].count != kPair[index][1]) {
            return false;
        }
        rate_total += config.slot_rate[index];
    }
    return rate_total == kBlueHorseSlotRateTotal;
}

class ConfigCache {
public:
    ConfigCache() : attempted_(false), valid_(false), config_() {}

    const BlueHorseConfig* Get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = Load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool Load() {
        if (!loadRDARScriptFile("Event/", "tw_bluehorse.evt")) {
            return false;
        }
        RdarUnloadGuard unload;
        BlueHorseConfig parsed;
        if (!ReadTaggedString("[check time for first show ui]",
                              parsed.first_show_time) ||
            !ReadTaggedInteger("[fatigue ticket]", parsed.fatigue_ticket) ||
            !ReadTaggedInteger("[max distance]", parsed.maximum_distance) ||
            !ReadPairList("[combo distance info]",
                          "[/combo distance info]",
                          parsed.combo_distance, 3u) ||
            !ReadPairList("[combo slot rate info]",
                          "[/combo slot rate info]",
                          parsed.slot_rate, 4u) ||
            !ReadComboRewards(parsed) ||
            !ReadTaggedInteger("[weekday complete distance reward]",
                               parsed.weekday_reward.item_id) ||
            !ReadInteger(parsed.weekday_reward.count) ||
            !ReadTaggedInteger("[weekend complete distance reward]",
                               parsed.weekend_reward.item_id) ||
            !ReadInteger(parsed.weekend_reward.count) ||
            !ReadTaggedString("[combo reward mail title]",
                              parsed.combo_mail_title) ||
            !ReadTaggedString("[combo reward mail content]",
                              parsed.combo_mail_content) ||
            !ReadTaggedString("[complete distance reward mail title]",
                              parsed.complete_mail_title) ||
            !ReadTaggedString("[complete distance reward mail content]",
                              parsed.complete_mail_content)) {
            return false;
        }
        std::string trailing;
        if (ScanType(trailing, true) == 1 || !ValidateConfig(parsed)) {
            return false;
        }
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    BlueHorseConfig config_;
};

ConfigCache& Config() {
    static ConfigCache cache;
    return cache;
}

struct Session {
    Session()
        : account_id(0u), charac_no(0u), session_token(0u),
          next_request_token(1u), queued_fatigue(0u), server_group(-1),
          ticket_count(0u), fatigue_remainder(0u), distance(0u),
          completed(0u), loaded(false), db_pending(false), pending() {
        slots[0] = slots[1] = slots[2] = 0u;
        std::memset(&pending, 0, sizeof(pending));
    }

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t queued_fatigue;
    int server_group;
    uint8_t ticket_count;
    uint8_t fatigue_remainder;
    uint8_t distance;
    uint8_t slots[blue_horse_event_db::kSlotCount];
    uint8_t completed;
    bool loaded;
    bool db_pending;
    Message pending;
};

Mutex g_mutex;
std::map<CUser*, Session> g_sessions;
BlueHorseEvent* g_blue_horse_event_sidecar = 0;

uint32_t InitialSessionToken() {
    const uint32_t epoch = static_cast<uint32_t>(std::time(0));
    const uint32_t process = static_cast<uint32_t>(::getpid());
    const uint32_t seeded = epoch ^ (process << 16u) ^ 0xD8290731u;
    return seeded == 0u ? 1u : seeded;
}

uint32_t g_session_token = InitialSessionToken();

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) result = value++;
    if (value == 0u) ++value;
    return result;
}

uint32_t SaturatingAdd(uint32_t left, uint32_t right) {
    const uint32_t maximum = std::numeric_limits<uint32_t>::max();
    return maximum - left < right ? maximum : left + right;
}

uint32_t CurrentCharacterNo(CUser* user) {
    return user && user->getCurCharacR() && user->getCurCharacNo() > 0
               ? static_cast<uint32_t>(user->getCurCharacNo()) : 0u;
}

bool HasIdentity(CUser* user) {
    return user && user->get_acc_id() > 0 && CurrentCharacterNo(user) != 0u &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               blue_horse_event_db::kMaximumServerGroupCount;
}

bool IsBlueHorseEventActive(CUser* user) {
    CEventManager* const manager = GlobalData::s_event_manager;
    CEventBase* const event = manager
                                  ? manager->GetRepeatEvent(kBlueHorseEventId)
                                  : 0;
    return event && event->IsEventing(user) == 1;
}

bool IsTownReady(CUser* user) {
    return HasIdentity(user) && user->get_state() == 3 &&
           IsBlueHorseEventActive(user);
}

bool IdentityMatches(CUser* user, const Session& session) {
    return HasIdentity(user) &&
           session.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           session.charac_no == CurrentCharacterNo(user) &&
           session.server_group == user->GetServerGroup();
}

void TraceFailure(const char* text, CUser* user) {
    cMyTrace trace("CMD730/731 Blue Horse 0828", __LINE__, 5);
    trace("[0828 Blue Horse] %s account(%u) charac(%u)",
          text ? text : "unknown",
          static_cast<unsigned int>(user && user->get_acc_id() > 0
                                        ? user->get_acc_id() : 0),
          static_cast<unsigned int>(CurrentCharacterNo(user)));
}

bool InitializeRequest(Session& session,
                       uint8_t operation,
                       Message& message) {
    CEnvironment* const environment = G_CEnvironment();
    const int reset_hour = environment
                               ? environment->get_fatigue_time()
                               : -1;
    if (reset_hour < 0 || reset_hour > 23) return false;
    std::memset(&message, 0, sizeof(message));
    message.account_id = session.account_id;
    message.charac_no = session.charac_no;
    message.session_token = session.session_token;
    message.request_token = NextNonZero(session.next_request_token);
    message.server_group = session.server_group;
    message.operation = operation;
    message.event_active = 1u;
    message.daily_reset_hour = static_cast<uint8_t>(reset_hour);
    return true;
}

bool BuildRequest(CUser* user, uint8_t operation, Message& message) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() ||
        !IdentityMatches(user, found->second) ||
        found->second.db_pending) {
        return false;
    }
    return InitializeRequest(found->second, operation, message);
}

bool QueueTracked(CUser* user, const Message& message) {
    if (!user) return false;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() ||
            !IdentityMatches(user, found->second) ||
            found->second.db_pending ||
            found->second.session_token != message.session_token) {
            return false;
        }
        found->second.db_pending = true;
        found->second.pending = message;
    }
    if (blue_horse_event_db::DBBlueHorseEvent::QueueRequest(
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

bool TryQueueDeferredFatigue(CUser* user) {
    if (!user) return false;
    Message request;
    uint32_t submitted = 0u;
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() ||
            !IdentityMatches(user, found->second) ||
            !found->second.loaded || found->second.db_pending ||
            found->second.queued_fatigue == 0u) {
            return false;
        }
        Session& session = found->second;
        submitted = session.queued_fatigue > 10000u
                        ? 10000u : session.queued_fatigue;
        session.queued_fatigue -= submitted;
        if (!InitializeRequest(
                session,
                blue_horse_event_db::OPERATION_ADD_BLUE_HORSE_FATIGUE,
                request)) {
            session.queued_fatigue = SaturatingAdd(
                session.queued_fatigue, submitted);
            return false;
        }
        request.fatigue_delta = submitted;
        session.db_pending = true;
        session.pending = request;
    }
    if (blue_horse_event_db::DBBlueHorseEvent::QueueRequest(
            user->GetUID(), request)) {
        return true;
    }
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found != g_sessions.end() &&
        found->second.session_token == request.session_token &&
        found->second.pending.request_token == request.request_token) {
        found->second.db_pending = false;
        found->second.queued_fatigue = SaturatingAdd(
            found->second.queued_fatigue, submitted);
    }
    return false;
}

bool SnapshotValid(const Message& response) {
    if (response.fatigue_remainder >= kBlueHorseTicketFatigue ||
        response.distance > kBlueHorseMaximumDistance ||
        response.completed > 1u) {
        return false;
    }
    for (unsigned int index = 0u;
         index < blue_horse_event_db::kSlotCount;
         ++index) {
        if (response.slots[index] > 4u) return false;
    }
    return true;
}

bool ResponseEnvelopeValid(const Message& response) {
    if (response.operation <
            blue_horse_event_db::OPERATION_LOAD_BLUE_HORSE ||
        response.operation >
            blue_horse_event_db::OPERATION_COMMIT_BLUE_HORSE_REWARD ||
        response.result > blue_horse_event_db::RESULT_EVENT_CLOSED ||
        response.phase > blue_horse_event_db::PHASE_DELIVERED ||
        response.show_ui > 1u) {
        return false;
    }
    if (response.result != blue_horse_event_db::RESULT_SUCCESS) {
        return response.show_ui == 0u;
    }
    if (response.operation !=
            blue_horse_event_db::OPERATION_LOAD_BLUE_HORSE &&
        response.show_ui != 0u) {
        return false;
    }
    if (!SnapshotValid(response)) return false;
    if (response.operation ==
        blue_horse_event_db::OPERATION_RESERVE_BLUE_HORSE_REWARD) {
        return response.claim_id != 0ULL &&
               response.reward_charac_no != 0u &&
               response.phase == blue_horse_event_db::PHASE_RESERVED;
    }
    if (response.operation ==
        blue_horse_event_db::OPERATION_COMMIT_BLUE_HORSE_REWARD) {
        return response.claim_id != 0ULL &&
               response.reward_charac_no != 0u &&
               response.phase == blue_horse_event_db::PHASE_DELIVERED;
    }
    return response.phase == blue_horse_event_db::PHASE_NONE;
}

bool ConsumeResponse(CUser* user,
                     const Message& response,
                     Session& snapshot) {
    MutexGuard lock(&g_mutex);
    std::map<CUser*, Session>::iterator found = g_sessions.find(user);
    if (found == g_sessions.end() || !found->second.db_pending ||
        !IdentityMatches(user, found->second) ||
        found->second.pending.operation != response.operation ||
        found->second.pending.request_token != response.request_token ||
        found->second.session_token != response.session_token ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.server_group != response.server_group) {
        return false;
    }
    Session& session = found->second;
    const Message pending = session.pending;
    session.db_pending = false;
    if (response.result == blue_horse_event_db::RESULT_SUCCESS) {
        session.ticket_count = response.ticket_count;
        session.fatigue_remainder = response.fatigue_remainder;
        session.distance = response.distance;
        session.completed = response.completed;
        for (unsigned int index = 0u;
             index < blue_horse_event_db::kSlotCount;
             ++index) {
            session.slots[index] = response.slots[index];
        }
        if (response.operation ==
            blue_horse_event_db::OPERATION_LOAD_BLUE_HORSE) {
            session.loaded = true;
        }
    } else if (pending.operation ==
               blue_horse_event_db::OPERATION_ADD_BLUE_HORSE_FATIGUE) {
        session.queued_fatigue = SaturatingAdd(
            session.queued_fatigue, pending.fatigue_delta);
    }
    snapshot = session;
    return true;
}

bool GetSession(CUser* user, Session& destination) {
    MutexGuard lock(&g_mutex);
    const std::map<CUser*, Session>::const_iterator found =
        g_sessions.find(user);
    if (found == g_sessions.end() ||
        !IdentityMatches(user, found->second)) {
        return false;
    }
    destination = found->second;
    return true;
}

void SendCommand730(CUser* user, bool success, uint8_t result) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_PLAY_BLUE_HORSE_SLOT);
    packet.put_byte(success ? 1 : 0);
    if (success) packet.put_byte(result);
    packet.finalize(true);
    user->Send(packet);
}

void SendCommand731(CUser* user,
                    bool success,
                    uint8_t combo_reward,
                    uint8_t complete_reward) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REWARD_BLUE_HORSE_SLOT);
    packet.put_byte(success ? 1 : 0);
    if (success) {
        packet.put_byte(combo_reward);
        packet.put_byte(complete_reward);
    }
    packet.finalize(true);
    user->Send(packet);
}

uint8_t DrawSlot(const BlueHorseConfig& config) {
    const int roll = GetRandomRange(1, kBlueHorseSlotRateTotal);
    uint32_t cumulative = 0u;
    for (uint8_t index = 0u; index < 4u; ++index) {
        cumulative += config.slot_rate[index];
        if (roll <= static_cast<int>(cumulative)) return index + 1u;
    }
    return 0u;
}

bool FillMailReward(uint32_t item_id,
                    uint32_t item_count,
                    MailReward& destination) {
    std::memset(&destination, 0, sizeof(destination));
    if (item_id == 0u || item_count == 0u ||
        item_count > static_cast<uint32_t>(
                         std::numeric_limits<int>::max())) {
        return false;
    }
    CItem* const prototype = G_CDataManager()
        ? G_CDataManager()->find_item(static_cast<int>(item_id)) : 0;
    if (!prototype || !prototype->vtable_ ||
        prototype->is_stackable() != 1) {
        return false;
    }
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = static_cast<int32_t>(item_id);
    item.set_add_info(static_cast<int>(item_count));
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
    return destination.item_id == item_id &&
           destination.add_info == item_count;
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

bool PrepareCommit(CUser* user,
                   const Message& reserve,
                   Message& commit) {
    const BlueHorseConfig* const config = Config().Get();
    if (!config || reserve.phase != blue_horse_event_db::PHASE_RESERVED ||
        reserve.claim_id == 0ULL || reserve.reward_charac_no == 0u ||
        !BuildRequest(
            user,
            blue_horse_event_db::OPERATION_COMMIT_BLUE_HORSE_REWARD,
            commit)) {
        return false;
    }
    commit.claim_id = reserve.claim_id;
    commit.reward_charac_no = reserve.reward_charac_no;
    commit.phase = blue_horse_event_db::PHASE_RESERVED;
    commit.combo_item_id = reserve.combo_item_id;
    commit.combo_item_count = reserve.combo_item_count;
    commit.complete_item_id = reserve.complete_item_id;
    commit.complete_item_count = reserve.complete_item_count;
    commit.combo_reward_sent = reserve.combo_item_id != 0u ? 1u : 0u;
    commit.complete_reward_sent =
        reserve.complete_item_id != 0u ? 1u : 0u;
    unsigned int reward_index = 0u;
    if (commit.combo_reward_sent != 0u) {
        if (!FillMailReward(commit.combo_item_id,
                            commit.combo_item_count,
                            commit.rewards[reward_index++]) ||
            !CopyResolvedText(commit.combo_mail_title,
                              sizeof(commit.combo_mail_title),
                              config->combo_mail_title) ||
            !CopyResolvedText(commit.combo_mail_content,
                              sizeof(commit.combo_mail_content),
                              config->combo_mail_content)) {
            return false;
        }
    }
    if (commit.complete_reward_sent != 0u) {
        if (!FillMailReward(commit.complete_item_id,
                            commit.complete_item_count,
                            commit.rewards[reward_index++]) ||
            !CopyResolvedText(commit.complete_mail_title,
                              sizeof(commit.complete_mail_title),
                              config->complete_mail_title) ||
            !CopyResolvedText(commit.complete_mail_content,
                              sizeof(commit.complete_mail_content),
                              config->complete_mail_content)) {
            return false;
        }
    }
    commit.reward_count = static_cast<uint8_t>(reward_index);
    return true;
}

void SendSnapshot(CUser* user,
                  const Session& session,
                  uint8_t show_ui) {
    blue_horse_event::SendBlueHorseData(
        user, show_ui, session.ticket_count, session.slots,
        session.distance, session.completed);
}

}  // namespace

namespace blue_horse_event {

bool InstallBlueHorseEventSidecar(CEventBase* event) {
    if (!event) return false;
    if (g_blue_horse_event_sidecar == event) return true;
    if (g_blue_horse_event_sidecar) return false;
    g_blue_horse_event_sidecar = static_cast<BlueHorseEvent*>(event);
    return true;
}

CEventBase* GetBlueHorseEventSidecar() {
    return g_blue_horse_event_sidecar;
}

void DestroyBlueHorseEventSidecar() {
    delete g_blue_horse_event_sidecar;
    g_blue_horse_event_sidecar = 0;
}

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
    Message request;
    if (!BuildRequest(user,
                      blue_horse_event_db::OPERATION_LOAD_BLUE_HORSE,
                      request) ||
        !QueueTracked(user, request)) {
        TraceFailure("load DB_Q submission failed", user);
        ResetSession(user);
    }
}

void ResetSession(CUser* user) {
    MutexGuard lock(&g_mutex);
    g_sessions.erase(user);
}

void FlushAndErase(CUser* user) {
    // Accepted DB_Q work is durable and reward delivery is keyed by claim.
    // Teardown starts no best-effort mutation against a disappearing CUser.
    ResetSession(user);
}

void OnFatigueUsed(CUser* user, uint32_t amount) {
    if (!HasIdentity(user) || amount == 0u ||
        !IsBlueHorseEventActive(user) || !Config().Get()) {
        return;
    }
    {
        MutexGuard lock(&g_mutex);
        std::map<CUser*, Session>::iterator found = g_sessions.find(user);
        if (found == g_sessions.end() ||
            !IdentityMatches(user, found->second)) {
            return;
        }
        found->second.queued_fatigue = SaturatingAdd(
            found->second.queued_fatigue, amount);
    }
    (void)TryQueueDeferredFatigue(user);
}

void SendOnTimeImdState(CUser* user, uint8_t state) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_ON_TIME_IMD_STAT);
    packet.put_byte(state);
    packet.finalize(true);
    user->Send(packet);
}

void SendOnTimeImdRewardState(CUser* user, uint8_t available) {
    if (!user || available > 1u) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_ON_TIME_IMD_REWARD_STAT);
    packet.put_byte(available);
    packet.finalize(true);
    user->Send(packet);
}

void SendSeaBottomInfo(CUser* user,
                       uint32_t field_0,
                       const uint32_t* item_ids,
                       uint32_t count) {
    if (!user || (count != 0u && !item_ids) || count > 20000u) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_THE_SEA_BOTTOM2000_INFO);
    packet.put_int(static_cast<int>(field_0));
    packet.put_int(static_cast<int>(count));
    for (uint32_t index = 0u; index < count; ++index) {
        packet.put_int(static_cast<int>(item_ids[index]));
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendTripleExpKeepingInfo(CUser* user,
                              uint8_t field_0,
                              uint8_t field_1,
                              uint8_t field_2,
                              uint32_t field_3,
                              uint32_t field_4,
                              uint8_t field_5) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_TRIPLE_EXP_KEEPING_INFO);
    packet.put_byte(field_0);
    packet.put_byte(field_1);
    packet.put_byte(field_2);
    packet.put_int(static_cast<int>(field_3));
    packet.put_int(static_cast<int>(field_4));
    packet.put_byte(field_5);
    packet.finalize(true);
    user->Send(packet);
}

void SendFortuneData(CUser* user, const FortuneData& data) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_FORTUNE_DATA);
    packet.put_short(data.field_0);
    packet.put_short(data.field_1);
    packet.put_short(data.field_2);
    packet.put_byte(data.field_3);
    packet.put_byte(data.field_4);
    packet.put_byte(data.field_5);
    packet.put_byte(data.field_6);
    packet.finalize(true);
    user->Send(packet);
}

void SendFortuneBroadcast(CUser* user,
                          const char* character_name,
                          uint32_t character_name_bytes,
                          uint16_t field_0,
                          uint16_t field_1,
                          uint16_t field_2,
                          uint8_t field_3) {
    if (!user || !character_name || character_name_bytes == 0u ||
        character_name_bytes > 255u ||
        std::memchr(character_name, 0,
                    static_cast<size_t>(character_name_bytes)) != 0) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_FORTUNE_BROADCAST);
    packet.put_int(static_cast<int>(character_name_bytes));
    packet.put_binary(character_name,
                      static_cast<int>(character_name_bytes));
    packet.put_short(field_0);
    packet.put_short(field_1);
    packet.put_short(field_2);
    packet.put_byte(field_3);
    packet.finalize(true);
    user->Send(packet);
}

void SendBlueHorseData(CUser* user,
                       uint8_t show_ui,
                       uint8_t ticket_count,
                       const uint8_t slots[3],
                       uint8_t distance,
                       uint8_t completed) {
    if (!user || !slots || show_ui > 1u ||
        distance > kBlueHorseMaximumDistance || completed > 1u ||
        slots[0] > 4u || slots[1] > 4u || slots[2] > 4u) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_BLUE_HORSE_CHARAC_DATA);
    packet.put_byte(show_ui);
    packet.put_byte(ticket_count);
    packet.put_byte(3);
    packet.put_byte(slots[0]);
    packet.put_byte(slots[1]);
    packet.put_byte(slots[2]);
    packet.put_byte(distance);
    packet.put_byte(completed);
    packet.finalize(true);
    user->Send(packet);
}

bool HasNativeOnTimeImdClosure() {
    return false;
}

bool HasNativeSeaBottomClosure() {
    return false;
}

bool HasNativeTripleExpKeepingClosure() {
    return false;
}

bool HasNativeFortuneClosure() {
    return false;
}

bool HasNativeBlueHorseClosure() {
    // Registration is a code-closure predicate, not a PVF load hook.  The
    // strict parser runs only after the event/PVF lifecycle is ready and then
    // fails closed at BeginSession/dispatch time.
    return true;
}

}  // namespace tw_late_event_0828

BlueHorseEvent::BlueHorseEvent() : CEventBase() {}
BlueHorseEvent::~BlueHorseEvent() {}

void BlueHorseEvent::StartEvent() {
    if (!Config().Get()) {
        SetEventFlag(false);
        TraceFailure("strict Event/tw_bluehorse.evt parse failed", 0);
        return;
    }
    SetEventFlag(true);
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers([](CUser* user) {
            blue_horse_event::BeginSession(user);
        });
    }
}

void BlueHorseEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

void BlueHorseEvent::EndEvent() {
    SetEventFlag(false);
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers([](CUser* user) {
            blue_horse_event::ResetSession(user);
        });
    }
}

Dispatcher_PlayBlueHorseSlot::Dispatcher_PlayBlueHorseSlot()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_PlayBlueHorseSlot::~Dispatcher_PlayBlueHorseSlot() {}

int Dispatcher_PlayBlueHorseSlot::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    unsigned char slot = 0u;
    if (packet.get_byte(slot) != 1 ||
        slot >= blue_horse_event_db::kSlotCount || !IsTownReady(user)) {
        SendCommand730(user, false, 0u);
        return 0;
    }
    const BlueHorseConfig* const config = Config().Get();
    Session session;
    if (!config || !GetSession(user, session) || !session.loaded ||
        session.db_pending || session.completed != 0u ||
        session.ticket_count == 0u || session.slots[slot] != 0u) {
        SendCommand730(user, false, 0u);
        return 0;
    }
    const uint8_t result = DrawSlot(*config);
    Message request;
    if (result == 0u ||
        !BuildRequest(user,
                      blue_horse_event_db::OPERATION_ROLL_BLUE_HORSE_SLOT,
                      request)) {
        SendCommand730(user, false, 0u);
        return 0;
    }
    request.slot_index = slot;
    request.roll_result = result;
    if (!QueueTracked(user, request)) {
        SendCommand730(user, false, 0u);
        TraceFailure("roll DB_Q submission failed", user);
    }
    return 0;
}

int Dispatcher_PlayBlueHorseSlot::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_PlayBlueHorseSlot::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_RewardBlueHorseSlot::Dispatcher_RewardBlueHorseSlot()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RewardBlueHorseSlot::~Dispatcher_RewardBlueHorseSlot() {}

int Dispatcher_RewardBlueHorseSlot::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    if (!IsTownReady(user)) {
        SendCommand731(user, false, 0u, 0u);
        return 0;
    }
    Session session;
    if (!Config().Get() || !GetSession(user, session) || !session.loaded ||
        session.db_pending || session.completed != 0u ||
        session.slots[0] == 0u || session.slots[1] == 0u ||
        session.slots[2] == 0u) {
        SendCommand731(user, false, 0u, 0u);
        return 0;
    }
    Message request;
    if (!BuildRequest(
            user,
            blue_horse_event_db::OPERATION_RESERVE_BLUE_HORSE_REWARD,
            request) ||
        !QueueTracked(user, request)) {
        SendCommand731(user, false, 0u, 0u);
        TraceFailure("settlement reserve DB_Q submission failed", user);
    }
    return 0;
}

int Dispatcher_RewardBlueHorseSlot::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RewardBlueHorseSlot::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

InterBlueHorseEvent::InterBlueHorseEvent()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              blue_horse_event_db::kTaiwanInnerCode)) {}

InterBlueHorseEvent::~InterBlueHorseEvent() {}

int InterBlueHorseEvent::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    const Message response = *reinterpret_cast<const Message*>(payload);
    if (!ResponseEnvelopeValid(response)) {
        TraceFailure("invalid inner-29 response envelope", user);
        blue_horse_event::ResetSession(user);
        return 0;
    }
    Session session;
    if (!ConsumeResponse(user, response, session)) return 0;

    if (response.operation ==
        blue_horse_event_db::OPERATION_LOAD_BLUE_HORSE) {
        if (response.result == blue_horse_event_db::RESULT_SUCCESS) {
            SendSnapshot(user, session, response.show_ui);
            (void)TryQueueDeferredFatigue(user);
        } else {
            TraceFailure("load failed", user);
        }
        return 0;
    }

    if (response.operation ==
        blue_horse_event_db::OPERATION_ADD_BLUE_HORSE_FATIGUE) {
        if (response.result == blue_horse_event_db::RESULT_SUCCESS) {
            SendSnapshot(user, session, 0u);
            (void)TryQueueDeferredFatigue(user);
        } else {
            TraceFailure("fatigue persistence failed", user);
        }
        return 0;
    }

    if (response.operation ==
        blue_horse_event_db::OPERATION_ROLL_BLUE_HORSE_SLOT) {
        if (response.result == blue_horse_event_db::RESULT_SUCCESS) {
            SendCommand730(user, true, response.roll_result);
            SendSnapshot(user, session, 0u);
        } else {
            SendCommand730(user, false, 0u);
        }
        (void)TryQueueDeferredFatigue(user);
        return 0;
    }

    if (response.operation ==
        blue_horse_event_db::OPERATION_RESERVE_BLUE_HORSE_REWARD) {
        if (response.result != blue_horse_event_db::RESULT_SUCCESS) {
            SendCommand731(user, false, 0u, 0u);
            (void)TryQueueDeferredFatigue(user);
            return 0;
        }
        Message commit;
        if (!PrepareCommit(user, response, commit) ||
            !QueueTracked(user, commit)) {
            SendCommand731(user, false, 0u, 0u);
            TraceFailure("settlement commit preparation failed", user);
            (void)TryQueueDeferredFatigue(user);
        }
        return 0;
    }

    if (response.result == blue_horse_event_db::RESULT_SUCCESS) {
        SendCommand731(user, true, response.combo_reward_sent,
                       response.complete_reward_sent);
        SendSnapshot(user, session, 0u);
    } else {
        SendCommand731(user, false, 0u, 0u);
    }
    (void)TryQueueDeferredFatigue(user);
    return 0;
}
