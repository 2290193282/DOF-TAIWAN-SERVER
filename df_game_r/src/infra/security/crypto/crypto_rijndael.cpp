#include "infra/security/crypto/crypto_rijndael.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"

namespace {

const int kRijndaelKeySize = 16;
const int kRijndaelBlockSize = 16;
const int kRijndaelRounds = 10;
const int kOldRijndaelSuccess = 0x6FFFFFFF;
const int kOldRijndaelNullKey = 0x70000001;
const int kOldRijndaelInvalidKeyLength = 0x70000002;
const int kOldRijndaelInvalidMode = 0x70000003;
const int kOldRijndaelInvalidPadding = 0x70000004;
const int kOldRijndaelNotInitialized = 0x70000005;
const int kOldRijndaelInvalidBlockSize = 0x70000006;
const int kOldRijndaelInvalidKeySize = 0x70000009;
const int kOldRijndaelInvalidDataLength = 0x7000000A;

#include "infra/security/crypto/crypto_rijndael_tables.inc"

const unsigned char kOldRijndaelShiftTable[3][8] = {
    {0, 0, 1, 3, 2, 2, 3, 1},
    {0, 0, 1, 5, 2, 4, 3, 3},
    {0, 0, 1, 7, 3, 5, 4, 4},
};

unsigned char GfMul(unsigned char left, unsigned char right) {
    unsigned char result = 0;
    for (int bit = 0; bit < 8; ++bit) {
        if (right & 1) {
            result ^= left;
        }
        const bool high = (left & 0x80) != 0;
        left = static_cast<unsigned char>(left << 1);
        if (high) {
            left ^= 0x1B;
        }
        right >>= 1;
    }
    return result;
}

void XorBlock(unsigned char* target, const unsigned char* source) {
    for (int index = 0; index < kRijndaelBlockSize; ++index) {
        target[index] ^= source[index];
    }
}

void AddRoundKey(unsigned char state[16], const unsigned char key[16]) {
    XorBlock(state, key);
}

void SubBytes(unsigned char state[16]) {
    for (int index = 0; index < kRijndaelBlockSize; ++index) {
        state[index] = kRijndaelSBox[state[index]];
    }
}

void InvSubBytes(unsigned char state[16]) {
    for (int index = 0; index < kRijndaelBlockSize; ++index) {
        state[index] = kRijndaelInvSBox[state[index]];
    }
}

void ShiftRows(unsigned char state[16]) {
    unsigned char temp[16];
    temp[0] = state[0];
    temp[1] = state[5];
    temp[2] = state[10];
    temp[3] = state[15];
    temp[4] = state[4];
    temp[5] = state[9];
    temp[6] = state[14];
    temp[7] = state[3];
    temp[8] = state[8];
    temp[9] = state[13];
    temp[10] = state[2];
    temp[11] = state[7];
    temp[12] = state[12];
    temp[13] = state[1];
    temp[14] = state[6];
    temp[15] = state[11];
    std::memcpy(state, temp, sizeof(temp));
}

void InvShiftRows(unsigned char state[16]) {
    unsigned char temp[16];
    temp[0] = state[0];
    temp[1] = state[13];
    temp[2] = state[10];
    temp[3] = state[7];
    temp[4] = state[4];
    temp[5] = state[1];
    temp[6] = state[14];
    temp[7] = state[11];
    temp[8] = state[8];
    temp[9] = state[5];
    temp[10] = state[2];
    temp[11] = state[15];
    temp[12] = state[12];
    temp[13] = state[9];
    temp[14] = state[6];
    temp[15] = state[3];
    std::memcpy(state, temp, sizeof(temp));
}

void MixColumns(unsigned char state[16]) {
    for (int column = 0; column < 4; ++column) {
        unsigned char* c = state + column * 4;
        const unsigned char a0 = c[0];
        const unsigned char a1 = c[1];
        const unsigned char a2 = c[2];
        const unsigned char a3 = c[3];
        c[0] = GfMul(a0, 2) ^ GfMul(a1, 3) ^ a2 ^ a3;
        c[1] = a0 ^ GfMul(a1, 2) ^ GfMul(a2, 3) ^ a3;
        c[2] = a0 ^ a1 ^ GfMul(a2, 2) ^ GfMul(a3, 3);
        c[3] = GfMul(a0, 3) ^ a1 ^ a2 ^ GfMul(a3, 2);
    }
}

void InvMixColumns(unsigned char state[16]) {
    for (int column = 0; column < 4; ++column) {
        unsigned char* c = state + column * 4;
        const unsigned char a0 = c[0];
        const unsigned char a1 = c[1];
        const unsigned char a2 = c[2];
        const unsigned char a3 = c[3];
        c[0] = GfMul(a0, 14) ^ GfMul(a1, 11) ^ GfMul(a2, 13) ^ GfMul(a3, 9);
        c[1] = GfMul(a0, 9) ^ GfMul(a1, 14) ^ GfMul(a2, 11) ^ GfMul(a3, 13);
        c[2] = GfMul(a0, 13) ^ GfMul(a1, 9) ^ GfMul(a2, 14) ^ GfMul(a3, 11);
        c[3] = GfMul(a0, 11) ^ GfMul(a1, 13) ^ GfMul(a2, 9) ^ GfMul(a3, 14);
    }
}

void ExpandKey(const unsigned char key[16], unsigned char round_keys[11][16],
               unsigned char decrypt_round_keys[11][16]) {
    unsigned char expanded[176];
    std::memcpy(expanded, key, kRijndaelKeySize);

    int bytes_generated = kRijndaelKeySize;
    int rcon_index = 0;
    unsigned char temp[4];
    while (bytes_generated < static_cast<int>(sizeof(expanded))) {
        std::memcpy(temp, expanded + bytes_generated - 4, sizeof(temp));
        if ((bytes_generated % kRijndaelKeySize) == 0) {
            const unsigned char rotated = temp[0];
            temp[0] = kRijndaelSBox[temp[1]];
            temp[1] = kRijndaelSBox[temp[2]];
            temp[2] = kRijndaelSBox[temp[3]];
            temp[3] = kRijndaelSBox[rotated];
            temp[0] ^= kRijndaelRcon[rcon_index++];
        }
        for (int index = 0; index < 4; ++index) {
            expanded[bytes_generated] =
                expanded[bytes_generated - kRijndaelKeySize] ^ temp[index];
            ++bytes_generated;
        }
    }

    for (int round = 0; round <= kRijndaelRounds; ++round) {
        std::memcpy(round_keys[round], expanded + round * kRijndaelBlockSize,
                    kRijndaelBlockSize);
        std::memcpy(decrypt_round_keys[round],
                    expanded + (kRijndaelRounds - round) * kRijndaelBlockSize,
                    kRijndaelBlockSize);
    }
}

void RijndaelEncryptBlock(const unsigned char round_keys[11][16],
                          const unsigned char* input, unsigned char* output) {
    unsigned char state[16];
    std::memcpy(state, input, sizeof(state));
    AddRoundKey(state, round_keys[0]);
    for (int round = 1; round < kRijndaelRounds; ++round) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, round_keys[round]);
    }
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, round_keys[kRijndaelRounds]);
    std::memcpy(output, state, sizeof(state));
}

