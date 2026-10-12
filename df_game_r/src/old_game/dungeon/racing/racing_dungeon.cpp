#include "old_game/dungeon/racing/racing_dungeon.h"

#include "old_game/dungeon/racing/racing_dungeon_types.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/game_manager.h"
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
#include "old_data/db/event/db_racing_dungeon.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/world/region/game_world.h"

namespace {

enum {
    kTownState = 3,
    kInventoryType = 1,
    kQueueGraceSeconds = 10,
    kQueueLifetimeSeconds = 300,
    kPendingRetrySeconds = 10,
    kRaceLifetimeSeconds = 30 * 60,
    kForcedReturnSeconds = 120,
    kInvalidWireRank = 0xff,

    // Every value used here has a concrete branch in the 0828 CMD523/524/526
    // client handlers.  Unsupported errors are never invented on the wire.
    kErrorGeneric = 1,
    kErrorParty = 18,
    kErrorPending = 19,
    kErrorItem = 21,
    kErrorMoney = 22,
    kErrorFatigue = 60,
    kErrorLevel = 95,
    kErrorEntryLimit = 191,

    kRacingMoneySubReason = 4,
    kRacingItemDelReason = 4,
    kRollbackMoneyAddReason = 12,
    kRollbackItemAddReason = 12,
};

enum RacePhase {
    RACE_WAITING_DUNGEON = 1,
    RACE_LOADING = 2,
    RACE_RUNNING = 3,
    RACE_RESULTS = 4,
};

enum LocalClaimState {
    LOCAL_CLAIM_NONE = 0,
    LOCAL_CLAIM_PENDING = 1,
    LOCAL_CLAIM_DURABLE = 2,
};

struct EntryCondition {
    EntryCondition() : money(0), item_id(0), item_count(0) {}
    int money;
    int item_id;
    int item_count;
};

struct RacingConfig {
    RacingConfig()
        : minimum_members(0), level_upper(0), level_lower(0),
          required_fatigue(0), entry_conditions() {
        std::memset(reward_item_ids, 0, sizeof(reward_item_ids));
        std::memset(reward_counts, 0, sizeof(reward_counts));
    }

    int minimum_members;
    int level_upper;
    int level_lower;
    int reward_item_ids[racing_dungeon::kMaximumRacers];
    int reward_counts[racing_dungeon::kMaximumRacers];
    int required_fatigue;
    std::vector<EntryCondition> entry_conditions;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }
private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

class RacingConfigCache {
public:
    RacingConfigCache() : attempted_(false), valid_(false), config_() {}

    const RacingConfig* Get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = Load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    enum ParseField {
        FIELD_NONE = 0,
        FIELD_MEMBER_COUNT,
        FIELD_LEVEL_UPPER,
        FIELD_LEVEL_LOWER,
        FIELD_REWARD_ITEM,
        FIELD_REWARD_COUNT,
        FIELD_FATIGUE,
        FIELD_ENTER,
        FIELD_RECOMMEND_CHANNEL,
    };

