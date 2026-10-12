#include "old_game/activity/schedule/packet_dispatcher_facebook_like.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_facebook_like.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace {

const char kFacebookLikePvf[] = "tw_facebooklike.evt";
const char kFirstShowTime[] = "2013-11-28 09:00:00";

enum CommandError {
    ERROR_TOWN_ONLY = 1,
    ERROR_DATABASE = 3,
    ERROR_MAIL = 17,
    ERROR_PENDING = 19,
    ERROR_STATE = 21,
    ERROR_EVENT_CLOSED = 178,
};

struct RewardConfig {
    RewardConfig()
        : row(0), npc_index(0), item_id(0), count(0), text_key() {}

    int row;
    int npc_index;
    int item_id;
    int count;
    std::string text_key;
};

struct FacebookLikeConfig {
    FacebookLikeConfig()
        : first_show_time(),
          fatigue_condition(0),
          countdown_seconds(0),
          rewards(),
          weekday_rows(),
          mail_title_key(),
          mail_content_key() {
        std::memset(weekday_rows, 0, sizeof(weekday_rows));
    }

    std::string first_show_time;
    int fatigue_condition;
    int countdown_seconds;
    std::vector<RewardConfig> rewards;
    int weekday_rows[7][7];
    std::string mail_title_key;
    std::string mail_content_key;

    const RewardConfig* findReward(int row) const {
        if (row <= 0 || row > static_cast<int>(rewards.size())) {
            return 0;
        }
        const RewardConfig& reward = rewards[static_cast<size_t>(row - 1)];
        return reward.row == row ? &reward : 0;
    }

    const RewardConfig* resolve(uint8_t weekday, uint8_t index) const {
        return weekday < 7u && index < 7u
                   ? findReward(weekday_rows[weekday][index])
                   : 0;
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

class FacebookLikeConfigCache {
public:
    FacebookLikeConfigCache() : attempted_(false), valid_(false), config_() {}

    const FacebookLikeConfig* get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile("Event/", kFacebookLikePvf)) {
            return false;
        }
        RdarUnloadGuard unload;

        FacebookLikeConfig parsed;
        bool have_first_show = false;
        bool have_fatigue = false;
        bool have_countdown = false;
        bool have_rewards = false;
        bool have_weekdays = false;
        bool have_title = false;
        bool have_content = false;
        bool malformed = false;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag == "[check time for first show ui]") {
                if (have_first_show || ScanStr(&parsed.first_show_time) != 1) {
                    malformed = true;
                }
                have_first_show = true;
            } else if (tag == "[condition fatigue]") {
                bool success = false;
                const int value = ScanInt(&success);
                if (have_fatigue || !success || value <= 0) {
                    malformed = true;
                } else {
                    parsed.fatigue_condition = value;
                }
                have_fatigue = true;
            } else if (tag == "[condition time]") {
                bool success = false;
                const int value = ScanInt(&success);
                if (have_countdown || !success || value <= 0) {
                    malformed = true;
                } else {
                    parsed.countdown_seconds = value;
                }
                have_countdown = true;
            } else if (tag == "[npc reward data]") {
                if (have_rewards) {
                    malformed = true;
                }
                have_rewards = true;
                for (;;) {
                    bool success = false;
                    RewardConfig reward;
                    reward.row = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    reward.npc_index = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    reward.item_id = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    reward.count = ScanInt(&success);
                    if (!success || ScanStr(&reward.text_key) != 1) {
                        malformed = true;
                        break;
                    }
                    parsed.rewards.push_back(reward);
                }
            } else if (tag == "[facebook like wday info]") {
                if (have_weekdays) {
                    malformed = true;
                }
                have_weekdays = true;
                int weekday_counts[7] = {};
                for (;;) {
                    bool success = false;
                    const int weekday = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    const int row = ScanInt(&success);
                    if (!success || weekday < 0 || weekday > 6 || row < 1 ||
                        row > 49 || weekday_counts[weekday] >= 7) {
                        malformed = true;
                        break;
                    }
                    parsed.weekday_rows[weekday][weekday_counts[weekday]++] =
                        row;
                }
                for (int weekday = 0; weekday < 7; ++weekday) {
                    if (weekday_counts[weekday] != 7) {
                        malformed = true;
                    }
                }
            } else if (tag == "[mail title]") {
                if (have_title || ScanStr(&parsed.mail_title_key) != 1) {
                    malformed = true;
                }
                have_title = true;
            } else if (tag == "[mail content]") {
                if (have_content || ScanStr(&parsed.mail_content_key) != 1) {
                    malformed = true;
                }
                have_content = true;
            }
        }

        if (malformed || !have_first_show || !have_fatigue ||
            !have_countdown || !have_rewards || !have_weekdays ||
            !have_title || !have_content ||
            parsed.first_show_time != kFirstShowTime ||
            parsed.fatigue_condition !=
                facebook_like_db::kFatigueCondition ||
            parsed.countdown_seconds !=
                facebook_like_db::kCountdownSeconds ||
            parsed.rewards.size() != 49u || parsed.mail_title_key.empty() ||
            parsed.mail_content_key.empty()) {
            return false;
        }

        CDataManager* const manager = G_CDataManager();
        if (!manager) {
            return false;
        }
        for (size_t index = 0; index < parsed.rewards.size(); ++index) {
            const RewardConfig& reward = parsed.rewards[index];
            const int expected_row = static_cast<int>(index + 1u);
            if (reward.row != expected_row || reward.npc_index <= 0 ||
                reward.item_id != 690000944 + expected_row ||
                reward.count != 1 || reward.text_key.empty()) {
                return false;
            }
            CItem* const item = manager->find_item(reward.item_id);
            if (!item || !item->vtable_) {
                return false;
            }
        }
        for (int weekday = 0; weekday < 7; ++weekday) {
            for (int index = 0; index < 7; ++index) {
                const int expected_row =
                    ((weekday + 3) % 7) * 7 + index + 1;
                if (parsed.weekday_rows[weekday][index] != expected_row) {
                    return false;
                }
            }
        }

        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    FacebookLikeConfig config_;
};

