#include "old_game/economy/inventory/packet_dispatcher_move_item.h"
#include "common/config.h"
#include "old_game/activity/schedule/freely_give_item.h"

#include <limits.h>

#include "old_game/economy/trade/trade_limit_item_abi.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "infra/log/format/trace.h"

namespace {

const int kTradeRestrictedDestination = 14;

bool ReadMessage(PacketBuf& packet, MSG_MOVE_ITEMSPACE& message) {
    // PacketBuf decodes into references.  The wire request is deliberately
    // packed (the original CMD19 offsets include unaligned 16/32-bit fields),
    // so decode each value through a naturally aligned local before storing it
    // at its physical field.  This preserves both the packet order and the
    // original partial-read behavior without forming a reference to a packed
    // field.
    char source_space = 0;
    int16_t source_slot = 0;
    uint32_t source_item_id = 0;
    int32_t source_add_info = 0;
    char destination_space = 0;
    int16_t destination_slot = 0;
    uint32_t destination_item_id = 0;
    int32_t target_add_info = 0;
    char channel_equip_switch = 0;
    char expand_equip_switch = 0;

    if (packet.get_byte(source_space) != 1) return false;
    message.source_space = source_space;
    if (packet.get_short(source_slot) != 1) return false;
    message.source_slot = source_slot;
    if (packet.get_int(source_item_id) != 1) return false;
    message.source_item_id = source_item_id;
    if (packet.get_int(source_add_info) != 1) return false;
    message.source_add_info = source_add_info;
    if (packet.get_byte(destination_space) != 1) return false;
    message.destination_space = destination_space;
    if (packet.get_short(destination_slot) != 1) return false;
    message.destination_slot = destination_slot;
    if (packet.get_int(destination_item_id) != 1) return false;
    message.destination_item_id = destination_item_id;
    if (packet.get_int(target_add_info) != 1) return false;
    message.target_add_info = target_add_info;
    if (packet.get_byte(channel_equip_switch) != 1) return false;
    message.channel_equip_switch = channel_equip_switch;
    if (packet.get_byte(expand_equip_switch) != 1) return false;
    message.expand_equip_switch = expand_equip_switch;
    return true;
}

}  // namespace

// IDA_NAME: stMoveItemParam::stMoveItemParam(void)
stMoveItemParam::stMoveItemParam()
    : ParamBase(),
      error(0),
      source_space(0),
      padding_09(0),
      source_slot(0),
      source_add_info(0),
      destination_space(0),
      padding_11{0, 0, 0},
      actual_destination_slot(0) {}

// IDA_EA: 0x85951DC
// IDA_NAME: _ZN19DisPatcher_MoveItemC2Ev
// DEMANGLED: DisPatcher_MoveItem::DisPatcher_MoveItem(void)
DisPatcher_MoveItem::DisPatcher_MoveItem()
    : IPacketDispatcher<MSG_MOVE_ITEMSPACE,
                        stMoveItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8232410
// IDA_NAME: _ZN19DisPatcher_MoveItemD2Ev
// DEMANGLED: DisPatcher_MoveItem::~DisPatcher_MoveItem()
// IDA_EA: 0x8232440
// IDA_NAME: _ZN19DisPatcher_MoveItemD0Ev
// DEMANGLED: DisPatcher_MoveItem::~DisPatcher_MoveItem()
DisPatcher_MoveItem::~DisPatcher_MoveItem() {}

// IDA_EA: 0x81C54E0
// IDA_NAME: _ZN19DisPatcher_MoveItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_MoveItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_MoveItem::read(PacketBuf& packet, MSG_BASE& message) {
    return ReadMessage(packet, static_cast<MSG_MOVE_ITEMSPACE&>(message)) ? 0 : 1;
}

// IDA_EA: 0x81C57A0
// IDA_NAME: _ZN19DisPatcher_MoveItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_MoveItem::check_error(CUser *,MSG_BASE &)
int DisPatcher_MoveItem::check_error(CUser* user, MSG_BASE& base_message) {
    MSG_MOVE_ITEMSPACE& message =
        static_cast<MSG_MOVE_ITEMSPACE&>(base_message);
    if (message.source_add_info < 0) return -1;

    bool check_trade_limit_count = false;
    bool check_item_lock = false;
    const int destination_space = message.destination_space;
    if (destination_space > 5) {
        if (destination_space == kTradeRestrictedDestination) {
            check_trade_limit_count = true;
            check_item_lock = true;
        }
    } else if (destination_space >= 4) {
        check_trade_limit_count = true;
        check_item_lock = true;
    }

    // Equipment destinations are reversible wear operations.  The lock gate
    // above intentionally remains only on trade/destructive destinations.

    if (check_trade_limit_count) {
        bool can_move = true;
        CheckItemTradeLimitCountToMoveByUser(
            *user,
            GetInvenTypeFromItemSpace(
                static_cast<ENUM_ITEMSPACE>(message.source_space)),
            message.source_slot,
            can_move);
        if (!can_move) return 114;
    }
    if (check_item_lock) {
        if (user->CheckItemLock(
                GetInvenTypeFromItemSpace(
                    static_cast<ENUM_ITEMSPACE>(message.source_space)),
                message.source_slot)) {
            return 213;
        }
    }
    return 0;
}

