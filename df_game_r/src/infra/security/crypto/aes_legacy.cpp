#include "common/manager/dnff_lib.h"
#include "infra/log/format/trace.h"
#include "infra/security/crypto/crypto_rijndael.h"

#include <stdint.h>
#include <string.h>

namespace {

// IMethod is the recovered CRijndael prefix.  encryptAES_ calls its concrete
// GetBlockSize/Pad bodies, which operate on precisely these fields.
struct RijndaelMethodPrefix {
    uint32_t vptr;
    uint8_t initialized;
    uint8_t pad_05_07[3];
    uint32_t block_size;
    uint32_t key_size;
    uint32_t mode;
    uint32_t padding;
};

int get_block_size(CRijndael* crypt) {
    const RijndaelMethodPrefix* const prefix =
        reinterpret_cast<const RijndaelMethodPrefix*>(crypt);
    return prefix->initialized == 1 ? static_cast<int>(prefix->block_size) : 0;
}

int pad(CRijndael* crypt, char* source, int source_length) {
    const RijndaelMethodPrefix* const prefix =
        reinterpret_cast<const RijndaelMethodPrefix*>(crypt);
    if (prefix->initialized != 1) {
        return 0;
    }

    const int remainder = source_length % static_cast<int>(prefix->block_size);
    if (!remainder) {
        return source_length;
    }

    const int padding_count = static_cast<int>(prefix->block_size) - remainder;
    char* const output = source + source_length;
    switch (prefix->padding) {
    case 1:
        memset(output, ' ', static_cast<size_t>(padding_count));
        break;
    case 2:
        memset(output, padding_count, static_cast<size_t>(padding_count));
        break;
    default:
        memset(output, 0, static_cast<size_t>(padding_count));
        break;
    }
    return source_length + padding_count;
}

}  // namespace

// IDA_EA: 0x846C398
// IDA_NAME: _Z11encryptAES_P9CRijndaelPcS1_
// DEMANGLED: encryptAES_(CRijndael *,char *,char *)
char* encryptAES_(CRijndael* crypt, char* source, char* destination) {
    unsigned char encrypted[132];
    int source_length = static_cast<int>(strlen(source));
    int encrypted_length = 0;
    const int block_size = get_block_size(crypt);

    if (block_size < 0) {
        encrypted_length = source_length;
    } else if (source_length % block_size) {
        encrypted_length = block_size * (source_length / block_size + 1);
    } else {
        encrypted_length = source_length;
    }

    if (block_size >= 0) {
        pad(crypt, source, source_length);
    }

    if (crypt->Encrypt(source, reinterpret_cast<char*>(encrypted),
                       static_cast<unsigned int>(encrypted_length)) ==
        0x6FFFFFFF) {
        const int output_length = 2 * encrypted_length + 1;
        DNFFLib::Binary2Hex(encrypted, encrypted_length, destination);
        destination[output_length] = '\0';
        return destination;
    }

    LogManager::logFormat(1, "DesignPattern.cpp", "encryptAES_", 158,
                          "[%s][%d][%s]", "encryptAES_", 158,
                          "Encrypt Error");
    return 0;
}
