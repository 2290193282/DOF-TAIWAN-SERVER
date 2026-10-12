#include "old_game/gm/player/game_master_character.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

#include <cstring>


// IDA_EA: 0x84B42B6
// IDA_NAME: _ZN20CGameMasterCharacter15sendServerStateEP5CUserPKc
// DEMANGLED: CGameMasterCharacter::sendServerState(CUser *,char const*)
void CGameMasterCharacter::sendServerState(CUser* user, const char* state) {
    (void)user;
    (void)state;
}

// IDA_EA: 0x84B4426
// IDA_NAME: _ZN20CGameMasterCharacterC2Ev
// DEMANGLED: CGameMasterCharacter::CGameMasterCharacter(void)
CGameMasterCharacter::CGameMasterCharacter() {}

// IDA_EA: 0x84B4330
// IDA_NAME: _ZN32CGameMasterCharacterItemDropRate15sendServerStateEP5CUserPKc
// DEMANGLED: CGameMasterCharacterItemDropRate::sendServerState(CUser *,char const*)
void CGameMasterCharacterItemDropRate::sendServerState(CUser* user,
                                                       const char* state) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_MESSAGE);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    const int state_len = static_cast<int>(std::strlen(state));
    packet.put_int(state_len);
    packet.put_str(state, state_len);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x84B4434
// IDA_NAME: _ZN32CGameMasterCharacterItemDropRateC2Ev
// DEMANGLED: CGameMasterCharacterItemDropRate::CGameMasterCharacterItemDropRate(void)
CGameMasterCharacterItemDropRate::CGameMasterCharacterItemDropRate()
    : CGameMasterCharacter() {}