struct SessionState {
    SessionState()
        : charac_no(0u),
          token(0u),
          cycle_key(0u),
          countdown_deadline(0u),
          selected_reward_row(0u),
          selected_reward_item_id(0u),
          claim_token(0u),
          server_group(-1),
          stage(facebook_like_db::STAGE_WAIT_FATIGUE),
          weekday(0u),
          mail_state(facebook_like_db::MAIL_STATE_NONE),
          pending_operation(0u),
          loaded(false),
          mail_waiting(false) {}

    uint32_t charac_no;
    uint32_t token;
    uint32_t cycle_key;
    uint32_t countdown_deadline;
    uint32_t selected_reward_row;
    uint32_t selected_reward_item_id;
    uint32_t claim_token;
    int server_group;
    uint8_t stage;
    uint8_t weekday;
    uint8_t mail_state;
    uint8_t pending_operation;
    bool loaded;
    bool mail_waiting;
};

struct PendingMail {
    PendingMail()
        : user(0),
          session_token(0u),
          charac_no(0u),
          reward_row(0u),
          reward_item_id(0u),
          server_group(-1) {}

    CUser* user;
    uint32_t session_token;
    uint32_t charac_no;
    uint32_t reward_row;
    uint32_t reward_item_id;
    int server_group;
};

Mutex g_facebook_like_mutex;
std::map<CUser*, SessionState> g_facebook_like_sessions;
std::map<uint64_t, PendingMail> g_facebook_like_pending_mail;
uint32_t g_next_session_token = 1u;

FacebookLikeConfigCache& ConfigCache() {
    static FacebookLikeConfigCache cache;
    return cache;
}

uint32_t NextNonZeroTokenLocked(uint32_t& source) {
    uint32_t token = source++;
    if (token == 0u) {
        token = source++;
    }
    if (source == 0u) {
        ++source;
    }
    return token;
}

uint64_t PendingMailKey(uint32_t charac_no, uint32_t claim_token) {
    return (static_cast<uint64_t>(charac_no) << 32u) |
           static_cast<uint64_t>(claim_token);
}

bool PendingMailMatches(const PendingMail& pending,
                        CUser* expected_user,
                        uint32_t session_token,
                        uint32_t charac_no,
                        uint32_t reward_item_id,
                        int server_group) {
    return session_token != 0u && pending.user == expected_user &&
           pending.session_token == session_token &&
           pending.charac_no == charac_no &&
           pending.reward_item_id == reward_item_id &&
           pending.server_group == server_group;
}

