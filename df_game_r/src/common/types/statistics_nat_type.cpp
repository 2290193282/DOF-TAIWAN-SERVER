#include "common/types/statistics_nat_type.h"

#include <cstring>
#include <ctime>
#include <new>
#include <utility>

#include "common/manager/global_data.h"
#include "old_data/db/core/nat_type_payload.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x860C704
// IDA_NAME: _ZN17StatisticsNatTypeC2Ev
// DEMANGLED: StatisticsNatType::StatisticsNatType(void)
StatisticsNatType::StatisticsNatType() : nat_type_counts_() {}

// IDA_EA: 0x860C718
// IDA_NAME: _ZN17StatisticsNatTypeD2Ev
// DEMANGLED: StatisticsNatType::~StatisticsNatType()
StatisticsNatType::~StatisticsNatType() {
    for (NatTypeMap::iterator it = nat_type_counts_.begin();
         it != nat_type_counts_.end();
         ++it) {
        if (it->second) {
            ::operator delete(it->second);
        }
    }
}

// IDA_EA: 0x860C7DA
// IDA_NAME: _ZN17StatisticsNatType5ResetEv
// DEMANGLED: StatisticsNatType::Reset(void)
int StatisticsNatType::Reset() {
    for (NatTypeMap::iterator it = nat_type_counts_.begin();
         it != nat_type_counts_.end();
         ++it) {
        RESULT_COUNT* const count = it->second;
        if (count) {
            count->fail_count = 0;
            count->success_count = 0;
        }
    }
    return 0;
}

// IDA_EA: 0x860C86C
// IDA_NAME: _ZN17StatisticsNatType15IncreaseNatTypeEiih
// DEMANGLED: StatisticsNatType::IncreaseNatType(int,int,unsigned char)
StatisticsNatType::RESULT_COUNT* StatisticsNatType::IncreaseNatType(
    int nat_type,
    int nat_type_other,
    unsigned char result) {
    const NAT_TYPE_PEER_TO_PEER key(nat_type, nat_type_other);
    NatTypeMap::iterator it = nat_type_counts_.find(key);
    RESULT_COUNT* count = 0;
    if (it != nat_type_counts_.end()) {
        count = it->second;
    } else {
        count = new RESULT_COUNT;
        nat_type_counts_.insert(std::make_pair(key, count));
    }

    if (result) {
        ++count->success_count;
    } else {
        ++count->fail_count;
    }
    return count;
}

// IDA_EA: 0x860C968
// IDA_NAME: _ZN17StatisticsNatType13UpdateNatTypeEv
// DEMANGLED: StatisticsNatType::UpdateNatType(void)
int StatisticsNatType::UpdateNatType() {
    const time_t occur_time = std::time(0);
    for (NatTypeMap::iterator it = nat_type_counts_.begin();
         it != nat_type_counts_.end();
         ++it) {
        RESULT_COUNT* const count = it->second;
        if (!count || (count->fail_count <= 0 && count->success_count <= 0)) {
            continue;
        }

        Stream* const stream = GlobalData::s_stream_pool->Acquire(
            "Statistics.cpp", 455);
        CStreamGuard guard(stream, true);
        guard << 131;
        guard << -1;
        SIG_NAT_TYPE* const payload = guard.GetInBuffer<SIG_NAT_TYPE>();
        std::memset(payload, 0, sizeof(*payload));
        payload->nat_type = it->first.nat_type;
        payload->nat_type_other = it->first.nat_type_other;
        payload->fail_count = count->fail_count;
        payload->success_count = count->success_count;
        payload->occ_time = static_cast<int32_t>(occur_time);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    }
    return Reset();
}

// IDA_EA: 0x861021E
// IDA_NAME: _ZNK17StatisticsNatType21NAT_TYPE_PEER_TO_PEERltERKS0_
// DEMANGLED: StatisticsNatType::NAT_TYPE_PEER_TO_PEER::operator<(StatisticsNatType::NAT_TYPE_PEER_TO_PEER const&)const
bool StatisticsNatType::NAT_TYPE_PEER_TO_PEER::operator<(
    const NAT_TYPE_PEER_TO_PEER& other) const {
    if (nat_type < other.nat_type) {
        return true;
    }
    return nat_type == other.nat_type && nat_type_other < other.nat_type_other;
}

// IDA_EA: 0x8610262
// IDA_NAME: _ZN17StatisticsNatType21NAT_TYPE_PEER_TO_PEERC2Eii
// DEMANGLED: StatisticsNatType::NAT_TYPE_PEER_TO_PEER::NAT_TYPE_PEER_TO_PEER(int,int)
StatisticsNatType::NAT_TYPE_PEER_TO_PEER::NAT_TYPE_PEER_TO_PEER(
    int first_nat_type,
    int second_nat_type)
    : nat_type(first_nat_type <= second_nat_type ? first_nat_type
                                                  : second_nat_type),
      nat_type_other(first_nat_type <= second_nat_type ? second_nat_type
                                                        : first_nat_type) {}

// IDA_EA: 0x8610294
// IDA_NAME: _ZN17StatisticsNatType12RESULT_COUNTC2Ev
// DEMANGLED: StatisticsNatType::RESULT_COUNT::RESULT_COUNT(void)
StatisticsNatType::RESULT_COUNT::RESULT_COUNT()
    : success_count(0), fail_count(0) {}
