#include "old_game/dungeon/drop/packet_dispatcher_drop_item.h"

#include "common/config.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// CMD050 owns the typed message/parameter specialization after the generic
// SR08 shell has been removed from the packet-dispatcher aggregate.
template class IPacketDispatcher<MSG_DROP_ITEM,
                                 stDropItemParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x859516C
// IDA_NAME: _ZN19DisPatcher_DropItemC2Ev
// DEMANGLED: DisPatcher_DropItem::DisPatcher_DropItem(void)
DisPatcher_DropItem::DisPatcher_DropItem()
    : IPacketDispatcher<MSG_DROP_ITEM,
                        stDropItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8232290
// IDA_NAME: _ZN19DisPatcher_DropItemD2Ev
// DEMANGLED: DisPatcher_DropItem::~DisPatcher_DropItem()
// IDA_EA: 0x82322C0
// IDA_NAME: _ZN19DisPatcher_DropItemD0Ev
// DEMANGLED: DisPatcher_DropItem::~DisPatcher_DropItem()
DisPatcher_DropItem::~DisPatcher_DropItem() {}

// IDA_EA: 0x81C2BEE
// IDA_NAME: _ZN19DisPatcher_DropItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DropItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DropItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DROP_ITEM& request = static_cast<MSG_DROP_ITEM&>(message);
    if (packet.get_short(request.pos_x) != 1) {
        return LineFunc(3340,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_short(request.pos_y) != 1) {
        return LineFunc(3341,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_byte(request.item_space) != 1) {
        return LineFunc(3342,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_short(request.slot) != 1) {
        return LineFunc(3343,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(request.count) != 1) {
        return LineFunc(3344,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_byte(request.security_bypass) != 1) {
        return LineFunc(3346,
                        "virtual int DisPatcher_DropItem::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    return 0;
}

// IDA_EA: 0x81C2D9A
// IDA_NAME: _ZN19DisPatcher_DropItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DropItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DropItem::check_error(CUser* user,
                                      const MSG_DROP_ITEM& request,
                                      stDropItemParam& param) {
    if (!user->getCurCharacR() || request.count <= 0) {
        return -1;
    }
    if (user->get_state() == 10) {
        return 19;
    }

    CParty* const party = user->GetParty();
    CDungeon* const dungeon = party ? party->battle_field().get_dungeon() : 0;
    if (dungeon && dungeon->get_blood_dungeon_type() > 0) {
        return 19;
    }
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 19;
    }
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 1)) {
        param.restriction_error = 1;
        return 209;
    }

    int security_error = 0;
    if (request.security_bypass == 1 ||
        (security_error = GlobalData::s_pSecuProtectionField->Check(
             user, static_cast<SECURITY_PROTCTION>(30))) == 0) {
        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(request.item_space));
        if (user->CheckItemLock(inventory_type, request.slot)) {
            return 213;
        }
        if (user->isHackUser()) {
            return 19;
        }
        if (request.item_space == 1 ||
            (request.item_space == 3 &&
             request.slot <= oldsrv::config::kAvatarEquipmentTypeLast)) {
            return 23;
        }
        if (request.item_space || request.slot || request.count > 1000) {
            return 0;
        }
        return 70;
    }

    param.restriction_error = 1;
    return security_error;
}

// IDA_EA: 0x81C2F94
// IDA_NAME: _ZN19DisPatcher_DropItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DropItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DropItem::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    const MSG_DROP_ITEM& request = static_cast<const MSG_DROP_ITEM&>(message);
    stDropItemParam& result = static_cast<stDropItemParam&>(param);
    result.result_code = check_error(user, request, result);
    if (result.result_code > 0) {
        result.item_space = request.item_space;
        return 0;
    }
    if (result.result_code < 0) {
        return LineFunc(3532,
                        "virtual int DisPatcher_DropItem::process(CUser*, MSG_BASE&, ParamBase&)",
                        result.result_code,
                        user->get_acc_id());
    }

    if (user->get_state() == 5) {
        CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
        if (party) {
            const int party_error = party->drop_item(user,
                                                      request.pos_x,
                                                      request.pos_y,
                                                      request.item_space,
                                                      request.slot,
                                                      request.count);
            if (party_error > 0) {
                result.result_code = party_error;
                result.item_space = request.item_space;
            }
        }
    } else if (user->get_state() == 12) {
        WongWork::CBossTower* const tower = user->getBossTower();
        if (tower) {
            static_cast<void>(tower->handleDropItem(user,
                                                    request.pos_x,
                                                    request.pos_y,
                                                    request.item_space,
                                                    request.slot,
                                                    request.count));
        }
    } else if (user->get_state() == 8) {
        WarRoom* const room = G_CGameManager()->GetWarRoom(user->GetWarRoomIndex());
        if (room) {
            const int war_error = room->HandleItemDrop(user,
                                                       request.pos_x,
                                                       request.pos_y,
                                                       request.item_space,
                                                       request.slot,
                                                       request.count);
            if (war_error > 0) {
                result.result_code = war_error;
                result.item_space = request.item_space;
            }
        }
    } else {
        result.result_code = 19;
        result.item_space = request.item_space;
    }
    return 0;
}

// IDA_EA: 0x81C3254
// IDA_NAME: _ZN19DisPatcher_DropItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_DropItem::send(CUser *,ParamBase &)
void DisPatcher_DropItem::send(CUser* user, ParamBase& param) {
    const stDropItemParam& result = static_cast<const stDropItemParam&>(param);
    if (!result.result_code) {
        return;
    }
    if (result.restriction_error == 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DROP_ITEM,
                                 static_cast<unsigned char>(result.result_code));
        return;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_DROP_ITEM);
    response.put_byte(0);
    response.put_byte(static_cast<unsigned char>(result.result_code));
    response.put_byte(result.item_space);
    response.finalize(true);
    user->Send(response);
}
