#include "old_data/db/event/db_shooting_star_dimension_pocket.h"

#include "common/config.h"
#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/db_inventory_mutation_barrier.h"

#include <cstring>
#include <limits>

namespace shooting_star_dimension_pocket_db {
namespace {

const char kCampaignTable[] = "tw_event_campaign_0828";
const char kShootingClaimTable[] = "shooting_star_claim_0828";
const char kDimensionStateTable[] = "dimension_pocket_state_0828";
const char kDimensionClaimTable[] = "dimension_pocket_claim_0828";
const uint64_t kShootingMailNamespace = 0xD831100000000000ULL;
const uint64_t kDimensionMailNamespace = 0xD831200000000000ULL;
const uint64_t kMaximumClaimId = 0x0000FFFFFFFFFFFFULL;
const uint32_t kShootingPaymentItem = 690000771u;

struct ShootingRule {
    uint32_t item_id;
    uint32_t item_count;
    uint32_t price;
    uint8_t account_once;
};

const ShootingRule kShootingRules[kMaximumShootingRewardCount] = {
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

const uint32_t kDimensionMaterials[7] = {
    3037u, 3030u, 3027u, 3151u, 3156u, 3142u, 3028u,
};

struct ShootingClaim {
    ShootingClaim()
        : claim_id(0ULL), campaign_key(0u), activity_day(0u),
          scope_charac_no(0u), scope_day(0u), item_id(0u), item_count(0u),
          payment_count(0u), source_before_count(0u), source_slot(0u),
          reward_index(0u), state(0u) {}
    uint64_t claim_id;
    uint32_t campaign_key;
    uint32_t activity_day;
    uint32_t scope_charac_no;
    uint32_t scope_day;
    uint32_t item_id;
    uint32_t item_count;
    uint32_t payment_count;
    uint32_t source_before_count;
    uint16_t source_slot;
    uint8_t reward_index;
    uint8_t state;
};

struct DimensionState {
    DimensionState()
        : campaign_key(0u), source_day(0u), source_serial(0u),
          pending_claim_id(0ULL), state(0u), clear_count(0u) {}
    uint32_t campaign_key;
    uint32_t source_day;
    uint32_t source_serial;
    uint64_t pending_claim_id;
    uint8_t state;
    uint8_t clear_count;
};

struct DimensionClaim {
    DimensionClaim()
        : claim_id(0ULL), campaign_key(0u), item_id(0u), item_count(0u),
          source_before_count(0u), source_slot(0u), kind(0u), state(0u) {}
    uint64_t claim_id;
    uint32_t campaign_key;
    uint32_t item_id;
    uint32_t item_count;
    uint32_t source_before_count;
    uint16_t source_slot;
    uint8_t kind;
    uint8_t state;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsDimensionMaterial(uint32_t item_id) {
    for (unsigned int index = 0u; index < 7u; ++index) {
        if (kDimensionMaterials[index] == item_id) return true;
    }
    return false;
}

const ShootingRule* Rule(uint8_t reward_index) {
    return reward_index >= 1u &&
                   reward_index <= kMaximumShootingRewardCount
               ? &kShootingRules[reward_index - 1u]
               : 0;
}

bool IsZeroFilled(const void* data, unsigned int size) {
    if (!data) return false;
    const uint8_t* const bytes = static_cast<const uint8_t*>(data);
    for (unsigned int index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return false;
    }
    return true;
}

bool IsCanonicalText(const char* text, unsigned int size) {
    if (!text || size < 2u || text[0] == 0 || text[size - 1u] != 0) {
        return false;
    }
    unsigned int terminator = 1u;
    while (terminator < size && text[terminator] != 0) ++terminator;
    if (terminator == size) return false;
    for (unsigned int index = terminator + 1u; index < size; ++index) {
        if (text[index] != 0) return false;
    }
    return true;
}

bool IsValidMailReward(const MailReward& reward,
                       uint32_t item_id,
                       uint32_t item_count) {
    return reward.item_id == item_id && reward.add_info == item_count &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u &&
           reward.upgrade == (reward.item_attr & 0x1fu) &&
           reward.reserved == 0u;
}

bool HasNoMail(const Message& message) {
    return IsZeroFilled(&message.mail_reward, sizeof(message.mail_reward)) &&
           IsZeroFilled(message.mail_title, sizeof(message.mail_title)) &&
           IsZeroFilled(message.mail_content, sizeof(message.mail_content));
}

bool HasExactMail(const Message& message) {
    return IsValidMailReward(message.mail_reward,
                             message.item_id,
                             message.item_count) &&
           IsCanonicalText(message.mail_title,
                           sizeof(message.mail_title)) &&
           IsCanonicalText(message.mail_content,
                           sizeof(message.mail_content));
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.request_token == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.activity_day == 0u || message.source_serial > 0xffffu ||
        message.daily_reset_hour != 6u ||
        message.local_mutation_done > 1u ||
        message.physical_delivery_present != 0u ||
        message.operation < OPERATION_LOAD ||
        message.operation > OPERATION_DIMENSION_COMMIT_REWARD) {
        return false;
    }

    switch (message.operation) {
        case OPERATION_LOAD:
        case OPERATION_REFRESH:
            return message.campaign_key == 0u &&
                   message.claim_id == 0ULL &&
                   message.claim_kind == CLAIM_NONE &&
                   message.reward_index == 0u && message.item_id == 0u &&
                   message.item_count == 0u && message.payment_count == 0u &&
                   message.source_before_count == 0u &&
                   message.local_mutation_done == 0u && HasNoMail(message);

        case OPERATION_DIMENSION_CLEAR:
        case OPERATION_DIMENSION_SEED:
            return message.campaign_key != 0u &&
                   message.claim_id == 0ULL &&
                   message.claim_kind == CLAIM_NONE &&
                   message.reward_index == 0u && message.item_id == 0u &&
                   message.item_count == 0u && message.payment_count == 0u &&
                   message.source_before_count == 0u &&
                   message.local_mutation_done == 0u && HasNoMail(message);

        case OPERATION_SHOOTING_RESERVE: {
            const ShootingRule* const rule = Rule(message.reward_index);
            return rule && message.claim_id == 0ULL &&
                   message.claim_kind == CLAIM_SHOOTING_REWARD &&
                   message.item_id == rule->item_id &&
                   message.item_count == rule->item_count &&
                   message.payment_count == rule->price &&
                   ((rule->price == 0u && message.source_slot == 0xffffu &&
                     message.source_before_count == 0u) ||
                    (rule->price != 0u &&
                     message.source_slot <
                         oldsrv::config::kMainInventoryItemCount &&
                     message.source_before_count >= rule->price &&
                     message.source_before_count <=
                         static_cast<uint32_t>(
                             std::numeric_limits<int>::max()))) &&
                   message.local_mutation_done == 0u && HasNoMail(message);
        }

        case OPERATION_SHOOTING_COMMIT: {
            const ShootingRule* const rule = Rule(message.reward_index);
            return rule && message.claim_id != 0ULL &&
                   message.claim_id <= kMaximumClaimId &&
                   message.claim_kind == CLAIM_SHOOTING_REWARD &&
                   message.item_id == rule->item_id &&
                   message.item_count == rule->item_count &&
                   message.payment_count == rule->price &&
                   ((rule->price == 0u && message.source_slot == 0xffffu &&
                     message.source_before_count == 0u &&
                     message.local_mutation_done == 0u) ||
                    (rule->price != 0u &&
                     message.source_slot <
                         oldsrv::config::kMainInventoryItemCount &&
                     message.source_before_count >= rule->price &&
                     message.source_before_count <=
                         static_cast<uint32_t>(
                             std::numeric_limits<int>::max()) &&
                     message.local_mutation_done == 1u)) &&
                   HasExactMail(message);
        }

        case OPERATION_SHOOTING_CANCEL: {
            const ShootingRule* const rule = Rule(message.reward_index);
            return rule && message.claim_id != 0ULL &&
                   message.claim_id <= kMaximumClaimId &&
                   message.claim_kind == CLAIM_SHOOTING_REWARD &&
                   message.item_id == rule->item_id &&
                   message.item_count == rule->item_count &&
                   message.payment_count == rule->price &&
                   ((rule->price == 0u &&
                     message.source_before_count == 0u) ||
                    (rule->price != 0u &&
                     message.source_before_count >= rule->price &&
                     message.source_before_count <=
                         static_cast<uint32_t>(
                             std::numeric_limits<int>::max()))) &&
                   message.local_mutation_done == 0u && HasNoMail(message);
        }

        case OPERATION_DIMENSION_RESERVE_MATERIAL:
            return message.campaign_key != 0u &&
                   message.claim_id == 0ULL &&
                   message.claim_kind == CLAIM_DIMENSION_MATERIAL &&
                   IsDimensionMaterial(message.item_id) &&
                   message.item_count == 20u && message.payment_count == 0u &&
                   message.source_slot <
                       oldsrv::config::kMainInventoryItemCount &&
                   message.source_before_count >= 20u &&
                   message.source_before_count <=
                       static_cast<uint32_t>(
                           std::numeric_limits<int>::max()) &&
                   message.local_mutation_done == 0u && HasNoMail(message);

        case OPERATION_DIMENSION_COMMIT_MATERIAL:
        case OPERATION_DIMENSION_CANCEL_MATERIAL:
            return message.campaign_key != 0u &&
                   message.claim_id != 0ULL &&
                   message.claim_id <= kMaximumClaimId &&
                   message.claim_kind == CLAIM_DIMENSION_MATERIAL &&
                   IsDimensionMaterial(message.item_id) &&
                   message.item_count == 20u && message.payment_count == 0u &&
                   message.source_slot <
                       oldsrv::config::kMainInventoryItemCount &&
                   message.source_before_count >= 20u &&
                   message.source_before_count <=
                       static_cast<uint32_t>(
                           std::numeric_limits<int>::max()) &&
                   message.local_mutation_done ==
                       (message.operation ==
                                OPERATION_DIMENSION_COMMIT_MATERIAL
                            ? 1u : 0u) &&
                   HasNoMail(message);

        case OPERATION_DIMENSION_RESERVE_REWARD:
            return message.campaign_key != 0u &&
                   message.claim_id == 0ULL &&
                   message.claim_kind == CLAIM_DIMENSION_REWARD &&
                   message.item_id == 690000773u && message.item_count == 1u &&
                   message.payment_count == 0u &&
                   message.source_before_count == 0u &&
                   message.local_mutation_done == 0u && HasNoMail(message);

        case OPERATION_DIMENSION_COMMIT_REWARD:
            return message.campaign_key != 0u &&
                   message.claim_id != 0ULL &&
                   message.claim_id <= kMaximumClaimId &&
                   message.claim_kind == CLAIM_DIMENSION_REWARD &&
                   message.item_id == 690000773u && message.item_count == 1u &&
                   message.payment_count == 0u &&
                   message.source_before_count == 0u &&
                   message.local_mutation_done == 0u && HasExactMail(message);

        default:
            return false;
    }
}

MySQL* GetPartitionDatabase(int handle, int server_group) {
    DBMgr* const manager = GlobalData::s_db_mgr;
    if (!manager || !IsValidServerGroup(server_group)) return 0;
    const ENUM_DB_HANDLE_IDX handle_index =
        static_cast<ENUM_DB_HANDLE_IDX>(handle);
    const ENUM_SERVER_GROUP group =
        static_cast<ENUM_SERVER_GROUP>(server_group);
    return manager->isActiveGlobalDB()
               ? manager->GetDBHandleServerGroup(handle_index, group)
               : manager->GetDBHandle(handle_index, group);
}

bool Begin(MySQL* database) {
    return database && database->set_query("start transaction") &&
           database->exec(true);
}

void Rollback(MySQL* database) {
    if (database && database->set_query("rollback")) {
        (void)database->exec(true);
    }
}

bool Commit(MySQL* database) {
    return database && database->set_query("commit") &&
           database->exec(true);
}

bool LoadOwnedCharacter(MySQL* database, const Message& request) {
    unsigned int level = 0u;
    return database &&
           database->set_query(
               "seLect lev from charac_info where charac_no=%u and m_id=%u "
               "and delete_flag=0 limit 2",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.account_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, level) &&
           level >= 1u && level <= 85u;
}

bool ReadActivityDay(MySQL* database,
                     const Message& request,
                     uint32_t& activity_day) {
    activity_day = 0u;
    return database &&
           database->set_query(
               "seLect date_format(date_sub(now(),interval %u hour),"
               "'%%Y%%m%%d')+0",
               static_cast<unsigned int>(request.daily_reset_hour)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, activity_day) &&
           activity_day != 0u;
}

bool ReadSourceActivityDay(MySQL* database,
                           const Message& request,
                           uint32_t& current_activity_day,
                           uint32_t& source_activity_day) {
    current_activity_day = 0u;
    source_activity_day = 0u;
    int day_delta = -1;
    return database && request.activity_day != 0u &&
           database->set_query(
               "seLect date_format(date_sub(now(),interval %u hour),"
               "'%%Y%%m%%d')+0,date_format(date_sub(from_unixtime(%u),"
               "interval %u hour),'%%Y%%m%%d')+0,datediff("
               "date(date_sub(now(),interval %u hour)),date(date_sub("
               "from_unixtime(%u),interval %u hour)))",
               static_cast<unsigned int>(request.daily_reset_hour),
               static_cast<unsigned int>(request.activity_day),
               static_cast<unsigned int>(request.daily_reset_hour),
               static_cast<unsigned int>(request.daily_reset_hour),
               static_cast<unsigned int>(request.activity_day),
               static_cast<unsigned int>(request.daily_reset_hour)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_uint(0, current_activity_day) &&
           database->get_uint(1, source_activity_day) &&
           database->get_int(2, day_delta) && current_activity_day != 0u &&
           source_activity_day != 0u && day_delta >= 0 && day_delta <= 1;
}

bool LoadCampaign(MySQL* database,
                  unsigned int event_code,
                  bool& active,
                  uint32_t& campaign_key,
                  bool for_update) {
    active = false;
    campaign_key = 0u;
    unsigned int enabled = 0u;
    unsigned int in_window = 0u;
    return database &&
           database->set_query(
               "seLect enabled,campaign_key,"
               "(start_at<=now() and end_at>now() and "
               "(%u<>720 or (hour(now())>=21 and hour(now())<23))) from %s "
               "where event_code=%u %s",
               event_code, kCampaignTable, event_code,
               for_update ? "for update" : "limit 2") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, enabled) &&
           database->get_uint(1, campaign_key) &&
           database->get_uint(2, in_window) && enabled <= 1u &&
           in_window <= 1u &&
           (active = enabled == 1u && in_window == 1u &&
                     campaign_key != 0u,
            true);
}

bool BuildShootingAvailability(MySQL* database,
                               const Message& request,
                               uint32_t campaign_key,
                               uint32_t activity_day,
                               uint16_t& mask) {
    mask = 0u;
    for (unsigned int index = 0u;
         index < kMaximumShootingRewardCount;
         ++index) {
        const ShootingRule& rule = kShootingRules[index];
        const uint32_t scope_charac = rule.account_once
                                          ? 0u : request.charac_no;
        const uint32_t scope_day = rule.account_once ? 0u : activity_day;
        unsigned int count = 0u;
        if (!database->set_query(
                "seLect count(*) from %s where server_group=%u and m_id=%u "
                "and campaign_key=%u and reward_index=%u and "
                "scope_charac_no=%u and scope_day=%u and claim_state in(1,2)",
                kShootingClaimTable,
                static_cast<unsigned int>(request.server_group),
                static_cast<unsigned int>(request.account_id),
                static_cast<unsigned int>(campaign_key), index + 1u,
                static_cast<unsigned int>(scope_charac),
                static_cast<unsigned int>(scope_day)) ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, count) ||
            count > 1u) {
            return false;
        }
        if (count == 0u) mask |= static_cast<uint16_t>(1u << index);
    }
    return true;
}

bool ReadShootingClaim(MySQL* database,
                       const Message& request,
                       uint64_t claim_id,
                       bool for_update,
                       ShootingClaim& claim) {
    unsigned int slot = 0u;
    unsigned int reward_index = 0u;
    unsigned int state = 0u;
    return database && claim_id != 0ULL &&
           database->set_query(
               "seLect claim_id,campaign_key,activity_day,scope_charac_no,"
               "scope_day,reward_index,item_id,item_count,payment_count,"
               "source_before_count,source_slot,claim_state from %s where "
               "claim_id=%llu and "
               "server_group=%u and m_id=%u and charac_no=%u %s",
               kShootingClaimTable,
               static_cast<unsigned long long>(claim_id),
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               for_update ? "for update" : "limit 2") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.campaign_key) &&
           database->get_uint(2, claim.activity_day) &&
           database->get_uint(3, claim.scope_charac_no) &&
           database->get_uint(4, claim.scope_day) &&
           database->get_uint(5, reward_index) &&
           database->get_uint(6, claim.item_id) &&
           database->get_uint(7, claim.item_count) &&
           database->get_uint(8, claim.payment_count) &&
           database->get_uint(9, claim.source_before_count) &&
           database->get_uint(10, slot) && database->get_uint(11, state) &&
           claim.claim_id == claim_id && claim.claim_id <= kMaximumClaimId &&
           reward_index >= 1u &&
           reward_index <= kMaximumShootingRewardCount && slot <= 0xffffu &&
           state >= PHASE_RESERVED && state <= PHASE_DELIVERED &&
           (claim.reward_index = static_cast<uint8_t>(reward_index), true) &&
           (claim.source_slot = static_cast<uint16_t>(slot), true) &&
           (claim.state = static_cast<uint8_t>(state), true);
}

bool ValidateShootingClaim(const Message& request,
                           const ShootingClaim& claim);

// -1 malformed/DB failure, 0 absent, 1 exact request found.
int FindShootingRequest(MySQL* database,
                        const Message& request,
                        ShootingClaim& claim) {
    if (!database ||
        !database->set_query(
            "seLect claim_id from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and session_token=%u and request_token=%u "
            "limit 2",
            kShootingClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    uint64_t claim_id = 0ULL;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, claim_id) || claim_id == 0ULL ||
        !ReadShootingClaim(database, request, claim_id, true, claim)) {
        return -1;
    }
    return 1;
}

// -1 malformed/DB failure or multiple unresolved mutations, 0 absent,
// 1 exact pending claim for this character.  A character cannot safely own
// two unresolved inventory mutations at once, so that legacy/corrupt image is
// deliberately left for operator reconciliation.
int FindPendingShooting(MySQL* database,
                        const Message& request,
                        ShootingClaim& claim) {
    if (!database ||
        !database->set_query(
            "seLect claim_id from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and claim_state=1 order by claim_id limit 2",
            kShootingClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    uint64_t claim_id = 0ULL;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, claim_id) || claim_id == 0ULL ||
        !ReadShootingClaim(database, request, claim_id, true, claim)) {
        return -1;
    }
    Message expected = request;
    expected.reward_index = claim.reward_index;
    expected.item_id = claim.item_id;
    expected.item_count = claim.item_count;
    expected.payment_count = claim.payment_count;
    expected.source_before_count = claim.source_before_count;
    expected.source_slot = claim.source_slot;
    return ValidateShootingClaim(expected, claim) ? 1 : -1;
}

bool ValidateShootingClaim(const Message& request,
                           const ShootingClaim& claim) {
    const ShootingRule* const rule = Rule(request.reward_index);
    if (!rule || claim.reward_index != request.reward_index ||
        claim.item_id != rule->item_id ||
        claim.item_count != rule->item_count ||
        claim.payment_count != rule->price ||
        claim.source_before_count != request.source_before_count ||
        claim.source_slot != request.source_slot) {
        return false;
    }
    return rule->account_once
               ? claim.scope_charac_no == 0u && claim.scope_day == 0u
               : claim.scope_charac_no == request.charac_no &&
                     claim.scope_day == claim.activity_day;
}

bool ReadDimensionState(MySQL* database,
                        const Message& request,
                        uint32_t campaign_key,
                        bool for_update,
                        DimensionState& state) {
    unsigned int raw_state = 0u;
    unsigned int clear_count = 0u;
    return database && campaign_key != 0u &&
           database->set_query(
               "seLect campaign_key,event_state,proper_clear_count,"
               "source_day,source_serial,pending_claim_id from %s where "
               "server_group=%u and m_id=%u and charac_no=%u and "
               "campaign_key=%u %s",
               kDimensionStateTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(campaign_key),
               for_update ? "for update" : "limit 2") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, state.campaign_key) &&
           database->get_uint(1, raw_state) &&
           database->get_uint(2, clear_count) &&
           database->get_uint(3, state.source_day) &&
           database->get_uint(4, state.source_serial) &&
           database->get_ulonglong(5, state.pending_claim_id) &&
           state.campaign_key == campaign_key && raw_state <= 3u &&
           clear_count <= kMaximumDimensionClearCount &&
           state.source_serial <= 0xffffu &&
           state.pending_claim_id <= kMaximumClaimId &&
           (state.state = static_cast<uint8_t>(raw_state), true) &&
           (state.clear_count = static_cast<uint8_t>(clear_count), true);
}

