#include "old_game/economy/mall/packet_dispatcher_commerce_auction.h"

#include <stdint.h>

#include "common/config.h"
#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_effect_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/trade/vending_machine_public_abi.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/prepay_info_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/slang_filter.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/core/db_runtime_payloads.h"
#include "old_data/db/statistics_log/db_update_vending_machine_statistic.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/social/party/party.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mall/crane_minigame_runtime.h"
#include "old_game/economy/mail/mail_trade_packets.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/trade/timer_vending_machine_item_notify.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/mt_rand.h"
#include "common/manager/system_time.h"

#include <algorithm>
#include <ctime>
#include <cstring>
#include <stdio.h>
#include <string>
#include <vector>

enum ENUM_ERROR : int;

namespace {

const char kDepositDispatchSignature[] =
    "virtual int Dispatcher_DepositMoney::dispatch_sig(CUser*, PacketBuf&)";
const char kWithdrawDispatchSignature[] =
    "virtual int Dispatcher_WithdrawMoney::dispatch_sig(CUser*, PacketBuf&)";
const char kBuyPrivateStoreItemDispatchSignature[] =
    "virtual int Dispatcher_BuyPrivateStoreItem::dispatch_sig(CUser*, PacketBuf&)";
const char kEnterPrivateStoreDispatchSignature[] =
    "virtual int Dispatcher_EnterPrivateStore::dispatch_sig(CUser*, PacketBuf&)";
const char kCreatePrivateStoreDispatchSignature[] =
    "virtual int Dispatcher_CreatePrivateStore::dispatch_sig(CUser*, PacketBuf&)";
const char kCompleteDisplayDispatchSignature[] =
    "virtual int Dispatcher_CompleteDisplay::dispatch_sig(CUser*, PacketBuf&)";
const char kVerifyPrivateStoreItemDispatchSignature[] =
    "virtual int Dispatcher_VerifyPrivateStoreItem::dispatch_sig(CUser*, PacketBuf&)";
const char kUseCraneStartDispatchSignature[] =
    "virtual int Dispatcher_UseCraneStart::dispatch_sig(CUser*, PacketBuf&)";
const char kCranePickUpDispatchSignature[] =
    "virtual int Dispatcher_CranePickUp::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionMyRegistedItemInfoDispatchSignature[] =
    "virtual int Dispatcher_AuctionMyRegistedItemInfo::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionMyBiddingInfoDispatchSignature[] =
    "virtual int Dispatcher_AuctionMyBiddingInfo::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionMyAuctionHistoryDispatchSignature[] =
    "virtual int Dispatcher_AuctionMyAuctionHistory::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionRegistItemDispatchSignature[] =
    "virtual int Dispatcher_AuctionRegistItem::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionRegistCancelDispatchSignature[] =
    "virtual int Dispatcher_AuctionRegistCancel::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionBiddingDispatchSignature[] =
    "virtual int Dispatcher_AuctionBidding::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionBuyItemApieceDispatchSignature[] =
    "virtual int Dispatcher_AuctionBuyItemApiece::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionSearchByNoItemKeyDispatchSignature[] =
    "virtual int Dispatcher_AuctionSearchByNoItmeKey::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionSearchByItemKeyDispatchSignature[] =
    "virtual int Dispatcher_AuctionSearchByItemKey::dispatch_sig(CUser*, PacketBuf&)";
const char kAuctionAskAveragePriceDispatchSignature[] =
    "virtual int Dispatcher_AuctionAskAveragePrice::dispatch_sig(CUser*, PacketBuf&)";
const char kUseVendingMachineDispatchSignature[] =
    "virtual int Dispatcher_UseVendingMachine::dispatch_sig(CUser*, PacketBuf&)";

// IDA 0x821B71C checks the post-create Inven_Item category byte against 5
// before it uses the creature manager.  The canonical Inven_Item owner keeps
// that byte public as part of its verified 0x3d-byte storage ABI.
bool IsVendingMachineCreatureEggRecord(const Inven_Item& item) {
    return item.item_category_ == 5u;
}

}  // namespace

// IDA_EA: 0x8597164
// IDA_NAME: _ZN23Dispatcher_DepositMoneyC2Ev
// DEMANGLED: Dispatcher_DepositMoney::Dispatcher_DepositMoney(void)
Dispatcher_DepositMoney::Dispatcher_DepositMoney()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240750
// IDA_NAME: _ZN23Dispatcher_DepositMoneyD2Ev
// DEMANGLED: Dispatcher_DepositMoney::~Dispatcher_DepositMoney()
// IDA_EA: 0x8240780
// IDA_NAME: _ZN23Dispatcher_DepositMoneyD0Ev
// DEMANGLED: Dispatcher_DepositMoney::~Dispatcher_DepositMoney()
Dispatcher_DepositMoney::~Dispatcher_DepositMoney() {}

// IDA_EA: 0x8224FE4
// IDA_NAME: _ZN23Dispatcher_DepositMoney12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DepositMoney::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DepositMoney::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    unsigned int money = 0u;
    if (packet.get_int(money) != 1) {
        return LineFunc(57282, kDepositDispatchSignature, 0, 0);
    }
    if (!user->IsExistAccountCargo()) {
        return 0;
    }

    CAccountCargo* const account_cargo =
        static_cast<CAccountCargo*>(user->GetAccountCargo());
    if (account_cargo->DepositMoney(money)) {
        // The old body constructs this local immediately before recording the
        // account-cargo history event, even though its payload is not passed
        // to that four-int log ABI.
        Inven_Item history_item;
        (void)history_item;
        user->history_log().AccountCargoAddItem(
            0,
            account_cargo->GetMoney(),
            static_cast<int>(money),
            static_cast<eItemAddReason>(45));
    }
    return 0;
}

// IDA_EA: 0x82341D6
// IDA_NAME: _ZN23Dispatcher_DepositMoney4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DepositMoney::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DepositMoney::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82341E0
// IDA_NAME: _ZN23Dispatcher_DepositMoney7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DepositMoney::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DepositMoney::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597180
// IDA_NAME: _ZN24Dispatcher_WithdrawMoneyC2Ev
// DEMANGLED: Dispatcher_WithdrawMoney::Dispatcher_WithdrawMoney(void)
Dispatcher_WithdrawMoney::Dispatcher_WithdrawMoney()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240702
// IDA_NAME: _ZN24Dispatcher_WithdrawMoneyD2Ev
// DEMANGLED: Dispatcher_WithdrawMoney::~Dispatcher_WithdrawMoney()
// IDA_EA: 0x8240732
// IDA_NAME: _ZN24Dispatcher_WithdrawMoneyD0Ev
// DEMANGLED: Dispatcher_WithdrawMoney::~Dispatcher_WithdrawMoney()
Dispatcher_WithdrawMoney::~Dispatcher_WithdrawMoney() {}

// IDA_EA: 0x82250EC
// IDA_NAME: _ZN24Dispatcher_WithdrawMoney12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_WithdrawMoney::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_WithdrawMoney::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    unsigned int money = 0u;
    if (packet.get_int(money) != 1) {
        return LineFunc(57306, kWithdrawDispatchSignature, 0, 0);
    }
    if (!user->IsExistAccountCargo()) {
        return 0;
    }

    CAccountCargo* const account_cargo =
        static_cast<CAccountCargo*>(user->GetAccountCargo());
    if (account_cargo->WithdrawMoney(money)) {
        // Preserve the matching old local-construction sequence before the
        // four-int account-cargo deletion history ABI call.
        Inven_Item history_item;
        (void)history_item;
        user->history_log().AccountCargoDelItem(
            0,
            account_cargo->GetMoney(),
            static_cast<int>(money),
            static_cast<eItemDelReason>(38));
    }
    return 0;
}

// IDA_EA: 0x82341EA
// IDA_NAME: _ZN24Dispatcher_WithdrawMoney4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_WithdrawMoney::read(PacketBuf &,MSG_BASE &)
int Dispatcher_WithdrawMoney::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82341F4
// IDA_NAME: _ZN24Dispatcher_WithdrawMoney7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_WithdrawMoney::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_WithdrawMoney::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597110
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemC2Ev
// DEMANGLED: Dispatcher_BiddingRoutingItem::Dispatcher_BiddingRoutingItem(void)
Dispatcher_BiddingRoutingItem::Dispatcher_BiddingRoutingItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82405CA
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemD2Ev
// DEMANGLED: Dispatcher_BiddingRoutingItem::~Dispatcher_BiddingRoutingItem()
// IDA_EA: 0x82405FA
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItemD0Ev
// DEMANGLED: Dispatcher_BiddingRoutingItem::~Dispatcher_BiddingRoutingItem()
Dispatcher_BiddingRoutingItem::~Dispatcher_BiddingRoutingItem() {}

// IDA_EA: 0x82255CA
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BiddingRoutingItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BiddingRoutingItem::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    const int state = user->get_state();
    if (state != 5 && state != 12) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BIDDING_ROUTING_ITEM, 19);
        return 0;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BIDDING_ROUTING_ITEM, 19);
        return 0;
    }

    char selection = 0;
    if (packet.get_byte(selection) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BIDDING_ROUTING_ITEM, 19);
        return 0;
    }

    CSpecialItemRoutingManager* const routing_manager =
        G_CGameManager()->GetSpecialItemRoutingManager();
    if (routing_manager->ArriveMemberSelection(user, party, selection) == 1) {
        PacketGuard response;
        response.put_header(1,
                            static_cast<int>(ENUM_CMDPACKET_BIDDING_ROUTING_ITEM));
        response.put_byte(1);
        response.finalize(true);
        user->Send(response);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BIDDING_ROUTING_ITEM, 19);
    }
    return 0;
}

// IDA_EA: 0x8234186
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BiddingRoutingItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BiddingRoutingItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234190
// IDA_NAME: _ZN29Dispatcher_BiddingRoutingItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BiddingRoutingItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BiddingRoutingItem::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595DB4
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemC2Ev
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::Dispatcher_BuyPrivateStoreItem(void)
Dispatcher_BuyPrivateStoreItem::Dispatcher_BuyPrivateStoreItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243072
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemD2Ev
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::~Dispatcher_BuyPrivateStoreItem()
// IDA_EA: 0x82430A2
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItemD0Ev
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::~Dispatcher_BuyPrivateStoreItem()
Dispatcher_BuyPrivateStoreItem::~Dispatcher_BuyPrivateStoreItem() {}

