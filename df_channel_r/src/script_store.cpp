#include "channel/script_store.hpp"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

namespace dnf { namespace channel {
namespace {

constexpr std::size_t kMaximumScriptSize = 1U << 20;

std::string Trim(std::string value) {
    const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), is_space));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), is_space).base(), value.end());
    return value;
}

bool ReadFile(const std::string& path, Bytes* output, std::string* error) {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        if (error) *error = "cannot open " + path;
        return false;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff length = input.tellg();
    if (length < 0 || static_cast<unsigned long long>(length) > kMaximumScriptSize) {
        if (error) *error = "script size is invalid";
        return false;
    }
    input.seekg(0, std::ios::beg);
    output->resize(static_cast<std::size_t>(length));
    if (!output->empty()) {
        input.read(reinterpret_cast<char*>(output->data()), static_cast<std::streamsize>(output->size()));
    }
    if (!input && !output->empty()) {
        if (error) *error = "short read from " + path;
        return false;
    }
    return true;
}

bool WriteFile(const std::string& path, const void* data, std::size_t size, std::string* error) {
    std::ofstream output(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!output) {
        if (error) *error = "cannot create " + path;
        return false;
    }
    output.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    output.flush();
    if (!output) {
        if (error) *error = "write failed for " + path;
        return false;
    }
    return true;
}

bool FileExists(const std::string& path) {
    struct stat status;
    return ::stat(path.c_str(), &status) == 0;
}

bool LoadSnapshotPair(const std::string& script_path, const std::string& version_path,
                      ScriptSnapshot* snapshot, Bytes* raw_version, std::string* error) {
    if (!snapshot) return false;
    ScriptSnapshot next;
    if (!ReadFile(script_path, &next.script, error) ||
        !ScriptStore::ValidateChannelScript(next.script, &next.rows, &next.stats, error)) return false;
    Bytes version_bytes;
    if (!ReadFile(version_path, &version_bytes, error)) return false;
    next.version = ScriptStore::ParseVersionText(
        std::string(reinterpret_cast<const char*>(version_bytes.data()), version_bytes.size()));
    if (next.version.empty()) {
        if (error) *error = version_path + " contains no version";
        return false;
    }
    if (raw_version) *raw_version = version_bytes;
    *snapshot = std::move(next);
    return true;
}

bool RestoreRollbackPair(const std::string& script_path, const std::string& version_path,
                         std::string* error) {
    const std::string script_rollback = script_path + ".rollback";
    const std::string version_rollback = version_path + ".rollback";
    ScriptSnapshot rollback;
    Bytes raw_version;
    std::string rollback_error;
    if (!LoadSnapshotPair(script_rollback, version_rollback, &rollback, &raw_version,
                          &rollback_error)) {
        if (error) *error = "invalid rollback pair: " + rollback_error;
        return false;
    }
    // Copy rather than consume the rollback pair.  If the process dies during
    // recovery, the next startup can deterministically repeat the operation.
    if (!WriteFile(script_path, rollback.script.data(), rollback.script.size(), error) ||
        !WriteFile(version_path, raw_version.data(), raw_version.size(), error)) return false;
    return true;
}

void CleanupRecoveryFiles(const std::string& script_path, const std::string& version_path) {
    std::remove((script_path + ".new").c_str());
    std::remove((version_path + ".new").c_str());
    std::remove((script_path + ".rollback").c_str());
    std::remove((version_path + ".rollback").c_str());
}

bool ParseIntToken(const std::string& token, int* value) {
    if (!value) return false;
    std::istringstream input(token);
    char extra = 0;
    return (input >> *value) && !(input >> extra);
}

bool ParseDoubleToken(const std::string& token, double* value) {
    if (!value) return false;
    std::istringstream input(token);
    char extra = 0;
    return (input >> *value) && !(input >> extra) && std::isfinite(*value);
}

}  // namespace

ScriptStore::ScriptStore(std::string script_path, std::string version_path)
    : script_path_(std::move(script_path)), version_path_(std::move(version_path)) {}

std::string ScriptStore::ParseVersionText(std::string text) {
    std::istringstream input(std::move(text));
    std::string line;
    while (std::getline(input, line)) {
        line = Trim(std::move(line));
        if (line.empty() || line.front() == '#') continue;
        if (line.size() > 15) line.resize(15);  // legacy wire field is char[16]
        return line;
    }
    return {};
}

