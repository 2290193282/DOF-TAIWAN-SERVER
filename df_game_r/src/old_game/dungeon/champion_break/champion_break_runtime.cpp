#include "old_game/dungeon/champion_break/champion_break_runtime.h"

#include <algorithm>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "common/manager/server_utility.h"
#include "common/config.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/social/party/party.h"

namespace {

const char kChampionConfigPath[] =
    "etc/dungeonetc/championbreaksystem.etc";
const char kVariousBuffPath[] = "etc/variousbufflist.etc";

enum CardPhase {
    CARD_PHASE_COUNTING = 0,
    CARD_PHASE_TRIGGERED = 1,
    CARD_PHASE_RIGHTS_SENT = 2,
    CARD_PHASE_COMPLETE = 3,
};

enum BuffEffectKind {
    BUFF_EFFECT_NONE = 0,
    BUFF_EFFECT_MAGIC_SEAL_DROP = 1,
    BUFF_EFFECT_UNIQUE_DROP = 2,
    BUFF_EFFECT_HUNTER_EXP = 3,
    BUFF_EFFECT_CLEAR_EXP = 4,
    BUFF_EFFECT_WEAPON_UPGRADE = 5,
};

struct BuffDefinition {
    BuffDefinition()
        : found(false), champion_group(false), one_match(false),
          effect(BUFF_EFFECT_NONE), params() {}

    bool found;
    bool champion_group;
    bool one_match;
    BuffEffectKind effect;
    std::vector<int> params;
};

struct Config {
    Config()
        : threshold(0), base_probability(0), probability_increment(0),
          rewards(), dungeon_indexes(), buffs(), valid(false) {}

    int threshold;
    int base_probability;
    int probability_increment;
    std::vector<int> rewards;
    std::vector<int> dungeon_indexes;
    std::map<int, BuffDefinition> buffs;
    bool valid;
};

struct PartyState {
    PartyState()
        : dungeon_index(0), eligible(false), champion_count(0),
          success_latched(false), phase(CARD_PHASE_COUNTING) {
        for (int index = 0; index < 4; ++index) {
            card_owner[index] = -1;
            card_buff[index] = 0;
            user_card[index] = -1;
        }
    }

    int dungeon_index;
    bool eligible;
    int champion_count;
    bool success_latched;
    CardPhase phase;
    int card_owner[4];
    int card_buff[4];
    int user_card[4];
};

struct UserState {
    UserState()
        : charac_no(0), pending_buff(0), active_buff(0),
          active_dungeon(0), client_buff_present(false),
          last_hunter_exp_bonus(0), weapon_projection_active(false),
          projected_weapon_item_id(0), projected_weapon_original_upgrade(0),
          projected_weapon_upgrade(0), projected_weapon_server_id(0),
          projected_weapon_datetime(0), projected_weapon_sequence_no(0) {}

    uint32_t charac_no;
    int pending_buff;
    int active_buff;
    int active_dungeon;
    bool client_buff_present;
    int last_hunter_exp_bonus;
    bool weapon_projection_active;
    int projected_weapon_item_id;
    int projected_weapon_original_upgrade;
    int projected_weapon_upgrade;
    int projected_weapon_server_id;
    int projected_weapon_datetime;
    uint16_t projected_weapon_sequence_no;
};

Mutex g_champion_break_mutex;
Config g_config;
std::map<CParty*, PartyState> g_party_states;
std::map<CUser*, UserState> g_user_states;

uint32_t CurrentCharacterNo(CUser* user) {
    const int value = user && user->getCurCharacR()
                          ? user->getCurCharacNo()
                          : 0;
    return value > 0 ? static_cast<uint32_t>(value) : 0u;
}

bool Contains(const std::vector<int>& values, int wanted) {
    return std::find(values.begin(), values.end(), wanted) != values.end();
}

bool IsDungeonEnabledLocked(int dungeon_index) {
    return g_config.valid && dungeon_index > 0 &&
           Contains(g_config.dungeon_indexes, dungeon_index);
}

UserState* CurrentUserStateLocked(CUser* user, bool create) {
    const uint32_t charac_no = CurrentCharacterNo(user);
    if (!charac_no) {
        return 0;
    }
    std::map<CUser*, UserState>::iterator found = g_user_states.find(user);
    if (found == g_user_states.end() || found->second.charac_no != charac_no) {
        if (!create) {
            return 0;
        }
        UserState state;
        state.charac_no = charac_no;
        g_user_states[user] = state;
        found = g_user_states.find(user);
    }
    return &found->second;
}

void SendBuffAdd(CUser* user, int buff_index) {
    if (!user || buff_index <= 0) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHARACTER_ADD_BUFF);
    // CLIENT_IDA_EVIDENCE: sub_46ADC0 consumes
    // {u8 count,u32 index,u32 apply_param0,u32 apply_param1}.
    packet.put_byte(1);
    packet.put_int(buff_index);
    packet.put_int(1);
    packet.put_int(0);
    packet.finalize(true);
    user->Send(packet);
}