bool EnsureDimensionState(MySQL* database,
                          const Message& request,
                          uint32_t campaign_key,
                          uint32_t activity_day,
                          DimensionState& state,
                          bool& inserted) {
    inserted = false;
    if (!database || campaign_key == 0u || activity_day == 0u ||
        !database->set_query(
            "inSert ignore into %s(server_group,m_id,charac_no,campaign_key,"
            "event_state,proper_clear_count,source_day,source_serial,"
            "pending_claim_id,revision,created_at,updated_at) values("
            "%u,%u,%u,%u,1,0,%u,%u,0,0,now(),now())",
            kDimensionStateTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(request.source_serial)) ||
        !database->exec(true)) {
        return false;
    }
    inserted = database->getAffectedRowCount() == 1u;
    return ReadDimensionState(database, request, campaign_key, true, state);
}

bool ReadDimensionClaim(MySQL* database,
                        const Message& request,
                        uint64_t claim_id,
                        bool for_update,
                        DimensionClaim& claim) {
    unsigned int source_slot = 0u;
    unsigned int kind = 0u;
    unsigned int state = 0u;
    return database && claim_id != 0ULL &&
           database->set_query(
               "seLect claim_id,campaign_key,claim_kind,item_id,item_count,"
               "source_before_count,source_slot,claim_state from %s where "
               "claim_id=%llu and "
               "server_group=%u and m_id=%u and charac_no=%u %s",
               kDimensionClaimTable,
               static_cast<unsigned long long>(claim_id),
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               for_update ? "for update" : "limit 2") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.campaign_key) &&
           database->get_uint(2, kind) &&
           database->get_uint(3, claim.item_id) &&
           database->get_uint(4, claim.item_count) &&
           database->get_uint(5, claim.source_before_count) &&
           database->get_uint(6, source_slot) &&
           database->get_uint(7, state) && claim.claim_id == claim_id &&
           claim.claim_id <= kMaximumClaimId &&
           (kind == CLAIM_DIMENSION_MATERIAL ||
            kind == CLAIM_DIMENSION_REWARD) &&
           source_slot <= 0xffffu && state >= PHASE_RESERVED &&
           state <= PHASE_DELIVERED &&
           (claim.kind = static_cast<uint8_t>(kind), true) &&
           (claim.source_slot = static_cast<uint16_t>(source_slot), true) &&
           (claim.state = static_cast<uint8_t>(state), true);
}

