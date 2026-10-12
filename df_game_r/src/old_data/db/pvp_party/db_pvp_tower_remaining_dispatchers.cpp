#include "old_data/db/pvp_party/db_pvp_tower_remaining_dispatchers.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_tools.h"
#include "old_game/pvp/season/pvp_rating_abi.h"
#include "common/manager/statistics_shusia_service_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/pvp/battlefield/statistics_pvp_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vector>

namespace {

const unsigned int kDeathTowerUpdateBytes = 108;
const unsigned int kDeathTowerTopRankerBytes = 220008;
const unsigned int kDeathTowerMaxMembers = 4;

// DB_UpdatePvPResult::dispatch @0x843097E constructs two of these 36-byte
// locals before each school-versus-school result.  `rating_before` is a copy
// of the SQL rating retained solely to derive the integer member contribution
// after CEloRating::calc mutates `rating`.
struct SchoolInfo {
    // IDA_EA: 0x843091E
    // IDA_NAME: _ZZN18DB_UpdatePvPResult8dispatchEiiP6StreamEN11school_infoC2Ev
    // DEMANGLED: DB_UpdatePvPResult::dispatch(int,int,Stream *)::school_info::school_info(void)
    SchoolInfo()
        : school_id(0),
          fight_count(0),
          last_update_time(0),
          contribution(0),
          school_point(0),
          school_point_prev(0),
          fight_count_prev(0),
          rating(0.0f),
          rating_before(0.0f) {}

    int school_id;
    int fight_count;
    int last_update_time;
    int contribution;
    int school_point;
    int school_point_prev;
    int fight_count_prev;
    float rating;
    float rating_before;
};

static_assert(sizeof(SchoolInfo) == 0x24u,
              "DB_UpdatePvPResult school_info ABI must stay 36 bytes");

uint16_t ReadU16(const unsigned char* bytes, unsigned int offset) {
    uint16_t value = 0;
    memcpy(&value, bytes + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const unsigned char* bytes, unsigned int offset) {
    uint32_t value = 0;
    memcpy(&value, bytes + offset, sizeof(value));
    return value;
}

void WriteU16(unsigned char* bytes, unsigned int offset, uint16_t value) {
    memcpy(bytes + offset, &value, sizeof(value));
}

void WriteU32(unsigned char* bytes, unsigned int offset, uint32_t value) {
    memcpy(bytes + offset, &value, sizeof(value));
}

}  // namespace

// IDA_EA: 0x844E16E
// IDA_NAME: _ZN18DB_StatDungeonFailC2Ev
// DEMANGLED: DB_StatDungeonFail::DB_StatDungeonFail(void)
DB_StatDungeonFail::DB_StatDungeonFail() {}

// IDA_EA: 0x8450070
// IDA_NAME: _ZN18DB_StatDungeonFailD2Ev
// DEMANGLED: DB_StatDungeonFail::~DB_StatDungeonFail()
// IDA_EA: 0x84500A0
// IDA_NAME: _ZN18DB_StatDungeonFailD0Ev
// DEMANGLED: DB_StatDungeonFail::~DB_StatDungeonFail()
DB_StatDungeonFail::~DB_StatDungeonFail() {}

// IDA_EA: 0x841B6E0
// IDA_NAME: _ZN18DB_StatDungeonFail8dispatchEiiP6Stream
// DEMANGLED: DB_StatDungeonFail::dispatch(int,int,Stream *)
bool DB_StatDungeonFail::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    StatisticsShusiaService* statistics =
        G_GameWorld()->getStatisticsShusiaService();
    statistics->lock();
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));

    StatisticsShusiaService* dungeon_statistics =
        G_GameWorld()->getStatisticsShusiaService();
    for (std::map<int, int>::iterator it =
             dungeon_statistics->dungeon_failures_.begin();
         it != dungeon_statistics->dungeon_failures_.end(); ++it) {
        db_handle->set_query(
            "inSert into dungeon_fail(occ_date,lev,occ_count) "
            "values(cast(now() as date),%d,%d)", it->first, it->second);
        if (db_handle->exec(true) != true) {
            db_handle->set_query(
                "upDate dungeon_fail set occ_count = occ_count + %d "
                "where occ_date = now() and lev = %d", it->second, it->first);
            if (db_handle->exec(true) != true) {
                cMyTrace trace(
                    "virtual bool DB_StatDungeonFail::dispatch(int, int, Stream*)",
                    20111,
                    5);
                trace("upDate dungeon_fail failed at %d", it->first);
            }
        }
    }

    MySQL* service_db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    StatisticsShusiaService* service_statistics =
        G_GameWorld()->getStatisticsShusiaService();
    for (std::map<int, int>::iterator it =
             service_statistics->service_usages_.begin();
         it != service_statistics->service_usages_.end(); ++it) {
        service_db_handle->set_query(
            "inSert into re_stamina_shusia(occ_date,lev,occ_count) "
            "values(cast(now() as date),%d,%d)", it->first, it->second);
        if (service_db_handle->exec(true) != true) {
            cMyTrace trace_insert(
                "virtual bool DB_StatDungeonFail::dispatch(int, int, Stream*)",
                20137,
                5);
            trace_insert("inSert into re_stamina_shusia failed at %d", it->first);
            service_db_handle->set_query(
                "upDate re_stamina_shusia set occ_count = occ_count + %d "
                "where occ_date = now() and lev = %d", it->second, it->first);
            if (service_db_handle->exec(true) != true) {
                cMyTrace trace_update(
                    "virtual bool DB_StatDungeonFail::dispatch(int, int, Stream*)",
                    20143,
                    5);
                trace_update("upDate re_stamina_shusia failed at %d", it->first);
            } else {
                cMyTrace trace_success(
                    "virtual bool DB_StatDungeonFail::dispatch(int, int, Stream*)",
                    20147,
                    5);
                trace_success("re_stamina_shusia : %d SHUSIA USAGE %d",
                              it->first,
                              it->second);
            }
        } else {
            cMyTrace trace_success(
                "virtual bool DB_StatDungeonFail::dispatch(int, int, Stream*)",
                20147,
                5);
            trace_success("re_stamina_shusia : %d SHUSIA USAGE %d",
                          it->first,
                          it->second);
        }
    }

    G_GameWorld()->getStatisticsShusiaService()->unlock();
    G_GameWorld()->getStatisticsShusiaService()->reset();
    return true;
}

