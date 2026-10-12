#ifndef OLDSRV_OLD_GAME_WORLD_VILLAGE_ATTACKED_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_VILLAGE_ATTACKED_RUNTIME_H_

#include "old_game/world/region/world_area_types.h"
#include "old_game/world/region/world_value_types.h"
#include "old_data/PVF/map/map_script_types.h"

#include <map>
#include <stdint.h>
#include <vector>

class CUser;
class PacketGuard;

// Both original member signatures place this enum in global scope.  Its
// values are event-script data and are intentionally not invented here.
enum ENUM_VILLAGE_ATTACKED_REWARD : int;

namespace village_attacked {

// The original named enum is required in the member ABI of
// CVillageMonster::OnChangeState.  Its values are script/runtime data and are
// deliberately not invented here.
enum ENUM_VILLAGE_MONSTER_STATE : int;

class CVillageMonsterArea;

class CVillageMonster {
public:
    // IDA_EA: 0x86B3004
    // IDA_NAME: _ZN16village_attacked15CVillageMonsterC2EtiiiPNS_19CVillageMonsterAreaEii
    // DEMANGLED: village_attacked::CVillageMonster::CVillageMonster(unsigned short,int,int,int,village_attacked::CVillageMonsterArea *,int,int)
    CVillageMonster(uint16_t monster_id,
                    int pos_x,
                    int pos_y,
                    int route,
                    CVillageMonsterArea* area,
                    int dungeon_index,
                    int dungeon_route);

    // IDA_EA: 0x86B3098
    // IDA_NAME: _ZN16village_attacked15CVillageMonsterC2Ev
    // DEMANGLED: village_attacked::CVillageMonster::CVillageMonster(void)
    CVillageMonster();

    // IDA_EA: 0x86B3106
    // IDA_NAME: _ZN16village_attacked15CVillageMonsterD2Ev
    // DEMANGLED: village_attacked::CVillageMonster::~CVillageMonster()
    ~CVillageMonster();

    // IDA_EA: 0x86B310C
    // IDA_NAME: _ZN16village_attacked15CVillageMonster19OnRunVillageMonsterEPSt6vectorI7MapAreaSaIS2_EEi
    // DEMANGLED: village_attacked::CVillageMonster::OnRunVillageMonster(std::vector<MapArea,std::allocator<MapArea>> *,int)
    int OnRunVillageMonster(std::vector<MapArea>* map_areas, int tick);

    uint16_t uid;
    uint16_t monster_id;
    int pos_x;
    int pos_y;
    int route;
    unsigned char position_dirty;
    unsigned char raw_11[3];
    int state;
    int party_index;
    CVillageMonsterArea* area;
    int dungeon_index;
    int dungeon_route;

    // IDA_EA: 0x86B3666
    // IDA_NAME: _ZN16village_attacked15CVillageMonster28MakeVillageMonsterListPacketER11PacketGuard
    // DEMANGLED: village_attacked::CVillageMonster::MakeVillageMonsterListPacket(PacketGuard &)
    int MakeVillageMonsterListPacket(PacketGuard& packet);

    // IDA_EA: 0x86B36E6
    // IDA_NAME: _ZN16village_attacked15CVillageMonster32MakeVillageMonsterPositionPacketER11PacketGuard
    // DEMANGLED: village_attacked::CVillageMonster::MakeVillageMonsterPositionPacket(PacketGuard &)
    int MakeVillageMonsterPositionPacket(PacketGuard& packet);

    // IDA_EA: 0x86B4B64
    // IDA_NAME: _ZNK16village_attacked15CVillageMonster6GetUIDEv
    // DEMANGLED: village_attacked::CVillageMonster::GetUID(void)const
    int GetUID() const;

    // IDA_EA: 0x86B3240
    // IDA_NAME: _ZN16village_attacked15CVillageMonster21OnFightVillageMonsterEP5CUserRiS3_
    // DEMANGLED: village_attacked::CVillageMonster::OnFightVillageMonster(CUser *,int &,int &)
    int OnFightVillageMonster(CUser* user,
                              int& dungeon_index,
                              int& dungeon_route);

    // IDA_EA: 0x86B330A
    // IDA_NAME: _ZN16village_attacked15CVillageMonster29SendVillageMonsterFightResultEP5CUserb
    // DEMANGLED: village_attacked::CVillageMonster::SendVillageMonsterFightResult(CUser *,bool)
    void SendVillageMonsterFightResult(CUser* user, bool clear);

