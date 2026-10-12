#include "old_game/dungeon/raid/dual_raid.h"

#include <algorithm>
#include <cstring>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/db/event/db_dual_raid_reward.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/party/party.h"

namespace {

const char kDualRaidDirectory[] = "event/dualraid/";
const char kDualRaidFilename[] = "dualraid.info";

enum {
    kNormalDungeonIndex = 3049,
    kBossDungeonIndex = 3050,
    kOfficialTeamCount = 2,
    kOfficialMemberCount = 3,
    kRequiredFatigue = 5,
    kMinimumLevel = 60,
    kMaximumLevel = 85,
    kNormalDifficulty = 2,
    kBossDifficulty = 2,
    kClearBonusRate = 120,
    kFailBonusRate = 80,
    kRewardItemId = 10000373,
    kClearRewardCount = 3,
    kFailRewardCount = 1,
    kUpgradeItemRevision = 10,
    kWaitingLifetimeSeconds = 120,
    kRaidLifetimeSeconds = 1800,
    kFinalizedLifetimeSeconds = 600,
    kClaimRetrySeconds = 10,
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

enum PolicySection {
    SECTION_NONE = 0,
    SECTION_NORMAL_DUNGEON,
    SECTION_BOSS_DUNGEON,
    SECTION_TEAM_COUNT,
    SECTION_USER_COUNT,
    SECTION_REQUIRED_FATIGUE,
    SECTION_IMPOSSIBLE_ITEM,
    SECTION_CLEAR_BONUS,
    SECTION_FAIL_BONUS,
    SECTION_KILL_RANK,
    SECTION_CLEAR_REWARD,
    SECTION_FAIL_REWARD,
    SECTION_BASIC_EXP,
    SECTION_MINIMUM_LEVEL,
    SECTION_UPGRADE_REVISION,
    SECTION_NORMAL_DIFFICULTY,
    SECTION_BOSS_DIFFICULTY,
    SECTION_KILL_EXP_RATE,
};

struct PolicyValue {
    PolicyValue() : token(DNF_TOK_EOF), integer(0), real(0.0f) {}
    DNFLEX_TOKEN token;
    int integer;
    float real;
};

struct DualRaidPolicy {
    DualRaidPolicy()
        : normal_dungeon(0), boss_dungeon(0), team_count(0),
          user_count(0), required_fatigue(0), clear_bonus_rate(0),
          fail_bonus_rate(0), clear_item_id(0), clear_item_count(0),
          fail_item_id(0), fail_item_count(0), minimum_level(0),
          maximum_level(0), upgrade_revision(0), normal_difficulty(0),
          boss_difficulty(0) {
        std::memset(basic_exp, 0, sizeof(basic_exp));
        std::memset(kill_rank, 0, sizeof(kill_rank));
        std::memset(kill_exp_min_level, 0, sizeof(kill_exp_min_level));
        std::memset(kill_exp_max_level, 0, sizeof(kill_exp_max_level));
        std::memset(kill_exp_rate_permyriad, 0,
                    sizeof(kill_exp_rate_permyriad));
    }

