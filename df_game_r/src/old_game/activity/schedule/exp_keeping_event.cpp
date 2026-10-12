#include "old_game/activity/schedule/exp_keeping_event.h"

#include "common/config.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_exp_keeping_event.h"
#include "old_game/player/core/cuser.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/combo_skill.h"

#include <climits>
#include <cstring>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace {

enum CommandError {
    ERROR_TOWN_ONLY = 1,
    ERROR_DATABASE = 3,
    ERROR_PENDING = 19,
    ERROR_STATE = 21,
    ERROR_EVENT_CLOSED = 178,
    ERROR_NOT_ELIGIBLE = 179,
    ERROR_NOTHING_TO_CLAIM = 180,
};

struct TripleKeepingRow {
    TripleKeepingRow() : day(0), kind(0), item_id(0), count(0) {}
    int day;
    int kind;
    int item_id;
    int count;
};

struct ExpKeepingConfig {
    ExpKeepingConfig()
        : minimum_keeping_level(0),
          minimum_reward_level(0),
          triple_rows(),
          mail_title(),
          mail_content(),
          disabled_dungeons() {}

    int minimum_keeping_level;
    int minimum_reward_level;
    std::vector<TripleKeepingRow> triple_rows;
    std::string mail_title;
    std::string mail_content;
    std::vector<int> disabled_dungeons;

    bool IsDisabledDungeon(uint32_t dungeon_index) const {
        for (std::vector<int>::const_iterator it = disabled_dungeons.begin();
             it != disabled_dungeons.end();
             ++it) {
            if (*it >= 0 && static_cast<uint32_t>(*it) == dungeon_index) {
                return true;
            }
        }
        return false;
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

class ExpKeepingConfigCache {
public:
    ExpKeepingConfigCache() : attempted_(false), valid_(false), config_() {}

    const ExpKeepingConfig* Get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = Load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool Load() {
        if (!loadRDARScriptFile("Event/", "tw_tripleexpkeeping.evt")) {
            return false;
        }
        RdarUnloadGuard unload;

        ExpKeepingConfig parsed;
        bool have_minimum_keeping = false;
        bool have_minimum_reward = false;
        bool have_triple_rows = false;
        bool have_mail_title = false;
        bool have_mail_content = false;
        bool have_disabled = false;
        bool malformed = false;
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag == "[minimum level for keeping]") {
                bool success = false;
                const int value = ScanInt(&success);
                if (have_minimum_keeping || !success) {
                    malformed = true;
                } else {
                    parsed.minimum_keeping_level = value;
                }
                have_minimum_keeping = true;
            } else if (tag == "[minimum level for reward]") {
                bool success = false;
                const int value = ScanInt(&success);
                if (have_minimum_reward || !success) {
                    malformed = true;
                } else {
                    parsed.minimum_reward_level = value;
                }
                have_minimum_reward = true;
            } else if (tag == "[triple exp keeping info]") {
                if (have_triple_rows) {
                    malformed = true;
                }
                have_triple_rows = true;
                for (int index = 0; index < 9; ++index) {
                    bool success = false;
                    TripleKeepingRow row;
                    row.day = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    row.kind = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    row.item_id = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    row.count = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    parsed.triple_rows.push_back(row);
                }
                std::string closing;
                if (ScanType(closing, true) != 1 ||
                    closing != "[/triple exp keeping info]") {
                    malformed = true;
                }
            } else if (tag == "[mail title]") {
                if (have_mail_title || ScanStr(&parsed.mail_title) != 1) {
                    malformed = true;
                }
                have_mail_title = true;
            } else if (tag == "[mail content]") {
                if (have_mail_content || ScanStr(&parsed.mail_content) != 1) {
                    malformed = true;
                }
                have_mail_content = true;
            } else if (tag == "[disable exp keeping dungeon list]") {
                if (have_disabled) {
                    malformed = true;
                }
                have_disabled = true;
                for (int index = 0; index < 5; ++index) {
                    bool success = false;
                    const int dungeon = ScanInt(&success);
                    if (!success) {
                        malformed = true;
                        break;
                    }
                    parsed.disabled_dungeons.push_back(dungeon);
                }
                std::string closing;
                if (ScanType(closing, true) != 1 ||
                    closing != "[/disable exp keeping dungeon list]") {
                    malformed = true;
                }
            } else {
                // This owner intentionally refuses a silently widened PVF.
                malformed = true;
            }
        }

        static const int kExpectedRows[9][4] = {
            {1, 9, 690001019, 1},
            {2, 1, 0, 0},
            {3, 1, 0, 0},
            {4, 3, 690001017, 1},
            {5, 1, 0, 0},
            {6, 1, 0, 0},
            {7, 6, 690001018, 1},
            {8, 1, 0, 0},
            {9, 1, 0, 0},
        };
        static const int kExpectedDisabled[] = {
            10000, 10001, 10002, 10003, 3506,
        };

        if (malformed || !have_minimum_keeping || !have_minimum_reward ||
            !have_triple_rows || !have_mail_title ||
            !have_mail_content || !have_disabled ||
            parsed.minimum_keeping_level !=
                exp_keeping::kMinimumKeepingLevelExclusive ||
            parsed.minimum_reward_level !=
                exp_keeping::kMinimumRewardLevelExclusive ||
            parsed.mail_title != "Triple_Event_01" ||
            parsed.mail_content != "Triple_Event_02" ||
            parsed.triple_rows.size() != 9u ||
            parsed.disabled_dungeons.size() != 5u) {
            return false;
        }
        for (size_t index = 0; index < parsed.triple_rows.size(); ++index) {
            const TripleKeepingRow& row = parsed.triple_rows[index];
            if (row.day != kExpectedRows[index][0] ||
                row.kind != kExpectedRows[index][1] ||
                row.item_id != kExpectedRows[index][2] ||
                row.count != kExpectedRows[index][3]) {
                return false;
            }
        }
        for (size_t index = 0;
             index < parsed.disabled_dungeons.size();
             ++index) {
            if (parsed.disabled_dungeons[index] != kExpectedDisabled[index]) {
                return false;
            }
        }
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    ExpKeepingConfig config_;
};

enum PendingPhase {
    PENDING_NONE = 0,
    PENDING_RESERVE = 1,
    PENDING_COMMIT = 2,
};

struct SessionState {
    SessionState()
        : account_id(0u),
          charac_no(0u),
          session_token(0u),
          next_request_token(1u),
          server_group(-1),
          pending_phase(PENDING_NONE),
          pending_commit(),
          has_local_target(false),
          applying_claim(false) {
        std::memset(&pending_commit, 0, sizeof(pending_commit));
    }

