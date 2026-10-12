#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_ROOM_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_ROOM_H_

#include <stddef.h>
#include <map>
#include <stdint.h>

#include "old_game/pvp/battlefield/war_field_runtime.h"

class PacketGuard;
class CUser;
enum TIMER_MESSAGE : int;
enum WARROOM_STATE : int;
enum ENUM_SEAT_STATE : int;
enum ENUM_WALKOUT_TYPE : int;

// IDA unit: 09_dungeon_pvp_party/WarRoom.  The room is pool-backed, so C owns
// the member ABI without guessing a standalone storage layout.
class WarRoom {
public:
    // IDA_EA: 0x86BA75C
    // IDA_NAME: _ZN7WarRoomC2Ev
    WarRoom();
    // IDA_EA: 0x86BA7E6
    // IDA_NAME: _ZN7WarRoomD2Ev
    ~WarRoom();
    // IDA_EA: 0x822EE8A
    // IDA_NAME: _ZN7WarRoom8GetIndexEv
    // DEMANGLED: WarRoom::GetIndex(void)
    int GetIndex();

    // IDA_EA: 0x822EE96
    // IDA_NAME: _ZN7WarRoom8GetStateEv
    // DEMANGLED: WarRoom::GetState(void)
    int GetState();

    // IDA_EA: 0x82A5A56
    // IDA_NAME: _ZN7WarRoom8SetStateE13WARROOM_STATE
    // DEMANGLED: WarRoom::SetState(WARROOM_STATE)
    WarRoom* SetState(WARROOM_STATE state);

    // IDA_EA: 0x863BD92
    // IDA_NAME: _ZN7WarRoom13CheckTimerKeyE13TIMER_MESSAGEi
    // DEMANGLED: WarRoom::CheckTimerKey(TIMER_MESSAGE,int)
    bool CheckTimerKey(TIMER_MESSAGE message, int key);

    // IDA_EA: 0x86BABCC
    // IDA_NAME: _ZNK7WarRoom14GetWaiterCountEv
    // DEMANGLED: WarRoom::GetWaiterCount(void)const
    int GetWaiterCount() const;

    // IDA_EA: 0x86BE0CC
    // IDA_NAME: _ZN7WarRoom10SendToRoomER11PacketGuard
    // DEMANGLED: WarRoom::SendToRoom(PacketGuard &)
    bool SendToRoom(PacketGuard& packet);

    // IDA_EA: 0x86BA834
    // IDA_NAME: _ZN7WarRoom8SetIndexEi
    // DEMANGLED: WarRoom::SetIndex(int)
    int SetIndex(int index);

    // IDA_EA: 0x86BA850
    // IDA_NAME: _ZN7WarRoom13UpdateBanListEv
    // DEMANGLED: WarRoom::UpdateBanList(void)
    bool UpdateBanList();

    // IDA_EA: 0x86BA948
    // IDA_NAME: _ZN7WarRoom10AddBanListEP5CUser
    // DEMANGLED: WarRoom::AddBanList(CUser *)
    int AddBanList(CUser* user);

    // IDA_EA: 0x86BA99E
    // IDA_NAME: _ZN7WarRoom9IsBanUserEP5CUser
    // DEMANGLED: WarRoom::IsBanUser(CUser *)
    bool IsBanUser(CUser* user);

    // IDA_EA: 0x86BABAE
    // IDA_NAME: _ZNK7WarRoom13GetRemainSeatEv
    // DEMANGLED: WarRoom::GetRemainSeat(void)const
    int GetRemainSeat() const;

    // IDA_EA: 0x86BAC0C
    // IDA_NAME: _ZNK7WarRoom12GetLiveCountEv
    // DEMANGLED: WarRoom::GetLiveCount(void)const
    int GetLiveCount() const;

    // IDA_EA: 0x86BAC5E
    // IDA_NAME: _ZN7WarRoom6CreateEP5CUserPcs
    // DEMANGLED: WarRoom::Create(CUser *,char *,short)
    int Create(CUser* manager, char* room_name, short dungeon_index);

    // IDA_EA: 0x86BACAE / 0x86BAE9A.  These are CMD011's war-room accept
    // path: first validate the candidate, then populate a concrete seat and
    // send the existing/member peer information frames.
    int IsJoinable(CUser* user);
    int Join(CUser* user, int& slot);

    // IDA_EA: 0x86BB53C
    // IDA_NAME: _ZN7WarRoom11ChangeStateE13WARROOM_STATE
    // DEMANGLED: WarRoom::ChangeState(WARROOM_STATE)
    int ChangeState(WARROOM_STATE state);

