#include "infra/security/crypto/crypto_twofish.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kTwofishKeySize = 32;
const int kTwofishBlockSize = 16;
const int kTwofishRounds = 16;

#include "infra/security/crypto/crypto_twofish_tables.inc"

unsigned int ReadLE32(const unsigned char* data) {
    return crypto_detail::ReadLE32(data);
}

void WriteLE32(unsigned char* data, unsigned int value) {
    crypto_detail::WriteLE32(data, value);
}

unsigned int Rol(unsigned int value, unsigned int bits) {
    return crypto_detail::RotateLeft32(value, bits);
}

unsigned int Ror(unsigned int value, unsigned int bits) {
    return crypto_detail::RotateRight32(value, bits);
}

unsigned char GfMult(unsigned int left, unsigned int right, unsigned int poly) {
    unsigned int result = 0;
    unsigned int value = right;
    unsigned int mask[2] = {0, poly};
    unsigned int source = left;
    for (int bit = 0; bit < 8; ++bit) {
        if (source & 1U) {
            result ^= value;
        }
        source >>= 1;
        value = (2 * value) ^ mask[value >> 7];
    }
    return static_cast<unsigned char>(result & 0xFFU);
}

void RsMult(const unsigned char input[8], unsigned char output[4]) {
    for (int row = 0; row <= 3; ++row) {
        output[row] = 0;
        for (int column = 0; column <= 7; ++column) {
            output[row] ^= GfMult(input[column],
                                  kTwofishRS[8 * row + column], 0x14D);
        }
    }
}

void MdsMult(const unsigned char input[4], unsigned char output[4]) {
    unsigned int value = 0;
    for (int index = 0; index <= 3; ++index) {
        value ^= kTwofishMdsTab[256 * index + input[index]];
    }
    WriteLE32(output, value);
}

void HFunc(const unsigned char input[4], unsigned char output[4],
           const unsigned char key[32], int key_words, int key_half) {
    unsigned char y0 = input[0];
    unsigned char y1 = input[1];
    unsigned char y2 = input[2];
    unsigned char y3 = input[3];

    if (key_words == 4) {
        y0 = kTwofishQ1[y0] ^ key[4 * key_half + 24];
        y1 = kTwofishQ0[y1] ^ key[4 * key_half + 25];
        y2 = kTwofishQ0[y2] ^ key[4 * key_half + 26];
        y3 = kTwofishQ1[y3] ^ key[4 * key_half + 27];
    }
    if (key_words == 3 || key_words == 4) {
        y0 = kTwofishQ1[y0] ^ key[4 * key_half + 16];
        y1 = kTwofishQ1[y1] ^ key[4 * key_half + 17];
        y2 = kTwofishQ0[y2] ^ key[4 * key_half + 18];
        y3 = kTwofishQ0[y3] ^ key[4 * key_half + 19];
    }

    y0 = kTwofishQ1[kTwofishQ0[kTwofishQ0[y0] ^ key[4 * key_half + 8]] ^
                    key[4 * key_half]];
    y1 = kTwofishQ0[kTwofishQ0[kTwofishQ1[y1] ^ key[4 * key_half + 9]] ^
                    key[4 * key_half + 1]];
    y2 = kTwofishQ1[kTwofishQ1[kTwofishQ0[y2] ^ key[4 * key_half + 10]] ^
                    key[4 * key_half + 2]];
    y3 = kTwofishQ0[kTwofishQ1[kTwofishQ1[y3] ^ key[4 * key_half + 11]] ^
                    key[4 * key_half + 3]];

    unsigned char mixed[4] = {y0, y1, y2, y3};
    MdsMult(mixed, output);
}

unsigned int HWord(unsigned int value, const unsigned char key[32],
                   int key_words, int key_half) {
    unsigned char input[4];
    unsigned char output[4];
    WriteLE32(input, value);
    HFunc(input, output, key, key_words, key_half);
    return ReadLE32(output);
}

unsigned int G0(const unsigned int sbox[1024], unsigned int value) {
    return sbox[value & 0xFFU] ^
           sbox[256 + ((value >> 8) & 0xFFU)] ^
           sbox[512 + ((value >> 16) & 0xFFU)] ^
           sbox[768 + ((value >> 24) & 0xFFU)];
}

unsigned int G1(const unsigned int sbox[1024], unsigned int value) {
    return sbox[(value >> 24) & 0xFFU] ^
           sbox[256 + (value & 0xFFU)] ^
           sbox[512 + ((value >> 8) & 0xFFU)] ^
           sbox[768 + ((value >> 16) & 0xFFU)];
}

