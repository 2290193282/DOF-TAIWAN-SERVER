#ifndef OLDSRV_SECURITY_CRYPTO_ANUBIS_H_
#define OLDSRV_SECURITY_CRYPTO_ANUBIS_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoAnubis, 16-byte key and 16-byte block.
class CNCryptoAnubis : public ICryptoGraph {
public:
    CNCryptoAnubis();
    CNCryptoAnubis(const CNCryptoAnubis& rhs);
    ~CNCryptoAnubis();
    CNCryptoAnubis& operator=(const CNCryptoAnubis& rhs);
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
    // IDA: CNCryptoAnubis::CNCryptoAnubis @0x8099264 allocates 0x268 bytes.
    // anubis_ecb_encrypt uses context+8 and anubis_ecb_decrypt uses
    // context+312; for 16-byte packet keys the old wrapper stores 13 round
    // keys for each direction.
    int rounds_;  // Round count; the packet Cipher path uses 12.
    unsigned int encrypt_keys_[13][4];  // Encryption round keys.
    unsigned int decrypt_keys_[13][4];  // Decryption round keys.
    bool initialized_;  // True after a successful Initialize call.
};


#endif  // OLDSRV_SECURITY_CRYPTO_ANUBIS_H_
