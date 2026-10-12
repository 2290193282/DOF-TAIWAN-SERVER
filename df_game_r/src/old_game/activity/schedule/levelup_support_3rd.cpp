#include "old_game/activity/schedule/levelup_support_3rd.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_levelup_support_3rd.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace {

const char kPvfFile[] = "tw_levelupsupport3rd.evt";
const uint8_t kSupportLevels[] = {
    5, 10, 15, 20, 25, 28, 30, 35, 40, 45,
    48, 50, 53, 55, 58, 60, 63, 65, 68, 70,
    72, 74, 76, 78, 80, 81, 82, 83, 84, 85,
};

struct Reward {
    Reward() : item_id(0), count(0) {}
    Reward(int item_id_value, int count_value)
        : item_id(item_id_value), count(count_value) {}

    int item_id;
    int count;
};

struct Group {
    Group() : minimum_level(0), maximum_level(0), support_levels() {}

    int minimum_level;
    int maximum_level;
    std::vector<int> support_levels;
};

typedef std::map<int, Group> GroupMap;
typedef std::map<int, std::vector<Reward> > RewardMap;

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

bool ParseGroupRanges(GroupMap& groups) {
    bool is_type = false;
    bool success = false;
    std::string token;
    for (;;) {
        const int group_index =
            ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == "[/group min max level]";
        }
        if (!success) {
            return false;
        }
        const int minimum_level = ScanInt(&success);
        if (!success) {
            return false;
        }
        const int maximum_level = ScanInt(&success);
        if (!success || group_index < 0 || minimum_level <= 0 ||
            maximum_level < minimum_level ||
            groups.find(group_index) != groups.end()) {
            return false;
        }
        Group group;
        group.minimum_level = minimum_level;
        group.maximum_level = maximum_level;
        groups.insert(std::make_pair(group_index, group));
    }
}

bool ParseGroupSupportLevels(GroupMap& groups) {
    bool is_type = false;
    bool success = false;
    std::string token;
    std::map<int, int> owners;
    for (GroupMap::const_iterator group = groups.begin();
         group != groups.end(); ++group) {
        for (std::vector<int>::const_iterator level =
                 group->second.support_levels.begin();
             level != group->second.support_levels.end(); ++level) {
            owners[*level] = group->first;
        }
    }
    for (;;) {
        const int group_index =
            ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == "[/group support level]";
        }
        if (!success) {
            return false;
        }
        const int level = ScanInt(&success);
        GroupMap::iterator group = groups.find(group_index);
        if (!success || group == groups.end() ||
            level < group->second.minimum_level ||
            level > group->second.maximum_level ||
            owners.find(level) != owners.end()) {
            return false;
        }
        group->second.support_levels.push_back(level);
        owners[level] = group_index;
    }
}

bool ParseRewards(RewardMap& rewards) {
    bool is_type = false;
    bool success = false;
    std::string token;
    for (;;) {
        const int level = ScanTypeOrInt(is_type, token, success, true);
        if (is_type) {
            return token == "[/level up support item]";
        }
        if (!success) {
            return false;
        }
        const int item_id = ScanInt(&success);
        if (!success) {
            return false;
        }
        const int count = ScanInt(&success);
        if (!success || level <= 0 || item_id <= 0 || count <= 0 ||
            rewards[level].size() >=
                levelup_support_3rd_db::kMaximumRewardCount) {
            return false;
        }
        rewards[level].push_back(Reward(item_id, count));
    }
}

bool ValidateConfig(GroupMap& groups, const RewardMap& rewards) {
    static const int kExpectedMinimums[] = {1, 26, 46, 59, 71, 81};
    static const int kExpectedMaximums[] = {25, 45, 58, 70, 80, 85};
    if (groups.size() != 6u ||
        rewards.size() !=
            sizeof(kSupportLevels) / sizeof(kSupportLevels[0])) {
        return false;
    }

    std::vector<int> all_support_levels;
    for (int group_index = 0; group_index < 6; ++group_index) {
        GroupMap::iterator group = groups.find(group_index);
        if (group == groups.end() ||
            group->second.minimum_level != kExpectedMinimums[group_index] ||
            group->second.maximum_level != kExpectedMaximums[group_index] ||
            group->second.support_levels.size() != 5u) {
            return false;
        }
        std::sort(group->second.support_levels.begin(),
                  group->second.support_levels.end());
        all_support_levels.insert(all_support_levels.end(),
                                  group->second.support_levels.begin(),
                                  group->second.support_levels.end());
    }
    if (all_support_levels.size() !=
        sizeof(kSupportLevels) / sizeof(kSupportLevels[0])) {
        return false;
    }
    for (unsigned int index = 0; index < all_support_levels.size(); ++index) {
        const int level = all_support_levels[index];
        RewardMap::const_iterator reward = rewards.find(level);
        if (level != kSupportLevels[index] || reward == rewards.end() ||
            reward->second.empty() ||
            reward->second.size() >
                levelup_support_3rd_db::kMaximumRewardCount) {
            return false;
        }
    }
    return true;
}