void RijndaelDecryptBlock(const unsigned char round_keys[11][16],
                          const unsigned char* input, unsigned char* output) {
    unsigned char state[16];
    std::memcpy(state, input, sizeof(state));
    AddRoundKey(state, round_keys[kRijndaelRounds]);
    for (int round = kRijndaelRounds - 1; round > 0; --round) {
        InvShiftRows(state);
        InvSubBytes(state);
        AddRoundKey(state, round_keys[round]);
        InvMixColumns(state);
    }
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state, round_keys[0]);
    std::memcpy(output, state, sizeof(state));
}

int TransformRijndael(const unsigned char round_keys[11][16],
                      unsigned char chain[16], CRYPTO_MODE_TYPE mode,
                      const unsigned char* input, int input_length,
                      unsigned char* output, int output_capacity,
                      bool decrypt) {
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }
    if ((input_length % kRijndaelBlockSize) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (input_length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    unsigned char temp[16];
    for (int offset = 0; offset < input_length; offset += kRijndaelBlockSize) {
        const unsigned char* read = input + offset;
        unsigned char* write = output + offset;
        if (!decrypt && mode == CRYPTO_MODE_CBC) {
            std::memcpy(temp, read, kRijndaelBlockSize);
            XorBlock(temp, chain);
            RijndaelEncryptBlock(round_keys, temp, write);
            std::memcpy(chain, write, kRijndaelBlockSize);
        } else if (decrypt && mode == CRYPTO_MODE_CBC) {
            RijndaelDecryptBlock(round_keys, read, write);
            XorBlock(write, chain);
            std::memcpy(chain, read, kRijndaelBlockSize);
        } else if (mode == CRYPTO_MODE_OFB) {
            RijndaelEncryptBlock(round_keys, chain, temp);
            std::memcpy(chain, temp, kRijndaelBlockSize);
            std::memcpy(write, read, kRijndaelBlockSize);
            XorBlock(write, chain);
        } else {
            if (decrypt) {
                RijndaelDecryptBlock(round_keys, read, write);
            } else {
                RijndaelEncryptBlock(round_keys, read, write);
            }
        }
    }
    return CRYPTO_RESULT_OK;
}

}  // namespace


