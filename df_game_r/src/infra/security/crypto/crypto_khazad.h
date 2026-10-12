#ifndef OLDSRV_SECURITY_CRYPTO_KHAZAD_H_
#define OLDSRV_SECURITY_CRYPTO_KHAZAD_H_

#include "infra/security/crypto/crypto_graph.h"


// IDA: CNCryptoKhazad, 16-byte key and 8-byte block.
class CNCryptoKhazad : public ICryptoGraph {
public:
    CNCryptoKhazad();
    CNCryptoKhazad(const CNCryptoKhazad& rhs);
    ~CNCryptoKhazad();
    CNCryptoKhazad& operator=(const CNCryptoKhazad& rhs);
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
    // IDA: CNCryptoKhazad::CNCryptoKhazad @0x809AA54 allocates a 0x90-byte
    // context. khazad_setup @0x80B1C38 writes nine encryption round qwords
    // followed by nine decryption round qwords; khazad_ecb_decrypt uses
    // context+72, matching round_keys_[9].
    unsigned long long round_keys_[18];
    bool initialized_;
};


#endif  // OLDSRV_SECURITY_CRYPTO_KHAZAD_H_
