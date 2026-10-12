#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#include "old_data/PVF/data/datamanager/script_sync_packets.h"
#include "old_data/channel/legacy_runtime/channel_proxy.h"
#include "core/config/env/environment.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketHeader.h"

namespace {

const int kLegacyFrameHeaderSize = 11;
const int kVersionReplySize = 31;
const uint8_t kVersionReplyId = 6u;
const uint8_t kScriptReplyId = 10u;

bool WriteAtomically(const char* path, const void* data, size_t size) {
    if (!path || !data || size == 0u) return false;
    char temporary[256];
    const int written = std::snprintf(
        temporary, sizeof(temporary), "%s.tmp.%ld", path,
        static_cast<long>(::getpid()));
    if (written <= 0 || written >= static_cast<int>(sizeof(temporary)))
        return false;

    FILE* const stream = std::fopen(temporary, "wb");
    if (!stream) return false;
    const bool complete =
        std::fwrite(data, 1u, size, stream) == size &&
        std::fflush(stream) == 0 && ::fsync(::fileno(stream)) == 0;
    const bool closed = std::fclose(stream) == 0;
    if (!complete || !closed || std::rename(temporary, path) != 0) {
        std::remove(temporary);
        return false;
    }
    return true;
}

}  // namespace

// IDA_EA: 0x81060F4
// IDA_NAME: _ZN15CmmChannelProxyC2ESsi
// DEMANGLED: CmmChannelProxy::CmmChannelProxy(std::string,int)
CmmChannelProxy::CmmChannelProxy(std::string bridge_ip, int udp_port_channel)
    : bridge_ip_(bridge_ip),
      udp_port_channel_(udp_port_channel),
      udp_handler_(),
      bridge_socket_(-1),
      recv_size_(0) {}

// IDA_EA: 0x810614A
// IDA_NAME: _ZN15CmmChannelProxy4InitEv
// DEMANGLED: CmmChannelProxy::Init(void)
int CmmChannelProxy::Init() {
    if (udp_handler_.InitClientSocket() == -1) {
        return 0;
    }
    recv_size_ = 0;
    std::memset(recv_buffer_, 0, sizeof(recv_buffer_));
    return 1;
}

// IDA_EA: 0x810619E
// IDA_NAME: _ZN15CmmChannelProxy15SendChannelInfoEv
// DEMANGLED: CmmChannelProxy::SendChannelInfo(void)
int CmmChannelProxy::SendChannelInfo() {
    tagCS_UPDATE_CHANNEL_INFO packet;
    CEnvironment* environment = G_CEnvironment();

    // IDA 0x81061BC..0x8106227: bridge registration is populated directly
    // from CEnvironment's old ABI fields (+408/+888/+432/+960/+436/+456).
    // Preserve that identity payload; channel_type/channel_name belong to
    // the PVF channel script and are not part of tagCS_UPDATE_CHANNEL_INFO.
    packet.gc_no = environment->get_gc_no();
    packet.server_group = environment->get_server_group();
    packet.channel_no = environment->get_channel_no();
    packet.max_user_num = environment->get_max_user_num();
    packet.current_user_count = G_GameWorld()->get_UserCount_InWorld();
    std::strcpy(packet.server_ip, environment->get_server_ip());
    packet.tcp_port = environment->get_tcp_port();

    udp_handler_.SendToServer(
        reinterpret_cast<char*>(&packet),
        static_cast<size_t>(packet.getSize()),
        static_cast<uint16_t>(udp_port_channel_),
        bridge_ip_.c_str());
    return 1;
}

// IDA_EA: 0x8106278
// IDA_NAME: _ZN15CmmChannelProxy19ConnectBridgeServerEv
// DEMANGLED: CmmChannelProxy::ConnectBridgeServer(void)
int CmmChannelProxy::ConnectBridgeServer() {
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));

    bridge_socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (bridge_socket_ < 0) {
        std::puts("[CmmChannelProxy::SendCheckChannelScript] Can't create socket");
        return 0;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ::inet_addr(bridge_ip_.c_str());
    CEnvironment* environment = G_CEnvironment();
    const uint16_t tcp_port =
        environment
            ? static_cast<uint16_t>(environment->get_tcp_port_channel())
            : 0;
    address.sin_port = htons(tcp_port);

    if (address.sin_addr.s_addr == INADDR_NONE || tcp_port == 0u) {
        ::close(bridge_socket_);
        bridge_socket_ = -1;
        return 0;
    }

    if (::connect(bridge_socket_, reinterpret_cast<sockaddr*>(&address),
                  sizeof(address)) < 0) {
        std::puts("[CmmChannelProxy::SendCheckChannelScript] Can't connect channel bridge server");
        ::close(bridge_socket_);
        bridge_socket_ = -1;
        return 0;
    }

    int non_blocking = 1;
    if (::ioctl(bridge_socket_, FIONBIO, &non_blocking) != 0) {
        ::close(bridge_socket_);
        bridge_socket_ = -1;
        return 0;
    }
    return 1;
}

