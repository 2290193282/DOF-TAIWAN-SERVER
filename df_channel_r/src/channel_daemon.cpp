#include "channel/channel_daemon.hpp"

#include "channel/channel_client.hpp"
#include "channel/channel_registry.hpp"
#include "channel/channel_service.hpp"
#include "channel/daemon_config.hpp"
#include "channel/legacy_codec.hpp"
#include "channel/script_store.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace dnf { namespace channel {
namespace {

const std::size_t kMaximumStreamFrame = 655360;
const std::size_t kMaximumQueuedOutput = 4U << 20;
const unsigned int kGameStateLifetimeSeconds = 40;
const unsigned int kSessionIdleSeconds = 600;
const int kBridgePeriodSeconds = 20;

enum class LogLevel { Info, Warn, Error, Critical };

const char* LogLevelName(LogLevel level) {
    switch (level) {
    case LogLevel::Info: return "Info";
    case LogLevel::Warn: return "Warn";
    case LogLevel::Error: return "Error";
    case LogLevel::Critical: return "Cri";
    }
    return "Info";
}

std::string Timestamp(std::time_t now) {
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[32] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

std::string DateStamp(std::time_t now) {
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[16] = {};
    std::strftime(buffer, sizeof(buffer), "%Y%m%d", &local);
    return buffer;
}

#ifndef _WIN32
bool EnsureDirectory(const std::string& path, mode_t mode, std::string* error) {
    if (::mkdir(path.c_str(), mode) == 0 || errno == EEXIST) return true;
    if (error) *error = "cannot create " + path + ": " + std::strerror(errno);
    return false;
}
#endif

class LegacyLogger {
public:
    bool Open(const std::string& service_name, std::string* error) {
#ifdef _WIN32
        (void)service_name;
        if (error) *error = "daemon logging is unavailable on Windows";
        return false;
#else
        if (!EnsureDirectory("./log", 0755, error) ||
            !EnsureDirectory("./log/old_log", 0755, error)) return false;
        info_.open(("./log/Info_" + service_name + ".log").c_str(), std::ios::app);
        warn_.open(("./log/Warn_" + service_name + ".log").c_str(), std::ios::app);
        error_.open(("./log/Error_" + service_name + ".log").c_str(), std::ios::app);
        critical_.open(("./log/Cri_" + service_name + ".log").c_str(), std::ios::app);
        if (!info_ || !warn_ || !error_ || !critical_) {
            if (error) *error = "cannot open one or more legacy log files";
            return false;
        }
        return true;
#endif
    }

    void Write(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostream* stream = &info_;
        if (level == LogLevel::Warn) stream = &warn_;
        else if (level == LogLevel::Error) stream = &error_;
        else if (level == LogLevel::Critical) stream = &critical_;
        if (*stream) {
            *stream << Timestamp(std::time(nullptr)) << " [" << LogLevelName(level) << "] "
                    << message << '\n';
            stream->flush();
        }
    }

private:
    std::mutex mutex_;
    std::ofstream info_;
    std::ofstream warn_;
    std::ofstream error_;
    std::ofstream critical_;
};

#ifndef _WIN32

volatile sig_atomic_t g_stop_requested = 0;
volatile sig_atomic_t g_pause_requested = 0;

void StopSignalHandler(int) { g_stop_requested = 1; }
void PauseSignalHandler(int) { g_pause_requested = 1; }

bool InstallSignalHandlers(std::string* error) {
    struct sigaction stop_action;
    std::memset(&stop_action, 0, sizeof(stop_action));
    stop_action.sa_handler = StopSignalHandler;
    sigemptyset(&stop_action.sa_mask);
    if (sigaction(SIGTERM, &stop_action, nullptr) != 0 ||
        sigaction(SIGINT, &stop_action, nullptr) != 0) {
        if (error) *error = std::string("cannot install stop signal handler: ") + std::strerror(errno);
        return false;
    }
    struct sigaction pause_action;
    std::memset(&pause_action, 0, sizeof(pause_action));
    pause_action.sa_handler = PauseSignalHandler;
    sigemptyset(&pause_action.sa_mask);
    if (sigaction(SIGUSR1, &pause_action, nullptr) != 0) {
        if (error) *error = std::string("cannot install pause signal handler: ") + std::strerror(errno);
        return false;
    }
    struct sigaction pipe_action;
    std::memset(&pipe_action, 0, sizeof(pipe_action));
    pipe_action.sa_handler = SIG_IGN;
    sigemptyset(&pipe_action.sa_mask);
    if (sigaction(SIGPIPE, &pipe_action, nullptr) != 0) {
        if (error) *error = std::string("cannot ignore SIGPIPE: ") + std::strerror(errno);
        return false;
    }
    struct rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;
    core_limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &core_limit);
    return true;
}

bool SetNonBlocking(int fd, std::string* error) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
        if (error) *error = std::strerror(errno);
        return false;
    }
    return true;
}

