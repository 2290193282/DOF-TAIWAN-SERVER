#ifndef OLDSRV_DATAMANAGER_ARAD_EVENT_PERIOD_DATA_MANAGER_H_
#define OLDSRV_DATAMANAGER_ARAD_EVENT_PERIOD_DATA_MANAGER_H_

#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/arad/datamanager/arad_event_reward.h"
#include "old_data/PVF/arad/datamanager/arad_reward_functor.h"
#include "old_data/PVF/common/arad_script_data.h"
#include "infra/concurrent/thread/mutex.h"

#include <stdint.h>

#include <list>
#include <vector>

class CUser;

namespace ARAD {

// IDA type: ARAD::Arad_EventPeriodDataManager (0x44 on i386).  The first
// event-period vector, reward-period vector, mutex, and reward de-dup list
// are all part of the parsed ARAD event-data owner; none are byte buffers.
class Arad_EventPeriodDataManager : public Arad_InterfaceGameScript {
public:
    Arad_EventPeriodDataManager();
    virtual ~Arad_EventPeriodDataManager();

    virtual bool loadScriptData();
    virtual void clearScriptData();
    int isRunning(ENUM_REPEAT_EVENT_CODE event_code);
    bool isApplied(ENUM_REPEAT_EVENT_CODE event_code, uint32_t current_sec);

    // Exact binary argument order at 0x8195794: event code, character-create
    // time, then account ID (the third argument is passed to CGMAccounts::isGM).
    bool checkCreateTime(ENUM_REPEAT_EVENT_CODE event_code,
                         uint32_t character_create_time,
                         uint32_t account_id);
    Arad_EventPeriodDataManager* setCreatePeriod_GM(int days);

    bool isRewarded(EventRewardItemArad item);
    bool addUserRewardData(CUser* user,
                           arad::SigLoadRewardEventItem& reward_items);
    int addUserRewardData(CUser* user, EventRewardItemArad& item);
    int delUserRewardData(CUser* user);

    template <typename RewardFunctor>
    bool eventReward(ENUM_REPEAT_EVENT_CODE event_code,
                     uint32_t current_sec,
                     CUser* user);

private:
    int create_period_gm_days_;
    std::vector<SCRIPT::SEventPeriodData> event_periods_;
    std::vector<SCRIPT::SRewardPeriodData> reward_periods_;
    Mutex reward_mutex_;
    std::list<EventRewardItemArad> reward_items_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_EventPeriodDataManager) == 0x44u,
              "Arad_EventPeriodDataManager IDA ABI size must stay 0x44");
#endif

template <>
bool Arad_EventPeriodDataManager::eventReward<FnRewardLethe>(
    ENUM_REPEAT_EVENT_CODE event_code,
    uint32_t current_sec,
    CUser* user);

template <>
bool Arad_EventPeriodDataManager::eventReward<FnRewardMomoji>(
    ENUM_REPEAT_EVENT_CODE event_code,
    uint32_t current_sec,
    CUser* user);

}  // namespace ARAD

#endif  // OLDSRV_DATAMANAGER_ARAD_EVENT_PERIOD_DATA_MANAGER_H_
