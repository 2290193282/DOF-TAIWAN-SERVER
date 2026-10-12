#pragma once

#include "dfbridge/channel_model.hpp"
#include "dfbridge/protocol.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace dfbridge {

struct ChannelServerEndpoint {
    std::int32_t id{};
    std::string ip;
    std::int32_t port{};
    std::chrono::system_clock::time_point last_seen{};
};

struct GameServerStatus {
    std::int32_t max_users{};
    std::int32_t current_users{};
    std::string ip;
    std::int32_t port{};
    std::int32_t gc_no{};
    std::int32_t group{};
    std::int32_t channel_no{};
    std::string channel_name;
    std::chrono::system_clock::time_point last_seen{};
};

struct ExpiryCounts {
    std::size_t channel_servers{};
    std::size_t game_servers{};
};

class ScriptCache {
public:
    void Replace(std::string version, std::string script,
                 std::vector<GcGroup> gc_groups);
    std::string Version() const;
    std::string Script() const;
    std::vector<GcGroup> GcGroups() const;

private:
    mutable std::mutex mutex_;
    std::string version_;
    std::string script_;
    std::vector<GcGroup> gc_groups_;
};

class ChannelRegistry {
public:
    static constexpr std::size_t kMaxChannelServers = 256;
    static constexpr std::size_t kMaxGameServersPerGroup = 4096;

    explicit ChannelRegistry(std::vector<GcGroup> gc_groups = {});
    void SetGcGroups(std::vector<GcGroup> groups);
    bool RegisterChannelServer(const wire::CsNoticeChannelServer& packet);
    bool UpdateGameServer(const wire::CsUpdateChannelInfo& packet);

    bool ResolveGcGroupIndex(std::int32_t gc_no, std::size_t* group_index) const;
    std::size_t ChannelServerCount() const;
    std::size_t GameServerCount(std::size_t group_index) const;
    std::vector<ChannelServerEndpoint> ChannelServers() const;
    std::vector<GameServerStatus> GameServers() const;
    ExpiryCounts ExpireStale(
        std::chrono::system_clock::time_point now,
        std::chrono::seconds game_timeout = std::chrono::seconds(20),
        std::chrono::seconds channel_timeout = std::chrono::seconds(200));

private:
    mutable std::mutex mutex_;
    std::vector<GcGroup> gc_groups_;
    std::vector<ChannelServerEndpoint> channel_servers_;
    std::vector<std::map<std::int32_t, GameServerStatus>> game_servers_;
};

struct ProtocolRegistration {
    std::uint8_t packet_id{};
    bool extra_dispatch{};
    const char* handler{};

    ProtocolRegistration(std::uint8_t id, bool extra, const char* name)
        : packet_id(id), extra_dispatch(extra), handler(name) {}
};

const std::vector<ProtocolRegistration>& RestoredProtocolTable();

}  // namespace dfbridge