bool ScriptStore::ParseChannelTable(const Bytes& script, std::vector<ChannelTableRow>* rows,
                                    std::string* error) {
    if (!rows) return false;
    rows->clear();
    std::string text(reinterpret_cast<const char*>(script.data()), script.size());
    std::istringstream input(text);
    std::string line;
    bool in_server = false;
    bool expect_group = false;
    bool saw_server = false;
    int group = -1;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        const std::string trimmed = Trim(line);
        if (trimmed.compare(0, 8, "[server]") == 0) {
            if (in_server) {
                if (error) *error = "nested [server] block at line " + std::to_string(line_number);
                return false;
            }
            in_server = true;
            saw_server = true;
            group = -1;
            const std::string inline_group = Trim(trimmed.substr(8));
            expect_group = inline_group.empty();
            if (!expect_group) {
                std::istringstream group_stream(inline_group);
                if (!(group_stream >> group) || group < 0) {
                    if (error) {
                        *error = "invalid inline [server] group at line " +
                                 std::to_string(line_number);
                    }
                    return false;
                }
            }
            continue;
        }
        if (trimmed == "[/server]") {
            if (!in_server || expect_group || group < 0) {
                if (error) *error = "unexpected [/server] at line " + std::to_string(line_number);
                return false;
            }
            in_server = false;
            expect_group = false;
            continue;
        }
        if (!in_server || trimmed.empty() || trimmed.front() == '#') continue;
        if (expect_group) {
            std::istringstream group_stream(trimmed);
            if (!(group_stream >> group) || group < 0) {
                if (error) *error = "invalid [server] group at line " + std::to_string(line_number);
                return false;
            }
            expect_group = false;
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(trimmed.front()))) continue;
        std::istringstream row_stream(trimmed);
        std::vector<std::string> fields;
        for (std::string field; row_stream >> field;) fields.push_back(field);
        if (!fields.empty() && fields.back() == "``") fields.pop_back();
        if (fields.size() != 15) {
            if (error) *error = "channel row must contain 15 fields at line " +
                                std::to_string(line_number);
            return false;
        }
        int channel_no = -1;
        int channel_type = -1;
        double exp_rate = 0.0;
        if (!ParseIntToken(fields[0], &channel_no) || fields[1].empty() ||
            !ParseIntToken(fields[2], &channel_type) || fields[3].empty() ||
            !ParseDoubleToken(fields[4], &exp_rate)) {
            if (error) *error = "invalid channel row at line " + std::to_string(line_number);
            return false;
        }
        for (std::size_t rate = 5; rate < fields.size(); ++rate) {
            double value = 0.0;
            if (!ParseDoubleToken(fields[rate], &value)) {
                if (error) *error = "invalid channel rate at line " + std::to_string(line_number);
                return false;
            }
        }
        if (channel_no < 0 || !IsSupportedChannelType(channel_type)) {
            if (error) {
                *error = "unsupported channel number/type at line " + std::to_string(line_number) +
                         " (supported types are 0..20 and Raid 23; 21/22 are rejected)";
            }
            return false;
        }
        rows->push_back(ChannelTableRow{group, channel_no, channel_type});
    }
    if (in_server || expect_group) {
        if (error) *error = "unterminated [server] block";
        return false;
    }
    if (!saw_server || rows->empty()) {
        if (error) *error = "no [server] channel rows found";
        return false;
    }
    return true;
}

bool ScriptStore::ValidateChannelScript(const Bytes& script, std::vector<ChannelTableRow>* rows,
                                        ChannelScriptStats* stats, std::string* error) {
    if (!rows || !stats) return false;
    if (!ParseChannelTable(script, rows, error)) return false;

    ChannelScriptStats next;
    next.channel_rows = rows->size();
    std::string text(reinterpret_cast<const char*>(script.data()), script.size());
    std::istringstream input(text);
    std::string line;
    bool in_dungeon = false;
    int metadata_lines = 0;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        const std::string trimmed = Trim(line);
        if (trimmed.compare(0, 9, "[dungeon]") == 0) {
            if (in_dungeon) {
                if (error) *error = "nested [dungeon] block at line " + std::to_string(line_number);
                return false;
            }
            in_dungeon = true;
            metadata_lines = 0;
            ++next.dungeon_groups;
            continue;
        }
        if (trimmed == "[/dungeon]") {
            if (!in_dungeon || metadata_lines < 2) {
                if (error) *error = "invalid [/dungeon] at line " + std::to_string(line_number);
                return false;
            }
            in_dungeon = false;
            continue;
        }
        if (!in_dungeon || trimmed.empty() || trimmed.front() == '#') continue;
        if (metadata_lines < 2) {
            ++metadata_lines;
            continue;
        }
        std::istringstream row_stream(trimmed);
        std::string first;
        row_stream >> first;
        double dungeon_id = 0.0;
        if (!ParseDoubleToken(first, &dungeon_id) || dungeon_id < 0.0) {
            if (error) *error = "invalid dungeon mapping at line " + std::to_string(line_number);
            return false;
        }
        ++next.dungeon_mappings;
    }
    if (in_dungeon) {
        if (error) *error = "unterminated [dungeon] block";
        return false;
    }
    if (next.dungeon_groups == 0) {
        if (error) *error = "no [dungeon] blocks found";
        return false;
    }
    *stats = next;
    return true;
}

