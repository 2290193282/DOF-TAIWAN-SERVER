#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD27. The old object contains only the inherited state-0 dispatcher vptr.
class DisPatcher_Compound
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_Compound();
    virtual ~DisPatcher_Compound();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD171. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_AgreeEnchant
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_AgreeEnchant();
    virtual ~Dispatcher_AgreeEnchant();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD172. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_TryEnchant
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_TryEnchant();
    virtual ~Dispatcher_TryEnchant();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD173. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_PutItemForEnchant
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PutItemForEnchant();
    virtual ~Dispatcher_PutItemForEnchant();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// CMD255. The old object contains only the inherited state-0 dispatcher vptr.
class Dispatcher_UseSharedEffectItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_UseSharedEffectItem();
    virtual ~Dispatcher_UseSharedEffectItem();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_Compound) == 0x04u,
              "CMD27 must remain the old vptr-only dispatcher object");
static_assert(sizeof(Dispatcher_AgreeEnchant) == 0x04u,
              "CMD171 must remain the old vptr-only dispatcher object");
static_assert(sizeof(Dispatcher_TryEnchant) == 0x04u,
              "CMD172 must remain the old vptr-only dispatcher object");
static_assert(sizeof(Dispatcher_PutItemForEnchant) == 0x04u,
              "CMD173 must remain the old vptr-only dispatcher object");
static_assert(sizeof(Dispatcher_UseSharedEffectItem) == 0x04u,
              "CMD255 must remain the old vptr-only dispatcher object");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_H_
