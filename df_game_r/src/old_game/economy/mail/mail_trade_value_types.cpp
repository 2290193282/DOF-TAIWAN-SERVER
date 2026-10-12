#include "old_game/economy/mail/mail_trade_value_types.h"
#include "common/config.h"

#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mall/purchase_cash_item_bonus_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_data/PVF/event/in_game_event_pvf_scripts.h"
#include "old_data/PVF/event/action_mail_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/auction_cashshop/db_load_auto_market_conditions_control.h"
#include "old_data/db/auction_cashshop/db_update_auto_market_conditions_control.h"
#include "old_data/db/avatar_aura/db_avatar_change_owner.h"
#include "old_data/db/avatar_aura/db_insert_cera_item_history.h"
#include "old_data/db/auction_cashshop/zero_trade_payload.h"
#include "old_data/db/mail/mailbox_payload.h"
#include "old_game/economy/mail/mailbox_system_multi_mail_payload.h"
#include "core/config/env/environment.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/find_price_abi.h"
#include "old_game/world/region/gameworld_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/history_log_abi.h"
#include "common/manager/os_api.h"
#include "infra/security/policy/hades_public_abi.h"
#include "common/manager/global_tools.h"
#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_game/economy/inventory/inventory_public_abi.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "common/manager/server_utility.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/transport/network_endpoint.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

#include <arpa/inet.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <limits>
#include <new>
#include <stdexcept>
#include <ctime>

// Old ABI call target: CItem::GetAttachType(void)const @ 0x80F12E2
// (_ZNK5CItem13GetAttachTypeEv).  The real CItem provider is canonical A
// source; this declaration is a caller-only boundary and must not masquerade
// as a second IDA provider in the C Commerce ledger.
// C-owned equipment and world-state call boundaries used by the direct trade
// rollback body below.  These retain the old member ABIs without rebuilding
// DataManager's PVF template object hierarchy in the trade module.
// A-owned public/security transport boundaries consumed by the direct CTrade
// state transition.  C keeps the gameplay ordering but does not duplicate
// protection policy or the shared packet writer.
int SendChangeTradeState(PacketGuard& packet,
                         CUser* source,
                         CUser* target,
                         int state)
    __asm__("_Z20SendChangeTradeStateR11PacketGuardP5CUserS2_15ENUM_TRADESTATE");

// The settlement path crosses established ABI surfaces from A/B and other C
// IDA units.  These declarations are call boundaries only; no transport,
// DB, avatar-manager, history, or security body is reproduced here.
int IsTradeLimitAttachTypeItem(const Inven_Item* item)
    __asm__("_Z26IsTradeLimitAttachTypeItemRK10Inven_Item");
int DecreaseTradeLimitCount(Inven_Item* item)
    __asm__("_Z23DecreaseTradeLimitCountR10Inven_Item");
// The B payload layout is not public yet; the exact tag still lets this C
// caller emit the original A-owned GetInBuffer specialization symbol.
struct SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL;

typedef char NexonCash_ByteBuffer_Size_Must_Be_0x10[
    (sizeof(nexon::cash::ByteBuffer) == 0x10) ? 1 : -1];
typedef char NexonCash_NCashMessage_Size_Must_Be_0x208[
    (sizeof(nexon::cash::NCashMessage) == 0x208) ? 1 : -1];
typedef char NexonCash_NCashImpl_Size_Must_Be_0x450[
    (sizeof(nexon::cash::NCashImpl) == 0x450) ? 1 : -1];
typedef char NexonCash_NexonCash_Size_Must_Be_0x04[
    (sizeof(nexon::cash::NexonCash) == 0x04) ? 1 : -1];
typedef char NexonCash_TransactionManager_Size_Must_Be_0x18[
    (sizeof(nexon::cash::TransactionManager) == 0x18) ? 1 : -1];
typedef char NexonCash_Transaction_Size_Must_Be_0x10[
    (sizeof(nexon::cash::Transaction) == 0x10) ? 1 : -1];
typedef char NexonCash_UserBalance_Size_Must_Be_0x14[
    (sizeof(nexon::cash::UserBalance) == 0x14) ? 1 : -1];

typedef char ARAD_DATAS_AradMailItemInfo_Size_Must_Be_0x18[
    (sizeof(ARAD_DATAS::AradMailItemInfo) == 0x18) ? 1 : -1];
typedef char ARAD_DATAS_AradMailInfo_Size_Must_Be_0x38[
    (sizeof(ARAD_DATAS::AradMailInfo) == 0x38) ? 1 : -1];
typedef char CTradeSpace_Size_Must_Be_0xf70[
    (sizeof(CTradeSpace) == 0xf70) ? 1 : -1];
typedef char CAuctionAveragePrice_Size_Must_Be_0x18[
    (sizeof(CAuctionAveragePrice) == 0x18) ? 1 : -1];
typedef char CAutoMarketConditionsControl_Size_Must_Be_0x444[
    (sizeof(CAutoMarketConditionsControl) == 0x444) ? 1 : -1];
typedef char CAutoMarketConditionsControlEvent_Size_Must_Be_0x0c[
    (sizeof(CAutoMarketConditionsControlEvent) == 0x0c) ? 1 : -1];
typedef char Inter_AuctionNotifyAuctionService_Size_Must_Be_0x04[
    (sizeof(Inter_AuctionNotifyAuctionService) == 0x04) ? 1 : -1];
typedef char Inter_UpdatePurchaseCashItemBonus_Size_Must_Be_0x04[
    (sizeof(Inter_UpdatePurchaseCashItemBonus) == 0x04) ? 1 : -1];
typedef char Inter_AuctionLogMessage_Size_Must_Be_0x04[
    (sizeof(Inter_AuctionLogMessage) == 0x04) ? 1 : -1];
typedef char Inter_PointAuctionLogMessage_Size_Must_Be_0x04[
    (sizeof(Inter_PointAuctionLogMessage) == 0x04) ? 1 : -1];
typedef char Inter_LoadAutoMarketConditionsControl_Size_Must_Be_0x04[
    (sizeof(Inter_LoadAutoMarketConditionsControl) == 0x04) ? 1 : -1];
typedef char WongWork_CMailBox_CMail_Size_Must_Be_0x80[
    (sizeof(WongWork::CMailBox::CMail) == 0x80) ? 1 : -1];
typedef char WongWork_CMailBox_stAddNewMailInput_Size_Must_Be_0x28[
    (sizeof(WongWork::CMailBox::stAddNewMailInput) == 0x28) ? 1 : -1];
typedef char WongWork_CMailBox_Size_Must_Be_0xa8[
    (sizeof(WongWork::CMailBox) == 0xa8) ? 1 : -1];

// This translation unit is retained as the source-of-truth recovery ledger.
// The non-transaction bodies below already have canonical providers in the
// focused commerce runtime objects.
#if 0
namespace ARAD_DATAS {

// IDA_EA: 0x8197E12
// IDA_NAME: _ZNK10ARAD_DATAS16AradMailItemInfo11GetItemAttrEv
// DEMANGLED: ARAD_DATAS::AradMailItemInfo::GetItemAttr(void)const
int AradMailItemInfo::GetItemAttr() const {
    return item_attr;
}

// IDA_EA: 0x81A3FC2
// IDA_NAME: _ZN10ARAD_DATAS16AradMailItemInfoC2Ev
// DEMANGLED: ARAD_DATAS::AradMailItemInfo::AradMailItemInfo(void)
AradMailItemInfo::AradMailItemInfo()
    : seal_flag(0),
      endurance(0),
      item_index(0),
      extend_info(0),
      item_attr(0) {}

// IDA_EA: 0x81A3FF2
// IDA_NAME: _ZN10ARAD_DATAS12AradMailInfoC2Ev
// DEMANGLED: ARAD_DATAS::AradMailInfo::AradMailInfo(void)
AradMailInfo::AradMailInfo()
    : server_group(0),
      gold(0),
      occ_time(0),
      receiver_charac_no(0),
      sender_charac_no(0),
      letter_text_length(0),
      content_address(0),
      title_address(0),
      item_info() {}

}  // namespace ARAD_DATAS

