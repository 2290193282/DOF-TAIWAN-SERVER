#include "old_data/PVF/equipment/datamanager/broadcast_item_script_data.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/stackable/arad_gift_item_set.h"
#include "old_game/activity/schedule/growth_equip_event.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/packet_dispatcher_use_booster_item.h"
#include "old_game/activity/schedule/freely_give_item.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <map>
#include <set>
#include <utility>
#include <vector>

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_UseBoosterItem::dispatch_sig(CUser*, PacketBuf&)";
const int kBoosterItemType = 16;
const int kSelectionBoosterItemType = 34;
const int kAlwaysUsableBoosterItem = 2675649;
const int kUpgradeJarBoosterItem = 2675123;
const int kGrowthWeaponEventId = 100;

}  // namespace

// IDA_EA: 0x8596428
// IDA_NAME: _ZN25Dispatcher_UseBoosterItemC2Ev
// DEMANGLED: Dispatcher_UseBoosterItem::Dispatcher_UseBoosterItem(void)
Dispatcher_UseBoosterItem::Dispatcher_UseBoosterItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824230A
// IDA_NAME: _ZN25Dispatcher_UseBoosterItemD2Ev
// DEMANGLED: Dispatcher_UseBoosterItem::~Dispatcher_UseBoosterItem()
// IDA_EA: 0x824233A
// IDA_NAME: _ZN25Dispatcher_UseBoosterItemD0Ev
// DEMANGLED: Dispatcher_UseBoosterItem::~Dispatcher_UseBoosterItem()
Dispatcher_UseBoosterItem::~Dispatcher_UseBoosterItem() {}

