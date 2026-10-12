#include "infra/security/crypto/crypto_anubis.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kAnubisKeySize = 16;
const int kAnubisBlockSize = 16;
const int kAnubisRounds = 12;

#include "infra/security/crypto/crypto_anubis_tables.inc"

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

unsigned int TableMix(unsigned int value, unsigned int key_word) {
    return (kAnubisT5[Byte3(value)] & 0xFF000000U) ^
           (kAnubisT5[Byte2(value)] & 0x00FF0000U) ^
           (kAnubisT5[Byte1(value)] & 0x0000FF00U) ^
           (static_cast<unsigned char>(kAnubisT5[Byte0(value)])) ^
           key_word;
}

int AnubisSetup(const unsigned char* key, int key_length, int rounds,
                int* out_rounds, unsigned int encrypt_keys[13][4],
                unsigned int decrypt_keys[13][4]) {
    if ((key_length & 3) != 0 || key_length <= 15 || key_length > 40) {
        return 3;
    }
    const int n_words = key_length / 4;
    const int generated_rounds = n_words + 8;
    if (rounds != 0 && rounds != generated_rounds) {
        return 4;
    }
    if (generated_rounds > kAnubisRounds) {
        return 4;
    }

    *out_rounds = generated_rounds;
    unsigned int state[10];
    unsigned int next_state[10];
    int key_offset = 0;
    for (int index = 0; index < n_words; ++index) {
        state[index] = (static_cast<unsigned int>(key[key_offset]) << 24) ^
                       (static_cast<unsigned int>(key[key_offset + 1]) << 16) ^
                       (static_cast<unsigned int>(key[key_offset + 2]) << 8) ^
                       key[key_offset + 3];
        key_offset += 4;
    }

    for (int round = 0; round <= generated_rounds; ++round) {
        unsigned int k0 = kAnubisT4[Byte3(state[n_words - 1])];
        unsigned int k1 = kAnubisT4[Byte2(state[n_words - 1])];
        unsigned int k2 = kAnubisT4[Byte1(state[n_words - 1])];
        unsigned int k3 = kAnubisT4[Byte0(state[n_words - 1])];
        for (int index = n_words - 2; index >= 0; --index) {
            k0 = TableMix(k0, kAnubisT4[Byte3(state[index])]);
            k1 = TableMix(k1, kAnubisT4[Byte2(state[index])]);
            k2 = TableMix(k2, kAnubisT4[Byte1(state[index])]);
            k3 = TableMix(k3, kAnubisT4[Byte0(state[index])]);
        }
        encrypt_keys[round][0] = k0;
        encrypt_keys[round][1] = k1;
        encrypt_keys[round][2] = k2;
        encrypt_keys[round][3] = k3;
        if (round == generated_rounds) {
            break;
        }

        for (int index = 0; index < n_words; ++index) {
            int cursor = index;
            next_state[index] = kAnubisT0[Byte3(state[index])];
            if (--cursor < 0) {
                cursor = n_words - 1;
            }
            next_state[index] ^= kAnubisT1[Byte2(state[cursor])];
            if (--cursor < 0) {
                cursor = n_words - 1;
            }
            next_state[index] ^= kAnubisT2[Byte1(state[cursor])];
            if (--cursor < 0) {
                cursor = n_words - 1;
            }
            next_state[index] ^= kAnubisT3[Byte0(state[cursor])];
        }
        state[0] = next_state[0] ^ kAnubisRc[round];
        for (int index = 1; index < n_words; ++index) {
            state[index] = next_state[index];
        }
    }

    for (int word = 0; word <= 3; ++word) {
        decrypt_keys[0][word] = encrypt_keys[generated_rounds][word];
        decrypt_keys[generated_rounds][word] = encrypt_keys[0][word];
    }
    for (int round = 1; round < generated_rounds; ++round) {
        for (int word = 0; word <= 3; ++word) {
            const unsigned int value = encrypt_keys[generated_rounds - round][word];
            decrypt_keys[round][word] =
                kAnubisT3[static_cast<unsigned char>(
                    kAnubisT4[Byte0(value)])] ^
                kAnubisT2[static_cast<unsigned char>(
                    kAnubisT4[Byte1(value)])] ^
                kAnubisT1[static_cast<unsigned char>(
                    kAnubisT4[Byte2(value)])] ^
                kAnubisT0[static_cast<unsigned char>(
                    kAnubisT4[Byte3(value)])];
        }
    }
    return 0;
}

