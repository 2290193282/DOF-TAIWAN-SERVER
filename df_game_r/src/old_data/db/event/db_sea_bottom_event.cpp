#include "old_data/db/event/db_sea_bottom_event.h"

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

namespace sea_bottom_event_db {
namespace {

const char kCampaignTable[] = "sea_bottom_campaign_0828";
const char kStateTable[] = "sea_bottom_state_0828";
const char kMoveTable[] = "sea_bottom_move_0828";
const char kPendingTable[] = "sea_bottom_pending_0828";
const uint64_t kMailNamespace = 0xD832700000000000ULL;
const uint64_t kMaximumMoveId = 0x0000FFFFFFFFFFFFULL;

struct BoxRule {
    uint32_t item_id;
    int16_t weight;
};

struct RewardRule {
    uint32_t weight;
    uint32_t item_id;
    uint32_t count;
};

const BoxRule kBoxes[15] = {
    {690000854u, 1},  {690000855u, 2},  {690000856u, 4},
    {690000857u, 5},  {690000858u, 10}, {690000859u, 11},
    {690000860u, 12}, {690000861u, 14}, {690000862u, 15},
    {690000863u, 20}, {690000864u, 30}, {690000865u, 45},
    {690000866u, -1}, {690000867u, -5}, {690000868u, -10},
};

const RewardRule kNormalRewards[9] = {
    {1750u, 3108u, 1u},    {499u, 7249u, 1u},
    {499u, 7248u, 1u},     {1750u, 3296u, 1u},
    {1500u, 7252u, 1u},    {1500u, 7251u, 1u},
    {1000u, 2600032u, 1u}, {1500u, 3297u, 1u},
    {2u, 2651113u, 1u},
};

const RewardRule kSpecifiedRewards[kSpecifiedRewardCount] = {
    {0u, 690001042u, 1u}, {0u, 15u, 2u},
    {0u, 690001046u, 1u}, {0u, 15u, 1u},
    {0u, 10u, 15u},       {0u, 21u, 1u},
    {0u, 690001047u, 1u}, {0u, 15u, 3u},
    {0u, 690001041u, 1u}, {0u, 7456u, 1u},
    {0u, 690001044u, 1u}, {0u, 690001045u, 1u},
    {0u, 900u, 2u},       {0u, 690001054u, 1u},
    {0u, 690000301u, 1u}, {0u, 690000651u, 1u},
    {0u, 690001054u, 2u}, {0u, 690001043u, 1u},
    {0u, 21u, 2u},        {0u, 690000650u, 1u},
};

struct SeaState {
    SeaState()
        : campaign_key(0u), revision(0u), current_depth(0u),
          claimed_mask(0u), pending_move_id(0ULL) {}
    uint32_t campaign_key;
    uint32_t revision;
    uint32_t current_depth;
    uint32_t claimed_mask;
    uint64_t pending_move_id;
};

struct MoveRecord {
    MoveRecord()
        : move_id(0ULL), campaign_key(0u), revision_before(0u),
          depth_before(0u), target_depth(0u), claimed_before(0u),
          claimed_after(0u), box_item_id(0u), box_count_before(0u),
          box_total_before(0u), reward_item_id(0u), reward_count(0u),
          source_slot(0u), box_weight(0), rollover(0u), state(0u) {
        std::memset(inventory_pre_sha256, 0, sizeof(inventory_pre_sha256));
        std::memset(inventory_post_sha256, 0, sizeof(inventory_post_sha256));
        std::memset(&mail_reward, 0, sizeof(mail_reward));
        std::memset(mail_title, 0, sizeof(mail_title));
        std::memset(mail_content, 0, sizeof(mail_content));
    }
    uint64_t move_id;
    uint32_t campaign_key;
    uint32_t revision_before;
    uint32_t depth_before;
    uint32_t target_depth;
    uint32_t claimed_before;
    uint32_t claimed_after;
    uint32_t box_item_id;
    uint32_t box_count_before;
    uint32_t box_total_before;
    uint32_t reward_item_id;
    uint32_t reward_count;
    uint16_t source_slot;
    int16_t box_weight;
    uint8_t rollover;
    uint8_t state;
    uint8_t inventory_pre_sha256[32];
    uint8_t inventory_post_sha256[32];
    MailReward mail_reward;
    char mail_title[kMailTitleBytes + 1];
    char mail_content[kMailContentBytes + 1];
};

struct PendingRecord {
    PendingRecord() : campaign_key(0u), move_id(0ULL) {}
    uint32_t campaign_key;
    uint64_t move_id;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

const BoxRule* FindBox(uint32_t item_id) {
    for (unsigned int index = 0u; index < 15u; ++index) {
        if (kBoxes[index].item_id == item_id) return &kBoxes[index];
    }
    return 0;
}

bool IsNormalReward(uint32_t item_id, uint32_t count) {
    for (unsigned int index = 0u; index < 9u; ++index) {
        if (kNormalRewards[index].item_id == item_id &&
            kNormalRewards[index].count == count) {
            return true;
        }
    }
    return false;
}

bool IsSpecifiedClaim(uint32_t target_depth, uint32_t claimed_mask,
                      uint32_t& index) {
    index = 0u;
    if (target_depth == 0u || target_depth % 100u != 0u) return false;
    index = target_depth / 100u - 1u;
    return index < kSpecifiedRewardCount &&
           (claimed_mask & (1u << index)) == 0u;
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

bool HasNoMail(const Message& message) {
    return IsZeroFilled(&message.mail_reward, sizeof(message.mail_reward)) &&
           IsZeroFilled(message.mail_title, sizeof(message.mail_title)) &&
           IsZeroFilled(message.mail_content, sizeof(message.mail_content));
}

bool HasExactMailProjection(uint32_t reward_item_id,
                            uint32_t reward_count,
                            const MailReward& reward,
                            const char* title,
                            unsigned int title_size,
                            const char* content,
                            unsigned int content_size) {
    return reward.item_id == reward_item_id &&
           reward.add_info == reward_count &&
           reward.upgrade == (reward.item_attr & 0x1fu) &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u &&
           reward.reserved == 0u &&
           IsCanonicalText(title, title_size) &&
           IsCanonicalText(content, content_size);
}

bool HasExactMail(const Message& message) {
    return HasExactMailProjection(
        message.reward_item_id, message.reward_count, message.mail_reward,
        message.mail_title, sizeof(message.mail_title), message.mail_content,
        sizeof(message.mail_content));
}

bool HasInventoryProof(const Message& message) {
    return !IsZeroFilled(message.inventory_pre_sha256,
                         sizeof(message.inventory_pre_sha256)) &&
           !IsZeroFilled(message.inventory_post_sha256,
                         sizeof(message.inventory_post_sha256)) &&
           std::memcmp(message.inventory_pre_sha256,
                       message.inventory_post_sha256,
                       sizeof(message.inventory_pre_sha256)) != 0;
}

bool HasValidMoveShape(const Message& message,
                       bool unresolved_normal_reward) {
    const BoxRule* const box = FindBox(message.box_item_id);
    if (!box || box->weight != message.box_weight ||
        message.source_slot >= 352u || message.box_count_before == 0u ||
        message.box_count_before >
            static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        message.box_total_before < message.box_count_before ||
        message.box_total_before >
            static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        message.current_depth >= kMaximumDepth ||
        message.target_depth > kMaximumDepth ||
        (message.claimed_mask & ~static_cast<uint32_t>(kClaimedMask)) != 0u ||
        (message.claimed_mask_after &
         ~static_cast<uint32_t>(kClaimedMask)) != 0u ||
        static_cast<int>(message.current_depth) + box->weight !=
            static_cast<int>(message.target_depth) ||
        message.rollover !=
            (message.target_depth == kMaximumDepth ? 1u : 0u) ||
        !HasInventoryProof(message)) {
        return false;
    }
    uint32_t specified_index = 0u;
    if (IsSpecifiedClaim(message.target_depth, message.claimed_mask,
                         specified_index)) {
        const uint32_t bit = 1u << specified_index;
        return message.claimed_mask_after == (message.claimed_mask | bit) &&
               message.reward_item_id ==
                   kSpecifiedRewards[specified_index].item_id &&
               message.reward_count ==
                   kSpecifiedRewards[specified_index].count;
    }
    if (message.claimed_mask_after != message.claimed_mask) return false;
    return unresolved_normal_reward
               ? message.reward_item_id == 0u && message.reward_count == 0u
               : IsNormalReward(message.reward_item_id,
                                message.reward_count);
}

bool HasNoMove(const Message& message) {
    return message.move_id == 0ULL && message.target_depth == 0u &&
           message.claimed_mask_after == 0u && message.box_item_id == 0u &&
           message.box_count_before == 0u && message.box_total_before == 0u &&
           message.reward_item_id == 0u &&
           message.reward_count == 0u && message.source_slot == 0u &&
           message.box_weight == 0 && message.local_mutation_done == 0u &&
           message.rollover == 0u && message.client_reply_expected == 0u &&
           message.recovery_action == 0u &&
           IsZeroFilled(message.inventory_pre_sha256,
                        sizeof(message.inventory_pre_sha256)) &&
           IsZeroFilled(message.inventory_post_sha256,
                        sizeof(message.inventory_post_sha256)) &&
           HasNoMail(message);
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.request_token == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.operation < OPERATION_LOAD ||
        message.operation > OPERATION_PREPARE_MAIL || message.result != 0u ||
        message.phase != 0u || message.active != 0u ||
        message.client_reply_expected > 1u ||
        message.recovery_action > 2u ||
        (message.claimed_mask & ~static_cast<uint32_t>(kClaimedMask)) != 0u) {
        return false;
    }
    switch (message.operation) {
        case OPERATION_LOAD:
        case OPERATION_REFRESH:
            return HasNoMove(message);
        case OPERATION_RESERVE:
            return message.move_id == 0ULL &&
                   message.local_mutation_done == 0u &&
                   message.client_reply_expected == 1u &&
                   message.recovery_action == 0u &&
                    HasNoMail(message) && HasValidMoveShape(message, true);
        case OPERATION_PREPARE_MAIL:
            return message.move_id != 0ULL &&
                   message.move_id <= kMaximumMoveId &&
                   message.local_mutation_done == 1u &&
                   (message.recovery_action == 0u ||
                    message.recovery_action == 1u) &&
                   HasExactMail(message) &&
                   HasValidMoveShape(message, false);
        case OPERATION_COMMIT:
            return message.move_id != 0ULL &&
                   message.move_id <= kMaximumMoveId &&
                   message.local_mutation_done == 1u &&
                   (message.recovery_action == 0u ||
                    message.recovery_action == 1u) &&
                    HasExactMail(message) && HasValidMoveShape(message, false);
        case OPERATION_CANCEL:
            return message.move_id != 0ULL &&
                   message.move_id <= kMaximumMoveId &&
                   message.local_mutation_done == 0u &&
                   (message.recovery_action == 0u ||
                    message.recovery_action == 2u) &&
                    HasNoMail(message) && HasValidMoveShape(message, false);
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
    if (database && database->set_query("rollback"))
        (void)database->exec(true);
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

bool LoadCampaign(MySQL* database,
                  bool& active,
                  uint32_t& campaign_key) {
    active = false;
    campaign_key = 0u;
    unsigned int enabled = 0u;
    unsigned int in_window = 0u;
    return database &&
           database->set_query(
               "seLect enabled,campaign_key,(start_at<=now() and "
               "end_at>now()) from %s where campaign_id=1 limit 2 for update",
               kCampaignTable) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, enabled) &&
           database->get_uint(1, campaign_key) &&
           database->get_uint(2, in_window) && enabled <= 1u &&
           in_window <= 1u &&
           (active = enabled == 1u && in_window == 1u && campaign_key != 0u,
            true);
}

bool ReadState(MySQL* database,
               const Message& request,
               uint32_t campaign_key,
               bool for_update,
               SeaState& state) {
    return database && campaign_key != 0u &&
           database->set_query(
               "seLect campaign_key,revision,current_depth,claimed_mask,"
               "pending_move_id from %s where server_group=%u and m_id=%u "
               "and charac_no=%u and campaign_key=%u %s",
               kStateTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(campaign_key),
               for_update ? "for update" : "limit 2") &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() &&
           database->get_uint(0, state.campaign_key) &&
           database->get_uint(1, state.revision) &&
           database->get_uint(2, state.current_depth) &&
           database->get_uint(3, state.claimed_mask) &&
           database->get_ulonglong(4, state.pending_move_id) &&
           state.campaign_key == campaign_key &&
           state.current_depth < kMaximumDepth &&
           (state.claimed_mask & ~static_cast<uint32_t>(kClaimedMask)) == 0u &&
           state.pending_move_id <= kMaximumMoveId;
}

bool EnsureState(MySQL* database,
                 const Message& request,
                 uint32_t campaign_key,
                 SeaState& state) {
    return database && campaign_key != 0u &&
           database->set_query(
               "inSert ignore into %s(server_group,m_id,charac_no,"
               "campaign_key,revision,current_depth,claimed_mask,"
               "pending_move_id,created_at,updated_at) values("
               "%u,%u,%u,%u,0,0,0,0,now(),now())",
               kStateTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(campaign_key)) &&
           database->exec(true) &&
           ReadState(database, request, campaign_key, true, state);
}

int FindPendingState(MySQL* database,
                     const Message& request,
                     SeaState& state) {
    if (!database ||
        !database->set_query(
            "seLect campaign_key,revision,current_depth,claimed_mask,"
            "pending_move_id from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and pending_move_id<>0 order by campaign_key "
            "limit 2 for update",
            kStateTable, static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, state.campaign_key) ||
        !database->get_uint(1, state.revision) ||
        !database->get_uint(2, state.current_depth) ||
        !database->get_uint(3, state.claimed_mask) ||
        !database->get_ulonglong(4, state.pending_move_id) ||
        state.campaign_key == 0u || state.current_depth >= kMaximumDepth ||
        (state.claimed_mask & ~static_cast<uint32_t>(kClaimedMask)) != 0u ||
        state.pending_move_id == 0ULL ||
        state.pending_move_id > kMaximumMoveId) {
        return -1;
    }
    return 1;
}

int ReadPendingGuard(MySQL* database,
                     const Message& request,
                     PendingRecord& pending) {
    if (!database ||
        !database->set_query(
            "seLect campaign_key,move_id from %s where server_group=%u "
            "and m_id=%u and charac_no=%u limit 2 for update",
            kPendingTable, static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, pending.campaign_key) ||
        !database->get_ulonglong(1, pending.move_id) ||
        pending.campaign_key == 0u || pending.move_id == 0ULL ||
        pending.move_id > kMaximumMoveId) {
        return -1;
    }
    return 1;
}

int FindPendingJournal(MySQL* database,
                       const Message& request,
                       SeaState& state) {
    PendingRecord pending;
    const int state_result = FindPendingState(database, request, state);
    if (state_result < 0) return -1;
    const int guard_result = ReadPendingGuard(database, request, pending);
    if (guard_result < 0 || state_result != guard_result) return -1;
    if (state_result == 1 &&
        (state.campaign_key != pending.campaign_key ||
         state.pending_move_id != pending.move_id)) {
        return -1;
    }
    return state_result;
}

bool InsertPendingGuard(MySQL* database,
                        const Message& request,
                        uint32_t campaign_key,
                        uint64_t move_id) {
    return database && campaign_key != 0u && move_id != 0ULL &&
           move_id <= kMaximumMoveId &&
           database->set_query(
               "inSert into %s(server_group,m_id,charac_no,campaign_key,"
               "move_id,created_at) values(%u,%u,%u,%u,%llu,now())",
               kPendingTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(campaign_key),
               static_cast<unsigned long long>(move_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool DeletePendingGuard(MySQL* database,
                        const Message& request,
                        uint32_t campaign_key,
                        uint64_t move_id) {
    return database &&
           database->set_query(
               "deLete from %s where server_group=%u and m_id=%u and "
               "charac_no=%u and campaign_key=%u and move_id=%llu",
               kPendingTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(campaign_key),
               static_cast<unsigned long long>(move_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool SelectNormalReward(MySQL* database,
                        uint32_t& item_id,
                        uint32_t& count) {
    item_id = 0u;
    count = 0u;
    unsigned int roll = 0u;
    if (!database ||
        !database->set_query("seLect floor(rand()*10000)") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, roll) || roll >= 10000u) {
        return false;
    }
    uint32_t cumulative = 0u;
    for (unsigned int index = 0u; index < 9u; ++index) {
        cumulative += kNormalRewards[index].weight;
        if (roll < cumulative) {
            item_id = kNormalRewards[index].item_id;
            count = kNormalRewards[index].count;
            return true;
        }
    }
    return false;
}

bool ReadMove(MySQL* database,
              const Message& request,
              uint64_t move_id,
              bool for_update,
              MoveRecord& move) {
    unsigned int source_slot = 0u;
    int box_weight = 0;
    unsigned int rollover = 0u;
    unsigned int state = 0u;
    unsigned int title_length = 0u;
    unsigned int content_length = 0u;
    if (!database || move_id == 0ULL || move_id > kMaximumMoveId ||
        !database->set_query(
            "seLect move_id,campaign_key,revision_before,depth_before,"
            "target_depth,claimed_before,claimed_after,box_item_id,"
            "box_count_before,box_total_before,reward_item_id,reward_count,"
            "source_slot,box_weight,rollover,move_state,"
            "inventory_pre_sha256,inventory_post_sha256,mail_item_id,"
            "mail_add_info,mail_extend_info,mail_chronicle_value,"
            "mail_endurance,mail_amplify_value,mail_seal_flag,"
            "mail_item_attr,mail_upgrade,mail_amplify_option,mail_item_guid,"
            "mail_random_option,mail_separate_upgrade,mail_avatar_flag,"
            "mail_creature_flag,mail_reserved,length(mail_title),mail_title,"
            "length(mail_content),mail_content from %s where move_id=%llu "
            "and server_group=%u and m_id=%u and charac_no=%u %s",
            kMoveTable, static_cast<unsigned long long>(move_id),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            for_update ? "for update" : "limit 2") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_ulonglong(0, move.move_id) ||
        !database->get_uint(1, move.campaign_key) ||
        !database->get_uint(2, move.revision_before) ||
        !database->get_uint(3, move.depth_before) ||
        !database->get_uint(4, move.target_depth) ||
        !database->get_uint(5, move.claimed_before) ||
        !database->get_uint(6, move.claimed_after) ||
        !database->get_uint(7, move.box_item_id) ||
        !database->get_uint(8, move.box_count_before) ||
        !database->get_uint(9, move.box_total_before) ||
        !database->get_uint(10, move.reward_item_id) ||
        !database->get_uint(11, move.reward_count) ||
        !database->get_uint(12, source_slot) ||
        !database->get_int(13, box_weight) ||
        !database->get_uint(14, rollover) ||
        !database->get_uint(15, state) ||
        database->get_binary_length(16) != 32 ||
        !database->get_binary(16, move.inventory_pre_sha256, 32) ||
        database->get_binary_length(17) != 32 ||
        !database->get_binary(17, move.inventory_post_sha256, 32) ||
        !database->get_uint(18, move.mail_reward.item_id) ||
        !database->get_uint(19, move.mail_reward.add_info) ||
        !database->get_uint(20, move.mail_reward.extend_info) ||
        !database->get_uint(21, move.mail_reward.chronicle_value) ||
        !database->get_ushort(22, move.mail_reward.endurance) ||
        !database->get_ushort(23, move.mail_reward.amplify_value) ||
        !database->get_ubyte(24, move.mail_reward.seal_flag) ||
        !database->get_ubyte(25, move.mail_reward.item_attr) ||
        !database->get_ubyte(26, move.mail_reward.upgrade) ||
        !database->get_ubyte(27, move.mail_reward.amplify_option) ||
        database->get_binary_length(28) !=
            static_cast<int>(sizeof(move.mail_reward.item_guid)) ||
        !database->get_binary(28, move.mail_reward.item_guid,
                              sizeof(move.mail_reward.item_guid)) ||
        database->get_binary_length(29) !=
            static_cast<int>(sizeof(move.mail_reward.random_option)) ||
        !database->get_binary(29, move.mail_reward.random_option,
                              sizeof(move.mail_reward.random_option)) ||
        !database->get_ubyte(30, move.mail_reward.separate_upgrade) ||
        !database->get_ubyte(31, move.mail_reward.avatar_flag) ||
        !database->get_ubyte(32, move.mail_reward.creature_flag) ||
        !database->get_ubyte(33, move.mail_reward.reserved) ||
        !database->get_uint(34, title_length) ||
        !database->get_str(35, move.mail_title, sizeof(move.mail_title)) ||
        !database->get_uint(36, content_length) ||
        !database->get_str(37, move.mail_content,
                           sizeof(move.mail_content))) {
        return false;
    }
    move.source_slot = static_cast<uint16_t>(source_slot);
    move.box_weight = static_cast<int16_t>(box_weight);
    move.rollover = static_cast<uint8_t>(rollover);
    move.state = static_cast<uint8_t>(state);
    if (move.move_id != move_id || source_slot >= 352u || box_weight < -10 ||
        box_weight > 45 || rollover > 1u ||
        (state != PHASE_RESERVED && state != PHASE_DELIVERED &&
         state != PHASE_CANCELLED && state != PHASE_MAIL_PREPARED) ||
        title_length != std::strlen(move.mail_title) ||
        content_length != std::strlen(move.mail_content)) {
        return false;
    }
    if (IsZeroFilled(move.inventory_pre_sha256,
                     sizeof(move.inventory_pre_sha256)) ||
        IsZeroFilled(move.inventory_post_sha256,
                     sizeof(move.inventory_post_sha256)) ||
        std::memcmp(move.inventory_pre_sha256,
                    move.inventory_post_sha256, 32u) == 0) {
        return false;
    }
    const bool has_projection =
        state == PHASE_MAIL_PREPARED || state == PHASE_DELIVERED;
    if (has_projection) {
        return HasExactMailProjection(
            move.reward_item_id, move.reward_count, move.mail_reward,
            move.mail_title, sizeof(move.mail_title), move.mail_content,
            sizeof(move.mail_content));
    }
    return IsZeroFilled(&move.mail_reward, sizeof(move.mail_reward)) &&
           IsZeroFilled(move.mail_title, sizeof(move.mail_title)) &&
           IsZeroFilled(move.mail_content, sizeof(move.mail_content));
}

int FindMoveRequest(MySQL* database,
                    const Message& request,
                    MoveRecord& move) {
    if (!database ||
        !database->set_query(
            "seLect move_id from %s where server_group=%u and m_id=%u "
            "and charac_no=%u and session_token=%u and request_token=%u "
            "limit 2",
            kMoveTable, static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        return -1;
    }
    if (database->get_n_rows() == 0) return 0;
    uint64_t move_id = 0ULL;
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_ulonglong(0, move_id) ||
        !ReadMove(database, request, move_id, true, move)) {
        return -1;
    }
    return 1;
}

bool MoveMatches(const Message& request, const MoveRecord& move) {
    return move.move_id == request.move_id &&
           move.campaign_key == request.campaign_key &&
           move.revision_before == request.revision &&
           move.depth_before == request.current_depth &&
           move.target_depth == request.target_depth &&
           move.claimed_before == request.claimed_mask &&
           move.claimed_after == request.claimed_mask_after &&
           move.box_item_id == request.box_item_id &&
           move.box_count_before == request.box_count_before &&
           move.box_total_before == request.box_total_before &&
           move.reward_item_id == request.reward_item_id &&
           move.reward_count == request.reward_count &&
           move.source_slot == request.source_slot &&
           move.box_weight == request.box_weight &&
           move.rollover == request.rollover &&
           std::memcmp(move.inventory_pre_sha256,
                       request.inventory_pre_sha256, 32u) == 0 &&
           std::memcmp(move.inventory_post_sha256,
                       request.inventory_post_sha256, 32u) == 0;
}

bool ReserveRequestMatchesMove(const Message& request,
                               const MoveRecord& move) {
    if (move.campaign_key != request.campaign_key ||
        move.revision_before != request.revision ||
        move.depth_before != request.current_depth ||
        move.target_depth != request.target_depth ||
        move.claimed_before != request.claimed_mask ||
        move.claimed_after != request.claimed_mask_after ||
        move.box_item_id != request.box_item_id ||
        move.box_count_before != request.box_count_before ||
        move.box_total_before != request.box_total_before ||
        move.source_slot != request.source_slot ||
        move.box_weight != request.box_weight ||
        move.rollover != request.rollover ||
        std::memcmp(move.inventory_pre_sha256,
                    request.inventory_pre_sha256, 32u) != 0 ||
        std::memcmp(move.inventory_post_sha256,
                    request.inventory_post_sha256, 32u) != 0) {
        return false;
    }
    uint32_t specified_index = 0u;
    if (IsSpecifiedClaim(request.target_depth, request.claimed_mask,
                         specified_index)) {
        return move.reward_item_id == request.reward_item_id &&
               move.reward_count == request.reward_count;
    }
    return request.reward_item_id == 0u && request.reward_count == 0u &&
           IsNormalReward(move.reward_item_id, move.reward_count);
}

bool MailMatches(const Message& request, const MoveRecord& move) {
    return HasExactMail(request) &&
           std::memcmp(&request.mail_reward, &move.mail_reward,
                       sizeof(request.mail_reward)) == 0 &&
           std::memcmp(request.mail_title, move.mail_title,
                       sizeof(request.mail_title)) == 0 &&
           std::memcmp(request.mail_content, move.mail_content,
                       sizeof(request.mail_content)) == 0;
}

void ProjectState(Message& response, const SeaState& state) {
    response.campaign_key = state.campaign_key;
    response.revision = state.revision;
    response.current_depth = state.current_depth;
    response.claimed_mask = state.claimed_mask;
}

void ProjectMove(Message& response, const MoveRecord& move) {
    response.move_id = move.move_id;
    response.campaign_key = move.campaign_key;
    response.revision = move.revision_before;
    response.current_depth = move.depth_before;
    response.target_depth = move.target_depth;
    response.claimed_mask = move.claimed_before;
    response.claimed_mask_after = move.claimed_after;
    response.box_item_id = move.box_item_id;
    response.box_count_before = move.box_count_before;
    response.box_total_before = move.box_total_before;
    response.reward_item_id = move.reward_item_id;
    response.reward_count = move.reward_count;
    response.source_slot = move.source_slot;
    response.box_weight = move.box_weight;
    response.rollover = move.rollover;
    response.phase = move.state;
    std::memcpy(response.inventory_pre_sha256, move.inventory_pre_sha256,
                sizeof(response.inventory_pre_sha256));
    std::memcpy(response.inventory_post_sha256, move.inventory_post_sha256,
                sizeof(response.inventory_post_sha256));
    response.mail_reward = move.mail_reward;
    std::memcpy(response.mail_title, move.mail_title,
                sizeof(response.mail_title));
    std::memcpy(response.mail_content, move.mail_content,
                sizeof(response.mail_content));
}

bool FindMail(MySQL* database,
              uint64_t delivery_key,
              uint32_t receiver,
              const MailReward& reward,
              const char* title,
              uint32_t expected_letter_id,
              bool& found,
              uint32_t& letter_id) {
    found = false;
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char* const item_guid = database ? database->blob_to_str(
        0, const_cast<uint8_t*>(reward.item_guid),
        static_cast<int>(sizeof(reward.item_guid))) : 0;
    char* const random_option = database ? database->blob_to_str(
        1, const_cast<uint8_t*>(reward.random_option),
        static_cast<int>(sizeof(reward.random_option))) : 0;
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
        if (!database->get_uint(static_cast<int>(index), values[index]))
            return false;
    }
    if (values[0] != receiver || values[1] != reward.item_id ||
        values[2] != reward.add_info || values[3] != expected_letter_id ||
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
                const char* title,
                uint32_t letter_id) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    if (!database || !title || letter_id == 0u ||
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
               static_cast<unsigned int>(letter_id),
               static_cast<unsigned int>(reward.extend_info), item_guid,
               static_cast<unsigned long long>(delivery_key), random_option,
               static_cast<unsigned int>(reward.chronicle_value),
               static_cast<unsigned int>(reward.separate_upgrade)) &&
           database->exec(true);
}

bool FindLetter(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const char* title,
                const char* content,
                bool& found,
                uint32_t& letter_id) {
    found = false;
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    if (!database || !title || !content ||
        database->escape_string(escaped_title, title) < 0 ||
        database->escape_string(escaped_content, content) < 0 ||
        !database->set_query(
            "seLect letter_id,charac_no,send_charac_no,stat,"
            "length(send_charac_name),length(letter_text),"
            "binary send_charac_name='%s',binary letter_text='%s' "
            "from letter where auction_id=%llu limit 2",
            escaped_title, escaped_content,
            static_cast<unsigned long long>(delivery_key)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int values[8] = {};
    if (database->get_n_rows() != 1 || !database->fetch()) return false;
    for (unsigned int index = 0u; index < 8u; ++index) {
        if (!database->get_uint(static_cast<int>(index), values[index]))
            return false;
    }
    if (values[0] == 0u ||
        values[0] > static_cast<unsigned int>(
                        std::numeric_limits<int>::max()) ||
        values[1] != receiver || values[2] != 0u || values[3] != 0u ||
        values[4] != std::strlen(title) ||
        values[5] != std::strlen(content) || values[6] != 1u ||
        values[7] != 1u) {
        return false;
    }
    letter_id = values[0];
    found = true;
    return true;
}

bool InsertLetter(MySQL* database,
                  uint64_t delivery_key,
                  uint32_t receiver,
                  const char* title,
                  const char* content) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    return database && title && content &&
           database->escape_string(escaped_title, title) >= 0 &&
           database->escape_string(escaped_content, content) >= 0 &&
           database->set_query(
               "inSert into letter(charac_no,send_charac_no,"
               "send_charac_name,letter_text,reg_date,stat,auction_id) "
               "values(%u,0,'%s','%s',now(),0,%llu)",
               static_cast<unsigned int>(receiver), escaped_title,
               escaped_content,
               static_cast<unsigned long long>(delivery_key)) &&
           database->exec(true);
}

bool EnsureLetter(MySQL* database,
                  uint64_t delivery_key,
                  uint32_t receiver,
                  const char* title,
                  const char* content,
                  uint32_t& letter_id) {
    bool found = false;
    if (!FindLetter(database, delivery_key, receiver, title, content, found,
                    letter_id)) {
        return false;
    }
    if (found) return true;
    (void)InsertLetter(database, delivery_key, receiver, title, content);
    return FindLetter(database, delivery_key, receiver, title, content, found,
                      letter_id) && found;
}

bool EnsureMail(MySQL* database,
                uint64_t delivery_key,
                uint32_t receiver,
                const MailReward& reward,
                const char* title,
                const char* content) {
    uint32_t letter_id = 0u;
    if (!EnsureLetter(database, delivery_key, receiver, title, content,
                      letter_id)) {
        return false;
    }
    bool found = false;
    uint32_t existing_letter = 0u;
    if (!FindMail(database, delivery_key, receiver, reward, title, letter_id,
                  found, existing_letter)) {
        return false;
    }
    if (found) return existing_letter == letter_id;
    (void)InsertMail(database, delivery_key, receiver, reward, title,
                     letter_id);
    return FindMail(database, delivery_key, receiver, reward, title,
                    letter_id, found, existing_letter) && found &&
           existing_letter == letter_id;
}

bool DeliverReward(const Message& request) {
    MySQL* const postal = GetPartitionDatabase(3, request.server_group);
    if (!Begin(postal)) return false;
    const bool delivered = EnsureMail(
        postal, kMailNamespace | request.move_id, request.charac_no,
        request.mail_reward, request.mail_title, request.mail_content);
    if (!delivered || !Commit(postal)) {
        Rollback(postal);
        return false;
    }
    return true;
}

bool HandleSnapshot(MySQL* database,
                    const Message& request,
                    Message& response) {
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    bool active = false;
    uint32_t campaign_key = 0u;
    if (!LoadCampaign(database, active, campaign_key)) {
        Rollback(database);
        return false;
    }
    response.active = active ? 1u : 0u;
    response.campaign_key = active ? campaign_key : 0u;
    response.revision = 0u;
    response.current_depth = 0u;
    response.claimed_mask = 0u;
    response.claimed_mask_after = 0u;
    response.move_id = 0ULL;
    response.phase = PHASE_NONE;
    response.client_reply_expected = 0u;
    response.recovery_action = 0u;
    SeaState pending_state;
    const int pending = FindPendingJournal(database, request, pending_state);
    if (pending < 0) {
        Rollback(database);
        return false;
    }
    if (pending == 1) {
        MoveRecord move;
        if (!ReadMove(database, request, pending_state.pending_move_id,
                      true, move) ||
            (move.state != PHASE_RESERVED &&
             move.state != PHASE_MAIL_PREPARED) ||
            move.campaign_key != pending_state.campaign_key ||
            move.revision_before != pending_state.revision ||
            move.depth_before != pending_state.current_depth ||
            move.claimed_before != pending_state.claimed_mask) {
            Rollback(database);
            return false;
        }
        ProjectMove(response, move);
        response.active = active ? 1u : 0u;
        response.client_reply_expected = 0u;
    } else if (active) {
        SeaState state;
        if (!EnsureState(database, request, campaign_key, state)) {
            Rollback(database);
            return false;
        }
        ProjectState(response, state);
        response.active = 1u;
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleReserve(MySQL* database,
                   const Message& request,
                   Message& response) {
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    bool active = false;
    uint32_t campaign_key = 0u;
    if (!LoadCampaign(database, active, campaign_key)) {
        Rollback(database);
        return false;
    }
    SeaState pending_state;
    const int pending = FindPendingJournal(database, request, pending_state);
    if (pending < 0) {
        Rollback(database);
        return false;
    }
    MoveRecord duplicate;
    const int found = FindMoveRequest(database, request, duplicate);
    if (found < 0) {
        Rollback(database);
        return false;
    }
    if (found == 1 && !ReserveRequestMatchesMove(request, duplicate)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    response.active = active ? 1u : 0u;
    if (pending == 1) {
        if (found == 1 && duplicate.state == PHASE_RESERVED &&
            duplicate.move_id == pending_state.pending_move_id &&
            duplicate.campaign_key == pending_state.campaign_key &&
            duplicate.revision_before == pending_state.revision &&
            duplicate.depth_before == pending_state.current_depth &&
            duplicate.claimed_before == pending_state.claimed_mask) {
            ProjectMove(response, duplicate);
            response.active = active ? 1u : 0u;
            response.client_reply_expected = 1u;
            response.result = RESULT_SUCCESS;
        } else {
            ProjectState(response, pending_state);
            response.active = active ? 1u : 0u;
            response.result = RESULT_PENDING;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (found == 1) {
        ProjectMove(response, duplicate);
        response.active = active ? 1u : 0u;
        response.result = RESULT_PENDING;
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    if (!active || request.campaign_key != campaign_key) {
        response.result = RESULT_EVENT_CLOSED;
        response.active = 0u;
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    SeaState state;
    if (!EnsureState(database, request, campaign_key, state)) {
        Rollback(database);
        return false;
    }
    if (state.pending_move_id != 0ULL) {
        Rollback(database);
        return false;
    }
    if (state.revision != request.revision ||
        state.current_depth != request.current_depth ||
        state.claimed_mask != request.claimed_mask) {
        ProjectState(response, state);
        response.result = RESULT_INVALID_STATE;
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        return true;
    }
    uint32_t reward_item_id = request.reward_item_id;
    uint32_t reward_count = request.reward_count;
    uint32_t specified_index = 0u;
    if (!IsSpecifiedClaim(request.target_depth, request.claimed_mask,
                          specified_index) &&
        !SelectNormalReward(database, reward_item_id, reward_count)) {
        Rollback(database);
        return false;
    }
    char* const inventory_pre_sha256 = database->blob_to_str(
        0, const_cast<uint8_t*>(request.inventory_pre_sha256), 32);
    char* const inventory_post_sha256 = database->blob_to_str(
        1, const_cast<uint8_t*>(request.inventory_post_sha256), 32);
    if (!inventory_pre_sha256 || !inventory_post_sha256) {
        Rollback(database);
        return false;
    }
    if (!database->set_query(
            "inSert into %s(server_group,m_id,charac_no,campaign_key,"
            "revision_before,depth_before,target_depth,claimed_before,"
            "claimed_after,box_item_id,box_count_before,box_total_before,"
            "inventory_pre_sha256,inventory_post_sha256,reward_item_id,"
            "reward_count,source_slot,box_weight,rollover,session_token,"
            "request_token,move_state,created_at,updated_at,delivered_at,"
            "cancelled_at) values(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
            "%u,'%s','%s',%u,%u,%u,%d,%u,%u,%u,1,now(),now(),null,null)",
            kMoveTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(request.revision),
            static_cast<unsigned int>(request.current_depth),
            static_cast<unsigned int>(request.target_depth),
            static_cast<unsigned int>(request.claimed_mask),
            static_cast<unsigned int>(request.claimed_mask_after),
            static_cast<unsigned int>(request.box_item_id),
            static_cast<unsigned int>(request.box_count_before),
            static_cast<unsigned int>(request.box_total_before),
            inventory_pre_sha256, inventory_post_sha256,
            static_cast<unsigned int>(reward_item_id),
            static_cast<unsigned int>(reward_count),
            static_cast<unsigned int>(request.source_slot),
            static_cast<int>(request.box_weight),
            static_cast<unsigned int>(request.rollover),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    const uint64_t move_id = WongWork::DBCommon::GetIdentity(database);
    if (move_id == 0ULL || move_id > kMaximumMoveId ||
        !InsertPendingGuard(database, request, campaign_key, move_id) ||
        !database->set_query(
            "upDate %s set pending_move_id=%llu,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and campaign_key=%u "
            "and revision=%u and current_depth=%u and claimed_mask=%u "
            "and pending_move_id=0",
            kStateTable, static_cast<unsigned long long>(move_id),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(campaign_key),
            static_cast<unsigned int>(request.revision),
            static_cast<unsigned int>(request.current_depth),
            static_cast<unsigned int>(request.claimed_mask)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    MoveRecord reserved;
    reserved.move_id = move_id;
    reserved.campaign_key = campaign_key;
    reserved.revision_before = request.revision;
    reserved.depth_before = request.current_depth;
    reserved.target_depth = request.target_depth;
    reserved.claimed_before = request.claimed_mask;
    reserved.claimed_after = request.claimed_mask_after;
    reserved.box_item_id = request.box_item_id;
    reserved.box_count_before = request.box_count_before;
    reserved.box_total_before = request.box_total_before;
    reserved.reward_item_id = reward_item_id;
    reserved.reward_count = reward_count;
    reserved.source_slot = request.source_slot;
    reserved.box_weight = request.box_weight;
    reserved.rollover = request.rollover;
    reserved.state = PHASE_RESERVED;
    std::memcpy(reserved.inventory_pre_sha256,
                request.inventory_pre_sha256, 32u);
    std::memcpy(reserved.inventory_post_sha256,
                request.inventory_post_sha256, 32u);
    ProjectMove(response, reserved);
    response.active = 1u;
    response.client_reply_expected = 1u;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandlePrepareMail(MySQL* database,
                       const Message& request,
                       Message& response) {
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState pending_state;
    const int pending = FindPendingJournal(database, request, pending_state);
    MoveRecord move;
    if (pending < 0 ||
        !ReadMove(database, request, request.move_id, true, move) ||
        !MoveMatches(request, move)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState state;
    if (!ReadState(database, request, move.campaign_key, true, state)) {
        Rollback(database);
        return false;
    }
    const bool pending_matches = pending == 1 &&
        pending_state.campaign_key == move.campaign_key &&
        pending_state.pending_move_id == move.move_id &&
        state.pending_move_id == move.move_id &&
        state.revision == move.revision_before &&
        state.current_depth == move.depth_before &&
        state.claimed_mask == move.claimed_before;
    if (move.state == PHASE_MAIL_PREPARED) {
        if (!pending_matches || !MailMatches(request, move) ||
            !Commit(database)) {
            Rollback(database);
            return false;
        }
        ProjectMove(response, move);
        response.active = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (move.state != PHASE_RESERVED || !pending_matches ||
        !inventory_mutation_barrier::
            HasExactPersistedInventoryMutation(
                request.server_group, request.charac_no, move.source_slot,
                move.box_item_id, move.box_count_before - 1u,
                move.box_total_before - 1u,
                move.inventory_post_sha256)) {
        Rollback(database);
        response.result = RESULT_RECOVERY_AMBIGUOUS;
        return true;
    }
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    char* const item_guid = database->blob_to_str(
        0, const_cast<uint8_t*>(request.mail_reward.item_guid),
        static_cast<int>(sizeof(request.mail_reward.item_guid)));
    char* const random_option = database->blob_to_str(
        1, const_cast<uint8_t*>(request.mail_reward.random_option),
        static_cast<int>(sizeof(request.mail_reward.random_option)));
    if (!item_guid || !random_option ||
        database->escape_string(escaped_title, request.mail_title) < 0 ||
        database->escape_string(escaped_content, request.mail_content) < 0 ||
        !database->set_query(
            "upDate %s set mail_item_id=%u,mail_add_info=%u,"
            "mail_extend_info=%u,mail_chronicle_value=%u,"
            "mail_endurance=%u,mail_amplify_value=%u,mail_seal_flag=%u,"
            "mail_item_attr=%u,mail_upgrade=%u,mail_amplify_option=%u,"
            "mail_item_guid='%s',mail_random_option='%s',"
            "mail_separate_upgrade=%u,mail_avatar_flag=%u,"
            "mail_creature_flag=%u,mail_reserved=%u,mail_title='%s',"
            "mail_content='%s',move_state=4,prepared_at=now(),"
            "updated_at=now() where move_id=%llu and move_state=1",
            kMoveTable,
            static_cast<unsigned int>(request.mail_reward.item_id),
            static_cast<unsigned int>(request.mail_reward.add_info),
            static_cast<unsigned int>(request.mail_reward.extend_info),
            static_cast<unsigned int>(request.mail_reward.chronicle_value),
            static_cast<unsigned int>(request.mail_reward.endurance),
            static_cast<unsigned int>(request.mail_reward.amplify_value),
            static_cast<unsigned int>(request.mail_reward.seal_flag),
            static_cast<unsigned int>(request.mail_reward.item_attr),
            static_cast<unsigned int>(request.mail_reward.upgrade),
            static_cast<unsigned int>(request.mail_reward.amplify_option),
            item_guid, random_option,
            static_cast<unsigned int>(
                request.mail_reward.separate_upgrade),
            static_cast<unsigned int>(request.mail_reward.avatar_flag),
            static_cast<unsigned int>(request.mail_reward.creature_flag),
            static_cast<unsigned int>(request.mail_reward.reserved),
            escaped_title, escaped_content,
            static_cast<unsigned long long>(move.move_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    move.state = PHASE_MAIL_PREPARED;
    move.mail_reward = request.mail_reward;
    std::memcpy(move.mail_title, request.mail_title,
                sizeof(move.mail_title));
    std::memcpy(move.mail_content, request.mail_content,
                sizeof(move.mail_content));
    ProjectMove(response, move);
    response.active = 1u;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCommit(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState pending_state;
    const int pending = FindPendingJournal(database, request, pending_state);
    MoveRecord move;
    if (pending < 0 ||
        !ReadMove(database, request, request.move_id, true, move) ||
        !MoveMatches(request, move) || !MailMatches(request, move)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState state;
    if (!ReadState(database, request, move.campaign_key, true, state)) {
        Rollback(database);
        return false;
    }
    if (move.state == PHASE_DELIVERED) {
        if (state.pending_move_id != 0ULL ||
            pending != 0 ||
            state.revision != move.revision_before + 1u ||
            state.current_depth !=
                (move.rollover ? 0u : move.target_depth) ||
            state.claimed_mask != move.claimed_after || !Commit(database)) {
            Rollback(database);
            return false;
        }
        ProjectMove(response, move);
        response.current_depth = state.current_depth;
        response.claimed_mask = state.claimed_mask;
        response.revision = state.revision;
        response.phase = PHASE_DELIVERED;
        response.active = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (move.state != PHASE_MAIL_PREPARED || pending != 1 ||
        pending_state.campaign_key != move.campaign_key ||
        pending_state.pending_move_id != move.move_id ||
        state.pending_move_id != move.move_id ||
        state.revision != move.revision_before ||
        state.current_depth != move.depth_before ||
        state.claimed_mask != move.claimed_before) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (!DeliverReward(request)) {
        Rollback(database);
        return false;
    }
    const uint32_t next_depth = move.rollover ? 0u : move.target_depth;
    if (!database->set_query(
            "upDate %s set move_state=2,updated_at=now(),delivered_at=now() "
            "where move_id=%llu and move_state=4",
            kMoveTable, static_cast<unsigned long long>(move.move_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set revision=revision+1,current_depth=%u,"
            "claimed_mask=%u,pending_move_id=0,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and campaign_key=%u "
            "and revision=%u and current_depth=%u and claimed_mask=%u "
            "and pending_move_id=%llu",
            kStateTable, static_cast<unsigned int>(next_depth),
            static_cast<unsigned int>(move.claimed_after),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(move.campaign_key),
            static_cast<unsigned int>(move.revision_before),
            static_cast<unsigned int>(move.depth_before),
            static_cast<unsigned int>(move.claimed_before),
            static_cast<unsigned long long>(move.move_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !DeletePendingGuard(database, request, move.campaign_key,
                            move.move_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    move.state = PHASE_DELIVERED;
    ProjectMove(response, move);
    response.current_depth = next_depth;
    response.claimed_mask = move.claimed_after;
    response.revision = move.revision_before + 1u;
    response.phase = PHASE_DELIVERED;
    response.active = 1u;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleCancel(MySQL* database,
                  const Message& request,
                  Message& response) {
    if (!Begin(database)) return false;
    if (!LoadOwnedCharacter(database, request)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState pending_state;
    const int pending = FindPendingJournal(database, request, pending_state);
    MoveRecord move;
    if (pending < 0 ||
        !ReadMove(database, request, request.move_id, true, move) ||
        !MoveMatches(request, move)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    SeaState state;
    if (!ReadState(database, request, move.campaign_key, true, state)) {
        Rollback(database);
        return false;
    }
    if (move.state == PHASE_CANCELLED) {
        if (state.pending_move_id != 0ULL ||
            pending != 0 ||
            state.revision != move.revision_before ||
            state.current_depth != move.depth_before ||
            state.claimed_mask != move.claimed_before || !Commit(database)) {
            Rollback(database);
            return false;
        }
        ProjectMove(response, move);
        response.phase = PHASE_CANCELLED;
        response.active = 1u;
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (move.state != PHASE_RESERVED || pending != 1 ||
        pending_state.campaign_key != move.campaign_key ||
        pending_state.pending_move_id != move.move_id ||
        state.pending_move_id != move.move_id ||
        state.revision != move.revision_before ||
        state.current_depth != move.depth_before ||
        state.claimed_mask != move.claimed_before ||
        !database->set_query(
            "upDate %s set move_state=3,updated_at=now(),cancelled_at=now() "
            "where move_id=%llu and move_state=1",
            kMoveTable, static_cast<unsigned long long>(move.move_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set pending_move_id=0,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and campaign_key=%u "
            "and revision=%u and current_depth=%u and claimed_mask=%u "
            "and pending_move_id=%llu",
            kStateTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(move.campaign_key),
            static_cast<unsigned int>(move.revision_before),
            static_cast<unsigned int>(move.depth_before),
            static_cast<unsigned int>(move.claimed_before),
            static_cast<unsigned long long>(move.move_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !DeletePendingGuard(database, request, move.campaign_key,
                            move.move_id) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    move.state = PHASE_CANCELLED;
    ProjectMove(response, move);
    response.phase = PHASE_CANCELLED;
    response.active = 1u;
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

DBSeaBottomEvent::DBSeaBottomEvent()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBSeaBottomEvent::~DBSeaBottomEvent() {}

bool IsLegacyFencedInventoryCommit(const Message& request) {
    return IsValidRequest(request) &&
           request.operation == OPERATION_PREPARE_MAIL &&
           request.local_mutation_done == 1u;
}

bool DBSeaBottomEvent::QueueRequest(int user_idx,
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
                kFenceTwSeaBottomInventoryCommit,
            guard);
    }
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBSeaBottomEvent::dispatch(int signal,
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
    response.phase = PHASE_NONE;
    response.active = 0u;
    MySQL* const database = GetPartitionDatabase(2, request.server_group);
    bool handled = false;
    if (database) {
        switch (request.operation) {
            case OPERATION_LOAD:
            case OPERATION_REFRESH:
                handled = HandleSnapshot(database, request, response);
                break;
            case OPERATION_RESERVE:
                handled = HandleReserve(database, request, response);
                break;
            case OPERATION_PREPARE_MAIL:
                handled = HandlePrepareMail(database, request, response);
                break;
            case OPERATION_COMMIT:
                handled = HandleCommit(database, request, response);
                break;
            case OPERATION_CANCEL:
                handled = HandleCancel(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) response.result = RESULT_DATABASE_FAILURE;
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace tw_seabottom_db_0828
