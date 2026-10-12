#include "old_game/economy/inventory/inventory.h"

#include <stdint.h>

// IDA_EA: 0x822F012
// IDA_NAME: _ZN13CAccountCargo11GetCapacityEv
// DEMANGLED: CAccountCargo::GetCapacity(void)
int CAccountCargo::GetCapacity() {
    return static_cast<int>(capacity_);
}

// IDA_EA: 0x8695A0C
// IDA_NAME: _ZN13CAccountCargo7IsAlterEv
// DEMANGLED: CAccountCargo::IsAlter(void)
int CAccountCargo::IsAlter() {
    return altered_;
}

// IDA_EA: 0x844DC16
// IDA_NAME: _ZN13CAccountCargo9SetStableEv
// DEMANGLED: CAccountCargo::SetStable(void)
CAccountCargo* CAccountCargo::SetStable() {
    altered_ = 0;
    return this;
}
