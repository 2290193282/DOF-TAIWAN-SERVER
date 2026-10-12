#include "dfbridge/service.hpp"

#include "dfbridge/bridge_core.hpp"
#include "dfbridge/config.hpp"
#include "dfbridge/protocol.hpp"
#include "dfbridge/repository.hpp"
#include "dfbridge/script_thread.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef _WIN32
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace dfbridge {

bool IsServiceCommand(const std::string& command) {
    return command == "start" || command == "run" || command == "test" ||
           command == "stop" || command == "pause" || command == "mess" ||
           command == "install" || command == "remove";
}

#ifdef _WIN32

int RunServiceCommand(const std::string&, const std::string&) {
    std::cerr << "df_bridge_r service mode requires Linux/POSIX\n";
    return 1;
}

#else
namespace {

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

volatile std::sig_atomic_t g_stop_requested = 0;

void StopSignalHandler(int) {
    g_stop_requested = 1;
}

void EnsureDirectory(const char* path) {
    if (::mkdir(path, 0755) != 0 && errno != EEXIST) {
        throw std::runtime_error(std::string("cannot create directory ") + path +
                                 ": " + std::strerror(errno));
    }
}

std::string ConfigPath(const std::string& instance_name) {
    return "./cfg/" + instance_name + ".cfg";
}

std::string PidPath(const std::string& instance_name) {
    return "./pid/" + instance_name + ".pid";
}

std::string Timestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    char text[32]{};
    std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", &local);
    return text;
}

class BridgeLogger {
public:
    enum Level { Info = 0, Warn = 1, Error = 2, Critical = 3 };

    void Open(const std::string& instance_name) {
        EnsureDirectory("./log");
        static const char* names[] = {"Info", "Warn", "Error", "Cri"};
        for (int i = 0; i < 4; ++i) {
            const std::string path =
                std::string("./log/") + names[i] + "_" + instance_name + ".log";
            files_[i].open(path.c_str(), std::ios::out | std::ios::app | std::ios::binary);
            if (!files_[i]) {
                throw std::runtime_error("cannot open bridge log: " + path);
            }
        }
    }

    void Write(const Level level, const std::string& message) {
        std::lock_guard<std::mutex> guard(mutex_);
        files_[static_cast<int>(level)] << Timestamp() << ' ' << message << '\n';
        files_[static_cast<int>(level)].flush();
    }

private:
    std::mutex mutex_;
    std::ofstream files_[4];
};

class FileDescriptor {
public:
    explicit FileDescriptor(const int fd = -1) : fd_(fd) {}
    ~FileDescriptor() {
        if (fd_ >= 0) ::close(fd_);
    }
    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;
    int Get() const { return fd_; }

private:
    int fd_;
};

long ReadPid(const std::string& path) {
    std::ifstream input(path.c_str());
    long pid = 0;
    input >> pid;
    return input ? pid : 0;
}

bool ProcessExists(const long pid) {
    if (pid <= 0) return false;
    if (::kill(static_cast<pid_t>(pid), 0) == 0) return true;
    return errno == EPERM;
}

std::string ProcessExeBasename(const long pid) {
    if (pid <= 0) return std::string();
    const std::string link = "/proc/" + std::to_string(pid) + "/exe";
    char target[4096]{};
    const ssize_t length = ::readlink(link.c_str(), target, sizeof(target) - 1);
    if (length <= 0) return std::string();
    target[length] = '\0';
    const std::string full(target);
    const std::string::size_type slash = full.find_last_of('/');
    return slash == std::string::npos ? full : full.substr(slash + 1);
}

bool IsExpectedBridgeProcess(const long pid) {
    return ProcessExeBasename(pid) == "df_bridge_r";
}

class PidFileGuard {
public:
    PidFileGuard(const std::string& path, const bool enabled)
        : path_(path), fd_(-1) {
        if (!enabled) return;
        fd_ = ::open(path_.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ < 0) {
            throw std::runtime_error("cannot open pid file: " + path_);
        }
        if (::flock(fd_, LOCK_EX | LOCK_NB) != 0) {
            const int failure = errno;
            ::close(fd_);
            fd_ = -1;
            throw std::runtime_error(std::string("cannot lock pid file: ") +
                                     std::strerror(failure));
        }
        const std::string value = std::to_string(static_cast<long>(::getpid())) + "\n";
        if (::ftruncate(fd_, 0) != 0 || ::lseek(fd_, 0, SEEK_SET) < 0 ||
            ::write(fd_, value.data(), value.size()) !=
                static_cast<ssize_t>(value.size()) ||
            ::fsync(fd_) != 0) {
            const int failure = errno;
            ::flock(fd_, LOCK_UN);
            ::close(fd_);
            fd_ = -1;
            throw std::runtime_error(std::string("cannot write pid file: ") +
                                     std::strerror(failure));
        }
    }
    ~PidFileGuard() {
        if (fd_ < 0) return;
        if (ReadPid(path_) == static_cast<long>(::getpid())) {
            std::remove(path_.c_str());
        }
        ::flock(fd_, LOCK_UN);
        ::close(fd_);
    }

    PidFileGuard(const PidFileGuard&) = delete;
    PidFileGuard& operator=(const PidFileGuard&) = delete;

private:
    std::string path_;
    int fd_;
};

void InstallRuntimeSignals() {
    g_stop_requested = 0;
    struct rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;
    core_limit.rlim_max = RLIM_INFINITY;
    ::setrlimit(RLIMIT_CORE, &core_limit);

    struct sigaction stop_action;
    std::memset(&stop_action, 0, sizeof(stop_action));
    stop_action.sa_handler = StopSignalHandler;
    sigemptyset(&stop_action.sa_mask);
    sigaction(SIGTERM, &stop_action, nullptr);
    sigaction(SIGINT, &stop_action, nullptr);

    struct sigaction pipe_action;
    std::memset(&pipe_action, 0, sizeof(pipe_action));
    pipe_action.sa_handler = SIG_IGN;
    sigemptyset(&pipe_action.sa_mask);
    sigaction(SIGPIPE, &pipe_action, nullptr);
}

void SetNonBlocking(const int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
        throw std::runtime_error(std::string("cannot set nonblocking socket: ") +
                                 std::strerror(errno));
    }
}

