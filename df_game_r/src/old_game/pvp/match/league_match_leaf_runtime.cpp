#include "old_game/pvp/match/league_match_leaf_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "common/manager/school_server_proxy_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/pvp/match/league_match_static_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/os_api.h"

// IDA_EA: 0x85687D4
// IDA_NAME: _ZN10PlayerInfo5resetEv
// DEMANGLED: PlayerInfo::reset(void)
bool PlayerInfo::reset() {
    account_id = 0;
    rating = 0.0f;
    user = 0;
    active = 0;
    school_no = 0;
    for (int index = 0; index <= 2; ++index) {
        daily_badges[index] = 0;
    }
    return false;
}

// IDA_EA: 0x855C952
// IDA_NAME: _Z13CompareRatingRK10PlayerInfoS1_
// DEMANGLED: CompareRating(PlayerInfo const&,PlayerInfo const&)
bool CompareRating(const PlayerInfo& left, const PlayerInfo& right) {
    return left.active > right.active ||
           (left.active == right.active && left.rating > right.rating);
}

// IDA_EA: 0x855C9AE
// IDA_NAME: _ZN8TeamInfo7refreshEv
// DEMANGLED: TeamInfo::refresh(void)
bool TeamInfo::refresh() {
    for (int index = 0; index <= 3; ++index) {
        PlayerInfo& player = players[index];
        if (player.active) {
            player.account_id = static_cast<uint32_t>(player.user->get_acc_id());
            player.rating = player.user->GetRating();
            player.school_no =
                static_cast<uint32_t>(player.user->GetSchoolNo());
            player.user->GetDailyBadge(player.daily_badges);
        }
    }
    return false;
}

// IDA_EA: 0x855CB16
// IDA_NAME: _ZN8TeamInfo5resetEv
// DEMANGLED: TeamInfo::reset(void)
bool TeamInfo::reset() {
    for (int index = 0; index <= 3; ++index) {
        players[index].reset();
    }
    return false;
}