void CloseSocket(int* fd) {
    if (fd && *fd >= 0) {
        ::close(*fd);
        *fd = -1;
    }
}

bool WaitForFd(int fd, short events, int timeout_ms, const std::atomic<bool>& stop) {
    int remaining = timeout_ms;
    while (!stop.load() && remaining > 0) {
        struct pollfd item;
        item.fd = fd;
        item.events = events;
        item.revents = 0;
        const int slice = std::min(remaining, 250);
        const int result = ::poll(&item, 1, slice);
        if (result > 0) return (item.revents & events) != 0;
        if (result < 0 && errno != EINTR) return false;
        remaining -= slice;
    }
    return false;
}

int ConnectBridgeSocket(const std::string& host, int port, const std::atomic<bool>& stop,
                        std::string* error) {
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* addresses = nullptr;
    const std::string service = std::to_string(port);
    const int lookup = getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses);
    if (lookup != 0) {
        if (error) *error = gai_strerror(lookup);
        return -1;
    }
    int connected = -1;
    for (struct addrinfo* current = addresses; current && !stop.load(); current = current->ai_next) {
        int candidate = ::socket(current->ai_family, current->ai_socktype, current->ai_protocol);
        if (candidate < 0) continue;
        std::string ignored;
        if (!SetNonBlocking(candidate, &ignored)) {
            ::close(candidate);
            continue;
        }
        const int result = ::connect(candidate, current->ai_addr, current->ai_addrlen);
        if (result == 0 || (result < 0 && errno == EINPROGRESS &&
                            WaitForFd(candidate, POLLOUT, 3000, stop))) {
            int socket_error = 0;
            socklen_t length = sizeof(socket_error);
            if (getsockopt(candidate, SOL_SOCKET, SO_ERROR, &socket_error, &length) == 0 &&
                socket_error == 0) {
                connected = candidate;
                break;
            }
        }
        ::close(candidate);
    }
    freeaddrinfo(addresses);
    if (connected < 0 && error && error->empty()) *error = std::strerror(errno);
    return connected;
}

bool SendAll(int fd, const Bytes& bytes, const std::atomic<bool>& stop, std::string* error) {
    std::size_t offset = 0;
    const std::chrono::steady_clock::time_point deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (offset < bytes.size() && !stop.load()) {
        const ssize_t sent = ::send(fd, bytes.data() + offset, bytes.size() - offset, MSG_NOSIGNAL);
        if (sent > 0) {
            offset += static_cast<std::size_t>(sent);
            continue;
        }
        if (sent < 0 && errno == EINTR) continue;
        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (std::chrono::steady_clock::now() >= deadline) {
                if (error) *error = "Bridge send timeout";
                return false;
            }
            WaitForFd(fd, POLLOUT, 250, stop);
            continue;
        }
        if (error) *error = sent == 0 ? "Bridge send returned zero" : std::strerror(errno);
        return false;
    }
    return offset == bytes.size();
}