int FindDimensionRequest(MySQL* database,
                         const Message& request,
                         DimensionClaim& claim) {
    if (!database ||
        !database->set_query(
            "seLect claim_id from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and session_token=%u and request_token=%u "
            "limit 2",
            kDimensionClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    uint64_t claim_id = 0ULL;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, claim_id) || claim_id == 0ULL ||
        !ReadDimensionClaim(database, request, claim_id, true, claim)) {
        return -1;
    }
    return 1;
}

int FindPendingDimension(MySQL* database,
                         const Message& request,
                         DimensionState& state,
                         DimensionClaim& claim) {
    uint32_t campaign_key = 0u;
    if (!database ||
        !database->set_query(
            "seLect campaign_key from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and pending_claim_id<>0 order by campaign_key "
            "limit 2",
            kDimensionStateTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, campaign_key) || campaign_key == 0u ||
        !ReadDimensionState(database, request, campaign_key, true, state) ||
        state.pending_claim_id == 0ULL ||
        !ReadDimensionClaim(database, request, state.pending_claim_id, true,
                            claim) ||
        claim.campaign_key != campaign_key ||
        claim.state != PHASE_RESERVED) {
        return -1;
    }
    if (claim.kind == CLAIM_DIMENSION_MATERIAL) {
        return IsDimensionMaterial(claim.item_id) &&
                       claim.item_count == 20u &&
                       claim.source_slot <
                           oldsrv::config::kMainInventoryItemCount &&
                       claim.source_before_count >= 20u &&
                       claim.source_before_count <=
                           static_cast<uint32_t>(
                               std::numeric_limits<int>::max()) &&
                       state.state == 2u
                   ? 1 : -1;
    }
    return claim.kind == CLAIM_DIMENSION_REWARD &&
                   claim.item_id == 690000773u && claim.item_count == 1u &&
                   claim.source_slot == 0u &&
                   claim.source_before_count == 0u && state.state == 3u
               ? 1 : -1;
}

