#include "dfbridge/bridge_core.hpp"
#include "dfbridge/channel_info.hpp"
#include "dfbridge/config.hpp"
#include "dfbridge/protocol.hpp"
#include "dfbridge/repository.hpp"
#include "dfbridge/script_thread.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

namespace {

dfbridge::ChannelSnapshot Fixture() {
    dfbridge::ChannelSnapshot snapshot;
    snapshot.script_version = "60";
    snapshot.dungeons = {
        {"zombie", "<4::channel_info_dname_18>", {3100, 3101}},
        {"tournament", "<4::channel_info_dname_19>", {}},
    };
    for (int channel = 186; channel <= 188; ++channel) {
        dfbridge::ChannelRow row;
        row.server_group = 17;
        row.is_use = true;
        row.channel_number = channel;
        row.channel_name = "<4::channel_info_cname_17" + std::to_string(channel - 185) + ">";
        row.channel_kind = dfbridge::kZombieChannelKind;
        row.kind_name = "zombie";
        snapshot.channels.push_back(row);
    }
    dfbridge::ChannelRow tournament;
    tournament.server_group = 98;
    tournament.is_use = true;
    tournament.channel_number = 164;
    tournament.channel_name = "<4::channel_info_cname_515>";
    tournament.channel_kind = dfbridge::kTournamentChannelKind;
    tournament.kind_name = "tournament";
    snapshot.channels.push_back(tournament);
    dfbridge::ChannelRow raid;
    raid.server_group = 98;
    raid.is_use = true;
    raid.channel_number = 165;
    raid.channel_name = "<4::channel_info_cname_516>";
    raid.channel_kind = dfbridge::kRaidChannelKind;
    raid.kind_name = "raid";
    snapshot.channels.push_back(raid);
    snapshot.gc_groups = {{"siroco", 4000}};
    return snapshot;
}

class HookRepository final : public dfbridge::IDChannelRepository {
public:
    HookRepository() : snapshot(Fixture()) {}

    void WorkerEnter() override {
        ++entered;
        const int current = ++active;
        int observed = max_active.load();
        while (current > observed &&
               !max_active.compare_exchange_weak(observed, current)) {
        }
    }

    void WorkerLeave() noexcept override {
        --active;
        ++left;
    }

    dfbridge::ChannelSnapshot LoadSnapshot() override {
        return snapshot;
    }

    std::string ReadScriptVersion() override {
        return "60";
    }

    std::atomic<int> entered{0};
    std::atomic<int> left{0};
    std::atomic<int> active{0};
    std::atomic<int> max_active{0};
    dfbridge::ChannelSnapshot snapshot;
};

std::string AuthoritativeInlineCrlfFixture() {
    return
        "[dungeon]\r\n"
        "`[tournament]`\r\n"
        "`<4::channel_info_dname_19>`\r\n"
        "[/dungeon]\r\n\r\n"
        "[dungeon]\r\n"
        "[zombie]\r\n"
        "<4::channel_info_dname_18>\r\n"
        "3101 ``\r\n"
        "3100\r\n"
        "[/dungeon]\r\n\r\n"
        "[server] 98\r\n"
        "164 `<4::channel_info_cname_515>` 20 `[tournament]` 0 0 0 0 0 0 0 0 0 0 0 ``\r\n"
        "165 `<4::channel_info_cname_516>` 23 `[raid]` 0 0 0 0 0 0 0 0 0 0 0 ``\r\n"
        "[/server]\r\n\r\n"
        "[server] 17\r\n"
        "188 `<4::channel_info_cname_173>` 19 `[zombie]` 0.0 0 0 0 0 0 0 0 0 0 0 ``\r\n"
        "187 `<4::channel_info_cname_172>` 19 `[zombie]` 0 0 0 0 0 0 0 0 0 0 0 ``\r\n"
        "186 `<4::channel_info_cname_171>` 19 `[zombie]` 0 0 0 0 0 0 0 0 0 0 0 ``\r\n"
        "[/server]";
}

void WriteBinaryFile(const char* path, const std::string& bytes) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    assert(output.good());
}