void AnubisCrypt(const unsigned char* input, unsigned char* output,
                 const unsigned int round_keys[13][4], int rounds) {
    unsigned int s0 = crypto_detail::ReadBE32(input) ^ round_keys[0][0];
    unsigned int s1 = crypto_detail::ReadBE32(input + 4) ^ round_keys[0][1];
    unsigned int s2 = crypto_detail::ReadBE32(input + 8) ^ round_keys[0][2];
    unsigned int s3 = crypto_detail::ReadBE32(input + 12) ^ round_keys[0][3];

    for (int round = 1; round < rounds; ++round) {
        const unsigned int t0 = kAnubisT3[Byte3(s3)] ^
                                kAnubisT2[Byte3(s2)] ^
                                kAnubisT1[Byte3(s1)] ^
                                kAnubisT0[Byte3(s0)] ^
                                round_keys[round][0];
        const unsigned int t1 = kAnubisT3[Byte2(s3)] ^
                                kAnubisT2[Byte2(s2)] ^
                                kAnubisT1[Byte2(s1)] ^
                                kAnubisT0[Byte2(s0)] ^
                                round_keys[round][1];
        const unsigned int t2 = kAnubisT3[Byte1(s3)] ^
                                kAnubisT2[Byte1(s2)] ^
                                kAnubisT1[Byte1(s1)] ^
                                kAnubisT0[Byte1(s0)] ^
                                round_keys[round][2];
        const unsigned int t3 = kAnubisT3[Byte0(s3)] ^
                                kAnubisT2[Byte0(s2)] ^
                                kAnubisT1[Byte0(s1)] ^
                                kAnubisT0[Byte0(s0)] ^
                                round_keys[round][3];
        s0 = t0;
        s1 = t1;
        s2 = t2;
        s3 = t3;
    }

    const unsigned int o0 =
        static_cast<unsigned char>(kAnubisT3[Byte3(s3)]) ^
        (kAnubisT2[Byte3(s2)] & 0x0000FF00U) ^
        (kAnubisT1[Byte3(s1)] & 0x00FF0000U) ^
        (kAnubisT0[Byte3(s0)] & 0xFF000000U) ^
        round_keys[rounds][0];
    const unsigned int o1 =
        static_cast<unsigned char>(kAnubisT3[Byte2(s3)]) ^
        (kAnubisT2[Byte2(s2)] & 0x0000FF00U) ^
        (kAnubisT1[Byte2(s1)] & 0x00FF0000U) ^
        (kAnubisT0[Byte2(s0)] & 0xFF000000U) ^
        round_keys[rounds][1];
    const unsigned int o2 =
        static_cast<unsigned char>(kAnubisT3[Byte1(s3)]) ^
        (kAnubisT2[Byte1(s2)] & 0x0000FF00U) ^
        (kAnubisT1[Byte1(s1)] & 0x00FF0000U) ^
        (kAnubisT0[Byte1(s0)] & 0xFF000000U) ^
        round_keys[rounds][2];
    const unsigned int o3 =
        static_cast<unsigned char>(kAnubisT3[Byte0(s3)]) ^
        (kAnubisT2[Byte0(s2)] & 0x0000FF00U) ^
        (kAnubisT1[Byte0(s1)] & 0x00FF0000U) ^
        (kAnubisT0[Byte0(s0)] & 0xFF000000U) ^
        round_keys[rounds][3];

    crypto_detail::WriteBE32(output, o0);
    crypto_detail::WriteBE32(output + 4, o1);
    crypto_detail::WriteBE32(output + 8, o2);
    crypto_detail::WriteBE32(output + 12, o3);
}

