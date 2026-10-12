#include "old_game/activity/schedule/event_shop_attendance.h"

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/system_time.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/itemshop/secret_shop_script.h"
#include "old_data/db/event/db_event_attendance.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/security/policy/secu_protection_field_abi.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <limits>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

const int kEventShopNpc = 164;
const int kSecretShopEventId = 58;
const int kAttendanceEventId = 96;
const char kEventShopPvf[] = "Etc/secretshop_event.etc";
const char kAttendancePvfDirectory[] = "Event/chn_event/";
const char kAttendancePvf[] = "chn_attendanceevent.evt";
const uint32_t kCheckpointSeconds = 60u;

struct Reward {
    Reward() : item_id(0), count(0) {}
    Reward(int item_id_value, int count_value)
        : item_id(item_id_value), count(count_value) {}

    int item_id;
    int count;
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
    for (unsigned int byte = 0; byte < 4u; ++byte) {
        hash ^= (value >> (byte * 8u)) & 0xFFu;
        hash *= 16777619u;
    }
}

class AttendanceConfig {
public:
    AttendanceConfig()
        : loaded_(false), event_key_(0u), process_seconds_(),
          daily_rewards_(), sum_seconds_(), sum_rewards_() {}

    bool available() { return loaded_ || Load(); }
    uint32_t event_key() const { return event_key_; }
    const std::vector<uint32_t>& process_seconds() const {
        return process_seconds_;
    }
    const std::vector<Reward>& daily_rewards() const {
        return daily_rewards_;
    }
    const std::vector<uint32_t>& sum_seconds() const {
        return sum_seconds_;
    }
    const std::vector<Reward>& sum_rewards() const {
        return sum_rewards_;
    }

private:
    bool Load() {
        if (!loadRDARScriptFile(kAttendancePvfDirectory,
                                kAttendancePvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        std::vector<int> process_values;
        std::vector<int> reward_values;
        std::vector<int> sum_values;
        std::string token;
        bool valid = true;
        while (valid && ScanType(token, true) == 1) {
            if (token == "[process time seconds]") {
                valid = process_values.empty() &&
                    ParseIntSection("[/process time seconds]",
                                    0,
                                    process_values);
            } else if (token == "[reward item]") {
                valid = reward_values.empty() &&
                    ParseIntSection("[/reward item]", 0, reward_values);
            } else if (token == "[goal time and reward item for sum]") {
                // The shipped Taiwan PVF closes this section with the
                // misspelled word "type".  Accept that exact artifact and a
                // corrected closing tag, but no unbounded/truncated section.
                valid = sum_values.empty() &&
                    ParseIntSection(
                        "[/goal type and reward item for sum]",
                        "[/goal time and reward item for sum]",
                        sum_values);
            }
        }
        if (!valid || process_values.size() != 4u ||
            reward_values.size() != 8u || sum_values.size() != 9u) {
            return false;
        }

        std::vector<uint32_t> process_seconds;
        std::vector<Reward> daily_rewards;
        std::vector<uint32_t> sum_seconds;
        std::vector<Reward> sum_rewards;
        uint32_t previous = 0u;
        for (unsigned int index = 0; index < 4u; ++index) {
            const int seconds = process_values[index];
            const int item_id = reward_values[index * 2u];
            const int count = reward_values[index * 2u + 1u];
            if (seconds <= 0 || static_cast<uint32_t>(seconds) <= previous ||
                item_id <= 0 || count <= 0) {
                return false;
            }
            process_seconds.push_back(static_cast<uint32_t>(seconds));
            daily_rewards.push_back(Reward(item_id, count));
            previous = static_cast<uint32_t>(seconds);
        }
        previous = 0u;
        for (unsigned int index = 0; index < 3u; ++index) {
            const int seconds = sum_values[index * 3u];
            const int item_id = sum_values[index * 3u + 1u];
            const int count = sum_values[index * 3u + 2u];
            if (seconds <= 0 || static_cast<uint32_t>(seconds) <= previous ||
                item_id <= 0 || count <= 0) {
                return false;
            }
            sum_seconds.push_back(static_cast<uint32_t>(seconds));
            sum_rewards.push_back(Reward(item_id, count));
            previous = static_cast<uint32_t>(seconds);
        }

        uint32_t event_key = 2166136261u;
        for (unsigned int index = 0; index < process_seconds.size(); ++index) {
            FnvMix(event_key, process_seconds[index]);
            FnvMix(event_key,
                   static_cast<uint32_t>(daily_rewards[index].item_id));
            FnvMix(event_key,
                   static_cast<uint32_t>(daily_rewards[index].count));
        }
        for (unsigned int index = 0; index < sum_seconds.size(); ++index) {
            FnvMix(event_key, sum_seconds[index]);
            FnvMix(event_key,
                   static_cast<uint32_t>(sum_rewards[index].item_id));
            FnvMix(event_key,
                   static_cast<uint32_t>(sum_rewards[index].count));
        }
        if (event_key == 0u) {
            event_key = 1u;
        }

        process_seconds_.swap(process_seconds);
        daily_rewards_.swap(daily_rewards);
        sum_seconds_.swap(sum_seconds);
        sum_rewards_.swap(sum_rewards);
        event_key_ = event_key;
        loaded_ = true;
        return true;
    }

    bool loaded_;
    uint32_t event_key_;
    std::vector<uint32_t> process_seconds_;
    std::vector<Reward> daily_rewards_;
    std::vector<uint32_t> sum_seconds_;
    std::vector<Reward> sum_rewards_;
};

AttendanceConfig& GetAttendanceConfig() {
    static AttendanceConfig config;
    return config;
}

class EventShopConfig {
public:
    EventShopConfig() : loaded_(false), script_() {}

    bool available() { return loaded_ || Load(); }

    bool ItemsForLevel(int level,
                       std::vector<secretshop::SALE_INFO>& items) {
        items.clear();
        if (!available()) {
            return false;
        }
        std::map<int, stNpcSales>::iterator npc =
            script_.npc_sales.find(kEventShopNpc);
        if (npc == script_.npc_sales.end()) {
            return false;
        }
        const int level_index = script_.GetLevelIdx(level);
        std::map<int, stDungeonSales>::iterator sales =
            npc->second.level_sales.find(level_index);
        if (sales == npc->second.level_sales.end() ||
            sales->second.sale_count != 3 ||
            sales->second.sales.size() != 3u) {
            return false;
        }
        for (std::map<int, stSaleInfo>::iterator item =
                 sales->second.sales.begin();
             item != sales->second.sales.end(); ++item) {
            if (item->second.item_index <= 0 ||
                item->second.buy_rule != eBuyRule_Gold ||
                item->second.price != 0 || item->second.value_0c != 0 ||
                item->second.value_10 != 1 || item->second.ratio != 1000) {
                items.clear();
                return false;
            }
            secretshop::SALE_INFO copied;
            script_.CopyItem(copied, item->second);
            items.push_back(copied);
        }
        return items.size() == 3u;
    }

private:
    bool Load() {
        const int imported = importSecretShopScript(&script_, kEventShopPvf);
        // importSecretShopScript mirrors the old DataManager loader and leaves
        // the global RDAR scanner open for its caller to release.
        unloadRDARScriptFile();
        if (imported != 1) {
            script_.clear();
            return false;
        }
        std::map<int, stNpcSales>::const_iterator npc =
            script_.npc_sales.find(kEventShopNpc);
        if (npc == script_.npc_sales.end() ||
            npc->second.level_sales.size() != 9u) {
            script_.clear();
            return false;
        }
        loaded_ = true;
        return true;
    }

    bool loaded_;
    SecretShopScript script_;
};

EventShopConfig& GetEventShopConfig() {
    static EventShopConfig config;
    return config;
}

bool IsRepeatEventActive(int event_id) {
    if (!GlobalData::s_event_manager) {
        return false;
    }
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(event_id);
    return event && event->IsEventing(0) == 1;
}

void SendCommandError(CUser* user,
                      ENUM_CMDPACKET command,
                      uint8_t error) {
    if (user) {
        user->SendCmdErrorPacket(command, error);
    }
}

bool SameSale(const secretshop::SALE_INFO& left,
              const secretshop::SALE_INFO& right) {
    return left.item_index == right.item_index &&
           left.buy_rule == right.buy_rule &&
           left.price == right.price &&
           left.value_0c == right.value_0c &&
           left.value_10 == right.value_10 &&
           left.value_14 == right.value_14 &&
           left.event_item == right.event_item;
}

const secretshop::SALE_INFO* FindSale(
    const std::vector<secretshop::SALE_INFO>& items,
    int item_id) {
    for (std::vector<secretshop::SALE_INFO>::const_iterator item =
             items.begin(); item != items.end(); ++item) {
        if (item->item_index == item_id) {
            return &*item;
        }
    }
    return 0;
}

bool SameRetailer(const secretshop::RETAILER& installed,
                  const std::vector<secretshop::SALE_INFO>& configured) {
    if (installed.size() != configured.size()) {
        return false;
    }
    for (unsigned int index = 0; index < configured.size(); ++index) {
        if (!SameSale(installed[index], configured[index])) {
            return false;
        }
    }
    return true;
}

int RemainingPurchaseCount(secretshop::SECRET_SHOP_INFO& shop,
                           const secretshop::SALE_INFO& sale,
                           int count) {
    secretshop::BUY_INFO* const bought =
        shop.shopper.GetBuyInfo(sale.item_index);
    const int already_bought = bought ? bought->count : 0;
    if (count <= 0 || already_bought < 0 || sale.value_0c < 0 ||
        already_bought > sale.value_0c ||
        count > sale.value_0c - already_bought) {
        return -1;
    }
    return sale.value_0c - already_bought - count;
}

int InsertShopItem(CUser* user,
                   CItem* prototype,
                   int item_id,
                   int count,
                   Inven_Item& item) {
    if (!user || !prototype || !prototype->vtable_) {
        return -1;
    }
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = item_id;
    if (prototype->is_stackable()) {
        item.add_info_ = count;
    }
    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        return -1;
    }
    const int slot = inventory->insertItemIntoInventory(
        item, static_cast<eItemAddReason>(44), true, false);
    return slot >= 0 ? slot : -1;
}

void SendEventShopSuccess(CUser* user,
                          int inventory_slot,
                          Inven_Item& item,
                          int remaining_count) {
    if (!user) {
        return;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory) {
        return;
    }
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: CMD687 handler sub_55A9F0 consumes exactly the
    // framework success byte followed by this 29-byte tail.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM);
    packet.put_byte(1);
    packet.put_int(inventory->get_money());
    packet.put_short(inventory_slot);
    packet.put_int(item.item_id_);
    packet.put_int(item.add_info_);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    packet.put_int(-1);
    packet.put_int(0);
    packet.put_int(remaining_count);
    packet.finalize(true);
    user->Send(packet);
}

uint32_t CurrentEpoch() {
    const int now = GlobalData::s_systemTime_.getCurSec();
    return now > 0 ? static_cast<uint32_t>(now) : 0u;
}

uint32_t LocalDayStamp(uint32_t epoch) {
    const time_t value = static_cast<time_t>(epoch);
    tm local_time;
    tm* const result = localtime_r(&value, &local_time);
    if (!result) {
        return 0u;
    }
    return static_cast<uint32_t>((result->tm_year + 1900) * 10000 +
                                 (result->tm_mon + 1) * 100 +
                                 result->tm_mday);
}

uint32_t SecondsSinceLocalMidnight(uint32_t epoch) {
    const time_t value = static_cast<time_t>(epoch);
    tm local_time;
    tm* const result = localtime_r(&value, &local_time);
    if (!result) {
        return 0u;
    }
    return static_cast<uint32_t>(result->tm_hour * 3600 +
                                 result->tm_min * 60 +
                                 result->tm_sec);
}

void CalculateDeltas(uint32_t accounted_epoch,
                     uint32_t observed_epoch,
                     uint32_t& daily_delta,
                     uint32_t& total_delta) {
    daily_delta = 0u;
    total_delta = 0u;
    if (accounted_epoch == 0u || observed_epoch <= accounted_epoch) {
        return;
    }
    total_delta = observed_epoch - accounted_epoch;
    if (LocalDayStamp(accounted_epoch) == LocalDayStamp(observed_epoch)) {
        daily_delta = total_delta;
    } else {
        daily_delta = SecondsSinceLocalMidnight(observed_epoch);
        if (daily_delta > total_delta) {
            daily_delta = total_delta;
        }
    }
}

struct AttendanceState {
    AttendanceState()
        : charac_no(0u), event_key(0u), session_token(0u),
          day_stamp(0u), daily_elapsed(0u), total_elapsed(0u),
          revision(0u), accounted_epoch(0u), closing_epoch(0u),
          claimed_mask(0u),
          sum_claimed_mask(0u), loaded(false), db_pending(false) {}

