#include "old_game/activity/schedule/fatigue_acceleration_runtime.h"

#include <algorithm>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/map/datamanager/world_stage_map_runtime.h"
#include "old_data/db/event/db_fatigue_acceleration.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/social/party/party.h"


namespace {

const char kConfigPath[] = "event/fatigue/eventfatigue.etc";
const char kVariousBuffConfigPath[] = "etc/variousbufflist.etc";

// RECONSTRUCTED_0828: eventfatigue.etc supplies the cap and the linked buff,
// while dstr 22392 supplies the event's separate hunting-EXP benefit.  Buff 23
// itself deliberately has no hunter_exp_rate effect.
const int kHunterExpBonusRate = 30;

enum PendingOperation {
    PENDING_NONE = 0,
    PENDING_LOAD = fatigue_acceleration_db::kOperationLoad,
    PENDING_SAVE = fatigue_acceleration_db::kOperationSave,
};

struct Config {
    Config()
        : threshold(0),
          buff_index(0),
          buff_param_0(0),
          buff_param_1(0),
          physical_attack(0),
          physical_defence(0),
          magical_attack(0),
          magical_defence(0),
          unique_rarity(0),
          unique_rarity_drop_rate(0),
          boss_drop_rate(0),
          area_indexes(),
          valid(false) {}

    uint16_t threshold;
    int buff_index;
    int buff_param_0;
    int buff_param_1;
    int physical_attack;
    int physical_defence;
    int magical_attack;
    int magical_defence;
    int unique_rarity;
    int unique_rarity_drop_rate;
    int boss_drop_rate;
    std::vector<int> area_indexes;
    bool valid;
};

struct VariousBuffDefinition {
    VariousBuffDefinition()
        : found(false),
          system_group_seen(false),
          icon_index_seen(false),
          apply_type_seen(false),
          system_group(),
          icon_index(0),
          apply_type(),
          apply_params(),
          physical_attack_seen(false),
          physical_defence_seen(false),
          magical_attack_seen(false),
          magical_defence_seen(false),
          unique_rarity_seen(false),
          boss_drop_seen(false),
          physical_attack(0),
          physical_defence(0),
          magical_attack(0),
          magical_defence(0),
          unique_rarity(0),
          unique_rarity_drop_rate(0),
          boss_drop_rate(0) {}

    bool found;
    bool system_group_seen;
    bool icon_index_seen;
    bool apply_type_seen;
    std::string system_group;
    int icon_index;
    std::string apply_type;
    std::vector<int> apply_params;

    bool physical_attack_seen;
    bool physical_defence_seen;
    bool magical_attack_seen;
    bool magical_defence_seen;
    bool unique_rarity_seen;
    bool boss_drop_seen;
    int physical_attack;
    int physical_defence;
    int magical_attack;
    int magical_defence;
    int unique_rarity;
    int unique_rarity_drop_rate;
    int boss_drop_rate;
};

struct SessionState {
    SessionState()
        : charac_no(0),
          token(0),
          loaded(false),
          requested_enabled(false),
          pending(PENDING_NONE),
          dungeon_run_started(false),
          dungeon_index(0),
          dungeon_fatigue_consumed(0),
          run_buff_sent(false),
          client_buff_present(false) {}

