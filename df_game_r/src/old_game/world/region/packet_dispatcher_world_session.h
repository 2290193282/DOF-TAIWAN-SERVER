#ifndef OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_H_
#define OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class Dispatcher_ReportAbuseUser
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ReportAbuseUser();
    virtual ~Dispatcher_ReportAbuseUser();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ChangeTutorialFlag
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ChangeTutorialFlag();
    virtual ~Dispatcher_ChangeTutorialFlag();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class DisPatcher_IncreStatus
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_IncreStatus();
    virtual ~DisPatcher_IncreStatus();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_UpdateServerMessage
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_UpdateServerMessage();
    virtual ~Dispatcher_UpdateServerMessage();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_CheckEnableServerMessage
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_CheckEnableServerMessage();
    virtual ~Dispatcher_CheckEnableServerMessage();
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_OperateRidableObject
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859730E
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObjectC2Ev
    // DEMANGLED: Dispatcher_OperateRidableObject::Dispatcher_OperateRidableObject(void)
    Dispatcher_OperateRidableObject();

    // IDA_EA: 0x82403F6
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObjectD2Ev
    // DEMANGLED: Dispatcher_OperateRidableObject::~Dispatcher_OperateRidableObject()
    // IDA_EA: 0x8240426
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObjectD0Ev
    // DEMANGLED: Dispatcher_OperateRidableObject::~Dispatcher_OperateRidableObject()
    virtual ~Dispatcher_OperateRidableObject();

    // IDA_EA: 0x8226DA0
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObject12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_OperateRidableObject::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x823424E
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObject4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_OperateRidableObject::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8234258
    // IDA_NAME: _ZN31Dispatcher_OperateRidableObject7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_OperateRidableObject::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_DieMobAll
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85961F8
    // IDA_NAME: _ZN20Dispatcher_DieMobAllC2Ev
    // DEMANGLED: Dispatcher_DieMobAll::Dispatcher_DieMobAll(void)
    Dispatcher_DieMobAll();

    // IDA_EA: 0x824385E
    // IDA_NAME: _ZN20Dispatcher_DieMobAllD2Ev
    // DEMANGLED: Dispatcher_DieMobAll::~Dispatcher_DieMobAll()
    // IDA_EA: 0x824388E
    // IDA_NAME: _ZN20Dispatcher_DieMobAllD0Ev
    // DEMANGLED: Dispatcher_DieMobAll::~Dispatcher_DieMobAll()
    virtual ~Dispatcher_DieMobAll();

    // IDA_EA: 0x81ECCE6
    // IDA_NAME: _ZN20Dispatcher_DieMobAll12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_DieMobAll::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8232B56
    // IDA_NAME: _ZN20Dispatcher_DieMobAll4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_DieMobAll::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8232B60
    // IDA_NAME: _ZN20Dispatcher_DieMobAll7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_DieMobAll::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ReportAbuseUser) == 0x04u,
              "CMD247 must remain vptr-only");
static_assert(sizeof(Dispatcher_ChangeTutorialFlag) == 0x04u,
              "CMD146 must remain vptr-only");
static_assert(sizeof(DisPatcher_IncreStatus) == 0x04u,
              "CMD032 must remain vptr-only");
static_assert(sizeof(Dispatcher_UpdateServerMessage) == 0x04u,
              "CMD221 must remain vptr-only");
static_assert(sizeof(Dispatcher_CheckEnableServerMessage) == 0x04u,
              "CMD222 must remain vptr-only");
static_assert(sizeof(Dispatcher_OperateRidableObject) == 0x04u,
              "CMD317 must remain vptr-only");
static_assert(sizeof(Dispatcher_DieMobAll) == 0x04u,
              "CMD141 must remain vptr-only");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_PACKET_DISPATCHER_WORLD_SESSION_H_
