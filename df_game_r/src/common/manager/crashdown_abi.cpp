#include "common/manager/crashdown_abi.h"

#include <cstdio>
#include <ctime>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/db_types.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/concurrent/thread/guard_mutex.h"
#include "common/manager/system_time.h"


// IDA_EA: 0x844D4D0
// IDA_NAME: _ZN24StatisticsToObserveCrash4lockEv
// DEMANGLED: StatisticsToObserveCrash::lock(void)
int StatisticsToObserveCrash::lock() {
    return mutex_.lock();
}

// IDA_EA: 0x844D4E6
// IDA_NAME: _ZN24StatisticsToObserveCrash6unlockEv
// DEMANGLED: StatisticsToObserveCrash::unlock(void)
int StatisticsToObserveCrash::unlock() {
    return mutex_.unlock();
}

// IDA_EA: 0x860B934
// IDA_NAME: _ZN24StatisticsToObserveCrashC2Ev
// DEMANGLED: StatisticsToObserveCrash::StatisticsToObserveCrash(void)
StatisticsToObserveCrash::StatisticsToObserveCrash()
    : abnormal_down_user_count_(0),
      mutex_(),
      packet_counts_(),
      dungeon_counts_(),
      play_status_counts_() {
}

// IDA_EA: 0x860B9D2
// IDA_NAME: _ZN24StatisticsToObserveCrashD2Ev
// DEMANGLED: StatisticsToObserveCrash::~StatisticsToObserveCrash()
StatisticsToObserveCrash::~StatisticsToObserveCrash() {
}

// IDA_EA: 0x860BA6E
// IDA_NAME: _ZN24StatisticsToObserveCrash5resetEv
// DEMANGLED: StatisticsToObserveCrash::reset(void)
int StatisticsToObserveCrash::reset() {
    Guard<Mutex> guard(&mutex_);
    abnormal_down_user_count_ = 0;
    packet_counts_.clear();
    dungeon_counts_.clear();
    play_status_counts_.clear();
    return 0;
}

// IDA_EA: 0x860BAEE
// IDA_NAME: _ZN24StatisticsToObserveCrash24getAbnormalDownUserCountEv
// DEMANGLED: StatisticsToObserveCrash::getAbnormalDownUserCount(void)
int StatisticsToObserveCrash::getAbnormalDownUserCount() {
    Guard<Mutex> guard(&mutex_);
    return abnormal_down_user_count_;
}

// IDA_EA: 0x860BB22
// IDA_NAME: _ZN24StatisticsToObserveCrash24setAbnormalDownUserCountEi
// DEMANGLED: StatisticsToObserveCrash::setAbnormalDownUserCount(int)
int StatisticsToObserveCrash::setAbnormalDownUserCount(int count) {
    Guard<Mutex> guard(&mutex_);
    abnormal_down_user_count_ = count;
    return 0;
}

// IDA_EA: 0x860BB52
// IDA_NAME: _ZN24StatisticsToObserveCrash24incAbnormalDownUserCountEv
// DEMANGLED: StatisticsToObserveCrash::incAbnormalDownUserCount(void)
int StatisticsToObserveCrash::incAbnormalDownUserCount() {
    Guard<Mutex> guard(&mutex_);
    return ++abnormal_down_user_count_;
}

// IDA_EA: 0x860BB88
// IDA_NAME: _ZN24StatisticsToObserveCrash24incUserLastCrashPacketIDEi
// DEMANGLED: StatisticsToObserveCrash::incUserLastCrashPacketID(int)
int StatisticsToObserveCrash::incUserLastCrashPacketID(int packet_id) {
    Guard<Mutex> guard(&mutex_);
    return ++packet_counts_[packet_id];
}

// IDA_EA: 0x860BC7A
// IDA_NAME: _ZN24StatisticsToObserveCrash24getUserLastCrashPacketIDEi
// DEMANGLED: StatisticsToObserveCrash::getUserLastCrashPacketID(int)
int StatisticsToObserveCrash::getUserLastCrashPacketID(int packet_id) {
    Guard<Mutex> guard(&mutex_);
    std::map<int, int>::const_iterator found = packet_counts_.find(packet_id);
    return found == packet_counts_.end() ? 0 : found->second;
}

// IDA_EA: 0x860BD2E
// IDA_NAME: _ZN24StatisticsToObserveCrash25incUserLastCrashDungeonIDEi
// DEMANGLED: StatisticsToObserveCrash::incUserLastCrashDungeonID(int)
int StatisticsToObserveCrash::incUserLastCrashDungeonID(int dungeon_id) {
    Guard<Mutex> guard(&mutex_);
    return ++dungeon_counts_[dungeon_id];
}