    uint32_t charac_no;
    uint32_t event_key;
    uint32_t session_token;
    uint32_t day_stamp;
    uint32_t daily_elapsed;
    uint32_t total_elapsed;
    uint32_t revision;
    uint32_t accounted_epoch;
    // Nonzero only after AttendanceEvent::EndEvent observes the exact shared
    // stop boundary.  Retried final checkpoints are always clamped here and
    // can never accrue post-event wall time.
    uint32_t closing_epoch;
    uint8_t claimed_mask;
    uint8_t sum_claimed_mask;
    bool loaded;
    bool db_pending;
};

typedef std::map<uint32_t, AttendanceState> AttendanceStateMap;

AttendanceStateMap& AttendanceStates() {
    static AttendanceStateMap states;
    return states;
}

uint32_t NextSessionToken() {
    static uint32_t next_token = 0u;
    ++next_token;
    if (next_token == 0u) {
        ++next_token;
    }
    return next_token;
}

AttendanceState* FindAttendanceState(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return 0;
    }
    AttendanceStateMap::iterator found = AttendanceStates().find(
        static_cast<uint32_t>(user->get_acc_id()));
    return found == AttendanceStates().end() ? 0 : &found->second;
}

bool IsCurrentAttendanceResponse(
    CUser* user,
    const event_attendance_db::Message& message) {
    AttendanceState* const state = FindAttendanceState(user);
    return state && message.session_token != 0u &&
           state->session_token == message.session_token &&
           state->charac_no == message.charac_no &&
           state->event_key == message.event_key &&
           message.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           message.server_group == user->GetServerGroup();
}

