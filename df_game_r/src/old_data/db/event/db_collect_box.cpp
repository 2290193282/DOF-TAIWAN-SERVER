#include "old_data/db/event/db_collect_box.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/db_inventory_mutation_barrier.h"
#include "old_data/PVF/localization/collect_box_script.h"

#include <algorithm>
#include <cstring>
#include <vector>


namespace collect_box_db {
namespace {

const uint32_t kOneDaySeconds = 86400u;

struct State {
    State() : expiry_at(0u), unlimited(false), beads() {}
    uint32_t expiry_at;
    bool unlimited;
    std::vector<uint32_t> beads;
};

struct Journal {
    Journal() { std::memset(this, 0, sizeof(*this)); }
    uint32_t saga_id;
    uint32_t item_id;
    uint32_t expiry_after;
    uint32_t slot_count_before;
    uint32_t total_count_before;
    uint32_t slot_count_after;
    uint32_t total_count_after;
    uint16_t source_slot;
    uint16_t bead_index;
    uint8_t operation;
    uint8_t unlimited_after;
    uint8_t state;
};

bool ExecuteLiteral(MySQL* database, const char* sql) {
    return database && database->set_query("%s", sql) &&
           database->exec(true);
}

class Transaction {
public:
    explicit Transaction(MySQL* database)
        : database_(database), active_(false) {}
    ~Transaction() { rollback(); }

