#ifndef OLDSRV_SECURITY_CTEA_ABI_H_
#define OLDSRV_SECURITY_CTEA_ABI_H_

#include <stdint.h>

// IDA unit: 99_misc_global/CTEA.  This is the old 0x48-byte, 8-byte-block
// TEA service used by the login and security-card packet paths.  It is not
// the newer ICryptoGraph CNCryptoTea wrapper.
class CTEA {
public:
    // IDA_EA: 0x80C32F4 / 0x80C3D70 / 0x80C3DA0.
    CTEA();
    virtual ~CTEA();

    // Keep this exact virtual order: the old caller invokes Decrypt through
    // vtable +0x10.
    virtual int Signature(char* output);
    virtual int Encrypt(const char* input, char* output, uint32_t length);
    virtual int Decrypt(const char* input, char* output, uint32_t length);
    virtual int ResetChain();

    // IDA_EA: 0x80C339C.
    int Initialize(const char* key,
                   int key_length,
                   const char* chain0,
                   int mode,
                   int padding);

    static const char sm_chain0[8];

private:
    void EncryptBlock(const unsigned char* input, unsigned char* output);
    void DecryptBlock(const unsigned char* input, unsigned char* output);

    bool initialized_;             // +0x04
    uint8_t reserved_05_[3];
    uint32_t block_size_;           // +0x08
    uint32_t key_size_;             // +0x0c
    uint32_t mode_;                 // +0x10
    uint32_t padding_;              // +0x14
    unsigned char key_bytes_[16];   // +0x18
    uint32_t key_words_[4];         // +0x28
    unsigned char chain0_[8];       // +0x38
    unsigned char chain_[8];        // +0x40
};

#if defined(__i386__)
static_assert(sizeof(CTEA) == 0x48u,
              "CTEA must retain the original i386 0x48-byte ABI");
#endif

#endif  // OLDSRV_SECURITY_CTEA_ABI_H_
