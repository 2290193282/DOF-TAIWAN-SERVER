#include "old_game/pvp/battlefield/power_war_runtime.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "core/config/env/environment.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/shared_server_message_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

typedef unsigned char (*IsEventingFn)(CEventBase*, CUser*);

}  // namespace

// IDA_EA: 0x8480F9E
// IDA_NAME: _ZN26STPowerWarStatueRankerInfoC2Ev
// DEMANGLED: STPowerWarStatueRankerInfo::STPowerWarStatueRankerInfo(void)
STPowerWarStatueRankerInfo::STPowerWarStatueRankerInfo() {
    load_complete = 0;
    charac_no = 0;
    job = 0;
    level = 0;
    grow_type = 0;
    std::memset(charac_name, 0, sizeof(charac_name));
    std::memset(guild_name, 0, sizeof(guild_name));
    std::memset(equip_item_id, 0, sizeof(equip_item_id));
}

// IDA_EA: 0x8481100
// IDA_NAME: _ZN11STPacketLogC2Ev
// DEMANGLED: STPacketLog::STPacketLog(void)
STPacketLog::STPacketLog() {}

// IDA_EA: 0x84810EA
// IDA_NAME: _ZN11STPacketLogD2Ev
// DEMANGLED: STPacketLog::~STPacketLog()
STPacketLog::~STPacketLog() {}

// IDA_EA: 0x8481030
// IDA_NAME: _ZN11STPacketLog7CompareEPKS_S1_
// DEMANGLED: STPacketLog::Compare(STPacketLog const*,STPacketLog const*)
bool STPacketLog::Compare(const STPacketLog* left,
                          const STPacketLog* right) {
    return left->count > right->count;
}

// IDA_EA: 0x848085A
// IDA_NAME: _ZN18CPowerWarPacketLogC2Ev
// DEMANGLED: CPowerWarPacketLog::CPowerWarPacketLog(void)
CPowerWarPacketLog::CPowerWarPacketLog() {
    Reset();
}

// IDA_EA: 0x848089C
// IDA_NAME: _ZN18CPowerWarPacketLogD2Ev
// DEMANGLED: CPowerWarPacketLog::~CPowerWarPacketLog()
CPowerWarPacketLog::~CPowerWarPacketLog() {
    Reset();
}

// IDA_EA: 0x84808DE
// IDA_NAME: _ZN18CPowerWarPacketLog5ResetEv
// DEMANGLED: CPowerWarPacketLog::Reset(void)
int CPowerWarPacketLog::Reset() {
    for (std::map<int, STPacketLog*>::iterator it = packet_logs_.begin();
         it != packet_logs_.end(); ++it) {
        delete it->second;
    }
    packet_logs_.clear();
    return 0;
}

// IDA_EA: 0x8480970
// IDA_NAME: _ZN18CPowerWarPacketLog15InsertPacketLogEit9eSendTypePKc
// DEMANGLED: CPowerWarPacketLog::InsertPacketLog(int,unsigned short,eSendType,char const*)
int CPowerWarPacketLog::InsertPacketLog(int packet_class,
                                        uint16_t packet_number,
                                        eSendType send_type,
                                        const char* packet_name) {
    const int map_key = static_cast<int>(packet_number);
    std::map<int, STPacketLog*>::iterator found = packet_logs_.find(map_key);
    if (found != packet_logs_.end()) {
        ++found->second->count;
        return static_cast<int>(
            reinterpret_cast<uintptr_t>(found->second));
    }

    STPacketLog* const entry = new STPacketLog();
    entry->send_type = send_type;
    entry->packet_class = packet_class;
    entry->packet_number = packet_number;
    entry->count = 1;
    entry->packet_name.assign(packet_name ? packet_name : "");
    packet_logs_.insert(std::make_pair(map_key, entry));
    return static_cast<int>(reinterpret_cast<uintptr_t>(entry));
}

// IDA_EA: 0x8480AC0
// IDA_NAME: _ZN18CPowerWarPacketLog28PrintPowerWarPacketLogReportEv
// DEMANGLED: CPowerWarPacketLog::PrintPowerWarPacketLogReport(void)
int CPowerWarPacketLog::PrintPowerWarPacketLogReport() {
    cMyTrace trace("void CPowerWarPacketLog::PrintPowerWarPacketLogReport()",
                   929, 0);
    trace("-------------------------------------------------------------------------------------------------------------");
    trace("|                                   POWAR WAR PACKET RESULT REPORT                                          |");
    trace("-------------------------------------------------------------------------------------------------------------");

    std::vector<STPacketLog*> ordered;
    for (std::map<int, STPacketLog*>::const_iterator it = packet_logs_.begin();
         it != packet_logs_.end(); ++it) {
        ordered.push_back(it->second);
    }
    std::sort(ordered.begin(), ordered.end(), STPacketLog::Compare);
    for (std::vector<STPacketLog*>::const_iterator it = ordered.begin();
         it != ordered.end(); ++it) {
        const STPacketLog* const entry = *it;
        trace("- PACKET CLASS:%d NO:%5d, COUNT:%8d, NAME:%s",
              entry->packet_class,
              static_cast<int>(entry->packet_number),
              entry->count,
              entry->packet_name.c_str());
    }
    return trace("-------------------------------------------------------------------------------------------------------------");
}

