#include "old_game/social/party/packet_dispatcher_get_item.h"

#include <limits.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

void AddPickupHack(CUser* user, int type, int value1, int value2) {
    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(type),
        1u,
        static_cast<unsigned int>(value1),
        static_cast<unsigned int>(value2));
}

bool ReadGetItem(PacketBuf& packet, MSG_GET_ITEM& message) {
    uint16_t value = 0;
    if (packet.get_short(value) != 1) return false;
    message.unique_id = value;
    if (packet.get_byte(message.client_routing_flag) != 1) return false;
    if (packet.get_byte(message.client_rarity) != 1) return false;
    if (packet.get_short(value) != 1) return false;
    message.validation_x = value;
    if (packet.get_short(value) != 1) return false;
    message.validation_y = value;

    message.read_status = static_cast<uint8_t>(packet.get_short(value));
    message.encoded_check = value;
    if (message.read_status != 1) return false;
    if (packet.get_short(value) != 1) return false;
    message.history_x = value;
    if (packet.get_short(value) != 1) return false;
    message.history_y = value;
    if (packet.get_short(value) != 1) return false;
    message.packet_extra = value;
    return true;
}

bool IsEncodedCheckValid(uint16_t value) {
    const uint16_t quotient = static_cast<uint16_t>((9363u * value) >> 16);
    const uint16_t half = static_cast<uint16_t>(value - quotient) >> 1;
    const uint16_t folded = static_cast<uint16_t>((half + quotient) >> 2);
    return value == static_cast<uint16_t>(7u * folded);
}

}  // namespace

// IDA_NAME: stGetItemParam::stGetItemParam(void)
stGetItemParam::stGetItemParam() : ParamBase(), error(INT_MAX) {}

// IDA_EA: 0x8595188
// IDA_NAME: _ZN18DisPatcher_GetItemC2Ev
// DEMANGLED: DisPatcher_GetItem::DisPatcher_GetItem(void)
DisPatcher_GetItem::DisPatcher_GetItem()
    : IPacketDispatcher<MSG_GET_ITEM,
                        stGetItemParam,
                        static_cast<ch_state>(5)>() {}

// IDA_EA: 0x82322DE
// IDA_NAME: _ZN18DisPatcher_GetItemD2Ev
// DEMANGLED: DisPatcher_GetItem::~DisPatcher_GetItem()
// IDA_EA: 0x823230E
// IDA_NAME: _ZN18DisPatcher_GetItemD0Ev
// DEMANGLED: DisPatcher_GetItem::~DisPatcher_GetItem()
DisPatcher_GetItem::~DisPatcher_GetItem() {}

// IDA_EA: 0x81C335C
// IDA_NAME: _ZN18DisPatcher_GetItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_GetItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_GetItem::read(PacketBuf& packet, MSG_BASE& message) {
    return ReadGetItem(packet, static_cast<MSG_GET_ITEM&>(message)) ? 0 : 1;
}

// IDA_EA: 0x81C35AC
// IDA_NAME: _ZN18DisPatcher_GetItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_GetItem::check_error(CUser *,MSG_BASE &)
int DisPatcher_GetItem::check_error(CUser*, MSG_GET_ITEM&) {
    return 0;
}

