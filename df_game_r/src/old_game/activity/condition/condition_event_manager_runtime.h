#ifndef OLDSRV_OLD_GAME_EVENT_CONDITION_EVENT_MANAGER_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_CONDITION_EVENT_MANAGER_RUNTIME_H_

#include <map>
#include <utility>
#include <vector>

#include "old_game/activity/schedule/event_value_types.h"

class CUser;

// IDA unit: 11_event_runtime/CConditionEventManager.
// The object itself is deliberately empty: CDataManager owns all condition
// event tables, and CGameManager lazily allocates the original one-byte C2.
class CConditionEventManager {
public:
    // IDA_EA: 0x8334844 / 0x833484A
    // IDA_NAME: _ZN22CConditionEventManagerC2Ev /
    //           _ZN22CConditionEventManagerD2Ev
    CConditionEventManager();
    ~CConditionEventManager();

    // IDA_EA: 0x8334850
    // IDA_NAME: _ZN22CConditionEventManager27RequestConditionEventRewardEP5CUserscRSt3mapIiiSt4lessIiESaISt4pairIKiiEEE
    int RequestConditionEventReward(CUser* user,
                                    short event_index,
                                    char reward_step,
                                    std::map<int, int>& rewards);

    // IDA_EA: 0x8335264 / 0x83353A0 / 0x83354F0
    bool ConditionRewardSameItemCombine(
        std::vector<stConditionEventRewardInfo>& source,
        std::map<int, int>& rewards);
    char GetConditionRewardInfo(
        char quick_party,
        std::vector<stConditionEventRewardInfo>& rewards,
        short event_index,
        char reward_step);
    int GetMaxRewardStep(short event_index);

    // IDA_EA: 0x8335566 / 0x83355E6 / 0x83356A4
    int ProcessCheckStepUp(CUser* user, short event_index);
    void _processProperDungeonClearEvent(CUser* user, short event_index);
    std::pair<int, int> _getRequiredParameter(short reward_step);

    // IDA_EA: 0x8335754 / 0x833579E / 0x833586C
    int GetCurEventIndex();
    int IsFreeGoldCard(CUser* user, char reward_step);
    int IsFreeGoldCardProcess(
        std::vector<stConditionEventRewardInfo>& rewards);

    // IDA_EA: 0x83358BC / 0x8335A3A / 0x8335A8A
    int IsDoubleGoldCard(CUser* user, char reward_step);
    int IsDoubleGoldCardProcess(
        std::vector<stConditionEventRewardInfo>& rewards);
    int CheckErrorCardForStep(CUser* user, char reward_step);
};

static_assert(sizeof(CConditionEventManager) == 1u,
              "CConditionEventManager raw C2 must remain a one-byte object");

#endif  // OLDSRV_OLD_GAME_EVENT_CONDITION_EVENT_MANAGER_RUNTIME_H_
