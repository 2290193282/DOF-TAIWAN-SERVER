#include "common/manager/bit_utils.h"

namespace {

// Old i386 `shl` consumes the low five bits of CL.  Mask explicitly so the
// source reproduces the old machine behavior for every int input, rather than
// relying on an undefined C++ shift count.
unsigned int old_i386_bit_mask(int index) {
    return 1u << (static_cast<unsigned int>(index) & 31u);
}

}  // namespace

// IDA_EA: 0x88921C2
// IDA_NAME: _Z11setBit_ByteRhbi
// DEMANGLED: setBit_Byte(unsigned char &,bool,int)
unsigned char* setBit_Byte(unsigned char& value, bool set, int index) {
    const unsigned int mask = old_i386_bit_mask(index);
    if (set) {
        value = static_cast<unsigned char>(value | mask);
    } else {
        value = static_cast<unsigned char>(value & ~mask);
    }
    return &value;
}

// IDA_EA: 0x88922D7
// IDA_NAME: _Z11setBit_UintRjbi
// DEMANGLED: setBit_Uint(unsigned int &,bool,int)
unsigned int* setBit_Uint(unsigned int& value, bool set, int index) {
    const unsigned int mask = old_i386_bit_mask(index);
    if (set) {
        value |= mask;
    } else {
        value &= ~mask;
    }
    return &value;
}

// IDA_EA: 0x889232E
// IDA_NAME: _Z13isSetBit_Uintji
// DEMANGLED: isSetBit_Uint(unsigned int,int)
bool isSetBit_Uint(unsigned int value, int index) {
    return (value & old_i386_bit_mask(index)) != 0u;
}
