#pragma once

#include <cstdint>
#include <string>

namespace dfbridge {

struct BridgeConfig {
    std::string instance_name;
    std::int32_t max_client{1000};
    std::string this_ip{"0.0.0.0"};
    std::uint16_t tcp_port{};
    std::uint16_t udp_port{};

    std::string db_host;
    std::uint16_t db_port{3306};
    std::string db_name{"d_channel"};
    std::string db_user;
    std::string db_password;
};

class BridgeConfigParser {
public:
    BridgeConfig Parse(const std::string& text, const std::string& instance_name) const;
    BridgeConfig ParseFile(const std::string& path,
                           const std::string& instance_name) const;
};

bool IsValidInstanceName(const std::string& value);

}  // namespace dfbridge
