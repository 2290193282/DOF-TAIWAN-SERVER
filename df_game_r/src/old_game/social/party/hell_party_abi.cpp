#include "old_game/social/party/hell_party_abi.h"

// IDA_EA: 0x8891BE6
// IDA_NAME: _Z33getHellPartyEntranceItemNeedCounti
// DEMANGLED: getHellPartyEntranceItemNeedCount(int)
int getHellPartyEntranceItemNeedCount(int min_level) {
    return (40 * min_level - 1800) / 100 + 10;
}
