#include "old_game/activity/schedule/king_of_pick.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_king_of_pick.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <ctime>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

const int kRepeatEventId = 129;
const int kMinimumLevel = 19;
const int kRefillChance = 1;
const uint32_t kExpectedItemIds[king_of_pick_db::kPoolItemCount] = {
    690001492u, 690001493u, 690001494u, 690001495u,
};
const uint32_t kExpectedItemCounts[king_of_pick_db::kPoolItemCount] = {
    10u, 10u, 5u, 75u,
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct PoolConfig {
    PoolConfig() : item_id(0u), count(0u) {}
    PoolConfig(uint32_t item, uint32_t quantity)
        : item_id(item), count(quantity) {}
    uint32_t item_id;
    uint32_t count;
};

bool ParsePairs(std::vector<PoolConfig>& values,
                const char* closing_tag,
                unsigned int maximum_count) {
    bool is_type = false;
    bool success = false;
    std::string token;
    for (;;) {
        const int first = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == closing_tag;
        }
        if (!success || first <= 0 || values.size() >= maximum_count) {
            return false;
        }
        const int second = ScanInt(&success);
        if (!success || second <= 0) {
            return false;
        }
        values.push_back(PoolConfig(static_cast<uint32_t>(first),
                                    static_cast<uint32_t>(second)));
    }
}

bool ParseEmptyBlock(const char* closing_tag) {
    bool is_type = false;
    bool success = false;
    std::string token;
    (void)ScanTypeOrInt(is_type, token, success, true);
    return is_type && token == closing_tag;
}

class KingOfPickConfig {
public:
    KingOfPickConfig() : attempted_(false), valid_(false), pool_() {}

    bool available() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = Load();
        }
        return valid_;
    }

    const std::vector<PoolConfig>& pool() const { return pool_; }

private:
    bool Load() {
        if (!loadRDARScriptFile("Event/", "kingofpickevent.evt")) {
            return false;
        }
        RdarUnloadGuard unload;

        int minimum_level = -1;
        int refill_chance = -1;
        std::vector<PoolConfig> condition_rewards;
        std::vector<PoolConfig> pool;
        bool has_minimum = false;
        bool has_refill = false;
        bool has_condition = false;
        bool has_pool = false;
        bool has_bonus = false;
        bool valid = true;
        std::string tag;
        while (valid && ScanType(tag, true) == 1) {
            bool success = false;
            if (tag == "[min level]" && !has_minimum) {
                minimum_level = ScanInt(&success);
                has_minimum = success;
                valid = success;
            } else if (tag == "[refill pick chance]" && !has_refill) {
                refill_chance = ScanInt(&success);
                has_refill = success;
                valid = success;
            } else if (tag == "[condition reward bonus]" &&
                       !has_condition) {
                has_condition = true;
                valid = ParsePairs(condition_rewards,
                                   "[/condition reward bonus]",
                                   10u);
            } else if (tag == "[pick item info]" && !has_pool) {
                has_pool = true;
                valid = ParsePairs(pool,
                                   "[/pick item info]",
                                   king_of_pick_db::kPoolItemCount);
            } else if (tag == "[pick bonus item info]" && !has_bonus) {
                has_bonus = true;
                valid = ParseEmptyBlock("[/pick bonus item info]");
            } else {
                valid = false;
            }
        }

        if (!valid || !has_minimum || !has_refill || !has_condition ||
            !has_pool || !has_bonus || minimum_level != kMinimumLevel ||
            refill_chance != kRefillChance ||
            condition_rewards.size() != 10u ||
            pool.size() != king_of_pick_db::kPoolItemCount ||
            !G_CDataManager()) {
            return false;
        }
        for (unsigned int index = 0; index < condition_rewards.size();
             ++index) {
            if (condition_rewards[index].item_id != index + 1u ||
                condition_rewards[index].count != 1u) {
                return false;
            }
        }
        for (unsigned int index = 0; index < pool.size(); ++index) {
            if (pool[index].item_id != kExpectedItemIds[index] ||
                pool[index].count != kExpectedItemCounts[index]) {
                return false;
            }
            CItem* const item =
                G_CDataManager()->find_item(static_cast<int>(pool[index].item_id));
            if (!item || !item->vtable_) {
                return false;
            }
        }
        pool_.swap(pool);
        return true;
    }

    bool attempted_;
    bool valid_;
    std::vector<PoolConfig> pool_;
};

