#include "old_game/player/core/cuser.h"

#include "old_game/economy/inventory/inventory.h"

// IDA_EA: 0x8682D3A
// IDA_NAME: _ZN15cUserHistoryLog12RandomOptionERK10Inven_ItemN18randomOptionReason1TE
// DEMANGLED: cUserHistoryLog::RandomOption(Inven_Item const&,randomOptionReason::T)
int cUserHistoryLog::RandomOption(const Inven_Item& item,
                                  randomOptionReason::T reason) {
    const ::RandomOption& option = item.chronicle_random_option_.random_option_;
    const uint8_t* const bytes = reinterpret_cast<const uint8_t*>(&option);
    return owner_->LogHistory(
        "RandomOptionItem,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d",
        item.item_id_,
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6],
        bytes[7], bytes[8], bytes[9], bytes[10], bytes[11], bytes[12],
        bytes[13], static_cast<int>(reason));
}