// IDA_EA: 0x81C5904
// IDA_NAME: _ZN19DisPatcher_MoveItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_MoveItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_MoveItem::process(CUser* user,
                                 MSG_BASE& base_message,
                                 ParamBase& base_param) {
    MSG_MOVE_ITEMSPACE& message = static_cast<MSG_MOVE_ITEMSPACE&>(base_message);
    stMoveItemParam& param = static_cast<stMoveItemParam&>(base_param);
    param.error = check_error(user, message);
    if (param.error < 0) return param.error;

    if (param.error == 0) {
        const bool is_expanded_equipment_destination =
            (message.destination_space == 3 || message.destination_space == 18) &&
            message.destination_slot >=
                oldsrv::config::kOrdinaryEquipmentTypeFirst &&
            message.destination_slot <=
                oldsrv::config::kOrdinaryEquipmentTypeLast;
        const int result = is_expanded_equipment_destination
            ? user->exchange_interspace_expand(
                  message.source_space,
                  message.source_slot,
                  message.source_item_id,
                  message.source_add_info,
                  message.destination_space,
                  message.destination_slot,
                  message.destination_item_id,
                  &param.actual_destination_slot,
                  message.channel_equip_switch,
                  message.expand_equip_switch)
            : user->exchange_interspace(message.source_space,
                                        message.source_slot,
                                        message.source_item_id,
                                        message.source_add_info,
                                        message.destination_space,
                                        message.destination_slot,
                                        message.destination_item_id,
                                        &param.actual_destination_slot);
        LogManager::logFormat(
            1, "move_item_diag.cpp", "DisPatcher_MoveItem::process", 0,
            "[DIAG0627][CMD19] source_space=%d source_slot=%d source_item=%u source_add=%d destination_space=%d destination_slot=%d destination_item=%u target_add=%d channel_switch=%d expand_switch=%d result=%d actual_destination_slot=%d",
            static_cast<int>(message.source_space),
            static_cast<int>(message.source_slot),
            static_cast<unsigned int>(message.source_item_id),
            static_cast<int>(message.source_add_info),
            static_cast<int>(message.destination_space),
            static_cast<int>(message.destination_slot),
            static_cast<unsigned int>(message.destination_item_id),
            static_cast<int>(message.target_add_info),
            static_cast<int>(message.channel_equip_switch),
            static_cast<int>(message.expand_equip_switch), result,
            param.actual_destination_slot);
        if (result != 0) {
            param.error = result;
            param.source_space = message.source_space;
            param.destination_space = message.destination_space;
            return 0;
        }
        param.source_space = message.source_space;
        param.source_slot = message.source_slot;
        param.source_add_info = message.source_add_info;
        param.destination_space = message.destination_space;
        freely_give_item::SendEventInfo(user);
    } else if (param.error == 213) {
        param.source_space = message.source_space;
        param.destination_space = message.destination_space;
    }
    return 0;
}

// IDA_EA: 0x81C5B76
// IDA_NAME: _ZN19DisPatcher_MoveItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_MoveItem::send(CUser *,ParamBase &)
void DisPatcher_MoveItem::send(CUser* user, ParamBase& base_param) {
    stMoveItemParam& param = static_cast<stMoveItemParam&>(base_param);
    if (param.error == INT_MAX) return;

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_MOVE_ITEMSPACE);
    if (param.error != 0) {
        response.put_byte(0);
        response.put_byte(static_cast<unsigned char>(param.error));
        response.put_byte(param.source_space);
        response.put_byte(param.destination_space);
    } else {
        response.put_byte(1);
        response.put_byte(param.source_space);
        response.put_short(param.source_slot);
        response.put_int(param.source_add_info);
        response.put_byte(param.destination_space);
        response.put_short(static_cast<int16_t>(param.actual_destination_slot));
        // CLIENT_IDA_EVIDENCE: CMD19 response reader @0x570B70 consumes no
        // fields after destination_slot @0x570BF6 on the success path.
    }
    response.finalize(true);
    user->Send(response);

    if (param.error == 0) {
        ARAD::Singleton<AvatarRechargeServer>::Get()->SendDurability(
            user,
            param.source_space,
            param.source_slot,
            param.destination_space,
            param.actual_destination_slot);
    }
}

// The typed state-3 dispatcher owns the raw template family for CMD19.
template class IPacketDispatcher<MSG_MOVE_ITEMSPACE,
                                 stMoveItemParam,
                                 static_cast<ch_state>(3)>;
