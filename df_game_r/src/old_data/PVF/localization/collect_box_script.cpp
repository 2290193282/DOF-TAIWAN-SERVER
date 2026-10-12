#include "old_data/PVF/localization/collect_box_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/localization/collect_box_time_policy.h"

#include <algorithm>
#include <atomic>
#include <set>
#include <string>
#include <vector>

namespace collect_box_script {
namespace {

enum TokenKind { kInteger, kString, kType };

struct Token {
    Token() : kind(kString), integer(0), text() {}
    TokenKind kind;
    int integer;
    std::string text;
};

struct Extension {
    Extension() : item_id(0u), days(0u), unlimited(false) {}
    uint32_t item_id;
    uint32_t days;
    bool unlimited;
};

struct Bead {
    Bead() : item_id(0u), enchant_type(0) {}
    uint32_t item_id;
    int enchant_type;
};

struct Config {
    Config()
        : box_id(0u), expiration(), extensions(), beads() {}
    uint8_t box_id;
    collect_box_time::Window expiration;
    std::vector<Extension> extensions;
    std::vector<Bead> beads;
};

Config& Published() {
    static Config config;
    return config;
}

std::atomic<bool>& ReadyFlag() {
    static std::atomic<bool> ready(false);
    return ready;
}

void LowerAscii(std::string* value) {
    if (!value) return;
    for (size_t index = 0u; index < value->size(); ++index) {
        char& ch = (*value)[index];
        if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch - 'A' + 'a');
    }
}

class ScriptUnloadGuard {
public:
    ScriptUnloadGuard() {}
    ~ScriptUnloadGuard() { unloadRDARScriptFile(); }

private:
    ScriptUnloadGuard(const ScriptUnloadGuard&);
    ScriptUnloadGuard& operator=(const ScriptUnloadGuard&);
};

bool ReadTokens(const char* filename, std::vector<Token>* tokens) {
    if (!filename || !tokens || !loadRDARScriptFile("", filename)) {
        return false;
    }
    ScriptUnloadGuard unload;
    tokens->clear();
    while (1) {
        int integer = 0;
        float real = 0.0f;
        std::string text;
        const DNFLEX_TOKEN value = ScanAll(&integer, &real, &text);
        if (value == DNF_TOK_EOF) return true;
        Token token;
        if (value == DNF_TOK_DECIMAL || value == DNF_TOK_HEXADECIMAL) {
            if (integer < 0) return false;
            token.kind = kInteger;
            token.integer = integer;
        } else if (value == DNF_TOK_STRING) {
            token.kind = kString;
            token.text = text;
        } else if (value == DNF_TOK_TYPE) {
            token.kind = kType;
            token.text = text;
            LowerAscii(&token.text);
        } else {
            return false;
        }
        tokens->push_back(token);
    }
}

bool TakeInteger(const std::vector<Token>& tokens,
                 size_t* cursor,
                 uint32_t* value) {
    if (!cursor || !value || *cursor >= tokens.size() ||
        tokens[*cursor].kind != kInteger) {
        return false;
    }
    *value = static_cast<uint32_t>(tokens[*cursor].integer);
    ++*cursor;
    return true;
}

bool TakeClosing(const std::vector<Token>& tokens,
                 size_t* cursor,
                 const char* closing) {
    if (!cursor || !closing || *cursor >= tokens.size() ||
        tokens[*cursor].kind != kType ||
        tokens[*cursor].text != closing) {
        return false;
    }
    ++*cursor;
    return true;
}

bool ParseExtensions(const std::vector<Token>& tokens,
                     size_t* cursor,
                     Config* config) {
    std::set<uint32_t> unique;
    while (*cursor < tokens.size() && tokens[*cursor].kind != kType) {
        uint32_t item_id = 0u;
        uint32_t days = 0u;
        if (!TakeInteger(tokens, cursor, &item_id) || item_id == 0u ||
            !TakeInteger(tokens, cursor, &days) ||
            !unique.insert(item_id).second) {
            return false;
        }
        Extension rule;
        rule.item_id = item_id;
        rule.days = days;
        rule.unlimited = days == 0u;
        config->extensions.push_back(rule);
    }
    return !config->extensions.empty() &&
           TakeClosing(tokens, cursor, "[/extend item]");
}

bool ParseBeads(const std::vector<Token>& tokens,
                size_t* cursor,
                Config* config) {
    uint32_t columns = 0u;
    uint32_t rows = 0u;
    if (!TakeInteger(tokens, cursor, &columns) || columns == 0u ||
        !TakeInteger(tokens, cursor, &rows) || rows == 0u ||
        columns > kMaximumBeadCount || rows > kMaximumBeadCount ||
        columns * rows > kMaximumBeadCount) {
        return false;
    }
    std::set<uint32_t> unique;
    while (*cursor < tokens.size() && tokens[*cursor].kind != kType) {
        uint32_t item_id = 0u;
        uint32_t enchant_type = 0u;
        if (!TakeInteger(tokens, cursor, &item_id) || item_id == 0u ||
            !TakeInteger(tokens, cursor, &enchant_type) ||
            enchant_type == 0u || !unique.insert(item_id).second) {
            return false;
        }
        Bead bead;
        bead.item_id = item_id;
        bead.enchant_type = static_cast<int>(enchant_type);
        config->beads.push_back(bead);
    }
    return config->beads.size() ==
               static_cast<size_t>(columns * rows) &&
           TakeClosing(tokens, cursor, "[/item position info]");
}

bool ParseMain(Config* config) {
    std::vector<Token> tokens;
    if (!config ||
        !ReadTokens("localization/collectbox/collectboxinfo.etc", &tokens) ||
        tokens.empty() || tokens[0].kind != kType ||
        tokens[0].text != "[collect box info]") {
        return false;
    }

    size_t cursor = 1u;
    bool found_index = false;
    bool found_base = false;
    bool found_maximum = false;
    bool found_extensions = false;
    bool found_beads = false;
    bool found_end = false;
    while (cursor < tokens.size()) {
        if (tokens[cursor].kind != kType) return false;
        const std::string tag = tokens[cursor++].text;
        if (tag == "[/collect box info]") {
            found_end = true;
            break;
        }
        if (tag == "[index]") {
            uint32_t value = 0u;
            if (found_index || !TakeInteger(tokens, &cursor, &value) ||
                value != 1u) {
                // The restored SQL owner has exactly one official box row.
                return false;
            }
            config->box_id = static_cast<uint8_t>(value);
            found_index = true;
        } else if (tag == "[base expiration date]") {
            if (found_base ||
                !TakeInteger(tokens, &cursor, &config->expiration.base)) {
                return false;
            }
            found_base = true;
        } else if (tag == "[max expiration date]") {
            if (found_maximum ||
                !TakeInteger(tokens, &cursor, &config->expiration.maximum)) {
                return false;
            }
            found_maximum = true;
        } else if (tag == "[extend item]") {
            if (found_extensions ||
                !ParseExtensions(tokens, &cursor, config)) {
                return false;
            }
            found_extensions = true;
        } else if (tag == "[item position info]") {
            if (found_beads || !ParseBeads(tokens, &cursor, config)) {
                return false;
            }
            found_beads = true;
        } else {
            // UI-only scalar/list values belong to the client.  Keep the
            // server grammar bounded by consuming only until the next tag.
            while (cursor < tokens.size() && tokens[cursor].kind != kType) {
                ++cursor;
            }
        }
    }
    return found_end && cursor == tokens.size() && found_index && found_base &&
           found_maximum && found_extensions && found_beads &&
           config->expiration.maximum > config->expiration.base;
}

bool ValidateBuffTypes(const Config& config) {
    std::vector<Token> tokens;
    if (!ReadTokens("localization/collectbox/collectboxbuffinfo.etc",
                    &tokens)) {
        return false;
    }
    std::set<int> available;
    size_t cursor = 0u;
    while (cursor < tokens.size()) {
        if (tokens[cursor].kind != kType ||
            tokens[cursor++].text != "[enchant info]") {
            return false;
        }
        uint32_t type = 0u;
        if (!TakeInteger(tokens, &cursor, &type) || type == 0u ||
            !available.insert(static_cast<int>(type)).second) {
            return false;
        }
        bool closed = false;
        while (cursor < tokens.size()) {
            if (tokens[cursor].kind == kType &&
                tokens[cursor].text == "[/enchant info]") {
                ++cursor;
                closed = true;
                break;
            }
            ++cursor;
        }
        if (!closed) return false;
    }
    for (size_t index = 0u; index < config.beads.size(); ++index) {
        if (available.find(config.beads[index].enchant_type) ==
            available.end()) {
            return false;
        }
    }
    return !available.empty();
}

}  // namespace