int CreateBoundSocket(const int type, const std::uint16_t port) {
    const int fd = ::socket(AF_INET, type, 0);
    if (fd < 0) {
        throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));
    }
    try {
        const int enabled = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
        SetNonBlocking(fd);
        sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        if (::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
            throw std::runtime_error(std::string("bind failed on port ") +
                                     std::to_string(port) + ": " +
                                     std::strerror(errno));
        }
        if (type == SOCK_STREAM && ::listen(fd, 1024) != 0) {
            throw std::runtime_error(std::string("listen failed: ") +
                                     std::strerror(errno));
        }
    } catch (...) {
        ::close(fd);
        throw;
    }
    return fd;
}

std::string FixedString(const char* value, const std::size_t capacity) {
    const void* end_value = std::memchr(value, '\0', capacity);
    const char* end = end_value ? static_cast<const char*>(end_value) : value + capacity;
    return std::string(value, end);
}

struct ClientConnection {
    int fd{-1};
    wire::FrameDecoder decoder;
    std::vector<std::uint8_t> output;
    std::size_t output_offset{};
    bool close_requested{};
    bool channel_registered{};
    std::int32_t channel_id{};
    std::chrono::system_clock::time_point last_notice{};
    std::string peer;
    bool first_frame_logged{};
    std::uint16_t listen_port{};
};

const std::size_t kMaximumQueuedBytes = 4U * 1024U * 1024U;

