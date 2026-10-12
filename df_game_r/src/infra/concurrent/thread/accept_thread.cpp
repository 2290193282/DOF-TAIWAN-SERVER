#include "infra/concurrent/thread/accept_thread.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "core/lifecycle/serve/game_server_demon_manager.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/transport/network_endpoint.h"

namespace {

const char kAcceptThreadSignature[] = "virtual void AcceptThread::dispatch(void*)";

// AcceptThread emits this exact CMD1 reject frame on a prohibited connection,
// a saturated user pool, and a DB-queue admission lock.  The byte sequence is
// constructed through PacketGuard so it follows the same finalize/encryption
// logic as the old function rather than treating it as a raw literal.
void RejectAcceptedSocket(int socket_fd) {
    PacketGuard packet;
    packet.put_header(1, 1);
    packet.put_byte(0);
    packet.put_byte(4);
    packet.finalize(true);
    write(socket_fd, packet.get_packet(0), packet.get_len());
    close(socket_fd);
}

}  // namespace

// IDA_EA: 0x857CBE8
// IDA_NAME: _ZN12AcceptThreadC2Ev
// DEMANGLED: AcceptThread::AcceptThread(void)
AcceptThread::AcceptThread() : ThreadInterface(), accept_locked_(false) {}

// IDA_EA: 0x857CC0C / 0x857CC3C
// IDA_NAME: _ZN12AcceptThreadD2Ev / _ZN12AcceptThreadD0Ev
AcceptThread::~AcceptThread() {}

// IDA_EA: 0x857CC5A
// IDA_NAME: _ZN12AcceptThread8dispatchEPv
// DEMANGLED: AcceptThread::dispatch(void *)
void AcceptThread::dispatch(void*) {
    const uint16_t tcp_port = static_cast<uint16_t>(
        G_CEnvironment()->get_tcp_port());
    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                              0x18e,
                              "[AcceptThread::dispatch] Fail to Create ListenSocket.");
        return;
    }

    sockaddr_in listen_address;
    std::memset(&listen_address, 0, sizeof(listen_address));
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_address.sin_port = htons(tcp_port);

    // IDA_EA: 0x857CCE9..0x857CD04.  The original passes optname 2
    // (SO_REUSEADDR) with its non-zero 60000 local, not SO_RCVBUF.  This
    // permits the channel listener to be rebound after a controlled restart
    // while prior client sockets finish their TCP shutdown.
    int reuse_address = 60000;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse_address,
                   sizeof(reuse_address)) < 0) {
        LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                              0x19b,
                              "[Error] listen socket setsockopt error");
        close(listener);
        return;
    }
    if (bind(listener, reinterpret_cast<sockaddr*>(&listen_address),
             sizeof(listen_address)) < 0) {
        LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                              0x1a1, "Fail to bind Listen - %s",
                              std::strerror(errno));
        close(listener);
        return;
    }
    if (listen(listener, 1024) < 0) {
        LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                              0x1a7, "Fail to Create AcceptThread.");
        close(listener);
        return;
    }
    if (!CNetwork4096_450000::SetNonBlock(listener)) {
        LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                              0x1ae, "Fail to SetNonBlock AcceptThread.");
        close(listener);
        return;
    }

    // Temporary 0627 interoperability trace: record endpoint readiness, but
    // never emit packet bytes or cipher material.
    LogManager::logFormat(1, "accept_diag.cpp", kAcceptThreadSignature, 0,
                          "[DIAG0627][LISTEN_READY] tcp_port=%u",
                          static_cast<unsigned int>(tcp_port));

    while (!G_CEnvironment()->get_suspend_state()) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listener, &read_fds);
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        const int select_result = select(listener + 1, &read_fds, 0, 0,
                                         &timeout);
        if (select_result < 0 || !FD_ISSET(listener, &read_fds)) {
            continue;
        }

        sockaddr_in peer_address;
        socklen_t peer_address_size = sizeof(peer_address);
        const int accepted = accept(listener,
                                    reinterpret_cast<sockaddr*>(&peer_address),
                                    &peer_address_size);
        if (accepted < 0) {
            LogManager::logFormat(1, "NetworkThread.cpp", kAcceptThreadSignature,
                                  0x1d4, "Accept Socket[%d] Error(%s)",
                                  accepted, std::strerror(errno));
            continue;
        }

        const char* const peer_ip = inet_ntoa(peer_address.sin_addr);
        LogManager::logFormat(1, "accept_diag.cpp", kAcceptThreadSignature, 0,
                              "[DIAG0627][ACCEPT] fd=%d peer=%s",
                              accepted, peer_ip);
        if (std::strcmp(G_CEnvironment()->get_system_monitoring_ip(), peer_ip) ==
            0) {
            close(accepted);
            continue;
        }

        const int lock_threshold =
            G_CEnvironment()->get_accept_lock_dbqueue_count();
        if (lock_threshold != 0) {
            const int remaining_db_messages =
                GlobalData::s_msgq_mgr->remain(MsgQueueMgr::DB_Q);
            if (!accept_locked_ && lock_threshold <= remaining_db_messages) {
                accept_locked_ = true;
            } else if (accept_locked_ &&
                       G_CEnvironment()->get_accept_unlock_dbqueue_count() >
                           remaining_db_messages) {
                accept_locked_ = false;
            }
        }

        CGameManager* const manager = G_CGameManager();
        const int current_session_count = manager->user_pool_.Size() -
                                          manager->user_pool_.Remain();
        const bool queue_or_capacity_blocked =
            accept_locked_ ||
            G_CEnvironment()->get_max_user_num() <= current_session_count;
        const bool is_original_bypass_ip =
            std::strcmp("192.168.30.146", peer_ip) == 0;
        if (queue_or_capacity_blocked && !is_original_bypass_ip &&
            G_CEnvironment()->get_max_user_num() + 5 <= current_session_count) {
            RejectAcceptedSocket(accepted);
            cMyTrace(kAcceptThreadSignature, 529, 0)(
                "[ACCEPT] LOCK STATE(Curr Session : %d)", current_session_count);
            continue;
        }

        // The original member is non-const (_ZN22...22IsProhibitConnectStateEv);
        // retain that ABI when obtaining the process singleton for admission.
        if (GameserverDemonManagerInstance()->IsProhibitConnectState()) {
            RejectAcceptedSocket(accepted);
            cMyTrace(kAcceptThreadSignature, 560, 0)(
                "[ACCEPT DENY] ProhibitUserConnect Signal STATE(Curr Session : %d)",
                current_session_count);
            continue;
        }

        CUser* const user = manager->createUser();
        if (!user) {
            RejectAcceptedSocket(accepted);
            cMyTrace(kAcceptThreadSignature, 631, 0)(
                "[ACCEPT] fail create user(Curr Session : %d)",
                current_session_count);
            continue;
        }

        user->InitOnAccept(accepted);
        user->set_public_ip_address(peer_address.sin_addr.s_addr);
        GlobalData::s_accept_lock.lock();
        GlobalData::s_accept_que.push(user);
        GlobalData::s_accept_lock.unlock();
    }

    close(listener);
}