    uint32_t charac_no;
    uint32_t token;
    bool loaded;
    bool requested_enabled;
    PendingOperation pending;
    bool dungeon_run_started;
    int dungeon_index;
    uint16_t dungeon_fatigue_consumed;
    bool run_buff_sent;
    bool client_buff_present;
};

Mutex g_fatigue_acceleration_mutex;
Config g_fatigue_acceleration_config;
std::map<CUser*, SessionState> g_fatigue_acceleration_states;
uint32_t g_next_fatigue_acceleration_token = 1;

uint32_t NextTokenLocked() {
    uint32_t token = g_next_fatigue_acceleration_token++;
    if (token == 0) {
        token = g_next_fatigue_acceleration_token++;
    }
    if (g_next_fatigue_acceleration_token == 0) {
        ++g_next_fatigue_acceleration_token;
    }
    return token;
}

bool StoreVariousBuffEffect(VariousBuffDefinition& definition,
                            const std::string& name,
                            const std::vector<int>& params) {
    if (name == "physical_attack") {
        if (definition.physical_attack_seen || params.size() != 1u) {
            return false;
        }
        definition.physical_attack_seen = true;
        definition.physical_attack = params[0];
        return true;
    }
    if (name == "physical_defence") {
        if (definition.physical_defence_seen || params.size() != 1u) {
            return false;
        }
        definition.physical_defence_seen = true;
        definition.physical_defence = params[0];
        return true;
    }
    if (name == "magical_attack") {
        if (definition.magical_attack_seen || params.size() != 1u) {
            return false;
        }
        definition.magical_attack_seen = true;
        definition.magical_attack = params[0];
        return true;
    }
    if (name == "magical_defence") {
        if (definition.magical_defence_seen || params.size() != 1u) {
            return false;
        }
        definition.magical_defence_seen = true;
        definition.magical_defence = params[0];
        return true;
    }
    if (name == "unique_rarity_item_drop_rate") {
        if (definition.unique_rarity_seen || params.size() != 2u ||
            params[0] < 0 || params[0] > 5 || params[1] < 0) {
            return false;
        }
        definition.unique_rarity_seen = true;
        definition.unique_rarity = params[0];
        definition.unique_rarity_drop_rate = params[1];
        return true;
    }
    if (name == "boss_drop_rate") {
        if (definition.boss_drop_seen || params.size() != 1u ||
            params[0] < 0) {
            return false;
        }
        definition.boss_drop_seen = true;
        definition.boss_drop_rate = params[0];
        return true;
    }

    // The client can display more effect kinds, but effects 0..6 require
    // server business.  Refuse a partially understood event buff instead of
    // silently granting only its client-side stat half.
    return false;
}

bool LoadVariousBuffDefinition(int wanted_index,
                               VariousBuffDefinition& definition) {
    if (wanted_index <= 0 ||
        loadRDARScriptFile("", kVariousBuffConfigPath) != 1) {
        return false;
    }

    bool malformed = false;
    bool current_target = false;
    bool in_effect = false;
    bool in_apply_type = false;
    std::string effect_name;
    std::vector<int> effect_params;
    std::string tag;
    // The list contains client-only display fields that this strict server
    // importer intentionally ignores.  ScanType(false) skips their scalar
    // payload without flooding startup logs with false mismatch errors.
    while (ScanType(tag, false) == 1) {
        if (tag == "[index]") {
            int index = 0;
            if (ScanInt(&index) != 1 || current_target || in_effect ||
                in_apply_type) {
                malformed = true;
            }
            current_target = index == wanted_index;
            if (current_target) {
                if (definition.found) {
                    malformed = true;
                }
                definition.found = true;
            }
        } else if (tag == "[/index]") {
            if (current_target && (in_effect || in_apply_type)) {
                malformed = true;
            }
            current_target = false;
            in_effect = false;
            in_apply_type = false;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[system group]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                malformed = true;
            } else if (current_target) {
                if (definition.system_group_seen) {
                    malformed = true;
                }
                definition.system_group_seen = true;
                definition.system_group = value;
            }
        } else if (tag == "[icon index]") {
            int value = 0;
            if (ScanInt(&value) != 1) {
                malformed = true;
            } else if (current_target) {
                if (definition.icon_index_seen || value < 0) {
                    malformed = true;
                }
                definition.icon_index_seen = true;
                definition.icon_index = value;
            }
        } else if (tag == "[effect]") {
            if (current_target && (in_effect || in_apply_type)) {
                malformed = true;
            }
            in_effect = true;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[/effect]") {
            if (current_target &&
                (!in_effect || effect_name.empty() ||
                 !StoreVariousBuffEffect(
                     definition, effect_name, effect_params))) {
                malformed = true;
            }
            in_effect = false;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[name]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                malformed = true;
            } else if (current_target && in_effect) {
                if (!effect_name.empty()) {
                    malformed = true;
                }
                effect_name = value;
            }
        } else if (tag == "[apply type]") {
            if (current_target && (in_effect || in_apply_type)) {
                malformed = true;
            }
            in_apply_type = true;
        } else if (tag == "[/apply type]") {
            if (current_target && !in_apply_type) {
                malformed = true;
            }
            in_apply_type = false;
        } else if (tag == "[type]") {
            std::string value;
            if (ScanStr(&value) != 1) {
                malformed = true;
            } else if (current_target && in_apply_type) {
                if (definition.apply_type_seen) {
                    malformed = true;
                }
                definition.apply_type_seen = true;
                definition.apply_type = value;
            }
        } else if (tag == "[param]") {
            std::vector<int> values;
            int value = 0;
            while (ScanInt(&value) == 1) {
                values.push_back(value);
            }
            if (current_target) {
                if (in_effect) {
                    if (!effect_params.empty()) {
                        malformed = true;
                    }
                    effect_params = values;
                } else if (in_apply_type) {
                    if (!definition.apply_params.empty()) {
                        malformed = true;
                    }
                    definition.apply_params = values;
                } else {
                    malformed = true;
                }
            }
        }
    }
    unloadRDARScriptFile();

    const bool complete =
        definition.found && definition.system_group_seen &&
        !definition.system_group.empty() && definition.icon_index_seen &&
        definition.apply_type_seen &&
        definition.apply_type == "dungeon_one_match" &&
        definition.apply_params.size() == 2u &&
        definition.apply_params[0] == 1 && definition.apply_params[1] == 0 &&
        definition.physical_attack_seen &&
        definition.physical_defence_seen &&
        definition.magical_attack_seen &&
        definition.magical_defence_seen &&
        definition.unique_rarity_seen && definition.boss_drop_seen;
    return complete && !malformed;
}

