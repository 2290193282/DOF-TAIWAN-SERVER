#include "old_game/activity/schedule/blue_marble_runtime.h"

// IDA_EA: 0x80D8C26
// IDA_NAME: _ZN10BlueMarble18setMemoryPoolIndexEi
// DEMANGLED: BlueMarble::setMemoryPoolIndex(int)
BlueMarble* BlueMarble::setMemoryPoolIndex(int index) {
    memory_pool_index_ = index;
    return this;
}

// IDA_EA: 0x80D8C34
// IDA_NAME: _ZNK10BlueMarble18getMemoryPoolIndexEv
// DEMANGLED: BlueMarble::getMemoryPoolIndex(void)const
int BlueMarble::getMemoryPoolIndex() const {
    return memory_pool_index_;
}