// IDA_EA: 0x8202322
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BuyPrivateStoreItem::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(39057, kBuyPrivateStoreItemDispatchSignature, 0, 0);
    }
    if (!user || static_cast<unsigned char>(user->CheckInTrade())) {
        return 0;
    }
    if (static_cast<unsigned char>(
            ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                user, RestrictCategory::AuthMobile, 11))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM, 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(61));
    if (security_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM,
            static_cast<unsigned char>(security_error));
        return 0;
    }
    if (static_cast<unsigned char>(user->isHackUser()) ||
        static_cast<unsigned char>(user->isHackUserTimer())) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM, 17);
        return 0;
    }

    short item_slot = 0;
    int item_count = 0;
    private_store::IBuyer* buyer = 0;
    if (packet.get_short(item_slot) != 1) {
        return LineFunc(39112, kBuyPrivateStoreItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(item_count) != 1) {
        return LineFunc(39113, kBuyPrivateStoreItemDispatchSignature, 0, 0);
    }
    if (item_count < 0) {
        return LineFunc(39114, kBuyPrivateStoreItemDispatchSignature, 0, 0);
    }

    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreBuyer(&buyer, user, static_cast<CUser*>(0));
    buyer->BuyPrivateStoreItem(static_cast<int>(item_slot), item_count);
    return 0;
}

// IDA_EA: 0x8233648
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BuyPrivateStoreItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233652
// IDA_NAME: _ZN30Dispatcher_BuyPrivateStoreItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BuyPrivateStoreItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BuyPrivateStoreItem::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595DD0
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreC2Ev
// DEMANGLED: Dispatcher_EnterPrivateStore::Dispatcher_EnterPrivateStore(void)
Dispatcher_EnterPrivateStore::Dispatcher_EnterPrivateStore()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243024
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreD2Ev
// DEMANGLED: Dispatcher_EnterPrivateStore::~Dispatcher_EnterPrivateStore()
// IDA_EA: 0x8243054
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStoreD0Ev
// DEMANGLED: Dispatcher_EnterPrivateStore::~Dispatcher_EnterPrivateStore()
Dispatcher_EnterPrivateStore::~Dispatcher_EnterPrivateStore() {}

// IDA_EA: 0x82025AE
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStore12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_EnterPrivateStore::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_EnterPrivateStore::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(39124, kEnterPrivateStoreDispatchSignature, 0, 0);
    }

    short seller_index = 0;
    private_store::IBuyer* buyer = 0;
    if (packet.get_short(seller_index) != 1) {
        return LineFunc(39128, kEnterPrivateStoreDispatchSignature, 0, 0);
    }

    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreBuyer(&buyer,
                           user,
                           static_cast<unsigned short>(seller_index));
    buyer->EnterPrivateStore();
    return 0;
}

// IDA_EA: 0x823365C
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_EnterPrivateStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_EnterPrivateStore::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233666
// IDA_NAME: _ZN28Dispatcher_EnterPrivateStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_EnterPrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_EnterPrivateStore::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595DEC
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreC2Ev
// DEMANGLED: Dispatcher_ExitPrivateStore::Dispatcher_ExitPrivateStore(void)
Dispatcher_ExitPrivateStore::Dispatcher_ExitPrivateStore()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242FD6
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreD2Ev
// DEMANGLED: Dispatcher_ExitPrivateStore::~Dispatcher_ExitPrivateStore()
// IDA_EA: 0x8243006
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStoreD0Ev
// DEMANGLED: Dispatcher_ExitPrivateStore::~Dispatcher_ExitPrivateStore()
Dispatcher_ExitPrivateStore::~Dispatcher_ExitPrivateStore() {}

// IDA_EA: 0x820269C
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStore12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ExitPrivateStore::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ExitPrivateStore::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 2 && user->getCurCharacR()) {
        private_store::IBuyer* buyer = 0;
        private_store::CPrivateStoreMgr* const manager =
            private_store::GetInstancePrivateStoreMgr();
        manager->GetStoreBuyer(&buyer, user, static_cast<CUser*>(0));
        buyer->ExitPrivateStore();
    }
    return 0;
}

// IDA_EA: 0x8233670
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ExitPrivateStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ExitPrivateStore::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823367A
// IDA_NAME: _ZN27Dispatcher_ExitPrivateStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ExitPrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ExitPrivateStore::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595E08
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreC2Ev
// DEMANGLED: Dispatcher_CreatePrivateStore::Dispatcher_CreatePrivateStore(void)
Dispatcher_CreatePrivateStore::Dispatcher_CreatePrivateStore()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242F88
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreD2Ev
// DEMANGLED: Dispatcher_CreatePrivateStore::~Dispatcher_CreatePrivateStore()
// IDA_EA: 0x8242FB8
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStoreD0Ev
// DEMANGLED: Dispatcher_CreatePrivateStore::~Dispatcher_CreatePrivateStore()
Dispatcher_CreatePrivateStore::~Dispatcher_CreatePrivateStore() {}

// IDA_EA: 0x820271C
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStore12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CreatePrivateStore::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CreatePrivateStore::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(39153, kCreatePrivateStoreDispatchSignature, 0, 0);
    }

    GameWorld* const world = G_GameWorld();
    if (world->IsIntegratedPvPBaseChannel() || world->GetChannelType() == 7) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_PRIVATE_STORE, 62);
        return 0;
    }

    Village* const village = G_GameWorld()->getUserVillage(user);
    if (!village) {
        return LineFunc(39173, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (user->get_area(false) == village->get_gate_area()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_PRIVATE_STORE, 62);
        return 0;
    }
    if (static_cast<unsigned char>(
            ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
                user, RestrictCategory::AuthMobile, 11))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_PRIVATE_STORE, 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(60));
    if (security_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CREATE_PRIVATE_STORE,
            static_cast<unsigned char>(security_error));
        return 0;
    }

    unsigned char requested_village = 0;
    unsigned char requested_area = 0;
    short position_x = 0;
    short position_y = 0;
    unsigned short doll_slot = 0;
    private_store::ISeller* seller = 0;
    if (packet.get_byte(requested_village) != 1) {
        return LineFunc(39236, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (packet.get_byte(requested_area) != 1) {
        return LineFunc(39237, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (packet.get_short(position_x) != 1) {
        return LineFunc(39238, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (packet.get_short(position_y) != 1) {
        return LineFunc(39239, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (packet.get_short(doll_slot) != 1) {
        return LineFunc(39243, kCreatePrivateStoreDispatchSignature, 0, 0);
    }
    if (user->getCurCharacVill() != static_cast<int>(requested_village) ||
        user->get_area(false) != static_cast<int>(requested_area)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_PRIVATE_STORE, 62);
        return 0;
    }

    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreSeller(&seller, user);
    private_store::CPrivateStore* const store =
        static_cast<private_store::CPrivateStore*>(seller);
    if (seller->CreatePrivateStore(requested_village,
                                   requested_area,
                                   position_x,
                                   position_y,
                                   doll_slot) != 1) {
        if (static_cast<unsigned char>(store->GetState())) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x32D),
                1,
                0,
                0);
        }
        if (static_cast<unsigned char>(store->GetState())) {
            return 0;
        }
        private_store::GetInstancePrivateStoreMgr()->FreeStoreSeller(user);
    }
    return 0;
}

// IDA_EA: 0x8233684
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CreatePrivateStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CreatePrivateStore::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823368E
// IDA_NAME: _ZN29Dispatcher_CreatePrivateStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CreatePrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CreatePrivateStore::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595E24
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreC2Ev
// DEMANGLED: Dispatcher_RemovePrivateStore::Dispatcher_RemovePrivateStore(void)
Dispatcher_RemovePrivateStore::Dispatcher_RemovePrivateStore()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242F3A
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreD2Ev
// DEMANGLED: Dispatcher_RemovePrivateStore::~Dispatcher_RemovePrivateStore()
// IDA_EA: 0x8242F6A
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStoreD0Ev
// DEMANGLED: Dispatcher_RemovePrivateStore::~Dispatcher_RemovePrivateStore()
Dispatcher_RemovePrivateStore::~Dispatcher_RemovePrivateStore() {}

// IDA_EA: 0x8202BB6
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStore12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RemovePrivateStore::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RemovePrivateStore::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        PacketGuard response;
        response.put_header(
            1, static_cast<int>(ENUM_CMDPACKET_REMOVE_PRIVATE_STORE));
        response.put_byte(0);
        response.put_byte(21);
        response.finalize(true);
        user->Send(response);
    } else {
        private_store::ISeller* seller = 0;
        private_store::CPrivateStoreMgr* const manager =
            private_store::GetInstancePrivateStoreMgr();
        manager->GetStoreSeller(&seller, user);
        seller->RemovePrivateStore(false);
    }
    return 0;
}

// IDA_EA: 0x8233698
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStore4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RemovePrivateStore::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RemovePrivateStore::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82336A2
// IDA_NAME: _ZN29Dispatcher_RemovePrivateStore7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RemovePrivateStore::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RemovePrivateStore::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595E40
// IDA_NAME: _ZN26Dispatcher_CompleteDisplayC2Ev
// DEMANGLED: Dispatcher_CompleteDisplay::Dispatcher_CompleteDisplay(void)
Dispatcher_CompleteDisplay::Dispatcher_CompleteDisplay()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242EEC
// IDA_NAME: _ZN26Dispatcher_CompleteDisplayD2Ev
// DEMANGLED: Dispatcher_CompleteDisplay::~Dispatcher_CompleteDisplay()
// IDA_EA: 0x8242F1C
// IDA_NAME: _ZN26Dispatcher_CompleteDisplayD0Ev
// DEMANGLED: Dispatcher_CompleteDisplay::~Dispatcher_CompleteDisplay()
Dispatcher_CompleteDisplay::~Dispatcher_CompleteDisplay() {}

// IDA_EA: 0x8202CDA
// IDA_NAME: _ZN26Dispatcher_CompleteDisplay12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompleteDisplay::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompleteDisplay::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(39300, kCompleteDisplayDispatchSignature, 0, 0);
    }

    char display_name[41] = {};
    private_store::PrivateStoreItem item;
    int display_length = 0;
    if (packet.get_int(display_length) != 1) {
        return LineFunc(39310, kCompleteDisplayDispatchSignature, 0, 0);
    }
    if (display_length > 40) {
        return LineFunc(39312, kCompleteDisplayDispatchSignature, 0, 0);
    }
    if (packet.get_str(display_name, 41, display_length) != 1) {
        return LineFunc(39315, kCompleteDisplayDispatchSignature, 0, 0);
    }

    const std::string filtered_display_name(display_name);
    if (CSyncSlangFilter::GetInstance()->HasSlang(filtered_display_name) ||
        G_CDataManager()->hasSpecialCharacter(display_name)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPLETE_DISPLAY, 159);
        return 0;
    }

    private_store::ISeller* seller = 0;
    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreSeller(&seller, user);

    unsigned char item_count = 0;
    if (packet.get_byte(item_count) != 1) {
        return LineFunc(39332, kCompleteDisplayDispatchSignature, 0, 0);
    }

    private_store::CPrivateStore* const store =
        static_cast<private_store::CPrivateStore*>(seller);
    const int trace_state = store->GetState();
    (void)trace_state;
    const unsigned char printed_state =
        static_cast<unsigned char>(store->GetState());
    printf("(private_store::CPrivateStore*)seller)->GetState()=%d",
           static_cast<int>(printed_state));
    if (static_cast<unsigned char>(store->GetState()) != 1) {
        return 0;
    }
    if (!item_count) {
        return LineFunc(39341, kCompleteDisplayDispatchSignature, 0, 0);
    }

    for (int index = 0; index < item_count; ++index) {
        short item_index = 0;
        int item_price = 0;
        char item_space = 0;
        short item_slot = 0;
        int item_count_value = 0;
        if (packet.get_short(item_index) != 1 ||
            packet.get_int(item_price) != 1 ||
            packet.get_byte(item_space) != 1 ||
            packet.get_short(item_slot) != 1 ||
            packet.get_int(item_count_value) != 1) {
            return LineFunc(39357, kCompleteDisplayDispatchSignature, 0, 0);
        }
        if (item_space != 1 && item_count_value <= 0) {
            return LineFunc(39360, kCompleteDisplayDispatchSignature, 0, 0);
        }

        item.store_slot = static_cast<uint16_t>(item_index);
        item.price = item_price;
        item.inventory_space = static_cast<uint8_t>(item_space);
        item.inventory_slot = static_cast<uint16_t>(item_slot);
        item.item.set_add_info(item_count_value);

        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(static_cast<int>(item_space)));
        if (user->CheckItemLock(inventory_type, static_cast<int>(item_slot))) {
            store->ResetItems();
            user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPLETE_DISPLAY, 213);
            return 0;
        }
        if (seller->AddItem(item, index) != 1) {
            store->ResetItems();
            user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPLETE_DISPLAY, 17);
            return 0;
        }
    }

    unsigned short doll_slot = 0;
    if (packet.get_short(doll_slot) != 1) {
        return LineFunc(39401, kCompleteDisplayDispatchSignature, 0, 0);
    }
    const int doll_error = store->isDollCreated()
        ? store->CheckPrivateStoreDoll(doll_slot)
        : 0;
    if (doll_error) {
        store->ResetItems();
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_COMPLETE_DISPLAY,
            static_cast<unsigned char>(doll_error));
        return 0;
    }
    if (seller->CompleteDisplay(display_name, doll_slot) != 1) {
        store->ResetItems();
    }
    return 0;
}

