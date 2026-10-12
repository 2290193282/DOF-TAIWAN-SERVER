#include "old_game/activity/reward/event_manager_abi.h"

#include "old_game/economy/mall/delivery_runtime.h"


// IDA_EA: 0x8116874
// IDA_NAME: _ZN13CEventManager19eventDeliveryNotifyEiR11DeliveryMsg
// DEMANGLED: CEventManager::eventDeliveryNotify(int,DeliveryMsg &)
int CEventManager::eventDeliveryNotify(int address, DeliveryMsg& msg) {
    if (delivery_) {
        return delivery_->notify(address, msg);
    }
    return 0;
}
