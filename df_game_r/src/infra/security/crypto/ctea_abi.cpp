#include "infra/security/crypto/ctea_abi.h"

#include <cstdio>
#include <cstring>

#include "infra/security/crypto/csha1_abi.h"
#include "infra/security/crypto/crypto_types.h"

namespace {

uint32_t ReadBigEndian(const unsigned char* value) {
    return (static_cast<uint32_t>(value[0]) << 24) |
           (static_cast<uint32_t>(value[1]) << 16) |
           (static_cast<uint32_t>(value[2]) << 8) |
           static_cast<uint32_t>(value[3]);
}

void WriteBigEndian(uint32_t value, unsigned char* output) {
    output[0] = static_cast<unsigned char>(value >> 24);
    output[1] = static_cast<unsigned char>(value >> 16);
    output[2] = static_cast<unsigned char>(value >> 8);
    output[3] = static_cast<unsigned char>(value);
}

void XorBlock(unsigned char* destination,
              const unsigned char* source,
              uint32_t length) {
    for (uint32_t index = 0; index < length; ++index) {
        destination[index] ^= source[index];
    }
}

}  // namespace

const char CTEA::sm_chain0[8] = {};

// IDA_EA: 0x80C32F4
// IDA_NAME: _ZN4CTEAC2Ev
CTEA::CTEA()
    : initialized_(false),
      reserved_05_{},
      block_size_(8),
      key_size_(16),
      mode_(0),
      padding_(0),
      key_bytes_{},
      key_words_{},
      chain0_{},
      chain_{} {}

// IDA_EA: 0x80C3D70 / 0x80C3DA0
// IDA_NAME: _ZN4CTEAD2Ev / _ZN4CTEAD0Ev
CTEA::~CTEA() {}

