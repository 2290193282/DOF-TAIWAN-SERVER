#ifndef OLDSRV_RUNTIME_MT_RAND_H_
#define OLDSRV_RUNTIME_MT_RAND_H_

#include <stdint.h>


// IDA: CMTRand @0x80CBA2A..0x80DA1D6.
// Object size is 0x9C8: 624 state dwords, current-state pointer, and left
// counter. The layout is used by expert_extraction::CExtraction at +0x04.
class CMTRand {
public:
    CMTRand();
    explicit CMTRand(const unsigned long& seed_value);

    // IDA: CMTRand::seed(unsigned long) @0x80CBB1E.
    CMTRand* seed(unsigned long seed_value);

    // IDA: CMTRand::seed(unsigned long*,unsigned long) @0x80D9F5C.
    CMTRand* seed(unsigned long* seed_values, unsigned long count);

    // IDA: CMTRand::seed() @0x80DA0E6.
    CMTRand* seed();

    // IDA: CMTRand::randInt() @0x80CBA46.
    unsigned int randInt();

    // IDA: CMTRand::randInt(unsigned long const&) @0x80CBABE.
    unsigned int randInt(const unsigned long& limit);

private:
    unsigned int hiBit(const unsigned long& value) const;
    unsigned int loBit(const unsigned long& value) const;
    unsigned int loBits(const unsigned long& value) const;
    unsigned int mixBits(const unsigned long& first,
                         const unsigned long& second) const;
    unsigned int twist(const unsigned long& middle,
                       const unsigned long& first,
                       const unsigned long& second) const;
    bool initialize(unsigned long seed_value);
    CMTRand* reload();
    static unsigned int hash(long first, long second);

    unsigned int state_[624];      // IDA +0x000: MT19937 state table.
    unsigned int* next_;           // IDA +0x9C0: next state dword pointer.
    unsigned int left_;            // IDA +0x9C4: remaining dwords before reload.
};


#endif  // OLDSRV_RUNTIME_MT_RAND_H_