void SendBuffDelete(CUser* user, int buff_index) {
    if (!user || buff_index <= 0) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CHARACTER_DEL_BUFF);
    packet.put_byte(1);
    packet.put_int(buff_index);
    packet.finalize(true);
    user->Send(packet);
}

void SendChampionSuccess(CParty* party) {
    if (!party) {
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPAKCET_SUCCESS_CHAMPION_BREAK_SYSTEM);
    // CLIENT_IDA_EVIDENCE: sub_453B10/NOTI603 reads no body and only latches
    // ChampionBreakManager+0x0c plus the active gauge state.
    packet.finalize(true);
    party->send_to_party(packet);
}

BuffEffectKind EffectKindForName(const std::string& name) {
    if (name == "upgrade_random_item_drop_rate") {
        return BUFF_EFFECT_MAGIC_SEAL_DROP;
    }
    if (name == "unique_rarity_item_drop_rate") {
        return BUFF_EFFECT_UNIQUE_DROP;
    }
    if (name == "hunter_exp_rate") {
        return BUFF_EFFECT_HUNTER_EXP;
    }
    if (name == "dungeon_clear_exp_rate") {
        return BUFF_EFFECT_CLEAR_EXP;
    }
    if (name == "upgrade_weapon_item") {
        return BUFF_EFFECT_WEAPON_UPGRADE;
    }
    return BUFF_EFFECT_NONE;
}

bool ValidateBuff(int index, const BuffDefinition& buff) {
    if (!buff.found || !buff.champion_group || !buff.one_match ||
        buff.effect == BUFF_EFFECT_NONE) {
        return false;
    }
    if (index == 1) {
        return buff.effect == BUFF_EFFECT_MAGIC_SEAL_DROP &&
               buff.params.size() == 3u && buff.params[0] == 7 &&
               buff.params[1] == 9 && buff.params[2] >= 0;
    }
    if (index == 2) {
        return buff.effect == BUFF_EFFECT_UNIQUE_DROP &&
               buff.params.size() == 2u && buff.params[0] == 3 &&
               buff.params[1] >= 0;
    }
    if (index == 3) {
        return buff.effect == BUFF_EFFECT_HUNTER_EXP &&
               buff.params.size() == 1u && buff.params[0] >= 0;
    }
    if (index == 4) {
        return buff.effect == BUFF_EFFECT_CLEAR_EXP &&
               buff.params.size() == 1u && buff.params[0] >= 0;
    }
    if (index == 5) {
        return buff.effect == BUFF_EFFECT_WEAPON_UPGRADE &&
               buff.params.size() == 1u && buff.params[0] >= 0;
    }
    return false;
}

