#include "old_game/economy/inventory/avatar_coin.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/economy/inventory/inventory.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x817FFE4
// IDA_NAME: _ZN10AvatarCoin14SendSyncPacketEP5CUser
// DEMANGLED: AvatarCoin::SendSyncPacket(CUser *)
int AvatarCoin::SendSyncPacket(CUser* user) {
    if (!user || !user->getCurCharacR()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_AVATAR_COIN_COUNT);
    packet.put_byte(1);
    packet.put_int(user->getCurCharacInvenR()->avatar_coin_.Get());
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x817FF54
// IDA_NAME: _ZN10AvatarCoin10HistoryLog6SubLogEP5CUserj
// DEMANGLED: AvatarCoin::HistoryLog::SubLog(CUser *,unsigned int)
void AvatarCoin::HistoryLog::SubLog(CUser* user, unsigned int amount) {
    if (!user) {
        return;
    }

    user->LogHistory("AvatarCoin-,%d,%d",
                     amount,
                     user->getCurCharacInvenR()->avatar_coin_.Get());
}

// IDA_EA: 0x817FF9C
// IDA_NAME: _ZN10AvatarCoin10HistoryLog6AddLogEP5CUserj
// DEMANGLED: AvatarCoin::HistoryLog::AddLog(CUser *,unsigned int)
void AvatarCoin::HistoryLog::AddLog(CUser* user, unsigned int amount) {
    if (!user) {
        return;
    }

    user->LogHistory("AvatarCoin+,%d,%d",
                     amount,
                     user->getCurCharacInvenR()->avatar_coin_.Get());
}
