#include "old_game/economy/shop/packet_dispatcher_sell_item.h"

#include <stdint.h>

#include "common/config.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

const ENUM_CMDPACKET kCmdSellItem = ENUM_CMDPACKET_SELL_ITEM;
const int kAuthMobileRestrictId = 3;
const int kSellSecurityProtection = 35;

void SendSellError(CUser* user, unsigned char error) {
    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET, kCmdSellItem);
    response.put_byte(0);
    response.put_byte(error);
    response.finalize(true);
    user->Send(response);
}

bool IsAllowedState(int state) {
    return state == 3 || state == 5 || state == 8 || state == 10 ||
           state == 12;
}

void RecordSellTimeHack(CUser* user) {
    const int interval_state = user->SellItemTimeChecker(
        GlobalData::s_systemTime_.getCurTickCount(), 1000u);
    if (interval_state == 2) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(312), 1u, 0u, 0u);
    } else if (interval_state == 4) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(801), 1u, 0u, 0u);
    }
}

void CheckNpcPresence(CUser* user) {
    if (user->get_state() == 5) {
        CParty* const party = user->GetParty();
        if (party) {
            CMap* const map = G_CDataManager()->find_map(
                party->battle_field().get_cur_map());
            if (map && map->getAssignedNPCCount() == 0 &&
                party->battle_field().IsAssignNPCByPotion() != 1) {
                user->getHackAnalyzer()->addServerHackCnt(
                    user,
                    static_cast<WongWork::ENUM_HACKTYPE>(803),
                    1u,
                    0u,
                    0u);
            }
        }
    }

    if (user->get_state() != 3) {
        return;
    }
    GameWorld* const world = G_GameWorld();
    const int village = static_cast<signed char>(user->getCurCharacVill());
    const int area_index = user->get_area(false);
    if (!world || !world->check_valid_area(village, area_index)) {
        return;
    }
    Village* const user_village = world->getUserVillage(user);
    Area* const area = user_village ? user_village->getArea(area_index) : 0;
    if (area && area->IsExistItemShopNPC() != 1) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(804), 1u, 0u, 0u);
    }
}

}  // namespace

// IDA_EA: 0x859552C
// IDA_NAME: _ZN19DisPatcher_SellItemC2Ev
// DEMANGLED: DisPatcher_SellItem::DisPatcher_SellItem(void)
DisPatcher_SellItem::DisPatcher_SellItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8232F18
// IDA_NAME: _ZN19DisPatcher_SellItemD2Ev
// DEMANGLED: DisPatcher_SellItem::~DisPatcher_SellItem()
// IDA_EA: 0x8232F48
// IDA_NAME: _ZN19DisPatcher_SellItemD0Ev
// DEMANGLED: DisPatcher_SellItem::~DisPatcher_SellItem()
DisPatcher_SellItem::~DisPatcher_SellItem() {}

int DisPatcher_SellItem::dispatch_template(CUser* user, PacketBuf& packet) {
    return dispatch_sig(user, packet);
}

// IDA_EA: 0x81F2D0C
// IDA_NAME: _ZN19DisPatcher_SellItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SellItem::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SellItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!IsAllowedState(user->get_state())) {
        return 0;
    }

    char item_space = 0;
    short slot = 0;
    short count = 0;
    if (packet.get_byte(item_space) != 1 || packet.get_short(slot) != 1 ||
        packet.get_short(count) != 1) {
        return 0;
    }

    ServiceRestrictManager* const restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (restrict_manager->isRestricted(
            user, RestrictCategory::AuthMobile, kAuthMobileRestrictId)) {
        SendSellError(user, 209);
        return 0;
    }

    Inven_Item* const inventory_item =
        GlobalData::s_pSecuProtectionField->GetInvenItemPtr(
            user, item_space, slot);
    CItem* const item = GlobalData::s_pSecuProtectionField->GetItemPtr(
        user, item_space, slot);
    const int security_error =
        GlobalData::s_pSecuProtectionField->CheckItemRarity(
            user,
            static_cast<SECURITY_PROTCTION>(kSellSecurityProtection),
            item,
            inventory_item);
    if (security_error) {
        SendSellError(user, static_cast<unsigned char>(security_error));
        return 0;
    }
    if (user->CheckItemLockSlotSpace(item_space, slot)) {
        SendSellError(user, 213);
        return 0;
    }

    if (user->get_state() == 5 &&
        user->getInDungeonItemSellCount(item_space, slot) == 7) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(313), 1u, 0u, 0u);
    }

    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        SendSellError(user, 60);
        return 0;
    }

    unsigned int client_checksum = 0;
    if (packet.get_int(client_checksum) != 1) {
        return 0;
    }
    if (static_cast<unsigned int>(slot) +
            static_cast<unsigned char>(item_space) +
            static_cast<unsigned short>(count) !=
        client_checksum) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(821), 1u, 0u, 0u);
    }
    if (item_space == 1 ||
        (item_space == 3 &&
         slot <= oldsrv::config::kAvatarEquipmentTypeLast)) {
        SendSellError(user, 23);
        return 0;
    }

    CheckNpcPresence(user);
    const int sell_error = G_Store()->user_sell_item(
        user, item_space, slot, count);
    if (sell_error) {
        SendSellError(user, static_cast<unsigned char>(sell_error));
        if (sell_error == 22) {
            user->SendMoneyFullReason(
                static_cast<ENUM_MONEY_FULL_REASON>(3), 0u, 0u);
        }
    }
    RecordSellTimeHack(user);
    return 0;
}

// IDA_EA: 0x8232F04
// IDA_NAME: _ZN19DisPatcher_SellItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SellItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SellItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232F0E
// IDA_NAME: _ZN19DisPatcher_SellItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SellItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SellItem::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
