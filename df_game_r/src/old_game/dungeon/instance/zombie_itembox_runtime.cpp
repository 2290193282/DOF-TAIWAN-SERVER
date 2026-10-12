#include "old_game/dungeon/instance/zombie_itembox_runtime.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"

#include <algorithm>
#include <map>
#include <vector>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"

namespace {

struct Config {
    Config()
        : passive_index(0),
          world_x(0),
          world_y(0),
          first_delay_ms(0),
          repeat_delay_ms(0),
          selected_zombie_count(0),
          valid(false) {}

    uint32_t passive_index;
    uint16_t world_x;
    uint16_t world_y;
    uint32_t first_delay_ms;
    uint32_t repeat_delay_ms;
    uint8_t selected_zombie_count;
    bool valid;
};

struct Spawned {
    Spawned() : grid_x(0), grid_y(0) {}
    Spawned(uint16_t x, uint16_t y) : grid_x(x), grid_y(y) {}

    uint16_t grid_x;
    uint16_t grid_y;
};

struct PartyState {
    PartyState()
        : party(0),
          timer_key(0),
          zombie_selection_sent(false),
          zombie_seats(),
          spawned() {}

    CParty* party;
    uint32_t timer_key;
    bool zombie_selection_sent;
    std::vector<uint32_t> zombie_seats;
    std::map<uint32_t, Spawned> spawned;
};

Mutex g_zombie_itembox_mutex;
Config g_zombie_itembox_config;
std::map<int, PartyState> g_zombie_itembox_states;
uint32_t g_next_zombie_itembox_timer_key = 1;

bool IsZombieParty(CParty* party) {
    if (!party) {
        return false;
    }
    CDungeon* const dungeon = party->battle_field().get_dungeon();
    return dungeon && dungeon->isZombieDungeon();
}

uint32_t NextTimerKeyLocked() {
    uint32_t key = g_next_zombie_itembox_timer_key++;
    if (key == 0) {
        key = g_next_zombie_itembox_timer_key++;
    }
    if (g_next_zombie_itembox_timer_key == 0) {
        ++g_next_zombie_itembox_timer_key;
    }
    return key;
}

void BroadcastGenerated(CParty* party,
                        uint32_t runtime_uid,
                        const Config& config,
                        uint16_t grid_x,
                        uint16_t grid_y) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GENERATE_ITEMBOX);
    packet.put_int(static_cast<int>(runtime_uid));
    packet.put_int(static_cast<int>(config.passive_index));
    packet.put_short(grid_x);
    packet.put_short(grid_y);
    packet.put_short(config.world_x);
    packet.put_short(config.world_y);
    packet.put_short(0);
    packet.finalize(true);
    party->send_to_party(packet);
}

void BroadcastClaimed(CParty* party, uint32_t runtime_uid) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_GET_ITEMBOX);
    packet.put_int(static_cast<int>(runtime_uid));
    packet.finalize(true);
    party->send_to_party(packet);
}

void BroadcastSelectedZombies(CParty* party,
                              const std::vector<uint32_t>& seats) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SELECT_ZOMBIE);
    packet.put_byte(static_cast<int>(seats.size()));
    for (std::vector<uint32_t>::const_iterator seat = seats.begin();
         seat != seats.end();
         ++seat) {
        packet.put_int(static_cast<int>(*seat));
    }
    packet.finalize(true);
    party->send_to_party(packet);
}

bool IsStateActiveLocked(CParty* party, uint32_t timer_key) {
    if (!party || !g_zombie_itembox_config.valid || !IsZombieParty(party)) {
        return false;
    }
    const int party_index = party->GetPartyIndex();
    const std::map<int, PartyState>::const_iterator found =
        g_zombie_itembox_states.find(party_index);
    return found != g_zombie_itembox_states.end() &&
           found->second.party == party &&
           found->second.timer_key == timer_key;
}

}  // namespace

namespace ZombieItemBox {

bool LoadConfig() {
    Config parsed;
    zombie_mode::ItemBoxPolicy policy;
    const bool loaded = zombie_mode::LoadConfig() &&
                        zombie_mode::GetItemBoxPolicy(policy);
    if (loaded) {
        parsed.passive_index = policy.passive_index;
        parsed.world_x = policy.world_x;
        parsed.world_y = policy.world_y;
        parsed.first_delay_ms = policy.first_delay_ms;
        parsed.repeat_delay_ms = policy.repeat_delay_ms;
        parsed.selected_zombie_count = policy.initial_zombie_count;
        parsed.valid = parsed.passive_index != 0u &&
                       parsed.repeat_delay_ms != 0u &&
                       parsed.selected_zombie_count != 0u;
    }
    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        g_zombie_itembox_config = parsed;
        g_zombie_itembox_states.clear();
    }
    return parsed.valid;
}

