#include "infra/network/transport/network_endpoint_impl.h"


// IDA_EA: 0x8493F20
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EEC2Ev
// DEMANGLED: CNetwork<95000,450000>::CNetwork(void)
template <>
CNetwork<95000, 450000>::CNetwork() {
    initialize_state();
}

// IDA_EA: 0x8493FBE
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EED2Ev
// DEMANGLED: CNetwork<95000,450000>::~CNetwork()
template <>
CNetwork<95000, 450000>::~CNetwork() {}

// IDA_EA: 0x8493FD6
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE10get_socketEv
// DEMANGLED: CNetwork<95000,450000>::get_socket(void)
template <>
int CNetwork<95000, 450000>::get_socket() {
    return socket_;
}

// IDA_EA: 0x8493FE0
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE11recv_packetEv
// DEMANGLED: CNetwork<95000,450000>::recv_packet(void)
template <>
int CNetwork<95000, 450000>::recv_packet() {
    return receive_packet();
}

// IDA_EA: 0x8494134
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE11send_packetEv
// DEMANGLED: CNetwork<95000,450000>::send_packet(void)
template <>
int CNetwork<95000, 450000>::send_packet() {
    return flush_send_packet();
}

// IDA_EA: 0x84943E0
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE10set_socketEib
// DEMANGLED: CNetwork<95000,450000>::set_socket(int,bool)
template <>
int CNetwork<95000, 450000>::set_socket(int fd, bool block) {
    return assign_socket(fd, block);
}

// IDA_EA: 0x8494434
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE11send_packetEPci
// DEMANGLED: CNetwork<95000,450000>::send_packet(char *,int)
template <>
int CNetwork<95000, 450000>::send_packet(char* src, int bytes) {
    return append_send_packet(src, bytes);
}

// IDA_EA: 0x8494666
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE18CreateListenSocketERiiPKc
// DEMANGLED: CNetwork<95000,450000>::CreateListenSocket(int &,int,char const*)
template <>
bool CNetwork<95000, 450000>::CreateListenSocket(int& fd, int port,
                                                 const char* ip) {
    return NetworkCreateListenSocket(fd, port, ip);
}

// IDA_EA: 0x84947B1
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE22CreateConnectionSocketERiPKci
// DEMANGLED: CNetwork<95000,450000>::CreateConnectionSocket(int &,char const*,int)
template <>
int CNetwork<95000, 450000>::CreateConnectionSocket(int& fd, const char* ip,
                                                    int port) {
    return NetworkCreateConnectionSocket(fd, ip, port, true);
}

// IDA_EA: 0x84948B4
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE10disconnectEv
// DEMANGLED: CNetwork<95000,450000>::disconnect(void)
template <>
int CNetwork<95000, 450000>::disconnect() {
    return close_socket();
}

// IDA_EA: 0x849589E
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE9send_lockEv
// DEMANGLED: CNetwork<95000,450000>::send_lock(void)
template <>
int CNetwork<95000, 450000>::send_lock() {
    return lock_send();
}

// IDA_EA: 0x84958B6
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE11send_unlockEv
// DEMANGLED: CNetwork<95000,450000>::send_unlock(void)
template <>
int CNetwork<95000, 450000>::send_unlock() {
    return unlock_send();
}

// IDA_EA: 0x84958CE
// IDA_NAME: _ZN8CNetworkILi95000ELi450000EE11SetNonBlockEi
// DEMANGLED: CNetwork<95000,450000>::SetNonBlock(int)
template <>
bool CNetwork<95000, 450000>::SetNonBlock(int fd) {
    return NetworkSetNonBlock(fd);
}
