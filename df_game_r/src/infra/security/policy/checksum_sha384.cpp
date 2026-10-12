#include "infra/security/policy/checksum.h"

#include <cstdio>


namespace {

const int kSha384ContextSize = 0xD0;

extern "C" {
int SHA384_Init(void* context);
int SHA384_Update(void* context, const void* data, unsigned long length);
int SHA384_Final(unsigned char* output, void* context);
}

void* Sha384Context(void* context) {
    return context;
}

}  // namespace

// IDA_EA: 0x8098A9C
// IDA_NAME: _ZN16CNChecksumSha384C2Ev
// DEMANGLED: CNChecksumSha384::CNChecksumSha384(void)
CNChecksumSha384::CNChecksumSha384()
    : IChecksum(CRYPTO_CHECKSUM_SHA384),
      context_(new unsigned char[kSha384ContextSize]) {}

// IDA_EA: 0x8098B00
// IDA_NAME: _ZN16CNChecksumSha384D2Ev
// DEMANGLED: CNChecksumSha384::~CNChecksumSha384()
// IDA_EA: 0x8098B52
// IDA_NAME: _ZN16CNChecksumSha384D0Ev
// DEMANGLED: CNChecksumSha384::~CNChecksumSha384() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNChecksumSha384::~CNChecksumSha384() {
    delete[] static_cast<unsigned char*>(context_);
    context_ = 0;
}

// IDA_EA: 0x8098B70
// IDA_NAME: _ZN16CNChecksumSha384C2ERKS_
// DEMANGLED: CNChecksumSha384::CNChecksumSha384(CNChecksumSha384 const&)
CNChecksumSha384::CNChecksumSha384(const CNChecksumSha384& rhs)
    : IChecksum(rhs), context_(0) {}

// IDA_EA: 0x8098B94
// IDA_NAME: _ZN16CNChecksumSha384aSERKS_
// DEMANGLED: CNChecksumSha384::operator=(CNChecksumSha384 const&)
CNChecksumSha384& CNChecksumSha384::operator=(const CNChecksumSha384& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x8098B9C
// IDA_NAME: _ZN16CNChecksumSha38410InitializeEPKhi
// DEMANGLED: CNChecksumSha384::Initialize(unsigned char const*,int)
int CNChecksumSha384::Initialize(const unsigned char* key, int length) {
    (void)key;
    (void)length;
    return CHECKSUM_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x8098BA6
// IDA_NAME: _ZN16CNChecksumSha38422MakeChecksumFromMemoryEPKhiPhi
// DEMANGLED: CNChecksumSha384::MakeChecksumFromMemory(unsigned char const*,int,unsigned char *,int)
int CNChecksumSha384::MakeChecksumFromMemory(const unsigned char* data,
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
    SHA384_Init(Sha384Context(context_));
    SHA384_Update(Sha384Context(context_), data, static_cast<unsigned long>(length));
    SHA384_Final(output, Sha384Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098C34
// IDA_NAME: _ZN16CNChecksumSha38420MakeChecksumFromFileEPKcPhi
// DEMANGLED: CNChecksumSha384::MakeChecksumFromFile(char const*,unsigned char *,int)
int CNChecksumSha384::MakeChecksumFromFile(const char* filename,
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
    SHA384_Init(Sha384Context(context_));
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
        SHA384_Update(Sha384Context(context_), buffer, size);
        offset += static_cast<long>(size);
    }
    SHA384_Final(output, Sha384Context(context_));
    std::fclose(stream);
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098DBA
// IDA_NAME: _ZN16CNChecksumSha38413ChecksumStartEv
// DEMANGLED: CNChecksumSha384::ChecksumStart(void)
int CNChecksumSha384::ChecksumStart() {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA384_Init(Sha384Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098DE6
// IDA_NAME: _ZN16CNChecksumSha38414ChecksumUpdateEPKhi
// DEMANGLED: CNChecksumSha384::ChecksumUpdate(unsigned char const*,int)
int CNChecksumSha384::ChecksumUpdate(const unsigned char* data, int length) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    SHA384_Update(Sha384Context(context_), data, static_cast<unsigned long>(length));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098E20
// IDA_NAME: _ZN16CNChecksumSha38414ChecksumFinishEPhi
// DEMANGLED: CNChecksumSha384::ChecksumFinish(unsigned char *,int)
int CNChecksumSha384::ChecksumFinish(unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    SHA384_Final(output, Sha384Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098E76
// IDA_NAME: _ZNK16CNChecksumSha38415GetChecksumSizeEv
// DEMANGLED: CNChecksumSha384::GetChecksumSize(void)const
int CNChecksumSha384::GetChecksumSize() const {
    return 48;
}