uint32_t CurrentCharacterNo(CUser* user) {
    const int value = user && user->getCurCharacR()
                          ? user->getCurCharacNo()
                          : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

Config ConfigSnapshot() {
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    return g_fatigue_acceleration_config;
}

bool SessionSnapshot(CUser* user, bool& loaded, bool& requested_enabled) {
    loaded = false;
    requested_enabled = false;
    const uint32_t charac_no = CurrentCharacterNo(user);
    if (!charac_no) {
        return false;
    }

    MutexGuard guard(&g_fatigue_acceleration_mutex);
    const std::map<CUser*, SessionState>::const_iterator found =
        g_fatigue_acceleration_states.find(user);
    if (found == g_fatigue_acceleration_states.end() ||
        found->second.charac_no != charac_no) {
        return false;
    }
    loaded = found->second.loaded;
    requested_enabled = found->second.requested_enabled;
    return true;
}

bool IsSupportedDungeon(const Config& config,
                        const CDungeon* dungeon,
                        int dungeon_index) {
    if (!config.valid || !dungeon || dungeon_index <= 0 ||
        dungeon->getDungeonKind() != 0 ||
        dungeon->isNoFatigueDungeon() == 1) {
        return false;
    }

    const CWorldMapList* const world_maps =
        CDataManagerWorldMapList(G_CDataManager());
    if (!world_maps) {
        return false;
    }
    for (std::vector<int>::const_iterator it = config.area_indexes.begin();
         it != config.area_indexes.end();
         ++it) {
        CWorldMap* const world_map = world_maps->find_world_map(*it);
        if (world_map &&
            world_map->hasDungeon(static_cast<unsigned int>(dungeon_index))) {
            return true;
        }
    }
    return false;
}

bool IsBaseFatigueApplicable(CUser* user,
                             const CDungeon* dungeon,
                             int dungeon_index) {
    CParty* const party = user ? user->GetParty() : 0;
    return party && dungeon && dungeon_index > 0 &&
           party->battle_field().instance_dungeon_mode() != 1 &&
           !dungeon->isRevengeDungeon() &&
           dungeon->get_blood_dungeon_type() == 0 &&
           dungeon_index != 10000 &&
           dungeon->isNoFatigueDungeon() != 1 &&
           dungeon->isTournamentDungeon() == 0;
}

SessionState* FindCurrentSessionLocked(CUser* user) {
    const uint32_t charac_no = CurrentCharacterNo(user);
    std::map<CUser*, SessionState>::iterator found =
        g_fatigue_acceleration_states.find(user);
    if (!charac_no || found == g_fatigue_acceleration_states.end() ||
        found->second.charac_no != charac_no) {
        return 0;
    }
    return &found->second;
}

bool IsBenefitActiveLocked(CUser* user,
                           const CDungeon* dungeon,
                           int dungeon_index,
                           SessionState*& state) {
    state = FindCurrentSessionLocked(user);
    return state && g_fatigue_acceleration_config.valid && state->loaded &&
           state->requested_enabled && state->dungeon_run_started &&
           state->dungeon_index == dungeon_index &&
           IsBaseFatigueApplicable(user, dungeon, dungeon_index) &&
           IsSupportedDungeon(
               g_fatigue_acceleration_config, dungeon, dungeon_index);
}

int ResolveFatigueCostLocked(CUser* user,
                             const CDungeon* dungeon,
                             int dungeon_index,
                             bool reserve) {
    SessionState* state = 0;
    if (!IsBenefitActiveLocked(user, dungeon, dungeon_index, state)) {
        return 1;
    }

    const int requested =
        user->getCurCharacTotalFatigue() >
                static_cast<int>(g_fatigue_acceleration_config.threshold)
            ? 2
            : 1;
    const int remaining = std::max(
        0,
        static_cast<int>(g_fatigue_acceleration_config.threshold) -
            static_cast<int>(state->dungeon_fatigue_consumed));
    const int charge = std::min(requested, remaining);
    if (reserve && charge > 0) {
        state->dungeon_fatigue_consumed = static_cast<uint16_t>(
            state->dungeon_fatigue_consumed + charge);
    }
    return charge;
}

void SendVariousBuffAdd(CUser* user, const Config& config) {
    if (!user) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHARACTER_ADD_BUFF);
    // CLIENT_IDA_EVIDENCE: sub_46ADC0 consumes count, then exactly
    // {u32 buff_index,u32 apply_param0,u32 apply_param1} per entry.
    packet.put_byte(1);
    packet.put_int(config.buff_index);
    packet.put_int(config.buff_param_0);
    packet.put_int(config.buff_param_1);
    packet.finalize(true);
    const zombie_map_runtime::HookResult route =
        zombie_map_runtime::RouteBootstrapFatiguePacket(
            user, packet, false);
    if (route == zombie_map_runtime::kHookNotManaged) user->Send(packet);
}