namespace WongWork {

// Migration marker: the ReqDBLoadMail through ReqDBSendNewSystemMail bodies
// in batch-023 slice2 now have their sole provider in old_game/mail_trade_cash.
// IDA_EA: 0x8555756
// IDA_NAME: _ZN8WongWork14CMailBoxHelper17_makeSendMailDataEP25SIG_MAILBOX_SEND_NEW_MAILPKciRK10Inven_ItemjjS4_iji
// DEMANGLED: WongWork::CMailBoxHelper::_makeSendMailData(SIG_MAILBOX_SEND_NEW_MAIL *,char const*,int,Inven_Item const&,unsigned int,unsigned int,char const*,int,unsigned int,int)
SIG_MAILBOX_SEND_NEW_MAIL* CMailBoxHelper::_makeSendMailData(
                                        SIG_MAILBOX_SEND_NEW_MAIL* payload,
                                        const char* title,
                                        int sender_charac_no,
                                        const Inven_Item& item,
                                        unsigned int money,
                                        unsigned int charac_no,
                                        const char* text,
                                        int text_len,
                                        unsigned int occ_time,
                                        int unknown) {
    SIG_MAILBOX_SEND_NEW_MAIL::Request& request = payload->request;
    InvenItemMailRecord item_record;
    item.ExportMailRecord(item_record);

    std::strncpy(request.sender_charac_name,
                 title,
                 sizeof(request.sender_charac_name) - 1u);
    request.sender_charac_no = sender_charac_no;
    request.receiver_charac_no = static_cast<int32_t>(charac_no);
    request.gold = static_cast<int32_t>(money);
    request.seal_flag = item_record.seal_flag;
    request.item_id = item_record.item_id;
    request.add_info = item_record.add_info;
    request.upgrade = item_record.item_attr;
    request.endurance = item_record.endurance;
    request.extend_info = item_record.extend_info;
    std::memcpy(request.item_guid, item_record.item_guid,
                sizeof(item_record.item_guid));
    request.amplify_option = item_record.amplify_option;
    request.amplify_value = item_record.amplify_value;
    std::memcpy(request.random_option, item_record.random_option,
                sizeof(item_record.random_option));
    request.separate_upgrade = item_record.separate_upgrade;
    request.avatar_flag = item_record.avatar_flag ? 1 : 0;
    request.creature_flag = item_record.creature_flag ? 1 : 0;
    request.letter_text_length = 0;
    std::memset(request.letter_text, 0, sizeof(request.letter_text));
    if (text && text_len) {
        const uint16_t requested = static_cast<uint16_t>(text_len);
        const uint16_t capped = requested < 255u ? requested : 255u;
        request.letter_text_length = capped;
        std::strncpy(request.letter_text, text, capped);
    }
    request.occ_time = static_cast<int32_t>(occ_time);
    request.old_letter_id = unknown;
    request.db_work = 0;
    return payload;
}

// IDA_EA: 0x8556B68
// IDA_NAME: _ZN8WongWork14CMailBoxHelper27ReqDBSendNewSystemMultiMailEPKcPK10Inven_ItemjjjS2_ij17ENUM_SERVER_GROUPb
// DEMANGLED: WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(char const*,Inven_Item const*,unsigned int,unsigned int,unsigned int,char const*,int,unsigned int,ENUM_SERVER_GROUP,bool)
int CMailBoxHelper::ReqDBSendNewSystemMultiMail(
    const char* title,
    const Inven_Item* items,
    unsigned int item_count,
    unsigned int sender,
    unsigned int charac_no,
    const char* text,
    int text_len,
    unsigned int occ_time,
    ENUM_SERVER_GROUP server_group,
    bool unlimit) {
    if (item_count > 0x0au || item_count == 0) {
        return 17;
    }

    Stream* stream = GlobalData::stream_pool()->Acquire(
        "MailBox.cpp",
        2327);
    CStreamGuard guard(stream, true);
    guard << 676;
    guard << -1;

    SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL* const payload =
        guard.GetInBuffer<SIG_MAILBOX_SEND_SYSTEM_MULTI_MAIL>();
    std::memset(payload, 0, sizeof(*payload));
    payload->sender_charac_no = 0;
    std::strcpy(payload->title, title);
    for (unsigned int index = 0; index < item_count; ++index) {
        payload->items[index].setCopy(items[index]);
    }
    payload->item_count = item_count;
    payload->sender = sender;
    payload->charac_no = charac_no;
    std::strcpy(payload->letter_text, text);
    payload->text_length = text_len;
    payload->occurrence_days = occ_time;
    payload->server_group = server_group;
    payload->unlimit_flag = unlimit ? 1 : 0;
    payload->letter_stat = 1;

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

}  // namespace WongWork

// IDA_EA: 0x823438E
// IDA_NAME: _ZN11CTradeSpace9get_indexEv
// DEMANGLED: CTradeSpace::get_index(void)
int CTradeSpace::get_index() {
    return index_;
}

// IDA_EA: 0x823439A
// IDA_NAME: _ZN11CTradeSpace8is_emptyEv
// DEMANGLED: CTradeSpace::is_empty(void)
int CTradeSpace::is_empty() {
    return occupied_ ^ 1;
}

// IDA_EA: 0x82A416C
// IDA_NAME: _ZN11CTradeSpaceC2Ev
// DEMANGLED: CTradeSpace::CTradeSpace(void)
CTradeSpace::CTradeSpace()
    : mutex_(),
      occupied_(0),
      locked_(0) {}

// IDA_EA: 0x82A41E0
// IDA_NAME: _ZN11CTradeSpaceD2Ev
// DEMANGLED: CTradeSpace::~CTradeSpace()
CTradeSpace::~CTradeSpace() {}

// IDA_EA: 0x82A41F4
// IDA_NAME: _ZN11CTradeSpace6SetIDXEi
// DEMANGLED: CTradeSpace::SetIDX(int)
CTradeSpace* CTradeSpace::SetIDX(int index) {
    index_ = index;
    return this;
}

// IDA_EA: 0x85295F0
// IDA_NAME: _ZN11CTradeSpace11set_tradersEP5CUserS1_
// DEMANGLED: CTradeSpace::set_traders(CUser *,CUser *)
int CTradeSpace::set_traders(CUser* first, CUser* second) {
    lock();
    occupied_ = 1;
    _SetLock(false);
    traders_[0] = first;
    first->trade_peer_unique_id_ =
        static_cast<unsigned short>(second->get_unique_id());
    traders_[1] = second;
    second->trade_peer_unique_id_ =
        static_cast<unsigned short>(first->get_unique_id());

    if (second->getCurCharacR() &&
        first->getCurCharacR()) {
        CUser_history_log(first)->SetTrader(
            second->get_acc_name(), second->getCurCharacName());
        CUser_history_log(second)->SetTrader(
            first->get_acc_name(), first->getCurCharacName());
    }

    first->SetTradeSpace(index_);
    second->SetTradeSpace(index_);
    trade_gold_[0] = 0;
    trade_gold_[1] = 0;
    trade_states_[0] = 0;
    trade_states_[1] = 0;
    std::memset(trade_slots_, 0, sizeof(trade_slots_));
    std::memset(received_creature_item_counts_,
                0,
                sizeof(received_creature_item_counts_));
    unlock();
    CUser_history_log(first)->TradeBegin();
    return CUser_history_log(second)->TradeBegin();
}

// IDA_EA: 0x85297F2
// IDA_NAME: _ZN11CTradeSpace5resetEv
// DEMANGLED: CTradeSpace::reset(void)
void CTradeSpace::reset() {
    lock();
    _SetLock(false);
    occupied_ = 0;

    for (int trader_index = 0; trader_index != 2; ++trader_index) {
        CUser* trader = traders_[trader_index];
        if (trader) {
            trader->SetTradeSpace(-1);
            CUser_history_log(trader)->SetTrader("", "");
        }
    }

    trade_gold_[0] = 0;
    trade_gold_[1] = 0;
    trade_states_[0] = 2;
    trade_states_[1] = 2;
    std::memset(trade_slots_, 0, sizeof(trade_slots_));
    std::memset(received_creature_item_counts_,
                0,
                sizeof(received_creature_item_counts_));
    unlock();
}

// IDA_EA: 0x852993C
// IDA_NAME: _ZN11CTradeSpace23check_trade_possibilityEv
// DEMANGLED: CTradeSpace::check_trade_possibility(void)
int CTradeSpace::check_trade_possibility() {
    if (occupied_ != 1) {
        return 0;
    }

    CUser* first = traders_[0];
    CUser* second = traders_[1];
    const int first_space = first->GetTradeSpace();
    if (first_space != second->GetTradeSpace()) {
        return 0;
    }

    const int first_state = first->get_state();
    return first_state == second->get_state();
}

// The preceding mail and lifecycle ledger is deliberately dormant: those
// bodies already have their canonical providers.  The escrow mutation slice
// below is enabled only after CUser's trade fields were restored as direct
// physical members.
#endif  // non-transaction mail/trade recovery ledger

// IDA_EA: 0x8529DCE
// IDA_NAME: _ZN11CTradeSpace11_IsTradableERK10Inven_Item
// DEMANGLED: CTradeSpace::_IsTradable(Inven_Item const&)
int CTradeSpace::_IsTradable(const Inven_Item& item) {
    const int item_id = item.item_id_;
    CItem* const item_template = G_CDataManager()->find_item(item_id);
    if (!item_template) {
        return 0;
    }

    if (item.upgrade_separate_.IsTradeRestriction()) {
        return 0;
    }

    const unsigned int attach_type =
        static_cast<unsigned int>(item_template->GetAttachType());
    if (attach_type > 5u) {
        return 1;
    }

    const unsigned int attach_mask = 1u << attach_type;
    if ((attach_mask & 0x36u) != 0u) {
        return 0;
    }
    return (attach_mask & 0x08u) == 0u || item.seal_or_state_ == 1u;
}

// IDA_EA: 0x8529E7E
// IDA_NAME: _ZN11CTradeSpace11regist_itemEP5CUseriii
// DEMANGLED: CTradeSpace::regist_item(CUser *,int,int,int)
int CTradeSpace::regist_item(CUser* owner,
                             int item_space,
                             int slot,
                             int item_count) {
    if (IsLocked()) return -99;
    if (item_count <= 0) return -98;

    CUser* opposite = 0;
    int opposite_index = 0;
    int owner_index = 0;
    if (traders_[0] == owner) {
        opposite = traders_[1];
        opposite_index = 1;
        owner_index = 0;
    } else {
        opposite = traders_[0];
        opposite_index = 0;
        owner_index = 1;
    }
    if (!opposite) return -1;

    TradeSlot source;
    source.item_space_ = static_cast<unsigned char>(item_space);
    if (!owner->getCurCharacR()) {
        cMyTrace trace("int CTradeSpace::regist_item(CUser*, int, int, int)",
                       327, 5);
        trace("CTradeSpace::regist_item()\t0 == owner->getCurCharacR()");
        return -2;
    }
    if (item_space && (owner->isHackUser() || opposite->isHackUser()))
        return -339;

    PacketGuard packet;
    if (item_space == 1) {
        CInventory* const inventory = reinterpret_cast<CInventory*>(
            owner->getCurCharacInvenR());
        source.item_ = inventory->GetInvenSlot(2, slot);
        source.original_slot_ = slot;
        source.owner_character_no_ = static_cast<uint32_t>(
            owner->getCurCharacNo());

        const WongWork::CAvatarItemMgr* const avatar_manager =
            reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                inventory->GetAvatarItemMgrR());
        const int expire_date = avatar_manager->GetExpireDate(
            source.item_.get_add_info());
        if (expire_date && GlobalData::system_time()->getCurSec() > expire_date)
            return -100;
        if (WongWork::CAvatarItemMgr::IsTempKey(
                source.item_.get_add_info()))
            return -102;
        if (!_IsTradable(source.item_)) return -102;

        for (int trade_slot = 3; trade_slot <= 26; ++trade_slot) {
            if (trade_slots_[owner_index][trade_slot]
                    .item_.get_add_info() ==
                source.item_.get_add_info())
                return -103;
        }

        CInventory* const writable_inventory = reinterpret_cast<CInventory*>(
            owner->getCurCharacInvenW());
        if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(2),
                                            static_cast<int>(slot), item_count,
                                            static_cast<eItemDelReason>(1),
                                            true) != 1)
            return -10;

        const int trade_slot = add_item(owner_index, source, item_count);
        if (trade_slot == -1) return -1;
        if (!checkTrade() || !checkCancelTrade()) {
            trade_slots_[owner_index][trade_slot].item_.reset();
            const int restored = writable_inventory->insertAvatarIntoSpecificSlot(
                source.item_, static_cast<int>(slot),
                static_cast<eAvatarItemAddReason>(4), true);
            if (restored >= 0) return -12;
            CUser_history_log(owner)->TradeItemAddFail(
                source.item_.item_id_,
                item_count);
            return -11;
        }

        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        Inven_Item& escrow = trade_slots_[owner_index][trade_slot].item_;
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(source.item_.item_id_);
        packet.put_int(avatar_manager->GetRemainDate(
            escrow.get_add_info(), OS_API::GetDateTimeTick()));
        packet.put_byte(source.item_.GetItemAttr());
        packet.put_short(source.item_.avatar_select_);
        packet.put_int(source.item_.extend_info_);
        packet.put_byte(0);
        packet.put_short(0);
        if (escrow.item_category_ == 8) {
            packet.put_int(30);
            packet.put_binary(
                reinterpret_cast<const char*>(
                    avatar_manager->getJewelSocketData(
                        escrow.get_add_info())),
                30);
            packet.put_int(4);
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager->GetExpansionInfo(escrow.get_add_info());
            if (expansion) {
                packet.put_binary(
                    reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                packet.put_binary(
                    reinterpret_cast<const char*>(&empty_expansion), 4);
            }
        }
        packet.put_packet(source.item_);
        packet.finalize(true);
        opposite->Send(packet);
        return trade_slot;
    }

    if (item_space == 0) {
        if (static_cast<unsigned int>(slot) >=
            oldsrv::config::kMainInventoryItemCount)
            return -3;
        CInventory* const writable_inventory = reinterpret_cast<CInventory*>(
            owner->getCurCharacInvenW());
        if (!slot) {
            if (GlobalData::g_autoPunishRuleHackTypeMgr->checkTotalTradeRuleOver(
                    static_cast<unsigned int>(owner->getTotalHackUserTradeCnt()),
                    owner->getTotalHackUserTradeGold()))
                return -362;

            if (!writable_inventory->use_money(
                    item_count, static_cast<eMoneySubReason>(2), true))
                return -5;

            trade_gold_[owner_index] += item_count;
            if (!checkCancelTrade() || !checkTrade()) {
                trade_gold_[owner_index] -= item_count;
                writable_inventory->gain_money(
                    item_count, static_cast<eMoneyAddReason>(12), true, 0);
                return -4;
            }

            packet.clear();
            packet.put_header(0, 15);
            packet.put_short(0);
            packet.put_int(0);
            packet.put_int(trade_gold_[owner_index]);
            packet.put_byte(0);
            packet.put_short(0);
            packet.put_int(0);
            packet.put_byte(0);
            packet.put_short(0);
            packet.put_packet(g_emptySlot);
            packet.finalize(true);
            opposite->Send(packet);
            trade_states_[owner_index] = 0;
            trade_states_[opposite_index] = 0;
            return 0;
        }

        if (owner->isHackUser() || opposite->isHackUser()) return -417;
        const CInventory* const inventory = reinterpret_cast<const CInventory*>(
            owner->getCurCharacInvenR());
        source.item_ = inventory->GetInvenSlot(1, slot);
        if (!_IsTradable(source.item_)) return -102;
        if (source.item_.item_category_ != 1 &&
            source.item_.get_add_info() < item_count)
            return -8;
        if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(1),
                                            static_cast<int>(slot), item_count,
                                            static_cast<eItemDelReason>(2),
                                            true) != 1)
            return -10;

        const int trade_slot = add_item(owner_index, source, item_count);
        if (trade_slot == -1) return -1;
        if (!checkTrade() || !checkCancelTrade()) {
            Inven_Item& escrow = trade_slots_[owner_index][trade_slot].item_;
            if (escrow.isEquipableItemType() ||
                escrow.get_add_info() <= item_count) {
                escrow.reset();
            } else {
                escrow.set_add_info(escrow.get_add_info() - item_count);
            }
            const int restored = writable_inventory->insert_item_special_slot(
                source.item_, item_count, static_cast<int>(slot),
                static_cast<eMoneyAddReason>(9),
                static_cast<eItemAddReason>(16));
            if (restored >= 0) return -12;
            CUser_history_log(owner)->TradeItemAddFail(
                source.item_.item_id_,
                item_count);
            return -11;
        }

        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        Inven_Item& escrow = trade_slots_[owner_index][trade_slot].item_;
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(source.item_.item_id_);
        if (escrow.item_category_ == 8) {
            const WongWork::CAvatarItemMgr* const avatar_manager =
                reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            packet.put_int(avatar_manager->GetRemainDate(
                escrow.get_add_info(), OS_API::GetDateTimeTick()));
        } else {
            packet.put_int(escrow.get_add_info());
        }
        packet.put_byte(source.item_.GetItemAttr());
        packet.put_short(source.item_.avatar_select_);
        packet.put_int(source.item_.extend_info_);
        packet.put_byte(source.item_.amplify_option_.getAbilityType());
        packet.put_short(source.item_.amplify_option_.getAbilityValue());
        if (escrow.item_category_ == 8) {
            const WongWork::CAvatarItemMgr* const avatar_manager =
                reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            packet.put_int(30);
            packet.put_binary(
                reinterpret_cast<const char*>(
                    avatar_manager->getJewelSocketData(
                        escrow.get_add_info())),
                30);
            packet.put_int(4);
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager->GetExpansionInfo(escrow.get_add_info());
            if (expansion) {
                packet.put_binary(
                    reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                packet.put_binary(
                    reinterpret_cast<const char*>(&empty_expansion), 4);
            }
        }
        packet.put_packet(source.item_);
        packet.finalize(true);
        opposite->Send(packet);
        return trade_slot;
    }

    if (item_space == 3) {
        if (static_cast<unsigned int>(slot) >=
            static_cast<unsigned int>(oldsrv::config::kEquipmentTypeCount))
            return -13;
        const CInventory* const inventory = reinterpret_cast<const CInventory*>(
            owner->getCurCharacInvenR());
        CInventory* const writable_inventory = reinterpret_cast<CInventory*>(
            owner->getCurCharacInvenW());
        source.item_ = inventory->GetInvenSlot(0, slot);
        if (!static_cast<unsigned int>(source.item_.item_id_))
            return -14;
        if (!_IsTradable(source.item_)) return -102;
        if (slot <= oldsrv::config::kAvatarEquipmentTypeLast) {
            if (WongWork::CAvatarItemMgr::IsTempKey(
                    source.item_.get_add_info()))
                return -102;
            source.original_slot_ = slot;
            source.owner_character_no_ = static_cast<uint32_t>(
                owner->getCurCharacNo());
            const WongWork::CAvatarItemMgr* const avatar_manager =
                reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            const int expire_date = avatar_manager->GetExpireDate(
                source.item_.get_add_info());
            if (expire_date &&
                GlobalData::system_time()->getCurSec() > expire_date)
                return -100;
        }
        if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(0),
                                            static_cast<int>(slot), 1,
                                            static_cast<eItemDelReason>(2),
                                            true) != 1)
            return -15;

        const int trade_slot = add_item(owner_index, source, item_count);
        if (trade_slot == -1) return -1;
        if (!checkTrade() || !checkCancelTrade()) {
            trade_slots_[owner_index][trade_slot].item_.reset();
            const int restored = writable_inventory->insert_item_special_slot(
                source.item_, item_count, static_cast<int>(slot),
                static_cast<eMoneyAddReason>(9),
                static_cast<eItemAddReason>(16));
            if (restored >= 0) return -17;
            CUser_history_log(owner)->TradeItemAddFail(
                source.item_.item_id_,
                item_count);
            return -16;
        }

        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        Inven_Item& escrow = trade_slots_[owner_index][trade_slot].item_;
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(source.item_.item_id_);
        if (escrow.item_category_ == 8) {
            const WongWork::CAvatarItemMgr* const avatar_manager =
                reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            packet.put_int(avatar_manager->GetRemainDate(
                escrow.get_add_info(), OS_API::GetDateTimeTick()));
        } else {
            packet.put_int(escrow.get_add_info());
        }
        packet.put_byte(source.item_.GetItemAttr());
        packet.put_short(source.item_.avatar_select_);
        packet.put_int(source.item_.extend_info_);
        packet.put_byte(source.item_.amplify_option_.getAbilityType());
        packet.put_short(source.item_.amplify_option_.getAbilityValue());
        if (escrow.item_category_ == 8) {
            const WongWork::CAvatarItemMgr* const avatar_manager =
                reinterpret_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            packet.put_int(30);
            packet.put_binary(
                reinterpret_cast<const char*>(
                    avatar_manager->getJewelSocketData(
                        escrow.get_add_info())),
                30);
            packet.put_int(4);
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager->GetExpansionInfo(escrow.get_add_info());
            if (expansion) {
                packet.put_binary(
                    reinterpret_cast<const char*>(expansion), 4);
            } else {
                stAvatarExpansionInfo_t empty_expansion;
                empty_expansion.init();
                packet.put_binary(
                    reinterpret_cast<const char*>(&empty_expansion), 4);
            }
        }
        packet.put_packet(source.item_);
        packet.finalize(true);
        opposite->Send(packet);
        return trade_slot;
    }

    if (item_space == 7) {
        const CInventory* const inventory = reinterpret_cast<const CInventory*>(
            owner->getCurCharacInvenR());
        CInventory* const writable_inventory = reinterpret_cast<CInventory*>(
            owner->getCurCharacInvenW());
        source.item_ = inventory->GetInvenSlot(3, slot);
        source.original_slot_ = slot;
        source.owner_character_no_ = static_cast<uint32_t>(
            owner->getCurCharacNo());
        if (!_IsTradable(source.item_)) return -102;
        if (writable_inventory->delete_item(static_cast<INVEN_TYPE>(3),
                                            static_cast<int>(slot), item_count,
                                            static_cast<eItemDelReason>(21),
                                            true) != 1)
            return -10;

        const int trade_slot = add_item(owner_index, source, item_count);
        if (trade_slot == -1) return -1;
        if (!checkTrade() || !checkCancelTrade()) {
            trade_slots_[owner_index][trade_slot].item_.reset();
            const int restored = writable_inventory->insertCreatureIntoSpecificSlot(
                source.item_, static_cast<int>(slot),
                static_cast<eItemAddReason>(29), true);
            if (restored >= 0) return -12;
            CUser_history_log(owner)->TradeItemAddFail(
                source.item_.item_id_,
                item_count);
            return -11;
        }

        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        Inven_Item& escrow = trade_slots_[owner_index][trade_slot].item_;
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(source.item_.item_id_);
        packet.put_int(escrow.get_add_info());
        packet.put_byte(source.item_.GetItemAttr());
        packet.put_short(source.item_.avatar_select_);
        packet.put_int(source.item_.extend_info_);
        packet.put_byte(0);
        packet.put_short(0);
        if (source.item_.item_category_ == 5) {
            packet.put_int(inventory->GetCreatureMgrR()->GetRemainDate(
                escrow.get_add_info(), OS_API::GetDateTimeTick()));
        }
        packet.put_packet(source.item_);
        packet.finalize(true);
        opposite->Send(packet);
        return trade_slot;
    }

    return -20;
}

