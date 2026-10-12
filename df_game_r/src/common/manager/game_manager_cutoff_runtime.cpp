#include "common/manager/game_manager.h"

#include <cstring>
#include <stdint.h>

#include "core/config/env/environment.h"
#include "common/manager/double_conn_check_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/udp_packets.h"

namespace {

struct TempUsers {
    uint16_t count;
    uint16_t reserved;
    uint32_t account_ids[600];

    TempUsers() : count(0) {}
};

static_assert(sizeof(TempUsers) == 0x964u,
              "CutOff account bucket must retain original 2404-byte extent");

}  // namespace

// IDA_EA: 0x8295E84
// IDA_NAME: _ZN12CGameManager31send_userinfos_to_cutoff_serverEv
// DEMANGLED: CGameManager::send_userinfos_to_cutoff_server(void)
char CGameManager::send_userinfos_to_cutoff_server() {
    TempUsers users[10];
    Packet_CutOff_UDP_Reply_UserInfo packet;
    packet.channel_no = static_cast<uint8_t>(G_CEnvironment()->get_channel_no());
    packet.state = G_CEnvironment()->get_accept_handshake_flag_888();

    if (users_by_account_.empty()) {
        return 1;
    }

    for (UserMap::const_iterator it = users_by_account_.begin();
         it != users_by_account_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() <= 1) {
            continue;
        }
        const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
        TempUsers& bucket = users[account_id % 10u];
        // The fixed original capacity is the same as CGameManager's 600-user
        // pool.  A guard keeps malformed externally-created users from
        // overflowing the old UDP packet while retaining every valid entry.
        if (bucket.count < 600u) {
            bucket.account_ids[bucket.count++] = account_id;
        }
    }

    for (int index = 0; index < 10; ++index) {
        const TempUsers& bucket = users[index];
        if (!bucket.count) {
            continue;
        }
        packet.result = bucket.count;
        const size_t account_bytes =
            static_cast<size_t>(bucket.count) * sizeof(bucket.account_ids[0]);
        std::memcpy(packet.user_info_blob, bucket.account_ids, account_bytes);
        GlobalData::s_double_check_proxy->SendPacket(
            reinterpret_cast<char*>(&packet),
            static_cast<int>(sizeof(PacketHeader) + sizeof(packet.result) +
                             sizeof(packet.channel_no) + sizeof(packet.state) +
                             account_bytes));
    }
    return 0;
}
