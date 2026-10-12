#include "dfbridge/channel_info.hpp"
#include "dfbridge/repository.hpp"
#include "dfbridge/script_thread.hpp"
#include "dfbridge/service.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace {

void PrintUsage(const char* executable) {
    std::cerr
        << "usage:\n"
        << "  " << executable << " <cfg-name> start|run|test|stop\n"
        << "  " << executable << " validate <channel_info.etc>\n"
        << "  " << executable << " rewrite <input.etc> <output.etc>\n"
#ifdef DFBRIDGE_WITH_MYSQL
        << "  " << executable
        << " db-once <host> <database> <user> <password-env> <output.etc>\n"
#endif
        ;
}

int ValidateFile(const std::string& path) {
    dfbridge::ChannelInfoParser parser;
    auto snapshot = parser.ParseFile(path);
    dfbridge::ValidateSnapshot(snapshot);
    const auto counts = dfbridge::CountSnapshot(snapshot);
    std::set<std::int32_t> kinds;
    std::set<std::int32_t> groups;
    std::size_t zombie_channels = 0;
    std::size_t raid_channels = 0;
    for (const auto& row : snapshot.channels) {
        kinds.insert(row.channel_kind);
        groups.insert(row.server_group);
        if (row.channel_kind == dfbridge::kZombieChannelKind) ++zombie_channels;
        if (row.channel_kind == dfbridge::kRaidChannelKind) ++raid_channels;
    }
    std::cout << "dungeons=" << counts.dungeon_groups
              << " dungeon_ids=" << counts.dungeon_ids
              << " channels=" << counts.channels
              << " server_groups=" << counts.server_groups << '\n';
    std::cout << "groups=";
    for (const auto group : groups) std::cout << group << ',';
    std::cout << " kinds=";
    for (const auto kind : kinds) std::cout << kind << ',';
    std::cout << " zombie_channels=" << zombie_channels
              << " raid_channels=" << raid_channels << '\n';
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 3 && dfbridge::IsServiceCommand(argv[2])) {
            return dfbridge::RunServiceCommand(argv[1], argv[2]);
        }
        if (argc == 3 && std::string(argv[1]) == "validate") {
            return ValidateFile(argv[2]);
        }
        if (argc == 4 && std::string(argv[1]) == "rewrite") {
            dfbridge::ChannelInfoParser parser;
            dfbridge::ChannelInfoWriter writer;
            auto snapshot = parser.ParseFile(argv[2]);
            writer.WriteAtomically(snapshot, argv[3]);
            return ValidateFile(argv[3]);
        }
#ifdef DFBRIDGE_WITH_MYSQL
        if (argc == 7 && std::string(argv[1]) == "db-once") {
            const char* password = std::getenv(argv[5]);
            if (!password) throw std::runtime_error("password environment variable is not set");
            dfbridge::MysqlConfig config{argv[2], 3306, argv[3], argv[4], password};
            auto repository = dfbridge::MakeMysqlRepository(std::move(config));
            dfbridge::ScriptCache cache;
            dfbridge::ScriptThread thread(*repository, cache, argv[6]);
            thread.ReloadOnce();
            std::cout << "generated " << argv[6] << " version=" << cache.Version() << '\n';
            return 0;
        }
#endif
        PrintUsage(argv[0]);
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "df_bridge_r: " << error.what() << '\n';
        return 1;
    }
}