void ClearPendingMailLocked(uint32_t charac_no,
                            uint32_t claim_token,
                            uint32_t reward_item_id,
                            int server_group,
                            uint32_t session_token) {
    const uint64_t key = PendingMailKey(charac_no, claim_token);
    std::map<uint64_t, PendingMail>::iterator pending =
        g_facebook_like_pending_mail.find(key);
    if (pending == g_facebook_like_pending_mail.end() ||
        !PendingMailMatches(pending->second,
                            pending->second.user,
                            session_token,
                            charac_no,
                            reward_item_id,
                            server_group)) {
        return;
    }

    CUser* const owner = pending->second.user;
    g_facebook_like_pending_mail.erase(pending);
    std::map<CUser*, SessionState>::iterator session =
        g_facebook_like_sessions.find(owner);
    if (session != g_facebook_like_sessions.end() &&
        session->second.token == session_token &&
        session->second.charac_no == charac_no &&
        session->second.server_group == server_group) {
        session->second.mail_waiting = false;
    }
}

uint32_t CurrentCharacterNo(CUser* user) {
    const int value = user && user->getCurCharacR()
                          ? user->getCurCharacNo()
                          : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

bool IsCharacterReady(CUser* user) {
    return user && user->get_state() > 2 && CurrentCharacterNo(user) != 0u &&
           user->GetServerGroup() >= 0 &&
           user->GetServerGroup() <
               facebook_like_db::kMaximumServerGroupCount;
}

bool IsTownReady(CUser* user) {
    return IsCharacterReady(user) && user->get_state() == 3;
}

void TraceFailure(const char* reason,
                  uint32_t charac_no,
                  uint32_t claim_token) {
    cMyTrace trace("CMD725 FacebookLike0828", __LINE__, 5);
    trace("[0828 FacebookLike] %s charac(%u) claim(%u)",
          reason ? reason : "unknown",
          static_cast<unsigned int>(charac_no),
          static_cast<unsigned int>(claim_token));
}

void SendCommandResult(CUser* user,
                       bool success,
                       uint8_t action,
                       uint8_t error) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_FACEBOOK_LIKE);
    packet.put_byte(success ? 1 : 0);
    if (!success) {
        packet.put_byte(error);
    }
    // CLIENT_IDA_EVIDENCE: sub_68F120 always consumes this action byte after
    // the framework prefix on both success and failure paths.
    packet.put_byte(action);
    packet.finalize(true);
    user->Send(packet);
}

void SendServerTime(CUser* user, uint32_t now_epoch) {
    if (!user || now_epoch == 0u) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SERVER_TIME);
    packet.put_int(static_cast<int>(now_epoch));
    packet.finalize(true);
    user->Send(packet);
}

