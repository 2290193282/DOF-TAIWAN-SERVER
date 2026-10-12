#include "infra/security/crypto/md5_legacy.h"

#include <cstring>


namespace {

// This is the historical scratch-slot extent, independently retained by the
// existing CNChecksumMd5 implementation.  The OpenSSL MD5 entry points use
// only the leading MD5_CTX bytes and therefore preserve the original wrapper's
// 0xd8-byte caller-local slot and clear range.
const unsigned int kLegacyMd5ContextBytes = 0xD8u;

extern "C" {
int MD5_Init(void* context);
int MD5_Update(void* context, const void* data, unsigned long length);
int MD5_Final(unsigned char* output, void* context);
}

}  // namespace

// IDA_EA: 0x80B3D57
// IDA_NAME: _Z3md5PhiS_
// DEMANGLED: md5(unsigned char *,int,unsigned char *)
void* md5(uint8_t* src, int n400, uint8_t* p_s) {
    unsigned char context[kLegacyMd5ContextBytes];

    MD5_Init(context);
    MD5_Update(context, src, static_cast<unsigned long>(n400));
    MD5_Final(p_s, context);
    return std::memset(context, 0, kLegacyMd5ContextBytes);
}
