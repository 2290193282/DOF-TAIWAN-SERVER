#include "old_game/economy/inventory/packet_dispatcher_sr08_inventory_unclaimed.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kProcessSignature[] =
    "virtual int Dispatcher_ItemDictionary::process(CUser*, MSG_BASE&, ParamBase&)";
const char kUseRenameCardReadSignature[] =
    "virtual int Dispatcher_UseRenameCard::read(PacketBuf&, MSG_BASE&)";
const char kUseRenameCardProcessSignature[] =
    "virtual int Dispatcher_UseRenameCard::process(CUser*, MSG_BASE&, ParamBase&)";
const char kUseRenameCardTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_USE_RENAME_CARD, PARAM_TYPE = stUseRenameCard, ch_state STATE = (ch_state)0u]";

}  // namespace

// IDA_EA: 0x82318D2
// IDA_NAME: _ZN16stItemDictionaryC2Ev
// DEMANGLED: stItemDictionary::stItemDictionary(void)
stItemDictionary::stItemDictionary() : ParamBase(), result(0) {}

// IDA_EA: 0x8246B94
// IDA_NAME: _ZN16stItemDictionaryD2Ev
// DEMANGLED: stItemDictionary::~stItemDictionary()
// IDA_EA: 0x8246BC4
// IDA_NAME: _ZN16stItemDictionaryD0Ev
// DEMANGLED: stItemDictionary::~stItemDictionary()
stItemDictionary::~stItemDictionary() {}

// IDA_EA: 0x822E3D8
// IDA_NAME: _ZN19MSG_USE_RENAME_CARDC2Ev
// DEMANGLED: MSG_USE_RENAME_CARD::MSG_USE_RENAME_CARD(void)
MSG_USE_RENAME_CARD::MSG_USE_RENAME_CARD() : MSG_BASE(), slot(0) {}

// IDA_EA: 0x8231E0A
// IDA_NAME: _ZN15stUseRenameCardC2Ev
// DEMANGLED: stUseRenameCard::stUseRenameCard(void)
stUseRenameCard::stUseRenameCard() : ParamBase(), result(0) {}

// IDA_EA: 0x8246528
// IDA_NAME: _ZN15stUseRenameCardD2Ev
// DEMANGLED: stUseRenameCard::~stUseRenameCard()
// IDA_EA: 0x8246558
// IDA_NAME: _ZN15stUseRenameCardD0Ev
// DEMANGLED: stUseRenameCard::~stUseRenameCard()
stUseRenameCard::~stUseRenameCard() {}

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// The original template specialization has its own vtable and seven direct
// identities.  Keep this as the single explicit owner; the dispatcher is not
// inserted into PacketDispatcher's inactive root table here.
// IDA_EA: 0x824D888
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824D9D8
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824D9E2
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825D648
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825BBEA
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EED2Ev
// IDA_EA: 0x825BC1A
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EED0Ev
// IDA_EA: 0x8598E12
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_ITEM_DICTIONARY16stItemDictionaryL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_ITEM_DICTIONARY,
                                 stItemDictionary,
                                 static_cast<ch_state>(3)>;

// The CMD510 template constructor creates MSG_USE_RENAME_CARD directly, as
// the old 0x8248ED4 body does.  Only its diagnostic specialization is written
// out so the recovered literal and packet-length evaluation order stay stable
// instead of depending on the host compiler's __PRETTY_FUNCTION__ spelling.
template <>
int IPacketDispatcher<MSG_USE_RENAME_CARD,
                      stUseRenameCard,
                      static_cast<ch_state>(0)>::check_state(
    CUser* user,
    PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    const int packet_len = packet.get_len();
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kUseRenameCardTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet_len);
    return 149;
}

// IDA_EA: 0x8248ED4
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8249030
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824903A
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x82490CC
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x825B140
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EED2Ev
// IDA_EA: 0x825B170
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EED0Ev
// IDA_EA: 0x85991AE
// IDA_NAME: _ZN17IPacketDispatcherI19MSG_USE_RENAME_CARD15stUseRenameCardL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_USE_RENAME_CARD,
                                 stUseRenameCard,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597982
// IDA_NAME: _ZN25Dispatcher_ItemDictionaryC2Ev
// DEMANGLED: Dispatcher_ItemDictionary::Dispatcher_ItemDictionary(void)
Dispatcher_ItemDictionary::Dispatcher_ItemDictionary() {}

// IDA_EA: 0x824479A
// IDA_NAME: _ZN25Dispatcher_ItemDictionaryD2Ev
// DEMANGLED: Dispatcher_ItemDictionary::~Dispatcher_ItemDictionary()
// IDA_EA: 0x82447CA
// IDA_NAME: _ZN25Dispatcher_ItemDictionaryD0Ev
// DEMANGLED: Dispatcher_ItemDictionary::~Dispatcher_ItemDictionary()
Dispatcher_ItemDictionary::~Dispatcher_ItemDictionary() {}

// IDA_EA: 0x81DD7EA
// IDA_NAME: _ZN25Dispatcher_ItemDictionary4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ItemDictionary::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ItemDictionary::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81DD7FE
// IDA_NAME: _ZN25Dispatcher_ItemDictionary7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ItemDictionary::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ItemDictionary::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    stItemDictionary& output = static_cast<stItemDictionary&>(param);
    output.result = check_error(user, message);
    if (output.result >= 0) {
        return 0;
    }
    return LineFunc(20050, kProcessSignature, output.result, 0);
}