bool FindMail(MySQL* database,
              uint64_t delivery_key,
              uint32_t receiver,
              const MailReward& reward,
              const char* title,
              bool& found,
              uint32_t& letter_id) {
    found = false;
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char* const item_guid = database
        ? database->blob_to_str(0, const_cast<uint8_t*>(reward.item_guid),
                                static_cast<int>(sizeof(reward.item_guid)))
        : 0;
    char* const random_option = database
        ? database->blob_to_str(
              1, const_cast<uint8_t*>(reward.random_option),
              static_cast<int>(sizeof(reward.random_option)))
        : 0;
    if (!database || !title ||
        !item_guid || !random_option ||
        database->escape_string(escaped_title, title) < 0 ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info,letter_id,"
            "length(send_charac_name),binary send_charac_name='%s',"
            "seal_flag,endurance,upgrade,amplify_option,amplify_value,"
            "creature_flag,avata_flag,extend_info,chronicle_value,"
            "seperate_upgrade,length(item_guid),binary item_guid='%s',"
            "length(random_option),binary random_option='%s',unlimit_flag,"
            "type,gold,send_charac_no from postal where auction_id=%llu "
            "limit 2",
            escaped_title, item_guid, random_option,
            static_cast<unsigned long long>(delivery_key)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int values[24] = {};
    if (database->get_n_rows() != 1 || !database->fetch()) return false;
    for (unsigned int index = 0u; index < 24u; ++index) {
        if (!database->get_uint(static_cast<int>(index), values[index])) {
            return false;
        }
    }
    if (values[0] != receiver || values[1] != reward.item_id ||
        values[2] != reward.add_info || values[3] != 0u ||
        values[4] != std::strlen(title) || values[5] != 1u ||
        values[6] != reward.seal_flag || values[7] != reward.endurance ||
        values[8] != reward.item_attr ||
        values[9] != reward.amplify_option ||
        values[10] != reward.amplify_value ||
        values[11] != reward.creature_flag ||
        values[12] != reward.avatar_flag ||
        values[13] != reward.extend_info ||
        values[14] != reward.chronicle_value ||
        values[15] != reward.separate_upgrade ||
        values[16] != sizeof(reward.item_guid) || values[17] != 1u ||
        values[18] != sizeof(reward.random_option) || values[19] != 1u ||
        values[20] != 0u || values[21] != 0u || values[22] != 0u ||
        values[23] != 0u) {
        return false;
    }
    letter_id = values[3];
    found = true;
    return true;
}

bool InsertMail(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const MailReward& reward,
                const char* title) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    if (!database || !title ||
        database->escape_string(escaped_title, title) < 0) {
        return false;
    }
    char* const item_guid = database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid)));
    char* const random_option = database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option)));
    return item_guid && random_option &&
           database->set_query(
               "inSert into postal(occ_time,send_charac_no,"
               "receive_charac_no,seal_flag,item_id,add_info,endurance,"
               "upgrade,amplify_option,amplify_value,gold,send_charac_name,"
               "creature_flag,avata_flag,letter_id,extend_info,item_guid,"
               "auction_id,random_option,chronicle_value,seperate_upgrade,"
               "unlimit_flag,type) values(now(),0,%u,%u,%u,%u,%u,%u,%u,"
               "%u,0,'%s',%u,%u,%u,%u,'%s',%llu,'%s',%u,%u,0,0)",
               static_cast<unsigned int>(receiver),
               static_cast<unsigned int>(reward.seal_flag),
               static_cast<unsigned int>(reward.item_id),
               static_cast<unsigned int>(reward.add_info),
               static_cast<unsigned int>(reward.endurance),
               static_cast<unsigned int>(reward.item_attr),
               static_cast<unsigned int>(reward.amplify_option),
               static_cast<unsigned int>(reward.amplify_value), escaped_title,
               static_cast<unsigned int>(reward.creature_flag),
               static_cast<unsigned int>(reward.avatar_flag),
               0u,
               static_cast<unsigned int>(reward.extend_info), item_guid,
               static_cast<unsigned long long>(delivery_key), random_option,
               static_cast<unsigned int>(reward.chronicle_value),
               static_cast<unsigned int>(reward.separate_upgrade)) &&
           database->exec(true);
}

bool EnsureMail(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const MailReward& reward,
                 const char* title,
                 const char* content) {
    (void)content;
    bool found = false;
    uint32_t letter_id = 0u;
    if (!FindMail(database, delivery_key, receiver, reward, title,
                  found, letter_id)) {
        return false;
    }
    if (found) return true;
    if (InsertMail(database, delivery_key, receiver, reward, title)) {
        return true;
    }
    uint32_t existing_letter = 0u;
    return FindMail(database, delivery_key, receiver, reward, title,
                    found, existing_letter) && found && existing_letter == 0u;
}

uint64_t DeliveryKey(uint8_t claim_kind, uint64_t claim_id) {
    return (claim_kind == CLAIM_SHOOTING_REWARD
                ? kShootingMailNamespace : kDimensionMailNamespace) |
           claim_id;
}