// IDA_EA: 0x847E998
// IDA_NAME: _ZN13CPowerManagerC2Ev
// DEMANGLED: CPowerManager::CPowerManager(void)
CPowerManager::CPowerManager()
    : winner_side_(0),
      power_a_(0),
      power_b_(0),
      channel_power_a_(0),
      channel_power_b_(0),
      power_user_count_a_(0),
      power_user_count_b_(0),
      power_up_remain_time_(0) {
    // Original C2 leaves +0x20 for Init(), then explicitly clears the
    // four-byte start-time record and the three 0x84-byte ranker objects.
    std::memset(&power_war_start_hour_, 0, 4u);
    std::memset(ranker_, 0, sizeof(ranker_));
    power_war_log_.Reset();
    power_war_packet_log_.Reset();
}

// IDA_EA: 0x847EABC
// IDA_NAME: _ZN13CPowerManager4InitEv
// DEMANGLED: CPowerManager::Init(void)
int CPowerManager::Init() {
    power_a_ = 0;
    power_b_ = 0;
    power_user_count_a_ = 0;
    power_user_count_b_ = 0;
    channel_power_a_ = 0;
    channel_power_b_ = 0;
    power_up_remain_time_ = 0;
    power_up_timer_period_ = 0;
    std::memset(&power_war_start_hour_, 0, 4u);
    std::memset(ranker_, 0, sizeof(ranker_));
    power_war_log_.Reset();
    power_war_packet_log_.Reset();
    return 1;
}

// IDA_EA: 0x847EB76
// IDA_NAME: _ZN13CPowerManager12SetPowerInfoEcii
// DEMANGLED: CPowerManager::SetPowerInfo(char,int,int)
CPowerManager* CPowerManager::SetPowerInfo(char winner_side,
                                           int power_a,
                                           int power_b) {
    winner_side_ = static_cast<uint8_t>(winner_side);
    power_a_ = power_a;
    power_b_ = power_b;
    return this;
}

// IDA_EA: 0x847EBA0
// IDA_NAME: _ZN13CPowerManager20GetRankingByCharacNoEj
// DEMANGLED: CPowerManager::GetRankingByCharacNo(unsigned int)
unsigned int CPowerManager::GetRankingByCharacNo(unsigned int charac_no) {
    for (unsigned int index = 0; index <= 2; ++index) {
        if (ranker_[index].charac_no == charac_no) {
            return index + 1;
        }
    }
    return 0;
}

// IDA_EA: 0x847F4FE
// IDA_NAME: _ZN13CPowerManager14LoadRankerInfoEv
// DEMANGLED: CPowerManager::LoadRankerInfo(void)
void CPowerManager::LoadRankerInfo() {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("DNF_PowerManager.cpp", 302),
        true);
    guard << 357;
    guard << -1;
    guard << G_CEnvironment()->get_server_group();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8480D98
// IDA_NAME: _ZN13CPowerManager16LoadPowerWarInfoEv
// DEMANGLED: CPowerManager::LoadPowerWarInfo(void)
void CPowerManager::LoadPowerWarInfo() {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("DNF_PowerManager.cpp", 964),
        true);
    guard << 411;
    guard << -1;
    guard << G_CEnvironment()->get_server_group();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x847F5DE
// IDA_NAME: _ZN13CPowerManager12ClearMVPInfoEv
// DEMANGLED: CPowerManager::ClearMVPInfo(void)
void CPowerManager::ClearMVPInfo() {
    cMyTrace clear_trace("void CPowerManager::ClearMVPInfo()", 323, 0);
    clear_trace("CLEAR MVP INFO");
    std::memset(ranker_, 0, sizeof(ranker_));

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("DNF_PowerManager.cpp", 329),
        true);
    guard << 421;
    guard << -1;
    guard << G_CEnvironment()->get_server_group();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);

    const int server_group = G_CEnvironment()->get_server_group();
    cMyTrace db_trace("void CPowerManager::ClearMVPInfo()", 338, 0);
    db_trace(
        "PUT DB MSG - E_DELETE_POWER_WAR_STATUE_MESSAGE / server_group : %d",
        server_group);

    CSharedServerMessageManager* const message_manager =
        G_CGameManager()->GetSharedServerMessageManager();
    if (message_manager) {
        message_manager->DeleteSpecificMessage(1);
        message_manager->DeleteSpecificMessage(2);
        message_manager->DeleteSpecificMessage(3);

        PacketGuard packet;
        packet.put_header(0, 192);
        packet.put_byte(2);
        packet.put_byte(3);
        const char* const null_string = "null string";
        const int null_string_length =
            static_cast<int>(std::strlen(null_string));
        for (int index = 0; index <= 2; ++index) {
            packet.put_byte(index + 1);
            packet.put_int(null_string_length);
            packet.put_str(null_string, null_string_length);
            packet.put_int(null_string_length);
            packet.put_str(null_string, null_string_length);
        }
        packet.finalize(true);
        G_GameWorld()->send_all_with_state(packet, 3);
    }
}

