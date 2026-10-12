#include "old_data/Login/packets/login_packets.h"

#include <cstring>


typedef char Packet_Community_Login_Size_Must_Be_0x5A2[
    (sizeof(Packet_Community_Login) == 0x5A2) ? 1 : -1];

// IDA_EA: 0x846CE14
// IDA_NAME: _ZN22Packet_Community_LoginC2Ev
// DEMANGLED: Packet_Community_Login::Packet_Community_Login(void)
Packet_Community_Login::Packet_Community_Login() : header(0x1B58, 0x5A2) {
    std::memset(payload, 0, sizeof(payload));
}