// IDA_EA: 0x844E18A
// IDA_NAME: _ZN15DB_StatPVPPlay2C2Ev
// DEMANGLED: DB_StatPVPPlay2::DB_StatPVPPlay2(void)
DB_StatPVPPlay2::DB_StatPVPPlay2() {}

// IDA_EA: 0x84500BE
// IDA_NAME: _ZN15DB_StatPVPPlay2D2Ev
// DEMANGLED: DB_StatPVPPlay2::~DB_StatPVPPlay2()
// IDA_EA: 0x84500EE
// IDA_NAME: _ZN15DB_StatPVPPlay2D0Ev
// DEMANGLED: DB_StatPVPPlay2::~DB_StatPVPPlay2()
DB_StatPVPPlay2::~DB_StatPVPPlay2() {}

// IDA_EA: 0x841BA96
// IDA_NAME: _ZN15DB_StatPVPPlay28dispatchEiiP6Stream
// DEMANGLED: DB_StatPVPPlay2::dispatch(int,int,Stream *)
bool DB_StatPVPPlay2::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;

    StatisticsPvP* statistics = G_GameWorld()->getStatisticsPvP();
    statistics->lock();
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));

    StatisticsPvP* result_statistics = G_GameWorld()->getStatisticsPvP();
    for (std::map<StatisticsPvP::PvP_Key, StatisticsPvP::PvP_Room>::iterator
             it = result_statistics->room_results_.begin();
         it != result_statistics->room_results_.end(); ++it) {
        const StatisticsPvP::PvP_Key& key = it->first;
        const StatisticsPvP::PvP_Room& room = it->second;
        db_handle->set_query(
            "inSert into pvp_status(occ_date,room_number,play_type,"
            "no_participants,occ_count) values(cast(now() as date),%d,%d,%d,%d)",
            key.room_number, key.play_type, room.participant_count,
            room.occurrence_count);
        if (db_handle->exec(true) != true) {
            db_handle->set_query(
                "upDate pvp_status set occ_count = occ_count + %d , "
                "no_participants = no_participants + %d where occ_date = "
                "cast(now() as date) and room_number = %d and play_type = %d",
                room.occurrence_count, room.participant_count, key.room_number,
                key.play_type);
            if (db_handle->exec(true) != true) {
                cMyTrace trace(
                    "virtual bool DB_StatPVPPlay2::dispatch(int, int, Stream*)",
                    20184,
                    5);
                trace("upDate pvp_status failed at %d", key.room_number);
            } else {
                cMyTrace trace_success(
                    "virtual bool DB_StatPVPPlay2::dispatch(int, int, Stream*)",
                    20188,
                    5);
                trace_success("pvp_status : %d ROOM %d TYPE %d Players %d Count ",
                              key.room_number,
                              key.play_type,
                              room.participant_count,
                              room.occurrence_count);
            }
        } else {
            cMyTrace trace_success(
                "virtual bool DB_StatPVPPlay2::dispatch(int, int, Stream*)",
                20188,
                5);
            trace_success("pvp_status : %d ROOM %d TYPE %d Players %d Count ",
                          key.room_number,
                          key.play_type,
                          room.participant_count,
                          room.occurrence_count);
        }
    }
    G_GameWorld()->getStatisticsPvP()->unlock();
    return true;
}

