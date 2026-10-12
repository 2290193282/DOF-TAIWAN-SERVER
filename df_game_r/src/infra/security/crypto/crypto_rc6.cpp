#include "infra/security/crypto/crypto_rc6.h"

#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kRc6KeySize = 60;
const int kRc6SetupKeyBytes = 32;
const int kRc6BlockSize = 16;
const int kRc6ScheduleBytes = 44;

void Rc6KeySetup(CNCryptoRc6Context& context, int key_bytes) {
    unsigned int l_words[8];
    std::memset(l_words, 0, sizeof(l_words));

    const int word_count = (key_bytes + 3) / 4;
    l_words[word_count - 1] = 0;
    for (int index = key_bytes - 1; index >= 0; --index) {
        l_words[index / 4] =
            (l_words[index / 4] << 8) + context.key_material[index];
    }

    unsigned char* const schedule = context.round_schedule;
    schedule[0] = 99;
    for (int index = 1; index <= 43; ++index) {
        schedule[index] = static_cast<unsigned char>(schedule[index - 1] - 71);
    }

    int l_index = 0;
    int s_index = 0;
    unsigned int a = 0;
    unsigned int b = 0;
    int mix_count = 44;
    if (word_count > mix_count) {
        mix_count = word_count;
    }
    mix_count *= 3;

    for (int round = 1; round <= mix_count; ++round) {
        schedule[s_index] = static_cast<unsigned char>(
            crypto_detail::RotateRight32(a + b + schedule[s_index], 29));
        b = schedule[s_index];
        l_words[l_index] = crypto_detail::RotateLeft32(
            a + b + l_words[l_index], (a + b) & 0x1F);
        a = l_words[l_index];
        s_index = (s_index + 1) % kRc6ScheduleBytes;
        l_index = (l_index + 1) % word_count;
    }
}

void Rc6BlockEncrypt(const CNCryptoRc6Context& context,
                     const unsigned char* input, unsigned char* output) {
    const unsigned char* const schedule = context.round_schedule;
    unsigned int a = crypto_detail::ReadLE32(input);
    unsigned int b = schedule[0] + crypto_detail::ReadLE32(input + 4);
    unsigned int c = crypto_detail::ReadLE32(input + 8);
    unsigned int d = schedule[1] + crypto_detail::ReadLE32(input + 12);

    for (int round = 2; round <= 40; round += 2) {
        const unsigned int t = crypto_detail::RotateRight32(b * (2 * b + 1), 27);
        const unsigned int u = crypto_detail::RotateRight32(d * (2 * d + 1), 27);
        const unsigned int next_a =
            crypto_detail::RotateLeft32(t ^ a, u & 0x1F) + schedule[round];
        a = b;
        b = crypto_detail::RotateLeft32(u ^ c, t & 0x1F) +
            schedule[round + 1];
        c = d;
        d = next_a;
    }

    crypto_detail::WriteLE32(output, schedule[42] + a);
    crypto_detail::WriteLE32(output + 4, b);
    crypto_detail::WriteLE32(output + 8, schedule[43] + c);
    crypto_detail::WriteLE32(output + 12, d);
}

void Rc6BlockDecrypt(const CNCryptoRc6Context& context,
                     const unsigned char* input, unsigned char* output) {
    const unsigned char* const schedule = context.round_schedule;
    unsigned int b = crypto_detail::ReadLE32(input + 4);
    unsigned int d = crypto_detail::ReadLE32(input + 12);
    unsigned int c = crypto_detail::ReadLE32(input + 8) - schedule[43];
    unsigned int a = crypto_detail::ReadLE32(input) - schedule[42];

    for (int round = 40; round > 1; round -= 2) {
        const unsigned int old_d = d;
        d = c;
        const unsigned int old_b = b;
        b = a;
        const unsigned int u = crypto_detail::RotateRight32(d * (2 * d + 1), 27);
        const unsigned int t = crypto_detail::RotateRight32(b * (2 * b + 1), 27);
        c = u ^ crypto_detail::RotateRight32(
                    old_b - schedule[round + 1], t & 0x1F);
        a = t ^ crypto_detail::RotateRight32(
                    old_d - schedule[round], u & 0x1F);
    }

    crypto_detail::WriteLE32(output, a);
    crypto_detail::WriteLE32(output + 4, b - schedule[0]);
    crypto_detail::WriteLE32(output + 8, c);
    crypto_detail::WriteLE32(output + 12, d - schedule[1]);
}

}  // namespace