bool QueueFrame(ClientConnection* client, const std::vector<std::uint8_t>& frame) {
    if (!client) return false;
    if (client->output_offset == client->output.size()) {
        client->output.clear();
        client->output_offset = 0;
    } else if (client->output_offset != 0) {
        client->output.erase(client->output.begin(),
                             client->output.begin() + client->output_offset);
        client->output_offset = 0;
    }
    if (frame.size() > kMaximumQueuedBytes - client->output.size()) {
        client->close_requested = true;
        return false;
    }
    client->output.insert(client->output.end(), frame.begin(), frame.end());
    return true;
}

bool ValidateHeader(const std::vector<std::uint8_t>& frame, wire::PacketHeader* header,
                    BridgeLogger* logger) {
    if (frame.size() < sizeof(wire::PacketHeader)) return false;
    std::memcpy(header, frame.data(), sizeof(*header));
    if (header->length != frame.size()) {
        if (logger) logger->Write(BridgeLogger::Warn, "rejected malformed bridge frame");
        return false;
    }
    return true;
}

void DispatchTcpFrame(ClientConnection* client,
                      std::vector<ClientConnection>* clients,
                      const std::vector<std::uint8_t>& frame,
                      ScriptCache* cache, ChannelRegistry* registry,
                      BridgeLogger* logger) {
    wire::PacketHeader header{};
    if (!ValidateHeader(frame, &header, logger)) {
        client->close_requested = true;
        return;
    }
    const wire::TcpRequestKind request = wire::ClassifyTcpRequest(header);
    if (!client->first_frame_logged) {
        const char* role = request == wire::TcpRequestKind::ChannelNotice
                               ? "channel"
                               : (request == wire::TcpRequestKind::Unknown ? "unknown"
                                                                          : "game");
        logger->Write(BridgeLogger::Info,
                      std::string("TCP first_frame peer=") + client->peer +
                          " category=" +
                          std::to_string(static_cast<unsigned>(header.classification)) +
                          " id=" + std::to_string(static_cast<unsigned>(header.msg_no)) +
                          " size=" + std::to_string(frame.size()) + " role=" + role +
                          " listen_port=" + std::to_string(client->listen_port));
        client->first_frame_logged = true;
    }
    if (request == wire::TcpRequestKind::ChannelNotice) {
        if (frame.size() != sizeof(wire::CsNoticeChannelServer)) {
            client->close_requested = true;
            return;
        }
        wire::CsNoticeChannelServer packet{};
        std::memcpy(&packet, frame.data(), sizeof(packet));
        if (!registry->RegisterChannelServer(packet)) {
            logger->Write(BridgeLogger::Warn, "rejected Channel server registration");
            client->close_requested = true;
            return;
        }
        for (std::vector<ClientConnection>::iterator it = clients->begin();
             it != clients->end(); ++it) {
            if (&*it != client && it->channel_registered &&
                it->channel_id == packet.id) {
                it->channel_registered = false;
            }
        }
        client->channel_registered = true;
        client->channel_id = packet.id;
        client->last_notice = std::chrono::system_clock::now();
        logger->Write(BridgeLogger::Info,
                      std::string("Channel registered id=") +
                          std::to_string(packet.id) + " peer=" + client->peer);
        return;
    }
    if (request == wire::TcpRequestKind::CheckScriptVersion) {
        if (frame.size() != sizeof(wire::CsCheckScriptVersion)) {
            client->close_requested = true;
            return;
        }
        wire::CsCheckScriptVersion packet{};
        std::memcpy(&packet, frame.data(), sizeof(packet));
        QueueFrame(client, wire::MakeVersionResponse(
                               cache->Version(),
                               FixedString(packet.channel_script_version,
                                           sizeof(packet.channel_script_version))));
        return;
    }
    if (request == wire::TcpRequestKind::GetScript) {
        if (frame.size() != sizeof(wire::PacketHeader)) {
            client->close_requested = true;
            return;
        }
        QueueFrame(client, wire::MakeScriptResponse(cache->Script()));
        return;
    }
    if (request == wire::TcpRequestKind::GetGcInfo) {
        if (frame.size() != sizeof(wire::PacketHeader)) {
            client->close_requested = true;
            return;
        }
        QueueFrame(client, wire::MakeGcInfoResponse(cache->GcGroups()));
        return;
    }
    // Original GetMessageHandler returns null for unknown ids and leaves the
    // TCPUser connected. Old Game builds legitimately send such frames.
}

