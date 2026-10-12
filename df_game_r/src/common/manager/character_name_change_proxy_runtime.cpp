#include "common/manager/character_name_change_proxy_runtime.h"

#include <cstring>

#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"

// IDA_EA: 0x846FA00
// IDA_NAME: _ZN23Packet_Change_Char_NameC2Ev
// DEMANGLED: Packet_Change_Char_Name::Packet_Change_Char_Name(void)
Packet_Change_Char_Name::Packet_Change_Char_Name()
    : PacketHeader(0x0451u, 0x0034u), account_id(0), charac_no(0), guild_id(0) {
    std::memset(charac_name, 0, sizeof(charac_name));
}

// IDA_EA: 0x846E3C8
// IDA_NAME: _ZN17CGuildServerProxy18SendChangeCharNameEjjjPc
// DEMANGLED: CGuildServerProxy::SendChangeCharName(unsigned int,unsigned int,unsigned int,char *)
char CGuildServerProxy::SendChangeCharName(unsigned int account_id,
                                           unsigned int charac_no,
                                           unsigned int guild_id,
                                           char* charac_name) {
    Packet_Change_Char_Name packet;
    packet.account_id = account_id;
    packet.charac_no = charac_no;
    packet.guild_id = guild_id;
    std::strcpy(packet.charac_name, charac_name);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

// IDA_EA: 0x8471288
// IDA_NAME: _ZN19CMonitorServerProxy18SendChangeCharNameEjjjPc
// DEMANGLED: CMonitorServerProxy::SendChangeCharName(unsigned int,unsigned int,unsigned int,char *)
char CMonitorServerProxy::SendChangeCharName(unsigned int account_id,
                                             unsigned int charac_no,
                                             unsigned int guild_id,
                                             char* charac_name) {
    Packet_Change_Char_Name packet;
    packet.account_id = account_id;
    packet.charac_no = charac_no;
    packet.guild_id = guild_id;
    std::strcpy(packet.charac_name, charac_name);
    return SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}
