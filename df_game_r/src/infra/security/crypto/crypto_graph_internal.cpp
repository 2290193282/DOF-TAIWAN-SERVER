#include "infra/security/crypto/crypto_graph_internal.h"

#include <cstring>

namespace crypto_detail {

TeaContext::TeaContext()
    : initialized(false),
      block_size(8),
      key_size(16),
      mode(CRYPTO_MODE_ECB),
      padding(0),
      key_bytes(),
      key_words(),
      chain0(),
      chain() {}

Multi2Context::Multi2Context() : rounds(0), subkeys(), initialized(false) {}

unsigned int ReadLE32(const unsigned char* data) {
    return static_cast<unsigned int>(data[0]) |
           (static_cast<unsigned int>(data[1]) << 8) |
           (static_cast<unsigned int>(data[2]) << 16) |
           (static_cast<unsigned int>(data[3]) << 24);
}

void WriteLE32(unsigned char* data, unsigned int value) {
    data[0] = static_cast<unsigned char>(value & 0xFF);
    data[1] = static_cast<unsigned char>((value >> 8) & 0xFF);
    data[2] = static_cast<unsigned char>((value >> 16) & 0xFF);
    data[3] = static_cast<unsigned char>((value >> 24) & 0xFF);
}

unsigned int ReadBE32(const unsigned char* data) {
    return (static_cast<unsigned int>(data[0]) << 24) |
           (static_cast<unsigned int>(data[1]) << 16) |
           (static_cast<unsigned int>(data[2]) << 8) |
           static_cast<unsigned int>(data[3]);
}

void WriteBE32(unsigned char* data, unsigned int value) {
    data[0] = static_cast<unsigned char>((value >> 24) & 0xFF);
    data[1] = static_cast<unsigned char>((value >> 16) & 0xFF);
    data[2] = static_cast<unsigned char>((value >> 8) & 0xFF);
    data[3] = static_cast<unsigned char>(value & 0xFF);
}

unsigned int RotateLeft32(unsigned int value, unsigned int bits) {
    bits &= 31;
    if (!bits) {
        return value;
    }
    return (value << bits) | (value >> (32 - bits));
}

unsigned int RotateRight32(unsigned int value, unsigned int bits) {
    bits &= 31;
    if (!bits) {
        return value;
    }
    return (value >> bits) | (value << (32 - bits));
}

void ShiftTransform(const unsigned char* input, int input_length,
                    unsigned char* output, unsigned int shift_bits,
                    unsigned int xor_mask, bool decrypt) {
    for (int offset = 0; offset < input_length; offset += 4) {
        const unsigned int source = ReadLE32(input + offset);
        const unsigned int value = decrypt
            ? RotateLeft32(source ^ xor_mask, shift_bits)
            : (RotateRight32(source, shift_bits) ^ xor_mask);
        WriteLE32(output + offset, value);
    }
}

void TeaEncryptBlock(const TeaContext* context, const unsigned char* input,
                     unsigned char* output) {
    unsigned int left = ReadBE32(input);
    unsigned int right = ReadBE32(input + 4);
    unsigned int sum = 0;
    for (int round = 0; round < 32; ++round) {
        left += (((right << 4) ^ (right >> 5)) + right) ^
                (sum + context->key_words[sum & 3]);
        sum -= 0x61C88647U;
        right += (((left << 4) ^ (left >> 5)) + left) ^
                 (sum + context->key_words[(sum >> 11) & 3]);
    }
    WriteBE32(output, left);
    WriteBE32(output + 4, right);
}

void TeaDecryptBlock(const TeaContext* context, const unsigned char* input,
                     unsigned char* output) {
    unsigned int left = ReadBE32(input);
    unsigned int right = ReadBE32(input + 4);
    unsigned int sum = 0xC6EF3720U;
    for (int round = 0; round < 32; ++round) {
        right -= (((left << 4) ^ (left >> 5)) + left) ^
                 (sum + context->key_words[(sum >> 11) & 3]);
        sum += 0x61C88647U;
        left -= (((right << 4) ^ (right >> 5)) + right) ^
                (sum + context->key_words[sum & 3]);
    }
    WriteBE32(output, left);
    WriteBE32(output + 4, right);
}

void XorBlock(unsigned char* target, const unsigned char* source, int length) {
    for (int index = 0; index < length; ++index) {
        target[index] ^= source[index];
    }
}

void Multi2Pi1(unsigned int block[2]) {
    block[1] ^= block[0];
}

void Multi2Pi2(unsigned int block[2], const unsigned int key[4]) {
    unsigned int value = block[1] + key[0];
    value = value + RotateLeft32(value, 1) - 1;
    block[0] ^= value ^ RotateLeft32(value, 4);
}

void Multi2Pi3(unsigned int block[2], const unsigned int key[4]) {
    unsigned int value = block[0] + key[1];
    value = value + RotateLeft32(value, 2) + 1;
    value = key[2] + (RotateLeft32(value, 8) ^ value);
    value = RotateLeft32(value, 1) - value;
    block[1] ^= (value | block[0]) ^ RotateLeft32(value, 16);
}

void Multi2Pi4(unsigned int block[2], const unsigned int key[4]) {
    const unsigned int value = block[1] + key[3];
    block[0] ^= value + RotateLeft32(value, 2) + 1;
}

void Multi2BuildSchedule(const unsigned int seed[2], const unsigned int key[8],
                         unsigned int subkeys[8]) {
    unsigned int block[2] = {seed[0], seed[1]};
    int index = 0;
    Multi2Pi1(block);
    Multi2Pi2(block, key);
    subkeys[index++] = block[0];
    Multi2Pi3(block, key);
    subkeys[index++] = block[1];
    Multi2Pi4(block, key);
    subkeys[index++] = block[0];
    Multi2Pi1(block);
    subkeys[index++] = block[1];
    Multi2Pi2(block, key + 4);
    subkeys[index++] = block[0];
    Multi2Pi3(block, key + 4);
    subkeys[index++] = block[1];
    Multi2Pi4(block, key + 4);
    subkeys[index++] = block[0];
    Multi2Pi1(block);
    subkeys[index] = block[1];
}

void Multi2EncryptBlock(Multi2Context* context, const unsigned char* input,
                        unsigned char* output) {
    unsigned int block[2] = {ReadBE32(input), ReadBE32(input + 4)};
    int rounds_done = 0;
    for (int key_index = 0;; key_index ^= 4) {
        Multi2Pi1(block);
        if (++rounds_done == context->rounds) {
            break;
        }
        Multi2Pi2(block, context->subkeys + key_index);
        if (++rounds_done == context->rounds) {
            break;
        }
        Multi2Pi3(block, context->subkeys + key_index);
        if (++rounds_done == context->rounds) {
            break;
        }
        Multi2Pi4(block, context->subkeys + key_index);
        if (++rounds_done == context->rounds) {
            break;
        }
    }
    WriteBE32(output, block[0]);
    WriteBE32(output + 4, block[1]);
}

void Multi2DecryptBlock(Multi2Context* context, const unsigned char* input,
                        unsigned char* output) {
    unsigned int block[2] = {ReadBE32(input), ReadBE32(input + 4)};
    int key_index = (context->rounds & 1) ? 0 : 4;
    int rounds_left = context->rounds;
    while (rounds_left > 3) {
        Multi2Pi4(block, context->subkeys + key_index);
        --rounds_left;
        Multi2Pi3(block, context->subkeys + key_index);
        --rounds_left;
        Multi2Pi2(block, context->subkeys + key_index);
        --rounds_left;
        Multi2Pi1(block);
        --rounds_left;
        key_index ^= 4;
    }
    WriteBE32(output, block[0]);
    WriteBE32(output + 4, block[1]);
}

int TransformMulti2(Multi2Context* context, const unsigned char* input,
                    int input_length, unsigned char* output,
                    int output_capacity, bool decrypt) {
    if (context == 0 || !context->initialized) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length <= 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if ((input_length & 7) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += 8) {
        if (decrypt) {
            Multi2DecryptBlock(context, input + offset, output + offset);
        } else {
            Multi2EncryptBlock(context, input + offset, output + offset);
        }
    }
    return CRYPTO_RESULT_OK;
}

int TransformTea(TeaContext* context, const unsigned char* input,
                 int input_length, unsigned char* output, int output_capacity,
                 bool decrypt) {
    if (context == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (!context->initialized) {
        return CRYPTO_RESULT_NOT_INITIALIZED;
    }
    if (input_length <= 0 || (input_length % context->block_size) != 0) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }

    unsigned char temp[8];
    for (int offset = 0; offset < input_length; offset += context->block_size) {
        const unsigned char* read = input + offset;
        unsigned char* write = output + offset;
        if (context->mode == CRYPTO_MODE_CBC) {
            if (decrypt) {
                TeaDecryptBlock(context, read, write);
                XorBlock(write, context->chain, context->block_size);
                std::memcpy(context->chain, read, context->block_size);
            } else {
                std::memcpy(temp, read, context->block_size);
                XorBlock(temp, context->chain, context->block_size);
                TeaEncryptBlock(context, temp, write);
                std::memcpy(context->chain, write, context->block_size);
            }
        } else if (context->mode == CRYPTO_MODE_OFB) {
            TeaEncryptBlock(context, context->chain, temp);
            std::memcpy(context->chain, temp, context->block_size);
            std::memcpy(write, read, context->block_size);
            XorBlock(write, context->chain, context->block_size);
        } else {
            if (decrypt) {
                TeaDecryptBlock(context, read, write);
            } else {
                TeaEncryptBlock(context, read, write);
            }
        }
    }
    return CRYPTO_RESULT_OK;
}

}  // namespace crypto_detail
