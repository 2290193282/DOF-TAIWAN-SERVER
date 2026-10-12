#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SR08_INVENTORY_UNCLAIMED_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SR08_INVENTORY_UNCLAIMED_H_

#include <stddef.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

struct Inven_Item;

// CMD440 carries no payload beyond the 13-byte legacy C2S header.  It still
// has a distinct C++ message type because the old dispatcher template vtable
// is specialized on MSG_ITEM_DICTIONARY, rather than MSG_BASE.
struct MSG_ITEM_DICTIONARY : public MSG_BASE {};

// IDA unit: 99_misc_global/MSG_USE_RENAME_CARD.  Its two wire bytes are read
// after the packed 13-byte C2S header, but the old C++ object deliberately
// has one compiler alignment byte at +0x0d and stores the short at +0x0e.
// Thus the protocol frame remains 13+2=15 bytes while this stack object is
// 16 bytes; do not pack the C++ type.
struct MSG_USE_RENAME_CARD : public MSG_BASE {
    // IDA_EA: 0x822E3D8
    // IDA_NAME: _ZN19MSG_USE_RENAME_CARDC2Ev
    // DEMANGLED: MSG_USE_RENAME_CARD::MSG_USE_RENAME_CARD(void)
    MSG_USE_RENAME_CARD();

    short slot;  // IDA +0x0e
};

// IDA unit: 99_misc_global/stUseRenameCard.
class stUseRenameCard : public ParamBase {
public:
    // IDA_EA: 0x8231E0A
    // IDA_NAME: _ZN15stUseRenameCardC2Ev
    // DEMANGLED: stUseRenameCard::stUseRenameCard(void)
    stUseRenameCard();

    // IDA_EA: 0x8246528
    // IDA_NAME: _ZN15stUseRenameCardD2Ev
    // DEMANGLED: stUseRenameCard::~stUseRenameCard()
    // IDA_EA: 0x8246558
    // IDA_NAME: _ZN15stUseRenameCardD0Ev
    // DEMANGLED: stUseRenameCard::~stUseRenameCard()
    virtual ~stUseRenameCard();

    int result;  // IDA +0x04
};

// IDA unit: 99_misc_global/stItemDictionary.
class stItemDictionary : public ParamBase {
public:
    // IDA_EA: 0x82318D2
    // IDA_NAME: _ZN16stItemDictionaryC2Ev
    // DEMANGLED: stItemDictionary::stItemDictionary(void)
    stItemDictionary();

    // IDA_EA: 0x8246B94
    // IDA_NAME: _ZN16stItemDictionaryD2Ev
    // DEMANGLED: stItemDictionary::~stItemDictionary()
    // IDA_EA: 0x8246BC4
    // IDA_NAME: _ZN16stItemDictionaryD0Ev
    // DEMANGLED: stItemDictionary::~stItemDictionary()
    virtual ~stItemDictionary();

    int result;
};

// IDA unit: 13_database_dispatch/Dispatcher_ItemDictionary.
// PacketDispatcher's original slot 440 owns this concrete object.  This
// source intentionally does not register that slot while the full 606-slot
// root constructor and return chain remain disabled.
class Dispatcher_ItemDictionary
    : public IPacketDispatcher<MSG_ITEM_DICTIONARY,
                               stItemDictionary,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597982
    // IDA_NAME: _ZN25Dispatcher_ItemDictionaryC2Ev
    // DEMANGLED: Dispatcher_ItemDictionary::Dispatcher_ItemDictionary(void)
    Dispatcher_ItemDictionary();

    // IDA_EA: 0x824479A
    // IDA_NAME: _ZN25Dispatcher_ItemDictionaryD2Ev
    // DEMANGLED: Dispatcher_ItemDictionary::~Dispatcher_ItemDictionary()
    // IDA_EA: 0x82447CA
    // IDA_NAME: _ZN25Dispatcher_ItemDictionaryD0Ev
    // DEMANGLED: Dispatcher_ItemDictionary::~Dispatcher_ItemDictionary()
    virtual ~Dispatcher_ItemDictionary();

    // IDA_EA: 0x81DD7EA
    // IDA_NAME: _ZN25Dispatcher_ItemDictionary4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ItemDictionary::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DD7FE
    // IDA_NAME: _ZN25Dispatcher_ItemDictionary7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ItemDictionary::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DD8B6
    // IDA_NAME: _ZN25Dispatcher_ItemDictionary4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_ItemDictionary::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81DD87A
    // IDA_NAME: _ZN25Dispatcher_ItemDictionary11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ItemDictionary::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

// IDA unit: 13_database_dispatch/Dispatcher_UseRenameCard.
//
// PacketDispatcher's original slot 510 owns this concrete object.  Its full
// item lock/check/delete/character-view transition is restored below, but the
// object deliberately is not inserted into the inactive 606-slot root table.
class Dispatcher_UseRenameCard
    : public IPacketDispatcher<MSG_USE_RENAME_CARD,
                               stUseRenameCard,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597E8A
    // IDA_NAME: _ZN24Dispatcher_UseRenameCardC2Ev
    // DEMANGLED: Dispatcher_UseRenameCard::Dispatcher_UseRenameCard(void)
    Dispatcher_UseRenameCard();

    // IDA_EA: 0x8243BB8
    // IDA_NAME: _ZN24Dispatcher_UseRenameCardD2Ev
    // DEMANGLED: Dispatcher_UseRenameCard::~Dispatcher_UseRenameCard()
    // IDA_EA: 0x8243BE8
    // IDA_NAME: _ZN24Dispatcher_UseRenameCardD0Ev
    // DEMANGLED: Dispatcher_UseRenameCard::~Dispatcher_UseRenameCard()
    virtual ~Dispatcher_UseRenameCard();

    // IDA_EA: 0x81E72C4
    // IDA_NAME: _ZN24Dispatcher_UseRenameCard4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UseRenameCard::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E731A
    // IDA_NAME: _ZN24Dispatcher_UseRenameCard7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UseRenameCard::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E756C
    // IDA_NAME: _ZNK24Dispatcher_UseRenameCard7getItemERK5CUserisR10Inven_Item
    // DEMANGLED: Dispatcher_UseRenameCard::getItem(CUser const&,int,short,Inven_Item &)const
    int getItem(const CUser& user,
                int inventory_type,
                short slot,
                Inven_Item& item) const;

    // IDA_EA: 0x81E7652
    // IDA_NAME: _ZN24Dispatcher_UseRenameCard11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_UseRenameCard::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

static_assert(sizeof(MSG_ITEM_DICTIONARY) == 0x0du,
              "CMD440 message must retain the 13-byte C2S-header ABI");
static_assert(sizeof(MSG_USE_RENAME_CARD) == 0x10u,
              "CMD510 C++ message object must retain its 16-byte ABI");

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_USE_RENAME_CARD, slot) == 0x0eu,
              "CMD510 slot offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__i386__)
static_assert(sizeof(stItemDictionary) == 0x08u,
              "CMD440 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(Dispatcher_ItemDictionary) == 0x04u,
              "CMD440 dispatcher must retain its vptr-only i386 ABI");
static_assert(sizeof(stUseRenameCard) == 0x08u,
              "CMD510 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(Dispatcher_UseRenameCard) == 0x04u,
              "CMD510 dispatcher must retain its vptr-only i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(stItemDictionary, result) == 0x04u,
              "CMD440 result offset changed");
static_assert(offsetof(stUseRenameCard, result) == 0x04u,
              "CMD510 result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SR08_INVENTORY_UNCLAIMED_H_
