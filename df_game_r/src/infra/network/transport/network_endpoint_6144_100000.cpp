#include "infra/network/transport/network_endpoint_impl.h"


// IDA_EA: 0x818DF60
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EEC2Ev
// DEMANGLED: CNetwork<6144,100000>::CNetwork(void)
template <>
CNetwork<6144, 100000>::CNetwork() {
    initialize_state();
}

// IDA_EA: 0x818DFFE
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE22CreateConnectionSocketERiPKci
// DEMANGLED: CNetwork<6144,100000>::CreateConnectionSocket(int &,char const*,int)
template <>
int CNetwork<6144, 100000>::CreateConnectionSocket(int& fd, const char* ip,
                                                   int port) {
    return NetworkCreateConnectionSocket(fd, ip, port, true);
}

// IDA_EA: 0x818E100
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE10disconnectEv
// DEMANGLED: CNetwork<6144,100000>::disconnect(void)
template <>
int CNetwork<6144, 100000>::disconnect() {
    return close_socket();
}

// IDA_EA: 0x818E174
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE11recv_packetEv
// DEMANGLED: CNetwork<6144,100000>::recv_packet(void)
template <>
int CNetwork<6144, 100000>::recv_packet() {
    return receive_packet();
}

// IDA_EA: 0x818E2C8
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE20Parsing_ServerPacketEi
// DEMANGLED: CNetwork<6144,100000>::Parsing_ServerPacket(int)
template <>
int CNetwork<6144, 100000>::Parsing_ServerPacket(int received_bytes) {
    return parse_server_packet(received_bytes);
}

// IDA_EA: 0x818E4F4
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE11send_packetEv
// DEMANGLED: CNetwork<6144,100000>::send_packet(void)
template <>
int CNetwork<6144, 100000>::send_packet() {
    return flush_send_packet();
}

// IDA_EA: 0x818E7A0
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE17GetRemainSendSizeEv
// DEMANGLED: CNetwork<6144,100000>::GetRemainSendSize(void)
template <>
int CNetwork<6144, 100000>::GetRemainSendSize() const {
    return remain_send_len_;
}

// IDA_EA: 0x818E7AE
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE11SetNonBlockEi
// DEMANGLED: CNetwork<6144,100000>::SetNonBlock(int)
template <>
bool CNetwork<6144, 100000>::SetNonBlock(int fd) {
    return NetworkSetNonBlock(fd);
}

// IDA_EA: 0x818E806
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE9send_lockEv
// DEMANGLED: CNetwork<6144,100000>::send_lock(void)
template <>
int CNetwork<6144, 100000>::send_lock() {
    return lock_send();
}

// IDA_EA: 0x818E81E
// IDA_NAME: _ZN8CNetworkILi6144ELi100000EE11send_unlockEv
// DEMANGLED: CNetwork<6144,100000>::send_unlock(void)
template <>
int CNetwork<6144, 100000>::send_unlock() {
    return unlock_send();
}
