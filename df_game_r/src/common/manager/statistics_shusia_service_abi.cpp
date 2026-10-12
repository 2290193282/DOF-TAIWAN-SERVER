#include "common/manager/statistics_shusia_service_abi.h"

#include "infra/concurrent/thread/guard_mutex.h"

#include <utility>


// IDA_EA: 0x844D4FC
// IDA_NAME: _ZN23StatisticsShusiaService4lockEv
// DEMANGLED: StatisticsShusiaService::lock(void)
int StatisticsShusiaService::lock() {
    return Mutex::lock();
}

// IDA_EA: 0x844D510
// IDA_NAME: _ZN23StatisticsShusiaService6unlockEv
// DEMANGLED: StatisticsShusiaService::unlock(void)
int StatisticsShusiaService::unlock() {
    return Mutex::unlock();
}

// IDA_EA: 0x860C07A
// IDA_NAME: _ZN23StatisticsShusiaServiceC2Ev
// DEMANGLED: StatisticsShusiaService::StatisticsShusiaService(void)
StatisticsShusiaService::StatisticsShusiaService()
    : Mutex(), dungeon_failures_(), service_usages_() {}

// IDA_EA: 0x860C0E6
// IDA_NAME: _ZN23StatisticsShusiaServiceD2Ev
// DEMANGLED: StatisticsShusiaService::~StatisticsShusiaService()
StatisticsShusiaService::~StatisticsShusiaService() {}

// IDA_EA: 0x860C154
// IDA_NAME: _ZN23StatisticsShusiaService5resetEv
// DEMANGLED: StatisticsShusiaService::reset(void)
int StatisticsShusiaService::reset() {
    Guard<Mutex> guard(this);
    dungeon_failures_.clear();
    service_usages_.clear();
    return 0;
}

// IDA_EA: 0x860C1BA
// IDA_NAME: _ZN23StatisticsShusiaService17incDungeonFailureEi
// DEMANGLED: StatisticsShusiaService::incDungeonFailure(int)
int StatisticsShusiaService::incDungeonFailure(int character_job) {
    Guard<Mutex> guard(this);
    std::map<int, int>::iterator found = dungeon_failures_.find(character_job);
    if (found == dungeon_failures_.end()) {
        dungeon_failures_.insert(std::make_pair(character_job, 1));
    } else {
        ++found->second;
    }
    return 0;
}

// IDA_EA: 0x860C2A8
// IDA_NAME: _ZN23StatisticsShusiaService17getDungeonFailureEi
// DEMANGLED: StatisticsShusiaService::getDungeonFailure(int)
int StatisticsShusiaService::getDungeonFailure(int character_job) {
    Guard<Mutex> guard(this);
    const std::map<int, int>::const_iterator found =
        dungeon_failures_.find(character_job);
    return found == dungeon_failures_.end() ? 0 : found->second;
}

// IDA_EA: 0x860C358
// IDA_NAME: _ZN23StatisticsShusiaService15incServiceUsageEi
// DEMANGLED: StatisticsShusiaService::incServiceUsage(int)
int StatisticsShusiaService::incServiceUsage(int character_job) {
    Guard<Mutex> guard(this);
    std::map<int, int>::iterator found = service_usages_.find(character_job);
    if (found == service_usages_.end()) {
        service_usages_.insert(std::make_pair(character_job, 1));
    } else {
        ++found->second;
    }
    return 0;
}

// IDA_EA: 0x860C446
// IDA_NAME: _ZN23StatisticsShusiaService15getServiceUSageEi
// DEMANGLED: StatisticsShusiaService::getServiceUSage(int)
int StatisticsShusiaService::getServiceUSage(int character_job) {
    Guard<Mutex> guard(this);
    const std::map<int, int>::const_iterator found =
        service_usages_.find(character_job);
    return found == service_usages_.end() ? 0 : found->second;
}
