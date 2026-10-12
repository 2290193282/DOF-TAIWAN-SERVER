#include "infra/security/crypto/crypto_kasumi.h"

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kKasumiKeySize = 16;
const int kKasumiBlockSize = 8;

// IDA: kasumi_setup(... )::C @0x8B1A620.
const unsigned int kKasumiC[8] = {
    0x0123, 0x4567, 0x89AB, 0xCDEF, 0xFEDC, 0xBA98, 0x7654, 0x3210,
};

// IDA: FI(unsigned int,unsigned int)::S9 @0x8B1A640, DWORD table.
const unsigned int kKasumiS9[512] = {
    0x0A7, 0x0EF, 0x0A1, 0x17B, 0x187, 0x14E, 0x009, 0x152,
    0x026, 0x0E2, 0x030, 0x166, 0x1C4, 0x181, 0x05A, 0x18D,
    0x0B7, 0x0FD, 0x093, 0x14B, 0x19F, 0x154, 0x033, 0x16A,
    0x132, 0x1F4, 0x106, 0x052, 0x0D8, 0x09F, 0x164, 0x0B1,
    0x0AF, 0x0F1, 0x1E9, 0x025, 0x0CE, 0x011, 0x000, 0x14D,
    0x02C, 0x0FE, 0x17A, 0x03A, 0x08F, 0x0DC, 0x051, 0x190,
    0x05F, 0x003, 0x13B, 0x0F5, 0x036, 0x0EB, 0x0DA, 0x195,
    0x1D8, 0x108, 0x0AC, 0x1EE, 0x173, 0x122, 0x18F, 0x04C,
    0x0A5, 0x0C5, 0x18B, 0x079, 0x101, 0x1E0, 0x1A7, 0x0D4,
    0x0F0, 0x01C, 0x1CE, 0x0B0, 0x196, 0x1FB, 0x120, 0x0DF,
    0x1F5, 0x197, 0x0F9, 0x109, 0x059, 0x0BA, 0x0DD, 0x1AC,
    0x0A4, 0x04A, 0x1B8, 0x0C4, 0x1CA, 0x1A5, 0x15E, 0x0A3,
    0x0E8, 0x09E, 0x086, 0x162, 0x00D, 0x0FA, 0x1EB, 0x08E,
    0x0BF, 0x045, 0x0C1, 0x1A9, 0x098, 0x0E3, 0x16E, 0x087,
    0x158, 0x12C, 0x114, 0x0F2, 0x1B5, 0x140, 0x071, 0x116,
    0x00B, 0x0F3, 0x057, 0x13D, 0x024, 0x05D, 0x1F0, 0x01B,
    0x1E7, 0x1BE, 0x1E2, 0x029, 0x044, 0x09C, 0x1C9, 0x083,
    0x146, 0x193, 0x153, 0x014, 0x027, 0x073, 0x1BA, 0x07C,
    0x1DB, 0x180, 0x1FC, 0x035, 0x070, 0x0AA, 0x1DF, 0x097,
    0x07E, 0x0A9, 0x049, 0x10C, 0x117, 0x141, 0x0A8, 0x16C,
    0x16B, 0x124, 0x02E, 0x1F3, 0x189, 0x147, 0x144, 0x018,
    0x1C8, 0x10B, 0x09D, 0x1CC, 0x1E8, 0x1AA, 0x135, 0x0E5,
    0x1B7, 0x1FA, 0x0D0, 0x10F, 0x15D, 0x191, 0x1B2, 0x0EC,
    0x010, 0x0D1, 0x167, 0x034, 0x038, 0x078, 0x0C7, 0x115,
    0x1D1, 0x1A0, 0x0FC, 0x11F, 0x0F6, 0x006, 0x053, 0x131,
    0x1A4, 0x159, 0x099, 0x1F6, 0x041, 0x03D, 0x0F4, 0x11A,
    0x0AD, 0x0DE, 0x1A2, 0x043, 0x182, 0x170, 0x105, 0x065,
    0x1DC, 0x123, 0x0C3, 0x1AE, 0x031, 0x04F, 0x0A6, 0x14A,
    0x118, 0x17F, 0x175, 0x080, 0x17E, 0x198, 0x09B, 0x1EF,
    0x16F, 0x184, 0x112, 0x06B, 0x1CB, 0x1A1, 0x03E, 0x1C6,
    0x084, 0x0E1, 0x0CB, 0x13C, 0x0EA, 0x00E, 0x12D, 0x05B,
    0x1F7, 0x11E, 0x1A8, 0x0D3, 0x15B, 0x133, 0x08C, 0x176,
    0x023, 0x067, 0x07D, 0x1AB, 0x013, 0x0D6, 0x1C5, 0x092,
    0x1F2, 0x13A, 0x1BC, 0x0E6, 0x100, 0x149, 0x0C6, 0x11D,
    0x032, 0x074, 0x04E, 0x19A, 0x00A, 0x0CD, 0x1FE, 0x0AB,
    0x0E7, 0x02D, 0x08B, 0x1D3, 0x01D, 0x056, 0x1F9, 0x020,
    0x048, 0x01A, 0x156, 0x096, 0x139, 0x1EA, 0x1AF, 0x0EE,
    0x19B, 0x145, 0x095, 0x1D9, 0x028, 0x077, 0x0AE, 0x163,
    0x0B9, 0x0E9, 0x185, 0x047, 0x1C0, 0x111, 0x174, 0x037,
    0x06E, 0x0B2, 0x142, 0x00C, 0x1D5, 0x188, 0x171, 0x0BE,
    0x001, 0x06D, 0x177, 0x089, 0x0B5, 0x058, 0x04B, 0x134,
    0x104, 0x1E4, 0x062, 0x110, 0x172, 0x113, 0x19C, 0x06F,
    0x150, 0x13E, 0x004, 0x1F8, 0x1EC, 0x103, 0x130, 0x04D,
    0x151, 0x1B3, 0x015, 0x165, 0x12F, 0x14C, 0x1E3, 0x012,
    0x02F, 0x055, 0x019, 0x1F1, 0x1DA, 0x121, 0x064, 0x10D,
    0x128, 0x1DE, 0x10E, 0x06A, 0x01F, 0x068, 0x1B1, 0x054,
    0x19E, 0x1E6, 0x18A, 0x060, 0x063, 0x09A, 0x1FF, 0x094,
    0x19D, 0x169, 0x199, 0x0FF, 0x0A2, 0x0D7, 0x12E, 0x0C9,
    0x10A, 0x15F, 0x157, 0x090, 0x1B9, 0x16D, 0x06C, 0x12A,
    0x0FB, 0x022, 0x0B6, 0x1FD, 0x08A, 0x0D2, 0x14F, 0x085,
    0x137, 0x160, 0x148, 0x08D, 0x18C, 0x15A, 0x07B, 0x13F,
    0x1C2, 0x119, 0x1AD, 0x0E4, 0x1BB, 0x1E1, 0x05C, 0x194,
    0x1E5, 0x1A6, 0x0F8, 0x129, 0x017, 0x0D5, 0x082, 0x1D2,
    0x016, 0x0D9, 0x11B, 0x046, 0x126, 0x168, 0x1A3, 0x07F,
    0x138, 0x179, 0x007, 0x1D4, 0x0C2, 0x002, 0x075, 0x127,
    0x1CF, 0x102, 0x0E0, 0x1BF, 0x0F7, 0x0BB, 0x050, 0x18E,
    0x11C, 0x161, 0x069, 0x186, 0x12B, 0x1D7, 0x1D6, 0x0B8,
    0x039, 0x0C8, 0x15C, 0x03F, 0x0CC, 0x0BC, 0x021, 0x1C3,
    0x061, 0x01E, 0x136, 0x0DB, 0x05E, 0x0A0, 0x081, 0x1ED,
    0x040, 0x0B3, 0x107, 0x066, 0x0BD, 0x0CF, 0x072, 0x192,
    0x1B6, 0x1DD, 0x183, 0x07A, 0x0C0, 0x02A, 0x17D, 0x005,
    0x091, 0x076, 0x0B4, 0x1C1, 0x125, 0x143, 0x088, 0x17C,
    0x02B, 0x042, 0x03C, 0x1C7, 0x155, 0x1BD, 0x0CA, 0x1B0,
    0x008, 0x0ED, 0x00F, 0x178, 0x1B4, 0x1D0, 0x03B, 0x1CD,
};

