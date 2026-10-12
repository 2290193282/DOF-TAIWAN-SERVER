#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_MGR_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_MGR_RUNTIME_H_

#include <stdint.h>

#include <set>
#include <vector>

class CDungeon;
class CUser;
class TowerOfDespair_APC_Mgr;
struct _APCInfo;
struct TOD_Layer;

// Canonical old Tower-of-Despair manager boundary.  CDataManager::init
// allocates this exact 0x35C-byte i386 object before loading
// Etc/towerofdespair.etc.
class TowerOfDespairMgr {
public:
    // IDA_EA: 0x8643AC4
    // IDA_NAME: _ZN17TowerOfDespairMgrC2Ev
    // DEMANGLED: TowerOfDespairMgr::TowerOfDespairMgr(void)
    TowerOfDespairMgr();

    // IDA_EA: 0x8643BE2
    // IDA_NAME: _ZN17TowerOfDespairMgrD2Ev
    // DEMANGLED: TowerOfDespairMgr::~TowerOfDespairMgr()
    ~TowerOfDespairMgr();

    // IDA_EA: 0x8643A1E
    // IDA_NAME: _ZN17TowerOfDespairMgr8GetMinLvEv
    // DEMANGLED: TowerOfDespairMgr::GetMinLv(void)
    int GetMinLv();

    // IDA_EA: 0x8643E7E
    // IDA_NAME: _ZN17TowerOfDespairMgr10LoadScriptEPKcS1_
    // DEMANGLED: TowerOfDespairMgr::LoadScript(char const*,char const*)
    int LoadScript(const char* directory, const char* filename);

    // IDA_EA: 0x8643D14..0x8643E4A.  These are the runtime layer/APC
    // lookups used by CParty's normal CMD16 Tower-of-Despair redirect.
    const CDungeon* Get_CDungeonByLayer(unsigned int layer);
    int Get_DungeonIndexByLayer(unsigned int layer) const;
    int Get_APCIndexByLayer(unsigned int layer) const;
    const char* Get_APCNameByLayer(const TOD_Layer& layer) const;
    int GetBaseLayerIndex();
    int CheckEnterable(CUser* user, const CDungeon* dungeon);
    bool IsTowerOfDespairDungeon_ByIndex(int dungeon_index) const;

    // IDA_EA: 0x86442CA..0x8644366.  DB-side APC refreshes populate this
    // manager; CMD16 and tower packets only consume it.
    void SetAPCInfo(unsigned int apc_index, const _APCInfo& info);
    int ResetAPCInfo();
    void SendAPCInfo(const TOD_Layer& layer, CUser* user);
    static void SendMessageToCUser(CUser* user, char* message);

    // IDA_EA: 0x8644360
    // IDA_NAME: _ZN17TowerOfDespairMgr15Reload_APC_InfoEv
    // DEMANGLED: TowerOfDespairMgr::Reload_APC_Info(void)
    void Reload_APC_Info();

    // IDA_EA: 0x8644470
    // IDA_NAME: _ZN17TowerOfDespairMgr10_layerInfoEP5CUser
    // DEMANGLED: TowerOfDespairMgr::_layerInfo(CUser *)
    void _layerInfo(CUser* user);

private:
    // IDA_EA: 0x8643C6C
    // IDA_NAME: _ZN17TowerOfDespairMgr24_Set_LayerByDungeonIndexEjPK8CDungeon
    // DEMANGLED: TowerOfDespairMgr::_Set_LayerByDungeonIndex(unsigned int,CDungeon const*)
    void _Set_LayerByDungeonIndex(unsigned int layer,
                                  const CDungeon* dungeon);

    // IDA_EA: 0x8643C86
    // IDA_NAME: _ZN17TowerOfDespairMgr24_Set_LayerByDungeonIndexEjj
    // DEMANGLED: TowerOfDespairMgr::_Set_LayerByDungeonIndex(unsigned int,unsigned int)
    void _Set_LayerByDungeonIndex(unsigned int layer,
                                  unsigned int dungeon_index);

    // IDA_EA: 0x8643CF4
    // IDA_NAME: _ZN17TowerOfDespairMgr20_Set_APCIndexByLayerEj
    // DEMANGLED: TowerOfDespairMgr::_Set_APCIndexByLayer(unsigned int)
    void _Set_APCIndexByLayer(unsigned int layer);

    const CDungeon* dungeons_by_layer_[101];       // IDA +0x000
    unsigned int dungeon_indices_by_layer_[101];  // IDA +0x194
    std::set<int> dungeon_indices_;                // IDA +0x328
    std::vector<unsigned int> apc_layers_;         // IDA +0x340
    uint16_t enterable_count_;                     // IDA +0x34C
    uint16_t min_level_;                           // IDA +0x34E
    uint16_t top_layer_;                           // IDA +0x350
    uint16_t ida_align_352_;                       // IDA +0x352
    TowerOfDespair_APC_Mgr* apc_manager_;          // IDA +0x354
    unsigned int base_layer_index_;                // IDA +0x358
};

#if defined(__i386__)
static_assert(sizeof(TowerOfDespairMgr) == 0x35Cu,
              "TowerOfDespairMgr IDA allocation must stay 0x35C");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_TOWER_OF_DESPAIR_MGR_RUNTIME_H_