// IDA_EA: 0x80C339C
// IDA_NAME: _ZN4CTEA10InitializeEPKciS1_ii
int CTEA::Initialize(const char* key,
                     int key_length,
                     const char* chain0,
                     int mode,
                     int padding) {
    if (!key) {
        return CRYPTO_RESULT_NULL_KEY;
    }
    if (key_length <= 0) {
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }
    if (mode > 2) {
        return CRYPTO_RESULT_INVALID_MODE;
    }
    if (padding > 2) {
        return CRYPTO_RESULT_INVALID_PADDING;
    }

    mode_ = static_cast<uint32_t>(mode);
    padding_ = static_cast<uint32_t>(padding);

    unsigned char normalized_key[16];
    for (uint32_t index = 0; index < key_size_; ++index) {
        normalized_key[index] =
            static_cast<unsigned char>(key[index % key_length]);
    }

    const bool same_chain =
        initialized_ && std::memcmp(chain0_, chain0, block_size_) == 0;
    const bool same_key =
        initialized_ && std::memcmp(key_bytes_, normalized_key, key_size_) == 0;

    if (same_chain) {
        std::memcpy(chain_, chain0_, block_size_);
    } else {
        std::memcpy(chain0_, chain0, block_size_);
        std::memcpy(chain_, chain0, block_size_);
    }

    if (!same_key) {
        std::memcpy(key_bytes_, normalized_key, key_size_);
        for (int index = 0; index < 4; ++index) {
            key_words_[index] = ReadBigEndian(key_bytes_ + index * 4);
        }
        initialized_ = true;
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x80C35A8
// IDA_NAME: _ZN4CTEA10ResetChainEv
int CTEA::ResetChain() {
    if (!initialized_) {
        return CRYPTO_RESULT_NOT_INITIALIZED;
    }
    std::memcpy(chain_, chain0_, block_size_);
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x80C35EC
// IDA_NAME: _ZN4CTEA9SignatureEPc
int CTEA::Signature(char* output) {
    char signature_input[23] = {};
    std::strcpy(signature_input, "TEA");
    const size_t prefix_length = std::strlen(signature_input);
    std::memcpy(signature_input + prefix_length, key_bytes_, key_size_);
    std::sprintf(signature_input + prefix_length + key_size_, "%d%d",
                 static_cast<int>(mode_), static_cast<int>(padding_));

    CSHA1 sha;
    sha.Update(reinterpret_cast<const uint8_t*>(signature_input),
               static_cast<uint32_t>(std::strlen(signature_input)));
    sha.Final();
    return sha.GetHash(reinterpret_cast<uint8_t*>(output));
}

// IDA_EA: 0x80C3724
// IDA_NAME: _ZN4CTEA12EncryptBlockEPKhPh
void CTEA::EncryptBlock(const unsigned char* input, unsigned char* output) {
    uint32_t left = ReadBigEndian(input);
    uint32_t right = ReadBigEndian(input + 4);
    uint32_t sum = 0;
    for (int round = 0; round < 32; ++round) {
        left += (((right << 4) ^ (right >> 5)) + right) ^
                (sum + key_words_[sum & 3]);
        sum -= 0x61C88647u;
        right += (((left << 4) ^ (left >> 5)) + left) ^
                 (sum + key_words_[(sum >> 11) & 3]);
    }
    WriteBigEndian(left, output);
    WriteBigEndian(right, output + 4);
}

// IDA_EA: 0x80C3804
// IDA_NAME: _ZN4CTEA12DecryptBlockEPKhPh
void CTEA::DecryptBlock(const unsigned char* input, unsigned char* output) {
    uint32_t left = ReadBigEndian(input);
    uint32_t right = ReadBigEndian(input + 4);
    uint32_t sum = 0xc6ef3720u;
    for (int round = 0; round < 32; ++round) {
        right -= (((left << 4) ^ (left >> 5)) + left) ^
                 (sum + key_words_[(sum >> 11) & 3]);
        sum += 0x61C88647u;
        left -= (((right << 4) ^ (right >> 5)) + right) ^
                (sum + key_words_[sum & 3]);
    }
    WriteBigEndian(left, output);
    WriteBigEndian(right, output + 4);
}

// IDA_EA: 0x80C38E4
// IDA_NAME: _ZN4CTEA7EncryptEPKcPcj
int CTEA::Encrypt(const char* input, char* output, uint32_t length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NOT_INITIALIZED;
    }
    if (!length || length % block_size_) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }

    for (uint32_t offset = 0; offset < length; offset += block_size_) {
        const unsigned char* source =
            reinterpret_cast<const unsigned char*>(input + offset);
        unsigned char* destination = reinterpret_cast<unsigned char*>(output + offset);
        if (mode_ == 1) {
            unsigned char scratch[8];
            std::memcpy(scratch, source, block_size_);
            XorBlock(scratch, chain_, block_size_);
            EncryptBlock(scratch, destination);
            std::memcpy(chain_, destination, block_size_);
        } else if (mode_ == 2) {
            unsigned char stream[8];
            EncryptBlock(chain_, stream);
            for (uint32_t index = 0; index < block_size_; ++index) {
                destination[index] = source[index] ^ stream[index];
            }
            std::memcpy(chain_, destination, block_size_);
        } else {
            EncryptBlock(source, destination);
        }
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x80C3AFE
// IDA_NAME: _ZN4CTEA7DecryptEPKcPcj
int CTEA::Decrypt(const char* input, char* output, uint32_t length) {
    if (!initialized_) {
        return CRYPTO_RESULT_NOT_INITIALIZED;
    }
    if (!length || length % block_size_) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }

    for (uint32_t offset = 0; offset < length; offset += block_size_) {
        const unsigned char* source =
            reinterpret_cast<const unsigned char*>(input + offset);
        unsigned char* destination = reinterpret_cast<unsigned char*>(output + offset);
        if (mode_ == 1) {
            DecryptBlock(source, destination);
            XorBlock(destination, chain_, block_size_);
            std::memcpy(chain_, source, block_size_);
        } else if (mode_ == 2) {
            unsigned char stream[8];
            EncryptBlock(chain_, stream);
            for (uint32_t index = 0; index < block_size_; ++index) {
                destination[index] = source[index] ^ stream[index];
            }
            std::memcpy(chain_, source, block_size_);
        } else {
            DecryptBlock(source, destination);
        }
    }
    return CRYPTO_RESULT_OK;
}