void InterruptibleSleep(const std::atomic<bool>& stop, int milliseconds) {
    int remaining = milliseconds;
    while (!stop.load() && remaining > 0) {
        const int slice = std::min(remaining, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(slice));
        remaining -= slice;
    }
}

class BridgeRunner {
public:
    BridgeRunner(const DaemonConfig& config, BridgeProtocolClient& protocol, LegacyLogger& logger,
                 std::atomic<bool>& stop)
        : config_(config), protocol_(protocol), logger_(logger), stop_(stop) {}

    void operator()() {
        while (!stop_.load()) {
            std::string error;
            int fd = ConnectBridgeSocket(config_.bridge_ip, config_.bridge_port, stop_, &error);
            if (fd < 0) {
                if (!stop_.load()) logger_.Write(LogLevel::Warn, "Bridge connect failed: " + error);
                InterruptibleSleep(stop_, 1000);
                continue;
            }
            protocol_.Reset();
            logger_.Write(LogLevel::Info, "Bridge connected to " + config_.bridge_ip + ":" +
                                                  std::to_string(config_.bridge_port));
            std::vector<Bytes> frames = protocol_.BuildInitialFrames();
            bool connected = frames.size() == 3 && SendAll(fd, frames[0], stop_, &error);
            bool received_gc = false;
            const std::chrono::steady_clock::time_point gc_deadline =
                std::chrono::steady_clock::now() + std::chrono::seconds(5);
            std::time_t last_periodic = 0;
            Bytes input;
            while (connected && !stop_.load()) {
                if (!received_gc && std::chrono::steady_clock::now() >= gc_deadline) {
                    error = "Bridge did not provide a valid GC table within 5 seconds";
                    connected = false;
                    break;
                }
                const std::time_t now = std::time(nullptr);
                if (last_periodic == 0 || now - last_periodic >= kBridgePeriodSeconds) {
                    frames = protocol_.BuildInitialFrames();
                    // Original CheckThread sends 13 once after connect, then refreshes
                    // script version (5) and Channel identity (4) every 20 seconds.
                    if (frames.size() != 3 || !SendAll(fd, frames[1], stop_, &error) ||
                        !SendAll(fd, frames[2], stop_, &error)) {
                        connected = false;
                        break;
                    }
                    last_periodic = now;
                }

                struct pollfd item;
                item.fd = fd;
                item.events = POLLIN;
                item.revents = 0;
                const int ready = ::poll(&item, 1, 250);
                if (ready < 0) {
                    if (errno == EINTR) continue;
                    error = std::strerror(errno);
                    connected = false;
                    break;
                }
                if (ready == 0) continue;
                if (item.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    error = "Bridge socket closed";
                    connected = false;
                    break;
                }
                if (!(item.revents & POLLIN)) continue;
                std::uint8_t buffer[65536];
                for (;;) {
                    const ssize_t received = ::recv(fd, buffer, sizeof(buffer), 0);
                    if (received > 0) {
                        input.insert(input.end(), buffer, buffer + received);
                    } else if (received == 0) {
                        error = "Bridge closed connection";
                        connected = false;
                        break;
                    } else if (errno == EINTR) {
                        continue;
                    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    } else {
                        error = std::strerror(errno);
                        connected = false;
                        break;
                    }
                }
                while (connected && input.size() >= sizeof(PacketHeader)) {
                    PacketHeader header{};
                    std::memcpy(&header, input.data(), sizeof(header));
                    if (header.size < sizeof(PacketHeader) || header.size > kMaximumStreamFrame) {
                        error = "Bridge frame size out of range";
                        connected = false;
                        break;
                    }
                    if (input.size() < header.size) break;
                    Bytes frame(input.begin(), input.begin() + header.size);
                    input.erase(input.begin(), input.begin() + header.size);
                    BridgeHandleResult result = protocol_.HandleFrame(frame);
                    if (!result.accepted) {
                        error = result.error;
                        connected = false;
                        break;
                    }
                    if (result.gc_ready) received_gc = true;
                    if (!result.error.empty()) logger_.Write(LogLevel::Warn, result.error);
                    for (std::size_t index = 0; index < result.outbound.size(); ++index) {
                        if (!SendAll(fd, result.outbound[index], stop_, &error)) {
                            connected = false;
                            break;
                        }
                    }
                }
                if (input.size() > kMaximumStreamFrame) {
                    error = "Bridge input buffer exceeded legacy limit";
                    connected = false;
                }
            }
            ::close(fd);
            protocol_.Reset();
            if (!stop_.load()) logger_.Write(LogLevel::Warn, "Bridge disconnected: " + error);
            InterruptibleSleep(stop_, 1000);
        }
    }

private:
    const DaemonConfig& config_;
    BridgeProtocolClient& protocol_;
    LegacyLogger& logger_;
    std::atomic<bool>& stop_;
};