    bool Load() {
        if (!loadRDARScriptFile("Event/RacingDungeon/",
                                "RacingDungeon.info")) {
            return false;
        }
        RdarUnloadGuard unload;
        RacingConfig parsed;
        ParseField field = FIELD_NONE;
        std::vector<int> reward_items;
        std::vector<int> reward_counts;
        std::vector<int> enter_values;
        unsigned int single_mask = 0u;
        bool enter_closed = false;
        bool recommend_closed = false;
        bool malformed = false;

        for (;;) {
            int integer_value = 0;
            float float_value = 0.0f;
            std::string text;
            const DNFLEX_TOKEN token =
                ScanAll(&integer_value, &float_value, &text);
            if (token == DNF_TOK_EOF) break;
            if (token == DNF_TOK_TYPE) {
                if (field == FIELD_ENTER && text == "[/enter condition]") {
                    field = FIELD_NONE;
                    enter_closed = true;
                    continue;
                }
                if (field == FIELD_RECOMMEND_CHANNEL &&
                    text == "[/recommand channel]") {
                    field = FIELD_NONE;
                    recommend_closed = true;
                    continue;
                }
                if (field == FIELD_REWARD_ITEM && reward_items.size() == 4u) {
                    field = FIELD_NONE;
                } else if (field == FIELD_REWARD_COUNT &&
                           reward_counts.size() == 4u) {
                    field = FIELD_NONE;
                }
                if (field != FIELD_NONE) {
                    malformed = true;
                    continue;
                }
                if (text == "[dungeon start condition member count]") {
                    field = FIELD_MEMBER_COUNT;
                } else if (text == "[enter condition level upper]") {
                    field = FIELD_LEVEL_UPPER;
                } else if (text == "[enter condition level lower]") {
                    field = FIELD_LEVEL_LOWER;
                } else if (text == "[reward item index]") {
                    field = FIELD_REWARD_ITEM;
                } else if (text == "[reward item count]") {
                    field = FIELD_REWARD_COUNT;
                } else if (text == "[require fatigue]") {
                    field = FIELD_FATIGUE;
                } else if (text == "[enter condition]") {
                    field = FIELD_ENTER;
                } else if (text == "[recommand channel]") {
                    field = FIELD_RECOMMEND_CHANNEL;
                } else {
                    malformed = true;
                }
                continue;
            }
            if (token != DNF_TOK_DECIMAL && token != DNF_TOK_HEXADECIMAL) {
                malformed = true;
                continue;
            }
            switch (field) {
                case FIELD_MEMBER_COUNT:
                    if (single_mask & 1u) malformed = true;
                    parsed.minimum_members = integer_value;
                    single_mask |= 1u;
                    field = FIELD_NONE;
                    break;
                case FIELD_LEVEL_UPPER:
                    if (single_mask & 2u) malformed = true;
                    parsed.level_upper = integer_value;
                    single_mask |= 2u;
                    field = FIELD_NONE;
                    break;
                case FIELD_LEVEL_LOWER:
                    if (single_mask & 4u) malformed = true;
                    parsed.level_lower = integer_value;
                    single_mask |= 4u;
                    field = FIELD_NONE;
                    break;
                case FIELD_REWARD_ITEM:
                    reward_items.push_back(integer_value);
                    if (reward_items.size() > 4u) malformed = true;
                    break;
                case FIELD_REWARD_COUNT:
                    reward_counts.push_back(integer_value);
                    if (reward_counts.size() > 4u) malformed = true;
                    break;
                case FIELD_FATIGUE:
                    if (single_mask & 8u) malformed = true;
                    parsed.required_fatigue = integer_value;
                    single_mask |= 8u;
                    field = FIELD_NONE;
                    break;
                case FIELD_ENTER:
                    enter_values.push_back(integer_value);
                    break;
                default:
                    malformed = true;
                    break;
            }
        }

        if (field == FIELD_REWARD_ITEM && reward_items.size() == 4u) {
            field = FIELD_NONE;
        } else if (field == FIELD_REWARD_COUNT &&
                   reward_counts.size() == 4u) {
            field = FIELD_NONE;
        }
        if (malformed || field != FIELD_NONE || single_mask != 15u ||
            !enter_closed || !recommend_closed ||
            reward_items.size() != 4u || reward_counts.size() != 4u ||
            enter_values.size() !=
                racing_dungeon::kOfficialEntryConditionCount * 3u) {
            return false;
        }
        for (int rank = 0; rank < racing_dungeon::kMaximumRacers;
             ++rank) {
            parsed.reward_item_ids[rank] = reward_items[rank];
            parsed.reward_counts[rank] = reward_counts[rank];
        }
        for (size_t index = 0u; index < enter_values.size(); index += 3u) {
            EntryCondition condition;
            condition.money = enter_values[index];
            condition.item_id = enter_values[index + 1u];
            condition.item_count = enter_values[index + 2u];
            parsed.entry_conditions.push_back(condition);
        }

        static const int expected_counts[4] = {3, 2, 1, 0};
        if (parsed.minimum_members != racing_dungeon::kMinimumRacers ||
            parsed.level_lower != 10 || parsed.level_upper != 80 ||
            parsed.required_fatigue !=
                racing_dungeon::kOfficialRequiredFatigue) {
            return false;
        }
        for (int rank = 0; rank < 4; ++rank) {
            if (parsed.reward_item_ids[rank] !=
                    racing_dungeon::kOfficialRewardItemId ||
                parsed.reward_counts[rank] != expected_counts[rank]) {
                return false;
            }
        }
        for (size_t index = 0u; index < parsed.entry_conditions.size();
             ++index) {
            const EntryCondition& condition = parsed.entry_conditions[index];
            const int expected_money =
                index == 0u ? 10000 :
                index == 1u ? 20000 :
                index == 2u ? 50000 :
                index == 3u ? 500000 : 1000000;
            if (condition.money != expected_money ||
                condition.item_id !=
                    racing_dungeon::kOfficialInvitationItemId ||
                condition.item_count != 1) {
                return false;
            }
        }
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    RacingConfig config_;
};

struct Identity {
    Identity()
        : user(0), user_uid(-1), account_id(0u), charac_no(0u),
          server_group(-1), member_unique_id(0xffffu) {}
    CUser* user;
    int user_uid;
    uint32_t account_id;
    uint32_t charac_no;
    int server_group;
    uint16_t member_unique_id;
};

struct PendingJoin {
    PendingJoin()
        : identity(), entry_token(0ULL), request_token(0u),
          created_epoch(0u), last_try_epoch(0u), cancel_requested(false) {}
    Identity identity;
    uint64_t entry_token;
    uint32_t request_token;
    uint32_t created_epoch;
    uint32_t last_try_epoch;
    bool cancel_requested;
};

struct QueueEntry {
    QueueEntry()
        : identity(), entry_token(0ULL), activity_day(0u),
          entry_ordinal(0u), joined_epoch(0u) {}
    Identity identity;
    uint64_t entry_token;
    uint32_t activity_day;
    uint16_t entry_ordinal;
    uint32_t joined_epoch;
};

struct Participant {
    Participant()
        : identity(), entry_token(0ULL), activity_day(0u), entry_ordinal(0u),
          finish_rank(0u), finish_time_ms(0u), claim_request_token(0u),
          claim_state(LOCAL_CLAIM_NONE), disconnected(false),
          return_requested(false), return_acked(false) {}
    Identity identity;
    uint64_t entry_token;
    uint32_t activity_day;
    uint16_t entry_ordinal;
    uint8_t finish_rank;
    uint32_t finish_time_ms;
    uint32_t claim_request_token;
    uint8_t claim_state;
    bool disconnected;
    bool return_requested;
    bool return_acked;
};

struct RaceSession {
    RaceSession()
        : party(0), party_index(-1), token(0ULL), phase(0u),
          participant_count(0u), start_tick(0u), start_epoch(0u),
          result_epoch(0u), fee_committed(false), return_started(false),
          participants() {}
    CParty* party;
    int party_index;
    uint64_t token;
    uint8_t phase;
    uint8_t participant_count;
    uint32_t start_tick;
    uint32_t start_epoch;
    uint32_t result_epoch;
    bool fee_committed;
    bool return_started;
    Participant participants[racing_dungeon::kMaximumRacers];
};

struct DetachedClaim {
    DetachedClaim() : request(), next_retry_epoch(0u) {
        std::memset(&request, 0, sizeof(request));
    }
    racing_dungeon::RewardClaimMessage request;
    uint32_t next_retry_epoch;
};

struct ResultWire {
    uint16_t member_unique_id;
    uint8_t rank;
    uint32_t time_ms;
};

struct AdmissionPlan {
    AdmissionPlan()
        : user(0), inventory(0), item_slot(-1), money(0), item_id(0),
          item_count(0), fatigue(0), money_committed(false),
          item_committed(false) {}
    CUser* user;
    CInventory* inventory;
    int item_slot;
    int money;
    int item_id;
    int item_count;
    int fatigue;
    bool money_committed;
    bool item_committed;
};

Mutex g_racing_mutex;
std::map<uint64_t, PendingJoin> g_pending_joins;
std::vector<QueueEntry> g_queue;
std::map<CParty*, RaceSession> g_sessions;
std::map<std::pair<uint64_t, uint32_t>, DetachedClaim> g_detached_claims;
uint32_t g_next_token = 1u;
uint32_t g_next_request_token = 1u;

RacingConfigCache& ConfigCache() {
    static RacingConfigCache cache;
    return cache;
}

uint32_t NextNonZero(uint32_t& source) {
    uint32_t result = source++;
    if (result == 0u) result = source++;
    if (source == 0u) ++source;
    return result;
}

uint32_t NowEpoch() {
    CSystemTime* const time = GlobalData::system_time();
    const int value = time ? time->getCurSec() : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

uint32_t NowTick() {
    CSystemTime* const time = GlobalData::system_time();
    const int value = time ? time->getCurTickCount() : 0;
    return static_cast<uint32_t>(value);
}

uint64_t MakeToken(const Identity& identity) {
    uint64_t hash = 14695981039346656037ULL;
    const uint32_t values[] = {
        NowEpoch(), NowTick(), identity.account_id, identity.charac_no,
        NextNonZero(g_next_token)};
    for (size_t index = 0u; index < sizeof(values) / sizeof(values[0]);
         ++index) {
        uint32_t value = values[index];
        for (unsigned int byte = 0u; byte < 4u; ++byte) {
            hash ^= static_cast<uint64_t>(value & 0xffu);
            hash *= 1099511628211ULL;
            value >>= 8u;
        }
    }
    uint32_t hi = static_cast<uint32_t>(hash >> 32u);
    uint32_t lo = static_cast<uint32_t>(hash & 0xffffffffULL);
    if (hi == 0u) hi = NowEpoch() ? NowEpoch() : 1u;
    if (lo == 0u) lo = NextNonZero(g_next_token);
    return (static_cast<uint64_t>(hi) << 32u) | lo;
}

uint32_t TokenHi(uint64_t token) {
    return static_cast<uint32_t>(token >> 32u);
}

uint32_t TokenLo(uint64_t token) {
    return static_cast<uint32_t>(token & 0xffffffffULL);
}

bool CaptureIdentity(CUser* user, Identity& identity) {
    if (!user || !user->getCurCharacR() || user->get_acc_id() <= 0 ||
        user->getCurCharacNo() <= 0 || user->GetUID() < 0 ||
        user->GetServerGroup() < 0 ||
        user->GetServerGroup() >=
            racing_dungeon::kMaximumServerGroupCount) {
        return false;
    }
    identity.user = user;
    identity.user_uid = user->GetUID();
    identity.account_id = static_cast<uint32_t>(user->get_acc_id());
    identity.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    identity.server_group = user->GetServerGroup();
    identity.member_unique_id =
        static_cast<uint16_t>(user->get_unique_id());
    return identity.member_unique_id != 0xffffu;
}

bool IsCurrent(const Identity& identity) {
    CUser* const user = identity.user;
    return user && user->GetUID() == identity.user_uid &&
           user->get_acc_id() > 0 &&
           static_cast<uint32_t>(user->get_acc_id()) == identity.account_id &&
           user->getCurCharacNo() > 0 &&
           static_cast<uint32_t>(user->getCurCharacNo()) ==
               identity.charac_no &&
           user->GetServerGroup() == identity.server_group &&
           static_cast<uint16_t>(user->get_unique_id()) ==
               identity.member_unique_id;
}

bool IsTownEligible(CUser* user, const RacingConfig& config) {
    if (!user || user->get_state() != kTownState || user->GetParty() ||
        user->CheckInTrade() || user->get_charac_level() < config.level_lower ||
        user->get_charac_level() > config.level_upper) {
        return false;
    }
    return true;
}

bool HasFatigue(CUser* user, int required) {
    if (!user || required <= 0) return false;
    WongWork::CUserPremium* const premium = user->GetPremiumInfo();
    if (premium && premium->isAffectedUnlimitFatigue()) return true;
    return user->getCurCharacTotalMaxFatigue() -
               user->getCurCharacTotalFatigue() >= required;
}

bool CheckAdmission(CUser* user,
                    const RacingConfig& config,
                    uint16_t entry_ordinal,
                    uint8_t& error) {
    error = kErrorGeneric;
    if (!user || entry_ordinal == 0u ||
        entry_ordinal > config.entry_conditions.size()) {
        error = kErrorEntryLimit;
        return false;
    }
    if (user->get_charac_level() < config.level_lower ||
        user->get_charac_level() > config.level_upper) {
        error = kErrorLevel;
        return false;
    }
    if (!HasFatigue(user, config.required_fatigue)) {
        error = kErrorFatigue;
        return false;
    }
    const EntryCondition& condition =
        config.entry_conditions[entry_ordinal - 1u];
    const CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory) return false;
    const int slot = inventory->check_item_exist(condition.item_id);
    if (slot <= 0) {
        error = kErrorItem;
        return false;
    }
    const Inven_Item item = inventory->GetInvenSlot(kInventoryType, slot);
    if (item.isEmpty() || item.getKey() != condition.item_id ||
        item.get_add_info() < condition.item_count ||
        user->CheckItemLock(kInventoryType, slot)) {
        error = kErrorItem;
        return false;
    }
    if (inventory->get_money() < condition.money) {
        error = kErrorMoney;
        return false;
    }
    return true;
}

void TraceFailure(const char* reason, const Identity* identity) {
    cMyTrace trace("racing_dungeon_0828", __LINE__, 5);
    trace("%s account(%u) charac(%u)", reason ? reason : "failure",
          identity ? identity->account_id : 0u,
          identity ? identity->charac_no : 0u);
}

void SendNotice(CUser* user, ENUM_NOTIPACKET command) {
    if (!user) return;
    PacketGuard packet;
    packet.put_header(0, command);
    packet.finalize(true);
    user->Send(packet);
}

void SendJoinCountToQueue() {
    std::vector<Identity> recipients;
    uint8_t count = 0u;
    {
        MutexGuard guard(&g_racing_mutex);
        count = static_cast<uint8_t>(std::min<size_t>(g_queue.size(), 255u));
        for (size_t index = 0u; index < g_queue.size(); ++index) {
            recipients.push_back(g_queue[index].identity);
        }
    }
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_RACING_DUNGEON_JOIN_COUNT);
    packet.put_byte(count);
    packet.finalize(true);
    for (size_t index = 0u; index < recipients.size(); ++index) {
        if (IsCurrent(recipients[index])) recipients[index].user->Send(packet);
    }
}

void SendResultNotice(const RaceSession& session,
                      ENUM_NOTIPACKET command,
                      const std::vector<ResultWire>& results) {
    PacketGuard packet;
    packet.put_header(0, command);
    packet.put_byte(static_cast<uint8_t>(results.size()));
    for (size_t index = 0u; index < results.size(); ++index) {
        packet.put_short(results[index].member_unique_id);
        packet.put_byte(results[index].rank);
        packet.put_int(results[index].time_ms);
    }
    packet.finalize(true);
    for (int index = 0; index < session.participant_count; ++index) {
        const Participant& participant = session.participants[index];
        if (!participant.disconnected && IsCurrent(participant.identity)) {
            participant.identity.user->Send(packet);
        }
    }
}

bool EmptyPacket(PacketBuf& packet) {
    (void)packet;
    return true;
}

void FillIdentityMessage(const Identity& identity,
                         racing_dungeon::RewardClaimMessage& message) {
    std::memset(&message, 0, sizeof(message));
    message.account_id = identity.account_id;
    message.charac_no = identity.charac_no;
    message.server_group = identity.server_group;
    message.reply_user_idx = identity.user_uid;
    message.member_unique_id = identity.member_unique_id;
}

bool BuildClaimRequest(
    const RaceSession& session,
    Participant& participant,
    const RacingConfig& config,
    racing_dungeon::RewardClaimMessage& request) {
    const uint8_t effective_rank = participant.finish_rank != 0u
                                       ? participant.finish_rank : 4u;
    if (effective_rank < 1u || effective_rank > 4u ||
        participant.finish_time_ms == 0u) {
        return false;
    }
    FillIdentityMessage(participant.identity, request);
    request.session_token_hi = TokenHi(session.token);
    request.session_token_lo = TokenLo(session.token);
    request.entry_token_hi = TokenHi(participant.entry_token);
    request.entry_token_lo = TokenLo(participant.entry_token);
    if (participant.claim_request_token == 0u) {
        participant.claim_request_token = NextNonZero(g_next_request_token);
    }
    request.request_token = participant.claim_request_token;
    request.finish_time_ms = participant.finish_time_ms;
    request.reward_item_id = static_cast<uint32_t>(
        config.reward_item_ids[effective_rank - 1u]);
    request.activity_day = participant.activity_day;
    request.entry_ordinal = participant.entry_ordinal;
    request.operation = racing_dungeon::REWARD_OPERATION_CLAIM;
    request.finish_rank = effective_rank;
    request.reward_count = static_cast<uint8_t>(
        config.reward_counts[effective_rank - 1u]);
    return true;
}

bool QueueReserveRequest(const PendingJoin& pending) {
    racing_dungeon::RewardClaimMessage request;
    FillIdentityMessage(pending.identity, request);
    request.entry_token_hi = TokenHi(pending.entry_token);
    request.entry_token_lo = TokenLo(pending.entry_token);
    request.request_token = pending.request_token;
    request.operation = racing_dungeon::REWARD_OPERATION_RESERVE_ENTRY;
    return racing_dungeon::DBRacingDungeon::QueueRequest(
        pending.identity.user_uid, request);
}

bool QueueCancelRequest(const QueueEntry& entry) {
    racing_dungeon::RewardClaimMessage request;
    FillIdentityMessage(entry.identity, request);
    request.entry_token_hi = TokenHi(entry.entry_token);
    request.entry_token_lo = TokenLo(entry.entry_token);
    request.request_token = NextNonZero(g_next_request_token);
    request.activity_day = entry.activity_day;
    request.entry_ordinal = entry.entry_ordinal;
    request.operation = racing_dungeon::REWARD_OPERATION_CANCEL_ENTRY;
    return racing_dungeon::DBRacingDungeon::QueueRequest(
        entry.identity.user_uid, request);
}

bool SameIdentity(const Identity& lhs, const Identity& rhs) {
    return lhs.user == rhs.user && lhs.user_uid == rhs.user_uid &&
           lhs.account_id == rhs.account_id &&
           lhs.charac_no == rhs.charac_no &&
           lhs.server_group == rhs.server_group &&
           lhs.member_unique_id == rhs.member_unique_id;
}

bool UserExistsInQueueUnlocked(const Identity& identity) {
    for (size_t index = 0u; index < g_queue.size(); ++index) {
        if (g_queue[index].identity.account_id == identity.account_id ||
            SameIdentity(g_queue[index].identity, identity)) {
            return true;
        }
    }
    return false;
}

bool UserExistsInSessionUnlocked(const Identity& identity) {
    for (std::map<CParty*, RaceSession>::const_iterator session =
             g_sessions.begin(); session != g_sessions.end(); ++session) {
        for (int index = 0; index < session->second.participant_count;
             ++index) {
            const Identity& current =
                session->second.participants[index].identity;
            if (current.account_id == identity.account_id ||
                SameIdentity(current, identity)) {
                return true;
            }
        }
    }
    return false;
}

void ReturnAndDisband(const RaceSession& snapshot) {
    CParty* const party = snapshot.party;
    if (!party) return;
    if (party->get_member_count() > 0) {
        party->ReturnToVillage();
    }
    for (int index = 0; index < snapshot.participant_count; ++index) {
        const Identity& identity = snapshot.participants[index].identity;
        if (IsCurrent(identity) && identity.user->GetParty() == party) {
            (void)party->leave_user(
                identity.user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
        }
    }
}

void CancelEntries(const RaceSession& snapshot) {
    for (int index = 0; index < snapshot.participant_count; ++index) {
        QueueEntry entry;
        entry.identity = snapshot.participants[index].identity;
        entry.entry_token = snapshot.participants[index].entry_token;
        entry.activity_day = snapshot.participants[index].activity_day;
        entry.entry_ordinal = snapshot.participants[index].entry_ordinal;
        if (!QueueCancelRequest(entry)) {
            TraceFailure("entry cancel queue failed", &entry.identity);
        }
    }
}

void AbortWaitingSession(CParty* party) {
    RaceSession snapshot;
    bool found = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator session =
            g_sessions.find(party);
        if (session != g_sessions.end() &&
            session->second.phase == RACE_WAITING_DUNGEON &&
            !session->second.fee_committed) {
            snapshot = session->second;
            g_sessions.erase(session);
            found = true;
        }
    }
    if (!found) return;
    for (int index = 0; index < snapshot.participant_count; ++index) {
        const Identity& identity = snapshot.participants[index].identity;
        if (IsCurrent(identity)) {
            SendNotice(identity.user,
                       ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
        }
    }
    CancelEntries(snapshot);
    ReturnAndDisband(snapshot);
}

bool BuildAdmissionPlan(const Participant& participant,
                        const RacingConfig& config,
                        AdmissionPlan& plan,
                        uint8_t& error) {
    if (participant.disconnected || !IsCurrent(participant.identity) ||
        !CheckAdmission(participant.identity.user, config,
                        participant.entry_ordinal, error)) {
        return false;
    }
    CUser* const user = participant.identity.user;
    const EntryCondition& condition =
        config.entry_conditions[participant.entry_ordinal - 1u];
    const CInventory* const readable = user->getCurCharacInvenR();
    CInventory* const writable = user->getCurCharacInvenW();
    if (!readable || !writable) return false;
    const int slot = readable->check_item_exist(condition.item_id);
    if (slot <= 0) {
        error = kErrorItem;
        return false;
    }
    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!shadow.setCopy(*readable) ||
        shadow.delete_item(
            static_cast<INVEN_TYPE>(kInventoryType), slot,
            condition.item_count,
            static_cast<eItemDelReason>(kRacingItemDelReason), false) != 1) {
        error = kErrorItem;
        return false;
    }
    plan.user = user;
    plan.inventory = writable;
    plan.item_slot = slot;
    plan.money = condition.money;
    plan.item_id = condition.item_id;
    plan.item_count = condition.item_count;
    plan.fatigue = config.required_fatigue;
    return true;
}

void RollbackAdmission(std::vector<AdmissionPlan>& plans,
                       size_t committed_count) {
    for (size_t index = 0u; index < committed_count; ++index) {
        AdmissionPlan& plan = plans[index];
        if (plan.item_committed) {
            ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
            if (plan.user->AddItem(
                    plan.item_id, plan.item_count,
                    static_cast<eItemAddReason>(kRollbackItemAddReason),
                    item_space, 0) < 0) {
                TraceFailure("admission item rollback failed", 0);
            }
        }
        if (plan.money_committed) {
            if (plan.inventory->gain_money(
                    plan.money,
                    static_cast<eMoneyAddReason>(kRollbackMoneyAddReason),
                    true, 0) != 1) {
                TraceFailure("admission money rollback failed", 0);
            }
        }
        plan.user->SendUpdateItemList(
            static_cast<CUser::eSendTarget>(1),
            static_cast<ENUM_ITEMSPACE>(0), 0);
    }
}

bool CommitAdmissions(std::vector<AdmissionPlan>& plans, uint8_t& error) {
    size_t committed = 0u;
    for (; committed < plans.size(); ++committed) {
        AdmissionPlan& plan = plans[committed];
        if (plan.money > 0 &&
            plan.inventory->use_money(
                plan.money,
                static_cast<eMoneySubReason>(kRacingMoneySubReason),
                true) != 1) {
            error = kErrorMoney;
            RollbackAdmission(plans, committed);
            return false;
        }
        plan.money_committed = plan.money > 0;
        if (plan.inventory->delete_item(
                static_cast<INVEN_TYPE>(kInventoryType), plan.item_slot,
                plan.item_count,
                static_cast<eItemDelReason>(kRacingItemDelReason), true) != 1) {
            error = kErrorItem;
            if (plan.money_committed) {
                (void)plan.inventory->gain_money(
                    plan.money,
                    static_cast<eMoneyAddReason>(kRollbackMoneyAddReason),
                    true, 0);
                plan.money_committed = false;
            }
            RollbackAdmission(plans, committed);
            return false;
        }
        plan.item_committed = true;
    }

    // FatigueUp has no failure return.  It is deliberately last, after every
    // inventory/money mutation has passed its shadow preflight and commit.
    for (size_t index = 0u; index < plans.size(); ++index) {
        plans[index].user->FatigueUp(plans[index].fatigue);
        plans[index].user->SendFatigue();
        plans[index].user->SendUpdateItem(
            static_cast<CUser::eSendTarget>(1),
            static_cast<ENUM_ITEMSPACE>(0), plans[index].item_slot);
        if (plans[index].money > 0) {
            plans[index].user->SendUpdateItemList(
                static_cast<CUser::eSendTarget>(1),
                static_cast<ENUM_ITEMSPACE>(0), 0);
        }
    }
    return true;
}

void TryFormMatch(uint32_t now_epoch);
void FinalizeSession(CParty* party);
void QueueClaim(CParty* party, int participant_index);
void TryReturnSession(CParty* party, bool force);

void TryFormMatch(uint32_t now_epoch) {
    const RacingConfig* const config = ConfigCache().Get();
    if (!config || now_epoch == 0u) return;

    std::vector<QueueEntry> stale;
    std::vector<QueueEntry> selected;
    bool changed = false;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::vector<QueueEntry>::iterator it = g_queue.begin();
             it != g_queue.end();) {
            if (!IsCurrent(it->identity) ||
                !IsTownEligible(it->identity.user, *config)) {
                stale.push_back(*it);
                it = g_queue.erase(it);
                changed = true;
            } else {
                ++it;
            }
        }

