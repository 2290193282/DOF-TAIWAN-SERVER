#include "old_game/player/core/cuser.h"
#include "common/config.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "old_data/db/auth_security/db_passpad_update_cancel_cnt.h"
#include "old_data/db/auth_security/db_security_card_update_cancel_cnt.h"
#include "old_data/db/legacy_dispatch/db_action_point_update.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>
#include <utility>
#include <vector>

// IDA_EA: 0x8657450
// IDA_NAME: _ZN5CUser14resetDailyDataEv
// DEMANGLED: CUser::resetDailyData(void)
void CUser::resetDailyData() {
    static_cast<CDungeonGainedGold*>(getDungeonGainedGold())->reset();

    WongWork::CSecurityCard* const security_card = getSecurityCard();
    if (security_card->getCancelCnt()) {
        security_card->resetCancelCnt();
        DB_SecurityCardUpdateCancelCnt::makeRequest(
            static_cast<uint32_t>(get_acc_id()), 0u);
    }

    Sanicova::CPad* const pad = getPad();
    if (pad->getCancelCnt() > 0) {
        pad->setCancelCnt(0);
        DB_PassPadUpdateCancelCnt::makeRequest(
            static_cast<uint32_t>(get_acc_id()), 0u, getWebAddress());
    }

    resetNPCRelationShipDailyData();
    sendNPCRelationShipFavor();

    const APSystem::_SIG_LOAD_ACTION_POINT* const action_point =
        action_point_manager_.GetActionPoint();
    APSystem::DB_UpdateActionPoint::makeRequest(
        GetUID(), *action_point, true);
    APSystem::CUserProc::SetTodayActionAndCheckMedalReward(this);
}

// IDA_EA: 0x865784E
// IDA_NAME: _ZN5CUser15recover_staminaEv
// DEMANGLED: CUser::recover_stamina(void)
int CUser::recover_stamina() {
    if (get_state() <= 2 || get_state() == 5) {
        return 0;
    }

    if (getCurCharacStamina() <= 99) {
        addCurcharacStamina(9);
        if (getCurCharacStamina() > 100) {
            setCurCharacStamina(100);
        }

        PacketGuard packet;
        packet.put_header(0, 4);
        packet.put_byte(getCurCharacStamina());
        packet.finalize(true);
        Send(packet);
    }

    return getCurCharacStamina();
}

// IDA_EA: 0x867E092
// IDA_NAME: _ZN5CUser19processDelDailyItemEv
// DEMANGLED: CUser::processDelDailyItem(void)
void CUser::processDelDailyItem() {
    std::vector<std::pair<int, int> > changed_slots;
    deleteDailyStackableItem(changed_slots,
                             oldsrv::config::kMainInventoryQuickSlotFirst,
                             oldsrv::config::kMainInventoryQuickSlotLast);
    deleteDailyStackableItem(changed_slots,
                             oldsrv::config::kMainInventoryConsumableSlotFirst,
                             oldsrv::config::kMainInventoryConsumableSlotLast);
    if (changed_slots.empty()) {
        return;
    }

    PacketGuard packets[29];
    int packet_indices[29] = {};
    int packet_counts[29] = {};
    for (int item_space = 0; item_space <= 28; ++item_space) {
        PacketGuard& packet = packets[item_space];
        packet.put_header(0, 14);
        packet.put_byte(item_space);
        packet_indices[item_space] = packet.get_index();
        packet.put_short(0);
    }

    for (std::vector<std::pair<int, int> >::const_iterator it =
             changed_slots.begin();
         it != changed_slots.end();
         ++it) {
        const int item_space = it->first;
        const int slot = it->second;
        PacketGuard& packet = packets[item_space];

        if (item_space == 2) {
            getCurCharacCargoR()->MakeItemPacket(&packet, slot);
            ++packet_counts[item_space];
        } else if (item_space == 12) {
            if (IsExistAccountCargo()) {
                GetAccountCargo()->MakeItemPacket(&packet, slot);
                ++packet_counts[item_space];
            }
        } else {
            int inventory_type = 1;
            switch (item_space) {
                case 3:
                    inventory_type = 0;
                    break;
                case 7:
                    inventory_type = 3;
                    break;
                case 1:
                    inventory_type = 2;
                    break;
            }

            const Inven_Item item = getCurCharacInvenR()->GetInvenSlot(
                inventory_type, slot);
            if (!item.item_id_) {
                packet.put_short(slot);
                packet.put_int(-1);
                packet.put_int(0);
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_int(0);
                packet.put_byte(0);
                packet.put_short(0);
                packet.put_byte(0);
                packet.put_packet(g_emptySlot);
            }
            ++packet_counts[item_space];
        }
    }

    for (int item_space = 0; item_space <= 28; ++item_space) {
        if (packet_counts[item_space]) {
            PacketGuard& packet = packets[item_space];
            packet.put_short(packet_indices[item_space],
                             packet_counts[item_space]);
            packet.finalize(true);
            Send(packet);
        }
    }
}
