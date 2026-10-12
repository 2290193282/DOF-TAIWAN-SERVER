#include "infra/concurrent/thread/network_thread.h"

#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/clog.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/transport/epoll_handler.h"
#include "infra/concurrent/thread/guard_mutex.h"
#include "infra/network/transport/network_endpoint.h"
#include "common/manager/os_api.h"

// IDA_EA: 0x857D554
// IDA_NAME: _ZN13NetworkThreadC2Ev
NetworkThread::NetworkThread() : ThreadInterface() {}

// IDA_EA: 0x857D570 / 0x857D5A0
// IDA_NAME: _ZN13NetworkThreadD2Ev / _ZN13NetworkThreadD0Ev
NetworkThread::~NetworkThread() {}

// IDA_EA: 0x857D5BE
// IDA_NAME: _ZN13NetworkThread8dispatchEPv
void NetworkThread::dispatch(void*) {
    EpollHandler* const epoll = G_EpollHandler();
    epoll_event* const events = epoll->GetEpollEvents();
    const int epoll_fd = epoll->GetEpollFD();

    while (!GlobalData::server_stop()) {
        errno = 0;
        {
            Guard<Mutex> guard(&GlobalData::s_accept_lock);
            while (!GlobalData::s_accept_que.empty()) {
                CUser* const user = GlobalData::s_accept_que.front();
                GlobalData::s_accept_que.pop();
                const int error = epoll->SetEpoll(
                    user, user->network_.get_socket(), false);
                if (error) {
                    LogManager::logFormat(
                        1, "NetworkThread.cpp",
                        "virtual void NetworkThread::dispatch(void*)", 0x2c4,
                        "(m_id: %s) G_EpollHandler()->SetEpoll(pUser->get_socket(%d)) %d(%s)",
                        NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
                        user->network_.get_socket(), error, std::strerror(error));
                }
            }
        }

        const int event_count = epoll_wait(epoll_fd, events, 600, 100);
        if (event_count < 0) {
            if (errno != EINTR && errno != 0) {
                return;
            }
            continue;
        }
        for (int index = 0; index < event_count; ++index) {
            CUser* const user = static_cast<CUser*>(events[index].data.ptr);
            if (!user) continue;
            const uint32_t flags = events[index].events;
            if (flags & EPOLLIN) user->Recv();
            if (user->network_.GetRemainSendSize() && (flags & EPOLLOUT)) {
                if (static_cast<unsigned int>(user->network_.GetRemainSendSize()) <=
                    0x6ddd0u) {
                    user->network_.send_packet();
                } else {
                    user->DisConnSig(static_cast<DISCONN_SIG>(6), false, 0);
                }
            }
            if (flags & (EPOLLERR | EPOLLHUP)) {
                user->DisConnSig(static_cast<DISCONN_SIG>(9), false, 0);
            }
        }
    }
}
