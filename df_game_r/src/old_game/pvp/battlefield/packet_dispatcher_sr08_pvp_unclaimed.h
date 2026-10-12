#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SR08_PVP_UNCLAIMED_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SR08_PVP_UNCLAIMED_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 99_misc_global/MsgMercenaryCompetition.  The C2S frame still
// begins with the packed 13-byte CS_Header; the compiler aligns the dword
// payload at +0x10, exactly as the old read routine does.
struct MsgMercenaryCompetition : public MSG_BASE {
    uint8_t reserved_0d_0f[3];
    uint32_t charac_no;        // IDA +0x10
    char competition_count;    // IDA +0x14
    char competition_type;     // IDA +0x15
    uint8_t reserved_16_17[2];
};

// IDA unit: 99_misc_global/SendMercenaryCompetitionResult.
class SendMercenaryCompetitionResult : public ParamBase {
public:
    // IDA_EA: 0x824D1E0
    // IDA_NAME: _ZN30SendMercenaryCompetitionResultC2Ev
    // DEMANGLED: SendMercenaryCompetitionResult::SendMercenaryCompetitionResult(void)
    SendMercenaryCompetitionResult();

    // IDA_EA: 0x824D1FC
    // IDA_NAME: _ZN30SendMercenaryCompetitionResultD2Ev
    // IDA_EA: 0x824D22C
    // IDA_NAME: _ZN30SendMercenaryCompetitionResultD0Ev
    // DEMANGLED: SendMercenaryCompetitionResult::~SendMercenaryCompetitionResult()
    virtual ~SendMercenaryCompetitionResult();

    uint8_t accepted;          // IDA +0x04
    uint8_t reserved_05_07[3];
    uint32_t charac_no;        // IDA +0x08
    char competition_type;     // IDA +0x0c
    char competition_count;    // IDA +0x0d
    uint8_t reserved_0e_0f[2];
};

// PacketDispatcher's old slot 170.  The concrete class is restored here but
// deliberately remains absent from the inactive 606-slot root registry.
class Dispatcher_EvadeAssault
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596524
    // IDA_NAME: _ZN23Dispatcher_EvadeAssaultC2Ev
    // DEMANGLED: Dispatcher_EvadeAssault::Dispatcher_EvadeAssault(void)
    Dispatcher_EvadeAssault();

    // IDA_EA: 0x8245CEE
    // IDA_NAME: _ZN23Dispatcher_EvadeAssaultD2Ev
    // IDA_EA: 0x8245D1E
    // IDA_NAME: _ZN23Dispatcher_EvadeAssaultD0Ev
    // DEMANGLED: Dispatcher_EvadeAssault::~Dispatcher_EvadeAssault()
    virtual ~Dispatcher_EvadeAssault();

    // IDA_EA: 0x81CE4EE
    // IDA_NAME: _ZN23Dispatcher_EvadeAssault4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_EvadeAssault::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CE4F8
    // IDA_NAME: _ZN23Dispatcher_EvadeAssault7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_EvadeAssault::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CE582
    // IDA_NAME: _ZN23Dispatcher_EvadeAssault11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_EvadeAssault::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

// PacketDispatcher's old slot 443.  The generic template instance is part of
// this same translation unit so it has exactly one vtable/provider owner.
class Dispatcher_Mercenary_Competition
    : public IPacketDispatcher<MsgMercenaryCompetition,
                               SendMercenaryCompetitionResult,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85979D6
    // IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionC2Ev
    // DEMANGLED: Dispatcher_Mercenary_Competition::Dispatcher_Mercenary_Competition(void)
    Dispatcher_Mercenary_Competition();

    // IDA_EA: 0x82446B0
    // IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionD2Ev
    // IDA_EA: 0x82446E0
    // IDA_NAME: _ZN32Dispatcher_Mercenary_CompetitionD0Ev
    // DEMANGLED: Dispatcher_Mercenary_Competition::~Dispatcher_Mercenary_Competition()
    virtual ~Dispatcher_Mercenary_Competition();

    // IDA_EA: 0x81DDC24
    // IDA_NAME: _ZN32Dispatcher_Mercenary_Competition4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Mercenary_Competition::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DDD00
    // IDA_NAME: _ZN32Dispatcher_Mercenary_Competition7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Competition::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DDE20
    // IDA_NAME: _ZN32Dispatcher_Mercenary_Competition4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Mercenary_Competition::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

static_assert(sizeof(MsgMercenaryCompetition) == 0x18u,
              "CMD443 message must retain the 24-byte old C++ ABI");

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MsgMercenaryCompetition, charac_no) == 0x10u &&
                  offsetof(MsgMercenaryCompetition, competition_count) ==
                      0x14u &&
                  offsetof(MsgMercenaryCompetition, competition_type) ==
                      0x15u,
              "CMD443 message offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EvadeAssault) == 0x04u,
              "CMD170 dispatcher must retain its vptr-only i386 ABI");
static_assert(sizeof(SendMercenaryCompetitionResult) == 0x10u,
              "CMD443 result must retain its 16-byte i386 ABI");
static_assert(sizeof(Dispatcher_Mercenary_Competition) == 0x04u,
              "CMD443 dispatcher must retain its vptr-only i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(SendMercenaryCompetitionResult, accepted) == 0x04u &&
                  offsetof(SendMercenaryCompetitionResult, charac_no) ==
                      0x08u &&
                  offsetof(SendMercenaryCompetitionResult, competition_type) ==
                      0x0cu &&
                  offsetof(SendMercenaryCompetitionResult, competition_count) ==
                      0x0du,
              "CMD443 result offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SR08_PVP_UNCLAIMED_H_