KingOfPickConfig& Config() {
    static KingOfPickConfig config;
    return config;
}

struct EventState {
    EventState()
        : charac_no(0u), session_token(0u), source_epoch(0u), revision(0u),
          day_key(0u),
          week_key(0u), remaining(0u), daily_clear_count(0u),
          history_count(0u), next_request_id(0u), pending_request_id(0u),
          pending_operation(0u), seed_baseline_serial(0u),
          seed_proper_watermark(false), loaded(false) {
        std::memset(history, 0, sizeof(history));
    }

    uint32_t charac_no;
    uint32_t session_token;
    uint32_t source_epoch;
    uint32_t revision;
    uint32_t day_key;
    uint32_t week_key;
    uint16_t remaining;
    uint16_t daily_clear_count;
    king_of_pick_db::HistoryEntry
        history[king_of_pick_db::kMaximumHistoryCount];
    uint16_t history_count;
    uint32_t next_request_id;
    uint32_t pending_request_id;
    uint8_t pending_operation;
    uint16_t seed_baseline_serial;
    bool seed_proper_watermark;
    bool loaded;
};

typedef std::map<uint32_t, EventState> StateMap;

StateMap& States() {
    static StateMap states;
    return states;
}

typedef std::set<uint64_t> SeededCharacterSet;
typedef std::map<uint64_t, uint16_t> NonRewardBoundaryMap;
typedef std::map<uint64_t, uint32_t> SourceEpochMap;

SeededCharacterSet& SeededCharactersForCurrentRun() {
    static SeededCharacterSet characters;
    return characters;
}

NonRewardBoundaryMap& NonRewardBoundariesForCurrentRun() {
    static NonRewardBoundaryMap boundaries;
    return boundaries;
}

bool& DailySerialGenerationHasReset() {
    static bool reset = false;
    return reset;
}

SourceEpochMap& CharacterSourceEpochOverrides() {
    static SourceEpochMap epochs;
    return epochs;
}

uint32_t& UnloadedSourceEpochLatch() {
    static uint32_t epoch = 0u;
    return epoch;
}

uint32_t ReadFreshSourceEpoch() {
    const std::time_t source_time = std::time(0);
    if (source_time <= 0 ||
        static_cast<unsigned long long>(source_time) > 0xffffffffULL) {
        return 0u;
    }
    return static_cast<uint32_t>(source_time);
}

bool& CurrentRunRequiresSeed() {
    static bool required = false;
    return required;
}

uint64_t CharacterSeedKey(CUser* user) {
    if (!user || user->get_acc_id() <= 0 || user->getCurCharacNo() <= 0) {
        return 0u;
    }
    return (static_cast<uint64_t>(
                static_cast<uint32_t>(user->get_acc_id()))
            << 32) |
           static_cast<uint32_t>(user->getCurCharacNo());
}

uint32_t GetUnloadedSourceEpoch(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    SourceEpochMap::const_iterator override_epoch =
        CharacterSourceEpochOverrides().find(key);
    if (override_epoch != CharacterSourceEpochOverrides().end()) {
        return override_epoch->second;
    }
    uint32_t& epoch = UnloadedSourceEpochLatch();
    if (epoch == 0u) {
        epoch = ReadFreshSourceEpoch();
    }
    return epoch;
}

bool NeedsCurrentRunSeed(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    return CurrentRunRequiresSeed() && key != 0u &&
           SeededCharactersForCurrentRun().find(key) ==
               SeededCharactersForCurrentRun().end();
}

uint16_t ReadProperClearSerial(CUser* user) {
    const int serial = user ? user->GetProperDungeonClearCount() : 0;
    return static_cast<uint16_t>(
        serial < 0 ? 0 : (serial > 0xffff ? 0xffff : serial));
}

uint16_t GetOrCaptureSeedBaseline(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    if (key == 0u) {
        return 0u;
    }
    NonRewardBoundaryMap& boundaries =
        NonRewardBoundariesForCurrentRun();
    NonRewardBoundaryMap::iterator found = boundaries.find(key);
    if (found != boundaries.end()) {
        return found->second;
    }
    const uint16_t baseline = ReadProperClearSerial(user);
    boundaries.insert(std::make_pair(key, baseline));
    return baseline;
}