int CreateBoundSocket(int type, int port, bool listening, std::string* error) {
    int fd = ::socket(AF_INET, type, 0);
    if (fd < 0) {
        if (error) *error = std::strerror(errno);
        return -1;
    }
    int enabled = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0 ||
        (listening && ::listen(fd, 1024) != 0) || !SetNonBlocking(fd, error)) {
        if (error && error->empty()) *error = std::strerror(errno);
        ::close(fd);
        return -1;
    }
    return fd;
}

struct GameSession {
    GameSession(int socket_fd, ChannelService& service)
        : fd(socket_fd), protocol(service), output_offset(0), last_activity(std::time(nullptr)) {}
    ~GameSession() { if (fd >= 0) ::close(fd); }

    int fd;
    ChannelSession protocol;
    Bytes input;
    Bytes output;
    std::size_t output_offset;
    std::time_t last_activity;
};

class ChannelRuntime {
public:
    ChannelRuntime(const std::string& service_name, const DaemonConfig& config)
        : service_name_(service_name), config_(config),
          scripts_("channel_info/channel_info.etc", "channel_info/version"),
          registry_(config.allow_pending_before_gc), codec_(LegacyCodec::TimestampKey(std::time(nullptr))),
          service_(scripts_, registry_, codec_),
          bridge_(scripts_, registry_, BridgeIdentity{config.id, config.this_ip, config.this_udp_port}),
          stop_(false), tcp_listener_(-1), udp_socket_(-1) {}