    int normal_dungeon;
    int boss_dungeon;
    int team_count;
    int user_count;
    int required_fatigue;
    int clear_bonus_rate;
    int fail_bonus_rate;
    int kill_rank[3];
    int clear_item_id;
    int clear_item_count;
    int fail_item_id;
    int fail_item_count;
    int minimum_level;
    int maximum_level;
    int upgrade_revision;
    int normal_difficulty;
    int boss_difficulty;
    uint32_t basic_exp[256];
    int kill_exp_min_level[3];
    int kill_exp_max_level[3];
    uint32_t kill_exp_rate_permyriad[3];
};

bool IsIntegerToken(DNFLEX_TOKEN token) {
    return token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL;
}

bool IsNumberToken(DNFLEX_TOKEN token) {
    return IsIntegerToken(token) || token == DNF_TOK_FLOAT;
}

const char* ClosingTag(PolicySection section) {
    switch (section) {
        case SECTION_NORMAL_DUNGEON: return "[/normal dungeon]";
        case SECTION_BOSS_DUNGEON: return "[/boss dungeon]";
        case SECTION_IMPOSSIBLE_ITEM: return "[/impossible item]";
        case SECTION_KILL_RANK: return "[/kill rank]";
        case SECTION_BASIC_EXP: return "[/basic reward exp]";
        case SECTION_KILL_EXP_RATE: return "[/kill monster exp rate]";
        default: return 0;
    }
}

PolicySection OpeningSection(const std::string& tag) {
    if (tag == "[normal dungeon]") return SECTION_NORMAL_DUNGEON;
    if (tag == "[boss dungeon]") return SECTION_BOSS_DUNGEON;
    if (tag == "[team count]") return SECTION_TEAM_COUNT;
    if (tag == "[user count]") return SECTION_USER_COUNT;
    if (tag == "[required fatigue]") return SECTION_REQUIRED_FATIGUE;
    if (tag == "[impossible item]") return SECTION_IMPOSSIBLE_ITEM;
    if (tag == "[clear bonus rate]") return SECTION_CLEAR_BONUS;
    if (tag == "[fail bonus rate]") return SECTION_FAIL_BONUS;
    if (tag == "[kill rank]") return SECTION_KILL_RANK;
    if (tag == "[clear reward item]") return SECTION_CLEAR_REWARD;
    if (tag == "[fail reward item]") return SECTION_FAIL_REWARD;
    if (tag == "[basic reward exp]") return SECTION_BASIC_EXP;
    if (tag == "[enter minimum level]") return SECTION_MINIMUM_LEVEL;
    if (tag == "[upgrade item revision]") return SECTION_UPGRADE_REVISION;
    if (tag == "[nomral dungeon difficulty]") {
        return SECTION_NORMAL_DIFFICULTY;
    }
    if (tag == "[boss dungeon difficulty]") {
        return SECTION_BOSS_DIFFICULTY;
    }
    if (tag == "[kill monster exp rate]") return SECTION_KILL_EXP_RATE;
    return SECTION_NONE;
}

bool OneInteger(const std::vector<PolicyValue>& values, int& output) {
    if (values.size() != 1u || !IsIntegerToken(values[0].token)) {
        return false;
    }
    output = values[0].integer;
    return true;
}

bool FinalizePolicySection(PolicySection section,
                           const std::vector<PolicyValue>& values,
                           DualRaidPolicy& policy,
                           uint32_t& seen_mask) {
    if (section <= SECTION_NONE || section > SECTION_KILL_EXP_RATE) {
        return false;
    }
    const uint32_t bit = 1u << (static_cast<unsigned int>(section) - 1u);
    if ((seen_mask & bit) != 0u) return false;
    seen_mask |= bit;

    switch (section) {
        case SECTION_NORMAL_DUNGEON:
            return OneInteger(values, policy.normal_dungeon);
        case SECTION_BOSS_DUNGEON:
            return OneInteger(values, policy.boss_dungeon);
        case SECTION_TEAM_COUNT:
            return OneInteger(values, policy.team_count);
        case SECTION_USER_COUNT:
            return OneInteger(values, policy.user_count);
        case SECTION_REQUIRED_FATIGUE:
            return OneInteger(values, policy.required_fatigue);
        case SECTION_IMPOSSIBLE_ITEM:
            return values.empty();
        case SECTION_CLEAR_BONUS:
            return OneInteger(values, policy.clear_bonus_rate);
        case SECTION_FAIL_BONUS:
            return OneInteger(values, policy.fail_bonus_rate);
        case SECTION_KILL_RANK:
            if (values.size() != 3u) return false;
            for (size_t index = 0u; index < 3u; ++index) {
                if (!IsIntegerToken(values[index].token)) return false;
                policy.kill_rank[index] = values[index].integer;
            }
            return true;
        case SECTION_CLEAR_REWARD:
        case SECTION_FAIL_REWARD: {
            if (values.size() != 2u ||
                !IsIntegerToken(values[0].token) ||
                !IsIntegerToken(values[1].token)) {
                return false;
            }
            int& item_id = section == SECTION_CLEAR_REWARD
                               ? policy.clear_item_id : policy.fail_item_id;
            int& count = section == SECTION_CLEAR_REWARD
                             ? policy.clear_item_count : policy.fail_item_count;
            item_id = values[0].integer;
            count = values[1].integer;
            return true;
        }
        case SECTION_BASIC_EXP:
            if (values.size() != 52u) return false;
            for (size_t index = 0u; index < values.size(); index += 2u) {
                if (!IsIntegerToken(values[index].token) ||
                    !IsIntegerToken(values[index + 1u].token)) {
                    return false;
                }
                const int level = values[index].integer;
                const int experience = values[index + 1u].integer;
                if (level < 0 || level > 255 || experience <= 0 ||
                    policy.basic_exp[level] != 0u) {
                    return false;
                }
                policy.basic_exp[level] =
                    static_cast<uint32_t>(experience);
                if (index == 0u) {
                    if (policy.minimum_level != 0 &&
                        policy.minimum_level != level) {
                        return false;
                    }
                    policy.minimum_level = level;
                }
                policy.maximum_level = level;
            }
            return true;
        case SECTION_MINIMUM_LEVEL: {
            int value = 0;
            if (!OneInteger(values, value)) return false;
            if (policy.minimum_level != 0 && policy.minimum_level != value) {
                return false;
            }
            policy.minimum_level = value;
            return true;
        }
        case SECTION_UPGRADE_REVISION:
            return OneInteger(values, policy.upgrade_revision);
        case SECTION_NORMAL_DIFFICULTY:
            return OneInteger(values, policy.normal_difficulty);
        case SECTION_BOSS_DIFFICULTY:
            return OneInteger(values, policy.boss_difficulty);
        case SECTION_KILL_EXP_RATE:
            if (values.size() != 9u) return false;
            for (size_t index = 0u; index < values.size(); ++index) {
                if (!IsNumberToken(values[index].token)) return false;
            }
            for (size_t bracket = 0u; bracket < 3u; ++bracket) {
                const size_t offset = bracket * 3u;
                const float rate = values[offset + 2u].token == DNF_TOK_FLOAT
                    ? values[offset + 2u].real
                    : static_cast<float>(values[offset + 2u].integer);
                policy.kill_exp_min_level[bracket] =
                    values[offset].integer;
                policy.kill_exp_max_level[bracket] =
                    values[offset + 1u].integer;
                policy.kill_exp_rate_permyriad[bracket] =
                    static_cast<uint32_t>(rate * 10000.0f + 0.5f);
            }
            return policy.kill_exp_min_level[0] == 60 &&
                   policy.kill_exp_max_level[0] == 65 &&
                   policy.kill_exp_rate_permyriad[0] == 8000u &&
                   policy.kill_exp_min_level[1] == 66 &&
                   policy.kill_exp_max_level[1] == 75 &&
                   policy.kill_exp_rate_permyriad[1] == 11200u &&
                   policy.kill_exp_min_level[2] == 76 &&
                   policy.kill_exp_max_level[2] == 85 &&
                   policy.kill_exp_rate_permyriad[2] == 14400u;
        default:
            return false;
    }
}

bool ParseDualRaidPolicy(DualRaidPolicy& policy) {
    PolicySection section = SECTION_NONE;
    std::vector<PolicyValue> values;
    uint32_t seen_mask = 0u;
    for (;;) {
        int integer = 0;
        float real = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token = ScanAll(&integer, &real, &text);
        if (token == DNF_TOK_EOF) {
            if (section != SECTION_NONE) {
                if (ClosingTag(section) ||
                    !FinalizePolicySection(section, values, policy,
                                           seen_mask)) {
                    return false;
                }
            }
            break;
        }
        if (token == DNF_TOK_TYPE) {
            if (section != SECTION_NONE) {
                const char* const closing = ClosingTag(section);
                if (closing) {
                    if (text != closing ||
                        !FinalizePolicySection(section, values, policy,
                                               seen_mask)) {
                        return false;
                    }
                    section = SECTION_NONE;
                    values.clear();
                    continue;
                }
                if (!FinalizePolicySection(section, values, policy,
                                           seen_mask)) {
                    return false;
                }
                section = SECTION_NONE;
                values.clear();
            }
            section = OpeningSection(text);
            if (section == SECTION_NONE) return false;
            continue;
        }
        if (section == SECTION_NONE || !IsNumberToken(token)) return false;
        PolicyValue value;
        value.token = token;
        value.integer = integer;
        value.real = real;
        values.push_back(value);
    }

    const uint32_t complete_mask =
        (1u << static_cast<unsigned int>(SECTION_KILL_EXP_RATE)) - 1u;
    if (seen_mask != complete_mask ||
        policy.normal_dungeon != kNormalDungeonIndex ||
        policy.boss_dungeon != kBossDungeonIndex ||
        policy.team_count != kOfficialTeamCount ||
        policy.user_count != kOfficialMemberCount ||
        policy.required_fatigue != kRequiredFatigue ||
        policy.clear_bonus_rate != kClearBonusRate ||
        policy.fail_bonus_rate != kFailBonusRate ||
        policy.kill_rank[0] != 120 || policy.kill_rank[1] != 110 ||
        policy.kill_rank[2] != 100 ||
        policy.clear_item_id != kRewardItemId ||
        policy.fail_item_id != kRewardItemId ||
        policy.clear_item_count != kClearRewardCount ||
        policy.fail_item_count != kFailRewardCount ||
        policy.minimum_level != kMinimumLevel ||
        policy.maximum_level != kMaximumLevel ||
        policy.upgrade_revision != kUpgradeItemRevision ||
        policy.normal_difficulty != kNormalDifficulty ||
        policy.boss_difficulty != kBossDifficulty) {
        return false;
    }
    for (int level = kMinimumLevel; level <= kMaximumLevel; ++level) {
        if (policy.basic_exp[level] == 0u) return false;
    }
    CDataManager* const manager = G_CDataManager();
    return manager && manager->find_dungeon(policy.normal_dungeon) &&
           manager->find_dungeon(policy.boss_dungeon) &&
           manager->find_item(policy.clear_item_id) &&
           manager->find_item(policy.fail_item_id);
}

class DualRaidPolicyCache {
public:
    DualRaidPolicyCache() : attempted_(false), valid_(false), policy_() {}

    const DualRaidPolicy* get() {
        MutexGuard guard(&mutex_);
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &policy_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile(kDualRaidDirectory, kDualRaidFilename)) {
            return false;
        }
        RdarUnloadGuard unload;
        DualRaidPolicy parsed;
        if (!ParseDualRaidPolicy(parsed)) return false;
        policy_ = parsed;
        return true;
    }

