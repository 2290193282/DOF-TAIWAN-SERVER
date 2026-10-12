#include "old_game/economy/mail/mail_trade_runtime.h"

#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/town/town_script.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

void LogPrivateStoreDecision(const char* stage,
                             int error_code,
                             CUser* seller,
                             int requested_village,
                             int requested_area,
                             int store_state,
                             int commercial_restriction) {
    GameWorld* const world = G_GameWorld();
    LogManager::logFormat(
        1,
        "PrivateStore.cpp",
        "private_store::CPrivateStore::CreatePrivateStore",
        0,
        "[PRIVATE_STORE_DIAG] stage=%s error=%d request_village=%d "
        "request_area=%d current_village=%d current_area=%d user_state=%d "
        "store_state=%d channel_type=%d commercial=%d",
        stage,
        error_code,
        requested_village,
        requested_area,
        seller ? seller->getCurCharacVill() : -1,
        seller ? seller->get_area(false) : -1,
        seller ? seller->get_state() : -1,
        store_state,
        world ? world->GetChannelType() : -1,
        commercial_restriction);
}

}  // namespace

namespace private_store {

// IDA_EA: 0x85C4FF2
// IDA_NAME: _ZN13private_store13CPrivateStore14IsLimitedSpaceERKNS_15PrivateStorePosE
// DEMANGLED: private_store::CPrivateStore::IsLimitedSpace(private_store::PrivateStorePos const&)
bool CPrivateStore::IsLimitedSpace(const PrivateStorePos& other) {
    return position.zone.village == other.zone.village &&
           position.zone.area == other.zone.area &&
           position.x - 50 < other.x && position.x + 50 > other.x &&
           position.y - 80 < other.y && position.y + 80 > other.y;
}

// IDA_EA: 0x85C5082
// IDA_NAME: _ZN13private_store13CPrivateStore25IsAreaNearEntranceDungeonEP5CUser
// DEMANGLED: private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser *)
bool CPrivateStore::IsAreaNearEntranceDungeon(CUser* user) {
    if (!user) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser*)",
            563, 5);
        trace("pUser == NULL");
        return false;
    }

    GameWorld* const world = G_GameWorld();
    if (!world) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser*)",
            569, 5);
        trace("G_GameWorld call failed");
        return false;
    }

    const int area_index = user->get_area(false);
    if (!world->check_valid_area(user->getCurCharacVill(), area_index)) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser*)",
            575, 5);
        trace("G_GameWorld call failed");
        return false;
    }

    Village* const village = world->getUserVillage(user);
    if (!village) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser*)",
            582, 5);
        trace("getUserVillage call failed");
        return false;
    }

    Area* const area = village->getArea(area_index);
    if (!area) {
        cMyTrace trace(
            "bool private_store::CPrivateStore::IsAreaNearEntranceDungeon(CUser*)",
            590, 5);
        trace("getArea call failed (area no:%d)", area_index);
        return false;
    }
    return area->town_area_type != TOWN_AREA_DUNGEON_GATE;
}