void AdvanceNonRewardBoundary(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    if (key == 0u) {
        return;
    }
    const uint16_t current = ReadProperClearSerial(user);
    NonRewardBoundaryMap& boundaries =
        NonRewardBoundariesForCurrentRun();
    NonRewardBoundaryMap::iterator found = boundaries.find(key);
    if (found == boundaries.end()) {
        boundaries.insert(std::make_pair(key, current));
    } else if (found->second < current) {
        found->second = current;
    }
}

bool FindNonRewardBoundary(CUser* user, uint16_t& boundary) {
    const uint64_t key = CharacterSeedKey(user);
    if (key == 0u) {
        return false;
    }
    NonRewardBoundaryMap& boundaries =
        NonRewardBoundariesForCurrentRun();
    NonRewardBoundaryMap::iterator found = boundaries.find(key);
    if (found == boundaries.end()) {
        if (!DailySerialGenerationHasReset()) {
            return false;
        }
        boundaries.insert(std::make_pair(key, static_cast<uint16_t>(0u)));
        boundary = 0u;
        return true;
    }
    boundary = found->second;
    return true;
}

void MarkCurrentRunSeeded(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    if (key != 0u) {
        SeededCharactersForCurrentRun().insert(key);
    }
}

uint32_t NextSessionToken() {
    static uint32_t token = 0u;
    if (++token == 0u) {
        ++token;
    }
    return token;
}

uint32_t NextRequestId(EventState& state) {
    if (++state.next_request_id == 0u) {
        ++state.next_request_id;
    }
    return state.next_request_id;
}

EventState* FindState(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return 0;
    }
    StateMap::iterator found =
        States().find(static_cast<uint32_t>(user->get_acc_id()));
    return found == States().end() ? 0 : &found->second;
}

bool IsReadyUser(CUser* user) {
    return user && user->get_state() == 3 && user->get_acc_id() > 0 &&
           user->getCurCharacNo() > 0 && user->getCurCharacR();
}

bool HasCharacterIdentity(CUser* user) {
    return user && user->get_acc_id() > 0 && user->getCurCharacNo() > 0 &&
           user->getCurCharacR();
}

bool IsEventActive(CUser* user) {
    CEventBase* const event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(kRepeatEventId)
            : 0;
    return event && event->IsEventing(user) == 1;
}

void CopyText(char* destination,
              unsigned int capacity,
              const char* source) {
    if (!destination || capacity == 0u) {
        return;
    }
    std::memset(destination, 0, capacity);
    if (source) {
        std::strncpy(destination, source, capacity - 1u);
    }
}

bool FillPool(king_of_pick_db::Message& message) {
    if (!Config().available() || !G_CDataManager()) {
        return false;
    }
    const std::vector<PoolConfig>& pool = Config().pool();
    for (unsigned int index = 0; index < pool.size(); ++index) {
        CItem* const prototype =
            G_CDataManager()->find_item(static_cast<int>(pool[index].item_id));
        if (!prototype || !prototype->vtable_) {
            return false;
        }
        Inven_Item item;
        prototype->vtable_->make_item(prototype, item);
        item.item_id_ = prototype->get_index();
        item.set_add_info(1);
        InvenItemMailRecord record;
        std::memset(&record, 0, sizeof(record));
        item.ExportMailRecord(record);

        king_of_pick_db::PoolReward& output = message.pool[index];
        output.initial_count = pool[index].count;
        output.reward.item_id = record.item_id;
        output.reward.add_info = record.add_info;
        output.reward.extend_info = record.extend_info;
        output.reward.chronicle_value = record.chronicle_value;
        output.reward.endurance = record.endurance;
        output.reward.amplify_value = record.amplify_value;
        output.reward.seal_flag = record.seal_flag;
        output.reward.item_attr = record.item_attr;
        output.reward.amplify_option = record.amplify_option;
        output.reward.separate_upgrade = record.separate_upgrade;
        output.reward.avatar_flag = record.avatar_flag ? 1u : 0u;
        output.reward.creature_flag = record.creature_flag ? 1u : 0u;
        std::memcpy(output.reward.item_guid,
                    record.item_guid,
                    sizeof(output.reward.item_guid));
        std::memcpy(output.reward.random_option,
                    record.random_option,
                    sizeof(output.reward.random_option));
    }
    CopyText(message.mail_title,
             sizeof(message.mail_title),
             "King of Pick");
    CopyText(message.mail_content,
             sizeof(message.mail_content),
             "King of Pick reward");
    return true;
}

