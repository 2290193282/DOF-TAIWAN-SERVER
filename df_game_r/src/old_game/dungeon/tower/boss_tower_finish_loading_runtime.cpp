#include "old_game/pvp/battlefield/battle_field.h"

#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace WongWork {

// IDA_EA: 0x8143F6A
// IDA_NAME: _ZN8WongWork10CBossTower19handleFinishLoadingEP5CUser
// DEMANGLED: WongWork::CBossTower::handleFinishLoading(CUser *)
int CBossTower::handleFinishLoading(CUser* user) {
    const int member_slot = party_->GetMemberSlotNo(user);
    if (member_slot == -1 || party_->checkValidUser(member_slot) != 1) {
        return 0;
    }
    setMemberReady(member_slot, true);
    user->SendCmdOkPacket(ENUM_CMDPACKET_FINISH_LOADING);
    if (checkAllMemberReady(party_->get_member_count())) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
        packet.put_int(0);
        packet.put_byte(0);
        packet.finalize(true);
        party_->send_to_party(packet);
    }
    return 1;
}

// IDA_EA: 0x8466024
// IDA_NAME: _ZN8WongWork11CDeathTower19handleFinishLoadingEP5CUser
// DEMANGLED: WongWork::CDeathTower::handleFinishLoading(CUser *)
int CDeathTower::handleFinishLoading(CUser* user) {
    const int member_slot = party_->GetMemberSlotNo(user);
    if (member_slot == -1 || party_->checkValidUser(member_slot) != 1) {
        return 0;
    }
    if (play_data_.checkEnterStartMap(member_slot)) {
        user->SendCmdOkPacket(ENUM_CMDPACKET_FINISH_LOADING);
    }
    play_data_.setMemberReady(member_slot, true);
    if (play_data_.checkAllMemberReady(party_->get_member_count())) {
        PacketGuard packet;
        packet.put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
        packet.put_int(0);
        packet.put_byte(0);
        packet.finalize(true);
        party_->send_to_party(packet);
    }
    return 1;
}

}  // namespace WongWork
