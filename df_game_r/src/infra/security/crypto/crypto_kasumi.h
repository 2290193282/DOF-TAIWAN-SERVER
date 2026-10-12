#ifndef OLDSRV_SECURITY_CRYPTO_KASUMI_H_
#define OLDSRV_SECURITY_CRYPTO_KASUMI_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoKasumi, 16-byte key and 8-byte block.
class CNCryptoKasumi : public ICryptoGraph {
public:
    CNCryptoKasumi();
    CNCryptoKasumi(const CNCryptoKasumi& rhs);
    ~CNCryptoKasumi();
    CNCryptoKasumi& operator=(const CNCryptoKasumi& rhs);
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
    // IDA: kasumi_setup @0x80B1A27 writes eight groups of 8 DWORD subkeys
    // into the 0x100-byte context allocated by CNCryptoKasumi::CNCryptoKasumi.
    unsigned int round_keys_[64];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_KASUMI_H_
