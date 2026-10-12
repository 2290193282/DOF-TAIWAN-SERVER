#include "common/manager/double_conn_check_server_proxy_abi.h"

#include "infra/network/packet/udp_packets.h"


// IDA_EA: 0x846D300
// IDA_NAME: _ZN27CDoubleConnCheckServerProxyC2ESsi
// DEMANGLED: CDoubleConnCheckServerProxy::CDoubleConnCheckServerProxy(std::string,int)
CDoubleConnCheckServerProxy::CDoubleConnCheckServerProxy(
    std::string server_ip,
    int udp_port)
    : server_ip_(server_ip), udp_port_(udp_port), udp_handler_() {}

// IDA_EA: 0x846D356
// IDA_NAME: _ZN27CDoubleConnCheckServerProxy4InitEv
// DEMANGLED: CDoubleConnCheckServerProxy::Init(void)
bool CDoubleConnCheckServerProxy::Init() {
    return udp_handler_.InitClientSocket() != -1;
}

// IDA_EA: 0x846D412
// IDA_NAME: _ZN27CDoubleConnCheckServerProxy9SendLoginEjii
// DEMANGLED: CDoubleConnCheckServerProxy::SendLogin(unsigned int,int,int)
int CDoubleConnCheckServerProxy::SendLogin(uint32_t account_id,
                                           int channel_no,
                                           int server_group) {
    Packet_DoubleCheck_UDP_Login packet;
    packet.account_id = account_id;
    packet.channel_no = static_cast<uint8_t>(channel_no);
    packet.server_group = static_cast<uint8_t>(server_group);
    return udp_handler_.SendToServer(reinterpret_cast<char*>(&packet),
                                     packet.size,
                                     static_cast<uint16_t>(udp_port_),
                                     server_ip_.c_str());
}

// IDA_EA: 0x846D47C
// IDA_NAME: _ZN27CDoubleConnCheckServerProxy10SendLogoutEjii
// DEMANGLED: CDoubleConnCheckServerProxy::SendLogout(unsigned int,int,int)
void CDoubleConnCheckServerProxy::SendLogout(uint32_t account_id,
                                             int channel_no,
                                             int server_group) {
    if (!account_id) {
        return;
    }

    Packet_DoubleCheck_UDP_Logout packet;
    packet.account_id = account_id;
    packet.channel_no = static_cast<uint8_t>(channel_no);
    packet.server_group = static_cast<uint8_t>(server_group);
    udp_handler_.SendToServer(reinterpret_cast<char*>(&packet),
                              packet.size,
                              static_cast<uint16_t>(udp_port_),
                              server_ip_.c_str());
}

// IDA_EA: 0x846D4EE
// IDA_NAME: _ZN27CDoubleConnCheckServerProxy10SendPacketEPci
// DEMANGLED: CDoubleConnCheckServerProxy::SendPacket(char *,int)
int CDoubleConnCheckServerProxy::SendPacket(char* packet, int size) {
    return udp_handler_.SendToServer(packet, static_cast<size_t>(size),
                                     static_cast<uint16_t>(udp_port_),
                                     server_ip_.c_str());
}