void SendState(CUser* user,
               const facebook_like_db::StateMessage& state) {
    if (!user || state.stage > facebook_like_db::STAGE_REWARD_MAILED ||
        state.weekday > 6u) {
        return;
    }
    uint32_t state_param = 0u;
    uint32_t reward_param = 0u;
    if (state.stage == facebook_like_db::STAGE_COUNTDOWN) {
        state_param = state.countdown_deadline > state.now_epoch
                          ? state.countdown_deadline - state.now_epoch
                          : 0u;
    } else if (state.stage >=
               facebook_like_db::STAGE_REWARD_SELECTED) {
        state_param = state.selected_reward_row;
        reward_param = state.selected_reward_item_id;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_FACEBOOK_LIKE_CHARAC_DATA);
    // CLIENT_IDA_EVIDENCE: sub_6902B0 reads exactly
    // {u8 show,u8 stage,u32 state_param,u32 reward_item,u8 weekday}.
    packet.put_byte(state.show_ui);
    packet.put_byte(state.stage);
    packet.put_int(static_cast<int>(state_param));
    packet.put_int(static_cast<int>(reward_param));
    packet.put_byte(state.weekday);
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapDatabaseError(uint8_t result) {
    if (result == facebook_like_db::RESULT_PENDING_RECENT) {
        return ERROR_PENDING;
    }
    if (result == facebook_like_db::RESULT_INVALID_STATE) {
        return ERROR_STATE;
    }
    return ERROR_DATABASE;
}

bool QueueOperation(CUser* user,
                    uint8_t operation,
                    uint8_t action,
                    uint8_t selection_index,
                    uint32_t reward_row,
                    uint32_t reward_item_id,
                    uint32_t claim_token,
                    uint8_t mail_result) {
    facebook_like_db::StateMessage request;
    std::memset(&request, 0, sizeof(request));
    request.reply_user_idx = -1;
    {
        MutexGuard guard(&g_facebook_like_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_facebook_like_sessions.find(user);
        if (found == g_facebook_like_sessions.end() ||
            found->second.charac_no != CurrentCharacterNo(user) ||
            found->second.server_group != user->GetServerGroup() ||
            found->second.pending_operation != 0u) {
            return false;
        }
        found->second.pending_operation = operation;
        request.charac_no = found->second.charac_no;
        request.session_token = found->second.token;
        request.server_group = found->second.server_group;
    }
    request.used_fatigue = static_cast<uint16_t>(
        user->getCurCharacUsedFatigue());
    request.operation = operation;
    request.action = action;
    request.selection_index = selection_index;
    request.selected_reward_row = reward_row;
    request.selected_reward_item_id = reward_item_id;
    request.claim_token = claim_token;
    request.mail_result = mail_result;

    if (facebook_like_db::DB_FacebookLike::queueRequest(
            user->GetUID(), request)) {
        return true;
    }
    MutexGuard guard(&g_facebook_like_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_facebook_like_sessions.find(user);
    if (found != g_facebook_like_sessions.end() &&
        found->second.token == request.session_token &&
        found->second.pending_operation == operation) {
        found->second.pending_operation = 0u;
    }
    return false;
}

bool SubmitRewardMail(CUser* user,
                      const FacebookLikeConfig& config,
                      const facebook_like_db::StateMessage& state,
                      SIG_MAILBOX_PERSISTENCE_CONTEXT& persistence) {
    const RewardConfig* const reward =
        config.findReward(static_cast<int>(state.selected_reward_row));
    CDataManager* const manager = G_CDataManager();
    CItem* const prototype =
        reward && manager ? manager->find_item(reward->item_id) : 0;
    if (!user || !reward || !prototype || !prototype->vtable_ ||
        state.claim_token == 0u ||
        state.selected_reward_item_id !=
            static_cast<uint32_t>(reward->item_id)) {
        return false;
    }

    const char* const title = g_scriptStringManager_.findString(
        4, config.mail_title_key.c_str(), 0);
    const char* const content = g_scriptStringManager_.findString(
        4, config.mail_content_key.c_str(), 0);
    const char* const safe_title = title ? title : config.mail_title_key.c_str();
    const char* const safe_content =
        content ? content : config.mail_content_key.c_str();
    if (!safe_title[0] || !safe_content[0]) {
        return false;
    }

    Inven_Item item;
    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(reward->count);

    std::memset(&persistence, 0, sizeof(persistence));
    persistence.magic = MAILBOX_PERSISTENCE_CONTEXT_MAGIC;
    persistence.version = MAILBOX_PERSISTENCE_CONTEXT_VERSION;
    persistence.consumer =
        MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE;
    persistence.claim_token = state.claim_token;
    persistence.receiver_charac_no = state.charac_no;
    persistence.reward_item_id = state.selected_reward_item_id;
    persistence.server_group = state.server_group;

    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMailTracked(
               safe_title,
               item,
               0u,
               state.charac_no,
               safe_content,
               static_cast<int>(std::strlen(safe_content)),
               14u,
               static_cast<ENUM_SERVER_GROUP>(state.server_group),
               false,
               false,
               persistence) == 0;
}

bool SnapshotSession(CUser* user, SessionState& state) {
    MutexGuard guard(&g_facebook_like_mutex);
    const std::map<CUser*, SessionState>::const_iterator found =
        g_facebook_like_sessions.find(user);
    if (found == g_facebook_like_sessions.end() || !found->second.loaded ||
        found->second.charac_no != CurrentCharacterNo(user) ||
        found->second.server_group != user->GetServerGroup()) {
        return false;
    }
    state = found->second;
    return true;
}

}  // namespace

namespace facebook_like {

void BeginSession(CUser* user) {
    const FacebookLikeConfig* const config = ConfigCache().get();
    if (!config || !IsTownReady(user)) {
        return;
    }

    facebook_like_db::StateMessage request;
    std::memset(&request, 0, sizeof(request));
    request.reply_user_idx = -1;
    {
        MutexGuard guard(&g_facebook_like_mutex);
        for (std::map<uint64_t, PendingMail>::iterator it =
                 g_facebook_like_pending_mail.begin();
             it != g_facebook_like_pending_mail.end();) {
            if (it->second.user == user) {
                g_facebook_like_pending_mail.erase(it++);
            } else {
                ++it;
            }
        }
        SessionState& state = g_facebook_like_sessions[user];
        state = SessionState();
        state.charac_no = CurrentCharacterNo(user);
        state.server_group = user->GetServerGroup();
        state.token = NextNonZeroTokenLocked(g_next_session_token);
        state.pending_operation = facebook_like_db::OPERATION_LOAD;

        request.charac_no = state.charac_no;
        request.session_token = state.token;
        request.server_group = state.server_group;
    }
    request.used_fatigue = static_cast<uint16_t>(
        user->getCurCharacUsedFatigue());
    request.operation = facebook_like_db::OPERATION_LOAD;
    if (!facebook_like_db::DB_FacebookLike::queueRequest(
            user->GetUID(), request)) {
        TraceFailure("login-state DB_Q submission failed",
                     request.charac_no,
                     0u);
        ResetSession(user);
    }
}

void ResetSession(CUser* user) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_facebook_like_mutex);
    g_facebook_like_sessions.erase(user);
    for (std::map<uint64_t, PendingMail>::iterator it =
             g_facebook_like_pending_mail.begin();
         it != g_facebook_like_pending_mail.end();) {
        if (it->second.user == user) {
            g_facebook_like_pending_mail.erase(it++);
        } else {
            ++it;
        }
    }
}

void OnFatigueChanged(CUser* user, uint16_t used_fatigue) {
    const FacebookLikeConfig* const config = ConfigCache().get();
    if (!config || !user || CurrentCharacterNo(user) == 0u ||
        used_fatigue < static_cast<uint16_t>(config->fatigue_condition)) {
        return;
    }

    bool should_queue = false;
    {
        MutexGuard guard(&g_facebook_like_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_facebook_like_sessions.find(user);
        should_queue =
            found != g_facebook_like_sessions.end() &&
            found->second.loaded && found->second.pending_operation == 0u &&
            found->second.charac_no == CurrentCharacterNo(user) &&
            found->second.stage ==
                facebook_like_db::STAGE_WAIT_FATIGUE;
    }
    if (should_queue &&
        !QueueOperation(user,
                        facebook_like_db::OPERATION_FATIGUE_SYNC,
                        0u,
                        0u,
                        0u,
                        0u,
                        0u,
                        0u)) {
        TraceFailure("fatigue-state DB_Q submission failed",
                     CurrentCharacterNo(user),
                     0u);
    }
}

void OnTrackedMailResult(const SIG_MAILBOX_PERSISTENCE_CONTEXT& context,
                         bool persisted) {
    if (context.magic != MAILBOX_PERSISTENCE_CONTEXT_MAGIC ||
        context.version != MAILBOX_PERSISTENCE_CONTEXT_VERSION ||
        context.consumer !=
            MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE ||
        context.claim_token == 0u || context.receiver_charac_no == 0u ||
        context.reward_item_id < 690000945u ||
        context.reward_item_id > 690000993u || context.server_group < 0 ||
        context.server_group >=
            facebook_like_db::kMaximumServerGroupCount) {
        return;
    }

    facebook_like_db::StateMessage request;
    std::memset(&request, 0, sizeof(request));
    request.reply_user_idx = -1;
    {
        MutexGuard guard(&g_facebook_like_mutex);
        const std::map<uint64_t, PendingMail>::const_iterator pending =
            g_facebook_like_pending_mail.find(PendingMailKey(
                context.receiver_charac_no, context.claim_token));
        if (pending != g_facebook_like_pending_mail.end() &&
            pending->second.charac_no == context.receiver_charac_no &&
            pending->second.reward_item_id == context.reward_item_id &&
            pending->second.server_group == context.server_group) {
            request.session_token = pending->second.session_token;
            CUser* const user = pending->second.user;
            std::map<CUser*, SessionState>::iterator session =
                g_facebook_like_sessions.find(user);
            if (user && session != g_facebook_like_sessions.end() &&
                session->second.token == pending->second.session_token &&
                session->second.charac_no == context.receiver_charac_no &&
                session->second.server_group == context.server_group &&
                session->second.mail_waiting) {
                const int reply_user_idx = user->GetUID();
                if (reply_user_idx >= 0) {
                    request.reply_user_idx = reply_user_idx;
                }
            }
            request.selected_reward_row = pending->second.reward_row;
        }
    }
    if (request.selected_reward_row == 0u) {
        request.selected_reward_row =
            context.reward_item_id - 690000944u;
    }
    request.charac_no = context.receiver_charac_no;
    request.server_group = context.server_group;
    request.operation = facebook_like_db::OPERATION_MAIL_RESULT;
    request.action = 2u;
    request.selected_reward_item_id = context.reward_item_id;
    request.claim_token = context.claim_token;
    request.mail_result_reward_item_id = context.reward_item_id;
    request.mail_result = persisted ? 1u : 0u;

    // The DB outer route is always userless.  DBDispatcher therefore cannot
    // reject this durable ACK merely because CUser disappeared between postal
    // persistence and SQL018 completion.
    if (!facebook_like_db::DB_FacebookLike::queueRequest(
            -1, request)) {
        CUser* failure_user = 0;
        {
            MutexGuard guard(&g_facebook_like_mutex);
            const std::map<uint64_t, PendingMail>::const_iterator pending =
                g_facebook_like_pending_mail.find(PendingMailKey(
                    context.receiver_charac_no, context.claim_token));
            if (request.reply_user_idx >= 0 &&
                pending != g_facebook_like_pending_mail.end() &&
                PendingMailMatches(pending->second,
                                   pending->second.user,
                                   request.session_token,
                                   context.receiver_charac_no,
                                   context.reward_item_id,
                                   context.server_group)) {
                failure_user = pending->second.user;
            }
            ClearPendingMailLocked(context.receiver_charac_no,
                                   context.claim_token,
                                   context.reward_item_id,
                                   context.server_group,
                                   request.session_token);
        }
        if (failure_user && failure_user->GetUID() == request.reply_user_idx &&
            IsCharacterReady(failure_user) &&
            CurrentCharacterNo(failure_user) == context.receiver_charac_no &&
            failure_user->GetServerGroup() == context.server_group) {
            SendCommandResult(failure_user, false, 2u, ERROR_DATABASE);
        }
        TraceFailure("mail-result DB_Q submission failed",
                     context.receiver_charac_no,
                     context.claim_token);
    }
}

}  // namespace facebook_like_0828

Dispatcher_FacebookLike::Dispatcher_FacebookLike()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_FacebookLike::~Dispatcher_FacebookLike() {}

int Dispatcher_FacebookLike::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    uint8_t action = 0xffu;
    uint8_t selection_index = 0u;
    if (packet.get_byte(action) != 1 ||
        (action == 1u && packet.get_byte(selection_index) != 1) ||
        action > 2u) {
        SendCommandResult(user, false, action <= 2u ? action : 0u, ERROR_STATE);
        return 0;
    }
    if (!IsTownReady(user)) {
        SendCommandResult(user, false, action, ERROR_TOWN_ONLY);
        return 0;
    }
    const FacebookLikeConfig* const config = ConfigCache().get();
    if (!config) {
        SendCommandResult(user, false, action, ERROR_EVENT_CLOSED);
        return 0;
    }

    SessionState session;
    if (!SnapshotSession(user, session) || session.pending_operation != 0u ||
        session.mail_waiting) {
        SendCommandResult(user, false, action, ERROR_PENDING);
        return 0;
    }

    uint8_t operation = 0u;
    uint32_t reward_row = 0u;
    uint32_t reward_item_id = 0u;
    uint32_t claim_token = 0u;
    if (action == 0u) {
        operation = facebook_like_db::OPERATION_COUNTDOWN_COMPLETE;
    } else if (action == 1u) {
        const RewardConfig* const reward =
            config->resolve(session.weekday, selection_index);
        if (!reward) {
            SendCommandResult(user, false, action, ERROR_STATE);
            return 0;
        }
        operation = facebook_like_db::OPERATION_SELECT_REWARD;
        reward_row = static_cast<uint32_t>(reward->row);
        reward_item_id = static_cast<uint32_t>(reward->item_id);
    } else {
        operation = facebook_like_db::OPERATION_RESERVE_MAIL;
        reward_row = session.selected_reward_row;
        reward_item_id = session.selected_reward_item_id;
        // SQL018 generates the durable nonzero token inside its row-locked
        // transaction.  A process-local counter could collide with an
        // unexpired lease after a server restart.
        claim_token = 0u;
    }

    if (!QueueOperation(user,
                        operation,
                        action,
                        selection_index,
                        reward_row,
                        reward_item_id,
                        claim_token,
                        0u)) {
        SendCommandResult(user, false, action, ERROR_DATABASE);
    }
    return 0;
}

