#include "old_game/social/party/party.h"

#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x85B8BF8
// IDA_NAME: _ZN6CParty22BloodClearRewardPacketEiiiPK10Inven_ItemiS2_iS2_iS2_
// DEMANGLED: CParty::BloodClearRewardPacket(
//     int,int,int,Inven_Item const*,int,Inven_Item const*,int,
//     Inven_Item const*,int,Inven_Item const*)
void CParty::BloodClearRewardPacket(int member_slot,
                                    int current_blood_round,
                                    int first_item_count,
                                    const Inven_Item* first_items,
                                    int second_item_count,
                                    const Inven_Item* second_items,
                                    int third_item_count,
                                    const Inven_Item* third_items,
                                    int fourth_item_count,
                                    const Inven_Item* fourth_items) {
    PacketGuard packet;
    packet.put_header(0, 209);
    packet.put_byte(current_blood_round);
    packet.put_byte(battle_field_.getBloodMaxRound());

    packet.put_byte(first_item_count);
    for (int item_slot = 0; item_slot < first_item_count; ++item_slot) {
        packet.put_int(first_items[item_slot].item_id_);
        packet.put_int(first_items[item_slot].get_add_info());
    }

    packet.put_byte(second_item_count);
    for (int item_slot = 0; item_slot < second_item_count; ++item_slot) {
        packet.put_int(second_items[item_slot].item_id_);
        packet.put_int(second_items[item_slot].get_add_info());
    }

    packet.put_byte(third_item_count);
    for (int item_slot = 0; item_slot < third_item_count; ++item_slot) {
        packet.put_int(third_items[item_slot].item_id_);
        packet.put_int(third_items[item_slot].get_add_info());
    }

    packet.put_byte(fourth_item_count);
    for (int item_slot = 0; item_slot < fourth_item_count; ++item_slot) {
        packet.put_int(fourth_items[item_slot].item_id_);
        packet.put_int(fourth_items[item_slot].get_add_info());
    }

    packet.finalize(true);
    members_[member_slot].user->Send(packet);
}
