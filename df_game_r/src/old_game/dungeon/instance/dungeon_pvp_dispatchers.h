#ifndef OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_DISPATCHERS_H_
#define OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_DISPATCHERS_H_

#include "old_data/db/event/db_pcroom_daily_reward.h"

#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/packet_dispatcher_call_party_member_realtime_info.h"
#include "old_game/social/party/packet_dispatcher_gathering_party_status.h"
#include "old_game/pvp/season/packet_dispatcher_res_pvp_rank.h"
#include "old_game/pvp/battlefield/packet_dispatcher_pvp_buddy.h"
#include "old_game/social/party/packet_dispatcher_set_party_info.h"
#include "old_game/dungeon/room/packet_dispatcher_pc_room_rent.h"
#include "old_game/dungeon/room/packet_dispatcher_seria_room_deco.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/pvp/battlefield/dispatcher_fair_pvp_score.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;
class Stream;

// The A-owned generic dispatcher translation unit provides these exact
// original specializations.  They must be declared before either concrete
// dispatcher derives from the primary template; otherwise this header
// implicitly instantiates the primary virtual members before their explicit
// specializations are seen.
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       PcRoomPlayTimeRewardParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

// The exact CMD435 generic template is A-owned, while this concrete C
// dispatcher owns its gameplay callbacks.  Keep all specializations declared
// before the derived class so no primary-template COMDAT can become a second
// vtable provider.
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT,
                  stPartyTeleport,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT,
                      stPartyTeleport,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT,
                       stPartyTeleport,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                  stPartyTeleportConfirm,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                      stPartyTeleportConfirm,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                       stPartyTeleportConfirm,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA unit: 09_dungeon_pvp_party/DisPatcher_LeaveParty.
// IDA refs: 0x81C424C
class DisPatcher_LeaveParty
    : public IPacketDispatcher<MSG_BASE,
                               stLeavePartyParam,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_LeaveParty();
    virtual ~DisPatcher_LeaveParty();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_LeaveParty) == 0x04u,
              "CMD13 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_PvPTimeout.
// IDA refs: 0x81C5D78
class DisPatcher_PvPTimeout
    : public IPacketDispatcher<MSG_PVP_TIMEOUT,
                               stOnPvPTimeOut,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_PvPTimeout();
    virtual ~DisPatcher_PvPTimeout();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_PvPTimeout) == 0x04u,
              "CMD59 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_SelectDungeon.  This is the
// original state-0 typed dispatcher; it intentionally remains unregistered
// while the frozen 606-entry root transaction is incomplete.
class DisPatcher_SelectDungeon
    : public IPacketDispatcher<MSG_SELECT_DUNGEON,
                               stSelectDungeonParam,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_SelectDungeon();
    virtual ~DisPatcher_SelectDungeon();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int checkBlueMarble(CParty* party);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SelectDungeon) == 0x04u,
              "CMD16 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_SetPVPReady.
// IDA refs: 0x81C99BE
class DisPatcher_SetPVPReady
    : public IPacketDispatcher<MSG_SET_READY_STATE,
                               stOnSetPvPReadyParam,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_SetPVPReady();
    virtual ~DisPatcher_SetPVPReady();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPVPReady) == 0x04u,
              "CMD56 dispatcher must retain its i386 base-only ABI");
#endif

template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_WALKOUT_PARTY,
                       stWalkoutPartyParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

// CMD445 uses the exact state-5 specialization restored in the A-domain
// protocol unit.  Declare every explicit member before the derived class so
// this C-domain header never instantiates a primary-template substitute.
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stQuickPartyParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stQuickPartyParam,
                      static_cast<ch_state>(5)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       stQuickPartyParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&);

// CMD445's MSG_QUICK_PARTY base is explicitly instantiated by the A protocol
// provider. Keep this declaration ahead of the C-owned derived dispatcher so
// this translation unit does not emit a second COMDAT/vtable family.
extern template class IPacketDispatcher<MSG_QUICK_PARTY,
                                        stQuickPartyParam,
                                        static_cast<ch_state>(5)>;

// CMD120's generic state-5 specialization is A-owned.  Keep all explicit
// members visible before the C-owned derived class so it cannot instantiate a
// primary-template vtable family.
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                  stBossDieCheckParam,
                  static_cast<ch_state>(5)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                      stBossDieCheckParam,
                      static_cast<ch_state>(5)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                       stBossDieCheckParam,
                       static_cast<ch_state>(5)>::send(CUser*, ParamBase&);

