#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace dfbridge {

// Persistent channel kinds shared with Channel and Game.
static const std::int32_t kZombieChannelKind = 19;
static const std::int32_t kTournamentChannelKind = 20;
static const std::int32_t kRaidChannelKind = 23;

enum class ChannelKind : std::int32_t {
    Common = 0,
    TradeAuction = 3,
    Broadcast = 7,
    IntegratedPvp = 8,
    SpecialDungeon = 11,
    FreePractice = 13,
    LeagueQualifier = 14,
    RankedPvp = 15,
    FreePvp = 16,
    Report = 17,
    Zombie = 19,
    Tournament = 20,
    InvalidSentinel = 21,
    Reserved = 22,
    Raid = 23,
};

bool IsSupportedChannelKind(std::int32_t kind) noexcept;

struct DungeonGroup {
    // Database form may be "zombie", "[zombie]", or "`[zombie]`".
    std::string kind_name;
    std::string display_name;
    std::vector<std::int32_t> dungeon_ids;

    DungeonGroup() {}
    DungeonGroup(std::string kind, std::string display,
                 std::vector<std::int32_t> ids)
        : kind_name(std::move(kind)),
          display_name(std::move(display)),
          dungeon_ids(std::move(ids)) {}
};

struct ChannelRow {
    std::int32_t server_group{};
    bool is_use{};
    std::int32_t channel_number{};
    std::string channel_name;
    std::int32_t channel_kind{};
    std::string kind_name;
    double exp_rate{};
    std::array<double, 10> exp_bonus{};
};

struct GcGroup {
    std::string group_name;
    // Original bridge lookup: 1 <= upper_gc_no - gc_no <= 999.
    std::int32_t upper_gc_no{};

    GcGroup() {}
    GcGroup(std::string name, std::int32_t upper)
        : group_name(std::move(name)), upper_gc_no(upper) {}
};

struct ChannelSnapshot {
    std::string script_version;
    std::vector<DungeonGroup> dungeons;
    std::vector<ChannelRow> channels;
    std::vector<GcGroup> gc_groups;
};

struct SnapshotCounts {
    std::size_t dungeon_groups{};
    std::size_t dungeon_ids{};
    std::size_t channels{};
    std::size_t server_groups{};
};

SnapshotCounts CountSnapshot(const ChannelSnapshot& snapshot);
void ValidateSnapshot(const ChannelSnapshot& snapshot);

}  // namespace dfbridge
