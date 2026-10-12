#pragma once

#include "channel/protocol.hpp"

#include <mutex>
#include <string>
#include <vector>

namespace dnf { namespace channel {

struct ChannelTableRow {
    int server_group;
    int channel_no;
    int channel_type;
};

struct ChannelScriptStats {
    ChannelScriptStats() : dungeon_groups(0), dungeon_mappings(0), channel_rows(0) {}
    std::size_t dungeon_groups;
    std::size_t dungeon_mappings;
    std::size_t channel_rows;
};

struct ScriptSnapshot {
    std::string version;
    Bytes script;
    std::vector<ChannelTableRow> rows;
    ChannelScriptStats stats;
};

class ScriptStore {
public:
    ScriptStore(std::string script_path, std::string version_path);

    bool Reload(std::string* error = nullptr);
    bool ReplaceFromBridge(std::string version, const Bytes& script, std::string* error = nullptr);
    ScriptSnapshot Snapshot() const;

    static bool ParseChannelTable(const Bytes& script, std::vector<ChannelTableRow>* rows,
                                  std::string* error = nullptr);
    static bool ValidateChannelScript(const Bytes& script, std::vector<ChannelTableRow>* rows,
                                      ChannelScriptStats* stats,
                                      std::string* error = nullptr);
    static std::string ParseVersionText(std::string text);

private:
    bool LoadUnlocked(std::string* error);

    std::string script_path_;
    std::string version_path_;
    mutable std::mutex mutex_;
    ScriptSnapshot snapshot_;
};

} }  // namespace dnf::channel