void BuildKeyDependentSbox(unsigned int sbox[1024],
                           const unsigned char rs_key[16], int key_words) {
    for (int index = 0; index <= 255; ++index) {
        const unsigned char q0 = kTwofishQ0[index];
        const unsigned char q1 = kTwofishQ1[index];
        if (key_words == 2) {
            sbox[index] =
                kTwofishMdsTab[kTwofishQ1[
                    kTwofishQ0[static_cast<unsigned char>(q0 ^ rs_key[0])] ^
                    rs_key[4]]];
            sbox[index + 256] =
                kTwofishMdsTab[256 + kTwofishQ0[
                    kTwofishQ0[static_cast<unsigned char>(q1 ^ rs_key[1])] ^
                    rs_key[5]]];
            sbox[index + 512] =
                kTwofishMdsTab[512 + kTwofishQ1[
                    kTwofishQ1[static_cast<unsigned char>(q0 ^ rs_key[2])] ^
                    rs_key[6]]];
            sbox[index + 768] =
                kTwofishMdsTab[768 + kTwofishQ0[
                    kTwofishQ1[static_cast<unsigned char>(q1 ^ rs_key[3])] ^
                    rs_key[7]]];
        } else if (key_words == 3) {
            sbox[index] =
                kTwofishMdsTab[kTwofishQ1[
                    kTwofishQ0[
                        kTwofishQ0[static_cast<unsigned char>(q1 ^ rs_key[0])] ^
                        rs_key[4]] ^
                    rs_key[8]]];
            sbox[index + 256] =
                kTwofishMdsTab[256 + kTwofishQ0[
                    kTwofishQ0[
                        kTwofishQ1[static_cast<unsigned char>(q1 ^ rs_key[1])] ^
                        rs_key[5]] ^
                    rs_key[9]]];
            sbox[index + 512] =
                kTwofishMdsTab[512 + kTwofishQ1[
                    kTwofishQ1[
                        kTwofishQ0[static_cast<unsigned char>(q0 ^ rs_key[2])] ^
                        rs_key[6]] ^
                    rs_key[10]]];
            sbox[index + 768] =
                kTwofishMdsTab[768 + kTwofishQ0[
                    kTwofishQ1[
                        kTwofishQ1[static_cast<unsigned char>(q0 ^ rs_key[3])] ^
                        rs_key[7]] ^
                    rs_key[11]]];
        } else {
            sbox[index] =
                kTwofishMdsTab[kTwofishQ1[
                    kTwofishQ0[
                        kTwofishQ0[
                            kTwofishQ1[static_cast<unsigned char>(q1 ^ rs_key[0])] ^
                            rs_key[4]] ^
                        rs_key[8]] ^
                    rs_key[12]]];
            sbox[index + 256] =
                kTwofishMdsTab[256 + kTwofishQ0[
                    kTwofishQ0[
                        kTwofishQ1[
                            kTwofishQ1[static_cast<unsigned char>(q0 ^ rs_key[1])] ^
                            rs_key[5]] ^
                        rs_key[9]] ^
                    rs_key[13]]];
            sbox[index + 512] =
                kTwofishMdsTab[512 + kTwofishQ1[
                    kTwofishQ1[
                        kTwofishQ0[
                            kTwofishQ0[static_cast<unsigned char>(q0 ^ rs_key[2])] ^
                            rs_key[6]] ^
                        rs_key[10]] ^
                    rs_key[14]]];
            sbox[index + 768] =
                kTwofishMdsTab[768 + kTwofishQ0[
                    kTwofishQ1[
                        kTwofishQ1[
                            kTwofishQ0[static_cast<unsigned char>(q1 ^ rs_key[3])] ^
                            rs_key[7]] ^
                        rs_key[11]] ^
                    rs_key[15]]];
        }
    }
}

int TwofishSetup(const unsigned char* key, int key_length, int rounds,
                 unsigned int sbox[1024], unsigned int subkeys[40]) {
    if (rounds != kTwofishRounds && rounds != 0) {
        return 4;
    }
    if (key_length != 16 && key_length != 24 && key_length != 32) {
        return 3;
    }

    unsigned char local_key[32];
    std::memset(local_key, 0, sizeof(local_key));
    std::memcpy(local_key, key, key_length);
    const int key_words = key_length / 8;

    unsigned char rs_key[16];
    for (int index = 0; index < key_words; ++index) {
        RsMult(local_key + 8 * index, rs_key + 4 * index);
    }

    for (int index = 0; index <= 19; ++index) {
        unsigned char input[4];
        std::memset(input, 2 * index, sizeof(input));
        const unsigned int a = HWord(ReadLE32(input), local_key, key_words, 0);
        std::memset(input, 2 * index + 1, sizeof(input));
        unsigned int b = HWord(ReadLE32(input), local_key, key_words, 1);
        b = Rol(b, 8);
        subkeys[2 * index] = a + b;
        subkeys[2 * index + 1] = Rol(a + 2 * b, 9);
    }

    BuildKeyDependentSbox(sbox, rs_key, key_words);
    return 0;
}