        size_t take = 0u;
        if (g_queue.size() >= racing_dungeon::kMaximumRacers) {
            take = racing_dungeon::kMaximumRacers;
        } else if (g_queue.size() >=
                       racing_dungeon::kMinimumRacers &&
                   now_epoch - g_queue[0].joined_epoch >=
                       static_cast<uint32_t>(kQueueGraceSeconds)) {
            take = racing_dungeon::kMinimumRacers;
        }
        if (take != 0u) {
            selected.assign(g_queue.begin(), g_queue.begin() + take);
            g_queue.erase(g_queue.begin(), g_queue.begin() + take);
            changed = true;
        }
    }

    for (size_t index = 0u; index < stale.size(); ++index) {
        (void)QueueCancelRequest(stale[index]);
        if (IsCurrent(stale[index].identity)) {
            SendNotice(stale[index].identity.user,
                       ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
        }
    }
    if (changed) SendJoinCountToQueue();
    if (selected.empty()) return;

    for (size_t index = 0u; index < selected.size(); ++index) {
        if (!IsCurrent(selected[index].identity) ||
            !IsTownEligible(selected[index].identity.user, *config)) {
            for (size_t cancel = 0u; cancel < selected.size(); ++cancel) {
                (void)QueueCancelRequest(selected[cancel]);
                if (IsCurrent(selected[cancel].identity)) {
                    SendNotice(selected[cancel].identity.user,
                               ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
                }
            }
            return;
        }
    }

    CParty* const party = G_CGameManager()
                              ? G_CGameManager()->GetParty() : 0;
    if (!party || party->create_party(selected[0].identity.user) != 1) {
        for (size_t index = 0u; index < selected.size(); ++index) {
            (void)QueueCancelRequest(selected[index]);
            if (IsCurrent(selected[index].identity)) {
                SendNotice(selected[index].identity.user,
                           ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
            }
        }
        return;
    }
    party->setUserMax(racing_dungeon::kMaximumRacers);
    party->set_quick_party(true);
    party->SetSinglePlay(false);
    party->SetAutoCreated(false);

    bool joined = true;
    for (size_t index = 1u; index < selected.size(); ++index) {
        if (party->join_user(selected[index].identity.user) != 1) {
            joined = false;
            break;
        }
    }
    if (!joined || party->get_member_count() !=
                       static_cast<int>(selected.size())) {
        for (size_t index = 0u; index < selected.size(); ++index) {
            (void)QueueCancelRequest(selected[index]);
            if (IsCurrent(selected[index].identity)) {
                SendNotice(selected[index].identity.user,
                           ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
            }
        }
        RaceSession temporary;
        temporary.party = party;
        temporary.participant_count =
            static_cast<uint8_t>(selected.size());
        for (size_t index = 0u; index < selected.size(); ++index) {
            temporary.participants[index].identity = selected[index].identity;
        }
        ReturnAndDisband(temporary);
        return;
    }

    RaceSession session;
    session.party = party;
    session.party_index = party->GetPartyIndex();
    session.token = MakeToken(selected[0].identity);
    session.phase = RACE_WAITING_DUNGEON;
    session.start_epoch = now_epoch;
    session.participant_count = static_cast<uint8_t>(selected.size());
    for (size_t index = 0u; index < selected.size(); ++index) {
        Participant& participant = session.participants[index];
        participant.identity = selected[index].identity;
        participant.entry_token = selected[index].entry_token;
        participant.activity_day = selected[index].activity_day;
        participant.entry_ordinal = selected[index].entry_ordinal;
    }
    {
        MutexGuard guard(&g_racing_mutex);
        g_sessions[party] = session;
    }

    party->send_party_realtime_info();
    party->send_party_ipinfo();
    if (G_GameWorld()) G_GameWorld()->send_party_info_to_all(party, 2);
    for (size_t index = 0u; index < selected.size(); ++index) {
        if (IsCurrent(selected[index].identity)) {
            // CLIENT_IDA_EVIDENCE: NOTI466/sub_581480 reads no body and sends
            // exact empty CMD15, transferring control to ordinary game_start.
            SendNotice(selected[index].identity.user,
                       ENUM_NOTIPACKET_ENTER_RACING_DUNGEON);
        }
    }
}

void QueueClaim(CParty* party, int participant_index) {
    const RacingConfig* const config = ConfigCache().Get();
    if (!config) return;
    racing_dungeon::RewardClaimMessage request;
    Identity identity;
    uint32_t request_token = 0u;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end() ||
            found->second.phase != RACE_RESULTS || participant_index < 0 ||
            participant_index >= found->second.participant_count) {
            return;
        }
        Participant& participant =
            found->second.participants[participant_index];
        if (participant.claim_state != LOCAL_CLAIM_NONE) return;
        if (!BuildClaimRequest(found->second, participant, *config,
                               request)) {
            return;
        }
        request_token = request.request_token;
        participant.claim_state = LOCAL_CLAIM_PENDING;
        identity = participant.identity;
    }
    if (racing_dungeon::DBRacingDungeon::QueueRequest(
            identity.user_uid, request)) {
        return;
    }
    MutexGuard guard(&g_racing_mutex);
    std::map<CParty*, RaceSession>::iterator found = g_sessions.find(party);
    if (found != g_sessions.end() &&
        participant_index < found->second.participant_count) {
        Participant& participant =
            found->second.participants[participant_index];
        if (participant.claim_state == LOCAL_CLAIM_PENDING &&
            participant.claim_request_token == request_token) {
            participant.claim_state = LOCAL_CLAIM_NONE;
        }
    }
    TraceFailure("reward claim queue failed", &identity);
}

void FinalizeSession(CParty* party) {
    RaceSession snapshot;
    std::vector<ResultWire> results;
    bool finalized = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end() ||
            (found->second.phase != RACE_RUNNING &&
             found->second.phase != RACE_LOADING)) {
            return;
        }
        RaceSession& session = found->second;
        uint32_t elapsed = session.start_tick != 0u
                               ? NowTick() - session.start_tick : 1u;
        if (elapsed == 0u) elapsed = 1u;
        if (elapsed > racing_dungeon::kMaximumFinishTimeMs) {
            elapsed = racing_dungeon::kMaximumFinishTimeMs;
        }
        for (int index = 0; index < session.participant_count; ++index) {
            Participant& participant = session.participants[index];
            if (participant.finish_time_ms == 0u) {
                participant.finish_time_ms = elapsed;
            }
            ResultWire wire;
            wire.member_unique_id = participant.identity.member_unique_id;
            wire.rank = participant.finish_rank != 0u
                            ? participant.finish_rank : kInvalidWireRank;
            wire.time_ms = participant.finish_time_ms;
            results.push_back(wire);
        }
        session.phase = RACE_RESULTS;
        session.result_epoch = NowEpoch();
        snapshot = session;
        finalized = true;
    }
    if (!finalized) return;
    // CLIENT_IDA_EVIDENCE: NOTI469/sub_5814D0 consumes
    // u8 count + repeated {u16 member-uid,u8 rank,u32 milliseconds}.
    SendResultNotice(snapshot,
                     ENUM_NOTIPACKET_RACING_DUNGEON_PLAY_RESULT,
                     results);
    for (int index = 0; index < snapshot.participant_count; ++index) {
        QueueClaim(party, index);
    }
}