// IDA_EA: 0x809BF08
// IDA_NAME: _ZN16CNCryptoRijndaelC2Ev
// DEMANGLED: CNCryptoRijndael::CNCryptoRijndael(void)
CNCryptoRijndael::CNCryptoRijndael()
    : ICryptoGraph(CRYPTO_GRAPH_RIJNDAEL),
      round_keys_(),
      decrypt_round_keys_(),
      initial_chain_(),
      chain_(),
      initialized_(false) {}

// IDA_EA: 0x809C03E
// IDA_NAME: _ZN16CNCryptoRijndaelD2Ev
// DEMANGLED: CNCryptoRijndael::~CNCryptoRijndael()
// IDA_EA: 0x809C0C8
// IDA_NAME: _ZN16CNCryptoRijndaelD0Ev
// DEMANGLED: CNCryptoRijndael::~CNCryptoRijndael() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoRijndael::~CNCryptoRijndael() {}

// IDA_EA: 0x809C0E6
// IDA_NAME: _ZN16CNCryptoRijndaelC2ERKS_
// DEMANGLED: CNCryptoRijndael::CNCryptoRijndael(CNCryptoRijndael const&)
CNCryptoRijndael::CNCryptoRijndael(const CNCryptoRijndael& rhs)
    : ICryptoGraph(rhs),
      round_keys_(),
      decrypt_round_keys_(),
      initial_chain_(),
      chain_(),
      initialized_(false) {}

// IDA_EA: 0x809C10A
// IDA_NAME: _ZN16CNCryptoRijndaelaSERKS_
// DEMANGLED: CNCryptoRijndael::operator=(CNCryptoRijndael const&)
CNCryptoRijndael& CNCryptoRijndael::operator=(const CNCryptoRijndael& rhs) {
    (void)rhs;
    return *this;
}


