#include "dfbridge/channel_info.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace dfbridge {
namespace {

std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string StripBackticks(std::string value) {
    value = Trim(std::move(value));
    if (value.size() >= 2 && value.front() == '`' && value.back() == '`') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::int32_t ParseInt(const std::string& value, const char* field) {
    std::size_t consumed = 0;
    const long parsed = std::stol(value, &consumed, 10);
    if (consumed != value.size()) {
        throw std::runtime_error(std::string("invalid integer in ") + field + ": " + value);
    }
    return static_cast<std::int32_t>(parsed);
}

double ParseDouble(const std::string& value, const char* field) {
    std::size_t consumed = 0;
    const double parsed = std::stod(value, &consumed);
    if (consumed != value.size() || !std::isfinite(parsed)) {
        throw std::runtime_error(std::string("invalid number in ") + field + ": " + value);
    }
    return parsed;
}

std::vector<std::string> SplitWords(const std::string& line) {
    std::istringstream input(line);
    std::vector<std::string> words;
    for (std::string word; input >> word;) {
        words.push_back(std::move(word));
    }
    return words;
}

std::int32_t ParseDungeonId(const std::string& value) {
    const std::vector<std::string> words = SplitWords(value);
    if (words.empty()) throw std::runtime_error("empty dungeon_id");
    const double parsed = ParseDouble(words[0], "dungeon_id");
    if (parsed != std::floor(parsed) ||
        parsed < static_cast<double>(std::numeric_limits<std::int32_t>::min()) ||
        parsed > static_cast<double>(std::numeric_limits<std::int32_t>::max())) {
        throw std::runtime_error("dungeon_id is not an int32: " + value);
    }
    return static_cast<std::int32_t>(parsed);
}

std::string NormalizeChannelName(std::string value) {
    return StripBackticks(std::move(value));
}

std::string NormalizeSemanticText(std::string value) {
    value = Trim(std::move(value));
    bool changed = true;
    while (changed && value.size() >= 2) {
        changed = false;
        const char first = value.front();
        const char last = value.back();
        if ((first == '`' && last == '`') ||
            (first == '"' && last == '"') ||
            (first == '\'' && last == '\'') ||
            (first == '<' && last == '>')) {
            value = Trim(value.substr(1, value.size() - 2));
            changed = true;
        }
    }
    return value;
}

struct SemanticDungeon {
    std::string display_name;
    std::set<std::int32_t> dungeon_ids;
};

struct SemanticChannel {
    std::string channel_name;
    std::int32_t channel_kind{};
    std::string kind_name;
    double exp_rate{};
    std::array<double, 10> exp_bonus{};
};

bool SameNumber(const double lhs, const double rhs) {
    return lhs == rhs;
}

bool SameChannel(const SemanticChannel& lhs, const SemanticChannel& rhs) {
    if (lhs.channel_name != rhs.channel_name ||
        lhs.channel_kind != rhs.channel_kind ||
        lhs.kind_name != rhs.kind_name ||
        !SameNumber(lhs.exp_rate, rhs.exp_rate)) {
        return false;
    }
    for (std::size_t i = 0; i < lhs.exp_bonus.size(); ++i) {
        if (!SameNumber(lhs.exp_bonus[i], rhs.exp_bonus[i])) return false;
    }
    return true;
}

bool BuildSemanticDungeons(
    const ChannelSnapshot& snapshot,
    std::map<std::string, SemanticDungeon>* output) {
    for (std::vector<DungeonGroup>::const_iterator item = snapshot.dungeons.begin();
         item != snapshot.dungeons.end(); ++item) {
        const std::string kind = NormalizeKindName(item->kind_name);
        SemanticDungeon value;
        value.display_name = NormalizeSemanticText(item->display_name);
        value.dungeon_ids.insert(item->dungeon_ids.begin(), item->dungeon_ids.end());
        if (!output->insert(std::make_pair(kind, value)).second) return false;
    }
    return true;
}

bool BuildSemanticChannels(
    const ChannelSnapshot& snapshot,
    std::map<std::pair<std::int32_t, std::int32_t>, SemanticChannel>* output) {
    for (std::vector<ChannelRow>::const_iterator item = snapshot.channels.begin();
         item != snapshot.channels.end(); ++item) {
        if (!IsSupportedChannelKind(item->channel_kind)) return false;
        if (!item->is_use) continue;
        SemanticChannel value;
        value.channel_name = NormalizeSemanticText(item->channel_name);
        value.channel_kind = item->channel_kind;
        value.kind_name = NormalizeKindName(item->kind_name);
        value.exp_rate = item->exp_rate;
        value.exp_bonus = item->exp_bonus;
        const std::pair<std::int32_t, std::int32_t> key(
            item->server_group, item->channel_number);
        if (!output->insert(std::make_pair(key, value)).second) return false;
    }
    return true;
}

}  // namespace

bool IsSupportedChannelKind(std::int32_t kind) noexcept {
    return (kind >= 0 && kind <= kTournamentChannelKind) ||
           kind == kRaidChannelKind;
}

std::string NormalizeKindName(std::string value) {
    value = StripBackticks(std::move(value));
    if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

std::string RenderDungeonKind(std::string value) {
    return "`[" + NormalizeKindName(std::move(value)) + "]`";
}

std::string RenderServerKind(std::string value) {
    return RenderDungeonKind(std::move(value));
}

bool ChannelSnapshotsSemanticallyEquivalent(const ChannelSnapshot& lhs,
                                            const ChannelSnapshot& rhs) {
    std::map<std::string, SemanticDungeon> lhs_dungeons;
    std::map<std::string, SemanticDungeon> rhs_dungeons;
    if (!BuildSemanticDungeons(lhs, &lhs_dungeons) ||
        !BuildSemanticDungeons(rhs, &rhs_dungeons) ||
        lhs_dungeons.size() != rhs_dungeons.size()) {
        return false;
    }
    for (std::map<std::string, SemanticDungeon>::const_iterator item =
             lhs_dungeons.begin();
         item != lhs_dungeons.end(); ++item) {
        const std::map<std::string, SemanticDungeon>::const_iterator other =
            rhs_dungeons.find(item->first);
        if (other == rhs_dungeons.end() ||
            item->second.display_name != other->second.display_name ||
            item->second.dungeon_ids != other->second.dungeon_ids) {
            return false;
        }
    }

    std::map<std::pair<std::int32_t, std::int32_t>, SemanticChannel> lhs_channels;
    std::map<std::pair<std::int32_t, std::int32_t>, SemanticChannel> rhs_channels;
    if (!BuildSemanticChannels(lhs, &lhs_channels) ||
        !BuildSemanticChannels(rhs, &rhs_channels) ||
        lhs_channels.size() != rhs_channels.size()) {
        return false;
    }
    for (std::map<std::pair<std::int32_t, std::int32_t>,
                  SemanticChannel>::const_iterator item = lhs_channels.begin();
         item != lhs_channels.end(); ++item) {
        const std::map<std::pair<std::int32_t, std::int32_t>,
                       SemanticChannel>::const_iterator other =
            rhs_channels.find(item->first);
        if (other == rhs_channels.end() || !SameChannel(item->second, other->second)) {
            return false;
        }
    }
    return true;
}

SnapshotCounts CountSnapshot(const ChannelSnapshot& snapshot) {
    SnapshotCounts counts;
    counts.dungeon_groups = snapshot.dungeons.size();
    for (const auto& dungeon : snapshot.dungeons) {
        counts.dungeon_ids += dungeon.dungeon_ids.size();
    }
    counts.channels = snapshot.channels.size();
    std::set<std::int32_t> groups;
    for (const auto& channel : snapshot.channels) {
        groups.insert(channel.server_group);
    }
    counts.server_groups = groups.size();
    return counts;
}

void ValidateSnapshot(const ChannelSnapshot& snapshot) {
    if (snapshot.script_version.size() > 15) {
        throw std::runtime_error("script version exceeds the 16-byte wire field");
    }

    std::set<std::string> dungeon_names;
    for (const auto& dungeon : snapshot.dungeons) {
        const auto kind = NormalizeKindName(dungeon.kind_name);
        if (kind.empty() || kind.size() + 4 >= 20) {
            throw std::runtime_error("invalid dungeon kind_name: " + dungeon.kind_name);
        }
        if (dungeon.display_name.empty() || dungeon.display_name.size() >= 30) {
            throw std::runtime_error("invalid dungeon display_name for " + kind);
        }
        if (!dungeon_names.insert(kind).second) {
            throw std::runtime_error("duplicate dungeon group: " + kind);
        }
    }

    std::set<std::pair<std::int32_t, std::int32_t>> channel_keys;
    for (const auto& channel : snapshot.channels) {
        if (!IsSupportedChannelKind(channel.channel_kind)) {
            throw std::runtime_error(
                "unsupported channel kind: expected 0..20 or Raid 23; 21/22 are rejected");
        }
        if (!channel.is_use) {
            continue;
        }
        if (channel.server_group <= 0 || channel.channel_number <= 0) {
            throw std::runtime_error("server_group and channel_number must be positive");
        }
        if (channel.channel_name.empty()) {
            throw std::runtime_error("channel_name cannot be empty");
        }
        if (NormalizeKindName(channel.kind_name).empty()) {
            throw std::runtime_error("kind_name cannot be empty");
        }
        if (!std::isfinite(channel.exp_rate)) {
            throw std::runtime_error("exp_rate must be finite");
        }
        for (double value : channel.exp_bonus) {
            if (!std::isfinite(value)) {
                throw std::runtime_error("exp bonus must be finite");
            }
        }
        const auto key = std::make_pair(channel.server_group, channel.channel_number);
        if (!channel_keys.insert(key).second) {
            throw std::runtime_error("duplicate server_group/channel_number");
        }
    }

    std::set<std::int32_t> gc_bounds;
    for (const auto& group : snapshot.gc_groups) {
        if (group.group_name.empty() || group.group_name.size() >= 20) {
            throw std::runtime_error("GC group names must fit the 20-byte wire field");
        }
        if (group.upper_gc_no <= 0 || !gc_bounds.insert(group.upper_gc_no).second) {
            throw std::runtime_error("invalid or duplicate GC upper bound");
        }
    }
}

std::string ChannelInfoWriter::Render(const ChannelSnapshot& snapshot) const {
    ValidateSnapshot(snapshot);
    std::ostringstream out;

    for (const auto& dungeon : snapshot.dungeons) {
        out << "[dungeon]\n";
        out << RenderDungeonKind(dungeon.kind_name) << '\n';
        out << dungeon.display_name << '\n';
        for (const auto id : dungeon.dungeon_ids) {
            out << id << '\n';
        }
        out << "[/dungeon]\n\n\n";
    }

    std::vector<const ChannelRow*> rows;
    rows.reserve(snapshot.channels.size());
    for (const auto& channel : snapshot.channels) {
        if (channel.is_use) {
            rows.push_back(&channel);
        }
    }
    std::stable_sort(rows.begin(), rows.end(), [](const ChannelRow* lhs, const ChannelRow* rhs) {
        return std::tie(lhs->server_group, lhs->channel_number) <
               std::tie(rhs->server_group, rhs->channel_number);
    });

    std::int32_t current_group = -1;
    out << std::fixed << std::setprecision(1);
    for (const auto* row : rows) {
        if (row->server_group != current_group) {
            if (current_group != -1) {
                out << "[/server]\n\n\n";
            }
            current_group = row->server_group;
            // This is the exact layout emitted by the 0828 df_bridge_r binary.
            out << "[server]\n" << current_group << '\n';
        }
        out << "   " << row->channel_number
            << "   " << NormalizeChannelName(row->channel_name)
            << "   " << row->channel_kind
            << "   " << RenderServerKind(row->kind_name)
            << "   " << row->exp_rate;
        for (const double bonus : row->exp_bonus) {
            out << "   " << bonus;
        }
        out << '\n';
    }
    if (current_group != -1) {
        out << "[/server]";
    }
    return out.str();
}

void ChannelInfoWriter::WriteAtomically(const ChannelSnapshot& snapshot,
                                        const std::string& target) const {
    const std::string rendered = Render(snapshot);
    std::ostringstream temporary_name;
    temporary_name << target << ".tmp."
                   << std::chrono::high_resolution_clock::now()
                          .time_since_epoch().count();
    const std::string temporary = temporary_name.str();
    {
        std::ofstream output(temporary.c_str(), std::ios::binary | std::ios::trunc);
        if (!output) {
            throw std::runtime_error("cannot open temporary channel_info file: " + temporary);
        }
        output.write(rendered.data(), static_cast<std::streamsize>(rendered.size()));
        output.flush();
        if (!output) {
            throw std::runtime_error("failed while writing channel_info: " + temporary);
        }
    }
    if (std::rename(temporary.c_str(), target.c_str()) != 0) {
#ifdef _WIN32
        // Linux rename() atomically replaces an existing file. The Windows CRT
        // needs the target removed first; this branch is only for local tests.
        std::remove(target.c_str());
        if (std::rename(temporary.c_str(), target.c_str()) == 0) return;
#endif
        const std::string reason = std::strerror(errno);
        std::remove(temporary.c_str());
        throw std::runtime_error("cannot publish channel_info: " + reason);
    }
}

ChannelSnapshot ChannelInfoParser::Parse(const std::string& text) const {
    std::vector<std::string> lines;
    std::istringstream input(text);
    for (std::string line; std::getline(input, line);) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
    }

    ChannelSnapshot snapshot;
    for (std::size_t i = 0; i < lines.size();) {
        const auto line = Trim(lines[i]);
        if (line == "[dungeon]") {
            DungeonGroup group;
            if (++i >= lines.size()) throw std::runtime_error("truncated dungeon block");
            group.kind_name = NormalizeKindName(lines[i++]);
            if (i >= lines.size()) throw std::runtime_error("truncated dungeon display name");
            group.display_name = Trim(lines[i++]);
            bool dungeon_closed = false;
            for (; i < lines.size(); ++i) {
                const auto item = Trim(lines[i]);
                if (item == "[/dungeon]") {
                    ++i;
                    dungeon_closed = true;
                    break;
                }
                if (!item.empty()) {
                    group.dungeon_ids.push_back(ParseDungeonId(item));
                }
            }
            if (!dungeon_closed) throw std::runtime_error("unterminated dungeon block");
            snapshot.dungeons.push_back(std::move(group));
            continue;
        }

        if (line.rfind("[server]", 0) == 0) {
            std::string group_text = Trim(line.substr(8));
            ++i;
            while (group_text.empty() && i < lines.size()) {
                group_text = Trim(lines[i++]);
            }
            const auto server_group = ParseInt(group_text, "server_group");
            bool server_closed = false;
            for (; i < lines.size(); ++i) {
                const auto row_text = Trim(lines[i]);
                if (row_text == "[/server]") {
                    ++i;
                    server_closed = true;
                    break;
                }
                if (row_text.empty()) {
                    continue;
                }
                const auto words = SplitWords(row_text);
                // 15 business values; the official file may carry a 16th empty
                // localized-string trailer, which the old bridge did not store.
                if (words.size() < 15 || words.size() > 16 ||
                    (words.size() == 16 && words[15] != "``")) {
                    throw std::runtime_error("server row must contain 15 fields plus optional `` trailer: " + row_text);
                }
                ChannelRow row;
                row.server_group = server_group;
                row.is_use = true;
                row.channel_number = ParseInt(words[0], "channel_number");
                row.channel_name = NormalizeChannelName(words[1]);
                row.channel_kind = ParseInt(words[2], "channel_kind");
                if (!IsSupportedChannelKind(row.channel_kind)) {
                    throw std::runtime_error(
                        "unsupported channel_kind in server row: expected 0..20 or Raid 23");
                }
                row.kind_name = NormalizeKindName(words[3]);
                row.exp_rate = ParseDouble(words[4], "exp_rate");
                for (std::size_t n = 0; n < row.exp_bonus.size(); ++n) {
                    row.exp_bonus[n] = ParseDouble(words[5 + n], "exp_bonus");
                }
                snapshot.channels.push_back(std::move(row));
            }
            if (!server_closed) throw std::runtime_error("unterminated server block");
            continue;
        }
        ++i;
    }
    return snapshot;
}

ChannelSnapshot ChannelInfoParser::ParseFile(const std::string& path) const {
    return Parse(ReadChannelInfoFile(path));
}

std::string ReadChannelInfoFile(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open channel_info file: " + path);
    }
    std::ostringstream text;
    text << input.rdbuf();
    if (!input.good() && !input.eof()) {
        throw std::runtime_error("cannot read channel_info file: " + path);
    }
    return text.str();
}

}  // namespace dfbridge
