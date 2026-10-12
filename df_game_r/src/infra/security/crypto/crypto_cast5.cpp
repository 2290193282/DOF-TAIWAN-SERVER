#include "infra/security/crypto/crypto_cast5.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kCast5KeySize = 16;
const int kCast5BlockSize = 8;
const int kCast5RoundKeyCount = 32;
const int kCast5KeyLengthSlot = 32;

#include "infra/security/crypto/crypto_cast5_tables.inc"

unsigned int Byte0(unsigned int value) {
    return value & 0xFFU;
}

unsigned int Byte1(unsigned int value) {
    return (value >> 8) & 0xFFU;
}

unsigned int Byte2(unsigned int value) {
    return (value >> 16) & 0xFFU;
}

unsigned int Byte3(unsigned int value) {
    return (value >> 24) & 0xFFU;
}

unsigned int CastRol(unsigned int value, unsigned int count) {
    return crypto_detail::RotateLeft32(value, count & 0x1FU);
}

unsigned int CastFI(unsigned int data, unsigned int mask, unsigned int rotate) {
    const unsigned int v = CastRol(mask + data, rotate);
    return (kCastS2[Byte2(v)] ^ kCastS1[Byte3(v)]) -
           kCastS3[Byte1(v)] + kCastS4[Byte0(v)];
}

unsigned int CastFII(unsigned int data, unsigned int mask, unsigned int rotate) {
    const unsigned int v = CastRol(mask ^ data, rotate);
    return (kCastS3[Byte1(v)] + kCastS1[Byte3(v)] -
            kCastS2[Byte2(v)]) ^ kCastS4[Byte0(v)];
}

unsigned int CastFIII(unsigned int data, unsigned int mask,
                      unsigned int rotate) {
    const unsigned int v = CastRol(mask - data, rotate);
    return (kCastS3[Byte1(v)] ^ (kCastS2[Byte2(v)] + kCastS1[Byte3(v)])) -
           kCastS4[Byte0(v)];
}

unsigned int CastRound(unsigned int data, int round,
                       const unsigned int round_keys[33]) {
    const unsigned int mask = round_keys[round];
    const unsigned int rotate = round_keys[round + 16];
    switch (round % 3) {
        case 0:
            return CastFI(data, mask, rotate);
        case 1:
            return CastFII(data, mask, rotate);
        default:
            return CastFIII(data, mask, rotate);
    }
}

void Cast5GenerateBlockA(unsigned int* v6, unsigned int* v7, unsigned int* v8,
                         unsigned int* v9, unsigned int v10, unsigned int v11,
                         unsigned int v12, unsigned int v13) {
    *v9 = kCastS8[Byte1(v10)] ^ kCastS7[Byte3(v10)] ^
          kCastS6[Byte0(v10)] ^ kCastS5[Byte2(v10)] ^ v13 ^
          kCastS7[Byte3(v11)];
    *v8 = kCastS8[Byte0(*v9)] ^ kCastS7[Byte2(*v9)] ^
          kCastS6[Byte1(*v9)] ^ kCastS5[Byte3(*v9)] ^ v11 ^
          kCastS8[Byte1(v11)];
    *v7 = kCastS8[Byte3(*v8)] ^ kCastS7[Byte2(*v8)] ^
          kCastS6[Byte1(*v8)] ^ kCastS5[Byte0(*v8)] ^ v10 ^
          kCastS5[Byte2(v11)];
    *v6 = kCastS8[Byte3(*v7)] ^ kCastS7[Byte0(*v7)] ^
          kCastS6[Byte2(*v7)] ^ kCastS5[Byte1(*v7)] ^ v12 ^
          kCastS6[Byte0(v11)];
}

void Cast5GenerateBlockB(unsigned int* v10, unsigned int* v11,
                         unsigned int* v12, unsigned int* v13,
                         unsigned int v6, unsigned int v7, unsigned int v8,
                         unsigned int v9) {
    *v13 = kCastS8[Byte1(v8)] ^ kCastS7[Byte3(v8)] ^
           kCastS6[Byte0(v8)] ^ kCastS5[Byte2(v8)] ^ v7 ^
           kCastS7[Byte3(v9)];
    *v12 = kCastS8[Byte0(*v13)] ^ kCastS7[Byte2(*v13)] ^
           kCastS6[Byte1(*v13)] ^ kCastS5[Byte3(*v13)] ^ v9 ^
           kCastS8[Byte1(v9)];
    *v11 = kCastS8[Byte3(*v12)] ^ kCastS7[Byte2(*v12)] ^
           kCastS6[Byte1(*v12)] ^ kCastS5[Byte0(*v12)] ^ v8 ^
           kCastS5[Byte2(v9)];
    *v10 = kCastS8[Byte3(*v11)] ^ kCastS7[Byte0(*v11)] ^
           kCastS6[Byte2(*v11)] ^ kCastS5[Byte1(*v11)] ^ v6 ^
           kCastS6[Byte0(v9)];
}