// IDA_EA: 0x82336AC
// IDA_NAME: _ZN26Dispatcher_CompleteDisplay4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompleteDisplay::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompleteDisplay::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82336B6
// IDA_NAME: _ZN26Dispatcher_CompleteDisplay7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompleteDisplay::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompleteDisplay::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85977A6
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemC2Ev
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::Dispatcher_VerifyPrivateStoreItem(void)
Dispatcher_VerifyPrivateStoreItem::Dispatcher_VerifyPrivateStoreItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824004E
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemD2Ev
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::~Dispatcher_VerifyPrivateStoreItem()
// IDA_EA: 0x824007E
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItemD0Ev
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::~Dispatcher_VerifyPrivateStoreItem()
Dispatcher_VerifyPrivateStoreItem::~Dispatcher_VerifyPrivateStoreItem() {}

// IDA_EA: 0x8229768
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_VerifyPrivateStoreItem::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return LineFunc(59200,
                        kVerifyPrivateStoreItemDispatchSignature,
                        0,
                        0);
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(61));
    if (security_error) {
        // The native body deliberately reports the CMD85 packet id here.
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM,
            static_cast<unsigned char>(security_error));
        return 0;
    }
    if (static_cast<unsigned char>(user->isHackUser()) ||
        static_cast<unsigned char>(user->isHackUserTimer())) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_VERIFY_PRIVATE_STORE_ITEM, 17);
        return 0;
    }

    short item_slot = 0;
    int item_count = 0;
    private_store::IBuyer* buyer = 0;
    if (packet.get_short(item_slot) != 1) {
        return LineFunc(59242,
                        kVerifyPrivateStoreItemDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_int(item_count) != 1) {
        return LineFunc(59243,
                        kVerifyPrivateStoreItemDispatchSignature,
                        0,
                        0);
    }
    if (item_count < 0) {
        return LineFunc(59244,
                        kVerifyPrivateStoreItemDispatchSignature,
                        0,
                        0);
    }

    private_store::CPrivateStoreMgr* const manager =
        private_store::GetInstancePrivateStoreMgr();
    manager->GetStoreBuyer(&buyer, user, static_cast<CUser*>(0));
    buyer->VerifyPrivateStoreItem(static_cast<int>(item_slot), item_count);
    return 0;
}

// IDA_EA: 0x823433E
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_VerifyPrivateStoreItem::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234348
// IDA_NAME: _ZN33Dispatcher_VerifyPrivateStoreItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_VerifyPrivateStoreItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_VerifyPrivateStoreItem::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85969D8
// IDA_NAME: _ZN24Dispatcher_UseCraneStartC2Ev
// DEMANGLED: Dispatcher_UseCraneStart::Dispatcher_UseCraneStart(void)
Dispatcher_UseCraneStart::Dispatcher_UseCraneStart()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82415A2
// IDA_NAME: _ZN24Dispatcher_UseCraneStartD2Ev
// DEMANGLED: Dispatcher_UseCraneStart::~Dispatcher_UseCraneStart()
// IDA_EA: 0x82415D2
// IDA_NAME: _ZN24Dispatcher_UseCraneStartD0Ev
// DEMANGLED: Dispatcher_UseCraneStart::~Dispatcher_UseCraneStart()
Dispatcher_UseCraneStart::~Dispatcher_UseCraneStart() {}

// IDA_EA: 0x821D2C4
// IDA_NAME: _ZN24Dispatcher_UseCraneStart12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UseCraneStart::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UseCraneStart::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    short material_slot = -1;
    if (packet.get_short(material_slot) != 1) {
        return LineFunc(53480, kUseCraneStartDispatchSignature, 0, 0);
    }

    CraneMinigameManager* const manager =
        G_CGameManager()->GetCraneMinigameManager();
    const int start_error = manager->startUseMaterial(
        user, static_cast<int>(material_slot));
    if (start_error) {
        throw static_cast<ENUM_ERROR>(start_error);
    }

    const Inven_Item material = user->getCurCharacInvenR()->GetInvenSlot(
        1, static_cast<int>(material_slot));
    user->setUseCraneStart(true);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CRANE_START_USE);
    response.put_byte(1);
    response.put_short(material_slot);
    response.put_int(material.get_add_info());
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233DC6
// IDA_NAME: _ZN24Dispatcher_UseCraneStart4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseCraneStart::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseCraneStart::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233DD0
// IDA_NAME: _ZN24Dispatcher_UseCraneStart7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseCraneStart::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseCraneStart::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85969F4
// IDA_NAME: _ZN22Dispatcher_CranePickUpC2Ev
// DEMANGLED: Dispatcher_CranePickUp::Dispatcher_CranePickUp(void)
Dispatcher_CranePickUp::Dispatcher_CranePickUp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241554
// IDA_NAME: _ZN22Dispatcher_CranePickUpD2Ev
// DEMANGLED: Dispatcher_CranePickUp::~Dispatcher_CranePickUp()
// IDA_EA: 0x8241584
// IDA_NAME: _ZN22Dispatcher_CranePickUpD0Ev
// DEMANGLED: Dispatcher_CranePickUp::~Dispatcher_CranePickUp()
Dispatcher_CranePickUp::~Dispatcher_CranePickUp() {}

// IDA_EA: 0x821D508
// IDA_NAME: _ZN22Dispatcher_CranePickUp12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CranePickUp::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CranePickUp::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CRANE_PICKUP, 19);
        return 0;
    }
    if (static_cast<unsigned char>(user->isUseCraneStart()) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CRANE_PICKUP, 17);
        return 0;
    }

    // The native handler clears this guard before either packet parser runs.
    user->setUseCraneStart(false);
    short item_index = -1;
    int selected_item_id = -1;
    if (packet.get_short(item_index) != 1) {
        return LineFunc(53538, kCranePickUpDispatchSignature, 0, 0);
    }
    if (packet.get_int(selected_item_id) != 1) {
        return LineFunc(53539, kCranePickUpDispatchSignature, 0, 0);
    }

    int result = 0;
    bool pick_succeeded = false;
    if (G_CGameManager()->GetCraneMinigameManager()->isPickSuccess(
            item_index, selected_item_id)) {
        // The old body reacquires the manager for this stateful delivery call.
        result = G_CGameManager()->GetCraneMinigameManager()->sendItem(
            user, static_cast<int>(item_index));
        pick_succeeded = result == 0;
    } else {
        result = 190;
    }

    if (!pick_succeeded) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CRANE_PICKUP,
            static_cast<unsigned char>(result));
        return 0;
    }

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CRANE_PICKUP);
    response.put_byte(1);
    response.put_int(selected_item_id);
    response.put_short(
        G_CGameManager()->GetCraneMinigameManager()->getItemCnt(
            static_cast<int>(item_index)));
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233DDA
// IDA_NAME: _ZN22Dispatcher_CranePickUp4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CranePickUp::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CranePickUp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233DE4
// IDA_NAME: _ZN22Dispatcher_CranePickUp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CranePickUp::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CranePickUp::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596754
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoC2Ev
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::Dispatcher_AuctionMyRegistedItemInfo(void)
Dispatcher_AuctionMyRegistedItemInfo::Dispatcher_AuctionMyRegistedItemInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241B6C
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoD2Ev
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::~Dispatcher_AuctionMyRegistedItemInfo()
// IDA_EA: 0x8241B9C
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfoD0Ev
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::~Dispatcher_AuctionMyRegistedItemInfo()
Dispatcher_AuctionMyRegistedItemInfo::~Dispatcher_AuctionMyRegistedItemInfo() {}

