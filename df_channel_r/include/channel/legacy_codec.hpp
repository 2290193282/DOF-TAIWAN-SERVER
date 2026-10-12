#pragma once

#include "channel/protocol.hpp"

#include <array>
#include <ctime>
#include <mutex>
#include <string>

namespace dnf { namespace channel {

class LegacyCodec {
public:
    explicit LegacyCodec(std::array<std::uint8_t, 32> key);

    static std::array<std::uint8_t, 32> TimestampKey(std::time_t now);
    std::array<std::uint8_t, 32> key() const;
    void Rotate(std::time_t now);

    bool Encrypt(const Bytes& plaintext, Bytes* output, std::string* error = nullptr) const;
    bool Decrypt(const std::uint8_t* ciphertext, std::size_t size, Bytes* output,
                 std::string* error = nullptr) const;
    bool Compress(const Bytes& input, int level, Bytes* output, std::string* error = nullptr) const;

    bool EncodeGameReply(PacketHeader header, const Bytes& plaintext, bool compress, int compression_level,
                         Bytes* output, std::string* error = nullptr) const;

private:
    mutable std::mutex mutex_;
    std::array<std::uint8_t, 32> key_{};
};

} }  // namespace dnf::channel
