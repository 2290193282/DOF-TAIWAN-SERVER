#include "old_game/activity/schedule/sugar_time_event.h"

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/system_time.h"
#include "core/config/env/environment.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_sugar_time.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {

const char kSugarTimePvfDirectory[] = "Event/japan/";
const char kSugarTimePvf[] = "AttendanceEvent_jp.evt";
const uint32_t kCheckpointSeconds = 60u;
const uint32_t kPendingRecoverySeconds = 120u;

const uint32_t kOfficialDailySeconds[4] = {
    900u, 1800u, 3600u, 7200u,
};
const uint32_t kOfficialDailyItems[4] = {
    690001615u, 690001616u, 690001617u, 690001618u,
};
const uint32_t kOfficialGoalSeconds[3] = {
    90000u, 180000u, 270000u,
};
const uint32_t kOfficialGoalItems[3] = {
    690001627u, 690001628u, 690001629u,
};

struct Reward {
    Reward() : item_id(0u), count(0u) {}
    Reward(uint32_t item, uint32_t amount)
        : item_id(item), count(amount) {}
    uint32_t item_id;
    uint32_t count;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool ParseIntSection(const char* expected_end,
                     const char* alternate_end,
                     std::vector<int>& values) {
    bool is_type = false;
    bool success = false;
    std::string token;
    for (;;) {
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == expected_end ||
                   (alternate_end && token == alternate_end);
        }
        if (!success) {
            return false;
        }
        values.push_back(value);
    }
}

void FnvMix(uint32_t& hash, uint32_t value) {
    for (unsigned int byte = 0u; byte < 4u; ++byte) {
        hash ^= (value >> (byte * 8u)) & 0xFFu;
        hash *= 16777619u;
    }
}

class SugarTimeConfig {
public:
    SugarTimeConfig()
        : loaded_(false), event_key_(0u), daily_seconds_(),
          daily_rewards_(), goal_seconds_(), goal_rewards_() {}

    bool available() { return loaded_ || Load(); }
    uint32_t event_key() const { return event_key_; }
    const std::vector<uint32_t>& daily_seconds() const {
        return daily_seconds_;
    }
    const std::vector<Reward>& daily_rewards() const {
        return daily_rewards_;
    }
    const std::vector<uint32_t>& goal_seconds() const {
        return goal_seconds_;
    }
    const std::vector<Reward>& goal_rewards() const {
        return goal_rewards_;
    }

private:
    bool Load() {
        if (!loadRDARScriptFile(kSugarTimePvfDirectory,
                                kSugarTimePvf)) {
            return false;
        }
        RdarUnloadGuard unload;
        std::vector<int> process_values;
        std::vector<int> reward_values;
        std::vector<int> goal_values;
        std::string token;
        bool valid = true;
        while (valid && ScanType(token, true) == 1) {
            if (token == "[process time seconds]") {
                valid = process_values.empty() &&
                    ParseIntSection("[/process time seconds]", 0,
                                    process_values);
            } else if (token == "[reward item]") {
                valid = reward_values.empty() &&
                    ParseIntSection("[/reward item]", 0, reward_values);
            } else if (token ==
                       "[goal time and reward item for sum]") {
                // The official 0828 file ships the misspelled closing tag.
                valid = goal_values.empty() &&
                    ParseIntSection(
                        "[/goal type and reward item for sum]",
                        "[/goal time and reward item for sum]",
                        goal_values);
            }
        }
        if (!valid || process_values.size() != 4u ||
            reward_values.size() != 8u || goal_values.size() != 9u) {
            return false;
        }

        std::vector<uint32_t> daily_seconds;
        std::vector<Reward> daily_rewards;
        std::vector<uint32_t> goal_seconds;
        std::vector<Reward> goal_rewards;
        for (unsigned int index = 0u; index < 4u; ++index) {
            const int seconds = process_values[index];
            const int item_id = reward_values[index * 2u];
            const int count = reward_values[index * 2u + 1u];
            // Fail closed on the China test profile or on an edited PVF.
            if (seconds != static_cast<int>(kOfficialDailySeconds[index]) ||
                item_id != static_cast<int>(kOfficialDailyItems[index]) ||
                count != 1) {
                return false;
            }
            daily_seconds.push_back(static_cast<uint32_t>(seconds));
            daily_rewards.push_back(
                Reward(static_cast<uint32_t>(item_id),
                       static_cast<uint32_t>(count)));
        }
        for (unsigned int index = 0u; index < 3u; ++index) {
            const int seconds = goal_values[index * 3u];
            const int item_id = goal_values[index * 3u + 1u];
            const int count = goal_values[index * 3u + 2u];
            if (seconds != static_cast<int>(kOfficialGoalSeconds[index]) ||
                item_id != static_cast<int>(kOfficialGoalItems[index]) ||
                count != 1) {
                return false;
            }
            goal_seconds.push_back(static_cast<uint32_t>(seconds));
            goal_rewards.push_back(
                Reward(static_cast<uint32_t>(item_id),
                       static_cast<uint32_t>(count)));
        }

        uint32_t event_key = 2166136261u;
        for (unsigned int index = 0u; index < 4u; ++index) {
            FnvMix(event_key, daily_seconds[index]);
            FnvMix(event_key, daily_rewards[index].item_id);
            FnvMix(event_key, daily_rewards[index].count);
        }
        for (unsigned int index = 0u; index < 3u; ++index) {
            FnvMix(event_key, goal_seconds[index]);
            FnvMix(event_key, goal_rewards[index].item_id);
            FnvMix(event_key, goal_rewards[index].count);
        }
        if (event_key == 0u) {
            return false;
        }

        daily_seconds_.swap(daily_seconds);
        daily_rewards_.swap(daily_rewards);
        goal_seconds_.swap(goal_seconds);
        goal_rewards_.swap(goal_rewards);
        event_key_ = event_key;
        loaded_ = true;
        return true;
    }