// IDA_EA: 0x8420E1A
// IDA_NAME: _ZN22DB_GatheringParyStatus27save_dnf_pvp_result_historyEP16SIG_PARTY_STATUSi
// DEMANGLED: DB_GatheringParyStatus::save_dnf_pvp_result_history(SIG_PARTY_STATUS *,int)
bool DB_GatheringParyStatus::save_dnf_pvp_result_history(
    SIG_PARTY_STATUS* status,
    int index) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    const SIG_PARTY_STATUS::PartyResult& result = status->results[index];
    char peer_ip[16];
    sprintf(peer_ip, "%d.%d.%d.%d", result.peer_ip[0], result.peer_ip[1],
            result.peer_ip[2], result.peer_ip[3]);

    CEnvironment* environment = G_CEnvironment();
    db_handle->set_query(
        "inSert into dnf_party_result_history_pvp (str_ip,nat_type,m_id,party_result,occ_time,map_id,dungeon_id,peer_nat_type"
        ",peer_ip, established_time, IsConnectedToRelaySrv, ConnectedTimeToRelaySrv, relay_ip, relay_udp_port, relay_tcp_port"
        " ) values('%s',%d,%s,%d,now(),%d,%d,%d,'%s',%d, %d, %d, '%s', %d, %d)",
        status->source_ip,
        static_cast<int>(status->nat_type),
        NumberToString(status->member_id, 0),
        static_cast<int>(result.party_result),
        static_cast<int>(status->map_id),
        static_cast<int>(status->dungeon_id),
        static_cast<int>(result.peer_nat_type),
        peer_ip,
        static_cast<int>(result.established_time),
        static_cast<int>(status->is_connected_to_relay),
        static_cast<int>(status->connected_time_to_relay),
        environment->server_envir_.relay_ip,
        environment->server_envir_.relay_udp_port,
        environment->server_envir_.relay_tcp_port);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_GatheringParyStatus::save_dnf_pvp_result_history(SIG_PARTY_STATUS*, int)",
            22628,
            5);
        trace("DB_GatheringParyStatus failed");
    }
    return true;
}

// IDA_EA: 0x8421066
// IDA_NAME: _ZN22DB_GatheringParyStatus25save_dnf_pvp_ping_historyEP16SIG_PARTY_STATUS
// DEMANGLED: DB_GatheringParyStatus::save_dnf_pvp_ping_history(SIG_PARTY_STATUS *)
bool DB_GatheringParyStatus::save_dnf_pvp_ping_history(SIG_PARTY_STATUS* status) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    char peer_ips[8][16];
    for (int index = 0; index <= 7; ++index) {
        const SIG_PARTY_STATUS::PartyPing& ping = status->pings[index];
        sprintf(peer_ips[index], "%d.%d.%d.%d", ping.ip[0], ping.ip[1],
                ping.ip[2], ping.ip[3]);
    }

    const SIG_PARTY_STATUS::PartyPing* pings = status->pings;
    db_handle->set_query(
        "inSert into dnf_party_ping_history_pvp(str_ip,m_id,nat_type_1,ping1,mtu_1,nat_type_2,ping2,mtu_2,nat_type_3,ping3,mt"
        "u_3,nat_type_4,ping4,mtu_4,nat_type_5,ping5,mtu_5,nat_type_6,ping6,mtu_6,nat_type_7,ping7,mtu_7,nat_type_8,ping8,mtu"
        "_8,occ_time,map_id,dungeon_id,ip_1,ip_2,ip_3,ip_4,ip_5,ip_6,ip_7,ip_8, end_reason) values('%s',%s,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,now(),%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s',%d)",
        status->source_ip,
        NumberToString(status->member_id, 0),
        static_cast<int>(pings[0].nat_type), static_cast<int>(pings[0].ping),
        static_cast<int>(pings[0].mtu), static_cast<int>(pings[1].nat_type),
        static_cast<int>(pings[1].ping), static_cast<int>(pings[1].mtu),
        static_cast<int>(pings[2].nat_type), static_cast<int>(pings[2].ping),
        static_cast<int>(pings[2].mtu), static_cast<int>(pings[3].nat_type),
        static_cast<int>(pings[3].ping), static_cast<int>(pings[3].mtu),
        static_cast<int>(pings[4].nat_type), static_cast<int>(pings[4].ping),
        static_cast<int>(pings[4].mtu), static_cast<int>(pings[5].nat_type),
        static_cast<int>(pings[5].ping), static_cast<int>(pings[5].mtu),
        static_cast<int>(pings[6].nat_type), static_cast<int>(pings[6].ping),
        static_cast<int>(pings[6].mtu), static_cast<int>(pings[7].nat_type),
        static_cast<int>(pings[7].ping), static_cast<int>(pings[7].mtu),
        static_cast<int>(status->map_id), static_cast<int>(status->dungeon_id),
        peer_ips[0], peer_ips[1], peer_ips[2], peer_ips[3], peer_ips[4],
        peer_ips[5], peer_ips[6], peer_ips[7],
        static_cast<int>(status->end_reason));
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_GatheringParyStatus::save_dnf_pvp_ping_history(SIG_PARTY_STATUS*)",
            22747,
            5);
        trace("DB_GatheringParyStatus failed");
    }
    return true;
}

