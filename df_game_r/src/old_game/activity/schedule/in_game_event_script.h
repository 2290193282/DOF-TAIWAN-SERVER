#ifndef OLDSRV_OLD_GAME_EVENT_IN_GAME_EVENT_SCRIPT_H_
#define OLDSRV_OLD_GAME_EVENT_IN_GAME_EVENT_SCRIPT_H_

#include <string>

// IDA unit: 11_event_runtime/InGameEventScript.  This base is intentionally
// separate from the event aggregate header: inventory-derived scripts need
// its complete layout while the aggregate itself imports inventory runtime
// types.
struct InGameEventScript {
    InGameEventScript();
    ~InGameEventScript();

    std::string event_name;
    std::string script_file;
};

#endif  // OLDSRV_OLD_GAME_EVENT_IN_GAME_EVENT_SCRIPT_H_
