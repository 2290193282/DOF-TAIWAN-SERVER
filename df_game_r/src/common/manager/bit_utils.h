#ifndef OLDSRV_GLOBAL_BIT_UTILS_H_
#define OLDSRV_GLOBAL_BIT_UTILS_H_

#include <stdint.h>

// IDA_EA: 0x88921C2
// IDA_NAME: _Z11setBit_ByteRhbi
// DEMANGLED: setBit_Byte(unsigned char &,bool,int)
unsigned char* setBit_Byte(unsigned char& value, bool set, int index);

// IDA_EA: 0x88922D7
// IDA_NAME: _Z11setBit_UintRjbi
// DEMANGLED: setBit_Uint(unsigned int &,bool,int)
unsigned int* setBit_Uint(unsigned int& value, bool set, int index);

// IDA_EA: 0x889232E
// IDA_NAME: _Z13isSetBit_Uintji
// DEMANGLED: isSetBit_Uint(unsigned int,int)
bool isSetBit_Uint(unsigned int value, int index);

#endif  // OLDSRV_GLOBAL_BIT_UTILS_H_