void SendVariousBuffDelete(CUser* user, int buff_index) {
    if (!user || buff_index <= 0) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHARACTER_DEL_BUFF);
    // CLIENT_IDA_EVIDENCE: sub_46A880 consumes count, then u32 buff_index.
    packet.put_byte(1);
    packet.put_int(buff_index);
    packet.finalize(true);
    const zombie_map_runtime::HookResult route =
        zombie_map_runtime::RouteBootstrapFatiguePacket(
            user, packet, false);
    if (route == zombie_map_runtime::kHookNotManaged) user->Send(packet);
}

void SendStatePacket(CUser* user, uint16_t threshold, uint8_t state) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return;
    }

    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_NOTIPACKET,
        ENUM_NOTIPACKET_USER_FATIGUE_ACCELERATION_STATE);
    // CLIENT_IDA_EVIDENCE: sub_5F07E0 unconditionally reads u16 then u8.
    // sub_5F07C0 compares the first field strictly against the consumed
    // fatigue value installed by NOTI36: current_consumed > threshold.
    packet.put_short(threshold);
    packet.put_byte(state);
    packet.finalize(true);
    const zombie_map_runtime::HookResult route =
        zombie_map_runtime::RouteBootstrapFatiguePacket(
            user, packet, true);
    if (route == zombie_map_runtime::kHookNotManaged) user->Send(packet);
}

