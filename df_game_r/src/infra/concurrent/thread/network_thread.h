#ifndef OLDSRV_RUNTIME_NETWORK_THREAD_H_
#define OLDSRV_RUNTIME_NETWORK_THREAD_H_

#include "infra/concurrent/thread/thread_interface.h"

// IDA_EA: 0x857D554 / 0x857D5BE.  The client transport worker owns epoll
// receive/send progress after AcceptThread has initialized a CUser.
class NetworkThread : public ThreadInterface {
public:
    NetworkThread();
    virtual ~NetworkThread();
    virtual void dispatch(void* user_data);
};

#endif  // OLDSRV_RUNTIME_NETWORK_THREAD_H_