// IDA_EA: 0x81C35C0
// IDA_NAME: _ZN18DisPatcher_GetItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_GetItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_GetItem::process(CUser* user,
                                MSG_BASE& base_message,
                                ParamBase& base_param) {
    MSG_GET_ITEM& message = static_cast<MSG_GET_ITEM&>(base_message);
    stGetItemParam& param = static_cast<stGetItemParam&>(base_param);
    param.error = check_error(user, message);
    if (param.error != 0) return param.error < 0 ? param.error : 0;

    if (message.read_status != 1) {
        AddPickupHack(user, 955, 0, 0);
    }
    if (!IsEncodedCheckValid(message.encoded_check)) {
        AddPickupHack(user, 303, message.validation_x, message.validation_y);
    }
    user->history_log().SetItemXYPos(message.history_x, message.history_y);

    if (user->get_state() == 10) {
        WongWork::CDeathTower* const tower = user->getDeathTower();
        if (tower && tower->handlePickupItem(user,
                                             message.unique_id,
                                             message.client_routing_flag == 1,
                                             static_cast<char>(message.client_rarity))) {
            param.error = 0;
            return 0;
        }
        param.error = INT_MAX;
        return 0;
    }
    if (user->get_state() == 12) {
        Global::CBossTowerFunc::GetItem(user,
                                        message.unique_id,
                                        message.client_routing_flag == 1,
                                        static_cast<char>(message.client_rarity));
        param.error = INT_MAX;
        return 0;
    }
    if (user->get_state() == 8) {
        WarRoom* const room = G_CGameManager()->GetWarRoom(user->GetWarRoomIndex());
        if (room && room->HandleGetItem(user, message.unique_id)) {
            param.error = 0;
            return 0;
        }
        param.error = INT_MAX;
        return 0;
    }
    if (user->get_state() != 5) {
        param.error = 19;
        return 0;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        param.error = INT_MAX;
        return 0;
    }

    if (user->GetItemTimeChecker(GlobalData::s_systemTime_.getCurTickCount(),
                                 message.history_x,
                                 message.history_y,
                                 1500u) &&
        user->IsGetItemDistanceOver(500)) {
        AddPickupHack(user, 304, 0, 0);
    }
    if (user->GetItemSerialChecker(message.history_x,
                                   message.history_y,
                                   message.unique_id) == 2) {
        AddPickupHack(user, 305, 0, 0);
    }
    if (party->IsRoutingState()) {
        param.error = 216;
        return 0;
    }

    int item_id = -1;
    unsigned char item_upgrade = 0;
    bool stackable = false;
    if (!party->get_item(user,
                         message.unique_id,
                         message.client_routing_flag == 1,
                         static_cast<char>(message.client_rarity),
                         item_id,
                         item_upgrade,
                         stackable)) {
        param.error = INT_MAX;
        return 0;
    }

    if (party->IsAutoCreated() || party->get_member_count() == 1) {
        if (!checkGetItemPacket(user->getBackupSeed(),
                                message.validation_x,
                                message.validation_y)) {
            AddPickupHack(user, 302, 0, 0);
        }
    }

    CDungeon* const dungeon = G_CDataManager()->find_dungeon(
        party->battle_field().get_dungeon_index());
    if (dungeon && item_id != -1 && dungeon->isEventDungeon() &&
        (message.client_rarity == 2 || message.client_rarity == 3 ||
         message.client_rarity == 4) &&
        !stackable) {
        PacketGuard broadcast;
        broadcast.put_header(0, ENUM_NOTIPACKET_SERVER_BROADCAST);
        broadcast.put_byte(5);
        broadcast.put_short(user->get_unique_id());
        broadcast.put_int(item_id);
        broadcast.finalize(true);
        G_GameWorld()->send_all(broadcast);
    }

    (void)item_upgrade;
    param.error = 0;
    return 0;
}

// IDA_EA: 0x81C3CE8
// IDA_NAME: _ZN18DisPatcher_GetItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_GetItem::send(CUser *,ParamBase &)
void DisPatcher_GetItem::send(CUser* user, ParamBase& base_param) {
    const stGetItemParam& param = static_cast<const stGetItemParam&>(base_param);
    if (param.error == INT_MAX) return;
    if (param.error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GET_ITEM,
                                 static_cast<unsigned char>(param.error));
    } else {
        user->SendCmdOkPacket(ENUM_CMDPACKET_GET_ITEM);
    }
}

template class IPacketDispatcher<MSG_GET_ITEM,
                                 stGetItemParam,
                                 static_cast<ch_state>(5)>;
