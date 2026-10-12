#include "old_game/activity/schedule/stackable_package_event_process_manager.h"

#include <algorithm>

#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"

namespace StackablePackage {

namespace {

typedef int (*ProcessProcessorSlot)(EventProcessor*, CUser&, int, int);
typedef void (*DeleteProcessorSlot)(EventProcessor*);

}  // namespace

// IDA_EA: 0x82817C4
// IDA_NAME: _ZN16StackablePackage19EventProcessManager20setupEventProcessorsEv
// DEMANGLED: StackablePackage::EventProcessManager::setupEventProcessors(void)
void EventProcessManager::setupEventProcessors() {
    // The original function is a one-instruction return.  Processor
    // registration belongs to the event-specific owners.
}

// IDA_EA: 0x82817CA / 0x828182A
// IDA_NAME: _ZN16StackablePackage19EventProcessManagerC2Ev /
//           _ZN16StackablePackage19EventProcessManagerD2Ev
EventProcessManager::EventProcessManager()
    : impl_(new EventProcessManagerImpl()) {
    setupEventProcessors();
}

EventProcessManager::~EventProcessManager() {
    destroy();
}

// IDA_EA: 0x828186C
// IDA_NAME: _ZN16StackablePackage19EventProcessManager7ProcessENS_10EVENT_TYPEER5CUseri
// DEMANGLED: StackablePackage::EventProcessManager::Process(
//     StackablePackage::EVENT_TYPE,CUser &,int)
int EventProcessManager::Process(EVENT_TYPE event_type,
                                 CUser& user,
                                 int item_id) {
    if (!isStackablePackageItem(item_id)) {
        return 0;
    }

    ProcessorMaps::iterator event_processors =
        impl_->processors.find(event_type);
    if (event_processors == impl_->processors.end()) {
        return 0;
    }

    ProcessorMap& processors = event_processors->second;
    ProcessorMap::iterator processor = std::find_if(
        processors.begin(),
        processors.end(),
        FindControllableEventProcessor(item_id));
    if (processor == processors.end()) {
        return 0;
    }

    EventProcessor* const event_processor = processor->second;
    void** const vtable = *reinterpret_cast<void***>(event_processor);
    const ProcessProcessorSlot process =
        reinterpret_cast<ProcessProcessorSlot>(vtable[3]);
    // The fourth virtual argument is the copied item-id stored in the
    // find_if predicate; it is the same value as the third argument.
    return process(event_processor, user, item_id, item_id);
}

// IDA_EA: 0x82819C6
// IDA_NAME: _ZNK16StackablePackage19EventProcessManager22isStackablePackageItemEi
// DEMANGLED: StackablePackage::EventProcessManager::isStackablePackageItem(int)const
bool EventProcessManager::isStackablePackageItem(int item_id) const {
    CItem* const item = G_CDataManager()->find_item(item_id);
    return item && item->is_stackable() == 1 &&
           item->vtable_->get_item_type(item) == 38;
}

// IDA_EA: 0x8281A2A
// IDA_NAME: _ZN16StackablePackage19EventProcessManager22registerEventProcessorEPNS_14EventProcessorE
// DEMANGLED: StackablePackage::EventProcessManager::registerEventProcessor(
//     StackablePackage::EventProcessor *)
std::pair<EventProcessManager::ProcessorMap::iterator, bool>
EventProcessManager::registerEventProcessor(EventProcessor* processor) {
    return impl_->processors[static_cast<EVENT_TYPE>(
        processor->GetEventType())]
        .insert(std::make_pair(processor->GetEventId(), processor));
}

// IDA_EA: 0x8281A9A
// IDA_NAME: _ZN16StackablePackage19EventProcessManager7destroyEv
// DEMANGLED: StackablePackage::EventProcessManager::destroy(void)
void EventProcessManager::destroy() {
    for (ProcessorMaps::iterator event_processors = impl_->processors.begin();
         event_processors != impl_->processors.end();
         ++event_processors) {
        ProcessorMap& processors = event_processors->second;
        for (ProcessorMap::iterator processor = processors.begin();
             processor != processors.end();
             ++processor) {
            EventProcessor* const event_processor = processor->second;
            if (!event_processor) {
                continue;
            }
            void** const vtable = *reinterpret_cast<void***>(event_processor);
            const DeleteProcessorSlot deleting_destructor =
                reinterpret_cast<DeleteProcessorSlot>(vtable[1]);
            deleting_destructor(event_processor);
        }
    }
}

}  // namespace StackablePackage