    int Run() {
        std::string error;
        if (!logger_.Open(service_name_, &error)) {
            std::cerr << error << '\n';
            return 1;
        }
        if (!scripts_.Reload(&error)) {
            logger_.Write(LogLevel::Critical, "Channel script load failed: " + error);
            return 1;
        }
        const ScriptSnapshot snapshot = scripts_.Snapshot();
        logger_.Write(LogLevel::Info, "Loaded channel script version=" + snapshot.version +
                                              " dungeons=" + std::to_string(snapshot.stats.dungeon_groups) +
                                              " mappings=" + std::to_string(snapshot.stats.dungeon_mappings) +
                                              " channels=" + std::to_string(snapshot.stats.channel_rows));
        tcp_listener_ = CreateBoundSocket(SOCK_STREAM, config_.this_tcp_port, true, &error);
        if (tcp_listener_ < 0) {
            logger_.Write(LogLevel::Critical, "TCP listen failed: " + error);
            return 1;
        }
        udp_socket_ = CreateBoundSocket(SOCK_DGRAM, config_.this_udp_port, false, &error);
        if (udp_socket_ < 0) {
            logger_.Write(LogLevel::Critical, "UDP bind failed: " + error);
            CloseSocket(&tcp_listener_);
            return 1;
        }
        logger_.Write(LogLevel::Info, "Channel Server Start Ver1.0; TCP=" +
                                              std::to_string(config_.this_tcp_port) + " UDP=" +
                                              std::to_string(config_.this_udp_port));
        bridge_thread_ = std::thread(BridgeRunner(config_, bridge_, logger_, stop_));

        std::string active_date = DateStamp(std::time(nullptr));
        std::time_t last_maintenance = 0;
        while (!stop_.load() && !g_stop_requested) {
            if (g_pause_requested) {
                g_pause_requested = 0;
                logger_.Write(LogLevel::Info, "pause signal received (legacy no-op)");
            }
            PollOnce();
            const std::time_t now = std::time(nullptr);
            if (now != last_maintenance) {
                last_maintenance = now;
                const std::size_t expired = registry_.ExpireStale(now, kGameStateLifetimeSeconds);
                if (expired) logger_.Write(LogLevel::Info, "Expired game channel states=" + std::to_string(expired));
                ExpireIdleSessions(now);
                const std::string date = DateStamp(now);
                if (date != active_date) {
                    codec_.Rotate(now);
                    active_date = date;
                    logger_.Write(LogLevel::Info, "Rotated legacy AES key for date=" + date);
                }
            }
        }
        stop_.store(true);
        CloseSocket(&tcp_listener_);
        CloseSocket(&udp_socket_);
        sessions_.clear();
        if (bridge_thread_.joinable()) bridge_thread_.join();
        logger_.Write(LogLevel::Info, "Channel daemon stopped");
        return 0;
    }

private:
    void PollOnce() {
        std::vector<struct pollfd> descriptors;
        descriptors.reserve(sessions_.size() + 2);
        struct pollfd tcp_item = {tcp_listener_, POLLIN, 0};
        struct pollfd udp_item = {udp_socket_, POLLIN, 0};
        descriptors.push_back(tcp_item);
        descriptors.push_back(udp_item);
        for (std::size_t index = 0; index < sessions_.size(); ++index) {
            short events = POLLIN;
            if (sessions_[index]->output_offset < sessions_[index]->output.size()) events |= POLLOUT;
            struct pollfd item = {sessions_[index]->fd, events, 0};
            descriptors.push_back(item);
        }
        const int result = ::poll(descriptors.data(), descriptors.size(), 500);
        if (result < 0) {
            if (errno != EINTR) logger_.Write(LogLevel::Error, std::string("poll failed: ") + std::strerror(errno));
            return;
        }
        if (result == 0) return;
        const std::size_t polled_sessions = sessions_.size();
        for (std::size_t reverse = polled_sessions; reverse > 0; --reverse) {
            const std::size_t index = reverse - 1;
            const short revents = descriptors[index + 2].revents;
            bool keep = (revents & (POLLERR | POLLHUP | POLLNVAL)) == 0;
            if (keep && (revents & POLLIN)) keep = ReadSession(*sessions_[index]);
            if (keep && (revents & POLLOUT)) keep = WriteSession(*sessions_[index]);
            if (!keep) sessions_.erase(sessions_.begin() + index);
        }
        if (descriptors[1].revents & POLLIN) ReceiveUdpUpdates();
        if (descriptors[0].revents & POLLIN) AcceptSessions();
    }

    void AcceptSessions() {
        for (;;) {
            struct sockaddr_in peer;
            socklen_t length = sizeof(peer);
            const int fd = ::accept(tcp_listener_, reinterpret_cast<struct sockaddr*>(&peer), &length);
            if (fd < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    logger_.Write(LogLevel::Warn, std::string("accept failed: ") + std::strerror(errno));
                return;
            }
            std::string error;
            if (!SetNonBlocking(fd, &error) || sessions_.size() >= static_cast<std::size_t>(config_.max_client)) {
                ::close(fd);
                if (!error.empty()) logger_.Write(LogLevel::Warn, "accepted socket setup failed: " + error);
                continue;
            }
            sessions_.push_back(std::unique_ptr<GameSession>(new GameSession(fd, service_)));
        }
    }

