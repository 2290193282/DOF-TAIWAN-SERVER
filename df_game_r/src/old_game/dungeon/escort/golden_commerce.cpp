#include "old_game/dungeon/escort/golden_commerce.h"

#include "old_game/dungeon/escort/packet_dispatcher_golden_commerce.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <map>
#include <pthread.h>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/system_time.h"
#include "infra/log/format/trace.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/event/db_golden_commerce.h"
#include "old_game/dungeon/instance/monster_move_system.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

namespace golden_commerce {
namespace {

const char kGoldenCommerceDirectory[] = "Event/GoldenCommerce/";
const char kGoldenCommerceFilename[] = "GoldenCommerce.info";
const char kServerPolicyFilename[] = "GoldenCommerceServer0828.info";

enum {
    kInventoryType = 1,
    kDungeonUserState = 5,
    kInvalidMemberUniqueId = 0xffff,
    kNoSelection = -1,
    kMissingItemError = 21,
    kInsufficientFeeError = 22,
    kInvalidContextError = 23,
    kGoldenCommerceMoneySubReason = 4,
    kGoldenCommerceItemDelReason = 4,
    kRollbackMoneyAddReason = 12,
    kOfficialSelectTimeMs = 20000,
    kOfficialExperienceRatePermyriad = 17000,
    kSelectionCommandGraceSeconds = 5,
    kClaimRetrySeconds = 10,
    kAcknowledgedClaimLifetimeSeconds = 600,
    kSessionLifetimeSeconds = 3600,
    kCompletedLifetimeSeconds = 600,
    kMaximumDamageFingerprintsPerMap = 16384,
    kOverlayPolicyVersion = 1,
    kOverlayRewardFormula = 1,
    kOverlayRewardDenominator = 100,
    kOverlayResultAuxMode = 1,
    kOverlayBossX = 9,
    kOverlayBossY = 1,
};

class MutexGuard {
public:
    explicit MutexGuard(pthread_mutex_t* mutex) : mutex_(mutex) {
        pthread_mutex_lock(mutex_);
    }
    ~MutexGuard() { pthread_mutex_unlock(mutex_); }

private:
    MutexGuard(const MutexGuard&);
    MutexGuard& operator=(const MutexGuard&);
    pthread_mutex_t* mutex_;
};

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct CargoConfig {
    CargoConfig()
        : item_id(0), base_value(0), fee(0), parameter_3(0.0f),
          parameter_4(0.0f), free_choice(false), cargo_type(-1) {}

    int item_id;
    int base_value;
    int fee;
    float parameter_3;
    float parameter_4;
    bool free_choice;
    int cargo_type;
};

struct DungeonConfig {
    DungeonConfig()
        : dungeon_index(0), min_level(0), max_level(0),
          cart_monster_id(0u), boss_x(-1), boss_y(-1) {}

    int dungeon_index;
    int min_level;
    int max_level;
    uint32_t cart_monster_id;
    int boss_x;
    int boss_y;
    std::vector<int> allowed_item_ids;
};

struct GoldenCommerceConfig {
    GoldenCommerceConfig()
        : loaded(false), select_time_ms(0u),
          exp_rate_permyriad(0u), overlay_policy_version(0),
          reward_formula(0), reward_denominator(0), result_aux_mode(0) {}

    bool loaded;
    uint32_t select_time_ms;
    uint32_t exp_rate_permyriad;
    std::vector<int> rank_data;
    std::map<int, CargoConfig> cargo;
    std::map<int, DungeonConfig> dungeons;
    int overlay_policy_version;
    int reward_formula;
    int reward_denominator;
    int result_aux_mode;
};

enum LocalClaimState {
    LOCAL_CLAIM_IDLE = 0,
    LOCAL_CLAIM_QUEUED = 1,
    LOCAL_CLAIM_RETRY = 2,
    LOCAL_CLAIM_DELIVERED = 3,
    LOCAL_CLAIM_QUARANTINED = 4,
};

struct ClearExperienceSnapshot {
    ClearExperienceSnapshot() : total(0u), event_kind(0u) {
        std::memset(fields, 0, sizeof(fields));
    }
    uint32_t total;
    uint32_t fields[21];
    uint8_t event_kind;
};

struct PartyMemberState {
    PartyMemberState()
        : account_id(0u), charac_no(0u), server_group(-1),
          unique_id(kInvalidMemberUniqueId), selected_item_id(kNoSelection),
          reported_percent(0u), reward_reported(false), result_ui_sent(false),
          awarded_monster_exp(0u), clear_exp_awarded(false) {}

    uint32_t account_id;
    uint32_t charac_no;
    int server_group;
    uint16_t unique_id;
    int selected_item_id;
    uint8_t reported_percent;
    bool reward_reported;
    bool result_ui_sent;
    uint32_t awarded_monster_exp;
    bool clear_exp_awarded;
    ClearExperienceSnapshot clear_exp;
};

struct PartySession {
    PartySession()
        : party_index(-1), dungeon_index(-1), opened(false), started(false),
          completed(false), terminal_reached(false),
          terminal_room_bound(false), clear_seen(false), result_ui_complete(false),
          terminal_grid_x(0u), terminal_grid_y(0u),
          run_token_hi(0u), run_token_lo(0u),
          dungeon_start_tick(0u),
          opened_epoch(0u), selection_deadline_epoch(0u),
          completed_epoch(0u), current_map_index(-1), cart_unique_id(0u),
          cart_monster_id(0u), cart_max_hp(0u), cart_remaining_hp(0u),
          authoritative_percent(100u), final_percent_latched(false),
          damage_fingerprints() {}

    int party_index;
    int dungeon_index;
    bool opened;
    bool started;
    bool completed;
    bool terminal_reached;
    bool terminal_room_bound;
    bool clear_seen;
    bool result_ui_complete;
    uint8_t terminal_grid_x;
    uint8_t terminal_grid_y;
    uint32_t run_token_hi;
    uint32_t run_token_lo;
    uint64_t dungeon_start_tick;
    uint32_t opened_epoch;
    uint32_t selection_deadline_epoch;
    uint32_t completed_epoch;
    int current_map_index;
    uint16_t cart_unique_id;
    uint32_t cart_monster_id;
    uint32_t cart_max_hp;
    uint32_t cart_remaining_hp;
    uint8_t authoritative_percent;
    bool final_percent_latched;
    PartyMemberState members[kOrdinaryPartySeatCount];
    std::set<uint64_t> damage_fingerprints;
};

struct ClaimOutboxKey {
    ClaimOutboxKey()
        : server_group(-1), run_token_hi(0u), run_token_lo(0u),
          request_token(0u), charac_no(0u) {}

    bool operator<(const ClaimOutboxKey& other) const {
        if (server_group != other.server_group) {
            return server_group < other.server_group;
        }
        if (run_token_hi != other.run_token_hi) {
            return run_token_hi < other.run_token_hi;
        }
        if (run_token_lo != other.run_token_lo) {
            return run_token_lo < other.run_token_lo;
        }
        if (request_token != other.request_token) {
            return request_token < other.request_token;
        }
        return charac_no < other.charac_no;
    }

    int server_group;
    uint32_t run_token_hi;
    uint32_t run_token_lo;
    uint32_t request_token;
    uint32_t charac_no;
};

struct ClaimOutboxEntry {
    ClaimOutboxEntry()
        : state(LOCAL_CLAIM_IDLE), next_retry_epoch(0u),
          acknowledged_epoch(0u), request(), settled() {
        std::memset(&request, 0, sizeof(request));
        std::memset(&settled, 0, sizeof(settled));
    }

    LocalClaimState state;
    uint32_t next_retry_epoch;
    uint32_t acknowledged_epoch;
    RewardClaimMessage request;
    RewardClaimMessage settled;
};

struct ClaimDispatch {
    ClaimDispatch() : key(), replay(false), request() {
        std::memset(&request, 0, sizeof(request));
    }

    ClaimOutboxKey key;
    bool replay;
    RewardClaimMessage request;
};

typedef std::map<CParty*, PartySession> PartySessionMap;
typedef std::map<ClaimOutboxKey, ClaimOutboxEntry> ClaimOutboxMap;

GoldenCommerceConfig g_config;
PartySessionMap g_sessions;
ClaimOutboxMap g_claim_outbox;
pthread_mutex_t g_golden_commerce_mutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t g_next_run_token = 0x82800001u;
uint32_t g_next_request_token = 0x82810001u;
bool g_policy_missing_traced = false;

bool IsIntegerToken(DNFLEX_TOKEN token) {
    return token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL;
}

bool IsNumericToken(DNFLEX_TOKEN token) {
    return IsIntegerToken(token) || token == DNF_TOK_FLOAT;
}

float NumericValue(DNFLEX_TOKEN token, int integer_value, float float_value) {
    return token == DNF_TOK_FLOAT ? float_value
                                  : static_cast<float>(integer_value);
}

uint32_t ToPermyriad(DNFLEX_TOKEN token,
                     int integer_value,
                     float float_value) {
    const double value = token == DNF_TOK_FLOAT
        ? static_cast<double>(float_value)
        : static_cast<double>(integer_value);
    if (value <= 0.0 || value > 1000.0) return 0u;
    return static_cast<uint32_t>(value * 10000.0 + 0.5);
}

bool FinalizeDungeonRecord(const std::vector<int>& values,
                           GoldenCommerceConfig& config) {
    if (values.size() < 4u) return false;
    DungeonConfig dungeon;
    dungeon.dungeon_index = values[0];
    dungeon.min_level = values[1];
    dungeon.max_level = values[2];
    if (dungeon.dungeon_index <= 0 || dungeon.min_level <= 0 ||
        dungeon.max_level < dungeon.min_level || dungeon.max_level > 255 ||
        config.dungeons.find(dungeon.dungeon_index) != config.dungeons.end()) {
        return false;
    }
    std::set<int> unique_items;
    for (std::vector<int>::const_iterator item = values.begin() + 3;
         item != values.end(); ++item) {
        if (*item <= 0 || !unique_items.insert(*item).second) return false;
        dungeon.allowed_item_ids.push_back(*item);
    }
    config.dungeons.insert(std::make_pair(dungeon.dungeon_index, dungeon));
    return true;
}

bool ParseGoldenCommerceConfig(GoldenCommerceConfig& config) {
    bool in_load_item = false;
    bool in_dungeon_condition = false;
    bool in_rank_data = false;
    bool saw_load_item = false;
    bool saw_dungeon_condition = false;
    bool saw_rank_data = false;
    bool saw_select_time = false;
    bool saw_exp_rate = false;
    int pending_scalar = 0;
    int load_field = 0;
    CargoConfig cargo;
    std::vector<int> dungeon_values;

    for (;;) {
        int integer_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&integer_value, &float_value, &text);
        if (token == DNF_TOK_EOF) break;

        if (token == DNF_TOK_TYPE) {
            if (pending_scalar != 0) return false;
            if (in_load_item) {
                if (text != "[/load item]" || load_field != 0) return false;
                in_load_item = false;
                continue;
            }
            if (in_dungeon_condition) {
                if (text != "[/dungeon each condition]") return false;
                // The official 0828 file terminates the first two records
                // with -1 and leaves the final 3057 record to the closing
                // tag, exactly as the client importer accepts it.
                if (!dungeon_values.empty() &&
                    !FinalizeDungeonRecord(dungeon_values, config)) {
                    return false;
                }
                dungeon_values.clear();
                in_dungeon_condition = false;
                continue;
            }
            if (in_rank_data) {
                if (text != "[/rank data]" || config.rank_data.size() != 12u) {
                    return false;
                }
                in_rank_data = false;
                continue;
            }

            if (text == "[load item]") {
                if (saw_load_item) return false;
                saw_load_item = true;
                in_load_item = true;
            } else if (text == "[dungeon each condition]") {
                if (saw_dungeon_condition) return false;
                saw_dungeon_condition = true;
                in_dungeon_condition = true;
            } else if (text == "[rank data]") {
                if (saw_rank_data) return false;
                saw_rank_data = true;
                in_rank_data = true;
            } else if (text == "[select time]") {
                if (saw_select_time) return false;
                saw_select_time = true;
                pending_scalar = 1;
            } else if (text == "[exp rate]") {
                if (saw_exp_rate) return false;
                saw_exp_rate = true;
                pending_scalar = 2;
            }
            continue;
        }

        if (pending_scalar != 0) {
            if (!IsNumericToken(token)) return false;
            if (pending_scalar == 1) {
                if (!IsIntegerToken(token) || integer_value <= 0) return false;
                config.select_time_ms = static_cast<uint32_t>(integer_value);
            } else {
                config.exp_rate_permyriad =
                    ToPermyriad(token, integer_value, float_value);
                if (config.exp_rate_permyriad == 0u) return false;
            }
            pending_scalar = 0;
            continue;
        }

        if (in_load_item) {
            if (!IsNumericToken(token)) return false;
            switch (load_field) {
                case 0:
                    if (!IsIntegerToken(token)) return false;
                    cargo.item_id = integer_value;
                    break;
                case 1:
                    if (!IsIntegerToken(token)) return false;
                    cargo.base_value = integer_value;
                    break;
                case 2:
                    if (!IsIntegerToken(token)) return false;
                    cargo.fee = integer_value;
                    break;
                case 3:
                    cargo.parameter_3 =
                        NumericValue(token, integer_value, float_value);
                    break;
                case 4:
                    cargo.parameter_4 =
                        NumericValue(token, integer_value, float_value);
                    break;
                case 5:
                    if (!IsIntegerToken(token) ||
                        (integer_value != 0 && integer_value != 1)) return false;
                    cargo.free_choice = integer_value != 0;
                    break;
                case 6:
                    if (!IsIntegerToken(token)) return false;
                    cargo.cargo_type = integer_value;
                    break;
                default:
                    return false;
            }
            ++load_field;
            if (load_field == 7) {
                if (cargo.item_id <= 0 || cargo.base_value <= 0 ||
                    cargo.fee < 0 || cargo.parameter_3 <= 0.0f ||
                    cargo.parameter_4 <= 0.0f || cargo.cargo_type < 0 ||
                    (cargo.free_choice && cargo.fee != 0) ||
                    config.cargo.find(cargo.item_id) != config.cargo.end()) {
                    return false;
                }
                config.cargo.insert(std::make_pair(cargo.item_id, cargo));
                cargo = CargoConfig();
                load_field = 0;
            }
            continue;
        }

        if (in_dungeon_condition) {
            if (!IsIntegerToken(token)) return false;
            if (integer_value == -1) {
                if (!FinalizeDungeonRecord(dungeon_values, config)) return false;
                dungeon_values.clear();
            } else {
                dungeon_values.push_back(integer_value);
            }
            continue;
        }

        if (in_rank_data) {
            if (!IsIntegerToken(token) || integer_value < 0 ||
                integer_value > 100 || config.rank_data.size() >= 12u) {
                return false;
            }
            config.rank_data.push_back(integer_value);
        }
    }

