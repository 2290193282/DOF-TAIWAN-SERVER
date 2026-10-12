#pragma once

#include "dfbridge/channel_model.hpp"

#include <memory>
#include <string>
#include <utility>

namespace dfbridge {

class IDChannelRepository {
public:
    virtual ~IDChannelRepository() = default;
    // Called exactly once on the background ScriptThread before/after use.
    // Implementations without per-thread runtime state may keep the defaults.
    virtual void WorkerEnter() {}
    virtual void WorkerLeave() noexcept {}
    virtual ChannelSnapshot LoadSnapshot() = 0;
    virtual std::string ReadScriptVersion() = 0;
};

#ifdef DFBRIDGE_WITH_MYSQL
struct MysqlConfig {
    std::string host;
    std::uint16_t port{3306};
    std::string database{"d_channel"};
    std::string user;
    std::string password;

    MysqlConfig() {}
    MysqlConfig(std::string host_value, std::uint16_t port_value,
                std::string database_value, std::string user_value,
                std::string password_value)
        : host(std::move(host_value)),
          port(port_value),
          database(std::move(database_value)),
          user(std::move(user_value)),
          password(std::move(password_value)) {}
};

std::unique_ptr<IDChannelRepository> MakeMysqlRepository(MysqlConfig config);
#endif

}  // namespace dfbridge