bool BuildRequest(CUser* user,
                  uint8_t operation,
                  king_of_pick_db::Message& message) {
    EventState* const state = FindState(user);
    if (!HasCharacterIdentity(user) || !state ||
        state->charac_no != static_cast<uint32_t>(user->getCurCharacNo())) {
        return false;
    }
    king_of_pick_db::ClearMessage(message);
    message.account_id = static_cast<uint32_t>(user->get_acc_id());
    message.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    message.session_token = state->session_token;
    message.request_id = NextRequestId(*state);
    message.server_group = user->GetServerGroup();
    message.operation = operation;
    if (state->loaded && state->day_key != 0u && state->week_key != 0u &&
        state->day_key >= state->week_key &&
        state->day_key <= state->week_key + 6u) {
        // Preserve the DB-authoritative source generation until the explicit
        // daily reset hook swaps sessions.  This closes the narrow window in
        // which wall clock crossed 06:00 but reset dispatch has not run yet.
        message.day_key = state->day_key;
        message.week_key = state->week_key;
    } else {
        if (state->source_epoch == 0u) {
            return false;
        }
        // Before the first snapshot, the fixed response week-key slot carries
        // a request-only Unix timestamp.  DB converts it in its own timezone.
        message.week_key = state->source_epoch;
    }
    uint16_t non_reward_boundary = 0u;
    bool has_non_reward_boundary = false;
    if (operation == king_of_pick_db::kOperationLoad &&
        state->seed_proper_watermark) {
        non_reward_boundary = state->seed_baseline_serial;
        has_non_reward_boundary = true;
    }
    uint16_t tracked_boundary = 0u;
    if ((operation == king_of_pick_db::kOperationLoad ||
         operation == king_of_pick_db::kOperationProperDungeonClear) &&
        FindNonRewardBoundary(user, tracked_boundary)) {
        if (!has_non_reward_boundary ||
            non_reward_boundary < tracked_boundary) {
            non_reward_boundary = tracked_boundary;
        }
        has_non_reward_boundary = true;
    }
    message.has_non_reward_clear_boundary =
        has_non_reward_boundary ? 1u : 0u;
    message.non_reward_clear_serial =
        has_non_reward_boundary ? non_reward_boundary : 0u;
    const int level = user->get_charac_level();
    message.character_level = static_cast<uint8_t>(
        level < 0 ? 0 : (level > 255 ? 255 : level));
    message.proper_clear_serial = ReadProperClearSerial(user);
    return FillPool(message);
}

bool IsCurrentResponse(CUser* user,
                       const king_of_pick_db::Message& message) {
    EventState* const state = FindState(user);
    return HasCharacterIdentity(user) && state && message.session_token != 0u &&
           state->session_token == message.session_token &&
           state->charac_no == message.charac_no &&
           message.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           message.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           message.server_group == user->GetServerGroup();
}

bool ApplySnapshot(EventState& state,
                   const king_of_pick_db::Message& message) {
    if (message.history_count > king_of_pick_db::kMaximumHistoryCount ||
        message.remaining_pick_chance >
            king_of_pick_db::kMaximumHistoryCount ||
        message.daily_proper_clear_count > 10u) {
        return false;
    }
    state.revision = message.revision;
    state.day_key = message.day_key;
    state.week_key = message.week_key;
    state.remaining = message.remaining_pick_chance;
    state.daily_clear_count = message.daily_proper_clear_count;
    state.history_count = message.history_count;
    std::memset(state.history, 0, sizeof(state.history));
    std::memcpy(state.history,
                message.history,
                sizeof(message.history[0]) * message.history_count);
    state.loaded = true;
    return true;
}

void SendError(CUser* user, ENUM_CMDPACKET command) {
    if (user) {
        user->SendCmdErrorPacket(command, 1);
    }
}

