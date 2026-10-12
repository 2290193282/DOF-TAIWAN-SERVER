#pragma once

#include "dfbridge/channel_model.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dfbridge {
namespace wire {

static const std::uint8_t kClientToServerCategory = 123;
static const std::uint8_t kServerToClientCategory = 124;

static const std::uint8_t kCsNoticeChannelServer = 4;
static const std::uint8_t kCsCheckScriptVersion = 5;
static const std::uint8_t kScCheckScriptVersion = 6;
static const std::uint8_t kCsGetScript = 9;
static const std::uint8_t kScGetScript = 10;
static const std::uint8_t kCsGetGcInfo = 13;
static const std::uint8_t kScGetGcInfo = 14;
static const std::uint8_t kCsUpdateChannelInfoExtra = 2;

#pragma pack(push, 1)
struct PacketHeader {
    std::uint8_t classification{};
    std::uint8_t msg_no{};
    std::uint32_t length{};
    std::uint32_t checksum{};
    std::uint8_t ack{};
};

struct CsCheckScriptVersion {
    PacketHeader header;
    char channel_script_version[16]{};
};

struct ScCheckScriptVersion {
    PacketHeader header;
    std::int32_t is_valid_version{};
    char channel_script_version[16]{};
};

struct ScGetGcInfo {
    PacketHeader header;
    std::int32_t count{};
};

struct ServerGcInfo {
    char server_group_name[20]{};
    std::int32_t gc_no{};
};

struct CsUpdateChannelInfo {
    PacketHeader header;
    std::int32_t gc_no{};
    std::int32_t group{};
    std::int32_t channel_no{};
    std::int32_t max_user_num{};
    std::int32_t cur_user_num{};
    char server_ip[16]{};
    std::int32_t port{};
};

struct CsNoticeChannelServer {
    PacketHeader header;
    std::int32_t id{};
    char server_ip[16]{};
    std::int32_t port{};
};
#pragma pack(pop)

static_assert(sizeof(PacketHeader) == 11, "PacketHeader wire size mismatch");
static_assert(sizeof(CsCheckScriptVersion) == 27, "CS version wire size mismatch");
static_assert(sizeof(ScCheckScriptVersion) == 31, "SC version wire size mismatch");
static_assert(sizeof(ScGetGcInfo) == 15, "SC GC header wire size mismatch");
static_assert(sizeof(ServerGcInfo) == 24, "GC record wire size mismatch");
static_assert(sizeof(CsUpdateChannelInfo) == 51, "channel update wire size mismatch");
static_assert(sizeof(CsNoticeChannelServer) == 35, "channel notice wire size mismatch");

enum class TcpRequestKind {
    Unknown,
    ChannelNotice,
    CheckScriptVersion,
    GetScript,
    GetGcInfo,
};

PacketHeader MakeHeader(std::uint8_t category, std::uint8_t packet_id,
                        std::uint32_t length);
TcpRequestKind ClassifyTcpRequest(const PacketHeader& header);
bool IsChannelUpdatePacket(const PacketHeader& header, std::size_t frame_size);
std::vector<std::uint8_t> MakeVersionResponse(const std::string& current_version,
                                              const std::string& peer_version);
std::vector<std::uint8_t> MakeScriptResponse(const std::string& script);
std::vector<std::uint8_t> MakeGcInfoResponse(const std::vector<GcGroup>& groups);
std::vector<std::uint8_t> MakeChannelUpdate(
    std::int32_t gc_no, std::int32_t group, std::int32_t channel_no,
    std::int32_t max_user_num, std::int32_t cur_user_num,
    const std::string& server_ip, std::int32_t port);

class FrameDecoder {
public:
    // Original TCPUser::onRead_ uses a 0xa0000-byte receive/reassembly ceiling.
    static const std::size_t kMaximumFrameSize = 0xA0000U;

    bool Push(const std::uint8_t* data, std::size_t size,
              std::vector<std::vector<std::uint8_t>>* frames,
              std::string* error);
    void Reset();
    std::size_t BufferedSize() const;

private:
    std::vector<std::uint8_t> buffer_;
};

}  // namespace wire
}  // namespace dfbridge