    Mutex mutex_;
    bool attempted_;
    bool valid_;
    DualRaidPolicy policy_;
};

DualRaidPolicyCache& PolicyCache() {
    static DualRaidPolicyCache cache;
    return cache;
}

uint32_t NowEpoch() {
    const int value = GlobalData::system_time()
                          ? GlobalData::system_time()->getCurSec() : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace("DualRaid0828", __LINE__, 5);
    trace("[0828 DualRaid] %s account(%d) charac(%d)",
          reason ? reason : "unknown",
          user ? user->get_acc_id() : 0,
          user ? user->get_charac_no(-1) : 0);
}

int NoRead(PacketBuf&, MSG_BASE&) { return 0; }
int NoProcess(CUser*, MSG_BASE&, ParamBase&) { return 0; }

struct SessionKey {
    SessionKey() : hi(0u), lo(0u) {}
    SessionKey(uint32_t high, uint32_t low) : hi(high), lo(low) {}
    bool operator<(const SessionKey& rhs) const {
        return hi < rhs.hi || (hi == rhs.hi && lo < rhs.lo);
    }
    bool operator==(const SessionKey& rhs) const {
        return hi == rhs.hi && lo == rhs.lo;
    }
    uint32_t hi;
    uint32_t lo;
};

enum RaidPhase {
    RAID_PHASE_MATCHED = 1,
    RAID_PHASE_READY = 2,
    RAID_PHASE_NORMAL_STARTING = 3,
    RAID_PHASE_NORMAL_RUNNING = 4,
    RAID_PHASE_BOSS_PENDING = 5,
    RAID_PHASE_BOSS_RUNNING = 6,
    RAID_PHASE_FINALIZED = 7,
};

enum ClaimState {
    CLAIM_IDLE = 0,
    CLAIM_QUEUED = 1,
    CLAIM_RETRY = 2,
    CLAIM_DELIVERED = 3,
};

struct RaidMember {
    RaidMember()
        : user(0), account_id(0u), charac_no(0u), unique_id(0u),
          party_seat(-1), roster_slot(0u), reward_exp(0u),
          reward_item_id(0u), reward_item_count(0u), clear_result(0u),
          claim_state(CLAIM_IDLE), next_retry_epoch(0u), claim_message() {
        std::memset(&claim_message, 0, sizeof(claim_message));
    }
    CUser* user;
    uint32_t account_id;
    uint32_t charac_no;
    uint16_t unique_id;
    int party_seat;
    uint8_t roster_slot;
    uint32_t reward_exp;
    uint32_t reward_item_id;
    uint32_t reward_item_count;
    uint8_t clear_result;
    ClaimState claim_state;
    uint32_t next_retry_epoch;
    dual_raid::RewardClaimMessage claim_message;
};

struct RaidTeam {
    RaidTeam()
        : party(0), server_group(-1), team_index(0u), ready(false),
          start_requested(false), start_succeeded(false),
          normal_finished(false), normal_cleared(false),
          boss_started(false), progress(0u),
          map_x(0u), map_y(0u), map_notice_valid(false),
          map_clear_notice_valid(false), map_clear_flag(0u) {}
    CParty* party;
    int server_group;
    uint8_t team_index;
    bool ready;
    bool start_requested;
    bool start_succeeded;
    bool normal_finished;
    bool normal_cleared;
    bool boss_started;
    uint8_t progress;
    uint8_t map_x;
    uint8_t map_y;
    bool map_notice_valid;
    bool map_clear_notice_valid;
    uint8_t map_clear_flag;
    RaidMember members[kOfficialMemberCount];
};

struct RaidSession {
    RaidSession()
        : key(), wire_session_id(0u), server_group(-1),
          phase(RAID_PHASE_MATCHED), winner_team(0xffu),
          ready_notified(false), created_epoch(0u), finalized_epoch(0u) {}
    SessionKey key;
    uint16_t wire_session_id;
    int server_group;
    RaidPhase phase;
    uint8_t winner_team;
    bool ready_notified;
    uint32_t created_epoch;
    uint32_t finalized_epoch;
    RaidTeam teams[kOfficialTeamCount];
};

struct WaitingTeam {
    WaitingTeam() : team(), queued_epoch(0u) {}
    RaidTeam team;
    uint32_t queued_epoch;
};

struct ParticipantLocator {
    ParticipantLocator()
        : key(), team_index(0u), member_index(0u), account_id(0u),
          charac_no(0u), unique_id(0u) {}
    SessionKey key;
    uint8_t team_index;
    uint8_t member_index;
    uint32_t account_id;
    uint32_t charac_no;
    uint16_t unique_id;
};

Mutex g_dual_raid_mutex;
std::list<WaitingTeam> g_waiting_teams;
std::map<SessionKey, RaidSession> g_raid_sessions;
std::map<CParty*, SessionKey> g_party_to_session;
std::map<CUser*, ParticipantLocator> g_user_to_session;
uint32_t g_next_session_counter = 1u;
uint32_t g_next_request_token = 1u;
uint16_t g_next_wire_session_id = 1u;

uint32_t NextNonZero(uint32_t& source) {
    uint32_t value = source++;
    if (value == 0u) value = source++;
    if (source == 0u) ++source;
    return value;
}

uint16_t NextWireSessionIdLocked() {
    uint16_t value = g_next_wire_session_id++;
    if (value == 0u) value = g_next_wire_session_id++;
    if (g_next_wire_session_id == 0u) ++g_next_wire_session_id;
    return value;
}

bool IsCharacterIdentityCurrent(CUser* user,
                                uint32_t account_id,
                                uint32_t charac_no,
                                uint16_t unique_id) {
    return user && user->getCurCharacR() &&
           static_cast<uint32_t>(user->get_acc_id()) == account_id &&
           static_cast<uint32_t>(user->get_charac_no(-1)) == charac_no &&
           static_cast<uint16_t>(user->get_unique_id()) == unique_id;
}

bool IsMemberCurrent(const RaidTeam& team, const RaidMember& member) {
    return team.party && member.party_seat >= 0 && member.party_seat < 4 &&
           IsCharacterIdentityCurrent(member.user, member.account_id,
                                      member.charac_no, member.unique_id) &&
           member.user->GetParty() == team.party &&
           team.party->get_user(member.party_seat) == member.user;
}

bool IsTeamCurrent(const RaidTeam& team) {
    if (!team.party || team.party->get_member_count() != kOfficialMemberCount ||
        team.party->getManager() != team.members[0].user) {
        return false;
    }
    for (int index = 0; index < kOfficialMemberCount; ++index) {
        if (!IsMemberCurrent(team, team.members[index])) return false;
    }
    return true;
}

bool CaptureTeam(CParty* party, RaidTeam& output, unsigned char& error) {
    error = 8;
    if (!party || party->get_member_count() != kOfficialMemberCount) {
        return false;
    }
    CUser* const manager = party->getManager();
    if (!manager || manager->GetParty() != party) return false;

    int manager_seat = party->GetMemberSlotNo(manager);
    if (manager_seat < 0 || manager_seat >= 4) return false;
    int ordered_seats[kOfficialMemberCount] = {manager_seat, -1, -1};
    int written = 1;
    for (int seat = 0; seat < 4; ++seat) {
        if (seat == manager_seat || party->checkValidUser(seat) != 1) continue;
        CUser* const member = party->get_user(seat);
        if (!member || written >= kOfficialMemberCount) return false;
        ordered_seats[written++] = seat;
    }
    if (written != kOfficialMemberCount) return false;

    output = RaidTeam();
    output.party = party;
    for (int index = 0; index < kOfficialMemberCount; ++index) {
        CUser* const member = party->get_user(ordered_seats[index]);
        if (!member || !member->getCurCharacR() || member->GetParty() != party ||
            member->get_acc_id() <= 0 || member->get_charac_no(-1) <= 0 ||
            member->get_unique_id() <= 0 || member->get_unique_id() > 0xffff) {
            return false;
        }
        RaidMember& snapshot = output.members[index];
        snapshot.user = member;
        snapshot.account_id = static_cast<uint32_t>(member->get_acc_id());
        snapshot.charac_no = static_cast<uint32_t>(member->get_charac_no(-1));
        snapshot.unique_id = static_cast<uint16_t>(member->get_unique_id());
        snapshot.party_seat = ordered_seats[index];
        snapshot.roster_slot = static_cast<uint8_t>(index);
        if (member->get_charac_level() < kMinimumLevel ||
            member->get_charac_level() > kMaximumLevel) {
            error = 243;
            return false;
        }
        if (member->getCurCharacFatigue() < kRequiredFatigue) {
            error = 14;
            return false;
        }
        if (index == 0) output.server_group = member->GetServerGroup();
        if (member->GetServerGroup() != output.server_group ||
            output.server_group < 0 ||
            output.server_group >= dual_raid::kMaximumServerGroupCount) {
            error = 19;
            return false;
        }
    }
    return true;
}

void SendJoinSuccess(CUser* user) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_DUAL_RAID_DUNGEON_JOIN);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);
}

void SendJoinError(CUser* user, unsigned char error) {
    if (user) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DUAL_RAID_DUNGEON_JOIN,
                                 error ? error : 19);
    }
}

void SendEmptyNotice(CUser* user, ENUM_NOTIPACKET notice) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, notice);
    packet.finalize(true);
    user->Send(packet);
}