// IDA unit: 09_dungeon_pvp_party/Dispatcher_BossDieCheck.
// C2/D2/D0: 0x8596070 / 0x8245EC2 / 0x8245EF2; concrete callbacks:
// read 0x81CD642, check_error 0x81CD71E, process 0x81CD7A2, send 0x81CDA62.
class Dispatcher_BossDieCheck
    : public IPacketDispatcher<MSG_BOSS_DIE_CHECK,
                               stBossDieCheckParam,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_BossDieCheck();
    virtual ~Dispatcher_BossDieCheck();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BossDieCheck) == 0x04u,
              "CMD120 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_WalkoutParty.
// IDA refs: 0x81CAF42
class DisPatcher_WalkoutParty
    : public IPacketDispatcher<MSG_WALKOUT_PARTY,
                               stWalkoutPartyParam,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_WalkoutParty();
    virtual ~DisPatcher_WalkoutParty();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_WalkoutParty) == 0x04u,
              "CMD14 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_Register_Quick_Party.
// IDA refs: 0x81DDF36
class Dispatcher_Register_Quick_Party
    : public IPacketDispatcher<MSG_QUICK_PARTY,
                               stQuickPartyParam,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_Register_Quick_Party();
    virtual ~Dispatcher_Register_Quick_Party();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

// IDA unit: 09_dungeon_pvp_party/Dispatcher_Cancel_Quick_Party.
// IDA refs: 0x8597A0E, 0x8244614, 0x8244644, 0x81DE1AA, 0x81DE1B4,
//          0x81DE1F8, 0x81DE298.
class Dispatcher_Cancel_Quick_Party
    : public IPacketDispatcher<MSG_BASE,
                               stQuickPartyParam,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_Cancel_Quick_Party();
    virtual ~Dispatcher_Cancel_Quick_Party();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Cancel_Quick_Party) == 0x04u,
              "CMD445 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_Direct_Entrance_Quick_Party.
// IDA refs: 0x81DE32A
class Dispatcher_Direct_Entrance_Quick_Party
    : public IPacketDispatcher<MSG_BASE,
                               stQuickPartyParam,
                               static_cast<ch_state>(5)> {
public:
    Dispatcher_Direct_Entrance_Quick_Party();
    virtual ~Dispatcher_Direct_Entrance_Quick_Party();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

// IDA unit: 09_dungeon_pvp_party/Dispatcher_PcRoomPlayTimeReward.
// IDA refs: 0x81E4F52, 0x81E4F5C, 0x81E5020, 0x8243DDA, 0x8243E0A, 0x8597D72
class Dispatcher_PcRoomPlayTimeReward
    : public IPacketDispatcher<MSG_BASE,
                               PcRoomPlayTimeRewardParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_PcRoomPlayTimeReward();
    virtual ~Dispatcher_PcRoomPlayTimeReward();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PcRoomPlayTimeReward) == 0x04u,
              "CMD493 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/InterSelectPcroomDailyReward.