// IDA_EA: 0x8216966
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionMyRegistedItemInfo::dispatch_sig(CUser* user,
                                                        PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(51086,
                        kAuctionMyRegistedItemInfoDispatchSignature,
                        0,
                        0);
    }
    if (payment_type > 1) {
        return LineFunc(51089,
                        kAuctionMyRegistedItemInfoDispatchSignature,
                        0,
                        0);
    }
    if (IsAuctionServerDisconnected(payment_type)) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_AUCTION_MY_REGISTED_ITEM_INFO,
            140,
            static_cast<PAY_TYPE>(payment_type));
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(51126,
                        kAuctionMyRegistedItemInfoDispatchSignature,
                        0,
                        0);
    }

    if (payment_type == 1) {
        PCK_AUCTION_MY_REGISTED_ITEM_INFO_GP request;
        request.user_uid = user->GetUID();
        request.charac_no = user->getCurCharacNo();
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        return 0;
    }

    PCK_AUCTION_MY_REGISTED_ITEM_INFO_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The old trailing GM-list branch only emits a trace; it has no protocol
    // or auction state effect and its untyped GM-list owner remains separate.
    return 0;
}

// IDA_EA: 0x8233C72
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionMyRegistedItemInfo::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C7C
// IDA_NAME: _ZN36Dispatcher_AuctionMyRegistedItemInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionMyRegistedItemInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionMyRegistedItemInfo::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596770
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoC2Ev
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::Dispatcher_AuctionMyBiddingInfo(void)
Dispatcher_AuctionMyBiddingInfo::Dispatcher_AuctionMyBiddingInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241B1E
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoD2Ev
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::~Dispatcher_AuctionMyBiddingInfo()
// IDA_EA: 0x8241B4E
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfoD0Ev
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::~Dispatcher_AuctionMyBiddingInfo()
Dispatcher_AuctionMyBiddingInfo::~Dispatcher_AuctionMyBiddingInfo() {}

// IDA_EA: 0x8216B9A
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionMyBiddingInfo::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    // 140828 sub_10E9F40 writes one zero byte and immediately finalizes.
    uint8_t native_reserved_zero = 0xffu;
    if (packet.get_byte(native_reserved_zero) != 1 ||
        native_reserved_zero != 0u) {
        return LineFunc(51253,
                        kAuctionMyBiddingInfoDispatchSignature,
                        0,
                        0);
    }
    if (GlobalData::s_auction_proxy->IsRunning() != 1) {
        // The 140828 error-140 handler consumes the payment mode after the
        // standard failure/error bytes even though this request only supports
        // the normal auction proxy.
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_AUCTION_MY_BIDDING_INFO,
            140,
            static_cast<PAY_TYPE>(0));
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(51254,
                        kAuctionMyBiddingInfoDispatchSignature,
                        0,
                        0);
    }

    PCK_AUCTION_MY_BIDDING_INFO_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The old trailing GM-list branch only emits a trace; it has no protocol
    // or auction state effect and its untyped GM-list owner remains separate.
    return 0;
}

// IDA_EA: 0x8233C86
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionMyBiddingInfo::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C90
// IDA_NAME: _ZN31Dispatcher_AuctionMyBiddingInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionMyBiddingInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionMyBiddingInfo::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859678C
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryC2Ev
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::Dispatcher_AuctionMyAuctionHistory(void)
Dispatcher_AuctionMyAuctionHistory::Dispatcher_AuctionMyAuctionHistory()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241AD0
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryD2Ev
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::~Dispatcher_AuctionMyAuctionHistory()
// IDA_EA: 0x8241B00
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistoryD0Ev
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::~Dispatcher_AuctionMyAuctionHistory()
Dispatcher_AuctionMyAuctionHistory::~Dispatcher_AuctionMyAuctionHistory() {}

// IDA_EA: 0x8216D02
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionMyAuctionHistory::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    (void)packet;
    if (GlobalData::s_auction_proxy->IsRunning() != 1) {
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(51300,
                        kAuctionMyAuctionHistoryDispatchSignature,
                        0,
                        0);
    }

    PCK_AUCTION_MY_AUCTION_HISTORY_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    request.account_id = user->get_acc_id();
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    return 0;
}

// IDA_EA: 0x8233C9A
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionMyAuctionHistory::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233CA4
// IDA_NAME: _ZN34Dispatcher_AuctionMyAuctionHistory7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionMyAuctionHistory::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionMyAuctionHistory::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85966C8
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItemC2Ev
// DEMANGLED: Dispatcher_AuctionRegistItem::Dispatcher_AuctionRegistItem(void)
Dispatcher_AuctionRegistItem::Dispatcher_AuctionRegistItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241D40
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItemD2Ev
// DEMANGLED: Dispatcher_AuctionRegistItem::~Dispatcher_AuctionRegistItem()
// IDA_EA: 0x8241D70
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItemD0Ev
// DEMANGLED: Dispatcher_AuctionRegistItem::~Dispatcher_AuctionRegistItem()
Dispatcher_AuctionRegistItem::~Dispatcher_AuctionRegistItem() {}