    return !in_load_item && !in_dungeon_condition && !in_rank_data &&
           pending_scalar == 0 && saw_load_item && saw_dungeon_condition &&
           saw_rank_data && saw_select_time && saw_exp_rate &&
           !config.cargo.empty() && !config.dungeons.empty();
}

enum OverlaySection {
    OVERLAY_NONE = 0,
    OVERLAY_VERSION,
    OVERLAY_FORMULA,
    OVERLAY_AUX,
    OVERLAY_DUNGEON,
};

const char* OverlayClosingTag(OverlaySection section) {
    switch (section) {
        case OVERLAY_VERSION: return "[/policy version]";
        case OVERLAY_FORMULA: return "[/reward formula]";
        case OVERLAY_AUX: return "[/result auxiliary mode]";
        case OVERLAY_DUNGEON: return "[/dungeon cart policy]";
        default: return "";
    }
}

bool FinalizeOverlaySection(OverlaySection section,
                            std::vector<int>& values,
                            GoldenCommerceConfig& config) {
    if (section == OVERLAY_VERSION) {
        if (values.size() != 1u) return false;
        config.overlay_policy_version = values[0];
    } else if (section == OVERLAY_FORMULA) {
        if (values.size() != 2u) return false;
        config.reward_formula = values[0];
        config.reward_denominator = values[1];
    } else if (section == OVERLAY_AUX) {
        if (values.size() != 1u) return false;
        config.result_aux_mode = values[0];
    } else if (section == OVERLAY_DUNGEON) {
        if (!values.empty()) return false;
    } else {
        return false;
    }
    values.clear();
    return true;
}

bool ParseServerPolicy(GoldenCommerceConfig& config) {
    OverlaySection section = OVERLAY_NONE;
    bool saw_version = false;
    bool saw_formula = false;
    bool saw_aux = false;
    bool saw_dungeon = false;
    std::vector<int> values;

    for (;;) {
        int integer_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&integer_value, &float_value, &text);
        if (token == DNF_TOK_EOF) break;
        if (token == DNF_TOK_TYPE) {
            if (section != OVERLAY_NONE) {
                if (text != OverlayClosingTag(section) ||
                    !FinalizeOverlaySection(section, values, config)) {
                    return false;
                }
                section = OVERLAY_NONE;
                continue;
            }
            if (text == "[policy version]") {
                if (saw_version) return false;
                saw_version = true;
                section = OVERLAY_VERSION;
            } else if (text == "[reward formula]") {
                if (saw_formula) return false;
                saw_formula = true;
                section = OVERLAY_FORMULA;
            } else if (text == "[result auxiliary mode]") {
                if (saw_aux) return false;
                saw_aux = true;
                section = OVERLAY_AUX;
            } else if (text == "[dungeon cart policy]") {
                if (saw_dungeon) return false;
                saw_dungeon = true;
                section = OVERLAY_DUNGEON;
            }
            continue;
        }
        if (section == OVERLAY_NONE) continue;
        if (!IsIntegerToken(token)) return false;

        if (section != OVERLAY_DUNGEON) {
            values.push_back(integer_value);
            continue;
        }
        if (integer_value != -1) {
            values.push_back(integer_value);
            continue;
        }
        if (values.size() != 4u) return false;
        std::map<int, DungeonConfig>::iterator dungeon =
            config.dungeons.find(values[0]);
        if (dungeon == config.dungeons.end() || values[1] <= 0 ||
            values[2] < 0 || values[3] < 0 ||
            dungeon->second.cart_monster_id != 0u) return false;
        dungeon->second.cart_monster_id = static_cast<uint32_t>(values[1]);
        dungeon->second.boss_x = values[2];
        dungeon->second.boss_y = values[3];
        values.clear();
    }
    return section == OVERLAY_NONE && saw_version && saw_formula && saw_aux &&
           saw_dungeon;
}

bool ApplySafeDefaultServerPolicy(GoldenCommerceConfig& config) {
    if (config.dungeons.empty()) return false;
    config.overlay_policy_version = kOverlayPolicyVersion;
    config.reward_formula = kOverlayRewardFormula;
    config.reward_denominator = kOverlayRewardDenominator;
    config.result_aux_mode = kOverlayResultAuxMode;
    for (std::map<int, DungeonConfig>::iterator dungeon =
             config.dungeons.begin(); dungeon != config.dungeons.end();
         ++dungeon) {
        const uint32_t official_cart = OfficialCartForDungeon(dungeon->first);
        if (official_cart == 0u || dungeon->second.cart_monster_id != 0u) {
            return false;
        }
        dungeon->second.cart_monster_id = official_cart;
        dungeon->second.boss_x = kOverlayBossX;
        dungeon->second.boss_y = kOverlayBossY;
    }
    return true;
}

void TraceMissingPolicyOnce() {
    if (g_policy_missing_traced) return;
    g_policy_missing_traced = true;
    cMyTrace trace("GoldenCommerce0828Policy", __LINE__, 5);
    trace("[0828 GoldenCommerce] policy_missing_using_safe_defaults");
}

int OfficialFeeForCargo(int item_id) {
    switch (item_id) {
        case 10000910: return 0;
        case 10000911: return 20000;
        case 10000912: return 300000;
        case 10000913: return 2000000;
        case 10001574: return 0;
        case 10001575: return 0;
        default: return -1;
    }
}

bool OfficialFreeChoiceForCargo(int item_id) {
    return item_id == 10000910 || item_id == 10001574 || item_id == 10001575;
}

bool ExactAllowedCargo(const DungeonConfig& dungeon) {
    std::set<int> observed(dungeon.allowed_item_ids.begin(),
                           dungeon.allowed_item_ids.end());
    std::set<int> expected;
    const int all_items[] = {
        10000910, 10000911, 10000912, 10000913, 10001574, 10001575
    };
    for (unsigned int i = 0; i < sizeof(all_items) / sizeof(all_items[0]); ++i) {
        if (IsCargoAllowedInDungeon(dungeon.dungeon_index,
                                    static_cast<uint32_t>(all_items[i]))) {
            expected.insert(all_items[i]);
        }
    }
    return observed == expected;
}

bool ValidateGoldenCommerceConfig(const GoldenCommerceConfig& config) {
    static const int kOfficialRankData[12] = {
        100, 91, 90, 61, 60, 47, 46, 36, 35, 11, 10, 0
    };
    if (config.select_time_ms != kOfficialSelectTimeMs ||
        config.exp_rate_permyriad != kOfficialExperienceRatePermyriad ||
        config.rank_data.size() != 12u || config.cargo.size() != 6u ||
        config.dungeons.size() != 3u ||
        config.overlay_policy_version != kOverlayPolicyVersion ||
        config.reward_formula != kOverlayRewardFormula ||
        config.reward_denominator != kOverlayRewardDenominator ||
        config.result_aux_mode != kOverlayResultAuxMode) return false;
    for (int index = 0; index < 12; ++index) {
        if (config.rank_data[index] != kOfficialRankData[index]) return false;
    }

    CDataManager* const data_manager = G_CDataManager();
    if (!data_manager) return false;
    for (std::map<int, CargoConfig>::const_iterator cargo = config.cargo.begin();
         cargo != config.cargo.end(); ++cargo) {
        CItem* const item = data_manager->find_item(cargo->first);
        if (!item || item->is_stackable() != 1 ||
            OfficialCargoBaseValue(static_cast<uint32_t>(cargo->first)) !=
                static_cast<uint32_t>(cargo->second.base_value) ||
            OfficialFeeForCargo(cargo->first) != cargo->second.fee ||
            OfficialFreeChoiceForCargo(cargo->first) !=
                cargo->second.free_choice) return false;
    }

    for (std::map<int, DungeonConfig>::const_iterator dungeon =
             config.dungeons.begin(); dungeon != config.dungeons.end();
         ++dungeon) {
        const int index = dungeon->first;
        const int expected_min = index == kOfficialDungeonWest ? 40 :
                                 index == kOfficialDungeonSouth ? 61 : 75;
        const int expected_max = index == kOfficialDungeonWest ? 60 : 85;
        if (OfficialCartForDungeon(index) == 0u ||
            dungeon->second.min_level != expected_min ||
            dungeon->second.max_level != expected_max ||
            dungeon->second.cart_monster_id != OfficialCartForDungeon(index) ||
            dungeon->second.boss_x != kOverlayBossX ||
            dungeon->second.boss_y != kOverlayBossY ||
            !ExactAllowedCargo(dungeon->second)) return false;
    }
    return true;
}

const GoldenCommerceConfig* GetConfig() {
    if (g_config.loaded) return &g_config;

    GoldenCommerceConfig imported;
    if (!loadRDARScriptFile(kGoldenCommerceDirectory,
                            kGoldenCommerceFilename)) return 0;
    {
        RdarUnloadGuard unload;
        if (!ParseGoldenCommerceConfig(imported)) return 0;
    }
    if (!loadRDARScriptFile(kGoldenCommerceDirectory,
                            kServerPolicyFilename)) {
        if (!ApplySafeDefaultServerPolicy(imported)) return 0;
        TraceMissingPolicyOnce();
    } else {
        RdarUnloadGuard unload;
        if (!ParseServerPolicy(imported)) return 0;
    }
    if (!ValidateGoldenCommerceConfig(imported)) return 0;
    imported.loaded = true;
    g_config = imported;
    return &g_config;
}

uint32_t NowEpoch() {
    const int value = GlobalData::system_time()
        ? GlobalData::system_time()->getCurSec() : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

uint32_t NextNonZero(uint32_t& counter) {
    ++counter;
    if (counter == 0u) ++counter;
    return counter;
}

void TraceFailure(const char* reason, CUser* user) {
    cMyTrace trace("GoldenCommerce0828", __LINE__, 5);
    trace("[0828 GoldenCommerce] %s account(%d) charac(%d)",
          reason ? reason : "unknown",
          user ? user->get_acc_id() : 0,
          user ? user->get_charac_no(-1) : 0);
}

bool CaptureRoster(CParty* party,
                   const DungeonConfig& dungeon,
                   PartyMemberState* members) {
    if (!party || !members) return false;
    (void)dungeon;
    int member_count = 0;
    int server_group = -1;
    std::set<uint16_t> unique_ids;
    std::set<uint32_t> characters;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        members[seat] = PartyMemberState();
        CUser* const member = party->get_user(seat);
        if (!member) continue;
        // checkValidUser/_checkValidUser has a destructive mismatch branch
        // that calls DisConnSig(23) and clears the party seat.  Roster capture
        // is validation only; it must never disconnect a loading client.
        if (member->GetParty() != party ||
            member->get_state() != kDungeonUserState ||
            !member->getCurCharacR() || member->get_acc_id() <= 0 ||
            member->get_charac_no(-1) <= 0 || member->get_unique_id() <= 0 ||
            member->get_unique_id() >= kInvalidMemberUniqueId) return false;
        // GoldenCommerce.info's level tuple belongs to dungeon admission.
        // This sidecar runs only after CParty has already entered an official
        // Golden-Commerce dungeon; reapplying the entry range here makes a
        // successfully admitted run impossible to settle (for example an
        // already-admitted level-85 character in dungeon 3054).  Roster
        // capture therefore binds identity/state only.  The PVF tuple remains
        // parsed and validated, while reward policy is still independently
        // pinned to the official dungeon/cart/cargo matrix in the DB owner.
        const uint16_t unique_id =
            static_cast<uint16_t>(member->get_unique_id());
        const uint32_t charac_no =
            static_cast<uint32_t>(member->get_charac_no(-1));
        if (!unique_ids.insert(unique_id).second ||
            !characters.insert(charac_no).second) return false;
        if (server_group < 0) server_group = member->GetServerGroup();
        if (member->GetServerGroup() != server_group || server_group < 0 ||
            server_group >= kMaximumServerGroupCount) return false;
        members[seat].account_id =
            static_cast<uint32_t>(member->get_acc_id());
        members[seat].charac_no = charac_no;
        members[seat].server_group = server_group;
        members[seat].unique_id = unique_id;
        ++member_count;
    }
    return member_count > 0;
}

bool SameRoster(const PartySession& session,
                const PartyMemberState* current) {
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id != current[seat].unique_id ||
            session.members[seat].account_id != current[seat].account_id ||
            session.members[seat].charac_no != current[seat].charac_no ||
            session.members[seat].server_group != current[seat].server_group) {
            return false;
        }
    }
    return true;
}

int FindMemberSeat(const PartySession& session, CUser* user) {
    if (!user || user->get_unique_id() <= 0 ||
        user->get_unique_id() >= kInvalidMemberUniqueId) return -1;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        const PartyMemberState& member = session.members[seat];
        if (member.unique_id == static_cast<uint16_t>(user->get_unique_id()) &&
            member.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
            member.charac_no == static_cast<uint32_t>(user->get_charac_no(-1))) {
            return seat;
        }
    }
    return -1;
}

PartySessionMap::iterator FindBasicSession(CParty* party,
                                           const GoldenCommerceConfig& config) {
    PartySessionMap::iterator found = g_sessions.find(party);
    if (!party || found == g_sessions.end()) return g_sessions.end();
    const int party_index = party->GetPartyIndex();
    const int dungeon_index = party->battle_field().get_dungeon_index();
    if (found->second.party_index != party_index ||
        found->second.dungeon_index != dungeon_index ||
        config.dungeons.find(dungeon_index) == config.dungeons.end()) {
        g_sessions.erase(found);
        return g_sessions.end();
    }
    return found;
}

PartySessionMap::iterator FindCurrentRunSessionNoRoster(
    CParty* party, const GoldenCommerceConfig& config) {
    PartySessionMap::iterator found = FindBasicSession(party, config);
    if (found == g_sessions.end()) return found;

    // A CParty object and even its public party index can be reused for a
    // later entry into the same dungeon.  The battle-data start tick is the
    // server-owned per-entry identity; this helper deliberately omits only
    // the transient full-roster check, never the run check.
    const uint64_t current_start_tick =
        party->battle_data().dungeon_start_tick();
    if (current_start_tick == 0u) {
        return found->second.dungeon_start_tick == 0u
            ? found : g_sessions.end();
    }
    if (found->second.dungeon_start_tick == 0u) {
        found->second.dungeon_start_tick = current_start_tick;
        return found;
    }
    if (found->second.dungeon_start_tick != current_start_tick) {
        g_sessions.erase(found);
        return g_sessions.end();
    }
    return found;
}

PartySessionMap::iterator FindCurrentSession(
    CParty* party, const GoldenCommerceConfig& config,
    const char** rejection_reason = 0) {
    if (rejection_reason) *rejection_reason = "session_missing";
    PartySessionMap::iterator found =
        FindCurrentRunSessionNoRoster(party, config);
    if (found == g_sessions.end()) return found;
    const DungeonConfig& dungeon =
        config.dungeons.find(found->second.dungeon_index)->second;
    PartyMemberState current[kOrdinaryPartySeatCount];
    if (!CaptureRoster(party, dungeon, current) ||
        !SameRoster(found->second, current)) {
        if (rejection_reason) *rejection_reason = "roster";
        return g_sessions.end();
    }
    if (rejection_reason) *rejection_reason = 0;
    return found;
}

bool ScanCurrentCart(CParty* party,
                     uint32_t cart_monster_id,
                     uint16_t& cart_unique_id,
                     uint32_t& cart_max_hp) {
    cart_unique_id = 0u;
    cart_max_hp = 0u;
    if (!party || cart_monster_id == 0u) return false;
    MapInfo* const map_info = party->battle_field().GetCurrentMapInfo();
    if (!map_info) return false;
    int matches = 0;
    for (std::map<int, map_monster>::iterator monster =
             map_info->monsters_.begin(); monster != map_info->monsters_.end();
         ++monster) {
        if (monster->second.monster_id_ !=
            static_cast<int32_t>(cart_monster_id)) continue;
        const int raw_uid = monster->second.map_monster_unique_id_;
        const int raw_hp = monster->second.active_static_info_.getHpMax();
        if (raw_uid <= 0 || raw_uid >= kInvalidMemberUniqueId || raw_hp <= 0) {
            return false;
        }
        ++matches;
        cart_unique_id = static_cast<uint16_t>(raw_uid);
        cart_max_hp = static_cast<uint32_t>(raw_hp);
    }
    return matches == 1;
}

