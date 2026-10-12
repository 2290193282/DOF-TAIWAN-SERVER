#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_APC_MGR_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_APC_MGR_RUNTIME_H_

#include <map>

#include "old_game/dungeon/instance/sending_apc_info_runtime.h"
#include "old_game/dungeon/instance/tod_layer.h"

class CUser;

class TowerOfDespair_APC_Mgr {
public:
    // IDA_EA: 0x86444E6
    // IDA_NAME: _ZN22TowerOfDespair_APC_MgrC2Ev
    // DEMANGLED: TowerOfDespair_APC_Mgr::TowerOfDespair_APC_Mgr(void)
    TowerOfDespair_APC_Mgr();

    // IDA_EA: 0x86444FA
    // IDA_NAME: _ZN22TowerOfDespair_APC_MgrD2Ev
    // DEMANGLED: TowerOfDespair_APC_Mgr::~TowerOfDespair_APC_Mgr()
    ~TowerOfDespair_APC_Mgr();

    // IDA_EA: 0x85FED2E / 0x85FEEE4 / 0x85FF020.
    void Set_APC_Info(const TOD_Layer& layer, const _APCInfo& info);
    int Reset_APC_Info();
    void Send_APC_Info(const TOD_Layer& layer, CUser* user);
    const char* Get_APC_Name_byIndex(const TOD_Layer& layer) const;

private:
    std::map<TOD_Layer, SendingAPC_Info> apc_infos_;  // IDA +0x00
};

#if defined(__i386__)
static_assert(sizeof(TowerOfDespair_APC_Mgr) == 0x18u,
              "TowerOfDespair_APC_Mgr old-server object must stay 0x18");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_APC_MGR_RUNTIME_H_