// IDA: FI(unsigned int,unsigned int)::S7 @0x8B1AE40, DWORD table.
const unsigned int kKasumiS7[128] = {
    0x036, 0x032, 0x03E, 0x038, 0x016, 0x022, 0x05E, 0x060,
    0x026, 0x006, 0x03F, 0x05D, 0x002, 0x012, 0x07B, 0x021,
    0x037, 0x071, 0x027, 0x072, 0x015, 0x043, 0x041, 0x00C,
    0x02F, 0x049, 0x02E, 0x01B, 0x019, 0x06F, 0x07C, 0x051,
    0x035, 0x009, 0x079, 0x04F, 0x034, 0x03C, 0x03A, 0x030,
    0x065, 0x07F, 0x028, 0x078, 0x068, 0x046, 0x047, 0x02B,
    0x014, 0x07A, 0x048, 0x03D, 0x017, 0x06D, 0x00D, 0x064,
    0x04D, 0x001, 0x010, 0x007, 0x052, 0x00A, 0x069, 0x062,
    0x075, 0x074, 0x04C, 0x00B, 0x059, 0x06A, 0x000, 0x07D,
    0x076, 0x063, 0x056, 0x045, 0x01E, 0x039, 0x07E, 0x057,
    0x070, 0x033, 0x011, 0x005, 0x05F, 0x00E, 0x05A, 0x054,
    0x05B, 0x008, 0x023, 0x067, 0x020, 0x061, 0x01C, 0x042,
    0x066, 0x01F, 0x01A, 0x02D, 0x04B, 0x004, 0x055, 0x05C,
    0x025, 0x04A, 0x050, 0x031, 0x044, 0x01D, 0x073, 0x02C,
    0x040, 0x06B, 0x06C, 0x018, 0x06E, 0x053, 0x024, 0x04E,
    0x02A, 0x013, 0x00F, 0x029, 0x058, 0x077, 0x03B, 0x003,
};

