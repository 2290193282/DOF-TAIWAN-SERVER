#include "common/manager/global_tools.h"

// IDA_EA: 0x889209D
// IDA_NAME: _Z18checkGetItemPacketiii
// DEMANGLED: checkGetItemPacket(int,int,int)
int checkGetItemPacket(int backup_seed, int first, int second) {
    const int parity = (first + second) & 1;
    return ((backup_seed & 1) == 0 || parity == 0) &&
           ((backup_seed & 1) == 1 || parity == 1);
}
