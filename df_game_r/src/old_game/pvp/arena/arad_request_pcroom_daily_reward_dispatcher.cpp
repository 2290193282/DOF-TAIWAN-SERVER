#include "old_game/pvp/arena/arad_request_pcroom_daily_reward_dispatcher.h"

#include <cstring>

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"


namespace ARAD {
namespace DISPATCHER {

namespace {

// CItem's vptr is the canonical CItemVTable ABI in global/item.h.  Copy the
// pointer through a typed local rather than reopening an object byte offset.
const CItemVTable* ItemVTable(const CItem* item) {
    const CItemVTable* table = 0;
    std::memcpy(&table, item, sizeof(table));
    return table;
}

}  // namespace

// IDA_EA: 0x8597F6A
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::Dispatcher_RequestPrcoomDayilyReward(void)
Dispatcher_RequestPrcoomDayilyReward::Dispatcher_RequestPrcoomDayilyReward()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0BCE
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::~Dispatcher_RequestPrcoomDayilyReward()
// IDA_EA: 0x81A0BFE
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyRewardD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::~Dispatcher_RequestPrcoomDayilyReward()
Dispatcher_RequestPrcoomDayilyReward::~Dispatcher_RequestPrcoomDayilyReward() {}

// IDA_EA: 0x81A0612
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestPrcoomDayilyReward::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    if (user == 0) {
        return 2024;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }
    if (user->get_state() <= 2 || user->get_state() > 4) {
        return 0;
    }

    // 0x81A0687 is `test al, al; jg`: this is a signed-char state gate.
    if (static_cast<signed char>(user->getPcroomDailyRewardRemain()) <= 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_PCROOM_DAYILY_REWARD, 22);
        return 0;
    }

    int requested_item_id = 0;
    if (packet.get_int(requested_item_id) != 1) {
        return 2043;
    }
    if (requested_item_id != 8495 && requested_item_id != 8496) {
        return 2048;
    }

    CItem* const item = G_CDataManager()->find_item(requested_item_id);
    if (!item) {
        return 0;
    }

    const CItemVTable* const item_vtable = ItemVTable(item);
    if (item_vtable->get_item_type(item) != 0) {
        return 0;
    }

    Inven_Item inventory_item;
    // The old body writes the physical item-id member (+0x02) before the
    // slot-2 make_item call.
    inventory_item.item_id_ = item->get_index();
    item_vtable->make_item(item, inventory_item);
    inventory_item.set_add_info(1);

    const int inserted = user->getCurCharacInvenW()->insertItemIntoInventory(
        inventory_item, static_cast<eItemAddReason>(14), true, true);
    if (inserted < 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_PCROOM_DAYILY_REWARD, 4);
    } else {
        user->SendCmdOkPacket(ENUM_CMDPACKET_REQUEST_PCROOM_DAYILY_REWARD);
        user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             inserted);
        user->decrementPcroomDailyRewardRemain();
        InterSelectPcroomDailyReward::SendPacket(user);
        DBUpdatePcroomDailyReward::makeRequest(user);
    }
    return 0;
}

// IDA_EA: 0x81A0B86
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestPrcoomDayilyReward::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    // Old body: xor eax, eax; ret.
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B90
// IDA_NAME: _ZN4ARAD10DISPATCHER36Dispatcher_RequestPrcoomDayilyReward7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_RequestPrcoomDayilyReward::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestPrcoomDayilyReward::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    // Old body: xor eax, eax; ret.
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