    bool loaded_;
    uint32_t event_key_;
    std::vector<uint32_t> daily_seconds_;
    std::vector<Reward> daily_rewards_;
    std::vector<uint32_t> goal_seconds_;
    std::vector<Reward> goal_rewards_;
};

SugarTimeConfig& GetSugarTimeConfig() {
    static SugarTimeConfig config;
    return config;
}

bool& EventActive() {
    static bool active = false;
    return active;
}

bool& EventOwnerRequested() {
    static bool requested = false;
    return requested;
}

uint32_t& EventClosingEpoch() {
    static uint32_t epoch = 0u;
    return epoch;
}

uint32_t CurrentEpoch() {
    const int now = GlobalData::s_systemTime_.getCurSec();
    return now > 0 ? static_cast<uint32_t>(now) : 0u;
}

uint32_t ActivityDayStamp(uint32_t epoch, uint8_t reset_hour) {
    const uint32_t shift = static_cast<uint32_t>(reset_hour) * 3600u;
    if (epoch <= shift) {
        return 0u;
    }
    const time_t value = static_cast<time_t>(epoch - shift);
    tm local_time;
    tm* const result = localtime_r(&value, &local_time);
    return result
        ? static_cast<uint32_t>((result->tm_year + 1900) * 10000 +
                                (result->tm_mon + 1) * 100 +
                                result->tm_mday)
        : 0u;
}

uint32_t SecondsSinceActivityDayStart(uint32_t epoch,
                                      uint8_t reset_hour) {
    const uint32_t shift = static_cast<uint32_t>(reset_hour) * 3600u;
    if (epoch <= shift) {
        return 0u;
    }
    const time_t value = static_cast<time_t>(epoch - shift);
    tm local_time;
    tm* const result = localtime_r(&value, &local_time);
    return result
        ? static_cast<uint32_t>(result->tm_hour * 3600 +
                                result->tm_min * 60 +
                                result->tm_sec)
        : 0u;
}

void CalculateDeltas(uint32_t accounted_epoch,
                     uint32_t observed_epoch,
                     uint8_t reset_hour,
                     uint32_t& daily_delta,
                     uint32_t& total_delta) {
    daily_delta = 0u;
    total_delta = 0u;
    if (accounted_epoch == 0u || observed_epoch <= accounted_epoch) {
        return;
    }
    total_delta = observed_epoch - accounted_epoch;
    if (ActivityDayStamp(accounted_epoch, reset_hour) ==
        ActivityDayStamp(observed_epoch, reset_hour)) {
        daily_delta = total_delta;
    } else {
        daily_delta = SecondsSinceActivityDayStart(observed_epoch,
                                                   reset_hour);
        if (daily_delta > total_delta) {
            daily_delta = total_delta;
        }
    }
}

struct SugarTimeSession {
    SugarTimeSession()
        : charac_no(0u), event_key(0u), session_token(0u),
          day_stamp(0u), daily_elapsed(0u), total_elapsed(0u),
          revision(0u), selected_goal(0u), accounted_epoch(0u),
          closing_epoch(0u), pending_since_epoch(0u),
          daily_claimed_count(0u), final_claimed(0u),
          daily_reset_hour(0u), pending_operation(0u), loaded(false),
          campaign_authorized(false), db_pending(false) {}