void SendCurrentState(CUser* user) {
    if (!user) {
        return;
    }
    CParty* const party = user->GetParty();
    if (user->get_state() == 5 && party) {
        const CDungeon* const dungeon = party->battle_field().get_dungeon();
        fatigue_acceleration::SendDungeonState(
            user, dungeon, party->getDungIndex());
        return;
    }
    fatigue_acceleration::SendTownState(user);
}

void SendCommandError(CUser* user, int error) {
    if (user) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_FATIGUE_ACCELERATION_STATE_CHANGE,
            static_cast<unsigned char>(error));
    }
}

bool QueueMessage(CUser* user,
                  uint32_t charac_no,
                  uint32_t token,
                  PendingOperation operation,
                  bool requested_enabled) {
    fatigue_acceleration_db::Message request;
    request.charac_no = charac_no;
    request.token = token;
    request.server_group = user->GetServerGroup();
    request.operation = static_cast<uint8_t>(operation);
    request.requested_enabled = requested_enabled ? 1u : 0u;
    request.result = fatigue_acceleration_db::kResultFailed;
    request.reserved = 0;
    return fatigue_acceleration_db::DB_FatigueAcceleration::Queue(
        user->GetUID(), request);
}

void ClearPending(CUser* user,
                  uint32_t charac_no,
                  uint32_t token,
                  PendingOperation operation) {
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    std::map<CUser*, SessionState>::iterator found =
        g_fatigue_acceleration_states.find(user);
    if (found != g_fatigue_acceleration_states.end() &&
        found->second.charac_no == charac_no &&
        found->second.token == token && found->second.pending == operation) {
        found->second.pending = PENDING_NONE;
    }
}

void ApplyDatabaseResult(CUser* user,
                         const fatigue_acceleration_db::Message& result) {
    if (!user || result.charac_no == 0 || result.token == 0) {
        return;
    }

    bool send_command_success = false;
    bool send_command_error = false;
    bool send_state = false;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_fatigue_acceleration_states.find(user);
        if (found == g_fatigue_acceleration_states.end() ||
            found->second.charac_no != result.charac_no ||
            found->second.token != result.token ||
            CurrentCharacterNo(user) != result.charac_no ||
            static_cast<uint8_t>(found->second.pending) != result.operation) {
            return;
        }

        SessionState& state = found->second;
        state.pending = PENDING_NONE;
        if (result.result != fatigue_acceleration_db::kResultOk ||
            result.requested_enabled > 1u) {
            if (result.operation == fatigue_acceleration_db::kOperationLoad) {
                state.loaded = false;
                state.requested_enabled = false;
                send_state = true;
            } else {
                send_command_error = true;
            }
        } else if (result.operation ==
                   fatigue_acceleration_db::kOperationLoad) {
            state.loaded = true;
            state.requested_enabled = result.requested_enabled != 0;
            send_state = true;
        } else if (result.operation ==
                   fatigue_acceleration_db::kOperationSave) {
            state.loaded = true;
            state.requested_enabled = result.requested_enabled != 0;
            send_command_success = true;
            send_state = true;
        }
    }

    if (send_command_error) {
        SendCommandError(user, 1);
        return;
    }
    if (send_command_success) {
        user->SendCmdOkPacket(
            ENUM_CMDPACKET_FATIGUE_ACCELERATION_STATE_CHANGE);
    }
    if (send_state) {
        SendCurrentState(user);
    }
}

