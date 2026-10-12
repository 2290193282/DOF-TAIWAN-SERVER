#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_DELETE_ITEM_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_DELETE_ITEM_H_

#include <stdint.h>

#include "old_game/economy/inventory/delete_item_param.h"
#include "old_game/economy/inventory/inventory.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 13_database_dispatch/DisPatcher_DeleteItem (CMD18).  The old
// read routine accepts a variable prefix, but its message stack object keeps
// storage for the protocol maximum of 100 deletion records.
struct __attribute__((packed)) MSG_DELETE_ITEM : MSG_BASE {
    char item_space;                    // +0x0d
    char item_count;                    // +0x0e
    int16_t delete_type[100];           // +0x0f
    int16_t slots[100];                 // +0xd7
    uint32_t item_ids[100];             // +0x19f
    int32_t counts[100];                // +0x32f
    int32_t inventory_checksum;         // +0x4bf
};

static_assert(sizeof(MSG_DELETE_ITEM) == 0x4c3u,
              "CMD18 request storage must retain the old 1219-byte ABI");

// These declarations must precede the derived class.  Its base-class use
// otherwise instantiates generic members before the one canonical CMD18
// template provider can define the old-server specializations.
template <>
IPacketDispatcher<MSG_DELETE_ITEM,
                  stDeleteItemParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_DELETE_ITEM,
                  stDeleteItemParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_DELETE_ITEM,
                      stDeleteItemParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_DELETE_ITEM,
                       stDeleteItemParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

class DisPatcher_DeleteItem
    : public IPacketDispatcher<MSG_DELETE_ITEM,
                               stDeleteItemParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8595150
    // IDA_NAME: _ZN21DisPatcher_DeleteItemC2Ev
    DisPatcher_DeleteItem();

    // IDA_EA: 0x82321F4
    // IDA_NAME: _ZN21DisPatcher_DeleteItemD2Ev
    // IDA_EA: 0x8232224
    // IDA_NAME: _ZN21DisPatcher_DeleteItemD0Ev
    virtual ~DisPatcher_DeleteItem();

    // IDA_EA: 0x81C17C0
    // IDA_NAME: _ZN21DisPatcher_DeleteItem4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_DeleteItem::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81C1DDA
    // IDA_NAME: _ZN21DisPatcher_DeleteItem7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_DeleteItem::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C2792
    // IDA_NAME: _ZN21DisPatcher_DeleteItem4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_DeleteItem::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81C1A6C
    // IDA_NAME: _ZN21DisPatcher_DeleteItem11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_DeleteItem::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81C1AD6
    // IDA_NAME: _ZN21DisPatcher_DeleteItem13exercise_modeEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_DeleteItem::exercise_mode(CUser *,MSG_BASE &,ParamBase &)
    int exercise_mode(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C1CF6
    // IDA_NAME: _ZN21DisPatcher_DeleteItem16restricted_checkEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_DeleteItem::restricted_check(CUser *,MSG_BASE &)
    int restricted_check(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81C1D5C
    // IDA_NAME: _ZN21DisPatcher_DeleteItem15condition_checkEP5CUserR8MSG_BASER10INVEN_TYPEi
    // DEMANGLED: DisPatcher_DeleteItem::condition_check(CUser *,MSG_BASE &,INVEN_TYPE &,int)
    int condition_check(CUser* user,
                        MSG_BASE& message,
                        INVEN_TYPE& inventory_type,
                        int index);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DeleteItem) == 0x04u,
              "CMD18 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_DELETE_ITEM_H_