bool LoadChampionConfig(Config& parsed) {
    if (loadRDARScriptFile("", kChampionConfigPath) != 1) {
        return false;
    }

    bool have_wrapper = false;
    bool have_wrapper_end = false;
    bool have_threshold = false;
    bool have_probability = false;
    bool have_rewards = false;
    bool have_dungeons = false;
    bool malformed = false;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[champion break]") {
            if (have_wrapper) {
                malformed = true;
            }
            have_wrapper = true;
        } else if (tag == "[/champion break]") {
            have_wrapper_end = have_wrapper;
            break;
        } else if (!have_wrapper) {
            malformed = true;
        } else if (tag == "[monster numbers]") {
            int value = 0;
            if (have_threshold || ScanInt(&value) != 1 || value <= 0) {
                malformed = true;
            } else {
                parsed.threshold = value;
                have_threshold = true;
            }
        } else if (tag == "[booster card probability]") {
            int base = 0;
            int increment = 0;
            if (have_probability || ScanInt(&base) != 1 ||
                ScanInt(&increment) != 1 || base < 0 || base > 10000 ||
                increment < 0 || increment > 10000) {
                malformed = true;
            } else {
                parsed.base_probability = base;
                parsed.probability_increment = increment;
                have_probability = true;
            }
        } else if (tag == "[rewards]") {
            if (have_rewards) {
                malformed = true;
            }
            have_rewards = true;
            int value = 0;
            while (ScanInt(&value) == 1) {
                if (value <= 0 || Contains(parsed.rewards, value)) {
                    malformed = true;
                } else {
                    parsed.rewards.push_back(value);
                }
            }
        } else if (tag == "[dungeon index]") {
            if (have_dungeons) {
                malformed = true;
            }
            have_dungeons = true;
            int value = 0;
            while (ScanInt(&value) == 1) {
                if (value <= 0 || Contains(parsed.dungeon_indexes, value)) {
                    malformed = true;
                } else {
                    parsed.dungeon_indexes.push_back(value);
                }
            }
        }
    }
    unloadRDARScriptFile();
    return have_wrapper && have_wrapper_end && have_threshold &&
           have_probability && have_rewards &&
           have_dungeons && !parsed.rewards.empty() &&
           !parsed.dungeon_indexes.empty() && !malformed;
}

bool LoadBuffConfig(Config& parsed) {
    if (loadRDARScriptFile("", kVariousBuffPath) != 1) {
        return false;
    }

    int current_index = 0;
    bool in_effect = false;
    bool in_apply_type = false;
    std::string effect_name;
    std::vector<int> effect_params;
    std::string tag;
    while (ScanType(tag, false) == 1) {
        if (tag == "[index]") {
            int index = 0;
            if (ScanInt(&index) == 1) {
                current_index = index >= 1 && index <= 5 ? index : 0;
                if (current_index) {
                    parsed.buffs[current_index].found = true;
                }
            }
        } else if (tag == "[/index]") {
            current_index = 0;
            in_effect = false;
            in_apply_type = false;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[system group]") {
            std::string value;
            if (ScanStr(&value) == 1 && current_index) {
                parsed.buffs[current_index].champion_group =
                    value == "Champion_Break";
            }
        } else if (tag == "[effect]") {
            in_effect = true;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[/effect]") {
            if (current_index && in_effect) {
                BuffDefinition& buff = parsed.buffs[current_index];
                buff.effect = EffectKindForName(effect_name);
                buff.params = effect_params;
            }
            in_effect = false;
            effect_name.clear();
            effect_params.clear();
        } else if (tag == "[name]") {
            std::string value;
            if (ScanStr(&value) == 1 && current_index && in_effect) {
                effect_name = value;
            }
        } else if (tag == "[apply type]") {
            in_apply_type = true;
        } else if (tag == "[/apply type]") {
            in_apply_type = false;
        } else if (tag == "[type]") {
            std::string value;
            if (ScanStr(&value) == 1 && current_index && in_apply_type) {
                parsed.buffs[current_index].one_match =
                    value == "dungeon_one_match";
            }
        } else if (tag == "[param]") {
            std::vector<int> values;
            int value = 0;
            while (ScanInt(&value) == 1) {
                values.push_back(value);
            }
            if (current_index && in_effect) {
                effect_params = values;
            }
        }
    }
    unloadRDARScriptFile();

    for (int index = 1; index <= 5; ++index) {
        if (!ValidateBuff(index, parsed.buffs[index])) {
            return false;
        }
    }
    return true;
}

int UserSeat(CParty* party, CUser* user) {
    if (!party || !user) {
        return -1;
    }
    const int seat = party->GetMemberSlotNo(user);
    return seat >= 0 && seat < 4 ? seat : -1;
}