void SendState(CUser* user, const EventState& state) {
    if (!user || user->get_state() <= 2 || !state.loaded ||
        !IsEventActive(user)) {
        return;
    }
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: sub_4F5340 consumes success, u16 remaining,
    // u16 history-count, then count * {u16 position,u32 item-id}.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REQUEST_PICK_EVENT_INFO);
    packet.put_byte(1);
    packet.put_short(state.remaining);
    packet.put_short(state.history_count);
    for (unsigned int index = 0; index < state.history_count; ++index) {
        packet.put_short(state.history[index].selected_position);
        packet.put_int(static_cast<int>(state.history[index].reward_item_id));
    }
    packet.finalize(true);
    user->Send(packet);
}

void SendDailySuccess(CUser* user, uint16_t remaining) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: sub_4F4FF0 consumes exactly one u16 after the
    // framework success byte.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REQUEST_ADD_PICK_CHANCE);
    packet.put_byte(1);
    packet.put_short(remaining);
    packet.finalize(true);
    user->Send(packet);
}

bool FindSelectedReward(const king_of_pick_db::Message& message,
                        uint32_t& item_id) {
    for (unsigned int index = 0; index < message.history_count; ++index) {
        if (message.history[index].selected_position ==
            message.requested_position) {
            item_id = message.history[index].reward_item_id;
            return item_id != 0u;
        }
    }
    return false;
}

void SendDrawSuccess(CUser* user,
                     const king_of_pick_db::Message& message) {
    uint32_t item_id = 0u;
    if (!FindSelectedReward(message, item_id)) {
        SendError(user, ENUM_CMDPACKET_LETS_PICK_PRESENT);
        return;
    }
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: sub_4F5540 consumes u16 remaining, u16 selected
    // position, u16 reward-count, then reward-count * u32 item-id.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_LETS_PICK_PRESENT);
    packet.put_byte(1);
    packet.put_short(message.remaining_pick_chance);
    packet.put_short(message.requested_position);
    packet.put_short(1);
    packet.put_int(static_cast<int>(item_id));
    packet.finalize(true);
    user->Send(packet);
}

void NotifyNewMail(CUser* user) {
    if (!user || !GlobalData::s_monitor_proxy_mgr) {
        return;
    }
    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    if (proxy) {
        proxy->SendNotifyNewMail(
            static_cast<uint32_t>(user->getCurCharacNo()));
    }
}

bool QueueCommand(CUser* user,
                  uint8_t operation,
                  uint16_t selected_position) {
    EventState* const state = FindState(user);
    if (!state || state->pending_request_id != 0u) {
        return false;
    }
    king_of_pick_db::Message request;
    if (!BuildRequest(user, operation, request)) {
        return false;
    }
    request.requested_position = selected_position;
    if (!king_of_pick_db::Queue(user->GetUID(), request)) {
        return false;
    }
    state->pending_request_id = request.request_id;
    state->pending_operation = operation;
    return true;
}

}  // namespace

CKingOfPickEvent::CKingOfPickEvent() : CEventBase() {}

CKingOfPickEvent::~CKingOfPickEvent() {}

void CKingOfPickEvent::StartEvent() {
    if (event_flag_ != 0u) {
        return;
    }
    // The event is advertised through NOTI108 as soon as this flag is set.
    // Validate the complete PVF/item closure before publishing that state so
    // an offline startup cannot leave a missing kingofpickevent.evt active.
    if (!Config().available()) {
        SetEventFlag(false);
        return;
    }
    SetEventFlag(true);
    SeededCharactersForCurrentRun().clear();
    NonRewardBoundariesForCurrentRun().clear();
    CurrentRunRequiresSeed() = true;

    CGameManager* const manager = G_CGameManager();
    if (!manager) {
        return;
    }
    for (CGameManager::UserMap::iterator user =
             manager->users_by_account_.begin();
         user != manager->users_by_account_.end(); ++user) {
        king_of_pick::OnEventStarted(user->second);
    }
}

void CKingOfPickEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

void CKingOfPickEvent::EndEvent() {
    SetEventFlag(false);
    CurrentRunRequiresSeed() = false;
}

