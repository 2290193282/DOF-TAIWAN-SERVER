#ifndef OLDSRV_SECURITY_CRYPTO_BLOWFISH_H_
#define OLDSRV_SECURITY_CRYPTO_BLOWFISH_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoBlowFish pads key material to 56 bytes before setup.
class CNCryptoBlowFish : public ICryptoGraph {
public:
    CNCryptoBlowFish();
    CNCryptoBlowFish(const CNCryptoBlowFish& rhs);
    ~CNCryptoBlowFish();
    CNCryptoBlowFish& operator=(const CNCryptoBlowFish& rhs);
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
    // IDA: CBlowFish context @0x80A956C is 0x1058 bytes:
    // two SBlock values, 18 P-array DWORDs, and four 256-entry S-boxes.
    unsigned int init_chain_[2];
    unsigned int work_chain_[2];
    unsigned int p_[18];
    unsigned int s_[4][256];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_BLOWFISH_H_