void ReceiveClient(ClientConnection* client,
                   std::vector<ClientConnection>* clients,
                   ScriptCache* cache, ChannelRegistry* registry,
                   BridgeLogger* logger) {
    std::array<std::uint8_t, 65536> buffer{};
    while (!client->close_requested) {
        const ssize_t received = ::recv(client->fd, buffer.data(), buffer.size(), 0);
        if (received == 0) {
            client->close_requested = true;
            return;
        }
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
            client->close_requested = true;
            return;
        }
        std::vector<std::vector<std::uint8_t>> frames;
        std::string error;
        if (!client->decoder.Push(buffer.data(), static_cast<std::size_t>(received),
                                  &frames, &error)) {
            logger->Write(BridgeLogger::Warn, error + " peer=" + client->peer);
            client->close_requested = true;
            return;
        }
        for (std::vector<std::vector<std::uint8_t>>::const_iterator frame = frames.begin();
             frame != frames.end() && !client->close_requested; ++frame) {
            DispatchTcpFrame(client, clients, *frame, cache, registry, logger);
        }
    }
}

void FlushClient(ClientConnection* client) {
    while (!client->close_requested && client->output_offset < client->output.size()) {
        const std::uint8_t* data = client->output.data() + client->output_offset;
        const std::size_t remaining = client->output.size() - client->output_offset;
        const ssize_t sent = ::send(client->fd, data, remaining, MSG_NOSIGNAL);
        if (sent > 0) {
            client->output_offset += static_cast<std::size_t>(sent);
            continue;
        }
        if (sent < 0 && errno == EINTR) continue;
        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
        client->close_requested = true;
    }
    if (client->output_offset == client->output.size()) {
        client->output.clear();
        client->output_offset = 0;
    }
}

std::string PeerName(const sockaddr_in& address) {
    char ip[INET_ADDRSTRLEN]{};
    if (!inet_ntop(AF_INET, &address.sin_addr, ip, sizeof(ip))) {
        std::strcpy(ip, "unknown");
    }
    return std::string(ip) + ":" + std::to_string(ntohs(address.sin_port));
}

void AcceptClients(const int listen_fd, const std::size_t max_clients,
                   const std::uint16_t listen_port,
                   std::vector<ClientConnection>* clients,
                   BridgeLogger* logger) {
    while (true) {
        sockaddr_in peer_address;
        socklen_t peer_size = sizeof(peer_address);
        const int fd = ::accept(listen_fd, reinterpret_cast<sockaddr*>(&peer_address),
                                &peer_size);
        if (fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("accept failed: ") +
                                     std::strerror(errno));
        }
        if (clients->size() >= max_clients) {
            ::close(fd);
            logger->Write(BridgeLogger::Warn, "rejected TCP client: max_client reached");
            continue;
        }
        try {
            SetNonBlocking(fd);
            const int enabled = 1;
            ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled));
        } catch (...) {
            ::close(fd);
            throw;
        }
        ClientConnection client;
        client.fd = fd;
        client.peer = PeerName(peer_address);
        client.listen_port = listen_port;
        clients->push_back(client);
        logger->Write(BridgeLogger::Info, "TCP accepted peer=" + client.peer);
    }
}