// IDA_EA: 0x842156A
// IDA_NAME: _ZN22DB_GatheringParyStatus29save_dnf_party_result_historyEP16SIG_PARTY_STATUSi
// DEMANGLED: DB_GatheringParyStatus::save_dnf_party_result_history(SIG_PARTY_STATUS *,int)
bool DB_GatheringParyStatus::save_dnf_party_result_history(
    SIG_PARTY_STATUS* status,
    int index) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    const SIG_PARTY_STATUS::PartyResult& result = status->results[index];
    char peer_ip[16];
    sprintf(peer_ip, "%d.%d.%d.%d", result.peer_ip[0], result.peer_ip[1],
            result.peer_ip[2], result.peer_ip[3]);

    CEnvironment* environment = G_CEnvironment();
    db_handle->set_query(
        "inSert into dnf_party_result_history (str_ip,nat_type,m_id,party_result,occ_time,map_id,dungeon_id,peer_nat_type,pee"
        "r_ip,established_time, IsConnectedToRelaySrv, ConnectedTimeToRelaySrv, relay_ip, relay_udp_port, relay_tcp_port) val"
        "ues('%s',%d,%s,%d,now(),%d,%d,%d,'%s',%d, %d, %d, '%s', %d, %d)",
        status->source_ip,
        static_cast<int>(status->nat_type),
        NumberToString(status->member_id, 0),
        static_cast<int>(result.party_result),
        static_cast<int>(status->map_id),
        static_cast<int>(status->dungeon_id),
        static_cast<int>(result.peer_nat_type),
        peer_ip,
        static_cast<int>(result.established_time),
        static_cast<int>(status->is_connected_to_relay),
        static_cast<int>(status->connected_time_to_relay),
        environment->server_envir_.relay_ip,
        environment->server_envir_.relay_udp_port,
        environment->server_envir_.relay_tcp_port);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_GatheringParyStatus::save_dnf_party_result_history(SIG_PARTY_STATUS*, int)",
            22799,
            5);
        trace("DB_GatheringParyStatus failed");
    }
    return true;
}

// IDA_EA: 0x84217B6
// IDA_NAME: _ZN22DB_GatheringParyStatus27save_dnf_party_ping_historyEP16SIG_PARTY_STATUS
// DEMANGLED: DB_GatheringParyStatus::save_dnf_party_ping_history(SIG_PARTY_STATUS *)
bool DB_GatheringParyStatus::save_dnf_party_ping_history(SIG_PARTY_STATUS* status) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
    char peer_ips[4][16];
    for (int index = 0; index <= 3; ++index) {
        const SIG_PARTY_STATUS::PartyPing& ping = status->pings[index];
        sprintf(peer_ips[index], "%d.%d.%d.%d", ping.ip[0], ping.ip[1],
                ping.ip[2], ping.ip[3]);
    }

    const SIG_PARTY_STATUS::PartyPing* pings = status->pings;
    db_handle->set_query(
        "inSert into dnf_party_ping_history(str_ip,m_id,nat_type_1,ping1,mtu_1,nat_type_2,ping2,mtu_2,nat_type_3,ping3,mtu_3,"
        "nat_type_4,ping4,mtu_4,occ_time,map_id,dungeon_id,ip_1,ip_2,ip_3,ip_4, end_reason) values('%s',%s,%d,%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,%d,now(),%d,%d,'%s','%s','%s','%s',%d)",
        status->source_ip,
        NumberToString(status->member_id, 0),
        static_cast<int>(pings[0].nat_type), static_cast<int>(pings[0].ping),
        static_cast<int>(pings[0].mtu), static_cast<int>(pings[1].nat_type),
        static_cast<int>(pings[1].ping), static_cast<int>(pings[1].mtu),
        static_cast<int>(pings[2].nat_type), static_cast<int>(pings[2].ping),
        static_cast<int>(pings[2].mtu), static_cast<int>(pings[3].nat_type),
        static_cast<int>(pings[3].ping), static_cast<int>(pings[3].mtu),
        static_cast<int>(status->map_id), static_cast<int>(status->dungeon_id),
        peer_ips[0], peer_ips[1], peer_ips[2], peer_ips[3],
        static_cast<int>(status->end_reason));
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "bool DB_GatheringParyStatus::save_dnf_party_ping_history(SIG_PARTY_STATUS*)",
            22884,
            5);
        trace("DB_GatheringParyStatus failed");
    }
    return true;
}