bool LoadConfig() {
    ReadyFlag().store(false, std::memory_order_relaxed);
    Config parsed;
    if (!ParseMain(&parsed) || !ValidateBuffTypes(parsed)) return false;
    Published() = parsed;
    ReadyFlag().store(true, std::memory_order_release);
    return true;
}

bool IsReady() { return ReadyFlag().load(std::memory_order_acquire); }

uint8_t BoxId() { return IsReady() ? Published().box_id : 0u; }

uint32_t BaseExpiry() {
    return IsReady() ? Published().expiration.base : 0u;
}

uint32_t MaximumExpiry() {
    return IsReady() ? Published().expiration.maximum : 0u;
}

bool IsRawExpiry(uint32_t expiry_at) {
    return IsReady() && expiry_at >= Published().expiration.base &&
           expiry_at <= Published().expiration.maximum;
}

bool IsBead(uint32_t item_id) {
    if (!IsReady()) return false;
    const std::vector<Bead>& beads = Published().beads;
    for (size_t index = 0u; index < beads.size(); ++index) {
        if (beads[index].item_id == item_id) return true;
    }
    return false;
}

bool IsExtension(uint32_t item_id,
                 uint32_t* extension_days,
                 bool* unlimited) {
    if (!IsReady()) return false;
    const std::vector<Extension>& extensions = Published().extensions;
    for (size_t index = 0u; index < extensions.size(); ++index) {
        if (extensions[index].item_id != item_id) continue;
        if (extension_days) *extension_days = extensions[index].days;
        if (unlimited) *unlimited = extensions[index].unlimited;
        return true;
    }
    return false;
}

uint32_t RemainingSeconds(uint32_t expiry_at, uint32_t now) {
    return IsReady() ? collect_box_time::RemainingSeconds(
                           Published().expiration, expiry_at, now)
                     : 0u;
}

}  // namespace collect_box_script
