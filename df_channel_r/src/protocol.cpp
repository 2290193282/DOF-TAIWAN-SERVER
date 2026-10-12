#include "channel/protocol.hpp"

#include <cstring>

namespace dnf { namespace channel {

bool IsPvpChannel(ChannelType type) noexcept {
    switch (type) {
    case ChannelType::LegacyPvp4:
    case ChannelType::FreePvp5:
    case ChannelType::LegacyPvp6:
    case ChannelType::BroadcastPvp:
    case ChannelType::IntegratedPvp:
    case ChannelType::SameTeamPvp9:
    case ChannelType::LegacyPvp10:
    case ChannelType::FreePractice:
    case ChannelType::LeagueQualifier:
    case ChannelType::RankedPvp:
    case ChannelType::FreePvp:
    case ChannelType::Tournament:
        return true;
    default:
        return false;
    }
}

bool IsFreePvpChannel(ChannelType type) noexcept {
    switch (type) {
    case ChannelType::FreePvp5:
    case ChannelType::BroadcastPvp:
    case ChannelType::SameTeamPvp9:
    case ChannelType::LegacyPvp10:
    case ChannelType::FreePractice:
    case ChannelType::LeagueQualifier:
    case ChannelType::FreePvp:
        return true;
    default:
        return false;
    }
}

bool IsSameTeamJoinChannel(ChannelType type) noexcept {
    return type == ChannelType::SameTeamPvp9 || type == ChannelType::LeagueQualifier ||
           type == ChannelType::RankedPvp;
}

bool IsIntegratedPvpChannel(ChannelType type) noexcept {
    switch (type) {
    case ChannelType::IntegratedPvp:
    case ChannelType::SameTeamPvp9:
    case ChannelType::LegacyPvp10:
    case ChannelType::FreePractice:
    case ChannelType::LeagueQualifier:
    case ChannelType::RankedPvp:
    case ChannelType::FreePvp:
        return true;
    default:
        return false;
    }
}

bool IsSupportedChannelType(std::int32_t type) noexcept {
    return (type >= 0 &&
            type <= static_cast<std::int32_t>(ChannelType::Tournament)) ||
           type == static_cast<std::int32_t>(ChannelType::Raid);
}

PacketHeader MakeHeader(Category category, MessageId id, std::uint32_t size, bool ack) noexcept {
    return PacketHeader{static_cast<std::uint8_t>(category), static_cast<std::uint8_t>(id), size, 0,
                        static_cast<std::uint8_t>(ack ? 1 : 0)};
}

bool ReadHeader(const Bytes& frame, PacketHeader* output) noexcept {
    return ReadPod(frame, output);
}

bool ValidateFrame(const Bytes& frame, std::size_t maximum_size) noexcept {
    PacketHeader header{};
    return ReadHeader(frame, &header) && header.size >= sizeof(PacketHeader) && header.size == frame.size() &&
           header.size <= maximum_size;
}

std::string FixedString(const char* data, std::size_t capacity) {
    std::size_t length = 0;
    while (length < capacity && data[length] != '\0') {
        ++length;
    }
    return std::string(data, length);
}

void AppendBytes(Bytes& destination, const void* source, std::size_t size) {
    const auto* begin = static_cast<const std::uint8_t*>(source);
    destination.insert(destination.end(), begin, begin + size);
}

void FinalizeFrame(Bytes& frame, bool ack) {
    if (frame.size() < sizeof(PacketHeader)) {
        return;
    }
    auto* header = reinterpret_cast<PacketHeader*>(frame.data());
    header->size = static_cast<std::uint32_t>(frame.size());
    header->ack = static_cast<std::uint8_t>(ack ? 1 : 0);
}

} }  // namespace dnf::channel