    // IDA_EA: 0x86B34A0
    // IDA_NAME: _ZN16village_attacked15CVillageMonster20OnKillVillageMonsterEP5CUseriiib
    // DEMANGLED: village_attacked::CVillageMonster::OnKillVillageMonster(CUser *,int,int,int,bool)
    int OnKillVillageMonster(CUser* user,
                             int village,
                             int area,
                             int regen_seconds,
                             bool clear);

    // IDA_EA: 0x86B35A0
    // IDA_NAME: _ZN16village_attacked15CVillageMonster13OnChangeStateENS_26ENUM_VILLAGE_MONSTER_STATEE
    // DEMANGLED: village_attacked::CVillageMonster::OnChangeState(village_attacked::ENUM_VILLAGE_MONSTER_STATE)
    void OnChangeState(ENUM_VILLAGE_MONSTER_STATE state);
};

class CVillageMonsterArea {
public:
    // IDA_EA: 0x86B3752
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaC2EiiiiPSt6vectorI17STAttackedMonsterSaIS2_EEPS1_I7MapAreaSaIS6_EE
    // DEMANGLED: village_attacked::CVillageMonsterArea::CVillageMonsterArea(int,int,int,int,std::vector<STAttackedMonster,std::allocator<STAttackedMonster>> *,std::vector<MapArea,std::allocator<MapArea>> *)
    CVillageMonsterArea(int village,
                        int area,
                        int max_count,
                        int regen_seconds,
                        std::vector<STAttackedMonster>* attacked_monsters,
                        std::vector<MapArea>* map_areas);

    // IDA_EA: 0x86B4C44
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaD2Ev
    // DEMANGLED: village_attacked::CVillageMonsterArea::~CVillageMonsterArea()
    ~CVillageMonsterArea();

    // IDA_EA: 0x86B52F0
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterAreaC2ERKS0_
    // DEMANGLED: village_attacked::CVillageMonsterArea::CVillageMonsterArea(village_attacked::CVillageMonsterArea const&)
    CVillageMonsterArea(const CVillageMonsterArea& rhs);

    std::map<uint16_t, CVillageMonster> monsters;
    std::vector<MapArea>* map_area_list;
    std::vector<STAttackedMonster> attacked_monsters;
    int village;
    int area;
    int raw_30;
    int max_count;
    int regen_seconds;

    // IDA_EA: 0x86B37FA
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea22SendVillageMonsterListEP5CUser
    // DEMANGLED: village_attacked::CVillageMonsterArea::SendVillageMonsterList(CUser *)
    void SendVillageMonsterList(CUser* user);

    // IDA_EA: 0x86B3962
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea26SendVillageMonsterPositionEv
    // DEMANGLED: village_attacked::CVillageMonsterArea::SendVillageMonsterPosition(void)
    void SendVillageMonsterPosition();

    // IDA_EA: 0x86B3AEA
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea18GetAttackedMonsterEi
    // DEMANGLED: village_attacked::CVillageMonsterArea::GetAttackedMonster(int)
    STAttackedMonster* GetAttackedMonster(int random_value);

    // IDA_EA: 0x86B3BD4
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea22OnCreateVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnCreateVillageMonster(void)
    int OnCreateVillageMonster();

    // IDA_EA: 0x86B3EE0
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea23OnDestroyVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnDestroyVillageMonster(void)
    void OnDestroyVillageMonster();

    // IDA_EA: 0x86B3F06
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea19OnRunVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnRunVillageMonster(void)
    char OnRunVillageMonster();

    // IDA_EA: 0x86B3ABE
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea10OnMoveAreaEP5CUser
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnMoveArea(CUser *)
    void OnMoveArea(CUser* user);

    // IDA_EA: 0x86B3FE2
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea21OnFightVillageMonsterEP5CUsertRiS3_
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnFightVillageMonster(CUser *,unsigned short,int &,int &)
    int OnFightVillageMonster(CUser* user,
                              uint16_t monster_uid,
                              int& dungeon_index,
                              int& dungeon_route);

    // IDA_EA: 0x86B4070
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea20OnKillVillageMonsterEP5CUsertb
    // DEMANGLED: village_attacked::CVillageMonsterArea::OnKillVillageMonster(CUser *,unsigned short,bool)
    int OnKillVillageMonster(CUser* user,
                             uint16_t monster_uid,
                             bool clear);

