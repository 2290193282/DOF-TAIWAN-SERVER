#include "infra/security/policy/checksum.h"

#include <cstdio>


namespace {

const int kSha256ContextSize = 0x70;

extern "C" {
int SHA256_Init(void* context);
int SHA256_Update(void* context, const void* data, unsigned long length);
int SHA256_Final(unsigned char* output, void* context);
}

void* Sha256Context(void* context) {
    return context;
}

}  // namespace

// IDA_EA: 0x80986B8
// IDA_NAME: _ZN16CNChecksumSha256C2Ev
// DEMANGLED: CNChecksumSha256::CNChecksumSha256(void)
CNChecksumSha256::CNChecksumSha256()
    : IChecksum(CRYPTO_CHECKSUM_SHA256),
      context_(new unsigned char[kSha256ContextSize]) {}

// IDA_EA: 0x809871C
// IDA_NAME: _ZN16CNChecksumSha256D2Ev
// DEMANGLED: CNChecksumSha256::~CNChecksumSha256()
// IDA_EA: 0x809876E
// IDA_NAME: _ZN16CNChecksumSha256D0Ev
// DEMANGLED: CNChecksumSha256::~CNChecksumSha256() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNChecksumSha256::~CNChecksumSha256() {
    delete[] static_cast<unsigned char*>(context_);
    context_ = 0;
}

// IDA_EA: 0x809878C
// IDA_NAME: _ZN16CNChecksumSha256C2ERKS_
// DEMANGLED: CNChecksumSha256::CNChecksumSha256(CNChecksumSha256 const&)
CNChecksumSha256::CNChecksumSha256(const CNChecksumSha256& rhs)
    : IChecksum(rhs), context_(0) {}

// IDA_EA: 0x80987B0
// IDA_NAME: _ZN16CNChecksumSha256aSERKS_
// DEMANGLED: CNChecksumSha256::operator=(CNChecksumSha256 const&)
CNChecksumSha256& CNChecksumSha256::operator=(const CNChecksumSha256& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x80987B8
// IDA_NAME: _ZN16CNChecksumSha25610InitializeEPKhi
// DEMANGLED: CNChecksumSha256::Initialize(unsigned char const*,int)
int CNChecksumSha256::Initialize(const unsigned char* key, int length) {
    (void)key;
    (void)length;
    return CHECKSUM_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x80987C2
// IDA_NAME: _ZN16CNChecksumSha25622MakeChecksumFromMemoryEPKhiPhi
// DEMANGLED: CNChecksumSha256::MakeChecksumFromMemory(unsigned char const*,int,unsigned char *,int)
int CNChecksumSha256::MakeChecksumFromMemory(const unsigned char* data,
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
    SHA256_Init(Sha256Context(context_));
    SHA256_Update(Sha256Context(context_), data, static_cast<unsigned long>(length));
    SHA256_Final(output, Sha256Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098850
// IDA_NAME: _ZN16CNChecksumSha25620MakeChecksumFromFileEPKcPhi
// DEMANGLED: CNChecksumSha256::MakeChecksumFromFile(char const*,unsigned char *,int)
int CNChecksumSha256::MakeChecksumFromFile(const char* filename,
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
    SHA256_Init(Sha256Context(context_));
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
        SHA256_Update(Sha256Context(context_), buffer, size);
        offset += static_cast<long>(size);
    }
    SHA256_Final(output, Sha256Context(context_));
    std::fclose(stream);
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x80989D6
// IDA_NAME: _ZN16CNChecksumSha25613ChecksumStartEv
// DEMANGLED: CNChecksumSha256::ChecksumStart(void)
int CNChecksumSha256::ChecksumStart() {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA256_Init(Sha256Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098A02
// IDA_NAME: _ZN16CNChecksumSha25614ChecksumUpdateEPKhi
// DEMANGLED: CNChecksumSha256::ChecksumUpdate(unsigned char const*,int)
int CNChecksumSha256::ChecksumUpdate(const unsigned char* data, int length) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA256_Update(Sha256Context(context_), data, static_cast<unsigned long>(length));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098A3C
// IDA_NAME: _ZN16CNChecksumSha25614ChecksumFinishEPhi
// DEMANGLED: CNChecksumSha256::ChecksumFinish(unsigned char *,int)
int CNChecksumSha256::ChecksumFinish(unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    SHA256_Final(output, Sha256Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098A92
// IDA_NAME: _ZNK16CNChecksumSha25615GetChecksumSizeEv
// DEMANGLED: CNChecksumSha256::GetChecksumSize(void)const
int CNChecksumSha256::GetChecksumSize() const {
    return 32;
}