namespace {

bool IsOldRijndaelSize(int size) {
    return size == 16 || size == 24 || size == 32;
}

int OldRijndaelRounds(int key_size, int block_size) {
    const int key_words = key_size / 4;
    const int block_words = block_size / 4;
    return (key_words > block_words ? key_words : block_words) + 6;
}

unsigned int ReadBEWord(const unsigned char* data) {
    return (static_cast<unsigned int>(data[0]) << 24) |
           (static_cast<unsigned int>(data[1]) << 16) |
           (static_cast<unsigned int>(data[2]) << 8) |
           static_cast<unsigned int>(data[3]);
}

void WriteBEWord(unsigned char* data, unsigned int value) {
    data[0] = static_cast<unsigned char>(value >> 24);
    data[1] = static_cast<unsigned char>(value >> 16);
    data[2] = static_cast<unsigned char>(value >> 8);
    data[3] = static_cast<unsigned char>(value);
}

unsigned int RotWord(unsigned int word) {
    return (word << 8) | (word >> 24);
}

unsigned int SubWord(unsigned int word) {
    return (static_cast<unsigned int>(kRijndaelSBox[(word >> 24) & 0xFF]) << 24) |
           (static_cast<unsigned int>(kRijndaelSBox[(word >> 16) & 0xFF]) << 16) |
           (static_cast<unsigned int>(kRijndaelSBox[(word >> 8) & 0xFF]) << 8) |
           static_cast<unsigned int>(kRijndaelSBox[word & 0xFF]);
}

void ExpandOldRijndaelKey(const unsigned char* key, int key_size,
                          int block_size, int rounds,
                          unsigned char* round_keys,
                          unsigned char* decrypt_round_keys) {
    const int key_words = key_size / 4;
    const int block_words = block_size / 4;
    const int total_words = block_words * (rounds + 1);
    unsigned int words[120];

    for (int index = 0; index < key_words; ++index) {
        words[index] = ReadBEWord(key + index * 4);
    }
    for (int index = key_words; index < total_words; ++index) {
        unsigned int temp = words[index - 1];
        if ((index % key_words) == 0) {
            temp = SubWord(RotWord(temp)) ^
                   (static_cast<unsigned int>(
                        kRijndaelRcon[index / key_words - 1])
                    << 24);
        } else if (key_words == 8 && (index % key_words) == 4) {
            temp = SubWord(temp);
        }
        words[index] = words[index - key_words] ^ temp;
    }

    for (int round = 0; round <= rounds; ++round) {
        for (int column = 0; column < block_words; ++column) {
            WriteBEWord(round_keys + round * block_size + column * 4,
                        words[round * block_words + column]);
            WriteBEWord(decrypt_round_keys +
                            (rounds - round) * block_size + column * 4,
                        words[round * block_words + column]);
        }
    }
}

void AddRoundKeyGeneric(unsigned char* state, const unsigned char* round_key,
                        int block_size) {
    for (int index = 0; index < block_size; ++index) {
        state[index] ^= round_key[index];
    }
}

void ShiftRowsGeneric(unsigned char* state, int block_words,
                      const unsigned char shifts[4]) {
    unsigned char temp[32];
    for (int column = 0; column < block_words; ++column) {
        temp[column * 4] = state[column * 4];
    }
    for (int row = 1; row < 4; ++row) {
        for (int column = 0; column < block_words; ++column) {
            const int source_column = (column + shifts[row]) % block_words;
            temp[column * 4 + row] = state[source_column * 4 + row];
        }
    }
    std::memcpy(state, temp, static_cast<unsigned int>(block_words * 4));
}

void SubBytesGeneric(unsigned char* state, int block_size) {
    for (int index = 0; index < block_size; ++index) {
        state[index] = kRijndaelSBox[state[index]];
    }
}

void InvSubBytesGeneric(unsigned char* state, int block_size) {
    for (int index = 0; index < block_size; ++index) {
        state[index] = kRijndaelInvSBox[state[index]];
    }
}

void MixColumnsGeneric(unsigned char* state, int block_words) {
    for (int column = 0; column < block_words; ++column) {
        unsigned char* c = state + column * 4;
        const unsigned char a0 = c[0];
        const unsigned char a1 = c[1];
        const unsigned char a2 = c[2];
        const unsigned char a3 = c[3];
        c[0] = GfMul(a0, 2) ^ GfMul(a1, 3) ^ a2 ^ a3;
        c[1] = a0 ^ GfMul(a1, 2) ^ GfMul(a2, 3) ^ a3;
        c[2] = a0 ^ a1 ^ GfMul(a2, 2) ^ GfMul(a3, 3);
        c[3] = GfMul(a0, 3) ^ a1 ^ a2 ^ GfMul(a3, 2);
    }
}

void InvMixColumnsGeneric(unsigned char* state, int block_words) {
    for (int column = 0; column < block_words; ++column) {
        unsigned char* c = state + column * 4;
        const unsigned char a0 = c[0];
        const unsigned char a1 = c[1];
        const unsigned char a2 = c[2];
        const unsigned char a3 = c[3];
        c[0] = GfMul(a0, 14) ^ GfMul(a1, 11) ^ GfMul(a2, 13) ^ GfMul(a3, 9);
        c[1] = GfMul(a0, 9) ^ GfMul(a1, 14) ^ GfMul(a2, 11) ^ GfMul(a3, 13);
        c[2] = GfMul(a0, 13) ^ GfMul(a1, 9) ^ GfMul(a2, 14) ^ GfMul(a3, 11);
        c[3] = GfMul(a0, 11) ^ GfMul(a1, 13) ^ GfMul(a2, 9);
        c[3] ^= GfMul(a3, 14);
    }
}

void OldRijndaelEncryptBlockGeneric(const unsigned char* round_keys,
                                    int block_size, int rounds,
                                    const char* input, char* output) {
    const int block_words = block_size / 4;
    const int shift_index = block_words == 4 ? 0 : (block_words == 6 ? 1 : 2);
    const unsigned char shifts[4] = {
        0,
        kOldRijndaelShiftTable[shift_index][2],
        kOldRijndaelShiftTable[shift_index][4],
        kOldRijndaelShiftTable[shift_index][6],
    };
    unsigned char state[32];
    std::memcpy(state, input, static_cast<unsigned int>(block_size));
    AddRoundKeyGeneric(state, round_keys, block_size);
    for (int round = 1; round < rounds; ++round) {
        SubBytesGeneric(state, block_size);
        ShiftRowsGeneric(state, block_words, shifts);
        MixColumnsGeneric(state, block_words);
        AddRoundKeyGeneric(state, round_keys + round * block_size, block_size);
    }
    SubBytesGeneric(state, block_size);
    ShiftRowsGeneric(state, block_words, shifts);
    AddRoundKeyGeneric(state, round_keys + rounds * block_size, block_size);
    std::memcpy(output, state, static_cast<unsigned int>(block_size));
}

void OldRijndaelDecryptBlockGeneric(const unsigned char* round_keys,
                                    int block_size, int rounds,
                                    const char* input, char* output) {
    const int block_words = block_size / 4;
    const int shift_index = block_words == 4 ? 0 : (block_words == 6 ? 1 : 2);
    const unsigned char shifts[4] = {
        0,
        kOldRijndaelShiftTable[shift_index][3],
        kOldRijndaelShiftTable[shift_index][5],
        kOldRijndaelShiftTable[shift_index][7],
    };
    unsigned char state[32];
    std::memcpy(state, input, static_cast<unsigned int>(block_size));
    AddRoundKeyGeneric(state, round_keys + rounds * block_size, block_size);
    for (int round = rounds - 1; round > 0; --round) {
        ShiftRowsGeneric(state, block_words, shifts);
        InvSubBytesGeneric(state, block_size);
        AddRoundKeyGeneric(state, round_keys + round * block_size, block_size);
        InvMixColumnsGeneric(state, block_words);
    }
    ShiftRowsGeneric(state, block_words, shifts);
    InvSubBytesGeneric(state, block_size);
    AddRoundKeyGeneric(state, round_keys, block_size);
    std::memcpy(output, state, static_cast<unsigned int>(block_size));
}

}  // namespace