int Dispatcher_FacebookLike::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_FacebookLike::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_FacebookLike::Inter_FacebookLike() : ISigDispatcher() {}

Inter_FacebookLike::~Inter_FacebookLike() {}

int Inter_FacebookLike::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    typedef facebook_like_db::StateMessage StateMessage;
    if (!payload || payload_size != static_cast<int>(sizeof(StateMessage))) {
        return 0;
    }
    const StateMessage* const result =
        reinterpret_cast<const StateMessage*>(payload);
    if (!user) {
        return 0;
    }

    bool current = false;
    {
        MutexGuard guard(&g_facebook_like_mutex);
        std::map<CUser*, SessionState>::iterator session =
            g_facebook_like_sessions.find(user);
        bool mail_result_correlated = false;
        if (result->operation ==
            facebook_like_db::OPERATION_MAIL_RESULT) {
            const std::map<uint64_t, PendingMail>::const_iterator pending =
                g_facebook_like_pending_mail.find(PendingMailKey(
                    result->charac_no, result->claim_token));
            mail_result_correlated =
                result->reply_user_idx >= 0 &&
                result->reply_user_idx == user->GetUID() &&
                pending != g_facebook_like_pending_mail.end() &&
                PendingMailMatches(pending->second,
                                   user,
                                   result->session_token,
                                   result->charac_no,
                                   result->mail_result_reward_item_id,
                                   result->server_group);
        }
        current = session != g_facebook_like_sessions.end() &&
                  session->second.token == result->session_token &&
                  session->second.charac_no == result->charac_no &&
                  session->second.charac_no == CurrentCharacterNo(user) &&
                  session->second.server_group == result->server_group &&
                  session->second.server_group == user->GetServerGroup() &&
                  (result->operation ==
                           facebook_like_db::OPERATION_MAIL_RESULT
                       ? (mail_result_correlated &&
                          session->second.mail_waiting)
                       : session->second.pending_operation ==
                             result->operation);
        if (current) {
            if (result->operation !=
                facebook_like_db::OPERATION_MAIL_RESULT) {
                session->second.pending_operation = 0u;
            }
            if (result->result !=
                facebook_like_db::RESULT_DATABASE_FAILURE) {
                session->second.loaded = true;
                session->second.cycle_key = result->cycle_key;
                session->second.countdown_deadline =
                    result->countdown_deadline;
                session->second.selected_reward_row =
                    result->selected_reward_row;
                session->second.selected_reward_item_id =
                    result->selected_reward_item_id;
                if (result->operation ==
                    facebook_like_db::OPERATION_MAIL_RESULT) {
                    session->second.claim_token =
                        result->mail_result != 0u &&
                                result->stage ==
                                    facebook_like_db::STAGE_REWARD_MAILED &&
                                result->mail_state ==
                                    facebook_like_db::MAIL_STATE_PERSISTED
                            ? result->claim_token
                            : 0u;
                } else {
                    session->second.claim_token = result->claim_token;
                }
                session->second.stage = result->stage;
                session->second.weekday = result->weekday;
                session->second.mail_state = result->mail_state;
            }
        }
        if (mail_result_correlated) {
            // This runs before every result branch below.  Database failure,
            // cycle rollover/INVALID_STATE and negative postal ACK all consume
            // the exact in-memory correlation and release mail_waiting.
            ClearPendingMailLocked(result->charac_no,
                                   result->claim_token,
                                   result->mail_result_reward_item_id,
                                   result->server_group,
                                   result->session_token);
        }
    }
    // Fatigue crosses the threshold inside a dungeon, and the tracked postal
    // ACK may return after the character leaves town.  Once identity and
    // generation match, native NOTI698/699 and the action-2 result remain
    // safe to deliver; only accepting a new CMD725 is town-restricted.
    if (!current || !IsCharacterReady(user)) {
        return 0;
    }

    if (result->result == facebook_like_db::RESULT_DATABASE_FAILURE) {
        if (result->operation >=
            facebook_like_db::OPERATION_COUNTDOWN_COMPLETE) {
            SendCommandResult(user, false, result->action, ERROR_DATABASE);
        }
        TraceFailure("database operation failed",
                     result->charac_no,
                     result->claim_token);
        return 0;
    }

    SendServerTime(user, result->now_epoch);
    if (result->operation == facebook_like_db::OPERATION_LOAD ||
        result->operation == facebook_like_db::OPERATION_FATIGUE_SYNC) {
        SendState(user, *result);
        if (result->operation == facebook_like_db::OPERATION_LOAD) {
            // The first dungeon fatigue point can race the asynchronous login
            // load.  Reconcile against the current authoritative character
            // value after installing the DB snapshot so an exact threshold
            // crossing is not delayed until the next room.
            facebook_like::OnFatigueChanged(
                user,
                static_cast<uint16_t>(user->getCurCharacUsedFatigue()));
        }
        return 0;
    }

    if (result->result != facebook_like_db::RESULT_SUCCESS) {
        SendCommandResult(
            user, false, result->action, MapDatabaseError(result->result));
        SendState(user, *result);
        return 0;
    }

    if (result->operation ==
            facebook_like_db::OPERATION_COUNTDOWN_COMPLETE ||
        result->operation ==
            facebook_like_db::OPERATION_SELECT_REWARD) {
        SendCommandResult(user, true, result->action, 0u);
        SendState(user, *result);
        return 0;
    }

    if (result->operation ==
        facebook_like_db::OPERATION_RESERVE_MAIL) {
        if (result->stage == facebook_like_db::STAGE_REWARD_MAILED &&
            result->mail_state ==
                facebook_like_db::MAIL_STATE_PERSISTED) {
            SendCommandResult(user, true, 2u, 0u);
            SendState(user, *result);
            return 0;
        }
        if (result->stage !=
                facebook_like_db::STAGE_REWARD_SELECTED ||
            result->mail_state !=
                facebook_like_db::MAIL_STATE_RESERVED ||
            result->claim_token == 0u) {
            SendCommandResult(user, false, 2u, ERROR_STATE);
            SendState(user, *result);
            return 0;
        }

        PendingMail pending;
        pending.user = user;
        pending.session_token = result->session_token;
        pending.charac_no = result->charac_no;
        pending.reward_row = result->selected_reward_row;
        pending.reward_item_id = result->selected_reward_item_id;
        pending.server_group = result->server_group;
        {
            MutexGuard guard(&g_facebook_like_mutex);
            g_facebook_like_pending_mail[PendingMailKey(
                result->charac_no, result->claim_token)] = pending;
            std::map<CUser*, SessionState>::iterator session =
                g_facebook_like_sessions.find(user);
            if (session != g_facebook_like_sessions.end() &&
                session->second.token == result->session_token) {
                session->second.mail_waiting = true;
            }
        }

        const FacebookLikeConfig* const config = ConfigCache().get();
        SIG_MAILBOX_PERSISTENCE_CONTEXT persistence;
        std::memset(&persistence, 0, sizeof(persistence));
        if (!config || !SubmitRewardMail(user, *config, *result, persistence)) {
            // Queue acceptance is not persistence.  Treat a local submission
            // failure exactly like a negative tracked ACK and release SQL018.
            if (persistence.claim_token == 0u) {
                persistence.magic = MAILBOX_PERSISTENCE_CONTEXT_MAGIC;
                persistence.version = MAILBOX_PERSISTENCE_CONTEXT_VERSION;
                persistence.consumer =
                    MAILBOX_PERSISTENCE_CONSUMER_FACEBOOK_LIKE;
                persistence.claim_token = result->claim_token;
                persistence.receiver_charac_no = result->charac_no;
                persistence.reward_item_id = result->selected_reward_item_id;
                persistence.server_group = result->server_group;
            }
            facebook_like::OnTrackedMailResult(persistence, false);
        }
        return 0;
    }

    if (result->operation == facebook_like_db::OPERATION_MAIL_RESULT) {
        if (result->mail_result != 0u &&
            result->stage == facebook_like_db::STAGE_REWARD_MAILED &&
            result->mail_state ==
                facebook_like_db::MAIL_STATE_PERSISTED) {
            SendCommandResult(user, true, 2u, 0u);
        } else {
            SendCommandResult(user, false, 2u, ERROR_MAIL);
        }
        SendState(user, *result);
    }
    return 0;
}