bool ExportMailReward(const Reward& source,
                      event_attendance_db::MailReward& destination) {
    if (!G_CDataManager() || source.item_id <= 0 || source.count <= 0) {
        return false;
    }
    CItem* const prototype = G_CDataManager()->find_item(source.item_id);
    if (!prototype || !prototype->vtable_) {
        return false;
    }
    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.item_id_ = prototype->get_index();
    item.set_add_info(source.count);
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
    std::memcpy(destination.item_guid,
                record.item_guid,
                sizeof(destination.item_guid));
    std::memcpy(destination.random_option,
                record.random_option,
                sizeof(destination.random_option));
    return destination.item_id != 0u && destination.add_info != 0u;
}

bool FillAttendanceMessage(
    CUser* user,
    AttendanceState& state,
    uint8_t operation,
    uint32_t observed_epoch,
    event_attendance_db::Message& message) {
    AttendanceConfig& config = GetAttendanceConfig();
    if (!user || !config.available() ||
        config.event_key() != state.event_key) {
        return false;
    }
    event_attendance_db::ClearMessage(message);
    message.account_id = user->get_acc_id() > 0
        ? static_cast<uint32_t>(user->get_acc_id()) : 0u;
    message.charac_no = user->getCurCharacNo() > 0
        ? static_cast<uint32_t>(user->getCurCharacNo()) : 0u;
    message.event_key = config.event_key();
    message.session_token = state.session_token;
    message.server_group = user->GetServerGroup();
    message.operation = operation;
    message.observed_epoch = observed_epoch;
    message.day_stamp = LocalDayStamp(observed_epoch);
    message.daily_elapsed_seconds = state.daily_elapsed;
    message.total_elapsed_seconds = state.total_elapsed;
    message.revision = state.revision;
    message.claimed_mask = state.claimed_mask;
    message.sum_claimed_mask = state.sum_claimed_mask;
    if (operation != event_attendance_db::kOperationLoad) {
        CalculateDeltas(state.accounted_epoch,
                        observed_epoch,
                        message.daily_delta_seconds,
                        message.total_delta_seconds);
    }

    const std::vector<uint32_t>& daily_times = config.process_seconds();
    const std::vector<Reward>& daily_rewards = config.daily_rewards();
    const std::vector<uint32_t>& sum_times = config.sum_seconds();
    const std::vector<Reward>& sum_rewards = config.sum_rewards();
    for (unsigned int index = 0; index < daily_times.size(); ++index) {
        message.process_seconds[index] = daily_times[index];
        if (!ExportMailReward(daily_rewards[index],
                              message.daily_rewards[index])) {
            return false;
        }
    }
    for (unsigned int index = 0; index < sum_times.size(); ++index) {
        message.sum_seconds[index] = sum_times[index];
        if (!ExportMailReward(sum_rewards[index],
                              message.sum_rewards[index])) {
            return false;
        }
    }
    return message.account_id != 0u && message.charac_no != 0u &&
           message.session_token != 0u && message.day_stamp != 0u;
}