int Cast5Setup(const unsigned char* key, int key_length, int rounds,
               unsigned int round_keys[33]) {
    if (rounds != 12 && rounds != 16 && rounds != 0) {
        return 4;
    }
    if (rounds == 12 && key_length > 10) {
        return 4;
    }
    if (key_length <= 4 || key_length > 16) {
        return 3;
    }

    unsigned char padded_key[16];
    std::memset(padded_key, 0, sizeof(padded_key));
    std::memcpy(padded_key, key, key_length);

    unsigned int v10 = crypto_detail::ReadBE32(padded_key + 12);
    unsigned int v11 = crypto_detail::ReadBE32(padded_key + 8);
    unsigned int v12 = crypto_detail::ReadBE32(padded_key + 4);
    unsigned int v13 = crypto_detail::ReadBE32(padded_key);
    unsigned int v6 = 0;
    unsigned int v7 = 0;
    unsigned int v8 = 0;
    unsigned int v9 = 0;
    int out = 0;

    for (int pass = 0; pass <= 1; ++pass) {
        Cast5GenerateBlockA(&v6, &v7, &v8, &v9, v10, v11, v12, v13);
        round_keys[out++] = kCastS5[Byte1(v9)] ^ kCastS8[Byte1(v8)] ^
                            kCastS7[Byte0(v8)] ^ kCastS6[Byte2(v7)] ^
                            kCastS5[Byte3(v7)];
        round_keys[out++] = kCastS6[Byte1(v8)] ^ kCastS8[Byte3(v8)] ^
                            kCastS7[Byte2(v8)] ^ kCastS6[Byte0(v7)] ^
                            kCastS5[Byte1(v7)];
        round_keys[out++] = kCastS7[Byte2(v7)] ^ kCastS8[Byte1(v9)] ^
                            kCastS7[Byte0(v9)] ^ kCastS6[Byte2(v6)] ^
                            kCastS5[Byte3(v6)];
        round_keys[out++] = kCastS8[Byte3(v6)] ^ kCastS8[Byte3(v9)] ^
                            kCastS7[Byte2(v9)] ^ kCastS6[Byte0(v6)] ^
                            kCastS5[Byte1(v6)];

        Cast5GenerateBlockB(&v10, &v11, &v12, &v13, v6, v7, v8, v9);
        round_keys[out++] = kCastS5[Byte3(v11)] ^ kCastS8[Byte2(v10)] ^
                            kCastS7[Byte3(v10)] ^ kCastS6[Byte1(v13)] ^
                            kCastS5[Byte0(v13)];
        round_keys[out++] = kCastS6[Byte2(v10)] ^ kCastS8[Byte0(v10)] ^
                            kCastS7[Byte1(v10)] ^ kCastS6[Byte3(v13)] ^
                            kCastS5[Byte2(v13)];
        round_keys[out++] = kCastS7[Byte0(v13)] ^ kCastS8[Byte2(v11)] ^
                            kCastS7[Byte3(v11)] ^ kCastS6[Byte1(v12)] ^
                            kCastS5[Byte0(v12)];
        round_keys[out++] = kCastS8[Byte0(v12)] ^ kCastS8[Byte0(v11)] ^
                            kCastS7[Byte1(v11)] ^ kCastS6[Byte3(v12)] ^
                            kCastS5[Byte2(v12)];

        Cast5GenerateBlockA(&v6, &v7, &v8, &v9, v10, v11, v12, v13);
        round_keys[out++] = kCastS5[Byte2(v7)] ^ kCastS8[Byte2(v6)] ^
                            kCastS7[Byte3(v6)] ^ kCastS6[Byte1(v9)] ^
                            kCastS5[Byte0(v9)];
        round_keys[out++] = kCastS6[Byte3(v6)] ^ kCastS8[Byte0(v6)] ^
                            kCastS7[Byte1(v6)] ^ kCastS6[Byte3(v9)] ^
                            kCastS5[Byte2(v9)];
        round_keys[out++] = kCastS7[Byte1(v9)] ^ kCastS8[Byte2(v7)] ^
                            kCastS7[Byte3(v7)] ^ kCastS6[Byte1(v8)] ^
                            kCastS5[Byte0(v8)];
        round_keys[out++] = kCastS8[Byte1(v8)] ^ kCastS8[Byte0(v7)] ^
                            kCastS7[Byte1(v7)] ^ kCastS6[Byte3(v8)] ^
                            kCastS5[Byte2(v8)];

        Cast5GenerateBlockB(&v10, &v11, &v12, &v13, v6, v7, v8, v9);
        round_keys[out++] = kCastS5[Byte0(v13)] ^ kCastS8[Byte1(v12)] ^
                            kCastS7[Byte0(v12)] ^ kCastS6[Byte2(v11)] ^
                            kCastS5[Byte3(v11)];
        round_keys[out++] = kCastS6[Byte0(v12)] ^ kCastS8[Byte3(v12)] ^
                            kCastS7[Byte2(v12)] ^ kCastS6[Byte0(v11)] ^
                            kCastS5[Byte1(v11)];
        round_keys[out++] = kCastS7[Byte3(v11)] ^ kCastS8[Byte1(v13)] ^
                            kCastS7[Byte0(v13)] ^ kCastS6[Byte2(v10)] ^
                            kCastS5[Byte3(v10)];
        round_keys[out++] = kCastS8[Byte2(v10)] ^ kCastS8[Byte3(v13)] ^
                            kCastS7[Byte2(v13)] ^ kCastS6[Byte0(v10)] ^
                            kCastS5[Byte1(v10)];
    }

    round_keys[kCast5KeyLengthSlot] = key_length;
    std::memset(padded_key, 0, sizeof(padded_key));
    return 0;
}