bool RequestToggle(CUser* user, bool requested_enabled) {
    if (!user || !user->getCurCharacR()) {
        return false;
    }
    if (user->get_state() != 3) {
        SendCommandError(user, 19);
        return true;
    }

    const Config config = ConfigSnapshot();
    if (!config.valid) {
        SendCommandError(user, 178);
        return true;
    }
    if (requested_enabled &&
        user->getCurCharacTotalFatigue() <=
            static_cast<int>(config.threshold)) {
        SendCommandError(user, 22);
        return true;
    }

    const uint32_t charac_no = CurrentCharacterNo(user);
    uint32_t token = 0;
    bool already_current = false;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        std::map<CUser*, SessionState>::iterator found =
            g_fatigue_acceleration_states.find(user);
        if (found == g_fatigue_acceleration_states.end() ||
            found->second.charac_no != charac_no ||
            !found->second.loaded || found->second.pending != PENDING_NONE) {
            SendCommandError(user, 1);
            return true;
        }
        already_current =
            found->second.requested_enabled == requested_enabled;
        token = found->second.token;
        if (!already_current) {
            found->second.pending = PENDING_SAVE;
        }
    }

    if (already_current) {
        user->SendCmdOkPacket(
            ENUM_CMDPACKET_FATIGUE_ACCELERATION_STATE_CHANGE);
        SendCurrentState(user);
        return true;
    }

    if (!QueueMessage(
            user, charac_no, token, PENDING_SAVE, requested_enabled)) {
        ClearPending(user, charac_no, token, PENDING_SAVE);
        SendCommandError(user, 1);
    }
    return true;
}

}  // namespace


namespace fatigue_acceleration {

bool LoadConfig() {
    Config parsed;
    bool have_threshold = false;
    bool have_areas = false;
    bool have_dungeon_type = false;
    bool have_buff_index = false;
    bool malformed = false;

    if (loadRDARScriptFile("", kConfigPath) == 1) {
        std::string tag;
        while (ScanType(tag, true) == 1) {
            if (tag == "[max fatigue]") {
                int value = 0;
                if (have_threshold || !ScanInt(&value) || value <= 0 ||
                    value > 0xffff) {
                    malformed = true;
                } else {
                    parsed.threshold = static_cast<uint16_t>(value);
                    have_threshold = true;
                }
            } else if (tag == "[apply area list]") {
                if (have_areas) {
                    malformed = true;
                }
                have_areas = true;
                int area = 0;
                while (ScanInt(&area) == 1) {
                    if (area <= 0 ||
                        std::find(parsed.area_indexes.begin(),
                                  parsed.area_indexes.end(),
                                  area) != parsed.area_indexes.end()) {
                        malformed = true;
                    } else {
                        parsed.area_indexes.push_back(area);
                    }
                }
                if (parsed.area_indexes.empty()) {
                    malformed = true;
                }
            } else if (tag == "[apply dungeon type]") {
                std::string value;
                if (have_dungeon_type || ScanStr(&value) != 1 ||
                    value != "normal") {
                    malformed = true;
                } else {
                    have_dungeon_type = true;
                }
            } else if (tag == "[apply buff index]") {
                int value = 0;
                if (have_buff_index || !ScanInt(&value) || value <= 0) {
                    malformed = true;
                } else {
                    parsed.buff_index = value;
                    have_buff_index = true;
                }
            }
        }
        unloadRDARScriptFile();
    } else {
        malformed = true;
    }

    VariousBuffDefinition buff_definition;
    if (!have_buff_index ||
        !LoadVariousBuffDefinition(parsed.buff_index, buff_definition)) {
        malformed = true;
    } else {
        parsed.buff_param_0 = buff_definition.apply_params[0];
        parsed.buff_param_1 = buff_definition.apply_params[1];
        parsed.physical_attack = buff_definition.physical_attack;
        parsed.physical_defence = buff_definition.physical_defence;
        parsed.magical_attack = buff_definition.magical_attack;
        parsed.magical_defence = buff_definition.magical_defence;
        parsed.unique_rarity = buff_definition.unique_rarity;
        parsed.unique_rarity_drop_rate =
            buff_definition.unique_rarity_drop_rate;
        parsed.boss_drop_rate = buff_definition.boss_drop_rate;
    }

    const CWorldMapList* const world_maps =
        CDataManagerWorldMapList(G_CDataManager());
    if (!world_maps) {
        malformed = true;
    } else {
        for (std::vector<int>::const_iterator it = parsed.area_indexes.begin();
             it != parsed.area_indexes.end();
             ++it) {
            if (!world_maps->find_world_map(*it)) {
                malformed = true;
            }
        }
    }

    parsed.valid = have_threshold && have_areas && have_dungeon_type &&
                   have_buff_index && !malformed;
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    g_fatigue_acceleration_config = parsed;
    g_fatigue_acceleration_states.clear();
    return parsed.valid;
}

void BeginSession(CUser* user) {
    const uint32_t charac_no = CurrentCharacterNo(user);
    if (!user || !charac_no) {
        return;
    }

    uint32_t token = 0;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        SessionState state;
        state.charac_no = charac_no;
        state.token = NextTokenLocked();
        state.pending = PENDING_LOAD;
        token = state.token;
        g_fatigue_acceleration_states[user] = state;
    }