// IDA_EA: 0x852BFCC
// IDA_NAME: _ZN11CTradeSpace11remove_itemEP5CUseriiii
// DEMANGLED: CTradeSpace::remove_item(CUser *,int,int,int,int)
int CTradeSpace::remove_item(CUser* owner,
                             int item_space,
                             int slot,
                             int trade_slot,
                             int item_count) {
    if (IsLocked()) {
        return -1;
    }
    if (item_count <= 0) {
        return -98;
    }

    CUser* opposite = 0;
    int opposite_index = 0;
    int owner_index = 0;
    if (traders_[0] == owner) {
        opposite = traders_[1];
        opposite_index = 1;
    } else {
        opposite = traders_[0];
        opposite_index = 0;
        owner_index = 1;
    }
    trade_states_[owner_index] = 0;
    trade_states_[opposite_index] = 0;

    if (!opposite || trade_slot > 26) {
        return -1;
    }

    CUser* const trader = traders_[owner_index];
    if (!trader->getCurCharacR() ||
        !owner->getCurCharacR()) {
        cMyTrace trace(
            "int CTradeSpace::remove_item(CUser*, int, int, int, int)",
            1114,
            5);
        trace("CTradeSpace::remove_item()\\t0 == m_pTraders[owner_index]->getCurCharacR() || 0 == owner->getCurCharacR()");
        return -1;
    }

    TradeSlot& escrow = trade_slots_[owner_index][trade_slot];
    if (escrow.item_space_ != static_cast<unsigned char>(item_space)) {
        return -1;
    }

    PacketGuard packet;
    if (item_space == 1) {
        TradeSlot source = escrow;
        if (source.item_space_ != 1) {
            return -1;
        }

        CInventory* const inventory = static_cast<CInventory*>(
            trader->getCurCharacInvenW());
        const int returned_slot = inventory->insertAvatarIntoSpecificSlot(
            source.item_,
            slot,
            static_cast<eAvatarItemAddReason>(4),
            true);
        if (returned_slot < 0) {
            return returned_slot;
        }

        escrow.Reset();
        if (static_cast<int>(source.original_slot_) != returned_slot) {
            inventory->SendAvatarEvent(
                50,
                static_cast<int>(source.original_slot_) +
                    oldsrv::config::kAvatarInventorySlotBase,
                returned_slot + oldsrv::config::kAvatarInventorySlotBase,
                source.item_.get_add_info(),
                0);
        }
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(-1);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_packet(g_emptySlot);
        packet.finalize(true);
        opposite->Send(packet);
        return returned_slot;
    }

    if (item_space == 7) {
        TradeSlot source = escrow;
        if (source.item_space_ != 7) {
            return -1;
        }

        CInventory* const inventory = static_cast<CInventory*>(
            trader->getCurCharacInvenW());
        user_creature::CCreatureMgr* const creature_manager =
            static_cast<user_creature::CCreatureMgr*>(
                inventory->GetCreatureMgrW());
        if (source.item_.item_category_ == 5 &&
            !creature_manager->IsCreatureEquipmentScope(slot)) {
            return -1;
        }

        const int returned_slot = inventory->insertCreatureIntoSpecificSlot(
            source.item_,
            slot,
            static_cast<eItemAddReason>(29),
            true);
        if (returned_slot < 0) {
            return returned_slot;
        }

        escrow.Reset();
        if (static_cast<int>(source.original_slot_) == returned_slot) {
            inventory->WriteCreatureLog(
                static_cast<INVEN_TYPE>(3),
                source.item_.item_id_,
                source.item_.get_add_info(),
                1,
                source.item_.item_category_,
                static_cast<eItemAddReason>(29));
        } else {
            creature_manager->ChangeSlotNo(
                &source.item_,
                returned_slot,
                29);
        }
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(-1);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_packet(g_emptySlot);
        packet.finalize(true);
        opposite->Send(packet);
        return returned_slot;
    }

    if (item_space == 3) {
        if (!trade_slot) {
            return -1;
        }

        TradeSlot source = escrow;
        if (source.item_space_ != 3 || !source.item_.item_id_) {
            return -1;
        }
        if (escrow.item_.item_category_ == 8) {
            if (slot > oldsrv::config::kAvatarEquipmentTypeLast) {
                return -1;
            }
        } else if (slot < oldsrv::config::kOrdinaryEquipmentTypeFirst ||
                   slot > oldsrv::config::kOrdinaryEquipmentTypeLast) {
            return -1;
        }

        const CInventory* const readable_inventory =
            static_cast<const CInventory*>(
                owner->getCurCharacInvenR());
        Inven_Item* const destination =
            const_cast<CInventory*>(readable_inventory)->GetInvenRef(0, slot);
        if (destination->item_id_) {
            return -1;
        }

        CItem* const template_item = G_CDataManager()->find_item(
            source.item_.item_id_);
        if (!template_item ||
            reinterpret_cast<CEquipItem*>(template_item)
                ->check_able_To_equip(
                static_cast<char>(slot),
                owner->get_charac_job(),
                owner->get_charac_level())) {
            return -1;
        }

        destination->setCopy(source.item_);
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(trade_slot);
        packet.put_int(-1);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_packet(g_emptySlot);
        packet.finalize(true);
        opposite->Send(packet);

        escrow.Reset();
        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        if (trade_slot <= 11 && owner->get_state() != 5 &&
            owner->get_state() != 8) {
            packet.clear();
            packet.put_header(0, 2);
            packet.put_byte(0);
            packet.put_short(1);
            owner->make_basic_info(packet, 0);
            packet.finalize(true);
            G_GameWorld()->send_all(packet, owner);
        }
        return slot;
    }

    if (item_space != 0) {
        return -1;
    }
    if (slot) {
        return -1;
    }

    if (!trade_slot) {
        if (trade_gold_[owner_index] - item_count < 0) {
            return -1;
        }

        CInventory* const inventory = static_cast<CInventory*>(
            owner->getCurCharacInvenW());
        inventory->gain_money(item_count,
                              static_cast<eMoneyAddReason>(1),
                              true,
                              0);
        trade_gold_[owner_index] -= item_count;
        packet.clear();
        packet.put_header(0, 15);
        packet.put_short(0);
        packet.put_int(trade_gold_[owner_index] <= 0 ? -1 : 0);
        packet.put_int(trade_gold_[owner_index]);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_int(0);
        packet.put_byte(0);
        packet.put_short(0);
        packet.put_packet(g_emptySlot);
        packet.finalize(true);
        opposite->Send(packet);
        trade_states_[owner_index] = 0;
        trade_states_[opposite_index] = 0;
        return 0;
    }

    TradeSlot source = escrow;
    CInventory* const readable_inventory = static_cast<CInventory*>(
        trader->getCurCharacInvenR());
    const int destination_type = readable_inventory->GetSlotItemType(slot);
    if (destination_type &&
        destination_type !=
            static_cast<int>(source.item_.item_category_)) {
        return -1;
    }

    CInventory* const writable_inventory = static_cast<CInventory*>(
        trader->getCurCharacInvenW());
    int returned_slot = -1;
    if (source.item_.item_category_ == 1) {
        if (source.item_space_) {
            return -1;
        }
        if (!readable_inventory->check_empty_count(
                static_cast<Inven_Item::ITEM_TYPE>(
                    source.item_.item_category_),
                1)) {
            return -1;
        }
        returned_slot = writable_inventory->insert_item_special_slot(
            source.item_,
            item_count,
            slot,
            static_cast<eMoneyAddReason>(9),
            static_cast<eItemAddReason>(16));
        if (returned_slot < 0) {
            return returned_slot;
        }
    } else {
        if (source.item_.get_add_info() < item_count) {
            return -1;
        }
        source.item_.set_add_info(item_count);
        const int item_id = source.item_.item_id_;
        if (readable_inventory->check_item_exist(item_id) == -1) {
            if (!readable_inventory->check_empty_count(
                    static_cast<Inven_Item::ITEM_TYPE>(
                        source.item_.item_category_),
                    1)) {
                return -1;
            }
            returned_slot = writable_inventory->insert_item_special_slot(
                source.item_,
                item_count,
                slot,
                static_cast<eMoneyAddReason>(9),
                static_cast<eItemAddReason>(16));
        } else {
            returned_slot = writable_inventory->insertItemIntoInventory(
                source.item_,
                static_cast<eItemAddReason>(16),
                true,
                true);
        }
        if (returned_slot < 0) {
            return returned_slot;
        }
    }

    trade_states_[owner_index] = 0;
    trade_states_[opposite_index] = 0;
    packet.clear();
    packet.put_header(0, 15);
    packet.put_short(trade_slot);
    if (escrow.item_.item_category_ == 1 ||
        escrow.item_.get_add_info() <= item_count) {
        packet.put_int(-1);
        packet.put_int(0);
        escrow.Reset();
    } else {
        packet.put_int(source.item_.item_id_);
        packet.put_int(escrow.item_.get_add_info() - item_count);
        escrow.item_.set_add_info(
            escrow.item_.get_add_info() - item_count);
    }
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_int(0);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_packet(g_emptySlot);
    packet.finalize(true);
    opposite->Send(packet);
    return returned_slot;
}


