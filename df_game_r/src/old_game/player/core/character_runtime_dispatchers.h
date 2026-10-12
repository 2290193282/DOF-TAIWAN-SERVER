#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_RUNTIME_DISPATCHERS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_RUNTIME_DISPATCHERS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x82022F6 in the companion source.


#include "old_game/economy/mall/npc_shop_params.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/player/core/character_messages.h"
#include "old_game/player/session/character_select_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// IDA unit: 04_character_select_create/Dispatcher_CharacterStatistic.
class Dispatcher_CharacterStatistic
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596118
    // IDA_NAME: _ZN29Dispatcher_CharacterStatisticC2Ev
    // DEMANGLED: Dispatcher_CharacterStatistic::Dispatcher_CharacterStatistic(void)
    Dispatcher_CharacterStatistic();

    // IDA_EA: 0x8242A5A
    // IDA_NAME: _ZN29Dispatcher_CharacterStatisticD2Ev
    // DEMANGLED: Dispatcher_CharacterStatistic::~Dispatcher_CharacterStatistic()
    // IDA_EA: 0x8242A8A
    // IDA_NAME: _ZN29Dispatcher_CharacterStatisticD0Ev
    // DEMANGLED: Dispatcher_CharacterStatistic::~Dispatcher_CharacterStatistic()
    virtual ~Dispatcher_CharacterStatistic();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CharacterStatistic) == 0x04u,
              "CMD126 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/DisPatcher_DieCharacInDungeon.
class DisPatcher_DieCharacInDungeon
    : public IPacketDispatcher<MSG_DIE_CHARACTER,
                               stDieCharacInDungeon,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_DieCharacInDungeon();
    virtual ~DisPatcher_DieCharacInDungeon();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DieCharacInDungeon) == 0x04u,
              "CMD43 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_RequestCharacSkillInfo.
class Dispatcher_RequestCharacSkillInfo
    : public IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                               stRequestCharacSkillInfo,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_RequestCharacSkillInfo();
    virtual ~Dispatcher_RequestCharacSkillInfo();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestCharacSkillInfo) == 0x04u,
              "RequestCharacSkillInfo dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_Equipslot_Switch.
class Dispatcher_Equipslot_Switch
    : public IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,
                               stEquipslotSwitch,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_Equipslot_Switch();
    virtual ~Dispatcher_Equipslot_Switch();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Equipslot_Switch) == 0x04u,
              "CMD397 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit:
// 04_character_select_create/Dispatcher_Expand_Equipslot_Flag_Update.
class Dispatcher_Expand_Equipslot_Flag_Update
    : public IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,
                               stExpandEquipslotFlagUpdate,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_Expand_Equipslot_Flag_Update();
    virtual ~Dispatcher_Expand_Equipslot_Flag_Update();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Expand_Equipslot_Flag_Update) == 0x04u,
              "CMD398 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_Buy_Charac_Status_Using_QP.
class Dispatcher_Buy_Charac_Status_Using_QP
    : public IPacketDispatcher<MSG_QUEST_SHOP,
                               STQuestShopBuyParam,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_Buy_Charac_Status_Using_QP();
    virtual ~Dispatcher_Buy_Charac_Status_Using_QP();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Buy_Charac_Status_Using_QP) == 0x04u,
              "BuyCharacStatusUsingQP dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_ChangeCharacName.
class Dispatcher_ChangeCharacName
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85961A4
    // IDA_NAME: _ZN27Dispatcher_ChangeCharacNameC2Ev
    // DEMANGLED: Dispatcher_ChangeCharacName::Dispatcher_ChangeCharacName(void)
    Dispatcher_ChangeCharacName();

    // IDA_EA: 0x8242970
    // IDA_NAME: _ZN27Dispatcher_ChangeCharacNameD2Ev
    // DEMANGLED: Dispatcher_ChangeCharacName::~Dispatcher_ChangeCharacName()
    // IDA_EA: 0x82429A0
    // IDA_NAME: _ZN27Dispatcher_ChangeCharacNameD0Ev
    // DEMANGLED: Dispatcher_ChangeCharacName::~Dispatcher_ChangeCharacName()
    virtual ~Dispatcher_ChangeCharacName();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeCharacName) == 0x04u,
              "CMD138 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_police_queryCharacName_useMid.
class Dispatcher_police_queryCharacName_useMid
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85973B6
    // IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidC2Ev
    // DEMANGLED: Dispatcher_police_queryCharacName_useMid::Dispatcher_police_queryCharacName_useMid(void)
    Dispatcher_police_queryCharacName_useMid();

    // IDA_EA: 0x8240270
    // IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidD2Ev
    // DEMANGLED: Dispatcher_police_queryCharacName_useMid::~Dispatcher_police_queryCharacName_useMid()
    // IDA_EA: 0x82402A0
    // IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidD0Ev
    // DEMANGLED: Dispatcher_police_queryCharacName_useMid::~Dispatcher_police_queryCharacName_useMid()
    virtual ~Dispatcher_police_queryCharacName_useMid();

    // IDA_EA: 0x82273AE
    // IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMid12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_police_queryCharacName_useMid::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_police_queryCharacName_useMid) == 0x04u,
              "CMD331 dispatcher must retain its i386 base-only ABI");
#endif


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_RUNTIME_DISPATCHERS_H_