    uint32_t charac_no;
    uint32_t event_key;
    uint32_t session_token;
    uint32_t day_stamp;
    uint32_t daily_elapsed;
    uint32_t total_elapsed;
    uint32_t revision;
    uint32_t selected_goal;
    uint32_t accounted_epoch;
    uint32_t closing_epoch;
    uint32_t pending_since_epoch;
    uint8_t daily_claimed_count;
    uint8_t final_claimed;
    uint8_t daily_reset_hour;
    uint8_t pending_operation;
    bool loaded;
    bool campaign_authorized;
    bool db_pending;
};

typedef std::map<uint32_t, SugarTimeSession> SugarTimeSessionMap;

SugarTimeSessionMap& SugarTimeSessions() {
    static SugarTimeSessionMap sessions;
    return sessions;
}

uint32_t NextSessionToken() {
    static uint32_t next = 0u;
    ++next;
    if (next == 0u) {
        ++next;
    }
    return next;
}

SugarTimeSession* FindSession(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return 0;
    }
    SugarTimeSessionMap::iterator found = SugarTimeSessions().find(
        static_cast<uint32_t>(user->get_acc_id()));
    return found == SugarTimeSessions().end() ? 0 : &found->second;
}

void PutWideString(PacketGuard& packet,
                   const std::vector<uint16_t>& value) {
    const int bytes = static_cast<int>(value.size() * sizeof(uint16_t));
    packet.put_int(bytes);
    if (bytes > 0) {
        packet.put_binary(reinterpret_cast<const char*>(&value[0]), bytes);
    }
}

bool ExportMailReward(const Reward& source,
                      sugar_time_db::MailReward& destination) {
    if (!G_CDataManager() || source.item_id == 0u || source.count == 0u) {
        return false;
    }
    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(source.item_id));
    if (!prototype || !prototype->vtable_) {
        return false;
    }
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = prototype->get_index();
    item.set_add_info(static_cast<int>(source.count));
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
    destination.amplify_option = record.amplify_option;
    destination.separate_upgrade = record.separate_upgrade;
    destination.avatar_flag = record.avatar_flag ? 1u : 0u;
    destination.creature_flag = record.creature_flag ? 1u : 0u;
    std::memcpy(destination.item_guid, record.item_guid,
                sizeof(destination.item_guid));
    std::memcpy(destination.random_option, record.random_option,
                sizeof(destination.random_option));
    return destination.item_id != 0u && destination.add_info != 0u;
}

bool FillMessage(CUser* user,
                 SugarTimeSession& session,
                 uint8_t operation,
                 uint32_t observed_epoch,
                 sugar_time_db::Message& message) {
    SugarTimeConfig& config = GetSugarTimeConfig();
    if (!user || !config.available() ||
        config.event_key() != session.event_key || observed_epoch == 0u) {
        return false;
    }
    sugar_time_db::ClearMessage(message);
    message.account_id = user->get_acc_id() > 0
        ? static_cast<uint32_t>(user->get_acc_id()) : 0u;
    message.charac_no = user->getCurCharacNo() > 0
        ? static_cast<uint32_t>(user->getCurCharacNo()) : 0u;
    message.event_key = config.event_key();
    message.session_token = session.session_token;
    message.observed_epoch = observed_epoch;
    message.server_group = user->GetServerGroup();
    message.day_stamp = ActivityDayStamp(observed_epoch,
                                         session.daily_reset_hour);
    message.daily_elapsed_seconds = session.daily_elapsed;
    message.total_elapsed_seconds = session.total_elapsed;
    message.revision = session.revision;
    message.selected_goal_seconds = session.selected_goal;
    message.daily_claimed_count = session.daily_claimed_count;
    message.final_claimed = session.final_claimed;
    message.operation = operation;
    message.daily_reset_hour = session.daily_reset_hour;

    const std::vector<uint32_t>& daily_seconds = config.daily_seconds();
    const std::vector<Reward>& daily_rewards = config.daily_rewards();
    const std::vector<uint32_t>& goal_seconds = config.goal_seconds();
    const std::vector<Reward>& goal_rewards = config.goal_rewards();
    for (unsigned int index = 0u; index < 4u; ++index) {
        message.process_seconds[index] = daily_seconds[index];
        if (!ExportMailReward(daily_rewards[index],
                              message.daily_rewards[index])) {
            return false;
        }
    }
    for (unsigned int index = 0u; index < 3u; ++index) {
        message.goal_seconds[index] = goal_seconds[index];
        if (!ExportMailReward(goal_rewards[index],
                              message.goal_rewards[index])) {
            return false;
        }
    }
    return message.account_id != 0u && message.charac_no != 0u &&
           message.session_token != 0u && message.day_stamp != 0u &&
           message.server_group >= 0 &&
           message.server_group <
               sugar_time_db::kMaximumServerGroupCount;
}