void Cast5EncryptBlock(const unsigned char* input, unsigned char* output,
                       const unsigned int round_keys[33]) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);
    const int rounds = round_keys[kCast5KeyLengthSlot] > 10 ? 16 : 12;

    for (int round = 0; round < rounds; ++round) {
        const unsigned int next = left ^ CastRound(right, round, round_keys);
        left = right;
        right = next;
    }

    crypto_detail::WriteBE32(output, right);
    crypto_detail::WriteBE32(output + 4, left);
}

void Cast5DecryptBlock(const unsigned char* input, unsigned char* output,
                       const unsigned int round_keys[33]) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);
    const int rounds = round_keys[kCast5KeyLengthSlot] > 10 ? 16 : 12;

    for (int round = rounds - 1; round >= 0; --round) {
        const unsigned int next = left ^ CastRound(right, round, round_keys);
        left = right;
        right = next;
    }

    crypto_detail::WriteBE32(output, right);
    crypto_detail::WriteBE32(output + 4, left);
}

}  // namespace

// IDA_EA: 0x8099F0C
// IDA_NAME: _ZN13CNCryptoCast5C2Ev
// DEMANGLED: CNCryptoCast5::CNCryptoCast5(void)
CNCryptoCast5::CNCryptoCast5()
    : ICryptoGraph(CRYPTO_GRAPH_CAST5), round_keys_(), initialized_(false) {}

// IDA_EA: 0x8099FAA
// IDA_NAME: _ZN13CNCryptoCast5D2Ev
// DEMANGLED: CNCryptoCast5::~CNCryptoCast5()
// IDA_EA: 0x8099FFE
// IDA_NAME: _ZN13CNCryptoCast5D0Ev
// DEMANGLED: CNCryptoCast5::~CNCryptoCast5() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoCast5::~CNCryptoCast5() {}

// IDA_EA: 0x809A01C
// IDA_NAME: _ZN13CNCryptoCast5C2ERKS_
// DEMANGLED: CNCryptoCast5::CNCryptoCast5(CNCryptoCast5 const&)
CNCryptoCast5::CNCryptoCast5(const CNCryptoCast5& rhs)
    : ICryptoGraph(rhs), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809A040
// IDA_NAME: _ZN13CNCryptoCast5aSERKS_
// DEMANGLED: CNCryptoCast5::operator=(CNCryptoCast5 const&)
CNCryptoCast5& CNCryptoCast5::operator=(const CNCryptoCast5& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809A060
// IDA_NAME: _ZN13CNCryptoCast510InitializeEPKhi
// DEMANGLED: CNCryptoCast5::Initialize(unsigned char const*,int)
int CNCryptoCast5::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (GetKeySize() > length) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = Cast5Setup(key, GetKeySize(), 16, round_keys_);
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

// IDA_EA: 0x809A048
// IDA_NAME: _ZN13CNCryptoCast57SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoCast5::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoCast5::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A108
// IDA_NAME: _ZN13CNCryptoCast57EncryptEPhi
// DEMANGLED: CNCryptoCast5::Encrypt(unsigned char *,int)
int CNCryptoCast5::Encrypt(unsigned char* data, int length) {
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
        Cast5EncryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A1E0
// IDA_NAME: _ZN13CNCryptoCast57DecryptEPhi
// DEMANGLED: CNCryptoCast5::Decrypt(unsigned char *,int)
int CNCryptoCast5::Decrypt(unsigned char* data, int length) {
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
        Cast5DecryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A2B8
// IDA_NAME: _ZN13CNCryptoCast57EncryptEPKhiPhi
// DEMANGLED: CNCryptoCast5::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoCast5::Encrypt(const unsigned char* input, int input_length,
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
        Cast5EncryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A3AA
// IDA_NAME: _ZN13CNCryptoCast57DecryptEPKhiPhi
// DEMANGLED: CNCryptoCast5::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoCast5::Decrypt(const unsigned char* input, int input_length,
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
        Cast5DecryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A4A6
// IDA_NAME: _ZNK13CNCryptoCast510GetKeySizeEv
// DEMANGLED: CNCryptoCast5::GetKeySize(void)const
int CNCryptoCast5::GetKeySize() const {
    return kCast5KeySize;
}

// IDA_EA: 0x809A49C
// IDA_NAME: _ZNK13CNCryptoCast512GetBlockSizeEv
// DEMANGLED: CNCryptoCast5::GetBlockSize(void)const
int CNCryptoCast5::GetBlockSize() const {
    return kCast5BlockSize;
}
