#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace dnf { namespace channel {

enum class Category : std::uint8_t {
    ClientToServer = 123,
    ServerToClient = 124,
};

enum class MessageId : std::uint8_t {
    AskChannelInfo = 1,
    UpdateChannelInfo = 2,
    AskChannelInfoReply = 3,
    NoticeChannelServer = 4,
    CheckScriptVersion = 5,
    CheckScriptVersionReply = 6,
    GetScript = 9,
    GetScriptReply = 10,
    Connect = 11,
    ConnectReply = 12,
    GetGcInfo = 13,
    GetGcInfoReply = 14,
    GetServerGroupChannelInfo = 17,
    GetServerGroupChannelInfoReply = 18,
};

// Channel types shared with Game and Bridge.  Values 21 and 22 are not
// persistent channel types; Raid resumes the official range at 23.
enum class ChannelType : std::int32_t {
    Common = 0,
    TradeAuction = 3,
    LegacyPvp4 = 4,
    FreePvp5 = 5,
    LegacyPvp6 = 6,
    BroadcastPvp = 7,
    IntegratedPvp = 8,
    SameTeamPvp9 = 9,
    LegacyPvp10 = 10,
    SpecialDungeon = 11,
    FreePractice = 13,
    LeagueQualifier = 14,
    RankedPvp = 15,
    FreePvp = 16,
    Report = 17,
    Zombie = 19,
    Tournament = 20,
    InvalidSentinel = 21,
    Reserved = 22,
    Raid = 23,
};

bool IsPvpChannel(ChannelType type) noexcept;
bool IsFreePvpChannel(ChannelType type) noexcept;
bool IsSameTeamJoinChannel(ChannelType type) noexcept;
bool IsIntegratedPvpChannel(ChannelType type) noexcept;
bool IsSupportedChannelType(std::int32_t type) noexcept;

#pragma pack(push, 1)
struct PacketHeader {
    std::uint8_t classification;
    std::uint8_t message_id;
    std::uint32_t size;
    std::uint32_t checksum;
    std::uint8_t ack;
};

struct CsUpdateChannelInfo {
    PacketHeader header;
    std::int32_t gc_no;
    std::int32_t group;
    std::int32_t channel_no;
    std::int32_t max_user_num;
    std::int32_t cur_user_num;
    char server_ip[16];
    std::int32_t port;
};

struct CsCheckScriptVersion {
    PacketHeader header;
    char channel_script_version[16];
};

struct ScCheckScriptVersion {
    PacketHeader header;
    std::int32_t is_valid_version;
    char channel_script_version[16];
};

struct CsConnect {
    PacketHeader header;
    char encrypted_data[32];
};

struct ScConnect {
    PacketHeader header;
    std::int32_t is_valid_key;
    char key[32];
};

struct CsNoticeChannelServer {
    PacketHeader header;
    std::int32_t id;
    char server_ip[16];
    std::int32_t port;
};

struct ScGetGcInfo {
    PacketHeader header;
    std::int32_t count;
};

// DNF12 CN client request built at DNF.exe:0xAB5210.  The client copies the
// ASCII name returned by ServerGroupType_ToWireName into the fixed field and
// appends the parser session's server-group enum from +0x88.
struct CsGetServerGroupChannelInfo {
    PacketHeader header;
    char server_group_name[20];
    std::uint32_t parser_server_group;
};

// Plaintext prefix of the encrypted+zlib-compressed message 18 response.
// DNF.exe:0xAB6C5D reads the first field as two bytes and 0xAB6CD6 reads the
// second as four bytes.  Exactly one server group follows in this protocol.
struct ScServerGroupChannelInfoPlainHeader {
    std::uint16_t fallback_server_group;
    std::uint32_t channel_count;
};

struct ServerGcInfoWire {
    char server_group_name[20];
    std::int32_t gc_no;
};

struct ScAskChannelInfo {
    PacketHeader header;
    std::int32_t server_group_count;
};

struct ServerGroupInfoWire {
    char server_group_name[20];
    std::int32_t server_count;
};

struct ServerInfoWire {
    char channel_name[20];
    std::int32_t max_user_num;
    std::int32_t cur_user_num;
    char server_ip[16];
    std::int32_t port;
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 11, "PacketHeader wire size");
static_assert(sizeof(CsUpdateChannelInfo) == 51, "CsUpdateChannelInfo wire size");
static_assert(sizeof(CsCheckScriptVersion) == 27, "CsCheckScriptVersion wire size");
static_assert(sizeof(ScCheckScriptVersion) == 31, "ScCheckScriptVersion wire size");
static_assert(sizeof(CsConnect) == 43, "CsConnect wire size");
static_assert(sizeof(ScConnect) == 47, "ScConnect wire size");
static_assert(sizeof(CsNoticeChannelServer) == 35, "CsNoticeChannelServer wire size");
static_assert(sizeof(ScGetGcInfo) == 15, "ScGetGcInfo wire size");
static_assert(sizeof(CsGetServerGroupChannelInfo) == 35,
              "CsGetServerGroupChannelInfo wire size");
static_assert(sizeof(ScServerGroupChannelInfoPlainHeader) == 6,
              "ScServerGroupChannelInfoPlainHeader wire size");
static_assert(sizeof(ServerGcInfoWire) == 24, "ServerGcInfoWire wire size");
static_assert(sizeof(ScAskChannelInfo) == 15, "ScAskChannelInfo wire size");
static_assert(sizeof(ServerGroupInfoWire) == 24, "ServerGroupInfoWire wire size");
static_assert(sizeof(ServerInfoWire) == 48, "ServerInfoWire wire size");

using Bytes = std::vector<std::uint8_t>;

PacketHeader MakeHeader(Category category, MessageId id, std::uint32_t size, bool ack = false) noexcept;
bool ReadHeader(const Bytes& frame, PacketHeader* output) noexcept;
bool ValidateFrame(const Bytes& frame, std::size_t maximum_size = 0xA0000U) noexcept;
std::string FixedString(const char* data, std::size_t capacity);

template <typename T>
Bytes PodFrame(const T& value) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(&value);
    return Bytes(begin, begin + sizeof(T));
}

template <typename T>
bool ReadPod(const Bytes& bytes, T* output, std::size_t offset = 0) noexcept {
    if (!output || offset > bytes.size() || bytes.size() - offset < sizeof(T)) return false;
    std::memcpy(output, bytes.data() + offset, sizeof(T));
    return true;
}

void AppendBytes(Bytes& destination, const void* source, std::size_t size);

template <typename T>
void AppendPod(Bytes& destination, const T& value) {
    AppendBytes(destination, &value, sizeof(value));
}

void FinalizeFrame(Bytes& frame, bool ack = true);

} }  // namespace dnf::channel