// IDA_EA: 0x853087A
// IDA_NAME: _ZN11CTradeSpace13proceed_tradeEv
// DEMANGLED: CTradeSpace::proceed_trade(void)
int CTradeSpace::proceed_trade() {
    if (IsLocked() != 1 || checkTrade() != 1) {
        return 0;
    }

    // IDA initializes both arrays before the security and settlement passes.
    int offered_average_price[2] = {0, 0};
    unsigned int offered_gold_for_history[2] = {0, 0};
    PacketGuard packets[2];
    std::map<CUser*, std::vector<int> > changed_slots;

    // This first pass is deliberately performed before any money or item
    // mutation: Hades receives the original offered money and the opposite
    // side's avatar ids, exactly as the old fraud-detection ordering.
    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const owner = traders_[trader_index];
        CUser* const opposite = traders_[1 - trader_index];
        char hades_ip[16] = {};
        opposite->getHades()->GetIP(hades_ip);

        CInventory* const owner_inventory = static_cast<CInventory*>(
            owner->getCurCharacInvenR());
        const unsigned int owner_money = static_cast<unsigned int>(
            owner_inventory->get_money());
        void* const account_hack = owner->getAcccountHackInfo();
        reinterpret_cast<Secu_AccountHacking*>(account_hack)->sendGold(
                                     0,
                                     static_cast<unsigned int>(
                                         opposite->get_acc_id()),
                                     static_cast<unsigned int>(
                                         trade_gold_[trader_index]),
                                     owner_money,
                                     0,
                                     hades_ip);

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const int item_id =
                trade_slots_[1 - trader_index][slot_index].item_.item_id_;
            if (reinterpret_cast<Secu_AccountHacking*>(account_hack)->isAvatar(static_cast<unsigned int>(item_id))) {
                reinterpret_cast<Secu_AccountHacking*>(account_hack)->sendAvatar(
                    0,
                    static_cast<unsigned int>(opposite->get_acc_id()));
            }
        }
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const owner = traders_[trader_index];
        CUser* const opposite = traders_[1 - trader_index];
        PacketGuard& packet = packets[trader_index];
        packet.clear();
        packet.put_header(0, 18);

        int entry_count = 0;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            if (trade_slots_[trader_index][slot_index]
                    .item_.item_id_ != 0) {
                ++entry_count;
            }
        }

        if (trade_gold_[trader_index] <= 0) {
            packet.put_short(entry_count);
        } else {
            packet.put_short(++entry_count);
            packet.put_short(0);
            packet.put_short(0);

            if (opposite) {
                CInventory* const opposite_inventory =
                    static_cast<CInventory*>(
                        opposite->getCurCharacInvenW());
                opposite_inventory->gain_money(
                    trade_gold_[trader_index],
                    static_cast<eMoneyAddReason>(1),
                    true,
                    0);
            }

            if (trade_gold_[trader_index] % 10000) {
                const unsigned int etc = static_cast<unsigned int>(opposite->SetETC(
                    708,
                    owner->get_acc_id()));
                reinterpret_cast<WongWork::CHackAnalyzer*>((opposite->getHackAnalyzer()))->addServerHackCnt(
                                                        opposite,
                                                        static_cast<WongWork::ENUM_HACKTYPE>(0x2c4u),
                                                        1,
                                                        etc,
                                                        0);
            }

            if ((owner->isHackUser() || owner->isHackUserTimer()) &&
                trade_gold_[trader_index]) {
                opposite->gainMoneyFromHackUser(
                    static_cast<unsigned int>(owner->get_acc_id()),
                    static_cast<unsigned long long>(trade_gold_[trader_index]),
                    1u,
                    0);
            }

            if (owner && opposite && owner->getCurCharacMoney() == 0) {
                // B owns the signal layout/dispatcher.  C writes the exact
                // source/destination connection fields used by the old game
                // state transition and queues it to the existing LogDB lane.
                CStreamGuard guard(
                    GlobalData::stream_pool()->Acquire("item_trade.cpp", 3153),
                    true);
                guard << 161;
                guard << -1;
                SIG_SAVE_GOLD_ZERO_TRADE_LOG* const payload =
                    guard.GetInBuffer<SIG_SAVE_GOLD_ZERO_TRADE_LOG>();
                CNetwork<4096, 450000>* const recipient_network =
                    &opposite->network_;
                CNetwork<4096, 450000>* const source_network =
                    &owner->network_;

                payload->to_m_id = static_cast<unsigned int>(
                    opposite->get_acc_id());
                std::strncpy(payload->to_nat_ip,
                             recipient_network->get_str_ip(),
                             sizeof(payload->to_nat_ip));
                recipient_network->GetPeerIP2(payload->to_peer_ip,
                                              sizeof(payload->to_peer_ip));
                payload->to_nat_type = static_cast<uint8_t>(
                    recipient_network->get_nat_type());
                std::strncpy(payload->to_mac,
                             recipient_network->get_mac_addr(),
                             sizeof(payload->to_mac));
                payload->from_m_id = static_cast<unsigned int>(
                    owner->get_acc_id());
                std::strncpy(payload->from_nat_ip,
                             source_network->get_str_ip(),
                             sizeof(payload->from_nat_ip));
                source_network->GetPeerIP2(payload->from_peer_ip,
                                           sizeof(payload->from_peer_ip));
                payload->from_nat_type = static_cast<uint8_t>(
                    source_network->get_nat_type());
                std::strncpy(payload->from_mac,
                             source_network->get_mac_addr(),
                             sizeof(payload->from_mac));
                payload->injury_money = trade_gold_[trader_index];
                GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
            }

            // The original writes zero before these two history messages;
            // retain that ordering rather than logging the offered value.
            trade_gold_[trader_index] = 0;
            HistoryLog::WriteTradeUp(owner->history_stream(),
                                    0,
                                    trade_gold_[trader_index]);
            HistoryLog::WriteTradeDown(opposite->history_stream(),
                                      0,
                                      trade_gold_[trader_index]);
        }

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            TradeSlot& trade_slot = trade_slots_[trader_index][slot_index];
            Inven_Item& item = trade_slot.item_;
            const int item_id = item.item_id_;
            if (!opposite || !item_id) {
                continue;
            }

            CAuctionAveragePrice* const price_manager =
                (G_CGameManager())->GetAuctionAveragePriceManager();
            if (price_manager) {
                int average_price = price_manager->GetAveragePrice(
                    item_id, static_cast<unsigned char>(item.GetUpgrade()));
                if (item.item_category_ == 2u || item.item_category_ == 3u) {
                    int item_count = 0;
                    item_count = item.add_info_;
                    average_price *= item_count;
                }
                offered_average_price[trader_index] += average_price;
            }

            int destination_slot = -1;
            CInventory* const opposite_inventory = static_cast<CInventory*>(
                opposite->getCurCharacInvenW());
            if (item.item_category_ == 8u) {
                destination_slot = opposite_inventory->insertAvatarIntoInventory(
                    item,
                    static_cast<eAvatarItemAddReason>(1),
                    false);
                if (destination_slot >= 0) {
                    const int avatar_ui_id = item.get_add_info();
                    const unsigned int destination_charac_no =
                        static_cast<unsigned int>(
                            opposite->getCurCharacNo());
                    _SaveAvatarItemOwnerChange(
                        static_cast<unsigned int>(avatar_ui_id),
                        trade_slot.owner_character_no_,
                        destination_charac_no,
                        static_cast<unsigned char>(destination_slot));
                    DB_InsertCeraItemHistory::makeRequest(
                        avatar_ui_id,
                        trade_slot.owner_character_no_,
                        destination_charac_no,
                        2);

                    WongWork::CAvatarItemMgr* const source_avatar_manager =
                        reinterpret_cast<WongWork::CAvatarItemMgr*>(
                            static_cast<CInventory*>(owner->getCurCharacInvenW())
                                ->GetAvatarItemMgrW());
                    WongWork::Avatar_Item* const avatar_item =
                        source_avatar_manager->EraseAvatarItem(avatar_ui_id);
                    WongWork::CAvatarItemMgr* const destination_avatar_manager =
                        reinterpret_cast<WongWork::CAvatarItemMgr*>(
                            opposite_inventory->GetAvatarItemMgrW());
                    destination_avatar_manager->AddAvatarItem(avatar_ui_id,
                                                              avatar_item);
                    const char* const ipg_no =
                        static_cast<const WongWork::Avatar_Item*>(avatar_item)
                            ->GetIPGAgencyNo();
                    CUser_history_log(owner)->AvatarItemDel(
                        item_id,
                        avatar_ui_id,
                        ipg_no,
                        static_cast<eAvatarItemDelReason>(0));
                    CUser_history_log(opposite)->AvatarItemAdd(
                        item_id,
                        avatar_ui_id,
                        ipg_no,
                        static_cast<eAvatarItemAddReason>(1));
                }
            } else if (trade_slot.item_space_ == 7u) {
                destination_slot = opposite_inventory->insertItemIntoInventory(
                    item,
                    static_cast<eItemAddReason>(26),
                    false,
                    true);
                if (destination_slot >= 0 && item.item_category_ == 5u) {
                    CInventory* const owner_inventory =
                        static_cast<CInventory*>(
                            owner->getCurCharacInvenW());
                    user_creature::CCreatureMgr* const owner_creature_manager =
                        static_cast<user_creature::CCreatureMgr*>(
                            owner_inventory->GetCreatureMgrW());
                    const int creature_key = item.get_add_info();
                    if (!owner_creature_manager->FindCreatureItem(creature_key)) {
                        return 0;
                    }
                    const int old_charac_no = *reinterpret_cast<const int*>(
                        owner->getCurCharacR());
                    const int old_creature_flag =
                        received_creature_item_counts_[trader_index] +
                        owner_creature_manager->GetCreatureItemSize();
                    user_creature::CCreatureMgr* const opposite_creature_manager =
                        static_cast<user_creature::CCreatureMgr*>(
                            opposite_inventory->GetCreatureMgrW());
                    opposite_creature_manager->ChangeOwner(
                        &item,
                        destination_slot,
                        26,
                        old_charac_no,
                        old_creature_flag);
                    ++received_creature_item_counts_[1 - trader_index];
                    owner_creature_manager->DeleteCreatureItem(creature_key,
                                                                slot_index);
                    CUser_history_log(owner)->CreatureItemDel(
                        static_cast<INVEN_TYPE>(3),
                        item_id,
                        creature_key,
                        0,
                        item.item_category_,
                        static_cast<eItemDelReason>(20));
                }
            } else {
                if (IsTradeLimitAttachTypeItem(&item)) {
                    DecreaseTradeLimitCount(&item);
                    CUser_history_log(owner)->ChangeItemLimitCount(
                        1,
                        slot_index,
                        item_id,
                        -1,
                        2);
                }
                destination_slot = opposite_inventory->insertItemIntoInventory(
                    item,
                    static_cast<eItemAddReason>(1),
                    true,
                    true);
                changed_slots[opposite].push_back(destination_slot);
            }

            if (destination_slot < 0) {
                cMyTrace trace("bool CTradeSpace::proceed_trade()", 3301, 5);
                trace("insert_item(INVEN_TYPE_INVENTORY) fail - fatal! %d",
                      opposite->getCurCharacNo());
                item.reset();
                return 0;
            }

            const int avatar_ui_id = item.get_add_info();
            HistoryLog::WriteTradeUp(owner->history_stream(),
                                    item_id,
                                    avatar_ui_id);
            HistoryLog::WriteTradeDown(opposite->history_stream(),
                                      item_id,
                                      avatar_ui_id);
            item.reset();
            packet.put_short(slot_index);
            packet.put_short(destination_slot);
        }
        packet.finalize(true);
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const recipient = traders_[1 - trader_index];
        recipient->Send(packets[trader_index]);
        recipient->incTradeCount();
    }

    for (std::map<CUser*, std::vector<int> >::const_iterator it =
             changed_slots.begin();
         it != changed_slots.end();
         ++it) {
        CUser* const user = it->first;
        if (!user) {
            return 0;
        }
        for (std::vector<int>::const_iterator slot = it->second.begin();
             slot != it->second.end();
             ++slot) {
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     *slot);
        }
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const owner = traders_[trader_index];
        CUser_history_log(owner)->TradeEnd(static_cast<eTradeEndReason>(2),
                                           offered_average_price[trader_index],
                                           offered_gold_for_history[trader_index],
                                           offered_average_price[1 - trader_index],
                                           offered_gold_for_history[1 - trader_index]);
        (&owner->gold_control_)->TradeComplete(
            offered_gold_for_history[trader_index],
            offered_gold_for_history[1 - trader_index]);
    }
    return 1;
}