    bool begin() {
        if (!ExecuteLiteral(database_, "START TRANSACTION")) return false;
        active_ = true;
        return true;
    }
    bool commit() {
        if (!active_ || !ExecuteLiteral(database_, "COMMIT")) return false;
        active_ = false;
        return true;
    }
    void rollback() {
        if (active_) {
            (void)ExecuteLiteral(database_, "ROLLBACK");
            active_ = false;
        }
    }

private:
    MySQL* database_;
    bool active_;
};

bool IsValidServerGroup(int value) {
    // The durable inventory proof has exact native DB3 handles only for the
    // 18 character shards.  Reject pseudo-groups before reserving a saga that
    // could never be finalized.
    return value >= 0 && value < 18;
}

bool IsBead(uint32_t item_id) {
    return collect_box_script::IsBead(item_id);
}

bool IsExtension(uint32_t item_id) {
    return collect_box_script::IsExtension(item_id, 0, 0);
}

bool IsZeroStateProjection(const Message& message) {
    if (message.expiry_at != 0u || message.unlimited != 0u ||
        message.bead_count != 0u) {
        return false;
    }
    for (int index = 0; index < kMaximumBeadCount; ++index) {
        if (message.bead_items[index] != 0u) return false;
    }
    return true;
}

bool HasValidProof(const Message& message, uint8_t operation) {
    if (message.source_slot >= oldsrv::config::kMainInventoryItemCount ||
        message.item_id == 0u || message.item_id > 0x7fffffffu ||
        message.slot_count_before > 0x7fffffffu ||
        message.total_count_before > 0x7fffffffu ||
        message.slot_count_after > 0x7fffffffu ||
        message.total_count_after > 0x7fffffffu ||
        message.total_count_before < message.slot_count_before ||
        message.total_count_after < message.slot_count_after) {
        return false;
    }
    if (operation == kOperationPrepareAdd ||
        operation == kOperationPrepareExtend) {
        return message.slot_count_before != 0u &&
               message.total_count_before != 0u &&
               message.slot_count_after + 1u ==
                   message.slot_count_before &&
               message.total_count_after + 1u ==
                   message.total_count_before;
    }
    if (operation == kOperationPrepareRemove) {
        return message.slot_count_before < 0x7fffffffu &&
               message.total_count_before < 0x7fffffffu &&
               message.slot_count_after ==
                   message.slot_count_before + 1u &&
               message.total_count_after ==
                   message.total_count_before + 1u;
    }
    return false;
}

bool HasEmptyReserved(const Message& message) {
    return message.reserved[0] == 0u && message.reserved[1] == 0u;
}

bool IsValidRequest(const Message& message, int user_idx) {
    if (!collect_box_script::IsReady() || user_idx == -1 ||
        message.user_idx != user_idx ||
        message.account_id == 0u || message.charac_no == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.session_token == 0u || message.request_token == 0u ||
        message.result != kResultRequest || message.phase != kPhaseNone ||
        message.box_id != collect_box_script::BoxId() ||
        message.client_reply_expected > 1u ||
        !HasEmptyReserved(message) || !IsZeroStateProjection(message)) {
        return false;
    }

    if (message.operation == kOperationSelect) {
        return message.saga_operation == kOperationSelect &&
               message.saga_id == 0u && message.item_id == 0u &&
               message.source_slot == 0u && message.bead_index == 0u &&
               message.client_command != 0u &&
               message.slot_count_before == 0u &&
               message.total_count_before == 0u &&
               message.slot_count_after == 0u &&
               message.total_count_after == 0u;
    }

    if (message.operation == kOperationPrepareAdd ||
        message.operation == kOperationPrepareRemove ||
        message.operation == kOperationPrepareExtend) {
        const bool item_valid =
            message.operation == kOperationPrepareExtend
                ? IsExtension(message.item_id)
                : IsBead(message.item_id);
        return message.saga_operation == message.operation &&
               message.saga_id == 0u && message.bead_index == 0u && item_valid &&
               message.client_command != 0u &&
               HasValidProof(message, message.operation);
    }

    if (message.operation == kOperationFinalize ||
        message.operation == kOperationCancel) {
        return message.saga_id != 0u &&
               message.saga_operation >= kOperationPrepareAdd &&
               message.saga_operation <= kOperationPrepareExtend &&
               message.client_command != 0u &&
               HasValidProof(message, message.saga_operation) &&
               ((message.saga_operation == kOperationPrepareExtend &&
                 IsExtension(message.item_id)) ||
                (message.saga_operation != kOperationPrepareExtend &&
                 IsBead(message.item_id)));
    }
    return false;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const output = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!output) return false;
    *output = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool LockState(MySQL* database, uint32_t charac_no, State& state) {
    if (!database->set_query(
            "inSert ignore into charac_collectbox_state_0828"
            "(charac_no,box_id,expiry_at,unlimited_flag) values(%u,1,%u,0)",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(collect_box_script::BaseExpiry())) ||
        !database->exec(true) ||
        !database->set_query(
            "seLect expiry_at,unlimited_flag from "
            "charac_collectbox_state_0828 where charac_no=%u and box_id=1 "
            "for update",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch()) {
        return false;
    }
    unsigned int expiry = 0u;
    unsigned int unlimited = 0u;
    if (!database->get_uint(0, expiry) ||
        !database->get_uint(1, unlimited) ||
        !collect_box_script::IsRawExpiry(static_cast<uint32_t>(expiry)) ||
        unlimited > 1u) {
        return false;
    }
    state.expiry_at = static_cast<uint32_t>(expiry);
    state.unlimited = unlimited != 0u;
    return true;
}

bool LockBeads(MySQL* database, uint32_t charac_no, State& state) {
    if (!database->set_query(
            "seLect bead_index,item_id from charac_collectbox_bead_0828 "
            "where charac_no=%u and box_id=1 order by bead_index asc "
            "limit 16 for update",
            static_cast<unsigned int>(charac_no)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows < 0 || rows > kMaximumBeadCount) return false;
    state.beads.clear();
    state.beads.reserve(static_cast<size_t>(rows));
    for (int row = 0; row < rows; ++row) {
        unsigned int bead_index = 0u;
        unsigned int item_id = 0u;
        if (!database->fetch() || !database->get_uint(0, bead_index) ||
            !database->get_uint(1, item_id) ||
            bead_index != static_cast<unsigned int>(row) ||
            !IsBead(static_cast<uint32_t>(item_id)) ||
            std::find(state.beads.begin(), state.beads.end(),
                      static_cast<uint32_t>(item_id)) != state.beads.end()) {
            return false;
        }
        state.beads.push_back(static_cast<uint32_t>(item_id));
    }
    return true;
}

bool LoadStateLocked(MySQL* database, uint32_t charac_no, State& state) {
    return LockState(database, charac_no, state) &&
           LockBeads(database, charac_no, state);
}

void CopyState(const State& state, Message& response) {
    response.expiry_at = state.expiry_at;
    response.unlimited = state.unlimited ? 1u : 0u;
    response.bead_count = static_cast<uint8_t>(state.beads.size());
    std::memset(response.bead_items, 0, sizeof(response.bead_items));
    for (size_t index = 0; index < state.beads.size(); ++index) {
        response.bead_items[index] = state.beads[index];
    }
}

bool ReadLastInsertId(MySQL* database, uint32_t& value) {
    unsigned long long result = 0u;
    if (!database || !database->set_query("seLect last_insert_id()") ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !database->fetch() || !database->get_ulonglong(0, result) ||
        result == 0u || result > 0xffffffffULL) {
        return false;
    }
    value = static_cast<uint32_t>(result);
    return true;
}

bool ReadJournalRow(MySQL* database, Journal& journal) {
    unsigned int values[11] = {};
    if (!database->fetch()) return false;
    for (int column = 0; column < 11; ++column) {
        if (!database->get_uint(column, values[column])) return false;
    }
    journal.saga_id = values[0];
    journal.operation = static_cast<uint8_t>(values[1]);
    journal.item_id = values[2];
    journal.source_slot = static_cast<uint16_t>(values[3]);
    journal.bead_index = static_cast<uint16_t>(values[4]);
    journal.expiry_after = values[5];
    journal.unlimited_after = static_cast<uint8_t>(values[6]);
    journal.slot_count_before = values[7];
    journal.total_count_before = values[8];
    journal.slot_count_after = values[9];
    journal.total_count_after = values[10];
    return journal.saga_id != 0u &&
           journal.operation >= kOperationPrepareAdd &&
           journal.operation <= kOperationPrepareExtend &&
           journal.source_slot < oldsrv::config::kMainInventoryItemCount &&
           journal.unlimited_after <= 1u;
}

bool LoadPending(MySQL* database,
                 const Message& request,
                 Journal& journal,
                 bool& found) {
    found = false;
    if (!database->set_query(
            "seLect j.saga_id,j.operation,j.item_id,j.source_slot,"
            "j.bead_index,j.expiry_after,j.unlimited_after,"
            "j.slot_count_before,j.total_count_before,j.slot_count_after,"
            "j.total_count_after from charac_collectbox_pending_0828 p "
            "join charac_collectbox_saga_0828 j on j.saga_id=p.saga_id "
            "where p.charac_no=%u and p.box_id=1 and j.server_group=%u "
            "and j.m_id=%u and j.charac_no=%u and j.box_id=1 "
            "and j.saga_state=1 for update",
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true)) {
        return false;
    }
    const int rows = database->get_n_rows();
    if (rows == 0) return true;
    if (rows != 1 || !ReadJournalRow(database, journal)) return false;
    found = true;
    return true;
}

void CopyJournal(const Journal& journal, Message& response) {
    response.saga_id = journal.saga_id;
    response.saga_operation = journal.operation;
    response.item_id = journal.item_id;
    response.source_slot = journal.source_slot;
    response.bead_index = journal.bead_index;
    response.slot_count_before = journal.slot_count_before;
    response.total_count_before = journal.total_count_before;
    response.slot_count_after = journal.slot_count_after;
    response.total_count_after = journal.total_count_after;
}

bool InsertJournal(MySQL* database,
                   const Message& request,
                   uint16_t bead_index,
                   uint32_t expiry_after,
                   bool unlimited_after,
                   uint32_t& saga_id) {
    if (!database->set_query(
            "inSert into charac_collectbox_saga_0828("
            "server_group,m_id,charac_no,box_id,operation,item_id,"
            "source_slot,bead_index,expiry_after,unlimited_after,"
            "slot_count_before,total_count_before,slot_count_after,"
            "total_count_after,session_token,request_token,saga_state,"
            "created_at,updated_at) values("
            "%u,%u,%u,1,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,1,"
            "now(),now())",
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no),
            static_cast<unsigned int>(request.operation),
            static_cast<unsigned int>(request.item_id),
            static_cast<unsigned int>(request.source_slot),
            static_cast<unsigned int>(bead_index),
            static_cast<unsigned int>(expiry_after),
            unlimited_after ? 1u : 0u,
            static_cast<unsigned int>(request.slot_count_before),
            static_cast<unsigned int>(request.total_count_before),
            static_cast<unsigned int>(request.slot_count_after),
            static_cast<unsigned int>(request.total_count_after),
            static_cast<unsigned int>(request.session_token),
            static_cast<unsigned int>(request.request_token)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u ||
        !ReadLastInsertId(database, saga_id)) {
        return false;
    }
    return database->set_query(
               "inSert into charac_collectbox_pending_0828"
               "(charac_no,box_id,saga_id,created_at) values(%u,1,%u,now())",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(saga_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool LoadJournal(MySQL* database,
                 const Message& request,
                 Journal& journal) {
    if (!database->set_query(
            "seLect saga_id,operation,item_id,source_slot,bead_index,"
            "expiry_after,unlimited_after,slot_count_before,"
            "total_count_before,slot_count_after,total_count_after,"
            "saga_state from charac_collectbox_saga_0828 where "
            "saga_id=%u and server_group=%u and m_id=%u and charac_no=%u "
            "and box_id=1 for update",
            static_cast<unsigned int>(request.saga_id),
            static_cast<unsigned int>(request.server_group),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.charac_no)) ||
        !database->exec(true) || database->get_n_rows() != 1 ||
        !ReadJournalRow(database, journal)) {
        return false;
    }
    unsigned int state = 0u;
    if (!database->get_uint(11, state) || state < 1u || state > 3u) {
        return false;
    }
    journal.state = static_cast<uint8_t>(state);
    return true;
}

bool JournalMatches(const Journal& journal, const Message& request) {
    return journal.saga_id == request.saga_id &&
           journal.operation == request.saga_operation &&
           journal.item_id == request.item_id &&
           journal.source_slot == request.source_slot &&
           journal.bead_index == request.bead_index &&
           journal.slot_count_before == request.slot_count_before &&
           journal.total_count_before == request.total_count_before &&
           journal.slot_count_after == request.slot_count_after &&
           journal.total_count_after == request.total_count_after;
}

bool HasPersistedProof(const Message& request, bool after) {
    return inventory_mutation_barrier::HasExactPersistedStackAndTotal(
        request.server_group,
        request.charac_no,
        request.source_slot,
        request.item_id,
        after ? request.slot_count_after : request.slot_count_before,
        after ? request.total_count_after : request.total_count_before);
}

bool InsertBead(MySQL* database,
                uint32_t charac_no,
                const Journal& journal,
                const State& state) {
    return journal.bead_index == state.beads.size() &&
           state.beads.size() < kMaximumBeadCount &&
           std::find(state.beads.begin(), state.beads.end(),
                     journal.item_id) == state.beads.end() &&
           database->set_query(
               "inSert into charac_collectbox_bead_0828"
               "(charac_no,box_id,bead_index,item_id) values(%u,1,%u,%u)",
               static_cast<unsigned int>(charac_no),
               static_cast<unsigned int>(journal.bead_index),
               static_cast<unsigned int>(journal.item_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool RemoveBead(MySQL* database,
                uint32_t charac_no,
                const Journal& journal,
                const State& state) {
    if (journal.bead_index >= state.beads.size() ||
        state.beads[journal.bead_index] != journal.item_id ||
        !database->set_query(
            "deLete from charac_collectbox_bead_0828 where charac_no=%u "
            "and box_id=1 and bead_index=%u and item_id=%u",
            static_cast<unsigned int>(charac_no),
            static_cast<unsigned int>(journal.bead_index),
            static_cast<unsigned int>(journal.item_id)) ||
        !database->exec(true) || database->getAffectedRowCount() != 1u) {
        return false;
    }
    const size_t shifted = state.beads.size() - journal.bead_index - 1u;
    return database->set_query(
               "upDate charac_collectbox_bead_0828 set bead_index=bead_index-1 "
               "where charac_no=%u and box_id=1 and bead_index>%u "
               "order by bead_index asc",
               static_cast<unsigned int>(charac_no),
               static_cast<unsigned int>(journal.bead_index)) &&
           database->exec(true) &&
           database->getAffectedRowCount() ==
               static_cast<unsigned long long>(shifted);
}

bool UpdateExpiry(MySQL* database,
                  uint32_t charac_no,
                  const Journal& journal,
                  const State& state) {
    if (journal.operation != kOperationPrepareExtend ||
        journal.expiry_after < state.expiry_at ||
        !collect_box_script::IsRawExpiry(journal.expiry_after) ||
        (state.unlimited && journal.unlimited_after == 0u) ||
        journal.unlimited_after > 1u) {
        return false;
    }
    return database->set_query(
               "upDate charac_collectbox_state_0828 set expiry_at=%u,"
               "unlimited_flag=%u where charac_no=%u and box_id=1",
               static_cast<unsigned int>(journal.expiry_after),
               static_cast<unsigned int>(journal.unlimited_after),
               static_cast<unsigned int>(charac_no)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool ApplyVisibleMutation(MySQL* database,
                          uint32_t charac_no,
                          const Journal& journal,
                          const State& state) {
    if (journal.operation == kOperationPrepareAdd) {
        return InsertBead(database, charac_no, journal, state);
    }
    if (journal.operation == kOperationPrepareRemove) {
        return RemoveBead(database, charac_no, journal, state);
    }
    if (journal.operation == kOperationPrepareExtend) {
        return UpdateExpiry(database, charac_no, journal, state);
    }
    return false;
}

bool CloseJournal(MySQL* database,
                  const Message& request,
                  uint8_t terminal_state) {
    return database->set_query(
               "upDate charac_collectbox_saga_0828 set saga_state=%u,"
               "updated_at=now() where saga_id=%u and saga_state=1",
               static_cast<unsigned int>(terminal_state),
               static_cast<unsigned int>(request.saga_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u &&
           database->set_query(
               "deLete from charac_collectbox_pending_0828 where "
               "charac_no=%u and box_id=1 and saga_id=%u",
               static_cast<unsigned int>(request.charac_no),
               static_cast<unsigned int>(request.saga_id)) &&
           database->exec(true) && database->getAffectedRowCount() == 1u;
}

bool HandleSelectOrPrepare(MySQL* database,
                           const Message& request,
                           Message& response) {
    Transaction transaction(database);
    State state;
    Journal pending;
    bool has_pending = false;
    if (!transaction.begin() ||
        !LoadStateLocked(database, request.charac_no, state) ||
        !LoadPending(database, request, pending, has_pending)) {
        return false;
    }
    CopyState(state, response);
    if (has_pending) {
        CopyJournal(pending, response);
        response.result = kResultPendingRecovery;
        response.phase = kPhaseReserved;
        response.client_reply_expected = 0u;
        return transaction.commit();
    }
    if (request.operation == kOperationSelect) {
        response.result = kResultSuccess;
        response.phase = kPhaseNone;
        return transaction.commit();
    }

    uint16_t bead_index = 0u;
    uint32_t expiry_after = state.expiry_at;
    bool unlimited_after = state.unlimited;
    if (request.operation == kOperationPrepareAdd) {
        if (state.beads.size() >= kMaximumBeadCount ||
            std::find(state.beads.begin(), state.beads.end(),
                      request.item_id) != state.beads.end()) {
            response.result = kResultInvalidState;
            return transaction.commit();
        }
        bead_index = static_cast<uint16_t>(state.beads.size());
    } else if (request.operation == kOperationPrepareRemove) {
        const std::vector<uint32_t>::const_iterator found =
            std::find(state.beads.begin(), state.beads.end(), request.item_id);
        if (found == state.beads.end()) {
            response.result = kResultInvalidState;
            return transaction.commit();
        }
        bead_index = static_cast<uint16_t>(found - state.beads.begin());
    } else if (request.operation == kOperationPrepareExtend) {
        uint32_t extension_days = 0u;
        bool extension_unlimited = false;
        if (!collect_box_script::IsExtension(request.item_id,
                                             &extension_days,
                                             &extension_unlimited)) {
            response.result = kResultInvalidState;
            return transaction.commit();
        }
        if (extension_unlimited) {
            if (state.unlimited) {
                response.result = kResultInvalidState;
                return transaction.commit();
            }
            unlimited_after = true;
        } else {
            const uint32_t maximum = collect_box_script::MaximumExpiry();
            const uint64_t extension_seconds =
                static_cast<uint64_t>(extension_days) * kOneDaySeconds;
            if (state.unlimited || extension_days == 0u ||
                state.expiry_at >= maximum) {
                response.result = kResultInvalidState;
                return transaction.commit();
            }
            const uint64_t candidate =
                static_cast<uint64_t>(state.expiry_at) + extension_seconds;
            expiry_after = candidate >= maximum
                               ? maximum
                               : static_cast<uint32_t>(candidate);
        }
    } else {
        response.result = kResultInvalidState;
        return transaction.commit();
    }

    uint32_t saga_id = 0u;
    if (!InsertJournal(database, request, bead_index, expiry_after,
                       unlimited_after, saga_id)) {
        return false;
    }
    response.saga_id = saga_id;
    response.saga_operation = request.operation;
    response.bead_index = bead_index;
    response.expiry_at = expiry_after;
    response.unlimited = unlimited_after ? 1u : 0u;
    response.result = kResultSuccess;
    response.phase = kPhaseReserved;
    return transaction.commit();
}

bool HandleTerminal(MySQL* database,
                    const Message& request,
                    Message& response) {
    Transaction transaction(database);
    Journal journal;
    State state;
    if (!transaction.begin() || !LoadJournal(database, request, journal) ||
        !JournalMatches(journal, request) ||
        !LoadStateLocked(database, request.charac_no, state)) {
        return false;
    }
    CopyJournal(journal, response);
    CopyState(state, response);

    if (request.operation == kOperationFinalize && journal.state == 2u) {
        response.result = kResultSuccess;
        response.phase = kPhaseCommitted;
        return transaction.commit();
    }
    if (request.operation == kOperationCancel && journal.state == 3u) {
        response.result = kResultSuccess;
        response.phase = kPhaseCancelled;
        return transaction.commit();
    }
    if (journal.state != 1u) {
        response.result = kResultInvalidState;
        return transaction.commit();
    }

    if (request.operation == kOperationFinalize) {
        if (!HasPersistedProof(request, true)) {
            transaction.rollback();
            response.result = kResultInventoryNotDurable;
            response.phase = kPhaseReserved;
            return true;
        }
        if (!ApplyVisibleMutation(database, request.charac_no, journal,
                                  state) ||
            !CloseJournal(database, request, 2u)) {
            return false;
        }
        State committed;
        if (!LoadStateLocked(database, request.charac_no, committed)) {
            return false;
        }
        CopyState(committed, response);
        response.result = kResultSuccess;
        response.phase = kPhaseCommitted;
        return transaction.commit();
    }

    if (request.operation == kOperationCancel) {
        if (!HasPersistedProof(request, false)) {
            transaction.rollback();
            response.result = kResultInventoryNotDurable;
            response.phase = kPhaseReserved;
            return true;
        }
        if (!CloseJournal(database, request, 3u)) return false;
        response.result = kResultSuccess;
        response.phase = kPhaseCancelled;
        return transaction.commit();
    }
    return false;
}

}  // namespace


DB_CollectBox::DB_CollectBox() : DBDispatcher() {}

DB_CollectBox::~DB_CollectBox() {}

bool DB_CollectBox::Queue(int user_idx, const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!IsValidRequest(request, user_idx) || !pool || !queues) return false;
    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) return false;
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const queued = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!queued) return false;
    *queued = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_CollectBox::dispatch(int signal,
                                 int user_idx,
                                 Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }
    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || !IsValidRequest(*request, user_idx)) return false;

    Message response = *request;
    response.result = kResultDatabaseFailure;
    response.phase = kPhaseNone;
    response.expiry_at = 0u;
    response.unlimited = 0u;
    response.bead_count = 0u;
    std::memset(response.bead_items, 0, sizeof(response.bead_items));

    MySQL* const database = GlobalData::s_db_mgr
        ? GlobalData::s_db_mgr->GetDBHandleExact(
              DB_HANDLE_FEATURE_DB3,
              static_cast<ENUM_SERVER_GROUP>(request->server_group))
        : 0;
    bool handled = false;
    if (database && (request->operation == kOperationSelect ||
                     request->operation == kOperationPrepareAdd ||
                     request->operation == kOperationPrepareRemove ||
                     request->operation == kOperationPrepareExtend)) {
        handled = HandleSelectOrPrepare(database, *request, response);
    } else if (database &&
               (request->operation == kOperationFinalize ||
                request->operation == kOperationCancel)) {
        handled = HandleTerminal(database, *request, response);
    }
    if (!handled) {
        response.result = kResultDatabaseFailure;
        response.phase = kPhaseNone;
    }
    return SendResult(user_idx, response);
}

}  // namespace collect_box_db_0828