void TryReturnSession(CParty* party, bool force) {
    RaceSession snapshot;
    std::vector<Identity> acknowledge;
    bool return_party = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end() ||
            found->second.phase != RACE_RESULTS ||
            found->second.return_started) {
            return;
        }
        RaceSession& session = found->second;
        bool all_durable = true;
        bool all_requested = true;
        for (int index = 0; index < session.participant_count; ++index) {
            Participant& participant = session.participants[index];
            if (participant.claim_state != LOCAL_CLAIM_DURABLE) {
                all_durable = false;
            }
            if (!participant.disconnected && !participant.return_requested) {
                all_requested = false;
            }
            if (!participant.disconnected && participant.return_requested &&
                participant.claim_state == LOCAL_CLAIM_DURABLE &&
                !participant.return_acked && IsCurrent(participant.identity)) {
                participant.return_acked = true;
                acknowledge.push_back(participant.identity);
            }
        }
        if (all_durable && (all_requested || force)) {
            session.return_started = true;
            snapshot = session;
            g_sessions.erase(found);
            return_party = true;
        }
    }
    for (size_t index = 0u; index < acknowledge.size(); ++index) {
        if (IsCurrent(acknowledge[index])) {
            acknowledge[index].user->SendCmdOkPacket(
                ENUM_CMDPACKET_RACING_DUNGEON_RETURN_TO_VILLAGE);
        }
    }
    if (return_party) ReturnAndDisband(snapshot);
}

