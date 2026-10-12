#include "infra/concurrent/thread/monitor_tcp_thread.h"

#include <sys/select.h>
#include <sys/socket.h>

#include "common/manager/global_data.h"
#include "infra/security/policy/hades_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/transport/network_endpoint.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/world/region/game_world.h"

namespace {

const char kMonitorDispatchSignature[] =
    "virtual void MonitorTcpThread::dispatch(void*)";

bool selectable_fd(int socket_fd) {
    // The original i386 binary only excludes -1.  select(2)'s fd_set is a
    // fixed-size object, so preserve every valid original descriptor while
    // preventing an out-of-range descriptor from corrupting the worker stack.
    return socket_fd >= 0 && socket_fd < FD_SETSIZE;
}

template <typename AuctionProxy>
void add_auction_socket(AuctionProxy* proxy,
                        fd_set* read_fds,
                        fd_set* write_fds,
                        int* max_fd) {
    CNetwork100000_100000* const handler =
        proxy ? proxy->GetTcpHandler() : 0;
    const int socket_fd = handler ? handler->get_socket() : -1;
    if (!selectable_fd(socket_fd)) {
        return;
    }

    FD_SET(socket_fd, read_fds);
    FD_SET(socket_fd, write_fds);
    if (socket_fd > *max_fd) {
        *max_fd = socket_fd;
    }
}

template <typename AuctionProxy>
void service_auction_proxy(AuctionProxy* proxy,
                           fd_set* read_fds,
                           fd_set* write_fds,
                           int* last_connect_second,
                           const char* service_name) {
    if (!proxy) {
        return;
    }

    CNetwork100000_100000* handler = proxy->GetTcpHandler();
    int socket_fd = handler ? handler->get_socket() : -1;
    if (selectable_fd(socket_fd) &&
        proxy->GetSocketState() == BaseServerProxy::SOCKET_STATE_CONNECTED &&
        FD_ISSET(socket_fd, read_fds) && proxy->OnTcpRecv() != 1) {
        LogManager::logFormat(
            1,
            "NetworkThread.cpp",
            kMonitorDispatchSignature,
            0x734,
            "%s - OnTcpRecv == false!!!",
            service_name);
    }

    // OnTcpRecv may disconnect and replace the descriptor.  Re-read it before
    // the writable half, matching the original MonitorTcpThread discipline.
    handler = proxy->GetTcpHandler();
    socket_fd = handler ? handler->get_socket() : -1;
    if (selectable_fd(socket_fd) && FD_ISSET(socket_fd, write_fds)) {
        if (proxy->GetSocketState() ==
            BaseServerProxy::SOCKET_STATE_CONNECTING) {
            int socket_error = 0;
            socklen_t option_length = sizeof(socket_error);
            if (getsockopt(socket_fd,
                           SOL_SOCKET,
                           SO_ERROR,
                           &socket_error,
                           &option_length) == 0 &&
                socket_error == 0) {
                proxy->SetSocketState(
                    BaseServerProxy::SOCKET_STATE_CONNECTED);
                // Registration is queued only after the non-blocking connect
                // has completed.  Packet zero makes the auction service mark
                // this game channel ready and emit the running notification.
                proxy->SendRegistPacket();
            }
        }

        const int send_result = proxy->OnTcpSend();
        if (send_result < 0) {
            LogManager::logFormat(
                1,
                "NetworkThread.cpp",
                kMonitorDispatchSignature,
                0x74C,
                "%s - OnTcpSend == false!!!remain_size(%d)",
                service_name,
                send_result);
        }
    }

    const int now = GlobalData::s_systemTime_.getCurSec();
    if (proxy->GetSocketState() ==
        BaseServerProxy::SOCKET_STATE_CONNECTING) {
        if (now - *last_connect_second > 60) {
            *last_connect_second = now;
            cMyTrace trace(kMonitorDispatchSignature, 1889, 0);
            trace("%s - Disconnect (timeout while connecting)", service_name);
            proxy->Disconnect();
        }
        return;
    }

    if (proxy->GetSocketState() ==
            BaseServerProxy::SOCKET_STATE_DISCONNECTED &&
        now - *last_connect_second > 60) {
        cMyTrace trace(kMonitorDispatchSignature, 1876, 0);
        trace("%s - invoke InitTcp() after timeout or disconnect",
              service_name);
        proxy->InitTcp(true);
        proxy->SetSocketState(BaseServerProxy::SOCKET_STATE_CONNECTING);
        *last_connect_second = now;
    }
}

}  // namespace

