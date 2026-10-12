#ifndef OLDSRV_OLD_GAME_WORLD_WORLD_DISPATCHER_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_WORLD_DISPATCHER_RUNTIME_H_

#include "old_game/world/broadcast/world_messages.h"
#include "old_game/world/region/world_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 05_world_enter_village/DisPatcher_SetArea.
class DisPatcher_SetArea
    : public IPacketDispatcher<MSG_SET_USER_AREA,
                               stSetAreaParam,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_SetArea();
    virtual ~DisPatcher_SetArea();

    // IDA_EA: 0x81C91E6
    // IDA_NAME: _ZN18DisPatcher_SetArea4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_SetArea::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81C93E8
    // IDA_NAME: _ZN18DisPatcher_SetArea11check_errorEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_SetArea::check_error(CUser *,MSG_BASE &,ParamBase &)
    int check_error(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C951C
    // IDA_NAME: _ZN18DisPatcher_SetArea7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_SetArea::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81C9656
    // IDA_NAME: _ZN18DisPatcher_SetArea4sendEP5CUserR9ParamBase
    // DEMANGLED: DisPatcher_SetArea::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetArea) == 0x04u,
              "CMD38 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 05_world_enter_village/DisPatcher_MoveMap.
class DisPatcher_MoveMap
    : public IPacketDispatcher<MSG_MOVE_MAP,
                               stMoveMapParam,
                               static_cast<ch_state>(0)> {
public:
    DisPatcher_MoveMap();
    virtual ~DisPatcher_MoveMap();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    int check_error(CUser* user, MSG_BASE& message);
    char CheckTrapBitsHackCnt(CUser* user, MSG_MOVE_MAP& message);
    char CheckOneMapPlayTimeHackCnt(CUser* user, MSG_MOVE_MAP& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_MoveMap) == 0x04u,
              "CMD48 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 05_world_enter_village/DisPatcher_SetPVPMapIndex.
class DisPatcher_SetPVPMapIndex
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_SetPVPMapIndex();
    virtual ~DisPatcher_SetPVPMapIndex();

    // IDA_EA: 0x81F8602
    // IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_SetPVPMapIndex::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82332D8
    // IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_SetPVPMapIndex::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82332E2
    // IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_SetPVPMapIndex::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPVPMapIndex) == 0x04u,
              "CMD62 dispatcher must retain its i386 base-only ABI");
#endif

// IDA unit: 05_world_enter_village/Dispatcher_Back2Village.
class Dispatcher_Back2Village
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_Back2Village();
    virtual ~Dispatcher_Back2Village();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233876
    // IDA_NAME: _ZN23Dispatcher_Back2Village4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Back2Village::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233880
    // IDA_NAME: _ZN23Dispatcher_Back2Village7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Back2Village::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_Back2Village) == 0x04u,
              "CMD142 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_WORLD_DISPATCHER_RUNTIME_H_
