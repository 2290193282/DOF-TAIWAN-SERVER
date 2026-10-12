#include "infra/security/policy/checksum.h"

#include <cstdio>


namespace {

const int kSha512ContextSize = 0xD0;

extern "C" {
int SHA512_Init(void* context);
int SHA512_Update(void* context, const void* data, unsigned long length);
int SHA512_Final(unsigned char* output, void* context);
}

void* Sha512Context(void* context) {
    return context;
}

}  // namespace

// IDA_EA: 0x8098E80
// IDA_NAME: _ZN16CNChecksumSha512C2Ev
// DEMANGLED: CNChecksumSha512::CNChecksumSha512(void)
CNChecksumSha512::CNChecksumSha512()
    : IChecksum(CRYPTO_CHECKSUM_SHA512),
      context_(new unsigned char[kSha512ContextSize]) {}

// IDA_EA: 0x8098EE4
// IDA_NAME: _ZN16CNChecksumSha512D2Ev
// DEMANGLED: CNChecksumSha512::~CNChecksumSha512()
// IDA_EA: 0x8098F36
// IDA_NAME: _ZN16CNChecksumSha512D0Ev
// DEMANGLED: CNChecksumSha512::~CNChecksumSha512() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNChecksumSha512::~CNChecksumSha512() {
    delete[] static_cast<unsigned char*>(context_);
    context_ = 0;
}

// IDA_EA: 0x8098F54
// IDA_NAME: _ZN16CNChecksumSha512C2ERKS_
// DEMANGLED: CNChecksumSha512::CNChecksumSha512(CNChecksumSha512 const&)
CNChecksumSha512::CNChecksumSha512(const CNChecksumSha512& rhs)
    : IChecksum(CRYPTO_CHECKSUM_SHA256), context_(0) {
    (void)rhs;
}

// IDA_EA: 0x8098F78
// IDA_NAME: _ZN16CNChecksumSha512aSERKS_
// DEMANGLED: CNChecksumSha512::operator=(CNChecksumSha512 const&)
CNChecksumSha512& CNChecksumSha512::operator=(const CNChecksumSha512& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x8098F80
// IDA_NAME: _ZN16CNChecksumSha51210InitializeEPKhi
// DEMANGLED: CNChecksumSha512::Initialize(unsigned char const*,int)
int CNChecksumSha512::Initialize(const unsigned char* key, int length) {
    (void)key;
    (void)length;
    return CHECKSUM_RESULT_INIT_REJECTED;
}

// IDA_EA: 0x8098F8A
// IDA_NAME: _ZN16CNChecksumSha51222MakeChecksumFromMemoryEPKhiPhi
// DEMANGLED: CNChecksumSha512::MakeChecksumFromMemory(unsigned char const*,int,unsigned char *,int)
int CNChecksumSha512::MakeChecksumFromMemory(const unsigned char* data,
                                             int length,
                                             unsigned char* output,
                                             int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    if (length <= 0) {
        return CHECKSUM_RESULT_INVALID_LENGTH;
    }
    SHA512_Init(Sha512Context(context_));
    SHA512_Update(Sha512Context(context_), data, static_cast<unsigned long>(length));
    SHA512_Final(output, Sha512Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8099018
// IDA_NAME: _ZN16CNChecksumSha51220MakeChecksumFromFileEPKcPhi
// DEMANGLED: CNChecksumSha512::MakeChecksumFromFile(char const*,unsigned char *,int)
int CNChecksumSha512::MakeChecksumFromFile(const char* filename,
                                           unsigned char* output,
                                           int output_capacity) {
    unsigned char buffer[1024];
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    if (filename == 0) {
        return CHECKSUM_RESULT_INVALID_ARGUMENT;
    }
    SHA512_Init(Sha512Context(context_));
    FILE* stream = std::fopen(filename, "rb");
    if (stream == 0) {
        return CHECKSUM_RESULT_FILE_OPEN_FAILED;
    }
    std::fseek(stream, 0, SEEK_END);
    const long total = std::ftell(stream);
    if (total < 0) {
        std::fclose(stream);
        return CHECKSUM_RESULT_INVALID_ARGUMENT;
    }
    std::fseek(stream, 0, SEEK_SET);
    for (long offset = 0; offset < total;) {
        size_t size = static_cast<size_t>(total - offset);
        if (size > sizeof(buffer)) {
            size = sizeof(buffer);
        }
        std::fread(buffer, size, 1, stream);
        SHA512_Update(Sha512Context(context_), buffer, size);
        offset += static_cast<long>(size);
    }
    SHA512_Final(output, Sha512Context(context_));
    std::fclose(stream);
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x809919E
// IDA_NAME: _ZN16CNChecksumSha51213ChecksumStartEv
// DEMANGLED: CNChecksumSha512::ChecksumStart(void)
int CNChecksumSha512::ChecksumStart() {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA512_Init(Sha512Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x80991CA
// IDA_NAME: _ZN16CNChecksumSha51214ChecksumUpdateEPKhi
// DEMANGLED: CNChecksumSha512::ChecksumUpdate(unsigned char const*,int)
int CNChecksumSha512::ChecksumUpdate(const unsigned char* data, int length) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA512_Update(Sha512Context(context_), data, static_cast<unsigned long>(length));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8099204
// IDA_NAME: _ZN16CNChecksumSha51214ChecksumFinishEPhi
// DEMANGLED: CNChecksumSha512::ChecksumFinish(unsigned char *,int)
int CNChecksumSha512::ChecksumFinish(unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    SHA512_Final(output, Sha512Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x809925A
// IDA_NAME: _ZNK16CNChecksumSha51215GetChecksumSizeEv
// DEMANGLED: CNChecksumSha512::GetChecksumSize(void)const
int CNChecksumSha512::GetChecksumSize() const {
    return 64;
}
