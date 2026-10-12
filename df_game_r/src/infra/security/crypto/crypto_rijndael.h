#ifndef OLDSRV_SECURITY_CRYPTO_RIJNDAEL_H_
#define OLDSRV_SECURITY_CRYPTO_RIJNDAEL_H_

#include "infra/security/crypto/crypto_graph.h"

// Old public CRijndael ABI used directly by DB/login security paths.
// IDA_EA: ctor @0x80B60AC, dtor @0x80B61D2,
// deleting dtor @0x80B6202, sm_chain0 @0x08B22F20.
// The object layout matches the raw stack allocation size observed in IDA.
class CRijndael {
public:
    static const char sm_chain0[16];

    CRijndael();
    ~CRijndael();
    // IDA_EA: 0x80B6220.
    int Initialize(const char* key, int key_length, const char* chain,
                   int key_size, int block_size, int mode, int padding);
    // IDA_EA: 0x80B6A54.
    int ResetChain();
    // IDA_EA: 0x80B6BA0.
    int DefEncryptBlock(const char* input, char* output);
    // IDA_EA: 0x80B70E8.
    int DefDecryptBlock(const char* input, char* output);
    // IDA_EA: 0x80B7636.
    int EncryptBlock(const char* input, char* output);
    // IDA_EA: 0x80B7A20.
    int DecryptBlock(const char* input, char* output);
    // IDA_EA: 0x80B7E00.
    int Encrypt(const char* input, char* output, unsigned int length);
    // IDA_EA: 0x80B8028.
    int Decrypt(const char* input, char* output, unsigned int length);

private:
    unsigned int vptr_;
    unsigned char initialized_;
    unsigned char reserved_[3];
    unsigned int block_size_;
    unsigned int key_size_;
    unsigned int mode_;
    unsigned int padding_;
    unsigned char key_bytes_[32];
    unsigned char round_keys_[448];
    unsigned char decrypt_round_keys_[480];
    unsigned int rounds_;
    unsigned char initial_chain_[32];
    unsigned char chain_[32];
    unsigned char key_words_[32];
    unsigned char work_[32];
    unsigned char work2_[32];
};

static_assert(sizeof(CRijndael) == 0x47C, "CRijndael old object size");


// IDA: CNCryptoRijndael, 16-byte key, 16-byte block.
class CNCryptoRijndael : public ICryptoGraph {
public:
    CNCryptoRijndael();
    CNCryptoRijndael(const CNCryptoRijndael& rhs);
    ~CNCryptoRijndael();
    CNCryptoRijndael& operator=(const CNCryptoRijndael& rhs);
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
    // IDA: CNCryptoRijndael wraps CRijndael, whose Initialize path stores
    // block size, key size, chain, and 11 AES-128 round keys. The rebuild keeps
    // the packet Cipher path at the old wrapper's 16-byte key/block settings.
    unsigned char round_keys_[11][16];  // AES-128 encryption round keys.
    unsigned char decrypt_round_keys_[11][16];  // Retained CRijndael decrypt table.
    unsigned char initial_chain_[16];  // CBC/OFB initial chain from sm_chain0.
    unsigned char chain_[16];  // Active CBC/OFB chain.
    bool initialized_;  // True after Initialize succeeds.
};


#endif  // OLDSRV_SECURITY_CRYPTO_RIJNDAEL_H_
