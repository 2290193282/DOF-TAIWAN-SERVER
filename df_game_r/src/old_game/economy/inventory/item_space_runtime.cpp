#include "old_game/economy/inventory/item_space_runtime.h"

// IDA_EA: 0x80F7845
// IDA_NAME: _Z25GetInvenTypeFromItemSpace14ENUM_ITEMSPACE
// DEMANGLED: GetInvenTypeFromItemSpace(ENUM_ITEMSPACE)
int GetInvenTypeFromItemSpace(ENUM_ITEMSPACE item_space) {
    switch (static_cast<int>(item_space)) {
        case 0:
            return 1;
        case 1:
            return 2;
        case 2:
            return 4;
        case 3:
            return 0;
        case 7:
            return 3;
        case 18:
            return 5;
        default:
            return 6;
    }
}
