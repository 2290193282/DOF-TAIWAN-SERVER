#ifndef OLDSRV_RUNTIME_MONITOR_TCP_THREAD_H_
#define OLDSRV_RUNTIME_MONITOR_TCP_THREAD_H_

#include "infra/concurrent/thread/thread_interface.h"

// IDA unit: NetworkThread.cpp / MonitorTcpThread.
//
// This worker is intentionally the Hades-only portion of the original
// monitor TCP loop.  Other proxy families from that loop remain owned by
// their separate restorations.
class MonitorTcpThread : public ThreadInterface {
public:
    // IDA_EA: 0x857D8F0
    // IDA_NAME: _ZN16MonitorTcpThreadC2Ev
    MonitorTcpThread();

    // IDA_EA: 0x857D90C
    // IDA_NAME: _ZN16MonitorTcpThreadD2Ev
    // DEMANGLED: MonitorTcpThread::~MonitorTcpThread()
    // IDA_EA: 0x857D93C
    // IDA_NAME: _ZN16MonitorTcpThreadD0Ev
    // DEMANGLED: MonitorTcpThread::~MonitorTcpThread() deleting destructor
    // The virtual destructor emits the D0 ABI wrapper, which calls D2 and
    // then operator delete(void *), exactly as the old-server disassembly.
    virtual ~MonitorTcpThread();

    // IDA_EA: 0x857D95A
    // IDA_NAME: _ZN16MonitorTcpThread8dispatchEPv
    virtual void dispatch(void* user_data);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(MonitorTcpThread) == 0x0cu,
              "MonitorTcpThread must retain the 12-byte ThreadInterface layout");
#endif

#endif  // OLDSRV_RUNTIME_MONITOR_TCP_THREAD_H_