uint32_t SaturatingAdd(uint32_t left, uint32_t right) {
    return left > std::numeric_limits<uint32_t>::max() - right
        ? std::numeric_limits<uint32_t>::max() : left + right;
}

void EffectiveElapsed(const SugarTimeSession& session,
                      uint32_t now,
                      uint32_t& daily,
                      uint32_t& total) {
    uint32_t daily_delta = 0u;
    uint32_t total_delta = 0u;
    CalculateDeltas(session.accounted_epoch, now,
                    session.daily_reset_hour,
                    daily_delta, total_delta);
    daily = session.day_stamp ==
                ActivityDayStamp(now, session.daily_reset_hour)
        ? session.daily_elapsed : 0u;
    daily = SaturatingAdd(daily, daily_delta);
    total = SaturatingAdd(session.total_elapsed, total_delta);
}

uint8_t EffectiveClaimedCount(const SugarTimeSession& session,
                              uint32_t now) {
    return session.day_stamp ==
               ActivityDayStamp(now, session.daily_reset_hour)
        ? session.daily_claimed_count : 0u;
}

bool IsGoalSeconds(uint32_t seconds) {
    SugarTimeConfig& config = GetSugarTimeConfig();
    if (!config.available()) {
        return false;
    }
    const std::vector<uint32_t>& goals = config.goal_seconds();
    return std::find(goals.begin(), goals.end(), seconds) != goals.end();
}

void SendCommandError(CUser* user, int command) {
    if (user) {
        user->SendCmdErrorPacket(static_cast<ENUM_CMDPACKET>(command), 1);
    }
}

void SendCommandSuccess(CUser* user, int command) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, command);
    packet.put_byte(1);
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

bool QueueOperation(CUser* user,
                    SugarTimeSession& session,
                    uint8_t operation,
                    uint8_t requested_stage,
                    uint32_t selected_goal,
                    uint32_t observed_epoch = 0u,
                    bool allow_pending = false) {
    if (!user || (session.db_pending && !allow_pending)) {
        return false;
    }
    const uint32_t now = observed_epoch != 0u
        ? observed_epoch : CurrentEpoch();
    sugar_time_db::Message request;
    if (!FillMessage(user, session, operation, now, request)) {
        return false;
    }
    request.requested_stage = requested_stage;
    if (operation == sugar_time_db::OPERATION_PICK_FINAL) {
        request.selected_goal_seconds = selected_goal;
    }
    if (user->GetUID() < 0 ||
        !sugar_time_db::DBSugarTime::QueueRequest(
            user->GetUID(), request)) {
        return false;
    }
    if (!allow_pending) {
        session.db_pending = true;
        session.pending_since_epoch = now;
        session.pending_operation = operation;
    }
    return true;
}

bool SealSession(CUser* user,
                 SugarTimeSession& session,
                 uint32_t closing_epoch) {
    if (!user || closing_epoch == 0u ||
        session.charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo())) {
        return false;
    }
    return QueueOperation(user, session,
                          sugar_time_db::OPERATION_CHECKPOINT,
                          0u, 0u, closing_epoch, true);
}

void ApplyResponse(SugarTimeSession& session,
                   const sugar_time_db::Message& response) {
    session.day_stamp = response.day_stamp;
    session.daily_elapsed = response.daily_elapsed_seconds;
    session.total_elapsed = response.total_elapsed_seconds;
    session.revision = response.revision;
    session.selected_goal = response.selected_goal_seconds;
    session.daily_claimed_count = response.daily_claimed_count;
    session.final_claimed = response.final_claimed;
    if (response.observed_epoch >= session.accounted_epoch) {
        session.accounted_epoch = response.observed_epoch;
    }
    session.loaded = true;
    session.campaign_authorized = true;
}

bool IsCurrentResponse(CUser* user,
                       const sugar_time_db::Message& response) {
    SugarTimeSession* const session = FindSession(user);
    return user && session && response.account_id ==
               static_cast<uint32_t>(user->get_acc_id()) &&
           response.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           response.server_group == user->GetServerGroup() &&
           response.session_token != 0u &&
           response.session_token == session->session_token &&
           response.event_key == session->event_key &&
           response.charac_no == session->charac_no &&
           response.daily_reset_hour == session->daily_reset_hour;
}

