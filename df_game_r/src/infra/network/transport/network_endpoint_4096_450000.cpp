#include "infra/network/transport/network_endpoint_impl.h"


// IDA_EA: 0x816282E
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE10GetPeerIP2EPci
// DEMANGLED: CNetwork<4096,450000>::GetPeerIP2(char *,int)
template <>
char* CNetwork<4096, 450000>::GetPeerIP2(char* out, int bytes) {
    return peer_ip_string(out, bytes);
}

// IDA_EA: 0x8236D0C
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE10get_str_ipEv
// DEMANGLED: CNetwork<4096,450000>::get_str_ip(void)
template <>
char* CNetwork<4096, 450000>::get_str_ip() {
    return str_ip_;
}

// IDA_EA: 0x8236DE0
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE12get_nat_typeEv
// DEMANGLED: CNetwork<4096,450000>::get_nat_type(void)
template <>
int CNetwork<4096, 450000>::get_nat_type() {
    return nat_type_;
}

// IDA_EA: 0x823779E
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE7set_natEcjjtj
// DEMANGLED: CNetwork<4096,450000>::set_nat(char,unsigned int,unsigned int,unsigned short,unsigned int)
template <>
char* CNetwork<4096, 450000>::set_nat(char nat_type, uint32_t inner_ip,
                                      uint32_t ip, uint16_t port,
                                      uint32_t mtu) {
    return set_nat_with_mtu(nat_type, inner_ip, ip, port, mtu);
}

// IDA_EA: 0x8237826
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE7set_natEcjjtPKc
// DEMANGLED: CNetwork<4096,450000>::set_nat(char,unsigned int,unsigned int,unsigned short,char const*)
template <>
char* CNetwork<4096, 450000>::set_nat(char nat_type, uint32_t inner_ip,
                                      uint32_t ip, uint16_t port,
                                      const char* mac_addr) {
    return set_nat_with_mac(nat_type, inner_ip, ip, port, mac_addr);
}

// IDA_EA: 0x8288050
// IDA_NAME: _ZNK8CNetworkILi4096ELi450000EE12get_mac_addrEv
// DEMANGLED: CNetwork<4096,450000>::get_mac_addr(void)const
template <>
const char* CNetwork<4096, 450000>::get_mac_addr() const {
    return mac_addr_;
}

// IDA_EA: 0x82B2CB8
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE10get_socketEv
// DEMANGLED: CNetwork<4096,450000>::get_socket(void)
template <>
int CNetwork<4096, 450000>::get_socket() {
    return socket_;
}

// IDA_EA: 0x82F1BD8
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE12get_inner_ipEv
// DEMANGLED: CNetwork<4096,450000>::get_inner_ip(void)
template <>
uint32_t CNetwork<4096, 450000>::get_inner_ip() {
    return inner_ip_;
}

// IDA_EA: 0x82F1BE6
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE6get_ipEv
// DEMANGLED: CNetwork<4096,450000>::get_ip(void)
template <>
uint32_t CNetwork<4096, 450000>::get_ip() {
    return ip_;
}

// IDA_EA: 0x82F1BF4
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE8get_portEv
// DEMANGLED: CNetwork<4096,450000>::get_port(void)
template <>
uint16_t CNetwork<4096, 450000>::get_port() {
    return port_;
}

// IDA_EA: 0x82F1C04
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE7get_mtuEv
// DEMANGLED: CNetwork<4096,450000>::get_mtu(void)
template <>
uint32_t CNetwork<4096, 450000>::get_mtu() {
    return mtu_;
}

// IDA_EA: 0x857F484
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE11SetNonBlockEi
// DEMANGLED: CNetwork<4096,450000>::SetNonBlock(int)
template <>
bool CNetwork<4096, 450000>::SetNonBlock(int fd) {
    return NetworkSetNonBlock(fd);
}

// IDA_EA: 0x857F616
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE11send_packetEv
// DEMANGLED: CNetwork<4096,450000>::send_packet(void)
template <>
int CNetwork<4096, 450000>::send_packet() {
    return flush_send_packet();
}

// IDA_EA: 0x857FA96
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE9send_lockEv
// DEMANGLED: CNetwork<4096,450000>::send_lock(void)
template <>
int CNetwork<4096, 450000>::send_lock() {
    return lock_send();
}

// IDA_EA: 0x857FAAE
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE11send_unlockEv
// DEMANGLED: CNetwork<4096,450000>::send_unlock(void)
template <>
int CNetwork<4096, 450000>::send_unlock() {
    return unlock_send();
}

// IDA_EA: 0x86985F0
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE11recv_packetEv
// DEMANGLED: CNetwork<4096,450000>::recv_packet(void)
template <>
int CNetwork<4096, 450000>::recv_packet() {
    return receive_packet();
}

// IDA_EA: 0x8698744
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE11send_packetEPci
// DEMANGLED: CNetwork<4096,450000>::send_packet(char *,int)
template <>
int CNetwork<4096, 450000>::send_packet(char* src, int bytes) {
    return append_send_packet(src, bytes);
}

// IDA_EA: 0x8698976
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE10set_socketEib
// DEMANGLED: CNetwork<4096,450000>::set_socket(int,bool)
template <>
int CNetwork<4096, 450000>::set_socket(int fd, bool block) {
    return assign_socket(fd, block);
}

// IDA_EA: 0x86989CA
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE17initNetworkBufferEv
// DEMANGLED: CNetwork<4096,450000>::initNetworkBuffer(void)
template <>
void* CNetwork<4096, 450000>::initNetworkBuffer() {
    return reset_network_buffers();
}

// IDA_EA: 0x8698CBA
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EEC2Ev
// DEMANGLED: CNetwork<4096,450000>::CNetwork(void)
template <>
CNetwork<4096, 450000>::CNetwork() {
    initialize_state();
}

// IDA_EA: 0x8698D58
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EED2Ev
// DEMANGLED: CNetwork<4096,450000>::~CNetwork()
template <>
CNetwork<4096, 450000>::~CNetwork() {}

// IDA_EA: 0x869AE30
// IDA_NAME: _ZN8CNetworkILi4096ELi450000EE10disconnectEv
// DEMANGLED: CNetwork<4096,450000>::disconnect(void)
template <>
int CNetwork<4096, 450000>::disconnect() {
    return close_socket();
}
