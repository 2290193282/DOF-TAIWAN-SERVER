#include "infra/security/crypto/nx_encryption_adapter.h"

#include <dlfcn.h>


namespace {

const char kAdler32Symbol[] = "_ZN10Encryption7Adler32EPcj";
const char kEncodeDressStringSymbol[] =
    "_ZN10Encryption17EncodeDressStringEPKcPcm";
const char kDecodeDressStringSymbol[] =
    "_ZN10Encryption17DecodeDressStringEPKcPcm";
const char kEncodeDressStringWideSymbol[] =
    "_ZN10Encryption21EncodeDressStringWideEPKwPwm";
const char kDecodeDressStringWideSymbol[] =
    "_ZN10Encryption21DecodeDressStringWideEPKwPwm";

template <typename Fn>
Fn Resolve(void* handle, const char* symbol) {
    return reinterpret_cast<Fn>(dlsym(handle, symbol));
}

}  // namespace

NxEncryptionAdapter::NxEncryptionAdapter()
    : handle_(0),
      adler32_(0),
      encode_dress_string_(0),
      decode_dress_string_(0),
      encode_dress_string_wide_(0),
      decode_dress_string_wide_(0) {
}

NxEncryptionAdapter::~NxEncryptionAdapter() {
    if (handle_) {
        dlclose(handle_);
        handle_ = 0;
    }
}

bool NxEncryptionAdapter::load(const char* library_path) {
    if (handle_) {
        return true;
    }

    handle_ = dlopen(library_path, RTLD_NOW);
    if (!handle_) {
        return false;
    }

    adler32_ =
        Resolve<uint32_t (*)(char*, unsigned int)>(handle_, kAdler32Symbol);
    encode_dress_string_ =
        Resolve<const char* (*)(const char*, char*, unsigned long)>(
            handle_, kEncodeDressStringSymbol);
    decode_dress_string_ =
        Resolve<const char* (*)(const char*, char*, unsigned long)>(
            handle_, kDecodeDressStringSymbol);
    encode_dress_string_wide_ =
        Resolve<const wchar_t* (*)(const wchar_t*, wchar_t*, unsigned long)>(
            handle_, kEncodeDressStringWideSymbol);
    decode_dress_string_wide_ =
        Resolve<const wchar_t* (*)(const wchar_t*, wchar_t*, unsigned long)>(
            handle_, kDecodeDressStringWideSymbol);

    if (!adler32_ || !encode_dress_string_ || !decode_dress_string_ ||
        !encode_dress_string_wide_ || !decode_dress_string_wide_) {
        dlclose(handle_);
        handle_ = 0;
        adler32_ = 0;
        encode_dress_string_ = 0;
        decode_dress_string_ = 0;
        encode_dress_string_wide_ = 0;
        decode_dress_string_wide_ = 0;
        return false;
    }
    return true;
}

bool NxEncryptionAdapter::is_loaded() const {
    return handle_ != 0;
}

uint32_t NxEncryptionAdapter::Adler32(char* data,
                                      unsigned int length) const {
    return adler32_ ? adler32_(data, length) : 0;
}

const char* NxEncryptionAdapter::EncodeDressString(
    const char* input,
    char* output,
    unsigned long key) const {
    return encode_dress_string_
               ? encode_dress_string_(input, output, key)
               : 0;
}

const char* NxEncryptionAdapter::DecodeDressString(
    const char* input,
    char* output,
    unsigned long key) const {
    return decode_dress_string_
               ? decode_dress_string_(input, output, key)
               : 0;
}

const wchar_t* NxEncryptionAdapter::EncodeDressStringWide(
    const wchar_t* input,
    wchar_t* output,
    unsigned long key) const {
    return encode_dress_string_wide_
               ? encode_dress_string_wide_(input, output, key)
               : 0;
}

const wchar_t* NxEncryptionAdapter::DecodeDressStringWide(
    const wchar_t* input,
    wchar_t* output,
    unsigned long key) const {
    return decode_dress_string_wide_
               ? decode_dress_string_wide_(input, output, key)
               : 0;
}
