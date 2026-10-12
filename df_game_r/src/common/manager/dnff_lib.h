#ifndef OLDSRV_GLOBAL_DNFF_LIB_H_
#define OLDSRV_GLOBAL_DNFF_LIB_H_

// IDA unit: 99_misc_global/DNFFLib.
// IDA_EA: CharacSetSwitch @0x8108BFB.
class DNFFLib {
public:
    // IDA_EA: 0x8108873
    // IDA_NAME: _ZN7DNFFLib10Binary2HexEPKhiPc
    // DEMANGLED: DNFFLib::Binary2Hex(unsigned char const*,int,char *)
    static bool Binary2Hex(const unsigned char* source,
                           int length,
                           char* destination);
    // IDA_EA: 0x81088CF
    // IDA_NAME: _ZN7DNFFLib10Hex2BinaryEPKcPhi
    // DEMANGLED: DNFFLib::Hex2Binary(char const*,unsigned char *,int)
    static int Hex2Binary(const char* input,
                          unsigned char* output,
                          int length);
    // IDA_EA: 0x8108B06
    // IDA_NAME: _ZN7DNFFLib12get_rand_intEi
    // DEMANGLED: DNFFLib::get_rand_int(int)
    static unsigned int get_rand_int(int limit);
    static int CharacSetSwitch(const char* fromcode,
                               const char* tocode,
                               char* input,
                               char* outbuf);
};

#endif  // OLDSRV_GLOBAL_DNFF_LIB_H_