class LevelupSupport3rdConfig {
public:
    LevelupSupport3rdConfig()
        : loaded_(false), groups_(), rewards_(),
          mail_title_key_(), mail_content_key_() {}

    bool available() { return loaded_ || Load(); }

    const std::vector<Reward>* Rewards(int level) {
        if (!available()) {
            return 0;
        }
        RewardMap::const_iterator found = rewards_.find(level);
        return found == rewards_.end() ? 0 : &found->second;
    }

    const std::string& mail_title_key() const { return mail_title_key_; }
    const std::string& mail_content_key() const { return mail_content_key_; }

private:
    bool Load() {
        if (!loadRDARScriptFile("Event/", kPvfFile)) {
            return false;
        }
        RdarUnloadGuard unload;

        GroupMap groups;
        RewardMap rewards;
        std::string mail_title_key;
        std::string mail_content_key;
        std::string token;
        bool valid = true;
        while (valid && ScanType(token, true) == 1) {
            if (token == "[group min max level]") {
                valid = ParseGroupRanges(groups);
            } else if (token == "[group support level]") {
                valid = ParseGroupSupportLevels(groups);
            } else if (token == "[level up support item]") {
                valid = ParseRewards(rewards);
            } else if (token == "[mail title]") {
                ScanStr(&mail_title_key);
            } else if (token == "[mail content]") {
                ScanStr(&mail_content_key);
            }
        }
        if (!valid || mail_title_key.empty() || mail_content_key.empty() ||
            !ValidateConfig(groups, rewards)) {
            return false;
        }

        groups_.swap(groups);
        rewards_.swap(rewards);
        mail_title_key_.swap(mail_title_key);
        mail_content_key_.swap(mail_content_key);
        loaded_ = true;
        return true;
    }

    bool loaded_;
    GroupMap groups_;
    RewardMap rewards_;
    std::string mail_title_key_;
    std::string mail_content_key_;
};

LevelupSupport3rdConfig& Config() {
    static LevelupSupport3rdConfig config;
    return config;
}

uint32_t ExpectedNextLevel(uint32_t last_claimed_level) {
    if (last_claimed_level == 0u) {
        return kSupportLevels[0];
    }
    for (unsigned int index = 0;
         index + 1u < sizeof(kSupportLevels) / sizeof(kSupportLevels[0]);
         ++index) {
        if (last_claimed_level == kSupportLevels[index]) {
            return kSupportLevels[index + 1u];
        }
    }
    return 0u;
}

bool IsSupportLevel(uint32_t level) {
    for (unsigned int index = 0;
         index < sizeof(kSupportLevels) / sizeof(kSupportLevels[0]);
         ++index) {
        if (level == kSupportLevels[index]) {
            return true;
        }
    }
    return false;
}

struct EventState {
    EventState()
        : charac_no(0u), last_claimed_level(0u), claim_revision(0u),
          session_token(0u), loaded(false), db_pending(false) {}

    uint32_t charac_no;
    uint32_t last_claimed_level;
    uint32_t claim_revision;
    uint32_t session_token;
    bool loaded;
    bool db_pending;
};

typedef std::map<uint32_t, EventState> StateMap;

