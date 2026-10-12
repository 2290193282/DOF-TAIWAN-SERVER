#ifndef OLDSRV_SECURITY_CSHA1_ABI_H_
#define OLDSRV_SECURITY_CSHA1_ABI_H_

#include <stdint.h>

class CSHA1 {
public:
    CSHA1();
    ~CSHA1();

    CSHA1* Reset();
    uint32_t Transform(uint32_t* state, const uint8_t* source);
    void* Update(const uint8_t* source, uint32_t length);
    bool Final();
    int GetHash(uint8_t* destination) const;
    int GetHexHash(char* destination);

private:
    uint32_t state_[5];
    uint32_t bit_count_[2];
    uint8_t reserved_1c[4];
    uint8_t buffer_[64];
    uint8_t digest_[20];
    uint8_t reserved_74[12];
    uint32_t schedule_[16];
    uint32_t* schedule_pointer_;
};

static_assert(sizeof(CSHA1) == 0xc4, "CSHA1_Size_Must_Be_0xc4");

#endif  // OLDSRV_SECURITY_CSHA1_ABI_H_
