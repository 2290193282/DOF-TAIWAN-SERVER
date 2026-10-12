#ifndef OLDSRV_SECURITY_CRYPTO_SHIFT_H_
#define OLDSRV_SECURITY_CRYPTO_SHIFT_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoShift. The 8-byte key stores a checked shift count and XOR
// mask. The original four transform bodies call ROL/ROR by value and discard
// the return value, so the persisted transformation is XOR-only.
class CNCryptoShift : public ICryptoGraph {
public:
    CNCryptoShift();
    CNCryptoShift(const CNCryptoShift& rhs);
    ~CNCryptoShift();
    CNCryptoShift& operator=(const CNCryptoShift& rhs);
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
    unsigned int shift_bits_;  // key[0..3] & 0x1F.
    unsigned int xor_mask_;  // key[4..7].
};


#endif  // OLDSRV_SECURITY_CRYPTO_SHIFT_H_
