#include "old_game/economy/equipment/item_growth_state.h"

#include "infra/concurrent/thread/mutex.h"
#include "old_game/economy/equipment/item_option.h"

#include <map>
#include <string>


namespace item_growth_state {
namespace {

typedef std::map<std::string, State> StateMap;

StateMap& States() {
    static StateMap values;
    return values;
}

Mutex& StateMutex() {
    static Mutex mutex;
    return mutex;
}

std::string MakeKey(const void* guid_bytes) {
    return std::string(static_cast<const char*>(guid_bytes), kGuidBytes);
}

}  // namespace


State::State() : progress(0), fatigue_state(0) {}

State::State(uint16_t progress_value, uint8_t fatigue_value)
    : progress(progress_value), fatigue_state(fatigue_value) {}

bool IsValidGuid(const void* guid_bytes) {
    if (!guid_bytes) return false;
    const uint8_t* const bytes = static_cast<const uint8_t*>(guid_bytes);
    for (int index = 0; index < kGuidBytes; ++index) {
        if (bytes[index] != 0) return true;
    }
    return false;
}

bool LookupBytes(const void* guid_bytes, State* state) {
    if (state) *state = State();
    if (!IsValidGuid(guid_bytes)) return false;

    MutexGuard guard(&StateMutex());
    const StateMap::const_iterator found = States().find(MakeKey(guid_bytes));
    if (found == States().end()) return false;
    if (state) *state = found->second;
    return true;
}

bool Lookup(const itemGloballyUniqueIdentifier_t& guid, State* state) {
    return LookupBytes(&guid, state);
}

bool InstallBytes(const void* guid_bytes, const State& state) {
    if (!IsValidGuid(guid_bytes)) return false;
    MutexGuard guard(&StateMutex());
    States()[MakeKey(guid_bytes)] = state;
    return true;
}

bool Install(const itemGloballyUniqueIdentifier_t& guid,
             const State& state) {
    return InstallBytes(&guid, state);
}

bool Update(const itemGloballyUniqueIdentifier_t& guid, const State& state) {
    return Install(guid, state);
}

}  // namespace item_growth_state_0828
