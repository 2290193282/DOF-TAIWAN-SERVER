#include "infra/security/crypto/crypto_blowfish.h"

#include <algorithm>
#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


namespace {

const int kBlowFishKeySize = 56;
const int kBlowFishBlockSize = 8;

#include "infra/security/crypto/crypto_blowfish_tables.inc"

unsigned int BlowFishF(const unsigned int s[4][256], unsigned int value) {
    const unsigned int a = (value >> 24) & 0xFFU;
    const unsigned int b = (value >> 16) & 0xFFU;
    const unsigned int c = (value >> 8) & 0xFFU;
    const unsigned int d = value & 0xFFU;
    return ((s[0][a] + s[1][b]) ^ s[2][c]) + s[3][d];
}

void BlowFishEncryptWords(const unsigned int p[18], const unsigned int s[4][256],
                          unsigned int* left, unsigned int* right) {
    unsigned int l = *left;
    unsigned int r = *right;
    for (int round = 0; round < 16; ++round) {
        l ^= p[round];
        r ^= BlowFishF(s, l);
        const unsigned int temp = l;
        l = r;
        r = temp;
    }
    const unsigned int temp = l;
    l = r;
    r = temp;
    r ^= p[16];
    l ^= p[17];
    *left = l;
    *right = r;
}

void BlowFishDecryptWords(const unsigned int p[18], const unsigned int s[4][256],
                          unsigned int* left, unsigned int* right) {
    unsigned int l = *left;
    unsigned int r = *right;
    for (int round = 17; round > 1; --round) {
        l ^= p[round];
        r ^= BlowFishF(s, l);
        const unsigned int temp = l;
        l = r;
        r = temp;
    }
    const unsigned int temp = l;
    l = r;
    r = temp;
    r ^= p[1];
    l ^= p[0];
    *left = l;
    *right = r;
}

void BlowFishEncryptBlock(const unsigned int p[18], const unsigned int s[4][256],
                          const unsigned char* input,
                          unsigned char* output) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);
    BlowFishEncryptWords(p, s, &left, &right);
    crypto_detail::WriteBE32(output, left);
    crypto_detail::WriteBE32(output + 4, right);
}

void BlowFishDecryptBlock(const unsigned int p[18], const unsigned int s[4][256],
                          const unsigned char* input,
                          unsigned char* output) {
    unsigned int left = crypto_detail::ReadBE32(input);
    unsigned int right = crypto_detail::ReadBE32(input + 4);
    BlowFishDecryptWords(p, s, &left, &right);
    crypto_detail::WriteBE32(output, left);
    crypto_detail::WriteBE32(output + 4, right);
}

void BlowFishInitializeContext(unsigned int init_chain[2],
                               unsigned int work_chain[2],
                               unsigned int p[18], unsigned int s[4][256],
                               const unsigned char* key, int key_length) {
    init_chain[0] = 0;
    init_chain[1] = 0;
    work_chain[0] = 0;
    work_chain[1] = 0;
    std::memcpy(p, kBlowFishInitP, 18 * sizeof(unsigned int));
    std::memcpy(s, kBlowFishInitS, 4 * 256 * sizeof(unsigned int));

    const int capped_length = std::min(key_length, kBlowFishKeySize);
    int offset = 0;
    for (int index = 0; index < 18; ++index) {
        unsigned int word = 0;
        for (int byte_index = 0; byte_index < 4; ++byte_index) {
            word = (word << 8) | key[offset++];
            if (offset == capped_length) {
                offset = 0;
            }
        }
        p[index] ^= word;
    }

    unsigned int left = 0;
    unsigned int right = 0;
    for (int index = 0; index < 18; index += 2) {
        BlowFishEncryptWords(p, s, &left, &right);
        p[index] = left;
        p[index + 1] = right;
    }
    for (int box = 0; box < 4; ++box) {
        for (int index = 0; index < 256; index += 2) {
            BlowFishEncryptWords(p, s, &left, &right);
            s[box][index] = left;
            s[box][index + 1] = right;
        }
    }
}

int BlowFishTransform(const unsigned int p[18], const unsigned int s[4][256],
                      unsigned int work_chain[2], CRYPTO_MODE_TYPE mode,
                      const unsigned char* input, int input_length,
                      unsigned char* output, int output_capacity,
                      bool decrypt) {
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (input_length <= 0 || (input_length & 7) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }

    unsigned char temp[kBlowFishBlockSize];
    unsigned char chain[kBlowFishBlockSize];
    crypto_detail::WriteBE32(chain, work_chain[0]);
    crypto_detail::WriteBE32(chain + 4, work_chain[1]);

    for (int offset = 0; offset < input_length; offset += kBlowFishBlockSize) {
        const unsigned char* read = input + offset;
        unsigned char* write = output + offset;
        if (mode == CRYPTO_MODE_CBC) {
            if (decrypt) {
                BlowFishDecryptBlock(p, s, read, write);
                for (int index = 0; index < kBlowFishBlockSize; ++index) {
                    write[index] ^= chain[index];
                    chain[index] = read[index];
                }
            } else {
                for (int index = 0; index < kBlowFishBlockSize; ++index) {
                    temp[index] = read[index] ^ chain[index];
                }
                BlowFishEncryptBlock(p, s, temp, write);
                std::memcpy(chain, write, kBlowFishBlockSize);
            }
        } else if (mode == CRYPTO_MODE_OFB) {
            BlowFishEncryptBlock(p, s, chain, chain);
            for (int index = 0; index < kBlowFishBlockSize; ++index) {
                write[index] = read[index] ^ chain[index];
            }
        } else if (mode == CRYPTO_MODE_ECB) {
            if (decrypt) {
                BlowFishDecryptBlock(p, s, read, write);
            } else {
                BlowFishEncryptBlock(p, s, read, write);
            }
        } else {
            return CRYPTO_RESULT_UNSUPPORTED;
        }
    }

    work_chain[0] = crypto_detail::ReadBE32(chain);
    work_chain[1] = crypto_detail::ReadBE32(chain + 4);
    return CRYPTO_RESULT_OK;
}

}  // namespace

