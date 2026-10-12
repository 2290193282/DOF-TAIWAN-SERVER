#include "infra/network/transport/network_endpoint.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


namespace {

NetworkPacketDispatch g_server_packet_dispatch = 0;
NetworkPacketDispatch g_auction_packet_dispatch = 0;

}  // namespace

void SetServerPacketDispatch(NetworkPacketDispatch callback) {
    g_server_packet_dispatch = callback;
}

void SetAuctionPacketDispatch(NetworkPacketDispatch callback) {
    g_auction_packet_dispatch = callback;
}

NetworkPacketDispatch GetServerPacketDispatch() {
    return g_server_packet_dispatch;
}

NetworkPacketDispatch GetAuctionPacketDispatch() {
    return g_auction_packet_dispatch;
}

bool NetworkErrnoIsNonBlocking() {
    return errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK ||
           errno == 0 || errno == EINPROGRESS;
}

bool NetworkSetNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) >= 0;
}

int NetworkCreateConnectionSocket(int& fd, const char* ip, int port,
                                  bool nonblock) {
    struct sockaddr_in addr;
    int connect_rc = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return 1;
    }
    if (nonblock && !NetworkSetNonBlock(fd)) {
        return 2;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    connect_rc = connect(fd, reinterpret_cast<struct sockaddr*>(&addr),
                         sizeof(addr));
    if (connect_rc >= 0 || NetworkErrnoIsNonBlocking()) {
        return 0;
    }
    return 3;
}

bool NetworkCreateListenSocket(int& fd, int port, const char* ip) {
    struct sockaddr_in addr;
    int optval = 60000;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip ? inet_addr(ip) : htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        return false;
    }
    if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        return false;
    }
    if (listen(fd, 1024) < 0) {
        return false;
    }
    return NetworkSetNonBlock(fd);
}

void NetworkLogError(const char* func, const char* text, int fd) {
    fprintf(stderr, "[Network.h:%s] %s FD[%d] errno=%d(%s)\n", func, text, fd,
            errno, strerror(errno));
}

uint16_t NetworkReadU16(const char* src) {
    uint16_t value = 0;
    memcpy(&value, src, sizeof(value));
    return value;
}

uint32_t NetworkReadU32(const char* src) {
    uint32_t value = 0;
    memcpy(&value, src, sizeof(value));
    return value;
}