// IDA_EA: 0x85C53EA
// IDA_NAME: _ZN13private_store13CPrivateStore18CreatePrivateStoreEhhsst
// DEMANGLED: private_store::CPrivateStore::CreatePrivateStore(unsigned char,unsigned char,short,short,unsigned short)
int CPrivateStore::CreatePrivateStore(uint8_t village,
                                      uint8_t channel,
                                      int16_t x,
                                      int16_t y,
                                      uint16_t doll_slot) {
    error_handler.Set(seller, 1, ENUM_CMDPACKET_CREATE_PRIVATE_STORE);
    LogPrivateStoreDecision("enter", 0, seller, village, channel, state, 0);
    if (!seller) {
        LogPrivateStoreDecision("reject_no_seller", 0, seller, village,
                                channel, state, 0);
        return 0;
    }

    if (seller->IsBusyDisjointStore() || seller->getCurCharacVill() == 7) {
        LogPrivateStoreDecision("reject_busy_or_village", 62, seller,
                                village, channel, state, 0);
        error_handler.Do(62);
        return 0;
    }
    if (!manager) {
        LogPrivateStoreDecision("reject_no_manager", 58, seller, village,
                                channel, state, 0);
        error_handler.Do(58);
        return 0;
    }
    if ((seller->IsPermissionPrivateStore() == 1 && doll_slot != 0xffffu) ||
        (seller->IsPermissionPrivateStore() != 1 && doll_slot == 0xffffu)) {
        LogPrivateStoreDecision("reject_permission_doll", 63, seller,
                                village, channel, state, 0);
        error_handler.Do(63);
        return 0;
    }
    if (seller->get_state() == 6 || G_GameWorld()->GetChannelType() == 2) {
        LogPrivateStoreDecision("reject_user_or_channel_state", 62, seller,
                                village, channel, state, 0);
        error_handler.Do(62);
        return 0;
    }
    if (state) {
        LogPrivateStoreDecision("reject_store_state", 20, seller, village,
                                channel, state, 0);
        error_handler.Do(20);
        return 0;
    }

    const int restriction = G_GameWorld()
        ->check_restrictive_commercial_transaction_zone(village, channel, x, y);
    if (restriction) {
        LogPrivateStoreDecision("reject_commercial_zone", restriction,
                                seller, village, channel, state, restriction);
        error_handler.Do(restriction);
        return 0;
    }
    if (G_GameWorld()->IsPVPChannel()) {
        LogPrivateStoreDecision("reject_pvp_channel", 62, seller, village,
                                channel, state, restriction);
        error_handler.Do(62);
        return 0;
    }
    if (!IsAreaNearEntranceDungeon(seller)) {
        LogPrivateStoreDecision("reject_dungeon_gate", 62, seller, village,
                                channel, state, restriction);
        error_handler.Do(62);
        return 0;
    }

    if (doll_slot != 0xffffu) {
        const int doll_error = CheckPrivateStoreDoll(doll_slot);
        if (doll_error) {
            LogPrivateStoreDecision("reject_doll", doll_error, seller,
                                    village, channel, state, restriction);
            error_handler.Do(doll_error);
            return 0;
        }
        SetDollCreated(true);
        const Inven_Item doll =
            seller->getCurCharacInvenR()->GetInvenSlot(1, doll_slot);
        SetDollItemID(static_cast<unsigned long>(doll.getKey()));
    }

    if (isDollCreated() != 1) {
        seller->set_position(static_cast<uint16_t>(x),
                             static_cast<uint16_t>(y), 0);
    }
    world_position = CPrivateStorePos(this, x, y);

    village_object::Zone object_zone(village, channel);
    if (GlobalData::s_villageObjectMgr->register_object(object_zone,
                                                         &world_position) != 1) {
        LogPrivateStoreDecision("reject_register_object", 56, seller,
                                village, channel, state, restriction);
        error_handler.Do(56);
        return 0;
    }

    position = PrivateStorePos(village, channel, x, y);
    buyers.clear();
    ResetItems();
    state = 1;

    CPacketBufPrivate packet;
    packet.MakeCmdpacketCreatePrivateStore();
    seller->Send(packet);
    packet.MakeNotipacketCreatePrivateStore(GetSellerIndex(), position,
                                            isDollCreated() != 0);
    CBroadcastPrivate::Send(position.zone.village, position.zone.area, packet);

    if (seller->GetParty()) {
        static DisPatcher_LeaveParty leave_party;
        leave_party.dispatch_template(seller, *packet.get());
    }

    cMyTrace trace(
        "virtual bool private_store::CPrivateStore::CreatePrivateStore(unsigned char, unsigned char, short int, short int, short unsigned int)",
        821, 0);
    trace("CreatePrivateStore() uid:%d\n", seller->get_unique_id());
    seller->history_log().CreatePrivateStore(village, channel,
                                             seller->getCurCharacName());

    if (GlobalData::s_auction_proxy->IsRunning()) {
        PCK_AUCTION_OPEN_PRIVATE_STORE_GA auction_packet;
        auction_packet.account_id = seller->get_acc_id();
        const int charac_count = seller->get_charac_count();
        for (int index = 0; index < charac_count; ++index) {
            auction_packet.charac_no[index] = seller->get_charac_no(index);
        }
        GlobalData::s_auction_proxy->SendPacket(
            reinterpret_cast<nsl::PACKET_HEADER*>(&auction_packet));
    }
    LogPrivateStoreDecision("success", 0, seller, village, channel, state,
                            restriction);
    return 1;
}

}  // namespace private_store