void TestRenderAndParse() {
    const auto fixture = Fixture();
    const dfbridge::ChannelInfoWriter writer;
    const auto text = writer.Render(fixture);
    assert(text.find("[server]\n17\n") != std::string::npos);
    assert(text.find("186   <4::channel_info_cname_171>   19   `[zombie]`") !=
           std::string::npos);
    assert(text.find("165   <4::channel_info_cname_516>   23   `[raid]`") !=
           std::string::npos);
    assert(text.find("   21   ") == std::string::npos);
    assert(text.find("   22   ") == std::string::npos);

    const dfbridge::ChannelInfoParser parser;
    const auto parsed = parser.Parse(text);
    const auto counts = dfbridge::CountSnapshot(parsed);
    assert(counts.dungeon_groups == 2);
    assert(counts.dungeon_ids == 2);
    assert(counts.channels == 5);
    assert(counts.server_groups == 2);

    const std::string official_form =
        "[dungeon]\n`[zombie]`\n<4::channel_info_dname_18>\n3100\n[/dungeon]\n\n"
        "[server] 17\n"
        "186 `<4::channel_info_cname_171>` 19 `[zombie]` 0 0 0 0 0 0 0 0 0 0 0 ``\n"
        "[/server]\n";
    const auto official = parser.Parse(official_form);
    assert(official.channels.size() == 1);
    assert(official.channels[0].server_group == 17);
    assert(official.channels[0].channel_kind == 19);

    bool truncated_rejected = false;
    try {
        parser.Parse(official_form.substr(0, official_form.find("[/server]")));
    } catch (const std::runtime_error&) {
        truncated_rejected = true;
    }
    assert(truncated_rejected);

    for (const std::int32_t unsupported_kind : {21, 22}) {
        std::string unsupported = official_form;
        const std::size_t kind_offset = unsupported.find(" 19 `[zombie]`");
        assert(kind_offset != std::string::npos);
        unsupported.replace(kind_offset, 4, " " + std::to_string(unsupported_kind) + " ");
        bool kind_rejected = false;
        try {
            parser.Parse(unsupported);
        } catch (const std::runtime_error&) {
            kind_rejected = true;
        }
        assert(kind_rejected);
    }
}

void TestSupportedChannelKinds() {
    for (std::int32_t kind = 0; kind <= 20; ++kind) {
        assert(dfbridge::IsSupportedChannelKind(kind));
    }
    assert(!dfbridge::IsSupportedChannelKind(-1));
    assert(!dfbridge::IsSupportedChannelKind(21));
    assert(!dfbridge::IsSupportedChannelKind(22));
    assert(dfbridge::IsSupportedChannelKind(23));
    assert(!dfbridge::IsSupportedChannelKind(24));
    assert(static_cast<std::int32_t>(dfbridge::ChannelKind::Raid) == 23);

    for (const std::int32_t unsupported_kind : {21, 22}) {
        auto fixture = Fixture();
        fixture.channels[0].channel_kind = unsupported_kind;
        bool rejected = false;
        try {
            dfbridge::ValidateSnapshot(fixture);
        } catch (const std::runtime_error&) {
            rejected = true;
        }
        assert(rejected);

        fixture.channels[0].is_use = false;
        rejected = false;
        try {
            dfbridge::ValidateSnapshot(fixture);
        } catch (const std::runtime_error&) {
            rejected = true;
        }
        assert(rejected);
    }

    dfbridge::ValidateSnapshot(Fixture());
}