    // IDA_EA: 0x86B3B9C
    // IDA_NAME: _ZN16village_attacked19CVillageMonsterArea10SendPacketER11PacketGuard
    // DEMANGLED: village_attacked::CVillageMonsterArea::SendPacket(PacketGuard &)
    int SendPacket(PacketGuard& packet);
};

class CVillageMonsterMgr {
public:
    // IDA_EA: 0x86B4A4C
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgrC2Ev
    // DEMANGLED: village_attacked::CVillageMonsterMgr::CVillageMonsterMgr(void)
    CVillageMonsterMgr();

    // IDA_EA: 0x86B4A80
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgrD2Ev
    // DEMANGLED: village_attacked::CVillageMonsterMgr::~CVillageMonsterMgr()
    ~CVillageMonsterMgr();

    int raw_0;
    int event_state;
    std::map<Zone, CVillageMonsterArea> areas;
    int reward_type;

    // IDA_EA: 0x82F0F44
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr7OnEventEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnEvent(void)
    bool OnEvent();

    // IDA_EA: 0x847DD44
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr13SetRewardTypeE28ENUM_VILLAGE_ATTACKED_REWARD
    // DEMANGLED: village_attacked::CVillageMonsterMgr::SetRewardType(ENUM_VILLAGE_ATTACKED_REWARD)
    CVillageMonsterMgr* SetRewardType(ENUM_VILLAGE_ATTACKED_REWARD reward);

    // IDA_EA: 0x84ED3F8
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr13GetRewardTypeEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::GetRewardType(void)
    int GetRewardType();

    // IDA_EA: 0x86B4144
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20InsertVillageMonsterEiiiiPSt6vectorI17STAttackedMonsterSaIS2_EEPS1_I7MapAreaSaIS6_EE
    // DEMANGLED: village_attacked::CVillageMonsterMgr::InsertVillageMonster(int,int,int,int,std::vector<STAttackedMonster,std::allocator<STAttackedMonster>> *,std::vector<MapArea,std::allocator<MapArea>> *)
    void InsertVillageMonster(int village,
                              int area,
                              int max_count,
                              int regen_seconds,
                              std::vector<STAttackedMonster>* attacked_monsters,
                              std::vector<MapArea>* map_areas);

    // IDA_EA: 0x86B4280
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr22OnCreateVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnCreateVillageMonster(void)
    int OnCreateVillageMonster();

    // IDA_EA: 0x86B4338
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr21OnRegenVillageMonsterEii
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnRegenVillageMonster(int,int)
    int OnRegenVillageMonster(int village, int area);

    // IDA_EA: 0x86B43D4
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr23OnDestroyVillageMonsterE28ENUM_VILLAGE_ATTACKED_REWARD
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnDestroyVillageMonster(ENUM_VILLAGE_ATTACKED_REWARD)
    void OnDestroyVillageMonster(ENUM_VILLAGE_ATTACKED_REWARD reward);

    // IDA_EA: 0x86B44EC
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr25OnEndRewardVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnEndRewardVillageMonster(void)
    CVillageMonsterMgr* OnEndRewardVillageMonster();

    // IDA_EA: 0x86B44FC
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr19OnRunVillageMonsterEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnRunVillageMonster(void)
    int OnRunVillageMonster();

    // IDA_EA: 0x86B45BC
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr10OnMoveAreaEP5CUser
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnMoveArea(CUser *)
    void OnMoveArea(CUser* user);

    // IDA_EA: 0x86B4672
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr21OnFightVillageMonsterEP5CUsert
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnFightVillageMonster(CUser *,unsigned short)
    int OnFightVillageMonster(CUser* user, uint16_t monster_uid);

    // IDA_EA: 0x86B4866
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20OnKillVillageMonsterEP5CUserb
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnKillVillageMonster(CUser *,bool)
    int OnKillVillageMonster(CUser* user, bool clear);

    // IDA_EA: 0x86B4964
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr20OnKillVillageMonsterEiitb
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnKillVillageMonster(int,int,unsigned short,bool)
    int OnKillVillageMonster(int village,
                             int area,
                             uint16_t monster_uid,
                             bool clear);

    // IDA_EA: 0x86B4A20
    // IDA_NAME: _ZN16village_attacked18CVillageMonsterMgr22OnStartVillageAttackedEv
    // DEMANGLED: village_attacked::CVillageMonsterMgr::OnStartVillageAttacked(void)
    CVillageMonsterMgr* OnStartVillageAttacked();
};

}  // namespace village_attacked

#endif  // OLDSRV_OLD_GAME_WORLD_VILLAGE_ATTACKED_RUNTIME_H_
