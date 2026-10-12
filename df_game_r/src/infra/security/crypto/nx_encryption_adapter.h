#ifndef OLDSRV_SECURITY_NX_ENCRYPTION_ADAPTER_H_
#define OLDSRV_SECURITY_NX_ENCRYPTION_ADAPTER_H_

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

#include "infra/security/policy/packet_integrity.h"

// Runtime adapter used when the rebuild should not directly link
// libnxencryption.so. It mirrors the old dynamic-library boundary while keeping
// the exported C++ symbol names explicit.
class NxEncryptionAdapter {
public:
    NxEncryptionAdapter();
    ~NxEncryptionAdapter();

    bool load(const char* library_path);
    bool is_loaded() const;

    uint32_t Adler32(char* data, unsigned int length) const;
    const char* EncodeDressString(const char* input,
                                  char* output,
                                  unsigned long key) const;
    const char* DecodeDressString(const char* input,
                                  char* output,
                                  unsigned long key) const;
    const wchar_t* EncodeDressStringWide(const wchar_t* input,
                                         wchar_t* output,
                                         unsigned long key) const;
    const wchar_t* DecodeDressStringWide(const wchar_t* input,
                                         wchar_t* output,
                                         unsigned long key) const;

private:
    void* handle_;
    uint32_t (*adler32_)(char*, unsigned int);
    const char* (*encode_dress_string_)(const char*, char*, unsigned long);
    const char* (*decode_dress_string_)(const char*, char*, unsigned long);
    const wchar_t* (*encode_dress_string_wide_)(const wchar_t*,
                                                wchar_t*,
                                                unsigned long);
    const wchar_t* (*decode_dress_string_wide_)(const wchar_t*,
                                                wchar_t*,
                                                unsigned long);

    NxEncryptionAdapter(const NxEncryptionAdapter&);
    NxEncryptionAdapter& operator=(const NxEncryptionAdapter&);
};


#endif  // OLDSRV_SECURITY_NX_ENCRYPTION_ADAPTER_H_
