#include "infra/concurrent/thread/udp_thread.h"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/security/policy/hades_server_packet_bridge.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_counter.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/network/transport/packet_tracer.h"

namespace {

const int kUdpReceiveCapacity = 0x1800;

const char kBindSignature[] =
    "SOCKET UdpThread::_bindUDPSocket(short unsigned int)";

}  // namespace

// IDA_EA: 0x857C354
// IDA_NAME: _ZN9UdpThreadC2Ev
// DEMANGLED: UdpThread::UdpThread(void)
UdpThread::UdpThread() : ThreadInterface() {}

// IDA_EA: 0x857C370
// IDA_NAME: _ZN9UdpThreadD2Ev
// DEMANGLED: UdpThread::~UdpThread()
// IDA_EA: 0x857C3A0
// IDA_NAME: _ZN9UdpThreadD0Ev
// DEMANGLED: UdpThread::~UdpThread() deleting destructor
// The virtual destructor's compiler-emitted D0 wrapper invokes this D2 body
// and operator delete(void *), matching IDA 0x857C3AC and 0x857C3B7.
UdpThread::~UdpThread() {}

// IDA_EA: 0x857C3BE
// IDA_NAME: _ZN9UdpThread14_bindUDPSocketEt
// DEMANGLED: UdpThread::_bindUDPSocket(unsigned short)
int UdpThread::_bindUDPSocket(uint16_t port) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        LogManager::logFormat(1,
                              "NetworkThread.cpp",
                              kBindSignature,
                              0x84,
                              "[ERROR]UDP Thread Socket Binding ERROR");
        std::exit(-1);
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        const int error_no = errno;
        LogManager::logFormat(1,
                              "NetworkThread.cpp",
                              kBindSignature,
                              0x84,
                              "[ERROR]UDP Thread Socket Binding ERROR");
        if (error_no == EADDRINUSE) {
            LogManager::logFormat(1,
                                  "NetworkThread.cpp",
                                  kBindSignature,
                                  0x9E,
                                  "Port %d for receiving UDP is in use\n",
                                  static_cast<int>(port));
        } else if (error_no == EADDRNOTAVAIL) {
            LogManager::logFormat(1,
                                  "NetworkThread.cpp",
                                  kBindSignature,
                                  0xA2,
                                  "Cannot assign requested address\n");
        } else if (error_no != 0) {
            LogManager::logFormat(
                1,
                "NetworkThread.cpp",
                kBindSignature,
                0xA6,
                "Could not bind UDP receive port. Error= %d , strerror = %s\n",
                error_no,
                std::strerror(error_no));
        }
        std::exit(-1);
    }

    if (!CNetwork4096_450000::SetNonBlock(fd)) {
        LogManager::logFormat(1,
                              "NetworkThread.cpp",
                              kBindSignature,
                              0xB0,
                              "Fail to SetNonBlock UdpThread.");
        std::exit(-1);
    }
    return fd;
}

// IDA_EA: 0x857C5A4
// IDA_NAME: _ZN9UdpThread15_processUDPDataEPci
// DEMANGLED: UdpThread::_processUDPData(char *,int)
void UdpThread::_processUDPData(char* packet, int received_size) {
    if (!packet || received_size < 2) {
        return;
    }

    const uint16_t command = NetworkReadU16(packet);
    CPacketTracerInstance()->AddUdpLog(3, command);

    static CPacketCounter<1000, 10240> packet_counter(
        const_cast<char*>(G_CEnvironment()->get_file_name()),
        const_cast<char*>("InterDispatcher"));
    packet_counter.IncrementPacketCount(command);

    // IDA 0x857C688 invokes CDispatchServerPacket first.  Its complete CFG
    // remains a separate restoration, but the recovered bridge owns the
    // proven server-frame cases and must run before these raw UDP statuses.
    // The original router has no command-0..3 receiver; those are its false
    // return fallbacks at 0x857C69B..0x857C9C7.
    if (received_size >= 4) {
        const uint16_t declared_size = NetworkReadU16(packet + 2);
        if (declared_size >= sizeof(PacketHeader) &&
            static_cast<int>(declared_size) <= received_size) {
            if (DispatchServerPacket(packet, static_cast<int>(declared_size))) {
                return;
            }
        }
    }

    const int raw_type = static_cast<int>(static_cast<signed char>(packet[0]));
    if (raw_type == 0) {
        CStreamGuard stream_guard(
            GlobalData::stream_pool()->Acquire("NetworkThread.cpp", 232), true);
        *stream_guard << 61;
        *stream_guard << -1;
        stream_guard->put_binary(packet, 0x52);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        return;
    }

    if (raw_type == 1) {
        CStreamGuard stream_guard(
            GlobalData::stream_pool()->Acquire("NetworkThread.cpp", 248), true);
        *stream_guard << 159;
        *stream_guard << -1;
        stream_guard->put_binary(packet + 2, received_size);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
        return;
    }

    if (raw_type > 1) {
        if (raw_type == 2) {
            return;
        }
        if (raw_type == 3) {
            GlobalData::s_auction_proxy->SetRunning(false);
            GlobalData::s_cera_auction_proxy->SetRunning(false);

            CStreamGuard stream_guard(
                GlobalData::stream_pool()->Acquire("NetworkThread.cpp", 294), true);
            *stream_guard << 61;
            *stream_guard << -1;
            stream_guard->put_binary(packet, 0x52);
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, stream_guard);
            return;
        }
    }

    if (command <= 0x3E7u) {
        LogManager::logFormat(1,
                              "NetworkThread.cpp",
                              "void UdpThread::_processUDPData(char*, int)",
                              0x112,
                              "ABNORMAL UDP PACKET [%d]",
                              raw_type);
    }
}

// IDA_EA: 0x857C9DE
// IDA_NAME: _ZN9UdpThread8dispatchEPv
// DEMANGLED: UdpThread::dispatch(void *)
void UdpThread::dispatch(void*) {
    const int fd = _bindUDPSocket(
        static_cast<uint16_t>(G_CEnvironment()->get_udp_port()));
    char packet[kUdpReceiveCapacity];

    while (!GlobalData::server_stop()) {
        std::memset(packet, 0, sizeof(packet));

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        const int ready = select(fd + 1, &read_fds, 0, 0, &timeout);
        if (ready < 0 || !FD_ISSET(fd, &read_fds)) {
            continue;
        }

        const int received_size = static_cast<int>(
            recv(fd, packet, sizeof(packet), 0));
        if (received_size > 0) {
            _processUDPData(packet, received_size);
        }
    }
}
