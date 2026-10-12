#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"

namespace online_preliminary {

// IDA_EA: 0x8588D60
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam8saveDataEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::saveData(void)
int COnlinePreliminaryTeam::saveData() {
    this->_saveTeam();
    return this->_saveTeamMatchList();
}

// IDA_EA: 0x8588D7E
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam19GetCurrentUserCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::GetCurrentUserCount(void)
int COnlinePreliminaryTeam::GetCurrentUserCount() {
    return current_user_count_;
}

// IDA_EA: 0x8588DA8
// IDA_NAME: _ZN18online_preliminary22COnlinePreliminaryTeam24DecreaseCurrentUserCountEv
// DEMANGLED: online_preliminary::COnlinePreliminaryTeam::DecreaseCurrentUserCount(void)
COnlinePreliminaryTeam*
COnlinePreliminaryTeam::DecreaseCurrentUserCount() {
    --current_user_count_;
    return this;
}

// IDA_EA: 0x8588DEC
// IDA_NAME: _ZN18online_preliminary12CTeamFactory4FreeEPNS_22COnlinePreliminaryTeamE
// DEMANGLED: online_preliminary::CTeamFactory::Free(online_preliminary::COnlinePreliminaryTeam *)
int CTeamFactory::Free(COnlinePreliminaryTeam* team) {
    return reinterpret_cast<int>(
        StaticPool<COnlinePreliminaryTeam, 600>::Free(team));
}

// COnlinePreliminaryTeamMgr::OnLeaveUser moved to the canonical C-owned
// old_game/userinfo/online_preliminary_team_runtime.cpp.

}  // namespace online_preliminary