// IDA_EA: 0x855CB54
// IDA_NAME: _ZN8TeamInfo3addEP5CUser
// DEMANGLED: TeamInfo::add(CUser *)
int TeamInfo::add(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        PlayerInfo& player = players[index];
        if (player.active != 1u) {
            player.active = 1;
            player.user = user;
            player.account_id = static_cast<uint32_t>(user->get_acc_id());
            player.rating = user->GetRating();
            player.school_no = static_cast<uint32_t>(user->GetSchoolNo());
            user->GetDailyBadge(player.daily_badges);
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x855CC9C
// IDA_NAME: _ZN8TeamInfo3delEP5CUser
// DEMANGLED: TeamInfo::del(CUser *)
int TeamInfo::del(CUser* user) {
    for (int index = 0; index <= 3; ++index) {
        PlayerInfo& player = players[index];
        if (player.active && player.user == user) {
            player.active = 0;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x855CD26
// IDA_NAME: _ZNK8TeamInfo4sizeEv
// DEMANGLED: TeamInfo::size(void)const
int TeamInfo::size() const {
    int count = 0;
    for (int index = 0; index <= 3; ++index) {
        if (players[index].active) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x855E7A8
// IDA_NAME: _ZN12CLeagueMatch14_getSameSchoolEPjiij
// DEMANGLED: CLeagueMatch::_getSameSchool(unsigned int *,int,int,unsigned int)
int CLeagueMatch::_getSameSchool(unsigned int* school_numbers,
                                 int self_index,
                                 int player_count,
                                 unsigned int school_number) {
    for (int index = 0; index < player_count; ++index) {
        if (index != self_index && school_numbers[index] == school_number) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x855E7EE
// IDA_NAME: _ZN12CLeagueMatch8addBadgeEP10PlayerInfo15ENUM_BADGE_TYPEPhb
// DEMANGLED: CLeagueMatch::addBadge(PlayerInfo *,ENUM_BADGE_TYPE,unsigned char *,bool)
int CLeagueMatch::addBadge(PlayerInfo* player,
                           ENUM_BADGE_TYPE badge_type,
                           unsigned char* awarded,
                           bool same_school) {
    if (player->active != 1u) {
        return 0;
    }
    if (!player->user) {
        return 0;
    }
    if (!player->user->getCurCharacR()) {
        return 0;
    }

    const int badge_value = static_cast<int>(badge_type);
    if (badge_value != 0) {
        const int red_badges = static_cast<int16_t>(
            player->user->GetDailyBadge(
                static_cast<ENUM_BADGE_TYPE>(ENUM_BADGE_TYPE_RED)));
        const int blue_badges = static_cast<int16_t>(
            player->user->GetDailyBadge(
                static_cast<ENUM_BADGE_TYPE>(ENUM_BADGE_TYPE_BLUE)));
        if (red_badges + blue_badges > 9) {
            return 0;
        }
    } else if (static_cast<int16_t>(
                   player->user->GetDailyBadge(
                       static_cast<ENUM_BADGE_TYPE>(ENUM_BADGE_TYPE_GREEN))) >
               9) {
        return 0;
    }

    player->user->AddDailyBadge(badge_type);
    *awarded = 1;

    const int message_value = (same_school ? 1 : 0) +
                              static_cast<unsigned int>(*awarded);
    char message[256];
    std::memset(message, 0, sizeof(message));

    switch (badge_value) {
        case ENUM_BADGE_TYPE_RED:
            OS_API::snprintf(
                message,
                0xffu,
                g_scriptStringManager_.findString(4, "school_red_badge_msg", 0),
                message_value);
            break;
        case ENUM_BADGE_TYPE_BLUE:
            OS_API::snprintf(
                message,
                0xffu,
                g_scriptStringManager_.findString(4, "school_blue_badge_msg", 0),
                message_value);
            break;
        case ENUM_BADGE_TYPE_GREEN:
            OS_API::snprintf(
                message,
                0xffu,
                g_scriptStringManager_.findString(4, "school_green_badge_msg", 0),
                message_value);
            break;
        default:
            break;
    }

    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(static_cast<int>(std::strlen(message)));
    packet.put_str(message, static_cast<int>(std::strlen(message)));
    packet.finalize(true);
    player->user->Send(packet);
    return 1;
}

// IDA_EA: 0x855EB22
// IDA_NAME: _ZN12CLeagueMatch8OnRewardEP8PvP_Roomib
// DEMANGLED: CLeagueMatch::OnReward(PvP_Room *,int,bool)
void CLeagueMatch::OnReward(PvP_Room* room, int winning_team, bool draw) {
    Packet_PvPResult result_packet;
    const int player_count =
        static_cast<int>(nPlayerCountPerOneTeam_);

    // The packet constructor initializes only its ten-byte PacketHeader.
    // The old body initializes exactly the player_count entries below and
    // deliberately leaves the unused fixed-capacity entries untouched.
    for (int team = 0; team <= 1; ++team) {
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            for (int badge = 0; badge <= 2; ++badge) {
                result_packet.teams[team]
                    .daily_badges[player_index][badge] =
                    static_cast<uint8_t>(
                        teams_[team]
                            .players[player_index]
                            .daily_badges[badge]);
                result_packet.teams[team]
                    .awarded_badges[player_index][badge] = 0;
            }
        }
    }

    // Each arm of the complete draw/winner branch writes this before its
    // first use, matching the target's uninitialised stack local at entry.
    int rating_result;
    if (draw) {
        rating_result = 1;
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            result_packet.teams[0]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_RED] = 0;
            addBadge(
                &teams_[0].players[player_index],
                ENUM_BADGE_TYPE_BLUE,
                &result_packet.teams[0]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE],
                false);
            result_packet.teams[1]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_RED] = 0;
            addBadge(
                &teams_[1].players[player_index],
                ENUM_BADGE_TYPE_BLUE,
                &result_packet.teams[1]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE],
                false);
        }
    } else if (winning_team == 1) {
        rating_result = 2;
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            addBadge(
                &teams_[0].players[player_index],
                ENUM_BADGE_TYPE_RED,
                &result_packet.teams[0]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_RED],
                false);
            result_packet.teams[0]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE] = 0;
            result_packet.teams[1]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_RED] = 0;
            addBadge(
                &teams_[1].players[player_index],
                ENUM_BADGE_TYPE_BLUE,
                &result_packet.teams[1]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE],
                false);
        }
    } else {
        rating_result = 0;
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            result_packet.teams[0]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_RED] = 0;
            addBadge(
                &teams_[0].players[player_index],
                ENUM_BADGE_TYPE_BLUE,
                &result_packet.teams[0]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE],
                false);
            addBadge(
                &teams_[1].players[player_index],
                ENUM_BADGE_TYPE_RED,
                &result_packet.teams[1]
                     .awarded_badges[player_index][ENUM_BADGE_TYPE_RED],
                false);
            result_packet.teams[1]
                .awarded_badges[player_index][ENUM_BADGE_TYPE_BLUE] = 0;
        }
    }

    result_packet.school_match_elo_k2 =
        PvPGlobalEnvironmentVariable::school_match_elo_k2_;
    result_packet.rating_result =
        static_cast<uint8_t>(rating_result);
    result_packet.player_count_per_team =
        nPlayerCountPerOneTeam_;

    for (int team = 0; team <= 1; ++team) {
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            result_packet.teams[team].school_numbers[player_index] =
                teams_[team].players[player_index].school_no;
        }

        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            PlayerInfo& player = teams_[team].players[player_index];
            unsigned char* const awarded =
                result_packet.teams[team]
                    .awarded_badges[player_index];
            if (_getSameSchool(
                    result_packet.teams[team].school_numbers,
                    player_index,
                    player_count,
                    result_packet.teams[team]
                        .school_numbers[player_index])) {
                addBadge(
                    &player,
                    ENUM_BADGE_TYPE_GREEN,
                    &awarded[ENUM_BADGE_TYPE_GREEN],
                    awarded[ENUM_BADGE_TYPE_RED] ||
                        awarded[ENUM_BADGE_TYPE_BLUE]);
            } else {
                awarded[ENUM_BADGE_TYPE_GREEN] = 0;
                if ((awarded[ENUM_BADGE_TYPE_RED] ||
                     awarded[ENUM_BADGE_TYPE_BLUE]) &&
                    player.active == 1u &&
                    player.user &&
                    player.user->getCurCharacR()) {
                    char message[256];
                    std::memset(message, 0, sizeof(message));
                    OS_API::snprintf(
                        message,
                        0xffu,
                        g_scriptStringManager_.findString(
                            4, "school_green_badge_msg", 0),
                        1);

                    PacketGuard packet;
                    packet.put_header(0, 12);
                    packet.put_byte(0);
                    packet.put_short(0);
                    packet.put_byte(0);
                    packet.put_int(
                        static_cast<int>(std::strlen(message)));
                    packet.put_str(
                        message,
                        static_cast<int>(std::strlen(message)));
                    packet.finalize(true);
                    player.user->Send(packet);
                }
            }
        }

        // IDA 0x855F539 gates both team refreshes on the corresponding
        // team-1 slot, then reads the selected team's user.  This asymmetric
        // gate is an original behavior, not a reconstruction typo.
        for (int player_index = 0;
             player_index < player_count;
             ++player_index) {
            const PlayerInfo& gate = teams_[1].players[player_index];
            if (gate.active && gate.user) {
                CUser* const user =
                    teams_[team].players[player_index].user;
                result_packet.teams[team]
                    .daily_badges[player_index][ENUM_BADGE_TYPE_GREEN] =
                    static_cast<uint8_t>(
                        user->GetDailyBadge(ENUM_BADGE_TYPE_GREEN));
                result_packet.teams[team]
                    .daily_badges[player_index][ENUM_BADGE_TYPE_RED] =
                    static_cast<uint8_t>(
                        user->GetDailyBadge(ENUM_BADGE_TYPE_RED));
                result_packet.teams[team]
                    .daily_badges[player_index][ENUM_BADGE_TYPE_BLUE] =
                    static_cast<uint8_t>(
                        user->GetDailyBadge(ENUM_BADGE_TYPE_BLUE));
            }
        }
    }

    float ratings[2] = {0.0f, 0.0f};
    for (unsigned int player_index = 0;
         player_index <
             static_cast<unsigned int>(nPlayerCountPerOneTeam_);
         ++player_index) {
        for (int team = 0; team <= 1; ++team) {
            ratings[team] = teams_[team].players[player_index].rating;
        }
        CEloRating::calc(
            ratings[0],
            ratings[1],
            static_cast<ENUM_ELO_RATING_SYSTEM>(rating_result),
            PvPGlobalEnvironmentVariable::school_match_elo_k_);
        for (int team = 0; team <= 1; ++team) {
            PlayerInfo& player =
                teams_[team].players[player_index];
            if (player.active) {
                player.user->UpdateRating(ratings[team]);
            }
            result_packet.teams[team].account_ids[player_index] =
                player.account_id;
        }
    }

    if (nPlayerCountPerOneTeam_ == 1u) {
        Stream* const stream = GlobalData::s_stream_pool->Acquire(
            "MatchingSystem.cpp", 0x4a8);
        CStreamGuard guard(stream, true);
        (*guard) << 0x136;
        (*guard) << -1;
        guard->put_binary(
            &result_packet,
            static_cast<int>(sizeof(result_packet)));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    } else {
        GlobalData::s_school_proxy->SendPacket(
            reinterpret_cast<char*>(&result_packet),
            static_cast<int>(sizeof(result_packet)));
    }

    (void)room;
}