bool AllValidUsersSelected(CParty* party, const PartyState& state) {
    if (!party) {
        return false;
    }
    for (int seat = 0; seat < 4; ++seat) {
        if (party->checkValidUser(seat) == 1 && state.user_card[seat] < 0) {
            return false;
        }
    }
    return true;
}

bool AllValidUsersSelectedNormalCard(CParty* party) {
    if (!party) {
        return false;
    }
    const eClearRewardCardType_t normal_type =
        static_cast<eClearRewardCardType_t>(0);
    for (int seat = 0; seat < 4; ++seat) {
        if (party->checkValidUser(seat) == 1 &&
            party->HaveAlreadySelectedCard(static_cast<char>(seat),
                                           normal_type) != 1) {
            return false;
        }
    }
    return true;
}

int ProbabilityLocked(const PartyState& state) {
    // RECONSTRUCTED_0828: both values are stored verbatim by client
    // sub_14EAF30.  The 3000 base and 500-per-extra-kill values are treated as
    // per-10000 units; this is the smallest monotonic interpretation and is
    // fail-closed at 10000 until a server producer is recovered.
    const int extras = std::max(0, state.champion_count - g_config.threshold);
    const long long probability =
        static_cast<long long>(g_config.base_probability) +
        static_cast<long long>(extras) * g_config.probability_increment;
    return static_cast<int>(std::min<long long>(10000, probability));
}

int RollBuffLocked(const PartyState& state) {
    if (g_config.rewards.empty() ||
        static_cast<int>(server_utility::get_rand_int(10000)) >=
            ProbabilityLocked(state)) {
        return 0;
    }
    // RECONSTRUCTED_0828: [rewards] is an unweighted vector in the client
    // config.  Select uniformly until a recovered 0828 server producer proves
    // a separate weight owner.
    const unsigned int position = server_utility::get_rand_int(
        static_cast<int>(g_config.rewards.size()));
    return g_config.rewards[position];
}

void StorePendingBuff(CUser* user, int buff_index) {
    if (!user || buff_index <= 0) {
        return;
    }

    MutexGuard guard(&g_champion_break_mutex);
    UserState* const state = CurrentUserStateLocked(user, true);
    if (state) {
        // The clear-screen card itself is rendered by S2C71.  NOTI484 belongs
        // to the next eligible dungeon's BeginDungeonRun, not this click.
        state->pending_buff = buff_index;
    }
}

void WriteOrdinaryRights(CParty* party, PacketGuard& packet) {
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const user = party ? party->get_user(seat) : 0;
        if (!user) {
            packet.put_short(-1);
        } else if (static_cast<unsigned short>(
                       user->getCurCharacTotalFatigue()) ==
                   static_cast<unsigned short>(
                       user->getCurCharacTotalMaxFatigue())) {
            packet.put_short(0);
        } else {
            packet.put_short(1);
        }
    }
    // CLIENT_IDA_EVIDENCE: sub_55C480 reads all eight u16 values and enters
    // special state 9 only when every value is non-zero.
    for (int seat = 0; seat < 4; ++seat) {
        packet.put_short(0);
    }
}

void SendChampionCardState(CParty* party, const PartyState& state) {
    if (!party) {
        return;
    }
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SELECT_CARD);
    packet.put_byte(1);
    for (int card = 0; card < 4; ++card) {
        // State 9 is one composite round: owner A is the normal upper row
        // retained by CParty, owner B is the ChampionBreak lower row.
        packet.put_byte(
            party->GetSelectedCardOwnerForRuntime(0, card));
        const int champion_owner =
            party->GetSelectedCardOwnerForRuntime(1, card);
        packet.put_byte(champion_owner);
        if (champion_owner < 0) {
            packet.put_byte(0);
        } else {
            int count_index = packet.get_index();
            int reward_count = 0;
            packet.put_byte(0);

            // OFFICIAL_OWNER: the Champion card is the free gold-card row.
            // Preserve both canonical event_reward_item records, then append
            // the optional Champion_Break sentinel as a third tuple.
            const Inven_Item& money_reward =
                party->battle_data().event_reward_item(champion_owner, 0);
            const Inven_Item& item_reward =
                party->battle_data().event_reward_item(champion_owner, 1);
            packet.put_int(0);
            packet.put_int(money_reward.get_add_info());
            ++reward_count;
            if (item_reward.getKey() != 0) {
                packet.put_int(item_reward.getKey());
                packet.put_int(item_reward.get_add_info());
                ++reward_count;
            }
            if (state.card_buff[card] > 0) {
                // CLIENT_IDA_EVIDENCE: sub_557000 routes 0xffffffff plus
                // add_info to result group 4 while ChampionBreak is active.
                packet.put_int(-1);
                packet.put_int(state.card_buff[card]);
                ++reward_count;
            }
            packet.put_byte(count_index, reward_count);
        }
        packet.put_byte(party->battle_data().card_info_member_state(card));
    }
    // The handler is fixed-width: retain the ordinary four dummy rows without
    // sharing or mutating CParty::selected_cards_.
    for (int card = 4; card < 8; ++card) {
        packet.put_byte(-1);
        packet.put_byte(-1);
        packet.put_byte(0);
        packet.put_byte(0);
    }
    packet.finalize(true);
    party->send_to_party(packet);
}