    bool ReadSession(GameSession& session) {
        std::uint8_t buffer[65536];
        for (;;) {
            const ssize_t received = ::recv(session.fd, buffer, sizeof(buffer), 0);
            if (received > 0) {
                session.input.insert(session.input.end(), buffer, buffer + received);
                session.last_activity = std::time(nullptr);
            } else if (received == 0) {
                return false;
            } else if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                return false;
            }
        }
        while (session.input.size() >= sizeof(PacketHeader)) {
            PacketHeader header{};
            std::memcpy(&header, session.input.data(), sizeof(header));
            if (header.size < sizeof(PacketHeader) || header.size > kMaximumStreamFrame) {
                logger_.Write(LogLevel::Warn, "invalid game frame size=" + std::to_string(header.size));
                return false;
            }
            if (session.input.size() < header.size) break;
            Bytes frame(session.input.begin(), session.input.begin() + header.size);
            session.input.erase(session.input.begin(), session.input.begin() + header.size);
            HandleResult handled = session.protocol.HandleGameFrame(frame);
            if (!handled.accepted) {
                logger_.Write(LogLevel::Warn, "game frame rejected: " + handled.error);
                return false;
            }
            if (handled.has_response) {
                const std::size_t pending = session.output.size() - session.output_offset;
                if (pending + handled.response.size() > kMaximumQueuedOutput) {
                    logger_.Write(LogLevel::Warn, "game output queue limit reached");
                    return false;
                }
                if (session.output_offset && pending) {
                    session.output.erase(session.output.begin(), session.output.begin() + session.output_offset);
                    session.output_offset = 0;
                } else if (session.output_offset == session.output.size()) {
                    session.output.clear();
                    session.output_offset = 0;
                }
                session.output.insert(session.output.end(), handled.response.begin(), handled.response.end());
            }
        }
        return session.input.size() <= kMaximumStreamFrame;
    }

    bool WriteSession(GameSession& session) {
        while (session.output_offset < session.output.size()) {
            const ssize_t sent = ::send(session.fd, session.output.data() + session.output_offset,
                                        session.output.size() - session.output_offset, MSG_NOSIGNAL);
            if (sent > 0) {
                session.output_offset += static_cast<std::size_t>(sent);
                session.last_activity = std::time(nullptr);
            } else if (sent < 0 && errno == EINTR) {
                continue;
            } else if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return true;
            } else {
                return false;
            }
        }
        session.output.clear();
        session.output_offset = 0;
        return true;
    }

    void ReceiveUdpUpdates() {
        std::uint8_t buffer[65536];
        for (;;) {
            const ssize_t received = ::recvfrom(udp_socket_, buffer, sizeof(buffer), 0, nullptr, nullptr);
            if (received < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    logger_.Write(LogLevel::Warn, std::string("UDP receive failed: ") + std::strerror(errno));
                return;
            }
            if (received == 0) return;
            // registerProtocolsExtra is empty in the 0828 binary.  The UDP
            // socket is part of the observable bind contract, but datagrams
            // have no registered business handler.  Game message 2 reaches
            // Channel only through Bridge's authenticated TCP forwarding.
        }
    }

    void ExpireIdleSessions(std::time_t now) {
        for (std::size_t reverse = sessions_.size(); reverse > 0; --reverse) {
            const std::size_t index = reverse - 1;
            if (now > sessions_[index]->last_activity &&
                static_cast<unsigned long long>(now - sessions_[index]->last_activity) > kSessionIdleSeconds) {
                sessions_.erase(sessions_.begin() + index);
            }
        }
    }

    std::string service_name_;
    DaemonConfig config_;
    LegacyLogger logger_;
    ScriptStore scripts_;
    ChannelRegistry registry_;
    LegacyCodec codec_;
    ChannelService service_;
    BridgeProtocolClient bridge_;
    std::atomic<bool> stop_;
    int tcp_listener_;
    int udp_socket_;
    std::thread bridge_thread_;
    std::vector<std::unique_ptr<GameSession>> sessions_;
};

