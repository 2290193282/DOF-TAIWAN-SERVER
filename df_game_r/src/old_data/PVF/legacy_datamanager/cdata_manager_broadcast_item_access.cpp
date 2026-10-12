#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/equipment/datamanager/broadcast_item_script_data.h"

// CDataManager::C2/D2 construct/destroy stBroadCastItemScript at +0xB5BC.
// This B-owned accessor keeps that layout detail out of gameplay dispatchers.
const stBroadCastItemScript* CDataManager::get_broadcast_item_script() const {
    return &broadcast_item_script_;
}