void Reset(CParty* party) {
    if (!party) {
        return;
    }

    const int party_index = party->GetPartyIndex();
    MutexGuard guard(&g_zombie_itembox_mutex);
    std::map<int, PartyState>::iterator found =
        g_zombie_itembox_states.find(party_index);
    if (found != g_zombie_itembox_states.end() &&
        found->second.party == party) {
        g_zombie_itembox_states.erase(found);
    }
}

void Start(CParty* party) {
    Reset(party);
    if (zombie_map_runtime::IsRegisteredParty(party)) {
        // A matching adapter must never acquire a per-party timer/claim
        // table.  StartRoom owns the only legal central transition.
        return;
    }
    if (!party || !IsZombieParty(party)) {
        return;
    }

    const int party_index = party->GetPartyIndex();
    if (party_index < 0) {
        return;
    }

    uint32_t timer_key = 0;
    uint32_t first_delay_ms = 0;
    bool schedule_itembox = false;
    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        if (!g_zombie_itembox_config.valid &&
            g_zombie_itembox_config.selected_zombie_count == 0u) {
            return;
        }
        timer_key = NextTimerKeyLocked();
        first_delay_ms = g_zombie_itembox_config.first_delay_ms;
        schedule_itembox = g_zombie_itembox_config.valid;
        PartyState state;
        state.party = party;
        state.timer_key = timer_key;
        g_zombie_itembox_states[party_index] = state;
    }

    if (schedule_itembox) {
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            party_index,
            kGenerateTimerMessage,
            static_cast<int64_t>(first_delay_ms),
            timer_key,
            0);
    }
}

bool StartRoom(PvP_Room* room, uint32_t generation) {
    Config config;
    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        if (!g_zombie_itembox_config.valid) return false;
        config = g_zombie_itembox_config;
    }
    zombie_map_runtime::ItemBoxConfig central;
    central.passive_index = config.passive_index;
    central.world_x = config.world_x;
    central.world_y = config.world_y;
    central.first_delay_ms = config.first_delay_ms;
    central.repeat_delay_ms = config.repeat_delay_ms;
    return zombie_map_runtime::StartRoomItemBox(
        room, generation, central);
}

int OnGenerateTimer(CParty* party, uint32_t timer_key) {
    if (!party) {
        return 0;
    }

    Config config;
    uint32_t runtime_uid = 0;
    uint16_t grid_x = 0;
    uint16_t grid_y = 0;
    bool generated = false;
    const int party_index = party->GetPartyIndex();

    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        if (!IsStateActiveLocked(party, timer_key)) {
            std::map<int, PartyState>::iterator stale =
                g_zombie_itembox_states.find(party_index);
            if (stale != g_zombie_itembox_states.end() &&
                stale->second.party == party &&
                stale->second.timer_key == timer_key) {
                g_zombie_itembox_states.erase(stale);
            }
            return 0;
        }

        config = g_zombie_itembox_config;
        int current_x = 0;
        int current_y = 0;
        party->battle_field().getCurPosXY(current_x, current_y);
        MapInfo* const map_info =
            party->battle_field().GetCurrentMapInfo();
        if (map_info && current_x >= 0 && current_x <= 0xffff &&
            current_y >= 0 && current_y <= 0xffff) {
            grid_x = static_cast<uint16_t>(current_x);
            grid_y = static_cast<uint16_t>(current_y);

            PartyState& state = g_zombie_itembox_states[party_index];
            // MapInfo counters are map-local while this sidecar survives map
            // transitions.  Consume colliding candidates until the party's
            // unclaimed UID set accepts one; never broadcast an unowned UID.
            while (true) {
                runtime_uid = static_cast<uint32_t>(
                    map_info->next_drop_item_unique_id_++);
                if (state.spawned
                        .insert(std::make_pair(
                            runtime_uid, Spawned(grid_x, grid_y)))
                        .second) {
                    break;
                }
            }
            generated = true;
        }
    }

    if (generated) {
        // The 0828 client creates/applies the passive object before it sends
        // CMD517.  The server owns only runtime identity and claim-once.
        BroadcastGenerated(party, runtime_uid, config, grid_x, grid_y);
    }

    bool repeat = false;
    uint32_t repeat_delay_ms = 0;
    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        repeat = IsStateActiveLocked(party, timer_key);
        if (repeat) {
            repeat_delay_ms = g_zombie_itembox_config.repeat_delay_ms;
        }
    }
    if (repeat) {
        G_TimerQueue()->InsertTimerInMilisecond(
            1,
            party_index,
            kGenerateTimerMessage,
            static_cast<int64_t>(repeat_delay_ms),
            timer_key,
            0);
    }
    return 0;
}

