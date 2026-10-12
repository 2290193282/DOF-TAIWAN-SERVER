#include "infra/security/crypto/crypto_khazad.h"

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kKhazadKeySize = 16;
const int kKhazadBlockSize = 8;
const int kKhazadRounds = 8;

#include "infra/security/crypto/crypto_khazad_tables.inc"

unsigned int Low32(unsigned long long value) {
    return static_cast<unsigned int>(value & 0xFFFFFFFFULL);
}

unsigned int High32(unsigned long long value) {
    return static_cast<unsigned int>((value >> 32) & 0xFFFFFFFFULL);
}

unsigned long long Pack64(unsigned int low, unsigned int high) {
    return static_cast<unsigned long long>(low) |
           (static_cast<unsigned long long>(high) << 32);
}

unsigned int ByteOf64(unsigned long long value, int index) {
    return static_cast<unsigned int>((value >> (index * 8)) & 0xFFULL);
}

unsigned int ByteOf32(unsigned int value, int index) {
    return (value >> (index * 8)) & 0xFFU;
}

unsigned long long ReadKhazadBlock(const unsigned char* data) {
    // IDA builds LODWORD from bytes 4..7 and HIDWORD from bytes 0..3, both in
    // big-endian byte order. Keeping the low/high dword split mirrors the
    // original 32-bit compiler output.
    return Pack64(crypto_detail::ReadBE32(data + 4),
                  crypto_detail::ReadBE32(data));
}

void WriteKhazadBlock(unsigned char* data, unsigned long long value) {
    crypto_detail::WriteBE32(data, High32(value));
    crypto_detail::WriteBE32(data + 4, Low32(value));
}

unsigned long long KhazadRound(unsigned long long value) {
    return kKhazadT7[ByteOf64(value, 0)] ^
           kKhazadT6[ByteOf64(value, 1)] ^
           kKhazadT5[ByteOf64(value, 2)] ^
           kKhazadT4[ByteOf64(value, 3)] ^
           kKhazadT3[ByteOf64(value, 4)] ^
           kKhazadT2[ByteOf64(value, 5)] ^
           kKhazadT1[ByteOf64(value, 6)] ^
           kKhazadT0[ByteOf64(value, 7)];
}

unsigned long long KhazadInverseRoundKey(unsigned long long value) {
    const unsigned int b7 = Low32(kKhazadT7[ByteOf64(value, 7)]) & 0xFFU;
    const unsigned int b6 = Low32(kKhazadT7[ByteOf64(value, 6)]) & 0xFFU;
    const unsigned int b5 = Low32(kKhazadT7[ByteOf64(value, 5)]) & 0xFFU;
    const unsigned int b4 = Low32(kKhazadT7[ByteOf64(value, 4)]) & 0xFFU;
    const unsigned int b3 = Low32(kKhazadT7[ByteOf64(value, 3)]) & 0xFFU;
    const unsigned int b2 = Low32(kKhazadT7[ByteOf64(value, 2)]) & 0xFFU;
    const unsigned int b1 = Low32(kKhazadT7[ByteOf64(value, 1)]) & 0xFFU;
    const unsigned int b0 = Low32(kKhazadT7[ByteOf64(value, 0)]) & 0xFFU;

    // IDA: khazad_setup @0x80B21A1 maps encryption round keys through the T
    // tables in reverse byte order to build the decrypt schedule.
    return kKhazadT0[b7] ^ kKhazadT1[b6] ^ kKhazadT2[b5] ^
           kKhazadT3[b4] ^ kKhazadT4[b3] ^ kKhazadT5[b2] ^
           kKhazadT6[b1] ^ kKhazadT7[b0];
}

unsigned long long KhazadFinalRound(unsigned long long value,
                                    unsigned long long round_key) {
    const unsigned int low =
        Low32(round_key) ^
        (Low32(kKhazadT7[ByteOf32(Low32(value), 0)]) & 0x000000FFU) ^
        (Low32(kKhazadT6[ByteOf32(Low32(value), 1)]) & 0x0000FF00U) ^
        (Low32(kKhazadT5[ByteOf32(Low32(value), 2)]) & 0x00FF0000U) ^
        (Low32(kKhazadT4[ByteOf32(Low32(value), 3)]) & 0xFF000000U);
    const unsigned int high =
        High32(round_key) ^
        (High32(kKhazadT3[ByteOf32(High32(value), 0)]) & 0x000000FFU) ^
        (High32(kKhazadT2[ByteOf32(High32(value), 1)]) & 0x0000FF00U) ^
        (High32(kKhazadT1[ByteOf32(High32(value), 2)]) & 0x00FF0000U) ^
        (High32(kKhazadT0[ByteOf32(High32(value), 3)]) & 0xFF000000U);
    return Pack64(low, high);
}

int KhazadSetup(const unsigned char* key, int key_length, int rounds,
                unsigned long long round_keys[18]) {
    if (key_length != kKhazadKeySize) {
        return 3;
    }
    if (rounds != kKhazadRounds && rounds != 0) {
        return 4;
    }

    unsigned long long previous = ReadKhazadBlock(key);
    unsigned long long current = ReadKhazadBlock(key + 8);
    for (int round = 0; round <= kKhazadRounds; ++round) {
        const unsigned long long next =
            previous ^ kKhazadC[round] ^ KhazadRound(current);
        round_keys[round] = next;
        previous = current;
        current = next;
    }

    round_keys[9] = round_keys[8];
    for (int round = 1; round <= 7; ++round) {
        round_keys[round + 9] =
            KhazadInverseRoundKey(round_keys[kKhazadRounds - round]);
    }
    round_keys[17] = round_keys[0];
    return 0;
}