// IDA_EA: 0x853084A
// IDA_NAME: _ZN11CTradeSpace26_SaveAvatarItemOwnerChangeEjjjh
// DEMANGLED: CTradeSpace::_SaveAvatarItemOwnerChange(unsigned int,unsigned int,unsigned int,unsigned char)
void CTradeSpace::_SaveAvatarItemOwnerChange(unsigned int avatar_ui_id,
                                             unsigned int old_charac_no,
                                             unsigned int new_charac_no,
                                             unsigned char slot) {
    // IDA treats DB_AvatarChangeOwner::makeRequest as a four-argument static
    // request writer. B's current public declaration is non-static, so keep
    // the receiver (which the body does not read) and pass the same UI id as
    // the actual first payload argument required by the old symbol.
    reinterpret_cast<DB_AvatarChangeOwner*>(avatar_ui_id)->makeRequest(
        avatar_ui_id, old_charac_no, new_charac_no, slot);
}

// The preflight is the original CTradeSpace::checkCancelTrade body.  It is
// enabled with regist_item/remove_item because those operations invoke it
// before mutating escrow; retaining it dormant would leave their rollback
// contract incomplete.
// IDA_EA: 0x852DBB8
// IDA_NAME: _ZN11CTradeSpace16checkCancelTradeEv
// DEMANGLED: CTradeSpace::checkCancelTrade(void)
int CTradeSpace::checkCancelTrade() {
    if (!(traders_[0])->getCurCharacR() ||
        !(traders_[1])->getCurCharacR()) {
        cMyTrace trace("bool CTradeSpace::checkCancelTrade()", 1652, 5);
        trace("CTradeSpace::cancel_trade()\t0 == m_pTraders[0]->getCurCharacR() || m_pTraders[1]->getCurCharacR()");
        return 0;
    }

    CInventory preview_inventories[2];
    InventoryMemory preview_memory[2];
    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        preview_inventories[trader_index].SetInventoryMemory(
            &preview_memory[trader_index]);
        const CInventory* const source =
            traders_[trader_index]->getCurCharacInvenRefR();
        preview_inventories[trader_index].setCopy(*source);
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        int rollback_entry_count = 0;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const TradeSlot& slot = trade_slots_[trader_index][slot_index];
            if (slot.item_.item_id_) {
                ++rollback_entry_count;
            }
        }
        if (trade_gold_[trader_index] > 0) {
            ++rollback_entry_count;
            if (traders_[trader_index]) {
                preview_inventories[trader_index].gain_money(
                    trade_gold_[trader_index],
                    static_cast<eMoneyAddReason>(2),
                    false,
                    0);
            }
        }

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            TradeSlot& slot = trade_slots_[trader_index][slot_index];
            if (!slot.item_.item_id_ ||
                !traders_[trader_index]) {
                continue;
            }

            Inven_Item& item = slot.item_;
            int inserted = -1;
            if (slot.item_space_) {
                if (slot.item_space_ == 1) {
                    inserted = preview_inventories[trader_index]
                        .insertAvatarIntoInventory(
                            item,
                            static_cast<eAvatarItemAddReason>(3),
                            false);
                    if (inserted < 0) {
                        cMyTrace trace("bool CTradeSpace::checkCancelTrade()",
                                       1721,
                                       5);
                        trace("try insert_item(ENUM_ITEMSPACE_AVATAR) fail - [Fail]Insert_item - fatal! canceled ch=%d",
                              (traders_[trader_index])->getCurCharacNo());
                        return 0;
                    }
                } else if (slot.item_space_ == 7) {
                    inserted = preview_inventories[trader_index]
                        .insertItemIntoCreature(item,
                                                static_cast<eItemAddReason>(28),
                                                false,
                                                false);
                    if (inserted < 0) {
                        cMyTrace trace("bool CTradeSpace::checkCancelTrade()",
                                       1731,
                                       5);
                        trace("try insert_item(ENUM_ITEMSPACE_CREATURE) fail - [Fail]Insert_item - fatal! canceled ch=%d",
                              (traders_[trader_index])->getCurCharacNo());
                        return 0;
                    }
                } else {
                    inserted = preview_inventories[trader_index]
                        .insertItemIntoEquipment(item,
                                                 static_cast<eItemAddReason>(2),
                                                 false,
                                                 false);
                    if (inserted < 0) {
                        cMyTrace trace("bool CTradeSpace::checkCancelTrade()",
                                       1752,
                                       5);
                        trace("try insert_item(INVEN_TYPE_EQUIP) fail - [Fail]Insert_item - fatal! canceled ch=%d",
                              (traders_[trader_index])->getCurCharacNo());
                        return 0;
                    }
                }
            } else {
                inserted = preview_inventories[trader_index]
                    .insertItemIntoInventory(item,
                                             static_cast<eItemAddReason>(2),
                                             false,
                                             true);
                if (inserted < 0) {
                    cMyTrace trace("bool CTradeSpace::checkCancelTrade()",
                                   1711,
                                   5);
                    trace("try insert_item(INVEN_TYPE_INVENTORY) fail - [Fail]Insert_item - fatal! canceled ch=%d",
                          (traders_[trader_index])->getCurCharacNo());
                    return 0;
                }
            }
        }

        // The original counts this for the rollback preflight before each
        // simulated insertion.  Its value is intentionally not a return
        // payload; all observable results are the actual insertion outcomes.
        (void)rollback_entry_count;
    }
    return 1;
}