void BroadcastEmptyNotice(const RaidSession& session,
                          ENUM_NOTIPACKET notice) {
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            const RaidMember& target = session.teams[team].members[member];
            if (IsMemberCurrent(session.teams[team], target)) {
                SendEmptyNotice(target.user, notice);
            }
        }
    }
}

void SendPartyGroupInfo(const RaidSession& session) {
    for (int target_team = 0; target_team < kOfficialTeamCount; ++target_team) {
        for (int target_member = 0; target_member < kOfficialMemberCount;
             ++target_member) {
            const RaidMember& target =
                session.teams[target_team].members[target_member];
            if (!IsMemberCurrent(session.teams[target_team], target)) continue;
            PacketGuard packet;
            packet.put_header(
                ENUM_PACKETCLASS_NOTIPACKET,
                ENUM_NOTIPACKET_DUAL_RAID_PARTY_GROUP_INFO);
            packet.put_short(session.wire_session_id);
            packet.put_byte(kOfficialTeamCount);
            for (int team = 0; team < kOfficialTeamCount; ++team) {
                packet.put_byte(team);
                packet.put_byte(kOfficialMemberCount);
                for (int member = 0; member < kOfficialMemberCount; ++member) {
                    packet.put_byte(
                        session.teams[team].members[member].roster_slot);
                    packet.put_short(
                        session.teams[team].members[member].unique_id);
                }
            }
            packet.finalize(true);
            target.user->Send(packet);
        }
    }
}

void BroadcastBossMove(const RaidSession& session, uint8_t team_index) {
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            const RaidMember& target = session.teams[team].members[member];
            if (!IsMemberCurrent(session.teams[team], target)) continue;
            PacketGuard packet;
            packet.put_header(
                ENUM_PACKETCLASS_NOTIPACKET,
                ENUM_NOTIPACKET_MOVE_DUAL_RAID_BOSS_DUNGEON);
            packet.put_byte(team_index);
            packet.finalize(true);
            target.user->Send(packet);
        }
    }
}

void BroadcastMapClear(const RaidSession& session,
                       uint8_t team_index,
                       uint8_t progress,
                       uint8_t clear_flag) {
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            const RaidMember& target = session.teams[team].members[member];
            if (!IsMemberCurrent(session.teams[team], target)) continue;
            PacketGuard packet;
            packet.put_header(
                ENUM_PACKETCLASS_NOTIPACKET,
                ENUM_NOTIPACKET_DUAL_RAID_DUNGEON_MAP_CLEAR);
            packet.put_byte(team_index);
            packet.put_byte(progress);
            packet.put_byte(clear_flag);
            packet.finalize(true);
            target.user->Send(packet);
        }
    }
}

void SendMoveToOppositeTeam(const RaidSession& session,
                            uint8_t moving_team,
                            uint8_t map_x,
                            uint8_t map_y) {
    const int opposite = moving_team == 0u ? 1 : 0;
    for (int member = 0; member < kOfficialMemberCount; ++member) {
        const RaidMember& target = session.teams[opposite].members[member];
        if (!IsMemberCurrent(session.teams[opposite], target)) continue;
        PacketGuard packet;
        packet.put_header(
            ENUM_PACKETCLASS_NOTIPACKET,
            ENUM_NOTIPACKET_DUAL_RAID_DUNGEON_MOVE_INFO);
        // The 0828 handler consumes this byte but updates its locally-derived
        // opposite group.  Sending only to the opposite roster is therefore
        // part of the wire contract; broadcasting would mirror the wrong map.
        packet.put_byte(moving_team);
        packet.put_byte(map_x);
        packet.put_byte(map_y);
        packet.finalize(true);
        target.user->Send(packet);
    }
}

std::map<SessionKey, RaidSession>::iterator FindSessionByPartyLocked(
    CParty* party) {
    std::map<CParty*, SessionKey>::const_iterator key =
        g_party_to_session.find(party);
    return key == g_party_to_session.end()
               ? g_raid_sessions.end() : g_raid_sessions.find(key->second);
}

std::map<SessionKey, RaidSession>::iterator FindSessionByUserLocked(
    CUser* user, ParticipantLocator*& locator) {
    locator = 0;
    std::map<CUser*, ParticipantLocator>::iterator found =
        g_user_to_session.find(user);
    if (found == g_user_to_session.end()) return g_raid_sessions.end();
    if (!IsCharacterIdentityCurrent(user, found->second.account_id,
                                    found->second.charac_no,
                                    found->second.unique_id)) {
        return g_raid_sessions.end();
    }
    std::map<SessionKey, RaidSession>::iterator session =
        g_raid_sessions.find(found->second.key);
    if (session == g_raid_sessions.end()) return session;
    locator = &found->second;
    return session;
}

void RemoveWaitingPartyLocked(CParty* party, bool notify) {
    for (std::list<WaitingTeam>::iterator it = g_waiting_teams.begin();
         it != g_waiting_teams.end();) {
        if (it->team.party != party) {
            ++it;
            continue;
        }
        if (notify) {
            for (int member = 0; member < kOfficialMemberCount; ++member) {
                if (IsMemberCurrent(it->team, it->team.members[member])) {
                    SendEmptyNotice(
                        it->team.members[member].user,
                        ENUM_NOTIPACKET_EXIT_DUAL_RAID_USER);
                }
            }
        }
        it = g_waiting_teams.erase(it);
    }
}

void EraseSessionLocked(std::map<SessionKey, RaidSession>::iterator session,
                        bool notify_exit) {
    if (session == g_raid_sessions.end()) return;
    if (notify_exit) {
        BroadcastEmptyNotice(session->second,
                             ENUM_NOTIPACKET_EXIT_DUAL_RAID_USER);
    }
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        g_party_to_session.erase(session->second.teams[team].party);
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            CUser* const user = session->second.teams[team].members[member].user;
            std::map<CUser*, ParticipantLocator>::iterator locator =
                g_user_to_session.find(user);
            if (locator != g_user_to_session.end() &&
                locator->second.key == session->first) {
                g_user_to_session.erase(locator);
            }
        }
    }
    g_raid_sessions.erase(session);
}

SessionKey MakeSessionKeyLocked(const RaidTeam& first,
                                const RaidTeam& second) {
    uint32_t high = NowEpoch();
    if (high == 0u) high = 1u;
    for (;;) {
        uint32_t low = NextNonZero(g_next_session_counter);
        low ^= static_cast<uint32_t>((first.server_group + 1) << 24);
        for (int index = 0; index < kOfficialMemberCount; ++index) {
            low = (low * 16777619u) ^ first.members[index].charac_no;
            low = (low * 16777619u) ^ second.members[index].charac_no;
        }
        if (low == 0u) low = 1u;
        const SessionKey candidate(high, low);
        if (g_raid_sessions.find(candidate) == g_raid_sessions.end()) {
            return candidate;
        }
    }
}

void RegisterSessionMembersLocked(RaidSession& session) {
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        session.teams[team].team_index = static_cast<uint8_t>(team);
        g_party_to_session[session.teams[team].party] = session.key;
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            const RaidMember& snapshot = session.teams[team].members[member];
            ParticipantLocator locator;
            locator.key = session.key;
            locator.team_index = static_cast<uint8_t>(team);
            locator.member_index = static_cast<uint8_t>(member);
            locator.account_id = snapshot.account_id;
            locator.charac_no = snapshot.charac_no;
            locator.unique_id = snapshot.unique_id;
            g_user_to_session[snapshot.user] = locator;
        }
    }
}

bool AllClaimsDelivered(const RaidSession& session) {
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            if (session.teams[team].members[member].claim_state !=
                CLAIM_DELIVERED) {
                return false;
            }
        }
    }
    return true;
}

uint32_t RewardExperience(const DualRaidPolicy& policy,
                          int level,
                          bool clear) {
    if (level < policy.minimum_level || level > policy.maximum_level) {
        return 0u;
    }
    const uint32_t base = policy.basic_exp[level];
    const uint32_t rate = static_cast<uint32_t>(
        clear ? policy.clear_bonus_rate : policy.fail_bonus_rate);
    const unsigned long long scaled =
        static_cast<unsigned long long>(base) * rate / 100ULL;
    return scaled > 1000000ULL ? 1000000u
                              : static_cast<uint32_t>(scaled);
}

