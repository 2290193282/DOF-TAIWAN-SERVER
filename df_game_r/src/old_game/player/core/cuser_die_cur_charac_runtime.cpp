#include "old_game/player/core/cuser.h"

#include "old_data/PVF/data/datamanager/data_manager.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "common/manager/system_time.h"

#include <stdint.h>

// IDA_EA: 0x864628E
// IDA_NAME: _ZN15CUserCharacInfo12DieCurCharacEiP5CUser
// DEMANGLED: CUserCharacInfo::DieCurCharac(int,CUser *)
int CUserCharacInfo::DieCurCharac(int reason, CUser* attacker) {
    _Charac_info* const current_charac =
        const_cast<_Charac_info*>(getCurCharacR());
    int result = static_cast<int>(
        reinterpret_cast<intptr_t>(current_charac));
    if (!current_charac) {
        return result;
    }

    enableSaveCharacStat();
    current_charac->fields_.ghost_state_ = 1;

    const int current_second = GlobalData::system_time()->getCurSec();
    current_charac->fields_.assault_session_state_ =
        current_second - current_charac->fields_.chaos_die_time_;
    current_charac->fields_.chaos_die_time_ = reason + current_second;

    int ghost_time = 0;
    if (CPowerManager::IsPowerWarEventOn() || getCurCharacVill() == 7) {
        const long double response_penalty =
            GlobalData::s_power_manager->GetPowerWarResponPenalty(attacker);
        ghost_time = GlobalData::s_power_manager->GetPowerWarGhostTime(
            attacker, static_cast<float>(response_penalty));
    } else {
        const ServerParameterScript* const server_parameter =
            G_CDataManager()->get_server_parameter_script();
        ghost_time = server_parameter->getAssaultGhostTime(
            get_charac_level(), GetCurCharacChaosPoint());
    }

    result = IsCurCharacGhost() && ghost_time;
    if (result != 0) {
        const int response_second = GlobalData::system_time()->getCurSec();
        SetCurCharacChaosDieTime(response_second);
        return SetCurCharacChaosResponTime(response_second + ghost_time);
    }
    return result;
}

// IDA_EA: 0x8646406
// IDA_NAME: _ZN15CUserCharacInfo13LiveCurCharacEv
// DEMANGLED: CUserCharacInfo::LiveCurCharac(void)
int CUserCharacInfo::LiveCurCharac() {
    _Charac_info* const current_charac =
        const_cast<_Charac_info*>(getCurCharacR());
    int result = static_cast<int>(reinterpret_cast<intptr_t>(current_charac));
    if (!current_charac) {
        return result;
    }

    enableSaveCharacStat();
    current_charac->fields_.ghost_state_ = 0;
    current_charac->fields_.chaos_die_time_ = 0;
    current_charac->fields_.assault_session_state_ = 0;
    result = !CPowerManager::IsPowerWarEventOn() || getCurCharacVill() != 7;
    if (result) {
        current_charac->fields_.stamina_ =
            current_charac->fields_.backup_stamina_;
    }
    return result;
}
