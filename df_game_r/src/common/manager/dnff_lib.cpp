#include "common/manager/dnff_lib.h"

#include "common/manager/global_tools.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iconv.h>

namespace {

std::uint32_t AdvanceLegacyRandState(std::uint32_t state) {
    return 1103515245u * state + 12345u;
}

// This is the signed truncation-toward-zero divide emitted by the original
// i386 sequence: sar; shr; lea; sar.  Keep it explicit so the result does
// not depend on signed-overflow behavior in a modern compiler.
std::int32_t DivideLegacyRandStateBy65536(std::uint32_t state) {
    if ((state & 0x80000000u) == 0u) {
        return static_cast<std::int32_t>(state >> 16);
    }
    return -static_cast<std::int32_t>((std::uint32_t(0) - state) >> 16);
}

}  // namespace

// IDA_EA: 0x8108873
// IDA_NAME: _ZN7DNFFLib10Binary2HexEPKhiPc
// DEMANGLED: DNFFLib::Binary2Hex(unsigned char const*,int,char *)
bool DNFFLib::Binary2Hex(const unsigned char* source,
                         int length,
                         char* destination) {
    char hex_byte[3];
    int index = 0;

    *destination = '\0';
    while (true) {
        const bool result = index < length;
        if (index >= length) {
            return result;
        }
        Char2Hex(*source, hex_byte);
        std::strcat(destination, hex_byte);
        ++index;
        ++source;
    }
}

// IDA_EA: 0x81088CF
// IDA_NAME: _ZN7DNFFLib10Hex2BinaryEPKcPhi
// DEMANGLED: DNFFLib::Hex2Binary(char const*,unsigned char *,int)
int DNFFLib::Hex2Binary(const char* input,
                        unsigned char* output,
                        int length) {
    int index = 0;
    while (index < length) {
        unsigned char value = 0;
        if (Hex2Char(input, value) != 1) {
            return 0;
        }
        *output = value;
        ++index;
        input += 2;
        ++output;
    }
    return 1;
}

// IDA_EA: 0x8108B06
// IDA_NAME: _ZN7DNFFLib12get_rand_intEi
// DEMANGLED: DNFFLib::get_rand_int(int)
unsigned int DNFFLib::get_rand_int(int limit) {
    if (limit < 0) {
        return 0;
    }
    if (limit == 0) {
        return static_cast<unsigned int>(::rand());
    }

    const int first = ::rand();
    if (first > limit) {
        return static_cast<unsigned int>(::rand() % limit);
    }

    std::uint32_t state = static_cast<std::uint32_t>(first);
    state = AdvanceLegacyRandState(state);
    std::uint32_t value =
        static_cast<std::uint32_t>(DivideLegacyRandStateBy65536(state)) &
        0x7FFu;

    state = AdvanceLegacyRandState(state);
    value <<= 10;
    value ^=
        static_cast<std::uint32_t>(DivideLegacyRandStateBy65536(state)) &
        0x3FFu;

    state = AdvanceLegacyRandState(state);
    value <<= 10;
    value ^=
        static_cast<std::uint32_t>(DivideLegacyRandStateBy65536(state)) &
        0x3FFu;

    if (static_cast<std::uint32_t>(limit) >= value) {
        return value;
    }
    return value % static_cast<std::uint32_t>(limit);
}

// IDA_EA: 0x8108BFB
// IDA_NAME: _ZN7DNFFLib15CharacSetSwitchEPKcS1_PcS2_
// DEMANGLED: DNFFLib::CharacSetSwitch(char const*,char const*,char *,char *)
int DNFFLib::CharacSetSwitch(const char* fromcode,
                             const char* tocode,
                             char* input,
                             char* outbuf_1) {
    char* s = 0;
    char* outbuf = 0;
    size_t inbytesleft = 0;
    size_t outbytesleft = 0;
    size_t v13 = 0;
    iconv_t cd = iconv_open(tocode, fromcode);
    if (cd == reinterpret_cast<iconv_t>(-1)) {
        std::printf("iconv_open error : %s\n", std::strerror(errno));
        return 0;
    }

    s = input;
    outbuf = outbuf_1;
    inbytesleft = std::strlen(input);
    outbytesleft = 3 * inbytesleft;
    v13 = 3 * inbytesleft;
    const size_t converted =
        iconv(cd, &s, &inbytesleft, &outbuf, &outbytesleft);
    if (converted == static_cast<size_t>(-1)) {
        std::printf("iconv error : %s\n", std::strerror(errno));
        return 0;
    }

    outbuf_1[v13 - outbytesleft] = 0;
    iconv_close(cd);
    return 1;
}
