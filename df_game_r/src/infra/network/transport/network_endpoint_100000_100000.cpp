#include "infra/network/transport/network_endpoint_impl.h"


// IDA_EA: 0x82AB4F0
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EEC2Ev
// DEMANGLED: CNetwork<100000,100000>::CNetwork(void)
template <>
CNetwork<100000, 100000>::CNetwork() {
    initialize_state();
}

// IDA_EA: 0x82AB58E
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EED2Ev
// DEMANGLED: CNetwork<100000,100000>::~CNetwork()
template <>
CNetwork<100000, 100000>::~CNetwork() {}

// IDA_EA: 0x82F7EE2
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE27Parsing_AuctionServerPacketEi
// DEMANGLED: CNetwork<100000,100000>::Parsing_AuctionServerPacket(int)
template <>
int CNetwork<100000, 100000>::Parsing_AuctionServerPacket(int received_bytes) {
    return parse_auction_packet(received_bytes);
}

// IDA_EA: 0x82FDD99
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE22CreateConnectionSocketERiPKci
// DEMANGLED: CNetwork<100000,100000>::CreateConnectionSocket(int &,char const*,int)
template <>
int CNetwork<100000, 100000>::CreateConnectionSocket(int& fd, const char* ip,
                                                     int port) {
    return NetworkCreateConnectionSocket(fd, ip, port, true);
}

// IDA_EA: 0x82FDE9B
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE27CreateConnectionBlockSocketERiPKci
// DEMANGLED: CNetwork<100000,100000>::CreateConnectionBlockSocket(int &,char const*,int)
template <>
int CNetwork<100000, 100000>::CreateConnectionBlockSocket(int& fd,
                                                          const char* ip,
                                                          int port) {
    return NetworkCreateConnectionSocket(fd, ip, port, false);
}

// IDA_EA: 0x82FDF4E
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE10get_socketEv
// DEMANGLED: CNetwork<100000,100000>::get_socket(void)
template <>
int CNetwork<100000, 100000>::get_socket() {
    return socket_;
}

// IDA_EA: 0x82FDF58
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE10disconnectEv
// DEMANGLED: CNetwork<100000,100000>::disconnect(void)
template <>
int CNetwork<100000, 100000>::disconnect() {
    return close_socket();
}

// IDA_EA: 0x82FDFCC
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE11send_packetEPci
// DEMANGLED: CNetwork<100000,100000>::send_packet(char *,int)
template <>
int CNetwork<100000, 100000>::send_packet(char* src, int bytes) {
    return append_send_packet(src, bytes);
}

// IDA_EA: 0x82FE1FE
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE11recv_packetEv
// DEMANGLED: CNetwork<100000,100000>::recv_packet(void)
template <>
int CNetwork<100000, 100000>::recv_packet() {
    return receive_packet();
}

// IDA_EA: 0x82FE352
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE11send_packetEv
// DEMANGLED: CNetwork<100000,100000>::send_packet(void)
template <>
int CNetwork<100000, 100000>::send_packet() {
    return flush_send_packet();
}

// IDA_EA: 0x82FE5FD
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE11SetNonBlockEi
// DEMANGLED: CNetwork<100000,100000>::SetNonBlock(int)
template <>
bool CNetwork<100000, 100000>::SetNonBlock(int fd) {
    return NetworkSetNonBlock(fd);
}

// IDA_EA: 0x82FE656
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE9send_lockEv
// DEMANGLED: CNetwork<100000,100000>::send_lock(void)
template <>
int CNetwork<100000, 100000>::send_lock() {
    return lock_send();
}

// IDA_EA: 0x82FE66E
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE11send_unlockEv
// DEMANGLED: CNetwork<100000,100000>::send_unlock(void)
template <>
int CNetwork<100000, 100000>::send_unlock() {
    return unlock_send();
}

// IDA_EA: 0x846D050
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE20Parsing_ServerPacketEi
// DEMANGLED: CNetwork<100000,100000>::Parsing_ServerPacket(int)
template <>
int CNetwork<100000, 100000>::Parsing_ServerPacket(int received_bytes) {
    return parse_server_packet(received_bytes);
}

// IDA_EA: 0x8471748
// IDA_NAME: _ZN8CNetworkILi100000ELi100000EE17GetRemainSendSizeEv
// DEMANGLED: CNetwork<100000,100000>::GetRemainSendSize(void)
template <>
int CNetwork<100000, 100000>::GetRemainSendSize() const {
    return remain_send_len_;
}