bool PhysicalDeliveryExists(const Message& request,
                            uint8_t claim_kind,
                            uint64_t claim_id,
                            uint32_t item_id,
                            uint32_t item_count,
                            bool& found) {
    found = false;
    MySQL* const postal = GetPartitionDatabase(3, request.server_group);
    if (!postal || claim_id == 0ULL || item_id == 0u || item_count == 0u ||
        !postal->set_query(
            "seLect receive_charac_no,item_id,add_info,letter_id,"
            "send_charac_no,gold,unlimit_flag,type from postal where "
            "auction_id=%llu limit 2",
            static_cast<unsigned long long>(
                DeliveryKey(claim_kind, claim_id))) ||
        !postal->exec(true)) {
        return false;
    }
    if (postal->get_n_rows() == 0) return true;
    unsigned int receiver = 0u;
    unsigned int persisted_item = 0u;
    unsigned int persisted_count = 0u;
    unsigned int zero_fields[5] = {};
    if (postal->get_n_rows() != 1 || !postal->fetch() ||
        !postal->get_uint(0, receiver) ||
        !postal->get_uint(1, persisted_item) ||
        !postal->get_uint(2, persisted_count) ||
        !postal->get_uint(3, zero_fields[0]) ||
        !postal->get_uint(4, zero_fields[1]) ||
        !postal->get_uint(5, zero_fields[2]) ||
        !postal->get_uint(6, zero_fields[3]) ||
        !postal->get_uint(7, zero_fields[4]) ||
        receiver != request.charac_no || persisted_item != item_id ||
        persisted_count != item_count || zero_fields[0] != 0u ||
        zero_fields[1] != 0u || zero_fields[2] != 0u ||
        zero_fields[3] != 0u || zero_fields[4] != 0u) {
        return false;
    }
    found = true;
    return true;
}

bool DeliverReward(const Message& request) {
    MySQL* const postal = GetPartitionDatabase(3, request.server_group);
    if (!Begin(postal)) return false;
    const bool delivered = EnsureMail(
        postal, DeliveryKey(request.claim_kind, request.claim_id),
        request.charac_no, request.mail_reward, request.mail_title,
        request.mail_content);
    if (!delivered || !Commit(postal)) {
        Rollback(postal);
        return false;
    }
    return true;
}