// IDA_EA: 0x8213E8A
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionRegistItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionRegistItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(49812, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (payment_type > 1) {
        return LineFunc(49815, kAuctionRegistItemDispatchSignature, 0, 0);
    }

    const PAY_TYPE payment = static_cast<PAY_TYPE>(payment_type);
    if (payment_type == 1 && user->get_charac_level() <= 30) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 117,
                                 payment);
        return 0;
    }

    const int precheck =
        user->AuctionPreCheck(ENUM_CMDPACKET_AUCTION_REGIST_ITEM, payment);
    if (precheck >= 0) {
        return precheck;
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 5)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 209,
                                 payment);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(50));
    if (security_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 static_cast<unsigned char>(security_error),
                                 payment);
        return 0;
    }

    if (payment_type == 1) {
        PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GP request;
        request.user_uid = user->GetUID();
        request.charac_no = user->getCurCharacNo();
        if (packet.get_byte(request.item_space) != 1) {
            return LineFunc(49914, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (packet.get_short(request.item_slot) != 1) {
            return LineFunc(49916, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (packet.get_int(request.field_1d) != 1) {
            return LineFunc(49922, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (packet.get_int(request.field_21) != 1) {
            return LineFunc(49925, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (packet.get_int(request.field_25) != 1) {
            return LineFunc(49926, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (packet.get_int(request.field_29) != 1) {
            return LineFunc(49927, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (IsPriceError(request.field_25, request.field_29) ||
            request.field_29 > 9000000) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     152,
                                     payment);
            return 0;
        }
        if (request.field_1d <= 0x28D287u ||
            request.field_1d > 0x28D299u || request.field_21 > 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment);
            return 0;
        }
        if (request.field_25 != -1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     145,
                                     payment);
            return LineFunc(49954, kAuctionRegistItemDispatchSignature, 0, 0);
        }
        if (request.field_29 <= 49 || request.field_29 > 9000000) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                     152,
                                     payment);
            return 0;
        }

        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        // The native trailing GM-list branch is trace-only and has no packet
        // or auction-state effect; its owner remains separate L1 debt.
        return 0;
    }

    PCK_AUCTION_ASK_REGISTED_ITEM_NUM_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    if (packet.get_byte(request.item_space) != 1) {
        return LineFunc(50058, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.item_slot) != 1) {
        return LineFunc(50060, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.item_index) != 1) {
        return LineFunc(50066, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.field_21) != 1) {
        return LineFunc(50069, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.field_25) != 1) {
        return LineFunc(50070, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.field_29) != 1) {
        return LineFunc(50071, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.field_2d) != 1) {
        return LineFunc(50072, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.category.category_0) != 1) {
        return LineFunc(50082, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_byte(request.category.flag_08) != 1) {
        return LineFunc(50085, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.category.category_1) != 1) {
        return LineFunc(50082, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_byte(request.category.flag_09) != 1) {
        return LineFunc(50085, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.category.category_2) != 1) {
        return LineFunc(50082, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (packet.get_byte(request.category.flag_0a) != 1) {
        return LineFunc(50085, kAuctionRegistItemDispatchSignature, 0, 0);
    }

    CItem* const initial_item = G_CDataManager()->find_item(
        static_cast<int>(request.item_index));
    if (!initial_item) {
        return LineFunc(50090, kAuctionRegistItemDispatchSignature, 0, 0);
    }
    if (initial_item->vtable_->is_random_option(initial_item)) {
        if (!request.category.category_0) {
            request.category.category_0 = 30000;
        }
        if (!request.category.category_1) {
            request.category.category_1 = 30000;
        }
        if (!request.category.category_2) {
            request.category.category_2 = 30000;
        }
    }
    request.category._sort();

    const ServerParameterScript* const server_parameters =
        G_CDataManager()->get_server_parameter_script();
    const AuctionRegisterItemLimitVector& limits =
        server_parameters->get_auction_register_item_limits();
    const int current_sec = GlobalData::system_time()->getCurSec();
    for (AuctionRegisterItemLimitVector::const_iterator it = limits.begin();
         it != limits.end();
         ++it) {
        const AuctionRegisterItemLimit& limit = *it;
        if ((limit.item_index == -1 || static_cast<uint32_t>(limit.item_index) ==
                                      request.item_index) &&
            limit.start_sec < current_sec && limit.end_sec > current_sec) {
            PacketGuard response;
            response.put_header(1,
                                static_cast<int>(
                                    ENUM_CMDPACKET_AUCTION_REGIST_ITEM));
            response.put_byte(0);
            response.put_byte(7);
            response.put_int(limit.item_index);
            response.put_int(limit.start_sec);
            response.put_int(limit.end_sec);
            response.finalize(true);
            static_cast<void>(user->Send(response));
            return 0;
        }
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(static_cast<int>(request.item_space)));
    if (user->CheckItemLock(inventory_type, request.item_slot)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 213,
                                 static_cast<PAY_TYPE>(0));
        return 0;
    }
    if (IsPriceError(request.field_25, request.field_29)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 152,
                                 static_cast<PAY_TYPE>(0));
        return 0;
    }

    CItem* const validation_item = G_CDataManager()->find_item(
        static_cast<int>(request.item_index));
    if (!validation_item) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 145,
                                 payment);
        return 0;
    }
    if (validation_item->is_stackable() &&
        (!IsValidMultiplyResult(request.field_25, request.field_21) ||
         !IsValidMultiplyResult(request.field_29, request.field_21))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
                                 152,
                                 payment);
        return 0;
    }

    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The native trailing GM-list branch is trace-only and has no packet or
    // auction-state effect; its owner remains separate L1 debt.
    return 0;
}

// IDA_EA: 0x8233C0E
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionRegistItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionRegistItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C18
// IDA_NAME: _ZN28Dispatcher_AuctionRegistItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionRegistItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionRegistItem::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85966E4
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelC2Ev
// DEMANGLED: Dispatcher_AuctionRegistCancel::Dispatcher_AuctionRegistCancel(void)
Dispatcher_AuctionRegistCancel::Dispatcher_AuctionRegistCancel()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241CF2
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelD2Ev
// DEMANGLED: Dispatcher_AuctionRegistCancel::~Dispatcher_AuctionRegistCancel()
// IDA_EA: 0x8241D22
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancelD0Ev
// DEMANGLED: Dispatcher_AuctionRegistCancel::~Dispatcher_AuctionRegistCancel()
Dispatcher_AuctionRegistCancel::~Dispatcher_AuctionRegistCancel() {}

// IDA_EA: 0x8214B96
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionRegistCancel::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionRegistCancel::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(50303,
                        kAuctionRegistCancelDispatchSignature,
                        0,
                        0);
    }
    if (payment_type > 1) {
        return LineFunc(50306,
                        kAuctionRegistCancelDispatchSignature,
                        0,
                        0);
    }

    const int precheck = user->AuctionPreCheck(
        ENUM_CMDPACKET_AUCTION_REGIST_CANCEL,
        static_cast<PAY_TYPE>(payment_type));
    if (precheck >= 0) {
        return precheck;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(51));
    if (security_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_REGIST_CANCEL,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }

    if (payment_type == 1) {
        PCK_AUCTION_REGIST_CANCEL_GP request;
        request.user_uid = user->GetUID();
        request.charac_no = user->getCurCharacNo();
        if (packet.get_binary(request.registration_key.bytes,
                              sizeof(request.registration_key.bytes)) != 1) {
            return LineFunc(50353,
                            kAuctionRegistCancelDispatchSignature,
                            0,
                            0);
        }
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        return 0;
    }

    PCK_AUCTION_REGIST_CANCEL_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    if (packet.get_binary(request.registration_key.bytes,
                          sizeof(request.registration_key.bytes)) != 1) {
        return LineFunc(50372,
                        kAuctionRegistCancelDispatchSignature,
                        0,
                        0);
    }
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The old trailing GM-list branch only emits a trace; it has no protocol
    // or auction state effect and its untyped GM-list owner remains separate.
    return 0;
}

// IDA_EA: 0x8233C22
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionRegistCancel::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionRegistCancel::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C2C
// IDA_NAME: _ZN30Dispatcher_AuctionRegistCancel7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionRegistCancel::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionRegistCancel::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596700
// IDA_NAME: _ZN25Dispatcher_AuctionBiddingC2Ev
// DEMANGLED: Dispatcher_AuctionBidding::Dispatcher_AuctionBidding(void)
Dispatcher_AuctionBidding::Dispatcher_AuctionBidding()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241C56
// IDA_NAME: _ZN25Dispatcher_AuctionBiddingD2Ev
// DEMANGLED: Dispatcher_AuctionBidding::~Dispatcher_AuctionBidding()
// IDA_EA: 0x8241C86
// IDA_NAME: _ZN25Dispatcher_AuctionBiddingD0Ev
// DEMANGLED: Dispatcher_AuctionBidding::~Dispatcher_AuctionBidding()
Dispatcher_AuctionBidding::~Dispatcher_AuctionBidding() {}

// IDA_EA: 0x821522E
// IDA_NAME: _ZN25Dispatcher_AuctionBidding12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionBidding::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionBidding::dispatch_sig(CUser* user, PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(50516, kAuctionBiddingDispatchSignature, 0, 0);
    }
    if (payment_type > 1) {
        return LineFunc(50519, kAuctionBiddingDispatchSignature, 0, 0);
    }

    const PAY_TYPE payment = static_cast<PAY_TYPE>(payment_type);
    if (payment_type == 1 && user->get_charac_level() <= 19) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 116, payment);
        return 0;
    }

    const int precheck =
        user->AuctionPreCheck(ENUM_CMDPACKET_AUCTION_BIDDING, payment);
    if (precheck >= 0) {
        return precheck;
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 5)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(52));
    if (security_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING,
                                 static_cast<unsigned char>(security_error),
                                 payment);
        return 0;
    }

    if (payment_type == 1) {
        uint32_t bid_cera = 0;
        if (packet.get_int(bid_cera) != 1) {
            return LineFunc(50601, kAuctionBiddingDispatchSignature, 0, 0);
        }
        if (static_cast<uint32_t>(user->GetCera()) < bid_cera) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING,
                                     144,
                                     payment);
            return 0;
        }

        const int charac_no = user->getCurCharacNo();
        CPrePayInfoMgr* const prepay_info_mgr = G_CPrePayInfoMgr();
        if (prepay_info_mgr->FindPrePayInfo(charac_no) != 0 ||
            static_cast<unsigned char>(user->GetBuyingGold()) != 0) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING,
                                     158,
                                     payment);
            return 0;
        }

        AuctionBiddingKey key;
        if (packet.get_binary(key.bytes, sizeof(key.bytes)) != 1) {
            return LineFunc(50619, kAuctionBiddingDispatchSignature, 0, 0);
        }
        int field_33 = 0;
        if (packet.get_int(field_33) != 1) {
            return LineFunc(50625, kAuctionBiddingDispatchSignature, 0, 0);
        }

        PCK_AUCTION_ASK_OWNER_IS_VIP_GP request;
        request.user_uid = user->GetUID();
        request.key = key;
        request.charac_no = charac_no;
        std::strncpy(request.charac_name,
                     user->getCurCharacName(),
                     sizeof(request.charac_name));
        request.bid_cera = bid_cera;
        request.field_33 = field_33;
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        user->SetBuyingGold(true);
        return 0;
    }

    PCK_AUCTION_BIDDING_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    std::strncpy(request.charac_name,
                 user->getCurCharacName(),
                 sizeof(request.charac_name));
    if (packet.get_int(request.bid_money) != 1) {
        return LineFunc(50754, kAuctionBiddingDispatchSignature, 0, 0);
    }
    if (packet.get_binary(request.key.bytes, sizeof(request.key.bytes)) != 1) {
        return LineFunc(50755, kAuctionBiddingDispatchSignature, 0, 0);
    }

    char source_charac_name[13] = {0};
    if (packet.get_binary(source_charac_name, sizeof(source_charac_name)) !=
        1) {
        return LineFunc(50760, kAuctionBiddingDispatchSignature, 0, 0);
    }
    if (user->getCurCharacMoney() < request.bid_money) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 144, payment);
        return 0;
    }
    if (user->getCurCharacInvenW()->use_money(
            request.bid_money, static_cast<eMoneySubReason>(26), true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 147, payment);
        return 0;
    }

    user->SaveMoneyCoin();
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    if (request.bid_money > 4999) {
        CStreamGuard db_guard(
            GlobalData::s_stream_pool->Acquire("PacketDispatcher_Impl_1.cpp",
                                               50808),
            true);
        *db_guard << 777;
        *db_guard << user->GetUID();
        SIG_AUCTION_MONEY_SUB* const db_request =
            static_cast<SIG_AUCTION_MONEY_SUB*>(db_guard.GetInBufferUntyped(
                sizeof(SIG_AUCTION_MONEY_SUB)));
        std::memset(db_request, 0, sizeof(*db_request));
        db_request->money = static_cast<uint32_t>(request.bid_money);
        std::strncpy(db_request->charac_name,
                     source_charac_name,
                     sizeof(db_request->charac_name));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, db_guard);
    }

    // The native trailing GM-list loop only emits a trace, with no auction or
    // protocol state change.  Its untyped trace owner stays explicit L1 debt.
    return 0;
}

// IDA_EA: 0x8233C36
// IDA_NAME: _ZN25Dispatcher_AuctionBidding4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionBidding::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionBidding::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C40
// IDA_NAME: _ZN25Dispatcher_AuctionBidding7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionBidding::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionBidding::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8597346
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceC2Ev
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::Dispatcher_AuctionBuyItemApiece(void)
Dispatcher_AuctionBuyItemApiece::Dispatcher_AuctionBuyItemApiece()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241CA4
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceD2Ev
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::~Dispatcher_AuctionBuyItemApiece()
// IDA_EA: 0x8241CD4
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApieceD0Ev
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::~Dispatcher_AuctionBuyItemApiece()
Dispatcher_AuctionBuyItemApiece::~Dispatcher_AuctionBuyItemApiece() {}

