#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>


namespace {

int ChronicleEnchantItemId(unsigned char attr) {
    switch ((attr >> 1) & 3u) {
        case 0:
            return 1254;
        case 1:
            return 1255;
        case 2:
            return 1256;
        case 3:
            return 1257;
    }
    return -1;
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x854A9AA
// IDA_NAME: _ZN8WongWork12CItemUpgrade22make3rdChroniclePacketEP5CUserRK10Inven_ItemP11PacketGuard
// DEMANGLED: WongWork::CItemUpgrade::make3rdChroniclePacket(CUser*, Inven_Item const&, PacketGuard*)
int CItemUpgrade::make3rdChroniclePacket(CUser* user,
                                         const Inven_Item& item,
                                         PacketGuard* packet) {
    if (!user) {
        return 0;
    }

    unsigned char attr_count = 0;
    const unsigned char attributes[2] = {
        item.chronicle_random_option_.enchanted_skill_options_[0].flags_,
        item.chronicle_random_option_.enchanted_skill_options_[1].flags_};
    const unsigned char attr0 = attributes[0];
    const unsigned char attr1 = attributes[1];
    if ((attr0 & 1u) != 0) {
        ++attr_count;
        if ((attr1 & 1u) != 0) {
            ++attr_count;
        }

        CDataManager* data_manager = G_CDataManager();
        CItem* data_item =
            data_manager
                ? data_manager->find_item(
                      item.item_id_)
                : 0;
        if (!data_item) {
            return 0;
        }

        packet->put_byte(attr_count);
        for (int i = 0; i < attr_count; ++i) {
            const unsigned char attr = attributes[i];
            packet->put_int(ChronicleEnchantItemId(attr));
            packet->put_byte(user->get_charac_job());
            packet->put_byte(user->getCurCharFirstGrowType());
            packet->put_byte(data_item->vtable_->get_item_type(data_item));
            packet->put_byte(attr >> 3);
        }
        return 1;
    }

    packet->put_byte(attr_count);
    return 0;
}

}  // namespace WongWork