    uint32_t account_id;
    uint32_t charac_no;
    uint32_t session_token;
    uint32_t next_request_token;
    int server_group;
    uint8_t pending_phase;
    exp_keeping::Message pending_commit;
    bool has_local_target;
    bool applying_claim;
};

Mutex g_exp_keeping_mutex;
std::map<CUser*, SessionState> g_exp_keeping_sessions;
uint32_t g_next_session_token = 1u;

ExpKeepingConfigCache& ConfigCache() {
    static ExpKeepingConfigCache cache;
    return cache;
}

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) {
        result = value++;
    }
    if (value == 0u) {
        ++value;
    }
    return result;
}

uint32_t CurrentCharacterNo(CUser* user) {
    if (!user || !user->getCurCharacR()) {
        return 0u;
    }
    const int charac_no = user->getCurCharacNo();
    return charac_no > 0 ? static_cast<uint32_t>(charac_no) : 0u;
}

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 &&
           server_group < exp_keeping::kMaximumServerGroupCount;
}

bool IsSelectedCharacter(CUser* user) {
    return user && user->get_state() > 2 && CurrentCharacterNo(user) != 0u &&
           user->get_acc_id() > 0 && IsValidServerGroup(user->GetServerGroup());
}

bool IsTownReady(CUser* user) {
    return IsSelectedCharacter(user) && user->get_state() == 3;
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace("CMD594 EXP keeping 0828", __LINE__, 5);
    trace("[0828 EXP keeping] %s account(%u) charac(%u)",
          reason ? reason : "unknown",
          static_cast<unsigned int>(
              user && user->get_acc_id() > 0 ? user->get_acc_id() : 0),
          static_cast<unsigned int>(CurrentCharacterNo(user)));
}

exp_keeping::SplitToken MakeSplitToken(uint64_t value) {
    exp_keeping::SplitToken token;
    token.low = static_cast<uint32_t>(value & 0xffffffffULL);
    token.high = static_cast<uint32_t>(value >> 32u);
    return token;
}

uint64_t JoinToken(const exp_keeping::SplitToken& value) {
    return (static_cast<uint64_t>(value.high) << 32u) |
           static_cast<uint64_t>(value.low);
}

