#include "infra/security/crypto/crypto_noekeon.h"

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kNoekeonKeySize = 16;
const int kNoekeonBlockSize = 16;

// IDA: `_ZL2RC @0x8B1F440`, indexed by encrypt/decrypt rounds.
const unsigned int kNoekeonRC[17] = {
    0x80, 0x1B, 0x36, 0x6C, 0xD8, 0xAB, 0x4D, 0x9A, 0x2F,
    0x5E, 0xBC, 0x63, 0xC6, 0x97, 0x35, 0x6A, 0xD4,
};

void NoekeonTheta(unsigned int* a0, unsigned int* a1, unsigned int* a2,
                  unsigned int* a3, const unsigned int* key) {
    unsigned int temp = *a0 ^ *a2;
    temp ^= crypto_detail::RotateLeft32(temp, 8) ^
            crypto_detail::RotateRight32(temp, 8);
    *a1 ^= temp ^ key[1];
    *a3 ^= temp ^ key[3];

    temp = *a1 ^ *a3;
    temp ^= crypto_detail::RotateLeft32(temp, 8) ^
            crypto_detail::RotateRight32(temp, 8);
    *a0 ^= temp ^ key[0];
    *a2 ^= temp ^ key[2];
}

void NoekeonGammaPi(unsigned int* a0, unsigned int* a1, unsigned int* a2,
                    unsigned int* a3) {
    unsigned int b0 = crypto_detail::RotateLeft32(*a0, 1);
    unsigned int b2 = crypto_detail::RotateLeft32(*a2, 5);
    unsigned int b3 = crypto_detail::RotateLeft32(*a3, 2);
    unsigned int b1 = ~(b3 | b2) ^ *a1;
    b0 ^= b2 & b1;
    b2 ^= b0 ^ b3 ^ b1;
    b1 ^= ~(b0 | b2);
    *a0 = crypto_detail::RotateRight32(b2 & b1 ^ b3, 1);
    *a1 = b1;
    *a2 = crypto_detail::RotateRight32(b2, 5);
    *a3 = crypto_detail::RotateRight32(b0, 2);
}

int NoekeonSetup(const unsigned char* key, int key_length, int rounds,
                 unsigned int round_keys[8]) {
    if (key_length != kNoekeonKeySize) {
        return 3;
    }
    if (rounds != 0 && rounds != 16) {
        return 4;
    }

    for (int index = 0; index < 4; ++index) {
        const unsigned int word = crypto_detail::ReadBE32(key + index * 4);
        round_keys[index] = word;
        round_keys[index + 4] = word;
    }

    // IDA 0x80B5162..0x80B5217 derives the inverse-key half with theta(null).
    unsigned int temp = round_keys[4] ^ round_keys[6];
    temp ^= crypto_detail::RotateLeft32(temp, 8) ^
            crypto_detail::RotateRight32(temp, 8);
    round_keys[5] ^= temp;
    round_keys[7] ^= temp;

    temp = round_keys[5] ^ round_keys[7];
    temp ^= crypto_detail::RotateLeft32(temp, 8) ^
            crypto_detail::RotateRight32(temp, 8);
    round_keys[4] ^= temp;
    round_keys[6] ^= temp;
    return 0;
}

void NoekeonEncryptBlock(const unsigned char* input, unsigned char* output,
                         const unsigned int round_keys[8]) {
    unsigned int a0 = crypto_detail::ReadBE32(input);
    unsigned int a1 = crypto_detail::ReadBE32(input + 4);
    unsigned int a2 = crypto_detail::ReadBE32(input + 8);
    unsigned int a3 = crypto_detail::ReadBE32(input + 12);

    for (int round = 0; round <= 15; ++round) {
        a0 ^= kNoekeonRC[round];
        NoekeonTheta(&a0, &a1, &a2, &a3, round_keys);
        NoekeonGammaPi(&a0, &a1, &a2, &a3);
    }

    a0 ^= kNoekeonRC[16];
    NoekeonTheta(&a0, &a1, &a2, &a3, round_keys);

    crypto_detail::WriteBE32(output, a0);
    crypto_detail::WriteBE32(output + 4, a1);
    crypto_detail::WriteBE32(output + 8, a2);
    crypto_detail::WriteBE32(output + 12, a3);
}