void TestWire() {
    const auto version = dfbridge::wire::MakeVersionResponse("60", "60");
    assert(version.size() == sizeof(dfbridge::wire::ScCheckScriptVersion));
    const auto* version_packet =
        reinterpret_cast<const dfbridge::wire::ScCheckScriptVersion*>(version.data());
    assert(version_packet->header.classification == 124);
    assert(version_packet->header.msg_no == 6);
    assert(version_packet->is_valid_version == 1);

    const std::string script = "[server]\n17\n[/server]";
    const auto script_packet = dfbridge::wire::MakeScriptResponse(script);
    assert(script_packet.size() == sizeof(dfbridge::wire::PacketHeader) + script.size());
    const auto* script_header =
        reinterpret_cast<const dfbridge::wire::PacketHeader*>(script_packet.data());
    assert(script_header->msg_no == 10);
    assert(script_header->length == script_packet.size());

    const auto gc_packet = dfbridge::wire::MakeGcInfoResponse({{"siroco", 4000}});
    assert(gc_packet.size() == sizeof(dfbridge::wire::ScGetGcInfo) +
                                   sizeof(dfbridge::wire::ServerGcInfo));
    const auto* gc_header =
        reinterpret_cast<const dfbridge::wire::ScGetGcInfo*>(gc_packet.data());
    assert(gc_header->header.msg_no == 14);
    assert(gc_header->count == 1);

    const auto update =
        dfbridge::wire::MakeChannelUpdate(3186, 0, 186, 1000, 10, "127.0.0.1", 7000);
    assert(update.size() == sizeof(dfbridge::wire::CsUpdateChannelInfo));
    const auto* update_packet =
        reinterpret_cast<const dfbridge::wire::CsUpdateChannelInfo*>(update.data());
    assert(update_packet->header.classification == 123);
    assert(update_packet->header.msg_no == 2);
    assert(update_packet->group == 0);
    assert(update_packet->gc_no == 3186);

    dfbridge::wire::PacketHeader legacy_request =
        dfbridge::wire::MakeHeader(7, dfbridge::wire::kCsCheckScriptVersion,
                                   sizeof(dfbridge::wire::CsCheckScriptVersion));
    assert(dfbridge::wire::ClassifyTcpRequest(legacy_request) ==
           dfbridge::wire::TcpRequestKind::CheckScriptVersion);
    assert(version_packet->header.msg_no == dfbridge::wire::kScCheckScriptVersion);
    legacy_request.msg_no = dfbridge::wire::kCsGetScript;
    legacy_request.length = sizeof(dfbridge::wire::PacketHeader);
    assert(dfbridge::wire::ClassifyTcpRequest(legacy_request) ==
           dfbridge::wire::TcpRequestKind::GetScript);
    assert(script_header->msg_no == dfbridge::wire::kScGetScript);
    legacy_request.msg_no = dfbridge::wire::kCsGetGcInfo;
    assert(dfbridge::wire::ClassifyTcpRequest(legacy_request) ==
           dfbridge::wire::TcpRequestKind::GetGcInfo);
    assert(gc_header->header.msg_no == dfbridge::wire::kScGetGcInfo);

    dfbridge::wire::CsUpdateChannelInfo alternate_category = *update_packet;
    alternate_category.header.classification = 7;
    assert(dfbridge::wire::IsChannelUpdatePacket(
        alternate_category.header, sizeof(alternate_category)));
}

void TestFrameDecoder() {
    const auto first = dfbridge::wire::MakeVersionResponse("60", "60");
    const auto second = dfbridge::wire::MakeScriptResponse("abc");
    dfbridge::wire::FrameDecoder decoder;
    std::vector<std::vector<std::uint8_t>> frames;
    std::string error;
    assert(decoder.Push(first.data(), 5, &frames, &error));
    assert(frames.empty());
    assert(decoder.BufferedSize() == 5);

    std::vector<std::uint8_t> tail(first.begin() + 5, first.end());
    tail.insert(tail.end(), second.begin(), second.end());
    assert(decoder.Push(tail.data(), tail.size(), &frames, &error));
    assert(frames.size() == 2);
    assert(frames[0] == first);
    assert(frames[1] == second);
    assert(decoder.BufferedSize() == 0);

    dfbridge::wire::PacketHeader invalid{};
    invalid.length = 10;
    frames.clear();
    assert(!decoder.Push(reinterpret_cast<const std::uint8_t*>(&invalid), sizeof(invalid),
                         &frames, &error));
    assert(!error.empty());

    dfbridge::wire::PacketHeader old_game =
        dfbridge::wire::MakeHeader(7, dfbridge::wire::kCsCheckScriptVersion,
                                   sizeof(dfbridge::wire::PacketHeader));
    assert(dfbridge::wire::ClassifyTcpRequest(old_game) ==
           dfbridge::wire::TcpRequestKind::CheckScriptVersion);
    old_game.msg_no = 1;
    assert(dfbridge::wire::ClassifyTcpRequest(old_game) ==
           dfbridge::wire::TcpRequestKind::Unknown);
    old_game.msg_no = 11;
    assert(dfbridge::wire::ClassifyTcpRequest(old_game) ==
           dfbridge::wire::TcpRequestKind::Unknown);

    decoder.Reset();
    frames.clear();
    old_game.length = sizeof(old_game);
    assert(decoder.Push(reinterpret_cast<const std::uint8_t*>(&old_game),
                        sizeof(old_game), &frames, &error));
    assert(frames.size() == 1);  // unknown/non-123 is still a valid length frame
}