bool EqualSnapshot(const exp_keeping::CharacterSnapshot& lhs,
                   const exp_keeping::CharacterSnapshot& rhs) {
    return lhs.level == rhs.level &&
           lhs.experience == rhs.experience &&
           lhs.remain_sp_first == rhs.remain_sp_first &&
           lhs.remain_sp_second == rhs.remain_sp_second &&
           lhs.remain_sfp_first == rhs.remain_sfp_first &&
           lhs.remain_sfp_second == rhs.remain_sfp_second;
}

bool CaptureSnapshot(CUser* user,
                     exp_keeping::CharacterSnapshot& snapshot) {
    if (!IsSelectedCharacter(user)) {
        return false;
    }
    SkillSlot* const skills = user->getCurCharacSkillR();
    const int level = user->get_charac_level();
    const int experience = user->getCurCharacExp();
    if (!skills || level <= 0 || experience < 0) {
        return false;
    }
    const int sp_first =
        skills->get_remain_sp_at_index(ENUM_SKILL_TREE_0);
    const int sp_second =
        skills->get_remain_sp_at_index(ENUM_SKILL_TREE_1);
    const int sfp_first =
        skills->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_0);
    const int sfp_second =
        skills->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_1);
    if (sp_first < 0 || sp_second < 0 || sfp_first < 0 || sfp_second < 0) {
        return false;
    }
    snapshot.level = static_cast<uint32_t>(level);
    snapshot.experience = static_cast<uint32_t>(experience);
    snapshot.remain_sp_first = static_cast<uint32_t>(sp_first);
    snapshot.remain_sp_second = static_cast<uint32_t>(sp_second);
    snapshot.remain_sfp_first = static_cast<uint32_t>(sfp_first);
    snapshot.remain_sfp_second = static_cast<uint32_t>(sfp_second);
    return true;
}

bool ComputeTargetSnapshot(
    const exp_keeping::CharacterSnapshot& baseline,
    uint32_t amount,
    exp_keeping::CharacterSnapshot& target) {
    if (amount == 0u ||
        amount > exp_keeping::kDailyExperienceCap ||
        baseline.level <=
            exp_keeping::kMinimumRewardLevelExclusive ||
        baseline.level >= exp_keeping::kMaximumCharacterLevel ||
        baseline.experience > static_cast<uint32_t>(INT_MAX) ||
        baseline.remain_sp_first > static_cast<uint32_t>(INT_MAX) ||
        baseline.remain_sp_second > static_cast<uint32_t>(INT_MAX) ||
        baseline.remain_sfp_first > static_cast<uint32_t>(INT_MAX) ||
        baseline.remain_sfp_second > static_cast<uint32_t>(INT_MAX)) {
        return false;
    }

    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) {
        return false;
    }

    uint32_t level = baseline.level;
    int experience = static_cast<int>(baseline.experience);
    int remaining = static_cast<int>(amount);
    int gained_sp = 0;
    int gained_sfp = 0;
    while (remaining > 0 &&
           level < exp_keeping::kMaximumCharacterLevel) {
        const int bottom = data_manager->get_level_exp(
            static_cast<int>(level));
        const int top = data_manager->get_level_exp(
            static_cast<int>(level + 1u));
        if (bottom < 0 || top <= bottom || experience < bottom ||
            experience > top) {
            // Native _check_level_up repairs an inconsistent row to bottom,
            // but a durable claim must not silently replace unrelated damage.
            return false;
        }
        const int needed = top - experience;
        if (remaining < needed) {
            experience += remaining;
            remaining = 0;
            break;
        }
        remaining -= needed;
        experience = top;
        ++level;
        const int level_sp = data_manager->GetSpAtLevelUp(
            static_cast<int>(level));
        if (level_sp < 0 || gained_sp > INT_MAX - level_sp) {
            return false;
        }
        gained_sp += level_sp;
        if (level > 49u) {
            if (gained_sfp == INT_MAX) {
                return false;
            }
            ++gained_sfp;
        }
    }

    target = baseline;
    target.level = level;
    target.experience = static_cast<uint32_t>(experience);
    target.remain_sp_first = static_cast<uint32_t>(
        baseline.remain_sp_first > static_cast<uint32_t>(INT_MAX - gained_sp)
            ? INT_MAX
            : static_cast<int>(baseline.remain_sp_first) + gained_sp);
    target.remain_sp_second = static_cast<uint32_t>(
        baseline.remain_sp_second > static_cast<uint32_t>(INT_MAX - gained_sp)
            ? INT_MAX
            : static_cast<int>(baseline.remain_sp_second) + gained_sp);
    target.remain_sfp_first = static_cast<uint32_t>(
        baseline.remain_sfp_first >
                static_cast<uint32_t>(INT_MAX - gained_sfp)
            ? INT_MAX
            : static_cast<int>(baseline.remain_sfp_first) + gained_sfp);
    target.remain_sfp_second = static_cast<uint32_t>(
        baseline.remain_sfp_second >
                static_cast<uint32_t>(INT_MAX - gained_sfp)
            ? INT_MAX
            : static_cast<int>(baseline.remain_sfp_second) + gained_sfp);
    return true;
}

