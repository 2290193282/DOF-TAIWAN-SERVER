#ifndef OLDSRV_RUNTIME_THREAD_INTERFACE_H_
#define OLDSRV_RUNTIME_THREAD_INTERFACE_H_

#include <pthread.h>


// IDA class layout (i386): vptr +0, pthread_t +4, saved thread id +8.
// vtable off_8CE6B98 @ 0x8CE6B98: stop, D2, D0, dispatch.
class ThreadInterface {
public:
    // IDA_EA: 0x8630C14
    // IDA_NAME: _ZN15ThreadInterface4stopEv
    // DEMANGLED: ThreadInterface::stop(void)
    virtual void stop();

    // IDA_EA: 0x8630B46 (D2), 0x8630B6C (D0)
    // IDA_NAME: _ZN15ThreadInterfaceD2Ev / _ZN15ThreadInterfaceD0Ev
    // DEMANGLED: ThreadInterface::~ThreadInterface()
    virtual ~ThreadInterface();

    // IDA vtable slot +0x0C is a pure virtual call from dispatch_proxy.
    virtual void dispatch(void* user_data) = 0;

    // IDA_EA: 0x8630B8A
    // IDA_NAME: _ZN15ThreadInterface5beginEv
    // DEMANGLED: ThreadInterface::begin(void)
    int begin();

    // IDA_EA: 0x8630C46
    // IDA_NAME: _ZN15ThreadInterface4joinEv
    // DEMANGLED: ThreadInterface::join(void)
    bool join();

    // IDA_EA: 0x82A4218
    // IDA_NAME: _ZN15ThreadInterface11getThreadIDEv
    // DEMANGLED: ThreadInterface::getThreadID(void)
    int getThreadID();

protected:
    // IDA_EA: 0x8630B38
    // IDA_NAME: _ZN15ThreadInterfaceC2Ev
    // DEMANGLED: ThreadInterface::ThreadInterface(void)
    ThreadInterface();

private:
    // IDA_EA: 0x8630C1A
    // IDA_NAME: _ZN15ThreadInterface14dispatch_proxyEPv
    // DEMANGLED: ThreadInterface::dispatch_proxy(void *)
    static void* dispatch_proxy(void* user_data);

    pthread_t thread_;
    int thread_id_;
};


#endif  // OLDSRV_RUNTIME_THREAD_INTERFACE_H_
