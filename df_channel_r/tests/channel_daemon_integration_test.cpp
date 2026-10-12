#include "channel/legacy_codec.hpp"
#include "channel/protocol.hpp"
#include "channel/daemon_config.hpp"

#include <zlib.h>

#include <atomic>
#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace dnf::channel;

namespace {

bool SendAll(int fd, const Bytes& data) {
    std::size_t offset = 0;
    while (offset < data.size()) {
        const ssize_t sent = ::send(fd, data.data() + offset, data.size() - offset, MSG_NOSIGNAL);
        if (sent > 0) offset += static_cast<std::size_t>(sent);
        else if (sent < 0 && errno == EINTR) continue;
        else return false;
    }
    return true;
}

bool RecvExact(int fd, void* output, std::size_t size, int timeout_ms) {
    std::size_t offset = 0;
    while (offset < size) {
        struct pollfd item = {fd, POLLIN, 0};
        const int ready = ::poll(&item, 1, timeout_ms);
        if (ready <= 0 || (item.revents & (POLLERR | POLLHUP | POLLNVAL))) return false;
        const ssize_t received = ::recv(fd, static_cast<char*>(output) + offset, size - offset, 0);
        if (received > 0) offset += static_cast<std::size_t>(received);
        else if (received < 0 && errno == EINTR) continue;
        else return false;
    }
    return true;
}

bool RecvFrame(int fd, Bytes* frame, int timeout_ms = 5000) {
    if (!frame) return false;
    PacketHeader header{};
    if (!RecvExact(fd, &header, sizeof(header), timeout_ms) ||
        header.size < sizeof(header) || header.size > 0xA0000U) return false;
    frame->resize(header.size);
    std::memcpy(frame->data(), &header, sizeof(header));
    return RecvExact(fd, frame->data() + sizeof(header), frame->size() - sizeof(header), timeout_ms);
}

template <std::size_t N>
void CopyFixed(char (&destination)[N], const std::string& source) {
    std::memset(destination, 0, N);
    std::memcpy(destination, source.data(), std::min(source.size(), N - 1));
}

int BoundPort(int type, int* retained_fd, int requested_port = 0) {
    const int fd = ::socket(AF_INET, type, 0);
    assert(fd >= 0);
    int enabled = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<std::uint16_t>(requested_port));
    assert(::bind(fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == 0);
    socklen_t length = sizeof(address);
    assert(getsockname(fd, reinterpret_cast<struct sockaddr*>(&address), &length) == 0);
    const int port = ntohs(address.sin_port);
    if (retained_fd) *retained_fd = fd;
    else ::close(fd);
    return port;
}

void WriteFile(const std::string& path, const std::string& content) {
    std::ofstream output(path.c_str(), std::ios::binary | std::ios::trunc);
    output.write(content.data(), static_cast<std::streamsize>(content.size()));
    assert(output.good());
}

void CopyFile(const std::string& source, const std::string& destination) {
    std::ifstream input(source.c_str(), std::ios::binary);
    std::ofstream output(destination.c_str(), std::ios::binary | std::ios::trunc);
    output << input.rdbuf();
    assert(input.good() || input.eof());
    assert(output.good());
}

struct ChildGuard {
    ChildGuard() : pid(-1) {}
    ~ChildGuard() {
        if (pid > 0) {
            ::kill(pid, SIGTERM);
            for (int index = 0; index < 50; ++index) {
                if (::waitpid(pid, nullptr, WNOHANG) == pid) {
                    pid = -1;
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            ::kill(pid, SIGKILL);
            ::waitpid(pid, nullptr, 0);
        }
    }
    pid_t pid;
};

struct MockBridge {
    MockBridge(int socket_fd, int game_port)
        : listener(socket_fd), advertised_game_port(game_port), stop(false), saw_gc(false),
          saw_version(false), saw_notice(false) {}

    void Run() {
        assert(::listen(listener, 8) == 0);
        struct pollfd listen_item = {listener, POLLIN, 0};
        if (::poll(&listen_item, 1, 5000) <= 0) return;
        int fd = ::accept(listener, nullptr, nullptr);
        if (fd < 0) return;
        bool forwarded = false;
        while (!stop.load()) {
            Bytes frame;
            if (!RecvFrame(fd, &frame, 1000)) continue;
            PacketHeader header{};
            if (!ReadHeader(frame, &header)) continue;
            if (header.message_id == static_cast<std::uint8_t>(MessageId::GetGcInfo)) {
                saw_gc.store(true);
                ScGetGcInfo reply{};
                reply.header = MakeHeader(Category::ServerToClient, MessageId::GetGcInfoReply,
                                          sizeof(reply) + sizeof(ServerGcInfoWire), true);
                reply.count = 1;
                ServerGcInfoWire item{};
                CopyFixed(item.server_group_name, "siroco");
                item.gc_no = 4000;
                Bytes response = PodFrame(reply);
                AppendPod(response, item);
                assert(SendAll(fd, response));
                if (!forwarded) {
                    CsUpdateChannelInfo update{};
                    update.header = MakeHeader(Category::ClientToServer, MessageId::UpdateChannelInfo,
                                               sizeof(update));
                    update.gc_no = 3186;
                    update.group = 17;
                    update.channel_no = 186;
                    update.max_user_num = 1000;
                    update.cur_user_num = 1;
                    CopyFixed(update.server_ip, "127.0.0.1");
                    update.port = advertised_game_port;
                    assert(SendAll(fd, PodFrame(update)));
                    forwarded = true;
                }
            } else if (header.message_id == static_cast<std::uint8_t>(MessageId::CheckScriptVersion)) {
                saw_version.store(true);
                ScCheckScriptVersion reply{};
                reply.header = MakeHeader(Category::ServerToClient, MessageId::CheckScriptVersionReply,
                                          sizeof(reply), true);
                reply.is_valid_version = 1;
                CopyFixed(reply.channel_script_version, "60");
                assert(SendAll(fd, PodFrame(reply)));
            } else if (header.message_id == static_cast<std::uint8_t>(MessageId::NoticeChannelServer)) {
                saw_notice.store(true);
            }
        }
        ::close(fd);
    }

    int listener;
    int advertised_game_port;
    std::atomic<bool> stop;
    std::atomic<bool> saw_gc;
    std::atomic<bool> saw_version;
    std::atomic<bool> saw_notice;
};

int ConnectRetry(int port) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd >= 0) {
            struct sockaddr_in address;
            std::memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = htons(static_cast<std::uint16_t>(port));
            if (::connect(fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == 0) return fd;
            ::close(fd);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return -1;
}

void SendUdpUpdate(int port, int gc_no, int channel_no) {
    const int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    assert(fd >= 0);
    CsUpdateChannelInfo update{};
    update.header = MakeHeader(Category::ClientToServer, MessageId::UpdateChannelInfo, sizeof(update));
    update.gc_no = gc_no;
    update.group = 17;
    update.channel_no = channel_no;
    update.max_user_num = 1000;
    update.cur_user_num = 2;
    CopyFixed(update.server_ip, "127.0.0.1");
    update.port = 19001;
    const Bytes frame = PodFrame(update);
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    assert(::sendto(fd, frame.data(), frame.size(), 0,
                    reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) ==
           static_cast<ssize_t>(frame.size()));
    ::close(fd);
}

std::unique_ptr<LegacyCodec> Authenticate(int fd) {
    CsConnect initial{};
    initial.header = MakeHeader(Category::ClientToServer, MessageId::Connect, sizeof(initial));
    initial.header.classification = 0;
    assert(SendAll(fd, PodFrame(initial)));
    Bytes response;
    assert(RecvFrame(fd, &response));
    ScConnect reply{};
    assert(response.size() == sizeof(reply) && ReadPod(response, &reply));
    assert(reply.header.classification == static_cast<std::uint8_t>(Category::ServerToClient));
    assert(reply.header.message_id == static_cast<std::uint8_t>(MessageId::ConnectReply));
    assert(reply.header.size == sizeof(reply) && reply.header.ack == 1);
    assert(reply.is_valid_key == 0);

    std::array<std::uint8_t, 32> key{};
    std::memcpy(key.data(), reply.key, key.size());
    std::unique_ptr<LegacyCodec> codec(new LegacyCodec(key));
    return codec;
}

void Cleanup(const std::string& root) {
    const char* files[] = {
        "channel_info/channel_info.etc", "channel_info/version",
        "log/Info_smoke.log", "log/Warn_smoke.log", "log/Error_smoke.log", "log/Cri_smoke.log"};
    for (std::size_t index = 0; index < sizeof(files) / sizeof(files[0]); ++index)
        ::unlink((root + "/" + files[index]).c_str());
    ::rmdir((root + "/log/old_log").c_str());
    ::rmdir((root + "/log").c_str());
    ::rmdir((root + "/channel_info").c_str());
}

}  // namespace

int main(int argc, char** argv) {
    assert(argc == 4);
    char daemon_path[PATH_MAX] = {};
    char table_path[PATH_MAX] = {};
    char root_path[PATH_MAX] = {};
    assert(realpath(argv[1], daemon_path));
    assert(realpath(argv[2], table_path));
    assert(realpath(argv[3], root_path));

    const std::string root(root_path);
    const std::string config_path = root + "/cfg/smoke.cfg";

    DaemonConfig daemon_config;
    std::string config_error;
    assert(LoadDaemonConfig(config_path, &daemon_config, &config_error));

    if (::mkdir((root + "/channel_info").c_str(), 0755) != 0) {
        assert(errno == EEXIST);
    }

    int bridge_listener = -1;
    const int bridge_port = BoundPort(SOCK_STREAM, &bridge_listener,
                                      daemon_config.bridge_port);
    const int tcp_port = BoundPort(SOCK_STREAM, nullptr,
                                   daemon_config.this_tcp_port);
    const int udp_port = BoundPort(SOCK_DGRAM, nullptr,
                                   daemon_config.this_udp_port);
    assert(bridge_port == daemon_config.bridge_port);
    assert(tcp_port == daemon_config.this_tcp_port);
    assert(udp_port == daemon_config.this_udp_port);
    CopyFile(table_path, root + "/channel_info/channel_info.etc");
    WriteFile(root + "/channel_info/version", "60\n");

    MockBridge bridge(bridge_listener, tcp_port);
    std::thread bridge_thread(&MockBridge::Run, &bridge);
    ChildGuard child;
    child.pid = ::fork();
    assert(child.pid >= 0);
    if (child.pid == 0) {
        assert(::chdir(root.c_str()) == 0);
        execl(daemon_path, daemon_path, "smoke", "run", static_cast<char*>(nullptr));
        _exit(127);
    }

    const int game_fd = ConnectRetry(tcp_port);
    assert(game_fd >= 0);
    for (int attempt = 0; attempt < 100 && !bridge.saw_gc.load(); ++attempt)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(bridge.saw_gc.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SendUdpUpdate(udp_port, 3187, 187);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::unique_ptr<LegacyCodec> codec = Authenticate(game_fd);
    assert(codec.get());
    const Bytes ask = PodFrame(MakeHeader(Category::ClientToServer, MessageId::AskChannelInfo,
                                           sizeof(PacketHeader)));
    Bytes real_client_ask = ask;
    real_client_ask[0] = 0;
    assert(::send(game_fd, real_client_ask.data(), 5, MSG_NOSIGNAL) == 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    assert(SendAll(game_fd, Bytes(real_client_ask.begin() + 5, real_client_ask.end())));
    Bytes response;
    assert(RecvFrame(game_fd, &response));
    PacketHeader header{};
    assert(ReadHeader(response, &header));
    assert(header.classification == static_cast<std::uint8_t>(Category::ServerToClient));
    assert(header.message_id == static_cast<std::uint8_t>(MessageId::AskChannelInfoReply));
    Bytes encrypted(0xA0000U);
    uLongf encrypted_size = static_cast<uLongf>(encrypted.size());
    assert(uncompress(encrypted.data(), &encrypted_size, response.data() + sizeof(PacketHeader),
                      static_cast<uLong>(response.size() - sizeof(PacketHeader))) == Z_OK);
    encrypted.resize(encrypted_size);
    Bytes plaintext;
    std::string error;
    assert(codec->Decrypt(encrypted.data(), encrypted.size(), &plaintext, &error));
    std::int32_t group_count = 0;
    ServerGroupInfoWire group{};
    assert(ReadPod(plaintext, &group_count));
    assert(ReadPod(plaintext, &group, sizeof(group_count)));
    // The original UDP dispatcher has no registered extra protocol; the UDP
    // update above is deliberately ignored.  Only Bridge-forwarded 3186 exists.
    assert(group_count == 1 && group.server_count == 1);
    assert(bridge.saw_version.load() && bridge.saw_notice.load());

    ::close(game_fd);
    ::kill(child.pid, SIGTERM);
    assert(::waitpid(child.pid, nullptr, 0) == child.pid);
    child.pid = -1;
    bridge.stop.store(true);
    ::shutdown(bridge_listener, SHUT_RDWR);
    ::close(bridge_listener);
    bridge_thread.join();
    Cleanup(root);
    std::cout << "channel_daemon_integration_test: PASS\n";
    return 0;
}
