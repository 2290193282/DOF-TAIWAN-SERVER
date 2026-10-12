#include "infra/network/transport/udp_handler.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>


namespace {

int udp_errno() {
    return errno;
}

int set_receive_timeout(int fd) {
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int recv_from_socket(int fd, char* buffer, int* bytes, uint32_t* ip,
                     uint16_t* port) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (fd == -1) {
        return 0;
    }

    *bytes = recvfrom(fd, buffer, *bytes, 0,
                      reinterpret_cast<struct sockaddr*>(&addr), &addr_len);
    if (*bytes == -1) {
        int errnum = udp_errno();
        if (errnum == ENOTSOCK) {
            puts("Error fd not a socket");
        } else if (errnum == ECONNRESET) {
            puts("Error connection reset - host not reachable");
        } else {
            printf("Hm! Time out Or Socket Error = %d\n", errnum);
        }
        return 0;
    }
    if (*bytes > 0) {
        *port = ntohs(addr.sin_port);
        *ip = ntohl(addr.sin_addr.s_addr);
        buffer[*bytes] = 0;
        return 1;
    }

    printf("Socket closed? Recv size = %d\n", *bytes);
    return 0;
}

}  // namespace

// IDA_EA: 0x8483518
// IDA_NAME: _ZN11CUdpHandlerC2Ev
// DEMANGLED: CUdpHandler::CUdpHandler(void)
CUdpHandler::CUdpHandler() : server_socket_(-1), client_socket_(-1) {}

// IDA_EA: 0x8483530
// IDA_NAME: _ZN11CUdpHandler16InitServerSocketEi
// DEMANGLED: CUdpHandler::InitServerSocket(int)
int CUdpHandler::InitServerSocket(int port) {
    struct sockaddr_in addr;

    server_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket_ == -1) {
        int errnum = udp_errno();
        printf("Could not create a UDP socket : %d\n", errnum);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&addr),
             sizeof(addr))) {
        int errnum = udp_errno();
        if (errnum == EADDRINUSE) {
            printf("Port %d for receiving UDP is in use\n", port);
        } else if (errnum == EADDRNOTAVAIL) {
            puts("Cannot assign requested address");
        } else if (errnum) {
            printf("Could not bind UDP receive port. Error= %d , strerror = %s\n",
                   errnum, strerror(errnum));
        }
        server_socket_ = -1;
    }
    set_receive_timeout(server_socket_);
    return server_socket_;
}

// IDA_EA: 0x848369A
// IDA_NAME: _ZN11CUdpHandler16InitClientSocketEv
// DEMANGLED: CUdpHandler::InitClientSocket(void)
int CUdpHandler::InitClientSocket() {
    client_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket_ == -1) {
        int errnum = udp_errno();
        printf("Could not create a UDP socket : %d\n", errnum);
        return -1;
    }
    set_receive_timeout(client_socket_);
    return client_socket_;
}

// IDA_EA: 0x8483732
// IDA_NAME: _ZNK11CUdpHandler14RecvFromClientEPcPiPjPt
// DEMANGLED: CUdpHandler::RecvFromClient(char *,int *,unsigned int *,unsigned short *)const
int CUdpHandler::RecvFromClient(char* buffer, int* bytes, uint32_t* ip,
                                uint16_t* port) const {
    return recv_from_socket(server_socket_, buffer, bytes, ip, port);
}

// IDA_EA: 0x8483846
// IDA_NAME: _ZNK11CUdpHandler12SendToClientEPcitPKcj
// DEMANGLED: CUdpHandler::SendToClient(char *,int,unsigned short,char const*,unsigned int)const
int CUdpHandler::SendToClient(char* buffer, int bytes, uint16_t port,
                              const char* ip, uint32_t net_ip) const {
    struct sockaddr_in addr;
    ssize_t sent = -1;

    if (client_socket_ == -1) {
        fprintf(stderr, "[SendToClient][225]\n");
        return 0;
    }
    if (!ip && !net_ip) {
        return 0;
    }
    if (ip) {
        net_ip = inet_addr(ip);
    }
    if (server_socket_ == -1) {
        return 0;
    }

    if (port) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = ntohl(net_ip);
        sent = sendto(server_socket_, buffer, bytes, 0,
                      reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    } else if (!net_ip) {
        sent = send(server_socket_, buffer, bytes, 0);
    }

    if (sent == -1) {
        int errnum = udp_errno();
        if (errnum == EAFNOSUPPORT) {
            puts("err EAFNOSUPPORT in send");
        } else if (errnum == ECONNREFUSED || errnum == EHOSTDOWN ||
                   errnum == EHOSTUNREACH) {
            printf("Error( ECONNREFUSED, EHOSTDOWN, EHOSTUNREACH ) = %d\n",
                   errnum);
        } else {
            printf("err = %d , strerror = %s in send\n", errnum,
                   strerror(errnum));
        }
        return 0;
    }
    if (sent) {
        if (sent == bytes) {
            return 1;
        }
        printf("Only %d out of %d bytes sent\n", static_cast<int>(sent), bytes);
        return 0;
    }

    puts("no data sent in send");
    return 0;
}

// IDA_EA: 0x8483A52
// IDA_NAME: _ZNK11CUdpHandler14RecvFromServerEPcPiPjPt
// DEMANGLED: CUdpHandler::RecvFromServer(char *,int *,unsigned int *,unsigned short *)const
int CUdpHandler::RecvFromServer(char* buffer, int* bytes, uint32_t* ip,
                                uint16_t* port) const {
    return recv_from_socket(client_socket_, buffer, bytes, ip, port);
}

// IDA_EA: 0x8483B68
// IDA_NAME: _ZNK11CUdpHandler12SendToServerEPcitPKc
// DEMANGLED: CUdpHandler::SendToServer(char *,int,unsigned short,char const*)const
int CUdpHandler::SendToServer(char* buffer, size_t bytes, uint16_t port,
                              const char* ip) const {
    struct sockaddr_in addr;
    ssize_t sent = -1;

    if (client_socket_ == -1) {
        fprintf(stderr, "[SendToServer][404]\n");
        return 0;
    }

    if (port) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);
        sent = sendto(client_socket_, buffer, bytes, 0,
                      reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    } else if (!ip) {
        sent = send(client_socket_, buffer, bytes, 0);
    }

    if (sent == -1) {
        int errnum = udp_errno();
        if (errnum == EAFNOSUPPORT) {
            fprintf(stderr, "CUdpHandler::SendToServer Error: EAFNOSUPPORT\n");
        } else if (errnum == ECONNREFUSED || errnum == EHOSTDOWN ||
                   errnum == EHOSTUNREACH) {
            fprintf(stderr, "CUdpHandler::SendToServer Error: EHOSTUNREACH\n");
        } else {
            fprintf(stderr, "CUdpHandler::SendToServer Error : other err\n");
        }
        return 0;
    }
    if (sent) {
        if (sent == static_cast<ssize_t>(bytes)) {
            return 1;
        }
        fprintf(stderr, "CUdpHandler::SendToServer Error : len != s\n");
        return 0;
    }

    fprintf(stderr, "CUdpHandler::SendToServer Error: 0 == s\n");
    return 0;
}

// IDA_EA: 0x8483E06
// IDA_NAME: _ZNK11CUdpHandler8InetAddrEPKc
// DEMANGLED: CUdpHandler::InetAddr(char const*)const
uint32_t CUdpHandler::InetAddr(const char* ip) const {
    return inet_addr(ip);
}