namespace king_of_pick {

void RequestLoginState(CUser* user, bool seed_proper_watermark) {
    if (!HasCharacterIdentity(user) || !Config().available() ||
        !IsEventActive(user)) {
        return;
    }
    EventState state;
    state.source_epoch = GetUnloadedSourceEpoch(user);
    if (state.source_epoch == 0u) {
        return;
    }
    state.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    state.session_token = NextSessionToken();
    // A true StartEvent establishes a new runtime boundary for every
    // account/character, including characters that were offline at the edge.
    // Keep requesting a seed-only load until one succeeds; otherwise a
    // transient load failure could turn inactive-period clears into rewards.
    state.seed_proper_watermark =
        seed_proper_watermark || NeedsCurrentRunSeed(user);
    state.seed_baseline_serial = state.seed_proper_watermark
                                     ? GetOrCaptureSeedBaseline(user)
                                     : 0u;
    States()[static_cast<uint32_t>(user->get_acc_id())] = state;

    EventState* const installed = FindState(user);
    king_of_pick_db::Message request;
    if (!installed ||
        !BuildRequest(user, king_of_pick_db::kOperationLoad, request) ||
        !king_of_pick_db::Queue(user->GetUID(), request)) {
        States().erase(static_cast<uint32_t>(user->get_acc_id()));
        return;
    }
    installed->pending_request_id = request.request_id;
    installed->pending_operation = request.operation;
}

void OnEventStarted(CUser* user) {
    if (!HasCharacterIdentity(user)) {
        return;
    }
    States().erase(static_cast<uint32_t>(user->get_acc_id()));
    RequestLoginState(user, true);
}

void OnGlobalMinuteTick() {
    CGameManager* const manager = G_CGameManager();
    if (!manager) {
        return;
    }
    for (CGameManager::UserMap::iterator user =
             manager->users_by_account_.begin();
         user != manager->users_by_account_.end(); ++user) {
        CUser* const current = user->second;
        if (!HasCharacterIdentity(current) || !IsEventActive(current)) {
            continue;
        }
        EventState* const state = FindState(current);
        if (!state) {
            RequestLoginState(current);
        } else if (!state->loaded && state->pending_request_id == 0u) {
            States().erase(static_cast<uint32_t>(current->get_acc_id()));
            RequestLoginState(current);
        }
    }
}

void ResetSession(CUser* user) {
    if (user && user->get_acc_id() > 0) {
        States().erase(static_cast<uint32_t>(user->get_acc_id()));
    }
}

void OnProperClearGlobalReset() {
    // Reset every known generation before GameWorld visits its online users.
    // Characters that are offline or still in state2 receive the same zero
    // default when their next load request is built.
    DailySerialGenerationHasReset() = true;
    UnloadedSourceEpochLatch() = ReadFreshSourceEpoch();
    CharacterSourceEpochOverrides().clear();
    NonRewardBoundariesForCurrentRun().clear();
    States().clear();
}

void OnProperClearSerialReset(CUser* user) {
    const uint64_t key = CharacterSeedKey(user);
    if (key == 0u) {
        return;
    }
    // This is an explicit source-counter generation boundary.  Keep a zero
    // boundary (rather than inferring reset from 16-bit serial magnitude), so
    // the first eligible clear of the new day is awarded and a failed first
    // DB_Q submission remains recoverable by the next clear.
    NonRewardBoundariesForCurrentRun()[key] = 0u;
    CharacterSourceEpochOverrides()[key] = ReadFreshSourceEpoch();

    if (user->get_acc_id() > 0) {
        // Invalidate all pre-06:00 replies and enqueue a fresh snapshot after
        // earlier DB_Q work.  Each request also carries its latched source
        // generation, so a delayed old request can only reconcile the old
        // activity-day row.
        States().erase(static_cast<uint32_t>(user->get_acc_id()));
        if (user->get_state() > 2 && IsEventActive(user)) {
            RequestLoginState(user);
        }
    }
}

void OnProperDungeonClear(CUser* user) {
    if (!HasCharacterIdentity(user) || user->get_state() != 5 ||
        !IsEventActive(user) || !Config().available()) {
        return;
    }
    // The canonical source serial increments before this hook.  Low-level
    // clears must still advance the durable watermark without granting a
    // chance, otherwise a later level-up would backfill those ineligible
    // clears.  Keep the boundary even when the DB queue is temporarily down;
    // the next load/clear carries it through the same two-phase transaction.
    if (user->get_charac_level() <= kMinimumLevel) {
        AdvanceNonRewardBoundary(user);
    }
    EventState* const state = FindState(user);
    if (!state) {
        return;
    }
    king_of_pick_db::Message request;
    if (BuildRequest(user,
                     king_of_pick_db::kOperationProperDungeonClear,
                     request)) {
        (void)king_of_pick_db::Queue(user->GetUID(), request);
    }
}

}  // namespace king_of_pick_0828