void SendCommandResult(CUser* user, bool success, uint8_t error) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REQUEST_EXP_KEEPING_EVENT_REWARD);
    packet.put_byte(success ? 1 : 0);
    if (!success) {
        packet.put_byte(error);
    }
    // CLIENT_IDA_EVIDENCE: sub_40C450 consumes no business-success tail.
    packet.finalize(true);
    user->Send(packet);
}

void SendKeepingAmount(CUser* user, uint32_t amount, bool available) {
    if (!user || amount > exp_keeping::kDailyExperienceCap) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_EXP_KEEPING_AMOUNT);
    // CLIENT_IDA_EVIDENCE: sub_40C350 reads exactly u32 amount,u8 available.
    packet.put_int(static_cast<int>(amount));
    packet.put_byte(available ? 1 : 0);
    packet.finalize(true);
    user->Send(packet);
}

uint8_t MapCommandError(uint8_t result) {
    switch (result) {
        case exp_keeping::RESULT_EVENT_CLOSED:
            return ERROR_EVENT_CLOSED;
        case exp_keeping::RESULT_NOT_ELIGIBLE:
            return ERROR_NOT_ELIGIBLE;
        case exp_keeping::RESULT_NOTHING_TO_CLAIM:
            return ERROR_NOTHING_TO_CLAIM;
        case exp_keeping::RESULT_PENDING:
            return ERROR_PENDING;
        case exp_keeping::RESULT_STALE_CHARACTER:
            return ERROR_STATE;
        default:
            return ERROR_DATABASE;
    }
}

bool FillIdentity(CUser* user,
                  uint8_t operation,
                  exp_keeping::Message& message) {
    std::memset(&message, 0, sizeof(message));
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found == g_exp_keeping_sessions.end() ||
        found->second.account_id !=
            static_cast<uint32_t>(user->get_acc_id()) ||
        found->second.charac_no != CurrentCharacterNo(user) ||
        found->second.server_group != user->GetServerGroup()) {
        return false;
    }
    message.account_id = found->second.account_id;
    message.charac_no = found->second.charac_no;
    message.session_token = found->second.session_token;
    message.request_token = NextNonZero(found->second.next_request_token);
    message.server_group = found->second.server_group;
    message.event_id = exp_keeping::kEventId;
    message.operation = operation;
    return true;
}

bool QueueMessage(CUser* user, const exp_keeping::Message& message) {
    return user && exp_keeping::DBExpKeepingEvent::QueueRequest(
                       user->GetUID(), message);
}

bool QueueClaimRequest(CUser* user) {
    exp_keeping::Message request;
    std::memset(&request, 0, sizeof(request));
    {
        MutexGuard guard(&g_exp_keeping_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_exp_keeping_sessions.find(user);
        if (found == g_exp_keeping_sessions.end() ||
            found->second.account_id !=
                static_cast<uint32_t>(user->get_acc_id()) ||
            found->second.charac_no != CurrentCharacterNo(user) ||
            found->second.server_group != user->GetServerGroup()) {
            return false;
        }
        SessionState& session = found->second;
        if (session.pending_phase == PENDING_COMMIT &&
            session.has_local_target) {
            request = session.pending_commit;
            request.request_token = NextNonZero(session.next_request_token);
            session.pending_commit.request_token = request.request_token;
        } else {
            request.account_id = session.account_id;
            request.charac_no = session.charac_no;
            request.session_token = session.session_token;
            request.request_token = NextNonZero(session.next_request_token);
            request.server_group = session.server_group;
            request.event_id = exp_keeping::kEventId;
            request.operation = exp_keeping::OPERATION_RESERVE;
            session.pending_phase = PENDING_RESERVE;
            session.has_local_target = false;
        }
    }
    if (QueueMessage(user, request)) {
        return true;
    }
    // A target-bearing commit remains recoverable for the next click.  A
    // reserve queue failure has not crossed any mutation boundary.
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found != g_exp_keeping_sessions.end() &&
        found->second.session_token == request.session_token &&
        found->second.pending_phase == PENDING_RESERVE) {
        found->second.pending_phase = PENDING_NONE;
    }
    return false;
}

