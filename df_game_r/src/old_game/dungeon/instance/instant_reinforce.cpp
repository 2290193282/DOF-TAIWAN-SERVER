#include "old_game/dungeon/instance/instant_reinforce.h"

#include <limits>
#include <map>

#include "common/manager/play_info_public_abi.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/packet/packet_buf.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

namespace instant_reinforce {
namespace {

struct AuthoritativeContext {
    AuthoritativeContext()
        : user(0),
          party(0),
          charac_no(0),
          unique_id(0),
          party_index(-1),
          seat(-1),
          dungeon_index(0),
          map_index(0),
          select_index(-1) {}

    CUser* user;
    CParty* party;
    int charac_no;
    int unique_id;
    int party_index;
    int seat;
    int dungeon_index;
    int map_index;
    int select_index;
};

struct ReviveLatch {
    ReviveLatch()
        : user(0),
          party(0),
          charac_no(0),
          unique_id(0),
          party_index(-1),
          dungeon_index(0),
          map_index(0),
          select_index(-1),
          generation(0),
          corpse_seen(false),
          armed(false),
          consumed(false) {}

    CUser* user;
    CParty* party;
    int charac_no;
    int unique_id;
    int party_index;
    int dungeon_index;
    int map_index;
    int select_index;
    uint64_t generation;
    bool corpse_seen;
    bool armed;
    bool consumed;
};

struct PartyActivation {
    PartyActivation()
        : party(0),
          party_index(-1),
          dungeon_index(0),
          map_index(0),
          generation(0) {}