// End of CTradeSpace::checkCancelTrade.

// IDA_EA: 0x8530210
// IDA_NAME: _ZN11CTradeSpace10checkTradeEv
// DEMANGLED: CTradeSpace::checkTrade(void)
int CTradeSpace::checkTrade() {
    CInventory preview_inventories[2];
    InventoryMemory preview_memory[2];
    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        preview_inventories[trader_index].SetInventoryMemory(
            &preview_memory[trader_index]);
        const CInventory* const source =
            traders_[trader_index]->getCurCharacInvenRefR();
        preview_inventories[trader_index].setCopy(*source);
    }

    for (int owner_index = 0; owner_index <= 1; ++owner_index) {
        int transfer_entry_count = 0;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            const TradeSlot& slot = trade_slots_[owner_index][slot_index];
            if (slot.item_.item_id_) {
                ++transfer_entry_count;
            }
        }

        const int receiver_index = 1 - owner_index;
        if (trade_gold_[owner_index] > 0) {
            ++transfer_entry_count;
            if (traders_[receiver_index] &&
                preview_inventories[receiver_index].gain_money(
                    trade_gold_[owner_index],
                    static_cast<eMoneyAddReason>(1),
                    false,
                    0) != trade_gold_[owner_index]) {
                return 0;
            }
        }

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            TradeSlot& slot = trade_slots_[owner_index][slot_index];
            if (!traders_[receiver_index] ||
                !slot.item_.item_id_) {
                continue;
            }

            Inven_Item& item = slot.item_;
            int inserted = -1;
            if (item.item_category_ == 8u) {
                inserted = preview_inventories[receiver_index]
                    .insertAvatarIntoInventory(
                        item,
                        static_cast<eAvatarItemAddReason>(1),
                        false);
            } else if (slot.item_space_ == 7u) {
                inserted = preview_inventories[receiver_index]
                    .insertItemIntoCreature(item,
                                            static_cast<eItemAddReason>(26),
                                            false,
                                            false);
            } else {
                // Trade recipients receive the original non-avatar/non-
                // creature item through inventory, including equipment.
                inserted = preview_inventories[receiver_index]
                    .insertItemIntoInventory(item,
                                             static_cast<eItemAddReason>(1),
                                             false,
                                             true);
            }

            if (inserted < 0) {
                if (item.item_category_ == 8u) {
                    cMyTrace trace("bool CTradeSpace::checkTrade()", 2983, 5);
                    trace("try insert_item(INVEN_TYPE_AVATAR) fail - fatal! ch=%d",
                          (traders_[receiver_index])->getCurCharacNo());
                } else {
                    cMyTrace trace("bool CTradeSpace::checkTrade()", 2987, 5);
                    trace("try insert_item(INVEN_TYPE_INVENTORY) fail - fatal! ch=%d",
                          (traders_[receiver_index])->getCurCharacNo());
                }
                return 0;
            }
        }

        (void)transfer_entry_count;
    }
    return 1;
}