bool ResponseMatches(CUser* user,
                     const exp_keeping::Message& response,
                     SessionState& copy) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found == g_exp_keeping_sessions.end() ||
        found->second.account_id != response.account_id ||
        found->second.charac_no != response.charac_no ||
        found->second.session_token != response.session_token ||
        found->second.server_group != response.server_group ||
        response.account_id != static_cast<uint32_t>(user->get_acc_id()) ||
        response.charac_no != CurrentCharacterNo(user) ||
        response.server_group != user->GetServerGroup()) {
        return false;
    }
    if (response.operation == exp_keeping::OPERATION_RESERVE &&
        (found->second.pending_phase != PENDING_RESERVE ||
         response.request_token + 1u != found->second.next_request_token)) {
        return false;
    }
    if (response.operation == exp_keeping::OPERATION_COMMIT &&
        (found->second.pending_phase != PENDING_COMMIT ||
         !found->second.has_local_target ||
         response.request_token !=
             found->second.pending_commit.request_token ||
         JoinToken(response.claim_token) !=
             JoinToken(found->second.pending_commit.claim_token))) {
        return false;
    }
    copy = found->second;
    return true;
}

void ClearClaimPhase(CUser* user,
                     uint32_t session_token,
                     bool retain_commit) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found == g_exp_keeping_sessions.end() ||
        found->second.session_token != session_token) {
        return;
    }
    if (retain_commit && found->second.has_local_target) {
        found->second.pending_phase = PENDING_COMMIT;
        found->second.applying_claim = false;
        return;
    }
    found->second.pending_phase = PENDING_NONE;
    found->second.has_local_target = false;
    found->second.applying_claim = false;
    std::memset(&found->second.pending_commit,
                0,
                sizeof(found->second.pending_commit));
}

bool InstallPendingCommit(CUser* user,
                          const exp_keeping::Message& reserve,
                          const exp_keeping::CharacterSnapshot& target,
                          exp_keeping::Message& commit) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found == g_exp_keeping_sessions.end() ||
        found->second.session_token != reserve.session_token ||
        found->second.pending_phase != PENDING_RESERVE) {
        return false;
    }
    commit = reserve;
    commit.operation = exp_keeping::OPERATION_COMMIT;
    commit.request_token = NextNonZero(found->second.next_request_token);
    commit.target = target;
    commit.result = 0u;
    commit.reward_available = 0u;
    commit.replayed = 0u;
    found->second.pending_commit = commit;
    found->second.pending_phase = PENDING_COMMIT;
    found->second.has_local_target = true;
    return true;
}

bool AdvancePendingCommit(CUser* user,
                          const exp_keeping::Message& stored,
                          exp_keeping::Message& commit) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_exp_keeping_sessions.find(user);
    if (found == g_exp_keeping_sessions.end() ||
        found->second.session_token != stored.session_token ||
        found->second.pending_phase != PENDING_COMMIT ||
        !found->second.has_local_target ||
        JoinToken(found->second.pending_commit.claim_token) !=
            JoinToken(stored.claim_token) ||
        !EqualSnapshot(found->second.pending_commit.baseline,
                       stored.baseline) ||
        !EqualSnapshot(found->second.pending_commit.target, stored.target)) {
        return false;
    }
    commit = found->second.pending_commit;
    commit.claim_state = exp_keeping::CLAIM_STATE_TARGET_STORED;
    commit.request_token = NextNonZero(found->second.next_request_token);
    commit.result = 0u;
    commit.reward_available = 0u;
    commit.replayed = 0u;
    found->second.pending_commit = commit;
    return true;
}