int Claim(CParty* party, uint32_t runtime_uid) {
    if (!party || !IsZombieParty(party)) {
        return 19;
    }

    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        const int party_index = party->GetPartyIndex();
        std::map<int, PartyState>::iterator state =
            g_zombie_itembox_states.find(party_index);
        if (state == g_zombie_itembox_states.end() ||
            state->second.party != party) {
            return 19;
        }

        std::map<uint32_t, Spawned>::iterator spawned =
            state->second.spawned.find(runtime_uid);
        if (spawned == state->second.spawned.end()) {
            return 19;
        }

        int current_x = 0;
        int current_y = 0;
        party->battle_field().getCurPosXY(current_x, current_y);
        if (current_x != spawned->second.grid_x ||
            current_y != spawned->second.grid_y) {
            return 19;
        }
        state->second.spawned.erase(spawned);
    }

    BroadcastClaimed(party, runtime_uid);
    return 0;
}

int SelectZombies(CParty* party,
                  std::vector<uint32_t>& selected_seats,
                  bool& newly_selected) {
    selected_seats.clear();
    newly_selected = false;
    if (!party || !IsZombieParty(party)) {
        return 19;
    }

    {
        MutexGuard guard(&g_zombie_itembox_mutex);
        const int party_index = party->GetPartyIndex();
        std::map<int, PartyState>::iterator state =
            g_zombie_itembox_states.find(party_index);
        if (party_index < 0 || state == g_zombie_itembox_states.end() ||
            state->second.party != party ||
            g_zombie_itembox_config.selected_zombie_count == 0u) {
            return 19;
        }

        if (!state->second.zombie_selection_sent) {
            std::vector<uint32_t> candidates;
            for (int seat = 0; seat < 4; ++seat) {
                if (static_cast<unsigned char>(
                        party->checkValidUser(seat)) == 1u) {
                    candidates.push_back(static_cast<uint32_t>(seat));
                }
            }
            if (candidates.empty()) {
                return 19;
            }
            std::random_shuffle(candidates.begin(), candidates.end());
            const size_t selection_count = std::min(
                candidates.size(),
                static_cast<size_t>(
                    g_zombie_itembox_config.selected_zombie_count));
            state->second.zombie_seats.assign(
                candidates.begin(), candidates.begin() + selection_count);
            std::sort(state->second.zombie_seats.begin(),
                      state->second.zombie_seats.end());
            state->second.zombie_selection_sent = true;
            newly_selected = true;
        }
        selected_seats = state->second.zombie_seats;
    }
    return selected_seats.empty() ? 19 : 0;
}

}  // namespace ZombieItemBox

stGetItemBoxParam::stGetItemBoxParam() : ParamBase(), error(19) {}

DisPatcher_GetItemBox::DisPatcher_GetItemBox()
    : IPacketDispatcher<MSG_GET_ITEMBOX,
                        stGetItemBoxParam,
                        static_cast<ch_state>(5)>() {}

DisPatcher_GetItemBox::~DisPatcher_GetItemBox() {}

int DisPatcher_GetItemBox::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GET_ITEMBOX& request = static_cast<MSG_GET_ITEMBOX&>(message);
    return packet.get_int(request.runtime_uid) == 1 ? 0 : 1;
}

int DisPatcher_GetItemBox::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    const MSG_GET_ITEMBOX& request =
        static_cast<const MSG_GET_ITEMBOX&>(message);
    stGetItemBoxParam& result = static_cast<stGetItemBoxParam&>(param);
    if (!user || user->get_state() != 5) {
        result.error = 19;
        return 0;
    }

    CParty* const party = user->GetParty();
    uint32_t provenance_token = 0u;
    int central_error = 19;
    const zombie_map_runtime::HookResult central_claim =
        zombie_map_runtime::ClaimRoomItemBox(
            user,
            request.runtime_uid,
            &central_error,
            &provenance_token);
    if (central_claim == zombie_map_runtime::kHookNotManaged) {
        result.error = party
                           ? ZombieItemBox::Claim(party, request.runtime_uid)
                           : 19;
    } else {
        // CMD517 is the complete use transaction for the temporary passive
        // object; it is not an inventory stackable.  Consume the exact
        // claimant/cell provenance produced by ClaimRoomItemBox here so a
        // later ordinary CMD44 can never borrow it.
        uint32_t consumed_token = 0u;
        if (central_error == 0 &&
            (provenance_token == 0u ||
             !zombie_map_runtime::ConsumeItemBoxProvenance(
                 user, &consumed_token) ||
             consumed_token != provenance_token)) {
            central_error = 19;
        }
        result.error = central_error;
    }
    return 0;
}

