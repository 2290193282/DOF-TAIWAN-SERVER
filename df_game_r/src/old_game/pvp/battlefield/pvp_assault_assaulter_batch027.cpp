#include "old_game/pvp/battlefield/pvp_assault.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <cstring>

namespace {

const int kMaxPvpPlayer = 8;

}  // namespace

namespace pvp_assault {

// IDA_EA: 0x82E661E
// IDA_NAME: _ZN11pvp_assault10CAssaulterC2Ev
// DEMANGLED: pvp_assault::CAssaulter::CAssaulter(void)
CAssaulter::CAssaulter()
    : user(0),
      team(0) {}

// IDA_EA: 0x82E6636
// IDA_NAME: _ZN11pvp_assault10CAssaulterD2Ev
// DEMANGLED: pvp_assault::CAssaulter::~CAssaulter()
CAssaulter::~CAssaulter() {}

// IDA_EA: 0x82E663C
// IDA_NAME: _ZNK11pvp_assault10CAssaulter13GetCharacNameEv
// DEMANGLED: pvp_assault::CAssaulter::GetCharacName(void)const
const char* CAssaulter::GetCharacName() const {
    return user ? user->getCurCharacName() : "";
}

// IDA_EA: 0x82E6688
// IDA_NAME: _ZN11pvp_assault10CAssaulter5GetHpEv
// DEMANGLED: pvp_assault::CAssaulter::GetHp(void)
int CAssaulter::GetHp() {
    return hp;
}

// IDA_EA: 0x82E6694
// IDA_NAME: _ZN11pvp_assault10CAssaulter8ChangeHpEi
// DEMANGLED: pvp_assault::CAssaulter::ChangeHp(int)
bool CAssaulter::ChangeHp(int hp_value) {
    if (hp_value < 0 || hp_value > 100) {
        cMyTrace trace("bool pvp_assault::CAssaulter::ChangeHp(int)",
                       394, 0);
        trace("ASSAULT_CHANGE_HP error (%d)", hp_value);
        return false;
    }

    if (CPowerManager::IsPowerWarEventOn() && user &&
        static_cast<unsigned char>(user->getCurCharacVill()) == 7u) {
        user->setPowerWarHP(static_cast<uint16_t>(hp_value));
    } else {
        hp = hp_value;
    }

    const int current_second = GlobalData::system_time()->getCurSec();
    const int elapsed = current_second - update_time;
    update_time = current_second;
    return elapsed > 2;
}

// IDA_EA: 0x82E6780
// IDA_NAME: _ZN11pvp_assault10CAssaulter15_PrepareAssaultEP5CUser
// DEMANGLED: pvp_assault::CAssaulter::_PrepareAssault(CUser *)
void CAssaulter::_PrepareAssault(CUser* assault_user) {
    if (!assault_user || !assault_user->getCurCharacR()) {
        return;
    }
    hp = static_cast<unsigned char>(assault_user->getCurCharacStamina());
    assault_user->SetCurCharacLeaveTimeout(false);
    CInventory* inventory = assault_user->getCurCharacInvenW();
    if (inventory && inventory->GetCreatureMgrW()) {
        inventory->GetCreatureMgrW()->TurnStomach(true);
    }
}

// IDA_EA: 0x82E67E6
// IDA_NAME: _ZN11pvp_assault10CAssaulter10AttachUserEP5CUser17ENUM_ASSAULT_TEAM
// DEMANGLED: pvp_assault::CAssaulter::AttachUser(CUser *,ENUM_ASSAULT_TEAM)
void CAssaulter::AttachUser(CUser* assault_user,
                            ENUM_ASSAULT_TEAM assault_team) {
    user = assault_user;
    team = static_cast<int>(assault_team);
    _PrepareAssault(assault_user);
}

// IDA_EA: 0x82E6812
// IDA_NAME: _ZN11pvp_assault10CAssaulter10DetachUserEv
// DEMANGLED: pvp_assault::CAssaulter::DetachUser(void)
void CAssaulter::DetachUser() {
    if (user && user->getCurCharacR()) {
        CInventory* inventory = user->getCurCharacInvenW();
        if (inventory && inventory->GetCreatureMgrW()) {
            inventory->GetCreatureMgrW()->TurnStomach(false);
        }
    }
    user = 0;
    team = 0;
}

// IDA_EA: 0x82E6868
// IDA_NAME: _ZN11pvp_assault10CAssaulter13GiveupPanaltyEv
// DEMANGLED: pvp_assault::CAssaulter::GiveupPanalty(void)
void CAssaulter::GiveupPanalty() {
    if (!user || !user->getCurCharacR()) {
        return;
    }

    user->setCurCharacBackupStamina(
        static_cast<uint8_t>(user->getCurCharacStamina()));
    const bool power_war = CPowerManager::IsPowerWarEventOn() ||
        static_cast<unsigned char>(user->getCurCharacVill()) == 7u;
    CUser_history_log(user)->GiveupPvP(
        static_cast<ENUM_PVP_HISTORY_REASON>(power_war ? 3 : 7));

    int ghost_time = 0;
    if (power_war) {
        const long double response_penalty =
            GlobalData::s_power_manager->GetPowerWarResponPenalty(user);
        ghost_time = GlobalData::s_power_manager->GetPowerWarGhostTime(
            user, static_cast<float>(response_penalty));
    } else {
        ServerParameterScript* server_parameter =
            G_CDataManager()->get_server_parameter_script();
        ghost_time = server_parameter->getAssaultGhostTime(
            user->get_charac_level(), user->GetCurCharacChaosPoint());
    }

    if (ghost_time != 0) {
        user->SetCurCharacGhost(true);
        const int current_second = GlobalData::system_time()->getCurSec();
        user->SetCurCharacChaosDieTime(current_second);
        user->SetCurCharacChaosResponTime(current_second + ghost_time);
        user->character_view().enableSaveCharacView();
    }
}

// IDA_EA: 0x82E6A40
// IDA_NAME: _ZN11pvp_assault10CAssaulter7DieUserEv
// DEMANGLED: pvp_assault::CAssaulter::DieUser(void)
void CAssaulter::DieUser() {
    if (!user) {
        return;
    }
    user->DieCurCharac(0, user);
    user->character_view().enableSaveCharacView();
}

// IDA_EA: 0x82E6A82
// IDA_NAME: _ZN11pvp_assault10CAssaulter17SetUserConnectP2PEPci
// DEMANGLED: pvp_assault::CAssaulter::SetUserConnectP2P(char *,int)
void CAssaulter::SetUserConnectP2P(char* connect_state, int player_count) {
    const int copy_count = std::min(player_count, kMaxPvpPlayer);
    if (connect_state && copy_count > 0) {
        std::memcpy(connect_p2p, connect_state,
                    static_cast<unsigned int>(copy_count));
    }
}

// IDA_EA: 0x82F057C
// IDA_NAME: _ZN11pvp_assault10CAssaulter8SetStateENS_18ENUM_ASSAULT_STATEE
// DEMANGLED: pvp_assault::CAssaulter::SetState(pvp_assault::ENUM_ASSAULT_STATE)
void CAssaulter::SetState(ENUM_ASSAULT_STATE assault_state) {
    state = static_cast<int>(assault_state);
}

// IDA_EA: 0x82F0596
// IDA_NAME: _ZN11pvp_assault10CAssaulter17SetPowerWarWinnerEb
// DEMANGLED: pvp_assault::CAssaulter::SetPowerWarWinner(bool)
void CAssaulter::SetPowerWarWinner(bool winner) {
    power_war_winner = winner ? 1u : 0u;
}

// IDA_EA: 0x82F05AE
// IDA_NAME: _ZN11pvp_assault10CAssaulter16IsPowerWarWinnerEv
// DEMANGLED: pvp_assault::CAssaulter::IsPowerWarWinner(void)
bool CAssaulter::IsPowerWarWinner() {
    return power_war_winner != 0;
}

// IDA_EA: 0x82F05BA
// IDA_NAME: _ZN11pvp_assault10CAssaulter5ResetEv
// DEMANGLED: pvp_assault::CAssaulter::Reset(void)
void CAssaulter::Reset() {
    user = 0;
    team = 0;
    state = 0;
    update_time = 0;
    power_war_winner = 0;
}

}  // namespace pvp_assault
