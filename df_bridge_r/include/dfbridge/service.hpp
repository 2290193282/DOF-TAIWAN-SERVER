#pragma once

#include <string>

namespace dfbridge {

bool IsServiceCommand(const std::string& command);
int RunServiceCommand(const std::string& instance_name,
                      const std::string& command);

}  // namespace dfbridge