void DisPatcher_GetItemBox::send(CUser* user, ParamBase& param) {
    const stGetItemBoxParam& result =
        static_cast<const stGetItemBoxParam&>(param);
    if (result.error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_GET_ITEMBOX,
            static_cast<unsigned char>(result.error));
    } else {
        user->SendCmdOkPacket(ENUM_CMDPACKET_GET_ITEMBOX);
    }
}

stSelectZombieParam::stSelectZombieParam() : ParamBase(), error(19) {}

DisPatcher_SelectZombie::DisPatcher_SelectZombie()
    : IPacketDispatcher<MSG_SELECT_ZOMBIE,
                        stSelectZombieParam,
                        static_cast<ch_state>(5)>() {}

DisPatcher_SelectZombie::~DisPatcher_SelectZombie() {}

int DisPatcher_SelectZombie::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SELECT_ZOMBIE& request =
        static_cast<MSG_SELECT_ZOMBIE&>(message);
    return packet.get_short(request.reserved_zero) == 1 ? 0 : 1;
}

int DisPatcher_SelectZombie::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    const MSG_SELECT_ZOMBIE& request =
        static_cast<const MSG_SELECT_ZOMBIE&>(message);
    stSelectZombieParam& result =
        static_cast<stSelectZombieParam&>(param);
    if (!user || user->get_state() != 5 || request.reserved_zero != 0u) {
        result.error = 19;
        return 0;
    }

    std::vector<uint32_t> seats;
    bool newly_selected = false;
    PvP_Room* const room = user->GetPVPRoom();
    CParty* const party = user->GetParty();
    if (room) {
        result.error = zombie_mode::SelectInitialZombies(
            room, seats, newly_selected);
    } else {
        result.error = party
                           ? ZombieItemBox::SelectZombies(
                                 party, seats, newly_selected)
                           : 19;
    }
    if (!room && result.error == 0 && newly_selected) {
        // CLIENT_IDA_EVIDENCE: NOTI474 sub_534FE0 consumes an array of
        // party-seat u32 values; sub_534D50 maps listed seats to zombie and
        // every remaining seat to human.
        BroadcastSelectedZombies(party, seats);
    }
    return 0;
}

void DisPatcher_SelectZombie::send(CUser* user, ParamBase& param) {
    const stSelectZombieParam& result =
        static_cast<const stSelectZombieParam&>(param);
    if (result.error != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_SELECT_ZOMBIE,
            static_cast<unsigned char>(result.error));
    } else {
        user->SendCmdOkPacket(ENUM_CMDPACKET_SELECT_ZOMBIE);
    }
}

TimerZombieItemBoxGenerate::TimerZombieItemBoxGenerate() : TimerSig() {}

TimerZombieItemBoxGenerate::~TimerZombieItemBoxGenerate() {}

int TimerZombieItemBoxGenerate::dispatch_sig(int object_id,
                                             int param1,
                                             unsigned int param2) {
    if (param2 != 0u) {
        // Central timers use room_index/object_id and generation/param2.
        // Never reinterpret a stale room timer as an unrelated pooled party.
        (void)zombie_map_runtime::OnRoomItemBoxTimer(
            object_id,
            static_cast<uint32_t>(param1),
            static_cast<uint32_t>(param2));
        return 0;
    }
    CParty* const party = G_CGameManager()->GetParty(object_id);
    return party ? ZombieItemBox::OnGenerateTimer(
                       party, static_cast<uint32_t>(param1))
                 : 0;
}

template class IPacketDispatcher<MSG_GET_ITEMBOX,
                                 stGetItemBoxParam,
                                 static_cast<ch_state>(5)>;
template class IPacketDispatcher<MSG_SELECT_ZOMBIE,
                                 stSelectZombieParam,
                                 static_cast<ch_state>(5)>;