// IDA_EA: 0x8214E44
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionBuyItemApiece::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    const int precheck = user->AuctionPreCheck(
        ENUM_CMDPACKET_AUCTION_BUY_ITEM_APIECE, static_cast<PAY_TYPE>(0));
    if (precheck >= 0) {
        return precheck;
    }

    PCK_AUCTION_BUY_ITEM_APIECE_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    if (packet.get_int(request.purchase_money) != 1) {
        return LineFunc(50433, kAuctionBuyItemApieceDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.field_26) != 1) {
        return LineFunc(50434, kAuctionBuyItemApieceDispatchSignature, 0, 0);
    }
    if (packet.get_binary(request.key.bytes, sizeof(request.key.bytes)) != 1) {
        return LineFunc(50435, kAuctionBuyItemApieceDispatchSignature, 0, 0);
    }

    char source_charac_name[13] = {0};
    if (packet.get_binary(source_charac_name, sizeof(source_charac_name)) !=
        1) {
        return LineFunc(50440, kAuctionBuyItemApieceDispatchSignature, 0, 0);
    }
    if (user->getCurCharacMoney() < request.purchase_money) {
        // The old CMD335 body intentionally reports CMD188 on this branch.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 144);
        return 0;
    }
    if (user->getCurCharacInvenW()->use_money(
            request.purchase_money, static_cast<eMoneySubReason>(26), true) !=
        1) {
        // The old CMD335 body intentionally reports CMD188 on this branch.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_BIDDING, 147);
        return 0;
    }

    user->SaveMoneyCoin();
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    if (request.purchase_money > 4999) {
        CStreamGuard db_guard(
            GlobalData::s_stream_pool->Acquire("PacketDispatcher_Impl_1.cpp",
                                               50473),
            true);
        *db_guard << 777;
        *db_guard << user->GetUID();
        SIG_AUCTION_MONEY_SUB* const db_request =
            static_cast<SIG_AUCTION_MONEY_SUB*>(db_guard.GetInBufferUntyped(
                sizeof(SIG_AUCTION_MONEY_SUB)));
        std::memset(db_request, 0, sizeof(*db_request));
        db_request->money = static_cast<uint32_t>(request.purchase_money);
        std::strncpy(db_request->charac_name,
                     source_charac_name,
                     sizeof(db_request->charac_name));
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, db_guard);
    }
    return 0;
}

// IDA_EA: 0x8233BE6
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionBuyItemApiece::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233BF0
// IDA_NAME: _ZN31Dispatcher_AuctionBuyItemApiece7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionBuyItemApiece::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionBuyItemApiece::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85966AC
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceC2Ev
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::Dispatcher_AuctionAskAveragePrice(void)
Dispatcher_AuctionAskAveragePrice::Dispatcher_AuctionAskAveragePrice()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241D8E
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceD2Ev
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::~Dispatcher_AuctionAskAveragePrice()
// IDA_EA: 0x8241DBE
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePriceD0Ev
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::~Dispatcher_AuctionAskAveragePrice()
Dispatcher_AuctionAskAveragePrice::~Dispatcher_AuctionAskAveragePrice() {}

