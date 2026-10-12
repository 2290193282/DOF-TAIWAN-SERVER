#include "dfbridge/bridge_core.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace dfbridge {
namespace {

std::string FixedString(const char* value, const std::size_t capacity) {
    const auto* end = static_cast<const char*>(std::memchr(value, '\0', capacity));
    return std::string(value, end ? end : value + capacity);
}

}  // namespace

void ScriptCache::Replace(std::string version, std::string script,
                          std::vector<GcGroup> gc_groups) {
    std::lock_guard<std::mutex> guard(mutex_);
    version_ = std::move(version);
    script_ = std::move(script);
    gc_groups_ = std::move(gc_groups);
}

std::string ScriptCache::Version() const {
    std::lock_guard<std::mutex> guard(mutex_);
    return version_;
}

std::string ScriptCache::Script() const {
    std::lock_guard<std::mutex> guard(mutex_);
    return script_;
}

std::vector<GcGroup> ScriptCache::GcGroups() const {
    std::lock_guard<std::mutex> guard(mutex_);
    return gc_groups_;
}

ChannelRegistry::ChannelRegistry(std::vector<GcGroup> gc_groups) {
    SetGcGroups(std::move(gc_groups));
}

void ChannelRegistry::SetGcGroups(std::vector<GcGroup> groups) {
    std::sort(groups.begin(), groups.end(), [](const GcGroup& lhs, const GcGroup& rhs) {
        return lhs.upper_gc_no < rhs.upper_gc_no;
    });
    std::lock_guard<std::mutex> guard(mutex_);
    bool changed = groups.size() != gc_groups_.size();
    if (!changed) {
        for (std::size_t i = 0; i < groups.size(); ++i) {
            if (groups[i].upper_gc_no != gc_groups_[i].upper_gc_no ||
                groups[i].group_name != gc_groups_[i].group_name) {
                changed = true;
                break;
            }
        }
    }
    if (!changed) return;
    gc_groups_ = std::move(groups);
    // GC index is positional after sorting. Any name/boundary change invalidates
    // the old index-to-map association, so preserve status only for an identical map.
    game_servers_.clear();
    game_servers_.resize(gc_groups_.size());
}

bool ChannelRegistry::RegisterChannelServer(const wire::CsNoticeChannelServer& packet) {
    if (packet.id <= 0 || packet.port <= 0) {
        return false;
    }
    const auto now = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> guard(mutex_);
    const std::vector<ChannelServerEndpoint>::iterator found =
        std::find_if(channel_servers_.begin(), channel_servers_.end(),
                     [&](const ChannelServerEndpoint& item) { return item.id == packet.id; });
    ChannelServerEndpoint value;
    value.id = packet.id;
    value.ip = FixedString(packet.server_ip, sizeof(packet.server_ip));
    value.port = packet.port;
    value.last_seen = now;
    if (found != channel_servers_.end()) {
        *found = std::move(value);
        return true;
    }
    if (channel_servers_.size() >= kMaxChannelServers) {
        return false;
    }
    channel_servers_.push_back(std::move(value));
    return true;
}

bool ChannelRegistry::ResolveGcGroupIndex(const std::int32_t gc_no,
                                          std::size_t* group_index) const {
    if (!group_index) return false;
    std::lock_guard<std::mutex> guard(mutex_);
    for (std::size_t i = 0; i < gc_groups_.size(); ++i) {
        const auto difference = gc_groups_[i].upper_gc_no - gc_no;
        if (difference > 0 && difference <= 999) {
            *group_index = i;
            return true;
        }
    }
    return false;
}

