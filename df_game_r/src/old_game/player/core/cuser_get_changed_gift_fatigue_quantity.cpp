#include "old_game/player/core/cuser.h"


// IDA_EA: 0x86975EE
// IDA_NAME: _ZNK5CUser29GetChangedGiftFatigueQuantityEv
// DEMANGLED: CUser::GetChangedGiftFatigueQuantity(void)const
// Active reconstructed-tree ABI: _ZNK5CUser29GetChangedGiftFatigueQuantityEv.
int CUser::GetChangedGiftFatigueQuantity() const {
    return gift_fatigue_changed_;
}
