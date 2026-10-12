#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_GROWTH_WEAPON_EVENT_USE_MATERIAL_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_GROWTH_WEAPON_EVENT_USE_MATERIAL_H_

#include "old_game/player/core/creature_guild_params.h"
#include "old_game/player/core/userinfo_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD506.  This provider deliberately has no PacketDispatcher-table side
// effect: the frozen 606-entry registration table remains untouched.
class Dispatcher_GrowthWeaponEventUseMaterial
    : public IPacketDispatcher<MSG_GROWTH_EQUIPMENT,
                               stGrowthEquipment,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597E36
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterialC2Ev
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::Dispatcher_GrowthWeaponEventUseMaterial(void)
    Dispatcher_GrowthWeaponEventUseMaterial();

    // IDA_EA: 0x8243ACE / 0x8243AFE
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterialD2Ev / D0Ev
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::~Dispatcher_GrowthWeaponEventUseMaterial()
    virtual ~Dispatcher_GrowthWeaponEventUseMaterial();

    // IDA_EA: 0x81E810C
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E81A4
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E8496
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E82FA
    // IDA_NAME: _ZN39Dispatcher_GrowthWeaponEventUseMaterial11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_GrowthWeaponEventUseMaterial::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GrowthWeaponEventUseMaterial) == 0x04u,
              "CMD506 dispatcher must retain its i386 base-only ABI");
#endif

// CMD505.  The generic MSG_GROWTH_EQUIPMENT dispatcher instantiation is
// deliberately shared with CMD506; the concrete provider below supplies only
// its own old-server virtual surface and does not register a new table slot.
class Dispatcher_GrowthWeaponEventChangeInfinity
    : public IPacketDispatcher<MSG_GROWTH_EQUIPMENT,
                               stGrowthEquipment,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597E1A
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinityC2Ev
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::Dispatcher_GrowthWeaponEventChangeInfinity(void)
    Dispatcher_GrowthWeaponEventChangeInfinity();

    // IDA_EA: 0x8243B1C / 0x8243B4C
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinityD2Ev / D0Ev
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::~Dispatcher_GrowthWeaponEventChangeInfinity()
    virtual ~Dispatcher_GrowthWeaponEventChangeInfinity();

    // IDA_EA: 0x81E7D1C
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E7DB4
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E80BE
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E7F2E
    // IDA_NAME: _ZN42Dispatcher_GrowthWeaponEventChangeInfinity11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_GrowthWeaponEventChangeInfinity::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GrowthWeaponEventChangeInfinity) == 0x04u,
              "CMD505 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_GROWTH_WEAPON_EVENT_USE_MATERIAL_H_