bool SafeServiceName(const std::string& name) {
    if (name.empty() || name.size() > 64 || name == "." || name == "..") return false;
    for (std::size_t index = 0; index < name.size(); ++index) {
        const unsigned char c = static_cast<unsigned char>(name[index]);
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

bool ProcessAlive(pid_t pid) {
    return ::kill(pid, 0) == 0 || errno == EPERM;
}

bool ReadPidFromFd(int fd, pid_t* pid, std::string* error) {
    if (!pid || ::lseek(fd, 0, SEEK_SET) < 0) {
        if (error) *error = "cannot seek PID file";
        return false;
    }
    char buffer[64] = {};
    const ssize_t length = ::read(fd, buffer, sizeof(buffer) - 1);
    if (length <= 0) {
        if (error) *error = "cannot read PID file";
        return false;
    }
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(buffer, &end, 10);
    while (end && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) ++end;
    if (errno != 0 || end == buffer || (end && *end != '\0') || value <= 1 || value > 0x7fffffffL) {
        if (error) *error = "PID file contains an invalid value";
        return false;
    }
    *pid = static_cast<pid_t>(value);
    return true;
}

bool IsChannelExecutable(pid_t pid, std::string* error) {
    const std::string link = "/proc/" + std::to_string(static_cast<long long>(pid)) + "/exe";
    char target[PATH_MAX + 1] = {};
    const ssize_t length = ::readlink(link.c_str(), target, PATH_MAX);
    if (length <= 0) {
        if (error) *error = "cannot resolve " + link;
        return false;
    }
    target[length] = '\0';
    const char* base = std::strrchr(target, '/');
    base = base ? base + 1 : target;
    if (std::strcmp(base, "df_channel_r") != 0) {
        if (error) *error = "PID executable basename is not df_channel_r";
        return false;
    }
    return true;
}

bool ValidateActivePidFile(const std::string& path, pid_t* pid, std::string* error) {
    const int fd = ::open(path.c_str(), O_RDWR);
    if (fd < 0) {
        if (error) *error = "cannot open " + path + ": " + std::strerror(errno);
        return false;
    }
    errno = 0;
    if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
        flock(fd, LOCK_UN);
        ::close(fd);
        if (error) *error = "PID file has no active process lock";
        return false;
    }
    if (errno != EWOULDBLOCK && errno != EAGAIN) {
        if (error) *error = "cannot inspect PID lock: " + std::string(std::strerror(errno));
        ::close(fd);
        return false;
    }
    bool valid = ReadPidFromFd(fd, pid, error);
    if (valid && !ProcessAlive(*pid)) {
        if (error) *error = "PID file lock exists but the process is not alive";
        valid = false;
    }
    if (valid) valid = IsChannelExecutable(*pid, error);
    ::close(fd);
    return valid;
}

bool RemoveUnlockedPidFile(const std::string& path, std::string* error) {
    const int fd = ::open(path.c_str(), O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) return true;
        if (error) *error = "cannot open stale PID file: " + std::string(std::strerror(errno));
        return false;
    }
    if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
        if (error) *error = "refusing to remove an actively locked PID file";
        ::close(fd);
        return false;
    }
    const int removed = ::unlink(path.c_str());
    const int remove_error = errno;
    flock(fd, LOCK_UN);
    ::close(fd);
    if (removed != 0 && remove_error != ENOENT) {
        if (error) *error = "cannot remove stale PID file: " + std::string(std::strerror(remove_error));
        return false;
    }
    return true;
}

class PidFile {
public:
    PidFile() : fd_(-1) {}
    ~PidFile() { Release(); }

    bool Acquire(const std::string& path, std::string* error) {
        path_ = path;
        fd_ = ::open(path.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ < 0 || flock(fd_, LOCK_EX | LOCK_NB) != 0 || ftruncate(fd_, 0) != 0) {
            if (error) *error = "cannot acquire " + path + ": " + std::strerror(errno);
            if (fd_ >= 0) ::close(fd_);
            fd_ = -1;
            return false;
        }
        const std::string text = std::to_string(static_cast<long long>(::getpid())) + "\n";
        if (::write(fd_, text.data(), text.size()) != static_cast<ssize_t>(text.size()) || fsync(fd_) != 0) {
            if (error) *error = "cannot write " + path + ": " + std::strerror(errno);
            Release();
            return false;
        }
        return true;
    }

