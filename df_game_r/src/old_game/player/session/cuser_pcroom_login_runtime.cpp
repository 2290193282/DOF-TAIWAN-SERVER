#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_game/dungeon/room/pc_room_server_proxy_runtime.h"
#include "old_game/player/session/nexon_packet_login_runtime.h"

#include <cstdio>
#include <cstring>
#include <new>

// IDA_EA: 0x868152C
// IDA_NAME: _ZN5CUser23SendLoginToPCRoomServerEv
// DEMANGLED: CUser::SendLoginToPCRoomServer(void)
int CUser::SendLoginToPCRoomServer() {
    char packet_bytes[4100];
    nexon_packet::stLogin login;
    std::memset(&login, 0, sizeof(login));

    login.login_type = 3;
    std::strcpy(login.account_name, get_acc_name());
    login.account_name_length =
        static_cast<unsigned char>(std::strlen(get_acc_name()));
    std::strcpy(login.charac_name, getCurCharacName());
    login.charac_name_length =
        static_cast<unsigned char>(std::strlen(getCurCharacName()));
    login.ip_address = get_public_ip_address();
    login.can_try = 0;
    login.uid = GetUID();
    login.account_id = get_acc_id();

    std::printf(
        "SEND nexon_packet::stLogin login:%d AccLen:%d Acc:%s CharLen:%d Char:%d Addr:%d canTry_:%d UserIdx:%d m_id:%d",
        login.login_type,
        login.account_name_length,
        login.account_name,
        login.charac_name_length,
        login.charac_name,
        login.ip_address,
        login.can_try,
        login.uid,
        login.account_id);

    nexon_packet::stLoginPacket* const packet =
        new (packet_bytes) nexon_packet::stLoginPacket(login);
    GlobalData::s_pcroom_proxy->SendTcpPacket(packet_bytes, packet->GetLength());

    nexon_packet::stLogin decoded_login;
    std::memset(&decoded_login, 0, sizeof(decoded_login));
    return nexon_packet::stLoginPacket::ReadPacket(packet_bytes, decoded_login);
}