void ReceiveUdp(const int udp_fd, ChannelRegistry* registry, BridgeLogger* logger) {
    std::array<std::uint8_t, 65536> data{};
    while (true) {
        sockaddr_in peer;
        socklen_t peer_size = sizeof(peer);
        const ssize_t received =
            ::recvfrom(udp_fd, data.data(), data.size(), 0,
                       reinterpret_cast<sockaddr*>(&peer), &peer_size);
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("UDP recvfrom failed: ") +
                                     std::strerror(errno));
        }
        if (static_cast<std::size_t>(received) != sizeof(wire::CsUpdateChannelInfo)) {
            logger->Write(BridgeLogger::Warn, "rejected UDP bridge packet length");
            continue;
        }
        wire::CsUpdateChannelInfo packet{};
        std::memcpy(&packet, data.data(), sizeof(packet));
        if (!wire::IsChannelUpdatePacket(packet.header,
                                         static_cast<std::size_t>(received))) {
            logger->Write(BridgeLogger::Warn, "rejected UDP bridge packet header");
            continue;
        }
        if (!registry->UpdateGameServer(packet)) {
            logger->Write(BridgeLogger::Warn,
                          std::string("rejected Game update gc_no=") +
                              std::to_string(packet.gc_no));
        }
    }
}

void BroadcastGameStatus(std::vector<ClientConnection>* clients,
                         ChannelRegistry* registry, BridgeLogger* logger) {
    const std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    const ExpiryCounts expired = registry->ExpireStale(now);
    if (expired.game_servers != 0 || expired.channel_servers != 0) {
        logger->Write(BridgeLogger::Info,
                      std::string("expired game=") +
                          std::to_string(expired.game_servers) + " channel=" +
                          std::to_string(expired.channel_servers));
    }
    const std::vector<GameServerStatus> games = registry->GameServers();
    std::size_t forwarded = 0;
    std::size_t channel_sessions = 0;
    for (std::vector<ClientConnection>::iterator client = clients->begin();
         client != clients->end(); ++client) {
        if (!client->channel_registered) continue;
        if (now - client->last_notice > std::chrono::seconds(200)) {
            client->channel_registered = false;
            continue;
        }
        ++channel_sessions;
        for (std::vector<GameServerStatus>::const_iterator game = games.begin();
             game != games.end(); ++game) {
            if (!QueueFrame(&*client, wire::MakeChannelUpdate(
                                         game->gc_no, 0, game->channel_no,
                                         game->max_users, game->current_users,
                                         game->ip, game->port))) {
                logger->Write(BridgeLogger::Warn,
                              "closed Channel connection after output backpressure");
                break;
            }
            ++forwarded;
        }
    }
    if (forwarded != 0) {
        logger->Write(BridgeLogger::Info,
                      std::string("forwarded game_updates=") +
                          std::to_string(forwarded) + " channel_sessions=" +
                          std::to_string(channel_sessions));
    }
}

void CloseClients(std::vector<ClientConnection>* clients) {
    for (std::vector<ClientConnection>::iterator it = clients->begin();
         it != clients->end(); ++it) {
        if (it->fd >= 0) ::close(it->fd);
        it->fd = -1;
    }
    clients->clear();
}

