#include "old_game/activity/reward/event_manager_abi.h"

#include <cstring>
#include <new>

#include "old_game/economy/mall/delivery_runtime.h"
#include "old_game/activity/schedule/blue_horse_event.h"


namespace {

const int kHeroMissionEventSlot = 164;

}  // namespace


// IDA_EA: 0x8114CE4
// IDA_NAME: _ZN13CEventManagerC2Ev
// DEMANGLED: CEventManager::CEventManager(void)
CEventManager::CEventManager() {
    std::memset(this, 0, 0x298u);
    delivery_ = new CDelivery();
}

// IDA_EA: 0x8114D4E
// IDA_NAME: _ZN13CEventManagerD2Ev
// DEMANGLED: CEventManager::~CEventManager()
CEventManager::~CEventManager() {
    for (int event_id = 0; event_id <= 165; ++event_id) {
        // Slot 164 aliases the function-local HeroMissionEvent singleton.  Its
        // official __cxa_atexit lifetime owns destruction, so this manager must
        // release only its non-owning reference and must not invoke the
        // deleting destructor on static storage.
        if (event_id == kHeroMissionEventSlot) {
            repeat_events_[event_id] = 0;
            continue;
        }

        CEventBase* event = repeat_events_[event_id];
        if (event) {
            typedef void (*DeleteThunk)(CEventBase*);
            void** vtable = *reinterpret_cast<void***>(event);
            DeleteThunk delete_event = reinterpret_cast<DeleteThunk>(vtable[1]);
            delete_event(event);
            repeat_events_[event_id] = 0;
        }
    }
    blue_horse_event::DestroyBlueHorseEventSidecar();

    delete delivery_;
    delivery_ = 0;
}