bool FinalizeRaidLocked(RaidSession& session,
                        const DualRaidPolicy& policy) {
    if (session.phase == RAID_PHASE_FINALIZED ||
        session.winner_team >= kOfficialTeamCount ||
        !IsTeamCurrent(session.teams[0]) ||
        !IsTeamCurrent(session.teams[1])) {
        return false;
    }
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        const bool clear = team == session.winner_team;
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            RaidMember& target = session.teams[team].members[member];
            target.clear_result = clear ? 1u : 0u;
            target.reward_item_id = static_cast<uint32_t>(
                clear ? policy.clear_item_id : policy.fail_item_id);
            target.reward_item_count = static_cast<uint32_t>(
                clear ? policy.clear_item_count : policy.fail_item_count);
            target.reward_exp = RewardExperience(
                policy, target.user->get_charac_level(), clear);
            target.claim_state = CLAIM_IDLE;
            target.next_retry_epoch = 0u;

            int gained_sp = 0;
            int gained_sfp = 0;
            if (target.reward_exp != 0u) {
                (void)target.user->gain_exp_sp(
                    static_cast<int>(target.reward_exp), gained_sp, gained_sfp,
                    static_cast<eExpAddReason>(2), 0, false);
            }

            PacketGuard packet;
            packet.put_header(
                ENUM_PACKETCLASS_NOTIPACKET,
                ENUM_NOTIPACKET_DUAL_RAID_DUNGEON_RESULT_REWARD);
            packet.put_byte(session.winner_team);
            packet.put_int(static_cast<int>(target.reward_exp));
            packet.finalize(true);
            target.user->Send(packet);
        }
    }
    session.phase = RAID_PHASE_FINALIZED;
    session.finalized_epoch = NowEpoch();
    return true;
}

bool SubmitClaim(CUser* user,
                 const dual_raid::RewardClaimMessage& request) {
    return user && dual_raid::DBDualRaidReward::QueueRequest(
                       user->GetUID(), request);
}

}  // namespace

namespace {

bool WaitingContainsPartyLocked(CParty* party) {
    for (std::list<WaitingTeam>::const_iterator it = g_waiting_teams.begin();
         it != g_waiting_teams.end(); ++it) {
        if (it->team.party == party) return true;
    }
    return false;
}

bool TeamMembersAreUnownedLocked(const RaidTeam& team) {
    for (int member = 0; member < kOfficialMemberCount; ++member) {
        if (g_user_to_session.find(team.members[member].user) !=
            g_user_to_session.end()) {
            return false;
        }
    }
    return true;
}

SessionKey JoinOrQueueLocked(const RaidTeam& incoming) {
    std::map<CParty*, SessionKey>::const_iterator existing =
        g_party_to_session.find(incoming.party);
    if (existing != g_party_to_session.end()) return existing->second;
    if (WaitingContainsPartyLocked(incoming.party)) return SessionKey();
    if (!TeamMembersAreUnownedLocked(incoming)) return SessionKey();

    const uint32_t now = NowEpoch();
    for (std::list<WaitingTeam>::iterator it = g_waiting_teams.begin();
         it != g_waiting_teams.end();) {
        if (!IsTeamCurrent(it->team) ||
            (now != 0u && it->queued_epoch != 0u &&
             now - it->queued_epoch > kWaitingLifetimeSeconds)) {
            for (int member = 0; member < kOfficialMemberCount; ++member) {
                if (IsMemberCurrent(it->team, it->team.members[member])) {
                    SendEmptyNotice(
                        it->team.members[member].user,
                        ENUM_NOTIPACKET_EXIT_DUAL_RAID_USER);
                }
            }
            it = g_waiting_teams.erase(it);
            continue;
        }
        ++it;
    }

    for (std::list<WaitingTeam>::iterator it = g_waiting_teams.begin();
         it != g_waiting_teams.end(); ++it) {
        if (it->team.party == incoming.party ||
            it->team.server_group != incoming.server_group ||
            !TeamMembersAreUnownedLocked(it->team)) {
            continue;
        }
        RaidSession session;
        session.teams[0] = it->team;
        session.teams[1] = incoming;
        session.server_group = incoming.server_group;
        session.key = MakeSessionKeyLocked(session.teams[0], session.teams[1]);
        session.wire_session_id = NextWireSessionIdLocked();
        session.created_epoch = now;
        const SessionKey key = session.key;
        std::pair<std::map<SessionKey, RaidSession>::iterator, bool> inserted =
            g_raid_sessions.insert(std::make_pair(key, session));
        if (!inserted.second) return SessionKey();
        RegisterSessionMembersLocked(inserted.first->second);
        g_waiting_teams.erase(it);
        return key;
    }

    WaitingTeam waiting;
    waiting.team = incoming;
    waiting.queued_epoch = now;
    g_waiting_teams.push_back(waiting);
    return SessionKey();
}

void HandleJoin(CUser* user) {
    const DualRaidPolicy* const policy = PolicyCache().get();
    if (!policy) {
        SendJoinError(user, 67);
        return;
    }
    if (!user || user->get_state() != 3 || !user->getCurCharacR()) {
        SendJoinError(user, 19);
        return;
    }
    CParty* const party = user->GetParty();
    if (!party || party->getManager() != user) {
        SendJoinError(user, 8);
        return;
    }
    RaidTeam team;
    unsigned char error = 8;
    if (!CaptureTeam(party, team, error)) {
        SendJoinError(user, error);
        return;
    }
    for (int member = 0; member < kOfficialMemberCount; ++member) {
        if (team.members[member].user->get_state() != 3) {
            SendJoinError(user, 19);
            return;
        }
    }

    SessionKey session_key;
    bool rejected = false;
    {
        MutexGuard guard(&g_dual_raid_mutex);
        std::map<CParty*, SessionKey>::const_iterator existing =
            g_party_to_session.find(party);
        if (existing != g_party_to_session.end()) {
            session_key = existing->second;
        } else if (WaitingContainsPartyLocked(party)) {
            session_key = SessionKey();
        } else if (!TeamMembersAreUnownedLocked(team)) {
            rejected = true;
        } else {
            session_key = JoinOrQueueLocked(team);
        }
    }
    if (rejected) {
        SendJoinError(user, 19);
        return;
    }

    // CMD514 success must precede NOTI453 for the newly matched captain: its
    // handler installs DualRaid client state 1, which owns the roster object.
    SendJoinSuccess(user);
    if (session_key.hi != 0u && session_key.lo != 0u) {
        MutexGuard guard(&g_dual_raid_mutex);
        std::map<SessionKey, RaidSession>::const_iterator session =
            g_raid_sessions.find(session_key);
        if (session != g_raid_sessions.end()) SendPartyGroupInfo(session->second);
    }
}

void HandleReady(CUser* user) {
    MutexGuard guard(&g_dual_raid_mutex);
    ParticipantLocator* locator = 0;
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByUserLocked(user, locator);
    if (session == g_raid_sessions.end() || !locator ||
        locator->member_index != 0u || locator->team_index >= kOfficialTeamCount ||
        session->second.teams[locator->team_index].members[0].user != user ||
        session->second.teams[locator->team_index].party->getManager() != user ||
        session->second.phase > RAID_PHASE_READY ||
        !IsTeamCurrent(session->second.teams[0]) ||
        !IsTeamCurrent(session->second.teams[1])) {
        TraceFailure("invalid ready owner/state", user);
        return;
    }
    session->second.teams[locator->team_index].ready = true;
    if (session->second.teams[0].ready && session->second.teams[1].ready) {
        session->second.phase = RAID_PHASE_READY;
        if (!session->second.ready_notified) {
            session->second.ready_notified = true;
            BroadcastEmptyNotice(
                session->second,
                ENUM_NOTIPACKET_DUAL_RAID_DUNGEON_READY_COMPLETE);
        }
    }
}

void HandleLeave(CUser* user) {
    if (!user) return;
    MutexGuard guard(&g_dual_raid_mutex);
    CParty* const party = user->GetParty();
    if (party) RemoveWaitingPartyLocked(party, true);
    ParticipantLocator* locator = 0;
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByUserLocked(user, locator);
    if (session != g_raid_sessions.end()) EraseSessionLocked(session, true);
}

void HandleStart(CUser* user) {
    const DualRaidPolicy* const policy = PolicyCache().get();
    if (!policy || !user) return;
    SessionKey key;
    CParty* parties[kOfficialTeamCount] = {0, 0};
    {
        MutexGuard guard(&g_dual_raid_mutex);
        ParticipantLocator* locator = 0;
        std::map<SessionKey, RaidSession>::iterator session =
            FindSessionByUserLocked(user, locator);
        if (session == g_raid_sessions.end() || !locator ||
            locator->member_index != 0u ||
            session->second.phase < RAID_PHASE_READY ||
            session->second.phase > RAID_PHASE_NORMAL_STARTING ||
            !session->second.teams[0].ready ||
            !session->second.teams[1].ready ||
            !IsTeamCurrent(session->second.teams[0]) ||
            !IsTeamCurrent(session->second.teams[1])) {
            TraceFailure("invalid start owner/state", user);
            return;
        }
        RaidTeam& team = session->second.teams[locator->team_index];
        if (team.start_requested) return;
        team.start_requested = true;
        if (!session->second.teams[0].start_requested ||
            !session->second.teams[1].start_requested) {
            return;
        }
        session->second.phase = RAID_PHASE_NORMAL_STARTING;
        key = session->first;
        parties[0] = session->second.teams[0].party;
        parties[1] = session->second.teams[1].party;
    }

    int results[kOfficialTeamCount] = {-1, -1};
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        results[team] = parties[team]->dungeon_start(
            policy->normal_dungeon,
            static_cast<char>(policy->normal_difficulty),
            static_cast<ENUM_DUNGEON_TYPE>(0));
    }