int NetworkLoop(const BridgeConfig& config, ScriptCache* cache,
                ChannelRegistry* registry, BridgeLogger* logger) {
    FileDescriptor tcp(CreateBoundSocket(SOCK_STREAM, config.tcp_port));
    FileDescriptor udp(CreateBoundSocket(SOCK_DGRAM, config.udp_port));
    logger->Write(BridgeLogger::Info,
                  std::string("Channel Bridge Start Ver1.0 TCP=") +
                      std::to_string(config.tcp_port) + " UDP=" +
                      std::to_string(config.udp_port) + " declared_ip=" +
                      config.this_ip);

    std::vector<ClientConnection> clients;
    std::chrono::steady_clock::time_point next_broadcast =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    try {
        while (!g_stop_requested) {
            const std::size_t polled_clients = clients.size();
            std::vector<pollfd> descriptors;
            descriptors.reserve(polled_clients + 2);
            pollfd tcp_poll = {tcp.Get(), POLLIN, 0};
            pollfd udp_poll = {udp.Get(), POLLIN, 0};
            descriptors.push_back(tcp_poll);
            descriptors.push_back(udp_poll);
            for (std::size_t i = 0; i < polled_clients; ++i) {
                short events = POLLIN;
                if (clients[i].output_offset < clients[i].output.size()) {
                    events = static_cast<short>(events | POLLOUT);
                }
                pollfd client_poll = {clients[i].fd, events, 0};
                descriptors.push_back(client_poll);
            }
            const int result = ::poll(descriptors.data(), descriptors.size(), 500);
            if (result < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error(std::string("poll failed: ") +
                                         std::strerror(errno));
            }
            if (descriptors[0].revents & POLLIN) {
                AcceptClients(tcp.Get(), static_cast<std::size_t>(config.max_client),
                              config.tcp_port, &clients, logger);
            }
            if (descriptors[1].revents & POLLIN) {
                ReceiveUdp(udp.Get(), registry, logger);
            }
            for (std::size_t i = 0; i < polled_clients; ++i) {
                const short revents = descriptors[i + 2].revents;
                ClientConnection* client = &clients[i];
                if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    client->close_requested = true;
                }
                if (!client->close_requested && (revents & POLLIN)) {
                    ReceiveClient(client, &clients, cache, registry, logger);
                }
                if (!client->close_requested && (revents & POLLOUT)) {
                    FlushClient(client);
                }
            }
            if (std::chrono::steady_clock::now() >= next_broadcast) {
                BroadcastGameStatus(&clients, registry, logger);
                next_broadcast = std::chrono::steady_clock::now() +
                                 std::chrono::seconds(3);
            }
            for (std::vector<ClientConnection>::iterator it = clients.begin();
                 it != clients.end();) {
                if (it->close_requested) {
                    logger->Write(BridgeLogger::Info, "TCP closed peer=" + it->peer);
                    ::close(it->fd);
                    it = clients.erase(it);
                } else {
                    ++it;
                }
            }
        }
    } catch (...) {
        CloseClients(&clients);
        throw;
    }
    CloseClients(&clients);
    return 0;
}

int RunBridgeService(const BridgeConfig& config, const bool write_pid) {
#ifndef DFBRIDGE_WITH_MYSQL
    (void)config;
    (void)write_pid;
    std::cerr << "df_bridge_r start/run requires a DFBRIDGE_WITH_MYSQL build\n";
    return 1;
#else
    EnsureDirectory("./pid");
    PidFileGuard pid_guard(PidPath(config.instance_name), write_pid);
    BridgeLogger logger;
    logger.Open(config.instance_name);
    InstallRuntimeSignals();
    try {
        MysqlConfig mysql(config.db_host, config.db_port, config.db_name,
                          config.db_user, config.db_password);
        std::unique_ptr<IDChannelRepository> repository =
            MakeMysqlRepository(std::move(mysql));
        ScriptCache cache;
        ChannelRegistry registry;
        ScriptThread script(
            *repository, cache, "./cfg/channel_info.etc", std::chrono::seconds(4),
            [&](const std::string& version, const std::vector<GcGroup>& groups) {
                registry.SetGcGroups(groups);
                logger.Write(BridgeLogger::Info,
                             std::string("channel script reloaded version=") +
                                 version + " gc_groups=" +
                                 std::to_string(groups.size()));
            },
            [&](const std::string& message) {
                logger.Write(BridgeLogger::Error, message);
            });
        script.ReloadOnce();
        script.Start();
        const int result = NetworkLoop(config, &cache, &registry, &logger);
        script.Stop();
        logger.Write(BridgeLogger::Info, "Channel Bridge stopped");
        return result;
    } catch (const std::exception& error) {
        logger.Write(BridgeLogger::Critical,
                     std::string("Channel Bridge fatal: ") + error.what());
        return 1;
    } catch (...) {
        logger.Write(BridgeLogger::Critical, "Channel Bridge fatal: unknown error");
        return 1;
    }
#endif
}

