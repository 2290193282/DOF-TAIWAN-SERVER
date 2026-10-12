#include "old_game/dungeon/tower/cdungeon_tower_of_despair_runtime.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8377606
// IDA_NAME: _ZN23CDungeon_TowerOfDespairC2Ev
// DEMANGLED: CDungeon_TowerOfDespair::CDungeon_TowerOfDespair(void)
CDungeon_TowerOfDespair::CDungeon_TowerOfDespair() : CDungeon() {}

// IDA_EA: 0x834D8EC
// IDA_NAME: _ZNK23CDungeon_TowerOfDespair23_do_after_dungeon_startEP5CUser
// DEMANGLED: CDungeon_TowerOfDespair::_do_after_dungeon_start(CUser *)const
int CDungeon_TowerOfDespair::_do_after_dungeon_start(CUser* user) const {
    if (!user) {
        return 0;
    }

    TOD_UserState* const state = static_cast<TOD_UserState*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
    state->IncreaseEnterCount();

    if (state->getEnterCount() > 1 && required_item_.delete_after_start) {
        CInventory* const inventory_read = user->getCurCharacInvenR();
        const int slot = inventory_read->check_item_exist(required_item_.item_id);
        if (slot == -1) {
            return 0;
        }

        const Inven_Item item = inventory_read->GetInvenSlot(1, slot);
        if (item.get_add_info() < required_item_.item_count) {
            return 0;
        }

        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                slot,
                required_item_.item_count,
                static_cast<eItemDelReason>(14),
                true) != 1) {
            cMyTrace trace(
                "virtual bool CDungeon_TowerOfDespair::_do_after_dungeon_start(CUser*) const",
                0x96f,
                5);
            trace("CDungeon_TowerOfDespair::_do_after_dungeon_start, "
                  "delete_item failed , User ch =%d , %d %d",
                  user->getCurCharacNo(),
                  slot,
                  required_item_.item_count);
            return 0;
        }

        const int remaining_count = item.get_add_info() - required_item_.item_count;
        PacketGuard packet;
        packet.put_header(0, 14);
        packet.put_byte(0);
        packet.put_short(1);
        packet.put_short(slot);
        packet.put_int(remaining_count ? item.item_id_ : -1);
        packet.put_int(remaining_count);
        packet.put_byte(item.GetItemAttr());
        packet.put_short(item.avatar_select_);
        packet.put_byte(item.seal_or_state_);
        packet.put_int(item.extend_info_);
        packet.put_byte(item.amplify_option_.getAbilityType());
        packet.put_short(item.amplify_option_.getAbilityValue());
        packet.put_byte(0);
        packet.put_packet(item);
        packet.finalize(true);
        user->Send(packet);
        user->SaveInventory();
    }

    Packet_TowerOfDespair_Statistic_GTS statistic;
    statistic.key.raw_04 = static_cast<int>(
        state->getTodayEnterLayer().GetLayer());
    statistic.byte_12 = 1;
    statistic.dword_13 = static_cast<uint32_t>(user->get_acc_id());
    GlobalData::s_statistic_proxy_mgr
        ->GetServerProxy(static_cast<ENUM_SERVER_GROUP>(0))
        ->SendPacket(reinterpret_cast<char*>(&statistic), sizeof(statistic));
    return 1;
}
