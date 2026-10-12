#pragma once

#include <string>

namespace dnf { namespace channel {

struct DaemonConfig {
    DaemonConfig();

    int id;
    int max_client;
    std::string this_ip;
    int this_tcp_port;
    int this_udp_port;
    std::string bridge_ip;
    int bridge_port;

    // Off by default to match the 0828 daemon.  When enabled, an early game
    // update is retained and replayed after the authoritative GC table arrives.
    bool allow_pending_before_gc;
};

bool LoadDaemonConfig(const std::string& path, DaemonConfig* output,
                      std::string* error = nullptr);
std::string LegacyConfigPath(const std::string& service_name);
std::string LegacyPidPath(const std::string& service_name);

} }  // namespace dnf::channel
