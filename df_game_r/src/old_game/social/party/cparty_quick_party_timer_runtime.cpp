#include "old_game/social/party/party.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x859BFE2
// IDA_NAME: _ZN6CParty30increase_dimension_inout_countEi
// DEMANGLED: CParty::increase_dimension_inout_count(int)
char CParty::increase_dimension_inout_count(int value) {
    CDungeon* const dungeon = battle_field_.get_dungeon();
    if (!dungeon) {
        return 0;
    }
    if (dungeon->get_dimension_possible() == 0) {
        return 1;
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (checkValidUser(seat) != 1) {
            continue;
        }

        CUser* const user = members_[seat].user;
        if (!user) {
            continue;
        }
        for (int dimension = 0; dimension <= 5; ++dimension) {
            const int maximum =
                value + static_cast<signed char>(
                            G_CDataManager()->get_dimensionInout(dimension));
            int updated = value + static_cast<signed char>(
                                      user->getDemensionInoutValue(
                                          static_cast<char>(dimension)));
            if (updated > maximum) {
                updated = maximum;
            }
            user->setDemensionInoutValue(static_cast<char>(dimension),
                                         static_cast<char>(updated));
        }
    }
    return 0;
}

// IDA_EA: 0x85BDD2A
// IDA_NAME: _ZN6CParty18getDungeonTypeKindEPK8CDungeon
// DEMANGLED: CParty::getDungeonTypeKind(CDungeon const*)
int CParty::getDungeonTypeKind(const CDungeon* dungeon) {
    if (!dungeon) {
        return 8;
    }
    if (dungeon->ancient_dungeon_) {
        return 1;
    }
    if (dungeon->get_dimension_possible() > 0) {
        return 2;
    }
    if (dungeon->getDungeonKind() == 1) {
        return 3;
    }
    if (dungeon->get_blood_dungeon_type() == 1) {
        return 4;
    }
    if (dungeon->get_blood_dungeon_type() == 2) {
        return 5;
    }
    if (dungeon->isTowerOfDespairDungeon()) {
        return 6;
    }
    if (dungeon->isTournamentDungeon()) {
        return 7;
    }
    return 0;
}

// IDA_EA: 0x85BA5D6
// IDA_NAME: _ZN6CParty20send_to_alive_memberER11PacketGuard
// DEMANGLED: CParty::send_to_alive_member(PacketGuard &)
bool CParty::send_to_alive_member(PacketGuard& packet) {
    for (int seat = 0; seat <= 3; ++seat) {
        if (_checkValidUser(seat) && members_[seat].user->get_state() == 5 &&
            battle_data_.member_live(seat)) {
            members_[seat].user->Send(packet);
        }
    }
    // The old i386 loop returns the terminating signed comparison (4 <= 3),
    // so the observable return value is false after broadcasting every seat.
    return false;
}

// IDA_EA: 0x85B3E12
// IDA_NAME: _ZN6CParty22OnTimerWalkOutLackUserEv
// DEMANGLED: CParty::OnTimerWalkOutLackUser(void)
int CParty::OnTimerWalkOutLackUser() {
    for (int seat = election_.GetElected(); seat >= 0;
         seat = election_.GetElected()) {
        election_.CancelCandidate(seat);
        election_.CancelVoter(seat);

        CUser* const victim = members_[seat].user;
        if (!victim) {
            LogManager::logFormat(
                1, "party.cpp", "bool CParty::OnTimerWalkOutLackUser()",
                0x35c6, "pVictim is null");
            return 0;
        }
        SendWalkout(seat, static_cast<ENUM_WALKOUT_TYPE>(1));
        walkout_member(manager_, seat);
        G_GameWorld()->out_from_dungeon(victim);
    }
    return 1;
}