// IDA_EA: 0x8421AEA
// IDA_NAME: _ZN22DB_GatheringParyStatus8dispatchEiiP6Stream
// DEMANGLED: DB_GatheringParyStatus::dispatch(int,int,Stream *)
bool DB_GatheringParyStatus::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    (void)stream;
    return true;
}

// IDA_EA: 0x844E3BA
// IDA_NAME: _ZN22DB_GatheringParyStatusC2Ev
// DEMANGLED: DB_GatheringParyStatus::DB_GatheringParyStatus(void)
DB_GatheringParyStatus::DB_GatheringParyStatus() {}

// IDA_EA: 0x845F2FE
// IDA_NAME: _ZN22DB_GatheringParyStatusD2Ev
// DEMANGLED: DB_GatheringParyStatus::~DB_GatheringParyStatus()
// IDA_EA: 0x845F32E
// IDA_NAME: _ZN22DB_GatheringParyStatusD0Ev
// DEMANGLED: DB_GatheringParyStatus::~DB_GatheringParyStatus()
DB_GatheringParyStatus::~DB_GatheringParyStatus() {}

// IDA_EA: 0x844E836 / 0x845E5E4
DB_UpdateDeathTowerBestRecord::DB_UpdateDeathTowerBestRecord() {}
DB_UpdateDeathTowerBestRecord::~DB_UpdateDeathTowerBestRecord() {}

// IDA_EA: 0x842A11E
bool DB_UpdateDeathTowerBestRecord::dispatch(int signal,
                                             int user_idx,
                                             Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned char request[kDeathTowerUpdateBytes];
    if (!stream || stream->get_binary(request, sizeof(request)) != 1) {
        return false;
    }
    const unsigned int charac_no = ReadU32(request, 0);
    const unsigned int tower_index = ReadU16(request, 4);
    const unsigned int member_count = ReadU16(request, 6);
    const unsigned int stage = ReadU32(request, 100);
    const unsigned int play_time = ReadU32(request, 104);
    if (!charac_no || !member_count || member_count > kDeathTowerMaxMembers) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    char* members = db_handle->blob_to_str(0, request + 8,
                                           23 * static_cast<int>(member_count));
    db_handle->set_query(
        "upDate charac_tower_record set member_info_%u='%s',stage_%u=%u,"
        "play_time_%u=%u,occ_time_%u=now() where charac_no=%u and "
        "tower_index=%u and stage_%u<=%u",
        member_count, members, member_count, stage, member_count, play_time,
        member_count, charac_no, tower_index, member_count, stage);
    if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
        return true;
    }
    db_handle->set_query(
        "inSert into charac_tower_record(charac_no,tower_index,member_info_%u,"
        "stage_%u,play_time_%u,occ_time_%u) values(%u,%u,'%s',%u,%u,now())",
        member_count, member_count, member_count, member_count, charac_no,
        tower_index, members, stage, play_time);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x842A35E
