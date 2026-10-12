#include "old_game/activity/schedule/event_runtime_db_sync.h"

#include <map>
#include <stdint.h>
#include <vector>

#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "core/config/env/environment.h"
#include "infra/log/format/clog.h"
#include "old_data/db/core/mysql.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"

namespace event_runtime_db_sync {
namespace {

struct EventParameters {
    EventParameters() : parameter1(0), parameter2(0) {}
    EventParameters(uint32_t first, uint32_t second)
        : parameter1(first), parameter2(second) {}

    uint32_t parameter1;
    uint32_t parameter2;
};

struct EventRow {
    EventRow(uint32_t id, const EventParameters& event_parameters)
        : event_id(id), parameters(event_parameters) {}

    uint32_t event_id;
    EventParameters parameters;
};

// Keep the SQL `order by start_time, log_id` result sequence intact.  A map is
// used only for duplicate counts below and never as the activation iterator.
typedef std::vector<EventRow> RawRows;

void LogRowState(const char* level,
                 uint32_t event_id,
                 const char* state,
                 const EventParameters& parameters,
                 uint32_t row_count) {
    if (level[0] == 'N') {
        CLog::instance()->logNotice(
            "[event-db-startup] event_id=%u state=%s rows=%u parameter1=%u parameter2=%u",
            event_id,
            state,
            row_count,
            parameters.parameter1,
            parameters.parameter2);
        return;
    }
    CLog::instance()->logError(
        "[event-db-startup] event_id=%u state=%s rows=%u parameter1=%u parameter2=%u",
        event_id,
        state,
        row_count,
        parameters.parameter1,
        parameters.parameter2);
}

bool ReadCompleteSnapshot(RawRows* snapshot) {
    if (!snapshot || !GlobalData::s_db_mgr || !G_CEnvironment()) {
        CLog::instance()->logError(
            "[event-db-startup] database runtime is not initialized");
        return false;
    }

    MySQL* const database = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_1, SERVER_GROUP_DEFAULT);
    if (!database) {
        CLog::instance()->logError(
            "[event-db-startup] DB_HANDLE_1 is unavailable");
        return false;
    }

    // dnf_event_log is the sole startup enable/disable authority.  Operators
    // edit the table and restart df_game_r; there is deliberately no runtime
    // polling path competing with the legacy DB worker for this handle.
    if (!database->set_query(
            "seLect event_type, parameter1, parameter2 "
            "from dnf_event_log where end_time = 0 "
            "and server_id in (0,%d) "
            "and unix_timestamp() >= start_time "
            "order by start_time, log_id",
            G_CEnvironment()->get_server_group()) ||
        !database->exec(true)) {
        CLog::instance()->logError(
            "[event-db-startup] authoritative snapshot query failed");
        return false;
    }

    RawRows candidate;
    const int row_count = database->get_n_rows();
    if (row_count < 0) {
        CLog::instance()->logError(
            "[event-db-startup] authoritative query returned a negative row count");
        return false;
    }

    for (int row = 0; row < row_count; ++row) {
        if (!database->fetch()) {
            CLog::instance()->logError(
                "[event-db-startup] authoritative snapshot fetch is incomplete");
            return false;
        }

        // Materialize every field at full width.  Per-id validation and
        // narrowing happen only after the entire result has been fetched.
        unsigned int event_id = 0;
        unsigned int parameter1 = 0;
        unsigned int parameter2 = 0;
        if (!database->get_uint(0, event_id) ||
            !database->get_uint(1, parameter1) ||
            !database->get_uint(2, parameter2)) {
            CLog::instance()->logError(
                "[event-db-startup] authoritative snapshot has an invalid unsigned column");
            return false;
        }
        candidate.push_back(EventRow(
            event_id, EventParameters(parameter1, parameter2)));
    }

    // Publish only a fully materialized result.  No partially fetched set can
    // start a subset of activities before bootstrap fails.
    snapshot->swap(candidate);
    return true;
}

void StartSnapshotRows(const RawRows& rows, CEventManager* manager) {
    typedef std::map<uint32_t, uint32_t> RowCountMap;
    RowCountMap row_counts;
    for (RawRows::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        ++row_counts[it->event_id];
    }

    // Track ids already handled so a duplicate id is rejected exactly once at
    // the position of its first SQL row and can never be partially activated.
    std::map<uint32_t, bool> handled_ids;
    bool needs_final_broadcast = false;
    for (RawRows::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        const uint32_t event_id = it->event_id;
        if (handled_ids.find(event_id) != handled_ids.end()) {
            continue;
        }
        handled_ids[event_id] = true;

        const EventParameters& parameters = it->parameters;
        const uint32_t row_count = row_counts[event_id];

        // Event ids and both parameters use unsigned-word protocol fields.
        // Bad rows stay in the operator's table for correction and never
        // narrow into another native or plugin event.
        if (event_id > 0xffffu) {
            LogRowState("ERROR", event_id, "INVALID_EVENT_ID", parameters,
                        row_count);
            continue;
        }
        if (row_count != 1u) {
            LogRowState("ERROR", event_id, "INVALID_DUPLICATE_ROWS",
                        parameters, row_count);
            continue;
        }
        if (parameters.parameter1 > 0xffffu ||
            parameters.parameter2 > 0xffffu) {
            LogRowState("ERROR", event_id, "INVALID_PARAMETER_RANGE",
                        parameters, 1u);
            continue;
        }
        CEventBase* const handler =
            manager->GetRepeatEvent(static_cast<int>(event_id));
        if (!handler) {
            // A plugin may own this id or register it in a different runtime.
            // Absence of a native handler is never a server-startup failure.
            LogRowState("NOTICE", event_id, "DEFERRED_EXTERNAL_HANDLER",
                        parameters, 1u);
            continue;
        }
        if (handler->IsEventFlagSet()) {
            // Do not re-run or adopt a handler already activated by an
            // explicit plugin/legacy startup hook.
            LogRowState("NOTICE", event_id, "DEFERRED_EXTERNAL_ACTIVE",
                        parameters, 1u);
            continue;
        }

        const Word_Param packed(
            parameters.parameter1 | (parameters.parameter2 << 16));
        if (!manager->TriggerEventStart(static_cast<int>(event_id), packed) ||
            !handler->IsEventFlagSet()) {
            // Handler-local/PVF readiness remains the fail-closed boundary.
            // A single unavailable activity must not stop unrelated rows or
            // prevent the game process from starting.
            if (manager->GetRepeatEvent(static_cast<int>(event_id)) == handler &&
                manager->TriggerEventEnd(static_cast<int>(event_id))) {
                needs_final_broadcast = true;
            }
            LogRowState("ERROR", event_id, "START_FAILED", parameters, 1u);
        }
    }

    // TriggerEventEnd publishes before EndEvent in the official sequence.
    // Publish once more after cleanup so clients observe the final off flags.
    if (needs_final_broadcast) {
        manager->BroadcastEventInfo();
    }
}

}  // namespace

bool SyncInitial() {
    CEventManager* const manager = GlobalData::s_event_manager;
    if (!manager) {
        CLog::instance()->logError(
            "[event-db-startup] event manager is not initialized");
        return false;
    }

    RawRows rows;
    if (!ReadCompleteSnapshot(&rows)) {
        return false;
    }
    StartSnapshotRows(rows, manager);
    return true;
}

}  // namespace event_runtime_db_sync