// IDA_EA: dispatch_sig/makeRequest/SendPacket @0x81616CE/@0x816179C/@0x81618A6.
struct InterSelectPcroomDailyReward
    : public TaiwanInterfaceInterDispatcher {
    // IDA_EA: 0x8162170
    // IDA_NAME: _ZN28InterSelectPcroomDailyRewardC2Ev
    // DEMANGLED: InterSelectPcroomDailyReward::InterSelectPcroomDailyReward(void)
    InterSelectPcroomDailyReward();

    // IDA_EA: 0x8162194
    // IDA_NAME: _ZN28InterSelectPcroomDailyRewardD2Ev
    // DEMANGLED: InterSelectPcroomDailyReward::~InterSelectPcroomDailyReward()
    // IDA_EA: 0x81621C4
    // IDA_NAME: _ZN28InterSelectPcroomDailyRewardD0Ev
    // DEMANGLED: InterSelectPcroomDailyReward::~InterSelectPcroomDailyReward()
    virtual ~InterSelectPcroomDailyReward();

    struct Msg {
        unsigned int account_id;
        char remain_reward_count;
        unsigned char reserved_05[3];
    };

    virtual int dispatch_sig(CUser* user, char* msg, int length);
    static void makeRequest(int uid, unsigned int account_id,
                            char remain_reward_count);
    static void SendPacket(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(InterSelectPcroomDailyReward) == 0x08u,
              "InterSelectPcroomDailyReward must retain the Taiwan ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Inter_LoadDungeonClear.
// IDA_EA: dispatch_sig/ctor/dtor @0x84C7C42/@0x84EA070/
// 0x84EA08C/@0x84EA0BC.
class Inter_LoadDungeonClear : public ISigDispatcher {
public:
    Inter_LoadDungeonClear();
    virtual ~Inter_LoadDungeonClear();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 09_dungeon_pvp_party/Inter_Check3rdPartyConcent.
// IDA_EA: dispatch_sig/ctor/dtor @0x84E6096/@0x84EECC4/
// 0x84F5E44/@0x84F5E74.
class Inter_Check3rdPartyConcent : public ISigDispatcher {
public:
    Inter_Check3rdPartyConcent();
    virtual ~Inter_Check3rdPartyConcent();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 09_dungeon_pvp_party/DisPatcher_DiePvP.
// Full dispatcher object: dispatch 0x81ECE64; read/process 0x8232B6A/
// 0x8232B74; D2/D0/C2 0x8243810/0x8243840/0x8595ADC.
class DisPatcher_DiePvP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_DiePvP();
    virtual ~DisPatcher_DiePvP();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DiePvP) == 0x04u,
              "CMD58 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_EnterPVP.
// IDA refs: 0x8232B7E
class DisPatcher_EnterPVP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_EnterPVP();
    virtual ~DisPatcher_EnterPVP();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_EnterPVP) == 0x04u,
              "CMD54 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_MakePVP.
// IDA refs: 0x8233214
class DisPatcher_MakePVP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_MakePVP();
    virtual ~DisPatcher_MakePVP();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int ExtractPacket(PacketBuf& packet, MSG_MAKE_PVP_ROOM& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_MakePVP) == 0x04u,
              "CMD53 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_EndPvPResult.
// The raw read provider is in dungeon_pvp_dispatchers.cpp; the remaining
// vtable members are restored in packet_dispatcher_derived_batch040.cpp.
// All five methods form the one C2 @ 0x8595660 state-0 dispatcher object.
class DisPatcher_EndPvPResult
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_EndPvPResult();
    virtual ~DisPatcher_EndPvPResult();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_EndPvPResult) == 0x04u,
              "CMD60 dispatcher must retain its i386 vptr-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_StartWarRoomTemp.
// IDA refs: 0x82335E4
class DisPatcher_StartWarRoomTemp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_StartWarRoomTemp();
    virtual ~DisPatcher_StartWarRoomTemp();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_StartWarRoomTemp) == 0x04u,
              "CMD96 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_MakeWarRoomTemp.
// The original class has only the state-0 packet-dispatcher base.  It is not
// installed in the frozen root PacketDispatcher map; its real direct handler
// body is retained for callers that construct it outside that map.
class DisPatcher_MakeWarRoomTemp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x82431AA / 0x82431DA
    // IDA_NAME: _ZN26DisPatcher_MakeWarRoomTempD2Ev / D0Ev
    virtual ~DisPatcher_MakeWarRoomTemp();

    // IDA_EA: 0x8201B32 / 0x82335F8 / 0x8233602
    // IDA_NAME: _ZN26DisPatcher_MakeWarRoomTemp12dispatch_sigEP5CUserR9PacketBuf /
    //           _ZN26DisPatcher_MakeWarRoomTemp4readER9PacketBufR8MSG_BASE /
    //           _ZN26DisPatcher_MakeWarRoomTemp7processEP5CUserR8MSG_BASER9ParamBase
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_MakeWarRoomTemp) == 0x04u,
              "MakeWarRoomTemp dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_PvpHeartBeat.
// IDA refs: 0x8233760
class DisPatcher_PvpHeartBeat
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_PvpHeartBeat();
    virtual ~DisPatcher_PvpHeartBeat();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_PvpHeartBeat) == 0x04u,
              "CMD113 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_CompleteLoadPvP.  The old C2
