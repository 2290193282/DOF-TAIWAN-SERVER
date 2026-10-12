#ifndef OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_2ND_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_2ND_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#pragma pack(push, 1)
struct MSG_INFORM_NOTICE_2ND : MSG_BASE {
    // IDA C2 writes 128 before the dispatcher reads the C2S field.
    uint32_t flag_index;

    // IDA_EA: 0x822E298
    // IDA_NAME: _ZN21MSG_INFORM_NOTICE_2NDC2Ev
    // DEMANGLED: MSG_INFORM_NOTICE_2ND::MSG_INFORM_NOTICE_2ND(void)
    MSG_INFORM_NOTICE_2ND();
};
#pragma pack(pop)

static_assert(sizeof(MSG_INFORM_NOTICE_2ND) == 0x11u,
              "CMD504 message must retain its 17-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_INFORM_NOTICE_2ND, flag_index) == 0x0du,
              "CMD504 flag offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class InformNoticeData2nd : public ParamBase {
public:
    // IDA_EA: 0x8231BF0
    // IDA_NAME: _ZN19InformNoticeData2ndC2Ev
    // DEMANGLED: InformNoticeData2nd::InformNoticeData2nd(void)
    InformNoticeData2nd();

    // IDA_EA: 0x82467CA
    // IDA_NAME: _ZN19InformNoticeData2ndD2Ev
    // DEMANGLED: InformNoticeData2nd::~InformNoticeData2nd()
    // IDA_EA: 0x82467FA
    // IDA_NAME: _ZN19InformNoticeData2ndD0Ev
    // DEMANGLED: InformNoticeData2nd::~InformNoticeData2nd()
    virtual ~InformNoticeData2nd();

    int result;
};

class Dispatcher_InformNotice_2nd
    : public IPacketDispatcher<MSG_INFORM_NOTICE_2ND,
                               InformNoticeData2nd,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597D02
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2ndC2Ev
    // DEMANGLED: Dispatcher_InformNotice_2nd::Dispatcher_InformNotice_2nd(void)
    Dispatcher_InformNotice_2nd();

    // IDA_EA: 0x824404A
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2ndD2Ev
    // DEMANGLED: Dispatcher_InformNotice_2nd::~Dispatcher_InformNotice_2nd()
    // IDA_EA: 0x824407A
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2ndD0Ev
    // DEMANGLED: Dispatcher_InformNotice_2nd::~Dispatcher_InformNotice_2nd()
    virtual ~Dispatcher_InformNotice_2nd();

    // IDA_EA: 0x81E30A4
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2nd4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_InformNotice_2nd::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E30FA
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2nd7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_InformNotice_2nd::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E31F4
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2nd4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_InformNotice_2nd::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E31A6
    // IDA_NAME: _ZN27Dispatcher_InformNotice_2nd11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_InformNotice_2nd::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(InformNoticeData2nd) == 0x08u,
              "CMD504 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(Dispatcher_InformNotice_2nd) == 0x04u,
              "CMD504 dispatcher must retain its four-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(InformNoticeData2nd, result) == 0x04u,
              "CMD504 result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_2ND_DISPATCHER_H_