// IDA_EA: 0x860BE20
// IDA_NAME: _ZN24StatisticsToObserveCrash25getUserLastCrashDungeonIDEi
// DEMANGLED: StatisticsToObserveCrash::getUserLastCrashDungeonID(int)
int StatisticsToObserveCrash::getUserLastCrashDungeonID(int dungeon_id) {
    Guard<Mutex> guard(&mutex_);
    std::map<int, int>::const_iterator found = dungeon_counts_.find(dungeon_id);
    return found == dungeon_counts_.end() ? 0 : found->second;
}

// IDA_EA: 0x860BED4
// IDA_NAME: _ZN24StatisticsToObserveCrash26incUserLastCrashPlayStatusEi
// DEMANGLED: StatisticsToObserveCrash::incUserLastCrashPlayStatus(int)
int StatisticsToObserveCrash::incUserLastCrashPlayStatus(int play_status) {
    Guard<Mutex> guard(&mutex_);
    return ++play_status_counts_[play_status];
}

// IDA_EA: 0x860BFC6
// IDA_NAME: _ZN24StatisticsToObserveCrash26getUserLastCrashPlayStatusEi
// DEMANGLED: StatisticsToObserveCrash::getUserLastCrashPlayStatus(int)
int StatisticsToObserveCrash::getUserLastCrashPlayStatus(int play_status) {
    Guard<Mutex> guard(&mutex_);
    std::map<int, int>::const_iterator found =
        play_status_counts_.find(play_status);
    return found == play_status_counts_.end() ? 0 : found->second;
}


// IDA_EA: 0x861007E
// IDA_NAME: _ZN29Packet_Reason_Crash_Down_InfoC2Ev
// DEMANGLED: Packet_Reason_Crash_Down_Info::Packet_Reason_Crash_Down_Info(void)
Packet_Reason_Crash_Down_Info::Packet_Reason_Crash_Down_Info()
    : header(0x0c48u, 0x001au), occ_time(0), channel_no(0), reason(0),
      count(0) {
}


// IDA_EA: 0x8610846
// IDA_NAME: _ZN28CClientCrashDownLogStatistic22stClientCrashDownLog_tC2Ev
// DEMANGLED: CClientCrashDownLogStatistic::stClientCrashDownLog_t::stClientCrashDownLog_t(void)
CClientCrashDownLogStatistic::stClientCrashDownLog_t::stClientCrashDownLog_t()
    : reason_counts_() {
}

// IDA_EA: 0x8612DFA
// IDA_NAME: _ZN28CClientCrashDownLogStatistic22stClientCrashDownLog_tC2ERKS0_
// DEMANGLED: CClientCrashDownLogStatistic::stClientCrashDownLog_t::stClientCrashDownLog_t(CClientCrashDownLogStatistic::stClientCrashDownLog_t const&)
CClientCrashDownLogStatistic::stClientCrashDownLog_t::stClientCrashDownLog_t(
    const stClientCrashDownLog_t& rhs)
    : occ_time_(rhs.occ_time_), reason_counts_(rhs.reason_counts_) {
}

// IDA_EA: 0x861085C
// IDA_NAME: _ZN28CClientCrashDownLogStatistic22stClientCrashDownLog_tD2Ev
// DEMANGLED: CClientCrashDownLogStatistic::stClientCrashDownLog_t::~stClientCrashDownLog_t()
CClientCrashDownLogStatistic::stClientCrashDownLog_t::~stClientCrashDownLog_t() {
}

// IDA_EA: 0x8612EA8
// IDA_NAME: _ZN28CClientCrashDownLogStatistic22stClientCrashDownLog_taSERKS0_
// DEMANGLED: CClientCrashDownLogStatistic::stClientCrashDownLog_t & CClientCrashDownLogStatistic::stClientCrashDownLog_t::operator=(CClientCrashDownLogStatistic::stClientCrashDownLog_t const&)
CClientCrashDownLogStatistic::stClientCrashDownLog_t&
CClientCrashDownLogStatistic::stClientCrashDownLog_t::operator=(
    const stClientCrashDownLog_t& rhs) {
    if (this != &rhs) {
        occ_time_ = rhs.occ_time_;
        reason_counts_ = rhs.reason_counts_;
    }
    return *this;
}

// IDA_EA: 0x86102DA
// IDA_NAME: _ZN28CClientCrashDownLogStatisticC2Ev
// DEMANGLED: CClientCrashDownLogStatistic::CClientCrashDownLogStatistic(void)
CClientCrashDownLogStatistic::CClientCrashDownLogStatistic()
    : current_reason_counts_(), carried_records_() {
}

// IDA_EA: 0x8610800
// IDA_NAME: _ZN28CClientCrashDownLogStatisticD2Ev
// DEMANGLED: CClientCrashDownLogStatistic::~CClientCrashDownLogStatistic()
CClientCrashDownLogStatistic::~CClientCrashDownLogStatistic() {
}

// IDA_EA: 0x860D038
// IDA_NAME: _ZN28CClientCrashDownLogStatistic23RegisterClientCrashDownEi
// DEMANGLED: CClientCrashDownLogStatistic::RegisterClientCrashDown(int)
int CClientCrashDownLogStatistic::RegisterClientCrashDown(int reason_code) {
    return ++current_reason_counts_[reason_code];
}