void TwofishEncryptBlock(const unsigned int sbox[1024],
                         const unsigned int subkeys[40],
                         const unsigned char* input, unsigned char* output) {
    unsigned int a = ReadLE32(input) ^ subkeys[0];
    unsigned int b = ReadLE32(input + 4) ^ subkeys[1];
    unsigned int c = ReadLE32(input + 8) ^ subkeys[2];
    unsigned int d = ReadLE32(input + 12) ^ subkeys[3];
    const unsigned int* round_key = subkeys + 8;

    for (int round = 0; round < 8; ++round) {
        const unsigned int t0 = G0(sbox, a);
        const unsigned int t1 = G1(sbox, b);
        const unsigned int f0 = t0 + t1;
        c = Ror(c ^ (f0 + round_key[0]), 1);
        d = (f0 + t1 + round_key[1]) ^ Rol(d, 1);

        const unsigned int t2 = G0(sbox, c);
        const unsigned int t3 = G1(sbox, d);
        const unsigned int f1 = t2 + t3;
        a = Ror(a ^ (f1 + round_key[2]), 1);
        b = (f1 + t3 + round_key[3]) ^ Rol(b, 1);
        round_key += 4;
    }

    WriteLE32(output, c ^ subkeys[4]);
    WriteLE32(output + 4, d ^ subkeys[5]);
    WriteLE32(output + 8, a ^ subkeys[6]);
    WriteLE32(output + 12, b ^ subkeys[7]);
}

void TwofishDecryptBlock(const unsigned int sbox[1024],
                         const unsigned int subkeys[40],
                         const unsigned char* input, unsigned char* output) {
    unsigned int a = ReadLE32(input + 8) ^ subkeys[6];
    unsigned int b = ReadLE32(input + 12) ^ subkeys[7];
    unsigned int c = ReadLE32(input) ^ subkeys[4];
    unsigned int d = ReadLE32(input + 4) ^ subkeys[5];
    const unsigned int* round_key = subkeys + 36;

    for (int round = 0; round < 8; ++round) {
        const unsigned int t0 = G0(sbox, c);
        const unsigned int t1 = G1(sbox, d);
        const unsigned int f0 = t0 + t1;
        a = (f0 + round_key[2]) ^ Rol(a, 1);
        b = Ror(b ^ (f0 + t1 + round_key[3]), 1);

        const unsigned int t2 = G0(sbox, a);
        const unsigned int t3 = G1(sbox, b);
        const unsigned int f1 = t2 + t3;
        c = (f1 + round_key[0]) ^ Rol(c, 1);
        d = Ror(d ^ (f1 + t3 + round_key[1]), 1);
        round_key -= 4;
    }

    WriteLE32(output, a ^ subkeys[0]);
    WriteLE32(output + 4, b ^ subkeys[1]);
    WriteLE32(output + 8, c ^ subkeys[2]);
    WriteLE32(output + 12, d ^ subkeys[3]);
}

int TransformTwofish(const unsigned int sbox[1024],
                     const unsigned int subkeys[40], bool initialized,
                     const unsigned char* input, int input_length,
                     unsigned char* output, int output_capacity,
                     bool decrypt, int block_error, bool reject_empty) {
    if (!initialized) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if ((input_length % kTwofishBlockSize) != 0) {
        return block_error;
    }
    if (reject_empty && input_length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += kTwofishBlockSize) {
        if (decrypt) {
            TwofishDecryptBlock(sbox, subkeys, input + offset, output + offset);
        } else {
            TwofishEncryptBlock(sbox, subkeys, input + offset, output + offset);
        }
    }
    return CRYPTO_RESULT_OK;
}

}  // namespace

// IDA_EA: 0x809D328
// IDA_NAME: _ZN15CNCryptoTwoFishC2Ev
// DEMANGLED: CNCryptoTwoFish::CNCryptoTwoFish(void)
CNCryptoTwoFish::CNCryptoTwoFish()
    : ICryptoGraph(CRYPTO_GRAPH_TWOFISH),
      sbox_(),
      subkeys_(),
      initialized_(false) {}

