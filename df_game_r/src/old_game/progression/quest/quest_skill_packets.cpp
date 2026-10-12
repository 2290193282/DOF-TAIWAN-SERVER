#include "old_game/progression/quest/quest_skill_packets.h"

#include <cstring>


typedef char Packet_Request_Power_War_Start_Info_Size_Must_Be_0x0e[
    (sizeof(Packet_Request_Power_War_Start_Info) == 0x0e) ? 1 : -1];
typedef char Packet_Request_Join_Power_Size_Must_Be_0x13[
    (sizeof(Packet_Request_Join_Power) == 0x13) ? 1 : -1];
typedef char Packet_Request_Secede_Power_Size_Must_Be_0x13[
    (sizeof(Packet_Request_Secede_Power) == 0x13) ? 1 : -1];
typedef char Packet_Request_PvP_Buddy_Conn_List_Size_Must_Be_0x12[
    (sizeof(Packet_Request_PvP_Buddy_Conn_List) == 0x12) ? 1 : -1];
typedef char Packet_Request_Add_PvP_Buddy_Size_Must_Be_0x31[
    (sizeof(Packet_Request_Add_PvP_Buddy) == 0x31) ? 1 : -1];
typedef char Packet_Request_Remove_PvP_Buddy_Size_Must_Be_0x31[
    (sizeof(Packet_Request_Remove_PvP_Buddy) == 0x31) ? 1 : -1];

// IDA_EA: 0x846F75E
// IDA_NAME: _ZN35Packet_Request_Power_War_Start_InfoC2Ev
// DEMANGLED: Packet_Request_Power_War_Start_Info::Packet_Request_Power_War_Start_Info(void)
Packet_Request_Power_War_Start_Info::Packet_Request_Power_War_Start_Info()
    : PacketHeader(0x06B1u, 0x000Eu),
      dword_0a(0) {}

// IDA_EA: 0x846F7EE
// IDA_NAME: _ZN25Packet_Request_Join_PowerC2Ev
// DEMANGLED: Packet_Request_Join_Power::Packet_Request_Join_Power(void)
Packet_Request_Join_Power::Packet_Request_Join_Power()
    : PacketHeader(0x06A8u, 0x0013u) {}

// IDA_EA: 0x846F812
// IDA_NAME: _ZN27Packet_Request_Secede_PowerC2Ev
// DEMANGLED: Packet_Request_Secede_Power::Packet_Request_Secede_Power(void)
Packet_Request_Secede_Power::Packet_Request_Secede_Power()
    : PacketHeader(0x06A9u, 0x0013u) {}

// IDA_EA: 0x846CEFC
// IDA_NAME: _ZN34Packet_Request_PvP_Buddy_Conn_ListC2Ev
// DEMANGLED: Packet_Request_PvP_Buddy_Conn_List::Packet_Request_PvP_Buddy_Conn_List(void)
Packet_Request_PvP_Buddy_Conn_List::Packet_Request_PvP_Buddy_Conn_List()
    : PacketHeader(0x1B5Au, 0x0012u),
      dword_0a(0),
      dword_0e(0) {}

// IDA_EA: 0x846CF34
// IDA_NAME: _ZN28Packet_Request_Add_PvP_BuddyC2Ev
// DEMANGLED: Packet_Request_Add_PvP_Buddy::Packet_Request_Add_PvP_Buddy(void)
Packet_Request_Add_PvP_Buddy::Packet_Request_Add_PvP_Buddy()
    : PacketHeader(0x1B5Cu, 0x0031u),
      acc_id(0),
      charac_no(0),
      request_kind(0) {
    std::memset(target_name, 0, sizeof(target_name));
}

// IDA_EA: 0x846CFEC
// IDA_NAME: _ZN31Packet_Request_Remove_PvP_BuddyC2Ev
// DEMANGLED: Packet_Request_Remove_PvP_Buddy::Packet_Request_Remove_PvP_Buddy(void)
Packet_Request_Remove_PvP_Buddy::Packet_Request_Remove_PvP_Buddy()
    : PacketHeader(0x1B60u, 0x0031u),
      dword_0a(0),
      dword_0e(0),
      byte_12(0) {
    std::memset(raw_13, 0, sizeof(raw_13));
}