// IDA_EA: 0x8213940
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionAskAveragePrice::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(49637,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (payment_type > 1) {
        return LineFunc(49640,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (IsAuctionServerDisconnected(payment_type)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_ASK_AVERAGE_PRICE,
                                 140,
                                 static_cast<PAY_TYPE>(payment_type));
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(49671,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }

    // The native body constructs and fills GA first, including for GP.
    PCK_AUCTION_ASK_AVERAGE_PRICE_GA ga_request;
    ga_request.user_uid = user->GetUID();
    ga_request.charac_no = user->getCurCharacNo();
    if (packet.get_int(ga_request.item_index) != 1) {
        return LineFunc(49687,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.field_1e) != 1) {
        return LineFunc(49690,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }

    if (payment_type == 1) {
        PCK_AUCTION_ASK_AVERAGE_PRICE_GP gp_request;
        gp_request.user_uid = user->GetUID();
        gp_request.charac_no = user->getCurCharacNo();
        gp_request.item_index = ga_request.item_index;
        gp_request.field_1e = ga_request.field_1e;
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &gp_request));
        // The old trailing GM-list branch is trace-only and has no protocol
        // or auction-state effect.
        return 0;
    }

    if (packet.get_byte(ga_request.field_1f) != 1) {
        return LineFunc(49711,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(ga_request.category.category_0) != 1) {
        return LineFunc(49714,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.category.flag_08) != 1) {
        return LineFunc(49717,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.field_20) != 1) {
        return LineFunc(49711,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(ga_request.category.category_1) != 1) {
        return LineFunc(49714,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.category.flag_09) != 1) {
        return LineFunc(49717,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.field_21) != 1) {
        return LineFunc(49711,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(ga_request.category.category_2) != 1) {
        return LineFunc(49714,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(ga_request.category.flag_0a) != 1) {
        return LineFunc(49717,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }

    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(ga_request.item_index));
    if (!item) {
        return LineFunc(49722,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    if (item->vtable_->is_random_option(item)) {
        if (!ga_request.category.category_0) {
            ga_request.category.category_0 = 30000;
        }
        if (!ga_request.category.category_1) {
            ga_request.category.category_1 = 30000;
        }
        if (!ga_request.category.category_2) {
            ga_request.category.category_2 = 30000;
        }
    }
    ga_request.category._sort();
    if (packet.get_byte(ga_request.field_2e) != 1) {
        return LineFunc(49735,
                        kAuctionAskAveragePriceDispatchSignature,
                        0,
                        0);
    }
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&ga_request));
    // The old trailing GM-list branch is trace-only and has no protocol or
    // auction-state effect.
    return 0;
}

// IDA_EA: 0x8233BFA
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionAskAveragePrice::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C04
// IDA_NAME: _ZN33Dispatcher_AuctionAskAveragePrice7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionAskAveragePrice::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionAskAveragePrice::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596738
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyC2Ev
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::Dispatcher_AuctionSearchByNoItmeKey(void)
Dispatcher_AuctionSearchByNoItmeKey::Dispatcher_AuctionSearchByNoItmeKey()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241BBA
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyD2Ev
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::~Dispatcher_AuctionSearchByNoItmeKey()
// IDA_EA: 0x8241BEA
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKeyD0Ev
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::~Dispatcher_AuctionSearchByNoItmeKey()
Dispatcher_AuctionSearchByNoItmeKey::~Dispatcher_AuctionSearchByNoItmeKey() {}

// IDA_EA: 0x82161E4
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionSearchByNoItmeKey::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(50969,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (payment_type > 1) {
        return LineFunc(50972,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (IsAuctionServerDisconnected(payment_type)) {
        // The old CMD190 body deliberately reports the CMD189 identifier in
        // this unavailable-server error packet.  Preserve that wire fact.
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_SEARCH_BY_ITEMKEY,
                                 140,
                                 static_cast<PAY_TYPE>(payment_type));
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(51007,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }

    if (payment_type == 1) {
        PCK_AUCTION_SEARCH_BY_NOITEMKEY_GP request;
        request.user_uid = user->GetUID();
        request.charac_no = user->getCurCharacNo();
        if (packet.get_int(request.search.query_value_) != 1) {
            return LineFunc(51016,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_short(request.search.query_filter_0_) != 1) {
            return LineFunc(51017,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_1_) != 1) {
            return LineFunc(51018,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_2_) != 1) {
            return LineFunc(51019,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_3_) != 1) {
            return LineFunc(51020,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_4_) != 1) {
            return LineFunc(51021,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_5_) != 1) {
            return LineFunc(51022,
                            kAuctionSearchByNoItemKeyDispatchSignature,
                            0,
                            0);
        }
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        // The old trailing GM-list branch is trace-only and has no protocol
        // or auction-state effect.
        return 0;
    }

    PCK_AUCTION_SEARCH_BY_NOITEMKEY_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    if (packet.get_int(request.search.query_value_) != 1) {
        return LineFunc(51040,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.query_filter_0_) != 1) {
        return LineFunc(51041,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_1_) != 1) {
        return LineFunc(51042,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_2_) != 1) {
        return LineFunc(51043,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_3_) != 1) {
        return LineFunc(51044,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_4_) != 1) {
        return LineFunc(51045,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_5_) != 1) {
        return LineFunc(51046,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.category_.category_0) != 1) {
        return LineFunc(51050,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.category_.category_1) != 1) {
        return LineFunc(51052,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.category_.category_2) != 1) {
        return LineFunc(51054,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.category_filter_0_) != 1) {
        return LineFunc(51058,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.category_filter_1_) != 1) {
        return LineFunc(51059,
                        kAuctionSearchByNoItemKeyDispatchSignature,
                        0,
                        0);
    }
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The old trailing GM-list branch is trace-only and has no protocol or
    // auction-state effect.
    return 0;
}

// IDA_EA: 0x8233C5E
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionSearchByNoItmeKey::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C68
// IDA_NAME: _ZN35Dispatcher_AuctionSearchByNoItmeKey7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionSearchByNoItmeKey::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionSearchByNoItmeKey::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859671C
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyC2Ev
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::Dispatcher_AuctionSearchByItemKey(void)
Dispatcher_AuctionSearchByItemKey::Dispatcher_AuctionSearchByItemKey()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241C08
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyD2Ev
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::~Dispatcher_AuctionSearchByItemKey()
// IDA_EA: 0x8241C38
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKeyD0Ev
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::~Dispatcher_AuctionSearchByItemKey()
Dispatcher_AuctionSearchByItemKey::~Dispatcher_AuctionSearchByItemKey() {}

// IDA_EA: 0x82159FC
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AuctionSearchByItemKey::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    unsigned char payment_type = 0;
    if (packet.get_byte(payment_type) != 1) {
        return LineFunc(50832,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (payment_type > 1) {
        return LineFunc(50835,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (IsAuctionServerDisconnected(payment_type)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_AUCTION_SEARCH_BY_ITEMKEY,
                                 140,
                                 static_cast<PAY_TYPE>(payment_type));
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(50871,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }

    if (payment_type == 1) {
        PCK_AUCTION_SEARCH_BY_ITEMKEY_GP request;
        request.user_uid = user->GetUID();
        request.charac_no = user->getCurCharacNo();
        if (packet.get_int(request.search.query_value_) != 1) {
            return LineFunc(50880,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_0_) != 1) {
            return LineFunc(50881,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.query_filter_1_) != 1) {
            return LineFunc(50882,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_byte(request.search.item_key_count_) != 1) {
            return LineFunc(50883,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (packet.get_short(request.search.query_filter_2_) != 1) {
            return LineFunc(50884,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        if (request.search.item_key_count_ > 0x14u) {
            return LineFunc(50887,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
        for (unsigned int index = 0; index < request.search.item_key_count_;
             ++index) {
            if (packet.get_int(request.search.item_keys_[index]) != 1) {
                return LineFunc(50892,
                                kAuctionSearchByItemKeyDispatchSignature,
                                0,
                                0);
            }
        }
        static_cast<void>(GlobalData::s_cera_auction_proxy->SendPacket(
            &request));
        // The old trailing GM-list branch is trace-only and has no protocol
        // or auction-state effect.
        return 0;
    }

    PCK_AUCTION_SEARCH_BY_ITEMKEY_GA request;
    request.user_uid = user->GetUID();
    request.charac_no = user->getCurCharacNo();
    if (packet.get_int(request.search.query_value_) != 1) {
        return LineFunc(50913,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_0_) != 1) {
        return LineFunc(50914,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.query_filter_1_) != 1) {
        return LineFunc(50915,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.item_key_count_) != 1) {
        return LineFunc(50916,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.query_filter_2_) != 1) {
        return LineFunc(50917,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (request.search.item_key_count_ > 0x14u) {
        return LineFunc(50920,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    for (unsigned int index = 0; index < request.search.item_key_count_; ++index) {
        if (packet.get_int(request.search.item_keys_[index]) != 1) {
            return LineFunc(50925,
                            kAuctionSearchByItemKeyDispatchSignature,
                            0,
                            0);
        }
    }
    if (packet.get_short(request.search.category_.category_0) != 1) {
        return LineFunc(50931,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.category_.category_1) != 1) {
        return LineFunc(50933,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_short(request.search.category_.category_2) != 1) {
        return LineFunc(50935,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    request.search.category_._sort();
    if (packet.get_byte(request.search.category_filter_0_) != 1) {
        return LineFunc(50941,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    if (packet.get_byte(request.search.category_filter_1_) != 1) {
        return LineFunc(50942,
                        kAuctionSearchByItemKeyDispatchSignature,
                        0,
                        0);
    }
    static_cast<void>(GlobalData::s_auction_proxy->SendPacket(&request));
    // The old trailing GM-list branch is trace-only and has no protocol or
    // auction-state effect.
    return 0;
}

// IDA_EA: 0x8233C4A
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AuctionSearchByItemKey::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233C54
// IDA_NAME: _ZN33Dispatcher_AuctionSearchByItemKey7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AuctionSearchByItemKey::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AuctionSearchByItemKey::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x821B60A
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine18_getVendingMachineEj
// DEMANGLED: Dispatcher_UseVendingMachine::_getVendingMachine(unsigned int)const
const stVendingMachine_t* Dispatcher_UseVendingMachine::_getVendingMachine(
    unsigned int script_index) const {
    const std::vector<stVendingMachine_t>* const vending_machines =
        G_CDataManager()->getVendingMachineTable();
    for (std::vector<stVendingMachine_t>::const_iterator it =
             vending_machines->begin();
         it != vending_machines->end();
         ++it) {
        if (static_cast<unsigned int>(it->script_index) == script_index) {
            return &*it;
        }
    }
    return 0;
}

// IDA_EA: 0x821B698
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine22_getVendingMachineInfoEPK18stVendingMachine_tj
// DEMANGLED: Dispatcher_UseVendingMachine::_getVendingMachineInfo(stVendingMachine_t const*,unsigned int)const
const stVendingMachineInfo_t*
Dispatcher_UseVendingMachine::_getVendingMachineInfo(
    const stVendingMachine_t* vending_machine,
    unsigned int group_number) const {
    for (std::vector<stVendingMachineInfo_t>::const_iterator it =
             vending_machine->groups.begin();
         it != vending_machine->groups.end();
         ++it) {
        if (static_cast<unsigned int>(it->group_num) == group_number) {
            return &*it;
        }
    }
    return 0;
}

// IDA_EA: 0x821BF82
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine12_getGiveItemEPK22stVendingMachineInfo_t
// DEMANGLED: Dispatcher_UseVendingMachine::_getGiveItem(stVendingMachineInfo_t const*)const
Inven_Item Dispatcher_UseVendingMachine::_getGiveItem(
    const stVendingMachineInfo_t* vending_machine_info) const {
    Inven_Item selected_item;
    selected_item.item_id_ = -1;

    const unsigned long roll_limit = static_cast<unsigned long>(
        vending_machine_info->normal_total_rate - 1);
    const unsigned int roll = random_->randInt(roll_limit);
    for (std::vector<stVendingMachineOutput_t>::const_iterator it =
             vending_machine_info->normal_outputs.begin();
         it != vending_machine_info->normal_outputs.end();
         ++it) {
        if (it->cumulative_rate > roll) {
            selected_item.item_id_ = static_cast<int>(it->item_index);
            selected_item.set_add_info(static_cast<int>(it->item_count));
            return selected_item;
        }
    }
    return selected_item;
}

// IDA_EA: 0x821C058
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine13_getBonusItemEPK22stVendingMachineInfo_t
// DEMANGLED: Dispatcher_UseVendingMachine::_getBonusItem(stVendingMachineInfo_t const*)const
Inven_Item Dispatcher_UseVendingMachine::_getBonusItem(
    const stVendingMachineInfo_t* vending_machine_info) const {
    Inven_Item selected_item;
    selected_item.item_id_ = -1;

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(35);
    if (repeat_event->IsEventing(0) != 1 ||
        vending_machine_info->bonus_outputs.empty()) {
        return selected_item;
    }

    const unsigned long roll_limit = static_cast<unsigned long>(
        vending_machine_info->bonus_total_rate - 1);
    const unsigned int roll = random_->randInt(roll_limit);
    for (std::vector<stVendingMachineOutput_t>::const_iterator it =
             vending_machine_info->bonus_outputs.begin();
         it != vending_machine_info->bonus_outputs.end();
         ++it) {
        if (it->cumulative_rate > roll) {
            selected_item.item_id_ = static_cast<int>(it->item_index);
            selected_item.set_add_info(static_cast<int>(it->item_count));
            return selected_item;
        }
    }
    return selected_item;
}

// IDA_EA: 0x821C17C
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine15_notifyItemInfoEPK22stVendingMachineInfo_tP5CUserRKNS_17stPutItemResult_tE
// DEMANGLED: Dispatcher_UseVendingMachine::_notifyItemInfo(stVendingMachineInfo_t const*,CUser *,Dispatcher_UseVendingMachine::stPutItemResult_t const&)const
char Dispatcher_UseVendingMachine::_notifyItemInfo(
    const stVendingMachineInfo_t* vending_machine_info,
    CUser* user,
    const stPutItemResult_t& result) const {
    const std::pair<int, int> result_item(
        result.item_id, static_cast<int>(result.item_count));
    const std::vector<std::pair<int, int> >& dictionary_items =
        vending_machine_info->dictionary_items;
    if (std::find(dictionary_items.begin(),
                  dictionary_items.end(),
                  result_item) != dictionary_items.end()) {
        return 1;
    }

    return static_cast<char>(TimerVendingMachineItemNotify::registTimer(
        user->GetUID(),
        static_cast<unsigned long>(static_cast<unsigned int>(result.item_id)),
        result.item_count,
        5u));
}

// IDA_EA: 0x821B71C
// IDA_NAME: _ZNK28Dispatcher_UseVendingMachine16_putItemIntoUserEP5CUsermjRNS_17stPutItemResult_tE
// DEMANGLED: Dispatcher_UseVendingMachine::_putItemIntoUser(CUser *,unsigned long,unsigned int,Dispatcher_UseVendingMachine::stPutItemResult_t &)const
int Dispatcher_UseVendingMachine::_putItemIntoUser(
    CUser* user,
    unsigned long item_index,
    unsigned int item_count,
    stPutItemResult_t& result) const {
    result.item_id = static_cast<int>(item_index);
    result.item_count = item_count;
    result.item_space = 0;
    result.slot = -1;
    result.success = 1;
    result.reserved = 0;
    if (result.item_id == -1) {
        return 0;
    }

    CInventory* const writable_inventory = user->getCurCharacInvenW();
    switch (item_index) {
        case 0u:
            writable_inventory->gain_money(
                static_cast<int>(item_count),
                static_cast<eMoneyAddReason>(21),
                true,
                0);
            result.slot = 0;
            break;

        case 1u: {
            const unsigned int event_coin = static_cast<unsigned int>(
                writable_inventory->GetEventCoin()) + item_count;
            writable_inventory->SetEventCoin(event_coin);
            user->history_log().EventCoinAdd(
                user->getCurCharacInvenR()->GetEventCoin(),
                static_cast<int>(item_count),
                static_cast<eCoinAddReason>(6));
            result.slot = 1;
            break;
        }

        case 2u:
            user->gainWinPoint(static_cast<int>(item_count),
                               static_cast<eWPAddReason>(3));
            result.slot = 2;
            break;

        default: {
            CItem* const item = G_CDataManager()->find_item(result.item_id);
            if (!item) {
                result.success = 0;
                return 0;
            }

            Inven_Item created_item;
            created_item.item_id_ = result.item_id;
            G_CDataManager()->get_arad_jumping_item_list()->create_item(
                ENUM_ITEM_CREATE_TYPE_DEFAULT,
                created_item,
                static_cast<int>(item_count));

            if (item->vtable_->get_item_type(item) == 23) {
                const std::vector<stGlobalEffect_t>& global_effects =
                    static_cast<const CStackableItem*>(item)
                        ->getGlobalEffectData();
                for (std::vector<stGlobalEffect_t>::const_iterator it =
                         global_effects.begin();
                     it != global_effects.end();
                     ++it) {
                    GlobalData::s_globalEffectManager_->registNewEffect(
                        it->effect_type,
                        it->effect_value,
                        user->getCurCharacName(),
                        5L,
                        static_cast<long>(it->duration_seconds));
                }
                break;
            }

            if (item->vtable_->is_avatar_item(item)) {
                char ipg_no[24];
                const char* const generated_ipg_no =
                    WongWork::CGenUniqueNo::genIPGNo(
                        static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(4),
                        static_cast<unsigned int>(user->getCurCharacNo()),
                        ipg_no);
                result.slot = writable_inventory->AddAvatarItem(
                    result.item_id,
                    item->getUsablePeriod(),
                    false,
                    0,
                    static_cast<char>(-1),
                    generated_ipg_no,
                    static_cast<eAvatarItemAddReason>(14),
                    false,
                    0);
                result.item_space = 1;
                break;
            }

            if (item->vtable_->is_creature_item(item) &&
                item->vtable_->get_item_type(item) ==
                    oldsrv::config::kEquipmentTypeCreature) {
                if (IsVendingMachineCreatureEggRecord(created_item)) {
                    if (!user_creature::CCreatureItemConverter::SetEggItem(
                            &created_item)) {
                        return 0;
                    }
                    result.slot = writable_inventory->insertItemIntoInventory(
                        created_item,
                        static_cast<eItemAddReason>(35),
                        true,
                        false);
                    if (result.slot >= 0) {
                        writable_inventory->GetCreatureMgrW()
                            ->InsertCreatureItem(
                                &created_item,
                                result.slot,
                                35,
                                item->getUsablePeriod(),
                                item->getExpirationDate());
                    }
                    result.item_space = 7;
                }
                break;
            }

            int special_type = 0;
            int special_value = 0;
            bool unique = false;
            if (GlobalData::s_pCeraShop->IsSpecialItem(result.item_id,
                                                       special_type,
                                                       special_value,
                                                       unique) &&
                item->vtable_->get_item_type(item) != 16) {
                std::vector<Inven_Item> output_items;
                int process_result = 0;
                const int restrict_code =
                    G_CDataManager()->get_arad_jumping_item_list()
                        ->GetRestrictCode(result.item_id);
                const int current_seconds =
                    GlobalData::system_time()->getCurSec();
                WongWork::CCeraShop::stCeraShopItemParam_t item_param;
                if (GlobalData::s_pCeraShop->ProcessSpecialItem(
                        user,
                        result.item_id,
                        restrict_code,
                        process_result,
                        static_cast<long>(current_seconds),
                        true,
                        output_items,
                        item_param,
                        0,
                        0,
                        false,
                        1)) {
                    if (process_result) {
                        return 0;
                    }
                    if (restrict_code &&
                        !user->IsRestrictedGoods(restrict_code)) {
                        user->SetRestrictedGoods(restrict_code);
                        GlobalData::s_pCeraShop->SaveFeaturedIdx(
                            user, restrict_code, current_seconds, 0);
                        user->SendRestrictedGoods(true, restrict_code);
                    }
                }
                break;
            }

            result.slot = writable_inventory->insertItemIntoInventory(
                created_item,
                static_cast<eItemAddReason>(35),
                true,
                false);
            if (result.slot == -1) {
                result.success = 0;
            }
            break;
        }
    }

    if (result.success) {
        return GetInstanceVendingMachineStatistic()->addOutputItem(
            static_cast<unsigned long>(
                static_cast<unsigned int>(result.item_id)),
            result.item_count);
    }
    return result.success;
}

// IDA_EA: 0x821C25C
// IDA_NAME: _ZN28Dispatcher_UseVendingMachineC2Ev
// DEMANGLED: Dispatcher_UseVendingMachine::Dispatcher_UseVendingMachine(void)
Dispatcher_UseVendingMachine::Dispatcher_UseVendingMachine()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>(),
      random_(new CMTRand(static_cast<unsigned long>(std::time(0)))) {}

// IDA_EA: 0x82415F0
// IDA_NAME: _ZN28Dispatcher_UseVendingMachineD2Ev
// DEMANGLED: Dispatcher_UseVendingMachine::~Dispatcher_UseVendingMachine()
// IDA_EA: 0x8241620
// IDA_NAME: _ZN28Dispatcher_UseVendingMachineD0Ev
// DEMANGLED: Dispatcher_UseVendingMachine::~Dispatcher_UseVendingMachine()
// The native D2 only restores the dispatcher/base vtable; it intentionally
// does not free random_, so the historical allocation lifetime is preserved.
Dispatcher_UseVendingMachine::~Dispatcher_UseVendingMachine() {}

// IDA_EA: 0x821C2E6
// IDA_NAME: _ZN28Dispatcher_UseVendingMachine12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UseVendingMachine::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UseVendingMachine::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (!user || user->get_state() != 3 || user->CheckInTrade()) {
        return 0;
    }

    unsigned int script_index = 0;
    unsigned int group_number = 0;
    short material_slot = -1;
    if (packet.get_int(script_index) != 1) {
        return LineFunc(53289, kUseVendingMachineDispatchSignature, 0, 0);
    }
    if (packet.get_int(group_number) != 1) {
        return LineFunc(53290, kUseVendingMachineDispatchSignature, 0, 0);
    }
    if (packet.get_short(material_slot) != 1) {
        return LineFunc(53291, kUseVendingMachineDispatchSignature, 0, 0);
    }

    const stVendingMachine_t* const vending_machine =
        _getVendingMachine(script_index);
    if (!vending_machine) {
        return LineFunc(53295, kUseVendingMachineDispatchSignature, 0, 0);
    }
    const stVendingMachineInfo_t* const vending_machine_info =
        _getVendingMachineInfo(vending_machine, group_number);
    if (!vending_machine_info) {
        return LineFunc(53299, kUseVendingMachineDispatchSignature, 0, 0);
    }

    Inven_Item normal_item;
    normal_item.item_id_ = -1;
    unsigned int retry_count = 0;
    do {
        if (retry_count > 2u) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 1);
            return 0;
        }

        normal_item = _getGiveItem(vending_machine_info);
        if (normal_item.getKey() == -1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 1);
            return 0;
        }

        CItem* const selected_template =
            G_CDataManager()->find_item(normal_item.getKey());
        if (selected_template &&
            selected_template->vtable_->get_item_type(selected_template) == 23 &&
            GlobalData::s_globalEffectManager_->getEffectNum()) {
            normal_item.item_id_ = -1;
            normal_item.set_add_info(0);
            ++retry_count;
        }
    } while (normal_item.getKey() == -1);

    Inven_Item bonus_item = _getBonusItem(vending_machine_info);
    CInventory* const writable_inventory = user->getCurCharacInvenW();
    if (writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(1), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(2), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(3), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(10), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(9), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(5), 2) != 1 ||
        writable_inventory->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(6), 2) != 1 ||
        writable_inventory->GetRemainCapacity(
            static_cast<INVEN_TYPE>(2),
            static_cast<Inven_Item::ITEM_TYPE>(8)) <= 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 4);
        return 0;
    }

    int remaining_payment = 0;
    if (vending_machine_info->material_item == 0) {
        if (writable_inventory->use_money(
                vending_machine_info->material_count,
                static_cast<eMoneySubReason>(28),
                true) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 22);
            return 0;
        }
        remaining_payment = writable_inventory->get_money();
    } else if (vending_machine_info->material_item == 1) {
        if (user->getCurCharacInvenR()->getTotalCoin() <
            vending_machine_info->material_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 22);
            return 0;
        }
        for (unsigned int count = 0;
             count < static_cast<unsigned int>(
                         vending_machine_info->material_count);
             ++count) {
            writable_inventory->UseCoin(static_cast<eCoinSubReason>(6));
        }
        remaining_payment = user->getCurCharacInvenR()->getTotalCoin();
    } else if (vending_machine_info->material_item == 2) {
        if (user->GetWinPoint() < vending_machine_info->material_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 22);
            return 0;
        }
        user->UseWinPoint(vending_machine_info->material_count,
                          static_cast<eWPSubReason>(4));
        remaining_payment = user->GetWinPoint();
    } else {
        Inven_Item material_item = user->getCurCharacInvenR()->GetInvenSlot(
            1, material_slot);
        if (material_item.getKey() != vending_machine_info->material_item) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 17);
            return 0;
        }
        if (material_item.get_add_info() <
            vending_machine_info->material_count) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 22);
            return 0;
        }
        if (writable_inventory->delete_item(
                static_cast<INVEN_TYPE>(1),
                material_slot,
                vending_machine_info->material_count,
                static_cast<eItemDelReason>(28),
                true) != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_VENDING_MACHINE, 22);
            return 0;
        }
        remaining_payment = user->getCurCharacInvenR()
                                ->GetInvenSlot(1, material_slot)
                                .get_add_info();
    }

    GetInstanceVendingMachineStatistic()->incUseCount();
    stPutItemResult_t result_items[2];
    _putItemIntoUser(
        user,
        static_cast<unsigned long>(static_cast<unsigned int>(normal_item.getKey())),
        static_cast<unsigned int>(normal_item.get_add_info()),
        result_items[0]);
    _putItemIntoUser(
        user,
        static_cast<unsigned long>(static_cast<unsigned int>(bonus_item.getKey())),
        static_cast<unsigned int>(bonus_item.get_add_info()),
        result_items[1]);

    if (result_items[0].success != 1) {
        const stVendingMachineOutput_t& fallback_item =
            vending_machine_info->normal_outputs.front();
        _putItemIntoUser(user,
                         static_cast<unsigned long>(fallback_item.item_index),
                         fallback_item.item_count,
                         result_items[0]);
    }
    if (result_items[0].item_id != -1 && result_items[0].item_count != 0u) {
        _notifyItemInfo(vending_machine_info, user, result_items[0]);
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_USE_VENDING_MACHINE);
    response.put_byte(1);
    response.put_short(material_slot);
    response.put_int(remaining_payment);
    for (int index = 0; index < 2; ++index) {
        response.put_item_idx(static_cast<unsigned long>(
            static_cast<unsigned int>(result_items[index].item_id)));
        response.put_int(static_cast<int>(result_items[index].item_count));
    }

    int item_packet_count_index = response.get_index();
    response.put_short(0);
    int item_packet_count = 0;
    for (int index = 0; index < 2; ++index) {
        const stPutItemResult_t& result = result_items[index];
        if (result.item_id == -1 || result.slot == -1 ||
            result.success != 1) {
            continue;
        }
        response.put_byte(result.item_space);
        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(result.item_space));
        user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(inventory_type), result.slot, response);
        ++item_packet_count;
    }
    response.put_short(item_packet_count_index, item_packet_count);
    response.finalize(true);
    user->Send(response);
    APSystem::CUserProc::ClearActionAndSendtoUser(
        user,
        static_cast<APSystem::_ActionGroupIndex>(39),
        0,
        ENUM_PACKETCLASS_NOTIPACKET);
    return 0;
}

// IDA_EA: 0x8233DB2
// IDA_NAME: _ZN28Dispatcher_UseVendingMachine4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseVendingMachine::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseVendingMachine::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233DBC
// IDA_NAME: _ZN28Dispatcher_UseVendingMachine7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseVendingMachine::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseVendingMachine::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