unsigned int Rotl16(unsigned int value, unsigned int bits) {
    value &= 0xFFFFU;
    return ((value << bits) | (value >> (16 - bits))) & 0xFFFFU;
}

unsigned int Low16(unsigned int value) {
    return value & 0xFFFFU;
}

unsigned int High16(unsigned int value) {
    return (value >> 16) & 0xFFFFU;
}

unsigned int KasumiFI(unsigned int value, unsigned int subkey) {
    unsigned int left9 = (value >> 7) & 0x1FFU;
    unsigned int right7 = value & 0x7FU;

    // IDA FI @0x80B153C uses S9/S7 DWORD tables and keeps the 9/7-bit split
    // as local DWORDs until it recombines `(right7 << 9) + left9`.
    left9 = kKasumiS9[left9] ^ right7;
    right7 = kKasumiS7[right7] ^ (left9 & 0x7FU) ^ (subkey >> 9);
    left9 ^= subkey & 0x1FFU;
    left9 = kKasumiS9[left9] ^ right7;
    right7 = kKasumiS7[right7] ^ (left9 & 0x7FU);
    return left9 + (right7 << 9);
}

unsigned int KasumiFO(unsigned int value, int round,
                      const unsigned int round_keys[64]) {
    unsigned int v4 = Low16(value) ^
        KasumiFI(Low16(round_keys[round + 16]) ^ High16(value),
                 round_keys[round + 40]);
    unsigned int v5 = v4 ^
        KasumiFI(Low16(round_keys[round + 24]) ^ Low16(value),
                 round_keys[round + 48]);
    return (v5 ^
            KasumiFI(Low16(round_keys[round + 32]) ^ Low16(v4),
                     round_keys[round + 56])) +
           (v5 << 16);
}

unsigned int KasumiFL(unsigned int value, int round,
                      const unsigned int round_keys[64]) {
    unsigned int v5 = High16(value) & round_keys[round];
    unsigned int v4 = Rotl16(v5, 1) ^ Low16(value);
    unsigned int v6 = v4 | round_keys[round + 8];
    return v4 + ((Rotl16(v6, 1) ^ High16(value)) << 16);
}

int KasumiSetup(const unsigned char* key, int key_length, int rounds,
                unsigned int round_keys[64]) {
    if (key_length != kKasumiKeySize) {
        return 3;
    }
    if (rounds != 0 && rounds != 8) {
        return 4;
    }

    unsigned int words[16];
    for (int index = 0; index <= 7; ++index) {
        words[index + 8] = (static_cast<unsigned int>(key[2 * index]) << 8) |
                           key[2 * index + 1];
    }
    for (int index = 0; index <= 7; ++index) {
        words[index] = words[index + 8] ^ kKasumiC[index];
    }

    for (int index = 0; index <= 7; ++index) {
        round_keys[index] = Rotl16(words[index + 8], 1);
        round_keys[index + 8] = words[(index + 2) & 7];
        round_keys[index + 16] = Rotl16(words[((index + 1) & 7) + 8], 5);
        round_keys[index + 24] = Rotl16(words[((index + 5) & 7) + 8], 8);
        round_keys[index + 32] = Rotl16(words[((index + 6) & 7) + 8], 13);
        round_keys[index + 40] = words[(index + 4) & 7];
        round_keys[index + 48] = words[(index + 3) & 7];
        round_keys[index + 56] = words[(index + 7) & 7];
    }
    return 0;
}

