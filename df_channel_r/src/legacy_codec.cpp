#include "channel/legacy_codec.hpp"

#include <openssl/evp.h>
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <zlib.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>

namespace dnf { namespace channel {
namespace {

EVP_CIPHER_CTX* NewCipherContext() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX* context = static_cast<EVP_CIPHER_CTX*>(OPENSSL_malloc(sizeof(EVP_CIPHER_CTX)));
    if (context) EVP_CIPHER_CTX_init(context);
    return context;
#else
    return EVP_CIPHER_CTX_new();
#endif
}

void FreeCipherContext(EVP_CIPHER_CTX* context) {
    if (!context) return;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX_cleanup(context);
    OPENSSL_free(context);
#else
    EVP_CIPHER_CTX_free(context);
#endif
}

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, void (*)(EVP_CIPHER_CTX*)>;

}  // namespace

LegacyCodec::LegacyCodec(std::array<std::uint8_t, 32> key) : key_(key) {}

std::array<std::uint8_t, 32> LegacyCodec::key() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return key_;
}

std::array<std::uint8_t, 32> LegacyCodec::TimestampKey(std::time_t now) {
    std::array<std::uint8_t, 32> result{};
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::snprintf(reinterpret_cast<char*>(result.data()), 15, "%04d%02d%02d%02d%02d%02d",
                  local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min,
                  local.tm_sec);
    result[14] = 0;  // exact 0828 createEncKey layout
    return result;
}

void LegacyCodec::Rotate(std::time_t now) {
    std::lock_guard<std::mutex> lock(mutex_);
    key_ = TimestampKey(now);
}

bool LegacyCodec::Encrypt(const Bytes& plaintext, Bytes* output, std::string* error) const {
    if (!output) return false;
    const std::size_t padded_size = (plaintext.size() + 15U) & ~std::size_t(15U);
    Bytes padded(padded_size, 0);  // CRijndael padding mode 0
    std::copy(plaintext.begin(), plaintext.end(), padded.begin());
    output->assign(padded_size + 16, 0);
    CipherContext context(NewCipherContext(), FreeCipherContext);
    const std::array<std::uint8_t, 32> active_key = key();
    int first = 0;
    int final = 0;
    if (!context || EVP_EncryptInit_ex(context.get(), EVP_aes_128_ecb(), nullptr, active_key.data(), nullptr) != 1 ||
        EVP_CIPHER_CTX_set_padding(context.get(), 0) != 1 ||
        (!padded.empty() && EVP_EncryptUpdate(context.get(), output->data(), &first, padded.data(),
                                             static_cast<int>(padded.size())) != 1) ||
        EVP_EncryptFinal_ex(context.get(), output->data() + first, &final) != 1) {
        if (error) *error = "AES-128-ECB encryption failed";
        return false;
    }
    output->resize(static_cast<std::size_t>(first + final));
    return true;
}

bool LegacyCodec::Decrypt(const std::uint8_t* ciphertext, std::size_t size, Bytes* output,
                          std::string* error) const {
    if (!output) return false;
    if (!ciphertext || size == 0 || size % 16 != 0) {
        if (error) *error = "encrypted body is not a non-empty 16-byte multiple";
        return false;
    }
    output->assign(size + 16, 0);
    CipherContext context(NewCipherContext(), FreeCipherContext);
    const std::array<std::uint8_t, 32> active_key = key();
    int first = 0;
    int final = 0;
    if (!context || EVP_DecryptInit_ex(context.get(), EVP_aes_128_ecb(), nullptr, active_key.data(), nullptr) != 1 ||
        EVP_CIPHER_CTX_set_padding(context.get(), 0) != 1 ||
        EVP_DecryptUpdate(context.get(), output->data(), &first, ciphertext, static_cast<int>(size)) != 1 ||
        EVP_DecryptFinal_ex(context.get(), output->data() + first, &final) != 1) {
        if (error) *error = "AES-128-ECB decryption failed";
        return false;
    }
    output->resize(static_cast<std::size_t>(first + final));
    return true;
}

bool LegacyCodec::Compress(const Bytes& input, int level, Bytes* output, std::string* error) const {
    if (!output) return false;
    uLongf output_size = compressBound(static_cast<uLong>(input.size()));
    output->assign(output_size, 0);
    const int result = compress2(output->data(), &output_size, input.data(), static_cast<uLong>(input.size()), level);
    if (result != Z_OK) {
        if (error) *error = "zlib compress2 failed: " + std::to_string(result);
        return false;
    }
    output->resize(output_size);
    return true;
}

bool LegacyCodec::EncodeGameReply(PacketHeader header, const Bytes& plaintext, bool compress,
                                  int compression_level, Bytes* output, std::string* error) const {
    if (!output) return false;
    Bytes body;
    if (!Encrypt(plaintext, &body, error)) return false;
    if (compress) {
        Bytes compressed;
        if (!Compress(body, compression_level, &compressed, error)) return false;
        body.swap(compressed);
    }
    header.size = static_cast<std::uint32_t>(sizeof(PacketHeader) + body.size());
    header.ack = 1;
    *output = PodFrame(header);
    AppendBytes(*output, body.data(), body.size());
    return true;
}

} }  // namespace dnf::channel
