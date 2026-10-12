#include "channel/channel_service.hpp"

#include <algorithm>
#include <cstring>
#include <zlib.h>

namespace dnf { namespace channel {
namespace {

std::string ZeroTerminated(const Bytes& bytes) {
    const auto end = std::find(bytes.begin(), bytes.end(), 0);
    return std::string(bytes.begin(), end);
}

template <std::size_t N>
void CopyFixed(char (&destination)[N], const std::string& source) {
    std::memset(destination, 0, N);
    std::memcpy(destination, source.data(), std::min<std::size_t>(source.size(), N - 1));
}

}  // namespace

HandleResult ChannelService::HandleGameFrame(const Bytes& frame) {
    if (!ValidateFrame(frame)) return HandleResult(false, false, Bytes(), "invalid frame");
    PacketHeader header{};
    ReadHeader(frame, &header);
    switch (static_cast<MessageId>(header.message_id)) {
    case MessageId::Connect: return HandleConnect(frame);
    case MessageId::CheckScriptVersion: return HandleCheckScriptVersion(frame);
    case MessageId::GetScript:
        if (frame.size() != sizeof(PacketHeader))
            return HandleResult(false, false, Bytes(), "bad GET_SCRIPT size");
        return HandleGetScript();
    case MessageId::AskChannelInfo:
        if (frame.size() != sizeof(PacketHeader))
            return HandleResult(false, false, Bytes(), "bad ASK_CHANNEL_INFO size");
        return HandleAskChannelInfo();
    case MessageId::GetServerGroupChannelInfo:
        return HandleGetServerGroupChannelInfo(frame);
    case MessageId::UpdateChannelInfo: return HandleUpdateChannelInfo(frame);
    default: return HandleResult(false, false, Bytes(), "unsupported channel message");
    }
}

HandleResult ChannelService::HandleConnect(const Bytes& frame) {
    const std::size_t encrypted_size = frame.size() - sizeof(PacketHeader);
    const std::size_t maximum_encrypted_size = sizeof(CsConnect) - sizeof(PacketHeader);
    if (encrypted_size == 0 || encrypted_size > maximum_encrypted_size ||
        encrypted_size % 16 != 0) {
        return HandleResult(false, false, Bytes(), "bad CONNECT size");
    }
    const auto* encrypted = frame.data() + sizeof(PacketHeader);
    std::string error;
    Bytes plaintext;
    const bool decrypted = codec_.Decrypt(encrypted, encrypted_size, &plaintext, &error);
    static const char expected[] = "dnf_game";
    const bool valid = decrypted && plaintext.size() >= sizeof(expected) - 1 &&
                       std::memcmp(plaintext.data(), expected, sizeof(expected) - 1) == 0;
    ScConnect reply{};
    reply.header = MakeHeader(Category::ServerToClient, MessageId::ConnectReply, sizeof(reply), true);
    reply.is_valid_key = valid ? 1 : 0;
    if (!valid) std::memcpy(reply.key, codec_.key().data(), sizeof(reply.key));
    return HandleResult(true, true, PodFrame(reply), std::string());
}

HandleResult ChannelService::HandleCheckScriptVersion(const Bytes& frame) {
    if (frame.size() < sizeof(PacketHeader) + 16 || (frame.size() - sizeof(PacketHeader)) % 16 != 0) {
        return HandleResult(false, false, Bytes(), "bad CHECK_SCRIPT_VERSION size");
    }
    std::string error;
    Bytes plaintext;
    if (!codec_.Decrypt(frame.data() + sizeof(PacketHeader), frame.size() - sizeof(PacketHeader),
                        &plaintext, &error) || plaintext.size() < 16) {
        return HandleResult(false, false, Bytes(), error);
    }
    const auto snapshot = scripts_.Snapshot();
    const std::string supplied = ZeroTerminated(plaintext);
    std::int32_t valid = supplied == snapshot.version ? 1 : 0;
    Bytes body;
    AppendPod(body, valid);
    std::array<char, 16> version{};
    std::memcpy(version.data(), snapshot.version.data(), std::min<std::size_t>(15, snapshot.version.size()));
    AppendBytes(body, version.data(), version.size());
    auto header = MakeHeader(Category::ServerToClient, MessageId::CheckScriptVersionReply, 0, true);
    Bytes response;
    if (!codec_.EncodeGameReply(header, body, false, Z_DEFAULT_COMPRESSION, &response, &error))
        return HandleResult(false, false, Bytes(), error);
    return HandleResult(true, true, std::move(response), std::string());
}

HandleResult ChannelService::HandleGetScript() {
    const auto snapshot = scripts_.Snapshot();
    std::string error;
    auto header = MakeHeader(Category::ServerToClient, MessageId::GetScriptReply, 0, true);
    Bytes response;
    if (!codec_.EncodeGameReply(header, snapshot.script, true, Z_DEFAULT_COMPRESSION, &response, &error))
        return HandleResult(false, false, Bytes(), error);
    return HandleResult(true, true, std::move(response), std::string());
}

HandleResult ChannelService::HandleAskChannelInfo() {
    const auto groups = registry_.Snapshot();
    Bytes body;
    const auto count = static_cast<std::int32_t>(groups.size());
    AppendPod(body, count);
    const auto serialized = ChannelRegistry::SerializeSnapshot(groups);
    AppendBytes(body, serialized.data(), serialized.size());
    std::string error;
    auto header = MakeHeader(Category::ServerToClient, MessageId::AskChannelInfoReply, 0, true);
    Bytes response;
    if (!codec_.EncodeGameReply(header, body, true, Z_BEST_SPEED, &response, &error))
        return HandleResult(false, false, Bytes(), error);
    return HandleResult(true, true, std::move(response), std::string());
}

HandleResult ChannelService::HandleGetServerGroupChannelInfo(const Bytes& frame) {
    if (frame.size() != sizeof(CsGetServerGroupChannelInfo)) {
        return HandleResult(false, false, Bytes(), "bad GET_SERVER_GROUP_CHANNEL_INFO size");
    }
    CsGetServerGroupChannelInfo request{};
    if (!ReadPod(frame, &request)) {
        return HandleResult(false, false, Bytes(), "cannot read GET_SERVER_GROUP_CHANNEL_INFO");
    }
    // DNF12 defines 265 as SERVER_GROUP_TYPE_MAX, and the client rejects that
    // sentinel at 0xAB6CBB before it consumes any channel record.
    if (request.parser_server_group == 0 || request.parser_server_group >= 265) {
        return HandleResult(false, false, Bytes(), "invalid parser server group");
    }
    if (!std::memchr(request.server_group_name, '\0', sizeof(request.server_group_name))) {
        return HandleResult(false, false, Bytes(), "unterminated server group name");
    }
    const std::string requested_name =
        FixedString(request.server_group_name, sizeof(request.server_group_name));
    if (requested_name.empty()) {
        return HandleResult(false, false, Bytes(), "empty server group name");
    }

    const std::vector<ChannelGroupSnapshot> groups = registry_.Snapshot();
    const ChannelGroupSnapshot* selected = nullptr;
    for (std::size_t index = 0; index < groups.size(); ++index) {
        if (groups[index].name != requested_name) continue;
        if (selected) {
            return HandleResult(false, false, Bytes(), "duplicate server group name");
        }
        selected = &groups[index];
    }
    if (!selected || selected->channels.empty()) {
        return HandleResult(false, false, Bytes(), "requested server group is unavailable");
    }

    Bytes body;
    const std::uint16_t fallback_server_group =
        static_cast<std::uint16_t>(request.parser_server_group);
    const std::uint32_t channel_count =
        static_cast<std::uint32_t>(selected->channels.size());
    AppendPod(body, fallback_server_group);
    AppendPod(body, channel_count);
    for (std::size_t index = 0; index < selected->channels.size(); ++index) {
        const ChannelEndpoint& channel = selected->channels[index];
        ServerInfoWire server{};
        // The client extracts decimal digits from this field at 0xAB5A70 and
        // resolves the resulting channel number through its local CN
        // etc/channel_info.etc table.  Localized dname/chn strings therefore
        // stay PVF-owned and are not copied into message 18.
        CopyFixed(server.channel_name, "#ch." + std::to_string(channel.channel_no));
        server.max_user_num = channel.max_users;
        server.cur_user_num = channel.current_users;
        CopyFixed(server.server_ip, channel.ip);
        server.port = channel.port;
        AppendPod(body, server);
    }

    std::string error;
    Bytes response;
    const PacketHeader header = MakeHeader(
        Category::ServerToClient, MessageId::GetServerGroupChannelInfoReply, 0, true);
    if (!codec_.EncodeGameReply(header, body, true, Z_BEST_COMPRESSION, &response, &error)) {
        return HandleResult(false, false, Bytes(), error);
    }
    // DNF.exe:0xAB57D3 accepts no more than 0x20000 compressed bytes.
    if (response.size() - sizeof(PacketHeader) > 0x20000U) {
        return HandleResult(false, false, Bytes(), "server-group response exceeds client limit");
    }
    return HandleResult(true, true, std::move(response), std::string());
}

HandleResult ChannelService::HandleUpdateChannelInfo(const Bytes& frame) {
    if (frame.size() != sizeof(CsUpdateChannelInfo)) {
        return HandleResult(false, false, Bytes(), "bad UPDATE_CHANNEL_INFO size");
    }
    CsUpdateChannelInfo update{};
    std::string error;
    if (!ReadPod(frame, &update) || !registry_.ApplyChannelUpdate(update, &error)) {
        return HandleResult(false, false, Bytes(), error);
    }
    return HandleResult(true, false, Bytes(), std::string());
}

HandleResult ChannelSession::HandleGameFrame(const Bytes& frame) {
    PacketHeader header{};
    if (!ValidateFrame(frame) || !ReadHeader(frame, &header)) {
        return HandleResult(false, false, Bytes(), "invalid session frame");
    }
    const MessageId message = static_cast<MessageId>(header.message_id);
    HandleResult result = service_.HandleGameFrame(frame);
    if (message == MessageId::Connect && result.accepted && result.has_response) {
        ScConnect response{};
        if (ReadPod(result.response, &response) && response.is_valid_key == 1) {
            authenticated_ = true;
        }
    }
    return result;
}

} }  // namespace dnf::channel
