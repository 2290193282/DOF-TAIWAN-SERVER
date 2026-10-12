#include "channel/channel_client.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>

#ifndef _WIN32
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace dnf { namespace channel {
namespace {

template <std::size_t N>
void CopyFixed(char (&destination)[N], const std::string& source) {
    std::memset(destination, 0, N);
    std::memcpy(destination, source.data(), std::min(source.size(), N - 1));
}

}  // namespace

Bytes BridgeProtocolClient::BuildGetGcInfo() const {
    return PodFrame(MakeHeader(Category::ClientToServer, MessageId::GetGcInfo, sizeof(PacketHeader)));
}

Bytes BridgeProtocolClient::BuildCheckScriptVersion() const {
    CsCheckScriptVersion packet{};
    packet.header = MakeHeader(Category::ClientToServer, MessageId::CheckScriptVersion, sizeof(packet));
    CopyFixed(packet.channel_script_version, scripts_.Snapshot().version);
    return PodFrame(packet);
}

Bytes BridgeProtocolClient::BuildNotice() const {
    CsNoticeChannelServer packet{};
    packet.header = MakeHeader(Category::ClientToServer, MessageId::NoticeChannelServer, sizeof(packet));
    packet.id = identity_.id;
    CopyFixed(packet.server_ip, identity_.advertised_ip);
    packet.port = identity_.udp_port;
    return PodFrame(packet);
}

Bytes BridgeProtocolClient::BuildGetScript() const {
    return PodFrame(MakeHeader(Category::ClientToServer, MessageId::GetScript, sizeof(PacketHeader)));
}

std::vector<Bytes> BridgeProtocolClient::BuildInitialFrames() const {
    std::vector<Bytes> result;
    result.push_back(BuildGetGcInfo());
    result.push_back(BuildCheckScriptVersion());
    result.push_back(BuildNotice());
    return result;
}

BridgeHandleResult BridgeProtocolClient::HandleFrame(const Bytes& frame) {
    BridgeHandleResult result;
    PacketHeader header{};
    if (!ValidateFrame(frame) || !ReadHeader(frame, &header)) {
        result.error = "invalid Bridge frame";
        return result;
    }
    // Bridge forwards game-server message 2 over the same TCP connection.
    if (header.classification == static_cast<std::uint8_t>(Category::ClientToServer) &&
        header.message_id == static_cast<std::uint8_t>(MessageId::UpdateChannelInfo)) {
        if (!connection_gc_ready_) {
            result.accepted = true;
            result.error = "GC table is not ready for this Bridge connection";
            return result;
        }
        CsUpdateChannelInfo update{};
        if (frame.size() != sizeof(update) || !ReadPod(frame, &update)) {
            result.error = "bad forwarded channel update";
            return result;
        }
        result.accepted = registry_.ApplyChannelUpdate(update, &result.error);
        // Strict 0828 mode drops an early update but keeps the Bridge transport
        // alive so message 14 can establish the authoritative GC table.
        if (!result.accepted && result.error == "GC table is not ready") result.accepted = true;
        return result;
    }
    if (header.classification != static_cast<std::uint8_t>(Category::ServerToClient)) {
        result.error = "unexpected Bridge frame category";
        return result;
    }
    if (header.message_id == static_cast<std::uint8_t>(MessageId::GetGcInfoReply)) {
        result.accepted = registry_.ApplyGcInfoReply(frame, &result.error);
        if (result.accepted) {
            connection_gc_ready_ = true;
            result.gc_ready = true;
        }
        return result;
    }
    if (header.message_id == static_cast<std::uint8_t>(MessageId::CheckScriptVersionReply)) {
        ScCheckScriptVersion response{};
        if (frame.size() != sizeof(response) || !ReadPod(frame, &response)) {
            result.error = "bad Bridge script-version response";
            return result;
        }
        result.accepted = true;
        if (response.is_valid_version == 0) {
            pending_version_ = FixedString(response.channel_script_version, 16);
            if (pending_version_.empty()) {
                result.accepted = false;
                result.error = "Bridge returned an empty version";
                return result;
            }
            result.outbound.push_back(BuildGetScript());
        }
        return result;
    }
    if (header.message_id == static_cast<std::uint8_t>(MessageId::GetScriptReply)) {
        if (pending_version_.empty()) {
            result.error = "unsolicited Bridge script";
            return result;
        }
        Bytes script(frame.begin() + sizeof(PacketHeader), frame.end());
        result.accepted = scripts_.ReplaceFromBridge(pending_version_, script, &result.error);
        if (result.accepted) pending_version_.clear();
        return result;
    }
    result.error = "unsupported Bridge response";
    return result;
}

void BridgeProtocolClient::Reset() {
    pending_version_.clear();
    connection_gc_ready_ = false;
}

PosixBridgeConnection::PosixBridgeConnection() : fd_(-1) {}
PosixBridgeConnection::~PosixBridgeConnection() { Close(); }

bool PosixBridgeConnection::Connect(const std::string& host, int port, std::string* error) {
    Close();
#ifdef _WIN32
    if (error) *error = "POSIX transport is unavailable on Windows";
    return false;
#else
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* addresses = nullptr;
    const std::string service = std::to_string(port);
    const int status = getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses);
    if (status != 0) {
        if (error) *error = gai_strerror(status);
        return false;
    }
    for (struct addrinfo* current = addresses; current; current = current->ai_next) {
        const int candidate = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
        if (candidate < 0) continue;
        if (connect(candidate, current->ai_addr, current->ai_addrlen) == 0) {
            fd_ = candidate;
            break;
        }
        close(candidate);
    }
    freeaddrinfo(addresses);
    if (fd_ < 0) {
        if (error) *error = std::strerror(errno);
        return false;
    }
    return true;
#endif
}

bool PosixBridgeConnection::Send(const Bytes& frame, std::string* error) {
#ifdef _WIN32
    if (error) *error = "POSIX transport is unavailable on Windows";
    return false;
#else
    std::size_t offset = 0;
    while (offset < frame.size()) {
        const ssize_t sent = send(fd_, frame.data() + offset, frame.size() - offset, MSG_NOSIGNAL);
        if (sent < 0 && errno == EINTR) continue;
        if (sent <= 0) {
            if (error) *error = std::strerror(errno);
            return false;
        }
        offset += static_cast<std::size_t>(sent);
    }
    return true;
#endif
}

bool PosixBridgeConnection::Receive(Bytes* frame, std::string* error) {
    if (!frame) return false;
#ifdef _WIN32
    if (error) *error = "POSIX transport is unavailable on Windows";
    return false;
#else
    PacketHeader header{};
    std::size_t offset = 0;
    while (offset < sizeof(header)) {
        const ssize_t got = recv(fd_, reinterpret_cast<char*>(&header) + offset, sizeof(header) - offset, 0);
        if (got < 0 && errno == EINTR) continue;
        if (got <= 0) {
            if (error) *error = got == 0 ? "Bridge closed connection" : std::strerror(errno);
            return false;
        }
        offset += static_cast<std::size_t>(got);
    }
    if (header.size < sizeof(header) || header.size > 0xA0000U) {
        if (error) *error = "Bridge frame size out of range";
        return false;
    }
    frame->resize(header.size);
    std::memcpy(frame->data(), &header, sizeof(header));
    offset = sizeof(header);
    while (offset < frame->size()) {
        const ssize_t got = recv(fd_, frame->data() + offset, frame->size() - offset, 0);
        if (got < 0 && errno == EINTR) continue;
        if (got <= 0) {
            if (error) *error = got == 0 ? "Bridge closed connection" : std::strerror(errno);
            return false;
        }
        offset += static_cast<std::size_t>(got);
    }
    return true;
#endif
}

void PosixBridgeConnection::Close() {
#ifndef _WIN32
    if (fd_ >= 0) close(fd_);
#endif
    fd_ = -1;
}

} }  // namespace dnf::channel
