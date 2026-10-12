#ifndef OLDSRV_OLD_GAME_EVENT_STACKABLE_PACKAGE_EVENT_PROCESS_MANAGER_H_
#define OLDSRV_OLD_GAME_EVENT_STACKABLE_PACKAGE_EVENT_PROCESS_MANAGER_H_

#include <map>
#include <memory>
#include <utility>

#include "old_game/activity/schedule/event_value_types.h"

class CUser;

namespace StackablePackage {

// The mangled ABI names this enum but the recovered functions only use it as
// an opaque map key.  Its concrete enumerators are supplied by event-specific
// processors, not by EventProcessManager.
enum EVENT_TYPE {
    EVENT_TYPE_INVALID = 0
};

class EventProcessManager {
public:
    typedef std::map<int, EventProcessor*> ProcessorMap;
    typedef std::map<EVENT_TYPE, ProcessorMap> ProcessorMaps;

    class EventProcessManagerImpl {
    public:
        ProcessorMaps processors;
    };

    EventProcessManager();
    ~EventProcessManager();

    void setupEventProcessors();
    int Process(EVENT_TYPE event_type, CUser& user, int item_id);
    bool isStackablePackageItem(int item_id) const;
    std::pair<ProcessorMap::iterator, bool>
    registerEventProcessor(EventProcessor* processor);
    void destroy();

private:
    std::auto_ptr<EventProcessManagerImpl> impl_;
};

#if defined(__i386__)
static_assert(sizeof(EventProcessManager) == 0x04u,
              "EventProcessManager must retain its auto_ptr-only ABI");
static_assert(sizeof(EventProcessManager::EventProcessManagerImpl) == 0x18u,
              "EventProcessManagerImpl must retain the original map ABI");
#endif

}  // namespace StackablePackage

#endif  // OLDSRV_OLD_GAME_EVENT_STACKABLE_PACKAGE_EVENT_PROCESS_MANAGER_H_