void FinishUsers(CParty* party) {
    if (!party) {
        return;
    }
    for (int seat = 0; seat < 4; ++seat) {
        CUser* const user = party->get_user(seat);
        if (!user) {
            continue;
        }
        int active = 0;
        int pending = 0;
        {
            MutexGuard guard(&g_champion_break_mutex);
            UserState* const state = CurrentUserStateLocked(user, false);
            if (!state) {
                continue;
            }
            active = state->active_buff;
            pending = state->pending_buff;
            state->active_buff = 0;
            state->active_dungeon = 0;
            state->client_buff_present = false;
            state->weapon_projection_active = false;
            state->projected_weapon_item_id = 0;
            state->projected_weapon_original_upgrade = 0;
            state->projected_weapon_upgrade = 0;
            state->projected_weapon_server_id = 0;
            state->projected_weapon_datetime = 0;
            state->projected_weapon_sequence_no = 0;
        }
        if (active > 0) {
            SendBuffDelete(user, active);
        }
        (void)pending;
    }
}

int ActiveEffectParam(CUser* user, BuffEffectKind effect, size_t position) {
    MutexGuard guard(&g_champion_break_mutex);
    UserState* const state = CurrentUserStateLocked(user, false);
    if (!state || state->active_buff <= 0) {
        return 0;
    }
    const std::map<int, BuffDefinition>::const_iterator found =
        g_config.buffs.find(state->active_buff);
    if (found == g_config.buffs.end() || found->second.effect != effect ||
        position >= found->second.params.size()) {
        return 0;
    }
    return found->second.params[position];
}

}  // namespace