uint32_t EffectiveDailyElapsed(const AttendanceState& state,
                               uint32_t now) {
    uint32_t daily_delta = 0u;
    uint32_t total_delta = 0u;
    CalculateDeltas(state.accounted_epoch, now, daily_delta, total_delta);
    (void)total_delta;
    const uint32_t today = LocalDayStamp(now);
    const uint32_t base = state.day_stamp == today
        ? state.daily_elapsed : 0u;
    if (base > std::numeric_limits<uint32_t>::max() - daily_delta) {
        return std::numeric_limits<uint32_t>::max();
    }
    return base + daily_delta;
}

uint8_t MaturedDailyCount(uint32_t elapsed) {
    AttendanceConfig& config = GetAttendanceConfig();
    if (!config.available()) {
        return 0u;
    }
    uint8_t matured = 0u;
    const std::vector<uint32_t>& times = config.process_seconds();
    for (unsigned int index = 0; index < times.size(); ++index) {
        if (elapsed >= times[index]) {
            ++matured;
        }
    }
    return matured;
}

void SendAttendanceState(CUser* user, const AttendanceState& state) {
    if (!user || user->get_state() <= 2 || !state.loaded ||
        !GetAttendanceConfig().available()) {
        return;
    }
    const uint32_t now = CurrentEpoch();
    const uint32_t elapsed = EffectiveDailyElapsed(state, now);
    const uint8_t matured = MaturedDailyCount(elapsed);
    uint32_t remaining = 0u;
    const std::vector<uint32_t>& times =
        GetAttendanceConfig().process_seconds();
    if (matured < times.size() && elapsed < times[matured]) {
        remaining = times[matured] - elapsed;
    }

    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: NOTI651 handler sub_409E90 consumes exactly
    // u32 remaining seconds, u8 matured count, u8 daily claimed mask.
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_EVENT_ATTENDANCE_DATA);
    packet.put_int(static_cast<int>(remaining));
    packet.put_byte(static_cast<int>(matured));
    packet.put_byte(static_cast<int>(
        state.day_stamp == LocalDayStamp(now) ? state.claimed_mask : 0u));
    packet.finalize(true);
    user->Send(packet);
}

