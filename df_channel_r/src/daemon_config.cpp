#include "channel/daemon_config.hpp"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

namespace dnf { namespace channel {
namespace {

std::string Trim(std::string value) {
    const std::string whitespace = " \t\r\n";
    const std::string::size_type first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return std::string();
    const std::string::size_type last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

std::string Unquote(std::string value) {
    value = Trim(value);
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '`' && value.back() == '`'))) {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

bool ParseInt(const std::string& text, int minimum, int maximum, int* output) {
    if (!output) return false;
    errno = 0;
    char* end = nullptr;
    const long value = std::strtol(text.c_str(), &end, 10);
    if (errno != 0 || end == text.c_str() || *end != '\0' || value < minimum || value > maximum)
        return false;
    *output = static_cast<int>(value);
    return true;
}

bool ParseBool(const std::string& text, bool* output) {
    if (!output) return false;
    std::string lowered = text;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lowered == "1" || lowered == "true" || lowered == "yes" || lowered == "on") {
        *output = true;
        return true;
    }
    if (lowered == "0" || lowered == "false" || lowered == "no" || lowered == "off") {
        *output = false;
        return true;
    }
    return false;
}

}  // namespace

DaemonConfig::DaemonConfig()
    : id(-1), max_client(0), this_tcp_port(0), this_udp_port(0), bridge_port(0),
      allow_pending_before_gc(false) {}

bool LoadDaemonConfig(const std::string& path, DaemonConfig* output, std::string* error) {
    if (!output) return false;
    std::ifstream input(path.c_str());
    if (!input) {
        if (error) *error = "cannot open " + path;
        return false;
    }

    std::map<std::string, std::string> values;
    std::string section;
    std::string line;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        const std::string::size_type slash = line.find("//");
        const std::string::size_type hash = line.find('#');
        std::string::size_type comment = std::string::npos;
        if (slash != std::string::npos) comment = slash;
        if (hash != std::string::npos && (comment == std::string::npos || hash < comment)) comment = hash;
        if (comment != std::string::npos) line.erase(comment);
        line = Trim(line);
        if (line.empty() || line.front() == ';') continue;
        if (line.front() == '[' && line.back() == ']') {
            section = Trim(line.substr(1, line.size() - 2));
            std::transform(section.begin(), section.end(), section.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            continue;
        }
        if (!section.empty() && section != "server") continue;

        std::string key;
        std::string value;
        const std::string::size_type equals = line.find('=');
        if (equals != std::string::npos) {
            key = Trim(line.substr(0, equals));
            value = Trim(line.substr(equals + 1));
        } else {
            std::istringstream tokens(line);
            tokens >> key;
            std::getline(tokens, value);
            value = Trim(value);
        }
        if (key.empty() || value.empty()) {
            if (error) *error = "invalid config line " + std::to_string(line_number);
            return false;
        }
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        values[key] = Unquote(value);
    }

    const char* required[] = {"id", "max_client", "this_ip", "this_tcp_port",
                              "this_udp_port", "bridge_ip", "bridge_port"};
    for (std::size_t index = 0; index < sizeof(required) / sizeof(required[0]); ++index) {
        if (!values.count(required[index])) {
            if (error) *error = std::string("missing [server] key: ") + required[index];
            return false;
        }
    }

    DaemonConfig next;
    if (!ParseInt(values["id"], 1, INT_MAX, &next.id) ||
        !ParseInt(values["max_client"], 1, 5000, &next.max_client) ||
        !ParseInt(values["this_tcp_port"], 1, 65535, &next.this_tcp_port) ||
        !ParseInt(values["this_udp_port"], 1, 65535, &next.this_udp_port) ||
        !ParseInt(values["bridge_port"], 1, 65535, &next.bridge_port)) {
        if (error) *error = "invalid numeric [server] value";
        return false;
    }
    next.this_ip = values["this_ip"];
    next.bridge_ip = values["bridge_ip"];
    if (next.this_ip.empty() || next.this_ip.size() > 15 ||
        next.bridge_ip.empty() || next.bridge_ip.size() > 255) {
        if (error) *error = "invalid this_ip or bridge_ip";
        return false;
    }
    const std::map<std::string, std::string>::const_iterator compatibility =
        values.find("allow_pending_before_gc");
    if (compatibility != values.end() &&
        !ParseBool(compatibility->second, &next.allow_pending_before_gc)) {
        if (error) *error = "invalid allow_pending_before_gc value";
        return false;
    }
    *output = next;
    return true;
}

std::string LegacyConfigPath(const std::string& service_name) {
    return "./cfg/" + service_name + ".cfg";
}

std::string LegacyPidPath(const std::string& service_name) {
    return "./pid/" + service_name + ".pid";
}

} }  // namespace dnf::channel