// IDA_EA: 0x809980C
// IDA_NAME: _ZN16CNCryptoBlowFishC2Ev
// DEMANGLED: CNCryptoBlowFish::CNCryptoBlowFish(void)
CNCryptoBlowFish::CNCryptoBlowFish()
    : ICryptoGraph(CRYPTO_GRAPH_BLOWFISH),
      init_chain_(),
      work_chain_(),
      p_(),
      s_(),
      initialized_(false) {}

// IDA_EA: 0x80998F8
// IDA_NAME: _ZN16CNCryptoBlowFishD2Ev
// DEMANGLED: CNCryptoBlowFish::~CNCryptoBlowFish()
// IDA_EA: 0x809994A
// IDA_NAME: _ZN16CNCryptoBlowFishD0Ev
// DEMANGLED: CNCryptoBlowFish::~CNCryptoBlowFish() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoBlowFish::~CNCryptoBlowFish() {}

// IDA_EA: 0x8099968
// IDA_NAME: _ZN16CNCryptoBlowFishC2ERKS_
// DEMANGLED: CNCryptoBlowFish::CNCryptoBlowFish(CNCryptoBlowFish const&)
CNCryptoBlowFish::CNCryptoBlowFish(const CNCryptoBlowFish& rhs)
    : ICryptoGraph(rhs),
      init_chain_(),
      work_chain_(),
      p_(),
      s_(),
      initialized_(false) {}

// IDA_EA: 0x809998C
// IDA_NAME: _ZN16CNCryptoBlowFishaSERKS_
// DEMANGLED: CNCryptoBlowFish::operator=(CNCryptoBlowFish const&)
CNCryptoBlowFish& CNCryptoBlowFish::operator=(const CNCryptoBlowFish& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x80999A8
// IDA_NAME: _ZN16CNCryptoBlowFish10InitializeEPKhi
// DEMANGLED: CNCryptoBlowFish::Initialize(unsigned char const*,int)
int CNCryptoBlowFish::Initialize(const unsigned char* key, int length) {
    if (length <= 0 || key == 0) {
        initialized_ = false;
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    unsigned char padded_key[kBlowFishKeySize];
    if (length <= 55) {
        std::memset(padded_key, 0, sizeof(padded_key));
        std::memcpy(padded_key, key, length);
        BlowFishInitializeContext(init_chain_, work_chain_, p_, s_, padded_key,
                                  kBlowFishKeySize);
    } else {
        BlowFishInitializeContext(init_chain_, work_chain_, p_, s_, key,
                                  kBlowFishKeySize);
    }

    key_.assign(key, key + std::min(length, GetKeySize()));
    initialized_ = true;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x8099994
// IDA_NAME: _ZN16CNCryptoBlowFish7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoBlowFish::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoBlowFish::SetMode(CRYPTO_MODE_TYPE mode) {
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x8099B2A
// IDA_NAME: _ZN16CNCryptoBlowFish7EncryptEPhi
// DEMANGLED: CNCryptoBlowFish::Encrypt(unsigned char *,int)
int CNCryptoBlowFish::Encrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    return BlowFishTransform(p_, s_, work_chain_, mode_, data, length, data,
                             length, false);
}

// IDA_EA: 0x8099C20
// IDA_NAME: _ZN16CNCryptoBlowFish7DecryptEPhi
// DEMANGLED: CNCryptoBlowFish::Decrypt(unsigned char *,int)
int CNCryptoBlowFish::Decrypt(unsigned char* data, int length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    return BlowFishTransform(p_, s_, work_chain_, mode_, data, length, data,
                             length, true);
}

// IDA_EA: 0x8099D16
// IDA_NAME: _ZN16CNCryptoBlowFish7EncryptEPKhiPhi
// DEMANGLED: CNCryptoBlowFish::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoBlowFish::Encrypt(const unsigned char* input, int input_length,
                              unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    const CRYPTO_MODE_TYPE ida_mode =
        (mode_ == CRYPTO_MODE_OFB) ? CRYPTO_MODE_CBC : mode_;
    return BlowFishTransform(p_, s_, work_chain_, ida_mode, input, input_length,
                             output, output_capacity, false);
}

// IDA_EA: 0x8099DFC
// IDA_NAME: _ZN16CNCryptoBlowFish7DecryptEPKhiPhi
// DEMANGLED: CNCryptoBlowFish::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoBlowFish::Decrypt(const unsigned char* input, int input_length,
                              unsigned char* output, int output_capacity) {
    if (!initialized_) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    return BlowFishTransform(p_, s_, work_chain_, mode_, input, input_length,
                             output, output_capacity, true);
}

// IDA_EA: 0x8099EE2
// IDA_NAME: _ZNK16CNCryptoBlowFish10GetKeySizeEv
// DEMANGLED: CNCryptoBlowFish::GetKeySize(void)const
int CNCryptoBlowFish::GetKeySize() const {
    return kBlowFishKeySize;
}

// IDA_EA: 0x8099EEC
// IDA_NAME: _ZNK16CNCryptoBlowFish12GetBlockSizeEv
// DEMANGLED: CNCryptoBlowFish::GetBlockSize(void)const
int CNCryptoBlowFish::GetBlockSize() const {
    return kBlowFishBlockSize;
}