void SendAttendanceError(CUser* user, uint8_t result) {
    const uint8_t error =
        result == event_attendance_db::kResultInvalidEligibility ||
                result == event_attendance_db::kResultRewardUnavailable
            ? result
            : 1u;
    SendCommandError(user,
                     ENUM_CMDPACKET_EVENT_ATTENDANCE_REWARD,
                     error);
}

void SendAttendanceSuccess(CUser* user) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: CMD689 response handler sub_409D50 consumes only
    // the framework success byte and no command-specific tail.
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_EVENT_ATTENDANCE_REWARD);
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

bool QueueAttendanceOperation(CUser* user,
                              AttendanceState& state,
                              uint8_t operation,
                              uint8_t requested_tier,
                              uint8_t client_stage,
                              uint32_t observed_epoch = 0u,
                              bool send_reply = true) {
    const bool checkpoint =
        operation == event_attendance_db::kOperationCheckpoint;
    const bool load =
        operation == event_attendance_db::kOperationLoad;
    const bool durable_final = checkpoint && !send_reply;
    if (state.db_pending && !durable_final) {
        return false;
    }
    const uint32_t now = observed_epoch != 0u
                             ? observed_epoch
                             : CurrentEpoch();
    event_attendance_db::Message request;
    if (now == 0u ||
        !FillAttendanceMessage(user, state, operation, now, request)) {
        return false;
    }
    request.requested_tier = requested_tier;
    request.client_stage = client_stage;
    request.reply_user_idx = (load || (checkpoint && send_reply))
                                 ? user->GetUID()
                                 : -1;
    // Load also uses a userless DB envelope.  The transaction therefore seeds
    // the durable high-watermark even if the character disconnects before the
    // DB worker runs; a following userless final checkpoint can then account
    // the exact login-to-disconnect interval.  Its reply remains best-effort.
    const int outer_user_idx = (load || checkpoint) ? -1 : user->GetUID();
    if ((load && request.reply_user_idx < 0) ||
        (checkpoint && send_reply && request.reply_user_idx < 0) ||
        !event_attendance_db::Queue(outer_user_idx, request)) {
        return false;
    }
    if (!durable_final) {
        state.db_pending = true;
    }
    return true;
}

bool SealAttendanceSession(CUser* user,
                           AttendanceState& state,
                           uint32_t closing_epoch) {
    if (!user || closing_epoch == 0u ||
        state.charac_no != static_cast<uint32_t>(user->getCurCharacNo())) {
        return false;
    }
    // This checkpoint is deliberately userless.  Once DB_Q accepts the value
    // copy, a later disconnect/pool reuse cannot suppress the DB transaction;
    // no in-memory reply is needed because the session is being discarded.
    return QueueAttendanceOperation(
        user,
        state,
        event_attendance_db::kOperationCheckpoint,
        0u,
        0u,
        closing_epoch,
        false);
}

void ApplyAttendanceResponse(
    AttendanceState& state,
    const event_attendance_db::Message& message) {
    state.day_stamp = message.day_stamp;
    state.daily_elapsed = message.daily_elapsed_seconds;
    state.total_elapsed = message.total_elapsed_seconds;
    state.revision = message.revision;
    state.claimed_mask = message.claimed_mask;
    state.sum_claimed_mask = message.sum_claimed_mask;
    if (message.observed_epoch >= state.accounted_epoch) {
        state.accounted_epoch = message.observed_epoch;
    }
    state.loaded = true;
}

}  // namespace

namespace secret_event_shop {

bool OpenForUser(CUser* user, int npc_index) {
    if (!user || user->get_state() != 5 || npc_index != kEventShopNpc ||
        !G_CDataManager() || !IsRepeatEventActive(kSecretShopEventId)) {
        return false;
    }
    std::vector<secretshop::SALE_INFO> items;
    if (!GetEventShopConfig().ItemsForLevel(user->get_charac_level(), items)) {
        return false;
    }
    secretshop::SECRET_SHOP_DATA* const data = user->GetSecretShopData();
    secretshop::SECRET_SHOP_INFO* const shop = user->GetUserSecretShop();
    if (!data || !shop) {
        return false;
    }
    // The role-77 entity owner may re-emit its open notification while the
    // same encounter is alive.  Preserve SHOPPER counts for an identical
    // retailer; resetting them here would turn the per-offer max into a
    // reconnect/reopen bypass.  A mismatched retailer under the same live
    // state is rejected until the authoritative entity owner closes it.
    if (data->state == kEventShopNpc) {
        if (!SameRetailer(shop->retailer, items)) {
            return false;
        }
    } else {
        shop->clear();
        for (std::vector<secretshop::SALE_INFO>::const_iterator item =
                 items.begin(); item != items.end(); ++item) {
            shop->retailer.push_back(*item);
        }
        data->state = kEventShopNpc;
    }

    PacketGuard npc;
    // CLIENT_IDA_EVIDENCE: NOTI647 handler sub_581650 reads one u32 NPC id.
    npc.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                   ENUM_NOTIPACKET_SECRET_EVENT_SHOP_NPC);
    npc.put_int(kEventShopNpc);
    npc.finalize(true);
    user->Send(npc);