bool RebindCurrentCartIdentity(CParty* party,
                               uint32_t cart_monster_id,
                               uint16_t room_unique_id,
                               uint16_t stable_unique_id) {
    if (!party || cart_monster_id == 0u || room_unique_id == 0u ||
        stable_unique_id == 0u ||
        room_unique_id == kInvalidMemberUniqueId ||
        stable_unique_id == kInvalidMemberUniqueId) {
        return false;
    }

    MapInfo* const map_info = party->battle_field().GetCurrentMapInfo();
    if (!map_info) return false;
    std::map<int, map_monster>::iterator room =
        map_info->monsters_.find(static_cast<int>(room_unique_id));
    if (room == map_info->monsters_.end() ||
        room->second.map_monster_unique_id_ != room_unique_id ||
        room->second.monster_id_ != static_cast<int32_t>(cart_monster_id)) {
        return false;
    }
    if (room_unique_id == stable_unique_id) return true;

    // CLIENT_IDA_EVIDENCE: sub_4F10B0 reads the NOTI476 record UID at
    // 0x4F1117, queries the persistent map-move controller at 0x4F1128 and
    // takes distinct existing/new-object paths at 0x4F11F6/0x4F1177.  The
    // monster UID is also the key in all three server MapInfo identity
    // indexes.  A field-only rewrite therefore both defeats client reuse and
    // leaves CMD541/drop/ordering lookups pointing at the discarded room UID.
    if (map_info->monsters_.find(static_cast<int>(stable_unique_id)) !=
            map_info->monsters_.end() ||
        map_info->monster_identity_by_unique_id_.find(
            static_cast<int>(stable_unique_id)) !=
            map_info->monster_identity_by_unique_id_.end()) {
        return false;
    }
    std::map<int, int>::iterator identity =
        map_info->monster_identity_by_unique_id_.find(
            static_cast<int>(room_unique_id));
    if (identity == map_info->monster_identity_by_unique_id_.end() ||
        identity->second != static_cast<int>(cart_monster_id)) {
        return false;
    }
    std::vector<int>::iterator order = map_info->monster_order_.end();
    int order_matches = 0;
    for (std::vector<int>::iterator current = map_info->monster_order_.begin();
         current != map_info->monster_order_.end(); ++current) {
        if (*current == static_cast<int>(stable_unique_id)) return false;
        if (*current == static_cast<int>(room_unique_id)) {
            order = current;
            ++order_matches;
        }
    }
    if (order_matches != 1 || order == map_info->monster_order_.end()) {
        return false;
    }

    map_monster rebound(room->second);
    rebound.map_monster_unique_id_ = stable_unique_id;
    std::pair<std::map<int, map_monster>::iterator, bool> entity_insert =
        map_info->monsters_.insert(std::make_pair(
            static_cast<int>(stable_unique_id), rebound));
    if (!entity_insert.second) return false;
    // map_monster's reconstructed copy constructor intentionally covers only
    // proven fields, while its assignment operator preserves the complete
    // recovered scalar surface.  Re-assign from the live room entity before
    // erasing it so a key migration cannot drop map runtime bytes.
    entity_insert.first->second = room->second;
    entity_insert.first->second.map_monster_unique_id_ = stable_unique_id;
    std::pair<std::map<int, int>::iterator, bool> identity_insert =
        map_info->monster_identity_by_unique_id_.insert(std::make_pair(
            static_cast<int>(stable_unique_id),
            static_cast<int>(cart_monster_id)));
    if (!identity_insert.second) {
        map_info->monsters_.erase(entity_insert.first);
        return false;
    }

    *order = static_cast<int>(stable_unique_id);
    map_info->monster_identity_by_unique_id_.erase(identity);
    map_info->monsters_.erase(room);
    return true;
}

bool FindCurrentEntity(CParty* party,
                       uint16_t entity_unique_id,
                       map_monster*& output) {
    output = 0;
    if (!party || entity_unique_id == 0u) return false;
    MapInfo* const map_info = party->battle_field().GetCurrentMapInfo();
    if (!map_info) return false;
    for (std::map<int, map_monster>::iterator monster =
             map_info->monsters_.begin(); monster != map_info->monsters_.end();
         ++monster) {
        if (monster->second.map_monster_unique_id_ == entity_unique_id) {
            if (output) return false;
            output = &monster->second;
        }
    }
    return output != 0;
}

bool CartStillBound(CParty* party, const PartySession& session) {
    map_monster* entity = 0;
    return session.current_map_index == party->battle_field().get_cur_map() &&
           FindCurrentEntity(party, session.cart_unique_id, entity) && entity &&
           entity->monster_id_ == static_cast<int32_t>(session.cart_monster_id);
}

bool HasTerminalMovementLocked(const DungeonConfig& dungeon,
                               const PartySession& session) {
    return dungeon.boss_x >= 0 && dungeon.boss_x <= 0xff &&
           dungeon.boss_y >= 0 && dungeon.boss_y <= 0xff &&
           session.terminal_reached &&
           session.terminal_grid_x ==
               static_cast<uint8_t>(dungeon.boss_x) &&
           session.terminal_grid_y ==
               static_cast<uint8_t>(dungeon.boss_y);
}

bool IsTerminalCartStateLocked(CParty* party,
                               const DungeonConfig& dungeon,
                               const PartySession& session,
                               int& current_x,
                               int& current_y,
                               bool& boss_room,
                               bool& cart_bound) {
    current_x = -1;
    current_y = -1;
    boss_room = false;
    cart_bound = false;
    if (!party || dungeon.dungeon_index != session.dungeon_index) return false;
    CBattle_Field& field = party->battle_field();
    field.getCurPosXY(current_x, current_y);
    boss_room = party->checkBossRoom();
    cart_bound = CartStillBound(party, session);
    return session.started && !session.completed &&
           field.get_dungeon_index() == session.dungeon_index &&
           session.current_map_index == field.get_cur_map() &&
           current_x == dungeon.boss_x && current_y == dungeon.boss_y &&
           boss_room && cart_bound;
}

bool LatchTerminalRoomBoundLocked(CParty* party,
                                  const DungeonConfig& dungeon,
                                  PartySession& session,
                                  const char* trigger) {
    int current_x = -1;
    int current_y = -1;
    bool boss_room = false;
    bool cart_bound = false;
    const bool live_terminal = IsTerminalCartStateLocked(
        party, dungeon, session, current_x, current_y, boss_room, cart_bound);
    if (live_terminal && !session.terminal_room_bound) {
        session.terminal_room_bound = true;
        cMyTrace trace("GoldenCommerce0828Settlement", __LINE__, 5);
        trace("[0828 GoldenCommerce settlement] terminal room bound "
              "trigger(%s) party_index(%d) dungeon(%d) map(%d) "
              "position(%d,%d) cart_uid(%u)",
              trigger ? trigger : "unknown", session.party_index,
              session.dungeon_index, session.current_map_index,
              current_x, current_y,
              static_cast<unsigned int>(session.cart_unique_id));
    }
    return session.terminal_room_bound;
}

bool HasPendingRewardReport(const PartySession& session) {
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].reward_reported) return true;
    }
    return false;
}

uint8_t PercentFor(uint32_t remaining, uint32_t maximum);

bool BeginSelectionLocked(CParty* party,
                          const GoldenCommerceConfig& config) {
    if (!party || !party->battle_field().GetDungeon()) return false;
    const int dungeon_index = party->battle_field().get_dungeon_index();
    std::map<int, DungeonConfig>::const_iterator dungeon =
        config.dungeons.find(dungeon_index);
    if (dungeon == config.dungeons.end()) return false;

    PartyMemberState current[kOrdinaryPartySeatCount];
    if (!CaptureRoster(party, dungeon->second, current)) return false;
    uint16_t cart_unique_id = 0u;
    uint32_t cart_max_hp = 0u;
    if (!ScanCurrentCart(party, dungeon->second.cart_monster_id,
                         cart_unique_id, cart_max_hp)) return false;
    const uint64_t dungeon_start_tick =
        party->battle_data().dungeon_start_tick();

    PartySessionMap::iterator found = g_sessions.find(party);
    if (found != g_sessions.end() &&
        (found->second.party_index != party->GetPartyIndex() ||
         found->second.dungeon_index != dungeon_index ||
         (dungeon_start_tick != 0u &&
          found->second.dungeon_start_tick != 0u &&
          found->second.dungeon_start_tick != dungeon_start_tick))) {
        // A CParty can be reused without init between consecutive entries.
        // The battle-data dungeon start tick is constant across rooms but is
        // regenerated for a new entry, so it distinguishes a same-dungeon
        // next run without consulting or deleting the detached reward outbox.
        g_sessions.erase(found);
        found = g_sessions.end();
    }
    if (found != g_sessions.end()) {
        PartySession& session = found->second;
        if (!SameRoster(session, current) || session.completed) return false;
        if (session.current_map_index == party->battle_field().get_cur_map() &&
            session.cart_unique_id == cart_unique_id &&
            session.cart_monster_id == dungeon->second.cart_monster_id) {
            return true;
        }
        // NOTI476 owns a map-move object by UID.  The room assembler creates a
        // fresh map_monster each room, but the client must observe one stable
        // cart identity for its locally simulated HP to survive NOTI30 scene
        // transitions.  SendCartBeforeFinishLoadingLocked re-keys the new
        // MapInfo before this bind; any remaining UID drift is a lifecycle bug
        // and must fail closed instead of silently resetting the cart.
        if (session.cart_unique_id != cart_unique_id ||
            session.cart_monster_id != dungeon->second.cart_monster_id) {
            cMyTrace trace("GoldenCommerce0828Session", __LINE__, 5);
            trace("[0828 GoldenCommerce session] cross-map identity drift "
                  "party_index(%d) dungeon(%d) old_map(%d) new_map(%d) "
                  "stable_uid(%u) room_uid(%u)",
                  session.party_index, session.dungeon_index,
                  session.current_map_index,
                  party->battle_field().get_cur_map(),
                  static_cast<unsigned int>(session.cart_unique_id),
                  static_cast<unsigned int>(cart_unique_id));
            return false;
        }
        const uint32_t old_max = session.cart_max_hp;
        const uint32_t old_remaining = session.cart_remaining_hp;
        session.current_map_index = party->battle_field().get_cur_map();
        session.cart_monster_id = dungeon->second.cart_monster_id;
        session.cart_max_hp = cart_max_hp;
        session.cart_remaining_hp = old_max == 0u ? cart_max_hp :
            static_cast<uint32_t>(static_cast<uint64_t>(old_remaining) *
                                  cart_max_hp / old_max);
        if (session.cart_remaining_hp > cart_max_hp) {
            session.cart_remaining_hp = cart_max_hp;
        }
        session.authoritative_percent =
            PercentFor(session.cart_remaining_hp, session.cart_max_hp);
        session.damage_fingerprints.clear();
        return true;
    }

    PartySession candidate;
    candidate.party_index = party->GetPartyIndex();
    candidate.dungeon_index = dungeon_index;
    candidate.opened = true;
    candidate.opened_epoch = NowEpoch();
    candidate.selection_deadline_epoch = candidate.opened_epoch == 0u ? 0u :
        candidate.opened_epoch +
            (config.select_time_ms + 999u) / 1000u +
            kSelectionCommandGraceSeconds;
    candidate.run_token_hi = candidate.opened_epoch == 0u
        ? 1u : candidate.opened_epoch;
    candidate.run_token_lo = NextNonZero(g_next_run_token) ^
        static_cast<uint32_t>(candidate.party_index) ^
        static_cast<uint32_t>(candidate.dungeon_index) ^ cart_unique_id;
    if (candidate.run_token_lo == 0u) {
        candidate.run_token_lo = NextNonZero(g_next_run_token);
    }
    candidate.dungeon_start_tick = dungeon_start_tick;
    candidate.current_map_index = party->battle_field().get_cur_map();
    candidate.cart_unique_id = cart_unique_id;
    candidate.cart_monster_id = dungeon->second.cart_monster_id;
    candidate.cart_max_hp = cart_max_hp;
    candidate.cart_remaining_hp = cart_max_hp;
    candidate.authoritative_percent = 100u;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        candidate.members[seat] = current[seat];
    }

    g_sessions[party] = candidate;
    const int deadline_delay = static_cast<int>(
        (config.select_time_ms + 999u) / 1000u) +
        kSelectionCommandGraceSeconds;
    TimerQueue* const timer_queue = G_TimerQueue();
    if (!timer_queue ||
        timer_queue->InsertTimer(
            1, candidate.party_index, kSelectionDeadlineTimerMessage,
            deadline_delay, candidate.run_token_hi,
            static_cast<int>(candidate.run_token_lo)) != 0) {
        g_sessions.erase(party);
        TraceFailure("selection deadline timer registration failed", 0);
        return false;
    }
    cMyTrace trace("GoldenCommerce0828Session", __LINE__, 5);
    trace("[0828 GoldenCommerce session] bound party_index(%d) dungeon(%d) "
          "map(%d) cart_uid(%u)",
          candidate.party_index, candidate.dungeon_index,
          candidate.current_map_index,
          static_cast<unsigned int>(candidate.cart_unique_id));
    // The picker is opened by the client's local Golden-Commerce state
    // machine.  NOTI480 is the committed-start/close signal and remains
    // withheld until CMD547 or the token-bound deadline wins the transition.
    return true;
}