void DB_UpdateDeathTowerBestRecord::makeRequest(
    unsigned int tower_index,
    unsigned int charac_no,
    unsigned int member_count,
    const stDeathTowerRecordMemberInfo_t* members,
    unsigned int stage,
    unsigned int play_time) {
    if (!members || !member_count || member_count > kDeathTowerMaxMembers) {
        return;
    }
    unsigned char request[kDeathTowerUpdateBytes];
    memset(request, 0, sizeof(request));
    WriteU32(request, 0, charac_no);
    WriteU16(request, 4, static_cast<uint16_t>(tower_index));
    WriteU16(request, 6, static_cast<uint16_t>(member_count));
    memcpy(request + 8, members,
           member_count * sizeof(stDeathTowerRecordMemberInfo_t));
    WriteU32(request, 100, stage);
    WriteU32(request, 104, play_time);
    Stream* stream = GlobalData::stream_pool()->Acquire("DBThread.cpp", 27646);
    CStreamGuard guard(stream, true);
    guard << 227;
    guard << -1;
    guard.put_binary(request, sizeof(request));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844E852 / 0x845E596
DB_DeathTowerLoadTopRanker::DB_DeathTowerLoadTopRanker() {}
DB_DeathTowerLoadTopRanker::~DB_DeathTowerLoadTopRanker() {}

// IDA_EA: 0x842A484
bool DB_DeathTowerLoadTopRanker::dispatch(int signal,
                                          int user_idx,
                                          Stream* stream) {
    (void)signal;
    (void)user_idx;
    unsigned char top_five_only = 0;
    if (!stream || stream->get_binary(&top_five_only, sizeof(top_five_only)) != 1) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    std::vector<unsigned char> response(kDeathTowerTopRankerBytes, 0);
    response[4] = top_five_only;
    uint32_t row_count = 0;
    for (unsigned int tower_index = 0; tower_index <= 4; ++tower_index) {
        for (unsigned int party_size = 1; party_size <= 4; ++party_size) {
            if (top_five_only) {
                db_handle->set_query(
                    "seLect b.rank,a.tower_index,a.member_info_%u,a.stage_%u,"
                    "a.play_time_%u from charac_tower_record a,"
                    "charac_tower_rank_top5 b where b.tower_index=%u and "
                    "b.tower_index=a.tower_index and b.rank<=5 and "
                    "a.charac_no=b.charac_no and b.part_type=%u order by "
                    "b.rank asc limit 5", party_size, party_size, party_size,
                    tower_index, party_size);
            } else {
                db_handle->set_query(
                    "seLect b.rank,a.tower_index,a.member_info_%u,a.stage_%u,"
                    "a.play_time_%u from charac_tower_record a,charac_tower_rank b "
                    "where b.tower_index=%u and b.tower_index=a.tower_index and "
                    "b.rank>5 and a.charac_no=b.charac_no and b.part_type=%u "
                    "order by b.rank asc limit 95", party_size, party_size,
                    party_size, tower_index, party_size);
            }
            if (db_handle->exec(true) != true) {
                return false;
            }
            const unsigned int rows = static_cast<unsigned int>(db_handle->get_n_rows());
            for (unsigned int row = 0; row < rows; ++row) {
                if (row_count >= 1000 || db_handle->fetch() != true) {
                    return false;
                }
                unsigned short rank = 0;
                unsigned short returned_tower_index = 0;
                unsigned int stage = 0;
                unsigned int play_time = 0;
                const unsigned int base = row_count * 108;
                if (db_handle->get_ushort(0, rank) != true ||
                    db_handle->get_ushort(1, returned_tower_index) != true ||
                    db_handle->get_binary(2, response.data() + 4016 + base,
                                          23 * static_cast<int>(party_size)) != true ||
                    db_handle->get_uint(3, stage) != true ||
                    db_handle->get_uint(4, play_time) != true) {
                    return false;
                }
                WriteU16(response.data(), 6 + row_count * 2, rank);
                WriteU16(response.data(), 4012 + base, returned_tower_index);
                WriteU16(response.data(), 4014 + base,
                         static_cast<uint16_t>(party_size));
                WriteU32(response.data(), 4108 + base, stage);
                WriteU32(response.data(), 4112 + base, play_time);
                ++row_count;
            }
        }
    }
    WriteU32(response.data(), 0, row_count);
    Stream* out = GlobalData::big_stream_pool()->Acquire("DBThread.cpp", 27681);
    CStreamGuard guard(out, true);
    guard << 228;
    guard << -1;
    guard.put_binary(response.data(), static_cast<int>(response.size()));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842A910
void DB_DeathTowerLoadTopRanker::makeRequest(bool top_five_only) {
    Stream* stream = GlobalData::big_stream_pool()->Acquire("DBThread.cpp", 27743);
    CStreamGuard guard(stream, true);
    guard << 228;
    guard << -1;
    guard << static_cast<unsigned char>(top_five_only ? 1 : 0);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x844ECB2 / 0x845DA9E
DB_UpdatePvPResult::DB_UpdatePvPResult() {}
DB_UpdatePvPResult::~DB_UpdatePvPResult() {}

// IDA_EA: 0x843097E
// IDA_NAME: _ZN18DB_UpdatePvPResult8dispatchEiiP6Stream
// DEMANGLED: DB_UpdatePvPResult::dispatch(int,int,Stream *)
bool DB_UpdatePvPResult::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SchoolInfo schools[2];
    SchoolInfo loaded_school;
    const time_t now =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    tm local_time;
    tm* const reset_time = localtime_r(&now, &local_time);
    const int week_day = reset_time->tm_wday;
    reset_time->tm_hour = 6;
    reset_time->tm_min = 0;
    reset_time->tm_sec = 0;
    time_t today_reset = mktime(reset_time);
    if (today_reset > now) {
        today_reset -= 86400;
    }
    const int current_reset = static_cast<int>(today_reset);
    const int previous_reset = current_reset - 86400;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(16), static_cast<ENUM_SERVER_GROUP>(0));
    Packet_PvPResult* const result = stream->GetOutBuffer<Packet_PvPResult>();

    for (int member = 0; member < result->player_count_per_team; ++member) {
        const int first_school_id =
            static_cast<int>(result->teams[0].school_numbers[member]);
        const int second_school_id =
            static_cast<int>(result->teams[1].school_numbers[member]);

        db_handle->set_query(
            "seLect school_id, rating, fight_cnt, unix_timestamp(last_update_time), "
            "school_point, school_point_prev, fight_cnt_prev from school_info "
            "where school_id in (%d,%d)",
            first_school_id,
            second_school_id);
        if (db_handle->exec(true) != true) {
            return false;
        }

        const int school_count = db_handle->get_n_rows();
        if (school_count != 2) {
            return false;
        }
        for (int row = 0; row < school_count && db_handle->fetch() == true;
             ++row) {
            if (db_handle->get_int(0, loaded_school.school_id) != true ||
                db_handle->get_float(1, loaded_school.rating) != true ||
                db_handle->get_int(2, loaded_school.fight_count) != true ||
                db_handle->get_int(3, loaded_school.last_update_time) != true ||
                db_handle->get_int(4, loaded_school.school_point) != true ||
                db_handle->get_int(5, loaded_school.school_point_prev) != true ||
                db_handle->get_int(6, loaded_school.fight_count_prev) != true) {
                return false;
            }
            loaded_school.rating_before = loaded_school.rating;

            if (loaded_school.school_id == first_school_id) {
                schools[0] = loaded_school;
            } else {
                if (loaded_school.school_id != second_school_id) {
                    return false;
                }
                schools[1] = loaded_school;
            }
        }

        CEloRating::calc(
            schools[0].rating,
            schools[1].rating,
            static_cast<ENUM_ELO_RATING_SYSTEM>(result->rating_result),
            result->school_match_elo_k2);

        for (int team = 0; team <= 1; ++team) {
            SchoolInfo& school = schools[team];
            school.contribution =
                static_cast<int>(school.rating - school.rating_before);

            if (school.last_update_time <= current_reset) {
                if (school.last_update_time <= previous_reset) {
                    school.fight_count_prev = 0;
                    school.school_point_prev = 0;
                } else {
                    school.fight_count_prev = school.fight_count;
                    school.school_point_prev = school.school_point;
                }
                school.fight_count = 0;
                school.school_point = 0;
            }

            ++school.fight_count;
            if (school.fight_count ==
                PvPGlobalEnvironmentVariable::
                    school_match_fight_count_for_gain_point_) {
                school.school_point =
                    PvPGlobalEnvironmentVariable::school_match_daily_point_;
            } else if (school.fight_count == 15) {
                if (week_day == 0 || week_day == 6) {
                    school.school_point =
                        PvPGlobalEnvironmentVariable::
                            school_match_daily_point_weekends_;
                }
                if (now > 1223845200 && now <= 1224277199) {
                    school.school_point =
                        PvPGlobalEnvironmentVariable::
                            school_match_daily_point_weekends_;
                }
            }

            db_handle->set_query(
                "upDate school_info set fight_cnt=%d,school_point=%d, "
                "fight_cnt_prev=%d, school_point_prev=%d, rating=%f, "
                "last_update_time=from_unixtime(%d) where school_id=%d",
                school.fight_count,
                school.school_point,
                school.fight_count_prev,
                school.school_point_prev,
                school.rating,
                static_cast<int>(now),
                static_cast<int>(result->teams[team].school_numbers[member]));
            if (db_handle->exec(true) != true) {
                return false;
            }

            cMyTrace school_trace(
                "virtual bool DB_UpdatePvPResult::dispatch(int, int, Stream*)",
                30823,
                5);
            school_trace(
                "fight_cnt=%d,school_point=%d, fight_cnt_prev=%d, "
                "school_point_prev=%d, rating=%f, last_update_time=%d, "
                "school_id=%d",
                school.fight_count,
                school.school_point,
                school.fight_count_prev,
                school.school_point_prev,
                school.rating,
                static_cast<int>(now),
                static_cast<int>(result->teams[team].school_numbers[member]));
        }
    }

    for (int member = 0; member < result->player_count_per_team; ++member) {
        for (int team = 0; team <= 1; ++team) {
            Packet_PvPResult::TeamResult& values = result->teams[team];
            if (values.awarded_badges[member][1] ||
                values.awarded_badges[member][2]) {
                ++values.awarded_badges[member][0];
            }

            const char* const account_id =
                NumberToString(values.account_ids[member], 0);
            cMyTrace badge_trace(
                "virtual bool DB_UpdatePvPResult::dispatch(int, int, Stream*)",
                30849,
                0);
            badge_trace(
                "BADGE_COUNT m_id(%s) badge green(%d), red(%d), blue(%d), "
                "daily badge green(%d), red(%d), blue(%d)",
                account_id,
                values.awarded_badges[member][0],
                values.awarded_badges[member][1],
                values.awarded_badges[member][2],
                values.daily_badges[member][0],
                values.daily_badges[member][1],
                values.daily_badges[member][2]);

            db_handle->set_query(
                "upDate school_member set contribute=contribute+%d, "
                "last_play_time=NOW(), green_badge=green_badge+%d, "
                "red_badge=red_badge+%d, blue_badge=blue_badge+%d, "
                "daily_green_badge=%d, daily_red_badge=%d, "
                "daily_blue_badge=%d where m_id=%s and school_id=%d",
                schools[team].contribution,
                values.awarded_badges[member][0],
                values.awarded_badges[member][1],
                values.awarded_badges[member][2],
                values.daily_badges[member][0],
                values.daily_badges[member][1],
                values.daily_badges[member][2],
                account_id,
                static_cast<int>(values.school_numbers[member]));
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    }
    return true;
}

// IDA_EA: 0x844ECCE / 0x845DA50
DB_UpdatePvPLastPlayTime::DB_UpdatePvPLastPlayTime() {}
DB_UpdatePvPLastPlayTime::~DB_UpdatePvPLastPlayTime() {}

// IDA_EA: 0x84317B2
bool DB_UpdatePvPLastPlayTime::dispatch(int signal,
                                        int user_idx,
                                        Stream* stream) {
    (void)signal;
    (void)user_idx;
    Packet_PvPResult result;
    if (!stream || stream->get_binary(&result, sizeof(result)) != 1 ||
        result.player_count_per_team > 4) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(16), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    for (unsigned int member = 0; member < result.player_count_per_team; ++member) {
        for (unsigned int team = 0; team < 2; ++team) {
            const Packet_PvPResult::TeamResult& values = result.teams[team];
            db_handle->set_query(
                "upDate school_member set last_play_time=NOW() where m_id=%u "
                "and school_id=%u", values.account_ids[member],
                values.school_numbers[member]);
            if (db_handle->exec(true) != true) {
                return false;
            }
        }
    }
    return true;
}

// IDA_EA: 0x844FBCA / 0x8128B34
CLocalGlobal_DB_STATICS_BOSSTOWER::CLocalGlobal_DB_STATICS_BOSSTOWER() {}
CLocalGlobal_DB_STATICS_BOSSTOWER::~CLocalGlobal_DB_STATICS_BOSSTOWER() {}

// IDA_EA: 0x812825E
bool CLocalGlobal_DB_STATICS_BOSSTOWER::dispatch(int signal,
                                                 int user_idx,
                                                 Stream* stream) {
    (void)signal;
    (void)user_idx;
    int count = 0;
    if (!stream || stream->get_binary(&count, sizeof(count)) != 1 || count <= 0 ||
        count > 1024) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(4), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    time_t now = time(0);
    tm date_value;
    localtime_r(&now, &date_value);
    char date[12];
    snprintf(date, sizeof(date), "%04d-%02d-%02d", date_value.tm_year + 1900,
             date_value.tm_mon + 1, date_value.tm_mday);
    for (int index = 0; index < count; ++index) {
        unsigned int key_id = 0;
        unsigned char metadata[4];
        STBossDungeonTowerLog totals;
        if (stream->get_binary(&key_id, sizeof(key_id)) != 1 ||
            stream->get_binary(metadata, sizeof(metadata)) != 1 ||
            stream->get_binary(&totals, sizeof(totals)) != 1) {
            return false;
        }
        db_handle->set_query(
            "upDate log_monstertower_layer_stat SET play_cnt=play_cnt+%u,"
            "success_cnt=success_cnt+%u,play_time=play_time+%u,"
            "uniq_cnt=uniq_cnt+%u WHERE occ_date='%s' and key_id=%u",
            totals.stage_start_count, totals.stage_end_count, totals.elapsed_time,
            totals.picked_item_count, date, key_id);
        if (db_handle->exec(true) == true && db_handle->getAffectedRowCount()) {
            continue;
        }
        db_handle->set_query(
            "inSert into log_monstertower_layer_stat(occ_date,key_id,layer,"
            "monster_diff,charac_job,charac_grow,level,play_cnt,success_cnt,"
            "play_time,uniq_cnt) values('%s',%u,%u,%u,%u,%u,%u,%u,%u,%u,%u)",
            date, key_id, metadata[0], metadata[3] >> 4, metadata[3] & 0x0f,
            metadata[2], metadata[1], totals.stage_start_count,
            totals.stage_end_count, totals.elapsed_time, totals.picked_item_count);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}