bool ApplyReservedExperience(CUser* user,
                             const exp_keeping::Message& reserve,
                             exp_keeping::CharacterSnapshot& target) {
    exp_keeping::CharacterSnapshot current;
    if (!CaptureSnapshot(user, current)) {
        return false;
    }
    if (EqualSnapshot(current, reserve.target) &&
        reserve.claim_state ==
            exp_keeping::CLAIM_STATE_TARGET_STORED) {
        target = current;
        return true;
    }
    if (!EqualSnapshot(current, reserve.baseline) ||
        reserve.kept_experience == 0u ||
        reserve.kept_experience >
            exp_keeping::kDailyExperienceCap ||
        reserve.baseline.level <=
            exp_keeping::kMinimumRewardLevelExclusive ||
        reserve.baseline.level >= exp_keeping::kMaximumCharacterLevel) {
        return false;
    }

    int gained_sp = 0;
    int gained_sfp = 0;
    int previous_sp = 0;
    int current_sp = 0;
    {
        MutexGuard guard(&g_exp_keeping_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_exp_keeping_sessions.find(user);
        if (found == g_exp_keeping_sessions.end() ||
            found->second.session_token != reserve.session_token ||
            found->second.pending_phase == PENDING_NONE ||
            found->second.applying_claim) {
            return false;
        }
        found->second.applying_claim = true;
    }
    // Reason 3 is the native same-account/link EXP path.  rewardExp remains
    // the authoritative level/SP/SFP, quest, guild and notification owner.
    user->rewardExp(static_cast<int>(reserve.kept_experience),
                    gained_sp,
                     gained_sfp,
                     previous_sp,
                     current_sp,
                     static_cast<eExpAddReason>(3),
                     true);
    {
        MutexGuard guard(&g_exp_keeping_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_exp_keeping_sessions.find(user);
        if (found == g_exp_keeping_sessions.end() ||
            found->second.session_token != reserve.session_token ||
            !found->second.applying_claim) {
            return false;
        }
        found->second.applying_claim = false;
    }
    if (!CaptureSnapshot(user, target)) {
        return false;
    }
    return reserve.claim_state !=
               exp_keeping::CLAIM_STATE_TARGET_STORED ||
           EqualSnapshot(target, reserve.target);
}

}  // namespace

namespace exp_keeping {

void BeginSession(CUser* user) {
    if (!IsTownReady(user)) {
        return;
    }
    Message load;
    std::memset(&load, 0, sizeof(load));
    {
        MutexGuard guard(&g_exp_keeping_mutex);
        SessionState& state = g_exp_keeping_sessions[user];
        state = SessionState();
        state.account_id = static_cast<uint32_t>(user->get_acc_id());
        state.charac_no = CurrentCharacterNo(user);
        state.session_token = NextNonZero(g_next_session_token);
        state.server_group = user->GetServerGroup();
        load.account_id = state.account_id;
        load.charac_no = state.charac_no;
        load.session_token = state.session_token;
        load.request_token = NextNonZero(state.next_request_token);
        load.server_group = state.server_group;
        load.event_id = kEventId;
        load.operation = OPERATION_LOAD;
    }
    if (!QueueMessage(user, load)) {
        TraceFailure("initial state queue failed", user);
        SendKeepingAmount(user, 0u, false);
    }
}

void ResetSession(CUser* user) {
    MutexGuard guard(&g_exp_keeping_mutex);
    g_exp_keeping_sessions.erase(user);
}

uint64_t MakeMonsterSourceToken(uint32_t party_index,
                                uint64_t map_start_time,
                                uint32_t map_index,
                                uint32_t monster_unique_id,
                                uint32_t charac_no) {
    // FNV-1a over the authoritative battle tuple.  A collision safely
    // under-awards; it can never duplicate EXP.  Never expose zero because it
    // is the internal "no source" sentinel.
    const uint64_t values[] = {
        static_cast<uint64_t>(party_index),
        map_start_time,
        static_cast<uint64_t>(map_index),
        static_cast<uint64_t>(monster_unique_id),
        static_cast<uint64_t>(charac_no),
    };
    uint64_t hash = 14695981039346656037ULL;
    for (size_t index = 0; index < sizeof(values) / sizeof(values[0]); ++index) {
        uint64_t value = values[index];
        for (unsigned int byte = 0; byte < 8u; ++byte) {
            hash ^= value & 0xffu;
            hash *= 1099511628211ULL;
            value >>= 8u;
        }
    }
    return hash ? hash : 1u;
}

void OnMonsterExperienceAwarded(CUser* user,
                                uint32_t final_experience,
                                uint32_t dungeon_index,
                                uint32_t source_level,
                                uint64_t source_token) {
    const ExpKeepingConfig* const config = ConfigCache().Get();
    if (!config || !IsSelectedCharacter(user) ||
        RejectUnrelatedExperienceMutation(user) ||
        final_experience == 0u ||
        final_experience >
            static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        source_token == 0u || dungeon_index == 0u ||
        config->IsDisabledDungeon(dungeon_index) ||
        user->get_charac_job() != oldsrv::config::kCharacterJobFighter ||
        source_level <=
            static_cast<uint32_t>(config->minimum_keeping_level) ||
        source_level >= static_cast<uint32_t>(
                            oldsrv::config::kCharacterLevelMaximum)) {
        return;
    }

    Message request;
    if (!FillIdentity(user, OPERATION_ACCUMULATE, request)) {
        TraceFailure("monster award has no correlated session", user);
        return;
    }
    request.source_experience = final_experience;
    request.source_dungeon = dungeon_index;
    request.source_level = static_cast<uint8_t>(source_level);
    request.source_token = MakeSplitToken(source_token);
    if (!QueueMessage(user, request)) {
        TraceFailure("monster award DB queue failed", user);
    }
}

bool HasUnconfirmedClaim(CUser* user) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::const_iterator found =
        g_exp_keeping_sessions.find(user);
    return found != g_exp_keeping_sessions.end() &&
           found->second.pending_phase != PENDING_NONE;
}

bool RejectUnrelatedExperienceMutation(CUser* user) {
    MutexGuard guard(&g_exp_keeping_mutex);
    std::map<CUser*, SessionState>::const_iterator found =
        g_exp_keeping_sessions.find(user);
    return found != g_exp_keeping_sessions.end() &&
           found->second.pending_phase != PENDING_NONE &&
           !found->second.applying_claim;
}

}  // namespace exp_keeping_0828