namespace champion_break {

bool LoadConfig() {
    Config parsed;
    const bool valid = LoadChampionConfig(parsed) && LoadBuffConfig(parsed);
    parsed.valid = valid;
    MutexGuard guard(&g_champion_break_mutex);
    g_config = parsed;
    g_party_states.clear();
    g_user_states.clear();
    return valid;
}

bool IsDungeonEnabled(int dungeon_index) {
    MutexGuard guard(&g_champion_break_mutex);
    return IsDungeonEnabledLocked(dungeon_index);
}

void WriteDungeonInfoEnableFlag(PacketGuard& packet, int dungeon_index) {
    packet.put_byte(IsDungeonEnabled(dungeon_index) ? 1 : 0);
}

void BeginDungeonRun(CParty* party, int dungeon_index) {
    if (!party) {
        return;
    }
    const bool eligible = IsDungeonEnabled(dungeon_index);
    {
        MutexGuard guard(&g_champion_break_mutex);
        PartyState state;
        state.dungeon_index = dungeon_index;
        state.eligible = eligible;
        g_party_states[party] = state;
    }

    for (int seat = 0; seat < 4; ++seat) {
        CUser* const user = party->get_user(seat);
        if (!user) {
            continue;
        }
        int stale_active = 0;
        int activated_buff = 0;
        int weapon_upgrade_target = 0;
        {
            MutexGuard guard(&g_champion_break_mutex);
            UserState* const state = CurrentUserStateLocked(user, true);
            if (!state) {
                continue;
            }
            stale_active = state->active_buff;
            state->active_buff = 0;
            state->active_dungeon = 0;
            state->client_buff_present = false;
            state->weapon_projection_active = false;
            state->projected_weapon_item_id = 0;
            state->projected_weapon_original_upgrade = 0;
            state->projected_weapon_upgrade = 0;
            state->projected_weapon_server_id = 0;
            state->projected_weapon_datetime = 0;
            state->projected_weapon_sequence_no = 0;
            if (eligible && state->pending_buff > 0) {
                state->active_buff = state->pending_buff;
                state->pending_buff = 0;
                state->active_dungeon = dungeon_index;
                state->client_buff_present = true;
                activated_buff = state->active_buff;
                const std::map<int, BuffDefinition>::const_iterator effect =
                    g_config.buffs.find(activated_buff);
                if (effect != g_config.buffs.end() &&
                    effect->second.effect == BUFF_EFFECT_WEAPON_UPGRADE &&
                    !effect->second.params.empty()) {
                    weapon_upgrade_target = effect->second.params[0];
                }
            }
        }
        if (stale_active > 0) {
            SendBuffDelete(user, stale_active);
        }
        if (weapon_upgrade_target > 0) {
            const Inven_Item* const weapon =
                user->getCurCharacInvenR()->GetInvenRef(
                    0, oldsrv::config::kEquipmentTypeWeapon);
            if (weapon && weapon->item_id_ > 0) {
                const int stored_upgrade = weapon->GetUpgrade();
                MutexGuard guard(&g_champion_break_mutex);
                UserState* const state = CurrentUserStateLocked(user, false);
                if (state && state->active_buff == activated_buff) {
                    state->weapon_projection_active = true;
                    state->projected_weapon_item_id = weapon->item_id_;
                    state->projected_weapon_original_upgrade = stored_upgrade;
                    state->projected_weapon_upgrade =
                        std::max(stored_upgrade, weapon_upgrade_target);
                    state->projected_weapon_server_id =
                        weapon->globally_unique_id_.server_id_;
                    state->projected_weapon_datetime =
                        weapon->globally_unique_id_.datetime_;
                    state->projected_weapon_sequence_no =
                        weapon->globally_unique_id_.sequence_no_;
                }
            }
        }
        if (activated_buff > 0) {
            // NOTI484 causes the 0828 client to apply the same one-match
            // effect (including its own temporary weapon clone) on entry.
            SendBuffAdd(user, activated_buff);
        }
    }
}

void OnMonsterKilled(CParty* party, int monster_kind) {
    if (!party || (monster_kind != 1 && monster_kind != 2)) {
        return;
    }
    bool notify = false;
    {
        MutexGuard guard(&g_champion_break_mutex);
        std::map<CParty*, PartyState>::iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() || !found->second.eligible ||
            found->second.phase == CARD_PHASE_COMPLETE) {
            return;
        }
        ++found->second.champion_count;
        if (!found->second.success_latched &&
            found->second.champion_count >= g_config.threshold) {
            found->second.success_latched = true;
            found->second.phase = CARD_PHASE_TRIGGERED;
            notify = true;
        }
    }
    if (notify) {
        SendChampionSuccess(party);
    }
}

bool WriteCardSelectRights(CParty* party, PacketGuard& packet) {
    bool champion_stage = false;
    {
        MutexGuard guard(&g_champion_break_mutex);
        std::map<CParty*, PartyState>::iterator found =
            g_party_states.find(party);
        if (found != g_party_states.end() && found->second.eligible &&
            found->second.success_latched &&
            found->second.phase != CARD_PHASE_COMPLETE) {
            found->second.phase = CARD_PHASE_RIGHTS_SENT;
            champion_stage = true;
        }
    }

    if (!champion_stage) {
        WriteOrdinaryRights(party, packet);
        return false;
    }

    // The client requires every one of the eight values to be non-zero for
    // state 9.  Existing seats therefore retain a right even at max fatigue;
    // empty seats use -1, which is also non-zero and cannot select a card.
    for (int seat = 0; seat < 4; ++seat) {
        packet.put_short(party && party->get_user(seat) ? 1 : -1);
    }
    for (int seat = 0; seat < 4; ++seat) {
        packet.put_short(party && party->get_user(seat) ? 1 : -1);
    }
    return true;
}

