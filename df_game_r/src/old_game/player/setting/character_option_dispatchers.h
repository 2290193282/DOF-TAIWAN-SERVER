#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_OPTION_DISPATCHERS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_OPTION_DISPATCHERS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x81CE8BC in the companion source.


#include "old_game/player/core/character_messages.h"
#include "old_game/player/session/character_select_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// IDA unit: 04_character_select_create/Dispatcher_SaveCharacterOption.
class Dispatcher_SaveCharacterOption
    : public IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                               stSaveCharacOption,
                               static_cast<ch_state>(2)> {
public:
    Dispatcher_SaveCharacterOption();
    virtual ~Dispatcher_SaveCharacterOption();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SaveCharacterOption) == 0x04u,
              "SaveCharacterOption dispatcher must retain i386 base-only ABI");
#endif

// IDA unit:
// 04_character_select_create/Dispatcher_ComboSkillExtensionQuickSlotReset.
class Dispatcher_ComboSkillExtensionQuickSlotReset
    : public IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                               stComboSkillExtensionQuickSlotReset,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_ComboSkillExtensionQuickSlotReset();
    virtual ~Dispatcher_ComboSkillExtensionQuickSlotReset();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ComboSkillExtensionQuickSlotReset) == 0x04u,
              "CMD511 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_QueryCharacInfo.
class Dispatcher_QueryCharacInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_QueryCharacInfo();
    virtual ~Dispatcher_QueryCharacInfo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_QueryCharacInfo) == 0x04u,
              "QueryCharacInfo dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_QueryCharacInfoMailbox.
class Dispatcher_QueryCharacInfoMailbox
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_QueryCharacInfoMailbox();
    virtual ~Dispatcher_QueryCharacInfoMailbox();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_QueryCharacInfoMailbox) == 0x04u,
              "QueryCharacInfoMailbox dispatcher must retain i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_Change_Charac_Slot.
class Dispatcher_Change_Charac_Slot
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Change_Charac_Slot();
    virtual ~Dispatcher_Change_Charac_Slot();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Change_Charac_Slot) == 0x04u,
              "ChangeCharacSlot dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 04_character_select_create/Dispatcher_CharacSlotExtendEffect.
class Dispatcher_CharacSlotExtendEffect
    : public IPacketDispatcher<MSG_BASE,
                               stCharacSlotExtendEffect,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_CharacSlotExtendEffect();
    virtual ~Dispatcher_CharacSlotExtendEffect();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CharacSlotExtendEffect) == 0x04u,
              "CharacSlotExtendEffect dispatcher must retain i386 base-only ABI");
#endif


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_OPTION_DISPATCHERS_H_
