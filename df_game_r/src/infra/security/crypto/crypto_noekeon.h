#ifndef OLDSRV_SECURITY_CRYPTO_NOEKEON_H_
#define OLDSRV_SECURITY_CRYPTO_NOEKEON_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoNoekeon, 16-byte key and 16-byte block.
class CNCryptoNoekeon : public ICryptoGraph {
public:
    CNCryptoNoekeon();
    CNCryptoNoekeon(const CNCryptoNoekeon& rhs);
    ~CNCryptoNoekeon();
    CNCryptoNoekeon& operator=(const CNCryptoNoekeon& rhs);
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
    // IDA: noekeon_setup @0x80B4EF6 writes direct encrypt keys at [0..3] and
    // inverse decrypt keys at [4..7] inside the graph context.
    unsigned int round_keys_[8];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_NOEKEON_H_
