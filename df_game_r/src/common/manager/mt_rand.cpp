#include "common/manager/mt_rand.h"

#include <cstdio>
#include <ctime>


namespace {

const unsigned int kDefaultSeed = 0x012BD6AAU;
const unsigned int kMatrixA = 0x9908B0DFU;

}  // namespace

// IDA_EA: 0x80D9F48
// IDA_NAME: _ZN7CMTRandC2Ev
// DEMANGLED: CMTRand::CMTRand(void)
CMTRand::CMTRand() : state_(), next_(0), left_(0) {
    seed();
}

// IDA_EA: 0x80CBA2A
// IDA_NAME: _ZN7CMTRandC2ERKm
// DEMANGLED: CMTRand::CMTRand(unsigned long const&)
CMTRand::CMTRand(const unsigned long& seed_value)
    : state_(), next_(0), left_(0) {
    seed(seed_value);
}

// IDA_EA: 0x80CB97C
// IDA_NAME: _ZNK7CMTRand5hiBitERKm
// DEMANGLED: CMTRand::hiBit(unsigned long const&)const
unsigned int CMTRand::hiBit(const unsigned long& value) const {
    return value & 0x80000000U;
}

// IDA_EA: 0x80CB98C
// IDA_NAME: _ZNK7CMTRand5loBitERKm
// DEMANGLED: CMTRand::loBit(unsigned long const&)const
unsigned int CMTRand::loBit(const unsigned long& value) const {
    return value & 1U;
}

// IDA_EA: 0x80CB99A
// IDA_NAME: _ZNK7CMTRand6loBitsERKm
// DEMANGLED: CMTRand::loBits(unsigned long const&)const
unsigned int CMTRand::loBits(const unsigned long& value) const {
    return value & 0x7FFFFFFFU;
}

// IDA_EA: 0x80CB9AA
// IDA_NAME: _ZNK7CMTRand7mixBitsERKmS1_
// DEMANGLED: CMTRand::mixBits(unsigned long const&,unsigned long const&)const
unsigned int CMTRand::mixBits(const unsigned long& first,
                              const unsigned long& second) const {
    return hiBit(first) | loBits(second);
}

// IDA_EA: 0x80CB9E0
// IDA_NAME: _ZNK7CMTRand5twistERKmS1_S1_
// DEMANGLED: CMTRand::twist(unsigned long const&,unsigned long const&,unsigned long const&)const
unsigned int CMTRand::twist(const unsigned long& middle,
                            const unsigned long& first,
                            const unsigned long& second) const {
    unsigned int value = (mixBits(first, second) >> 1) ^ middle;
    return value ^ ((0U - loBit(second)) & kMatrixA);
}

// IDA_EA: 0x80CBB44
// IDA_NAME: _ZN7CMTRand10initializeEm
// DEMANGLED: CMTRand::initialize(unsigned long)
bool CMTRand::initialize(unsigned long seed_value) {
    state_[0] = seed_value;
    for (int index = 1; index <= 623; ++index) {
        state_[index] =
            1812433253U * ((state_[index - 1] >> 30) ^ state_[index - 1]) +
            static_cast<unsigned int>(index);
    }
    return false;
}

// IDA_EA: 0x80CBB90
// IDA_NAME: _ZN7CMTRand6reloadEv
// DEMANGLED: CMTRand::reload(void)
CMTRand* CMTRand::reload() {
    unsigned int* cursor = state_;
    int count = 227;
    while (count-- != 0) {
        *cursor = twist(*(cursor + 397), *cursor, *(cursor + 1));
        ++cursor;
    }

    count = 397;
    while (--count) {
        *cursor = twist(*(cursor - 227), *cursor, *(cursor + 1));
        ++cursor;
    }

    *cursor = twist(*(cursor - 227), *cursor, state_[0]);
    left_ = 624;
    next_ = state_;
    return this;
}

// IDA_EA: 0x80CBB1E
// IDA_NAME: _ZN7CMTRand4seedEm
// DEMANGLED: CMTRand::seed(unsigned long)
CMTRand* CMTRand::seed(unsigned long seed_value) {
    initialize(seed_value);
    return reload();
}