    bool rollback = false;
    {
        MutexGuard guard(&g_dual_raid_mutex);
        std::map<SessionKey, RaidSession>::iterator session =
            g_raid_sessions.find(key);
        if (session == g_raid_sessions.end() ||
            session->second.teams[0].party != parties[0] ||
            session->second.teams[1].party != parties[1]) {
            rollback = true;
        } else if (results[0] != 0 || results[1] != 0) {
            TraceFailure("ordinary normal-dungeon start failed", user);
            EraseSessionLocked(session, true);
            rollback = true;
        } else {
            session->second.teams[0].start_succeeded = true;
            session->second.teams[1].start_succeeded = true;
            session->second.phase = RAID_PHASE_NORMAL_RUNNING;
            BroadcastEmptyNotice(
                session->second,
                ENUM_NOTIPACKET_DUAL_RAID_DUNGEON_START);
        }
    }

    if (!rollback) return;
    for (int team = 0; team < kOfficialTeamCount; ++team) {
        if (results[team] == 0 && parties[team] &&
            parties[team]->getManager()) {
            parties[team]->giveup_game(
                parties[team]->getManager(), false, false, true);
        }
    }
}

bool BuildClaimLocked(CUser* user,
                      uint8_t requested_team,
                      dual_raid::RewardClaimMessage& request) {
    ParticipantLocator* locator = 0;
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByUserLocked(user, locator);
    if (session == g_raid_sessions.end() || !locator ||
        session->second.phase != RAID_PHASE_FINALIZED ||
        requested_team != locator->team_index ||
        locator->team_index >= kOfficialTeamCount ||
        locator->member_index >= kOfficialMemberCount) {
        return false;
    }
    RaidMember& member = session->second.teams[locator->team_index]
                                      .members[locator->member_index];
    if (!IsMemberCurrent(session->second.teams[locator->team_index], member) ||
        member.claim_state == CLAIM_DELIVERED ||
        member.reward_item_id == 0u || member.reward_item_count == 0u) {
        return false;
    }
    const uint32_t now = NowEpoch();
    if (member.claim_state == CLAIM_QUEUED &&
        (member.next_retry_epoch == 0u || now < member.next_retry_epoch)) {
        return false;
    }

    std::memset(&request, 0, sizeof(request));
    request.account_id = member.account_id;
    request.charac_no = member.charac_no;
    request.raid_token_hi = session->first.hi;
    request.raid_token_lo = session->first.lo;
    request.request_token = NextNonZero(g_next_request_token);
    request.reward_item_id = member.reward_item_id;
    request.reward_item_count = member.reward_item_count;
    request.reward_exp = member.reward_exp;
    request.server_group = session->second.server_group;
    request.reply_user_idx = user->GetUID();
    request.operation = dual_raid::REWARD_OPERATION_CLAIM;
    request.team_index = locator->team_index;
    request.clear_result = member.clear_result;
    member.claim_message = request;
    member.claim_state = CLAIM_QUEUED;
    member.next_retry_epoch = now == 0u ? kClaimRetrySeconds
                                       : now + kClaimRetrySeconds;
    return true;
}

void MarkClaimQueueFailure(const dual_raid::RewardClaimMessage& request) {
    MutexGuard guard(&g_dual_raid_mutex);
    const SessionKey key(request.raid_token_hi, request.raid_token_lo);
    std::map<SessionKey, RaidSession>::iterator session =
        g_raid_sessions.find(key);
    if (session == g_raid_sessions.end() ||
        request.team_index >= kOfficialTeamCount) return;
    RaidTeam& team = session->second.teams[request.team_index];
    for (int member = 0; member < kOfficialMemberCount; ++member) {
        if (team.members[member].charac_no == request.charac_no &&
            team.members[member].claim_message.request_token ==
                request.request_token &&
            team.members[member].claim_state != CLAIM_DELIVERED) {
            team.members[member].claim_state = CLAIM_RETRY;
            const uint32_t now = NowEpoch();
            team.members[member].next_retry_epoch =
                now == 0u ? kClaimRetrySeconds : now + kClaimRetrySeconds;
            return;
        }
    }
}

void HandleReward(CUser* user, uint8_t requested_team) {
    if (!PolicyCache().get()) return;
    dual_raid::RewardClaimMessage request;
    {
        MutexGuard guard(&g_dual_raid_mutex);
        if (!BuildClaimLocked(user, requested_team, request)) {
            TraceFailure("invalid or duplicate reward selection", user);
            return;
        }
    }
    if (!SubmitClaim(user, request)) {
        MarkClaimQueueFailure(request);
        TraceFailure("reward DB queue failed", user);
    }
}

}  // namespace

