#include "old_game/player/core/cuser.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/progression/quest/character_mission_list.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <ctime>
#include <stdint.h>


// IDA unit: 99_misc_global/Packet_Notice_Guild_War_Point_Change.
// CUser::add_guild_pvp_result is the first restored C-domain consumer, so the
// concrete 15-byte server packet accompanies the PVP write chain rather than
// passing a raw local buffer through the guild proxy.
#pragma pack(push, 1)
struct Packet_Notice_Guild_War_Point_Change : PacketHeader {
    Packet_Notice_Guild_War_Point_Change();

    uint32_t charac_guildkey;
    int8_t point_change;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Notice_Guild_War_Point_Change) == 15u,
              "guild PVP point-change packet must stay 15 bytes");

void TracePvpPointError(CUser* user,
                        int line,
                        int error_index,
                        int point_change) {
    cMyTrace trace("bool CUser::update_pvp_point(int)", line, 5);
    trace("pvp_exp_error%d %d, charac_no(%d), grade(%d), exp(%d)",
          error_index,
          point_change,
          user->get_charac_no(-1),
          user->get_pvp_grade(),
          user->getPVPResultRefR()->values[2]);
}

// IDA_EA: 0x86944E0
// IDA_NAME: _ZN36Packet_Notice_Guild_War_Point_ChangeC2Ev
// DEMANGLED: Packet_Notice_Guild_War_Point_Change::Packet_Notice_Guild_War_Point_Change(void)
Packet_Notice_Guild_War_Point_Change::Packet_Notice_Guild_War_Point_Change()
    : PacketHeader(0x417u, 0x0fu) {}

// IDA_EA: 0x865D986
// IDA_NAME: _ZN5CUser17add_pvp_play_infoEjj
// DEMANGLED: CUser::add_pvp_play_info(unsigned int,unsigned int)
PvpResultType* CUser::add_pvp_play_info(unsigned int play_seconds,
                                        unsigned int play_count) {
    time_t current_time =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    if (getPVPResultRefR()->values[26] + 86400 <
        static_cast<int>(current_time)) {
        getPVPResultRefW()->values[24] = 0;

        struct tm local_time;
        struct tm* const midnight = localtime_r(&current_time, &local_time);
        midnight->tm_hour = 0;
        midnight->tm_min = 0;
        midnight->tm_sec = 0;
        getPVPResultRefW()->values[26] =
            static_cast<int>(mktime(midnight)) + 86400;
    }

    getPVPResultRefW()->values[24] +=
        static_cast<int>(play_seconds);
    PvpResultType* const result = getPVPResultRefW();
    result->values[25] += static_cast<int>(play_count);
    return result;
}

// IDA_EA: 0x864FC34
// IDA_NAME: _ZN5CUser21gainPowerWarRewardExpEi
// DEMANGLED: CUser::gainPowerWarRewardExp(int)
int CUser::gainPowerWarRewardExp(int) {
    return 0;
}

// IDA_EA: 0x865C678
// IDA_NAME: _ZN5CUser14add_pvp_resultEbPj
// DEMANGLED: CUser::add_pvp_result(bool,unsigned int *)
CUserCharacInfo* CUser::add_pvp_result(bool won, uint32_t* reward_exp) {
    PvP_Room* const room = GetPVPRoom();
    CUserCharacInfo* result = reinterpret_cast<CUserCharacInfo*>(room);
    if (!room) {
        return result;
    }

    const int seat = room->get_user_seat(this);
    if (room->IsPvpObserver(seat)) {
        return result;
    }

    lock();
    const int old_win_point = getPVPResultR()->values[22];
    int gained_win_point = static_cast<int>(pending_pvp_win_point_);
    pending_pvp_win_point_ = 0;

    if (won) {
        ++gained_win_point;
        if (!G_GameWorld()->IsIntegratedPvPBaseChannel() ||
            G_CDataManager()->GetPvPChannelType()) {
            ++getPVPResultW()->values[0];
        }
    } else if (!G_GameWorld()->IsIntegratedPvPBaseChannel() ||
               G_CDataManager()->GetPvPChannelType()) {
        ++getPVPResultW()->values[1];
    }

    getPVPResultW()->values[22] += gained_win_point;
    history_log().WPAdd(getPVPResultR()->values[22],
                        gained_win_point,
                        static_cast<eWPAddReason>(0));

    const int extra_win_point =
        room->get_extra_win_point(getPVPResultW()->values[22] - old_win_point);
    if (extra_win_point) {
        getPVPResultW()->values[22] += extra_win_point;
        history_log().WPAdd(getPVPResultR()->values[22],
                            extra_win_point,
                            static_cast<eWPAddReason>(0));
    }
    unlock();

    const int battle_mode = room->get_pvp_battle_mode();
    if ((battle_mode == 4 || battle_mode == 5) && reward_exp) {
        unsigned int reward_per_point = 0;
        G_CDataManager()->get_mob_reward(get_charac_level(), reward_per_point);
        *reward_exp = reward_per_point *
                      static_cast<unsigned int>(
                          getPVPResultR()->values[22] - old_win_point);
        *reward_exp += static_cast<unsigned int>(gainPowerWarRewardExp(0));
    }

    send_pvp_record();
    SetReliablePerson(false);
    return SetOverTradeGoldCount(0);
}

