#include "infra/security/crypto/crypto_xtea.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kXTeaKeySize = 16;
const int kXTeaBlockSize = 8;
const unsigned int kXTeaDelta = 0x61C88647U;

int XTeaSetup(const unsigned char* key, int key_length, int rounds,
              unsigned int round_keys[64]) {
    if (key_length != kXTeaKeySize) {
        return 3;
    }
    if (rounds != 0 && rounds != 32) {
        return 4;
    }

    // IDA 0x80C53F2..0x80C54BE: four little-endian key words.
    unsigned int key_words[4];
    key_words[0] = crypto_detail::ReadLE32(key);
    key_words[1] = crypto_detail::ReadLE32(key + 4);
    key_words[2] = crypto_detail::ReadLE32(key + 8);
    key_words[3] = crypto_detail::ReadLE32(key + 12);

    unsigned int sum = 0;
    for (unsigned int round = 0; round <= 0x1F; ++round) {
        round_keys[round] = sum + key_words[sum & 3];
        sum -= kXTeaDelta;
        round_keys[round + 32] = sum + key_words[(sum >> 11) & 3];
    }

    std::memset(key_words, 0, sizeof(key_words));
    return 0;
}

void XTeaEncryptBlock(const unsigned char* input, unsigned char* output,
                      const unsigned int round_keys[64]) {
    unsigned int left = crypto_detail::ReadLE32(input);
    unsigned int right = crypto_detail::ReadLE32(input + 4);

    // IDA xtea_ecb_encrypt @0x80C5536 unrolls four rounds per loop body.
    for (int round = 0; round <= 31; round += 4) {
        left += ((right + ((16 * right) ^ (right >> 5))) ^ round_keys[round]);
        right += ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 32]);
        left += ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round + 1]);
        right += ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 33]);
        left += ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round + 2]);
        right += ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 34]);
        left += ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round + 3]);
        right += ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 35]);
    }

    crypto_detail::WriteLE32(output, left);
    crypto_detail::WriteLE32(output + 4, right);
}

void XTeaDecryptBlock(const unsigned char* input, unsigned char* output,
                      const unsigned int round_keys[64]) {
    unsigned int left = crypto_detail::ReadLE32(input);
    unsigned int right = crypto_detail::ReadLE32(input + 4);

    // IDA xtea_ecb_decrypt @0x80C5786 mirrors the encrypted key order.
    for (int round = 31; round >= 0; round -= 4) {
        right -= ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 32]);
        left -= ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round]);
        right -= ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 31]);
        left -= ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round - 1]);
        right -= ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 30]);
        left -= ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round - 2]);
        right -= ((left + ((16 * left) ^ (left >> 5))) ^
                  round_keys[round + 29]);
        left -= ((right + ((16 * right) ^ (right >> 5))) ^
                 round_keys[round - 3]);
    }

    crypto_detail::WriteLE32(output, left);
    crypto_detail::WriteLE32(output + 4, right);
}

}  // namespace

// IDA_EA: 0x809D8CC
// IDA_NAME: _ZN12CNCryptoXTeaC2Ev
// DEMANGLED: CNCryptoXTea::CNCryptoXTea(void)
CNCryptoXTea::CNCryptoXTea()
    : ICryptoGraph(CRYPTO_GRAPH_XTEA), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809D96A
// IDA_NAME: _ZN12CNCryptoXTeaD2Ev
// DEMANGLED: CNCryptoXTea::~CNCryptoXTea()
// IDA_EA: 0x809D9BE
// IDA_NAME: _ZN12CNCryptoXTeaD0Ev
// DEMANGLED: CNCryptoXTea::~CNCryptoXTea() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoXTea::~CNCryptoXTea() {}

// IDA_EA: 0x809D9DC
// IDA_NAME: _ZN12CNCryptoXTeaC2ERKS_
// DEMANGLED: CNCryptoXTea::CNCryptoXTea(CNCryptoXTea const&)
CNCryptoXTea::CNCryptoXTea(const CNCryptoXTea& rhs)
    : ICryptoGraph(rhs), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809DA00
// IDA_NAME: _ZN12CNCryptoXTeaaSERKS_
// DEMANGLED: CNCryptoXTea::operator=(CNCryptoXTea const&)
CNCryptoXTea& CNCryptoXTea::operator=(const CNCryptoXTea& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809DA20
// IDA_NAME: _ZN12CNCryptoXTea10InitializeEPKhi
// DEMANGLED: CNCryptoXTea::Initialize(unsigned char const*,int)
int CNCryptoXTea::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (GetKeySize() > length) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = XTeaSetup(key, GetKeySize(), 32, round_keys_);
    switch (rc) {
        case 0:
            key_.assign(key, key + GetKeySize());
            initialized_ = true;
            return CRYPTO_RESULT_OK;
        case 3:
            initialized_ = false;
            return CRYPTO_RESULT_INVALID_KEY_SIZE;
        case 4:
            initialized_ = false;
            return CRYPTO_RESULT_INVALID_ROUNDS;
        default:
            initialized_ = false;
            return CRYPTO_RESULT_ERROR;
    }
}

// IDA_EA: 0x809DA08
// IDA_NAME: _ZN12CNCryptoXTea7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoXTea::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoXTea::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809DAC8
// IDA_NAME: _ZN12CNCryptoXTea7EncryptEPhi
// DEMANGLED: CNCryptoXTea::Encrypt(unsigned char *,int)
int CNCryptoXTea::Encrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < length; offset += GetBlockSize()) {
        XTeaEncryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809DBA0
// IDA_NAME: _ZN12CNCryptoXTea7DecryptEPhi
// DEMANGLED: CNCryptoXTea::Decrypt(unsigned char *,int)
int CNCryptoXTea::Decrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < length; offset += GetBlockSize()) {
        XTeaDecryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809DC78
// IDA_NAME: _ZN12CNCryptoXTea7EncryptEPKhiPhi
// DEMANGLED: CNCryptoXTea::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoXTea::Encrypt(const unsigned char* input, int input_length,
                          unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += GetBlockSize()) {
        XTeaEncryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809DD6A
// IDA_NAME: _ZN12CNCryptoXTea7DecryptEPKhiPhi
// DEMANGLED: CNCryptoXTea::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoXTea::Decrypt(const unsigned char* input, int input_length,
                          unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length % GetBlockSize()) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += GetBlockSize()) {
        XTeaDecryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809DE66
// IDA_NAME: _ZNK12CNCryptoXTea10GetKeySizeEv
// DEMANGLED: CNCryptoXTea::GetKeySize(void)const
int CNCryptoXTea::GetKeySize() const {
    return kXTeaKeySize;
}

// IDA_EA: 0x809DE5C
// IDA_NAME: _ZNK12CNCryptoXTea12GetBlockSizeEv
// DEMANGLED: CNCryptoXTea::GetBlockSize(void)const
int CNCryptoXTea::GetBlockSize() const {
    return kXTeaBlockSize;
}
