#ifndef OLDSRV_RUNTIME_EPOLL_HANDLER_H_
#define OLDSRV_RUNTIME_EPOLL_HANDLER_H_

#include "infra/concurrent/thread/mutex.h"

#include <stdint.h>
#include <sys/epoll.h>


// IDA layout for EpollHandler:
//   +0x00 temporary epoll_event storage used by ResetEpoll
//   +0x0C epoll event array pointer
//   +0x10 epoll fd returned by epoll_create
//   +0x14 Mutex protecting epoll_ctl
class EpollHandler {
public:
    EpollHandler();
    ~EpollHandler();

    int GetEpollFD() const;
    epoll_event* GetEpollEvents() const;
    EpollHandler* destroy();
    int init();
    void release();
    int SetEpoll(void* user_data, int fd, bool edge_triggered);
    int ResetEpoll(int fd);

private:
    epoll_event reset_event_;
    epoll_event* epoll_events_;
    int epoll_fd_;
    Mutex mutex_;
};

EpollHandler* G_EpollHandler();


#endif  // OLDSRV_RUNTIME_EPOLL_HANDLER_H_
