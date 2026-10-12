#include "infra/security/policy/antibot_business.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stddef.h>
#include <string.h>

namespace {

struct AntibotSendBufferHeader {
    int destination;
    unsigned int serial;
    short length;
};

struct AntibotPunishHeader {
    unsigned int destination;
    unsigned char mode;
    unsigned int punish_info_0;
    unsigned int punish_info_1;
};

static_assert(offsetof(AntibotSendBufferHeader, destination) == 0,
              "IDA 0x80DE348 writes the destination at +0x00");
static_assert(offsetof(AntibotSendBufferHeader, serial) == 4,
              "IDA 0x80DE34E writes the serial at +0x04");
static_assert(offsetof(AntibotSendBufferHeader, length) == 8,
              "IDA 0x80DE355 writes the length at +0x08");
static_assert(sizeof(AntibotSendBufferHeader) == 12,
              "IDA 0x80DE376 writes the 12-byte DPROTO callback header");
static_assert(offsetof(AntibotPunishHeader, destination) == 0,
              "IDA 0x80DE463 writes the destination at +0x00");
static_assert(offsetof(AntibotPunishHeader, mode) == 4,
              "IDA 0x80DE469 writes the punishment mode byte at +0x04");
static_assert(offsetof(AntibotPunishHeader, punish_info_0) == 8,
              "IDA 0x80DE474 copies punish_info[0] at +0x08");
static_assert(offsetof(AntibotPunishHeader, punish_info_1) == 12,
              "IDA 0x80DE477 copies punish_info[1] at +0x0C");
static_assert(sizeof(AntibotPunishHeader) == 16,
              "IDA 0x80DE497 writes the 16-byte punishment callback header");

}  // namespace

// IDA_EA: 0x82A7C24
// IDA_NAME: _ZN9IBusinessC2Ev
// DEMANGLED: IBusiness::IBusiness(void)
IBusiness::IBusiness() {}

// IDA_EA: 0x82A7C32
// IDA_NAME: _ZN13CBusinessImplC2Ev
// DEMANGLED: CBusinessImpl::CBusinessImpl(void)
CBusinessImpl::CBusinessImpl() : IBusiness() {}

// IDA_EA: 0x82A7C4E
// IDA_NAME: _ZN13CBusinessImplD2Ev
// DEMANGLED: CBusinessImpl::~CBusinessImpl()
CBusinessImpl::~CBusinessImpl() {}

// IDA_EA: 0x80DE240
// IDA_NAME: _ZN13CBusinessImpl18SendBuffertoClientEiiijPhs
// DEMANGLED: CBusinessImpl::SendBuffertoClient(int,int,int,unsigned int,unsigned char *,short)
int CBusinessImpl::SendBuffertoClient(int destination,
                                      int unused_1,
                                      int unused_2,
                                      unsigned int serial,
                                      unsigned char* buffer,
                                      short length) {
    const int total_length = static_cast<int>(length) + 20;
    CStreamGuard stream_guard(true);

    if (total_length <= 0xFFF) {
        stream_guard.Set(
            GlobalData::small_stream_pool()->Acquire("BusinessImpl.cpp", 58));
    } else if (total_length <= 0x3FFF) {
        stream_guard.Set(
            GlobalData::stream_pool()->Acquire("BusinessImpl.cpp", 62));
    } else {
        stream_guard.Set(
            GlobalData::big_stream_pool()->Acquire("BusinessImpl.cpp", 66));
    }

    *stream_guard << 0x252;
    *stream_guard << -1;

    AntibotSendBufferHeader header;
    header.destination = destination;
    header.serial = serial;
    header.length = length;
    stream_guard->put_binary(&header, 12);
    stream_guard->put_binary(buffer, static_cast<int>(length));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);

    static_cast<void>(unused_1);
    static_cast<void>(unused_2);
    return 0;
}

// IDA_EA: 0x80DE3EE
// IDA_NAME: _ZN13CBusinessImpl12PunishClientEiij11EPUNISHMODEP20tagAntibotPunishInfoPc
// DEMANGLED: CBusinessImpl::PunishClient(int,int,unsigned int,EPUNISHMODE,tagAntibotPunishInfo *,char *)
int CBusinessImpl::PunishClient(int unused_1,
                                int unused_2,
                                unsigned int destination,
                                EPUNISHMODE mode,
                                tagAntibotPunishInfo* punish_info,
                                char* unused_3) {
    CStreamGuard stream_guard(
        GlobalData::stream_pool()->Acquire("BusinessImpl.cpp", 89), true);
    *stream_guard << 0x253;
    *stream_guard << -1;

    AntibotPunishHeader header;
    header.destination = destination;
    header.mode = static_cast<unsigned char>(static_cast<int>(mode));
    memcpy(&header.punish_info_0, punish_info, sizeof(header.punish_info_0));
    memcpy(&header.punish_info_1,
           reinterpret_cast<const unsigned char*>(punish_info) +
               sizeof(header.punish_info_0),
           sizeof(header.punish_info_1));
    stream_guard->put_binary(&header, 16);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);

    static_cast<void>(unused_1);
    static_cast<void>(unused_2);
    static_cast<void>(unused_3);
    return 0;
}

// IDA_EA: 0x80DE4EC
// IDA_NAME: _ZN13CBusinessImpl17ProcessProtectMsgEP15tagTSafeModePkg
// DEMANGLED: CBusinessImpl::ProcessProtectMsg(tagTSafeModePkg *)
int CBusinessImpl::ProcessProtectMsg(tagTSafeModePkg* message) {
    CStreamGuard stream_guard(
        GlobalData::stream_pool()->Acquire("BusinessImpl.cpp", 136), true);
    *stream_guard << 0x254;
    *stream_guard << -1;
    stream_guard->put_binary(message, 0x34);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
    return 0;
}
