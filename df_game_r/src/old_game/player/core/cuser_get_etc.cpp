#include "old_game/player/core/cuser.h"

#include <map>


typedef std::map<short, int> UserEtcMap;

// IDA_EA: 0x8697474
// IDA_NAME: _ZN5CUser6GetETCEs
// DEMANGLED: CUser::GetETC(short)
// Active reconstructed-tree ABI: _ZN5CUser6GetETCEs.
int CUser::GetETC(short index) {
    UserEtcMap& values = etc_values_;
    UserEtcMap::iterator found = values.find(index);
    if (found == values.end()) {
        return 0;
    }

    const int value = found->second;
    values.erase(index);
    return value;
}
