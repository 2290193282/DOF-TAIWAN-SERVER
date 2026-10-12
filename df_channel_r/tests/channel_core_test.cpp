#include "channel/channel_client.hpp"
#include "channel/channel_service.hpp"
#include "channel/daemon_config.hpp"
#include "channel/legacy_codec.hpp"
#include "channel/protocol.hpp"
#include "channel/script_store.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <zlib.h>

using namespace dnf::channel;

namespace {

Bytes TextBytes(const std::string& text) { return Bytes(text.begin(), text.end()); }

void WriteText(const std::string& path, const std::string& text) {
    std::ofstream output(path.c_str(), std::ios::binary | std::ios::trunc);
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    assert(output.good());
}

bool FilePresent(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::binary);
    return static_cast<bool>(input);
}

template <std::size_t N>
void CopyFixed(char (&destination)[N], const std::string& source) {
    std::memset(destination, 0, N);
    std::memcpy(destination, source.data(), source.size() < N - 1 ? source.size() : N - 1);
}

Bytes DecodeCompressedGameReply(const Bytes& frame, const LegacyCodec& codec) {
    assert(ValidateFrame(frame));
    assert(frame.size() > sizeof(PacketHeader));
    Bytes inflated(0x40000U, 0);
    uLongf inflated_size = static_cast<uLongf>(inflated.size());
    const int zlib_result = uncompress(
        inflated.data(), &inflated_size, frame.data() + sizeof(PacketHeader),
        static_cast<uLong>(frame.size() - sizeof(PacketHeader)));
    assert(zlib_result == Z_OK);
    inflated.resize(static_cast<std::size_t>(inflated_size));
    assert(!inflated.empty() && inflated.size() % 16 == 0);
    Bytes plaintext;
    std::string error;
    assert(codec.Decrypt(inflated.data(), inflated.size(), &plaintext, &error));
    return plaintext;
}

}  // namespace