void NoekeonDecryptBlock(const unsigned char* input, unsigned char* output,
                         const unsigned int round_keys[8]) {
    unsigned int a0 = crypto_detail::ReadBE32(input);
    unsigned int a1 = crypto_detail::ReadBE32(input + 4);
    unsigned int a2 = crypto_detail::ReadBE32(input + 8);
    unsigned int a3 = crypto_detail::ReadBE32(input + 12);
    const unsigned int* decrypt_key = round_keys + 4;

    for (int round = 16; round > 0; --round) {
        NoekeonTheta(&a0, &a1, &a2, &a3, decrypt_key);
        a0 ^= kNoekeonRC[round];
        NoekeonGammaPi(&a0, &a1, &a2, &a3);
    }

    NoekeonTheta(&a0, &a1, &a2, &a3, decrypt_key);
    a0 ^= kNoekeonRC[0];

    crypto_detail::WriteBE32(output, a0);
    crypto_detail::WriteBE32(output + 4, a1);
    crypto_detail::WriteBE32(output + 8, a2);
    crypto_detail::WriteBE32(output + 12, a3);
}

}  // namespace

// IDA_EA: 0x809B59C
// IDA_NAME: _ZN15CNCryptoNoekeonC2Ev
// DEMANGLED: CNCryptoNoekeon::CNCryptoNoekeon(void)
CNCryptoNoekeon::CNCryptoNoekeon()
    : ICryptoGraph(CRYPTO_GRAPH_NOEKEON), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809B63A
// IDA_NAME: _ZN15CNCryptoNoekeonD2Ev
// DEMANGLED: CNCryptoNoekeon::~CNCryptoNoekeon()
// IDA_EA: 0x809B68E
// IDA_NAME: _ZN15CNCryptoNoekeonD0Ev
// DEMANGLED: CNCryptoNoekeon::~CNCryptoNoekeon() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoNoekeon::~CNCryptoNoekeon() {}

// IDA_EA: 0x809B6AC
// IDA_NAME: _ZN15CNCryptoNoekeonC2ERKS_
// DEMANGLED: CNCryptoNoekeon::CNCryptoNoekeon(CNCryptoNoekeon const&)
CNCryptoNoekeon::CNCryptoNoekeon(const CNCryptoNoekeon& rhs)
    : ICryptoGraph(rhs), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809B6D0
// IDA_NAME: _ZN15CNCryptoNoekeonaSERKS_
// DEMANGLED: CNCryptoNoekeon::operator=(CNCryptoNoekeon const&)
CNCryptoNoekeon& CNCryptoNoekeon::operator=(const CNCryptoNoekeon& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809B6F0
// IDA_NAME: _ZN15CNCryptoNoekeon10InitializeEPKhi
// DEMANGLED: CNCryptoNoekeon::Initialize(unsigned char const*,int)
int CNCryptoNoekeon::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (GetKeySize() > length) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = NoekeonSetup(key, GetKeySize(), 16, round_keys_);
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

// IDA_EA: 0x809B6D8
// IDA_NAME: _ZN15CNCryptoNoekeon7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoNoekeon::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoNoekeon::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809B798
// IDA_NAME: _ZN15CNCryptoNoekeon7EncryptEPhi
// DEMANGLED: CNCryptoNoekeon::Encrypt(unsigned char *,int)
int CNCryptoNoekeon::Encrypt(unsigned char* data, int length) {
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
        NoekeonEncryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809B870
// IDA_NAME: _ZN15CNCryptoNoekeon7DecryptEPhi
// DEMANGLED: CNCryptoNoekeon::Decrypt(unsigned char *,int)
int CNCryptoNoekeon::Decrypt(unsigned char* data, int length) {
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
        NoekeonDecryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809B948
// IDA_NAME: _ZN15CNCryptoNoekeon7EncryptEPKhiPhi
// DEMANGLED: CNCryptoNoekeon::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoNoekeon::Encrypt(const unsigned char* input, int input_length,
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
        NoekeonEncryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809BA3A
// IDA_NAME: _ZN15CNCryptoNoekeon7DecryptEPKhiPhi
// DEMANGLED: CNCryptoNoekeon::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoNoekeon::Decrypt(const unsigned char* input, int input_length,
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
        NoekeonDecryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809BB36
// IDA_NAME: _ZNK15CNCryptoNoekeon10GetKeySizeEv
// DEMANGLED: CNCryptoNoekeon::GetKeySize(void)const
int CNCryptoNoekeon::GetKeySize() const {
    return kNoekeonKeySize;
}

// IDA_EA: 0x809BB2C
// IDA_NAME: _ZNK15CNCryptoNoekeon12GetBlockSizeEv
// DEMANGLED: CNCryptoNoekeon::GetBlockSize(void)const
int CNCryptoNoekeon::GetBlockSize() const {
    return kNoekeonBlockSize;
}