    CParty* party;
    int party_index;
    int dungeon_index;
    int map_index;
    uint64_t generation;
};

typedef std::map<CUser*, ReviveLatch> LatchTable;
typedef std::map<CParty*, PartyActivation> ActivationTable;

Mutex& StateMutex() {
    static Mutex mutex;
    return mutex;
}

LatchTable& Latches() {
    static LatchTable latches;
    return latches;
}

ActivationTable& Activations() {
    static ActivationTable activations;
    return activations;
}

uint64_t& NextGeneration() {
    static uint64_t generation = 0;
    return generation;
}

uint64_t AllocateGeneration() {
    uint64_t& value = NextGeneration();
    ++value;
    if (value == 0) ++value;
    return value;
}

bool BuildContext(CUser* user,
                  CParty* expected_party,
                  bool require_live,
                  AuthoritativeContext* output) {
    if (!user || !output || user->get_state() != 5 ||
        user->is_fighting() == 0) {
        return false;
    }

    CParty* const party = user->GetParty();
    if (!party || (expected_party && expected_party != party)) return false;
    const int seat = party->get_party_seatno(user);
    if (seat < 0 || seat > 3 || party->_checkValidUser(seat) != 1 ||
        party->get_user(seat) != user ||
        (require_live && party->battle_data().member_live(seat) != 1u)) {
        return false;
    }

    CBattle_Field& field = party->battle_field();
    const int dungeon_index = field.get_dungeon_index();
    const int map_index = field.GetCurrentMapIndex();
    const int charac_no = user->getCurCharacNo();
    const int unique_id = user->get_unique_id();
    if (!field.get_dungeon() || dungeon_index <= 0 || map_index <= 0 ||
        charac_no <= 0 || unique_id <= 0) {
        return false;
    }

    output->user = user;
    output->party = party;
    output->charac_no = charac_no;
    output->unique_id = unique_id;
    output->party_index = party->GetPartyIndex();
    output->seat = seat;
    output->dungeon_index = dungeon_index;
    output->map_index = map_index;
    output->select_index = user->GetCurCharacSkillTreeIndex();
    return true;
}

bool SameIdentityAndMap(const ReviveLatch& latch,
                        const AuthoritativeContext& context) {
    return latch.user == context.user && latch.party == context.party &&
           latch.charac_no == context.charac_no &&
           latch.unique_id == context.unique_id &&
           latch.party_index == context.party_index &&
           latch.dungeon_index == context.dungeon_index &&
           latch.map_index == context.map_index &&
           latch.select_index == context.select_index;
}

bool SamePartyMap(const PartyActivation& activation,
                  const AuthoritativeContext& context) {
    return activation.party == context.party &&
           activation.party_index == context.party_index &&
           activation.dungeon_index == context.dungeon_index &&
           activation.map_index == context.map_index;
}

void ErasePartyLatches(CParty* party) {
    LatchTable& latches = Latches();
    for (LatchTable::iterator latch = latches.begin();
         latch != latches.end();) {
        if (latch->second.party == party) {
            LatchTable::iterator stale = latch++;
            latches.erase(stale);
        } else {
            ++latch;
        }
    }
}

uint32_t SaturatingAddU32(uint32_t base, int addition) {
    const uint64_t result = static_cast<uint64_t>(base) +
                            static_cast<uint64_t>(addition);
    return result > std::numeric_limits<uint32_t>::max()
               ? std::numeric_limits<uint32_t>::max()
               : static_cast<uint32_t>(result);
}

uint16_t SaturatingAddU16(uint16_t base, int addition) {
    const unsigned int result = static_cast<unsigned int>(base) +
                                static_cast<unsigned int>(addition);
    return result > std::numeric_limits<uint16_t>::max()
               ? std::numeric_limits<uint16_t>::max()
               : static_cast<uint16_t>(result);
}

bool ApplyStatusGrant(const StatusGrant& grant, _Additioal_info* value) {
    if (grant.additive_value <= 0) return false;
    switch (grant.status_type) {
        case 0:
            value->physical_attack = SaturatingAddU16(
                value->physical_attack, grant.additive_value);
            return true;
        case 1:
            value->magical_attack = SaturatingAddU16(
                value->magical_attack, grant.additive_value);
            return true;
        case 10:
            value->attack_speed = SaturatingAddU16(
                value->attack_speed, grant.additive_value);
            return true;
        case 11:
            value->move_speed = SaturatingAddU32(
                value->move_speed, grant.additive_value);
            return true;
        case 12:
            value->cast_speed = SaturatingAddU16(
                value->cast_speed, grant.additive_value);
            return true;
        case 13:
            value->max_hp = SaturatingAddU32(
                value->max_hp, grant.additive_value);
            return true;
        case 48:
            value->max_mp = SaturatingAddU32(
                value->max_mp, grant.additive_value);
            return true;
        default:
            return false;
    }
}

}  // namespace

AppliedSnapshot::AppliedSnapshot()
    : map_index(0),
      select_index(-1),
      skill_header(12),
      generation(0),
      skills(),
      statuses(),
      effects() {}

void ObserveAuthoritativeLifeTransition(CUser* user,
                                        CParty* party,
                                        int previous_live,
                                        int new_state) {
    // Only the real 1 -> 0 -> (1|2) CParty live-state sequence may mint a
    // CMD555 credential.  Calling this function with repeated states cannot
    // create a generation or re-arm a consumed request.
    const bool corpse_transition = previous_live == 1 && new_state == 0;
    const bool revive_transition = previous_live == 0 &&
                                   (new_state == 1 || new_state == 2);
    if (!corpse_transition && !revive_transition) return;

    AuthoritativeContext context;
    if (!IsConfigLoaded() || !BuildContext(user, party, false, &context)) {
        if (user) ResetUser(user);
        return;
    }

    const int current_live =
        context.party->battle_data().member_live(context.seat);
    if ((corpse_transition && current_live != 0) ||
        (revive_transition && current_live != 1)) {
        ResetUser(user);
        return;
    }

    Rule rule;
    if (!ResolveRule(context.map_index, context.select_index, &rule) ||
        !rule.on_revive_after_corpse) {
        ResetUser(user);
        return;
    }

    MutexGuard guard(&StateMutex());
    LatchTable& latches = Latches();
    if (corpse_transition) {
        ReviveLatch latch;
        latch.user = context.user;
        latch.party = context.party;
        latch.charac_no = context.charac_no;
        latch.unique_id = context.unique_id;
        latch.party_index = context.party_index;
        latch.dungeon_index = context.dungeon_index;
        latch.map_index = context.map_index;
        latch.select_index = context.select_index;
        latch.generation = AllocateGeneration();
        latch.corpse_seen = true;
        latch.armed = false;
        latch.consumed = false;
        latches[user] = latch;
        return;
    }

    LatchTable::iterator found = latches.find(user);
    if (found == latches.end() || !found->second.corpse_seen ||
        found->second.armed || found->second.consumed ||
        !SameIdentityAndMap(found->second, context)) {
        if (found != latches.end()) latches.erase(found);
        return;
    }
    found->second.armed = true;
}

void ResetUser(CUser* user) {
    if (!user) return;
    MutexGuard guard(&StateMutex());
    Latches().erase(user);
}

void ResetParty(CParty* party) {
    if (!party) return;
    MutexGuard guard(&StateMutex());
    Activations().erase(party);
    ErasePartyLatches(party);
}

int HandleRequest(CUser* user, PacketBuf& packet) {
    // CLIENT_IDA_EVIDENCE: sub_A664D0 calls BeginCmd(555) and immediately
    AuthoritativeContext context;
    Rule rule;
    if (!IsConfigLoaded() ||
        !BuildContext(user, 0, true, &context) ||
        !ResolveRule(context.map_index, context.select_index, &rule) ||
        !rule.on_revive_after_corpse) {
        return 0;
    }

    MutexGuard guard(&StateMutex());
    LatchTable& latches = Latches();
    LatchTable::iterator found = latches.find(user);
    if (found == latches.end() || !found->second.corpse_seen ||
        !found->second.armed || found->second.consumed ||
        !SameIdentityAndMap(found->second, context)) {
        return 0;
    }

    // Consume before publishing the party projection.  A replay can neither
    // create another activation nor refresh its generation.
    found->second.consumed = true;
    PartyActivation& activation = Activations()[context.party];
    if (!SamePartyMap(activation, context)) {
        activation.party = context.party;
        activation.party_index = context.party_index;
        activation.dungeon_index = context.dungeon_index;
        activation.map_index = context.map_index;
        activation.generation = found->second.generation;
    }
    return 0;
}

bool CopyAppliedSnapshot(CUser* user, AppliedSnapshot* output) {
    if (!output || !IsConfigLoaded()) return false;

    AuthoritativeContext context;
    Rule rule;
    if (!BuildContext(user, 0, false, &context) ||
        !ResolveRule(context.map_index, context.select_index, &rule)) {
        return false;
    }

    MutexGuard guard(&StateMutex());
    const ActivationTable& activations = Activations();
    const ActivationTable::const_iterator found =
        activations.find(context.party);
    if (found == activations.end() ||
        !SamePartyMap(found->second, context)) {
        return false;
    }

    AppliedSnapshot projected;
    projected.map_index = context.map_index;
    projected.select_index = rule.select_index;
    projected.skill_header = rule.skill_header;
    projected.generation = found->second.generation;
    projected.skills = rule.skills;
    projected.statuses = rule.statuses;
    projected.effects = rule.effects;
    *output = projected;
    return true;
}

int GetEffectiveSkillLevel(CUser* user,
                           int skill_index,
                           int base_level) {
    AppliedSnapshot projected;
    if (skill_index <= 0 ||
        !CopyAppliedSnapshot(user, &projected)) {
        return base_level;
    }
    for (std::vector<SkillGrant>::const_iterator skill =
             projected.skills.begin();
         skill != projected.skills.end(); ++skill) {
        if (skill->skill_index == skill_index && skill->level > base_level) {
            return skill->level;
        }
    }
    return base_level;
}

bool ApplyToStatusSnapshot(CUser* user, _Additioal_info* scratch) {
    if (!scratch) return false;
    AppliedSnapshot projected;
    if (!CopyAppliedSnapshot(user, &projected)) return false;

    _Additioal_info result = *scratch;
    for (std::vector<StatusGrant>::const_iterator status =
             projected.statuses.begin();
         status != projected.statuses.end(); ++status) {
        if (!ApplyStatusGrant(*status, &result)) return false;
    }
    *scratch = result;
    return true;
}

}  // namespace instant_reinforce_0828