    if (!QueueMessage(user, charac_no, token, PENDING_LOAD, false)) {
        ClearPending(user, charac_no, token, PENDING_LOAD);
        SendTownState(user);
    }
}

void ResetSession(CUser* user) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    g_fatigue_acceleration_states.erase(user);
}

void BeginDungeonRun(CUser* user,
                     const CDungeon* /*dungeon*/,
                     int dungeon_index) {
    if (!user || dungeon_index <= 0) {
        return;
    }
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    SessionState* const state = FindCurrentSessionLocked(user);
    if (!state) {
        return;
    }

    // RECONSTRUCTED_0828: the old server has no 0828 producer, but
    // eventfatigue [max fatigue] and dstr 22394/22395 require a per-match
    // accumulator.  CParty::dungeon_start is the narrow committed owner and
    // RestartDungeon re-enters it, so each successful new match resets here.
    state->dungeon_run_started = true;
    state->dungeon_index = dungeon_index;
    state->dungeon_fatigue_consumed = 0;
    state->run_buff_sent = false;
}

void AbortDungeonRun(CUser* user) {
    if (!user) {
        return;
    }
    Config config;
    bool send_delete = false;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        SessionState* const state = FindCurrentSessionLocked(user);
        if (!state) {
            return;
        }
        config = g_fatigue_acceleration_config;
        send_delete = state->client_buff_present;
        state->client_buff_present = false;
        state->dungeon_run_started = false;
        state->dungeon_index = 0;
        state->dungeon_fatigue_consumed = 0;
        state->run_buff_sent = false;
    }
    if (send_delete) {
        SendVariousBuffDelete(user, config.buff_index);
    }
}

int ResolveFatigueCost(CUser* user,
                       const CDungeon* dungeon,
                       int dungeon_index) {
    if (!user) {
        return 0;
    }
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    return ResolveFatigueCostLocked(
        user, dungeon, dungeon_index, false);
}

int ConsumeFatigue(CUser* user,
                   const CDungeon* dungeon,
                   int dungeon_index) {
    if (!user) {
        return 0;
    }

    int charge = 1;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        charge = ResolveFatigueCostLocked(
            user, dungeon, dungeon_index, true);
    }

    // CUser::FatigueUp rewrites zero to one.  Returning the current total is
    // therefore the only correct no-consumption transaction after cap 8.
    if (charge <= 0) {
        return user->getCurCharacTotalFatigue();
    }
    return user->FatigueUp(charge);
}

int GetHunterExpBonusRate(CUser* user,
                          const CDungeon* dungeon,
                          int dungeon_index) {
    if (!user) {
        return 0;
    }
    MutexGuard guard(&g_fatigue_acceleration_mutex);
    SessionState* state = 0;
    return IsBenefitActiveLocked(user, dungeon, dungeon_index, state)
               ? kHunterExpBonusRate
               : 0;
}