bool SendCartBeforeFinishLoadingLocked(CParty* party,
                                       const GoldenCommerceConfig& config) {
    if (!party) return false;
    const int dungeon_index = party->battle_field().get_dungeon_index();
    std::map<int, DungeonConfig>::const_iterator dungeon =
        config.dungeons.find(dungeon_index);
    if (dungeon == config.dungeons.end()) {
        // The hook is shared by every dungeon.  Non-Golden rooms have no
        // deferred map-move object and remain untouched.
        return true;
    }

    uint16_t cart_unique_id = 0u;
    uint32_t cart_max_hp = 0u;
    if (!ScanCurrentCart(party, dungeon->second.cart_monster_id,
                         cart_unique_id, cart_max_hp)) {
        cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
        trace("[0828 GoldenCommerce] pre-finish cart scan failed "
              "dungeon(%d) map(%d) monster(%u)",
              dungeon_index, party->battle_field().get_cur_map(),
              static_cast<unsigned int>(dungeon->second.cart_monster_id));
        return false;
    }

    dungeon_runtime::MapMoveMonster projection;
    map_monster* entity = 0;
    const bool projection_ok = dungeon_runtime::GetMapMoveMonsterProjection(
        dungeon_index, 0, &projection);
    const bool entity_ok =
        FindCurrentEntity(party, cart_unique_id, entity) && entity;
    if (!projection_ok || !entity_ok ||
        projection.monster_id != static_cast<int>(
            dungeon->second.cart_monster_id) ||
        entity->monster_id_ != projection.monster_id ||
        projection.level <= 0 || projection.level > 0xffff ||
        projection.position_x < 0 || projection.position_y < 0 ||
        projection.value_18 < 0) {
        cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
        trace("[0828 GoldenCommerce] pre-finish cart projection gate "
              "dungeon(%d) projection(%d) entity(%d) uid(%u) "
              "configured_monster(%u) projection_monster(%d) "
              "entity_monster(%d) projection_level(%d) entity_level(%u) "
              "xyz(%d,%d,%d)",
              dungeon_index,
              projection_ok ? 1 : 0,
              entity_ok ? 1 : 0,
              static_cast<unsigned int>(cart_unique_id),
              static_cast<unsigned int>(dungeon->second.cart_monster_id),
              projection_ok ? projection.monster_id : -1,
              entity_ok ? entity->monster_id_ : -1,
              projection_ok ? projection.level : -1,
              entity_ok ? static_cast<unsigned int>(entity->monster_level_) : 0u,
              projection_ok ? projection.position_x : -1,
              projection_ok ? projection.position_y : -1,
              projection_ok ? projection.value_18 : -1);
        return false;
    }

    PartySessionMap::iterator active = FindCurrentSession(party, config);
    if (active != g_sessions.end() && !active->second.completed) {
        PartySession& session = active->second;
        const uint16_t room_unique_id = cart_unique_id;
        if (session.current_map_index !=
            party->battle_field().get_cur_map()) {
            if (!RebindCurrentCartIdentity(
                    party, dungeon->second.cart_monster_id, room_unique_id,
                    session.cart_unique_id)) {
                cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
                trace("[0828 GoldenCommerce] cross-map cart rekey failed "
                      "party_index(%d) dungeon(%d) map(%d) room_uid(%u) "
                      "stable_uid(%u)",
                      session.party_index, session.dungeon_index,
                      party->battle_field().get_cur_map(),
                      static_cast<unsigned int>(room_unique_id),
                      static_cast<unsigned int>(session.cart_unique_id));
                return false;
            }
            cart_unique_id = session.cart_unique_id;
            entity = 0;
            if (!FindCurrentEntity(party, cart_unique_id, entity) || !entity ||
                entity->monster_id_ != projection.monster_id) {
                return false;
            }
            cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
            trace("[0828 GoldenCommerce] cross-map cart identity preserved "
                  "party_index(%d) dungeon(%d) old_map(%d) new_map(%d) "
                  "room_uid(%u) stable_uid(%u) hp_percent(%u)",
                  session.party_index, session.dungeon_index,
                  session.current_map_index,
                  party->battle_field().get_cur_map(),
                  static_cast<unsigned int>(room_unique_id),
                  static_cast<unsigned int>(session.cart_unique_id),
                  static_cast<unsigned int>(session.authoritative_percent));
        } else if (session.cart_unique_id != cart_unique_id) {
            cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
            trace("[0828 GoldenCommerce] same-map cart identity drift "
                  "party_index(%d) dungeon(%d) map(%d) session_uid(%u) "
                  "room_uid(%u)",
                  session.party_index, session.dungeon_index,
                  session.current_map_index,
                  static_cast<unsigned int>(session.cart_unique_id),
                  static_cast<unsigned int>(cart_unique_id));
            return false;
        }
    }

    // CLIENT_IDA_EVIDENCE: NOTI476 record+0x20 is team, not HP.  Golden carts
    // must be team 0 so team-100 ambush monsters can target and damage them.
    const uint8_t cart_team = 0u;

    // CLIENT_IDA_EVIDENCE: NOTI476/sub_4F0A30 only queues a 36-byte record.
    // The following NOTI30 transition calls 13A9220 -> 4F12D0 -> 4F10B0,
    // which creates the monster and inserts this uid into the map-move
    // controller.  Sending after NOTI30 leaves the record dormant until a
    // later scene transition and is therefore incorrect.
    if (!monster_move_system::SendBootstrap(
            party,
            cart_unique_id,
            dungeon->second.cart_monster_id,
            static_cast<uint16_t>(projection.level),
            static_cast<uint32_t>(projection.position_x),
            static_cast<uint32_t>(projection.position_y),
            static_cast<uint32_t>(projection.value_18),
            cart_team)) {
        return false;
    }
    // Bind the authoritative run while the exact cart/roster that produced
    // NOTI476 is still present.  Waiting until after NOTI30 made the session
    // depend on a later lifecycle observation and left CMD546/CMD547 with no
    // registry entry even though the cart bootstrap had succeeded.
    if (!BeginSelectionLocked(party, config)) {
        cMyTrace session_trace("GoldenCommerce0828Session", __LINE__, 5);
        session_trace("[0828 GoldenCommerce session] pre-finish bind failed "
                      "party_index(%d) dungeon(%d) map(%d) cart_uid(%u)",
                      party->GetPartyIndex(), dungeon_index,
                      party->battle_field().get_cur_map(),
                      static_cast<unsigned int>(cart_unique_id));
        return false;
    }
    // This is the last server-owned point before NOTI30/onStartMap can retire
    // or transform room entities.  Persist the final-room stable-cart binding
    // independently of the terminal CMD541 latch: either event may arrive
    // first.  Once both facts and the one-shot CMD548 report are present, the
    // common rendezvous completes the run even if the live cart is no longer
    // discoverable after the map-start transition.
    PartySessionMap::iterator bound_session =
        FindBasicSession(party, config);
    if (bound_session != g_sessions.end()) {
        (void)LatchTerminalRoomBoundLocked(
            party, dungeon->second, bound_session->second,
            "PRE_FINISH_LOADING");
    }
    cMyTrace trace("GoldenCommerce0828Bootstrap", __LINE__, 5);
    trace("[0828 GoldenCommerce] pre-finish cart bootstrap queued "
          "dungeon(%d) map(%d) "
          "uid(%u) monster(%u) level(%d) xyz(%d,%d,%d) team(%u)",
          dungeon_index,
          party->battle_field().get_cur_map(),
          static_cast<unsigned int>(cart_unique_id),
          static_cast<unsigned int>(dungeon->second.cart_monster_id),
          projection.level,
          projection.position_x,
          projection.position_y,
          projection.value_18,
          static_cast<unsigned int>(cart_team));
    return true;
}

uint8_t PercentFor(uint32_t remaining, uint32_t maximum) {
    if (maximum == 0u) return 0u;
    const uint64_t percent = static_cast<uint64_t>(remaining) * 100u / maximum;
    return static_cast<uint8_t>(percent > 100u ? 100u : percent);
}

bool PercentReportsAgree(uint8_t left, uint8_t right) {
    const int delta = static_cast<int>(left) - static_cast<int>(right);
    return delta >= -1 && delta <= 1;
}

bool RecordRewardReportLocked(PartySession& session,
                              int seat,
                              uint8_t client_percent) {
    if (seat < 0 || seat >= kOrdinaryPartySeatCount ||
        session.members[seat].unique_id == kInvalidMemberUniqueId) {
        return false;
    }
    PartyMemberState& member = session.members[seat];
    if (member.reward_reported) {
        // A replay from the same member is idempotent only when the exact
        // one-byte report is unchanged.
        return member.reported_percent == client_percent;
    }
    for (int other = 0; other < kOrdinaryPartySeatCount; ++other) {
        if (other == seat || !session.members[other].reward_reported) continue;
        if (!PercentReportsAgree(session.members[other].reported_percent,
                                 client_percent)) {
            return false;
        }
    }
    member.reported_percent = client_percent;
    member.reward_reported = true;
    return true;
}

bool ApplyRewardReportLocked(PartySession& session,
                             uint8_t client_percent) {
    if (!session.final_percent_latched) {
        session.authoritative_percent = client_percent;
        session.cart_remaining_hp = client_percent == 0u ? 0u :
            static_cast<uint32_t>(
                (static_cast<uint64_t>(session.cart_max_hp) *
                     client_percent + 99u) / 100u);
        session.final_percent_latched = true;
        return true;
    }
    return PercentReportsAgree(session.authoritative_percent,
                               client_percent);
}

bool SendPacketToCapturedRoster(CParty* party,
                                const PartySession& session,
                                PacketGuard& packet) {
    int expected = 0;
    int sent = 0;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id == kInvalidMemberUniqueId) continue;
        ++expected;
        CUser* const member = party ? party->get_user(seat) : 0;
        if (member && member->GetParty() == party &&
            member->get_unique_id() == session.members[seat].unique_id &&
            member->get_acc_id() ==
                static_cast<int>(session.members[seat].account_id) &&
            member->get_charac_no(-1) ==
                static_cast<int>(session.members[seat].charac_no)) {
            // The native CUser::Send ABI does not provide a reliable delivery
            // result.  A packet can already be queued/observed by the client
            // while the legacy return value is zero, so count a validated send
            // attempt rather than treating that return value as delivery.
            member->Send(packet);
            ++sent;
        }
    }
    return expected > 0 && sent == expected;
}

bool SendStartNotification(CParty* party, const PartySession& session) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GOLDEN_COMMERCE_START);
    packet.finalize(true);
    return SendPacketToCapturedRoster(party, session, packet);
}

bool SendSelectionSnapshot(CParty* party, const PartySession& session) {
    int selected_count = 0;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id != kInvalidMemberUniqueId &&
            session.members[seat].selected_item_id != kNoSelection) {
            ++selected_count;
        }
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_LOADING_CART);
    packet.put_byte(selected_count);
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id == kInvalidMemberUniqueId ||
            session.members[seat].selected_item_id == kNoSelection) continue;
        // CLIENT_IDA_EVIDENCE: sub_644A00 reads u16 member uid + u32 item.
        packet.put_short(session.members[seat].unique_id);
        packet.put_int(session.members[seat].selected_item_id);
    }
    packet.finalize(true);
    return SendPacketToCapturedRoster(party, session, packet);
}

bool FindUniqueFreeChoice(const GoldenCommerceConfig& config,
                          const DungeonConfig& dungeon,
                          int& item_id) {
    item_id = 0;
    for (std::vector<int>::const_iterator allowed =
             dungeon.allowed_item_ids.begin();
         allowed != dungeon.allowed_item_ids.end(); ++allowed) {
        std::map<int, CargoConfig>::const_iterator cargo =
            config.cargo.find(*allowed);
        if (cargo == config.cargo.end()) return false;
        if (!cargo->second.free_choice) continue;
        if (cargo->second.fee != 0 || item_id != 0) return false;
        item_id = cargo->first;
    }
    return item_id > 0;
}

bool FinalizeSelectionDeadlineLocked(CParty* party,
                                     PartySession& session,
                                     const GoldenCommerceConfig& config) {
    if (!party || !session.opened || session.started || session.completed ||
        !CartStillBound(party, session)) return false;
    std::map<int, DungeonConfig>::const_iterator dungeon =
        config.dungeons.find(session.dungeon_index);
    int free_item_id = 0;
    if (dungeon == config.dungeons.end() ||
        !FindUniqueFreeChoice(config, dungeon->second, free_item_id)) {
        return false;
    }
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id == kInvalidMemberUniqueId ||
            session.members[seat].selected_item_id != kNoSelection) continue;
        // Timeout fallback is deliberately free and configuration-driven.  It
        // executes no cargo-debit path and never overwrites a paid choice.
        session.members[seat].selected_item_id = free_item_id;
    }
    session.selection_deadline_epoch = 0u;
    return SendSelectionSnapshot(party, session);
}

bool FinalizeStartLocked(CParty* party, PartySession& session) {
    if (!party || !session.opened || session.completed ||
        !CartStillBound(party, session)) return false;
    // The UI command and the token-bound timer share this transition while
    // holding g_golden_commerce_mutex.  Whichever arrives second observes the
    // completed transition instead of starting the run twice.
    if (session.started) return true;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        if (session.members[seat].unique_id != kInvalidMemberUniqueId &&
            session.members[seat].selected_item_id == kNoSelection) {
            return false;
        }
    }
    session.selection_deadline_epoch = 0u;
    session.started = true;
    return true;
}

bool SendRewardNotification(CUser* user,
                            const RewardClaimMessage& settled) {
    if (!user) return false;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GOLDEN_COMMERCE_REWARD);
    // CLIENT_IDA_EVIDENCE: sub_5F5DD0 consumes u32 gold, u8 percentage,
    // u32 opaque result value.  The final field is a server receipt only.
    packet.put_int(settled.reward_gold);
    packet.put_byte(settled.authoritative_percent);
    packet.put_int(settled.server_result_token);
    packet.finalize(true);
    return user->Send(packet) > 0;
}

void SendContextError(CUser* user,
                      ENUM_CMDPACKET command,
                      const char* reason) {
    cMyTrace trace("GoldenCommerce0828Reject", __LINE__, 5);
    trace("[0828 GoldenCommerce reject] command(%d) reason(%s)",
          static_cast<int>(command), reason ? reason : "state");
    if (user) user->SendCmdErrorPacket(command, kInvalidContextError);
}

bool CommitCargoTransaction(CUser* user,
                            const CargoConfig& cargo,
                            uint8_t& error) {
    error = kInvalidContextError;
    if (!user) return false;
    if (cargo.free_choice) return true;

    const CInventory* const readable = user->getCurCharacInvenR();
    CInventory* const writable = user->getCurCharacInvenW();
    if (!readable || !writable) return false;
    const int item_slot = readable->check_item_exist(cargo.item_id);
    if (item_slot <= 0) {
        error = kMissingItemError;
        return false;
    }
    const Inven_Item item = readable->GetInvenSlot(kInventoryType, item_slot);
    if (item.isEmpty() || item.getKey() != cargo.item_id ||
        item.get_add_info() < 1 ||
        user->CheckItemLock(kInventoryType, item_slot)) {
        error = kMissingItemError;
        return false;
    }
    if (readable->get_money() < cargo.fee) {
        error = kInsufficientFeeError;
        return false;
    }

    InventoryMemory shadow_memory;
    CInventory shadow;
    shadow.SetInventoryMemory(&shadow_memory);
    if (!shadow.setCopy(*readable) ||
        shadow.delete_item(static_cast<INVEN_TYPE>(kInventoryType), item_slot, 1,
                           static_cast<eItemDelReason>(
                               kGoldenCommerceItemDelReason), false) != 1) {
        return false;
    }

    const int money_before = writable->get_money();
    if (cargo.fee > 0 &&
        writable->use_money(cargo.fee,
                            static_cast<eMoneySubReason>(
                                kGoldenCommerceMoneySubReason), true) != 1) {
        error = kInsufficientFeeError;
        return false;
    }
    if (writable->delete_item(static_cast<INVEN_TYPE>(kInventoryType),
                              item_slot, 1,
                              static_cast<eItemDelReason>(
                                  kGoldenCommerceItemDelReason), true) != 1) {
        if (cargo.fee > 0) {
            (void)writable->gain_money(
                cargo.fee, static_cast<eMoneyAddReason>(kRollbackMoneyAddReason),
                true, 0);
            if (writable->get_money() != money_before) {
                writable->set_money(static_cast<unsigned int>(money_before));
                cMyTrace trace("golden_commerce_0828::CommitCargoTransaction",
                               __LINE__, 5);
                trace("fee rollback repaired uid(%d) fee(%d)",
                      user->get_unique_id(), cargo.fee);
            }
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0), 0);
        }
        error = kMissingItemError;
        return false;
    }
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0), item_slot);
    if (cargo.fee > 0) {
        user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                 static_cast<ENUM_ITEMSPACE>(0), 0);
    }
    return true;
}

void CopySession(const PartySession& session, PartySnapshot& snapshot) {
    snapshot.party_index = session.party_index;
    snapshot.dungeon_index = session.dungeon_index;
    snapshot.opened = session.opened;
    snapshot.started = session.started;
    snapshot.completed = session.completed;
    snapshot.terminal_reached = session.terminal_reached;
    snapshot.terminal_room_bound = session.terminal_room_bound;
    snapshot.clear_seen = session.clear_seen;
    snapshot.terminal_grid_x = session.terminal_grid_x;
    snapshot.terminal_grid_y = session.terminal_grid_y;
    snapshot.cart_unique_id = session.cart_unique_id;
    snapshot.cart_monster_id = session.cart_monster_id;
    snapshot.cart_max_hp = session.cart_max_hp;
    snapshot.cart_remaining_hp = session.cart_remaining_hp;
    snapshot.authoritative_percent = session.authoritative_percent;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        snapshot.member_unique_ids[seat] = session.members[seat].unique_id;
        snapshot.selected_item_ids[seat] =
            session.members[seat].selected_item_id;
    }
}

bool IsCurrentMember(CParty* party,
                     int seat,
                     const PartyMemberState& snapshot,
                     CUser*& user) {
    user = 0;
    if (!party || seat < 0 || seat >= kOrdinaryPartySeatCount ||
        snapshot.unique_id == kInvalidMemberUniqueId) return false;
    CUser* const current = party->get_user(seat);
    if (!current || current->GetParty() != party ||
        current->get_unique_id() != snapshot.unique_id ||
        current->get_acc_id() != static_cast<int>(snapshot.account_id) ||
        current->get_charac_no(-1) != static_cast<int>(snapshot.charac_no) ||
        current->GetServerGroup() != snapshot.server_group) return false;
    user = current;
    return true;
}