// IDA_EA: 0x860D072
// IDA_NAME: _ZN28CClientCrashDownLogStatistic20CarryClientCrashDownEv
// DEMANGLED: CClientCrashDownLogStatistic::CarryClientCrashDown(void)
int CClientCrashDownLogStatistic::CarryClientCrashDown() {
    const time_t timer =
        static_cast<time_t>(GlobalData::s_systemTime_.getCurSec());
    if (!current_reason_counts_.empty()) {
        Packet_Reason_Crash_Down_Info packet;
        // These are real old-server stack locals.  Carry computes them before
        // the loop, but does not copy any of them into `packet`.
        const time_t timer_1 = timer;
        const int channel_no = G_CEnvironment()->get_channel_no();
        (void)timer_1;
        (void)channel_no;
        for (std::map<int, int>::const_iterator iter =
                 current_reason_counts_.begin();
             iter != current_reason_counts_.end(); ++iter) {
            const int reason = iter->first;
            const int count = iter->second;
            (void)reason;
            (void)count;
            // Old CarryClientCrashDown at 0x860D072 constructs this packet
            // once and sends the untouched 0x1A-byte constructor image for
            // each map entry.  The calculated time/channel/reason/count
            // locals are not stored into the wire packet.
            GlobalData::s_statistic_proxy_mgr
                ->GetServerProxy(SERVER_GROUP_DEFAULT)
                ->SendPacket(reinterpret_cast<char*>(&packet), sizeof(packet));
        }

        stClientCrashDownLog_t carried;
        carried.occ_time_ = static_cast<int32_t>(timer);
        carried.reason_counts_ = current_reason_counts_;
        carried_records_.push_back(carried);
        current_reason_counts_.clear();
        Init();
    }

    std::tm local_time;
    localtime_r(&timer, &local_time);
    return 1;
}

// IDA_EA: 0x860D1FE
// IDA_NAME: _ZN28CClientCrashDownLogStatistic24MakeClientCrashDownQueryERSt6vectorISsSaISsEEi
// DEMANGLED: CClientCrashDownLogStatistic::MakeClientCrashDownQuery(std::vector<std::string,std::allocator<std::string>> &,int)
void CClientCrashDownLogStatistic::MakeClientCrashDownQuery(
    std::vector<std::string>& queries, int prefix_length) {
    for (std::vector<stClientCrashDownLog_t>::const_iterator record =
             carried_records_.begin();
         record != carried_records_.end(); ++record) {
        std::string values;
        for (std::map<int, int>::const_iterator reason =
                 record->reason_counts_.begin();
             reason != record->reason_counts_.end(); ++reason) {
            if (24576 - static_cast<int>(values.size()) < prefix_length) {
                break;
            }
            if (!values.empty()) {
                values += ',';
            }
            char tuple[256];
            std::sprintf(tuple, "(from_unixtime(%d),%d,%d,%d)",
                         static_cast<int>(record->occ_time_),
                         G_CEnvironment()->get_channel_no(), reason->first,
                         reason->second);
            values += tuple;
        }
        if (!values.empty()) {
            queries.push_back(values);
        }
    }
}

// IDA_EA: 0x860D444
// IDA_NAME: _ZN28CClientCrashDownLogStatistic21UpdateClientCrashDownEv
// DEMANGLED: CClientCrashDownLogStatistic::UpdateClientCrashDown(void)
int CClientCrashDownLogStatistic::UpdateClientCrashDown() {
    if (!carried_records_.empty()) {
        MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
            DB_HANDLE_LOG, SERVER_GROUP_DEFAULT);
        const std::string query_prefix(
            "inSert into log_client_ting(occ_time,channel_no,reason,cnt) values%s");
        std::vector<std::string> queries;
        MakeClientCrashDownQuery(queries,
                                 static_cast<int>(query_prefix.size()));
        for (std::vector<std::string>::const_iterator query = queries.begin();
             query != queries.end(); ++query) {
            if (!query->empty()) {
                db_handle->set_query(query_prefix.c_str(), query->c_str());
                db_handle->exec(true);
            }
        }
        carried_records_.clear();
    }
    return 1;
}

// IDA_EA: 0x86102D4
// IDA_NAME: _ZN28CClientCrashDownLogStatistic4InitEv
// DEMANGLED: CClientCrashDownLogStatistic::Init(void)
void CClientCrashDownLogStatistic::Init() {
}

// IDA_EA: 0x860CFA5
// IDA_NAME: _Z38GetInstanceClientCrashDownLogStatisticv
// DEMANGLED: GetInstanceClientCrashDownLogStatistic(void)
CClientCrashDownLogStatistic* GetInstanceClientCrashDownLogStatistic() {
    static CClientCrashDownLogStatistic instance;
    return &instance;
}