// is a base-only state-0 dispatcher; dispatch_sig forwards a loaded user to
// the room's live IMatch object at PvP_Room +0x6e4.
class Dispatcher_CompleteLoadPvP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85970D8
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPC2Ev
    // DEMANGLED: Dispatcher_CompleteLoadPvP::Dispatcher_CompleteLoadPvP(void)
    Dispatcher_CompleteLoadPvP();

    // IDA_EA: 0x8240666
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPD2Ev
    // DEMANGLED: Dispatcher_CompleteLoadPvP::~Dispatcher_CompleteLoadPvP()
    // IDA_EA: 0x8240696
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvPD0Ev
    // DEMANGLED: Dispatcher_CompleteLoadPvP::~Dispatcher_CompleteLoadPvP()
    virtual ~Dispatcher_CompleteLoadPvP();

    // IDA_EA: 0x82252FE
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CompleteLoadPvP::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823415E
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CompleteLoadPvP::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234168
    // IDA_NAME: _ZN26Dispatcher_CompleteLoadPvP7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CompleteLoadPvP::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CompleteLoadPvP) == 0x04u,
              "CMD299 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_ConnectP2PPvP.  The original
// object has only the state-0 IPacketDispatcher base: C2 @ 0x85970F4 calls
// that base constructor and then installs this class's vtable.
class Dispatcher_ConnectP2PPvP
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85970F4
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPC2Ev
    // DEMANGLED: Dispatcher_ConnectP2PPvP::Dispatcher_ConnectP2PPvP(void)
    Dispatcher_ConnectP2PPvP();

    // IDA_EA: 0x8240618
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPD2Ev
    // DEMANGLED: Dispatcher_ConnectP2PPvP::~Dispatcher_ConnectP2PPvP()
    // IDA_EA: 0x8240648
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvPD0Ev
    // DEMANGLED: Dispatcher_ConnectP2PPvP::~Dispatcher_ConnectP2PPvP()
    virtual ~Dispatcher_ConnectP2PPvP();

    // IDA_EA: 0x8225394
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_ConnectP2PPvP::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8234172
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ConnectP2PPvP::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x823417C
    // IDA_NAME: _ZN24Dispatcher_ConnectP2PPvP7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ConnectP2PPvP::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ConnectP2PPvP) == 0x04u,
              "CMD300 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_Change_PartyMemberPosition.
// IDA refs: 0x82342C6
class Dispatcher_Change_PartyMemberPosition
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Change_PartyMemberPosition();
    virtual ~Dispatcher_Change_PartyMemberPosition();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Change_PartyMemberPosition) == 0x04u,
              "CMD336 dispatcher must retain its i386 base-only ABI");
#endif

// The CMD377 generic instance is A-owned.  Declare all original members
// before the C-owned concrete vtable so this header never emits a primary
// template COMDAT provider.
template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                  stWarRoomWp,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                      stWarRoomWp,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                       stWarRoomWp,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA unit: 09_dungeon_pvp_party/Dispatcher_WarRoomWpPerMonster.
// IDA refs: 0x85976C6, 0x8244EEA, 0x8244F1A, 0x81D5BCC, 0x81D5CA8,
//           0x81D5DF2, 0x81D5E82.
class Dispatcher_WarRoomWpPerMonster
    : public IPacketDispatcher<MSG_WARROOM_WP_PER_MONSTER,
                               stWarRoomWp,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_WarRoomWpPerMonster();
    virtual ~Dispatcher_WarRoomWpPerMonster();

    int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_WarRoomWpPerMonster) == 0x04u,
              "CMD377 dispatcher must retain its i386 base-only ABI");
#endif

// The A-owned CMD404 template family must be known before the concrete C
// derived class is declared, otherwise this header can instantiate an
// accidental primary-template vtable provider.
template <>
IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                  stChangeHostWarRoom,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                  stChangeHostWarRoom,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                      stChangeHostWarRoom,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                      stChangeHostWarRoom,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                      stChangeHostWarRoom,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                       stChangeHostWarRoom,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// IDA unit: 09_dungeon_pvp_party/Dispatcher_ChangeHostWarRoom.
// IDA refs: 0x859778A, 0x8244D16, 0x8244D46, 0x81D735E, 0x81D73F6,
//           0x81D74D4, 0x81D75B0.
class Dispatcher_ChangeHostWarRoom
    : public IPacketDispatcher<MSG_CHANGE_HOST_WARROOM,
                               stChangeHostWarRoom,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ChangeHostWarRoom();
    virtual ~Dispatcher_ChangeHostWarRoom();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeHostWarRoom) == 0x04u,
              "CMD404 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_PartyTeleport.
