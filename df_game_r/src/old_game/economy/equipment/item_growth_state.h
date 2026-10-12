#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_ITEM_GROWTH_STATE_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_ITEM_GROWTH_STATE_H_

#include <stdint.h>


struct itemGloballyUniqueIdentifier_t;

namespace item_growth_state {

enum { kGuidBytes = 10 };

// Packet-visible state stored outside the fixed 0627 Inven_Item ABI.  The
// ten-byte item GUID is the sole identity: slot, owner and item id can all
// change while a growth item moves through inventory, cargo or mail.
struct State {
    State();
    State(uint16_t progress_value, uint8_t fatigue_value);

    uint16_t progress;
    uint8_t fatigue_state;
};

bool IsValidGuid(const void* guid_bytes);
bool Lookup(const itemGloballyUniqueIdentifier_t& guid, State* state);
bool LookupBytes(const void* guid_bytes, State* state);
bool Install(const itemGloballyUniqueIdentifier_t& guid, const State& state);
bool InstallBytes(const void* guid_bytes, const State& state);
bool Update(const itemGloballyUniqueIdentifier_t& guid, const State& state);

}  // namespace item_growth_state_0828

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_ITEM_GROWTH_STATE_0828_H_