#if 0
// IDA_EA: 0x852E3A8
// IDA_NAME: _ZN11CTradeSpace24send_lose_item_for_chinaEP5CUserR10Inven_Item
// DEMANGLED: CTradeSpace::send_lose_item_for_china(CUser *,Inven_Item &)
int CTradeSpace::send_lose_item_for_china(CUser* user, Inven_Item& item) {
    if (!item.item_id_) {
        return 0;
    }

    char title[0x1d] = {};
    char text[0xff] = {};
    std::strncpy(title,
                 g_scriptStringManager_.findString(4,
                                                    "game_server_msg_287",
                                                    0),
                 sizeof(title));
    std::strncpy(text,
                 g_scriptStringManager_.findString(4,
                                                    "game_server_msg_288",
                                                    0),
                 sizeof(text));

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        user->getCurCharacNo(),
        text,
        std::strlen(text),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
    return 1;
}

// IDA_EA: 0x852E544
// IDA_NAME: _ZN11CTradeSpace22cancel_trade_for_chinaEP5CUser
// DEMANGLED: CTradeSpace::cancel_trade_for_china(CUser *)
int CTradeSpace::cancel_trade_for_china(CUser* disconnected_user) {
    PacketGuard packets[2];

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        int entry_count = 0;
        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            if (trade_slots_[trader_index][slot_index].item_.item_id_) {
                ++entry_count;
            }
        }

        PacketGuard& packet = packets[trader_index];
        packet.clear();
        packet.put_header(0, 16);

        int applied_count = 0;
        int count_index = 0;
        if (trade_gold_[trader_index] <= 0) {
            count_index = packet.get_index();
            packet.put_short(entry_count);
        } else {
            ++entry_count;
            CUser* const trader = traders_[trader_index];
            if (trader) {
                if (!trader->getCurCharacR()) {
                    cMyTrace trace(
                        "bool CTradeSpace::cancel_trade_for_china(CUser*)",
                        1820,
                        5);
                    trace("CTradeSpace::cancel_trade()\tm_pTraders[user_index]->getCurCharacR()");
                    return 0;
                }

                CInventory* const inventory = static_cast<CInventory*>(
                    trader->getCurCharacInvenW());
                inventory->gain_money(trade_gold_[trader_index],
                                      static_cast<eMoneyAddReason>(2),
                                      true,
                                      0);
                trade_gold_[trader_index] = 0;
                ++applied_count;
            }
            count_index = packet.get_index();
            packet.put_short(entry_count);
            packet.put_short(0);
            packet.put_byte(0);
            packet.put_short(0);
        }

        for (int slot_index = 3; slot_index <= 26; ++slot_index) {
            TradeSlot& trade_slot = trade_slots_[trader_index][slot_index];
            CUser* const trader = traders_[trader_index];
            if (!trader || !trader->getCurCharacR() ||
                !trade_slot.item_.item_id_) {
                continue;
            }

            packet.put_short(slot_index);
            const uint8_t trade_type = trade_slot.item_space_;
            Inven_Item& item = trade_slot.item_;
            CInventory* const inventory = static_cast<CInventory*>(
                trader->getCurCharacInvenW());
            int empty_slot = -1;

            if (trade_type == 1) {
                empty_slot = inventory->insertAvatarIntoInventory(
                    item,
                    static_cast<eAvatarItemAddReason>(3),
                    true);
                if (empty_slot >= 0 &&
                    static_cast<int>(trade_slot.original_slot_) != empty_slot) {
                    const int item_value = item.get_add_info();
                    const CInventory* const readable_inventory =
                        static_cast<const CInventory*>(
                            trader->getCurCharacInvenR());
                    readable_inventory->SendAvatarEvent(
                        50,
                        static_cast<int>(trade_slot.original_slot_) +
                            oldsrv::config::kAvatarInventorySlotBase,
                        empty_slot +
                            oldsrv::config::kAvatarInventorySlotBase,
                        item_value,
                        0);
                }
                packet.put_byte(1);
            } else if (trade_type == 7) {
                empty_slot = inventory->insertItemIntoCreature(
                    item,
                    static_cast<eItemAddReason>(28),
                    true,
                    true);
                if (empty_slot >= 0 &&
                    static_cast<int>(trade_slot.original_slot_) != empty_slot) {
                    user_creature::CCreatureMgr* const creature_manager =
                        static_cast<user_creature::CCreatureMgr*>(
                            inventory->GetCreatureMgrW());
                    creature_manager->ChangeSlotNo(&item, empty_slot, 28);
                }
                packet.put_byte(7);
            } else if (trade_type) {
                empty_slot = inventory->insertItemIntoEquipment(
                    item,
                    static_cast<eItemAddReason>(2),
                    true,
                    true);
                packet.put_byte(3);
            } else {
                empty_slot = inventory->insertItemIntoInventory(
                    item,
                    static_cast<eItemAddReason>(2),
                    true,
                    true);
                packet.put_byte(0);
            }

            if (empty_slot < 0) {
                send_lose_item_for_china(trader, item);
            }
            item.reset();
            packet.put_short(empty_slot);
            ++applied_count;
        }

        packet.put_short(count_index, applied_count);
        packet.finalize(true);
    }

    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const trader = traders_[trader_index];
        if (trader && trader != disconnected_user) {
            trader->Send(packets[trader_index]);
            trader->send_equip(0);
        }
    }
    for (int trader_index = 0; trader_index <= 1; ++trader_index) {
        CUser* const trader = traders_[trader_index];
        if (trader) {
            CUser_history_log(trader)->TradeEnd(
                static_cast<eTradeEndReason>(0), 0, 0, 0, 0);
        }
    }
    return 1;
}