// IDA_EA: 0x865C936
// IDA_NAME: _ZN5CUser20add_guild_pvp_resultEi
// DEMANGLED: CUser::add_guild_pvp_result(int)
void CUser::add_guild_pvp_result(int point_change) {
    PvP_Room* const room = GetPVPRoom();
    if (!room) {
        return;
    }

    const int seat = room->get_user_seat(this);
    if (room->IsPvpObserver(seat)) {
        return;
    }

    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(33);
    if (event->IsEventing(0) != 1 || !get_charac_guildkey()) {
        return;
    }

    Packet_Notice_Guild_War_Point_Change notice;
    notice.charac_guildkey = static_cast<uint32_t>(get_charac_guildkey());
    notice.point_change = static_cast<int8_t>(point_change);

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&notice), sizeof(notice));

    send_pvp_record();
    PacketGuard packet;
    packet.put_header(0, 110);
    packet.put_byte(static_cast<unsigned char>(point_change));
    packet.finalize(true);
    Send(packet);
    guild_pvp_point_ += point_change;
}

// IDA_EA: 0x865CD52
// IDA_NAME: _ZN5CUser15send_pvp_recordEv
// DEMANGLED: CUser::send_pvp_record(void)
void CUser::send_pvp_record() {
    const PvpResultType* const result = getPVPResultRefR();

    PacketGuard packet;
    packet.put_header(0, 48);
    packet.put_int(result->values[0]);
    packet.put_int(result->values[1]);
    packet.put_int(result->values[2]);
    packet.put_int(GlobalData::s_ref_pvp_grade->GetPvpCurrRankPoint(
        result->values[5]));
    packet.put_int(GlobalData::s_ref_pvp_grade->GetPvpNextRankPoint(
        result->values[5]));
    packet.put_byte(static_cast<unsigned char>(result->values[5]));
    packet.put_byte(static_cast<unsigned char>(result->values[23]));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8662774
// IDA_NAME: _ZNK5CUser25acceptable_within_missionEv
// DEMANGLED: CUser::acceptable_within_mission(void)const
int CUser::acceptable_within_mission() const {
    CMissionList_Charac* const missions =
        static_cast<CMissionList_Charac*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
    if (G_CDataManager()->get_WithinMissionIndex(get_pvp_grade())) {
        return missions->MakeMissionList_JustKind(*this, 27);
    }
    return 0;
}

// IDA_EA: 0x865CFD8
// IDA_NAME: _ZN5CUser16update_pvp_pointEi
// DEMANGLED: CUser::update_pvp_point(int)
int CUser::update_pvp_point(int point_change) {
    if (!point_change) {
        return 1;
    }

    if (point_change < 0) {
        if (point_change < -100000) {
            TracePvpPointError(this, 13782, 5, point_change);
        }
        if (getPVPResultRefR()->values[5] <= 9) {
            return 1;
        }

        CEventBase* const revision_event =
            GlobalData::s_event_manager->GetRepeatEvent(54);
        if (revision_event->IsEventing(0) &&
            getPVPResultRefR()->values[5] > 9 &&
            getPVPResultRefR()->values[5] <= 19) {
            point_change = static_cast<int>(
                static_cast<long double>(point_change) * 0.5L);
        }

        const long double weighted_change = static_cast<long double>(
            static_cast<int>(static_cast<long double>(point_change) *
                             0.89999998L));
        const long double grade_revision =
            G_CDataManager()->get_server_parameter_script()
                ->GetPvPPenaltyRevision(getPVPResultRefR()->values[5]);
        point_change = static_cast<int>(weighted_change * grade_revision);
        if (point_change < -100000 || point_change > 100000) {
            TracePvpPointError(this, 13805, 6, point_change);
        }
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        !G_CDataManager()->GetPvPChannelType()) {
        point_change = 70 * point_change / 100;
    }

    if (G_GameWorld()->GetChannelType() == 15 && point_change > 0) {
        const int grade_gap = getPVPResultRefR()->values[5] -
                              G_CDataManager()->GetMaxGradePvPChannel();
        if (grade_gap > 0) {
            point_change = G_CDataManager()->get_server_parameter_script()
                               ->GetPvPChannelGradePenalty(grade_gap) *
                           point_change / 100;
        }
    }

    if (point_change < -100000 || point_change > 100000) {
        TracePvpPointError(this, 13831, 7, point_change);
    }

    PvP_Room* const room = GetPVPRoom();
    if (!room || room->IsPvpObserver(room->get_user_seat(this))) {
        return 1;
    }

    if (has_within_Mission()) {
        if (point_change <= 0) {
            if (point_change < 0) {
                Remove_Mission(27);
                Remove_Mission(7);
                PacketGuard packet;
                packet.put_header(0, 344);
                packet.put_byte(1);
                packet.finalize(true);
                Send(packet);
            }
        } else if (G_GameWorld()->GetChannelType() == 15) {
            point_change = 0;
        } else {
            Remove_Mission(27);
            Remove_Mission(7);
            PacketGuard packet;
            packet.put_header(0, 344);
            packet.put_byte(0);
            packet.finalize(true);
            Send(packet);
        }
    } else if (G_GameWorld()->GetChannelType() == 15 &&
               acceptable_within_mission() && point_change > 0) {
        point_change = 0;
    }

    if (point_change < -100000 || point_change > 100000) {
        TracePvpPointError(this, 13907, 8, point_change);
    }

    PvpResultType* const writable = getPVPResultRefW();
    writable->values[2] += point_change;
    if (getPVPResultRefR()->values[2] < 0) {
        LogManager::logFormat(1,
                              "user.cpp",
                              "bool CUser::update_pvp_point(int)",
                              13916,
                              "PVP correct now<%d> point<%d>",
                              getPVPResultRefR()->values[2],
                              point_change);
        getPVPResultRefW()->values[2] = 0;
    }

    if (getPVPResultRefR()->values[5] <= 29) {
        if (!has_within_Mission() && G_GameWorld()->GetChannelType() == 15) {
            acceptable_within_mission();
        }

        int new_grade =
            GlobalData::s_ref_pvp_grade->GetPvpGrade(
                getPVPResultRefR()->values[2]);
        if (new_grade <= 0) {
            LogManager::logFormat(1,
                                  "user.cpp",
                                  "bool CUser::update_pvp_point(int)",
                                  14012,
                                  "PVP Grade Incorrect %d",
                                  getPVPResultRefR()->values[2]);
        } else {
            const int old_grade = getPVPResultRefR()->values[5];
            if (old_grade > new_grade && old_grade > 9 && old_grade <= 15) {
                getPVPResultRefW()->values[2] =
                    GlobalData::s_ref_pvp_grade->GetPvpPoint(old_grade) + 1;
                new_grade = old_grade;
            }

            getPVPResultRefW()->values[5] = new_grade;
            if (has_within_Mission() &&
                getPVPResultRefR()->values[5] > old_grade) {
                getPVPResultRefW()->values[5] = old_grade;
            }

            if (getPVPResultRefR()->values[5] != old_grade) {
                PacketGuard packet;
                getCurCharacQuestR()->get_quest_info(
                    reinterpret_cast<char*>(&packet));
                if (G_GameWorld()->GetChannelType() != 15) {
                    Remove_Mission(30);
                    CMissionList_Charac* const missions =
                        static_cast<CMissionList_Charac*>(GetCharacExpandData(
                            static_cast<ENUM_CHARAC_EXPAND_TYPE>(8)));
                    missions->MakeMissionList_JustKind(*this, 30);
                }
                Send(packet);
            }
        }
    }
    return 1;
}

// IDA_EA: 0x8676876
// IDA_NAME: _ZN5CUser13update_recordER14GameResultType
// DEMANGLED: CUser::update_record(GameResultType &)
int CUser::update_record(GameResultType& record) {
    const int unique_key = record.GetUniqueKey();
    std::map<int, GameResultType>* const records = getBattleRecordRefW();
    const std::map<int, GameResultType>::iterator found =
        records->find(unique_key);

    // Keep the second writable accessor call from the original: it marks the
    // battle-record persistence flag before the end comparison as well as
    // before insertion.
    if (found != getBattleRecordRefW()->end()) {
        return found->second.UpdateHighScore(record);
    }

    const int insertion_key = record.GetUniqueKey();
    getBattleRecordRefW()->insert(
        std::make_pair(insertion_key, record));
    return 15;
}