    PacketGuard list;
    // CLIENT_IDA_EVIDENCE: NOTI648 handler sub_58EDF0 reads u32 count and
    // count * 26-byte SALE_INFO projections.
    list.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                    ENUM_NOTIPACKET_SECRET_EVENT_SHOP_ITEM_LIST);
    list.put_int(static_cast<int>(items.size()));
    for (std::vector<secretshop::SALE_INFO>::const_iterator item =
             items.begin(); item != items.end(); ++item) {
        list.put_int(item->item_index);
        list.put_byte(item->buy_rule);
        list.put_int(item->price);
        list.put_int(item->value_0c);
        list.put_int(item->value_10);
        list.put_int(item->value_14);
        list.put_byte(item->event_item);
    }
    list.finalize(true);
    user->Send(list);
    return true;
}

void CloseForUser(CUser* user) {
    if (!user) {
        return;
    }
    secretshop::SECRET_SHOP_INFO* const shop = user->GetUserSecretShop();
    secretshop::SECRET_SHOP_DATA* const data = user->GetSecretShopData();
    if (shop) {
        shop->clear();
    }
    if (data && data->state == kEventShopNpc) {
        data->state = 0;
    }
}

}  // namespace secret_event_shop_0828

namespace event_attendance {

void RequestLoginState(CUser* user) {
    AttendanceConfig& config = GetAttendanceConfig();
    if (!user || user->get_acc_id() <= 0 ||
        user->getCurCharacNo() <= 0 || !config.available() ||
        !IsRepeatEventActive(kAttendanceEventId)) {
        return;
    }
    AttendanceState state;
    state.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    state.event_key = config.event_key();
    state.session_token = NextSessionToken();
    state.accounted_epoch = CurrentEpoch();
    if (state.accounted_epoch == 0u) {
        return;
    }
    AttendanceStates()[static_cast<uint32_t>(user->get_acc_id())] = state;
    AttendanceState* const installed = FindAttendanceState(user);
    if (!installed ||
        !QueueAttendanceOperation(user,
                                  *installed,
                                  event_attendance_db::kOperationLoad,
                                  0u,
                                  0u)) {
        AttendanceStates().erase(
            static_cast<uint32_t>(user->get_acc_id()));
    }
}

void OnWorldReady(CUser* user) {
    if (!user || user->get_state() <= 2) {
        return;
    }
    AttendanceState* const state = FindAttendanceState(user);
    if (state && state->closing_epoch != 0u) {
        if (SealAttendanceSession(user, *state, state->closing_epoch)) {
            AttendanceStates().erase(
                static_cast<uint32_t>(user->get_acc_id()));
            if (IsRepeatEventActive(kAttendanceEventId)) {
                event_attendance::RequestLoginState(user);
            }
        }
        return;
    }
    if (state && state->loaded && state->closing_epoch == 0u) {
        SendAttendanceState(user, *state);
        return;
    }
    if (!state && IsRepeatEventActive(kAttendanceEventId)) {
        // Covers an event activated after character load and any load request
        // that could not be installed before world entry.
        RequestLoginState(user);
    }
}

void OnTick(CUser* user) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return;
    }
    AttendanceState* const state = FindAttendanceState(user);
    if (!IsRepeatEventActive(kAttendanceEventId)) {
        if (state) {
            if (state->closing_epoch == 0u) {
                // EndEvent normally installs the exact shared boundary.  This
                // fallback is only for a pre-existing session observed after
                // an external event shutdown path skipped that callback.
                state->closing_epoch = CurrentEpoch();
            }
            if (SealAttendanceSession(user,
                                      *state,
                                      state->closing_epoch)) {
                AttendanceStates().erase(
                    static_cast<uint32_t>(user->get_acc_id()));
            }
        }
        return;
    }
    // Event 96 can be activated while a character is already online.  The
    // startup-installed global minute timer is the late-start bootstrap, so
    // those users do not have to relog before the server sends NOTI651.
    if (!state) {
        RequestLoginState(user);
        return;
    }
    if (state->closing_epoch != 0u) {
        if (SealAttendanceSession(user,
                                  *state,
                                  state->closing_epoch)) {
            AttendanceStates().erase(
                static_cast<uint32_t>(user->get_acc_id()));
            RequestLoginState(user);
        }
        return;
    }
    if (!state->loaded || state->db_pending ||
        state->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo())) {
        return;
    }
    const uint32_t now = CurrentEpoch();
    if (now <= state->accounted_epoch ||
        now - state->accounted_epoch < kCheckpointSeconds) {
        return;
    }
    QueueAttendanceOperation(
        user,
        *state,
        event_attendance_db::kOperationCheckpoint,
        0u,
        0u);
}

namespace {

struct AttendanceTickAllUsers {
    void operator()(CUser* user) const {
        event_attendance::OnTick(user);
    }
};

struct AttendanceStartAllUsers {
    void operator()(CUser* user) const {
        event_attendance::OnTick(user);
    }
};

struct AttendanceEndAllUsers {
    explicit AttendanceEndAllUsers(uint32_t epoch_value)
        : epoch(epoch_value) {}

