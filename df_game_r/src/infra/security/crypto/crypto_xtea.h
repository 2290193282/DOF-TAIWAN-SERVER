#ifndef OLDSRV_SECURITY_CRYPTO_XTEA_H_
#define OLDSRV_SECURITY_CRYPTO_XTEA_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoXTea, 16-byte key and 8-byte block.
class CNCryptoXTea : public ICryptoGraph {
public:
    CNCryptoXTea();
    CNCryptoXTea(const CNCryptoXTea& rhs);
    ~CNCryptoXTea();
    CNCryptoXTea& operator=(const CNCryptoXTea& rhs);
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
    // IDA: xtea_setup @0x80C5388 writes 64 little-endian round words to the
    // graph context allocated by CNCryptoXTea::CNCryptoXTea @0x809D910.
    unsigned int round_keys_[64];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_XTEA_H_
