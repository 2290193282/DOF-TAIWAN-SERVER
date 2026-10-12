#ifndef OLDSRV_RUNTIME_UDP_HANDLER_H_
#define OLDSRV_RUNTIME_UDP_HANDLER_H_

#include <stdint.h>
#include <sys/types.h>


// Old CUdpHandler layout is two file descriptors:
//   +0x00 server receive socket, +0x04 client send/receive socket.
class CUdpHandler {
public:
    CUdpHandler();

    int InitServerSocket(int port);
    int InitClientSocket();
    int RecvFromClient(char* buffer, int* bytes, uint32_t* ip,
                       uint16_t* port) const;
    int SendToClient(char* buffer, int bytes, uint16_t port, const char* ip,
                     uint32_t net_ip) const;
    int RecvFromServer(char* buffer, int* bytes, uint32_t* ip,
                       uint16_t* port) const;
    int SendToServer(char* buffer, size_t bytes, uint16_t port,
                     const char* ip) const;
    uint32_t InetAddr(const char* ip) const;

private:
    int server_socket_;
    int client_socket_;
};


#endif  // OLDSRV_RUNTIME_UDP_HANDLER_H_