// IDA_EA: 0x809D3C6
// IDA_NAME: _ZN15CNCryptoTwoFishD2Ev
// DEMANGLED: CNCryptoTwoFish::~CNCryptoTwoFish()
// IDA_EA: 0x809D41A
// IDA_NAME: _ZN15CNCryptoTwoFishD0Ev
// DEMANGLED: CNCryptoTwoFish::~CNCryptoTwoFish() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoTwoFish::~CNCryptoTwoFish() {}

// IDA_EA: 0x809D438
// IDA_NAME: _ZN15CNCryptoTwoFishC2ERKS_
// DEMANGLED: CNCryptoTwoFish::CNCryptoTwoFish(CNCryptoTwoFish const&)
CNCryptoTwoFish::CNCryptoTwoFish(const CNCryptoTwoFish& rhs)
    : ICryptoGraph(rhs), sbox_(), subkeys_(), initialized_(false) {}

// IDA_EA: 0x809D45C
// IDA_NAME: _ZN15CNCryptoTwoFishaSERKS_
// DEMANGLED: CNCryptoTwoFish::operator=(CNCryptoTwoFish const&)
CNCryptoTwoFish& CNCryptoTwoFish::operator=(const CNCryptoTwoFish& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809D47C
// IDA_NAME: _ZN15CNCryptoTwoFish10InitializeEPKhi
// DEMANGLED: CNCryptoTwoFish::Initialize(unsigned char const*,int)
int CNCryptoTwoFish::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length < GetKeySize()) {
        initialized_ = false;
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    const int rc = TwofishSetup(key, GetKeySize(), kTwofishRounds, sbox_,
                                subkeys_);
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

// IDA_EA: 0x809D464
// IDA_NAME: _ZN15CNCryptoTwoFish7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoTwoFish::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoTwoFish::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809D524
// IDA_NAME: _ZN15CNCryptoTwoFish7EncryptEPhi
// DEMANGLED: CNCryptoTwoFish::Encrypt(unsigned char *,int)
int CNCryptoTwoFish::Encrypt(unsigned char* data, int length) {
    return TransformTwofish(sbox_, subkeys_, initialized_, data, length, data,
                            length, false, CRYPTO_RESULT_INVALID_BLOCK_SIZE,
                            true);
}

// IDA_EA: 0x809D5FC
// IDA_NAME: _ZN15CNCryptoTwoFish7DecryptEPhi
// DEMANGLED: CNCryptoTwoFish::Decrypt(unsigned char *,int)
int CNCryptoTwoFish::Decrypt(unsigned char* data, int length) {
    return TransformTwofish(sbox_, subkeys_, initialized_, data, length, data,
                            length, true, CRYPTO_RESULT_INVALID_BLOCK_SIZE,
                            true);
}

// IDA_EA: 0x809D6D4
// IDA_NAME: _ZN15CNCryptoTwoFish7EncryptEPKhiPhi
// DEMANGLED: CNCryptoTwoFish::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoTwoFish::Encrypt(const unsigned char* input, int input_length,
                             unsigned char* output, int output_capacity) {
    return TransformTwofish(sbox_, subkeys_, initialized_, input, input_length,
                            output, output_capacity, false,
                            CRYPTO_RESULT_INVALID_BLOCK_SIZE, false);
}

// IDA_EA: 0x809D7C6
// IDA_NAME: _ZN15CNCryptoTwoFish7DecryptEPKhiPhi
// DEMANGLED: CNCryptoTwoFish::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoTwoFish::Decrypt(const unsigned char* input, int input_length,
                             unsigned char* output, int output_capacity) {
    return TransformTwofish(sbox_, subkeys_, initialized_, input, input_length,
                            output, output_capacity, true,
                            CRYPTO_RESULT_BUFFER_TOO_SMALL, false);
}

// IDA_EA: 0x809D8B8
// IDA_NAME: _ZNK15CNCryptoTwoFish10GetKeySizeEv
// DEMANGLED: CNCryptoTwoFish::GetKeySize(void)const
int CNCryptoTwoFish::GetKeySize() const {
    return kTwofishKeySize;
}

// IDA_EA: 0x809D8C2
// IDA_NAME: _ZNK15CNCryptoTwoFish12GetBlockSizeEv
// DEMANGLED: CNCryptoTwoFish::GetBlockSize(void)const
int CNCryptoTwoFish::GetBlockSize() const {
    return kTwofishBlockSize;
}
