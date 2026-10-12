#ifndef OLDSRV_SECURITY_CRYPTO_CAST5_H_
#define OLDSRV_SECURITY_CRYPTO_CAST5_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoCast5, 16-byte key and 8-byte block.
class CNCryptoCast5 : public ICryptoGraph {
public:
    CNCryptoCast5();
    CNCryptoCast5(const CNCryptoCast5& rhs);
    ~CNCryptoCast5();
    CNCryptoCast5& operator=(const CNCryptoCast5& rhs);
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
    // IDA: CNCryptoCast5::CNCryptoCast5 @0x8099F0C allocates 0x84 bytes.
    // `_cast5_setup @0x80AA671` writes 32 DWORD subkeys and stores the
    // original key length at DWORD index 32 for the 12/16-round branch.
    unsigned int round_keys_[33];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_CAST5_H_
