#ifndef OLDSRV_RUNTIME_UDP_THREAD_H_
#define OLDSRV_RUNTIME_UDP_THREAD_H_

#include <stdint.h>

#include "infra/concurrent/thread/thread_interface.h"

// IDA unit: 01_network_crypto_antibot/UdpThread.
//
// The original worker owns the process UDP listen socket.  This restored
// subset deliberately delivers only the already-restored Hades FE6/FE7
// server frames; all other original CDispatchServerPacket routes remain out
// of scope until their concrete router is restored.
class UdpThread : public ThreadInterface {
public:
    // IDA_EA: 0x857C354
    // IDA_NAME: _ZN9UdpThreadC2Ev
    UdpThread();

    // IDA_EA: 0x857C370
    // IDA_NAME: _ZN9UdpThreadD2Ev
    // DEMANGLED: UdpThread::~UdpThread()
    // IDA_EA: 0x857C3A0
    // IDA_NAME: _ZN9UdpThreadD0Ev
    // DEMANGLED: UdpThread::~UdpThread() deleting destructor
    // The virtual destructor emits the D0 ABI wrapper, which calls D2 and
    // then operator delete(void *), exactly as the old-server disassembly.
    virtual ~UdpThread();

    // IDA_EA: 0x857C9DE
    // IDA_NAME: _ZN9UdpThread8dispatchEPv
    virtual void dispatch(void* user_data);

private:
    // IDA_EA: 0x857C3BE
    // IDA_NAME: _ZN9UdpThread14_bindUDPSocketEt
    int _bindUDPSocket(uint16_t port);

    // IDA_EA: 0x857C5A4.  This preserves the UDP receive boundary but is
    // intentionally Hades-only; it is not a substitute for the generic
    // CDispatchServerPacket router.
    void _processUDPData(char* packet, int received_size);
};

#endif  // OLDSRV_RUNTIME_UDP_THREAD_H_