// IDA: CRijndael::sm_chain0 @0x08B22F20.
const char CRijndael::sm_chain0[16] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// IDA_EA: 0x80B60AC
// IDA_NAME: _ZN9CRijndaelC2Ev
// DEMANGLED: CRijndael::CRijndael(void)
CRijndael::CRijndael()
    : vptr_(0x08B22F68u),
      initialized_(0),
      reserved_(),
      block_size_(0),
      key_size_(0),
      mode_(0),
      padding_(0),
      key_bytes_(),
      round_keys_(),
      decrypt_round_keys_(),
      rounds_(0),
      initial_chain_(),
      chain_(),
      key_words_(),
      work_(),
      work2_() {}

// IDA_EA: 0x80B61D2
// IDA_NAME: _ZN9CRijndaelD2Ev
// DEMANGLED: CRijndael::~CRijndael()
// IDA_EA: 0x80B6202
// IDA_NAME: _ZN9CRijndaelD0Ev
// DEMANGLED: CRijndael::~CRijndael() deleting destructor
// NOTE: The deleting destructor wraps this D2 body in the old binary.
CRijndael::~CRijndael() {
    vptr_ = 0x08B22F68u;
}

// IDA_EA: 0x80B6220
// IDA_NAME: _ZN9CRijndael10InitializeEPKciS1_iiii
// DEMANGLED: CRijndael::Initialize(char const*,int,char const*,int,int,int,int)
int CRijndael::Initialize(const char* key, int key_length, const char* chain,
                          int key_size, int block_size, int mode,
                          int padding) {
    if (!key) {
        return kOldRijndaelNullKey;
    }
    if (key_length <= 0) {
        return kOldRijndaelInvalidKeyLength;
    }
    if (!IsOldRijndaelSize(key_size)) {
        return kOldRijndaelInvalidKeySize;
    }
    if (!IsOldRijndaelSize(block_size)) {
        return kOldRijndaelInvalidBlockSize;
    }
    if (mode > 2) {
        return kOldRijndaelInvalidMode;
    }
    if (padding > 2) {
        return kOldRijndaelInvalidPadding;
    }

    mode_ = static_cast<unsigned int>(mode);
    padding_ = static_cast<unsigned int>(padding);
    key_size_ = static_cast<unsigned int>(key_size);
    block_size_ = static_cast<unsigned int>(block_size);
    rounds_ = static_cast<unsigned int>(OldRijndaelRounds(key_size, block_size));

    for (int index = 0; index < key_size; ++index) {
        key_bytes_[index] = static_cast<unsigned char>(key[index % key_length]);
    }
    std::memset(key_bytes_ + key_size, 0,
                static_cast<unsigned int>(sizeof(key_bytes_) - key_size));

    if (chain) {
        std::memcpy(initial_chain_, chain, static_cast<unsigned int>(block_size));
    } else {
        std::memset(initial_chain_, 0, static_cast<unsigned int>(block_size));
    }
    std::memset(initial_chain_ + block_size, 0,
                static_cast<unsigned int>(sizeof(initial_chain_) - block_size));
    std::memcpy(chain_, initial_chain_, static_cast<unsigned int>(block_size));
    std::memset(chain_ + block_size, 0,
                static_cast<unsigned int>(sizeof(chain_) - block_size));

    ExpandOldRijndaelKey(key_bytes_, key_size, block_size,
                         static_cast<int>(rounds_), round_keys_,
                         decrypt_round_keys_);
    initialized_ = 1;
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B6A54
// IDA_NAME: _ZN9CRijndael10ResetChainEv
// DEMANGLED: CRijndael::ResetChain(void)
int CRijndael::ResetChain() {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    std::memcpy(chain_, initial_chain_, block_size_);
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B6BA0
// IDA_NAME: _ZN9CRijndael15DefEncryptBlockEPKcPc
// DEMANGLED: CRijndael::DefEncryptBlock(char const*,char *)
int CRijndael::DefEncryptBlock(const char* input, char* output) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    OldRijndaelEncryptBlockGeneric(round_keys_, 16, static_cast<int>(rounds_),
                                   input, output);
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B70E8
// IDA_NAME: _ZN9CRijndael15DefDecryptBlockEPKcPc
// DEMANGLED: CRijndael::DefDecryptBlock(char const*,char *)
int CRijndael::DefDecryptBlock(const char* input, char* output) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    OldRijndaelDecryptBlockGeneric(round_keys_, 16, static_cast<int>(rounds_),
                                   input, output);
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B7636
// IDA_NAME: _ZN9CRijndael12EncryptBlockEPKcPc
// DEMANGLED: CRijndael::EncryptBlock(char const*,char *)
int CRijndael::EncryptBlock(const char* input, char* output) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    if (block_size_ == 16) {
        DefEncryptBlock(input, output);
    } else {
        OldRijndaelEncryptBlockGeneric(round_keys_, static_cast<int>(block_size_),
                                       static_cast<int>(rounds_), input, output);
    }
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B7A20
// IDA_NAME: _ZN9CRijndael12DecryptBlockEPKcPc
// DEMANGLED: CRijndael::DecryptBlock(char const*,char *)
int CRijndael::DecryptBlock(const char* input, char* output) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    if (block_size_ == 16) {
        DefDecryptBlock(input, output);
    } else {
        OldRijndaelDecryptBlockGeneric(round_keys_, static_cast<int>(block_size_),
                                       static_cast<int>(rounds_), input, output);
    }
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B7E00
// IDA_NAME: _ZN9CRijndael7EncryptEPKcPcj
// DEMANGLED: CRijndael::Encrypt(char const*,char *,unsigned int)
int CRijndael::Encrypt(const char* input, char* output, unsigned int length) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    if (!length || length % block_size_) {
        return kOldRijndaelInvalidDataLength;
    }

    for (unsigned int offset = 0; offset < length; offset += block_size_) {
        const char* read = input + offset;
        char* write = output + offset;
        if (mode_ == 1) {
            for (unsigned int index = 0; index < block_size_; ++index) {
                chain_[index] ^= static_cast<unsigned char>(read[index]);
            }
            EncryptBlock(reinterpret_cast<const char*>(chain_), write);
            std::memcpy(chain_, write, block_size_);
        } else if (mode_ == 2) {
            EncryptBlock(reinterpret_cast<const char*>(chain_), write);
            for (unsigned int index = 0; index < block_size_; ++index) {
                write[index] ^= read[index];
            }
            std::memcpy(chain_, write, block_size_);
        } else {
            EncryptBlock(read, write);
        }
    }
    return kOldRijndaelSuccess;
}

// IDA_EA: 0x80B8028
// IDA_NAME: _ZN9CRijndael7DecryptEPKcPcj
// DEMANGLED: CRijndael::Decrypt(char const*,char *,unsigned int)
int CRijndael::Decrypt(const char* input, char* output, unsigned int length) {
    if (initialized_ != 1) {
        return kOldRijndaelNotInitialized;
    }
    if (!length || length % block_size_) {
        return kOldRijndaelInvalidDataLength;
    }

    for (unsigned int offset = 0; offset < length; offset += block_size_) {
        const char* read = input + offset;
        char* write = output + offset;
        if (mode_ == 1) {
            DecryptBlock(read, write);
            for (unsigned int index = 0; index < block_size_; ++index) {
                write[index] ^= static_cast<char>(chain_[index]);
            }
            std::memcpy(chain_, read, block_size_);
        } else if (mode_ == 2) {
            EncryptBlock(reinterpret_cast<const char*>(chain_), write);
            for (unsigned int index = 0; index < block_size_; ++index) {
                write[index] ^= read[index];
            }
            std::memcpy(chain_, read, block_size_);
        } else {
            DecryptBlock(read, write);
        }
    }
    return kOldRijndaelSuccess;
}


// IDA_EA: 0x809C18A
// IDA_NAME: _ZN16CNCryptoRijndael10InitializeEPKhi
// DEMANGLED: CNCryptoRijndael::Initialize(unsigned char const*,int)
int CNCryptoRijndael::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_NULL_KEY;
    }
    if (length <= 0) {
        initialized_ = false;
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    // IDA CRijndael::Initialize @0x80B6220 repeats caller key bytes until
    // the 16-byte AES key is filled; short keys do not fail here.
    unsigned char full_key[kRijndaelKeySize];
    for (int index = 0; index < kRijndaelKeySize; ++index) {
        full_key[index] = key[index % length];
    }
    std::memcpy(initial_chain_, kRijndaelChain0, kRijndaelBlockSize);
    std::memcpy(chain_, initial_chain_, kRijndaelBlockSize);
    ExpandKey(full_key, round_keys_, decrypt_round_keys_);
    key_.assign(full_key, full_key + kRijndaelKeySize);
    initialized_ = true;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C112
// IDA_NAME: _ZN16CNCryptoRijndael7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoRijndael::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoRijndael::SetMode(CRYPTO_MODE_TYPE mode) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (mode == CRYPTO_MODE_ECB || mode == CRYPTO_MODE_CBC ||
        mode == CRYPTO_MODE_OFB) {
        // IDA IMethod::SetMode resets the active chain to the initial chain.
        mode_ = mode;
        std::memcpy(chain_, initial_chain_, kRijndaelBlockSize);
        return CRYPTO_RESULT_OK;
    }
    return CRYPTO_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x809C1E8