    // IDA_EA: 0x86BB582
    // IDA_NAME: _ZN7WarRoom16InvalidUserCheckEv
    // DEMANGLED: WarRoom::InvalidUserCheck(void)
    bool InvalidUserCheck();

    // IDA_EA: 0x86BB672
    // IDA_NAME: _ZN7WarRoom15SendStatePacketEv
    // DEMANGLED: WarRoom::SendStatePacket(void)
    void SendStatePacket();

    // IDA_EA: 0x86BC460
    // IDA_NAME: _ZN7WarRoom9ReviveAllEv
    // DEMANGLED: WarRoom::ReviveAll(void)
    bool ReviveAll();

    // IDA_EA: 0x86BC4EC
    // IDA_NAME: _ZN7WarRoom13CheckFatuigueEv
    // DEMANGLED: WarRoom::CheckFatuigue(void)
    bool CheckFatuigue();

    // IDA_EA: 0x86BF324
    // IDA_NAME: _ZN7WarRoom16ReviveUserByCoinEP5CUsert
    int ReviveUserByCoin(CUser* user, uint16_t target_unique_id);

    // IDA_EA: 0x86BD3EC
    // IDA_NAME: _ZN7WarRoom10CheckStateEv
    // DEMANGLED: WarRoom::CheckState(void)
    int CheckState();

    // IDA_EA: 0x86BD0F8
    // IDA_NAME: _ZN7WarRoom12SetSlotStateEP5CUseri15ENUM_SEAT_STATEPS1_RbS4_S4_
    // DEMANGLED: WarRoom::SetSlotState(CUser *,int,ENUM_SEAT_STATE,CUser **,
    //                                  bool &,bool &,bool &)
    int SetSlotState(CUser* user,
                     int slot,
                     ENUM_SEAT_STATE state,
                     CUser** leaving_user,
                     bool& slot_info_changed,
                     bool& state_info_changed,
                     bool& room_reset);

    // IDA_EA: 0x86BD460
    // IDA_NAME: _ZN7WarRoom11WalkOutUserEP5CUser
    // DEMANGLED: WarRoom::WalkOutUser(CUser *)
    int WalkOutUser(CUser* user);

    // IDA_EA: 0x86BE1FE
    // IDA_NAME: _ZN7WarRoom17HandleMonsterKillEP5CUseriS1_
    int HandleMonsterKill(CUser* user,
                          int monster_unique_id,
                          CUser* target);

    // IDA_EA: 0x86BD49C
    // IDA_NAME: _ZN7WarRoom7DestroyEi
    // DEMANGLED: WarRoom::Destroy(int)
    CUser* Destroy(int preferred_slot);

    // IDA_EA: 0x86BD504
    // IDA_NAME: _ZN7WarRoom4OpenEv
    // DEMANGLED: WarRoom::Open(void)
    WarRoom* Open();

    // IDA_EA: 0x86BD520
    // IDA_NAME: _ZN7WarRoom5CloseEv
    // DEMANGLED: WarRoom::Close(void)
    WarRoom* Close();

    // IDA_EA: 0x86BD53C
    // IDA_NAME: _ZN7WarRoom8ShutDownEv
    // DEMANGLED: WarRoom::ShutDown(void)
    int ShutDown();

    // IDA_EA: 0x86BD5D4
    // IDA_NAME: _ZN7WarRoom7PrepareEv
    // DEMANGLED: WarRoom::Prepare(void)
    // The raw EAX after a failed LogManager call led the decompiler to infer
    // a pointer return.  The original pretty-function string proves void.
    void Prepare();

    // IDA_EA: 0x86BD6D4
    // IDA_NAME: _ZN7WarRoom5StartEv
    // DEMANGLED: WarRoom::Start(void)
    void Start();

    // IDA_EA: 0x86BD842
    // IDA_NAME: _ZN7WarRoom11GetUserSlotEP5CUser
    // DEMANGLED: WarRoom::GetUserSlot(CUser *)
    int GetUserSlot(CUser* user);

    // IDA_EA: 0x86BD87E
    // IDA_NAME: _ZN7WarRoom17HandleDieCharacerEP5CUseri
    // DEMANGLED: WarRoom::HandleDieCharacer(CUser *,int)
    int HandleDieCharacer(CUser* user, int killer_slot);

    // IDA_EA: 0x86BE11A
    // IDA_NAME: _ZN7WarRoom23SkipLastRankResultTimerEv
    // DEMANGLED: WarRoom::SkipLastRankResultTimer(void)
    void SkipLastRankResultTimer();

