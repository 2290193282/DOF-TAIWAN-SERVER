#include "infra/security/policy/checksum.h"

#include <cstdio>
#include <cstring>

// IDA_EA: 0x809E309
// IDA_NAME: _Z20CreateCryptoChecksum20CRYPTO_CHECKSUM_TYPE
// DEMANGLED: CreateCryptoChecksum(CRYPTO_CHECKSUM_TYPE)
IChecksum* CreateCryptoChecksum(CRYPTO_CHECKSUM_TYPE type) {
    if (type == CRYPTO_CHECKSUM_SHA256) {
        return new CNChecksumSha256();
    }
    if (type > CRYPTO_CHECKSUM_SHA256) {
        if (type == CRYPTO_CHECKSUM_SHA384) {
            return new CNChecksumSha384();
        }
        if (type == CRYPTO_CHECKSUM_SHA512) {
            return new CNChecksumSha512();
        }
    } else if (type == CRYPTO_CHECKSUM_MD5) {
        return new CNChecksumMd5();
    }
    return 0;
}

// IDA_EA: 0x809E423
// IDA_NAME: _Z21DestroyCryptoChecksumRP9IChecksum
// DEMANGLED: DestroyCryptoChecksum(IChecksum *&)
IChecksum** DestroyCryptoChecksum(IChecksum*& checksum) {
    if (checksum != 0) {
        delete checksum;
        checksum = 0;
        return &checksum;
    }
    return 0;
}

// IDA_EA: 0x808CC6A
// IDA_NAME: _Z18getSha256UsingSaltPKhiPc
// DEMANGLED: getSha256UsingSalt(unsigned char const*,int,char *)
int getSha256UsingSalt(const unsigned char* src, int length, char* output) {
    if (src == 0) {
        return 0;
    }
    if (*src == 0) {
        return 1;
    }

    unsigned char digest[32];
    std::memset(digest, 0, sizeof(digest));

    char salt[4];
    std::strcpy(salt, "nhs");
    char* salted = new char[length + 4];
    std::memset(salted, 0, static_cast<size_t>(length + 4));
    std::strcpy(salted, reinterpret_cast<const char*>(src));
    std::strcat(salted, salt);

    int result = 0;
    IChecksum* checksum = CreateCryptoChecksum(CRYPTO_CHECKSUM_SHA256);
    if (checksum->MakeChecksumFromMemory(
            reinterpret_cast<const unsigned char*>(salted),
            length + 4,
            digest,
            32) == CHECKSUM_RESULT_OK) {
        result = 1;
        for (unsigned int index = 0; index <= 0x1Fu; ++index) {
            std::sprintf(&output[2 * index], "%02X", digest[index]);
        }
    }
    DestroyCryptoChecksum(checksum);
    if (salted != 0) {
        delete[] salted;
    }
    return result;
}
