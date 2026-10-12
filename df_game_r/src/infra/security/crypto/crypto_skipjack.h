#ifndef OLDSRV_SECURITY_CRYPTO_SKIPJACK_H_
#define OLDSRV_SECURITY_CRYPTO_SKIPJACK_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoSkipjack, 10-byte key and 8-byte block.
class CNCryptoSkipjack : public ICryptoGraph {
public:
    CNCryptoSkipjack();
    CNCryptoSkipjack(const CNCryptoSkipjack& rhs);
    ~CNCryptoSkipjack();
    CNCryptoSkipjack& operator=(const CNCryptoSkipjack& rhs);
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
    // IDA: skipjack_setup @0x80C2BCC copies the 10-byte user key into the
    // graph context used by g_func/ig_func.
    unsigned char key_schedule_[10];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_SKIPJACK_H_
