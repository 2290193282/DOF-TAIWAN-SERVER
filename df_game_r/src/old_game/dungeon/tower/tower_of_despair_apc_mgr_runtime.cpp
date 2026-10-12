#include "old_game/dungeon/tower/tower_of_despair_apc_mgr_runtime.h"

#include <cstring>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x86444E6
// IDA_NAME: _ZN22TowerOfDespair_APC_MgrC2Ev
// DEMANGLED: TowerOfDespair_APC_Mgr::TowerOfDespair_APC_Mgr(void)
TowerOfDespair_APC_Mgr::TowerOfDespair_APC_Mgr() {
    _APCInfo empty;
    std::memset(&empty, 0, sizeof(empty));
    apc_infos_[TOD_Layer(0)].set_data(empty);
}

// IDA_EA: 0x86444FA
// IDA_NAME: _ZN22TowerOfDespair_APC_MgrD2Ev
// DEMANGLED: TowerOfDespair_APC_Mgr::~TowerOfDespair_APC_Mgr()
TowerOfDespair_APC_Mgr::~TowerOfDespair_APC_Mgr() {}

// IDA_EA: 0x85FED2E
// IDA_NAME: _ZN22TowerOfDespair_APC_Mgr12Set_APC_InfoERK9TOD_LayerRK8_APCInfo
// DEMANGLED: TowerOfDespair_APC_Mgr::Set_APC_Info(TOD_Layer const&,_APCInfo const&)
void TowerOfDespair_APC_Mgr::Set_APC_Info(const TOD_Layer& layer,
                                          const _APCInfo& info) {
    apc_infos_[layer].set_data(info);
}

// IDA_EA: 0x85FEED0
// IDA_NAME: _ZN22TowerOfDespair_APC_Mgr14Reset_APC_InfoEv
// DEMANGLED: TowerOfDespair_APC_Mgr::Reset_APC_Info(void)
int TowerOfDespair_APC_Mgr::Reset_APC_Info() {
    apc_infos_.clear();
    _APCInfo empty;
    std::memset(&empty, 0, sizeof(empty));
    apc_infos_[TOD_Layer(0)].set_data(empty);
    return 0;
}

// IDA_EA: 0x85FEEE4
// IDA_NAME: _ZN22TowerOfDespair_APC_Mgr13Send_APC_InfoERK9TOD_LayerP5CUser
// DEMANGLED: TowerOfDespair_APC_Mgr::Send_APC_Info(TOD_Layer const&,CUser *)
void TowerOfDespair_APC_Mgr::Send_APC_Info(const TOD_Layer& layer,
                                           CUser* user) {
    std::map<TOD_Layer, SendingAPC_Info>::iterator found =
        apc_infos_.find(layer);
    if (found == apc_infos_.end()) {
        cMyTrace trace(
            "void TowerOfDespair_APC_Mgr::Send_APC_Info(const TOD_Layer&, CUser*)",
            161, 5);
        trace("TOD:invalid layer %d", layer.GetLayer());
        return;
    }
    found->second.Send_APC_Info(layer, user);
}

// IDA_EA: 0x85FF020
// IDA_NAME: _ZN22TowerOfDespair_APC_Mgr20Get_APC_Name_byIndexERK9TOD_Layer
// DEMANGLED: TowerOfDespair_APC_Mgr::Get_APC_Name_byIndex(TOD_Layer const&)
const char* TowerOfDespair_APC_Mgr::Get_APC_Name_byIndex(
    const TOD_Layer& layer) const {
    std::map<TOD_Layer, SendingAPC_Info>::const_iterator found =
        apc_infos_.find(layer);
    return found == apc_infos_.end() ? 0 : found->second.get_user_charac_name();
}
