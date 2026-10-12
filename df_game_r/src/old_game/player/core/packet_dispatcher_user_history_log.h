#ifndef OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_USER_HISTORY_LOG_H_
#define OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_USER_HISTORY_LOG_H_

#include <stddef.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

// CMD426 uses ordinary i386 alignment after the packed 13-byte C2S base:
// action is followed by compiler padding before the length field at +0x10.
// The padding is deliberately implicit because the original constructor does
// not initialize it.
struct MSG_USER_HISTORY_LOG : MSG_BASE {
    unsigned char action;  // +0x0d
    int history_length;    // +0x10
    char history_text[256];  // +0x14

    // IDA_EA: 0x822DEA8
    // IDA_NAME: _ZN20MSG_USER_HISTORY_LOGC2Ev
    // DEMANGLED: MSG_USER_HISTORY_LOG::MSG_USER_HISTORY_LOG(void)
    MSG_USER_HISTORY_LOG();
};

static_assert(sizeof(MSG_USER_HISTORY_LOG) == 0x114u,
              "CMD426 user-history input must retain its 0x114-byte old ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_USER_HISTORY_LOG, action) == 0x0du &&
                  offsetof(MSG_USER_HISTORY_LOG, history_length) == 0x10u &&
                  offsetof(MSG_USER_HISTORY_LOG, history_text) == 0x14u,
              "CMD426 user-history input field offsets changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// This semantic user-info provider remains unregistered until the complete
// original PacketDispatcher construction transaction is restored.
class Dispatcher_UserHistoryLog
    : public IPacketDispatcher<MSG_USER_HISTORY_LOG,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85978BE
    // IDA_NAME: _ZN25Dispatcher_UserHistoryLogC2Ev
    // DEMANGLED: Dispatcher_UserHistoryLog::Dispatcher_UserHistoryLog(void)
    Dispatcher_UserHistoryLog();

    // IDA_EA: 0x8244A58
    // IDA_NAME: _ZN25Dispatcher_UserHistoryLogD2Ev
    // DEMANGLED: Dispatcher_UserHistoryLog::~Dispatcher_UserHistoryLog()
    // IDA_EA: 0x8244A88
    // IDA_NAME: _ZN25Dispatcher_UserHistoryLogD0Ev
    // DEMANGLED: Dispatcher_UserHistoryLog::~Dispatcher_UserHistoryLog()
    virtual ~Dispatcher_UserHistoryLog();

    // IDA_EA: 0x81DA5D2
    // IDA_NAME: _ZN25Dispatcher_UserHistoryLog4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_UserHistoryLog::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DA6C0
    // IDA_NAME: _ZN25Dispatcher_UserHistoryLog7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_UserHistoryLog::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UserHistoryLog) == 0x04u,
              "CMD426 dispatcher must retain the four-byte i386 base object");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_PACKET_DISPATCHER_USER_HISTORY_LOG_H_
