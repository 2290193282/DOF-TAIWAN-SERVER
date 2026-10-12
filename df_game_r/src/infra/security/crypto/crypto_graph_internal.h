#ifndef OLDSRV_SECURITY_CRYPTO_GRAPH_INTERNAL_H_
#define OLDSRV_SECURITY_CRYPTO_GRAPH_INTERNAL_H_

#include "infra/security/crypto/crypto_types.h"

namespace crypto_detail {

// Restored CTEA state. IDA shows initialized flag, block/key size, mode,
// padding, repeated key bytes, four key words, initial chain, and mutable chain.
struct TeaContext {
    bool initialized;  // Old offset +0x04 initialized flag.
    unsigned int block_size;  // Old offset +0x08, always 8 inside CTEA.
    unsigned int key_size;  // Old offset +0x0C, always 16.
    CRYPTO_MODE_TYPE mode;  // Old offset +0x10, wrapper accepts ECB/CBC/OFB.
    unsigned int padding;  // Old offset +0x14, retained for layout semantics.
    unsigned char key_bytes[16];  // Repeated input key material.
    unsigned int key_words[4];  // Big-endian words used by TEA rounds.
    unsigned char chain0[8];  // Initial all-zero chain.
    unsigned char chain[8];  // Mutable CBC/OFB chain.

    TeaContext();
};

// Restored MULTI2 state. IDA multi2_setup writes round count and eight
// generated schedule words.
struct Multi2Context {
    int rounds;  // Number of MULTI2 pi rounds, restored default is 128.
    unsigned int subkeys[8];  // Generated from the 40-byte key.
    bool initialized;  // True after key schedule generation succeeds.

    Multi2Context();
};

unsigned int ReadLE32(const unsigned char* data);
void WriteLE32(unsigned char* data, unsigned int value);
unsigned int ReadBE32(const unsigned char* data);
void WriteBE32(unsigned char* data, unsigned int value);
unsigned int RotateLeft32(unsigned int value, unsigned int bits);
unsigned int RotateRight32(unsigned int value, unsigned int bits);

void ShiftTransform(const unsigned char* input, int input_length,
                    unsigned char* output, unsigned int shift_bits,
                    unsigned int xor_mask, bool decrypt);
int TransformTea(TeaContext* context, const unsigned char* input,
                 int input_length, unsigned char* output, int output_capacity,
                 bool decrypt);
void Multi2BuildSchedule(const unsigned int seed[2], const unsigned int key[8],
                         unsigned int subkeys[8]);
int TransformMulti2(Multi2Context* context, const unsigned char* input,
                    int input_length, unsigned char* output,
                    int output_capacity, bool decrypt);

}  // namespace crypto_detail

#endif  // OLDSRV_SECURITY_CRYPTO_GRAPH_INTERNAL_H_