// IDA_EA: 0x809BB40
// IDA_NAME: _ZN11CNCryptoRc6C2Ev
// DEMANGLED: CNCryptoRc6::CNCryptoRc6(void)
CNCryptoRc6::CNCryptoRc6()
    : ICryptoGraph(CRYPTO_GRAPH_RC6), context_(), initialized_(false) {}

// IDA_EA: 0x809BBDE
// IDA_NAME: _ZN11CNCryptoRc6D2Ev
// DEMANGLED: CNCryptoRc6::~CNCryptoRc6()
// IDA_EA: 0x809BC30
// IDA_NAME: _ZN11CNCryptoRc6D0Ev
// DEMANGLED: CNCryptoRc6::~CNCryptoRc6() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoRc6::~CNCryptoRc6() {}

// IDA_EA: 0x809BC4E
// IDA_NAME: _ZN11CNCryptoRc6C2ERKS_
// DEMANGLED: CNCryptoRc6::CNCryptoRc6(CNCryptoRc6 const&)
CNCryptoRc6::CNCryptoRc6(const CNCryptoRc6& rhs)
    : ICryptoGraph(rhs), context_(), initialized_(false) {}

// IDA_EA: 0x809BC72
// IDA_NAME: _ZN11CNCryptoRc6aSERKS_
// DEMANGLED: CNCryptoRc6::operator=(CNCryptoRc6 const&)
CNCryptoRc6& CNCryptoRc6::operator=(const CNCryptoRc6& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809BC84
// IDA_NAME: _ZN11CNCryptoRc610InitializeEPKhi
// DEMANGLED: CNCryptoRc6::Initialize(unsigned char const*,int)
int CNCryptoRc6::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length <= 59) {
        initialized_ = false;
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    std::memcpy(context_.key_material, key, kRc6SetupKeyBytes);
    std::memcpy(context_.round_schedule, key + kRc6SetupKeyBytes,
                kRc6ScheduleBytes);
    Rc6KeySetup(context_, kRc6SetupKeyBytes);
    key_.assign(key, key + GetKeySize());
    initialized_ = true;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809BC7A
// IDA_NAME: _ZN11CNCryptoRc67SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoRc6::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoRc6::SetMode(CRYPTO_MODE_TYPE /*mode*/) {
    return CRYPTO_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x809BD06
// IDA_NAME: _ZN11CNCryptoRc67EncryptEPhi
// DEMANGLED: CNCryptoRc6::Encrypt(unsigned char *,int)
int CNCryptoRc6::Encrypt(unsigned char* /*data*/, int /*length*/) {
    return CRYPTO_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x809BD10
// IDA_NAME: _ZN11CNCryptoRc67DecryptEPhi
// DEMANGLED: CNCryptoRc6::Decrypt(unsigned char *,int)
int CNCryptoRc6::Decrypt(unsigned char* /*data*/, int /*length*/) {
    return CRYPTO_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x809BD1A
// IDA_NAME: _ZN11CNCryptoRc67EncryptEPKhiPhi
// DEMANGLED: CNCryptoRc6::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoRc6::Encrypt(const unsigned char* input, int input_length,
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
        Rc6BlockEncrypt(context_, input + offset, output + offset);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809BE06
// IDA_NAME: _ZN11CNCryptoRc67DecryptEPKhiPhi
// DEMANGLED: CNCryptoRc6::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoRc6::Decrypt(const unsigned char* input, int input_length,
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
        Rc6BlockDecrypt(context_, input + offset, output + offset);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809BEF2
// IDA_NAME: _ZNK11CNCryptoRc610GetKeySizeEv
// DEMANGLED: CNCryptoRc6::GetKeySize(void)const
int CNCryptoRc6::GetKeySize() const {
    return kRc6KeySize;
}

// IDA_EA: 0x809BEFC
// IDA_NAME: _ZNK11CNCryptoRc612GetBlockSizeEv
// DEMANGLED: CNCryptoRc6::GetBlockSize(void)const
int CNCryptoRc6::GetBlockSize() const {
    return kRc6BlockSize;
}
