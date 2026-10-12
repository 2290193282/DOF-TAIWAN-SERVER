#include "infra/security/crypto/crypto_skipjack.h"


namespace {

const int kSkipjackKeySize = 10;
const int kSkipjackBlockSize = 8;

// IDA: `_ZL4sbox @0x8B23200`.
const unsigned char kSkipjackSbox[256] = {
    0xA3, 0xD7, 0x09, 0x83, 0xF8, 0x48, 0xF6, 0xF4,
    0xB3, 0x21, 0x15, 0x78, 0x99, 0xB1, 0xAF, 0xF9,
    0xE7, 0x2D, 0x4D, 0x8A, 0xCE, 0x4C, 0xCA, 0x2E,
    0x52, 0x95, 0xD9, 0x1E, 0x4E, 0x38, 0x44, 0x28,
    0x0A, 0xDF, 0x02, 0xA0, 0x17, 0xF1, 0x60, 0x68,
    0x12, 0xB7, 0x7A, 0xC3, 0xE9, 0xFA, 0x3D, 0x53,
    0x96, 0x84, 0x6B, 0xBA, 0xF2, 0x63, 0x9A, 0x19,
    0x7C, 0xAE, 0xE5, 0xF5, 0xF7, 0x16, 0x6A, 0xA2,
    0x39, 0xB6, 0x7B, 0x0F, 0xC1, 0x93, 0x81, 0x1B,
    0xEE, 0xB4, 0x1A, 0xEA, 0xD0, 0x91, 0x2F, 0xB8,
    0x55, 0xB9, 0xDA, 0x85, 0x3F, 0x41, 0xBF, 0xE0,
    0x5A, 0x58, 0x80, 0x5F, 0x66, 0x0B, 0xD8, 0x90,
    0x35, 0xD5, 0xC0, 0xA7, 0x33, 0x06, 0x65, 0x69,
    0x45, 0x00, 0x94, 0x56, 0x6D, 0x98, 0x9B, 0x76,
    0x97, 0xFC, 0xB2, 0xC2, 0xB0, 0xFE, 0xDB, 0x20,
    0xE1, 0xEB, 0xD6, 0xE4, 0xDD, 0x47, 0x4A, 0x1D,
    0x42, 0xED, 0x9E, 0x6E, 0x49, 0x3C, 0xCD, 0x43,
    0x27, 0xD2, 0x07, 0xD4, 0xDE, 0xC7, 0x67, 0x18,
    0x89, 0xCB, 0x30, 0x1F, 0x8D, 0xC6, 0x8F, 0xAA,
    0xC8, 0x74, 0xDC, 0xC9, 0x5D, 0x5C, 0x31, 0xA4,
    0x70, 0x88, 0x61, 0x2C, 0x9F, 0x0D, 0x2B, 0x87,
    0x50, 0x82, 0x54, 0x64, 0x26, 0x7D, 0x03, 0x40,
    0x34, 0x4B, 0x1C, 0x73, 0xD1, 0xC4, 0xFD, 0x3B,
    0xCC, 0xFB, 0x7F, 0xAB, 0xE6, 0x3E, 0x5B, 0xA5,
    0xAD, 0x04, 0x23, 0x9C, 0x14, 0x51, 0x22, 0xF0,
    0x29, 0x79, 0x71, 0x7E, 0xFF, 0x8C, 0x0E, 0xE2,
    0x0C, 0xEF, 0xBC, 0x72, 0x75, 0x6F, 0x37, 0xA1,
    0xEC, 0xD3, 0x8E, 0x62, 0x8B, 0x86, 0x10, 0xE8,
    0x08, 0x77, 0x11, 0xBE, 0x92, 0x4F, 0x24, 0xC5,
    0x32, 0x36, 0x9D, 0xCF, 0xF3, 0xA6, 0xBB, 0xAC,
    0x5E, 0x6C, 0xA9, 0x13, 0x57, 0x25, 0xB5, 0xE3,
    0xBD, 0xA8, 0x3A, 0x01, 0x05, 0x59, 0x2A, 0x46,
};

const int kKeyStep[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
const int kInverseKeyStep[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};

int SkipjackSetup(const unsigned char* key, int key_length, int rounds,
                  unsigned char key_schedule[10]) {
    if (key_length != kSkipjackKeySize) {
        return 3;
    }
    if (rounds != 0 && rounds != 32) {
        return 4;
    }
    for (int index = 0; index <= 9; ++index) {
        key_schedule[index] = key[index];
    }
    return 0;
}

unsigned int GFunc(unsigned int word, int* key_index,
                   const unsigned char key_schedule[10]) {
    unsigned char high = static_cast<unsigned char>(word >> 8);
    unsigned char low = static_cast<unsigned char>(word);

    high ^= kSkipjackSbox[static_cast<unsigned char>(low ^ key_schedule[*key_index])];
    *key_index = kKeyStep[*key_index];
    low ^= kSkipjackSbox[static_cast<unsigned char>(high ^ key_schedule[*key_index])];
    *key_index = kKeyStep[*key_index];
    high ^= kSkipjackSbox[static_cast<unsigned char>(low ^ key_schedule[*key_index])];
    *key_index = kKeyStep[*key_index];
    low ^= kSkipjackSbox[static_cast<unsigned char>(high ^ key_schedule[*key_index])];
    *key_index = kKeyStep[*key_index];
    return (static_cast<unsigned int>(high) << 8) | low;
}

unsigned int IGFunc(unsigned int word, int* key_index,
                    const unsigned char key_schedule[10]) {
    unsigned char high = static_cast<unsigned char>(word >> 8);
    unsigned char low = static_cast<unsigned char>(word);

    *key_index = kInverseKeyStep[*key_index];
    low ^= kSkipjackSbox[static_cast<unsigned char>(high ^ key_schedule[*key_index])];
    *key_index = kInverseKeyStep[*key_index];
    high ^= kSkipjackSbox[static_cast<unsigned char>(low ^ key_schedule[*key_index])];
    *key_index = kInverseKeyStep[*key_index];
    low ^= kSkipjackSbox[static_cast<unsigned char>(high ^ key_schedule[*key_index])];
    *key_index = kInverseKeyStep[*key_index];
    high ^= kSkipjackSbox[static_cast<unsigned char>(low ^ key_schedule[*key_index])];
    return (static_cast<unsigned int>(high) << 8) | low;
}

void SkipjackEncryptBlock(const unsigned char* input, unsigned char* output,
                          const unsigned char key_schedule[10]) {
    unsigned int w1 = (static_cast<unsigned int>(input[0]) << 8) | input[1];
    unsigned int w2 = (static_cast<unsigned int>(input[2]) << 8) | input[3];
    unsigned int w3 = (static_cast<unsigned int>(input[4]) << 8) | input[5];
    unsigned int w4 = (static_cast<unsigned int>(input[6]) << 8) | input[7];
    int key_index = 0;

    for (int round = 1; round <= 8; ++round) {
        const unsigned int g = GFunc(w1, &key_index, key_schedule);
        w1 = w4 ^ g ^ round;
        w4 = w3;
        w3 = w2;
        w2 = g;
    }
    for (int round = 9; round <= 16; ++round) {
        const unsigned int g = GFunc(w1, &key_index, key_schedule);
        const unsigned int old_w4 = w4;
        w4 = w3;
        w3 = w2 ^ w1 ^ round;
        w1 = old_w4;
        w2 = g;
    }
    for (int round = 17; round <= 24; ++round) {
        const unsigned int g = GFunc(w1, &key_index, key_schedule);
        w1 = w4 ^ g ^ round;
        w4 = w3;
        w3 = w2;
        w2 = g;
    }
    for (int round = 25; round <= 32; ++round) {
        const unsigned int g = GFunc(w1, &key_index, key_schedule);
        const unsigned int old_w4 = w4;
        w4 = w3;
        w3 = w2 ^ w1 ^ round;
        w1 = old_w4;
        w2 = g;
    }

    output[0] = static_cast<unsigned char>(w1 >> 8);
    output[1] = static_cast<unsigned char>(w1);
    output[2] = static_cast<unsigned char>(w2 >> 8);
    output[3] = static_cast<unsigned char>(w2);
    output[4] = static_cast<unsigned char>(w3 >> 8);
    output[5] = static_cast<unsigned char>(w3);
    output[6] = static_cast<unsigned char>(w4 >> 8);
    output[7] = static_cast<unsigned char>(w4);
}

void SkipjackDecryptBlock(const unsigned char* input, unsigned char* output,
                          const unsigned char key_schedule[10]) {
    unsigned int w1 = (static_cast<unsigned int>(input[0]) << 8) | input[1];
    unsigned int w2 = (static_cast<unsigned int>(input[2]) << 8) | input[3];
    unsigned int w3 = (static_cast<unsigned int>(input[4]) << 8) | input[5];
    unsigned int w4 = (static_cast<unsigned int>(input[6]) << 8) | input[7];
    int round = 32;
    int key_index = 8;

    for (; round > 24; --round) {
        const unsigned int ig = IGFunc(w2, &key_index, key_schedule);
        w2 = w3 ^ ig ^ round;
        w3 = w4;
        w4 = w1;
        w1 = ig;
    }
    for (; round > 16; --round) {
        const unsigned int old_w4 = w2 ^ w1 ^ round;
        w1 = IGFunc(w2, &key_index, key_schedule);
        w2 = w3;
        w3 = w4;
        w4 = old_w4;
    }
    for (; round > 8; --round) {
        const unsigned int ig = IGFunc(w2, &key_index, key_schedule);
        w2 = w3 ^ ig ^ round;
        w3 = w4;
        w4 = w1;
        w1 = ig;
    }
    for (; round > 0; --round) {
        const unsigned int old_w4 = w2 ^ w1 ^ round;
        w1 = IGFunc(w2, &key_index, key_schedule);
        w2 = w3;
        w3 = w4;
        w4 = old_w4;
    }

    output[0] = static_cast<unsigned char>(w1 >> 8);
    output[1] = static_cast<unsigned char>(w1);
    output[2] = static_cast<unsigned char>(w2 >> 8);
    output[3] = static_cast<unsigned char>(w2);
    output[4] = static_cast<unsigned char>(w3 >> 8);
    output[5] = static_cast<unsigned char>(w3);
    output[6] = static_cast<unsigned char>(w4 >> 8);
    output[7] = static_cast<unsigned char>(w4);
}

}  // namespace

// IDA_EA: 0x809C8AC
// IDA_NAME: _ZN16CNCryptoSkipjackC2Ev
// DEMANGLED: CNCryptoSkipjack::CNCryptoSkipjack(void)
CNCryptoSkipjack::CNCryptoSkipjack()
    : ICryptoGraph(CRYPTO_GRAPH_SKIPJACK),
      key_schedule_(),
      initialized_(false) {}

// IDA_EA: 0x809C94A
// IDA_NAME: _ZN16CNCryptoSkipjackD2Ev
// DEMANGLED: CNCryptoSkipjack::~CNCryptoSkipjack()
// IDA_EA: 0x809C99E
// IDA_NAME: _ZN16CNCryptoSkipjackD0Ev
// DEMANGLED: CNCryptoSkipjack::~CNCryptoSkipjack() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoSkipjack::~CNCryptoSkipjack() {}

// IDA_EA: 0x809C9BC
// IDA_NAME: _ZN16CNCryptoSkipjackC2ERKS_
// DEMANGLED: CNCryptoSkipjack::CNCryptoSkipjack(CNCryptoSkipjack const&)
CNCryptoSkipjack::CNCryptoSkipjack(const CNCryptoSkipjack& rhs)
    : ICryptoGraph(rhs), key_schedule_(), initialized_(false) {}

// IDA_EA: 0x809C9E0
// IDA_NAME: _ZN16CNCryptoSkipjackaSERKS_
// DEMANGLED: CNCryptoSkipjack::operator=(CNCryptoSkipjack const&)
CNCryptoSkipjack& CNCryptoSkipjack::operator=(const CNCryptoSkipjack& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809CA00
// IDA_NAME: _ZN16CNCryptoSkipjack10InitializeEPKhi
// DEMANGLED: CNCryptoSkipjack::Initialize(unsigned char const*,int)
int CNCryptoSkipjack::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (GetKeySize() > length) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = SkipjackSetup(key, GetKeySize(), 32, key_schedule_);
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

// IDA_EA: 0x809C9E8
// IDA_NAME: _ZN16CNCryptoSkipjack7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoSkipjack::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoSkipjack::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809CAA8
// IDA_NAME: _ZN16CNCryptoSkipjack7EncryptEPhi
// DEMANGLED: CNCryptoSkipjack::Encrypt(unsigned char *,int)
int CNCryptoSkipjack::Encrypt(unsigned char* data, int length) {
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
        SkipjackEncryptBlock(data + offset, data + offset, key_schedule_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809CB80
// IDA_NAME: _ZN16CNCryptoSkipjack7DecryptEPhi
// DEMANGLED: CNCryptoSkipjack::Decrypt(unsigned char *,int)
int CNCryptoSkipjack::Decrypt(unsigned char* data, int length) {
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
        SkipjackDecryptBlock(data + offset, data + offset, key_schedule_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809CC58
// IDA_NAME: _ZN16CNCryptoSkipjack7EncryptEPKhiPhi
// DEMANGLED: CNCryptoSkipjack::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoSkipjack::Encrypt(const unsigned char* input, int input_length,
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
        SkipjackEncryptBlock(input + offset, output + offset, key_schedule_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809CD4A
// IDA_NAME: _ZN16CNCryptoSkipjack7DecryptEPKhiPhi
// DEMANGLED: CNCryptoSkipjack::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoSkipjack::Decrypt(const unsigned char* input, int input_length,
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
        SkipjackDecryptBlock(input + offset, output + offset, key_schedule_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809CE46
// IDA_NAME: _ZNK16CNCryptoSkipjack10GetKeySizeEv
// DEMANGLED: CNCryptoSkipjack::GetKeySize(void)const
int CNCryptoSkipjack::GetKeySize() const {
    return kSkipjackKeySize;
}

// IDA_EA: 0x809CE3C
// IDA_NAME: _ZNK16CNCryptoSkipjack12GetBlockSizeEv
// DEMANGLED: CNCryptoSkipjack::GetBlockSize(void)const
int CNCryptoSkipjack::GetBlockSize() const {
    return kSkipjackBlockSize;
}
