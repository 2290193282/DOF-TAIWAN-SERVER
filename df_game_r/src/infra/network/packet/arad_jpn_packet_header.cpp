#include "infra/network/packet/arad_jpn_packet_header.h"

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x8197812
// IDA_NAME: _ZN4ARAD10DISPATCHER25make_cmd_packetheader_jpnER11PacketGuard14ENUM_CMDPACKETb
// DEMANGLED: ARAD::DISPATCHER::make_cmd_packetheader_jpn(PacketGuard &,ENUM_CMDPACKET,bool)
int make_cmd_packetheader_jpn(PacketGuard& packet,
                              ENUM_CMDPACKET packet_type,
                              bool append_result) {
    packet.put_header(1, static_cast<int>(packet_type));
    return packet.put_byte(append_result ? 1 : 0);
}

// IDA_EA: 0x8197862
// IDA_NAME: _ZN4ARAD10DISPATCHER26make_noti_packetheader_jpnER11PacketGuard15ENUM_NOTIPACKETb
// DEMANGLED: ARAD::DISPATCHER::make_noti_packetheader_jpn(PacketGuard &,ENUM_NOTIPACKET,bool)
int make_noti_packetheader_jpn(PacketGuard& packet,
                               ENUM_NOTIPACKET packet_type,
                               bool append_result) {
    packet.put_header(0, static_cast<int>(packet_type));
    return packet.put_byte(append_result ? 1 : 0);
}

}  // namespace DISPATCHER
}  // namespace ARAD
