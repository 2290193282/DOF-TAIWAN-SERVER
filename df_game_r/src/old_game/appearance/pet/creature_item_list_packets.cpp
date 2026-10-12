#include "old_game/appearance/pet/creature.h"

#include "infra/network/packet/packet_guard.h"

#include <cstring>

namespace user_creature {

// IDA_EA: 0x83409E0
// IDA_NAME: _ZN13user_creature17CPacketRespondent30MakeNotipacketCreatureItemListEPKSt3mapIiPNS_13CCreatureItemESt4lessIiESaISt4pairIKiS3_EEEi
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketCreatureItemList(std::map<int,user_creature::CCreatureItem *,std::less<int>,std::allocator<std::pair<int const,user_creature::CCreatureItem *>>> const*,int)
int CPacketRespondent::MakeNotipacketCreatureItemList(
    const std::map<int, CCreatureItem*>* creature_items,
    int slot) {
    PacketGuard& packet = *this;
    packet.clear();
    packet.put_header(0, 105);
    int count_index = packet.get_index();
    packet.put_byte(0);

    int count = 0;
    for (std::map<int, CCreatureItem*>::const_iterator it =
             creature_items->begin();
         it != creature_items->end();
         ++it) {
        CCreatureItem* const item = it->second;
        if (!item || item->GetCreatureItemType() != CREATURE_ITME_TYPE_CREATURE ||
            (slot != -1 && item->GetSlotNo() != slot)) {
            continue;
        }

        CCreature* const creature = static_cast<CCreature*>(item);
        packet.put_int(creature->GetUid());
        packet.put_byte(creature->GetStomach());
        const int exp = creature->GetExp();
        if (creature->IsGrowCreature()) {
            packet.put_byte(1);
            packet.put_int(static_cast<short>(
                static_cast<unsigned int>(exp) >> 16));
            packet.put_byte(10);
            packet.put_byte(static_cast<unsigned char>(exp));
        } else {
            packet.put_byte(0);
            packet.put_int(exp);
        }
        packet.put_byte(creature->GetLevel());
        const char* const name = creature->GetName();
        const int name_length = static_cast<int>(std::strlen(name));
        packet.put_int(name_length);
        packet.put_str(name, name_length);
        packet.put_byte(creature->IsNoCharge());
        ++count;
    }

    packet.put_byte(count_index, count);
    return finalize(true);
}

// IDA_EA: 0x833BDD2
// IDA_NAME: _ZNK13user_creature12CCreatureMgr20SendCreatureItemListEv
// DEMANGLED: user_creature::CCreatureMgr::SendCreatureItemList(void)const
int CCreatureMgr::SendCreatureItemList() const {
    CPacketRespondent respondent(owner_, 0, 0);
    respondent.MakeNotipacketCreatureItemList(&creature_items_, -1);
    return respondent.Send();
}

}  // namespace user_creature