Dispatcher_LetsPickPresent::Dispatcher_LetsPickPresent()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_LetsPickPresent::~Dispatcher_LetsPickPresent() {}

int Dispatcher_LetsPickPresent::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    unsigned short selected_position = 0u;
    if (!IsReadyUser(user) || !IsEventActive(user) ||
        user->get_charac_level() <= kMinimumLevel ||
        packet.get_short(selected_position) != 1 ||
        selected_position >= king_of_pick_db::kMaximumHistoryCount ||
        !QueueCommand(user,
                      king_of_pick_db::kOperationDraw,
                      selected_position)) {
        SendError(user, ENUM_CMDPACKET_LETS_PICK_PRESENT);
    }
    return 0;
}

int Dispatcher_LetsPickPresent::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_LetsPickPresent::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_RequestAddPickChance::Dispatcher_RequestAddPickChance()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_RequestAddPickChance::~Dispatcher_RequestAddPickChance() {}

int Dispatcher_RequestAddPickChance::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    if (!IsReadyUser(user) || !IsEventActive(user) ||
        user->get_charac_level() <= kMinimumLevel ||
        !QueueCommand(user,
                      king_of_pick_db::kOperationDailyRefill,
                      0xffffu)) {
        SendError(user, ENUM_CMDPACKET_REQUEST_ADD_PICK_CHANCE);
    }
    return 0;
}

int Dispatcher_RequestAddPickChance::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestAddPickChance::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_KingOfPick::Inter_KingOfPick() : ISigDispatcher() {}

Inter_KingOfPick::~Inter_KingOfPick() {}

int Inter_KingOfPick::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!payload ||
        payload_size !=
            static_cast<int>(sizeof(king_of_pick_db::Message))) {
        return 0;
    }
    const king_of_pick_db::Message& message =
        *reinterpret_cast<const king_of_pick_db::Message*>(payload);
    if (!IsCurrentResponse(user, message)) {
        return 0;
    }
    EventState* const state = FindState(user);
    const bool is_command =
        message.operation == king_of_pick_db::kOperationDailyRefill ||
        message.operation == king_of_pick_db::kOperationDraw;
    if ((message.operation == king_of_pick_db::kOperationLoad ||
         is_command) &&
        (state->pending_request_id != message.request_id ||
         state->pending_operation != message.operation)) {
        return 0;
    }
    if (message.operation == king_of_pick_db::kOperationLoad ||
        is_command) {
        state->pending_request_id = 0u;
        state->pending_operation = 0u;
    }
    if (message.result != king_of_pick_db::kResultOk) {
        if (message.operation == king_of_pick_db::kOperationLoad) {
            // Remove the unloaded tombstone; the global minute timer retries
            // with a fresh session token while the event remains active.
            States().erase(static_cast<uint32_t>(user->get_acc_id()));
            return 0;
        }
        if (message.operation == king_of_pick_db::kOperationDraw) {
            SendError(user, ENUM_CMDPACKET_LETS_PICK_PRESENT);
        } else if (message.operation ==
                   king_of_pick_db::kOperationDailyRefill) {
            SendError(user, ENUM_CMDPACKET_REQUEST_ADD_PICK_CHANCE);
        }
        return 0;
    }
    if (!ApplySnapshot(*state, message)) {
        if (is_command) {
            SendError(user,
                      message.operation == king_of_pick_db::kOperationDraw
                          ? ENUM_CMDPACKET_LETS_PICK_PRESENT
                          : ENUM_CMDPACKET_REQUEST_ADD_PICK_CHANCE);
        }
        return 0;
    }
    if (message.operation == king_of_pick_db::kOperationLoad &&
        state->seed_proper_watermark) {
        MarkCurrentRunSeeded(user);
    }
    if (message.operation == king_of_pick_db::kOperationLoad ||
        message.operation ==
            king_of_pick_db::kOperationProperDungeonClear) {
        SendState(user, *state);
    } else if (message.operation ==
               king_of_pick_db::kOperationDailyRefill) {
        SendDailySuccess(user, message.remaining_pick_chance);
    } else if (message.operation == king_of_pick_db::kOperationDraw) {
        SendDrawSuccess(user, message);
        if (message.new_mail) {
            NotifyNewMail(user);
        }
    }
    return 0;
}