bool ScriptStore::Reload(std::string* error) {
    std::lock_guard<std::mutex> lock(mutex_);
    return LoadUnlocked(error);
}

bool ScriptStore::LoadUnlocked(std::string* error) {
    const std::string script_rollback = script_path_ + ".rollback";
    const std::string version_rollback = version_path_ + ".rollback";
    // ReplaceFromBridge moves the old script before it moves the old version.
    // A crash in that one-rename window leaves the old version in place and
    // only the script rollback.  Restore that exact old pair before normal
    // validation; no database/config values are synthesized here.
    if (!FileExists(script_path_) && FileExists(version_path_) &&
        FileExists(script_rollback) && !FileExists(version_rollback)) {
        if (std::rename(script_rollback.c_str(), script_path_.c_str()) != 0) {
            if (error) *error = "cannot restore single script rollback";
            return false;
        }
    }
    const bool main_pair_exists = FileExists(script_path_) && FileExists(version_path_);
    const bool rollback_pair_exists = FileExists(script_rollback) &&
                                      FileExists(version_rollback);
    if (!main_pair_exists && rollback_pair_exists &&
        !RestoreRollbackPair(script_path_, version_path_, error)) return false;

    ScriptSnapshot next;
    std::string main_error;
    if (LoadSnapshotPair(script_path_, version_path_, &next, nullptr, &main_error)) {
        CleanupRecoveryFiles(script_path_, version_path_);
        snapshot_ = std::move(next);
        return true;
    }
    if (rollback_pair_exists) {
        if (!RestoreRollbackPair(script_path_, version_path_, error)) return false;
        if (!LoadSnapshotPair(script_path_, version_path_, &next, nullptr, error)) return false;
        CleanupRecoveryFiles(script_path_, version_path_);
        snapshot_ = std::move(next);
        return true;
    }
    if (error) *error = main_error;
    return false;
}

bool ScriptStore::ReplaceFromBridge(std::string version, const Bytes& script, std::string* error) {
    version = ParseVersionText(std::move(version));
    std::vector<ChannelTableRow> parsed;
    ChannelScriptStats stats;
    if (version.empty() || !ValidateChannelScript(script, &parsed, &stats, error)) {
        if (error && version.empty()) *error = "Bridge supplied an empty script version";
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string script_tmp = script_path_ + ".new";
    const std::string version_tmp = version_path_ + ".new";
    const std::string script_rollback = script_path_ + ".rollback";
    const std::string version_rollback = version_path_ + ".rollback";
    std::remove(script_tmp.c_str());
    std::remove(version_tmp.c_str());
    std::remove(script_rollback.c_str());
    std::remove(version_rollback.c_str());
    if (!WriteFile(script_tmp, script.data(), script.size(), error) ||
        !WriteFile(version_tmp, version.data(), version.size(), error)) {
        std::remove(script_tmp.c_str());
        std::remove(version_tmp.c_str());
        return false;
    }

    bool saved_script = false;
    bool saved_version = false;
    bool installed_script = false;
    bool installed_version = false;
    const auto rollback = [&]() {
        if (installed_script) std::remove(script_path_.c_str());
        if (installed_version) std::remove(version_path_.c_str());
        if (saved_script) std::rename(script_rollback.c_str(), script_path_.c_str());
        if (saved_version) std::rename(version_rollback.c_str(), version_path_.c_str());
        std::remove(script_tmp.c_str());
        std::remove(version_tmp.c_str());
    };

    if (std::rename(script_path_.c_str(), script_rollback.c_str()) == 0) {
        saved_script = true;
    } else if (errno != ENOENT) {
        if (error) *error = "cannot save current channel script";
        rollback();
        return false;
    }
    if (std::rename(version_path_.c_str(), version_rollback.c_str()) == 0) {
        saved_version = true;
    } else if (errno != ENOENT) {
        if (error) *error = "cannot save current channel version";
        rollback();
        return false;
    }
    if (std::rename(script_tmp.c_str(), script_path_.c_str()) != 0) {
        if (error) *error = "cannot install Bridge script";
        rollback();
        return false;
    }
    installed_script = true;
    if (std::rename(version_tmp.c_str(), version_path_.c_str()) != 0) {
        if (error) *error = "cannot install Bridge version";
        rollback();
        return false;
    }
    installed_version = true;
    std::remove(script_rollback.c_str());
    std::remove(version_rollback.c_str());
    if (version.empty()) {
        if (error) *error = "empty installed version";
        return false;
    }
    ScriptSnapshot next;
    next.version = std::move(version);
    next.script = script;
    next.rows = std::move(parsed);
    next.stats = stats;
    snapshot_ = std::move(next);
    return true;
}

ScriptSnapshot ScriptStore::Snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return snapshot_;
}

} }  // namespace dnf::channel
