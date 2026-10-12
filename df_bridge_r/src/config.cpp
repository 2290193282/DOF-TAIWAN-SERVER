#include "dfbridge/config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

namespace dfbridge {
namespace {

std::string Trim(const std::string& value) {
    std::string::size_type first = 0;
    while (first < value.size() &&
           std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    std::string::size_type last = value.size();
    while (last > first &&
           std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const char ch) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    });
    return value;
}

std::string StripComment(const std::string& value) {
    bool quoted = false;
    char quote = 0;
    for (std::string::size_type i = 0; i < value.size(); ++i) {
        const char ch = value[i];
        if ((ch == '\'' || ch == '"') && (i == 0 || value[i - 1] != '\\')) {
            if (!quoted) {
                quoted = true;
                quote = ch;
            } else if (quote == ch) {
                quoted = false;
            }
        }
        if (!quoted && ch == '#') {
            return value.substr(0, i);
        }
        if (!quoted && ch == '/' && i + 1 < value.size() && value[i + 1] == '/') {
            return value.substr(0, i);
        }
    }
    return value;
}

std::string Unquote(const std::string& value) {
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

std::int32_t ParseInt(const std::string& value, const char* field) {
    std::size_t consumed = 0;
    long parsed = 0;
    try {
        parsed = std::stol(value, &consumed, 10);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string("invalid integer for ") + field);
    }
    if (consumed != value.size() || parsed < std::numeric_limits<std::int32_t>::min() ||
        parsed > std::numeric_limits<std::int32_t>::max()) {
        throw std::runtime_error(std::string("invalid integer for ") + field);
    }
    return static_cast<std::int32_t>(parsed);
}

std::uint16_t ParsePort(const std::string& value, const char* field) {
    const std::int32_t port = ParseInt(value, field);
    if (port <= 0 || port > 65535) {
        throw std::runtime_error(std::string(field) + " must be in 1..65535");
    }
    return static_cast<std::uint16_t>(port);
}

std::string Required(const std::map<std::string, std::string>& values,
                     const char* key) {
    const std::map<std::string, std::string>::const_iterator found = values.find(key);
    if (found == values.end() || found->second.empty()) {
        throw std::runtime_error(std::string("missing bridge config key: ") + key);
    }
    return found->second;
}

}  // namespace

bool IsValidInstanceName(const std::string& value) {
    if (value.empty() || value.size() > 64 || value == "." || value == "..") {
        return false;
    }
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        const unsigned char ch = static_cast<unsigned char>(*it);
        if (!std::isalnum(ch) && ch != '_' && ch != '-' && ch != '.') {
            return false;
        }
    }
    return value.find("..") == std::string::npos;
}

BridgeConfig BridgeConfigParser::Parse(const std::string& text,
                                       const std::string& instance_name) const {
    if (!IsValidInstanceName(instance_name)) {
        throw std::runtime_error("invalid bridge instance name");
    }

    std::map<std::string, std::string> values;
    std::istringstream input(text);
    std::string section;
    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        line = Trim(StripComment(line));
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            section = Lower(Trim(line.substr(1, line.size() - 2)));
            if (section != "server" && section != "db") {
                throw std::runtime_error("unknown bridge config section at line " +
                                         std::to_string(line_number));
            }
            continue;
        }
        const std::string::size_type equal = line.find('=');
        if (equal == std::string::npos || section.empty()) {
            throw std::runtime_error("invalid bridge config line " +
                                     std::to_string(line_number));
        }
        const std::string key = Lower(Trim(line.substr(0, equal)));
        const std::string value = Unquote(Trim(line.substr(equal + 1)));
        if (key.empty()) {
            throw std::runtime_error("empty bridge config key at line " +
                                     std::to_string(line_number));
        }
        values[section + "." + key] = value;
    }

    BridgeConfig config;
    config.instance_name = instance_name;
    const std::map<std::string, std::string>::const_iterator max_client =
        values.find("server.max_client");
    if (max_client != values.end()) {
        config.max_client = ParseInt(max_client->second, "max_client");
    }
    if (config.max_client <= 0 || config.max_client > 10000) {
        throw std::runtime_error("max_client must be in 1..10000");
    }
    const std::map<std::string, std::string>::const_iterator this_ip =
        values.find("server.this_ip");
    if (this_ip != values.end() && !this_ip->second.empty()) {
        config.this_ip = this_ip->second;
    }
    config.tcp_port = ParsePort(Required(values, "server.this_tcp_port"),
                                "this_tcp_port");
    config.udp_port = ParsePort(Required(values, "server.this_udp_port"),
                                "this_udp_port");
    config.db_host = Required(values, "db.db_ip");
    config.db_name = Required(values, "db.db_name");
    config.db_user = Required(values, "db.db_id");
    config.db_password = Required(values, "db.db_pwd");
    const std::map<std::string, std::string>::const_iterator db_port =
        values.find("db.db_port");
    if (db_port != values.end() && !db_port->second.empty()) {
        config.db_port = ParsePort(db_port->second, "db_port");
    }
    return config;
}

BridgeConfig BridgeConfigParser::ParseFile(const std::string& path,
                                           const std::string& instance_name) const {
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) {
        throw std::runtime_error("cannot open bridge config: " + path);
    }
    std::ostringstream text;
    text << input.rdbuf();
    if (!input.good() && !input.eof()) {
        throw std::runtime_error("cannot read bridge config: " + path);
    }
    return Parse(text.str(), instance_name);
}

}  // namespace dfbridge