bool AllActiveFinished(const RaceSession& session) {
    for (int index = 0; index < session.participant_count; ++index) {
        const Participant& participant = session.participants[index];
        if (!participant.disconnected && participant.finish_rank == 0u) {
            return false;
        }
    }
    return true;
}

int FindParticipant(const RaceSession& session, CUser* user) {
    Identity identity;
    if (!CaptureIdentity(user, identity)) return -1;
    for (int index = 0; index < session.participant_count; ++index) {
        if (SameIdentity(session.participants[index].identity, identity)) {
            return index;
        }
    }
    return -1;
}

int HandleJoin(CUser* user, PacketBuf& packet) {
    if (!EmptyPacket(packet)) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_JOIN, kErrorGeneric);
        return 0;
    }
    const RacingConfig* const config = ConfigCache().Get();
    Identity identity;
    if (!config || !CaptureIdentity(user, identity) ||
        user->get_state() != kTownState) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_JOIN, kErrorGeneric);
        return 0;
    }
    if (user->GetParty()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN,
                                 kErrorParty);
        return 0;
    }
    if (user->CheckInTrade()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN,
                                 kErrorGeneric);
        return 0;
    }
    uint8_t admission_error = kErrorGeneric;
    if (!CheckAdmission(user, *config, 1u, admission_error)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN,
                                 admission_error);
        return 0;
    }

    bool already_queued = false;
    bool conflict = false;
    PendingJoin pending;
    pending.identity = identity;
    pending.entry_token = MakeToken(identity);
    pending.request_token = NextNonZero(g_next_request_token);
    pending.created_epoch = NowEpoch();
    pending.last_try_epoch = pending.created_epoch;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<uint64_t, PendingJoin>::const_iterator it =
                 g_pending_joins.begin(); it != g_pending_joins.end(); ++it) {
            if (it->second.identity.account_id == identity.account_id) {
                conflict = true;
                break;
            }
        }
        already_queued = UserExistsInQueueUnlocked(identity);
        conflict = conflict || UserExistsInSessionUnlocked(identity);
        if (!conflict && !already_queued) {
            g_pending_joins[pending.entry_token] = pending;
        }
    }
    if (already_queued) {
        user->SendCmdOkPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN);
        SendJoinCountToQueue();
        return 0;
    }
    if (conflict) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN,
                                 kErrorPending);
        return 0;
    }
    if (!QueueReserveRequest(pending)) {
        MutexGuard guard(&g_racing_mutex);
        g_pending_joins.erase(pending.entry_token);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_JOIN,
                                 kErrorGeneric);
    }
    // No early ACK: CMD523 succeeds only after SQL024 has reserved the exact
    // daily PVF entry ordinal and the live fee/item/fatigue precheck repeats.
    return 0;
}

int HandleDisjoin(CUser* user, PacketBuf& packet) {
    if (!EmptyPacket(packet)) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_DISJOIN, kErrorGeneric);
        return 0;
    }
    Identity identity;
    if (!CaptureIdentity(user, identity)) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_DISJOIN, kErrorPending);
        return 0;
    }
    QueueEntry removed;
    bool have_removed = false;
    bool pending_cancel = false;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<uint64_t, PendingJoin>::iterator it =
                 g_pending_joins.begin(); it != g_pending_joins.end(); ++it) {
            if (SameIdentity(it->second.identity, identity)) {
                it->second.cancel_requested = true;
                pending_cancel = true;
                break;
            }
        }
        if (!pending_cancel) {
            for (std::vector<QueueEntry>::iterator it = g_queue.begin();
                 it != g_queue.end(); ++it) {
                if (SameIdentity(it->identity, identity)) {
                    removed = *it;
                    g_queue.erase(it);
                    have_removed = true;
                    break;
                }
            }
        }
    }
    if (!pending_cancel && !have_removed) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RACING_DUNGEON_DISJOIN,
                                 kErrorPending);
        return 0;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_RACING_DUNGEON_DISJOIN);
    if (have_removed) {
        if (!QueueCancelRequest(removed)) {
            TraceFailure("CMD524 cancel queue failed", &removed.identity);
        }
        SendJoinCountToQueue();
    }
    return 0;
}