void SendSessionState(CUser* user, const SugarTimeSession& session) {
    if (!user || user->get_state() <= 2 || !session.loaded ||
        !GetSugarTimeConfig().available()) {
        return;
    }
    const uint32_t now = CurrentEpoch();
    uint32_t daily = 0u;
    uint32_t total = 0u;
    EffectiveElapsed(session, now, daily, total);
    const uint8_t claimed = EffectiveClaimedCount(session, now);
    uint32_t remaining = 0u;
    uint8_t next_stage = 0u;
    uint8_t stage_index = 3u;
    if (claimed < sugar_time_db::kDailyRewardCount) {
        stage_index = claimed;
        next_stage = static_cast<uint8_t>(claimed + 1u);
        const uint32_t threshold =
            GetSugarTimeConfig().daily_seconds()[stage_index];
        remaining = daily < threshold ? threshold - daily : 0u;
    }
    sugar_time::SendDayState(user, remaining,
                                  next_stage, stage_index);

    uint32_t display_total = total;
    if (session.selected_goal != 0u &&
        display_total > session.selected_goal) {
        display_total = session.selected_goal;
    } else if (session.selected_goal == 0u) {
        const uint32_t maximum_goal =
            GetSugarTimeConfig().goal_seconds().back();
        if (display_total > maximum_goal) {
            display_total = maximum_goal;
        }
    }
    sugar_time::SendFinalState(
        user, display_total, session.selected_goal,
        session.final_claimed);
}

}  // namespace

namespace taiwan_reward_event_notifications {

bool SendSenseOfChoiceState(CUser* user,
                            const SenseOfChoiceState& state) {
    if (!user || state.label_utf16.size() > 30u) {
        return false;
    }
    PacketGuard packet;
    // 0828 CLIENT_IDA_EVIDENCE: sub_41B6A0 consumes
    // u32,u8,3*u8,wstring(max30),u8,3*u8,u8,3*u8.
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SENSE_OF_CHOICE);
    packet.put_int(static_cast<int>(state.day_or_result_id));
    packet.put_byte(state.active);
    for (unsigned int index = 0u; index < 3u; ++index) {
        packet.put_byte(state.first[index]);
    }
    PutWideString(packet, state.label_utf16);
    packet.put_byte(state.second_marker);
    for (unsigned int index = 0u; index < 3u; ++index) {
        packet.put_byte(state.second[index]);
    }
    packet.put_byte(state.third_marker);
    for (unsigned int index = 0u; index < 3u; ++index) {
        packet.put_byte(state.third[index]);
    }
    packet.finalize(true);
    user->Send(packet);
    return true;
}

bool SendPresentReturnState(CUser* user,
                            const uint32_t* item_ids,
                            uint32_t item_count,
                            const std::vector<uint16_t>& partner_utf16) {
    if (!user || (item_count != 0u && !item_ids) ||
        item_count > 4096u || partner_utf16.size() > 30u) {
        return false;
    }
    PacketGuard packet;
    // 0828 CLIENT_IDA_EVIDENCE: sub_410250 consumes
    // u32 count,count*u32 item_id,wstring(max30).
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_PRESENT_TO_RETURN_USER);
    packet.put_int(static_cast<int>(item_count));
    for (uint32_t index = 0u; index < item_count; ++index) {
        packet.put_int(static_cast<int>(item_ids[index]));
    }
    PutWideString(packet, partner_utf16);
    packet.finalize(true);
    user->Send(packet);
    return true;
}

bool SendBreakTrapSuccess(CUser* user,
                          uint8_t target_x,
                          uint8_t target_y,
                          uint8_t trap_result) {
    if (!user) {
        return false;
    }
    PacketGuard packet;
    // 0828 handler sub_558690 consumes the framework success byte followed
    // by exactly three u8 values.  This serializer is not an instance-state
    // authorization producer and therefore is not registration authority.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_BREAK_TRAP_RESULT);
    packet.put_byte(1);
    packet.put_byte(target_x);
    packet.put_byte(target_y);
    packet.put_byte(trap_result);
    packet.finalize(true);
    user->Send(packet);
    return true;
}

bool HasNativeSenseOfChoiceClosure() { return false; }
bool HasNativePresentReturnClosure() { return false; }
bool HasNativeBreakTrapClosure() { return false; }

}  // namespace tw_late_event_734_741_0828

namespace sugar_time {

void SendDayState(CUser* user,
                  uint32_t remaining_seconds,
                  uint8_t next_stage,
                  uint8_t stage_index) {
    if (!user || next_stage > 4u || stage_index > 3u) {
        return;
    }
    PacketGuard packet;
    // sub_5A8EA0 raw-reads one packed eight-byte record.
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_EVENT_SUGARTIME_DAY);
    packet.put_int(static_cast<int>(remaining_seconds));
    packet.put_byte(next_stage);
    packet.put_byte(stage_index);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.finalize(true);
    user->Send(packet);
}

