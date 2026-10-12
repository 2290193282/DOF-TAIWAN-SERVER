#include "infra/security/policy/checksum.h"

#include <cstdio>


namespace {

const int kMd5ContextSize = 0xD8;

extern "C" {
int MD5_Init(void* context);
int MD5_Update(void* context, const void* data, unsigned long length);
int MD5_Final(unsigned char* output, void* context);
}

void* Md5Context(void* context) {
    return context;
}

}  // namespace

// IDA_EA: 0x809830C
// IDA_NAME: _ZN13CNChecksumMd5C2Ev
// DEMANGLED: CNChecksumMd5::CNChecksumMd5(void)
CNChecksumMd5::CNChecksumMd5()
    : IChecksum(CRYPTO_CHECKSUM_MD5),
      context_(new unsigned char[kMd5ContextSize]) {}

// IDA_EA: 0x8098370
// IDA_NAME: _ZN13CNChecksumMd5D2Ev
// DEMANGLED: CNChecksumMd5::~CNChecksumMd5()
// IDA_EA: 0x80983C2
// IDA_NAME: _ZN13CNChecksumMd5D0Ev
// DEMANGLED: CNChecksumMd5::~CNChecksumMd5() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNChecksumMd5::~CNChecksumMd5() {
    delete[] static_cast<unsigned char*>(context_);
    context_ = 0;
}

// IDA_EA: 0x80983E0
// IDA_NAME: _ZN13CNChecksumMd510InitializeEPKhi
// DEMANGLED: CNChecksumMd5::Initialize(unsigned char const*,int)
int CNChecksumMd5::Initialize(const unsigned char* key, int length) {
    (void)key;
    (void)length;
    return CHECKSUM_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x80983EA
// IDA_NAME: _ZN13CNChecksumMd522MakeChecksumFromMemoryEPKhiPhi
// DEMANGLED: CNChecksumMd5::MakeChecksumFromMemory(unsigned char const*,int,unsigned char *,int)
int CNChecksumMd5::MakeChecksumFromMemory(const unsigned char* data, int length,
                                          unsigned char* output,
                                          int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    MD5_Init(Md5Context(context_));
    MD5_Update(Md5Context(context_), data, static_cast<unsigned long>(length));
    MD5_Final(output, Md5Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x809846A
// IDA_NAME: _ZN13CNChecksumMd520MakeChecksumFromFileEPKcPhi
// DEMANGLED: CNChecksumMd5::MakeChecksumFromFile(char const*,unsigned char *,int)
int CNChecksumMd5::MakeChecksumFromFile(const char* filename,
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
    MD5_Init(Md5Context(context_));
    for (long offset = 0; offset < total;) {
        size_t size = static_cast<size_t>(total - offset);
        if (size > sizeof(buffer)) {
            size = sizeof(buffer);
        }
        std::fread(buffer, size, 1, stream);
        MD5_Update(Md5Context(context_), buffer, size);
        offset += static_cast<long>(size);
    }
    MD5_Final(output, Md5Context(context_));
    std::fclose(stream);
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x80985F0
// IDA_NAME: _ZN13CNChecksumMd513ChecksumStartEv
// DEMANGLED: CNChecksumMd5::ChecksumStart(void)
int CNChecksumMd5::ChecksumStart() {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    MD5_Init(Md5Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x809861C
// IDA_NAME: _ZN13CNChecksumMd514ChecksumUpdateEPKhi
// DEMANGLED: CNChecksumMd5::ChecksumUpdate(unsigned char const*,int)
int CNChecksumMd5::ChecksumUpdate(const unsigned char* data, int length) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    MD5_Update(Md5Context(context_), data, static_cast<unsigned long>(length));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x8098656
// IDA_NAME: _ZN13CNChecksumMd514ChecksumFinishEPhi
// DEMANGLED: CNChecksumMd5::ChecksumFinish(unsigned char *,int)
int CNChecksumMd5::ChecksumFinish(unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CHECKSUM_RESULT_ERROR;
    }
    if (GetChecksumSize() > output_capacity) {
        return CHECKSUM_RESULT_OUTPUT_TOO_SMALL;
    }
    MD5_Final(output, Md5Context(context_));
    return CHECKSUM_RESULT_OK;
}

// IDA_EA: 0x80986AC
// IDA_NAME: _ZNK13CNChecksumMd515GetChecksumSizeEv
// DEMANGLED: CNChecksumMd5::GetChecksumSize(void)const
int CNChecksumMd5::GetChecksumSize() const {
    return 16;
}