void KhazadCryptBlock(const unsigned char* input, unsigned char* output,
                      const unsigned long long* round_keys) {
    unsigned long long state = ReadKhazadBlock(input) ^ round_keys[0];
    for (int round = 1; round <= 7; ++round) {
        state = KhazadRound(state) ^ round_keys[round];
    }
    WriteKhazadBlock(output, KhazadFinalRound(state, round_keys[8]));
}

int TransformKhazad(const unsigned long long* round_keys, bool initialized,
                    const unsigned char* input, int input_length,
                    unsigned char* output, int output_capacity,
                    bool decrypt, int block_error, bool reject_empty) {
    if (!initialized) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (reject_empty && input_length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if ((input_length % kKhazadBlockSize) != 0) {
        return block_error;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const unsigned long long* schedule =
        decrypt ? round_keys + 9 : round_keys;
    for (int offset = 0; offset < input_length; offset += kKhazadBlockSize) {
        KhazadCryptBlock(input + offset, output + offset, schedule);
    }
    return CRYPTO_RESULT_OK;
}

}  // namespace

// IDA_EA: 0x809AA54
// IDA_NAME: _ZN14CNCryptoKhazadC2Ev
// DEMANGLED: CNCryptoKhazad::CNCryptoKhazad(void)
CNCryptoKhazad::CNCryptoKhazad()
    : ICryptoGraph(CRYPTO_GRAPH_KHAZAD), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809AAF2
// IDA_NAME: _ZN14CNCryptoKhazadD2Ev
// DEMANGLED: CNCryptoKhazad::~CNCryptoKhazad()
// IDA_EA: 0x809AB46
// IDA_NAME: _ZN14CNCryptoKhazadD0Ev
// DEMANGLED: CNCryptoKhazad::~CNCryptoKhazad() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoKhazad::~CNCryptoKhazad() {}

// IDA_EA: 0x809AB64
// IDA_NAME: _ZN14CNCryptoKhazadC2ERKS_
// DEMANGLED: CNCryptoKhazad::CNCryptoKhazad(CNCryptoKhazad const&)
CNCryptoKhazad::CNCryptoKhazad(const CNCryptoKhazad& rhs)
    : ICryptoGraph(rhs), round_keys_(), initialized_(false) {}

// IDA_EA: 0x809AB88
// IDA_NAME: _ZN14CNCryptoKhazadaSERKS_
// DEMANGLED: CNCryptoKhazad::operator=(CNCryptoKhazad const&)
CNCryptoKhazad& CNCryptoKhazad::operator=(const CNCryptoKhazad& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809ABA8
// IDA_NAME: _ZN14CNCryptoKhazad10InitializeEPKhi
// DEMANGLED: CNCryptoKhazad::Initialize(unsigned char const*,int)
int CNCryptoKhazad::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length < GetKeySize()) {
        initialized_ = false;
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    const int rc = KhazadSetup(key, GetKeySize(), kKhazadRounds, round_keys_);
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

// IDA_EA: 0x809AB90
// IDA_NAME: _ZN14CNCryptoKhazad7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoKhazad::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoKhazad::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809AC50
// IDA_NAME: _ZN14CNCryptoKhazad7EncryptEPhi
// DEMANGLED: CNCryptoKhazad::Encrypt(unsigned char *,int)
int CNCryptoKhazad::Encrypt(unsigned char* data, int length) {
    return TransformKhazad(round_keys_, initialized_, data, length, data, length,
                           false, CRYPTO_RESULT_INVALID_BLOCK_SIZE, true);
}

// IDA_EA: 0x809AD28
// IDA_NAME: _ZN14CNCryptoKhazad7DecryptEPhi
// DEMANGLED: CNCryptoKhazad::Decrypt(unsigned char *,int)
int CNCryptoKhazad::Decrypt(unsigned char* data, int length) {
    return TransformKhazad(round_keys_, initialized_, data, length, data, length,
                           true, CRYPTO_RESULT_INVALID_BLOCK_SIZE, true);
}

// IDA_EA: 0x809AE00
// IDA_NAME: _ZN14CNCryptoKhazad7EncryptEPKhiPhi
// DEMANGLED: CNCryptoKhazad::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoKhazad::Encrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return TransformKhazad(round_keys_, initialized_, input, input_length,
                           output, output_capacity, false,
                           CRYPTO_RESULT_INVALID_BLOCK_SIZE, false);
}

// IDA_EA: 0x809AEF2
// IDA_NAME: _ZN14CNCryptoKhazad7DecryptEPKhiPhi
// DEMANGLED: CNCryptoKhazad::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoKhazad::Decrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return TransformKhazad(round_keys_, initialized_, input, input_length,
                           output, output_capacity, true,
                           CRYPTO_RESULT_BUFFER_TOO_SMALL, false);
}

// IDA_EA: 0x809AFEE
// IDA_NAME: _ZNK14CNCryptoKhazad10GetKeySizeEv
// DEMANGLED: CNCryptoKhazad::GetKeySize(void)const
int CNCryptoKhazad::GetKeySize() const {
    return kKhazadKeySize;
}

// IDA_EA: 0x809AFE4
// IDA_NAME: _ZNK14CNCryptoKhazad12GetBlockSizeEv
// DEMANGLED: CNCryptoKhazad::GetBlockSize(void)const
int CNCryptoKhazad::GetBlockSize() const {
    return kKhazadBlockSize;
}