void SendFinalState(CUser* user,
                    uint32_t accounted_seconds,
                    uint32_t selected_goal_seconds,
                    uint8_t final_claimed) {
    if (!user || final_claimed > 1u ||
        (selected_goal_seconds != 0u &&
         !IsGoalSeconds(selected_goal_seconds))) {
        return;
    }
    PacketGuard packet;
    // sub_5A8EE0 raw-reads one packed twelve-byte record.
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_EVENT_SUGARTIME_FINAL);
    packet.put_int(static_cast<int>(accounted_seconds));
    packet.put_int(static_cast<int>(selected_goal_seconds));
    packet.put_byte(final_claimed);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.put_byte(0);
    packet.finalize(true);
    user->Send(packet);
}

bool HasNativeSugarTimeClosure() {
    return GetSugarTimeConfig().available();
}

void RequestLoginState(CUser* user) {
    SugarTimeConfig& config = GetSugarTimeConfig();
    CEnvironment* const environment = G_CEnvironment();
    const int reset_hour = environment
        ? environment->get_fatigue_time() : -1;
    if (!EventOwnerRequested() || !user || user->get_acc_id() <= 0 ||
        user->getCurCharacNo() <= 0 || !config.available()) {
        return;
    }
    if (reset_hour < 0 || reset_hour > 23) {
        return;
    }
    SugarTimeSession session;
    session.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    session.event_key = config.event_key();
    session.session_token = NextSessionToken();
    session.daily_reset_hour = static_cast<uint8_t>(reset_hour);
    session.accounted_epoch = CurrentEpoch();
    if (session.accounted_epoch == 0u) {
        return;
    }
    SugarTimeSessions()[static_cast<uint32_t>(user->get_acc_id())] =
        session;
    SugarTimeSession* const installed = FindSession(user);
    if (!installed ||
        !QueueOperation(user, *installed,
                        sugar_time_db::OPERATION_LOAD,
                        0u, 0u)) {
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
    }
}

void OnWorldReady(CUser* user) {
    if (!user || user->get_state() <= 2) {
        return;
    }
    SugarTimeSession* const session = FindSession(user);
    if (!EventOwnerRequested()) {
        if (session) {
            const uint32_t closing = session->closing_epoch != 0u
                ? session->closing_epoch : EventClosingEpoch();
            if (closing != 0u) {
                (void)SealSession(user, *session, closing);
            }
            SugarTimeSessions().erase(
                static_cast<uint32_t>(user->get_acc_id()));
        }
        return;
    }
    if (session && session->closing_epoch != 0u) {
        const uint32_t closing = session->closing_epoch;
        (void)SealSession(user, *session, closing);
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
        RequestLoginState(user);
        return;
    }
    if (!session) {
        RequestLoginState(user);
    } else if (session->loaded && session->campaign_authorized &&
               EventActive()) {
        SendSessionState(user, *session);
    } else if (!session->db_pending) {
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
        RequestLoginState(user);
    }
}

void OnTick(CUser* user) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return;
    }
    SugarTimeSession* const session = FindSession(user);
    if (!EventOwnerRequested()) {
        if (session) {
            const uint32_t closing = session->closing_epoch != 0u
                ? session->closing_epoch : EventClosingEpoch();
            if (closing != 0u) {
                (void)SealSession(user, *session, closing);
            }
            SugarTimeSessions().erase(
                static_cast<uint32_t>(user->get_acc_id()));
        }
        return;
    }
    if (!session) {
        RequestLoginState(user);
        return;
    }
    if (session->closing_epoch != 0u) {
        const uint32_t closing = session->closing_epoch;
        (void)SealSession(user, *session, closing);
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
        RequestLoginState(user);
        return;
    }
    const uint32_t now = CurrentEpoch();
    if (session->db_pending) {
        if (now == 0u || session->pending_since_epoch == 0u ||
            now < session->pending_since_epoch ||
            now - session->pending_since_epoch >=
                kPendingRecoverySeconds) {
            SugarTimeSessions().erase(
                static_cast<uint32_t>(user->get_acc_id()));
            RequestLoginState(user);
        }
        return;
    }
    if (!session->loaded || !session->campaign_authorized ||
        !EventActive() || session->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo())) {
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
        RequestLoginState(user);
        return;
    }
    if (now > session->accounted_epoch &&
        now - session->accounted_epoch >= kCheckpointSeconds) {
        (void)QueueOperation(user, *session,
                             sugar_time_db::OPERATION_CHECKPOINT,
                             0u, 0u, now);
    }
}

namespace {

struct TickAllUsers {
    void operator()(CUser* user) const { sugar_time::OnTick(user); }
};

struct StartAllUsers {
    void operator()(CUser* user) const {
        sugar_time::RequestLoginState(user);
    }
};

struct EndAllUsers {
    explicit EndAllUsers(uint32_t value) : epoch(value) {}
    void operator()(CUser* user) const {
        SugarTimeSession* const session = FindSession(user);
        if (!user || !session) {
            return;
        }
        session->closing_epoch = epoch;
        (void)SealSession(user, *session, epoch);
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
    }
    uint32_t epoch;
};

}  // namespace