// IDA refs: 0x81DBE70
class Dispatcher_PartyTeleport
    : public IPacketDispatcher<MSG_PARTY_TELEPORT,
                               stPartyTeleport,
                               static_cast<ch_state>(0)> {
public:
    // IDA refs: 0x8595954, 0x82448D2, 0x8244902, 0x81DBE70, 0x81DBFD6,
    //           0x81DC19A, 0x81DC426.
    Dispatcher_PartyTeleport();
    virtual ~Dispatcher_PartyTeleport();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PartyTeleport) == 0x04u,
              "CMD435 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_PartyTeleportConfirm.
// IDA refs: 0x8595938, 0x8244884, 0x82448B4, 0x81DC598, 0x81DC5EE,
//          0x81DC75C, 0x81DC830.
class Dispatcher_PartyTeleportConfirm
    : public IPacketDispatcher<MSG_PARTY_TELEPORT_CONFIRM,
                               stPartyTeleportConfirm,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_PartyTeleportConfirm();
    virtual ~Dispatcher_PartyTeleportConfirm();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PartyTeleportConfirm) == 0x04u,
              "CMD436 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_Check3rdPartyConcent.
// IDA refs: 0x81DF3C6, 0x81DF3D0, 0x824448E, 0x82444BE, 0x8597A9A.
class Dispatcher_Check3rdPartyConcent
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Check3rdPartyConcent();
    virtual ~Dispatcher_Check3rdPartyConcent();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Check3rdPartyConcent) == 0x04u,
              "CMD451 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_DeathTowerStageCommand.
// IDA refs: 0x8208A9E, 0x8233A1A, 0x8233A24, 0x8242358, 0x8242388,
//           0x859640C.
class Dispatcher_DeathTowerStageCommand
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DeathTowerStageCommand();
    virtual ~Dispatcher_DeathTowerStageCommand();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DeathTowerStageCommand) == 0x04u,
              "CMD162 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_DungeonEventStoryPause.
// IDA refs: 0x8216DCC, 0x8233CAE, 0x8233CB8, 0x8241A82, 0x8241AB2,
// 0x85967A8.
class Dispatcher_DungeonEventStoryPause
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DungeonEventStoryPause();
    virtual ~Dispatcher_DungeonEventStoryPause();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DungeonEventStoryPause) == 0x04u,
              "CMD194 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_EventDungeon_ClearRoom.
// IDA refs: 0x812BE82, 0x812BEB0, 0x812BEBA, 0x812BF16, 0x812C1C6,
// 0x812C1F6, 0x85957E8.
class DisPatcher_EventDungeon_ClearRoom
    : public IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonClearRoom,
                               CMDPacketStruct::_STResEventDungeonClearRoom,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_EventDungeon_ClearRoom();
    virtual ~DisPatcher_EventDungeon_ClearRoom();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_EventDungeon_ClearRoom) == 0x04u,
              "CMD540 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_EventDungeon_DestoryObject.
// IDA refs: 0x812BD54, 0x812BD82, 0x812BD8C, 0x812BDCC, 0x812C178,
// 0x812C1A8, 0x85957CC.
class DisPatcher_EventDungeon_DestoryObject
    : public IPacketDispatcher<CMDPacketStruct::_STReqEventDungeonDestoryObject,
                               CMDPacketStruct::_STResEventDungeonDestoryObject,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_EventDungeon_DestoryObject();
    virtual ~DisPatcher_EventDungeon_DestoryObject();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_EventDungeon_DestoryObject) == 0x04u,
              "CMD539 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/Dispatcher_QuickJoinRoom.
// IDA refs: 0x82336E8
class Dispatcher_QuickJoinRoom
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_QuickJoinRoom();
    virtual ~Dispatcher_QuickJoinRoom();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_QuickJoinRoom) == 0x04u,
              "CMD101 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_SetPVPSeat.
// IDA refs: 0x823308C
class DisPatcher_SetPVPSeat
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_SetPVPSeat();
    virtual ~DisPatcher_SetPVPSeat();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPVPSeat) == 0x04u,
              "CMD55 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 09_dungeon_pvp_party/DisPatcher_SetPVPTeam.
// IDA refs: 0x82330EE
class DisPatcher_SetPVPTeam
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_SetPVPTeam();
    virtual ~DisPatcher_SetPVPTeam();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPVPTeam) == 0x04u,
              "CMD57 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_09_DUNGEON_PVP_PARTY_DUNGEON_PVP_DISPATCHERS_H_
