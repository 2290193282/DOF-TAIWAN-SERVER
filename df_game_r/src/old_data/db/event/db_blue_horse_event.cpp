#include "old_data/db/event/db_blue_horse_event.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"

#include <cstring>
#include <limits>

namespace blue_horse_event_db {
namespace {

const char kDayTable[] = "blue_horse_charac_day_0828";
const char kClaimTable[] = "blue_horse_reward_claim_0828";
const uint64_t kMailNamespace = 0xD829000000000000ULL;
const uint64_t kMaximumClaimId = 0x00007FFFFFFFFFFFULL;

struct DayState {
    DayState()
        : activity_day(0u), fatigue_remainder(0u), ticket_count(0u),
          distance(0u), completed(0u), round_ordinal(0u) {
        slots[0] = slots[1] = slots[2] = 0u;
    }
    uint32_t activity_day;
    uint32_t fatigue_remainder;
    uint32_t ticket_count;
    uint32_t distance;
    uint32_t slots[kSlotCount];
    uint32_t completed;
    uint32_t round_ordinal;
};

struct StoredClaim {
    StoredClaim()
        : claim_id(0ULL), charac_no(0u), activity_day(0u),
          round_ordinal(0u), distance_after(0u), completed_after(0u),
          combo_symbol(0u), combo_repeat(0u), combo_item_id(0u),
          combo_item_count(0u), complete_item_id(0u),
          complete_item_count(0u), phase(PHASE_NONE) {
        slots[0] = slots[1] = slots[2] = 0u;
    }
    uint64_t claim_id;
    uint32_t charac_no;
    uint32_t activity_day;
    uint32_t round_ordinal;
    uint32_t slots[kSlotCount];
    uint32_t distance_after;
    uint32_t completed_after;
    uint32_t combo_symbol;
    uint32_t combo_repeat;
    uint32_t combo_item_id;
    uint32_t combo_item_count;
    uint32_t complete_item_id;
    uint32_t complete_item_count;
    uint32_t phase;
};

bool IsValidServerGroup(int server_group) {
    return server_group >= 0 && server_group < kMaximumServerGroupCount;
}

bool IsZeroFilled(const char* text, unsigned int size) {
    if (!text) return false;
    for (unsigned int index = 0u; index < size; ++index) {
        if (text[index] != 0) return false;
    }
    return true;
}

bool IsCanonicalText(const char* text, unsigned int size) {
    if (!text || size < 2u || text[0] == 0 || text[size - 1u] != 0) {
        return false;
    }
    unsigned int end = 1u;
    while (end < size && text[end] != 0) ++end;
    if (end == size) return false;
    for (unsigned int index = end + 1u; index < size; ++index) {
        if (text[index] != 0) return false;
    }
    return true;
}

bool IsValidMailReward(const MailReward& reward,
                       uint32_t item_id,
                       uint32_t item_count) {
    return item_id != 0u && item_count != 0u &&
           reward.item_id == item_id && reward.add_info == item_count &&
           reward.upgrade == (reward.item_attr & 0x1fu) &&
           reward.avatar_flag <= 1u && reward.creature_flag <= 1u;
}

bool IsZeroReward(const MailReward& reward) {
    const uint8_t* const bytes =
        reinterpret_cast<const uint8_t*>(&reward);
    for (unsigned int index = 0u; index < sizeof(reward); ++index) {
        if (bytes[index] != 0u) return false;
    }
    return true;
}

bool IsValidRequest(const Message& message) {
    if (message.account_id == 0u || message.charac_no == 0u ||
        message.session_token == 0u || message.request_token == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.event_active != 1u || message.show_ui != 0u ||
        message.daily_reset_hour > 23u ||
        message.operation < OPERATION_LOAD_BLUE_HORSE ||
        message.operation > OPERATION_COMMIT_BLUE_HORSE_REWARD) {
        return false;
    }
    if (message.operation == OPERATION_LOAD_BLUE_HORSE) {
        return message.reward_charac_no == 0u &&
               message.claim_id == 0ULL && message.fatigue_delta == 0u &&
               message.slot_index == 0u && message.roll_result == 0u &&
               message.reward_count == 0u &&
               IsZeroReward(message.rewards[0]) &&
               IsZeroReward(message.rewards[1]) &&
               IsZeroFilled(message.combo_mail_title,
                            sizeof(message.combo_mail_title)) &&
               IsZeroFilled(message.combo_mail_content,
                            sizeof(message.combo_mail_content)) &&
               IsZeroFilled(message.complete_mail_title,
                            sizeof(message.complete_mail_title)) &&
               IsZeroFilled(message.complete_mail_content,
                            sizeof(message.complete_mail_content));
    }
    if (message.operation == OPERATION_ADD_BLUE_HORSE_FATIGUE) {
        return message.reward_charac_no == 0u &&
               message.claim_id == 0ULL && message.fatigue_delta > 0u &&
               message.fatigue_delta <= 10000u &&
               message.reward_count == 0u;
    }
    if (message.operation == OPERATION_ROLL_BLUE_HORSE_SLOT) {
        return message.reward_charac_no == 0u &&
               message.claim_id == 0ULL &&
               message.slot_index < kSlotCount &&
               message.roll_result >= 1u && message.roll_result <= 4u &&
               message.fatigue_delta == 0u && message.reward_count == 0u;
    }
    if (message.operation == OPERATION_RESERVE_BLUE_HORSE_REWARD) {
        return message.reward_charac_no == 0u &&
               message.claim_id == 0ULL && message.fatigue_delta == 0u &&
               message.reward_count == 0u;
    }

    if (message.reward_charac_no == 0u || message.claim_id == 0ULL ||
        message.claim_id > kMaximumClaimId || message.phase != PHASE_RESERVED ||
        message.combo_reward_sent > 1u ||
        message.complete_reward_sent > 1u ||
        message.reward_count != static_cast<uint8_t>(
            message.combo_reward_sent + message.complete_reward_sent)) {
        return false;
    }
    unsigned int reward_index = 0u;
    if (message.combo_reward_sent != 0u) {
        if (!IsValidMailReward(message.rewards[reward_index++],
                               message.combo_item_id,
                               message.combo_item_count) ||
            !IsCanonicalText(message.combo_mail_title,
                             sizeof(message.combo_mail_title)) ||
            !IsCanonicalText(message.combo_mail_content,
                             sizeof(message.combo_mail_content))) {
            return false;
        }
    } else if (message.combo_item_id != 0u ||
               message.combo_item_count != 0u ||
               !IsZeroFilled(message.combo_mail_title,
                             sizeof(message.combo_mail_title)) ||
               !IsZeroFilled(message.combo_mail_content,
                             sizeof(message.combo_mail_content))) {
        return false;
    }
    if (message.complete_reward_sent != 0u) {
        if (!IsValidMailReward(message.rewards[reward_index++],
                               message.complete_item_id,
                               message.complete_item_count) ||
            !IsCanonicalText(message.complete_mail_title,
                             sizeof(message.complete_mail_title)) ||
            !IsCanonicalText(message.complete_mail_content,
                             sizeof(message.complete_mail_content))) {
            return false;
        }
    } else if (message.complete_item_id != 0u ||
               message.complete_item_count != 0u ||
               !IsZeroFilled(message.complete_mail_title,
                             sizeof(message.complete_mail_title)) ||
               !IsZeroFilled(message.complete_mail_content,
                             sizeof(message.complete_mail_content))) {
        return false;
    }
    while (reward_index < kMaximumRewardCount) {
        if (!IsZeroReward(message.rewards[reward_index++])) return false;
    }
    return true;
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
    unsigned int count = 0u;
    return database &&
           database->set_query(
               "seLect count(*) from charac_info where charac_no=%u and "
               "m_id=%u and delete_flag=0",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.account_id)) &&
           database->exec(true) && database->get_n_rows() == 1 &&
           database->fetch() && database->get_uint(0, count) && count == 1u;
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

bool LockDayState(MySQL* database,
                  const Message& request,
                  uint32_t activity_day,
                  DayState& state,
                  bool& inserted) {
    inserted = false;
    if (!database || activity_day == 0u ||
        !database->set_query(
            "inSert ignore into %s(server_group,m_id,charac_no,activity_day,"
            "fatigue_remainder,ticket_count,distance,slot_0,slot_1,slot_2,"
            "completed,round_ordinal,created_at,updated_at) values("
            "%u,%u,%u,%u,0,0,0,0,0,0,0,0,now(),now())",
            kDayTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day)) ||
        !database->exec(true) || database->getAffectedRowCount() > 1u) {
        return false;
    }
    inserted = database->getAffectedRowCount() == 1u;
    if (
        !database->set_query(
            "seLect fatigue_remainder,ticket_count,distance,slot_0,slot_1,"
            "slot_2,completed,round_ordinal from %s where server_group=%u "
            "and m_id=%u and charac_no=%u and activity_day=%u for update",
            kDayTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() ||
        !database->get_uint(0, state.fatigue_remainder) ||
        !database->get_uint(1, state.ticket_count) ||
        !database->get_uint(2, state.distance) ||
        !database->get_uint(3, state.slots[0]) ||
        !database->get_uint(4, state.slots[1]) ||
        !database->get_uint(5, state.slots[2]) ||
        !database->get_uint(6, state.completed) ||
        !database->get_uint(7, state.round_ordinal)) {
        return false;
    }
    state.activity_day = activity_day;
    return state.fatigue_remainder < 8u && state.ticket_count <= 255u &&
           state.distance <= 8u && state.slots[0] <= 4u &&
           state.slots[1] <= 4u && state.slots[2] <= 4u &&
           state.completed <= 1u;
}

bool LockDayState(MySQL* database,
                  const Message& request,
                  uint32_t activity_day,
                  DayState& state) {
    bool inserted = false;
    return LockDayState(database, request, activity_day, state, inserted);
}

void ExportState(const DayState& state, Message& response) {
    response.activity_day = state.activity_day;
    response.fatigue_remainder =
        static_cast<uint8_t>(state.fatigue_remainder);
    response.ticket_count = static_cast<uint8_t>(state.ticket_count);
    response.distance = static_cast<uint8_t>(state.distance);
    for (unsigned int index = 0u; index < kSlotCount; ++index) {
        response.slots[index] = static_cast<uint8_t>(state.slots[index]);
    }
    response.completed = static_cast<uint8_t>(state.completed);
}

bool ReadClaimRow(MySQL* database, StoredClaim& claim) {
    return database && database->get_n_rows() == 1 && database->fetch() &&
           database->get_ulonglong(0, claim.claim_id) &&
           database->get_uint(1, claim.charac_no) &&
           database->get_uint(2, claim.activity_day) &&
           database->get_uint(3, claim.round_ordinal) &&
           database->get_uint(4, claim.slots[0]) &&
           database->get_uint(5, claim.slots[1]) &&
           database->get_uint(6, claim.slots[2]) &&
           database->get_uint(7, claim.distance_after) &&
           database->get_uint(8, claim.completed_after) &&
           database->get_uint(9, claim.combo_symbol) &&
           database->get_uint(10, claim.combo_repeat) &&
           database->get_uint(11, claim.combo_item_id) &&
           database->get_uint(12, claim.combo_item_count) &&
           database->get_uint(13, claim.complete_item_id) &&
           database->get_uint(14, claim.complete_item_count) &&
           database->get_uint(15, claim.phase) &&
           claim.claim_id != 0ULL && claim.claim_id <= kMaximumClaimId &&
           claim.charac_no != 0u && claim.activity_day != 0u &&
           claim.slots[0] >= 1u && claim.slots[0] <= 4u &&
           claim.slots[1] >= 1u && claim.slots[1] <= 4u &&
           claim.slots[2] >= 1u && claim.slots[2] <= 4u &&
           claim.distance_after <= 8u && claim.completed_after <= 1u &&
           claim.phase >= PHASE_RESERVED && claim.phase <= PHASE_DELIVERED;
}

bool FindPendingClaim(MySQL* database,
                      const Message& request,
                      uint32_t activity_day,
                      StoredClaim& claim) {
    return database &&
           database->set_query(
               "seLect claim_id,charac_no,activity_day,round_ordinal,slot_0,"
               "slot_1,slot_2,distance_after,completed_after,combo_symbol,"
               "combo_repeat,combo_item_id,combo_item_count,complete_item_id,"
               "complete_item_count,claim_state from %s where server_group=%u "
               "and m_id=%u and charac_no=%u and activity_day=%u and "
               "claim_state=1 limit 2 for update",
               kClaimTable,
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(activity_day)) &&
           database->exec(true) &&
           (database->get_n_rows() == 0 || ReadClaimRow(database, claim));
}

bool ReadClaimById(MySQL* database,
                   const Message& request,
                   uint64_t claim_id,
                   StoredClaim& claim) {
    return database && claim_id != 0ULL &&
           database->set_query(
               "seLect claim_id,charac_no,activity_day,round_ordinal,slot_0,"
               "slot_1,slot_2,distance_after,completed_after,combo_symbol,"
               "combo_repeat,combo_item_id,combo_item_count,complete_item_id,"
               "complete_item_count,claim_state from %s where claim_id=%llu "
               "and server_group=%u and m_id=%u and charac_no=%u limit 2 "
               "for update",
               kClaimTable,
               static_cast<unsigned long long>(claim_id),
               static_cast<unsigned int>(request.server_group),
               static_cast<unsigned int>(request.account_id),
               static_cast<unsigned int>(request.charac_no)) &&
           database->exec(true) && ReadClaimRow(database, claim);
}

void ExportClaim(const StoredClaim& claim, Message& response) {
    response.claim_id = claim.claim_id;
    response.reward_charac_no = claim.charac_no;
    response.activity_day = claim.activity_day;
    response.phase = static_cast<uint8_t>(claim.phase);
    response.combo_symbol = static_cast<uint8_t>(claim.combo_symbol);
    response.combo_repeat = static_cast<uint8_t>(claim.combo_repeat);
    response.combo_item_id = claim.combo_item_id;
    response.combo_item_count = claim.combo_item_count;
    response.complete_item_id = claim.complete_item_id;
    response.complete_item_count = claim.complete_item_count;
    response.combo_reward_sent = claim.combo_item_id != 0u ? 1u : 0u;
    response.complete_reward_sent =
        claim.complete_item_id != 0u ? 1u : 0u;
}

void ResolveCombo(const DayState& state,
                  uint32_t& symbol,
                  uint32_t& repeat) {
    symbol = 0u;
    repeat = 1u;
    for (uint32_t candidate = 1u; candidate <= 4u; ++candidate) {
        uint32_t count = 0u;
        for (unsigned int index = 0u; index < kSlotCount; ++index) {
            if (state.slots[index] == candidate) ++count;
        }
        if (count > repeat) {
            symbol = candidate;
            repeat = count;
        }
    }
}

bool ResolveComboReward(uint32_t symbol,
                        uint32_t repeat,
                        uint32_t& item_id,
                        uint32_t& item_count) {
    item_id = 0u;
    item_count = 0u;
    if (repeat < 2u || repeat > 3u || symbol < 1u || symbol > 4u) {
        return repeat == 1u;
    }
    static const uint32_t kPair[4][2] = {
        {690001155u, 1u}, {8u, 5u}, {42u, 1u}, {690001156u, 1u},
    };
    static const uint32_t kTriple[4][2] = {
        {690001157u, 1u}, {900u, 2u}, {15u, 1u}, {42u, 10u},
    };
    const uint32_t (*table)[2] = repeat == 3u ? kTriple : kPair;
    item_id = table[symbol - 1u][0];
    item_count = table[symbol - 1u][1];
    return true;
}

bool IsWeekend(MySQL* database,
               uint8_t daily_reset_hour,
               bool& weekend) {
    unsigned int day = 0u;
    if (!database || daily_reset_hour > 23u ||
        !database->set_query(
            "seLect dayofweek(date_sub(now(),interval %u hour))",
            static_cast<unsigned int>(daily_reset_hour)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_uint(0, day) ||
        day < 1u || day > 7u) {
        return false;
    }
    weekend = day == 1u || day == 7u;
    return true;
}

uint64_t DeliveryKey(uint64_t claim_id, unsigned int index) {
    return kMailNamespace | (claim_id << 1u) |
           static_cast<uint64_t>(index & 1u);
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
    if (!database || !title ||
        database->escape_string(escaped_title, title) < 0 ||
        !database->set_query(
            "seLect receive_charac_no,item_id,add_info,letter_id,"
            "length(send_charac_name),binary send_charac_name='%s',"
            "seal_flag,endurance,upgrade,amplify_option,amplify_value,"
            "creature_flag,avata_flag,extend_info,chronicle_value,"
            "seperate_upgrade from postal where auction_id=%llu limit 2",
            escaped_title,
            static_cast<unsigned long long>(delivery_key)) ||
        !database->exec(true)) {
        return false;
    }
    if (database->get_n_rows() == 0) return true;
    unsigned int values[16] = {};
    if (database->get_n_rows() != 1 || !database->fetch() ||
        !database->get_uint(0, values[0]) ||
        !database->get_uint(1, values[1]) ||
        !database->get_uint(2, values[2]) ||
        !database->get_uint(3, values[3]) ||
        !database->get_uint(4, values[4]) ||
        !database->get_uint(5, values[5]) ||
        !database->get_uint(6, values[6]) ||
        !database->get_uint(7, values[7]) ||
        !database->get_uint(8, values[8]) ||
        !database->get_uint(9, values[9]) ||
        !database->get_uint(10, values[10]) ||
        !database->get_uint(11, values[11]) ||
        !database->get_uint(12, values[12]) ||
        !database->get_uint(13, values[13]) ||
        !database->get_uint(14, values[14]) ||
        !database->get_uint(15, values[15]) ||
        values[0] != receiver || values[1] != reward.item_id ||
        values[2] != reward.add_info || values[3] == 0u ||
        values[3] > static_cast<unsigned int>(
            std::numeric_limits<int>::max()) ||
        values[4] != std::strlen(title) || values[5] != 1u ||
        values[6] != reward.seal_flag ||
        values[7] != reward.endurance ||
        values[8] != reward.item_attr ||
        values[9] != reward.amplify_option ||
        values[10] != reward.amplify_value ||
        values[11] != reward.creature_flag ||
        values[12] != reward.avatar_flag ||
        values[13] != reward.extend_info ||
        values[14] != reward.chronicle_value ||
        values[15] != reward.separate_upgrade) {
        return false;
    }
    found = true;
    letter_id = values[3];
    return true;
}

bool ValidateLetter(MySQL* database,
                    uint32_t letter_id,
                    uint32_t receiver,
                    const char* title,
                    const char* content) {
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    if (!database || letter_id == 0u || !title || !content ||
        database->escape_string(escaped_title, title) < 0 ||
        database->escape_string(escaped_content, content) < 0 ||
        !database->set_query(
            "seLect charac_no,send_charac_no,stat,"
            "length(send_charac_name),length(letter_text),"
            "binary send_charac_name='%s',binary letter_text='%s' "
            "from letter where letter_id=%u limit 2",
            escaped_title, escaped_content,
            static_cast<unsigned int>(letter_id)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int values[7] = {};
    return database->get_uint(0, values[0]) &&
           database->get_uint(1, values[1]) &&
           database->get_uint(2, values[2]) &&
           database->get_uint(3, values[3]) &&
           database->get_uint(4, values[4]) &&
           database->get_uint(5, values[5]) &&
           database->get_uint(6, values[6]) && values[0] == receiver &&
           values[1] == 0u && values[2] == 0u &&
           values[3] == std::strlen(title) &&
           values[4] == std::strlen(content) && values[5] == 1u &&
           values[6] == 1u;
}

bool InsertLetter(MySQL* database,
                  uint32_t receiver,
                  const char* title,
                  const char* content,
                  uint32_t& letter_id) {
    letter_id = 0u;
    char escaped_title[(kMailTitleBytes + 1u) * 2u + 1u] = {};
    char escaped_content[(kMailContentBytes + 1u) * 2u + 1u] = {};
    if (!database || !title || !content ||
        database->escape_string(escaped_title, title) < 0 ||
        database->escape_string(escaped_content, content) < 0 ||
        !database->set_query(
            "inSert into letter(charac_no,send_charac_no,send_charac_name,"
            "letter_text,reg_date,stat) values(%u,0,'%s','%s',now(),0)",
            static_cast<unsigned int>(receiver), escaped_title,
            escaped_content) || !database->exec(true)) {
        return false;
    }
    letter_id = WongWork::DBCommon::GetIdentity(database);
    return letter_id != 0u &&
           letter_id <= static_cast<unsigned int>(
               std::numeric_limits<int>::max());
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
               static_cast<unsigned int>(reward.amplify_value),
               escaped_title,
               static_cast<unsigned int>(reward.creature_flag),
               static_cast<unsigned int>(reward.avatar_flag),
               static_cast<unsigned int>(letter_id),
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
    bool found = false;
    uint32_t letter_id = 0u;
    if (!FindMail(database, delivery_key, receiver, reward, title,
                  found, letter_id)) {
        return false;
    }
    if (found) {
        return ValidateLetter(database, letter_id, receiver, title, content);
    }
    return InsertLetter(database, receiver, title, content, letter_id) &&
           ValidateLetter(database, letter_id, receiver, title, content) &&
           (InsertMail(database, delivery_key, receiver, reward, title,
                       letter_id) ||
            (FindMail(database, delivery_key, receiver, reward, title,
                      found, letter_id) && found));
}

bool HandleLoad(MySQL* database,
                const Message& request,
                Message& response) {
    uint32_t activity_day = 0u;
    DayState state;
    bool inserted = false;
    if (!LoadOwnedCharacter(database, request) ||
        !ReadActivityDay(database, request, activity_day) ||
        !Begin(database) ||
        !LockDayState(database, request, activity_day, state, inserted) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    ExportState(state, response);
    response.show_ui = inserted ? 1u : 0u;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleFatigue(MySQL* database,
                   const Message& request,
                   Message& response) {
    uint32_t activity_day = 0u;
    DayState state;
    if (!LoadOwnedCharacter(database, request) ||
        !ReadActivityDay(database, request, activity_day) ||
        !Begin(database) ||
        !LockDayState(database, request, activity_day, state)) {
        Rollback(database);
        return false;
    }
    if (state.completed == 0u) {
        const uint32_t total = state.fatigue_remainder +
                               request.fatigue_delta;
        const uint32_t earned = total / 8u;
        state.fatigue_remainder = total % 8u;
        state.ticket_count = state.ticket_count + earned > 255u
                                 ? 255u : state.ticket_count + earned;
        if (!database->set_query(
                "upDate %s set fatigue_remainder=%u,ticket_count=%u,"
                "updated_at=now() where server_group=%u and m_id=%u and "
                "charac_no=%u and activity_day=%u",
                kDayTable,
                static_cast<unsigned int>(state.fatigue_remainder),
                static_cast<unsigned int>(state.ticket_count),
                static_cast<unsigned int>(request.server_group),
                static_cast<unsigned int>(request.account_id),
                static_cast<unsigned int>(request.charac_no),
                static_cast<unsigned int>(activity_day)) ||
            !database->exec(true) || database->getAffectedRowCount() > 1u) {
            Rollback(database);
            return false;
        }
    }
    if (!Commit(database)) {
        Rollback(database);
        return false;
    }
    ExportState(state, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleRoll(MySQL* database,
                const Message& request,
                Message& response) {
    uint32_t activity_day = 0u;
    DayState state;
    if (!LoadOwnedCharacter(database, request) ||
        !ReadActivityDay(database, request, activity_day) ||
        !Begin(database) ||
        !LockDayState(database, request, activity_day, state)) {
        Rollback(database);
        return false;
    }
    if (state.completed != 0u) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (state.slots[request.slot_index] != 0u) {
        Rollback(database);
        response.result = RESULT_SLOT_ALREADY_ROLLED;
        return true;
    }
    if (state.ticket_count == 0u) {
        Rollback(database);
        response.result = RESULT_NO_TICKET;
        return true;
    }
    static const char* const kSlotColumns[kSlotCount] = {
        "slot_0", "slot_1", "slot_2",
    };
    --state.ticket_count;
    state.slots[request.slot_index] = request.roll_result;
    if (!database->set_query(
            "upDate %s set ticket_count=%u,%s=%u,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and "
            "activity_day=%u",
            kDayTable,
            static_cast<unsigned int>(state.ticket_count),
            kSlotColumns[request.slot_index],
            static_cast<unsigned int>(request.roll_result),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    ExportState(state, response);
    response.roll_result = request.roll_result;
    response.result = RESULT_SUCCESS;
    return true;
}

bool HandleReserve(MySQL* database,
                   const Message& request,
                   Message& response) {
    uint32_t activity_day = 0u;
    DayState state;
    if (!LoadOwnedCharacter(database, request) ||
        !ReadActivityDay(database, request, activity_day) ||
        !Begin(database) ||
        !LockDayState(database, request, activity_day, state)) {
        Rollback(database);
        return false;
    }
    StoredClaim pending;
    if (!FindPendingClaim(database, request, activity_day, pending)) {
        Rollback(database);
        return false;
    }
    if (pending.claim_id != 0ULL) {
        if (pending.round_ordinal != state.round_ordinal + 1u ||
            pending.slots[0] != state.slots[0] ||
            pending.slots[1] != state.slots[1] ||
            pending.slots[2] != state.slots[2]) {
            Rollback(database);
            response.result = RESULT_PENDING;
            return true;
        }
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        ExportState(state, response);
        ExportClaim(pending, response);
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (state.completed != 0u) {
        Rollback(database);
        response.result = RESULT_EVENT_CLOSED;
        return true;
    }
    if (state.slots[0] == 0u || state.slots[1] == 0u ||
        state.slots[2] == 0u) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    uint32_t symbol = 0u;
    uint32_t repeat = 1u;
    ResolveCombo(state, symbol, repeat);
    uint32_t combo_item = 0u;
    uint32_t combo_count = 0u;
    if (!ResolveComboReward(symbol, repeat, combo_item, combo_count)) {
        Rollback(database);
        return false;
    }
    const uint32_t distance_after =
        state.distance + repeat >= 8u ? 8u : state.distance + repeat;
    const uint32_t complete_after = distance_after == 8u ? 1u : 0u;
    uint32_t complete_item = 0u;
    uint32_t complete_count = 0u;
    if (complete_after != 0u) {
        bool weekend = false;
        if (!IsWeekend(database, request.daily_reset_hour, weekend)) {
            Rollback(database);
            return false;
        }
        complete_item = 690001154u;
        complete_count = weekend ? 8u : 3u;
    }
    const uint32_t ordinal = state.round_ordinal + 1u;
    if (!database->set_query(
            "inSert into %s(server_group,m_id,charac_no,activity_day,"
            "round_ordinal,slot_0,slot_1,slot_2,distance_after,"
            "completed_after,combo_symbol,combo_repeat,combo_item_id,"
            "combo_item_count,complete_item_id,complete_item_count,"
            "claim_state,created_at,updated_at,delivered_at) values("
            "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
            "1,now(),now(),null)",
            kClaimTable,
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(ordinal),
            static_cast<unsigned int>(state.slots[0]),
            static_cast<unsigned int>(state.slots[1]),
            static_cast<unsigned int>(state.slots[2]),
            static_cast<unsigned int>(distance_after),
            static_cast<unsigned int>(complete_after),
            static_cast<unsigned int>(symbol),
            static_cast<unsigned int>(repeat),
            static_cast<unsigned int>(combo_item),
            static_cast<unsigned int>(combo_count),
            static_cast<unsigned int>(complete_item),
            static_cast<unsigned int>(complete_count)) ||
        !database->exec(true)) {
        Rollback(database);
        return false;
    }
    const uint64_t claim_id =
        static_cast<uint64_t>(WongWork::DBCommon::GetIdentity(database));
    StoredClaim claim;
    if (claim_id == 0ULL ||
        !ReadClaimById(database, request, claim_id, claim) ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    ExportState(state, response);
    ExportClaim(claim, response);
    response.result = RESULT_SUCCESS;
    return true;
}

bool ValidateCommitAgainstClaim(const Message& request,
                                const StoredClaim& claim) {
    return request.reward_charac_no == claim.charac_no &&
           request.combo_item_id == claim.combo_item_id &&
           request.combo_item_count == claim.combo_item_count &&
           request.complete_item_id == claim.complete_item_id &&
           request.complete_item_count == claim.complete_item_count &&
           request.combo_reward_sent ==
               static_cast<uint8_t>(claim.combo_item_id != 0u) &&
           request.complete_reward_sent ==
               static_cast<uint8_t>(claim.complete_item_id != 0u);
}

bool HandleCommit(MySQL* database,
                  const Message& request,
                  Message& response) {
    uint32_t activity_day = 0u;
    DayState state;
    if (!LoadOwnedCharacter(database, request) ||
        !ReadActivityDay(database, request, activity_day) ||
        !Begin(database) ||
        !LockDayState(database, request, activity_day, state)) {
        Rollback(database);
        return false;
    }
    StoredClaim claim;
    if (!ReadClaimById(database, request, request.claim_id, claim) ||
        claim.activity_day != activity_day ||
        !ValidateCommitAgainstClaim(request, claim)) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    if (claim.phase == PHASE_DELIVERED) {
        if (!Commit(database)) {
            Rollback(database);
            return false;
        }
        ExportState(state, response);
        ExportClaim(claim, response);
        response.result = RESULT_SUCCESS;
        return true;
    }
    if (claim.phase != PHASE_RESERVED ||
        claim.round_ordinal != state.round_ordinal + 1u ||
        claim.slots[0] != state.slots[0] ||
        claim.slots[1] != state.slots[1] ||
        claim.slots[2] != state.slots[2]) {
        Rollback(database);
        response.result = RESULT_INVALID_STATE;
        return true;
    }
    MySQL* const postal = GetPartitionDatabase(3, request.server_group);
    if (!postal) {
        Rollback(database);
        return false;
    }
    if (!Begin(postal)) {
        Rollback(database);
        return false;
    }
    unsigned int reward_index = 0u;
    const bool delivered =
        (claim.combo_item_id == 0u ||
         EnsureMail(postal, DeliveryKey(claim.claim_id, 0u),
                    claim.charac_no, request.rewards[reward_index++],
                    request.combo_mail_title,
                    request.combo_mail_content)) &&
        (claim.complete_item_id == 0u ||
         EnsureMail(postal, DeliveryKey(claim.claim_id, 1u),
                    claim.charac_no, request.rewards[reward_index++],
                    request.complete_mail_title,
                    request.complete_mail_content));
    if (!delivered || !Commit(postal)) {
        Rollback(postal);
        Rollback(database);
        return false;
    }
    if (!database->set_query(
            "upDate %s set claim_state=2,updated_at=now(),delivered_at=now() "
            "where claim_id=%llu and claim_state=1",
            kClaimTable,
            static_cast<unsigned long long>(claim.claim_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !database->set_query(
            "upDate %s set distance=%u,slot_0=0,slot_1=0,slot_2=0,"
            "completed=%u,round_ordinal=%u,updated_at=now() where "
            "server_group=%u and m_id=%u and charac_no=%u and "
            "activity_day=%u and round_ordinal=%u",
            kDayTable,
            static_cast<unsigned int>(claim.distance_after),
            static_cast<unsigned int>(claim.completed_after),
            static_cast<unsigned int>(claim.round_ordinal),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(activity_day),
            static_cast<unsigned int>(state.round_ordinal)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !Commit(database)) {
        Rollback(database);
        return false;
    }
    state.distance = claim.distance_after;
    state.completed = claim.completed_after;
    state.round_ordinal = claim.round_ordinal;
    state.slots[0] = state.slots[1] = state.slots[2] = 0u;
    claim.phase = PHASE_DELIVERED;
    ExportState(state, response);
    ExportClaim(claim, response);
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

DBBlueHorseEvent::DBBlueHorseEvent()
    : TaiwanInterfaceDBDispatcher(
          static_cast<TaiwanInternalPack::T>(kTaiwanInnerCode)) {}

DBBlueHorseEvent::~DBBlueHorseEvent() {}

bool DBBlueHorseEvent::QueueRequest(int user_idx,
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
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

int DBBlueHorseEvent::dispatch(int signal,
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
            case OPERATION_LOAD_BLUE_HORSE:
                handled = HandleLoad(database, request, response);
                break;
            case OPERATION_ADD_BLUE_HORSE_FATIGUE:
                handled = HandleFatigue(database, request, response);
                break;
            case OPERATION_ROLL_BLUE_HORSE_SLOT:
                handled = HandleRoll(database, request, response);
                break;
            case OPERATION_RESERVE_BLUE_HORSE_REWARD:
                handled = HandleReserve(database, request, response);
                break;
            case OPERATION_COMMIT_BLUE_HORSE_REWARD:
                handled = HandleCommit(database, request, response);
                break;
            default:
                break;
        }
    }
    if (!handled) response.result = RESULT_DATABASE_FAILURE;
    return SendResult(user_idx, response) ? 1 : 0;
}

}  // namespace late_event_db_0828