bool ClaimsEqual(const RewardClaimMessage& expected,
                 const RewardClaimMessage& observed) {
    return expected.account_id == observed.account_id &&
           expected.charac_no == observed.charac_no &&
           expected.run_token_hi == observed.run_token_hi &&
           expected.run_token_lo == observed.run_token_lo &&
           expected.request_token == observed.request_token &&
           expected.cargo_item_id == observed.cargo_item_id &&
           expected.cargo_base_value == observed.cargo_base_value &&
           expected.reward_gold == observed.reward_gold &&
           expected.server_result_token == observed.server_result_token &&
           expected.cart_monster_id == observed.cart_monster_id &&
           expected.dungeon_index == observed.dungeon_index &&
           expected.server_group == observed.server_group &&
           expected.member_unique_id == observed.member_unique_id &&
           expected.cart_unique_id == observed.cart_unique_id &&
           expected.operation == observed.operation &&
           expected.authoritative_percent ==
               observed.authoritative_percent &&
           expected.rank_index == observed.rank_index;
}

ClaimOutboxKey ClaimKeyFor(const RewardClaimMessage& request) {
    ClaimOutboxKey key;
    key.server_group = request.server_group;
    key.run_token_hi = request.run_token_hi;
    key.run_token_lo = request.run_token_lo;
    key.request_token = request.request_token;
    key.charac_no = request.charac_no;
    return key;
}

bool ClaimMatchesSessionMember(const RewardClaimMessage& request,
                               const PartySession& session,
                               const PartyMemberState& member) {
    const uint32_t cargo_item_id =
        static_cast<uint32_t>(member.selected_item_id);
    const uint32_t cargo_base_value = OfficialCargoBaseValue(cargo_item_id);
    return member.selected_item_id != kNoSelection &&
           request.account_id == member.account_id &&
           request.charac_no == member.charac_no &&
           request.run_token_hi == session.run_token_hi &&
           request.run_token_lo == session.run_token_lo &&
           request.request_token != 0u &&
           request.cargo_item_id == cargo_item_id &&
           request.cargo_base_value == cargo_base_value &&
           request.reward_gold == RewardGoldFor(
               cargo_base_value, session.authoritative_percent) &&
           request.server_result_token == ResultTokenFor(
               session.run_token_lo, member.charac_no, cargo_item_id,
               session.authoritative_percent) &&
           request.cart_monster_id == session.cart_monster_id &&
           request.dungeon_index == session.dungeon_index &&
           request.server_group == member.server_group &&
           request.member_unique_id == member.unique_id &&
           request.cart_unique_id == session.cart_unique_id &&
           request.operation == REWARD_OPERATION_CLAIM &&
           request.authoritative_percent == session.authoritative_percent &&
           request.rank_index ==
               RankIndexForPercent(session.authoritative_percent) &&
           request.result_code == REWARD_RESULT_SUCCESS &&
           request.claim_state == REWARD_CLAIM_PENDING &&
           request.replayed == 0u &&
           request.delivery_key_hi == 0u &&
           request.delivery_key_lo == 0u;
}

ClaimOutboxMap::iterator FindClaimForMemberLocked(
    ClaimOutboxMap& outbox,
    const PartySession& session,
    const PartyMemberState& member,
    bool& duplicate) {
    duplicate = false;
    ClaimOutboxMap::iterator found = outbox.end();
    for (ClaimOutboxMap::iterator claim = outbox.begin();
         claim != outbox.end(); ++claim) {
        if (claim->first.server_group != member.server_group ||
            claim->first.run_token_hi != session.run_token_hi ||
            claim->first.run_token_lo != session.run_token_lo ||
            claim->first.charac_no != member.charac_no) {
            continue;
        }
        if (found != outbox.end()) {
            duplicate = true;
            return outbox.end();
        }
        found = claim;
    }
    return found;
}

CUser* FindOnlineClaimUser(const RewardClaimMessage& request) {
    CGameManager* const manager = G_CGameManager();
    if (!manager || request.account_id == 0u || request.charac_no == 0u) {
        return 0;
    }
    CUser* const user = manager->GetUserByAccId(request.account_id);
    if (!user ||
        user->get_acc_id() != static_cast<int>(request.account_id) ||
        user->get_charac_no(-1) != static_cast<int>(request.charac_no) ||
        user->GetServerGroup() != request.server_group ||
        user->GetUID() < 0 ||
        manager->getUser(user->GetUID(), 0) != user) {
        return 0;
    }
    return user;
}

// CLIENT_IDA: NOTI481/sub_5F5DD0 only caches the payout.  Result-screen
// initialization belongs to NOTI34/sub_582DA0 and NOTI35/sub_58F0F0.  Publish
// their result grammar after delivery, without entering the normal dungeon
// reward generator or granting the already-mailed gold/monster EXP again.
// Use the same clear-EXP inputs and bonus calculator as CParty::clear_reward.
// Monster EXP (including the 1.7 Golden-Commerce multiplier) is a separate
// award and is deliberately not used as this base or granted again here.
bool CalculateClearExperience(CParty* party, CUser* user, uint8_t rank,
                              ClearExperienceSnapshot& out) {
    CDungeon* const dungeon = party ? party->battle_field().GetDungeon() : 0;
    if (!party || !user || !dungeon || party->get_member_count() <= 0 ||
        OfficialCartForDungeon(dungeon->get_index()) == 0u) return false;
    const int killed = party->battle_data().GetTotalKilledMonsterCount();
    if (killed <= 0) return true;

    const int members = party->get_member_count();
    const int level = dungeon->get_standard_level();
    unsigned int mob_reward = 0u;
    G_CDataManager()->get_mob_reward(level, mob_reward);
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(24);
    const long double event_weight = event
        ? static_cast<CPartyExpBonusEvent*>(event)->getBonusRate(
              members, party->isHelpAbuseParty() != 0) : 0.0L;
    ServerParameterScript* const script = G_CDataManager()->get_server_parameter_script();
    const long double party_weight = script->GetClearDungeonPartyExpWeight(members) + event_weight;
    const long double difficulty_weight = script->GetClearDungeonDifficultyExpWeight(
        party->battle_field().get_dungeon_diff());
    const long double raw_base = static_cast<long double>(killed) * mob_reward /
        2.0L * party_weight * difficulty_weight * dungeon->get_exp_weight() *
        party->MemberPenaltyRate() * G_CDataManager()->BaseExpPenalty(
            user->get_charac_level(), level) / members;
    if (!(raw_base >= 0.0L &&
          raw_base <= static_cast<long double>(std::numeric_limits<int>::max()))) return false;

    unsigned int base = static_cast<unsigned int>(raw_base);
    QuickParty::RewardData_Param random_parameter(party);
    QuickParty::RewardData_Result random_result;
    random_result.raw_00 = base;
    party->set_random_reward(static_cast<QuickParty::RandomBuffType>(0),
                             random_parameter, random_result);
    base = random_result.raw_00;
    stClearRewardExpParameter_t parameter;
    stClearRewardBonusExp_t bonus;
    if (party->set_clear_reward_exp_parameter(user, base, parameter) != 1) return false;
    // Golden Commerce ranks come from the validated settlement, not a stale
    // ordinary-dungeon GameResultSet.
    parameter.rank_bonus_index = G_CDataManager()->getRankBonusIndex(rank);
    parameter.rank_bonus_rate = static_cast<float>(
        script->GetExpBonusWeight(parameter.rank_bonus_index));
    uint32_t total = party->getClearRewardBonusExp(user, base, parameter, bonus);
    const int champion_rate = champion_break::GetClearExpBonusRate(user);
    const uint64_t champion = champion_rate > 0
        ? static_cast<uint64_t>(base) * static_cast<unsigned int>(champion_rate) / 100u : 0u;
    const uint64_t full_total = static_cast<uint64_t>(total) + champion;
    if (full_total > static_cast<uint64_t>(std::numeric_limits<int>::max())) return false;
    out.total = static_cast<uint32_t>(full_total);
    out.event_kind = static_cast<uint8_t>(bonus.event_bonus_type);
    out.fields[0] = base + bonus.reserved_bonus_exp;
    out.fields[1] = bonus.rank_bonus_exp;
    int party_bonus = party_weight != 0.0L
        ? static_cast<int>(base) - static_cast<int>(base / party_weight) : 0;
    if (base != 0u) {
        const uint64_t scale = static_cast<uint64_t>(bonus.premium_advantage_bonus_exp) +
            bonus.server_event_bonus_exp + bonus.reserved_bonus_exp +
            bonus.burning_fatigue_bonus_exp + bonus.event_bonus_exp +
            bonus.avatar_bonus_exp + bonus.rank_bonus_exp + bonus.channel_bonus_exp;
        party_bonus *= static_cast<int>(scale / base) + 1;
    }
    out.fields[2] = party_bonus > 0 ? static_cast<uint32_t>(party_bonus) : 0u;
    out.fields[3] = bonus.avatar_bonus_exp;
    out.fields[4] = bonus.event_bonus_exp;
    out.fields[5] = bonus.period_exp_item_bonus_exp;
    out.fields[6] = bonus.premium_advantage_bonus_exp;
    out.fields[7] = bonus.same_pc_room_party_bonus_exp;
    out.fields[8] = bonus.channel_bonus_exp;
    out.fields[9] = bonus.upper_member_bonus_exp;
    out.fields[10] = bonus.creature_bonus_exp;
    out.fields[11] = bonus.return_user_party_bonus_exp;
    out.fields[12] = bonus.new_character_party_bonus_exp;
    out.fields[13] = bonus.character_manage_bonus_exp;
    out.fields[14] = bonus.growth_power_bonus_exp;
    out.fields[15] = bonus.premium_exp_bonus_exp;
    out.fields[16] = bonus.server_event_bonus_exp;
    out.fields[18] = bonus.burning_fatigue_bonus_exp;
    out.fields[19] = static_cast<uint32_t>(champion);
    // Guild/power-war additions have no dedicated slot in this reader.
    // Keep them in the base/residual display without losing or paying them twice.
    uint64_t displayed = out.fields[0];
    for (int index = 1; index < 21; ++index) {
        if (index != 2) displayed += out.fields[index]; // party is a subcategory of base
    }
    if (displayed > out.total) return false;
    out.fields[0] += out.total - static_cast<uint32_t>(displayed);
    return true;
}

bool GetOrAwardClearExperience(CParty* party, CUser* user,
                               const RewardClaimMessage& settled, int seat,
                               ClearExperienceSnapshot& out) {
    if (!party || !user || seat < 0 || seat >= kOrdinaryPartySeatCount) return false;
    const auto matches = [&](const PartySession& session) {
        return session.completed && session.run_token_hi == settled.run_token_hi &&
            session.run_token_lo == settled.run_token_lo &&
            session.dungeon_index == settled.dungeon_index &&
            session.cart_unique_id == settled.cart_unique_id &&
            session.members[seat].charac_no == settled.charac_no &&
            party->get_user(seat) == user;
    };
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        PartySessionMap::iterator found = g_sessions.find(party);
        if (found == g_sessions.end() || !matches(found->second)) return false;
        if (found->second.members[seat].clear_exp_awarded) {
            out = found->second.members[seat].clear_exp;
            return true;
        }
    }
    ClearExperienceSnapshot computed;
    if (!CalculateClearExperience(party, user, settled.rank_index, computed)) return false;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        PartySessionMap::iterator found = g_sessions.find(party);
        if (found == g_sessions.end() || !matches(found->second)) return false;
        PartyMemberState& member = found->second.members[seat];
        if (member.clear_exp_awarded) {
            out = member.clear_exp;
            return true;
        }
        member.clear_exp = computed;
        // Reserve once before the award/notifications. UI send retries reuse
        // this snapshot, never recompute bonuses or award EXP a second time.
        member.clear_exp_awarded = true;
    }
    out = computed;
    if (out.total != 0u) {
        user->setChattingMessageCount(0);
        user->calcHelpAbuseRatio(out.total, party->isHelpAbuseParty(user) != 0);
        user->reset_level_before_dungeon();
        int sp = 0, sfp = 0;
        if (user->gain_exp_sp(static_cast<int>(out.total), sp, sfp,
                              static_cast<eExpAddReason>(2), 0, false)) {
            PacketGuard level_packet;
            level_packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
            level_packet.put_byte(1);
            level_packet.put_short(1);
            if (user->make_basic_info(level_packet, 1) == 1) {
                level_packet.finalize(true);
                user->Send(level_packet);
            }
        }
    }
    cMyTrace trace("GoldenCommerce0828ClearExp", __LINE__, 5);
    trace("[0828 GoldenCommerce clear_exp] charac(%u) base(%u) rank(%u) avatar(%u) black_diamond(%u) total(%u)",
          settled.charac_no, out.fields[0], out.fields[1], out.fields[3],
          out.fields[6], out.total);
    return true;
}

void ShowDeliveredResult(CUser* user, const RewardClaimMessage& settled) {
    CParty* const party = user ? user->GetParty() : 0;
    if (!party || user->get_state() != kDungeonUserState) return;
    bool finish_party = false;
    int shown_seat = -1;
    uint64_t start_tick = 0u;
    uint32_t awarded_monster_exp = 0u;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        PartySessionMap::iterator found = g_sessions.find(party);
        if (found == g_sessions.end()) return;
        PartySession& session = found->second;
        if (!session.completed || !session.terminal_room_bound ||
            session.run_token_hi != settled.run_token_hi ||
            session.run_token_lo != settled.run_token_lo ||
            session.dungeon_index != settled.dungeon_index ||
            session.cart_unique_id != settled.cart_unique_id ||
            party->battle_field().get_dungeon_index() != session.dungeon_index ||
            party->battle_field().GetCurrentMapIndex() != session.current_map_index) {
            return;
        }
        const int seat = FindMemberSeat(session, user);
        if (seat < 0 || party->get_user(seat) != user ||
            session.members[seat].charac_no != settled.charac_no ||
            session.members[seat].result_ui_sent) return;
        session.members[seat].result_ui_sent = true;
        shown_seat = seat;
        awarded_monster_exp = session.members[seat].awarded_monster_exp;
        start_tick = session.dungeon_start_tick;
        bool all_online_shown = true;
        for (int index = 0; index < kOrdinaryPartySeatCount; ++index) {
            CUser* member = 0;
            if (IsCurrentMember(party, index, session.members[index], member) &&
                member && !session.members[index].result_ui_sent) {
                all_online_shown = false;
            }
        }
        if (all_online_shown && !session.result_ui_complete) {
            session.result_ui_complete = true;
            finish_party = true;
        }
    }
    const auto release_ui_reservation = [&]() {
        MutexGuard guard(&g_golden_commerce_mutex);
        PartySessionMap::iterator found = g_sessions.find(party);
        if (found != g_sessions.end() &&
            found->second.run_token_hi == settled.run_token_hi &&
            found->second.run_token_lo == settled.run_token_lo) {
            found->second.members[shown_seat].result_ui_sent = false;
            if (finish_party) found->second.result_ui_complete = false;
        }
    };
    const uint32_t now_tick = static_cast<uint32_t>(
        GlobalData::system_time()->getCurTickCount());
    const uint32_t elapsed = now_tick - static_cast<uint32_t>(start_tick);
    user->setPlayingDungeonIndex(-1);
    PacketGuard result;
    result.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_PLAY_RESULT);
    result.put_byte(settled.rank_index);
    result.put_int(elapsed);
    result.put_byte(0);
    result.put_byte(0);
    party->makeBestClearTimePacket(result, elapsed);
    result.finalize(true);
    if (user->Send(result) <= 0) {
        release_ui_reservation();
        return;
    }

    PacketGuard reward;
    reward.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_CLEAR_DUNGEON_REWARD);
    ClearExperienceSnapshot clear_exp;
    if (!GetOrAwardClearExperience(party, user, settled, shown_seat, clear_exp)) {
        release_ui_reservation();
        return;
    }
    for (int index = 0; index < 4; ++index) reward.put_int(clear_exp.fields[index]);
    reward.put_byte(clear_exp.event_kind);
    for (int index = 4; index < 21; ++index) reward.put_int(clear_exp.fields[index]);
    reward.put_byte(0);  // quest items
    // NOTI35's first eight rows are presentation records.  Show the delivered
    // payout in this recipient's row, while leaving every BattleData reward
    // pair empty so normal card completion cannot grant a second payment.
    for (int index = 0; index < 8; ++index) {
        if (index == shown_seat && settled.reward_gold != 0u) {
            reward.put_byte(1);
            reward.put_int(0); // gold, not an item template
            reward.put_int(settled.reward_gold);
        } else {
            reward.put_byte(0);
        }
    }
    reward.put_int(0);   // paid-card cost
    for (int index = 0; index < 16; ++index) reward.put_byte(0);
    reward.put_int(0);   // hell-party percentage
    reward.put_byte(0);
    reward.put_byte(0);
    reward.put_byte(0);
    reward.finalize(true);
    if (user->Send(reward) <= 0) {
        release_ui_reservation();
        return;
    }

    if (finish_party) {
        party->battle_data().dungeon_playing_time() = elapsed;
        party->set_dungeon_clear_state(2);
        party->SetEPLPState(2);
        party->set_state(1);
        const unsigned int key = static_cast<unsigned int>(
            party->gen_timer_key(TIMER_MESSAGE_EPLP_COMMAND_WAIT));
        G_TimerQueue()->InsertTimer(1, party->GetPartyIndex(),
            TIMER_MESSAGE_EPLP_COMMAND_WAIT, 60, key, 0);
    }
    cMyTrace trace("GoldenCommerce0828Result", __LINE__, 5);
    trace("[0828 GoldenCommerce result] ui_ready charac(%u) dungeon(%d) "
          "gold(%u) percent(%u) party_complete(%d) reported_monster_exp(%u)",
          settled.charac_no, settled.dungeon_index, settled.reward_gold,
          static_cast<unsigned int>(settled.authoritative_percent),
          finish_party ? 1 : 0, awarded_monster_exp);
}

