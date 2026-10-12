#pragma once

#include "channel/protocol.hpp"

#include <map>
#include <mutex>
#include <string>
#include <ctime>
#include <vector>

namespace dnf { namespace channel {

struct ChannelEndpoint {
    int gc_no;
    int group_hint;
    int channel_no;
    int max_users;
    int current_users;
    std::string ip;
    int port;
    std::time_t last_update;
};

struct ChannelGroupSnapshot {
    std::string name;
    std::vector<ChannelEndpoint> channels;
};

class ChannelRegistry {
public:
    explicit ChannelRegistry(bool allow_pending_before_gc = false)
        : allow_pending_before_gc_(allow_pending_before_gc) {}

    bool ApplyGcInfoReply(const Bytes& frame, std::string* error = nullptr);
    bool ApplyChannelUpdate(const CsUpdateChannelInfo& packet, std::string* error = nullptr);
    bool ApplyChannelUpdateAt(const CsUpdateChannelInfo& packet, std::time_t now,
                              std::string* error = nullptr);
    std::size_t ExpireStale(std::time_t now, unsigned int maximum_age_seconds = 40);
    bool GcReady() const;
    std::vector<ChannelGroupSnapshot> Snapshot() const;
    static Bytes SerializeSnapshot(const std::vector<ChannelGroupSnapshot>& groups);
    Bytes SerializeAskChannelBody() const;

private:
    static const std::size_t kMaximumEndpoints = 4096;
    struct Boundary {
        std::string name;
        int upper_gc;
    };
    bool ResolveGroup(int gc_no, int* output) const;
    void RebuildGroups();

    mutable std::mutex mutex_;
    bool allow_pending_before_gc_;
    std::vector<Boundary> boundaries_;
    std::map<int, std::string> group_names_;
    std::map<int, ChannelEndpoint> endpoints_;
    std::map<int, std::map<int, ChannelEndpoint>> groups_;
};

} }  // namespace dnf::channel