int TransformAnubis(const unsigned int encrypt_keys[13][4],
                    const unsigned int decrypt_keys[13][4], int rounds,
                    bool initialized, const unsigned char* input,
                    int input_length, unsigned char* output,
                    int output_capacity, bool decrypt, int block_error,
                    bool reject_empty) {
    if (!initialized) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (reject_empty && input_length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if ((input_length % kAnubisBlockSize) != 0) {
        return block_error;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    const unsigned int (*keys)[4] = decrypt ? decrypt_keys : encrypt_keys;
    for (int offset = 0; offset < input_length; offset += kAnubisBlockSize) {
        AnubisCrypt(input + offset, output + offset, keys, rounds);
    }
    return CRYPTO_RESULT_OK;
}

}  // namespace

// IDA_EA: 0x8099264
// IDA_NAME: _ZN14CNCryptoAnubisC2Ev
// DEMANGLED: CNCryptoAnubis::CNCryptoAnubis(void)
CNCryptoAnubis::CNCryptoAnubis()
    : ICryptoGraph(CRYPTO_GRAPH_ANUBIS),
      rounds_(0),
      encrypt_keys_(),
      decrypt_keys_(),
      initialized_(false) {}

// IDA_EA: 0x8099302
// IDA_NAME: _ZN14CNCryptoAnubisD2Ev
// DEMANGLED: CNCryptoAnubis::~CNCryptoAnubis()
// IDA_EA: 0x809935A
// IDA_NAME: _ZN14CNCryptoAnubisD0Ev
// DEMANGLED: CNCryptoAnubis::~CNCryptoAnubis() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoAnubis::~CNCryptoAnubis() {}

// IDA_EA: 0x8099378
// IDA_NAME: _ZN14CNCryptoAnubisC2ERKS_
// DEMANGLED: CNCryptoAnubis::CNCryptoAnubis(CNCryptoAnubis const&)
CNCryptoAnubis::CNCryptoAnubis(const CNCryptoAnubis& rhs)
    : ICryptoGraph(rhs),
      rounds_(0),
      encrypt_keys_(),
      decrypt_keys_(),
      initialized_(false) {}

// IDA_EA: 0x809939C
// IDA_NAME: _ZN14CNCryptoAnubisaSERKS_
// DEMANGLED: CNCryptoAnubis::operator=(CNCryptoAnubis const&)
CNCryptoAnubis& CNCryptoAnubis::operator=(const CNCryptoAnubis& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x80993A4
// IDA_NAME: _ZN14CNCryptoAnubis7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoAnubis::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoAnubis::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x80993BC
// IDA_NAME: _ZN14CNCryptoAnubis10InitializeEPKhi
// DEMANGLED: CNCryptoAnubis::Initialize(unsigned char const*,int)
int CNCryptoAnubis::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length < GetKeySize()) {
        initialized_ = false;
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    const int rc = AnubisSetup(key, GetKeySize(), kAnubisRounds, &rounds_,
                               encrypt_keys_, decrypt_keys_);
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

// IDA_EA: 0x8099464
// IDA_NAME: _ZN14CNCryptoAnubis7EncryptEPhi
// DEMANGLED: CNCryptoAnubis::Encrypt(unsigned char *,int)
int CNCryptoAnubis::Encrypt(unsigned char* data, int length) {
    return TransformAnubis(encrypt_keys_, decrypt_keys_, rounds_, initialized_,
                           data, length, data, length, false,
                           CRYPTO_RESULT_INVALID_BLOCK_SIZE, true);
}

// IDA_EA: 0x809953C
// IDA_NAME: _ZN14CNCryptoAnubis7DecryptEPhi
// DEMANGLED: CNCryptoAnubis::Decrypt(unsigned char *,int)
int CNCryptoAnubis::Decrypt(unsigned char* data, int length) {
    return TransformAnubis(encrypt_keys_, decrypt_keys_, rounds_, initialized_,
                           data, length, data, length, true,
                           CRYPTO_RESULT_INVALID_BLOCK_SIZE, true);
}

// IDA_EA: 0x8099614
// IDA_NAME: _ZN14CNCryptoAnubis7EncryptEPKhiPhi
// DEMANGLED: CNCryptoAnubis::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoAnubis::Encrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return TransformAnubis(encrypt_keys_, decrypt_keys_, rounds_, initialized_,
                           input, input_length, output, output_capacity, false,
                           CRYPTO_RESULT_INVALID_BLOCK_SIZE, false);
}

// IDA_EA: 0x8099706
// IDA_NAME: _ZN14CNCryptoAnubis7DecryptEPKhiPhi
// DEMANGLED: CNCryptoAnubis::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoAnubis::Decrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return TransformAnubis(encrypt_keys_, decrypt_keys_, rounds_, initialized_,
                           input, input_length, output, output_capacity, true,
                           CRYPTO_RESULT_BUFFER_TOO_SMALL, false);
}

// IDA_EA: 0x80997F8
// IDA_NAME: _ZNK14CNCryptoAnubis10GetKeySizeEv
// DEMANGLED: CNCryptoAnubis::GetKeySize(void)const
int CNCryptoAnubis::GetKeySize() const {
    return kAnubisKeySize;
}

// IDA_EA: 0x8099802
// IDA_NAME: _ZNK14CNCryptoAnubis12GetBlockSizeEv
// DEMANGLED: CNCryptoAnubis::GetBlockSize(void)const
int CNCryptoAnubis::GetBlockSize() const {
    return kAnubisBlockSize;
}