bool BuildClaimLocked(CUser* user,
                      PartySession& session,
                      ClaimOutboxMap& outbox,
                      int seat,
                      ClaimDispatch& dispatch) {
    if (!user || seat < 0 || seat >= kOrdinaryPartySeatCount ||
        !session.completed) return false;
    PartyMemberState& member = session.members[seat];
    if (member.selected_item_id == kNoSelection) return false;
    const uint32_t now = NowEpoch();
    bool duplicate = false;
    ClaimOutboxMap::iterator existing = FindClaimForMemberLocked(
        outbox, session, member, duplicate);
    if (duplicate) return false;
    if (existing != outbox.end()) {
        ClaimOutboxEntry& entry = existing->second;
        if (!ClaimMatchesSessionMember(entry.request, session, member)) {
            return false;
        }
        dispatch.key = existing->first;
        if (entry.state == LOCAL_CLAIM_DELIVERED) {
            dispatch.replay = true;
            dispatch.request = entry.settled;
            return true;
        }
        if (entry.state == LOCAL_CLAIM_QUARANTINED ||
            (entry.state == LOCAL_CLAIM_QUEUED &&
             (entry.next_retry_epoch == 0u ||
              (now != 0u && now < entry.next_retry_epoch)))) {
            return false;
        }
        entry.state = LOCAL_CLAIM_QUEUED;
        entry.next_retry_epoch = now == 0u
            ? static_cast<uint32_t>(kClaimRetrySeconds)
            : now + kClaimRetrySeconds;
        dispatch.request = entry.request;
        return true;
    }

    ClaimOutboxEntry entry;
    entry.request.account_id = member.account_id;
    entry.request.charac_no = member.charac_no;
    entry.request.run_token_hi = session.run_token_hi;
    entry.request.run_token_lo = session.run_token_lo;
    entry.request.request_token = NextNonZero(g_next_request_token);
    entry.request.cargo_item_id =
        static_cast<uint32_t>(member.selected_item_id);
    entry.request.cargo_base_value =
        OfficialCargoBaseValue(entry.request.cargo_item_id);
    entry.request.reward_gold = RewardGoldFor(
        entry.request.cargo_base_value, session.authoritative_percent);
    entry.request.server_result_token = ResultTokenFor(
        session.run_token_lo, member.charac_no,
        entry.request.cargo_item_id, session.authoritative_percent);
    entry.request.cart_monster_id = session.cart_monster_id;
    entry.request.dungeon_index = session.dungeon_index;
    entry.request.server_group = member.server_group;
    // This is only the first route hint.  Every queue attempt rewrites a
    // stack-local copy to the currently validated online UID; it is never an
    // entitlement identity and is deliberately excluded from ClaimsEqual.
    entry.request.reply_user_idx = user->GetUID();
    entry.request.member_unique_id = member.unique_id;
    entry.request.cart_unique_id = session.cart_unique_id;
    entry.request.operation = REWARD_OPERATION_CLAIM;
    entry.request.authoritative_percent = session.authoritative_percent;
    entry.request.rank_index =
        RankIndexForPercent(session.authoritative_percent);
    entry.request.result_code = REWARD_RESULT_SUCCESS;
    entry.request.claim_state = REWARD_CLAIM_PENDING;
    entry.state = LOCAL_CLAIM_QUEUED;
    entry.next_retry_epoch = now == 0u
        ? static_cast<uint32_t>(kClaimRetrySeconds)
        : now + kClaimRetrySeconds;

    const ClaimOutboxKey key = ClaimKeyFor(entry.request);
    if (outbox.find(key) != outbox.end()) return false;
    const std::pair<ClaimOutboxMap::iterator, bool> inserted =
        outbox.insert(std::make_pair(key, entry));
    if (!inserted.second) return false;
    dispatch.key = inserted.first->first;
    dispatch.request = inserted.first->second.request;
    return true;
}

void MarkClaimQueueFailure(const ClaimOutboxKey& key) {
    MutexGuard guard(&g_golden_commerce_mutex);
    ClaimOutboxMap::iterator found = g_claim_outbox.find(key);
    if (found == g_claim_outbox.end() ||
        found->second.state == LOCAL_CLAIM_DELIVERED ||
        found->second.state == LOCAL_CLAIM_QUARANTINED) return;
    found->second.state = LOCAL_CLAIM_RETRY;
    const uint32_t now = NowEpoch();
    found->second.next_retry_epoch = now == 0u
        ? static_cast<uint32_t>(kClaimRetrySeconds)
        : now + kClaimRetrySeconds;
}

bool SubmitClaim(const RewardClaimMessage& request, CUser*& routed_user) {
    routed_user = FindOnlineClaimUser(request);
    if (!routed_user) return false;
    RewardClaimMessage routed(request);
    routed.reply_user_idx = routed_user->GetUID();
    return DBGoldenCommerceReward::QueueRequest(
        routed.reply_user_idx, routed);
}

bool CompleteSessionAndCollectClaimsLocked(
    CParty* party,
    PartySession& session,
    const char* trigger,
    int final_report_seat,
    uint8_t final_report_percent,
    std::vector<ClaimDispatch>& dispatches) {
    if (!party || !session.started || session.completed) return false;

    // Completion is a transaction over the in-memory run.  A malformed or
    // conflicting final report, stale roster, or claim-build failure must not
    // leave completed=true with no claim path.  Work on a deep value copy and
    // publish it only after every validation below succeeds.
    PartySession candidate(session);
    ClaimOutboxMap candidate_outbox(g_claim_outbox);
    if (final_report_seat >= 0 &&
        !RecordRewardReportLocked(candidate, final_report_seat,
                                  final_report_percent)) {
        return false;
    }
    candidate.authoritative_percent =
        PercentFor(candidate.cart_remaining_hp, candidate.cart_max_hp);
    candidate.final_percent_latched = false;
    candidate.completed = true;
    candidate.completed_epoch = NowEpoch();

    int report_count = 0;
    std::vector<ClaimDispatch> candidate_dispatches;
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        PartyMemberState& member = candidate.members[seat];
        if (!member.reward_reported) continue;
        ++report_count;
        if (!ApplyRewardReportLocked(candidate, member.reported_percent)) {
            TraceFailure("pending CMD548 percentage mismatch", 0);
            return false;
        }
        CUser* current = 0;
        if (!IsCurrentMember(party, seat, member, current) || !current) {
            return false;
        }
        ClaimDispatch dispatch;
        if (!BuildClaimLocked(current, candidate, candidate_outbox, seat,
                              dispatch)) {
            return false;
        }
        candidate_dispatches.push_back(dispatch);
    }

    session = candidate;
    g_claim_outbox.swap(candidate_outbox);
    dispatches.insert(dispatches.end(), candidate_dispatches.begin(),
                      candidate_dispatches.end());

    cMyTrace trace("GoldenCommerce0828Settlement", __LINE__, 5);
    trace("[0828 GoldenCommerce settlement] completed trigger(%s) "
          "party_index(%d) dungeon(%d) map(%d) cart_uid(%u) "
          "percent(%u) reports(%d) claims(%u)",
          trigger ? trigger : "unknown", session.party_index,
          session.dungeon_index, session.current_map_index,
          static_cast<unsigned int>(session.cart_unique_id),
          static_cast<unsigned int>(session.authoritative_percent),
          report_count, static_cast<unsigned int>(dispatches.size()));
    return true;
}

bool TryCompleteTerminalSessionLocked(
    CParty* party,
    const DungeonConfig& dungeon,
    PartySession& session,
    const char* trigger,
    std::vector<ClaimDispatch>& dispatches) {
    // Settlement is deliberately a three-part rendezvous:
    //   1. a validated directional CMD541 committed the configured endpoint;
    //   2. the server instantiated that endpoint as the boss room and rebound
    //      the stable cart identity there; and
    //   3. at least one captured member supplied the official one-byte CMD548
    //      cart-health report.
    // None of those inputs is retried by the 0828 client, so every ordering
    // converges here while ordinary-room reports remain unable to complete a
    // run.
    if (!party || session.completed ||
        !HasTerminalMovementLocked(dungeon, session) ||
        !session.terminal_room_bound ||
        !HasPendingRewardReport(session)) {
        return false;
    }
    return CompleteSessionAndCollectClaimsLocked(
        party, session, trigger, -1, 0u, dispatches);
}

bool CollectCompletedMemberClaimLocked(
    CUser* user,
    PartySession& session,
    int seat,
    uint8_t client_percent,
    std::vector<ClaimDispatch>& dispatches) {
    if (!user || !session.completed) return false;
    PartySession candidate(session);
    ClaimOutboxMap candidate_outbox(g_claim_outbox);
    if (!RecordRewardReportLocked(candidate, seat, client_percent) ||
        !ApplyRewardReportLocked(candidate, client_percent)) {
        return false;
    }
    ClaimDispatch dispatch;
    if (!BuildClaimLocked(user, candidate, candidate_outbox, seat,
                          dispatch)) {
        return false;
    }
    session = candidate;
    g_claim_outbox.swap(candidate_outbox);
    dispatches.push_back(dispatch);
    return true;
}

void DispatchClaims(const std::vector<ClaimDispatch>& dispatches,
                    const char* failure_reason) {
    for (std::vector<ClaimDispatch>::const_iterator dispatch =
             dispatches.begin();
        dispatch != dispatches.end(); ++dispatch) {
        if (dispatch->replay) {
            CUser* const current = FindOnlineClaimUser(dispatch->request);
            if (current) {
                if (SendRewardNotification(current, dispatch->request)) {
                    ShowDeliveredResult(current, dispatch->request);
                }
            }
        } else {
            CUser* routed_user = 0;
            if (SubmitClaim(dispatch->request, routed_user)) continue;
            MarkClaimQueueFailure(dispatch->key);
            TraceFailure(failure_reason, routed_user);
        }
    }
}

}  // namespace

TimerSelectionDeadline::TimerSelectionDeadline() : TimerSig() {}

TimerSelectionDeadline::~TimerSelectionDeadline() {}

int TimerSelectionDeadline::dispatch_sig(int party_index,
                                         int run_token_hi,
                                         unsigned int run_token_lo) {
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) return 0;

    CParty* party = 0;
    for (PartySessionMap::iterator session = g_sessions.begin();
         session != g_sessions.end(); ++session) {
        if (session->second.party_index == party_index &&
            session->second.run_token_hi ==
                static_cast<uint32_t>(run_token_hi) &&
            session->second.run_token_lo == run_token_lo) {
            party = session->first;
            break;
        }
    }
    if (!party) return 0;

    PartySessionMap::iterator current = FindCurrentSession(party, *config);
    if (current == g_sessions.end()) return 0;
    if (current->second.party_index != party_index ||
        current->second.run_token_hi != static_cast<uint32_t>(run_token_hi) ||
        current->second.run_token_lo != run_token_lo ||
        current->second.completed) return 0;
    if (current->second.started) {
        // Idempotent recovery for the race where CMD547 committed just before
        // this deadline callback but its close notification was not observed.
        (void)SendStartNotification(party, current->second);
        return 1;
    }
    if (!FinalizeSelectionDeadlineLocked(party, current->second, *config) ||
        !FinalizeStartLocked(party, current->second)) {
        TraceFailure("selection deadline auto-start failed", 0);
        return 0;
    }
    // CLIENT_IDA_EVIDENCE: NOTI480/sub_6433B0 closes picker UI 494.  It must
    // follow the committed start transition; sending it during selection races
    // the client's local UI creation and leaves the picker stuck on screen.
    if (!SendStartNotification(party, current->second)) {
        TraceFailure("selection deadline start notification failed", 0);
    }
    return 1;
}

PartySnapshot::PartySnapshot()
    : party_index(-1), dungeon_index(-1), opened(false), started(false),
      completed(false), terminal_reached(false),
      terminal_room_bound(false), clear_seen(false), terminal_grid_x(0u),
      terminal_grid_y(0u), cart_unique_id(0u), cart_monster_id(0u),
      cart_max_hp(0u), cart_remaining_hp(0u), authoritative_percent(0u) {
    for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
        member_unique_ids[seat] = kInvalidMemberUniqueId;
        selected_item_ids[seat] = kNoSelection;
    }
}

bool SendCartBeforeFinishLoading(CParty* party) {
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    return party && config &&
           SendCartBeforeFinishLoadingLocked(party, *config);
}

bool BeginSelection(CParty* party) {
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    return party && config && BeginSelectionLocked(party, *config);
}

bool RecoverMissingSessionForCommand(CParty* party,
                                     ENUM_CMDPACKET command) {
    if (!party) return false;
    bool session_missing = false;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const GoldenCommerceConfig* const config = GetConfig();
        if (!config) return false;
        const char* reason = 0;
        PartySessionMap::iterator found =
            FindCurrentSession(party, *config, &reason);
        if (found != g_sessions.end()) return true;
        session_missing = reason && std::strcmp(reason, "session_missing") == 0;
    }
    if (!session_missing) return false;

    // BeginSelection takes the same mutex internally, so recovery must occur
    // after the probe guard has been released.  It is idempotent for an
    // already-created session and safely covers a late MapInfo/cart bind.
    const bool recovered = BeginSelection(party);
    cMyTrace trace("GoldenCommerce0828Session", __LINE__, 5);
    trace("[0828 GoldenCommerce session] command(%d) lazy_recovery(%d) "
          "party_index(%d) dungeon(%d) map(%d)",
          static_cast<int>(command), recovered ? 1 : 0,
          party->GetPartyIndex(), party->battle_field().get_dungeon_index(),
          party->battle_field().get_cur_map());
    return recovered;
}

