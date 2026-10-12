#ifndef OLDSRV_OLD_GAME_ACTIVITY_EVENT_RUNTIME_DB_SYNC_H_
#define OLDSRV_OLD_GAME_ACTIVITY_EVENT_RUNTIME_DB_SYNC_H_

namespace event_runtime_db_sync {

// Load the authoritative active-event snapshot during startup.  A database
// query/fetch failure is a startup failure; invalid or externally handled
// individual rows are deferred without rejecting the process.
bool SyncInitial();

}  // namespace event_runtime_db_sync

#endif  // OLDSRV_OLD_GAME_ACTIVITY_EVENT_RUNTIME_DB_SYNC_H_