// IDA_NAME: _ZN16CNCryptoRijndael7EncryptEPhi
// DEMANGLED: CNCryptoRijndael::Encrypt(unsigned char *,int)
int CNCryptoRijndael::Encrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return TransformRijndael(round_keys_, chain_, mode_, data, length, data,
                             length, false);
}

// IDA_EA: 0x809C268
// IDA_NAME: _ZN16CNCryptoRijndael7DecryptEPhi
// DEMANGLED: CNCryptoRijndael::Decrypt(unsigned char *,int)
int CNCryptoRijndael::Decrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return TransformRijndael(round_keys_, chain_, mode_, data, length, data,
                             length, true);
}

// IDA_EA: 0x809C2E8
// IDA_NAME: _ZN16CNCryptoRijndael7EncryptEPKhiPhi
// DEMANGLED: CNCryptoRijndael::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoRijndael::Encrypt(const unsigned char* input, int input_length,
                              unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }
    if (input_length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return TransformRijndael(round_keys_, chain_, mode_, input, input_length,
                             output, output_capacity, false);
}

// IDA_EA: 0x809C376
// IDA_NAME: _ZN16CNCryptoRijndael7DecryptEPKhiPhi
// DEMANGLED: CNCryptoRijndael::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoRijndael::Decrypt(const unsigned char* input, int input_length,
                              unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }
    if (input_length % GetBlockSize()) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return TransformRijndael(round_keys_, chain_, mode_, input, input_length,
                             output, output_capacity, true);
}

// IDA_EA: 0x809C404
// IDA_NAME: _ZNK16CNCryptoRijndael10GetKeySizeEv
// DEMANGLED: CNCryptoRijndael::GetKeySize(void)const
int CNCryptoRijndael::GetKeySize() const {
    return kRijndaelKeySize;
}

// IDA_EA: 0x809C40E
// IDA_NAME: _ZNK16CNCryptoRijndael12GetBlockSizeEv
// DEMANGLED: CNCryptoRijndael::GetBlockSize(void)const
int CNCryptoRijndael::GetBlockSize() const {
    return kRijndaelBlockSize;
}