bool OnMapLoaded(CParty* party) {
    // The cart was already queued immediately before NOTI30 and materialized
    // by that scene transition.  Besides binding the new room, this hook also
    // drains the only CMD548 report when it raced the final map bind.
    std::vector<ClaimDispatch> dispatches;
    bool bound = false;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const GoldenCommerceConfig* const config = GetConfig();
        if (!party || !config) return false;
        bound = BeginSelectionLocked(party, *config);
        // PRE_FINISH_LOADING may already have persisted the final-room/cart
        // fact before NOTI30/onStartMap retired the live entity.  In that
        // ordering BeginSelectionLocked can no longer rescan the cart, but the
        // one-shot CMD548 must still be drained from the run that owns the
        // same party/dungeon token.  Therefore completion is not gated by the
        // transient post-start rebind result.
        PartySessionMap::iterator found =
            FindCurrentRunSessionNoRoster(party, *config);
        if (found != g_sessions.end()) {
            std::map<int, DungeonConfig>::const_iterator dungeon =
                config->dungeons.find(found->second.dungeon_index);
            if (dungeon != config->dungeons.end()) {
                if (bound) {
                    (void)LatchTerminalRoomBoundLocked(
                        party, dungeon->second, found->second,
                        "ON_MAP_LOADED");
                }
                (void)TryCompleteTerminalSessionLocked(
                    party, dungeon->second, found->second,
                    "FINAL_MAP_BIND", dispatches);
            }
        }
    }
    DispatchClaims(dispatches, "reward DB queue failed after final map bind");
    return bound;
}

bool AuthorizeCartMovement(CParty* party, uint16_t entity_unique_id) {
    if (!party || entity_unique_id == 0u) return true;
    map_monster* entity = 0;
    if (!FindCurrentEntity(party, entity_unique_id, entity) || !entity) {
        return true;
    }
    const int dungeon_index = party->battle_field().get_dungeon_index();
    const uint32_t official_cart = OfficialCartForDungeon(dungeon_index);
    if (official_cart == 0u ||
        entity->monster_id_ != static_cast<int32_t>(official_cart)) return true;

    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) return false;
    PartySessionMap::iterator found = FindCurrentSession(party, *config);
    return found != g_sessions.end() && found->second.started &&
           !found->second.completed &&
           found->second.cart_unique_id == entity_unique_id &&
           found->second.cart_monster_id == official_cart &&
           found->second.cart_remaining_hp > 0u &&
           CartStillBound(party, found->second);
}

void OnCartMovementCommitted(CParty* party,
                             uint16_t entity_unique_id,
                             uint8_t grid_x,
                             uint8_t grid_y,
                             uint8_t action) {
    // Only a directional CMD541 (actions 0..3) proves movement.  Action 5 is
    // a bootstrap record and must never arm completion merely because the
    // final room was instantiated.
    if (!party || entity_unique_id == 0u || action > 3u) return;

    std::vector<ClaimDispatch> dispatches;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const GoldenCommerceConfig* const config = GetConfig();
        if (!config) return;
        PartySessionMap::iterator found = FindBasicSession(party, *config);
        if (found == g_sessions.end()) return;
        PartySession& session = found->second;
        std::map<int, DungeonConfig>::const_iterator dungeon =
            config->dungeons.find(session.dungeon_index);
        const uint64_t current_start_tick =
            party->battle_data().dungeon_start_tick();
        if (dungeon == config->dungeons.end() || !session.started ||
            session.completed || session.cart_unique_id != entity_unique_id ||
            session.cart_remaining_hp == 0u ||
            dungeon->second.boss_x < 0 || dungeon->second.boss_x > 0xff ||
            dungeon->second.boss_y < 0 || dungeon->second.boss_y > 0xff ||
            (current_start_tick != 0u && session.dungeon_start_tick != 0u &&
             current_start_tick != session.dungeon_start_tick) ||
            grid_x != static_cast<uint8_t>(dungeon->second.boss_x) ||
            grid_y != static_cast<uint8_t>(dungeon->second.boss_y)) {
            return;
        }

        session.terminal_reached = true;
        session.terminal_grid_x = grid_x;
        session.terminal_grid_y = grid_y;
        cMyTrace trace("GoldenCommerce0828Settlement", __LINE__, 5);
        trace("[0828 GoldenCommerce settlement] terminal movement latched "
              "party_index(%d) dungeon(%d) cart_uid(%u) grid(%u,%u) "
              "action(%u) pending_reports(%d)",
              session.party_index, session.dungeon_index,
              static_cast<unsigned int>(session.cart_unique_id),
              static_cast<unsigned int>(grid_x),
              static_cast<unsigned int>(grid_y),
              static_cast<unsigned int>(action),
              HasPendingRewardReport(session) ? 1 : 0);

        (void)LatchTerminalRoomBoundLocked(
            party, dungeon->second, session, "CMD541_TERMINAL");
        (void)TryCompleteTerminalSessionLocked(
            party, dungeon->second, session, "CMD541_TERMINAL", dispatches);
    }
    DispatchClaims(dispatches, "reward DB queue failed after terminal move");
}

bool ObserveValidatedCartDamage(CParty* party,
                                uint16_t entity_unique_id,
                                uint32_t authoritative_damage,
                                uint64_t request_fingerprint) {
    if (!party || entity_unique_id == 0u || authoritative_damage == 0u ||
        request_fingerprint == 0u) return false;
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) return false;
    PartySessionMap::iterator found = FindCurrentSession(party, *config);
    if (found == g_sessions.end()) return false;
    PartySession& session = found->second;
    if (!session.started || session.completed ||
        session.cart_unique_id != entity_unique_id ||
        !CartStillBound(party, session) || session.cart_remaining_hp == 0u ||
        session.damage_fingerprints.size() >=
            kMaximumDamageFingerprintsPerMap ||
        !session.damage_fingerprints.insert(request_fingerprint).second) {
        return false;
    }
    session.cart_remaining_hp = authoritative_damage >= session.cart_remaining_hp
        ? 0u : session.cart_remaining_hp - authoritative_damage;
    session.authoritative_percent =
        PercentFor(session.cart_remaining_hp, session.cart_max_hp);
    return true;
}

bool OnPartyDungeonResult(CParty* party,
                          int dungeon_index,
                          bool cleared) {
    if (OfficialCartForDungeon(dungeon_index) == 0u) return false;
    // All official Golden-Commerce dungeons are owned even if configuration
    // or session validation fails; never fall through to ordinary rewards.
    if (!party) return true;

    std::vector<ClaimDispatch> dispatches;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const GoldenCommerceConfig* const config = GetConfig();
        if (!config ||
            party->battle_field().get_dungeon_index() != dungeon_index) {
            return true;
        }
        PartySessionMap::iterator found = FindBasicSession(party, *config);
        if (found == g_sessions.end()) return true;
        PartySession& session = found->second;
        if (session.completed) return true;
        std::map<int, DungeonConfig>::const_iterator dungeon =
            config->dungeons.find(dungeon_index);
        const uint64_t current_start_tick =
            party->battle_data().dungeon_start_tick();
        if (!cleared || dungeon == config->dungeons.end() ||
            (current_start_tick != 0u && session.dungeon_start_tick != 0u &&
             current_start_tick != session.dungeon_start_tick)) {
            return true;
        }
        (void)LatchTerminalRoomBoundLocked(
            party, dungeon->second, session, "CMD49");

        // CMD49 is emitted after the server has accepted the room clear.  The
        // map-start/clear path may already have retired the cart entity, so a
        // strict live-cart lookup can legitimately miss here.  Treat the
        // server-owned clear as a fallback for the *room binding* fact only
        // when the same run has already committed the exact terminal CMD541
        // grid and the battle field is still the configured boss endpoint.
        // This cannot manufacture the movement fact or a CMD548 report.
        CBattle_Field& field = party->battle_field();
        int current_x = -1;
        int current_y = -1;
        field.getCurPosXY(current_x, current_y);
        const bool terminal_movement =
            HasTerminalMovementLocked(dungeon->second, session);
        const bool clear_endpoint =
            session.started && !session.completed && terminal_movement &&
            field.get_dungeon_index() == session.dungeon_index &&
            session.current_map_index == field.get_cur_map() &&
            current_x == dungeon->second.boss_x &&
            current_y == dungeon->second.boss_y && party->checkBossRoom();
        if (!terminal_movement || !clear_endpoint) {
            cMyTrace deferred("GoldenCommerce0828Settlement", __LINE__, 5);
            deferred("[0828 GoldenCommerce settlement] CMD49 deferred "
                     "party_index(%d) dungeon(%d) terminal_step(%d) "
                     "terminal_room_bound(%d) clear_endpoint(%d) "
                     "position(%d,%d) expected(%d,%d) pending_report(%d)",
                     session.party_index, session.dungeon_index,
                     terminal_movement ? 1 : 0,
                     session.terminal_room_bound ? 1 : 0,
                     clear_endpoint ? 1 : 0, current_x, current_y,
                     dungeon->second.boss_x, dungeon->second.boss_y,
                     HasPendingRewardReport(session) ? 1 : 0);
            return true;
        }
        if (!session.terminal_room_bound) {
            session.terminal_room_bound = true;
            cMyTrace fallback("GoldenCommerce0828Settlement", __LINE__, 5);
            fallback(
                "[0828 GoldenCommerce settlement] terminal room bound "
                "trigger(CMD49_CLEAR_FALLBACK) party_index(%d) dungeon(%d) "
                "map(%d) position(%d,%d) cart_uid(%u)",
                session.party_index, session.dungeon_index,
                session.current_map_index, current_x, current_y,
                static_cast<unsigned int>(session.cart_unique_id));
        }
        session.clear_seen = true;
        // CMD49 is only an ordering-independent clear latch.  The 0828 client
        // sends the authoritative cart percentage in CMD548 and marks that
        // send one-shot before transmission; completing here with zero reports
        // would erase the only later claim path.
        (void)TryCompleteTerminalSessionLocked(
            party, dungeon->second, session, "CMD49", dispatches);
    }
    DispatchClaims(dispatches, "reward DB queue failed after clear");
    return true;
}

unsigned int ScaleMonsterExperience(CParty* party,
                                    CUser* user,
                                    unsigned int experience) {
    if (!party || !user || experience == 0u) return experience;
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) return experience;
    PartySessionMap::iterator found = FindCurrentSession(party, *config);
    if (found == g_sessions.end() || !found->second.started ||
        found->second.completed || FindMemberSeat(found->second, user) < 0) {
        return experience;
    }
    const uint64_t scaled = static_cast<uint64_t>(experience) *
                            config->exp_rate_permyriad / 10000u;
    return scaled > std::numeric_limits<unsigned int>::max()
        ? std::numeric_limits<unsigned int>::max()
        : static_cast<unsigned int>(scaled);
}

void RecordMonsterExperienceAwarded(CParty* party, CUser* user,
                                    unsigned int experience) {
    if (!party || !user || experience == 0u ||
        OfficialCartForDungeon(party->battle_field().get_dungeon_index()) == 0u) {
        return;
    }
    MutexGuard guard(&g_golden_commerce_mutex);
    PartySessionMap::iterator found = g_sessions.find(party);
    if (found == g_sessions.end() || !found->second.started ||
        found->second.completed || found->second.dungeon_index !=
            party->battle_field().get_dungeon_index()) return;
    const int seat = FindMemberSeat(found->second, user);
    if (seat < 0 || party->get_user(seat) != user) return;
    uint32_t& total = found->second.members[seat].awarded_monster_exp;
    const uint64_t sum = static_cast<uint64_t>(total) + experience;
    total = sum > static_cast<uint64_t>(std::numeric_limits<int>::max())
        ? static_cast<uint32_t>(std::numeric_limits<int>::max())
        : static_cast<uint32_t>(sum);
}

void OnPeriodicTick(uint32_t now_epoch) {
    if (now_epoch == 0u) return;
    struct Retry {
        Retry() : key(), request() {
            std::memset(&request, 0, sizeof(request));
        }
        ClaimOutboxKey key;
        RewardClaimMessage request;
    };
    std::vector<Retry> retries;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        for (PartySessionMap::iterator session = g_sessions.begin();
             session != g_sessions.end();) {
            const uint32_t base = session->second.completed
                ? session->second.completed_epoch : session->second.opened_epoch;
            const uint32_t lifetime = session->second.completed
                ? kCompletedLifetimeSeconds : kSessionLifetimeSeconds;
            const bool expired = base != 0u && now_epoch > base &&
                                 now_epoch - base > lifetime;
            // PartySession is only the live run/UI state.  Reward durability
            // belongs to g_claim_outbox, so pointer reuse or ordinary session
            // expiry can no longer erase an unacknowledged entitlement.
            if (expired) {
                session = g_sessions.erase(session);
                continue;
            }
            ++session;
        }

        for (ClaimOutboxMap::iterator claim = g_claim_outbox.begin();
             claim != g_claim_outbox.end();) {
            ClaimOutboxEntry& entry = claim->second;
            if (entry.state == LOCAL_CLAIM_DELIVERED) {
                const bool acknowledged_expired =
                    entry.acknowledged_epoch != 0u &&
                    now_epoch > entry.acknowledged_epoch &&
                    now_epoch - entry.acknowledged_epoch >
                        kAcknowledgedClaimLifetimeSeconds;
                if (acknowledged_expired) {
                    ClaimOutboxMap::iterator acknowledged = claim++;
                    g_claim_outbox.erase(acknowledged);
                    continue;
                }
                ++claim;
                continue;
            }
            if (entry.state == LOCAL_CLAIM_QUARANTINED ||
                (entry.state != LOCAL_CLAIM_QUEUED &&
                 entry.state != LOCAL_CLAIM_RETRY) ||
                (entry.next_retry_epoch != 0u &&
                 now_epoch < entry.next_retry_epoch)) {
                ++claim;
                continue;
            }
            Retry retry;
            retry.key = claim->first;
            retry.request = entry.request;
            entry.state = LOCAL_CLAIM_QUEUED;
            entry.next_retry_epoch = now_epoch + kClaimRetrySeconds;
            retries.push_back(retry);
            ++claim;
        }
    }
    for (std::vector<Retry>::const_iterator retry = retries.begin();
         retry != retries.end(); ++retry) {
        CUser* routed_user = 0;
        if (!SubmitClaim(retry->request, routed_user)) {
            MarkClaimQueueFailure(retry->key);
        }
    }
}

void OnUserDisconnect(CUser* user) {
    if (!user) return;
    MutexGuard guard(&g_golden_commerce_mutex);
    for (PartySessionMap::iterator session = g_sessions.begin();
         session != g_sessions.end();) {
        bool matched = false;
        for (int seat = 0; seat < kOrdinaryPartySeatCount; ++seat) {
            if (session->second.members[seat].account_id ==
                    static_cast<uint32_t>(user->get_acc_id()) &&
                session->second.members[seat].charac_no ==
                    static_cast<uint32_t>(user->get_charac_no(-1)) &&
                session->second.members[seat].unique_id ==
                    static_cast<uint16_t>(user->get_unique_id())) {
                matched = true;
                break;
            }
        }
        if (matched && !session->second.completed) {
            session = g_sessions.erase(session);
        } else {
            ++session;
        }
    }
}

void ResetParty(CParty* party) {
    if (!party) return;
    MutexGuard guard(&g_golden_commerce_mutex);
    PartySessionMap::iterator found = g_sessions.find(party);
    if (found != g_sessions.end() && !found->second.completed) {
        g_sessions.erase(found);
    }
}

void ReleaseParty(CParty* party) {
    if (!party) return;
    MutexGuard guard(&g_golden_commerce_mutex);
    g_sessions.erase(party);
}

bool CopySnapshot(CParty* party, PartySnapshot& snapshot) {
    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!party || !config) return false;
    PartySessionMap::iterator found = FindBasicSession(party, *config);
    if (found == g_sessions.end()) return false;
    CopySession(found->second, snapshot);
    return true;
}

