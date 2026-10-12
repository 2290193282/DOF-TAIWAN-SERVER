#include "old_game/player/core/cuser.h"

#include <vector>

#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/inventory/inventory.h"

namespace {

// The two original exporters at 0x86769EA/0x8676BEC both enumerate the
// named rental list for the current character, then report only inventory
// items that are still present.  The output capacities are part of the DB
// payload ABI and are intentionally enforced before the typed array write.
bool FindRentalItem(CUser* user,
                    const InstanceRentalSystem::RentalInfo& rental,
                    int& inventory_slot,
                    bool& equipped) {
    return user->getCurCharacInvenR()->CheckExistItemInInventoryOrEquipment(
               static_cast<int>(rental.item_id), inventory_slot, equipped) ==
           1;
}

}  // namespace

// IDA_EA: 0x86769EA
// IDA_NAME: _ZN5CUser17GetPcRoomRentInfoEP26SIG_SAVE_PC_ROOM_RENT_ITEM
// DEMANGLED: CUser::GetPcRoomRentInfo(SIG_SAVE_PC_ROOM_RENT_ITEM *)
int CUser::GetPcRoomRentInfo(SIG_SAVE_PC_ROOM_RENT_ITEM* payload) {
    if (!payload) {
        return 0;
    }

    payload->clear();
    if (getPcRoomAuth() != 1) {
        return 1;
    }

    CUserRentalInfoList rental_infos;
    if (getCurrentCharacRentalInfo(rental_infos, 1) != 1) {
        return 1;
    }

    size_t output_count = 0;
    for (CUserRentalInfoList::const_iterator it = rental_infos.begin();
         it != rental_infos.end();
         ++it) {
        int inventory_slot = 0;
        bool equipped = false;
        if (!FindRentalItem(this, *it, inventory_slot, equipped)) {
            continue;
        }

        // The raw branch returns failure when more than the one-record
        // payload can represent.  Do not transiently write past the typed
        // payload while preserving that return contract.
        if (output_count == 1u) {
            return 0;
        }

        SIG_SAVE_PC_ROOM_RENT_INFO& output = payload->items[output_count++];
        output.item_id = it->item_id;
        output.slot = it->section_index;
        output.rent_type = it->package_index;
        output.state = equipped ? 1u : 0u;
        output.expire_time = it->expire_time;
    }
    return 1;
}

// IDA_EA: 0x8676BEC
// IDA_NAME: _ZN5CUser20GetDimensionRentInfoEP28SIG_SAVE_DIMENSION_RENT_ITEM
// DEMANGLED: CUser::GetDimensionRentInfo(SIG_SAVE_DIMENSION_RENT_ITEM *)
int CUser::GetDimensionRentInfo(SIG_SAVE_DIMENSION_RENT_ITEM* payload) {
    if (!payload) {
        return 0;
    }

    payload->clear();
    if (getPcRoomAuth() != 1) {
        return 1;
    }

    CUserRentalInfoList rental_infos;
    if (getCurrentCharacRentalInfo(rental_infos, 2) != 1) {
        return 1;
    }

    size_t output_count = 0;
    for (CUserRentalInfoList::const_iterator it = rental_infos.begin();
         it != rental_infos.end();
         ++it) {
        int inventory_slot = 0;
        bool equipped = false;
        if (!FindRentalItem(this, *it, inventory_slot, equipped)) {
            continue;
        }

        // SIG_SAVE_DIMENSION_RENT_ITEM is exactly eleven eight-byte rows.
        if (output_count == 11u) {
            return 0;
        }

        SIG_SAVE_DIMENSION_RENT_INFO& output =
            payload->items[output_count++];
        output.item_id = it->item_id;
        output.rental_type = it->section_index;
        output.inventory_slot = it->package_index;
        output.equipped = equipped ? 1u : 0u;
        output.rental_state = it->rent_type;
    }
    return 1;
}