bool ChannelRegistry::UpdateGameServer(const wire::CsUpdateChannelInfo& packet) {
    if (packet.gc_no <= 0 || packet.channel_no <= 0 || packet.port <= 0) {
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    std::size_t group_index = 0;
    bool group_found = false;
    for (std::size_t i = 0; i < gc_groups_.size(); ++i) {
        const auto difference = gc_groups_[i].upper_gc_no - packet.gc_no;
        if (difference > 0 && difference <= 999) {
            group_index = i;
            group_found = true;
            break;
        }
    }
    if (!group_found) {
        return false;
    }
    std::map<std::int32_t, GameServerStatus>& group = game_servers_[group_index];
    if (group.find(packet.gc_no) == group.end() &&
        group.size() >= kMaxGameServersPerGroup) {
        return false;
    }
    GameServerStatus status;
    status.max_users = packet.max_user_num;
    status.current_users = packet.cur_user_num;
    status.ip = FixedString(packet.server_ip, sizeof(packet.server_ip));
    status.port = packet.port;
    status.gc_no = packet.gc_no;
    status.group = packet.group;
    status.channel_no = packet.channel_no;
    status.channel_name = "#ch." + std::to_string(packet.channel_no);
    status.last_seen = std::chrono::system_clock::now();
    group[packet.gc_no] = std::move(status);
    return true;
}

std::size_t ChannelRegistry::ChannelServerCount() const {
    std::lock_guard<std::mutex> guard(mutex_);
    return channel_servers_.size();
}

std::size_t ChannelRegistry::GameServerCount(const std::size_t group_index) const {
    std::lock_guard<std::mutex> guard(mutex_);
    if (group_index >= game_servers_.size()) {
        return 0;
    }
    return game_servers_[group_index].size();
}

std::vector<ChannelServerEndpoint> ChannelRegistry::ChannelServers() const {
    std::lock_guard<std::mutex> guard(mutex_);
    return channel_servers_;
}

std::vector<GameServerStatus> ChannelRegistry::GameServers() const {
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<GameServerStatus> output;
    for (std::vector<std::map<std::int32_t, GameServerStatus>>::const_iterator group =
             game_servers_.begin();
         group != game_servers_.end(); ++group) {
        for (std::map<std::int32_t, GameServerStatus>::const_iterator item = group->begin();
             item != group->end(); ++item) {
            output.push_back(item->second);
        }
    }
    return output;
}

ExpiryCounts ChannelRegistry::ExpireStale(
    const std::chrono::system_clock::time_point now,
    const std::chrono::seconds game_timeout,
    const std::chrono::seconds channel_timeout) {
    std::lock_guard<std::mutex> guard(mutex_);
    ExpiryCounts expired;
    const std::vector<ChannelServerEndpoint>::iterator channel_end =
        std::remove_if(channel_servers_.begin(), channel_servers_.end(),
                       [&](const ChannelServerEndpoint& endpoint) {
                           if (now - endpoint.last_seen > channel_timeout) {
                               ++expired.channel_servers;
                               return true;
                           }
                           return false;
                       });
    channel_servers_.erase(channel_end, channel_servers_.end());
    for (std::vector<std::map<std::int32_t, GameServerStatus>>::iterator group =
             game_servers_.begin();
         group != game_servers_.end(); ++group) {
        for (std::map<std::int32_t, GameServerStatus>::iterator item = group->begin();
             item != group->end();) {
            if (now - item->second.last_seen > game_timeout) {
                item = group->erase(item);
                ++expired.game_servers;
            } else {
                ++item;
            }
        }
    }
    return expired;
}

const std::vector<ProtocolRegistration>& RestoredProtocolTable() {
    static const std::vector<ProtocolRegistration> table{
        {wire::kCsNoticeChannelServer, false, "onCS_NOTICE_CHANNEL_SERVER"},
        {wire::kCsCheckScriptVersion, false, "onCS_CHECK_SCRIPT_VERSION"},
        {wire::kCsGetScript, false, "onCS_GET_SCRIPT"},
        {wire::kCsGetGcInfo, false, "onCS_GET_GC_INFO"},
        {wire::kCsUpdateChannelInfoExtra, true, "onCS_UPDATE_CHANNEL_INFO"},
    };
    return table;
}

}  // namespace dfbridge