// IDA_EA: 0x857D8F0
// IDA_NAME: _ZN16MonitorTcpThreadC2Ev
// DEMANGLED: MonitorTcpThread::MonitorTcpThread(void)
MonitorTcpThread::MonitorTcpThread() : ThreadInterface() {}

// IDA_EA: 0x857D90C
// IDA_NAME: _ZN16MonitorTcpThreadD2Ev
// DEMANGLED: MonitorTcpThread::~MonitorTcpThread()
// IDA_EA: 0x857D93C
// IDA_NAME: _ZN16MonitorTcpThreadD0Ev
// DEMANGLED: MonitorTcpThread::~MonitorTcpThread() deleting destructor
// The virtual destructor's compiler-emitted D0 wrapper invokes this D2 body
// and operator delete(void *), matching IDA 0x857D948 and 0x857D953.
MonitorTcpThread::~MonitorTcpThread() {}

// IDA_EA: 0x857D95A
// IDA_NAME: _ZN16MonitorTcpThread8dispatchEPv
// DEMANGLED: MonitorTcpThread::dispatch(void *)
void MonitorTcpThread::dispatch(void*) {
    // IDA locals CurSecEv_6/CurSecEv_9: the two auction transports retry
    // independently.  Zero makes the first loop connect immediately because
    // the current epoch is already greater than the original 60-second gate.
    int auction_last_connect_second = 0;
    int cera_auction_last_connect_second = 0;

    while (!GlobalData::server_stop()) {
        // The raw loop yields for one millisecond before every select pass.
        OS_API::Sleep(1);

        // IDA 0x857D95A walks the manager's physical proxy array.  Do not
        // substitute a generic router: each configured monitor endpoint owns
        // its own TCP receive/send state and must be present in this select
        // pass.
        CServerProxyMgr<CMonitorServerProxy>* const monitor_proxy_mgr =
            GlobalData::s_monitor_proxy_mgr;
        CServerProxyMgr<CGuildServerProxy>* const guild_proxy_mgr =
            GlobalData::s_guild_proxy_mgr;
        CHadesServerProxy* const hades_proxy = GlobalData::s_hades_proxy;
        CAuctionServerProxy* const auction_proxy =
            GlobalData::s_auction_proxy;
        CCeraAuctionServerProxy* const cera_auction_proxy =
            GlobalData::s_cera_auction_proxy;
        GameWorld* const game_world = G_GameWorld();
        const bool use_auction_transports =
            !game_world || !game_world->IsIntegratedPvPBaseChannel();

        fd_set read_fds;
        fd_set write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        // Raw MonitorTcpThread initializes nfds to zero, yielding
        // select(1, empty sets, ...) when every endpoint is disconnected.
        int max_fd = 0;
        if (monitor_proxy_mgr) {
            const int monitor_count =
                monitor_proxy_mgr->GetServerProxyArraySize();
            for (int index = 0; index < monitor_count; ++index) {
                CMonitorServerProxy* const proxy =
                    monitor_proxy_mgr->GetServerProxyArrayAt(index);
                CNetwork100000_100000* const tcp_handler =
                    proxy ? proxy->GetTcpHandler() : 0;
                const int socket_fd =
                    tcp_handler ? tcp_handler->get_socket() : -1;
                if (selectable_fd(socket_fd)) {
                    FD_SET(socket_fd, &read_fds);
                    FD_SET(socket_fd, &write_fds);
                    if (socket_fd > max_fd) {
                        max_fd = socket_fd;
                    }
                }

                if (guild_proxy_mgr) {
                    CGuildServerProxy* const guild_proxy =
                        guild_proxy_mgr->GetServerProxyArrayAt(index);
                    CNetwork100000_100000* const guild_tcp_handler =
                        guild_proxy ? guild_proxy->GetTcpHandler() : 0;
                    const int guild_socket_fd =
                        guild_tcp_handler ? guild_tcp_handler->get_socket() : -1;
                    if (selectable_fd(guild_socket_fd)) {
                        FD_SET(guild_socket_fd, &read_fds);
                        FD_SET(guild_socket_fd, &write_fds);
                        if (guild_socket_fd > max_fd) {
                            max_fd = guild_socket_fd;
                        }
                    }
                }
            }
        }

        CNetwork100000_100000* const hades_tcp_handler =
            hades_proxy ? hades_proxy->GetTcpHandler() : 0;
        const int hades_socket_fd =
            hades_tcp_handler ? hades_tcp_handler->get_socket() : -1;
        const bool has_selectable_hades_socket = selectable_fd(hades_socket_fd);
        if (has_selectable_hades_socket) {
            FD_SET(hades_socket_fd, &read_fds);
            FD_SET(hades_socket_fd, &write_fds);
            if (hades_socket_fd > max_fd) {
                max_fd = hades_socket_fd;
            }
        }

        if (use_auction_transports) {
            add_auction_socket(
                auction_proxy, &read_fds, &write_fds, &max_fd);
            add_auction_socket(
                cera_auction_proxy, &read_fds, &write_fds, &max_fd);
        }

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        const int select_result =
            select(max_fd + 1, &read_fds, &write_fds, 0, &timeout);
        if (select_result < 0) {
            continue;
        }

        // IDA 0x857D95A runs a second monitor traversal after select.
        // Fetch the handler again before the write half: OnTcpRecv can call
        // Disconnect(), which changes the descriptor and must suppress the
        // stale write event exactly as the raw code does.
        if (monitor_proxy_mgr) {
            const int monitor_count =
                monitor_proxy_mgr->GetServerProxyArraySize();
            for (int index = 0; index < monitor_count; ++index) {
                CMonitorServerProxy* const read_proxy =
                    monitor_proxy_mgr->GetServerProxyArrayAt(index);
                CNetwork100000_100000* const read_handler =
                    read_proxy ? read_proxy->GetTcpHandler() : 0;
                const int read_socket_fd =
                    read_handler ? read_handler->get_socket() : -1;
                if (selectable_fd(read_socket_fd) &&
                    FD_ISSET(read_socket_fd, &read_fds) &&
                    read_proxy->OnTcpRecv() != 1) {
                    LogManager::logFormat(1,
                                          "NetworkThread.cpp",
                                          kMonitorDispatchSignature,
                                          0x449,
                                          "Monitor::OnTcpRecv == false!!!");
                    CStreamGuard guard(
                        GlobalData::s_stream_pool->Acquire(
                            "NetworkThread.cpp", 1100),
                        true);
                    guard << 351 << -1 << 201;
                    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q,
                                                 guard);
                }

                CMonitorServerProxy* const write_proxy =
                    monitor_proxy_mgr->GetServerProxyArrayAt(index);
                CNetwork100000_100000* const write_handler =
                    write_proxy ? write_proxy->GetTcpHandler() : 0;
                const int write_socket_fd =
                    write_handler ? write_handler->get_socket() : -1;
                if (selectable_fd(write_socket_fd) &&
                    FD_ISSET(write_socket_fd, &write_fds)) {
                    const int result = write_proxy->OnTcpSend();
                    if (result < 0) {
                        LogManager::logFormat(
                            1,
                            "NetworkThread.cpp",
                            kMonitorDispatchSignature,
                            0x45E,
                            "Monitor::OnTcpSend == false!!!remain_size(%d)",
                            result);
                        CStreamGuard guard(
                            GlobalData::s_stream_pool->Acquire(
                                "NetworkThread.cpp", 1120),
                            true);
                        guard << 351 << -1 << 201;
                        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q,
                                                     guard);
                    }
                }

                if (guild_proxy_mgr) {
                    CGuildServerProxy* const guild_read_proxy =
                        guild_proxy_mgr->GetServerProxyArrayAt(index);
                    CNetwork100000_100000* const guild_read_handler =
                        guild_read_proxy ? guild_read_proxy->GetTcpHandler() : 0;
                    const int guild_read_socket_fd = guild_read_handler
                                                         ? guild_read_handler->get_socket()
                                                         : -1;
                    if (selectable_fd(guild_read_socket_fd) &&
                        FD_ISSET(guild_read_socket_fd, &read_fds) &&
                        guild_read_proxy->OnTcpRecv() != 1) {
                        LogManager::logFormat(1,
                                              "NetworkThread.cpp",
                                              kMonitorDispatchSignature,
                                              0x46F,
                                              "Guild::OnTcpRecv == false!!!");
                        CStreamGuard guard(
                            GlobalData::s_stream_pool->Acquire(
                                "NetworkThread.cpp", 1137),
                            true);
                        guard << 351 << -1 << 203;
                        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q,
                                                     guard);
                    }

                    CGuildServerProxy* const guild_write_proxy =
                        guild_proxy_mgr->GetServerProxyArrayAt(index);
                    CNetwork100000_100000* const guild_write_handler =
                        guild_write_proxy ? guild_write_proxy->GetTcpHandler() : 0;
                    const int guild_write_socket_fd = guild_write_handler
                                                          ? guild_write_handler->get_socket()
                                                          : -1;
                    if (selectable_fd(guild_write_socket_fd) &&
                        FD_ISSET(guild_write_socket_fd, &write_fds)) {
                        const int result = guild_write_proxy->OnTcpSend();
                        if (result < 0) {
                            LogManager::logFormat(
                                1,
                                "NetworkThread.cpp",
                                kMonitorDispatchSignature,
                                0x480,
                                "Guild::OnTcpSend == false!!!remain_size(%d)",
                                result);
                            CStreamGuard guard(
                                GlobalData::s_stream_pool->Acquire(
                                    "NetworkThread.cpp", 1153),
                                true);
                            guard << 351 << -1 << 203;
                            GlobalData::s_msgq_mgr->put(
                                MsgQueueMgr::DISPATCH_Q, guard);
                        }
                    }
                }
            }
        }

        // Raw MonitorTcpThread gates Hades receives on state 2, but attempts
        // the pending send path whenever the descriptor is writable.
        if (has_selectable_hades_socket && hades_proxy &&
            hades_proxy->GetSocketState() ==
                BaseServerProxy::SOCKET_STATE_CONNECTED &&
            FD_ISSET(hades_socket_fd, &read_fds)) {
            if (hades_proxy->OnTcpRecv() != 1) {
                LogManager::logFormat(1,
                                      "NetworkThread.cpp",
                                      kMonitorDispatchSignature,
                                      0x493,
                                      "HADES::OnTcpRecv == false!!!");
            }
        }

        if (has_selectable_hades_socket && hades_proxy &&
            FD_ISSET(hades_socket_fd, &write_fds)) {
            const int result = hades_proxy->OnTcpSend();
            if (result < 0) {
                LogManager::logFormat(
                    1,
                    "NetworkThread.cpp",
                    kMonitorDispatchSignature,
                    0x49D,
                    "HADES::OnTcpSend == false!!!remain_size(%d)",
                    result);
            }
        }


        if (use_auction_transports) {
            service_auction_proxy(auction_proxy,
                                  &read_fds,
                                  &write_fds,
                                  &auction_last_connect_second,
                                  "AUCTION");
            service_auction_proxy(cera_auction_proxy,
                                  &read_fds,
                                  &write_fds,
                                  &cera_auction_last_connect_second,
                                  "Cera AUCTION");
        }
    }
}
