#include "old_data/PVF/event/upgrade_room.h"

#include <limits.h>
#include <stddef.h>

#include <string>
#include <vector>

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

namespace {

const char kUpgradeRoomDirectory[] = "event/upgraderoom/";
const char kUpgradeRoomFilename[] = "upgraderoom.info";
const uint32_t kGoldKey = 0u;
const uint32_t kClearCubeKey = 3037u;

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

enum Section {
    SECTION_NONE = 0,
    SECTION_GIVE_ITEM_INFO,
    SECTION_MAX_SPECTATOR,
    SECTION_DEFAULT_GOLD,
};

bool IsIntegerToken(DNFLEX_TOKEN token) {
    return token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL;
}

Section OpeningSection(const std::string& token) {
    if (token == "[give item info]") return SECTION_GIVE_ITEM_INFO;
    if (token == "[max spectator]") return SECTION_MAX_SPECTATOR;
    if (token == "[set default gold]") return SECTION_DEFAULT_GOLD;
    return SECTION_NONE;
}

bool FinishScalar(Section section,
                  const std::vector<int>& values,
                  upgrade_room::UpgradeRoomConfig& config,
                  uint32_t& seen_mask) {
    if (values.size() != 1u) return false;
    const int value = values[0];
    if (value <= 0) return false;

    if (section == SECTION_MAX_SPECTATOR) {
        if ((seen_mask & 2u) != 0u || value > 0xffff) return false;
        seen_mask |= 2u;
        config.max_spectators = static_cast<uint16_t>(value);
        return true;
    }
    if (section == SECTION_DEFAULT_GOLD) {
        if ((seen_mask & 4u) != 0u) return false;
        seen_mask |= 4u;
        config.default_gold = static_cast<uint32_t>(value);
        return true;
    }
    return false;
}

bool FinishDonationSection(
    const std::vector<int>& values,
    upgrade_room::UpgradeRoomConfig& config,
    uint32_t& seen_mask) {
    if ((seen_mask & 1u) != 0u || values.size() != 4u) return false;

    std::map<uint32_t, uint32_t> limits;
    for (size_t index = 0u; index < values.size(); index += 2u) {
        const int signed_key = values[index];
        const int signed_limit = values[index + 1u];
        if (signed_key < 0 || signed_limit <= 0) return false;
        const uint32_t key = static_cast<uint32_t>(signed_key);
        if (key != kGoldKey && key != kClearCubeKey) return false;
        if (!limits.insert(std::make_pair(
                key, static_cast<uint32_t>(signed_limit))).second) {
            return false;
        }
    }
    if (limits.size() != 2u || limits.find(kGoldKey) == limits.end() ||
        limits.find(kClearCubeKey) == limits.end()) {
        return false;
    }

    config.donation_limits.swap(limits);
    seen_mask |= 1u;
    return true;
}

bool ParseUpgradeRoomConfig(upgrade_room::UpgradeRoomConfig& config) {
    Section section = SECTION_NONE;
    std::vector<int> values;
    uint32_t seen_mask = 0u;

    for (;;) {
        int integer = 0;
        float real = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token = ScanAll(&integer, &real, &text);
        if (token == DNF_TOK_EOF) {
            if (section == SECTION_GIVE_ITEM_INFO) return false;
            if (section != SECTION_NONE &&
                !FinishScalar(section, values, config, seen_mask)) {
                return false;
            }
            break;
        }

        if (token == DNF_TOK_TYPE) {
            if (section == SECTION_GIVE_ITEM_INFO) {
                if (text != "[/give item info]" ||
                    !FinishDonationSection(values, config, seen_mask)) {
                    return false;
                }
                section = SECTION_NONE;
                values.clear();
                continue;
            }
            if (section != SECTION_NONE) {
                if (!FinishScalar(section, values, config, seen_mask)) {
                    return false;
                }
                section = SECTION_NONE;
                values.clear();
            }
            section = OpeningSection(text);
            if (section == SECTION_NONE) return false;
            continue;
        }

        if (section == SECTION_NONE || !IsIntegerToken(token)) return false;
        values.push_back(integer);
        if (section != SECTION_GIVE_ITEM_INFO && values.size() > 1u) {
            return false;
        }
        if (section == SECTION_GIVE_ITEM_INFO && values.size() > 4u) {
            return false;
        }
    }

    if (seen_mask != 7u || config.max_spectators == 0u ||
        config.default_gold == 0u) {
        return false;
    }
    const std::map<uint32_t, uint32_t>::const_iterator gold =
        config.donation_limits.find(kGoldKey);
    const std::map<uint32_t, uint32_t>::const_iterator cube =
        config.donation_limits.find(kClearCubeKey);
    return gold != config.donation_limits.end() &&
           cube != config.donation_limits.end() &&
           config.default_gold <= gold->second &&
           gold->second <= static_cast<uint32_t>(INT_MAX) &&
           cube->second <= static_cast<uint32_t>(INT_MAX);
}

class UpgradeRoomConfigCache {
public:
    UpgradeRoomConfigCache() : attempted_(false), valid_(false), config_() {}

    const upgrade_room::UpgradeRoomConfig* get() {
        if (!attempted_) {
            attempted_ = true;
            valid_ = load();
        }
        return valid_ ? &config_ : 0;
    }

private:
    bool load() {
        if (!loadRDARScriptFile(kUpgradeRoomDirectory,
                                kUpgradeRoomFilename)) {
            return false;
        }
        RdarUnloadGuard unload;
        upgrade_room::UpgradeRoomConfig parsed;
        if (!ParseUpgradeRoomConfig(parsed)) return false;
        config_ = parsed;
        return true;
    }

    bool attempted_;
    bool valid_;
    upgrade_room::UpgradeRoomConfig config_;
};

UpgradeRoomConfigCache& ConfigCache() {
    static UpgradeRoomConfigCache cache;
    return cache;
}

}  // namespace

namespace upgrade_room {

UpgradeRoomConfig::UpgradeRoomConfig()
    : donation_limits(), max_spectators(0u), default_gold(0u) {}

const UpgradeRoomConfig* GetUpgradeRoomConfig() {
    return ConfigCache().get();
}

bool IsUpgradeRoomConfigLoaded() {
    return GetUpgradeRoomConfig() != 0;
}

}  // namespace upgrade_room_0828