bool IsCombinedCardStage(CParty* party) {
    if (!party) {
        return false;
    }
    MutexGuard guard(&g_champion_break_mutex);
    const std::map<CParty*, PartyState>::const_iterator found =
        g_party_states.find(party);
    return found != g_party_states.end() &&
           found->second.phase == CARD_PHASE_RIGHTS_SENT;
}

bool TryHandleCardSelect(CUser* user, CParty* party, int card_index) {
    if (!user || !party) {
        return false;
    }
    {
        MutexGuard guard(&g_champion_break_mutex);
        const std::map<CParty*, PartyState>::const_iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() ||
            found->second.phase != CARD_PHASE_RIGHTS_SENT) {
            return false;
        }
    }

    if (card_index < 0 || card_index >= 4) {
        return true;
    }
    const int seat = UserSeat(party, user);
    if (seat < 0) {
        return true;
    }

    PartyState snapshot;
    int selected_buff = 0;
    {
        MutexGuard guard(&g_champion_break_mutex);
        std::map<CParty*, PartyState>::iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() ||
            found->second.phase != CARD_PHASE_RIGHTS_SENT ||
            found->second.user_card[seat] >= 0 ||
            found->second.card_owner[card_index] >= 0) {
            return true;
        }
        if (party->SetChampionCardNumberForRuntime(
                user, static_cast<char>(card_index)) != 1) {
            return true;
        }
        selected_buff = RollBuffLocked(found->second);
        found->second.user_card[seat] = card_index;
        found->second.card_owner[card_index] = seat;
        found->second.card_buff[card_index] = selected_buff;
        snapshot = found->second;
    }

    if (selected_buff > 0) {
        StorePendingBuff(user, selected_buff);
    }
    SendChampionCardState(party, snapshot);
    FinishCombinedCardSelectIfReady(party);
    return true;
}

void SendCombinedCardInfo(CParty* party) {
    if (!party) {
        return;
    }
    PartyState snapshot;
    {
        MutexGuard guard(&g_champion_break_mutex);
        const std::map<CParty*, PartyState>::const_iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() ||
            found->second.phase != CARD_PHASE_RIGHTS_SENT) {
            return;
        }
        snapshot = found->second;
    }
    SendChampionCardState(party, snapshot);
}

bool FinishCombinedCardSelectIfReady(CParty* party) {
    if (!AllValidUsersSelectedNormalCard(party)) {
        return false;
    }
    {
        MutexGuard guard(&g_champion_break_mutex);
        std::map<CParty*, PartyState>::iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() ||
            found->second.phase != CARD_PHASE_RIGHTS_SENT ||
            !AllValidUsersSelected(party, found->second)) {
            return false;
        }
        found->second.phase = CARD_PHASE_COMPLETE;
    }
    party->set_dungeon_clear_state(5);
    party->dungeon_clear();
    return true;
}

bool HandleCardSelectTimeout(CParty* party) {
    if (!party) {
        return false;
    }
    if (!IsCombinedCardStage(party)) {
        return false;
    }
    // CompleteCardSelect owns only selected_cards_[0], so it is safe for the
    // composite timeout and cannot manufacture legacy type-1/gold rewards.
    if (!AllValidUsersSelectedNormalCard(party)) {
        party->CompleteCardSelect();
    }

    PartyState snapshot;
    CUser* award_users[4] = {0, 0, 0, 0};
    int award_buffs[4] = {0, 0, 0, 0};
    int award_count = 0;
    {
        MutexGuard guard(&g_champion_break_mutex);
        std::map<CParty*, PartyState>::iterator found =
            g_party_states.find(party);
        if (found == g_party_states.end() ||
            found->second.phase != CARD_PHASE_RIGHTS_SENT) {
            return false;
        }
        for (int seat = 0; seat < 4; ++seat) {
            if (party->checkValidUser(seat) != 1 ||
                found->second.user_card[seat] >= 0) {
                continue;
            }
            int card = 0;
            while (card < 4 && found->second.card_owner[card] >= 0) {
                ++card;
            }
            if (card >= 4) {
                break;
            }
            CUser* const award_user = party->get_user(seat);
            if (!award_user ||
                party->SetChampionCardNumberForRuntime(
                    award_user, static_cast<char>(card)) != 1) {
                continue;
            }
            const int buff = RollBuffLocked(found->second);
            found->second.user_card[seat] = card;
            found->second.card_owner[card] = seat;
            found->second.card_buff[card] = buff;
            if (buff > 0 && award_count < 4) {
                award_users[award_count] = award_user;
                award_buffs[award_count] = buff;
                ++award_count;
            }
        }
        found->second.phase = CARD_PHASE_COMPLETE;
        snapshot = found->second;
    }
    for (int index = 0; index < award_count; ++index) {
        StorePendingBuff(award_users[index], award_buffs[index]);
    }
    SendChampionCardState(party, snapshot);
    party->set_dungeon_clear_state(5);
    party->dungeon_clear();
    return true;
}