int HandleLoadingCart(CUser* user, PacketBuf& packet) {
    unsigned int wire_item_id = 0u;
    if (packet.get_int(wire_item_id) != 1 || wire_item_id == 0u ||
        wire_item_id > static_cast<unsigned int>(
            std::numeric_limits<int>::max())) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                         "item_not_allowed");
        return 0;
    }
    CParty* const party = user ? user->GetParty() : 0;
    if (!user || !party || user->get_state() != kDungeonUserState) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "state");
        return 0;
    }
    (void)RecoverMissingSessionForCommand(
        party, ENUM_CMDPACKET_LOADING_CART);

    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "state");
        return 0;
    }
    const char* session_reason = 0;
    PartySessionMap::iterator found =
        FindCurrentSession(party, *config, &session_reason);
    if (found == g_sessions.end()) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                         session_reason ? session_reason : "session_missing");
        return 0;
    }
    if (!found->second.opened || found->second.completed) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "state");
        return 0;
    }
    if (!CartStillBound(party, found->second)) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "cart");
        return 0;
    }
    PartySession& session = found->second;
    const int seat = FindMemberSeat(session, user);
    if (seat < 0 || party->get_user(seat) != user) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "roster");
        return 0;
    }
    const int item_id = static_cast<int>(wire_item_id);
    PartyMemberState& member = session.members[seat];
    if (session.started) {
        // Timer/CMD547 may win the mutex after this exact choice was already
        // accepted.  A duplicate acknowledgement is read-only and therefore
        // never enters the cargo-debit path or charges the selection twice.
        if (member.selected_item_id != item_id) {
            (void)SendStartNotification(party, session);
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                             "started_conflict");
            return 0;
        }
        user->SendCmdOkPacket(ENUM_CMDPACKET_LOADING_CART);
        SendSelectionSnapshot(party, session);
        (void)SendStartNotification(party, session);
        return 0;
    }
    const uint32_t now = NowEpoch();
    if (session.selection_deadline_epoch != 0u &&
        now > session.selection_deadline_epoch) {
        // The client hides its picker before knowing whether CMD546 succeeded,
        // and error 23 is silent there.  Close that dead-end deterministically:
        // preserve every accepted choice, fill only missing seats with the
        // unique configured free cargo, resend the complete NOTI479, and
        // commit the same idempotent start transition as CMD547.
        if (!FinalizeSelectionDeadlineLocked(party, session, *config)) {
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "deadline");
            return 0;
        }
        if (!FinalizeStartLocked(party, session)) {
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                             "selection_incomplete");
            return 0;
        }
        if (member.selected_item_id != item_id) {
            (void)SendStartNotification(party, session);
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                             "started_conflict");
            return 0;
        }
        user->SendCmdOkPacket(ENUM_CMDPACKET_LOADING_CART);
        (void)SendStartNotification(party, session);
        return 0;
    }
    if (member.selected_item_id != kNoSelection) {
        if (member.selected_item_id != item_id) {
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                             "item_not_allowed");
            return 0;
        }
        user->SendCmdOkPacket(ENUM_CMDPACKET_LOADING_CART);
        SendSelectionSnapshot(party, session);
        return 0;
    }
    std::map<int, DungeonConfig>::const_iterator dungeon =
        config->dungeons.find(session.dungeon_index);
    std::map<int, CargoConfig>::const_iterator cargo =
        config->cargo.find(item_id);
    if (dungeon == config->dungeons.end() || cargo == config->cargo.end() ||
        !IsCargoAllowedInDungeon(session.dungeon_index, wire_item_id) ||
        std::find(dungeon->second.allowed_item_ids.begin(),
                  dungeon->second.allowed_item_ids.end(), item_id) ==
            dungeon->second.allowed_item_ids.end()) {
        SendContextError(user, ENUM_CMDPACKET_LOADING_CART,
                         "item_not_allowed");
        return 0;
    }
    uint8_t error = kInvalidContextError;
    if (!CommitCargoTransaction(user, cargo->second, error)) {
        if (error == kInvalidContextError) {
            SendContextError(user, ENUM_CMDPACKET_LOADING_CART, "state");
        } else {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_LOADING_CART, error);
        }
        return 0;
    }
    member.selected_item_id = item_id;
    user->SendCmdOkPacket(ENUM_CMDPACKET_LOADING_CART);
    SendSelectionSnapshot(party, session);
    return 0;
}

int HandleStart(CUser* user, PacketBuf& packet) {
    (void)packet;
    CParty* const party = user ? user->GetParty() : 0;
    if (!user || !party || user->get_state() != kDungeonUserState) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START, "state");
        return 0;
    }
    (void)RecoverMissingSessionForCommand(
        party, ENUM_CMDPACKET_GOLDEN_COMMERCE_START);

    MutexGuard guard(&g_golden_commerce_mutex);
    const GoldenCommerceConfig* const config = GetConfig();
    if (!config) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START, "state");
        return 0;
    }
    const char* session_reason = 0;
    PartySessionMap::iterator found =
        FindCurrentSession(party, *config, &session_reason);
    if (found == g_sessions.end()) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START,
                         session_reason ? session_reason : "session_missing");
        return 0;
    }
    if (!found->second.opened || found->second.completed) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START, "state");
        return 0;
    }
    if (party->getManager() != user) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START,
                         "manager");
        return 0;
    }
    if (!CartStillBound(party, found->second)) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START, "cart");
        return 0;
    }
    PartySession& session = found->second;
    if (!FinalizeStartLocked(party, session)) {
        SendContextError(user, ENUM_CMDPACKET_GOLDEN_COMMERCE_START,
                         "selection_incomplete");
        return 0;
    }
    // CLIENT_IDA_EVIDENCE: CMD547 is empty; response handler is nullsub_6.
    user->SendCmdOkPacket(ENUM_CMDPACKET_GOLDEN_COMMERCE_START);
    // CLIENT_IDA_EVIDENCE: NOTI480/sub_6433B0 removes UI type 494.  The client
    // does not close the outer picker from the CMD547 response handler.
    (void)SendStartNotification(party, session);
    return 0;
}

int HandleReward(CUser* user, PacketBuf& packet) {
    uint8_t client_percent = 0u;
    // CLIENT_IDA_EVIDENCE: sub_5F5D00 writes exactly one byte.  CMD548 has no
    // same-command response handler, so malformed/auth failures are silent.
    if (packet.get_byte(client_percent) != 1 || client_percent > 100u) {
        TraceFailure("malformed CMD548", user);
        return 0;
    }

    std::vector<ClaimDispatch> dispatches;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const GoldenCommerceConfig* const config = GetConfig();
        CParty* const party = user ? user->GetParty() : 0;
        if (!user || !config || !party) return 0;
        PartySessionMap::iterator found = FindBasicSession(party, *config);
        if (found == g_sessions.end()) {
            TraceFailure("CMD548 session missing", user);
            return 0;
        }
        PartySession& session = found->second;
        const int seat = FindMemberSeat(session, user);
        if (!session.started || seat < 0 || party->get_user(seat) != user) {
            TraceFailure("CMD548 state/roster mismatch", user);
            return 0;
        }
        const uint64_t current_start_tick =
            party->battle_data().dungeon_start_tick();
        if (current_start_tick != 0u && session.dungeon_start_tick != 0u &&
            current_start_tick != session.dungeon_start_tick) {
            TraceFailure("CMD548 stale run token", user);
            return 0;
        }
        // Client event 0x11B/CMD548 is the only Golden-Commerce report that
        // carries current/max cart HP.  NPC-to-cart damage and the PVF virtual
        // shield are simulated locally and do not pass through the player's
        // SDC damage packets.  The sender sets a one-shot flag before sending,
        // so an endpoint report may be queued but must never be discarded when
        // the final map/roster bind is one callback behind.  Ordinary-room
        // reports remain rejected and cannot poison the terminal percentage.
        if (!session.completed) {
            std::map<int, DungeonConfig>::const_iterator dungeon =
                config->dungeons.find(session.dungeon_index);
            if (dungeon == config->dungeons.end()) {
                TraceFailure("CMD548 dungeon policy missing", user);
                return 0;
            }
            int current_x = -1;
            int current_y = -1;
            bool boss_room = false;
            bool cart_bound = false;
            const bool live_terminal = IsTerminalCartStateLocked(
                party, dungeon->second, session, current_x, current_y,
                boss_room, cart_bound);
            if (live_terminal) {
                (void)LatchTerminalRoomBoundLocked(
                    party, dungeon->second, session, "CMD548");
            }
            const bool movement_terminal =
                HasTerminalMovementLocked(dungeon->second, session);
            if (!movement_terminal && !session.terminal_room_bound) {
                cMyTrace rejected(
                    "GoldenCommerce0828Settlement", __LINE__, 5);
                rejected(
                    "[0828 GoldenCommerce settlement] CMD548 rejected "
                    "before terminal evidence party_index(%d) dungeon(%d) "
                    "seat(%d) percent(%u) map(%d) position(%d,%d) "
                    "expected(%d,%d) terminal_step(%d) "
                    "terminal_room_bound(%d) boss_room(%d) cart_bound(%d)",
                    session.party_index, session.dungeon_index, seat,
                    static_cast<unsigned int>(client_percent),
                    party->battle_field().get_cur_map(), current_x, current_y,
                    dungeon->second.boss_x, dungeon->second.boss_y,
                    movement_terminal ? 1 : 0,
                    session.terminal_room_bound ? 1 : 0,
                    boss_room ? 1 : 0, cart_bound ? 1 : 0);
                return 0;
            }

            // Store the one-shot report atomically before attempting the
            // terminal rendezvous.  This validates only the captured sender
            // and run identity above; requiring every other client to have
            // left its transient loading state would recreate the original
            // race and discard CMD548 permanently.
            PartySession candidate(session);
            if (!RecordRewardReportLocked(candidate, seat, client_percent)) {
                TraceFailure("CMD548 pending percentage mismatch", user);
                return 0;
            }
            session = candidate;
            const bool completed = TryCompleteTerminalSessionLocked(
                party, dungeon->second, session, "CMD548_TERMINAL",
                dispatches);
            if (!completed) {
                cMyTrace trace("GoldenCommerce0828Settlement", __LINE__, 5);
                trace("[0828 GoldenCommerce settlement] CMD548 queued "
                      "party_index(%d) dungeon(%d) seat(%d) percent(%u) "
                      "map(%d) position(%d,%d) expected(%d,%d) "
                      "terminal_step(%d) terminal_grid(%u,%u) "
                      "terminal_room_bound(%d) clear_seen(%d) "
                      "boss_room(%d) cart_bound(%d)",
                      session.party_index, session.dungeon_index, seat,
                      static_cast<unsigned int>(client_percent),
                      party->battle_field().get_cur_map(),
                      current_x, current_y,
                      dungeon->second.boss_x, dungeon->second.boss_y,
                      session.terminal_reached ? 1 : 0,
                      static_cast<unsigned int>(session.terminal_grid_x),
                      static_cast<unsigned int>(session.terminal_grid_y),
                      session.terminal_room_bound ? 1 : 0,
                      session.clear_seen ? 1 : 0,
                      boss_room ? 1 : 0, cart_bound ? 1 : 0);
            }
        } else {
            // A completed session accepts a member report/claim as one atomic
            // value-copy transaction.  A replay mismatch or claim-build
            // failure cannot partially mutate report/claim state.
            if (!CollectCompletedMemberClaimLocked(
                    user, session, seat, client_percent, dispatches)) {
                TraceFailure("CMD548 percentage mismatch", user);
                return 0;
            }
        }
    }
    DispatchClaims(dispatches, "reward DB queue failed");
    return 0;
}

void OnRewardClaimResult(CUser* user,
                         const RewardClaimMessage& response) {
    (void)user;
    if (response.operation != REWARD_OPERATION_CLAIM) return;
    RewardClaimMessage settled;
    std::memset(&settled, 0, sizeof(settled));
    bool notify = false;
    bool quarantined = false;
    {
        MutexGuard guard(&g_golden_commerce_mutex);
        const ClaimOutboxKey key = ClaimKeyFor(response);
        ClaimOutboxMap::iterator found = g_claim_outbox.find(key);
        if (found == g_claim_outbox.end() ||
            !ClaimsEqual(found->second.request, response)) {
            return;
        }
        ClaimOutboxEntry& entry = found->second;
        if ((response.result_code == REWARD_RESULT_SUCCESS ||
             response.result_code == REWARD_RESULT_ALREADY_DELIVERED) &&
            response.claim_state == REWARD_CLAIM_DELIVERED &&
            (response.delivery_key_hi != 0u ||
             response.delivery_key_lo != 0u)) {
            entry.settled = response;
            entry.state = LOCAL_CLAIM_DELIVERED;
            entry.next_retry_epoch = 0u;
            entry.acknowledged_epoch = NowEpoch();
            settled = response;
            notify = true;
        } else if (response.result_code == REWARD_RESULT_DATABASE) {
            entry.state = LOCAL_CLAIM_RETRY;
            const uint32_t now = NowEpoch();
            entry.next_retry_epoch = now == 0u
                ? static_cast<uint32_t>(kClaimRetrySeconds)
                : now + kClaimRetrySeconds;
        } else {
            // Identity/policy/collision failures are not transient.  Keep the
            // immutable claim for diagnosis, but never busy-loop or overwrite
            // it with a different run/member payload.
            entry.state = LOCAL_CLAIM_QUARANTINED;
            entry.next_retry_epoch = 0u;
            quarantined = true;
        }
    }
    CUser* const current = notify ? FindOnlineClaimUser(settled) : 0;
    if (current) {
        if (SendRewardNotification(current, settled)) {
            ShowDeliveredResult(current, settled);
        }
    }
    if (quarantined) {
        CUser* const owner = FindOnlineClaimUser(response);
        TraceFailure("reward claim quarantined", owner);
    }
}

}  // namespace golden_commerce_0828

Dispatcher_LoadingCart::Dispatcher_LoadingCart()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_LoadingCart::~Dispatcher_LoadingCart() {}
int Dispatcher_LoadingCart::dispatch_sig(CUser* user, PacketBuf& packet) {
    return golden_commerce::HandleLoadingCart(user, packet);
}
int Dispatcher_LoadingCart::read(PacketBuf&, MSG_BASE&) { return 0; }
int Dispatcher_LoadingCart::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

Dispatcher_GoldenCommerceStart::Dispatcher_GoldenCommerceStart()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_GoldenCommerceStart::~Dispatcher_GoldenCommerceStart() {}
int Dispatcher_GoldenCommerceStart::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    return golden_commerce::HandleStart(user, packet);
}
int Dispatcher_GoldenCommerceStart::read(PacketBuf&, MSG_BASE&) { return 0; }
int Dispatcher_GoldenCommerceStart::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

Dispatcher_GoldenCommerceReward::Dispatcher_GoldenCommerceReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
Dispatcher_GoldenCommerceReward::~Dispatcher_GoldenCommerceReward() {}
int Dispatcher_GoldenCommerceReward::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    return golden_commerce::HandleReward(user, packet);
}
int Dispatcher_GoldenCommerceReward::read(PacketBuf&, MSG_BASE&) {
    return 0;
}
int Dispatcher_GoldenCommerceReward::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

InterGoldenCommerceReward::InterGoldenCommerceReward()
    : TaiwanInterfaceInterDispatcher(
          static_cast<TaiwanInternalPack::T>(
              golden_commerce::kTaiwanInnerCode)) {}
InterGoldenCommerceReward::~InterGoldenCommerceReward() {}
int InterGoldenCommerceReward::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    if (!payload || payload_size != static_cast<int>(
            sizeof(golden_commerce::RewardClaimMessage))) return 0;
    golden_commerce::RewardClaimMessage response;
    std::memcpy(&response, payload, sizeof(response));
    golden_commerce::OnRewardClaimResult(user, response);
    return 0;
}