int main(int argc, char** argv) {
    assert(sizeof(PacketHeader) == 11);
    assert(sizeof(CsUpdateChannelInfo) == 51);
    assert(!IsPvpChannel(ChannelType::Common));
    assert(!IsPvpChannel(ChannelType::TradeAuction));
    assert(!IsPvpChannel(ChannelType::SpecialDungeon));
    assert(!IsPvpChannel(ChannelType::Report));
    assert(!IsPvpChannel(ChannelType::Zombie));
    assert(IsPvpChannel(ChannelType::BroadcastPvp));
    assert(IsPvpChannel(ChannelType::IntegratedPvp));
    assert(IsPvpChannel(ChannelType::FreePractice));
    assert(IsPvpChannel(ChannelType::LeagueQualifier));
    assert(IsPvpChannel(ChannelType::RankedPvp));
    assert(IsPvpChannel(ChannelType::FreePvp));
    assert(IsPvpChannel(ChannelType::Tournament));
    assert(!IsIntegratedPvpChannel(ChannelType::Tournament));
    assert(!IsPvpChannel(ChannelType::Raid));
    assert(!IsFreePvpChannel(ChannelType::Raid));
    assert(!IsSameTeamJoinChannel(ChannelType::Raid));
    assert(!IsIntegratedPvpChannel(ChannelType::Raid));
    for (std::int32_t type = 0; type <= 20; ++type) {
        assert(IsSupportedChannelType(type));
    }
    assert(!IsSupportedChannelType(-1));
    assert(!IsSupportedChannelType(21));
    assert(!IsSupportedChannelType(22));
    assert(IsSupportedChannelType(23));
    assert(!IsSupportedChannelType(24));
    assert(static_cast<std::int32_t>(ChannelType::Raid) == 23);

    const std::string table =
        "[dungeon]\n"
        "`[test]`\n"
        "<4::channel_info_dname_0>\n"
        "1\n"
        "[/dungeon]\n"
        "[server]\n17\n"
        "  186 <4::channel_info_cname_186> 19 `[none]` 0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n"
        "  164 <4::channel_info_cname_164> 20 `[none]` 0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n"
        "  165 <4::channel_info_cname_165> 23 `[raid]` 0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n"
        "  6 <4::channel_info_cname_6> 3 `[none]` 0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n"
        "[/server]\n";
    std::vector<ChannelTableRow> rows;
    std::string error;
    assert(ScriptStore::ParseChannelTable(TextBytes(table), &rows, &error));
    assert(rows.size() == 4 && rows[0].server_group == 17 && rows[0].channel_type == 19 &&
           rows[1].channel_type == 20 && rows[2].channel_type == 23);
    const std::string bridge_writer_fixture =
        "[server]\n17\n"
        "  186 <4::channel_info_cname_186> 19 `[none]` 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 ``\n"
        "[/server]\n";
    rows.clear();
    assert(ScriptStore::ParseChannelTable(TextBytes(bridge_writer_fixture), &rows, &error));
    assert(rows.size() == 1 && rows[0].server_group == 17 && rows[0].channel_no == 186 &&
           rows[0].channel_type == 19);
    ChannelScriptStats table_stats;
    rows.clear();
    assert(ScriptStore::ValidateChannelScript(TextBytes(table), &rows, &table_stats, &error));
    assert(table_stats.dungeon_groups == 1 && table_stats.dungeon_mappings == 1 &&
           table_stats.channel_rows == 4);
    const std::string missing_close = table.substr(0, table.find("[/server]"));
    rows.clear();
    assert(!ScriptStore::ParseChannelTable(TextBytes(missing_close), &rows, &error));
    const std::string short_row =
        "[server] 17\n186 <4::channel_info_cname_186> 19 `[zombie]` 0 0.0\n[/server]\n";
    rows.clear();
    assert(!ScriptStore::ParseChannelTable(TextBytes(short_row), &rows, &error));

    const std::string unsupported_21 =
        "[server] 17\n186 <4::channel_info_cname_186> 21 `[invalid]` 0 0 0 0 0 0 0 0 0 0 0\n[/server]\n";
    rows.clear();
    assert(!ScriptStore::ParseChannelTable(TextBytes(unsupported_21), &rows, &error));
    const std::string unsupported_22 =
        "[server] 17\n186 <4::channel_info_cname_186> 22 `[reserved]` 0 0 0 0 0 0 0 0 0 0 0\n[/server]\n";
    rows.clear();
    assert(!ScriptStore::ParseChannelTable(TextBytes(unsupported_22), &rows, &error));

    if (argc > 1) {
        std::ifstream input(argv[1], std::ios::binary);
        Bytes official((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        rows.clear();
        ChannelScriptStats official_stats;
        assert(ScriptStore::ValidateChannelScript(official, &rows, &official_stats, &error));
        assert(official_stats.dungeon_groups == 21);
        assert(official_stats.dungeon_mappings == 130);
        assert(official_stats.channel_rows == 475 && rows.size() == 475);
        bool zombie186 = false;
        bool zombie187 = false;
        bool zombie188 = false;
        for (std::size_t index = 0; index < rows.size(); ++index) {
            if (rows[index].channel_type == 19) {
                if (rows[index].channel_no == 186) zombie186 = true;
                if (rows[index].channel_no == 187) zombie187 = true;
                if (rows[index].channel_no == 188) zombie188 = true;
            }
        }
        assert(zombie186 && zombie187 && zombie188);
    }

    ChannelRegistry registry;
    ScGetGcInfo gc_header{};
    gc_header.header = MakeHeader(Category::ServerToClient, MessageId::GetGcInfoReply,
                                  sizeof(ScGetGcInfo) + sizeof(ServerGcInfoWire), true);
    gc_header.count = 1;
    ServerGcInfoWire gc{};
    CopyFixed(gc.server_group_name, "cain");
    gc.gc_no = 4000;
    Bytes gc_frame = PodFrame(gc_header);
    AppendPod(gc_frame, gc);
    assert(registry.ApplyGcInfoReply(gc_frame, &error));

    ChannelRegistry invalid_gc_registry;
    ScGetGcInfo empty_gc{};
    empty_gc.header = MakeHeader(Category::ServerToClient, MessageId::GetGcInfoReply,
                                 sizeof(empty_gc), true);
    empty_gc.count = 0;
    assert(!invalid_gc_registry.ApplyGcInfoReply(PodFrame(empty_gc), &error));
    ScGetGcInfo bad_name_header{};
    bad_name_header.header = MakeHeader(Category::ServerToClient, MessageId::GetGcInfoReply,
                                        sizeof(bad_name_header) + sizeof(ServerGcInfoWire), true);
    bad_name_header.count = 1;
    ServerGcInfoWire unnamed_gc{};
    unnamed_gc.gc_no = 4000;
    Bytes unnamed_frame = PodFrame(bad_name_header);
    AppendPod(unnamed_frame, unnamed_gc);
    assert(!invalid_gc_registry.ApplyGcInfoReply(unnamed_frame, &error));
    CopyFixed(unnamed_gc.server_group_name, "nonpositive");
    unnamed_gc.gc_no = 0;
    Bytes nonpositive_frame = PodFrame(bad_name_header);
    AppendPod(nonpositive_frame, unnamed_gc);
    assert(!invalid_gc_registry.ApplyGcInfoReply(nonpositive_frame, &error));
    ScGetGcInfo unordered_header{};
    unordered_header.header = MakeHeader(Category::ServerToClient, MessageId::GetGcInfoReply,
                                         sizeof(unordered_header) + 2 * sizeof(ServerGcInfoWire), true);
    unordered_header.count = 2;
    ServerGcInfoWire upper_a{};
    ServerGcInfoWire upper_b{};
    CopyFixed(upper_a.server_group_name, "a");
    CopyFixed(upper_b.server_group_name, "b");
    upper_a.gc_no = 4000;
    upper_b.gc_no = 4000;
    Bytes duplicate_frame = PodFrame(unordered_header);
    AppendPod(duplicate_frame, upper_a);
    AppendPod(duplicate_frame, upper_b);
    assert(invalid_gc_registry.ApplyGcInfoReply(duplicate_frame, &error));
    upper_b.gc_no = 3000;
    Bytes unordered_frame = PodFrame(unordered_header);
    AppendPod(unordered_frame, upper_a);
    AppendPod(unordered_frame, upper_b);
    assert(invalid_gc_registry.ApplyGcInfoReply(unordered_frame, &error));

    ChannelRegistry varchar_order_registry;
    const int varchar_upper_gc[] = {10000, 100000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000};
    ScGetGcInfo varchar_header{};
    varchar_header.header = MakeHeader(
        Category::ServerToClient, MessageId::GetGcInfoReply,
        sizeof(varchar_header) + sizeof(varchar_upper_gc) / sizeof(varchar_upper_gc[0]) *
                                     sizeof(ServerGcInfoWire),
        true);
    varchar_header.count = static_cast<std::int32_t>(
        sizeof(varchar_upper_gc) / sizeof(varchar_upper_gc[0]));
    Bytes varchar_frame = PodFrame(varchar_header);
    for (std::size_t index = 0; index < sizeof(varchar_upper_gc) / sizeof(varchar_upper_gc[0]); ++index) {
        ServerGcInfoWire item{};
        CopyFixed(item.server_group_name, "g" + std::to_string(varchar_upper_gc[index]));
        item.gc_no = varchar_upper_gc[index];
        AppendPod(varchar_frame, item);
    }
    assert(varchar_order_registry.ApplyGcInfoReply(varchar_frame, &error));
    // Deterministic regression fixture only.  Production values are never
    // compiled into the daemon: it receives this packet from Game through
    // Bridge, while the GC boundaries/names above come from d_channel SC14.
    CsUpdateChannelInfo varchar_update{};
    varchar_update.header = MakeHeader(Category::ClientToServer, MessageId::UpdateChannelInfo,
                                       sizeof(varchar_update));
    varchar_update.gc_no = 3186;
    varchar_update.group = 17;
    varchar_update.channel_no = 186;
    varchar_update.max_user_num = 1000;
    varchar_update.cur_user_num = 1;
    CopyFixed(varchar_update.server_ip, "127.0.0.1");
    varchar_update.port = 10186;
    assert(varchar_order_registry.ApplyChannelUpdate(varchar_update, &error));
    const std::vector<ChannelGroupSnapshot> varchar_snapshot = varchar_order_registry.Snapshot();
    assert(varchar_snapshot.size() == 1 && varchar_snapshot[0].name == "g4000" &&
           varchar_snapshot[0].channels.size() == 1 &&
           varchar_snapshot[0].channels[0].gc_no == 3186);

    CsUpdateChannelInfo update{};
    update.header = MakeHeader(Category::ClientToServer, MessageId::UpdateChannelInfo, sizeof(update));
    update.gc_no = 3186;
    update.group = 17;
    update.channel_no = 186;
    update.max_user_num = 400000;
    update.cur_user_num = 1;
    CopyFixed(update.server_ip, "127.0.0.1");
    update.port = 7000;
    ChannelRegistry pending_registry;
    assert(!pending_registry.ApplyChannelUpdate(update, &error));
    assert(pending_registry.ApplyGcInfoReply(gc_frame, &error));
    assert(pending_registry.ApplyChannelUpdate(update, &error));
    assert(pending_registry.Snapshot().size() == 1);
    ChannelRegistry compatibility_registry(true);
    assert(compatibility_registry.ApplyChannelUpdate(update, &error));
    assert(compatibility_registry.Snapshot().empty());
    assert(compatibility_registry.ApplyGcInfoReply(gc_frame, &error));
    assert(compatibility_registry.Snapshot().size() == 1);
    assert(registry.ApplyChannelUpdate(update, &error));
    assert(registry.Snapshot().size() == 1);
    assert(registry.SerializeAskChannelBody().size() == sizeof(ServerGroupInfoWire) + sizeof(ServerInfoWire));
    const std::vector<ChannelGroupSnapshot> fixed_snapshot = registry.Snapshot();
    update.gc_no = 3187;
    update.channel_no = 187;
    assert(registry.ApplyChannelUpdate(update, &error));
    const Bytes fixed_body = ChannelRegistry::SerializeSnapshot(fixed_snapshot);
    assert(fixed_snapshot.size() == 1 && fixed_snapshot[0].channels.size() == 1);
    assert(fixed_body.size() == sizeof(ServerGroupInfoWire) + sizeof(ServerInfoWire));
    assert(registry.SerializeAskChannelBody().size() ==
           sizeof(ServerGroupInfoWire) + 2 * sizeof(ServerInfoWire));
    update.gc_no = 3186;
    update.channel_no = 186;

    ChannelRegistry expiry_registry;
    assert(expiry_registry.ApplyGcInfoReply(gc_frame, &error));
    assert(expiry_registry.ApplyChannelUpdateAt(update, 100, &error));
    assert(expiry_registry.ExpireStale(140, 40) == 0);
    assert(expiry_registry.ExpireStale(141, 40) == 1);
    assert(expiry_registry.Snapshot().empty());

    const std::array<std::uint8_t, 32> key = LegacyCodec::TimestampKey(1700000000);
    LegacyCodec codec(key);
    const Bytes plain = TextBytes("dnf_game");
    Bytes encrypted;
    Bytes decrypted;
    assert(codec.Encrypt(plain, &encrypted, &error));
    assert(encrypted.size() == 16);
    assert(codec.Decrypt(encrypted.data(), encrypted.size(), &decrypted, &error));
    assert(std::memcmp(decrypted.data(), plain.data(), plain.size()) == 0);

    const std::string script_path = "channel_core_test.etc";
    const std::string version_path = "channel_core_test.version";
    WriteText(script_path, table);
    WriteText(version_path, "0627\n");
    ScriptStore scripts(script_path, version_path);
    assert(scripts.Reload(&error));

    BridgeIdentity identity;
    identity.id = 1;
    identity.advertised_ip = "127.0.0.1";
    identity.udp_port = 7200;
    BridgeProtocolClient bridge(scripts, registry, identity);
    const std::vector<Bytes> initial = bridge.BuildInitialFrames();
    assert(initial.size() == 3 && initial[0].size() == 11 && initial[1].size() == 27 && initial[2].size() == 35);
    update.cur_user_num = 2;
    BridgeHandleResult forwarded = bridge.HandleFrame(PodFrame(update));
    assert(forwarded.accepted && !forwarded.gc_ready && !forwarded.error.empty());
    BridgeHandleResult gc_ready = bridge.HandleFrame(gc_frame);
    assert(gc_ready.accepted && gc_ready.gc_ready);
    forwarded = bridge.HandleFrame(PodFrame(update));
    assert(forwarded.accepted && forwarded.error.empty());

    ScCheckScriptVersion bridge_version{};
    bridge_version.header = MakeHeader(Category::ServerToClient, MessageId::CheckScriptVersionReply,
                                       sizeof(bridge_version), true);
    bridge_version.is_valid_version = 0;
    CopyFixed(bridge_version.channel_script_version, "0828");
    BridgeHandleResult bridge_result = bridge.HandleFrame(PodFrame(bridge_version));
    assert(bridge_result.accepted && bridge_result.outbound.size() == 1 && bridge_result.outbound[0].size() == 11);
    Bytes bridge_script = PodFrame(MakeHeader(Category::ServerToClient, MessageId::GetScriptReply,
                                              sizeof(PacketHeader) + table.size(), true));
    AppendBytes(bridge_script, table.data(), table.size());
    bridge_result = bridge.HandleFrame(bridge_script);
    assert(bridge_result.accepted && scripts.Snapshot().version == "0828");

    ChannelService service(scripts, registry, codec);
    ChannelSession session(service);
    const Bytes ask = PodFrame(MakeHeader(Category::ClientToServer, MessageId::AskChannelInfo,
                                          sizeof(PacketHeader)));
    HandleResult handled = session.HandleGameFrame(ask);
    assert(handled.accepted && handled.has_response && !session.authenticated());
    CsConnect initial_connect{};
    initial_connect.header = MakeHeader(Category::ClientToServer, MessageId::Connect,
                                        sizeof(initial_connect));
    initial_connect.header.classification = 0;
    handled = session.HandleGameFrame(PodFrame(initial_connect));
    assert(handled.accepted && handled.has_response && !session.authenticated());
    ScConnect initial_reply{};
    assert(ReadPod(handled.response, &initial_reply) && initial_reply.is_valid_key == 0);
    assert(initial_reply.header.classification ==
           static_cast<std::uint8_t>(Category::ServerToClient));
    assert(initial_reply.header.message_id ==
           static_cast<std::uint8_t>(MessageId::ConnectReply));
    assert(initial_reply.header.size == sizeof(ScConnect) && initial_reply.header.ack == 1);

    PacketHeader get_script_header = MakeHeader(Category::ClientToServer, MessageId::GetScript,
                                                sizeof(PacketHeader));
    get_script_header.classification = 0;
    handled = session.HandleGameFrame(PodFrame(get_script_header));
    assert(handled.accepted && handled.has_response && !session.authenticated());

    std::array<std::uint8_t, 32> negotiated_key{};
    std::memcpy(negotiated_key.data(), initial_reply.key, negotiated_key.size());
    LegacyCodec negotiated_codec(negotiated_key);
    Bytes connect_plaintext(32, 0xA5);
    std::copy(plain.begin(), plain.end(), connect_plaintext.begin());
    Bytes connect_ciphertext;
    assert(negotiated_codec.Encrypt(connect_plaintext, &connect_ciphertext, &error));
    assert(connect_ciphertext.size() == 32);
    PacketHeader connect_header = MakeHeader(Category::ClientToServer, MessageId::Connect,
                                              sizeof(PacketHeader) + connect_ciphertext.size());
    connect_header.classification = 0;
    Bytes connect = PodFrame(connect_header);
    AppendBytes(connect, connect_ciphertext.data(), connect_ciphertext.size());
    handled = session.HandleGameFrame(connect);
    assert(handled.accepted && handled.has_response && handled.response.size() == sizeof(ScConnect));
    ScConnect connect_reply{};
    assert(ReadPod(handled.response, &connect_reply) && connect_reply.is_valid_key == 1);
    assert(connect_reply.header.classification ==
           static_cast<std::uint8_t>(Category::ServerToClient));
    assert(connect_reply.header.message_id ==
           static_cast<std::uint8_t>(MessageId::ConnectReply));
    assert(connect_reply.header.size == sizeof(ScConnect) && connect_reply.header.ack == 1);
    assert(session.authenticated());

    CsGetServerGroupChannelInfo group_request{};
    group_request.header = MakeHeader(Category::ClientToServer,
                                      MessageId::GetServerGroupChannelInfo,
                                      sizeof(group_request));
    // The real client header constructor uses classification zero on this
    // direct Channel-service socket.
    group_request.header.classification = 0;
    CopyFixed(group_request.server_group_name, "cain");
    group_request.parser_server_group = 1;
    handled = session.HandleGameFrame(PodFrame(group_request));
    assert(handled.accepted && handled.has_response);
    PacketHeader group_response_header{};
    assert(ReadHeader(handled.response, &group_response_header));
    assert(group_response_header.classification ==
           static_cast<std::uint8_t>(Category::ServerToClient));
    assert(group_response_header.message_id ==
           static_cast<std::uint8_t>(MessageId::GetServerGroupChannelInfoReply));
    assert(group_response_header.ack == 1);
    const Bytes group_plaintext = DecodeCompressedGameReply(handled.response, codec);
    ScServerGroupChannelInfoPlainHeader group_plain_header{};
    assert(ReadPod(group_plaintext, &group_plain_header));
    assert(group_plain_header.fallback_server_group == 1);
    assert(group_plain_header.channel_count == 2);
    ServerInfoWire group_channel_186{};
    ServerInfoWire group_channel_187{};
    assert(ReadPod(group_plaintext, &group_channel_186,
                   sizeof(ScServerGroupChannelInfoPlainHeader)));
    assert(ReadPod(group_plaintext, &group_channel_187,
                   sizeof(ScServerGroupChannelInfoPlainHeader) + sizeof(ServerInfoWire)));
    assert(FixedString(group_channel_186.channel_name, sizeof(group_channel_186.channel_name)) ==
           "#ch.186");
    assert(FixedString(group_channel_187.channel_name, sizeof(group_channel_187.channel_name)) ==
           "#ch.187");
    assert(group_channel_186.max_user_num == 400000 && group_channel_186.cur_user_num == 2 &&
           FixedString(group_channel_186.server_ip, sizeof(group_channel_186.server_ip)) ==
               "127.0.0.1" &&
           group_channel_186.port == 7000);
    const std::size_t logical_group_body_size =
        sizeof(ScServerGroupChannelInfoPlainHeader) + 2 * sizeof(ServerInfoWire);
    assert(group_plaintext.size() == 112);
    for (std::size_t index = logical_group_body_size; index < group_plaintext.size(); ++index) {
        assert(group_plaintext[index] == 0);
    }

    CsGetServerGroupChannelInfo max_sentinel_request = group_request;
    CopyFixed(max_sentinel_request.server_group_name, "SERVER_GROUP_TYPE_MAX");
    max_sentinel_request.parser_server_group = 265;
    handled = session.HandleGameFrame(PodFrame(max_sentinel_request));
    assert(!handled.accepted && !handled.has_response &&
           handled.error == "invalid parser server group");

    CsGetServerGroupChannelInfo mismatched_name_request = group_request;
    CopyFixed(mismatched_name_request.server_group_name, "diregie");
    handled = session.HandleGameFrame(PodFrame(mismatched_name_request));
    assert(!handled.accepted && !handled.has_response &&
           handled.error == "requested server group is unavailable");

    CsGetServerGroupChannelInfo unterminated_name_request = group_request;
    std::memset(unterminated_name_request.server_group_name, 'x',
                sizeof(unterminated_name_request.server_group_name));
    handled = session.HandleGameFrame(PodFrame(unterminated_name_request));
    assert(!handled.accepted && !handled.has_response &&
           handled.error == "unterminated server group name");

    Bytes real_client_ask = ask;
    real_client_ask[0] = 0;
    handled = session.HandleGameFrame(real_client_ask);
    assert(handled.accepted && handled.has_response && ValidateFrame(handled.response));

    const std::string daemon_config_path = "channel_core_test.cfg";
    WriteText(daemon_config_path,
              "[server]\n"
              "id = 7\n"
              "max_client = 1000\n"
              "this_ip = 127.0.0.1\n"
              "this_tcp_port = 7000\n"
              "this_udp_port = 7001\n"
              "bridge_ip = 127.0.0.1\n"
              "bridge_port = 7100\n"
              "allow_pending_before_gc = no\n");
    DaemonConfig daemon_config;
    assert(LoadDaemonConfig(daemon_config_path, &daemon_config, &error));
    assert(daemon_config.id == 7 && daemon_config.max_client == 1000 &&
           daemon_config.this_tcp_port == 7000 && daemon_config.this_udp_port == 7001 &&
           daemon_config.bridge_port == 7100 && !daemon_config.allow_pending_before_gc);
    assert(LegacyConfigPath("channel") == "./cfg/channel.cfg");
    assert(LegacyPidPath("channel") == "./pid/channel.pid");
    WriteText(daemon_config_path,
              "[server]\n"
              "id = 7\nmax_client = 1000\nthis_ip = 1234567890123456\n"
              "this_tcp_port = 7000\nthis_udp_port = 7001\n"
              "bridge_ip = 127.0.0.1\nbridge_port = 7100\n");
    assert(!LoadDaemonConfig(daemon_config_path, &daemon_config, &error));

    const std::string recovery_script = "channel_recovery_test.etc";
    const std::string recovery_version = "channel_recovery_test.version";
    WriteText(recovery_script, "broken channel script\n");
    WriteText(recovery_version, "\n");
    WriteText(recovery_script + ".rollback", table);
    WriteText(recovery_version + ".rollback", "58\n");
    WriteText(recovery_script + ".new", "incomplete new script");
    WriteText(recovery_version + ".new", "59\n");
    ScriptStore recovery_store(recovery_script, recovery_version);
    assert(recovery_store.Reload(&error));
    assert(recovery_store.Snapshot().version == "58");
    assert(FilePresent(recovery_script) && FilePresent(recovery_version));
    assert(!FilePresent(recovery_script + ".rollback") &&
           !FilePresent(recovery_version + ".rollback") &&
           !FilePresent(recovery_script + ".new") && !FilePresent(recovery_version + ".new"));

    std::remove(recovery_script.c_str());
    std::remove(recovery_version.c_str());
    WriteText(recovery_script + ".rollback", table);
    WriteText(recovery_version + ".rollback", "57\n");
    ScriptStore missing_pair_store(recovery_script, recovery_version);
    assert(missing_pair_store.Reload(&error));
    assert(missing_pair_store.Snapshot().version == "57");
    assert(FilePresent(recovery_script) && FilePresent(recovery_version));
    assert(!FilePresent(recovery_script + ".rollback") &&
           !FilePresent(recovery_version + ".rollback"));

    // Crash after moving only the old script: the old version is still the
    // main file and a single script rollback must reconstruct the same pair.
    std::remove(recovery_script.c_str());
    std::remove(recovery_version.c_str());
    WriteText(recovery_version, "56\n");
    WriteText(recovery_script + ".rollback", table);
    ScriptStore single_rollback_store(recovery_script, recovery_version);
    assert(single_rollback_store.Reload(&error));
    assert(single_rollback_store.Snapshot().version == "56");
    assert(FilePresent(recovery_script) && FilePresent(recovery_version));
    assert(!FilePresent(recovery_script + ".rollback"));

    std::remove(script_path.c_str());
    std::remove(version_path.c_str());
    std::remove(daemon_config_path.c_str());
    std::remove(recovery_script.c_str());
    std::remove(recovery_version.c_str());
    std::cout << "channel_core_test: PASS\n";
    return 0;
}