// IDA_EA: 0x8208D40
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem12dispatch_sigEP5CUserR9PacketBuf
int Dispatcher_UseBoosterItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    short slot = -1;
    if (packet.get_short(slot) != 1) {
        return LineFunc(42038, kDispatchSignature, 0, 0);
    }

    if (user->CheckInTrade() ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 19);
        return 0;
    }
    if (!user->getCurCharacR()) {
        return 42046;
    }

    const Inven_Item source = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), slot);
    CItem* const prototype = G_CDataManager()->find_item(source.item_id_);
    if (!prototype) {
        return LineFunc(42064, kDispatchSignature, 0, 0);
    }
    const int item_type = prototype->vtable_->get_item_type(prototype);
    if (item_type != kBoosterItemType &&
        item_type != kSelectionBoosterItemType) {
        return LineFunc(42064, kDispatchSignature, 0, 0);
    }
    if (source.item_id_ != kAlwaysUsableBoosterItem && user->get_state() != 3) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 19);
        return 0;
    }

    CStackableItem* const booster = static_cast<CStackableItem*>(prototype);
    if (!booster->check_change_job_type(user->getCurCharFirstGrowType(),
                                        user->get_charac_job())) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 23);
        return 0;
    }

    std::vector<Inven_Item> selected_items;
    std::vector<Inven_Item> requested_selected_items;
    std::map<int, uint8_t> avatar_options;
    const std::vector<stStackableBoosterItemInfo_t>* selection_candidates = 0;
    bool selected_avatar_has_selectable_abilities = false;
    char first_category = 0;
    char second_category = 0;
    if (item_type == kSelectionBoosterItemType) {
        if (packet.get_byte(first_category) != 1) {
            return LineFunc(42099, kDispatchSignature, 0, 0);
        }
        if (packet.get_byte(second_category) != 1) {
            return LineFunc(42101, kDispatchSignature, 0, 0);
        }

        const std::pair<char, char> category(first_category, second_category);
        const std::map<std::pair<char, char>,
                       std::vector<stStackableBoosterItemInfo_t> >& categories =
            booster->booster_data().category_items;
        const std::map<std::pair<char, char>,
                       std::vector<stStackableBoosterItemInfo_t> >::const_iterator
            found = categories.find(category);
        if (found == categories.end() || found->second.empty()) {
            return LineFunc(42113, kDispatchSignature, 0, 0);
        }
        selection_candidates = &found->second;

        int select_count = booster->booster_select_info().value_08;
        if (select_count == 0) {
            select_count = static_cast<int>(selection_candidates->size());
        }
        if (select_count <= 0 || select_count > 20 ||
            static_cast<std::size_t>(select_count) >
                selection_candidates->size()) {
            return LineFunc(42113, kDispatchSignature, 0, 0);
        }

        std::set<int> selected_ids;
        for (int select_index = 0; select_index < select_count; ++select_index) {
            Inven_Item selected_item;
            int selected_item_id = 0;
            if (packet.get_int(selected_item_id) != 1) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }
            bool candidate_found = false;
            for (std::vector<stStackableBoosterItemInfo_t>::const_iterator
                     candidate = selection_candidates->begin();
                 candidate != selection_candidates->end();
                 ++candidate) {
                if (candidate->item_index == selected_item_id) {
                    candidate_found = true;
                    break;
                }
            }
            if (!candidate_found || !selected_ids.insert(selected_item_id).second) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }
            selected_item.item_id_ = selected_item_id;
            selected_item.set_add_info(1);
            selected_items.push_back(selected_item);
        }
    }

    // The three ordinary 140828 booster senders finalize immediately after
    // the u16 slot.  The transport decryptor nevertheless exposes the whole
    // 16-byte cipher block, so bytes after the semantic body are transport
    // padding and must not be interpreted as a synthetic business field.
    // Selection boosters still parse and validate every PVF-derived semantic
    // field below; only the bytes after that proven grammar belong to crypto.
    // Ordinary semantic processing is already complete after the slot.
    if (selection_candidates) {
        unsigned char avatar_option_count = 0;
        if (packet.get_byte(avatar_option_count) != 1) {
            return LineFunc(42124, kDispatchSignature, 0, 0);
        }

        // The 0828 client filters the chosen category to the selected reward
        // IDs before building its option vector.  It opens the option dialog
        // only when a selected type 0..10 avatar owns selectable abilities;
        // otherwise it writes A=0 (including SAO aura box 10088940).
        std::set<int> selected_avatar_ids;
        for (std::vector<Inven_Item>::const_iterator selected =
                 selected_items.begin();
             selected != selected_items.end();
             ++selected) {
            CItem* const selected_reward =
                G_CDataManager()->find_item(selected->item_id_);
            if (!selected_reward) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }
            if (selected_reward->vtable_->is_avatar_item(selected_reward)) {
                if (!selected_avatar_ids.insert(selected->item_id_).second) {
                    return LineFunc(42124, kDispatchSignature, 0, 0);
                }
                const CEquipItem* const avatar_item =
                    static_cast<const CEquipItem*>(selected_reward);
                const int avatar_type =
                    selected_reward->vtable_->get_item_type(selected_reward);
                if (avatar_type >= 0 && avatar_type <= 10 &&
                    !avatar_item->avatar_ability_variations_.empty()) {
                    selected_avatar_has_selectable_abilities = true;
                }
            }
        }
        const std::size_t expected_avatar_option_count =
            selected_avatar_has_selectable_abilities
                ? selected_avatar_ids.size()
                : 0u;
        if (avatar_option_count > 20u ||
            avatar_option_count != expected_avatar_option_count) {
            return LineFunc(42124, kDispatchSignature, 0, 0);
        }
        for (unsigned int option_index = 0;
             option_index < avatar_option_count;
             ++option_index) {
            int reward_item_id = 0;
            unsigned char ability = 0;
            if (packet.get_int(reward_item_id) != 1 ||
                packet.get_byte(ability) != 1 ||
                selected_avatar_ids.find(reward_item_id) ==
                    selected_avatar_ids.end() ||
                !avatar_options.insert(std::make_pair(reward_item_id, ability))
                     .second) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }

            const stStackableBoosterItemInfo_t* candidate_info = 0;
            for (std::vector<stStackableBoosterItemInfo_t>::const_iterator
                     candidate = selection_candidates->begin();
                 candidate != selection_candidates->end();
                 ++candidate) {
                if (candidate->item_index == reward_item_id) {
                    candidate_info = &*candidate;
                    break;
                }
            }
            CItem* const reward_item =
                G_CDataManager()->find_item(reward_item_id);
            if (!candidate_info || !reward_item ||
                !reward_item->vtable_->is_avatar_item(reward_item)) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }

            const CEquipItem* const avatar_item =
                static_cast<const CEquipItem*>(reward_item);
            const int avatar_type =
                reward_item->vtable_->get_item_type(reward_item);
            const bool ability_valid =
                ability == 0xffu
                    ? (avatar_type == 9 || avatar_type == 10)
                    : avatar_item->IsAvatarOptionValid(
                          static_cast<char>(ability));
            if (!ability_valid) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }

            for (std::vector<std::pair<int, int> >::const_iterator forced =
                     candidate_info->forced_avatar_options.begin();
                 forced != candidate_info->forced_avatar_options.end();
                 ++forced) {
                if (forced->first != reward_item_id) continue;

                // The 0828 client uses only the first matching pair.  A
                // negative value is its explicit "not forced" sentinel and
                // leaves the ordinary avatar-option validation above in
                // control; it is not a malformed option.
                if (forced->second >= 0 &&
                    (forced->second > 0xff ||
                     ability != static_cast<unsigned char>(forced->second))) {
                    return LineFunc(42124, kDispatchSignature, 0, 0);
                }
                break;
            }
        }

        // The selection semantic body is complete here.  The decryptor retains
        // cipher-block alignment, so an opaque transport tail is not a second
        // selection grammar and is deliberately left to the transport owner.
    }

    if (selection_candidates) {
        for (std::vector<Inven_Item>::const_iterator selected =
                 selected_items.begin();
             selected != selected_items.end();
             ++selected) {
            CItem* const reward_item =
                G_CDataManager()->find_item(selected->item_id_);
            if (!reward_item) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }
            if (selected_avatar_has_selectable_abilities &&
                reward_item->vtable_->is_avatar_item(reward_item) &&
                avatar_options.find(selected->item_id_) == avatar_options.end()) {
                return LineFunc(42124, kDispatchSignature, 0, 0);
            }
        }

        int special_item_type = 0;
        int special_value = 0;
        bool unique = false;
        if (GlobalData::s_pCeraShop->IsSpecialItem(source.item_id_,
                                                   special_item_type,
                                                   special_value,
                                                   unique) != 1 ||
            special_item_type != 12) {
            return LineFunc(42124, kDispatchSignature, 0, 0);
        }
        requested_selected_items = selected_items;
    }

    if (!GlobalInstance<Arad_GiftItem_Set>::inst()->level_check(user, booster)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 19);
        return 0;
    }

    const int protection_result = GlobalData::s_pSecuProtectionField->CheckItem(
        user, static_cast<SECURITY_PROTCTION>(42), prototype);
    if (protection_result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM,
                                 static_cast<unsigned char>(protection_result));
        return 0;
    }
    if (prototype->getUsableLevel() > user->get_charac_level()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 19);
        return 0;
    }
    if (!user->getCurCharacInvenW()->check_empty_count(
            static_cast<Inven_Item::ITEM_TYPE>(5), 1)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_BOOSTER_ITEM, 203);
        return 0;
    }
    if (user->CheckCoolTimeItem(static_cast<unsigned long>(source.item_id_))) {
        return 0;
    }
    if (booster->IsDeletableItem() &&
        !user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            slot,
            1,
            static_cast<eItemDelReason>(3),
            true)) {
        return LineFunc(42195, kDispatchSignature, 0, 0);
    }

    user->_processContinuousItem(static_cast<unsigned int>(source.item_id_),
                                 booster);
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(0),
                         slot);

    int restrict_code = 0;
    int processing_result = 0;
    WongWork::CCeraShop::stCeraShopItemParam_t item_param;
    const long current_seconds = GlobalData::s_systemTime_.getCurSec();
    const int special_item_result =
        item_type == kSelectionBoosterItemType
            ? GlobalData::s_pCeraShop->_processBoosterSelectWithAvatarOptions(
                  user,
                  source.item_id_,
                  restrict_code,
                  processing_result,
                  current_seconds,
                  true,
                  selected_items,
                  item_param,
                  first_category,
                  second_category,
                  avatar_options)
            : GlobalData::s_pCeraShop->ProcessSpecialItem(user,
                                                          source.item_id_,
                                                          restrict_code,
                                                          processing_result,
                                                          current_seconds,
                                                          true,
                                                          selected_items,
                                                          item_param,
                                                          first_category,
                                                          second_category,
                                                          false,
                                                          1);
    if (!special_item_result) {
        return LineFunc(42230, kDispatchSignature, 0, 0);
    }
    if (processing_result != 0) {
        return 0;
    }

    if (item_type == kSelectionBoosterItemType) {
        user->history_log().UseSelectBooster(
            static_cast<unsigned int>(source.item_id_),
            first_category,
            second_category,
            requested_selected_items);
    }

    if (restrict_code != 0 && !user->IsRestrictedGoods(restrict_code)) {
        user->SetRestrictedGoods(restrict_code);
        GlobalData::s_pCeraShop->SaveFeaturedIdx(
            user, restrict_code, current_seconds, 0);
        user->SendRestrictedGoods(true, restrict_code);
    }

    _onBoosterItemUseResult(user, selected_items, source.item_id_);
    _onBoosterItemUseResult_OnlyOne(user, selected_items, source.item_id_);
    if (selected_items.size() == 1u) {
        freely_give_item::OnBoosterSelectionResult(
            user,
            static_cast<uint32_t>(source.item_id_),
            static_cast<uint32_t>(selected_items.front().item_id_));
    }
    if (source.item_id_ == kUpgradeJarBoosterItem) {
        user->SendEventJarOutputItemStatistics(selected_items);
    }

    const Inven_Item source_after = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), slot);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_USE_BOOSTER_ITEM);
    response.put_byte(1);
    response.put_short(slot);
    response.put_int(source_after.add_info_);
    response.put_short(static_cast<int>(selected_items.size()));
    for (std::vector<Inven_Item>::const_iterator item = selected_items.begin();
         item != selected_items.end();
         ++item) {
        response.put_int(item->item_id_);
        response.put_int(item->add_info_);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82099BC
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem23_onBoosterItemUseResultEP5CUserRSt6vectorI10Inven_ItemSaIS3_EEi
// DEMANGLED: Dispatcher_UseBoosterItem::_onBoosterItemUseResult(CUser *,std::vector<Inven_Item,std::allocator<Inven_Item>> &,int)
bool Dispatcher_UseBoosterItem::_onBoosterItemUseResult(
    CUser* user,
    std::vector<Inven_Item>& output_items,
    int source_item_id) {
    for (std::vector<Inven_Item>::const_iterator item = output_items.begin();
         item != output_items.end();
         ++item) {
        sendBroadCastItems(user, *item, source_item_id);
    }
    return false;
}

// IDA_EA: 0x8209A5E
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem31_onBoosterItemUseResult_OnlyOneEP5CUserRSt6vectorI10Inven_ItemSaIS3_EEi
// DEMANGLED: Dispatcher_UseBoosterItem::_onBoosterItemUseResult_OnlyOne(CUser *,std::vector<Inven_Item,std::allocator<Inven_Item>> &,int)
void Dispatcher_UseBoosterItem::_onBoosterItemUseResult_OnlyOne(
    CUser* user,
    std::vector<Inven_Item>& output_items,
    int source_item_id) {
    if (output_items.size() != 1u) {
        return;
    }
    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(kGrowthWeaponEventId);
    if (repeat_event->IsEventing(0)) {
        static_cast<GrowthEquipEvent*>(repeat_event)->SelectGrowthWeapon(
            *user, source_item_id, output_items.front().item_id_);
    }
}

// IDA_EA: 0x8209B0E
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem18sendBroadCastItemsEP5CUserRK10Inven_Itemi
// DEMANGLED: Dispatcher_UseBoosterItem::sendBroadCastItems(CUser *,Inven_Item const&,int)
void Dispatcher_UseBoosterItem::sendBroadCastItems(CUser* user,
                                                    const Inven_Item& item,
                                                    int source_item_id) {
    if (!user) {
        return;
    }
    const stBroadCastItemScript* const broadcast_script =
        G_CDataManager()->get_broadcast_item_script();
    const bool should_broadcast =
        broadcast_script->isUnCommonItem(source_item_id)
            ? broadcast_script->checkUnCommonNestedItem(source_item_id,
                                                         item.item_id_)
            : broadcast_script->checkCommonItem(item.item_id_);
    if (!should_broadcast) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 86);
    packet.put_byte(2);
    packet.put_byte(1);
    packet.put_short(user->get_unique_id());
    packet.put_int(item.item_id_);
    packet.put_byte(item.GetUpgrade());
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x8233A2E
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem4readER9PacketBufR8MSG_BASE
int Dispatcher_UseBoosterItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A38
// IDA_NAME: _ZN25Dispatcher_UseBoosterItem7processEP5CUserR8MSG_BASER9ParamBase
int Dispatcher_UseBoosterItem::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