StateMap& States() {
    static StateMap states;
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

EventState* FindState(CUser* user) {
    if (!user || user->get_acc_id() <= 0) {
        return 0;
    }
    StateMap::iterator found =
        States().find(static_cast<uint32_t>(user->get_acc_id()));
    return found == States().end() ? 0 : &found->second;
}

bool IsCurrentResponse(
    CUser* user,
    const levelup_support_3rd_db::Message& message) {
    EventState* const state = FindState(user);
    return state && message.session_token != 0u &&
           state->session_token == message.session_token &&
           state->charac_no == message.charac_no &&
           message.charac_no ==
               static_cast<uint32_t>(user->getCurCharacNo()) &&
           message.server_group == user->GetServerGroup();
}

void InitIdentity(levelup_support_3rd_db::Message& message,
                  CUser* user,
                  uint8_t operation) {
    levelup_support_3rd_db::ClearMessage(message);
    EventState* const state = FindState(user);
    message.account_id = user && user->get_acc_id() > 0
                             ? static_cast<uint32_t>(user->get_acc_id())
                             : 0u;
    message.charac_no = user && user->getCurCharacNo() > 0
                            ? static_cast<uint32_t>(user->getCurCharacNo())
                            : 0u;
    message.session_token = state ? state->session_token : 0u;
    message.server_group = user ? user->GetServerGroup() : -1;
    message.operation = operation;
    const int character_level = user ? user->get_charac_level() : 0;
    message.character_level = static_cast<uint8_t>(
        character_level > 255 ? 255 : (character_level < 0 ? 0
                                                            : character_level));
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

bool BuildClaimMessage(
    CUser* user,
    uint8_t requested_level,
    levelup_support_3rd_db::Message& message) {
    const std::vector<Reward>* const rewards =
        Config().Rewards(requested_level);
    if (!user || !rewards || rewards->empty() ||
        rewards->size() >
            levelup_support_3rd_db::kMaximumRewardCount ||
        !G_CDataManager()) {
        return false;
    }

    InitIdentity(message,
                 user,
                 levelup_support_3rd_db::kOperationClaim);
    message.requested_level = requested_level;
    message.reward_count = static_cast<uint8_t>(rewards->size());
    for (unsigned int index = 0; index < rewards->size(); ++index) {
        const Reward& reward = (*rewards)[index];
        CItem* const prototype = G_CDataManager()->find_item(reward.item_id);
        if (!prototype || !prototype->vtable_ || reward.count <= 0) {
            return false;
        }
        Inven_Item item;
        prototype->vtable_->make_item(prototype, item);
        item.item_id_ = prototype->get_index();
        item.set_add_info(reward.count);
        InvenItemMailRecord mail_record;
        std::memset(&mail_record, 0, sizeof(mail_record));
        item.ExportMailRecord(mail_record);

        levelup_support_3rd_db::MailReward& destination =
            message.rewards[index];
        destination.item_id = mail_record.item_id;
        destination.add_info = mail_record.add_info;
        destination.extend_info = mail_record.extend_info;
        destination.chronicle_value = mail_record.chronicle_value;
        destination.endurance = mail_record.endurance;
        destination.amplify_value = mail_record.amplify_value;
        destination.seal_flag = mail_record.seal_flag;
        destination.item_attr = mail_record.item_attr;
        destination.amplify_option = mail_record.amplify_option;
        destination.separate_upgrade = mail_record.separate_upgrade;
        destination.avatar_flag = mail_record.avatar_flag ? 1u : 0u;
        destination.creature_flag = mail_record.creature_flag ? 1u : 0u;
        std::memcpy(destination.item_guid,
                    mail_record.item_guid,
                    sizeof(destination.item_guid));
        std::memcpy(destination.random_option,
                    mail_record.random_option,
                    sizeof(destination.random_option));
    }

    const char* title = g_scriptStringManager_.findString(
        4, Config().mail_title_key().c_str(), 0);
    const char* content = g_scriptStringManager_.findString(
        4, Config().mail_content_key().c_str(), 0);
    CopyText(message.mail_title,
             sizeof(message.mail_title),
             title ? title : Config().mail_title_key().c_str());
    CopyText(message.mail_content,
             sizeof(message.mail_content),
             content ? content : Config().mail_content_key().c_str());
    return message.mail_title[0] != 0;
}

void SendState(CUser* user, const EventState& state) {
    if (!user || state.last_claimed_level > 255u) {
        return;
    }
    int character_level = user->get_charac_level();
    if (character_level < 0) {
        character_level = 0;
    } else if (character_level > 255) {
        character_level = 255;
    }
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: NOTI660 handler sub_691B20 reads exactly
    // u8 last-claimed watermark followed by u8 current character level.
    packet.put_header(0, ENUM_NOTIPACKET_LEVELUP_SUPPORT_3RD_EVENT_INFO);
    packet.put_byte(static_cast<int>(state.last_claimed_level));
    packet.put_byte(character_level);
    packet.finalize(true);
    user->Send(packet);
}

void SendError(CUser* user, uint8_t result) {
    if (!user) {
        return;
    }
    const uint8_t error =
        result == levelup_support_3rd_db::kResultInvalidEligibility ||
                result == levelup_support_3rd_db::kResultRewardUnavailable
            ? result
            : 1u;
    user->SendCmdErrorPacket(
        ENUM_CMDPACKET_LEVELUP_SUPPORT_3RD_EVENT_GET_ITEM,
        error);
}

void SendSuccess(
    CUser* user,
    const levelup_support_3rd_db::Message& message) {
    PacketGuard packet;
    // CLIENT_IDA_EVIDENCE: CMD712 response handler sub_691580 consumes
    // framework-success u8, u32 reward count, then count * {u32 id,u32 qty}.
    packet.put_header(1,
                      ENUM_CMDPACKET_LEVELUP_SUPPORT_3RD_EVENT_GET_ITEM);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(message.reward_count));
    for (unsigned int index = 0; index < message.reward_count; ++index) {
        packet.put_int(static_cast<int>(message.rewards[index].item_id));
        packet.put_int(static_cast<int>(message.rewards[index].add_info));
    }
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

}  // namespace

namespace levelup_support_3rd {

void RequestLoginState(CUser* user) {
    if (!user || user->get_acc_id() <= 0 ||
        user->getCurCharacNo() <= 0 || !Config().available()) {
        return;
    }
    EventState state;
    state.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    state.session_token = NextSessionToken();
    States()[static_cast<uint32_t>(user->get_acc_id())] = state;

    levelup_support_3rd_db::Message request;
    InitIdentity(request,
                 user,
                 levelup_support_3rd_db::kOperationLoad);
    EventState* const installed = FindState(user);
    if (installed && levelup_support_3rd_db::Queue(user->GetUID(),
                                                        request)) {
        installed->db_pending = true;
    }
}

void ResetSession(CUser* user) {
    if (user && user->get_acc_id() > 0) {
        States().erase(static_cast<uint32_t>(user->get_acc_id()));
    }
}

void OnLevelUp(CUser* user) {
    EventState* const state = FindState(user);
    if (state && state->loaded &&
        state->charac_no ==
            static_cast<uint32_t>(user->getCurCharacNo()) &&
        Config().available()) {
        SendState(user, *state);
    }
}

}  // namespace levelup_support_3rd_0828

Dispatcher_LevelupSupport3rdEventGetItem::
    Dispatcher_LevelupSupport3rdEventGetItem()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(2)>() {}

Dispatcher_LevelupSupport3rdEventGetItem::~
    Dispatcher_LevelupSupport3rdEventGetItem() {}

int Dispatcher_LevelupSupport3rdEventGetItem::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    uint8_t requested_level = 0u;
    if (!user || packet.get_byte(requested_level) != 1) {
        SendError(user,
                  levelup_support_3rd_db::kResultInvalidEligibility);
        return 0;
    }
    EventState* const state = FindState(user);
    const uint32_t requested = requested_level;
    if (!Config().available()) {
        SendError(user,
                  levelup_support_3rd_db::kResultRewardUnavailable);
        return 0;
    }
    if (!state || !state->loaded || state->db_pending ||
        state->charac_no !=
            static_cast<uint32_t>(user->getCurCharacNo()) ||
        !IsSupportLevel(requested) ||
        user->get_charac_level() < static_cast<int>(requested) ||
        (requested != state->last_claimed_level &&
         requested != ExpectedNextLevel(state->last_claimed_level))) {
        SendError(user,
                  levelup_support_3rd_db::kResultInvalidEligibility);
        return 0;
    }

    levelup_support_3rd_db::Message request;
    if (!BuildClaimMessage(user, requested_level, request)) {
        SendError(user,
                  levelup_support_3rd_db::kResultRewardUnavailable);
        return 0;
    }
    if (!levelup_support_3rd_db::Queue(user->GetUID(), request)) {
        SendError(user,
                  levelup_support_3rd_db::kResultOperationFailed);
    } else {
        state->db_pending = true;
    }
    return 0;
}