void FinishDungeonRun(CParty* party) {
    if (!party) {
        return;
    }
    FinishUsers(party);
    MutexGuard guard(&g_champion_break_mutex);
    g_party_states.erase(party);
}

void AbortDungeonRun(CParty* party) {
    FinishDungeonRun(party);
}

void ResetSession(CUser* user) {
    if (!user) {
        return;
    }
    int active_buff = 0;
    {
        MutexGuard guard(&g_champion_break_mutex);
        const std::map<CUser*, UserState>::const_iterator found =
            g_user_states.find(user);
        if (found != g_user_states.end()) {
            active_buff = found->second.active_buff;
        }
        // Erasing the sidecar removes the weapon projection immediately; the
        // canonical equipment record was never modified.
        g_user_states.erase(user);
    }
    if (active_buff > 0) {
        SendBuffDelete(user, active_buff);
    }
}

int GetHunterExpBonusRate(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_HUNTER_EXP, 0u);
}

int GetClearExpBonusRate(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_CLEAR_EXP, 0u);
}

int GetMagicSealDropBonusRate(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_MAGIC_SEAL_DROP, 2u);
}

int GetMagicSealMinimumUpgrade(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_MAGIC_SEAL_DROP, 0u);
}

int GetMagicSealMaximumUpgrade(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_MAGIC_SEAL_DROP, 1u);
}

int GetUniqueDropBonusRate(CUser* user) {
    return ActiveEffectParam(user, BUFF_EFFECT_UNIQUE_DROP, 1u);
}

int GetProjectedEquipmentUpgrade(CUser* user,
                                 int equipment_slot,
                                 int item_id,
                                 const itemGloballyUniqueIdentifier_t& guid,
                                 int stored_upgrade) {
    if (!user || equipment_slot != oldsrv::config::kEquipmentTypeWeapon ||
        item_id <= 0) {
        return stored_upgrade;
    }
    MutexGuard guard(&g_champion_break_mutex);
    UserState* const state = CurrentUserStateLocked(user, false);
    if (!state || !state->weapon_projection_active ||
        state->projected_weapon_item_id != item_id ||
        state->projected_weapon_server_id != guid.server_id_ ||
        state->projected_weapon_datetime != guid.datetime_ ||
        state->projected_weapon_sequence_no != guid.sequence_no_) {
        return stored_upgrade;
    }
    return std::max(stored_upgrade, state->projected_weapon_upgrade);
}

void RecordHunterExpBonus(CUser* user, int bonus_exp) {
    if (!user) {
        return;
    }
    MutexGuard guard(&g_champion_break_mutex);
    UserState* const state = CurrentUserStateLocked(user, true);
    if (state) {
        state->last_hunter_exp_bonus = std::max(0, bonus_exp);
    }
}

int ConsumeHunterExpBonus(CUser* user) {
    if (!user) {
        return 0;
    }
    MutexGuard guard(&g_champion_break_mutex);
    UserState* const state = CurrentUserStateLocked(user, false);
    if (!state) {
        return 0;
    }
    const int value = state->last_hunter_exp_bonus;
    state->last_hunter_exp_bonus = 0;
    return value;
}

}  // namespace champion_break
