#include "old_data/db/event/db_decide_levelup_event.h"

#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_legacy_feature_fence.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/inventory/db_inventory_mutation_barrier.h"
#include "old_data/db/core/mysql.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/checksum.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace decide_levelup_db {
namespace {

static_assert(kDbSignal == 844,
              "decide-level native DB signal must remain slot 844");
static_assert(DB_HANDLE_FEATURE_DB2 == 18,
              "decide-level state must use dedicated native DB2 handle 18");
static_assert(DB_HANDLE_FEATURE_DB3 == 17,
              "decide-level proof must use dedicated native DB3 handle 17");
static_assert(MsgQueueMgr::FEATURE_DB_Q == 6,
              "decide-level DB requests must remain on native queue 6");

const char kStateTable[] = "account_decide_levelup_state_0828";
const char kClaimTable[] = "account_decide_levelup_claim_0828";
const char kMailAckTable[] = "account_decide_levelup_mail_ack_0828";
const int kServerGroupCount = 0x12;

void InitResponse(Message& response, const Message& request) {
    response = request;
    response.result = static_cast<uint8_t>(kResultOperationFailed);
    response.mail_persisted = 0;
}

bool IsAllowedTarget(uint32_t target_level) {
    return target_level == 20u || target_level == 30u ||
           target_level == 40u || target_level == 50u ||
           target_level == 60u || target_level == 70u;
}

bool IsValidServerGroup(int32_t server_group) {
    // RECONSTRUCTED_0828: the restored login dispatcher rejects values >=18.
    // Carrying the selected group through DB_Q prevents handle-2 cross-shard
    // reads when DBMgr global routing is enabled.
    return server_group >= 0 && server_group < kServerGroupCount;
}

MySQL* GetGameDb(const Message& request) {
    return GlobalData::s_db_mgr && IsValidServerGroup(request.server_group)
               ? GlobalData::s_db_mgr->GetDBHandleExact(
                     DB_HANDLE_FEATURE_DB2,
                     static_cast<ENUM_SERVER_GROUP>(request.server_group))
               : 0;
}

bool IsZeroFilled(const uint8_t* bytes, size_t size) {
    if (!bytes) return false;
    for (size_t index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return false;
    }
    return true;
}

bool DeliveryTokenIsZero(const uint8_t token[16]) {
    return IsZeroFilled(token, 16u);
}

bool MailProjectionIsZero(const RewardDelivery& delivery) {
    return IsZeroFilled(
        &delivery.seal_flag,
        sizeof(RewardDelivery) - offsetof(RewardDelivery, seal_flag));
}

void ClearPlan(Message& message) {
    message.plan_state = kPlanStateNone;
    message.reward_count = 0u;
    message.mail_count = 0u;
    message.mail_persisted = 0u;
    message.event_coin_before = 0u;
    message.event_coin_after = 0u;
    std::memset(message.ack_delivery_token, 0,
                sizeof(message.ack_delivery_token));
    message.ack_reward_item_id = 0u;
    std::memset(message.inventory_pre_sha256, 0,
                sizeof(message.inventory_pre_sha256));
    std::memset(message.inventory_post_sha256, 0,
                sizeof(message.inventory_post_sha256));
    std::memset(message.reward_plan_sha256, 0,
                sizeof(message.reward_plan_sha256));
    std::memset(message.deliveries, 0, sizeof(message.deliveries));
}

bool LoadPreparedPlan(MySQL* db, Message& response);

bool ReadStateRow(MySQL* db, Message& response) {
    ClearPlan(response);
    if (!db ||
        !db->set_query(
            "seLect target_level,ifnull(charac_no,0),main_reward_decided,"
            "last_reward_level,claim_token,pending_reward_level,"
            "pending_main_reward from %s where account_id=%u",
            kStateTable,
            static_cast<unsigned int>(response.account_id)) ||
        !db->exec(true)) {
        return false;
    }
    if (db->get_n_rows() == 0) {
        response.charac_no = 0;
        response.target_level = 0;
        response.previous_reward_level = 0;
        response.current_reward_level = 0;
        response.claim_token = 0;
        response.main_reward_decided = 0;
        response.main_reward_newly_decided = 0;
        return true;
    }
    if (db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }

    unsigned int target_level = 0;
    unsigned int charac_no = 0;
    unsigned int main_reward_decided = 0;
    unsigned int last_reward_level = 0;
    unsigned int claim_token = 0;
    unsigned int pending_reward_level = 0;
    unsigned int pending_main_reward = 0;
    if (!db->get_uint(0, target_level) || !db->get_uint(1, charac_no) ||
        !db->get_uint(2, main_reward_decided) ||
        !db->get_uint(3, last_reward_level) ||
        !db->get_uint(4, claim_token) ||
        !db->get_uint(5, pending_reward_level) ||
        !db->get_uint(6, pending_main_reward) ||
        !IsAllowedTarget(target_level) || main_reward_decided > 1u ||
        last_reward_level > 255u || pending_reward_level > 255u ||
        pending_main_reward > 1u ||
        (pending_reward_level != 0u &&
         (claim_token == 0u || pending_reward_level <= last_reward_level))) {
        return false;
    }
    response.target_level = target_level;
    response.charac_no = charac_no;
    response.previous_reward_level = last_reward_level;
    response.current_reward_level = pending_reward_level != 0u
                                        ? pending_reward_level
                                        : last_reward_level;
    response.claim_token = pending_reward_level != 0u ? claim_token : 0u;
    response.main_reward_decided =
        static_cast<uint8_t>(main_reward_decided != 0u);
    response.main_reward_newly_decided = static_cast<uint8_t>(
        pending_reward_level != 0u && pending_main_reward != 0u);
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool Load(MySQL* db, Message& response) {
    if (!ReadStateRow(db, response)) {
        return false;
    }
    if (response.claim_token != 0u && !LoadPreparedPlan(db, response)) {
        return false;
    }
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool Decide(MySQL* db, Message& response) {
    if (!IsAllowedTarget(response.target_level)) {
        response.result = static_cast<uint8_t>(kResultInvalidTarget);
        return true;
    }
    if (!db->set_query(
            "inSert ignore into %s(account_id,charac_no,target_level,"
            "main_reward_decided,last_reward_level,claim_token,"
            "pending_reward_level,pending_main_reward) "
            "values(%u,NULL,%u,0,0,0,0,0)",
            kStateTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(response.target_level)) ||
        !db->exec(true)) {
        return false;
    }
    if (db->getAffectedRowCount() == 1) {
        response.charac_no = 0;
        response.previous_reward_level = 0;
        response.current_reward_level = 0;
        response.claim_token = 0;
        response.main_reward_decided = 0;
        response.main_reward_newly_decided = 0;
        response.result = static_cast<uint8_t>(kResultOk);
        return true;
    }

    if (!ReadStateRow(db, response)) {
        return false;
    }
    response.result = static_cast<uint8_t>(kResultAlreadyDecided);
    return true;
}

bool BindCharacter(MySQL* db, Message& response) {
    if (!response.charac_no) {
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }
    if (!db->set_query(
            "upDate %s set charac_no=%u where account_id=%u and "
            "charac_no is null and main_reward_decided=0",
            kStateTable,
            static_cast<unsigned int>(response.charac_no),
            static_cast<unsigned int>(response.account_id)) ||
        !db->exec(true)) {
        return false;
    }
    const uint32_t requested_charac_no = response.charac_no;
    if (!ReadStateRow(db, response)) {
        return false;
    }
    response.result = static_cast<uint8_t>(
        response.charac_no == requested_charac_no
            ? kResultOk
            : kResultReservationRequired);
    return true;
}

bool RollbackTransaction(MySQL* db) {
    return db && db->set_query("rollback") && db->exec(true);
}

bool CommitTransaction(MySQL* db) {
    return db && db->set_query("commit") && db->exec(true);
}

bool HashRewardPlan(const Message& message, uint8_t digest[32]) {
    if (!digest || message.reward_count == 0u ||
        message.reward_count > kMaximumRewardDeliveries) {
        return false;
    }
    CNChecksumSha256 checksum;
    return checksum.MakeChecksumFromMemory(
               reinterpret_cast<const unsigned char*>(message.deliveries),
               static_cast<int>(message.reward_count *
                                sizeof(RewardDelivery)),
               digest,
               32) == CHECKSUM_RESULT_OK;
}

bool IsValidPreparedPlan(const Message& message) {
    if (!message.account_id || !message.charac_no || !message.claim_token ||
        !IsValidServerGroup(message.server_group) ||
        message.plan_state != kPlanStatePrepared ||
        message.reward_count == 0u ||
        message.reward_count > kMaximumRewardDeliveries ||
        message.mail_count > message.reward_count ||
        message.event_coin_after < message.event_coin_before ||
        IsZeroFilled(message.inventory_pre_sha256, 32u) ||
        IsZeroFilled(message.inventory_post_sha256, 32u) ||
        IsZeroFilled(message.reward_plan_sha256, 32u)) {
        return false;
    }

    uint8_t digest[32] = {};
    if (!HashRewardPlan(message, digest) ||
        std::memcmp(digest, message.reward_plan_sha256, 32u) != 0) {
        return false;
    }
    uint32_t event_coin_delta = 0u;
    uint8_t mail_count = 0u;
    for (uint8_t index = 0u; index < message.reward_count; ++index) {
        const RewardDelivery& delivery = message.deliveries[index];
        if (!delivery.item_id || !delivery.count ||
            delivery.item_id > 0x7fffffffu ||
            delivery.count > 0x7fffffffu || delivery.reserved != 0u) {
            return false;
        }
        if (delivery.kind == kDeliveryEventCoin) {
            if (delivery.item_id != 1u ||
                !DeliveryTokenIsZero(delivery.delivery_token) ||
                !MailProjectionIsZero(delivery) ||
                event_coin_delta > 0xffffffffu - delivery.count) {
                return false;
            }
            event_coin_delta += delivery.count;
        } else if (delivery.kind == kDeliveryDirectInventory) {
            if (delivery.item_id == 1u ||
                !DeliveryTokenIsZero(delivery.delivery_token) ||
                !MailProjectionIsZero(delivery)) {
                return false;
            }
        } else if (delivery.kind == kDeliveryTrackedMail) {
            if (delivery.item_id == 1u ||
                DeliveryTokenIsZero(delivery.delivery_token) ||
                delivery.avatar_flag > 1u ||
                delivery.creature_flag > 1u ||
                mail_count == 0xffu) {
                return false;
            }
            for (uint8_t prior = 0u; prior < index; ++prior) {
                if (std::memcmp(message.deliveries[prior].delivery_token,
                                delivery.delivery_token, 16u) == 0) {
                    return false;
                }
            }
            ++mail_count;
        } else {
            return false;
        }
    }
    for (uint8_t index = message.reward_count;
         index < kMaximumRewardDeliveries;
         ++index) {
        const uint8_t* const bytes = reinterpret_cast<const uint8_t*>(
            &message.deliveries[index]);
        if (!IsZeroFilled(bytes, sizeof(RewardDelivery))) return false;
    }
    return mail_count == message.mail_count &&
           event_coin_delta ==
               message.event_coin_after - message.event_coin_before;
}

bool ClaimPlanEquals(const Message& left, const Message& right) {
    return left.account_id == right.account_id &&
           left.charac_no == right.charac_no &&
           left.claim_token == right.claim_token &&
           left.server_group == right.server_group &&
           left.previous_reward_level == right.previous_reward_level &&
           left.current_reward_level == right.current_reward_level &&
           left.target_level == right.target_level &&
           left.reward_count == right.reward_count &&
           left.mail_count == right.mail_count &&
           left.event_coin_before == right.event_coin_before &&
           left.event_coin_after == right.event_coin_after &&
           std::memcmp(left.inventory_pre_sha256,
                       right.inventory_pre_sha256, 32u) == 0 &&
           std::memcmp(left.inventory_post_sha256,
                       right.inventory_post_sha256, 32u) == 0 &&
           std::memcmp(left.reward_plan_sha256,
                       right.reward_plan_sha256, 32u) == 0 &&
           std::memcmp(left.deliveries, right.deliveries,
                       sizeof(left.deliveries)) == 0;
}

bool LoadPreparedPlan(MySQL* db, Message& response) {
    if (!db || response.claim_token == 0u || response.account_id == 0u ||
        !db->set_query(
            "seLect charac_no,server_group,previous_reward_level,"
            "current_reward_level,target_level,event_coin_before,"
            "event_coin_after,inventory_pre_sha256,inventory_post_sha256,"
            "reward_plan_sha256,reward_count,mail_count,plan_blob,plan_state "
            "from %s where account_id=%u and claim_token=%u limit 2",
            kClaimTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(response.claim_token)) ||
        !db->exec(true)) {
        return false;
    }
    if (db->get_n_rows() == 0) {
        ClearPlan(response);
        return true;
    }
    if (db->get_n_rows() != 1 || !db->fetch()) return false;

    unsigned int charac_no = 0u;
    unsigned int server_group = 0u;
    unsigned int previous = 0u;
    unsigned int current = 0u;
    unsigned int target = 0u;
    unsigned int coin_before = 0u;
    unsigned int coin_after = 0u;
    unsigned int reward_count = 0u;
    unsigned int mail_count = 0u;
    unsigned int plan_state = 0u;
    const int plan_bytes = static_cast<int>(sizeof(response.deliveries));
    if (!db->get_uint(0, charac_no) || !db->get_uint(1, server_group) ||
        !db->get_uint(2, previous) || !db->get_uint(3, current) ||
        !db->get_uint(4, target) || !db->get_uint(5, coin_before) ||
        !db->get_uint(6, coin_after) ||
        db->get_binary_length(7) != 32 ||
        !db->get_binary(7, response.inventory_pre_sha256, 32) ||
        db->get_binary_length(8) != 32 ||
        !db->get_binary(8, response.inventory_post_sha256, 32) ||
        db->get_binary_length(9) != 32 ||
        !db->get_binary(9, response.reward_plan_sha256, 32) ||
        !db->get_uint(10, reward_count) ||
        !db->get_uint(11, mail_count) ||
        db->get_binary_length(12) != plan_bytes ||
        !db->get_binary(12, response.deliveries, plan_bytes) ||
        !db->get_uint(13, plan_state) ||
        charac_no != response.charac_no ||
        server_group != static_cast<unsigned int>(response.server_group) ||
        previous != response.previous_reward_level ||
        current != response.current_reward_level ||
        target != response.target_level || reward_count == 0u ||
        reward_count > kMaximumRewardDeliveries ||
        mail_count > reward_count || plan_state != kPlanStatePrepared) {
        ClearPlan(response);
        return false;
    }
    response.event_coin_before = coin_before;
    response.event_coin_after = coin_after;
    response.reward_count = static_cast<uint8_t>(reward_count);
    response.mail_count = static_cast<uint8_t>(mail_count);
    response.plan_state = static_cast<uint8_t>(plan_state);
    response.mail_persisted = 0u;
    std::memset(response.ack_delivery_token, 0,
                sizeof(response.ack_delivery_token));
    response.ack_reward_item_id = 0u;
    return IsValidPreparedPlan(response);
}

bool PrepareRewardPlan(MySQL* db, Message& response) {
    if (!db || !IsValidPreparedPlan(response) ||
        !db->set_query("start transaction") || !db->exec(true) ||
        !db->set_query(
            "seLect charac_no,target_level,last_reward_level,claim_token,"
            "pending_reward_level from %s where account_id=%u for update",
            kStateTable,
            static_cast<unsigned int>(response.account_id)) ||
        !db->exec(true)) {
        RollbackTransaction(db);
        return false;
    }
    unsigned int charac_no = 0u;
    unsigned int target_level = 0u;
    unsigned int last_reward_level = 0u;
    unsigned int claim_token = 0u;
    unsigned int pending_reward_level = 0u;
    if (db->get_n_rows() != 1 || !db->fetch() ||
        !db->get_uint(0, charac_no) || !db->get_uint(1, target_level) ||
        !db->get_uint(2, last_reward_level) ||
        !db->get_uint(3, claim_token) ||
        !db->get_uint(4, pending_reward_level) ||
        charac_no != response.charac_no ||
        target_level != response.target_level ||
        last_reward_level != response.previous_reward_level ||
        claim_token != response.claim_token ||
        pending_reward_level != response.current_reward_level) {
        RollbackTransaction(db);
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }

    Message stored = response;
    if (!LoadPreparedPlan(db, stored)) {
        RollbackTransaction(db);
        return false;
    }
    if (stored.plan_state == kPlanStatePrepared) {
        if (!ClaimPlanEquals(stored, response) || !CommitTransaction(db)) {
            RollbackTransaction(db);
            response.result = static_cast<uint8_t>(kResultReservationRequired);
            return true;
        }
        response.result = static_cast<uint8_t>(kResultOk);
        return true;
    }

    char* const pre_sha = db->blob_to_str(
        0, response.inventory_pre_sha256, 32);
    char* const post_sha = db->blob_to_str(
        1, response.inventory_post_sha256, 32);
    char* const plan_sha = db->blob_to_str(
        2, response.reward_plan_sha256, 32);
    char* const plan_blob = db->blob_to_str(
        3, response.deliveries, sizeof(response.deliveries));
    if (!pre_sha || !post_sha || !plan_sha || !plan_blob ||
        !db->set_query(
            "inSert into %s(account_id,claim_token,charac_no,server_group,"
            "previous_reward_level,current_reward_level,target_level,"
            "event_coin_before,event_coin_after,inventory_pre_sha256,"
            "inventory_post_sha256,reward_plan_sha256,reward_count,mail_count,"
            "plan_blob,plan_state,created_at,updated_at) values("
            "%u,%u,%u,%d,%u,%u,%u,%u,%u,'%s','%s','%s',%u,%u,'%s',1,"
            "now(),now())",
            kClaimTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(response.claim_token),
            static_cast<unsigned int>(response.charac_no),
            response.server_group,
            static_cast<unsigned int>(response.previous_reward_level),
            static_cast<unsigned int>(response.current_reward_level),
            static_cast<unsigned int>(response.target_level),
            static_cast<unsigned int>(response.event_coin_before),
            static_cast<unsigned int>(response.event_coin_after),
            pre_sha, post_sha, plan_sha,
            static_cast<unsigned int>(response.reward_count),
            static_cast<unsigned int>(response.mail_count), plan_blob) ||
        !db->exec(true) || !CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool ReserveRewards(MySQL* db, Message& response) {
    const uint32_t requested_charac_no = response.charac_no;
    const uint32_t requested_level = response.current_reward_level;
    if (!requested_charac_no || !requested_level || requested_level > 255u ||
        !db->set_query("start transaction") || !db->exec(true)) {
        return false;
    }
    if (!db->set_query(
            "seLect target_level,ifnull(charac_no,0),main_reward_decided,"
            "last_reward_level,claim_token,pending_reward_level,"
            "pending_main_reward from %s where account_id=%u for update",
            kStateTable,
            static_cast<unsigned int>(response.account_id)) ||
        !db->exec(true)) {
        RollbackTransaction(db);
        return false;
    }
    if (db->get_n_rows() != 1 || !db->fetch()) {
        RollbackTransaction(db);
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }

    unsigned int target_level = 0;
    unsigned int bound_charac_no = 0;
    unsigned int main_reward_decided = 0;
    unsigned int last_reward_level = 0;
    unsigned int claim_token = 0;
    unsigned int pending_reward_level = 0;
    unsigned int pending_main_reward = 0;
    if (!db->get_uint(0, target_level) ||
        !db->get_uint(1, bound_charac_no) ||
        !db->get_uint(2, main_reward_decided) ||
        !db->get_uint(3, last_reward_level) ||
        !db->get_uint(4, claim_token) ||
        !db->get_uint(5, pending_reward_level) ||
        !db->get_uint(6, pending_main_reward)) {
        RollbackTransaction(db);
        return false;
    }
    if (bound_charac_no != requested_charac_no ||
        !IsAllowedTarget(target_level) || main_reward_decided > 1u ||
        last_reward_level > 255u || pending_reward_level > 255u ||
        pending_main_reward > 1u) {
        RollbackTransaction(db);
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }

    if (pending_reward_level != 0u) {
        if (!claim_token || pending_reward_level <= last_reward_level ||
            !CommitTransaction(db)) {
            RollbackTransaction(db);
            return false;
        }
        response.target_level = target_level;
        response.charac_no = bound_charac_no;
        response.previous_reward_level = last_reward_level;
        response.current_reward_level = pending_reward_level;
        response.claim_token = claim_token;
        response.main_reward_decided =
            static_cast<uint8_t>(main_reward_decided != 0u);
        response.main_reward_newly_decided =
            static_cast<uint8_t>(pending_main_reward != 0u);
        if (!LoadPreparedPlan(db, response)) {
            return false;
        }
        response.result = static_cast<uint8_t>(kResultOk);
        return true;
    }

    const uint32_t advance_to = std::max(last_reward_level, requested_level);
    if (advance_to == last_reward_level) {
        if (!CommitTransaction(db)) {
            RollbackTransaction(db);
            return false;
        }
        response.target_level = target_level;
        response.charac_no = bound_charac_no;
        response.previous_reward_level = last_reward_level;
        response.current_reward_level = last_reward_level;
        response.claim_token = 0;
        response.main_reward_decided =
            static_cast<uint8_t>(main_reward_decided != 0u);
        response.main_reward_newly_decided = 0;
        response.result = static_cast<uint8_t>(kResultOk);
        return true;
    }

    uint32_t next_claim_token = static_cast<uint32_t>(claim_token + 1u);
    if (next_claim_token == 0u) {
        next_claim_token = 1u;
    }
    const bool newly_decided =
        main_reward_decided == 0u && advance_to >= target_level;
    if (!db->set_query(
            "upDate %s set claim_token=%u,pending_reward_level=%u,"
            "pending_main_reward=%u where account_id=%u and charac_no=%u",
            kStateTable,
            static_cast<unsigned int>(next_claim_token),
            static_cast<unsigned int>(advance_to),
            static_cast<unsigned int>(newly_decided),
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(bound_charac_no)) ||
        !db->exec(true) || !CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }

    response.charac_no = bound_charac_no;
    response.target_level = target_level;
    response.previous_reward_level = last_reward_level;
    response.current_reward_level = advance_to;
    response.claim_token = next_claim_token;
    response.main_reward_decided =
        static_cast<uint8_t>(main_reward_decided != 0u);
    response.main_reward_newly_decided =
        static_cast<uint8_t>(newly_decided);
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool FinalizeRewards(MySQL* db, Message& response) {
    const Message requested = response;
    if (!db || !requested.account_id || !requested.claim_token ||
        !requested.charac_no ||
        !db->set_query("start transaction") || !db->exec(true) ||
        !db->set_query(
            "seLect target_level,ifnull(charac_no,0),main_reward_decided,"
            "last_reward_level,claim_token,pending_reward_level,"
            "pending_main_reward from %s where account_id=%u for update",
            kStateTable,
            static_cast<unsigned int>(requested.account_id)) ||
        !db->exec(true)) {
        RollbackTransaction(db);
        return false;
    }

    unsigned int target_level = 0u;
    unsigned int bound_charac_no = 0u;
    unsigned int main_reward_decided = 0u;
    unsigned int last_reward_level = 0u;
    unsigned int claim_token = 0u;
    unsigned int pending_reward_level = 0u;
    unsigned int pending_main_reward = 0u;
    if (db->get_n_rows() != 1 || !db->fetch() ||
        !db->get_uint(0, target_level) ||
        !db->get_uint(1, bound_charac_no) ||
        !db->get_uint(2, main_reward_decided) ||
        !db->get_uint(3, last_reward_level) ||
        !db->get_uint(4, claim_token) ||
        !db->get_uint(5, pending_reward_level) ||
        !db->get_uint(6, pending_main_reward) ||
        bound_charac_no != requested.charac_no ||
        claim_token != requested.claim_token ||
        target_level != requested.target_level ||
        last_reward_level != requested.previous_reward_level ||
        pending_reward_level != requested.current_reward_level ||
        !IsAllowedTarget(target_level) || main_reward_decided > 1u ||
        pending_main_reward > 1u ||
        pending_reward_level <= last_reward_level ||
        pending_reward_level > 255u) {
        RollbackTransaction(db);
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }

    Message stored = requested;
    ClearPlan(stored);
    stored.account_id = requested.account_id;
    stored.charac_no = bound_charac_no;
    stored.server_group = requested.server_group;
    stored.target_level = target_level;
    stored.previous_reward_level = last_reward_level;
    stored.current_reward_level = pending_reward_level;
    stored.claim_token = claim_token;
    if (!LoadPreparedPlan(db, stored) ||
        stored.plan_state != kPlanStatePrepared ||
        (requested.plan_state == kPlanStatePrepared &&
         !ClaimPlanEquals(stored, requested))) {
        RollbackTransaction(db);
        response.result = static_cast<uint8_t>(kResultReservationRequired);
        return true;
    }

    unsigned int persisted_mail = 0u;
    for (uint8_t index = 0u; index < stored.reward_count; ++index) {
        const RewardDelivery& delivery = stored.deliveries[index];
        if (delivery.kind != kDeliveryTrackedMail) continue;
        char* const token = db->blob_to_str(
            0, const_cast<uint8_t*>(delivery.delivery_token),
            sizeof(delivery.delivery_token));
        if (!token ||
            !db->set_query(
                "seLect reward_item_id,persisted from %s where "
                "account_id=%u and claim_token=%u and delivery_token='%s' "
                "limit 2",
                kMailAckTable,
                static_cast<unsigned int>(stored.account_id),
                static_cast<unsigned int>(stored.claim_token), token) ||
            !db->exec(true)) {
            RollbackTransaction(db);
            return false;
        }
        if (db->get_n_rows() == 0) continue;
        unsigned int acknowledged_item_id = 0u;
        unsigned int acknowledged_persisted = 0u;
        if (db->get_n_rows() != 1 || !db->fetch() ||
            !db->get_uint(0, acknowledged_item_id) ||
            !db->get_uint(1, acknowledged_persisted) ||
            acknowledged_item_id != delivery.item_id ||
            acknowledged_persisted != 1u) {
            RollbackTransaction(db);
            return false;
        }
        ++persisted_mail;
    }
    if (persisted_mail > stored.mail_count) {
        RollbackTransaction(db);
        return false;
    }

    const bool inventory_durable =
        inventory_mutation_barrier::
            HasExactPersistedInventoryImageAndEventCoin(
                stored.server_group,
                stored.charac_no,
                stored.inventory_post_sha256,
                stored.event_coin_after);
    if (persisted_mail != stored.mail_count || !inventory_durable) {
        RollbackTransaction(db);
        response = stored;
        response.operation = kOperationFinalizeRewards;
        response.result = static_cast<uint8_t>(kResultDurabilityPending);
        response.mail_persisted = static_cast<uint8_t>(persisted_mail);
        return true;
    }

    if (!db->set_query(
            "upDate %s set last_reward_level=%u,main_reward_decided=%u,"
            "pending_reward_level=0,pending_main_reward=0 "
            "where account_id=%u and charac_no=%u and claim_token=%u",
            kStateTable,
            pending_reward_level,
            static_cast<unsigned int>(main_reward_decided != 0u ||
                                      pending_main_reward != 0u),
            static_cast<unsigned int>(stored.account_id),
            bound_charac_no,
            claim_token) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u ||
        !db->set_query(
            "upDate %s set plan_state=2,updated_at=now() where account_id=%u "
            "and claim_token=%u and plan_state=1",
            kClaimTable,
            static_cast<unsigned int>(stored.account_id),
            static_cast<unsigned int>(stored.claim_token)) ||
        !db->exec(true) || db->getAffectedRowCount() != 1u ||
        !CommitTransaction(db)) {
        RollbackTransaction(db);
        return false;
    }

    response = stored;
    response.operation = kOperationFinalizeRewards;
    response.plan_state = kPlanStateFinalized;
    response.main_reward_decided = static_cast<uint8_t>(
        main_reward_decided != 0u || pending_main_reward != 0u);
    response.main_reward_newly_decided =
        static_cast<uint8_t>(pending_main_reward != 0u);
    response.mail_persisted = stored.mail_count;
    response.result = static_cast<uint8_t>(kResultOk);
    return true;
}

bool ResolveMailAck(MySQL* db, Message& response) {
    uint8_t delivery_token[16];
    std::memcpy(delivery_token, response.ack_delivery_token,
                sizeof(delivery_token));
    const uint32_t reward_item_id = response.ack_reward_item_id;
    const uint8_t persisted = response.mail_persisted;
    const uint32_t session_token = response.session_token;
    if (!db || DeliveryTokenIsZero(delivery_token) || !reward_item_id ||
        !response.charac_no || !IsValidServerGroup(response.server_group) ||
        persisted > 1u ||
        !db->set_query(
            "seLect s.account_id,s.claim_token,s.target_level,"
            "s.last_reward_level,s.pending_reward_level,s.pending_main_reward,"
            "s.main_reward_decided from %s s join %s c on "
            "c.account_id=s.account_id and c.claim_token=s.claim_token where "
            "c.charac_no=%u and c.server_group=%d and c.plan_state=1 and "
            "s.pending_reward_level<>0 limit 2",
            kStateTable,
            kClaimTable,
            static_cast<unsigned int>(response.charac_no),
            response.server_group) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }
    unsigned int account_id = 0u;
    unsigned int claim_token = 0u;
    unsigned int target_level = 0u;
    unsigned int previous = 0u;
    unsigned int current = 0u;
    unsigned int pending_main = 0u;
    unsigned int main_decided = 0u;
    if (!db->get_uint(0, account_id) || !db->get_uint(1, claim_token) ||
        !db->get_uint(2, target_level) || !db->get_uint(3, previous) ||
        !db->get_uint(4, current) || !db->get_uint(5, pending_main) ||
        !db->get_uint(6, main_decided) || !account_id || !claim_token ||
        current <= previous || pending_main > 1u || main_decided > 1u) {
        return false;
    }
    response.account_id = account_id;
    response.claim_token = claim_token;
    response.target_level = target_level;
    response.previous_reward_level = previous;
    response.current_reward_level = current;
    response.main_reward_newly_decided = static_cast<uint8_t>(pending_main);
    response.main_reward_decided = static_cast<uint8_t>(main_decided);
    response.session_token = session_token;
    ClearPlan(response);
    response.session_token = session_token;
    if (!LoadPreparedPlan(db, response)) return false;

    bool delivery_matches = false;
    for (uint8_t index = 0u; index < response.reward_count; ++index) {
        const RewardDelivery& delivery = response.deliveries[index];
        if (delivery.kind == kDeliveryTrackedMail &&
            std::memcmp(delivery.delivery_token, delivery_token,
                        sizeof(delivery_token)) == 0 &&
            delivery.item_id == reward_item_id) {
            delivery_matches = true;
            break;
        }
    }
    if (!delivery_matches) return false;

    std::memcpy(response.ack_delivery_token, delivery_token,
                sizeof(delivery_token));
    response.ack_reward_item_id = reward_item_id;
    response.mail_persisted = persisted;
    if (!persisted) {
        response.operation = kOperationMailPersistenceAck;
        response.result = static_cast<uint8_t>(kResultDurabilityPending);
        return true;
    }
    char* token_blob = db->blob_to_str(
        0, delivery_token, sizeof(delivery_token));
    if (!token_blob || !db->set_query(
            "inSert ignore into %s(account_id,claim_token,delivery_token,"
            "reward_item_id,persisted,updated_at) values(%u,%u,'%s',%u,1,"
            "now())",
            kMailAckTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(response.claim_token),
            token_blob,
            static_cast<unsigned int>(reward_item_id)) ||
        !db->exec(true)) {
        return false;
    }
    token_blob = db->blob_to_str(0, delivery_token, sizeof(delivery_token));
    if (!token_blob ||
        !db->set_query(
            "seLect reward_item_id,persisted from %s where account_id=%u "
            "and claim_token=%u and delivery_token='%s' limit 2",
            kMailAckTable,
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(response.claim_token), token_blob) ||
        !db->exec(true) || db->get_n_rows() != 1 || !db->fetch()) {
        return false;
    }
    unsigned int acknowledged_item_id = 0u;
    unsigned int acknowledged_persisted = 0u;
    if (!db->get_uint(0, acknowledged_item_id) ||
        !db->get_uint(1, acknowledged_persisted) ||
        acknowledged_item_id != reward_item_id ||
        acknowledged_persisted != 1u) {
        return false;
    }
    return FinalizeRewards(db, response);
}

bool RollbackRewards(MySQL* db, Message& response) {
    (void)db;
    // Fail closed: a delivery failure must never erase the reserved
    // watermark.  Recovery replays the durable plan until both DB3 proofs and
    // all tracked-mail acknowledgements exist.
    response.result = static_cast<uint8_t>(kResultDurabilityPending);
    return response.charac_no != 0u && response.claim_token != 0u;
}

}  // namespace

DB_DecideLevelupEvent::DB_DecideLevelupEvent() : DBDispatcher() {}
DB_DecideLevelupEvent::~DB_DecideLevelupEvent() {}

bool DB_DecideLevelupEvent::Queue(int user_idx, const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    const bool mail_ack =
        request.operation == kOperationMailPersistenceAck;
    // Finalize is intentionally excluded from the direct queue-6 producer.
    // It must enter through QueueAfterLegacyInventory(), which puts signal
    // 823 behind the official signal-35 snapshot on the legacy DB queue.
    if (request.operation == kOperationFinalizeRewards ||
        (!mail_ack &&
         (user_idx == -1 || !request.account_id || !request.session_token)) ||
        (mail_ack &&
         (!request.charac_no ||
          DeliveryTokenIsZero(request.ack_delivery_token) ||
           !request.ack_reward_item_id || request.mail_persisted > 1u)) ||
        !IsValidServerGroup(request.server_group) || !pool || !queues) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) {
        return false;
    }
    *payload = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_DecideLevelupEvent::QueueAfterLegacyInventory(
    int user_idx,
    const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    if (user_idx == -1 || request.operation != kOperationFinalizeRewards ||
        !request.account_id || !request.charac_no || !request.claim_token ||
        !request.session_token || !IsValidServerGroup(request.server_group) ||
        !IsValidPreparedPlan(request) || !pool) {
        return false;
    }
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const payload = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!payload) return false;
    *payload = request;
    return legacy_feature_fence::QueueAfterLegacy(
        legacy_feature_fence::kFenceDecideLevelupFinalize, guard);
}

bool DB_DecideLevelupEvent::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    const bool mail_ack = request &&
        request->operation == kOperationMailPersistenceAck;
    if (!request ||
        (!mail_ack && (!request->account_id || !request->session_token)) ||
        (mail_ack &&
         (!request->charac_no ||
          DeliveryTokenIsZero(request->ack_delivery_token) ||
           !request->ack_reward_item_id || request->mail_persisted > 1u)) ||
        !IsValidServerGroup(request->server_group)) {
        return false;
    }

    Message response;
    InitResponse(response, *request);
    MySQL* const db = GetGameDb(*request);
    bool handled = false;
    if (db) {
        switch (request->operation) {
        case kOperationLoad:
            handled = Load(db, response);
            break;
        case kOperationDecide:
            handled = Decide(db, response);
            break;
        case kOperationBindCharacter:
            handled = BindCharacter(db, response);
            break;
        case kOperationReserveRewards:
            handled = ReserveRewards(db, response);
            break;
        case kOperationPrepareRewardPlan:
            handled = PrepareRewardPlan(db, response);
            break;
        case kOperationFinalizeRewards:
            handled = FinalizeRewards(db, response);
            break;
        case kOperationRollbackRewards:
            handled = RollbackRewards(db, response);
            break;
        case kOperationMailPersistenceAck:
            handled = ResolveMailAck(db, response);
            break;
        default:
            break;
        }
    }
    if (!handled) {
        response.result = static_cast<uint8_t>(kResultOperationFailed);
    }
    return SendResult(user_idx, response);
}

}  // namespace decide_levelup_db
