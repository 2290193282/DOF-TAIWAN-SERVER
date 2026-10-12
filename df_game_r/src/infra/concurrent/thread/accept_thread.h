#ifndef OLDSRV_RUNTIME_ACCEPT_THREAD_H_
#define OLDSRV_RUNTIME_ACCEPT_THREAD_H_

#include "infra/concurrent/thread/thread_interface.h"


// IDA_EA: 0x857CBE8 / 0x857CC5A.  This worker owns the TCP listener and
// transfers only fully initialized CUser instances to NetworkThread.
class AcceptThread : public ThreadInterface {
public:
    AcceptThread();
    virtual ~AcceptThread();
    virtual void dispatch(void* user_data);

private:
    bool accept_locked_;  // old +0x0c: DB queue high/low-water state.
};

#if defined(__i386__)
static_assert(sizeof(AcceptThread) == 0x10u,
              "AcceptThread old i386 layout must stay 0x10");
#endif

#endif  // OLDSRV_RUNTIME_ACCEPT_THREAD_H_