namespace dual_raid {

bool IsPolicyLoaded() { return PolicyCache().get() != 0; }

unsigned int ScaleMonsterExperience(CParty* party,
                                    CUser* user,
                                    unsigned int experience) {
    const DualRaidPolicy* const policy = PolicyCache().get();
    if (!party || !user || !policy || experience == 0u) return experience;
    MutexGuard guard(&g_dual_raid_mutex);
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByPartyLocked(party);
    if (session == g_raid_sessions.end() ||
        session->second.phase < RAID_PHASE_NORMAL_STARTING ||
        session->second.phase > RAID_PHASE_BOSS_RUNNING) {
        return experience;
    }
    const uint8_t team = session->second.teams[0].party == party ? 0u : 1u;
    const RaidTeam& raid_team = session->second.teams[team];
    bool participant = false;
    for (int member = 0; member < kOfficialMemberCount; ++member) {
        if (raid_team.members[member].user == user &&
            IsMemberCurrent(raid_team, raid_team.members[member])) {
            participant = true;
            break;
        }
    }
    if (!participant) return experience;

    const int level = user->get_charac_level();
    for (int bracket = 0; bracket < 3; ++bracket) {
        if (level < policy->kill_exp_min_level[bracket] ||
            level > policy->kill_exp_max_level[bracket]) {
            continue;
        }
        const unsigned long long scaled =
            static_cast<unsigned long long>(experience) *
            policy->kill_exp_rate_permyriad[bracket] / 10000ULL;
        return scaled > 0xffffffffULL
                   ? 0xffffffffu : static_cast<unsigned int>(scaled);
    }
    return experience;
}

void OnPeriodicTick(uint32_t now_epoch) {
    struct Retry {
        Retry() : user(0), message() {
            std::memset(&message, 0, sizeof(message));
        }
        CUser* user;
        RewardClaimMessage message;
    };
    std::vector<Retry> retries;
    {
        MutexGuard guard(&g_dual_raid_mutex);
        for (std::list<WaitingTeam>::iterator it = g_waiting_teams.begin();
             it != g_waiting_teams.end();) {
            const bool expired = now_epoch != 0u && it->queued_epoch != 0u &&
                now_epoch - it->queued_epoch > kWaitingLifetimeSeconds;
            if (!expired && IsTeamCurrent(it->team)) {
                ++it;
                continue;
            }
            for (int member = 0; member < kOfficialMemberCount; ++member) {
                if (IsMemberCurrent(it->team, it->team.members[member])) {
                    SendEmptyNotice(it->team.members[member].user,
                                    ENUM_NOTIPACKET_EXIT_DUAL_RAID_USER);
                }
            }
            it = g_waiting_teams.erase(it);
        }

        for (std::map<SessionKey, RaidSession>::iterator it =
                 g_raid_sessions.begin(); it != g_raid_sessions.end();) {
            std::map<SessionKey, RaidSession>::iterator current = it++;
            RaidSession& session = current->second;
            const bool invalid_live = session.phase != RAID_PHASE_FINALIZED &&
                (!IsTeamCurrent(session.teams[0]) ||
                 !IsTeamCurrent(session.teams[1]));
            const bool raid_expired = now_epoch != 0u &&
                session.created_epoch != 0u &&
                now_epoch - session.created_epoch > kRaidLifetimeSeconds;
            const bool final_expired =
                session.phase == RAID_PHASE_FINALIZED && now_epoch != 0u &&
                session.finalized_epoch != 0u &&
                now_epoch - session.finalized_epoch >
                    kFinalizedLifetimeSeconds;
            if (invalid_live || raid_expired || final_expired) {
                EraseSessionLocked(current, true);
                continue;
            }
            if (session.phase != RAID_PHASE_FINALIZED) continue;
            for (int team = 0; team < kOfficialTeamCount; ++team) {
                for (int member = 0; member < kOfficialMemberCount; ++member) {
                    RaidMember& claim = session.teams[team].members[member];
                    if ((claim.claim_state != CLAIM_QUEUED &&
                         claim.claim_state != CLAIM_RETRY) ||
                        now_epoch == 0u || claim.next_retry_epoch == 0u ||
                        now_epoch < claim.next_retry_epoch ||
                        !IsMemberCurrent(session.teams[team], claim)) {
                        continue;
                    }
                    Retry retry;
                    retry.user = claim.user;
                    retry.message = claim.claim_message;
                    retry.message.reply_user_idx = claim.user->GetUID();
                    claim.claim_message.reply_user_idx =
                        retry.message.reply_user_idx;
                    claim.claim_state = CLAIM_QUEUED;
                    claim.next_retry_epoch = now_epoch + kClaimRetrySeconds;
                    retries.push_back(retry);
                }
            }
        }
    }
    for (size_t index = 0u; index < retries.size(); ++index) {
        if (!SubmitClaim(retries[index].user, retries[index].message)) {
            MarkClaimQueueFailure(retries[index].message);
        }
    }
}

void OnUserDisconnect(CUser* user) {
    if (!user) return;
    MutexGuard guard(&g_dual_raid_mutex);
    for (std::list<WaitingTeam>::iterator it = g_waiting_teams.begin();
         it != g_waiting_teams.end(); ++it) {
        bool found = false;
        for (int member = 0; member < kOfficialMemberCount; ++member) {
            if (it->team.members[member].user == user) found = true;
        }
        if (found) {
            CParty* const party = it->team.party;
            RemoveWaitingPartyLocked(party, true);
            break;
        }
    }
    ParticipantLocator* locator = 0;
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByUserLocked(user, locator);
    if (session == g_raid_sessions.end() || !locator) return;
    if (session->second.phase != RAID_PHASE_FINALIZED) {
        EraseSessionLocked(session, true);
        return;
    }
    RaidMember& member = session->second.teams[locator->team_index]
                                      .members[locator->member_index];
    member.user = 0;
    g_user_to_session.erase(user);
}

void OnPartyMemberLeave(CParty* party, CUser* user) {
    if (!party) return;
    MutexGuard guard(&g_dual_raid_mutex);
    RemoveWaitingPartyLocked(party, true);
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByPartyLocked(party);
    if (session == g_raid_sessions.end()) return;
    if (session->second.phase != RAID_PHASE_FINALIZED) {
        EraseSessionLocked(session, true);
        return;
    }

    // A result-phase party mutation must not revoke the opposite roster's
    // already-earned entitlement.  Detach only the leaving identity; queued
    // DB work remains crash-idempotent and the remaining five users can still
    // submit CMD528.  A null user is the party-destroy hook and detaches every
    // still-associated member of this one party without erasing the peer team.
    const uint8_t team = session->second.teams[0].party == party ? 0u : 1u;
    RaidTeam& leaving_team = session->second.teams[team];
    for (int member_index = 0; member_index < kOfficialMemberCount;
         ++member_index) {
        RaidMember& member = leaving_team.members[member_index];
        if (user && member.user != user) continue;
        if (member.user) {
            g_user_to_session.erase(member.user);
            if (user) {
                SendEmptyNotice(member.user,
                                ENUM_NOTIPACKET_EXIT_DUAL_RAID_USER);
            }
        }
        member.user = 0;
    }
    if (!user) {
        g_party_to_session.erase(party);
        leaving_team.party = 0;
    }
}

void OnPartyDestroyed(CParty* party) {
    OnPartyMemberLeave(party, 0);
}

void OnPartyMoveMap(CParty* party,
                    unsigned int map_x,
                    unsigned int map_y) {
    if (!party) return;
    MutexGuard guard(&g_dual_raid_mutex);
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByPartyLocked(party);
    if (session == g_raid_sessions.end() ||
        session->second.phase < RAID_PHASE_NORMAL_RUNNING ||
        session->second.phase > RAID_PHASE_BOSS_RUNNING) return;
    const uint8_t team = session->second.teams[0].party == party ? 0u : 1u;
    RaidTeam& moving = session->second.teams[team];
    // Once one roster has won the normal leg, its subsequent movement belongs
    // to dungeon 3050 and must not be reflected as normal-map progress.  The
    // opposite roster may still finish dungeon 3049 while the winner is in the
    // boss leg, so the session-wide boss phase alone is not a valid gate.
    if (moving.normal_finished) return;
    const uint8_t bounded_x =
        static_cast<uint8_t>(std::min(map_x, 255u));
    const uint8_t bounded_y =
        static_cast<uint8_t>(std::min(map_y, 255u));
    if (moving.map_notice_valid && moving.map_x == bounded_x &&
        moving.map_y == bounded_y) {
        return;
    }
    moving.map_x = bounded_x;
    moving.map_y = bounded_y;
    moving.map_notice_valid = true;
    SendMoveToOppositeTeam(session->second, team, moving.map_x, moving.map_y);
}

void OnPartyMapClear(CParty* party,
                     unsigned int progress,
                     bool clear_flag) {
    if (!party) return;
    MutexGuard guard(&g_dual_raid_mutex);
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByPartyLocked(party);
    if (session == g_raid_sessions.end() ||
        session->second.phase < RAID_PHASE_NORMAL_RUNNING ||
        session->second.phase > RAID_PHASE_BOSS_RUNNING) return;
    const uint8_t team = session->second.teams[0].party == party ? 0u : 1u;
    RaidTeam& state = session->second.teams[team];
    if (state.normal_finished) return;
    const uint8_t bounded =
        static_cast<uint8_t>(std::min(progress, 255u));
    const uint8_t wire_clear = clear_flag ? 1u : 0u;
    if (state.map_clear_notice_valid && bounded < state.progress) return;
    if (state.map_clear_notice_valid && state.progress == bounded &&
        state.map_clear_flag == wire_clear) {
        return;
    }
    state.progress = bounded;
    state.map_clear_notice_valid = true;
    state.map_clear_flag = wire_clear;
    BroadcastMapClear(session->second, team, bounded, wire_clear);
}

bool OnPartyDungeonResult(CParty* party,
                          int dungeon_index,
                          bool cleared) {
    const DualRaidPolicy* const policy = PolicyCache().get();
    if (!party || !policy) return false;
    SessionKey key;
    CParty* boss_party = 0;
    bool start_boss = false;
    bool owned_result = false;
    {
        MutexGuard guard(&g_dual_raid_mutex);
        std::map<SessionKey, RaidSession>::iterator session =
            FindSessionByPartyLocked(party);
        if (session == g_raid_sessions.end()) return false;
        key = session->first;
        const uint8_t team =
            session->second.teams[0].party == party ? 0u : 1u;

        if (dungeon_index == policy->normal_dungeon) {
            owned_result = true;
            if (session->second.phase < RAID_PHASE_NORMAL_RUNNING ||
                session->second.phase > RAID_PHASE_BOSS_RUNNING ||
                session->second.teams[team].normal_finished) return true;
            session->second.teams[team].normal_finished = true;
            session->second.teams[team].normal_cleared = cleared;
            if (cleared && session->second.winner_team == 0xffu) {
                session->second.winner_team = team;
                session->second.phase = RAID_PHASE_BOSS_PENDING;
                BroadcastBossMove(session->second, team);
                boss_party = party;
                start_boss = true;
            } else if (!cleared &&
                       session->second.teams[0].normal_finished &&
                       session->second.teams[1].normal_finished &&
                       !session->second.teams[0].normal_cleared &&
                       !session->second.teams[1].normal_cleared) {
                EraseSessionLocked(session, true);
            }
        } else if (dungeon_index == policy->boss_dungeon) {
            owned_result = true;
            if (session->second.winner_team != team ||
                (session->second.phase != RAID_PHASE_BOSS_PENDING &&
                 session->second.phase != RAID_PHASE_BOSS_RUNNING)) {
                return true;
            }
            if (!cleared) {
                EraseSessionLocked(session, true);
                return true;
            }
            if (!FinalizeRaidLocked(session->second, *policy)) {
                EraseSessionLocked(session, true);
            }
        } else {
            return false;
        }
    }

    if (!start_boss || !boss_party) return owned_result;
    const int result = boss_party->dungeon_start(
        policy->boss_dungeon,
        static_cast<char>(policy->boss_difficulty),
        static_cast<ENUM_DUNGEON_TYPE>(0));
    MutexGuard guard(&g_dual_raid_mutex);
    std::map<SessionKey, RaidSession>::iterator session =
        g_raid_sessions.find(key);
    if (session == g_raid_sessions.end()) return true;
    if (result != 0) {
        EraseSessionLocked(session, true);
        return true;
    }
    session->second.teams[session->second.winner_team].boss_started = true;
    session->second.phase = RAID_PHASE_BOSS_RUNNING;
    return true;
}

void OnRewardClaimResult(CUser* user,
                         const RewardClaimMessage& response) {
    if (!user || response.operation != REWARD_OPERATION_CLAIM ||
        response.team_index >= kOfficialTeamCount) return;
    MutexGuard guard(&g_dual_raid_mutex);
    ParticipantLocator* locator = 0;
    std::map<SessionKey, RaidSession>::iterator session =
        FindSessionByUserLocked(user, locator);
    if (session == g_raid_sessions.end() || !locator ||
        session->first.hi != response.raid_token_hi ||
        session->first.lo != response.raid_token_lo ||
        locator->team_index != response.team_index ||
        locator->member_index >= kOfficialMemberCount) return;
    RaidMember& member = session->second.teams[locator->team_index]
                                      .members[locator->member_index];
    const RewardClaimMessage& request = member.claim_message;
    if (request.request_token != response.request_token ||
        request.account_id != response.account_id ||
        request.charac_no != response.charac_no ||
        request.reward_item_id != response.reward_item_id ||
        request.reward_item_count != response.reward_item_count ||
        request.reward_exp != response.reward_exp ||
        request.clear_result != response.clear_result ||
        request.server_group != response.server_group) {
        return;
    }
    if (response.result_code == REWARD_RESULT_SUCCESS ||
        response.result_code == REWARD_RESULT_ALREADY_DELIVERED) {
        member.claim_state = CLAIM_DELIVERED;
        member.next_retry_epoch = 0u;
        if (AllClaimsDelivered(session->second)) {
            EraseSessionLocked(session, true);
        }
    } else {
        member.claim_state = CLAIM_RETRY;
        const uint32_t now = NowEpoch();
        member.next_retry_epoch =
            now == 0u ? kClaimRetrySeconds : now + kClaimRetrySeconds;
    }
}

}  // namespace dual_raid_0828