    // IDA_EA: 0x86BE120
    // IDA_NAME: _ZN7WarRoom14CancelAllTimerEv
    // DEMANGLED: WarRoom::CancelAllTimer(void)
    bool CancelAllTimer();

    // IDA_EA: 0x86C077E
    // IDA_NAME: _ZN7WarRoom17CurSpawnStepResetEv
    // DEMANGLED: WarRoom::CurSpawnStepReset(void)
    WarRoom* CurSpawnStepReset();

    // IDA_EA: 0x86C08A6
    // IDA_NAME: _ZN7WarRoom18HostChangeWarRoomEP5CUsercm
    // DEMANGLED: WarRoom::HostChangeWarRoom(CUser *,char,unsigned long)
    int HostChangeWarRoom(CUser* user, char seat, unsigned long value);

    // IDA_EA: 0x86C0574
    // IDA_NAME: _ZN7WarRoom18HandleWpPerMonsterEP5CUseriiS1_
    // DEMANGLED: WarRoom::HandleWpPerMonster(CUser *,int,int,CUser *)
    int HandleWpPerMonster(CUser* user,
                           int monster_index,
                           int monster_hp,
                           CUser* target_user);

    // IDA_EA: 0x86BEA2E
    // IDA_NAME: _ZN7WarRoom13HandleGetItemEP5CUseri
    // DEMANGLED: WarRoom::HandleGetItem(CUser *,int)
    int HandleGetItem(CUser* user, int unique_id);

    // IDA_EA: 0x86BF5FC
    // IDA_NAME: _ZN7WarRoom14HandleItemDropEP5CUseriicii
    // DEMANGLED: WarRoom::HandleItemDrop(CUser *,int,int,char,int,int)
    int HandleItemDrop(CUser* user,
                       int pos_x,
                       int pos_y,
                       char item_space,
                       int slot,
                       int count);

    // IDA_EA: 0x86C0964
    // IDA_NAME: _ZN7WarRoom15CheckHostChangeEv
    // DEMANGLED: WarRoom::CheckHostChange(void)
    void CheckHostChange();

    // IDA_EA: 0x86C0B24
    // IDA_NAME: _ZN7WarRoom11GenTimerKeyE13TIMER_MESSAGE
    // DEMANGLED: WarRoom::GenTimerKey(TIMER_MESSAGE)
    int GenTimerKey(TIMER_MESSAGE message);

    // Direct boundaries used by the restored room transitions.  Their bodies
    // remain with their own IDA functions rather than being inlined here.
    bool Reset();
    int BattleReset();
    char OnLeaveState(WARROOM_STATE state);
    // IDA_EA: 0x86BB71C
    // IDA_NAME: _ZN7WarRoom12OnEnterStateEv
    // DEMANGLED: WarRoom::OnEnterState(void)
    void OnEnterState();
    void SetCharacterLive(int slot,
                          CUser* user,
                          bool live,
                          bool by_coin,
                          short position,
                          int direction);
    int WalkOutUserBySlot(int slot);
    // IDA_EA: 0x86BDEA0
    // IDA_NAME: _ZN7WarRoom12MakeSlotInfoEPci
    // DEMANGLED: WarRoom::MakeSlotInfo(char *,int)
    int MakeSlotInfo(char* packet, int slot);

    // IDA_EA: 0x86BDD58
    // IDA_NAME: _ZN7WarRoom12MakeRoomInfoEPc
    // DEMANGLED: WarRoom::MakeRoomInfo(char *)
    bool MakeRoomInfo(char* packet);

    // IDA_EA: 0x86BE07A
    // IDA_NAME: _ZN7WarRoom13MakeStateInfoEPc
    // DEMANGLED: WarRoom::MakeStateInfo(char *)
    // The original exported ABI deliberately takes the packet object's
    // InterfacePacketBuf address as char*.  Callers normally pass
    // reinterpret_cast<char*>(&PacketGuard).
    int MakeStateInfo(char* packet);
    int SendWalkOut(CUser* user, ENUM_WALKOUT_TYPE type);

    // IDA_EA: 0x86BF484
    // IDA_NAME: _ZN7WarRoom18HandleSetSeatStateEP5CUseri15ENUM_SEAT_STATE
    // DEMANGLED: WarRoom::HandleSetSeatState(CUser *,int,ENUM_SEAT_STATE)
    void HandleSetSeatState(CUser* user, int slot, ENUM_SEAT_STATE state);

    // IDA_EA: 0x86BC0B0
    // IDA_NAME: _ZN7WarRoom11SendGuildFPEv
    // DEMANGLED: WarRoom::SendGuildFP(void)
    void SendGuildFP();