// IDA_EA: 0x847F910
// IDA_NAME: _ZN13CPowerManager17IsPowerWarEventOnEv
// DEMANGLED: CPowerManager::IsPowerWarEventOn(void)
int CPowerManager::IsPowerWarEventOn() {
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(30);
    void** const vtable = *reinterpret_cast<void***>(repeat_event);
    IsEventingFn const is_eventing =
        reinterpret_cast<IsEventingFn>(vtable[13]);
    return is_eventing(repeat_event, 0) != 0;
}

// IDA_EA: 0x847F952
// IDA_NAME: _ZN13CPowerManager15IsAbleToRevivalEP5CUserjRi
// DEMANGLED: CPowerManager::IsAbleToRevival(CUser *,unsigned int,int &)
int CPowerManager::IsAbleToRevival(CUser* user,
                                   uint32_t current_second,
                                   int& remaining_seconds) {
    // The original stores the penalty in a float before multiplying the
    // CDataManager power-war ghost time.  Preserve that intermediate
    // conversion; it controls the integer countdown visible to the client.
    const float response_penalty =
        static_cast<float>(GetPowerWarResponPenalty(user));
    const int required_seconds = GetPowerWarGhostTime(user, response_penalty);
    const int elapsed_seconds = static_cast<int>(
        current_second - static_cast<uint32_t>(user->GetCurCharacChaosDieTime()));
    if (required_seconds <= elapsed_seconds) {
        remaining_seconds = 0;
        return 1;
    }
    remaining_seconds = required_seconds - elapsed_seconds;
    return 0;
}

// IDA_EA: 0x848013C
// IDA_NAME: _ZN13CPowerManager22DecreasePowerUserCountEc
// DEMANGLED: CPowerManager::DecreasePowerUserCount(char)
CPowerManager* CPowerManager::DecreasePowerUserCount(char power_side) {
    int& count = (power_side == 1 || power_side == 3)
                     ? power_user_count_a_
                     : power_user_count_b_;
    --count;
    if (count < 0) {
        count = 0;
        return this;
    }
    return reinterpret_cast<CPowerManager*>(count);
}

// IDA_EA: 0x84801A2
// IDA_NAME: _ZN13CPowerManager29IncreaePowerWarPointInChannelEci
// DEMANGLED: CPowerManager::IncreaePowerWarPointInChannel(char,int)
CPowerManager* CPowerManager::IncreaePowerWarPointInChannel(
    char power_side,
    int point) {
    if (power_side == 1) {
        channel_power_a_ += point;
    } else {
        channel_power_b_ += point;
    }
    return this;
}

// IDA_EA: 0x847FA2E
// IDA_NAME: _ZN13CPowerManager20SetPowerWarStartTimeEhhhh
// DEMANGLED: CPowerManager::SetPowerWarStartTime(unsigned char,unsigned char,unsigned char,unsigned char)
CPowerManager* CPowerManager::SetPowerWarStartTime(uint8_t hour,
                                                    uint8_t minute,
                                                    uint8_t second,
                                                    uint8_t day) {
    power_war_start_hour_ = hour;
    power_war_start_minute_ = minute;
    power_war_start_second_ = second;
    power_war_start_day_ = day;
    return this;
}

// IDA_EA: 0x84EBE2E
// IDA_NAME: _ZN13CPowerManager16SetPowerWarPointEii
// DEMANGLED: CPowerManager::SetPowerWarPoint(int,int)
CPowerManager* CPowerManager::SetPowerWarPoint(int power_a, int power_b) {
    power_a_ = power_a;
    power_b_ = power_b;
    return this;
}

// IDA_EA: 0x8480E92
// IDA_NAME: _ZN13CPowerManager20GetPowerWarGhostTimeEP5CUserf
// DEMANGLED: CPowerManager::GetPowerWarGhostTime(CUser *,float)
int CPowerManager::GetPowerWarGhostTime(CUser* user, float response_penalty) {
    static_cast<void>(user);
    const CDataManager* const data_manager = G_CDataManager();
    const int ghost_time =
        data_manager->power_parameter_script_.ghost_revival_base_time;
    return static_cast<int>(static_cast<long double>(ghost_time) *
                            response_penalty);
}

// IDA_EA: 0x8480EC8
// IDA_NAME: _ZN13CPowerManager24GetPowerWarResponPenaltyEP5CUser
// DEMANGLED: CPowerManager::GetPowerWarResponPenalty(CUser *)
long double CPowerManager::GetPowerWarResponPenalty(CUser* user) {
    const CDataManager* const data_manager = G_CDataManager();
    const unsigned int chaos_state_time =
        static_cast<unsigned int>(user->GetCurCharacChaosStateTime());
    const unsigned int chaos_state_limit = static_cast<unsigned int>(
        data_manager->power_parameter_script_.respon_penalty_time);
    if (chaos_state_time < chaos_state_limit) {
        return 2.0L - static_cast<long double>(chaos_state_time) /
                          static_cast<long double>(chaos_state_limit);
    }
    return 1.0L;
}
