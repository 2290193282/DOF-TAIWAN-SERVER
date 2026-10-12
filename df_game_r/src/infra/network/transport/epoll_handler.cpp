#include "infra/network/transport/epoll_handler.h"

#include "common/manager/global_instance.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>


// IDA_EA: 0x848FF64
// IDA_NAME: _ZN12EpollHandler10GetEpollFDEv
// DEMANGLED: EpollHandler::GetEpollFD(void)
int EpollHandler::GetEpollFD() const {
    return epoll_fd_;
}

// IDA_EA: 0x848FF70
// IDA_NAME: _ZN12EpollHandler14GetEpollEventsEv
// DEMANGLED: EpollHandler::GetEpollEvents(void)
epoll_event* EpollHandler::GetEpollEvents() const {
    return epoll_events_;
}

// IDA_EA: 0x857C084
// IDA_NAME: _ZN12EpollHandlerC2Ev
// DEMANGLED: EpollHandler::EpollHandler(void)
EpollHandler::EpollHandler()
    : reset_event_(), epoll_events_(0), epoll_fd_(-1), mutex_() {
    init();
}

// IDA_EA: 0x857C0CC
// IDA_NAME: _ZN12EpollHandlerD2Ev
// DEMANGLED: EpollHandler::~EpollHandler()
EpollHandler::~EpollHandler() {
    destroy();
    release();
}

// IDA_EA: 0x857C0F8
// IDA_NAME: _ZN12EpollHandler7destroyEv
// DEMANGLED: EpollHandler::destroy(void)
EpollHandler* EpollHandler::destroy() {
    if (epoll_events_ != 0) {
        delete[] epoll_events_;
    }
    epoll_events_ = 0;
    return this;
}

// IDA_EA: 0x857C122
// IDA_NAME: _ZN12EpollHandler4initEv
// DEMANGLED: EpollHandler::init(void)
int EpollHandler::init() {
    epoll_fd_ = epoll_create(600);
    if (epoll_fd_ < 0) {
        std::cerr << "[Epoll::init] Can't init epoll create" << std::endl;
        return 0;
    }

    epoll_events_ = new epoll_event[600];
    if (epoll_events_ == 0) {
        std::cerr << "[Epoll::init] Can't alloc event memory" << std::endl;
        return 0;
    }
    return 1;
}

// IDA_EA: 0x857C1C0
// IDA_NAME: _ZN12EpollHandler7releaseEv
// DEMANGLED: EpollHandler::release(void)
void EpollHandler::release() {}

// IDA_EA: 0x857C1C6
// IDA_NAME: _ZN12EpollHandler8SetEpollEPvib
// DEMANGLED: EpollHandler::SetEpoll(void *,int,bool)
int EpollHandler::SetEpoll(void* user_data, int fd, bool edge_triggered) {
    epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.events = edge_triggered ? 0x8000001Du : 29u;
    event.data.ptr = user_data;

    MutexGuard guard(&mutex_);
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) >= 0) {
        return 0;
    }
    return errno;
}

// IDA_EA: 0x857C258
// IDA_NAME: _ZN12EpollHandler10ResetEpollEi
// DEMANGLED: EpollHandler::ResetEpoll(int)
int EpollHandler::ResetEpoll(int fd) {
    std::memset(&reset_event_, 0, sizeof(reset_event_));
    reset_event_.events = EPOLLIN;

    MutexGuard guard(&mutex_);
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &reset_event_) >= 0) {
        return 0;
    }
    return errno;
}

// IDA_EA: 0x82A70AB
// IDA_NAME: _Z14G_EpollHandlerv
// DEMANGLED: G_EpollHandler(void)
EpollHandler* G_EpollHandler() {
    return GlobalInstance<EpollHandler>::inst_ptr();
}