void TestConfig() {
    const std::string text =
        "[server]\r\n"
        "max_client = 1000 # production ceiling\r\n"
        "this_ip = 192.168.200.131\r\n"
        "this_tcp_port = 7000\r\n"
        "this_udp_port = 7000\r\n"
        "[DB]\r\n"
        "db_ip = 127.0.0.1\r\n"
        "db_name = d_channel\r\n"
        "db_id = game\r\n"
        "db_pwd = \"secret#value\"\r\n";
    const dfbridge::BridgeConfig config =
        dfbridge::BridgeConfigParser().Parse(text, "bridge");
    assert(config.instance_name == "bridge");
    assert(config.max_client == 1000);
    assert(config.tcp_port == 7000);
    assert(config.udp_port == 7000);
    assert(config.db_port == 3306);
    assert(config.db_password == "secret#value");
    assert(dfbridge::IsValidInstanceName("bridge_01"));
    assert(!dfbridge::IsValidInstanceName("../bridge"));

    bool missing_rejected = false;
    try {
        dfbridge::BridgeConfigParser().Parse("[server]\nthis_tcp_port=7000\n", "bridge");
    } catch (const std::runtime_error&) {
        missing_rejected = true;
    }
    assert(missing_rejected);
}

void TestRepositoryWorkerHooksAndReloadCommit() {
    const char* worker_output = "dfbridge_worker_hook_test.etc";
    HookRepository repository;
    dfbridge::ScriptCache cache;
    {
        dfbridge::ScriptThread thread(repository, cache, worker_output,
                                      std::chrono::milliseconds(1));
        thread.Start();
        for (int i = 0; i < 100 && repository.entered.load() == 0; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        thread.Stop();
    }
    assert(repository.entered.load() == 1);
    assert(repository.left.load() == 1);
    assert(repository.active.load() == 0);
    assert(repository.max_active.load() == 1);
    std::remove(worker_output);

    const char* retry_output = "dfbridge_reload_commit_test.etc";
    dfbridge::ScriptCache retry_cache;
    retry_cache.Replace("59", "old", std::vector<dfbridge::GcGroup>());
    dfbridge::ScriptThread retry(
        repository, retry_cache, retry_output, std::chrono::milliseconds(1),
        [](const std::string&, const std::vector<dfbridge::GcGroup>&) {
            throw std::runtime_error("injected reload sink failure");
        });
    bool callback_failed = false;
    try {
        retry.ReloadOnce();
    } catch (const std::runtime_error&) {
        callback_failed = true;
    }
    assert(callback_failed);
    assert(retry_cache.Version() == "59");
    assert(retry_cache.Script() == "old");
    std::remove(retry_output);
}

void TestSemanticPreservesAuthoritativeBytes() {
    const char* output_path = "dfbridge_semantic_preserve_test.etc";
    const std::string authoritative = AuthoritativeInlineCrlfFixture();
    const dfbridge::ChannelInfoParser parser;
    dfbridge::ChannelSnapshot parsed = parser.Parse(authoritative);
    const dfbridge::ChannelSnapshot database = Fixture();
    assert(dfbridge::ChannelSnapshotsSemanticallyEquivalent(parsed, database));

    dfbridge::ChannelRow unsupported;
    unsupported.server_group = 17;
    unsupported.is_use = true;
    unsupported.channel_number = 999;
    unsupported.channel_name = "unsupported";
    unsupported.channel_kind = 21;
    unsupported.kind_name = "unsupported";
    parsed.channels.push_back(unsupported);
    assert(!dfbridge::ChannelSnapshotsSemanticallyEquivalent(parsed, database));
    parsed.channels.back().channel_kind = 22;
    assert(!dfbridge::ChannelSnapshotsSemanticallyEquivalent(parsed, database));
    parsed.channels.pop_back();

    WriteBinaryFile(output_path, authoritative);
    HookRepository repository;
    dfbridge::ScriptCache cache;
    dfbridge::ScriptThread preserve(repository, cache, output_path,
                                    std::chrono::milliseconds(1));
    preserve.ReloadOnce();
    assert(dfbridge::ReadChannelInfoFile(output_path) == authoritative);
    assert(cache.Script() == authoritative);
    assert(cache.Version() == "60");
    assert(cache.GcGroups().size() == database.gc_groups.size());

    repository.snapshot.channels[0].exp_rate = 1.0;
    preserve.ReloadOnce();
    const std::string rewritten = dfbridge::ReadChannelInfoFile(output_path);
    assert(rewritten != authoritative);
    assert(dfbridge::ChannelSnapshotsSemanticallyEquivalent(
        parser.Parse(rewritten), repository.snapshot));
    assert(cache.Script() == rewritten);
    std::remove(output_path);
}

void TestRegistry() {
    std::vector<dfbridge::GcGroup> groups;
    dfbridge::GcGroup siroco;
    siroco.group_name = "siroco";
    siroco.upper_gc_no = 4000;
    groups.push_back(siroco);
    dfbridge::ChannelRegistry registry(groups);
    std::size_t group = 99;
    assert(registry.ResolveGcGroupIndex(3186, &group));
    assert(group == 0);

    dfbridge::wire::CsUpdateChannelInfo update{};
    update.gc_no = 3186;
    update.group = 17;  // Original bridge intentionally resolves by gc_no instead.
    update.channel_no = 186;
    update.max_user_num = 1000;
    update.cur_user_num = 1;
    std::strcpy(update.server_ip, "127.0.0.1");
    update.port = 7000;
    assert(registry.UpdateGameServer(update));
    assert(registry.GameServerCount(0) == 1);
    assert(registry.GameServers().size() == 1);

    registry.SetGcGroups({{"siroco", 4000}});
    assert(registry.GameServerCount(0) == 1);  // identical map preserves state
    registry.SetGcGroups({{"renamed-siroco", 4000}});
    assert(registry.GameServerCount(0) == 0);  // name change invalidates index map
    assert(registry.UpdateGameServer(update));
    registry.SetGcGroups({{"renamed-siroco", 4100}});
    assert(registry.GameServerCount(0) == 0);  // upper boundary change clears state
    assert(registry.UpdateGameServer(update));

    dfbridge::ChannelRegistry reordered(
        {{"later", 5000}, {"siroco", 4000}});
    assert(reordered.UpdateGameServer(update));
    reordered.SetGcGroups({{"siroco", 4000}, {"later", 5000}});
    assert(reordered.GameServerCount(0) == 1);  // input order alone is not a change

    dfbridge::wire::CsNoticeChannelServer notice{};
    notice.id = 1;
    notice.port = 7001;
    std::strcpy(notice.server_ip, "127.0.0.1");
    assert(registry.RegisterChannelServer(notice));
    assert(registry.ChannelServerCount() == 1);

    const auto future = std::chrono::system_clock::now() + std::chrono::seconds(21);
    const auto game_expired = registry.ExpireStale(future);
    assert(game_expired.game_servers == 1);
    assert(game_expired.channel_servers == 0);
    assert(registry.GameServerCount(0) == 0);

    const auto far_future = std::chrono::system_clock::now() + std::chrono::seconds(201);
    const auto channel_expired = registry.ExpireStale(far_future);
    assert(channel_expired.channel_servers == 1);
    assert(registry.ChannelServerCount() == 0);
}

}  // namespace

int main() {
    TestRenderAndParse();
    TestSupportedChannelKinds();
    TestWire();
    TestFrameDecoder();
    TestConfig();
    TestRepositoryWorkerHooksAndReloadCommit();
    TestSemanticPreservesAuthoritativeBytes();
    TestRegistry();
    assert(dfbridge::RestoredProtocolTable().size() == 5);
    std::cout << "dfbridge tests passed\n";
    return 0;
}
