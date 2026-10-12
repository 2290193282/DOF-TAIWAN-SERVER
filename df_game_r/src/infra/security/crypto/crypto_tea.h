#ifndef OLDSRV_SECURITY_CRYPTO_TEA_H_
#define OLDSRV_SECURITY_CRYPTO_TEA_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoTea wraps restored CTEA block/mode code.
class CNCryptoTea : public ICryptoGraph {
public:
    CNCryptoTea();
    CNCryptoTea(const CNCryptoTea& rhs);
    ~CNCryptoTea();
    CNCryptoTea& operator=(const CNCryptoTea& rhs);
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


#endif  // OLDSRV_SECURITY_CRYPTO_TEA_H_