// IDA_EA: 0x810637C
// IDA_NAME: _ZN15CmmChannelProxy10SendPacketEPci
// DEMANGLED: CmmChannelProxy::SendPacket(char *,int)
int CmmChannelProxy::SendPacket(char* buffer, int bytes) {
    if (!buffer || bytes <= 0 || bridge_socket_ < 0) return -1;
    int total = 0;
    while (total < bytes) {
        const ssize_t sent = ::write(
            bridge_socket_, buffer + total,
            static_cast<size_t>(bytes - total));
        if (sent > 0) {
            total += static_cast<int>(sent);
            continue;
        }
        if (sent < 0 && errno == EINTR) continue;
        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(bridge_socket_, &writefds);
            timeval timeout;
            timeout.tv_sec = 30;
            timeout.tv_usec = 0;
            const int selected = ::select(
                bridge_socket_ + 1, 0, &writefds, 0, &timeout);
            if (selected > 0) continue;
        }

        const int saved_errno = errno;
        cMyTrace trace("int CmmChannelProxy::SendPacket(char*, int)", 200, 0);
        trace("SEND ERROR DISCONNNECT NOW FD[%d] : %d(%s)", bridge_socket_,
              saved_errno, std::strerror(saved_errno));
        return -1;
    }
    return total;
}

// IDA_EA: 0x810645C
// IDA_NAME: _ZN15CmmChannelProxy10RecvPacketEv
// DEMANGLED: CmmChannelProxy::RecvPacket(void)
int CmmChannelProxy::RecvPacket() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(bridge_socket_, &readfds);

    timeval timeout;
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

    const int selected = ::select(bridge_socket_ + 1, &readfds, 0, 0, &timeout);
    if (selected <= 0) {
        return 0;
    }

    int socket_error = 0;
    socklen_t optlen = sizeof(socket_error);
    ::getsockopt(bridge_socket_, SOL_SOCKET, SO_ERROR, &socket_error, &optlen);
    ::usleep(100000);

    const int remaining = 0x80000 - recv_size_;
    const ssize_t received = ::read(bridge_socket_, recv_buffer_ + recv_size_,
                                    static_cast<size_t>(remaining));
    if (received >= 0) {
        if (received != 0) {
            recv_size_ += static_cast<int>(received);
            return static_cast<int>(received);
        }

        const int saved_errno = errno;
        cMyTrace trace("int CmmChannelProxy::RecvPacket()", 282, 0);
        trace("Recv ERROR = 0 (%d) : %s, MaxRead(%d) nRead(%d)", saved_errno,
              std::strerror(saved_errno), remaining, static_cast<int>(received));
        return -1;
    }

    if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK || errno == 0) {
        return 0;
    }

    const int saved_errno = errno;
    cMyTrace trace("int CmmChannelProxy::RecvPacket()", 276, 0);
    trace("RECV ERROR DISCONNNECT NOW FD[%d] : %d(%s)", bridge_socket_,
          saved_errno, std::strerror(saved_errno));
    return -1;
}

