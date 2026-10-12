#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_ATTENDANCE_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_ATTENDANCE_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// CMD356 carries exactly one selector byte after the packed 13-byte C2S
// header.  The original template constructs this local stack object directly;
// there is no separately emitted MSG_GUILD_ATTENDANCE constructor to invent.
#pragma pack(push, 1)
struct MSG_GUILD_ATTENDANCE : public MSG_BASE {
    uint8_t request_type;  // +0x0d
};
#pragma pack(pop)

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(sizeof(MSG_GUILD_ATTENDANCE) == 0x0eu,
              "CMD356 request must retain its old packed extent");
static_assert(offsetof(MSG_GUILD_ATTENDANCE, request_type) == 0x0du,
              "CMD356 selector must remain at +0x0d");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// CMD356 / GUILD_ATTENDANCE_INFO.  This remains deliberately unregistered
// until the complete original 606-slot PacketDispatcher root is available.
class Dispatcher_GuildAttendanceInfo
    : public IPacketDispatcher<MSG_GUILD_ATTENDANCE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597522
    // IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfoC2Ev
    // DEMANGLED: Dispatcher_GuildAttendanceInfo::Dispatcher_GuildAttendanceInfo(void)
    Dispatcher_GuildAttendanceInfo();

    // IDA_EA: 0x8245022 / 0x8245052
    // IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfoD2Ev /
    //           _ZN30Dispatcher_GuildAttendanceInfoD0Ev
    // DEMANGLED: Dispatcher_GuildAttendanceInfo::~Dispatcher_GuildAttendanceInfo()
    virtual ~Dispatcher_GuildAttendanceInfo();

    // IDA_EA: 0x81D543C / 0x81D5492
    // IDA_NAME: _ZN30Dispatcher_GuildAttendanceInfo4readER9PacketBufR8MSG_BASE /
    //           _ZN30Dispatcher_GuildAttendanceInfo7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_GuildAttendanceInfo::read(PacketBuf &,MSG_BASE &) /
    //            Dispatcher_GuildAttendanceInfo::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_GuildAttendanceInfo) == 0x04u,
              "CMD356 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_GUILD_ATTENDANCE_DISPATCHER_H_