    // IDA_EA: 0x86BC5FE
    // IDA_NAME: _ZN7WarRoom11ClearRewardEv
    // DEMANGLED: WarRoom::ClearReward(void)
    bool ClearReward();

    // IDA_EA: 0x86C0404
    // IDA_NAME: _ZN7WarRoom22HandleTimerResultCountEv
    // DEMANGLED: WarRoom::HandleTimerResultCount(void)
    int HandleTimerResultCount();

    // IDA_EA: 0x86C0512
    // IDA_NAME: _ZN7WarRoom19HandleTimerTimeBombEv
    // DEMANGLED: WarRoom::HandleTimerTimeBomb(void)
    int HandleTimerTimeBomb();

    // IDA_EA: 0x86C078E
    // IDA_NAME: _ZN7WarRoom18SendWarRoomFailMsgEv
    // DEMANGLED: WarRoom::SendWarRoomFailMsg(void)
    int SendWarRoomFailMsg();

    // IDA_EA: 0x86C07EE
    // IDA_NAME: _ZN7WarRoom11WarRoomFailEv
    // DEMANGLED: WarRoom::WarRoomFail(void)
    char WarRoomFail();

    // IDA_EA: 0x86BD094
    // IDA_NAME: _ZN7WarRoom8RemoveKCEv
    // DEMANGLED: WarRoom::RemoveKC(void)
    bool RemoveKC();

    // IDA_EA: 0x86BBEA6
    // IDA_NAME: _ZN7WarRoom11CalcGuildFPEv
    // DEMANGLED: WarRoom::CalcGuildFP(void)
    bool CalcGuildFP();

public:
    // C2 does not initialize these scalar members.  Their offsets come from
    // 0x86BAA14/0x86BAA8E and the WarRoom state-machine bodies; no byte-view
    // storage is used for the physical room prefix.
    uint8_t room_mode_;                           // +0x000
    uint8_t room_level_;                          // +0x001
    int32_t room_index_;                          // +0x004
    uint8_t room_flag_008_;                       // +0x008
    int32_t current_spawn_step_;                  // +0x00c
    uint8_t result_phase_active_;                 // +0x010
    uint8_t result_phase_sent_;                   // +0x011
    uint8_t battle_finished_;                     // +0x012
    // 0x86C08A6 stores a submitted host-change value per WarField seat.
    // 0xffffffff is the original "not submitted" sentinel.
    uint32_t host_change_values_[6];              // +0x014
    int32_t host_change_request_count_;           // +0x02c
    uint8_t shutdown_pending_;                    // +0x030
    WarField war_field_;                         // +0x034
    std::map<uint32_t, int> ban_list_;           // +0x1d8
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(offsetof(WarRoom, room_mode_) == 0x000u,
              "WarRoom mode must stay at +0x000");
static_assert(offsetof(WarRoom, room_level_) == 0x001u,
              "WarRoom level must stay at +0x001");
static_assert(offsetof(WarRoom, room_index_) == 0x004u,
              "WarRoom index must stay at +0x004");
static_assert(offsetof(WarRoom, room_flag_008_) == 0x008u,
              "WarRoom room flag must stay at +0x008");
static_assert(offsetof(WarRoom, current_spawn_step_) == 0x00cu,
              "WarRoom spawn step must stay at +0x00c");
static_assert(offsetof(WarRoom, result_phase_active_) == 0x010u,
              "WarRoom result-active flag must stay at +0x010");
static_assert(offsetof(WarRoom, result_phase_sent_) == 0x011u,
              "WarRoom result-sent flag must stay at +0x011");
static_assert(offsetof(WarRoom, battle_finished_) == 0x012u,
              "WarRoom battle-finished flag must stay at +0x012");
static_assert(offsetof(WarRoom, host_change_values_) == 0x014u,
              "WarRoom host-change values must stay at +0x014");
static_assert(offsetof(WarRoom, host_change_request_count_) == 0x02cu,
              "WarRoom host-change count must stay at +0x02c");
static_assert(offsetof(WarRoom, shutdown_pending_) == 0x030u,
              "WarRoom shutdown flag must stay at +0x030");
static_assert(offsetof(WarRoom, war_field_) == 0x34u,
              "WarRoom WarField must stay at +0x34");
static_assert(offsetof(WarRoom, ban_list_) == 0x1d8u,
              "WarRoom ban map must stay at +0x1d8");
static_assert(sizeof(WarRoom) == 0x1f0u,
              "WarRoom IDA ABI size must stay 0x1f0");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_WAR_ROOM_H_