// IDA_EA: 0x81066EA
// IDA_NAME: _ZN15CmmChannelProxy22SendCheckChannelScriptEv
// DEMANGLED: CmmChannelProxy::SendCheckChannelScript(void)
int CmmChannelProxy::SendCheckChannelScript() {
    if (!ConnectBridgeServer()) return 0;
    const auto fail = [this]() -> int {
        if (bridge_socket_ >= 0) {
            ::close(bridge_socket_);
            bridge_socket_ = -1;
        }
        return 0;
    };
    const auto receive_frame = [this](uint8_t expected_id,
                                      int minimum_size,
                                      int exact_size) -> bool {
        while (true) {
            if (recv_size_ >= kLegacyFrameHeaderSize) {
                tagPacketHeader* const header =
                    reinterpret_cast<tagPacketHeader*>(recv_buffer_);
                const int declared = header->getSize();
                if (header->packet_id != expected_id ||
                    declared < minimum_size ||
                    declared > static_cast<int>(sizeof(recv_buffer_)) ||
                    (exact_size > 0 && declared != exact_size) ||
                    recv_size_ > declared) {
                    return false;
                }
                if (recv_size_ == declared) return true;
            }
            if (RecvPacket() <= 0) return false;
        }
    };

    char directory[20];
    std::memset(directory, 0, sizeof(directory));
    std::strncpy(directory, "channel_info", sizeof(directory));

    struct stat stat_buffer;
    if (::stat(directory, &stat_buffer) < 0) {
        if (errno != ENOENT) {
            return fail();
        }
        if (::mkdir(directory, 0755) != 0) return fail();
    } else if (!S_ISDIR(stat_buffer.st_mode)) {
        return fail();
    }

    char source[1024];
    std::memset(source, 0, sizeof(source));

    FILE* stream = std::fopen("channel_info/version", "rb");
    if (stream) {
        char line[1024];
        while (std::fgets(line, sizeof(line), stream)) {
            char* begin = line;
            while (*begin == ' ' || *begin == '\t') ++begin;
            if (*begin == 0 || *begin == '\n' || *begin == '\r' ||
                *begin == '#') continue;
            char* end = begin + std::strlen(begin);
            while (end > begin &&
                   (end[-1] == ' ' || end[-1] == '\t' ||
                    end[-1] == '\n' || end[-1] == '\r')) {
                --end;
            }
            const size_t length = static_cast<size_t>(end - begin);
            if (length > 0u) {
                std::memcpy(source, begin,
                            length < 16u ? length : 15u);
                break;
            }
        }
        std::fclose(stream);
    }

    char check_packet[27];
    std::memset(check_packet, 0, sizeof(check_packet));
    new (check_packet) tagCS_CHECK_SCRIPT_VERSION();
    std::strncpy(check_packet + 11, source, 0x10u);

    const int check_size =
        reinterpret_cast<tagPacketHeader*>(check_packet)->getSize();
    if (SendPacket(check_packet, check_size) != check_size ||
        !receive_frame(kVersionReplyId, kVersionReplySize,
                       kVersionReplySize)) {
        return fail();
    }

    const char* const received_version = recv_buffer_ + 15;
    size_t received_version_size = 0u;
    while (received_version_size < 16u &&
           received_version[received_version_size] != 0) {
        const unsigned char value = static_cast<unsigned char>(
            received_version[received_version_size]);
        if (value < 0x20u || value > 0x7eu) return fail();
        ++received_version_size;
    }
    if (received_version_size == 0u || received_version_size == 16u)
        return fail();
    char version_to_commit[16];
    std::memset(version_to_commit, 0, sizeof(version_to_commit));
    std::memcpy(version_to_commit, received_version,
                received_version_size);

    recv_size_ = 0;
    std::memset(recv_buffer_, 0, sizeof(recv_buffer_));

    char get_script_packet[11];
    std::memset(get_script_packet, 0, sizeof(get_script_packet));
    new (get_script_packet) tagCS_GET_SCRIPT();

    const int get_script_size =
        reinterpret_cast<tagPacketHeader*>(get_script_packet)->getSize();
    if (SendPacket(get_script_packet, get_script_size) != get_script_size ||
        !receive_frame(kScriptReplyId, kLegacyFrameHeaderSize + 1, 0)) {
        return fail();
    }

    tagPacketHeader* const script_response =
        reinterpret_cast<tagPacketHeader*>(recv_buffer_);
    const int script_size = script_response->getSize();
    const size_t script_body_size = static_cast<size_t>(
        script_size - kLegacyFrameHeaderSize);
    if (script_body_size == 0u ||
        std::memchr(recv_buffer_ + kLegacyFrameHeaderSize, 0,
                    script_body_size) != 0 ||
        !WriteAtomically("channel_info/channel_info.etc",
                         recv_buffer_ + kLegacyFrameHeaderSize,
                         script_body_size) ||
        !WriteAtomically("channel_info/version", version_to_commit,
                         received_version_size)) {
        return fail();
    }
    ::close(bridge_socket_);
    bridge_socket_ = -1;
    return 1;
}
