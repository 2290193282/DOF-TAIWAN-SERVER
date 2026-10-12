#ifndef OLDSRV_SECURITY_CRYPTO_MULTI2_H_
#define OLDSRV_SECURITY_CRYPTO_MULTI2_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoMulti2, 40-byte key schedule seed and 8-byte block.
class CNCryptoMulti2 : public ICryptoGraph {
public:
    CNCryptoMulti2();
    CNCryptoMulti2(const CNCryptoMulti2& rhs);
    ~CNCryptoMulti2();
    CNCryptoMulti2& operator=(const CNCryptoMulti2& rhs);
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
};


#endif  // OLDSRV_SECURITY_CRYPTO_MULTI2_H_
