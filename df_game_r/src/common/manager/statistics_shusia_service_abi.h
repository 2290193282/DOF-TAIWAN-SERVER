#ifndef OLDSRV_GLOBAL_STATISTICS_SHUSIA_SERVICE_ABI_H_
#define OLDSRV_GLOBAL_STATISTICS_SHUSIA_SERVICE_ABI_H_

#include <stddef.h>

#include <map>

#include "infra/concurrent/thread/mutex.h"


// IDA unit: 99_misc_global/StatisticsShusiaService.  GameWorld owns one
// embedded instance at +0xA4; this definition describes that exact object.
class StatisticsShusiaService : public Mutex {
public:
    StatisticsShusiaService();
    ~StatisticsShusiaService();

    int lock();
    int unlock();
    int reset();
    int incDungeonFailure(int character_job);
    int getDungeonFailure(int character_job);
    int incServiceUsage(int character_job);
    int getServiceUSage(int character_job);

    std::map<int, int> dungeon_failures_;  // +0x18
    std::map<int, int> service_usages_;    // +0x30
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(StatisticsShusiaService, dungeon_failures_) == 0x18u,
              "StatisticsShusiaService dungeon failures must stay at +0x18");
static_assert(offsetof(StatisticsShusiaService, service_usages_) == 0x30u,
              "StatisticsShusiaService service usage must stay at +0x30");
static_assert(sizeof(StatisticsShusiaService) == 0x48u,
              "StatisticsShusiaService IDA ABI size must stay 0x48");
#endif

#endif  // OLDSRV_GLOBAL_STATISTICS_SHUSIA_SERVICE_ABI_H_