Dispatcher_RequestExpKeepingEventReward::
    Dispatcher_RequestExpKeepingEventReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_RequestExpKeepingEventReward::~
    Dispatcher_RequestExpKeepingEventReward() {}

int Dispatcher_RequestExpKeepingEventReward::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: both sub_4F96B0@0x4F9707 and
    // sub_4FEE90@0x4FEEF4 finalize CMD594 without writing a payload.
    if (!IsTownReady(user)) {
        if (user) {
            SendCommandResult(user, false, ERROR_TOWN_ONLY);
        }
        return 0;
    }
    if (!ConfigCache().Get()) {
        TraceFailure("strict tw_tripleexpkeeping.evt parse failed", user);
        SendCommandResult(user, false, ERROR_EVENT_CLOSED);
        return 0;
    }

    bool have_session = false;
    {
        MutexGuard guard(&g_exp_keeping_mutex);
        std::map<CUser*, SessionState>::const_iterator found =
            g_exp_keeping_sessions.find(user);
        have_session = found != g_exp_keeping_sessions.end() &&
                       found->second.account_id ==
                           static_cast<uint32_t>(user->get_acc_id()) &&
                       found->second.charac_no == CurrentCharacterNo(user) &&
                       found->second.server_group == user->GetServerGroup();
    }
    if (!have_session) {
        exp_keeping::BeginSession(user);
    }
    if (!QueueClaimRequest(user)) {
        TraceFailure("claim DB queue failed", user);
        SendCommandResult(user, false, ERROR_DATABASE);
    }
    return 0;
}

int Dispatcher_RequestExpKeepingEventReward::read(PacketBuf& packet,
                                                       MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestExpKeepingEventReward::process(CUser* user,
                                                          MSG_BASE& message,
                                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

InterExpKeepingEvent::InterExpKeepingEvent()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              exp_keeping::kTaiwanInnerCode)) {}

InterExpKeepingEvent::~InterExpKeepingEvent() {}