int HandleGoal(CUser* user, PacketBuf& packet) {
    if (!EmptyPacket(packet) || !user || !user->GetParty()) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_GOAL_IN_PLAYER, kErrorPending);
        return 0;
    }
    CParty* const party = user->GetParty();
    RaceSession snapshot;
    std::vector<ResultWire> results;
    bool duplicate = false;
    bool finalize = false;
    bool accepted = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found != g_sessions.end() &&
            found->second.phase == RACE_RUNNING) {
            RaceSession& session = found->second;
            const int participant_index = FindParticipant(session, user);
            if (participant_index >= 0) {
                Participant& participant =
                    session.participants[participant_index];
                accepted = true;
                duplicate = participant.finish_rank != 0u;
                if (!duplicate) {
                    uint8_t rank = 1u;
                    for (int index = 0; index < session.participant_count;
                         ++index) {
                        if (session.participants[index].finish_rank != 0u) {
                            ++rank;
                        }
                    }
                    uint32_t elapsed = NowTick() - session.start_tick;
                    if (elapsed == 0u) elapsed = 1u;
                    if (elapsed >
                            racing_dungeon::kMaximumFinishTimeMs) {
                        elapsed =
                            racing_dungeon::kMaximumFinishTimeMs;
                    }
                    participant.finish_rank = rank;
                    participant.finish_time_ms = elapsed;
                }
                for (int rank = 1; rank <= session.participant_count;
                     ++rank) {
                    for (int index = 0; index < session.participant_count;
                         ++index) {
                        const Participant& current =
                            session.participants[index];
                        if (current.finish_rank == rank) {
                            ResultWire wire;
                            wire.member_unique_id =
                                current.identity.member_unique_id;
                            wire.rank = current.finish_rank;
                            wire.time_ms = current.finish_time_ms;
                            results.push_back(wire);
                        }
                    }
                }
                snapshot = session;
                finalize = AllActiveFinished(session);
            }
        }
    }
    if (!accepted) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_GOAL_IN_PLAYER, kErrorPending);
        return 0;
    }
    // Duplicate CMD525 is an idempotent replay: acknowledge it but never
    // allocate another rank or overwrite the first authoritative timestamp.
    user->SendCmdOkPacket(ENUM_CMDPACKET_RACING_DUNGEON_GOAL_IN_PLAYER);
    if (!duplicate) {
        SendResultNotice(snapshot,
                         ENUM_NOTIPACKET_RACING_DUNGEON_GOAL_IN_PLAYER,
                         results);
    }
    if (finalize) FinalizeSession(party);
    return 0;
}

int HandleReturn(CUser* user, PacketBuf& packet) {
    if (!EmptyPacket(packet) || !user || !user->GetParty()) {
        if (user) user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_RETURN_TO_VILLAGE,
            kErrorPending);
        return 0;
    }
    CParty* const party = user->GetParty();
    int participant_index = -1;
    bool accepted = false;
    bool queue_claim = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found != g_sessions.end() &&
            found->second.phase == RACE_RESULTS) {
            participant_index = FindParticipant(found->second, user);
            if (participant_index >= 0) {
                Participant& participant =
                    found->second.participants[participant_index];
                participant.return_requested = true;
                accepted = true;
                queue_claim =
                    participant.claim_state == LOCAL_CLAIM_NONE;
            }
        }
    }
    if (!accepted) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_RETURN_TO_VILLAGE,
            kErrorPending);
        return 0;
    }
    if (queue_claim) QueueClaim(party, participant_index);
    // No early ACK.  TryReturnSession emits CMD526 success only after this
    // character's SQL024 outbox is durable, and returns the party only when
    // every connected member has requested return (or cleanup times out).
    TryReturnSession(party, false);
    return 0;
}

void HandleReserveResponse(
    const racing_dungeon::RewardClaimMessage& response) {
    const uint64_t entry_token =
        (static_cast<uint64_t>(response.entry_token_hi) << 32u) |
        response.entry_token_lo;
    PendingJoin pending;
    bool found = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<uint64_t, PendingJoin>::iterator it =
            g_pending_joins.find(entry_token);
        if (it != g_pending_joins.end() &&
            it->second.request_token == response.request_token &&
            it->second.identity.account_id == response.account_id &&
            it->second.identity.charac_no == response.charac_no &&
            it->second.identity.server_group == response.server_group &&
            it->second.identity.user_uid == response.reply_user_idx) {
            pending = it->second;
            g_pending_joins.erase(it);
            found = true;
        }
    }
    if (!found) return;

    QueueEntry entry;
    entry.identity = pending.identity;
    entry.entry_token = pending.entry_token;
    entry.activity_day = response.activity_day;
    entry.entry_ordinal = response.entry_ordinal;
    entry.joined_epoch = NowEpoch();
    const bool reserved =
        response.result_code == racing_dungeon::REWARD_RESULT_SUCCESS &&
        entry.activity_day != 0u && entry.entry_ordinal >= 1u &&
        entry.entry_ordinal <=
            racing_dungeon::kOfficialEntryConditionCount;

    const RacingConfig* const config = ConfigCache().Get();
    uint8_t admission_error = kErrorGeneric;
    const bool current_and_eligible =
        reserved && config && !pending.cancel_requested &&
        IsCurrent(pending.identity) &&
        IsTownEligible(pending.identity.user, *config) &&
        CheckAdmission(pending.identity.user, *config,
                       entry.entry_ordinal, admission_error);
    if (!current_and_eligible) {
        if (reserved && !QueueCancelRequest(entry)) {
            TraceFailure("reserve response cancel queue failed",
                         &entry.identity);
        }
        if (!pending.cancel_requested && IsCurrent(pending.identity)) {
            uint8_t error = admission_error;
            if (!reserved) {
                error = response.result_code ==
                                racing_dungeon::REWARD_RESULT_POLICY
                            ? kErrorEntryLimit : kErrorGeneric;
            }
            pending.identity.user->SendCmdErrorPacket(
                ENUM_CMDPACKET_RACING_DUNGEON_JOIN, error);
        }
        return;
    }

    bool inserted = false;
    {
        MutexGuard guard(&g_racing_mutex);
        if (!UserExistsInQueueUnlocked(entry.identity) &&
            !UserExistsInSessionUnlocked(entry.identity)) {
            g_queue.push_back(entry);
            inserted = true;
        }
    }
    if (!inserted) {
        (void)QueueCancelRequest(entry);
        if (IsCurrent(entry.identity)) {
            entry.identity.user->SendCmdErrorPacket(
                ENUM_CMDPACKET_RACING_DUNGEON_JOIN, kErrorPending);
        }
        return;
    }
    if (IsCurrent(entry.identity)) {
        entry.identity.user->SendCmdOkPacket(
            ENUM_CMDPACKET_RACING_DUNGEON_JOIN);
    }
    SendJoinCountToQueue();
    TryFormMatch(NowEpoch());
}

