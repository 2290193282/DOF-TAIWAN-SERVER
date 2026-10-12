#ifndef OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#pragma pack(push, 1)
struct MSG_INFORM_NOTICE : MSG_BASE {
    // IDA_EA: 0x822E27A.  C2 stores 128 before the C2S reader overwrites it.
    uint32_t flag_index;

    // IDA_NAME: _ZN17MSG_INFORM_NOTICEC2Ev
    // DEMANGLED: MSG_INFORM_NOTICE::MSG_INFORM_NOTICE(void)
    MSG_INFORM_NOTICE();
};
#pragma pack(pop)

static_assert(sizeof(MSG_INFORM_NOTICE) == 0x11u,
              "CMD478 message must retain its 17-byte ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(MSG_INFORM_NOTICE, flag_index) == 0x0du,
              "CMD478 flag index offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

class InformNoticeData : public ParamBase {
public:
    // IDA_EA: 0x8231BCA
    // IDA_NAME: _ZN16InformNoticeDataC2Ev
    // DEMANGLED: InformNoticeData::InformNoticeData(void)
    InformNoticeData();

    // IDA_EA: 0x8246818 / 0x8246848
    // IDA_NAME: _ZN16InformNoticeDataD2Ev / _ZN16InformNoticeDataD0Ev
    // DEMANGLED: InformNoticeData::~InformNoticeData()
    virtual ~InformNoticeData();

    int result;
};

class Dispatcher_InformNotice
    : public IPacketDispatcher<MSG_INFORM_NOTICE,
                               InformNoticeData,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597CE6
    // IDA_NAME: _ZN23Dispatcher_InformNoticeC2Ev
    // DEMANGLED: Dispatcher_InformNotice::Dispatcher_InformNotice(void)
    Dispatcher_InformNotice();

    // IDA_EA: 0x8244098 / 0x82440C8
    // IDA_NAME: _ZN23Dispatcher_InformNoticeD2Ev /
    //           _ZN23Dispatcher_InformNoticeD0Ev
    // DEMANGLED: Dispatcher_InformNotice::~Dispatcher_InformNotice()
    virtual ~Dispatcher_InformNotice();

    // IDA_EA: 0x81E2F22
    // IDA_NAME: _ZN23Dispatcher_InformNotice4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E2F78
    // IDA_NAME: _ZN23Dispatcher_InformNotice7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E3056
    // IDA_NAME: _ZN23Dispatcher_InformNotice4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E3008
    // IDA_NAME: _ZN23Dispatcher_InformNotice11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(InformNoticeData) == 0x08u,
              "CMD478 parameter must retain its eight-byte i386 ABI");
static_assert(sizeof(Dispatcher_InformNotice) == 0x04u,
              "CMD478 dispatcher must retain its four-byte i386 ABI");
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
static_assert(offsetof(InformNoticeData, result) == 0x04u,
              "CMD478 result offset changed");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_INFORM_NOTICE_DISPATCHER_H_
