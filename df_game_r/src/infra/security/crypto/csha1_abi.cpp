#include "infra/security/crypto/csha1_abi.h"

#include <cstdio>
#include <cstring>

namespace {

uint32_t rotate_left(uint32_t value, unsigned int count) {
    return (value << count) | (value >> (32 - count));
}

uint32_t load_big_endian(const uint8_t* source) {
    return (static_cast<uint32_t>(source[0]) << 24) |
           (static_cast<uint32_t>(source[1]) << 16) |
           (static_cast<uint32_t>(source[2]) << 8) |
           static_cast<uint32_t>(source[3]);
}

}  // namespace

// IDA_EA: 0x8087D28
// IDA_NAME: _ZN5CSHA1C2Ev
// DEMANGLED: CSHA1::CSHA1(void)
CSHA1::CSHA1() : schedule_pointer_(schedule_) {
    Reset();
}

// IDA_EA: 0x8087D4C
// IDA_NAME: _ZN5CSHA1D2Ev
// DEMANGLED: CSHA1::~CSHA1()
CSHA1::~CSHA1() {
    Reset();
}

// IDA_EA: 0x8087D60
// IDA_NAME: _ZN5CSHA15ResetEv
// DEMANGLED: CSHA1::Reset(void)
CSHA1* CSHA1::Reset() {
    state_[0] = 0x67452301U;
    state_[1] = 0xefcdab89U;
    state_[2] = 0x98badcfeU;
    state_[3] = 0x10325476U;
    state_[4] = 0xc3d2e1f0U;
    bit_count_[0] = 0;
    bit_count_[1] = 0;
    return this;
}

// IDA_EA: 0x8087DAA
// IDA_NAME: _ZN5CSHA19TransformEPjPKh
// DEMANGLED: CSHA1::Transform(unsigned int *,unsigned char const*)
uint32_t CSHA1::Transform(uint32_t* state, const uint8_t* source) {
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];

    for (unsigned int index = 0; index != 80; ++index) {
        if (index < 16) {
            schedule_pointer_[index] = load_big_endian(source + index * 4);
        } else {
            schedule_pointer_[index & 15] = rotate_left(
                schedule_pointer_[(index + 13) & 15] ^
                schedule_pointer_[(index + 8) & 15] ^
                schedule_pointer_[(index + 2) & 15] ^
                schedule_pointer_[index & 15], 1);
        }

        uint32_t function_value;
        uint32_t constant;
        if (index < 20) {
            function_value = d ^ (b & (c ^ d));
            constant = 0x5a827999U;
        } else if (index < 40) {
            function_value = b ^ c ^ d;
            constant = 0x6ed9eba1U;
        } else if (index < 60) {
            function_value = (b & c) | (d & (b | c));
            constant = 0x8f1bbcdcU;
        } else {
            function_value = b ^ c ^ d;
            constant = 0xca62c1d6U;
        }

        const uint32_t temporary = rotate_left(a, 5) + function_value + e +
                                   schedule_pointer_[index & 15] + constant;
        e = d;
        d = c;
        c = rotate_left(b, 30);
        b = a;
        a = temporary;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    return state[4];
}

// IDA_EA: 0x808A19E
// IDA_NAME: _ZN5CSHA16UpdateEPKhj
// DEMANGLED: CSHA1::Update(unsigned char const*,unsigned int)
void* CSHA1::Update(const uint8_t* source, uint32_t length) {
    const uint32_t buffer_index = (bit_count_[0] >> 3) & 0x3fU;
    bit_count_[0] += 8 * length;
    if (bit_count_[0] < 8 * length) {
        ++bit_count_[1];
    }
    bit_count_[1] += length >> 29;

    uint32_t consumed = 0;
    if (buffer_index + length > 0x3fU) {
        consumed = 64 - buffer_index;
        std::memcpy(buffer_ + buffer_index, source, consumed);
        Transform(state_, buffer_);
        while (consumed + 63 < length) {
            Transform(state_, source + consumed);
            consumed += 64;
        }
    }
    if (length != consumed) {
        return std::memcpy(buffer_ + ((buffer_index + consumed) & 0x3fU),
                           source + consumed, length - consumed);
    }
    return reinterpret_cast<void*>(static_cast<uintptr_t>(length));
}

// IDA_EA: 0x808A2D2
// IDA_NAME: _ZN5CSHA15FinalEv
// DEMANGLED: CSHA1::Final(void)
bool CSHA1::Final() {
    uint8_t length_bytes[8];
    for (unsigned int index = 0; index <= 7; ++index) {
        length_bytes[index] = static_cast<uint8_t>(
            bit_count_[index <= 3 ? 1 : 0] >> (8 * (~index & 3)));
    }

    static const uint8_t one = 0x80;
    static const uint8_t zero = 0;
    Update(&one, 1);
    while ((bit_count_[0] & 0x1f8U) != 0x1c0U) {
        Update(&zero, 1);
    }
    Update(length_bytes, 8);
    for (unsigned int index = 0; index <= 19; ++index) {
        digest_[index] = static_cast<uint8_t>(
            state_[index >> 2] >> (8 * (~index & 3)));
    }
    return false;
}

// IDA_EA: 0x808A3D8
// IDA_NAME: _ZNK5CSHA17GetHashEPh
// DEMANGLED: CSHA1::GetHash(unsigned char *)const
int CSHA1::GetHash(uint8_t* destination) const {
    if (destination == 0) {
        return 0;
    }
    std::memcpy(destination, digest_, sizeof(digest_));
    return 1;
}

// IDA_EA: 0x808A410
// IDA_NAME: _ZN5CSHA110GetHexHashEPc
// DEMANGLED: CSHA1::GetHexHash(char *)
int CSHA1::GetHexHash(char* destination) {
    if (destination == 0) {
        return 0;
    }
    char hex[3];
    for (unsigned int index = 0; index <= 19; ++index) {
        std::sprintf(hex, "%02X", digest_[index]);
        std::memcpy(destination + 2 * index, hex, 2);
    }
    return 1;
}