bool HandleSnapshot(MySQL* database,
                    const Message& request,
                    Message& response,
                    bool seed_existing) {
    (void)seed_existing;
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    uint32_t activity_day = 0u;
    uint32_t source_activity_day = 0u;
    bool shooting_active = false;
    bool dimension_active = false;
    uint32_t shooting_campaign = 0u;
    uint32_t dimension_campaign = 0u;
    if (!ReadSourceActivityDay(database, request, activity_day,
                               source_activity_day) ||
        !LoadCampaign(database, 720u, shooting_active, shooting_campaign,
                      false) ||
        !LoadCampaign(database, 721u, dimension_active,
                      dimension_campaign, false)) {
        Rollback(database);
        return false;
    }
    response.activity_day = source_activity_day;
    response.shooting_active = shooting_active ? 1u : 0u;
    response.dimension_active = dimension_active ? 1u : 0u;
    response.shooting_available_mask = 0u;
    response.dimension_state = 0u;
    response.dimension_clear_count = 0u;
    response.claim_id = 0ULL;
    response.claim_kind = CLAIM_NONE;
    response.phase = PHASE_NONE;
    response.item_id = 0u;
    response.item_count = 0u;
    response.payment_count = 0u;
    response.source_before_count = 0u;
    response.source_slot = 0u;
    response.reward_index = 0u;
    response.physical_delivery_present = 0u;

    if (shooting_active &&
        !BuildShootingAvailability(database, request, shooting_campaign,
                                   activity_day,
                                   response.shooting_available_mask)) {
        Rollback(database);
        return false;
    }

    ShootingClaim pending_shooting;
    const int shooting_pending =
        FindPendingShooting(database, request, pending_shooting);
    DimensionState dimension_state;
    DimensionClaim pending_dimension;
    const int dimension_pending = FindPendingDimension(
        database, request, dimension_state, pending_dimension);
    if (shooting_pending < 0 || dimension_pending < 0 ||
        (shooting_pending == 1 && dimension_pending == 1)) {
        Rollback(database);
        return false;
    }

    bool have_dimension_state = dimension_pending == 1;
    if (!have_dimension_state && dimension_active) {
        bool inserted = false;
        if (!EnsureDimensionState(database, request, dimension_campaign,
                                  source_activity_day, dimension_state,
                                  inserted)) {
            Rollback(database);
            return false;
        }
        have_dimension_state = true;
        // LOAD and REFRESH both reconcile the authoritative daily serial.
        // This makes a lost reset hook or a lost clear request recover on the
        // next minute tick instead of freezing yesterday's state all day.
        if (!inserted) {
            if (dimension_state.source_day > source_activity_day) {
                Rollback(database);
                return false;
            }
            uint8_t next_state = dimension_state.state;
            uint8_t next_count = dimension_state.clear_count;
            uint32_t next_day = dimension_state.source_day;
            uint32_t next_serial = dimension_state.source_serial;
            bool changed = false;
            uint32_t delta = 0u;
            if (dimension_state.source_day == source_activity_day) {
                if (request.source_serial > dimension_state.source_serial) {
                    delta = request.source_serial -
                            dimension_state.source_serial;
                    next_serial = request.source_serial;
                    changed = true;
                }
            } else {
                next_day = source_activity_day;
                next_serial = request.source_serial;
                if (dimension_state.state <= 1u) {
                    next_state = 1u;
                    next_count = 0u;
                }
                // An existing row proves this campaign spans the authoritative
                // source generation.  LOAD/REFRESH may therefore reconstruct
                // the complete post-reset prefix after queue loss, disconnect,
                // or process restart.  A newly inserted campaign row is still
                // baselined above and never enters this branch.
                delta = request.source_serial <
                                kMaximumDimensionClearCount
                            ? request.source_serial
                            : kMaximumDimensionClearCount;
                changed = true;
            }
            if (next_state == 1u && delta != 0u) {
                const uint32_t remaining =
                    kMaximumDimensionClearCount - next_count;
                const uint32_t credited =
                    delta < remaining ? delta : remaining;
                next_count = static_cast<uint8_t>(next_count + credited);
                if (next_count == kMaximumDimensionClearCount)
                    next_state = 2u;
                changed = true;
            }
            if (changed &&
                (!database->set_query(
                     "upDate %s set event_state=%u,proper_clear_count=%u,"
                     "source_day=%u,source_serial=%u,revision=revision+1,"
                     "updated_at=now() where server_group=%u and m_id=%u "
                     "and charac_no=%u and campaign_key=%u and "
                     "pending_claim_id=0",
                     kDimensionStateTable,
                     static_cast<unsigned int>(next_state),
                     static_cast<unsigned int>(next_count),
                     static_cast<unsigned int>(next_day),
                     static_cast<unsigned int>(next_serial),
                     static_cast<unsigned int>(request.server_group),
                     static_cast<unsigned int>(request.account_id),
                     static_cast<unsigned int>(request.charac_no),
                     static_cast<unsigned int>(dimension_campaign)) ||
                 !database->exec(true) ||
                 database->getAffectedRowCount() != 1u ||
                 !ReadDimensionState(database, request, dimension_campaign,
                                     true, dimension_state))) {
                Rollback(database);
                return false;
            }
        }
    }

    if (shooting_pending == 1) {
        bool delivered = false;
        if (!PhysicalDeliveryExists(request, CLAIM_SHOOTING_REWARD,
                                    pending_shooting.claim_id,
                                    pending_shooting.item_id,
                                    pending_shooting.item_count, delivered)) {
            Rollback(database);
            return false;
        }
        response.physical_delivery_present = delivered ? 1u : 0u;
        response.claim_id = pending_shooting.claim_id;
        response.claim_kind = CLAIM_SHOOTING_REWARD;
        response.phase = PHASE_RESERVED;
        response.campaign_key = pending_shooting.campaign_key;
        response.activity_day = pending_shooting.activity_day;
        response.reward_index = pending_shooting.reward_index;
        response.item_id = pending_shooting.item_id;
        response.item_count = pending_shooting.item_count;
        response.payment_count = pending_shooting.payment_count;
        response.source_before_count =
            pending_shooting.source_before_count;
        response.source_slot = pending_shooting.source_slot;
    } else if (dimension_pending == 1) {
        if (pending_dimension.kind == CLAIM_DIMENSION_REWARD) {
            bool delivered = false;
            if (!PhysicalDeliveryExists(request, CLAIM_DIMENSION_REWARD,
                                        pending_dimension.claim_id,
                                        pending_dimension.item_id,
                                        pending_dimension.item_count,
                                        delivered)) {
                Rollback(database);
                return false;
            }
            response.physical_delivery_present = delivered ? 1u : 0u;
        }
        response.claim_id = pending_dimension.claim_id;
        response.claim_kind = pending_dimension.kind;
        response.phase = PHASE_RESERVED;
        response.campaign_key = pending_dimension.campaign_key;
        response.item_id = pending_dimension.item_id;
        response.item_count = pending_dimension.item_count;
        response.source_before_count =
            pending_dimension.source_before_count;
        response.source_slot = pending_dimension.source_slot;
    } else {
        response.campaign_key =
            have_dimension_state ? dimension_state.campaign_key : 0u;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    if (have_dimension_state) {
        response.source_serial = dimension_state.source_serial;
        response.dimension_state = dimension_state.state;
        response.dimension_clear_count = dimension_state.clear_count;
    }
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleShootingReserve(MySQL* database,
                           const Message& request,
                           Message& response) {
    if (!Begin(database)) return false;
    bool active = false;
    uint32_t campaign_key = 0u;
    uint32_t activity_day = 0u;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (!LoadCampaign(database, 720u, active, campaign_key, true) ||
        !ReadActivityDay(database, request, activity_day)) {
        Rollback(database);
        return false;
    }
    if (!active) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    const ShootingRule* const rule = Rule(request.reward_index);
    if (!rule) {
        response.result = RESULT_INVALID_ITEM;
        Rollback(database);
        return true;
    }

    ShootingClaim replay;
    const int replay_result = FindShootingRequest(database, request, replay);
    if (replay_result < 0) {
        Rollback(database);
        return false;
    }
    if (replay_result == 1) {
        if (!ValidateShootingClaim(request, replay) ||
            replay.campaign_key != campaign_key ||
            replay.activity_day != activity_day) {
            Rollback(database);
            response.result = RESULT_INVALID_STATE;
            return true;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.claim_id = replay.claim_id;
        response.campaign_key = replay.campaign_key;
        response.activity_day = replay.activity_day;
        response.phase = replay.state;
        response.result = RESULT_SUCCESS;
        return true;
    }

    const uint32_t scope_charac = rule->account_once ? 0u : request.charac_no;
    const uint32_t scope_day = rule->account_once ? 0u : activity_day;
    if (!database->set_query(
            "inSert ignore into %s(server_group,m_id,charac_no,campaign_key,"
            "activity_day,scope_charac_no,scope_day,reward_index,item_id,"
            "item_count,payment_count,source_before_count,source_slot,"
            "session_token,request_token,claim_state,created_at,updated_at,"
            "delivered_at) values("
            "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,1,now(),now(),null)",
            kShootingClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(scope_charac),
            static_cast<unsigned int>(scope_day),
            static_cast<unsigned int>(request.reward_index),
            static_cast<unsigned int>(rule->item_id),
            static_cast<unsigned int>(rule->item_count),
            static_cast<unsigned int>(rule->price),
            static_cast<unsigned int>(request.source_before_count),
            static_cast<unsigned int>(request.source_slot),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    if (database->getAffectedRowCount() != 1u) {
        ShootingClaim collided;
        const int exact = FindShootingRequest(database, request, collided);
        if (exact == 1 && ValidateShootingClaim(request, collided)) {
            if (!Commit(database)) {
                Rollback(database);
                return false;
            }
            response.claim_id = collided.claim_id;
            response.campaign_key = collided.campaign_key;
            response.activity_day = collided.activity_day;
            response.phase = collided.state;
            response.result = RESULT_SUCCESS;
            return true;
        }
        unsigned int state = 0u;
        if (exact < 0 ||
            !database->set_query(
                "seLect claim_state from %s where server_group=%u and "
                "m_id=%u and campaign_key=%u and reward_index=%u and "
                "scope_charac_no=%u and scope_day=%u limit 2",
                kShootingClaimTable,
                static_cast<unsigned int>(request.server_group),
                static_cast<unsigned int>(request.account_id),
                static_cast<unsigned int>(campaign_key),
                static_cast<unsigned int>(request.reward_index),
                static_cast<unsigned int>(scope_charac),
                static_cast<unsigned int>(scope_day)) ||
            !database->exec(true) || database->get_n_rows() != 1 ||
            !database->fetch() || !database->get_uint(0, state) ||
            state < PHASE_RESERVED || state > PHASE_DELIVERED) {
            Rollback(database);
            return false;
        }
        Rollback(database);
        response.result = state == PHASE_DELIVERED
                              ? RESULT_ALREADY_CLAIMED : RESULT_PENDING;
        return true;
    }

    ShootingClaim stored;
    if (FindShootingRequest(database, request, stored) != 1 ||
        !ValidateShootingClaim(request, stored) ||
        stored.campaign_key != campaign_key ||
        stored.activity_day != activity_day ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.claim_id = stored.claim_id;
    response.campaign_key = stored.campaign_key;
    response.activity_day = stored.activity_day;
    response.phase = PHASE_RESERVED;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleShootingCommit(MySQL* database,
                          const Message& request,
                          Message& response) {
    if (!Begin(database)) return false;
    ShootingClaim claim;
    if (!ReadShootingClaim(database, request, request.claim_id, true, claim) ||
        !ValidateShootingClaim(request, claim)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.state == PHASE_DELIVERED) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.campaign_key = claim.campaign_key;
        response.activity_day = claim.activity_day;
        response.phase = PHASE_DELIVERED;
        response.result = RESULT_SUCCESS;
        return BuildShootingAvailability(database, request,
                                         claim.campaign_key,
                                         claim.activity_day,
                                         response.shooting_available_mask);
    }
    if (claim.state != PHASE_RESERVED ||
        (claim.payment_count != 0u &&
         !inventory_mutation_barrier::HasExactPersistedStack(
             request.server_group, request.charac_no, claim.source_slot,
             kShootingPaymentItem,
             claim.source_before_count - claim.payment_count)) ||
        !DeliverReward(request) ||
        !database->set_query(
            "upDate %s set claim_state=2,updated_at=now(),delivered_at=now() "
            "where claim_id=%llu and claim_state=1",
            kShootingClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.campaign_key = claim.campaign_key;
    response.activity_day = claim.activity_day;
    response.phase = PHASE_DELIVERED;
    response.result = RESULT_SUCCESS;
    return BuildShootingAvailability(database, request, claim.campaign_key,
                                     claim.activity_day,
                                     response.shooting_available_mask);
}

bool HandleShootingCancel(MySQL* database,
                          const Message& request,
                          Message& response) {
    if (!Begin(database)) return false;
    ShootingClaim claim;
    if (!ReadShootingClaim(database, request, request.claim_id, true, claim) ||
        !ValidateShootingClaim(request, claim) ||
        claim.state != PHASE_RESERVED) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    // Recheck physical delivery while the DB2 claim row is locked.  A LOAD
    // snapshot that observed "not delivered" can race a delayed old COMMIT;
    // deleting the claim after that mail lands would reopen the reward scope.
    bool delivered = false;
    if (!PhysicalDeliveryExists(request, CLAIM_SHOOTING_REWARD,
                                claim.claim_id, claim.item_id,
                                claim.item_count, delivered)) {
        Rollback(database);
        return false;
    }
    if (delivered) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.physical_delivery_present = 1u;
        response.phase = PHASE_RESERVED;
        response.result = RESULT_PENDING;
        return true;
    }
    if (!database->set_query(
            "deLete from %s where claim_id=%llu and claim_state=1",
            kShootingClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.phase = PHASE_NONE;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleDimensionProgress(MySQL* database,
                             const Message& request,
                             Message& response,
                             bool seed_only) {
    if (!Begin(database)) return false;
    bool active = false;
    uint32_t campaign_key = 0u;
    uint32_t current_activity_day = 0u;
    uint32_t activity_day = 0u;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (!LoadCampaign(database, 721u, active, campaign_key, true) ||
        !ReadSourceActivityDay(database, request, current_activity_day,
                               activity_day)) {
        Rollback(database);
        return false;
    }
    (void)current_activity_day;
    if (!active) {
        Rollback(database);
        response.dimension_active = 0u;
        response.dimension_state = 0u;
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (request.campaign_key != campaign_key) {
        Rollback(database);
        response.campaign_key = campaign_key;
        response.dimension_active = 0u;
        response.dimension_state = 0u;
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    DimensionState state;
    bool inserted = false;
    if (!EnsureDimensionState(database, request, campaign_key, activity_day,
                              state, inserted)) {
        Rollback(database);
        return false;
    }

    uint32_t next_serial = state.source_serial;
    uint32_t next_day = state.source_day;
    uint8_t next_state = state.state;
    uint8_t next_count = state.clear_count;
    bool changed = false;
    if (seed_only) {
        if (!inserted && state.source_day > activity_day) {
            Rollback(database);
            return false;
        }
        if (state.source_day != activity_day) {
            next_day = activity_day;
            next_serial = request.source_serial;
            changed = true;
        } else if (request.source_serial > state.source_serial) {
            next_serial = request.source_serial;
            changed = true;
        }
        if ((inserted || state.source_day != activity_day ||
             request.source_serial >= state.source_serial) &&
            next_state <= 1u) {
            // SEED is emitted only by the authoritative reset latch.  It is
            // therefore an exact, idempotent normalization even when a claim
            // recovery LOAD already adopted this day's high-water serial.
            const uint32_t credited =
                request.source_serial < kMaximumDimensionClearCount
                    ? request.source_serial
                    : kMaximumDimensionClearCount;
            if (state.source_day == activity_day &&
                next_count > credited) {
                Rollback(database);
                return false;
            }
            const uint8_t normalized_count =
                static_cast<uint8_t>(credited);
            const uint8_t normalized_state =
                normalized_count == kMaximumDimensionClearCount ? 2u : 1u;
            if (next_count != normalized_count ||
                next_state != normalized_state) {
                next_count = normalized_count;
                next_state = normalized_state;
                changed = true;
            }
        }
    } else if (inserted) {
        // A real clear callback may race the first state creation.  It earns
        // exactly that clear, never the pre-existing serial prefix.
        if (state.state == 1u && request.source_serial != 0u) {
            next_count = 1u;
            changed = true;
        }
    } else {
        uint32_t delta = 0u;
        if (state.source_day > activity_day) {
            Rollback(database);
            return false;
        }
        if (state.source_day == activity_day) {
            if (request.source_serial > state.source_serial) {
                delta = request.source_serial - state.source_serial;
                next_serial = request.source_serial;
                changed = true;
            }
        } else {
            // The canonical reset hook makes the first eligible serial one.
            // Any larger first observation is seeded without backfill.
            next_day = activity_day;
            next_serial = request.source_serial;
            if (state.state <= 1u) {
                next_state = 1u;
                next_count = 0u;
            }
            delta = request.source_serial == 1u ? 1u : 0u;
            changed = true;
        }
        if (next_state == 1u && delta != 0u) {
            const uint32_t remaining =
                kMaximumDimensionClearCount - next_count;
            const uint32_t credited = delta < remaining ? delta : remaining;
            next_count = static_cast<uint8_t>(next_count + credited);
            if (next_count == kMaximumDimensionClearCount) next_state = 2u;
            changed = true;
        }
    }
    if (changed &&
        (!database->set_query(
             "upDate %s set event_state=%u,proper_clear_count=%u,"
             "source_day=%u,source_serial=%u,revision=revision+1,"
             "updated_at=now() where server_group=%u and m_id=%u and "
             "charac_no=%u and campaign_key=%u and pending_claim_id=0",
             kDimensionStateTable,
             static_cast<unsigned int>(next_state),
             static_cast<unsigned int>(next_count),
             static_cast<unsigned int>(next_day),
             static_cast<unsigned int>(next_serial),
             static_cast<unsigned int>(request.server_group),
             static_cast<unsigned int>(request.account_id),
             static_cast<unsigned int>(request.charac_no),
             static_cast<unsigned int>(campaign_key)) ||
         !database->exec(true) || database->getAffectedRowCount() != 1u)) {
        Rollback(database);
        return false;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.campaign_key = campaign_key;
    response.activity_day = activity_day;
    response.source_serial = next_serial;
    response.dimension_active = 1u;
    response.dimension_state = next_state;
    response.dimension_clear_count = next_count;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleDimensionReserve(MySQL* database,
                            const Message& request,
                            Message& response,
                            uint8_t kind) {
    if (!Begin(database)) return false;
    bool active = false;
    uint32_t campaign_key = 0u;
    uint32_t activity_day = 0u;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (!LoadCampaign(database, 721u, active, campaign_key, true) ||
        !ReadActivityDay(database, request, activity_day)) {
        Rollback(database);
        return false;
    }
    if (!active) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (request.campaign_key != campaign_key) {
        Rollback(database);
        response.campaign_key = campaign_key;
        response.dimension_active = 0u;
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    DimensionState state;
    bool inserted = false;
    if (!EnsureDimensionState(database, request, campaign_key, activity_day,
                              state, inserted)) {
        Rollback(database);
        return false;
    }
    DimensionClaim replay;
    const int replay_result = FindDimensionRequest(database, request, replay);
    if (replay_result < 0) {
        Rollback(database);
        return false;
    }
    if (replay_result == 1) {
        if (replay.kind != kind || replay.campaign_key != campaign_key ||
            replay.item_id != request.item_id ||
            replay.item_count != request.item_count ||
            replay.source_before_count != request.source_before_count ||
            replay.source_slot != request.source_slot ||
            state.pending_claim_id != replay.claim_id) {
            Rollback(database);
            response.result = RESULT_INVALID_STATE;
            return true;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.claim_id = replay.claim_id;
        response.campaign_key = replay.campaign_key;
        response.phase = replay.state;
        response.dimension_state = state.state;
        response.dimension_clear_count = state.clear_count;
        response.result = RESULT_SUCCESS;
        return true;
    }
    const uint8_t required_state =
        kind == CLAIM_DIMENSION_MATERIAL ? 2u : 3u;
    if (state.state != required_state || state.pending_claim_id != 0ULL) {
        Rollback(database);
        response.result = state.pending_claim_id != 0ULL
                              ? RESULT_PENDING : RESULT_INVALID_STATE;
        return true;
    }
    if (!database->set_query(
            "inSert into %s(server_group,m_id,charac_no,campaign_key,"
            "claim_kind,item_id,item_count,source_before_count,source_slot,"
            "session_token,request_token,claim_state,created_at,updated_at,"
            "delivered_at) values("
            "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,1,now(),now(),null)",
            kDimensionClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(kind),
            static_cast<unsigned int>(request.item_id),
            static_cast<unsigned int>(request.item_count),
            static_cast<unsigned int>(request.source_before_count),
            static_cast<unsigned int>(request.source_slot),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    DimensionClaim stored;
    if (FindDimensionRequest(database, request, stored) != 1 ||
        stored.kind != kind || stored.campaign_key != campaign_key ||
        stored.item_id != request.item_id ||
        stored.item_count != request.item_count ||
        stored.source_before_count != request.source_before_count ||
        stored.source_slot != request.source_slot ||
        !database->set_query(
            "upDate %s set pending_claim_id=%llu,revision=revision+1,"
            "updated_at=now() where server_group=%u and m_id=%u and "
            "charac_no=%u and campaign_key=%u and pending_claim_id=0 and "
            "event_state=%u",
            kDimensionStateTable,
            static_cast<unsigned long long>(stored.claim_id),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(required_state)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.claim_id = stored.claim_id;
    response.campaign_key = campaign_key;
    response.phase = PHASE_RESERVED;
    response.dimension_state = state.state;
    response.dimension_clear_count = state.clear_count;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleDimensionCommit(MySQL* database,
                           const Message& request,
                           Message& response,
                           uint8_t kind) {
    if (!Begin(database)) return false;
    DimensionClaim claim;
    if (!ReadDimensionClaim(database, request, request.claim_id, true, claim) ||
        claim.campaign_key != request.campaign_key || claim.kind != kind ||
        claim.item_id != request.item_id ||
        claim.item_count != request.item_count ||
        claim.source_before_count != request.source_before_count ||
        claim.source_slot != request.source_slot) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    DimensionState state;
    if (!ReadDimensionState(database, request, claim.campaign_key, true,
                            state)) {
        Rollback(database);
        return false;
    }
    if (claim.state == PHASE_DELIVERED) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        response.campaign_key = claim.campaign_key;
        response.phase = PHASE_DELIVERED;
        response.dimension_state = state.state;
        response.dimension_clear_count = state.clear_count;
        response.result = RESULT_SUCCESS;
        return true;
    }
    const uint8_t required_state =
        kind == CLAIM_DIMENSION_MATERIAL ? 2u : 3u;
    const uint8_t next_state =
        kind == CLAIM_DIMENSION_MATERIAL ? 3u : 0u;
    if (claim.state != PHASE_RESERVED ||
        state.pending_claim_id != claim.claim_id ||
        state.state != required_state ||
        (kind == CLAIM_DIMENSION_MATERIAL &&
         !inventory_mutation_barrier::HasExactPersistedStack(
             request.server_group, request.charac_no, claim.source_slot,
             claim.item_id, claim.source_before_count - claim.item_count)) ||
        (kind == CLAIM_DIMENSION_REWARD && !DeliverReward(request)) ||
        !database->set_query(
            "upDate %s set claim_state=2,updated_at=now(),delivered_at=now() "
            "where claim_id=%llu and claim_state=1",
            kDimensionClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set event_state=%u,pending_claim_id=0,"
            "revision=revision+1,updated_at=now() where server_group=%u and "
            "m_id=%u and charac_no=%u and campaign_key=%u and "
            "pending_claim_id=%llu and event_state=%u",
            kDimensionStateTable,
            static_cast<unsigned int>(next_state),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(claim.campaign_key),
            static_cast<unsigned long long>(claim.claim_id),
            static_cast<unsigned int>(required_state)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.campaign_key = claim.campaign_key;
    response.phase = PHASE_DELIVERED;
    response.dimension_state = next_state;
    response.dimension_clear_count = state.clear_count;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleDimensionCancel(MySQL* database,
                           const Message& request,
                           Message& response) {
    if (!Begin(database)) return false;
    DimensionClaim claim;
    if (!ReadDimensionClaim(database, request, request.claim_id, true, claim) ||
        claim.campaign_key != request.campaign_key ||
        claim.kind != CLAIM_DIMENSION_MATERIAL ||
        claim.state != PHASE_RESERVED || claim.item_id != request.item_id ||
        claim.item_count != request.item_count ||
        claim.source_before_count != request.source_before_count ||
        claim.source_slot != request.source_slot) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    DimensionState state;
    if (!ReadDimensionState(database, request, claim.campaign_key, true,
                            state) ||
        state.pending_claim_id != claim.claim_id || state.state != 2u ||
        !database->set_query(
            "upDate %s set pending_claim_id=0,revision=revision+1,"
            "updated_at=now() where server_group=%u and m_id=%u and "
            "charac_no=%u and campaign_key=%u and pending_claim_id=%llu",
            kDimensionStateTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(claim.campaign_key),
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "deLete from %s where claim_id=%llu and claim_state=1",
            kDimensionClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    response.campaign_key = claim.campaign_key;
    response.phase = PHASE_NONE;
    response.dimension_state = state.state;
    response.dimension_clear_count = state.clear_count;
    response.result = RESULT_SUCCESS;
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

}  // namespace

DBShootingStarDimensionPocket::DBShootingStarDimensionPocket()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBShootingStarDimensionPocket::~DBShootingStarDimensionPocket() {}

bool IsLegacyFencedInventoryCommit(const Message& request) {
    return IsValidRequest(request) && request.local_mutation_done == 1u &&
           ((request.operation == OPERATION_SHOOTING_COMMIT &&
             request.payment_count != 0u) ||
            request.operation == OPERATION_DIMENSION_COMMIT_MATERIAL);
}

bool DBShootingStarDimensionPocket::QueueRequest(int user_idx,
                                             const Message& request) {
    if (user_idx < 0 || !IsValidRequest(request)) return false;
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, kTaiwanInnerCode, user_idx);
    Message* const destination = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!destination) return false;
    *destination = request;
    if (IsLegacyFencedInventoryCommit(request)) {
        return legacy_feature_fence::QueueAfterLegacy(
            legacy_feature_fence::
                kFenceTwEvent720721InventoryCommit,
            guard);
    }
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBShootingStarDimensionPocket::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    if (user_idx < 0 || !stream ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return 0;
    }
    Message request;
    if (stream->get_binary(&request, static_cast<int>(sizeof(request))) != 1 ||
        !IsValidRequest(request)) {
        return 0;
    }
    Message response = request;
    response.result = RESULT_DATABASE_FAILURE;
    MySQL* const database = GetPartitionDatabase(2, request.server_group);
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD:
                handled = HandleSnapshot(database, request, response, true);
                break;
            case OPERATION_REFRESH:
                handled = HandleSnapshot(database, request, response, false);
                break;
            case OPERATION_SHOOTING_RESERVE:
                handled = HandleShootingReserve(database, request, response);
                break;
            case OPERATION_SHOOTING_COMMIT:
                handled = HandleShootingCommit(database, request, response);
                break;
            case OPERATION_SHOOTING_CANCEL:
                handled = HandleShootingCancel(database, request, response);
                break;
            case OPERATION_DIMENSION_CLEAR:
                handled = HandleDimensionProgress(database, request, response,
                                                  false);
                break;
            case OPERATION_DIMENSION_SEED:
                handled = HandleDimensionProgress(database, request, response,
                                                  true);
                break;
            case OPERATION_DIMENSION_RESERVE_MATERIAL:
                handled = HandleDimensionReserve(
                    database, request, response, CLAIM_DIMENSION_MATERIAL);
                break;
            case OPERATION_DIMENSION_COMMIT_MATERIAL:
                handled = HandleDimensionCommit(
                    database, request, response, CLAIM_DIMENSION_MATERIAL);
                break;
            case OPERATION_DIMENSION_CANCEL_MATERIAL:
                handled = HandleDimensionCancel(database, request, response);
                break;
            case OPERATION_DIMENSION_RESERVE_REWARD:
                handled = HandleDimensionReserve(
                    database, request, response, CLAIM_DIMENSION_REWARD);
                break;
            case OPERATION_DIMENSION_COMMIT_REWARD:
                handled = HandleDimensionCommit(
                    database, request, response, CLAIM_DIMENSION_REWARD);
                break;
            default:
                break;
        }
    }
    if (!handled) response.result = RESULT_DATABASE_FAILURE;
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace tw_event_720_721_db_0828