void OnGlobalMinuteTick() {
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(TickAllUsers());
    }
}

void OnEventStarted() {
    if (!GetSugarTimeConfig().available()) {
        EventOwnerRequested() = false;
        EventActive() = false;
        return;
    }
    EventClosingEpoch() = 0u;
    EventOwnerRequested() = true;
    // The authoritative DB campaign row, not this lifecycle hint, opens the
    // packet path.  EventActive becomes true only after a successful load.
    EventActive() = false;
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(StartAllUsers());
    }
}

void OnEventEnded() {
    const uint32_t closing = CurrentEpoch();
    EventOwnerRequested() = false;
    EventActive() = false;
    EventClosingEpoch() = closing;
    if (closing == 0u) {
        return;
    }
    for (SugarTimeSessionMap::iterator it = SugarTimeSessions().begin();
         it != SugarTimeSessions().end(); ++it) {
        it->second.closing_epoch = closing;
    }
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(EndAllUsers(closing));
    }
}

void ResetSession(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return;
    }
    SugarTimeSessionMap::iterator found = SugarTimeSessions().find(
        static_cast<uint32_t>(user->get_acc_id()));
    if (found == SugarTimeSessions().end()) {
        return;
    }
    uint32_t closing = found->second.closing_epoch;
    if (closing == 0u && EventActive()) {
        closing = CurrentEpoch();
    }
    if (closing != 0u) {
        (void)SealSession(user, found->second, closing);
    }
    SugarTimeSessions().erase(found);
}

}  // namespace sugar_time_0828

Dispatcher_SugarTimeDayReward::
    Dispatcher_SugarTimeDayReward()
    : IPacketDispatcher<MSG_BASE, ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_SugarTimeDayReward::~
    Dispatcher_SugarTimeDayReward() {}

int Dispatcher_SugarTimeDayReward::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    unsigned char stage_value = 0;
    if (!user || packet.get_byte(stage_value) != 1 || stage_value < 1u ||
        stage_value > 4 || !EventActive()) {
        SendCommandError(user, ENUM_CMDPACKET_EVENT_SUGARTIME_DAY_REWARD);
        return 0;
    }
    SugarTimeSession* const session = FindSession(user);
    if (!session || !session->loaded || !session->campaign_authorized ||
        session->db_pending ||
        session->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo())) {
        SendCommandError(user, ENUM_CMDPACKET_EVENT_SUGARTIME_DAY_REWARD);
        return 0;
    }
    const uint32_t now = CurrentEpoch();
    uint32_t daily = 0u;
    uint32_t total = 0u;
    EffectiveElapsed(*session, now, daily, total);
    (void)total;
    const uint8_t claimed = EffectiveClaimedCount(*session, now);
    const uint8_t requested = static_cast<uint8_t>(stage_value);
    if (requested != static_cast<uint8_t>(claimed + 1u) ||
        requested > GetSugarTimeConfig().daily_seconds().size() ||
        daily < GetSugarTimeConfig().daily_seconds()[requested - 1u] ||
        !QueueOperation(user, *session,
                        sugar_time_db::OPERATION_CLAIM_DAY,
                        requested, 0u, now)) {
        SendCommandError(user, ENUM_CMDPACKET_EVENT_SUGARTIME_DAY_REWARD);
    }
    return 0;
}

int Dispatcher_SugarTimeDayReward::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SugarTimeDayReward::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_SugarTimeFinalReward::
    Dispatcher_SugarTimeFinalReward()
    : IPacketDispatcher<MSG_BASE, ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_SugarTimeFinalReward::~
    Dispatcher_SugarTimeFinalReward() {}

int Dispatcher_SugarTimeFinalReward::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    if (!user || !EventActive()) {
        SendCommandError(user,
                         ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_REWARD);
        return 0;
    }
    SugarTimeSession* const session = FindSession(user);
    if (!session || !session->loaded || !session->campaign_authorized ||
        session->db_pending ||
        session->selected_goal == 0u || session->final_claimed != 0u) {
        SendCommandError(user,
                         ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_REWARD);
        return 0;
    }
    const uint32_t now = CurrentEpoch();
    uint32_t daily = 0u;
    uint32_t total = 0u;
    EffectiveElapsed(*session, now, daily, total);
    (void)daily;
    if (total < session->selected_goal ||
        !QueueOperation(user, *session,
                        sugar_time_db::OPERATION_CLAIM_FINAL,
                        0u, 0u, now)) {
        SendCommandError(user,
                         ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_REWARD);
    }
    return 0;
}