void KasumiEncryptBlock(const unsigned char* input, unsigned char* output,
                        const unsigned int round_keys[64]) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);

    for (int round = 0; round <= 7;) {
        unsigned int mixed = KasumiFL(left, round, round_keys);
        const int first_round = round;
        ++round;
        right ^= KasumiFO(mixed, first_round, round_keys);
        mixed = KasumiFO(right, round, round_keys);
        const int second_round = round;
        ++round;
        left ^= KasumiFL(mixed, second_round, round_keys);
    }

    crypto_detail::WriteBE32(output, left);
    crypto_detail::WriteBE32(output + 4, right);
}

void KasumiDecryptBlock(const unsigned char* input, unsigned char* output,
                        const unsigned int round_keys[64]) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);

    for (int round = 7; round >= 0;) {
        unsigned int mixed = KasumiFO(right, round, round_keys);
        const int first_round = round;
        --round;
        left ^= KasumiFL(mixed, first_round, round_keys);
        mixed = KasumiFL(left, round, round_keys);
        const int second_round = round;
        --round;
        right ^= KasumiFO(mixed, second_round, round_keys);
    }

    crypto_detail::WriteBE32(output, left);
    crypto_detail::WriteBE32(output + 4, right);
}

}  // namespace

// IDA_EA: 0x809A4B0
// IDA_NAME: _ZN14CNCryptoKasumiC2Ev
// DEMANGLED: CNCryptoKasumi::CNCryptoKasumi(void)
CNCryptoKasumi::CNCryptoKasumi()
    : ICryptoGraph(CRYPTO_GRAPH_KASUMI), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809A54E
// IDA_NAME: _ZN14CNCryptoKasumiD2Ev
// DEMANGLED: CNCryptoKasumi::~CNCryptoKasumi()
// IDA_EA: 0x809A5A2
// IDA_NAME: _ZN14CNCryptoKasumiD0Ev
// DEMANGLED: CNCryptoKasumi::~CNCryptoKasumi() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoKasumi::~CNCryptoKasumi() {}

// IDA_EA: 0x809A5C0
// IDA_NAME: _ZN14CNCryptoKasumiC2ERKS_
// DEMANGLED: CNCryptoKasumi::CNCryptoKasumi(CNCryptoKasumi const&)
CNCryptoKasumi::CNCryptoKasumi(const CNCryptoKasumi& rhs)
    : ICryptoGraph(rhs), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809A5E4
// IDA_NAME: _ZN14CNCryptoKasumiaSERKS_
// DEMANGLED: CNCryptoKasumi::operator=(CNCryptoKasumi const&)
CNCryptoKasumi& CNCryptoKasumi::operator=(const CNCryptoKasumi& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809A604
// IDA_NAME: _ZN14CNCryptoKasumi10InitializeEPKhi
// DEMANGLED: CNCryptoKasumi::Initialize(unsigned char const*,int)
int CNCryptoKasumi::Initialize(const unsigned char* key, int length) {
    if (key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (GetKeySize() > length) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = KasumiSetup(key, GetKeySize(), 8, round_keys_);
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

// IDA_EA: 0x809A5EC
// IDA_NAME: _ZN14CNCryptoKasumi7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoKasumi::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoKasumi::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A6AC
// IDA_NAME: _ZN14CNCryptoKasumi7EncryptEPhi
// DEMANGLED: CNCryptoKasumi::Encrypt(unsigned char *,int)
int CNCryptoKasumi::Encrypt(unsigned char* data, int length) {
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
        KasumiEncryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A784
// IDA_NAME: _ZN14CNCryptoKasumi7DecryptEPhi
// DEMANGLED: CNCryptoKasumi::Decrypt(unsigned char *,int)
int CNCryptoKasumi::Decrypt(unsigned char* data, int length) {
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
        KasumiDecryptBlock(data + offset, data + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A85C
// IDA_NAME: _ZN14CNCryptoKasumi7EncryptEPKhiPhi
// DEMANGLED: CNCryptoKasumi::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoKasumi::Encrypt(const unsigned char* input, int input_length,
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
        KasumiEncryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809A94E
// IDA_NAME: _ZN14CNCryptoKasumi7DecryptEPKhiPhi
// DEMANGLED: CNCryptoKasumi::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoKasumi::Decrypt(const unsigned char* input, int input_length,
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
        KasumiDecryptBlock(input + offset, output + offset, round_keys_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809AA4A
// IDA_NAME: _ZNK14CNCryptoKasumi10GetKeySizeEv
// DEMANGLED: CNCryptoKasumi::GetKeySize(void)const
int CNCryptoKasumi::GetKeySize() const {
    return kKasumiKeySize;
}

// IDA_EA: 0x809AA40
// IDA_NAME: _ZNK14CNCryptoKasumi12GetBlockSizeEv
// DEMANGLED: CNCryptoKasumi::GetBlockSize(void)const
int CNCryptoKasumi::GetBlockSize() const {
    return kKasumiBlockSize;
}
