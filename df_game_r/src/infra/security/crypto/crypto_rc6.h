#ifndef OLDSRV_SECURITY_CRYPTO_RC6_H_
#define OLDSRV_SECURITY_CRYPTO_RC6_H_

#include "infra/security/crypto/crypto_graph.h"

// rc6_key_setup @0x80B5B48 consumes the first 32 bytes as eight little-endian
// key words and stores the 44-byte expanded schedule immediately afterward.
// Model those two regions as the original physical value object so callers do
// not reconstruct its second member with `context + 32`.
struct CNCryptoRc6Context {
    unsigned char key_material[32];
    unsigned char round_schedule[44];
};

static_assert(sizeof(CNCryptoRc6Context) == 0x4Cu,
              "CNCryptoRc6Context IDA ABI size must stay 0x4C");


// IDA: CNCryptoRc6, 60-byte key slice, setup consumes first 32 bytes.
class CNCryptoRc6 : public ICryptoGraph {
public:
    CNCryptoRc6();
    CNCryptoRc6(const CNCryptoRc6& rhs);
    ~CNCryptoRc6();
    CNCryptoRc6& operator=(const CNCryptoRc6& rhs);
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
    CNCryptoRc6Context context_;
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_RC6_H_
