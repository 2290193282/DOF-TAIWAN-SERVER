#include "old_game/economy/shop/npc_name_values.h"

typedef char CNNpcName_Size_Must_Be_0x04[
    (sizeof(CNNpcName) == 0x04) ? 1 : -1];
typedef char CNNpcNameVector_Size_Must_Be_0x0c[
    (sizeof(CNNpcNameVector) == 0x0c) ? 1 : -1];

// IDA_EA: 0x89D7D16
// IDA_NAME: _ZN9CNNpcNameD2Ev
// DEMANGLED: CNNpcName::~CNNpcName()
CNNpcName::~CNNpcName() {}

// IDA_EA: 0x89C3654
// IDA_NAME: _ZN15CNNpcNameVector5clearEv
// DEMANGLED: CNNpcNameVector::clear(void)
int CNNpcNameVector::clear() {
    std::vector<CNNpcName>::clear();
    return 0;
}

// IDA_EA: 0x89C3668
// IDA_NAME: _ZN15CNNpcNameVectorC2Ev
// DEMANGLED: CNNpcNameVector::CNNpcNameVector(void)
CNNpcNameVector::CNNpcNameVector()
    : std::vector<CNNpcName>() {
    clear();
}

// IDA_EA: 0x89DC4BA
// IDA_NAME: _ZN15CNNpcNameVectorD2Ev
// DEMANGLED: CNNpcNameVector::~CNNpcNameVector()
CNNpcNameVector::~CNNpcNameVector() {}
