#include "old_data/db/core/action_point_db_exchange_proc_abi.h"

#include <cstring>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_game/player/core/cuser.h"

namespace APSystem {

// IDA_EA: 0x8122AF4
// IDA_NAME: _ZN8APSystem19CActionPointManager5ResetEv
// DEMANGLED: APSystem::CActionPointManager::Reset(void)
CActionPointManager* CActionPointManager::Reset() {
    dirty_ = 0;
    previous_action_point_ = 0;
    std::memset(&action_, 0, sizeof(action_));
    return this;
}

// IDA_EA: 0x8121B0C
// IDA_NAME: _ZN8APSystem19CActionPointManager3SetERNS_22_SIG_LOAD_ACTION_POINTE
// DEMANGLED: APSystem::CActionPointManager::Set(APSystem::_SIG_LOAD_ACTION_POINT &)
_SIG_LOAD_ACTION_POINT* CActionPointManager::Set(
    _SIG_LOAD_ACTION_POINT& action_point) {
    previous_action_point_ = action_.ap_sum;
    action_ = action_point;
    return &action_;
}

// IDA_EA: 0x8694BFC
// IDA_NAME: _ZNK8APSystem19CActionPointManager14GetActionPointEv
// DEMANGLED: APSystem::CActionPointManager::GetActionPoint(void)const
const _SIG_LOAD_ACTION_POINT* CActionPointManager::GetActionPoint() const {
    return &action_;
}

// IDA_EA: 0x8121B3E
// IDA_NAME: _ZN8APSystem19CActionPointManager6GM_SetER5CUserj
// DEMANGLED: APSystem::CActionPointManager::GM_Set(CUser &,unsigned int)
bool CActionPointManager::GM_Set(CUser& user, uint32_t action_point) {
    dirty_ = 1;
    action_.ap_sum = 0;
    previous_action_point_ = 0;

    for (int point = 0; point <= 299; ++point) {
        _ActionPoint& entry = action_.action_points[point];
        for (int level = 0; level <= 4; ++level) {
            const int clear_state = static_cast<int8_t>(entry.clear_state[level]);
            if (clear_state > 0) {
                entry.clear_state[level] = static_cast<uint8_t>(-clear_state);
            }
        }
    }

    for (int point = 0; point <= 299; ++point) {
        _ActionPoint& entry = action_.action_points[point];
        for (int level = 0;
             level <= 4 && action_.ap_sum < action_point;
             ++level) {
            const int clear_state = static_cast<int8_t>(entry.clear_state[level]);
            if (clear_state < 0) {
                previous_action_point_ = action_.ap_sum;
                entry.clear_state[level] = static_cast<uint8_t>(-clear_state);
                action_.ap_sum +=
                    static_cast<int>(static_cast<int8_t>(entry.clear_state[level]));
                CUserProc::SendActionClear(user,
                                           entry.action_index,
                                           entry.action_group_index,
                                           true,
                                           ENUM_PACKETCLASS_NOTIPACKET);
                CUserProc::CheckAndSendToMessageAllUser(
                    previous_action_point_,
                    action_.ap_sum,
                    user.getCurCharacName());
            }
        }
    }

    // The original loop condition is the return expression, so it returns
    // false after its final (point == 300) test.
    return false;
}

// IDA_EA: 0x8121D12
// IDA_NAME: _ZN8APSystem19CActionPointManager8GM_ResetEv
// DEMANGLED: APSystem::CActionPointManager::GM_Reset(void)
bool CActionPointManager::GM_Reset() {
    dirty_ = 1;
    std::memset(&action_.reward_item_1, 0, 4);
    action_.reward_medal = 0;
    action_.ap_sum = 0;
    previous_action_point_ = 0;

    for (int point = 0; point <= 299; ++point) {
        _ActionPoint& entry = action_.action_points[point];
        for (int level = 0; level <= 4; ++level) {
            const int clear_state = static_cast<int8_t>(entry.clear_state[level]);
            if (clear_state > 0) {
                entry.clear_state[level] = static_cast<uint8_t>(-clear_state);
            }
        }
    }

    return false;
}

// IDA_EA: 0x8121608
// IDA_NAME: _ZN8APSystem24CDBAndExchangeServerProc33RequestLoadActionPointFromDBAndESER5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: APSystem::CDBAndExchangeServerProc::RequestLoadActionPointFromDBAndES(CUser &,MSG_CHARAC_SELECT &)
int CDBAndExchangeServerProc::RequestLoadActionPointFromDBAndES(
    CUser& user,
    MSG_CHARAC_SELECT& select) {
    (void)user;
    (void)select;
    return 1;
}

// IDA_EA: 0x8121616
// IDA_NAME: _ZN8APSystem24CDBAndExchangeServerProc24GetActionPointFromMemoryERK5CUserRNS_22_SIG_LOAD_ACTION_POINTE
// DEMANGLED: APSystem::CDBAndExchangeServerProc::GetActionPointFromMemory(CUser const&,APSystem::_SIG_LOAD_ACTION_POINT &)
void CDBAndExchangeServerProc::GetActionPointFromMemory(
    const CUser& user,
    _SIG_LOAD_ACTION_POINT& action_point) {
    (void)user;
    (void)action_point;
}

}  // namespace APSystem

// IDA_DATA_EA: 0x93FF07C
// IDA_NAME: _ZN8APSystem19CActionPointManager7_IsGMOnE
// DEMANGLED: APSystem::CActionPointManager::_IsGMOn
bool APSystem::CActionPointManager::_IsGMOn = false;
