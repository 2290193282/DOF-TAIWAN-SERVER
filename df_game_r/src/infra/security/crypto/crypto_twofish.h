#ifndef OLDSRV_SECURITY_CRYPTO_TWOFISH_H_
#define OLDSRV_SECURITY_CRYPTO_TWOFISH_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoTwoFish, 32-byte key and 16-byte block.
class CNCryptoTwoFish : public ICryptoGraph {
public:
    CNCryptoTwoFish();
    CNCryptoTwoFish(const CNCryptoTwoFish& rhs);
    ~CNCryptoTwoFish();
    CNCryptoTwoFish& operator=(const CNCryptoTwoFish& rhs);
    int Initialize(const unsigned char* key, int length);
    int SetMode(CRYPTO_MODE_TYPE mode);
    int Encrypt(unsigned char* data, int length);
    int Decrypt(unsigned char* data, int length);
    int Encrypt(const unsigned char* input, int input_length,
                unsigned char* output, int output_capacity);
    int Decrypt(const unsigned char* input, int input_length,
                unsigned char* output, int output_capacity);
    int GetKeySize() const;
    int GetBlockSize() const;

private:
    // IDA: CNCryptoTwoFish::CNCryptoTwoFish @0x809D36C allocates 0x10A0
    // bytes. _twofish_setup stores 1024 key-dependent MDS entries followed by
    // 40 round/subkey DWORDs at index 1024.
    unsigned int sbox_[1024];  // Key-dependent S/MDS table.
    unsigned int subkeys_[40];  // Whitening and 16-round Twofish subkeys.
    bool initialized_;  // True after Initialize succeeds.
};


#endif  // OLDSRV_SECURITY_CRYPTO_TWOFISH_H_