    void Release() {
        if (fd_ >= 0) {
            ::unlink(path_.c_str());
            flock(fd_, LOCK_UN);
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    int fd_;
    std::string path_;
};

bool Preflight(const std::string& service_name, DaemonConfig* config, std::string* error) {
    if (!LoadDaemonConfig(LegacyConfigPath(service_name), config, error)) return false;
    ScriptStore scripts("channel_info/channel_info.etc", "channel_info/version");
    return scripts.Reload(error);
}

int SendControlSignal(const std::string& service_name, int signal_number, bool wait_for_exit) {
    const std::string path = LegacyPidPath(service_name);
    pid_t pid = 0;
    std::string error;
    if (!ValidateActivePidFile(path, &pid, &error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (::kill(pid, signal_number) != 0) {
        std::cerr << "cannot signal PID " << pid << ": " << std::strerror(errno) << '\n';
        return 1;
    }
    if (wait_for_exit) {
        for (int attempt = 0; attempt < 100 && ProcessAlive(pid); ++attempt)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (ProcessAlive(pid)) {
            std::cerr << "timeout waiting for PID " << pid << " to stop\n";
            return 1;
        }
    }
    return 0;
}

int RunService(const std::string& service_name, const DaemonConfig& config, bool daemon_mode) {
    if (daemon_mode) {
        if (!EnsureDirectory("./pid", 0755, nullptr)) {
            std::cerr << "cannot create ./pid\n";
            return 1;
        }
        const pid_t child = ::fork();
        if (child < 0) {
            std::cerr << "fork failed: " << std::strerror(errno) << '\n';
            return 1;
        }
        if (child > 0) return 0;
        if (::setsid() < 0) _exit(1);
        ::chdir("./");
        ::umask(0);
    }

    std::string error;
    if (!InstallSignalHandlers(&error)) {
        std::cerr << error << '\n';
        return 1;
    }
    PidFile pid_file;
    if (daemon_mode && !pid_file.Acquire(LegacyPidPath(service_name), &error)) {
        std::cerr << error << '\n';
        return 1;
    }
    ChannelRuntime runtime(service_name, config);
    return runtime.Run();
}

#endif  // !_WIN32

void PrintUsage(const char* program) {
    std::cerr << "usage: " << program
              << " <config-name> <start|run|test|stop|pause>\n";
}

}  // namespace

int ChannelDaemonMain(int argc, char** argv) {
#ifdef _WIN32
    (void)argc;
    (void)argv;
    std::cerr << "df_channel_r daemon is supported on POSIX systems only\n";
    return 1;
#else
    if (argc < 3) {
        PrintUsage(argc > 0 ? argv[0] : "df_channel_r");
        return 2;
    }
    const std::string service_name = argv[1];
    const std::string command = argv[2];
    if (!SafeServiceName(service_name)) {
        std::cerr << "invalid config name\n";
        return 2;
    }
    if (command == "stop") return SendControlSignal(service_name, SIGTERM, true);
    if (command == "pause") return SendControlSignal(service_name, SIGUSR1, false);
    if (command == "install" || command == "remove") return 0;  // legacy no-op verbs
    if (command != "start" && command != "run" && command != "test") {
        PrintUsage(argv[0]);
        return 2;
    }

    const std::string pid_path = LegacyPidPath(service_name);
    pid_t existing = 0;
    std::string pid_error;
    if (::access(pid_path.c_str(), F_OK) == 0) {
        if (ValidateActivePidFile(pid_path, &existing, &pid_error)) {
            std::cerr << "Process(" << service_name << ") is already operated.\n";
            return 1;
        }
        if (!RemoveUnlockedPidFile(pid_path, &pid_error)) {
            std::cerr << pid_error << '\n';
            return 1;
        }
    }

    DaemonConfig config;
    std::string error;
    if (!Preflight(service_name, &config, &error)) {
        std::cerr << error << '\n';
        return 1;
    }
    return RunService(service_name, config, command == "start");
#endif
}

} }  // namespace dnf::channel