void HandleClaimResponse(
    const racing_dungeon::RewardClaimMessage& response) {
    const uint64_t session_token =
        (static_cast<uint64_t>(response.session_token_hi) << 32u) |
        response.session_token_lo;
    CParty* party = 0;
    bool durable =
        (response.result_code == racing_dungeon::REWARD_RESULT_SUCCESS ||
         response.result_code ==
             racing_dungeon::REWARD_RESULT_ALREADY_DELIVERED) &&
        response.claim_state ==
            racing_dungeon::REWARD_CLAIM_DELIVERED;
    bool matched_live = false;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<CParty*, RaceSession>::iterator session =
                 g_sessions.begin(); session != g_sessions.end(); ++session) {
            if (session->second.token != session_token ||
                session->second.phase != RACE_RESULTS) {
                continue;
            }
            for (int index = 0; index < session->second.participant_count;
                 ++index) {
                Participant& participant =
                    session->second.participants[index];
                if (participant.identity.account_id == response.account_id &&
                    participant.identity.charac_no == response.charac_no &&
                    participant.identity.server_group ==
                        response.server_group &&
                    participant.entry_token ==
                        ((static_cast<uint64_t>(response.entry_token_hi)
                          << 32u) | response.entry_token_lo) &&
                    participant.claim_request_token ==
                        response.request_token &&
                    participant.finish_time_ms == response.finish_time_ms) {
                    participant.claim_state = durable
                                                  ? LOCAL_CLAIM_DURABLE
                                                  : LOCAL_CLAIM_NONE;
                    party = session->first;
                    matched_live = true;
                    break;
                }
            }
            break;
        }
        if (!matched_live) {
            const std::pair<uint64_t, uint32_t> key(
                session_token, response.charac_no);
            std::map<std::pair<uint64_t, uint32_t>, DetachedClaim>::iterator
                detached = g_detached_claims.find(key);
            if (detached != g_detached_claims.end()) {
                const racing_dungeon::RewardClaimMessage& request =
                    detached->second.request;
                const bool exact =
                    request.account_id == response.account_id &&
                    request.charac_no == response.charac_no &&
                    request.session_token_hi == response.session_token_hi &&
                    request.session_token_lo == response.session_token_lo &&
                    request.entry_token_hi == response.entry_token_hi &&
                    request.entry_token_lo == response.entry_token_lo &&
                    request.request_token == response.request_token &&
                    request.finish_time_ms == response.finish_time_ms &&
                    request.reward_item_id == response.reward_item_id &&
                    request.server_group == response.server_group &&
                    request.activity_day == response.activity_day &&
                    request.entry_ordinal == response.entry_ordinal &&
                    request.member_unique_id == response.member_unique_id &&
                    request.operation == response.operation &&
                    request.finish_rank == response.finish_rank &&
                    request.reward_count == response.reward_count;
                if (exact && durable) {
                    g_detached_claims.erase(detached);
                } else if (exact) {
                    const uint32_t now = NowEpoch();
                    detached->second.next_retry_epoch =
                        now == 0u ? kPendingRetrySeconds
                                  : now + kPendingRetrySeconds;
                }
            }
        }
    }
    if (party) TryReturnSession(party, false);
}

}  // namespace

namespace racing_dungeon {

bool IsRacingParty(CParty* party) {
    if (!party) return false;
    MutexGuard guard(&g_racing_mutex);
    return g_sessions.find(party) != g_sessions.end();
}

bool ShouldSkipOrdinaryFatigue(CParty* party) {
    if (!party) return false;
    MutexGuard guard(&g_racing_mutex);
    std::map<CParty*, RaceSession>::const_iterator found =
        g_sessions.find(party);
    return found != g_sessions.end() && found->second.fee_committed;
}

bool CommitDungeonStart(CParty* party, int dungeon_index) {
    if (!party) return true;
    const RacingConfig* const config = ConfigCache().Get();
    RaceSession snapshot;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::const_iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end()) return true;
        if (!config || dungeon_index != kOfficialDungeonIndex ||
            found->second.phase != RACE_WAITING_DUNGEON ||
            found->second.fee_committed ||
            found->second.participant_count < kMinimumRacers ||
            party->get_member_count() != found->second.participant_count) {
            return false;
        }
        snapshot = found->second;
    }

    std::vector<AdmissionPlan> plans;
    uint8_t error = kErrorGeneric;
    for (int index = 0; index < snapshot.participant_count; ++index) {
        AdmissionPlan plan;
        if (!BuildAdmissionPlan(snapshot.participants[index], *config,
                                plan, error)) {
            if (IsCurrent(snapshot.participants[index].identity)) {
                snapshot.participants[index].identity.user->SendCmdErrorPacket(
                    ENUM_CMDPACKET_RACING_DUNGEON_JOIN, error);
            }
            AbortWaitingSession(party);
            return false;
        }
        plans.push_back(plan);
    }
    if (!CommitAdmissions(plans, error)) {
        AbortWaitingSession(party);
        return false;
    }
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end() ||
            found->second.token != snapshot.token ||
            found->second.phase != RACE_WAITING_DUNGEON) {
            TraceFailure("session vanished after admission commit", 0);
            return false;
        }
        found->second.fee_committed = true;
        found->second.phase = RACE_LOADING;
        found->second.start_epoch = NowEpoch();
    }
    return true;
}

void OnAllMembersLoaded(CParty* party) {
    if (!party) return;
    bool finalize = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end() ||
            found->second.phase != RACE_LOADING ||
            !found->second.fee_committed) {
            return;
        }
        found->second.phase = RACE_RUNNING;
        found->second.start_tick = NowTick();
        if (found->second.start_tick == 0u) {
            found->second.start_tick = 1u;
        }
        found->second.start_epoch = NowEpoch();
        finalize = AllActiveFinished(found->second);
    }
    if (finalize) FinalizeSession(party);
}

void OnUserReset(CUser* user) {
    if (!user) return;
    QueueEntry queued;
    bool have_queued = false;
    CParty* session_party = 0;
    bool abort_waiting = false;
    bool finalize = false;
    bool try_return = false;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<uint64_t, PendingJoin>::iterator pending =
                 g_pending_joins.begin(); pending != g_pending_joins.end();
             ++pending) {
            if (pending->second.identity.user == user &&
                pending->second.identity.user_uid == user->GetUID()) {
                pending->second.cancel_requested = true;
                pending->second.identity.user = 0;
            }
        }
        for (std::vector<QueueEntry>::iterator it = g_queue.begin();
             it != g_queue.end(); ++it) {
            if (it->identity.user == user &&
                it->identity.user_uid == user->GetUID()) {
                queued = *it;
                queued.identity.user = 0;
                g_queue.erase(it);
                have_queued = true;
                break;
            }
        }
        for (std::map<CParty*, RaceSession>::iterator session =
                 g_sessions.begin(); session != g_sessions.end(); ++session) {
            for (int index = 0; index < session->second.participant_count;
                 ++index) {
                Participant& participant =
                    session->second.participants[index];
                if (participant.identity.user == user &&
                    participant.identity.user_uid == user->GetUID()) {
                    participant.disconnected = true;
                    participant.identity.user = 0;
                    session_party = session->first;
                    abort_waiting =
                        session->second.phase == RACE_WAITING_DUNGEON &&
                        !session->second.fee_committed;
                    finalize =
                        (session->second.phase == RACE_RUNNING ||
                         session->second.phase == RACE_LOADING) &&
                        AllActiveFinished(session->second);
                    try_return = session->second.phase == RACE_RESULTS;
                    break;
                }
            }
            if (session_party) break;
        }
    }
    if (have_queued) {
        (void)QueueCancelRequest(queued);
        SendJoinCountToQueue();
    }
    if (abort_waiting) {
        AbortWaitingSession(session_party);
    } else if (finalize) {
        FinalizeSession(session_party);
    } else if (try_return) {
        TryReturnSession(session_party, false);
    }
}

void OnPartyMemberLeaving(CParty* party, CUser* user) {
    if (!party || !user || !IsRacingParty(party)) return;
    OnUserReset(user);
}

void OnPartyDestroyed(CParty* party) {
    if (!party) return;
    uint8_t phase = 0u;
    int participant_count = 0;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::const_iterator found =
            g_sessions.find(party);
        if (found == g_sessions.end()) return;
        phase = found->second.phase;
        participant_count = found->second.participant_count;
    }
    if (phase == RACE_LOADING || phase == RACE_RUNNING) {
        FinalizeSession(party);
        phase = RACE_RESULTS;
    }
    if (phase == RACE_RESULTS) {
        for (int index = 0; index < participant_count; ++index) {
            QueueClaim(party, index);
        }
    }
    const RacingConfig* const config = ConfigCache().Get();
    RaceSession snapshot;
    bool erased = false;
    {
        MutexGuard guard(&g_racing_mutex);
        std::map<CParty*, RaceSession>::iterator found =
            g_sessions.find(party);
        if (found != g_sessions.end()) {
            if (phase == RACE_RESULTS && config) {
                for (int index = 0;
                     index < found->second.participant_count; ++index) {
                    Participant& participant =
                        found->second.participants[index];
                    if (participant.claim_state == LOCAL_CLAIM_DURABLE) {
                        continue;
                    }
                    DetachedClaim detached;
                    if (!BuildClaimRequest(found->second, participant,
                                           *config, detached.request)) {
                        continue;
                    }
                    const uint32_t now = NowEpoch();
                    detached.next_retry_epoch =
                        now == 0u ? kPendingRetrySeconds
                                  : now + kPendingRetrySeconds;
                    g_detached_claims[std::make_pair(
                        found->second.token,
                        participant.identity.charac_no)] = detached;
                }
            }
            snapshot = found->second;
            g_sessions.erase(found);
            erased = true;
        }
    }
    if (erased && phase == RACE_WAITING_DUNGEON &&
        !snapshot.fee_committed) {
        CancelEntries(snapshot);
        for (int index = 0; index < snapshot.participant_count; ++index) {
            if (IsCurrent(snapshot.participants[index].identity)) {
                SendNotice(snapshot.participants[index].identity.user,
                           ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
            }
        }
    }
}