// IDA_EA: 0x852EFFA
// IDA_NAME: _ZN11CTradeSpace12cancel_tradeEv
// DEMANGLED: CTradeSpace::cancel_trade(void)
int CTradeSpace::cancel_trade() {
    if ((traders_[0])->getCurCharacR() &&
        (traders_[1])->getCurCharacR()) {
        return cancel_trade_for_china(0);
    }

    cMyTrace trace("bool CTradeSpace::cancel_trade()", 1928, 5);
    trace("CTradeSpace::cancel_trade()\\t0 == m_pTraders[0]->getCurCharacR() || m_pTraders[1]->getCurCharacR()");
    return 0;
}

// IDA_EA: 0x852F086
// IDA_NAME: _ZN11CTradeSpace19cancel_trade_by_disEP5CUser
// DEMANGLED: CTradeSpace::cancel_trade_by_dis(CUser *)
int CTradeSpace::cancel_trade_by_dis(CUser* disconnected_user) {
    return cancel_trade_for_china(disconnected_user);
}

// The cancellation ledger above has canonical C providers.  Enable only the
// escrow-slot helpers required by regist_item/remove_item below.
#endif  // cancellation recovery ledger

// IDA_EA: 0x85299C0
// IDA_NAME: _ZN11CTradeSpace18get_empty_itemslotEi
// DEMANGLED: CTradeSpace::get_empty_itemslot(int)
int CTradeSpace::get_empty_itemslot(int trader_index) {
    for (int slot_index = 3; slot_index <= 26; ++slot_index) {
        if (!trade_slots_[trader_index][slot_index].item_.item_id_) {
            return slot_index;
        }
    }
    return -1;
}

// IDA_EA: 0x8529A18
// IDA_NAME: _ZN11CTradeSpace16check_item_existEii
// DEMANGLED: CTradeSpace::check_item_exist(int,int)
int CTradeSpace::check_item_exist(int trader_index, int item_key) {
    for (int slot_index = 3; slot_index <= 26; ++slot_index) {
        if (trade_slots_[trader_index][slot_index].item_.item_id_ ==
            item_key) {
            return slot_index;
        }
    }
    return -1;
}

// IDA_EA: 0x8529AA2
// IDA_NAME: _ZN11CTradeSpace8add_itemEi9TradeSloti
// DEMANGLED: CTradeSpace::add_item(int,TradeSlot,int)
int CTradeSpace::add_item(int trader_index,
                          TradeSlot source_slot,
                          int add_info) {
    if (add_info < 0) {
        return -1;
    }

    int empty_slot = -1;
    lock();
    if (source_slot.item_.isEquipableItemType()) {
        empty_slot = get_empty_itemslot(trader_index);
        if (empty_slot != -1 &&
            &trade_slots_[trader_index][empty_slot] != &source_slot) {
            trade_slots_[trader_index][empty_slot] = source_slot;
        }
    } else {
        const int item_id = source_slot.item_.item_id_;
        empty_slot = check_item_exist(trader_index, item_id);
        if (empty_slot == -1) {
            empty_slot = get_empty_itemslot(trader_index);
            if (empty_slot != -1) {
                TradeSlot& target = trade_slots_[trader_index][empty_slot];
                if (&target != &source_slot) {
                    target = source_slot;
                }
                target.item_.set_add_info(add_info);
                if (add_info > 999) {
                    cMyTrace trace(
                        "int CTradeSpace::add_item(int, TradeSlot, int)",
                        222,
                        5);
                    trace("CTradeSpace::add_item, ITEM #%d, %d",
                          target.item_.item_id_,
                          add_info);
                }
            }
        } else {
            TradeSlot& target = trade_slots_[trader_index][empty_slot];
            const int merged_add_info = add_info +
                target.item_.add_info_;
            target.item_.set_add_info(merged_add_info);
            if (merged_add_info > 999) {
                cMyTrace trace(
                    "int CTradeSpace::add_item(int, TradeSlot, int)",
                    209,
                    5);
                trace("CTradeSpace::add_item, ITEM #%d, %d",
                      target.item_.item_id_,
                      merged_add_info);
            }
        }
    }
    unlock();
    return empty_slot;
}

// GetOppositeUser is provided by trade_space_lifecycle_runtime.cpp.
#if 0
// IDA_EA: 0x8529A72
// IDA_NAME: _ZN11CTradeSpace15GetOppositeUserEP5CUser
// DEMANGLED: CTradeSpace::GetOppositeUser(CUser *)
CUser* CTradeSpace::GetOppositeUser(CUser* user) {
    if (!occupied_) {
        return 0;
    }
    if (traders_[0] == user) {
        return traders_[1];
    }
    return traders_[0];
}

#endif  // lifecycle duplicate

// IDA_EA: 0x8529D48
// IDA_NAME: _ZN11CTradeSpace22_CheckTrade4SameAvatarEPK9TradeSlotRK10Inven_Item
// DEMANGLED: CTradeSpace::_CheckTrade4SameAvatar(TradeSlot const*,Inven_Item const&)
int CTradeSpace::_CheckTrade4SameAvatar(const TradeSlot* slots,
                                        const Inven_Item& item) {
    for (int slot_index = 3; slot_index <= 26; ++slot_index) {
        const Inven_Item& candidate = slots[slot_index].item_;
        if (candidate.item_id_ == item.item_id_ &&
            candidate.add_info_ == item.add_info_ &&
            candidate.avatar_select_ == item.avatar_select_) {
            return 1;
        }
    }
    return 0;
}

// Mutex and lock-state methods are provided by
// trade_space_lifecycle_runtime.cpp.
#if 0
// IDA_EA: 0x85320E2
// IDA_NAME: _ZN11CTradeSpace4lockEv
// DEMANGLED: CTradeSpace::lock(void)
void CTradeSpace::lock() {}

// IDA_EA: 0x85320E8
// IDA_NAME: _ZN11CTradeSpace6unlockEv
// DEMANGLED: CTradeSpace::unlock(void)
void CTradeSpace::unlock() {}

// IDA_EA: 0x85320EE
// IDA_NAME: _ZN11CTradeSpace8IsLockedEv
// DEMANGLED: CTradeSpace::IsLocked(void)
int CTradeSpace::IsLocked() {
    return locked_;
}

// IDA_EA: 0x85320FA
// IDA_NAME: _ZN11CTradeSpace8_SetLockEb
// DEMANGLED: CTradeSpace::_SetLock(bool)
CTradeSpace* CTradeSpace::_SetLock(bool locked) {
    locked_ = locked;
    return this;
}
#endif  // lifecycle duplicate