void SendDungeonState(CUser* user,
                      const CDungeon* dungeon,
                      int dungeon_index) {
    Config config;
    uint8_t state = 0;
    bool send_add = false;
    bool send_delete = false;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        config = g_fatigue_acceleration_config;
        SessionState* session = FindCurrentSessionLocked(user);
        SessionState* active_session = 0;
        const bool benefit_active = IsBenefitActiveLocked(
            user, dungeon, dungeon_index, active_session);

        if (config.valid &&
            (!IsBaseFatigueApplicable(user, dungeon, dungeon_index) ||
             !IsSupportedDungeon(config, dungeon, dungeon_index))) {
            state = 3;
        } else if (config.valid && session && session->loaded &&
                   session->requested_enabled) {
            // RECONSTRUCTED_0828: state1 is the personal/contract switch.
            // State2 remains reserved for an unrecovered event-wide producer.
            state = 1;
        }

        if (benefit_active && active_session &&
            !active_session->run_buff_sent) {
            // CLIENT_IDA_EVIDENCE: NOTI484 sub_46ADC0 routes an existing
            // index through sub_46ACC0 and dispatches the add path again.
            // End the old one-match generation first so RestartDungeon can
            // never apply effects 7..10 twice before installing the new one.
            send_delete = active_session->client_buff_present;
            send_add = true;
            active_session->run_buff_sent = true;
            active_session->client_buff_present = true;
        } else if (!benefit_active && session &&
                   session->client_buff_present) {
            send_delete = true;
            session->client_buff_present = false;
            session->run_buff_sent = false;
        }
    }
    if (send_delete) {
        SendVariousBuffDelete(user, config.buff_index);
    }
    if (send_add) {
        SendVariousBuffAdd(user, config);
    }
    SendStatePacket(user, config.valid ? config.threshold : 0u, state);
}

void SendTownState(CUser* user) {
    Config config;
    bool loaded = false;
    bool requested_enabled = false;
    bool send_delete = false;
    {
        MutexGuard guard(&g_fatigue_acceleration_mutex);
        config = g_fatigue_acceleration_config;
        SessionState* const session = FindCurrentSessionLocked(user);
        if (session) {
            loaded = session->loaded;
            requested_enabled = session->requested_enabled;
            send_delete = session->client_buff_present;
            session->client_buff_present = false;
            session->run_buff_sent = false;
            session->dungeon_run_started = false;
            session->dungeon_index = 0;
            session->dungeon_fatigue_consumed = 0;
        }
    }
    if (send_delete) {
        SendVariousBuffDelete(user, config.buff_index);
    }
    const uint8_t state = config.valid && loaded && requested_enabled ? 1u : 0u;
    SendStatePacket(user, config.valid ? config.threshold : 0u, state);
}

}  // namespace fatigue_acceleration


Dispatcher_FatigueAccelerationStateChange::
    Dispatcher_FatigueAccelerationStateChange()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

Dispatcher_FatigueAccelerationStateChange::
    ~Dispatcher_FatigueAccelerationStateChange() {}

int Dispatcher_FatigueAccelerationStateChange::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    unsigned char requested_enabled = 0;
    // CLIENT_IDA_EVIDENCE: all four send branches converge on exactly one
    // u8 writer before finalizing CMD556.
    if (packet.get_byte(requested_enabled) != 1 || requested_enabled > 1u) {
        SendCommandError(user, 1);
        return 0;
    }
    RequestToggle(user, requested_enabled != 0);
    return 0;
}

int Dispatcher_FatigueAccelerationStateChange::read(PacketBuf& packet,
                                                     MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_FatigueAccelerationStateChange::process(CUser* user,
                                                        MSG_BASE& message,
                                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}


Inter_FatigueAcceleration::Inter_FatigueAcceleration()
    : ISigDispatcher() {}

Inter_FatigueAcceleration::~Inter_FatigueAcceleration() {}

int Inter_FatigueAcceleration::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int payload_size) {
    if (!payload ||
        payload_size !=
            static_cast<int>(sizeof(fatigue_acceleration_db::Message))) {
        return 0;
    }
    const fatigue_acceleration_db::Message* const result =
        reinterpret_cast<const fatigue_acceleration_db::Message*>(payload);
    ApplyDatabaseResult(user, *result);
    return 0;
}