int Dispatcher_LevelupSupport3rdEventGetItem::read(PacketBuf& packet,
                                                    MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_LevelupSupport3rdEventGetItem::process(CUser* user,
                                                       MSG_BASE& message,
                                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_LevelupSupport3rd::Inter_LevelupSupport3rd()
    : ISigDispatcher() {}

Inter_LevelupSupport3rd::~Inter_LevelupSupport3rd() {}

int Inter_LevelupSupport3rd::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(
                            sizeof(levelup_support_3rd_db::Message))) {
        return 0;
    }
    const levelup_support_3rd_db::Message& message =
        *reinterpret_cast<const levelup_support_3rd_db::Message*>(
            payload);
    if (user->get_acc_id() <= 0 ||
        message.account_id !=
            static_cast<uint32_t>(user->get_acc_id()) ||
        !IsCurrentResponse(user, message)) {
        return 0;
    }

    EventState* const state = FindState(user);
    state->db_pending = false;
    if (message.operation == levelup_support_3rd_db::kOperationLoad) {
        if (message.result == levelup_support_3rd_db::kResultOk &&
            message.last_claimed_level <= 255u &&
            (message.last_claimed_level == 0u ||
             IsSupportLevel(message.last_claimed_level))) {
            state->last_claimed_level = message.last_claimed_level;
            state->claim_revision = message.claim_revision;
            state->loaded = true;
            SendState(user, *state);
        }
        return 0;
    }
    if (message.operation != levelup_support_3rd_db::kOperationClaim) {
        return 0;
    }
    if (message.result != levelup_support_3rd_db::kResultOk) {
        SendError(user, message.result);
        return 0;
    }

    state->last_claimed_level = message.last_claimed_level;
    state->claim_revision = message.claim_revision;
    state->loaded = true;
    SendSuccess(user, message);
    SendState(user, *state);
    if (!message.replayed) {
        NotifyNewMail(user);
    }
    return 0;
}