void OnPeriodicTick(uint32_t now_epoch) {
    if (now_epoch == 0u || !ConfigCache().Get()) return;
    std::vector<PendingJoin> retry_pending;
    std::vector<QueueEntry> expired;
    std::vector<racing_dungeon::RewardClaimMessage> detached_retries;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<uint64_t, PendingJoin>::iterator pending =
                 g_pending_joins.begin(); pending != g_pending_joins.end();
             ++pending) {
            if (now_epoch - pending->second.last_try_epoch >=
                    static_cast<uint32_t>(kPendingRetrySeconds)) {
                pending->second.last_try_epoch = now_epoch;
                retry_pending.push_back(pending->second);
            }
        }
        for (std::vector<QueueEntry>::iterator it = g_queue.begin();
             it != g_queue.end();) {
            if (now_epoch - it->joined_epoch >=
                    static_cast<uint32_t>(kQueueLifetimeSeconds)) {
                expired.push_back(*it);
                it = g_queue.erase(it);
            } else {
                ++it;
            }
        }
        for (std::map<std::pair<uint64_t, uint32_t>, DetachedClaim>::iterator
                 detached = g_detached_claims.begin();
             detached != g_detached_claims.end(); ++detached) {
            if (detached->second.next_retry_epoch == 0u ||
                now_epoch >= detached->second.next_retry_epoch) {
                detached_retries.push_back(detached->second.request);
                detached->second.next_retry_epoch =
                    now_epoch + kPendingRetrySeconds;
            }
        }
    }
    for (size_t index = 0u; index < retry_pending.size(); ++index) {
        (void)QueueReserveRequest(retry_pending[index]);
    }
    for (size_t index = 0u; index < expired.size(); ++index) {
        (void)QueueCancelRequest(expired[index]);
        if (IsCurrent(expired[index].identity)) {
            SendNotice(expired[index].identity.user,
                       ENUM_NOTIPACKET_ENTER_RACING_DUNGEON_FAIL);
        }
    }
    for (size_t index = 0u; index < detached_retries.size(); ++index) {
        const racing_dungeon::RewardClaimMessage& request =
            detached_retries[index];
        if (!racing_dungeon::DBRacingDungeon::QueueRequest(
                request.reply_user_idx, request)) {
            TraceFailure("detached reward retry queue failed", 0);
        }
    }
    if (!expired.empty()) SendJoinCountToQueue();
    TryFormMatch(now_epoch);

    std::vector<CParty*> finalize;
    std::vector<CParty*> retry_claims;
    std::vector<CParty*> force_return;
    std::vector<CParty*> abort_waiting;
    {
        MutexGuard guard(&g_racing_mutex);
        for (std::map<CParty*, RaceSession>::const_iterator session =
                 g_sessions.begin(); session != g_sessions.end(); ++session) {
            if (session->second.phase == RACE_WAITING_DUNGEON &&
                session->second.start_epoch != 0u &&
                now_epoch - session->second.start_epoch >= 120u) {
                abort_waiting.push_back(session->first);
            } else if ((session->second.phase == RACE_RUNNING &&
                        session->second.start_epoch != 0u &&
                        now_epoch - session->second.start_epoch >=
                            static_cast<uint32_t>(kRaceLifetimeSeconds)) ||
                       (session->second.phase == RACE_LOADING &&
                        session->second.start_epoch != 0u &&
                        now_epoch - session->second.start_epoch >= 300u)) {
                finalize.push_back(session->first);
            } else if (session->second.phase == RACE_RESULTS) {
                retry_claims.push_back(session->first);
                if (session->second.result_epoch != 0u &&
                    now_epoch - session->second.result_epoch >=
                        static_cast<uint32_t>(kForcedReturnSeconds)) {
                    force_return.push_back(session->first);
                }
            }
        }
    }
    for (size_t index = 0u; index < abort_waiting.size(); ++index) {
        AbortWaitingSession(abort_waiting[index]);
    }
    for (size_t index = 0u; index < finalize.size(); ++index) {
        FinalizeSession(finalize[index]);
    }
    for (size_t party_index = 0u; party_index < retry_claims.size();
         ++party_index) {
        for (int participant = 0;
             participant < racing_dungeon::kMaximumRacers;
             ++participant) {
            QueueClaim(retry_claims[party_index], participant);
        }
    }
    for (size_t index = 0u; index < force_return.size(); ++index) {
        TryReturnSession(force_return[index], true);
    }
}

}  // namespace racing_dungeon_0828

Dispatcher_RacingDungeonJoin::Dispatcher_RacingDungeonJoin()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_RacingDungeonJoin::~Dispatcher_RacingDungeonJoin() {}
int Dispatcher_RacingDungeonJoin::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    return HandleJoin(user, packet);
}
int Dispatcher_RacingDungeonJoin::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_RacingDungeonJoin::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

Dispatcher_RacingDungeonDisjoin::Dispatcher_RacingDungeonDisjoin()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_RacingDungeonDisjoin::~Dispatcher_RacingDungeonDisjoin() {}
int Dispatcher_RacingDungeonDisjoin::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    return HandleDisjoin(user, packet);
}
int Dispatcher_RacingDungeonDisjoin::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_RacingDungeonDisjoin::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

Dispatcher_RacingDungeonGoal::Dispatcher_RacingDungeonGoal()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_RacingDungeonGoal::~Dispatcher_RacingDungeonGoal() {}
int Dispatcher_RacingDungeonGoal::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    return HandleGoal(user, packet);
}
int Dispatcher_RacingDungeonGoal::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_RacingDungeonGoal::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

Dispatcher_RacingDungeonReturn::Dispatcher_RacingDungeonReturn()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_RacingDungeonReturn::~Dispatcher_RacingDungeonReturn() {}
int Dispatcher_RacingDungeonReturn::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    return HandleReturn(user, packet);
}
int Dispatcher_RacingDungeonReturn::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet; (void)message; return 0;
}
int Dispatcher_RacingDungeonReturn::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user; (void)message; (void)param; return 0;
}

InterRacingDungeon::InterRacingDungeon()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              racing_dungeon::kTaiwanInnerCode)) {}

InterRacingDungeon::~InterRacingDungeon() {}

int InterRacingDungeon::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)user;
    if (!payload || payload_size != static_cast<int>(
            sizeof(racing_dungeon::RewardClaimMessage))) {
        return 0;
    }
    const racing_dungeon::RewardClaimMessage response =
        *reinterpret_cast<const racing_dungeon::RewardClaimMessage*>(
            payload);
    if (response.account_id == 0u || response.charac_no == 0u ||
        response.entry_token_hi == 0u || response.entry_token_lo == 0u ||
        response.request_token == 0u || response.server_group < 0 ||
        response.server_group >=
            racing_dungeon::kMaximumServerGroupCount ||
        response.member_unique_id == 0xffffu ||
        response.result_code != racing_dungeon::REWARD_RESULT_SUCCESS &&
        response.result_code !=
            racing_dungeon::REWARD_RESULT_ALREADY_DELIVERED &&
        response.result_code != racing_dungeon::REWARD_RESULT_IDENTITY &&
        response.result_code != racing_dungeon::REWARD_RESULT_POLICY &&
        response.result_code != racing_dungeon::REWARD_RESULT_COLLISION &&
        response.result_code != racing_dungeon::REWARD_RESULT_DATABASE) {
        return 0;
    }
    if (response.operation ==
            racing_dungeon::REWARD_OPERATION_RESERVE_ENTRY) {
        HandleReserveResponse(response);
    } else if (response.operation ==
               racing_dungeon::REWARD_OPERATION_CLAIM) {
        if (response.session_token_hi == 0u ||
            response.session_token_lo == 0u ||
            response.reward_item_id !=
                racing_dungeon::kOfficialRewardItemId ||
            response.finish_rank < 1u || response.finish_rank > 4u ||
            response.finish_time_ms == 0u ||
            response.finish_time_ms >
                racing_dungeon::kMaximumFinishTimeMs) {
            return 0;
        }
        HandleClaimResponse(response);
    }
    // CANCEL_ENTRY is fire-and-forget.  The SQL transaction is idempotent;
    // no client command is waiting on its response.
    return 0;
}
