#include "old_data/PVF/data/data_manager.h"

#include "old_data/PVF/etc/action_point_etc_parameter.h"

// IDA_EA: 0x8122ACE
// IDA_NAME: _ZN12CDataManager26GetActionPointEtcParameterEb
// DEMANGLED: CDataManager::GetActionPointEtcParameter(bool)
APSystem::CActionPointEtcParameter*
CDataManager::GetActionPointEtcParameter(bool is_gm) {
    return is_gm ? &gm_action_point_parameter_ : &action_point_parameter_;
}
