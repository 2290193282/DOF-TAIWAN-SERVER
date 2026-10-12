#include "dfbridge/protocol.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace dfbridge {
namespace wire {

const std::size_t FrameDecoder::kMaximumFrameSize;

namespace {

template <typename T>
void AppendObject(std::vector<std::uint8_t>& output, const T& object) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&object);
    output.insert(output.end(), bytes, bytes + sizeof(T));
}

template <std::size_t N>
void CopyString(char (&target)[N], const std::string& source) {
    const auto count = std::min(source.size(), N - 1);
    std::memcpy(target, source.data(), count);
    target[count] = '\0';
}

}  // namespace

PacketHeader MakeHeader(const std::uint8_t category, const std::uint8_t packet_id,
                        const std::uint32_t length) {
    PacketHeader header{};
    header.classification = category;
    header.msg_no = packet_id;
    header.length = length;
    return header;
}

TcpRequestKind ClassifyTcpRequest(const PacketHeader& header) {
    // The original TCPHandlerRelay indexes only byte +1 (packet id). It does
    // not gate dispatch on byte +0 classification.
    switch (header.msg_no) {
        case kCsNoticeChannelServer:
            return TcpRequestKind::ChannelNotice;
        case kCsCheckScriptVersion:
            return TcpRequestKind::CheckScriptVersion;
        case kCsGetScript:
            return TcpRequestKind::GetScript;
        case kCsGetGcInfo:
            return TcpRequestKind::GetGcInfo;
        default:
            return TcpRequestKind::Unknown;
    }
}

bool IsChannelUpdatePacket(const PacketHeader& header,
                           const std::size_t frame_size) {
    // UDPHandlerRelay also dispatches by packet id only. Keep length safety,
    // while accepting the classifications emitted by both old Game builds.
    return header.msg_no == kCsUpdateChannelInfoExtra &&
           header.length == frame_size &&
           frame_size == sizeof(CsUpdateChannelInfo);
}

std::vector<std::uint8_t> MakeVersionResponse(const std::string& current_version,
                                              const std::string& peer_version) {
    ScCheckScriptVersion packet{};
    packet.header = MakeHeader(kServerToClientCategory, kScCheckScriptVersion,
                               sizeof(packet));
    // Preserve df_bridge_r's prefix comparison. The current version field is
    // authoritative and its full length must match the peer's prefix.
    packet.is_valid_version =
        peer_version.size() >= current_version.size() &&
        peer_version.compare(0, current_version.size(), current_version) == 0;
    CopyString(packet.channel_script_version, current_version);
    std::vector<std::uint8_t> output;
    output.reserve(sizeof(packet));
    AppendObject(output, packet);
    return output;
}

std::vector<std::uint8_t> MakeScriptResponse(const std::string& script) {
    if (script.size() > std::numeric_limits<std::uint32_t>::max() - sizeof(PacketHeader)) {
        throw std::runtime_error("channel script exceeds the wire length field");
    }
    const auto length = static_cast<std::uint32_t>(sizeof(PacketHeader) + script.size());
    const auto header = MakeHeader(kServerToClientCategory, kScGetScript, length);
    std::vector<std::uint8_t> output;
    output.reserve(length);
    AppendObject(output, header);
    output.insert(output.end(), script.begin(), script.end());
    return output;
}

std::vector<std::uint8_t> MakeGcInfoResponse(const std::vector<GcGroup>& groups) {
    if (groups.size() > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max())) {
        throw std::runtime_error("too many GC groups");
    }
    const auto wire_size = sizeof(ScGetGcInfo) + groups.size() * sizeof(ServerGcInfo);
    if (wire_size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("GC response exceeds the wire length field");
    }

    ScGetGcInfo header{};
    header.header = MakeHeader(kServerToClientCategory, kScGetGcInfo,
                               static_cast<std::uint32_t>(wire_size));
    header.count = static_cast<std::int32_t>(groups.size());
    std::vector<std::uint8_t> output;
    output.reserve(wire_size);
    AppendObject(output, header);
    for (const auto& group : groups) {
        ServerGcInfo item{};
        CopyString(item.server_group_name, group.group_name);
        item.gc_no = group.upper_gc_no;
        AppendObject(output, item);
    }
    return output;
}

std::vector<std::uint8_t> MakeChannelUpdate(
    const std::int32_t gc_no, const std::int32_t group,
    const std::int32_t channel_no, const std::int32_t max_user_num,
    const std::int32_t cur_user_num, const std::string& server_ip,
    const std::int32_t port) {
    CsUpdateChannelInfo packet{};
    packet.header = MakeHeader(kClientToServerCategory, kCsUpdateChannelInfoExtra,
                               sizeof(packet));
    packet.gc_no = gc_no;
    packet.group = group;
    packet.channel_no = channel_no;
    packet.max_user_num = max_user_num;
    packet.cur_user_num = cur_user_num;
    CopyString(packet.server_ip, server_ip);
    packet.port = port;
    std::vector<std::uint8_t> output;
    output.reserve(sizeof(packet));
    AppendObject(output, packet);
    return output;
}

bool FrameDecoder::Push(const std::uint8_t* data, const std::size_t size,
                        std::vector<std::vector<std::uint8_t>>* frames,
                        std::string* error) {
    if (!frames) {
        if (error) *error = "frame output is null";
        return false;
    }
    if (size != 0 && !data) {
        if (error) *error = "frame input is null";
        return false;
    }
    if (buffer_.size() > kMaximumFrameSize ||
        size > kMaximumFrameSize - buffer_.size()) {
        if (error) *error = "frame input buffer exceeds limit";
        buffer_.clear();
        return false;
    }
    if (size != 0) buffer_.insert(buffer_.end(), data, data + size);
    while (buffer_.size() >= sizeof(PacketHeader)) {
        PacketHeader header{};
        std::memcpy(&header, buffer_.data(), sizeof(header));
        const std::size_t frame_size = static_cast<std::size_t>(header.length);
        if (frame_size < sizeof(PacketHeader) || frame_size > kMaximumFrameSize) {
            if (error) *error = "invalid bridge frame length";
            buffer_.clear();
            return false;
        }
        if (buffer_.size() < frame_size) break;
        frames->push_back(
            std::vector<std::uint8_t>(buffer_.begin(), buffer_.begin() + frame_size));
        buffer_.erase(buffer_.begin(), buffer_.begin() + frame_size);
    }
    return true;
}

void FrameDecoder::Reset() {
    buffer_.clear();
}

std::size_t FrameDecoder::BufferedSize() const {
    return buffer_.size();
}

}  // namespace wire
}  // namespace dfbridge