int StopService(const std::string& instance_name) {
    const std::string path = PidPath(instance_name);
    const long pid = ReadPid(path);
    if (pid <= 0) {
        std::cerr << "df_bridge_r: no valid pid in " << path << '\n';
        return 1;
    }
    if (!ProcessExists(pid)) {
        std::remove(path.c_str());
        return 0;
    }
    if (!IsExpectedBridgeProcess(pid)) {
        std::cerr << "df_bridge_r: refusing to signal pid " << pid
                  << " because /proc/<pid>/exe is not df_bridge_r\n";
        return 1;
    }
    if (::kill(static_cast<pid_t>(pid), SIGTERM) != 0) {
        const int failure = errno;
        if (failure == ESRCH) std::remove(path.c_str());
        std::cerr << "df_bridge_r: kill(" << pid << ") failed: "
                  << std::strerror(failure) << '\n';
        return failure == ESRCH ? 0 : 1;
    }
    for (int i = 0; i < 100; ++i) {
        if (!ProcessExists(pid)) {
            std::remove(path.c_str());
            return 0;
        }
        ::usleep(100000);
    }
    std::cerr << "df_bridge_r: timed out waiting for pid " << pid << '\n';
    return 1;
}

}  // namespace

int RunServiceCommand(const std::string& instance_name,
                      const std::string& command) {
    if (!IsValidInstanceName(instance_name)) {
        std::cerr << "df_bridge_r: invalid instance name\n";
        return 2;
    }
    if (command == "stop") return StopService(instance_name);
    if (command == "pause" || command == "mess" || command == "install" ||
        command == "remove") {
        // These legacy verbs were no-op hooks in the recovered LinuxService.
        return 0;
    }
    BridgeConfig config;
    try {
        config = BridgeConfigParser().ParseFile(ConfigPath(instance_name), instance_name);
    } catch (const std::exception& error) {
        std::cerr << "df_bridge_r: " << error.what() << '\n';
        return 1;
    }
    if (command == "test") {
        std::cout << "bridge_config=ok instance=" << config.instance_name
                  << " tcp=" << config.tcp_port << " udp=" << config.udp_port
                  << " db=" << config.db_host << '/' << config.db_name << '\n';
        return 0;
    }
#ifndef DFBRIDGE_WITH_MYSQL
    std::cerr << "df_bridge_r start/run requires a DFBRIDGE_WITH_MYSQL build\n";
    return 1;
#endif
    const std::string pid_path = PidPath(instance_name);
    const long existing_pid = ReadPid(pid_path);
    if (ProcessExists(existing_pid)) {
        if (IsExpectedBridgeProcess(existing_pid)) {
            std::cerr << "df_bridge_r: process is already running pid=" << existing_pid
                      << '\n';
            return 1;
        }
        std::remove(pid_path.c_str());
    }
    if (existing_pid > 0) std::remove(pid_path.c_str());
    if (command == "run") return RunBridgeService(config, true);
    if (command != "start") return 2;

    try {
        EnsureDirectory("./pid");
        const pid_t child = ::fork();
        if (child < 0) {
            throw std::runtime_error(std::string("fork failed: ") +
                                     std::strerror(errno));
        }
        if (child > 0) return 0;
        if (::setsid() < 0) {
            throw std::runtime_error(std::string("setsid failed: ") +
                                     std::strerror(errno));
        }
        if (::chdir("./") != 0) {
            throw std::runtime_error(std::string("chdir failed: ") +
                                     std::strerror(errno));
        }
        ::umask(0);
        return RunBridgeService(config, true);
    } catch (const std::exception& error) {
        std::cerr << "df_bridge_r: " << error.what() << '\n';
        return 1;
    }
}

#endif

}  // namespace dfbridge