int InterExpKeepingEvent::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(exp_keeping::Message))) {
        return 0;
    }
    const exp_keeping::Message response =
        *reinterpret_cast<const exp_keeping::Message*>(payload);
    if (response.event_id != exp_keeping::kEventId ||
        response.operation < exp_keeping::OPERATION_LOAD ||
        response.operation > exp_keeping::OPERATION_COMMIT ||
        response.kept_experience >
            exp_keeping::kDailyExperienceCap ||
        response.reward_available > 1u ||
        response.claim_state > exp_keeping::CLAIM_STATE_COMMITTED) {
        return 0;
    }

    SessionState session;
    if (!ResponseMatches(user, response, session)) {
        return 0;
    }

    if (response.operation == exp_keeping::OPERATION_LOAD ||
        response.operation == exp_keeping::OPERATION_ACCUMULATE) {
        if (response.result == exp_keeping::RESULT_SUCCESS ||
            response.result == exp_keeping::RESULT_PENDING) {
            SendKeepingAmount(user,
                              response.kept_experience,
                              response.reward_available != 0u);
        } else if (response.result ==
                   exp_keeping::RESULT_EVENT_CLOSED) {
            SendKeepingAmount(user, 0u, false);
        }
        return 0;
    }

    if (response.result != exp_keeping::RESULT_SUCCESS) {
        const bool retain_commit =
            response.operation == exp_keeping::OPERATION_COMMIT &&
            (response.result ==
                 exp_keeping::RESULT_DATABASE_FAILURE ||
             response.claim_state ==
                 exp_keeping::CLAIM_STATE_TARGET_STORED);
        ClearClaimPhase(user, response.session_token, retain_commit);
        SendCommandResult(user, false, MapCommandError(response.result));
        return 0;
    }

    if (response.operation == exp_keeping::OPERATION_RESERVE) {
        if (response.claim_state ==
            exp_keeping::CLAIM_STATE_COMMITTED) {
            exp_keeping::CharacterSnapshot current;
            if (!CaptureSnapshot(user, current) ||
                !EqualSnapshot(current, response.target)) {
                ClearClaimPhase(user, response.session_token, false);
                SendCommandResult(user, false, ERROR_STATE);
                return 0;
            }
            ClearClaimPhase(user, response.session_token, false);
            SendKeepingAmount(user, 0u, false);
            SendCommandResult(user, true, 0u);
            return 0;
        }
        if ((response.claim_state !=
                 exp_keeping::CLAIM_STATE_RESERVED &&
             response.claim_state !=
                 exp_keeping::CLAIM_STATE_TARGET_STORED) ||
            JoinToken(response.claim_token) == 0u) {
            ClearClaimPhase(user, response.session_token, false);
            SendCommandResult(user, false, ERROR_STATE);
            return 0;
        }

        exp_keeping::CharacterSnapshot target;
        if (response.claim_state ==
            exp_keeping::CLAIM_STATE_RESERVED) {
            if (!ComputeTargetSnapshot(response.baseline,
                                       response.kept_experience,
                                       target)) {
                ClearClaimPhase(user, response.session_token, false);
                TraceFailure("cannot derive canonical target", user);
                SendCommandResult(user, false, ERROR_STATE);
                return 0;
            }
        } else {
            exp_keeping::CharacterSnapshot expected;
            if (!ComputeTargetSnapshot(response.baseline,
                                       response.kept_experience,
                                       expected) ||
                !EqualSnapshot(expected, response.target) ||
                !ApplyReservedExperience(user, response, target)) {
                ClearClaimPhase(user, response.session_token, false);
                TraceFailure("stored target/recovery mismatch", user);
                SendCommandResult(user, false, ERROR_STATE);
                return 0;
            }
        }
        exp_keeping::Message commit;
        if (!InstallPendingCommit(user, response, target, commit)) {
            TraceFailure("failed to install pending commit", user);
            SendCommandResult(user, false, ERROR_STATE);
            return 0;
        }
        if (!QueueMessage(user, commit)) {
            // Before canonical mutation this retries target publication; after
            // a recovery response it retries deterministic MyISAM replay.
            TraceFailure("claim phase DB queue failed", user);
            SendCommandResult(user, false, ERROR_DATABASE);
        }
        return 0;
    }

    if (response.operation == exp_keeping::OPERATION_COMMIT) {
        if (response.claim_state ==
            exp_keeping::CLAIM_STATE_TARGET_STORED) {
            exp_keeping::CharacterSnapshot target;
            if (!ApplyReservedExperience(user, response, target) ||
                !EqualSnapshot(target, response.target)) {
                ClearClaimPhase(user, response.session_token, true);
                TraceFailure("canonical reward target mismatch", user);
                SendCommandResult(user, false, ERROR_STATE);
                return 0;
            }
            exp_keeping::Message commit;
            if (!AdvancePendingCommit(user, response, commit)) {
                TraceFailure("failed to advance durable target", user);
                SendCommandResult(user, false, ERROR_STATE);
                return 0;
            }
            if (!QueueMessage(user, commit)) {
                TraceFailure("target replay DB queue failed", user);
                SendCommandResult(user, false, ERROR_DATABASE);
            }
            return 0;
        }
        exp_keeping::CharacterSnapshot current;
        if (response.claim_state !=
                exp_keeping::CLAIM_STATE_COMMITTED ||
            !CaptureSnapshot(user, current) ||
            !EqualSnapshot(current, response.target)) {
            ClearClaimPhase(user, response.session_token, true);
            TraceFailure("commit confirmation snapshot mismatch", user);
            SendCommandResult(user, false, ERROR_STATE);
            return 0;
        }
        ClearClaimPhase(user, response.session_token, false);
        SendKeepingAmount(user, 0u, false);
        SendCommandResult(user, true, 0u);
    }
    return 0;
}