    void operator()(CUser* user) const {
        AttendanceState* const state = FindAttendanceState(user);
        if (!user || !state) {
            return;
        }
        state->closing_epoch = epoch;
        if (SealAttendanceSession(user, *state, epoch)) {
            AttendanceStates().erase(
                static_cast<uint32_t>(user->get_acc_id()));
        }
    }

    uint32_t epoch;
};

}  // namespace

void OnGlobalMinuteTick() {
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(AttendanceTickAllUsers());
    }
}

void OnEventStarted() {
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(AttendanceStartAllUsers());
    }
}

void OnEventEnded() {
    const uint32_t closing_epoch = CurrentEpoch();
    if (closing_epoch == 0u) {
        return;
    }
    // Character-load state is installed before GameWorld entry.  Stamp every
    // state first, including state-2 users that ProcessAllUsers cannot see, so
    // their later world-entry/reset path reuses this exact event-end boundary.
    for (AttendanceStateMap::iterator state = AttendanceStates().begin();
         state != AttendanceStates().end(); ++state) {
        state->second.closing_epoch = closing_epoch;
    }
    GameWorld* const world = G_GameWorld();
    if (world) {
        world->ProcessAllUsers(AttendanceEndAllUsers(closing_epoch));
    }
}

void ResetSession(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return;
    }
    AttendanceStateMap::iterator found = AttendanceStates().find(
        static_cast<uint32_t>(user->get_acc_id()));
    if (found == AttendanceStates().end()) {
        return;
    }
    uint32_t closing_epoch = found->second.closing_epoch;
    if (closing_epoch == 0u &&
        IsRepeatEventActive(kAttendanceEventId)) {
        closing_epoch = CurrentEpoch();
    }
    if (closing_epoch != 0u) {
        (void)SealAttendanceSession(user,
                                    found->second,
                                    closing_epoch);
    }
    // The queued DB message owns a value copy; erase immediately so a pooled
    // CUser can never accept the old session's asynchronous response.
    AttendanceStates().erase(found);
}

}  // namespace event_attendance_0828

Dispatcher_SecretEventShopBuyItem::
    Dispatcher_SecretEventShopBuyItem()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_SecretEventShopBuyItem::~
    Dispatcher_SecretEventShopBuyItem() {}

int Dispatcher_SecretEventShopBuyItem::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    unsigned int item_id = 0u;
    unsigned int requested_count = 0u;
    if (!user || packet.get_int(item_id) != 1 ||
        packet.get_int(requested_count) != 1 || item_id == 0u ||
        requested_count == 0u ||
        requested_count >
            static_cast<unsigned int>(std::numeric_limits<int>::max())) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         19u);
        return 0;
    }
    if (user->get_state() != 5) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         19u);
        return 0;
    }
    if (!IsRepeatEventActive(kSecretShopEventId)) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         17u);
        return 0;
    }
    secretshop::SECRET_SHOP_DATA* const data = user->GetSecretShopData();
    secretshop::SECRET_SHOP_INFO* const shop = user->GetUserSecretShop();
    if (!data || !shop || data->state != kEventShopNpc) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         17u);
        return 0;
    }

    std::vector<secretshop::SALE_INFO> configured_items;
    if (!GetEventShopConfig().ItemsForLevel(user->get_charac_level(),
                                            configured_items)) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         17u);
        return 0;
    }
    const secretshop::SALE_INFO* const configured =
        FindSale(configured_items, static_cast<int>(item_id));
    secretshop::SALE_INFO* const installed =
        shop->GetSaleInfo(static_cast<int>(item_id));
    if (!configured || !installed || !SameSale(*configured, *installed) ||
        installed->buy_rule != eBuyRule_Gold) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         17u);
        return 0;
    }
    if (!GlobalData::s_pSecuProtectionField) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         19u);
        return 0;
    }
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(66));
    if (security_error) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         static_cast<uint8_t>(security_error));
        return 0;
    }

    CItem* const prototype = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!prototype) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         17u);
        return 0;
    }
    int count = static_cast<int>(requested_count);
    if (prototype->is_stackable() != 1) {
        count = 1;
    }
    const int remaining = RemainingPurchaseCount(*shop, *installed, count);
    if (remaining < 0) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         95u);
        return 0;
    }
    const unsigned long long required_money =
        static_cast<unsigned long long>(count) *
        static_cast<unsigned int>(installed->price);
    if (required_money >
            static_cast<unsigned long long>(
                std::numeric_limits<int>::max()) ||
        user->getCurCharacMoney() < static_cast<int>(required_money)) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         10u);
        return 0;
    }

    Inven_Item item;
    const int slot = InsertShopItem(user,
                                    prototype,
                                    static_cast<int>(item_id),
                                    count,
                                    item);
    if (slot < 0) {
        SendCommandError(user,
                         ENUM_CMDPACKET_SECRET_EVENT_SHOP_BUY_ITEM,
                         4u);
        return 0;
    }
    CInventory* const inventory = user->getCurCharacInvenW();
    inventory->use_money(static_cast<int>(required_money),
                         static_cast<eMoneySubReason>(39),
                         true);
    shop->shopper.BuyItem(static_cast<int>(item_id), count);
    SendEventShopSuccess(user, slot, item, remaining);
    return 0;
}

