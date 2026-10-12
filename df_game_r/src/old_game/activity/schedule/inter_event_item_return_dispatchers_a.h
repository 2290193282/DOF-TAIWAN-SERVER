#ifndef OLDSRV_OLD_GAME_EVENT_INTER_EVENT_ITEM_RETURN_DISPATCHERS_A_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_EVENT_ITEM_RETURN_DISPATCHERS_A_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 11_event_runtime/Inter_MonitorEventItemUpdate.
// InterDispatcher slot 74 carries an event-web item write request.
class Inter_MonitorEventItemUpdate : public ISigDispatcher {
public:
    Inter_MonitorEventItemUpdate();
    virtual ~Inter_MonitorEventItemUpdate();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 11_event_runtime/Inter_LoadEventItem.
// InterDispatcher slot 75 applies DB-returned event items and then sends the
// exact completed-row list back to DB slot 77.
class Inter_LoadEventItem : public ISigDispatcher {
public:
    Inter_LoadEventItem();
    virtual ~Inter_LoadEventItem();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_MonitorEventItemUpdate) == 0x04u,
              "Inter_MonitorEventItemUpdate must remain base-only");
static_assert(sizeof(Inter_LoadEventItem) == 0x04u,
              "Inter_LoadEventItem must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_EVENT_ITEM_RETURN_DISPATCHERS_A_H_