// IDA_EA: 0x80D9F5C
// IDA_NAME: _ZN7CMTRand4seedEPmm
// DEMANGLED: CMTRand::seed(unsigned long *,unsigned long)
CMTRand* CMTRand::seed(unsigned long* seed_values, unsigned long count) {
    initialize(kDefaultSeed);
    int state_index = 1;
    unsigned int seed_index = 0;
    unsigned int loop_count = count < 624 ? 624 : count;

    for (; loop_count != 0; --loop_count) {
        state_[state_index] ^=
            1664525U * (state_[state_index - 1] ^
                        (state_[state_index - 1] >> 30));
        state_[state_index] += seed_index + seed_values[seed_index];
        ++state_index;
        ++seed_index;
        if (state_index > 623) {
            state_[0] = state_[623];
            state_index = 1;
        }
        if (seed_index >= count) {
            seed_index = 0;
        }
    }

    for (int remaining = 623; remaining != 0; --remaining) {
        state_[state_index] ^=
            1566083941U * (state_[state_index - 1] ^
                           (state_[state_index - 1] >> 30));
        state_[state_index] -= static_cast<unsigned int>(state_index);
        if (++state_index > 623) {
            state_[0] = state_[623];
            state_index = 1;
        }
    }

    state_[0] = 0x80000000U;
    return reload();
}

// IDA_EA: 0x80DA0E6
// IDA_NAME: _ZN7CMTRand4seedEv
// DEMANGLED: CMTRand::seed(void)
CMTRand* CMTRand::seed() {
    unsigned long seed_values[624];
    FILE* stream = std::fopen("/dev/urandom", "rb");
    if (stream != 0) {
        bool read_ok = true;
        unsigned long* cursor = seed_values;
        int remaining = 624;
        while (read_ok) {
            if (remaining-- == 0) {
                break;
            }
            read_ok = std::fread(cursor, 4, 1, stream) != 0;
            ++cursor;
        }
        std::fclose(stream);
        if (read_ok) {
            return seed(seed_values, 624);
        }
    }

    const long clock_value = static_cast<long>(std::clock());
    const long time_value = static_cast<long>(std::time(0));
    seed(hash(time_value, clock_value));
    return this;
}

// IDA_EA: 0x80CBA46
// IDA_NAME: _ZN7CMTRand7randIntEv
// DEMANGLED: CMTRand::randInt(void)
unsigned int CMTRand::randInt() {
    if (left_ == 0) {
        reload();
    }
    --left_;
    unsigned int value = *next_++;
    unsigned int mixed =
        ((((value >> 11) ^ value) << 7) & 0x9D2C5680U) ^
        (value >> 11) ^ value;
    return ((mixed << 15) & 0xEFC60000U) ^ mixed ^
           ((((mixed << 15) & 0xEFC60000U) ^ mixed) >> 18);
}

// IDA_EA: 0x80CBABE
// IDA_NAME: _ZN7CMTRand7randIntERKm
// DEMANGLED: CMTRand::randInt(unsigned long const&)
unsigned int CMTRand::randInt(const unsigned long& limit) {
    unsigned int mask = (((limit >> 1) | limit) >> 2) | (limit >> 1) | limit;
    mask = (((mask >> 4) | mask) >> 8) | (mask >> 4) | mask;
    mask = (mask >> 16) | mask;

    unsigned int value = 0;
    do {
        value = mask & randInt();
    } while (limit < value);
    return value;
}

// IDA_EA: 0x80DA1D6
// IDA_NAME: _ZN7CMTRand4hashEll
// DEMANGLED: CMTRand::hash(long,long)
unsigned int CMTRand::hash(long first, long second) {
    static unsigned int differ = 0;
    unsigned int left = 0;
    const unsigned char* first_bytes =
        reinterpret_cast<const unsigned char*>(&first);
    for (unsigned int index = 0; index <= 3; ++index) {
        left = (left << 8) + left;
        left += first_bytes[index];
    }

    unsigned int right = 0;
    const unsigned char* second_bytes =
        reinterpret_cast<const unsigned char*>(&second);
    for (unsigned int index = 0; index <= 3; ++index) {
        right = (right << 8) + right;
        right += second_bytes[index];
    }

    const unsigned int result = (differ + left) ^ right;
    ++differ;
    return result;
}
