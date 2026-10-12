#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_game/dungeon/room/pc_room_server_proxy_runtime.h"
#include "old_game/player/session/nexon_packet_login_runtime.h"

#include <cstring>
#include <new>

// IDA_EA: 0x868170C
// IDA_NAME: _ZN5CUser24SendLogoutToPCRoomServerEv
// DEMANGLED: CUser::SendLogoutToPCRoomServer(void)
int CUser::SendLogoutToPCRoomServer() {
    char packet_bytes[4100];
    nexon_packet::stLogin login;

    login.login_type = 1;
    std::strcpy(login.account_name, get_acc_name());
    (void)std::strlen(get_acc_name());
    std::strcpy(login.account_name, get_acc_name());
    login.account_name_length =
        static_cast<unsigned char>(std::strlen(get_acc_name()));
    std::strcpy(login.charac_name, getCurCharacName());
    login.charac_name_length =
        static_cast<unsigned char>(std::strlen(getCurCharacName()));
    login.ip_address = get_local_ip_address();
    login.can_try = 1;

    // The original function does not initialize uid/account_id before the
    // fixed 8-byte tail is serialized.  Preserve that exact packet behavior.
    nexon_packet::stLoginPacket* const packet =
        new (packet_bytes) nexon_packet::stLoginPacket(login);
    return GlobalData::s_pcroom_proxy->SendTcpPacket(
        packet_bytes, packet->GetLength());
}
