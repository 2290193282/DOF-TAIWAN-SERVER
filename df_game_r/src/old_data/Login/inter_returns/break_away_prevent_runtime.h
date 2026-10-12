#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_BREAK_AWAY_PREVENT_RUNTIME_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_BREAK_AWAY_PREVENT_RUNTIME_H_

#include <map>

#include "old_game/activity/schedule/event_messages.h"
#include "old_game/activity/schedule/event_value_types.h"

class CUser;
struct SIG_LOGIN_DATA;

namespace break_away_prevent {

// IDA unit: 11_event_runtime/break_away_prevent/CBreakAwayPreventSystem.
// The DB signal is the base subobject; the map and the five three-byte keys
// immediately follow it in the old i386 allocation.
class CBreakAwayPreventSystem : public SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER {
public:
    // IDA_EA: 0x831C3E0 / 0x831C486.
    CBreakAwayPreventSystem();
    ~CBreakAwayPreventSystem();

    // IDA_EA: 0x831C49E.
    bool loadDbData(SIG_BREAK_AWAY_PREVENT_SYSTEM_MANAGER* source);

    // IDA_EA: 0x831C794 / 0x831C810 / 0x831C872.
    unsigned int getDiffConnectDay(unsigned int timestamp);
    bool calBreakAwayUserPoint();
    int selectReward(int reward_kind, int reward_point, CUser* user);

    // IDA_EA: 0x831CA5A / 0x831CB56 / 0x831CE5E / 0x831CE78.
    int GM_UpdateUvList(unsigned int account_id, unsigned int update_kind);
    int updateUvList(const SIG_LOGIN_DATA* login_data);
    void updateUsedCera(unsigned int account_id, unsigned int used_cera);
    int startBreakAwayPreventSystem(const SIG_LOGIN_DATA* login_data,
                                    CUser* user);

    // IDA_EA: 0x831D164.  The dungeon-clear mail transaction remains owned
    // by Inter_LoadEtc; this declaration retains the original ABI while that
    // separate client-command closure is restored.
    int giveBreakAwayDungeonClearReward(CUser* user);

private:
    std::map<STRewardDataKey, STRewardDataList> reward_data_;  // +0x2e4
    STRewardDataKey reward_keys_[5];                           // +0x2fc
    char uv_list_[0x1f];                                       // +0x30b
};

}  // namespace break_away_prevent

#if defined(__i386__)
static_assert(sizeof(break_away_prevent::CBreakAwayPreventSystem) == 0x32cu,
              "CBreakAwayPreventSystem must retain the old 0x32c-byte ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_BREAK_AWAY_PREVENT_RUNTIME_H_