Dispatcher_DualRaidJoin::Dispatcher_DualRaidJoin()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_DualRaidJoin::~Dispatcher_DualRaidJoin() {}
int Dispatcher_DualRaidJoin::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_5DA880 finalizes CMD514 without a writer.
    (void)packet;
    HandleJoin(user);
    return 0;
}
int Dispatcher_DualRaidJoin::read(PacketBuf& packet, MSG_BASE& message) {
    return NoRead(packet, message);
}
int Dispatcher_DualRaidJoin::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    return NoProcess(user, message, param);
}

Dispatcher_DualRaidCommand::Dispatcher_DualRaidCommand()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_DualRaidCommand::~Dispatcher_DualRaidCommand() {}
int Dispatcher_DualRaidCommand::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    uint8_t action = 0xffu;
    // CLIENT_IDA_EVIDENCE: sub_5DA8A0 writes 0; sub_5DA8D0 writes 1.
    if (packet.get_byte(action) != 1 || action > 1u) {
        TraceFailure("malformed CMD515", user);
        return 0;
    }
    if (action == 0u) HandleStart(user);
    else HandleLeave(user);
    return 0;
}
int Dispatcher_DualRaidCommand::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    return NoRead(packet, message);
}
int Dispatcher_DualRaidCommand::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    return NoProcess(user, message, param);
}

Dispatcher_DualRaidReady::Dispatcher_DualRaidReady()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_DualRaidReady::~Dispatcher_DualRaidReady() {}
int Dispatcher_DualRaidReady::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_5DA9F0 finalizes CMD527 without a writer.
    (void)packet;
    HandleReady(user);
    return 0;
}
int Dispatcher_DualRaidReady::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    return NoRead(packet, message);
}
int Dispatcher_DualRaidReady::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    return NoProcess(user, message, param);
}

Dispatcher_DualRaidReward::Dispatcher_DualRaidReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_DualRaidReward::~Dispatcher_DualRaidReward() {}
int Dispatcher_DualRaidReward::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    uint8_t team = 0xffu;
    // CLIENT_IDA_EVIDENCE: sub_5DAA50 writes exactly one selected-team byte.
    if (packet.get_byte(team) == 1 && team < kOfficialTeamCount) {
        HandleReward(user, team);
    } else {
        TraceFailure("malformed CMD528", user);
    }
    return 0;
}
int Dispatcher_DualRaidReward::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    return NoRead(packet, message);
}
int Dispatcher_DualRaidReward::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    return NoProcess(user, message, param);
}

InterDualRaidReward::InterDualRaidReward()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              dual_raid::kTaiwanInnerCode)) {}
InterDualRaidReward::~InterDualRaidReward() {}
int InterDualRaidReward::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    if (!payload || payload_size != static_cast<int>(
            sizeof(dual_raid::RewardClaimMessage))) {
        return 0;
    }
    const dual_raid::RewardClaimMessage response =
        *reinterpret_cast<const dual_raid::RewardClaimMessage*>(payload);
    dual_raid::OnRewardClaimResult(user, response);
    return 0;
}