int Dispatcher_SugarTimeFinalReward::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SugarTimeFinalReward::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_SugarTimeFinalPick::Dispatcher_SugarTimeFinalPick()
    : IPacketDispatcher<MSG_BASE, ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_SugarTimeFinalPick::~Dispatcher_SugarTimeFinalPick() {}

int Dispatcher_SugarTimeFinalPick::dispatch_sig(
    CUser* user, PacketBuf& packet) {
    int goal_value = 0;
    if (!user || packet.get_int(goal_value) != 1 || goal_value <= 0 ||
        !EventActive() ||
        !IsGoalSeconds(static_cast<uint32_t>(goal_value))) {
        SendCommandError(user, ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_PICK);
        return 0;
    }
    SugarTimeSession* const session = FindSession(user);
    if (!session || !session->loaded || !session->campaign_authorized ||
        session->db_pending ||
        session->selected_goal != 0u ||
        !QueueOperation(user, *session,
                        sugar_time_db::OPERATION_PICK_FINAL,
                        0u, static_cast<uint32_t>(goal_value))) {
        SendCommandError(user, ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_PICK);
    }
    return 0;
}

int Dispatcher_SugarTimeFinalPick::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SugarTimeFinalPick::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

InterSugarTime::InterSugarTime()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              sugar_time_db::kTaiwanInnerCode)) {}

InterSugarTime::~InterSugarTime() {}

int InterSugarTime::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    if (!user || !payload || payload_size !=
        static_cast<int>(sizeof(sugar_time_db::Message))) {
        return 0;
    }
    const sugar_time_db::Message response =
        *reinterpret_cast<const sugar_time_db::Message*>(payload);
    if (!IsCurrentResponse(user, response)) {
        return 0;
    }
    SugarTimeSession* const session = FindSession(user);
    if (session->db_pending && session->pending_operation !=
                                   response.operation) {
        return 0;
    }
    session->db_pending = false;
    session->pending_since_epoch = 0u;
    session->pending_operation = 0u;
    if (response.result != sugar_time_db::RESULT_SUCCESS) {
        const bool campaign_inactive = response.result ==
            sugar_time_db::RESULT_CAMPAIGN_INACTIVE;
        if (campaign_inactive) {
            session->campaign_authorized = false;
            EventActive() = false;
        }
        if (response.operation == sugar_time_db::OPERATION_LOAD) {
            SugarTimeSessions().erase(
                static_cast<uint32_t>(user->get_acc_id()));
        } else if (response.operation !=
                   sugar_time_db::OPERATION_CHECKPOINT) {
            const int command = response.operation ==
                    sugar_time_db::OPERATION_CLAIM_DAY
                ? ENUM_CMDPACKET_EVENT_SUGARTIME_DAY_REWARD
                : response.operation ==
                      sugar_time_db::OPERATION_CLAIM_FINAL
                    ? ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_REWARD
                    : ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_PICK;
            SendCommandError(user, command);
        }
        if (campaign_inactive && response.operation !=
                sugar_time_db::OPERATION_LOAD) {
            SugarTimeSessions().erase(
                static_cast<uint32_t>(user->get_acc_id()));
        }
        return 0;
    }

    ApplyResponse(*session, response);
    if (EventOwnerRequested()) {
        EventActive() = true;
    }
    if (session->closing_epoch != 0u || !EventActive()) {
        const uint32_t closing = session->closing_epoch != 0u
            ? session->closing_epoch : EventClosingEpoch();
        if (closing != 0u && response.observed_epoch < closing) {
            (void)SealSession(user, *session, closing);
        }
        SugarTimeSessions().erase(
            static_cast<uint32_t>(user->get_acc_id()));
        if (EventActive()) {
            sugar_time::RequestLoginState(user);
        }
        return 0;
    }

    if (response.operation == sugar_time_db::OPERATION_LOAD ||
        response.operation ==
            sugar_time_db::OPERATION_CHECKPOINT) {
        SendSessionState(user, *session);
    } else {
        const int command = response.operation ==
                sugar_time_db::OPERATION_CLAIM_DAY
            ? ENUM_CMDPACKET_EVENT_SUGARTIME_DAY_REWARD
            : response.operation ==
                  sugar_time_db::OPERATION_CLAIM_FINAL
                ? ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_REWARD
                : ENUM_CMDPACKET_EVENT_SUGARTIME_FINAL_PICK;
        SendCommandSuccess(user, command);
        SendSessionState(user, *session);
    }
    if (response.new_mail != 0u) {
        NotifyNewMail(user);
    }
    return 0;
}
