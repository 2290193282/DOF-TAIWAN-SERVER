#include "common/manager/common_statistics_manager_abi.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA unit: 99_misc_global/CStatisticContainer,
// 99_misc_global/CCommonStatisticsMgr, and Global::GetInstanceCommonStatisticsMgr.
// Every concrete statistic container is the same 0x1c i386 base object: a
// vptr followed by std::map<int, int>.  The three derived names add no state.
CStatisticContainer::CStatisticContainer()
    : quantities_() {}

CStatisticContainer::~CStatisticContainer() {}

// IDA_EA: 0x860E7F8
// IDA_NAME: _ZN19CStatisticContainer16IncreaseQuantityEi
// DEMANGLED: CStatisticContainer::IncreaseQuantity(int)
int* CStatisticContainer::IncreaseQuantity(int key) {
    int& quantity = quantities_[key];
    ++quantity;
    return &quantity;
}

// IDA_EA: 0x8610494
// IDA_NAME: _ZN17CHelpedPartyCountC2Ev
// DEMANGLED: CHelpedPartyCount::CHelpedPartyCount(void)
CHelpedPartyCount::CHelpedPartyCount() : CStatisticContainer() {}

// IDA_EA: 0x860E8AA
// IDA_NAME: _ZN17CHelpedPartyCount14InsertDatabaseEv
// DEMANGLED: CHelpedPartyCount::InsertDatabase(void)
void CHelpedPartyCount::InsertDatabase() {
    if (quantities_.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1298), true);
    guard << 259;
    guard << -1;
    for (int key = 0; key <= 4; ++key) {
        guard << quantities_[key];
    }
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    quantities_.clear();
}

// IDA_EA: 0x86104B0
// IDA_NAME: _ZN14CPVPPlayerRateC2Ev
// DEMANGLED: CPVPPlayerRate::CPVPPlayerRate(void)
CPVPPlayerRate::CPVPPlayerRate() : CStatisticContainer() {}

// IDA_EA: 0x860E9E0
// IDA_NAME: _ZN14CPVPPlayerRate14InsertDatabaseEv
// DEMANGLED: CPVPPlayerRate::InsertDatabase(void)
void CPVPPlayerRate::InsertDatabase() {
    if (quantities_.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1317), true);
    guard << 260;
    guard << -1;
    for (int key = 0; key <= 5; ++key) {
        guard << quantities_[key];
    }
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    quantities_.clear();
}

// IDA_EA: 0x86104CC
// IDA_NAME: _ZN12CPVPMapCountC2Ev
// DEMANGLED: CPVPMapCount::CPVPMapCount(void)
CPVPMapCount::CPVPMapCount() : CStatisticContainer() {}

// IDA_EA: 0x860EB16
// IDA_NAME: _ZN12CPVPMapCount14InsertDatabaseEv
// DEMANGLED: CPVPMapCount::InsertDatabase(void)
void CPVPMapCount::InsertDatabase() {
    if (quantities_.empty()) {
        return;
    }

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("Statistics.cpp", 1336), true);
    guard << 261;
    guard << -1;
    // The original passes the entry count via a byte-sized local.
    guard << static_cast<char>(quantities_.size());
    for (std::map<int, int>::const_iterator entry = quantities_.begin();
         entry != quantities_.end();
         ++entry) {
        guard << entry->first;
        guard << entry->second;
    }
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, guard);
    quantities_.clear();
}

// IDA_EA: 0x86104E8
// IDA_NAME: _ZN20CCommonStatisticsMgrC2Ev
// DEMANGLED: CCommonStatisticsMgr::CCommonStatisticsMgr(void)
CCommonStatisticsMgr::CCommonStatisticsMgr()
    : containers_() {
    containers_[ENUM_HELPED_PARTY_COUNT] = new CHelpedPartyCount();
    containers_[ENUM_PVP_PLAYER_RATE] = new CPVPPlayerRate();
    containers_[ENUM_PVP_MAP_COUNT] = new CPVPMapCount();
}

// IDA_EA: 0x8610432
// IDA_NAME: _ZN20CCommonStatisticsMgrD2Ev
// DEMANGLED: CCommonStatisticsMgr::~CCommonStatisticsMgr()
CCommonStatisticsMgr::~CCommonStatisticsMgr() {
    for (int index = 0; index <= 2; ++index) {
        delete containers_[index];
    }
}

// IDA_EA: 0x860E86E
// IDA_NAME: _ZN20CCommonStatisticsMgr14InsertDatabaseEv
// DEMANGLED: CCommonStatisticsMgr::InsertDatabase(void)
void CCommonStatisticsMgr::InsertDatabase() {
    for (int index = 0; index <= 2; ++index) {
        containers_[index]->InsertDatabase();
    }
}

int* CCommonStatisticsMgr::IncreaseQuantity(
    ENUM_STATISTIC_CONTAINER container,
    int key) {
    const int index = static_cast<int>(container);
    if (index < 0 || index > 2) {
        return reinterpret_cast<int*>(index);
    }
    return containers_[index]->IncreaseQuantity(key);
}

// IDA_EA: 0x860E765
// IDA_NAME: _Z30GetInstanceCommonStatisticsMgrv
// DEMANGLED: GetInstanceCommonStatisticsMgr(void)
CCommonStatisticsMgr* GetInstanceCommonStatisticsMgr() {
    static CCommonStatisticsMgr instance;
    return &instance;
}
