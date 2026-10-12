#include "infra/security/crypto/arad_crypt.h"

#include "common/manager/dnff_lib.h"
#include "infra/log/format/trace.h"
#include "infra/security/crypto/crypto_rijndael.h"

#include <stdint.h>
#include <string.h>

namespace {

// The original IMethod prefix is the first 24 bytes of CRijndael.  The old
// encryptAES body invokes its concrete GetBlockSize and Pad logic directly.
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

namespace ARAD {
namespace ARAD_CRYPT {

// IDA_EA: 0x8184ABC
// IDA_NAME: _ZN4ARAD10ARAD_CRYPT10encryptAESEP9CRijndaelPcS3_
// DEMANGLED: ARAD::ARAD_CRYPT::encryptAES(CRijndael *,char *,char *)
char* encryptAES(CRijndael* crypt, char* source, char* destination) {
    unsigned char encrypted[132];
    const int source_length = static_cast<int>(strlen(source));
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

    LogManager::logFormat(1, "localjapan/Arad_Crypt.cpp", "encryptAES", 31,
                          "[%s][%d][%s]", "encryptAES", 31,
                          "Encrypt Error");
    return 0;
}

}  // namespace ARAD_CRYPT
}  // namespace ARAD
