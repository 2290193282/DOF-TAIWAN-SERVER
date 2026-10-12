#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_PVPRequestFight
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PVPRequestFight();
    virtual ~Dispatcher_PVPRequestFight();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CompleteLoadAssault.  Its
// dispatch path first gives a PvP room's live IMatch a complete-load event;
// otherwise it enters the typed assault-place lifecycle through CAssaultMgr.
class Dispatcher_CompleteLoadAssault
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596268
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultC2Ev
    // DEMANGLED: Dispatcher_CompleteLoadAssault::Dispatcher_CompleteLoadAssault(void)
    Dispatcher_CompleteLoadAssault();

    // IDA_EA: 0x824274E
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultD2Ev
    // DEMANGLED: Dispatcher_CompleteLoadAssault::~Dispatcher_CompleteLoadAssault()
    // IDA_EA: 0x824277E
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultD0Ev
    // DEMANGLED: Dispatcher_CompleteLoadAssault::~Dispatcher_CompleteLoadAssault()
    virtual ~Dispatcher_CompleteLoadAssault();

    // IDA_EA: 0x8206F10
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CompleteLoadAssault::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233916
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_CompleteLoadAssault::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233920
    // IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_CompleteLoadAssault::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// IDA unit: 13_database_dispatch/Dispatcher_CompleteLoadAfterAssault.
// This separate packet confirms a party's post-PowerWar load state and
// schedules the Power.etc-defined super-state timeout for the user.
class Dispatcher_CompleteLoadAfterAssault
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85962D8
    // IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssaultC2Ev
    // DEMANGLED: Dispatcher_CompleteLoadAfterAssault::Dispatcher_CompleteLoadAfterAssault(void)
    Dispatcher_CompleteLoadAfterAssault();

    // IDA_EA: 0x82427EA / 0x824281A
    // IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssaultD2Ev /
    //           _ZN35Dispatcher_CompleteLoadAfterAssaultD0Ev
    virtual ~Dispatcher_CompleteLoadAfterAssault();

    // IDA_EA: 0x8206B5C
    // IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssault12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_CompleteLoadAfterAssault::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82338EE / 0x82338F8
    // IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssault4readER9PacketBufR8MSG_BASE /
    //           _ZN35Dispatcher_CompleteLoadAfterAssault7processEP5CUserR8MSG_BASER9ParamBase
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ConnectP2PAssault
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ConnectP2PAssault();
    virtual ~Dispatcher_ConnectP2PAssault();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ChangeHp
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ChangeHp();
    virtual ~Dispatcher_ChangeHp();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ConnectP2PAfterAssault
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ConnectP2PAfterAssault();
    virtual ~Dispatcher_ConnectP2PAfterAssault();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DieBloodMonster
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_DieBloodMonster();
    virtual ~Dispatcher_DieBloodMonster();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CheckAssaultMotionHack
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CheckAssaultMotionHack();
    virtual ~Dispatcher_CheckAssaultMotionHack();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_PowerWarProcessInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_PowerWarProcessInfo();
    virtual ~Dispatcher_PowerWarProcessInfo();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_RequestAddPvPBuddy
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestAddPvPBuddy();
    virtual ~Dispatcher_RequestAddPvPBuddy();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_PVPRequestFight) == 0x04u,
              "CMD115 must remain vptr-only");
static_assert(sizeof(Dispatcher_CompleteLoadAssault) == 0x04u,
              "CMD148 must remain vptr-only");
static_assert(sizeof(Dispatcher_CompleteLoadAfterAssault) == 0x04u,
              "CMD248 must remain vptr-only");
static_assert(sizeof(Dispatcher_ConnectP2PAssault) == 0x04u,
              "CMD149 must remain vptr-only");
static_assert(sizeof(Dispatcher_ChangeHp) == 0x04u,
              "CMD152 must remain vptr-only");
static_assert(sizeof(Dispatcher_ConnectP2PAfterAssault) == 0x04u,
              "CMD249 must remain vptr-only");
static_assert(sizeof(Dispatcher_DieBloodMonster) == 0x04u,
              "CMD259 must remain vptr-only");
static_assert(sizeof(Dispatcher_CheckAssaultMotionHack) == 0x04u,
              "CMD261 must remain vptr-only");
static_assert(sizeof(Dispatcher_PowerWarProcessInfo) == 0x04u,
              "CMD236 must remain vptr-only");
static_assert(sizeof(Dispatcher_RequestAddPvPBuddy) == 0x04u,
              "CMD289 must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_PVP_ASSAULT_H_