int Dispatcher_SecretEventShopBuyItem::read(PacketBuf& packet,
                                                 MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SecretEventShopBuyItem::process(CUser* user,
                                                    MSG_BASE& message,
                                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_EventAttendanceReward::
    Dispatcher_EventAttendanceReward()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_EventAttendanceReward::~
    Dispatcher_EventAttendanceReward() {}

int Dispatcher_EventAttendanceReward::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    uint8_t opaque_request[15] = {};
    if (!user ||
        packet.get_binary(reinterpret_cast<char*>(opaque_request),
                          sizeof(opaque_request)) != 1) {
        SendAttendanceError(
            user,
            event_attendance_db::kResultInvalidEligibility);
        return 0;
    }
    // CLIENT_IDA_EVIDENCE: sender sub_4FDCF0 raw-copies a legacy 15-byte
    // MSG_BASE-sized stack block.  Bytes 0..12 are opaque/uninitialized;
    // only byte 13 (matured stage) and byte 14 (requested 1-based tier) are
    // defined.  Never validate or persist the opaque prefix.
    const uint8_t client_stage = opaque_request[13];
    const uint8_t requested_tier = opaque_request[14];
    AttendanceState* const state = FindAttendanceState(user);
    if (!IsRepeatEventActive(kAttendanceEventId) ||
        !state || !state->loaded || state->db_pending ||
        state->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo()) ||
        client_stage > event_attendance_db::kDailyRewardCount ||
        requested_tier == 0u ||
        requested_tier > event_attendance_db::kDailyRewardCount ||
        requested_tier > client_stage) {
        SendAttendanceError(
            user,
            event_attendance_db::kResultInvalidEligibility);
        return 0;
    }
    const uint32_t now = CurrentEpoch();
    if (requested_tier >
        MaturedDailyCount(EffectiveDailyElapsed(*state, now))) {
        SendAttendanceError(
            user,
            event_attendance_db::kResultInvalidEligibility);
        return 0;
    }
    if (!QueueAttendanceOperation(
            user,
            *state,
            event_attendance_db::kOperationClaim,
            requested_tier,
            client_stage)) {
        SendAttendanceError(
            user,
            event_attendance_db::kResultOperationFailed);
    }
    return 0;
}

int Dispatcher_EventAttendanceReward::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_EventAttendanceReward::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_EventAttendance::Inter_EventAttendance()
    : ISigDispatcher() {}

Inter_EventAttendance::~Inter_EventAttendance() {}

int Inter_EventAttendance::dispatch_sig(CUser* user,
                                             char* payload,
                                             int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(
                            sizeof(event_attendance_db::Message))) {
        return 0;
    }
    const event_attendance_db::Message& message =
        *reinterpret_cast<const event_attendance_db::Message*>(payload);
    if (user->get_acc_id() <= 0 ||
        message.account_id !=
            static_cast<uint32_t>(user->get_acc_id()) ||
        !IsCurrentAttendanceResponse(user, message)) {
        return 0;
    }
    AttendanceState* const state = FindAttendanceState(user);
    state->db_pending = false;
    if (message.result != event_attendance_db::kResultOk) {
        if (message.operation == event_attendance_db::kOperationLoad) {
            // Do not retain an unloaded tombstone: world-ready or the global
            // minute timer can install a fresh token and retry cleanly.
            AttendanceStates().erase(
                static_cast<uint32_t>(user->get_acc_id()));
            return 0;
        }
        if (message.operation == event_attendance_db::kOperationClaim) {
            SendAttendanceError(user, message.result);
        }
        return 0;
    }

    ApplyAttendanceResponse(*state, message);
    if (state->closing_epoch != 0u) {
        const uint32_t closing_epoch = state->closing_epoch;
        if (SealAttendanceSession(user, *state, closing_epoch)) {
            AttendanceStates().erase(
                static_cast<uint32_t>(user->get_acc_id()));
            if (IsRepeatEventActive(kAttendanceEventId)) {
                event_attendance::RequestLoginState(user);
            }
        }
        return 0;
    }
    if (message.operation == event_attendance_db::kOperationLoad ||
        message.operation ==
            event_attendance_db::kOperationCheckpoint) {
        SendAttendanceState(user, *state);
        if (message.new_mail) {
            NotifyNewMail(user);
        }
        return 0;
    }
    if (message.operation != event_attendance_db::kOperationClaim) {
        return 0;
    }
    SendAttendanceSuccess(user);
    SendAttendanceState(user, *state);
    if (message.new_mail) {
        NotifyNewMail(user);
    }
    return 0;
}