// IDA_EA: 0x81DD87A
// IDA_NAME: _ZN25Dispatcher_ItemDictionary11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ItemDictionary::check_error(CUser *,MSG_BASE &)
int Dispatcher_ItemDictionary::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    return user->get_state() <= 2 ? 1 : 0;
}

// IDA_EA: 0x81DD8B6
// IDA_NAME: _ZN25Dispatcher_ItemDictionary4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ItemDictionary::send(CUser* user, ParamBase& param)
void Dispatcher_ItemDictionary::send(CUser* user, ParamBase& param) {
    const stItemDictionary& output =
        static_cast<const stItemDictionary&>(param);
    if (output.result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ITEM_DICTIONARY,
                                 static_cast<unsigned char>(output.result));
        return;
    }
    user->SendCmdOkPacket(ENUM_CMDPACKET_ITEM_DICTIONARY);
}

// IDA_EA: 0x8597E8A
// IDA_NAME: _ZN24Dispatcher_UseRenameCardC2Ev
// DEMANGLED: Dispatcher_UseRenameCard::Dispatcher_UseRenameCard(void)
Dispatcher_UseRenameCard::Dispatcher_UseRenameCard()
    : IPacketDispatcher<MSG_USE_RENAME_CARD,
                        stUseRenameCard,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243BB8
// IDA_NAME: _ZN24Dispatcher_UseRenameCardD2Ev
// DEMANGLED: Dispatcher_UseRenameCard::~Dispatcher_UseRenameCard()
// IDA_EA: 0x8243BE8
// IDA_NAME: _ZN24Dispatcher_UseRenameCardD0Ev
// DEMANGLED: Dispatcher_UseRenameCard::~Dispatcher_UseRenameCard()
Dispatcher_UseRenameCard::~Dispatcher_UseRenameCard() {}

// IDA_EA: 0x81E72C4
// IDA_NAME: _ZN24Dispatcher_UseRenameCard4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseRenameCard::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseRenameCard::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USE_RENAME_CARD& request =
        static_cast<MSG_USE_RENAME_CARD&>(message);
    if (packet.get_short(request.slot) == 1) {
        return 0;
    }
    return LineFunc(25331, kUseRenameCardReadSignature, 0, 0);
}

// IDA_EA: 0x81E731A
// IDA_NAME: _ZN24Dispatcher_UseRenameCard7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseRenameCard::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseRenameCard::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    MSG_USE_RENAME_CARD& request =
        static_cast<MSG_USE_RENAME_CARD&>(message);
    stUseRenameCard& output = static_cast<stUseRenameCard&>(param);
    output.result = check_error(user, message);
    if (output.result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_RENAME_CARD,
                                 static_cast<unsigned char>(output.result));
        return 0;
    }

    Inven_Item rename_card;
    const int item_result = getItem(*user, 1, request.slot, rename_card);
    if (item_result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_RENAME_CARD,
                                 static_cast<unsigned char>(item_result));
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(rename_card.getKey());
    if (!item) {
        return 0;
    }
    if (item->is_stackable() != 1) {
        return 0;
    }
    if (static_cast<const CStackableItem*>(item)->verifyStackableAction(
            static_cast<StackableAction::Type>(51)) != true) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_RENAME_CARD,
                                 static_cast<unsigned char>(item_result));
        return 0;
    }

    user->SetCharacViewState(255);
    CInventory* const inventory = user->getCurCharacInvenW();
    if (inventory->delete_item(static_cast<INVEN_TYPE>(1),
                               static_cast<int>(request.slot),
                               1,
                               static_cast<eItemDelReason>(3),
                               true) == 1) {
        user->SendCmdOkPacket(ENUM_CMDPACKET_USE_RENAME_CARD);
        user->ReturnToSelectCharacList(true);
        user->UpdateCharacView();
        return 0;
    }

    user->SetCharacViewState(0);
    user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_RENAME_CARD, 17);
    return LineFunc(25390,
                    kUseRenameCardProcessSignature,
                    0,
                    user->get_acc_id());
}

// IDA_EA: 0x81E756C
// IDA_NAME: _ZNK24Dispatcher_UseRenameCard7getItemERK5CUserisR10Inven_Item
// DEMANGLED: Dispatcher_UseRenameCard::getItem(CUser const&,int,short,Inven_Item &)const
int Dispatcher_UseRenameCard::getItem(const CUser& user,
                                      int inventory_type,
                                      short slot,
                                      Inven_Item& item) const {
    if (user.CheckItemLock(inventory_type, static_cast<int>(slot))) {
        return 1;
    }

    item = user.getCurCharacInvenR()->GetInvenSlot(
        inventory_type, static_cast<int>(slot));
    return item.getKey() == 0;
}

// IDA_EA: 0x81E7652
// IDA_NAME: _ZN24Dispatcher_UseRenameCard11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_UseRenameCard::check_error(CUser *,MSG_BASE &)
int Dispatcher_UseRenameCard::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() != 3) {
        return 19;
    }
    if (user->CheckInTrade()) {
        return 19;
    }

    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type == 15 || channel_type == 16) {
        return 19;
    }

    if (!user->getCurCharacR()) {
        return 19;
    }

    // IDA dereferences the current _Charac_info's first dword here.  The
    // canonical CUserCharacInfo accessor is the same checked field boundary;
    // using it keeps this dispatcher out of the CUser layout.
    return user->IsWaitingStateCharac(
               static_cast<unsigned int>(user->getCurCharacNo()))
               ? 0
               : 18;
}
