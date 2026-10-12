#ifndef OLDSRV_PROTOCOL_LOADING_TIME_REPORT_DISPATCHER_H_
#define OLDSRV_PROTOCOL_LOADING_TIME_REPORT_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

// CMD 254 sends the raw fixed-size statistic record directly. The original
// constructor initializes only its 10-byte PacketHeader; input-selected body
// regions retain the original stack-object semantics.
#pragma pack(push, 1)
struct Packet_Loading_Time_Report_Statistics {
    PacketHeader header;
    unsigned char raw_0a[0x7FC];

    // IDA_EA: 0x822C040
    // IDA_NAME: _ZN37Packet_Loading_Time_Report_StatisticsC2Ev
    // DEMANGLED: Packet_Loading_Time_Report_Statistics::Packet_Loading_Time_Report_Statistics(void)
    Packet_Loading_Time_Report_Statistics();
};
#pragma pack(pop)

static_assert(sizeof(Packet_Loading_Time_Report_Statistics) == 0x806u,
              "CMD254 loading-time statistic packet must stay 0x806 bytes");
static_assert(offsetof(Packet_Loading_Time_Report_Statistics, raw_0a) == 0x0Au,
              "CMD254 loading-time statistic body offset changed");

// CMD 254 / LODING_TIME_REPORT (the enum spelling is original). This class
// remains an unregistered concrete state-0 provider until PacketDispatcher's
// complete original 606-slot transaction is available.
class Dispatcher_LoadingTimeReport
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596CE8
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReportC2Ev
    // DEMANGLED: Dispatcher_LoadingTimeReport::Dispatcher_LoadingTimeReport(void)
    Dispatcher_LoadingTimeReport();

    // IDA_EA: 0x82410C2
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReportD2Ev
    // DEMANGLED: Dispatcher_LoadingTimeReport::~Dispatcher_LoadingTimeReport()
    // IDA_EA: 0x82410F2
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReportD0Ev
    // DEMANGLED: Dispatcher_LoadingTimeReport::~Dispatcher_LoadingTimeReport()
    virtual ~Dispatcher_LoadingTimeReport();

    // IDA_EA: 0x821EFEC
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReport12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_LoadingTimeReport::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233F06
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReport4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_LoadingTimeReport::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233F10
    // IDA_NAME: _ZN28Dispatcher_LoadingTimeReport7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_LoadingTimeReport::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#endif  // OLDSRV_PROTOCOL_LOADING_TIME_REPORT_DISPATCHER_H_
