#include "channel/channel_registry.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace dnf { namespace channel {
namespace {

template <std::size_t N>
void CopyFixed(char (&destination)[N], const std::string& source) {
    std::memset(destination, 0, N);
    std::memcpy(destination, source.data(), std::min(source.size(), N - 1));
}

}  // namespace

bool ChannelRegistry::ApplyGcInfoReply(const Bytes& frame, std::string* error) {
    if (!ValidateFrame(frame)) {
        if (error) *error = "invalid GC-info frame";
        return false;
    }
    ScGetGcInfo response{};
    if (!ReadPod(frame, &response) ||
        response.header.classification != static_cast<std::uint8_t>(Category::ServerToClient) ||
        response.header.message_id != static_cast<std::uint8_t>(MessageId::GetGcInfoReply) ||
        response.count <= 0 || response.count > 128) {
        if (error) *error = "invalid GC-info count";
        return false;
    }
    const std::size_t expected = sizeof(ScGetGcInfo) + static_cast<std::size_t>(response.count) * sizeof(ServerGcInfoWire);
    if (frame.size() != expected) {
        if (error) *error = "GC-info count/size mismatch";
        return false;
    }
    std::vector<Boundary> boundaries;
    boundaries.reserve(response.count);
    for (int index = 0; index < response.count; ++index) {
        ServerGcInfoWire item{};
        if (!ReadPod(frame, &item, sizeof(ScGetGcInfo) + index * sizeof(ServerGcInfoWire)) || item.gc_no <= 0) {
            if (error) *error = "invalid GC boundary";
            return false;
        }
        Boundary boundary;
        boundary.name = FixedString(item.server_group_name, 20);
        boundary.upper_gc = item.gc_no;
        if (boundary.name.empty()) {
            if (error) *error = "GC boundaries require non-empty names";
            return false;
        }
        boundaries.push_back(boundary);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    boundaries_ = std::move(boundaries);  // preserve Bridge order exactly
    group_names_.clear();
    for (std::size_t index = 0; index < boundaries_.size(); ++index) {
        group_names_[static_cast<int>(index)] = boundaries_[index].name;
    }
    RebuildGroups();
    return true;
}

bool ChannelRegistry::ResolveGroup(int gc_no, int* output) const {
    if (!output) return false;
    // Original 0828 rule: first Bridge upper boundary with 1..999 distance.
    for (std::size_t index = 0; index < boundaries_.size(); ++index) {
        const int distance = boundaries_[index].upper_gc - gc_no;
        if (distance > 0 && distance <= 999) {
            *output = static_cast<int>(index);
            return true;
        }
    }
    return false;
}

void ChannelRegistry::RebuildGroups() {
    groups_.clear();
    for (std::map<int, ChannelEndpoint>::const_iterator item = endpoints_.begin();
         item != endpoints_.end(); ++item) {
        int group = -1;
        if (ResolveGroup(item->second.gc_no, &group)) {
            groups_[group][item->second.gc_no] = item->second;
        }
    }
}

bool ChannelRegistry::ApplyChannelUpdate(const CsUpdateChannelInfo& packet, std::string* error) {
    return ApplyChannelUpdateAt(packet, std::time(nullptr), error);
}

bool ChannelRegistry::ApplyChannelUpdateAt(const CsUpdateChannelInfo& packet, std::time_t now,
                                           std::string* error) {
    if (packet.header.classification != static_cast<std::uint8_t>(Category::ClientToServer) ||
        packet.header.message_id != static_cast<std::uint8_t>(MessageId::UpdateChannelInfo) ||
        packet.header.size != sizeof(CsUpdateChannelInfo)) {
        if (error) *error = "invalid channel update header";
        return false;
    }
    if (packet.gc_no <= 0 || packet.channel_no < 0 || packet.max_user_num < 0 || packet.cur_user_num < 0 ||
        packet.port <= 0 || packet.port > 65535) {
        if (error) *error = "invalid channel update fields";
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    // The original 0828 daemon ignores message 2 until message 14 has made
    // the Bridge GC boundary table authoritative.  A compatibility mode may
    // retain an early update, but it is still not published until GC arrives.
    if (boundaries_.empty() && !allow_pending_before_gc_) {
        if (error) *error = "GC table is not ready";
        return false;
    }
    if (!endpoints_.count(packet.gc_no) && endpoints_.size() >= kMaximumEndpoints) {
        if (error) *error = "channel registry endpoint limit reached";
        return false;
    }
    ChannelEndpoint endpoint;
    endpoint.gc_no = packet.gc_no;
    endpoint.group_hint = packet.group;
    endpoint.channel_no = packet.channel_no;
    endpoint.max_users = packet.max_user_num;
    endpoint.current_users = packet.cur_user_num;
    endpoint.ip = std::string(FixedString(packet.server_ip, sizeof(packet.server_ip)));
    endpoint.port = packet.port;
    endpoint.last_update = now;
    endpoints_[packet.gc_no] = std::move(endpoint);
    RebuildGroups();
    return true;
}

std::size_t ChannelRegistry::ExpireStale(std::time_t now, unsigned int maximum_age_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::size_t removed = 0;
    for (std::map<int, ChannelEndpoint>::iterator it = endpoints_.begin(); it != endpoints_.end();) {
        const std::time_t timestamp = it->second.last_update;
        if (timestamp > 0 && now > timestamp &&
            static_cast<unsigned long long>(now - timestamp) > maximum_age_seconds) {
            endpoints_.erase(it++);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed) RebuildGroups();
    return removed;
}

bool ChannelRegistry::GcReady() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !boundaries_.empty();
}

std::vector<ChannelGroupSnapshot> ChannelRegistry::Snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ChannelGroupSnapshot> result;
    for (std::map<int, std::map<int, ChannelEndpoint>>::const_iterator group_it = groups_.begin();
         group_it != groups_.end(); ++group_it) {
        const int group_id = group_it->first;
        const std::map<int, ChannelEndpoint>& channels = group_it->second;
        if (channels.empty()) continue;
        ChannelGroupSnapshot group;
        const auto name = group_names_.find(group_id);
        group.name = name == group_names_.end() ? "#sg." + std::to_string(group_id) : name->second;
        for (std::map<int, ChannelEndpoint>::const_iterator channel_it = channels.begin();
             channel_it != channels.end(); ++channel_it) {
            group.channels.push_back(channel_it->second);
        }
        result.push_back(std::move(group));
    }
    return result;
}

Bytes ChannelRegistry::SerializeSnapshot(const std::vector<ChannelGroupSnapshot>& groups) {
    Bytes body;
    for (std::size_t group_index = 0; group_index < groups.size(); ++group_index) {
        const ChannelGroupSnapshot& group = groups[group_index];
        ServerGroupInfoWire group_wire{};
        CopyFixed(group_wire.server_group_name, group.name);
        group_wire.server_count = static_cast<std::int32_t>(group.channels.size());
        AppendPod(body, group_wire);
        for (std::size_t channel_index = 0; channel_index < group.channels.size(); ++channel_index) {
            const ChannelEndpoint& channel = group.channels[channel_index];
            ServerInfoWire server{};
            CopyFixed(server.channel_name, "#ch." + std::to_string(channel.channel_no));
            server.max_user_num = channel.max_users;
            server.cur_user_num = channel.current_users;
            CopyFixed(server.server_ip, channel.ip);
            server.port = channel.port;
            AppendPod(body, server);
        }
    }
    return body;
}

Bytes ChannelRegistry::SerializeAskChannelBody() const {
    return SerializeSnapshot(Snapshot());
}

} }  // namespace dnf::channel
