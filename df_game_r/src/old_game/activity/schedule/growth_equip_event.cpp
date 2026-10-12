#include "old_game/activity/schedule/growth_equip_event.h"

#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_event_save_growth_weapon.h"
#include "old_data/db/event/db_event_update_growth_weapon_charac.h"
#include "old_data/db/event/db_event_update_growth_weapon_timepiece.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"


namespace {

class SendGrowthWeaponEventInfoToUser {
public:
    explicit SendGrowthWeaponEventInfoToUser(GrowthEquipEvent& event)
        : event_(event) {}

    void operator()(CUser* user) const {
        if (user->get_state() > 2) {
            event_.SendGrowthWeaponEventInfo(*user);
        }
    }

private:
    GrowthEquipEvent& event_;
};

}  // namespace


// IDA_EA: 0x80F6EF0
// IDA_NAME: _ZN16GrowthEquipEventC2Ev
// DEMANGLED: GrowthEquipEvent::GrowthEquipEvent(void)
GrowthEquipEvent::GrowthEquipEvent()
    : CEventBase() {}


// IDA_EA: 0x80F6F0C (D2), 0x80F6F3C (D0)
// IDA_NAME: _ZN16GrowthEquipEventD2Ev / _ZN16GrowthEquipEventD0Ev
// DEMANGLED: GrowthEquipEvent::~GrowthEquipEvent()
GrowthEquipEvent::~GrowthEquipEvent() {}


// IDA_EA: 0x80F6F5A
// IDA_NAME: _ZN16GrowthEquipEvent10StartEventEv
// DEMANGLED: GrowthEquipEvent::StartEvent(void)
void GrowthEquipEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void GrowthEquipEvent::StartEvent()", 24, 0);
    trace("GrowthEquipEvent Event start!");
}


// IDA_EA: 0x80F6FFE
// IDA_NAME: _ZN16GrowthEquipEvent8EndEventEv
// DEMANGLED: GrowthEquipEvent::EndEvent(void)
void GrowthEquipEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void GrowthEquipEvent::EndEvent()", 36, 0);
    trace("GrowthEquipEvent Event end!");
}


// IDA_EA: 0x80F6FAC
// IDA_NAME: _ZN16GrowthEquipEvent10StartEventE10Word_Param
// DEMANGLED: GrowthEquipEvent::StartEvent(Word_Param)
void GrowthEquipEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("virtual void GrowthEquipEvent::StartEvent(Word_Param)",
                   30,
                   0);
    trace("GrowthEquipEvent Event start! [monitor request]");
}


// IDA_EA: 0x80F7710
// IDA_NAME: _ZN16GrowthEquipEvent11StartActionEv
// DEMANGLED: GrowthEquipEvent::StartAction(void)
void GrowthEquipEvent::StartAction() {
    // 0x80F7710 walks GameWorld's canonical +0x134 user map in key order and
    // sends the event payload to users after state 2.  ProcessAllUsers is the
    // typed reconstruction of that exact map walk.
    G_GameWorld()->ProcessAllUsers(SendGrowthWeaponEventInfoToUser(*this));
}


// IDA_EA: 0x80F7820
// IDA_NAME: _ZNK16GrowthEquipEvent10IsEventingEP5CUser
// DEMANGLED: GrowthEquipEvent::IsEventing(CUser *)const
int GrowthEquipEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user);
}


// IDA_EA: 0x80F7050
// IDA_NAME: _ZNK16GrowthEquipEvent19GiftGrowthWeaponBoxER5CUser
// DEMANGLED: GrowthEquipEvent::GiftGrowthWeaponBox(CUser &)const
int GrowthEquipEvent::GiftGrowthWeaponBox(CUser& user) const {
    char* const event_data = user.getGrowthWeaponEventdata();
    int result = static_cast<unsigned char>(*event_data) ^ 1;
    if (*event_data == 1) {
        return result;
    }

    *event_data = 1;
    const int item_id = G_CDataManager()->get_GiftGrowthWeaponBox();
    Inven_Item item;
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user.GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    result = server_group;
    if (!prototype) {
        return result;
    }

    prototype->vtable_->make_item(prototype, item);
    item.set_add_info(1);

    char text[256] = {};
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_278", 0),
                 255u);
    if (item_id != 0) {
        const char* const title = g_scriptStringManager_.findString(
            4, "game_server_msg_277", 0);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<unsigned int>(user.getCurCharacNo()),
            text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(server_group),
            false,
            false);
    }

    DB_EventSaveGrowthWeapon::makeRequest(
        static_cast<unsigned int>(user.get_acc_id()),
        static_cast<short>(server_group));
    user.SendEventMailAlaramTooltip();
    return 0;
}


// IDA_EA: 0x80F74B2
// IDA_NAME: _ZNK16GrowthEquipEvent24SendInfinityGrowthWeaponEP5CUserPK10Inven_Itemii
// DEMANGLED: GrowthEquipEvent::SendInfinityGrowthWeapon(CUser *,Inven_Item const*,int,int)const
int GrowthEquipEvent::SendInfinityGrowthWeapon(CUser* user,
                                               const Inven_Item* source,
                                               int item_space,
                                               int slot) const {
    const int reward_item_id =
        G_CDataManager()->get_GrowthWeaponEventReward(source->item_id_);
    if (reward_item_id == 0) {
        return 23;
    }

    CItem* const reward_item = G_CDataManager()->find_item(reward_item_id);
    if (!reward_item) {
        // The old body assumes the PVF reward entry is resolvable after the
        // nonzero script lookup.  Keep a deterministic item-rule failure in
        // the reconstructed server instead of updating the slot with the
        // otherwise uninitialized automatic Inven_Item.
        return 23;
    }

    Inven_Item replacement;
    replacement.item_id_ = reward_item_id;
    reward_item->vtable_->make_item(reward_item, replacement);
    replacement.item_id_ = reward_item_id;
    replacement.SetUpgrade(static_cast<uint8_t>(source->GetUpgrade()));
    replacement.set_add_info(source->get_add_info());

    // 0x80F74B2 copies the original item extension/amplify payload and then
    // applies only the five-bit separate-upgrade value to the fresh reward
    // record.  The source trade-restriction bit is intentionally not copied.
    replacement.extend_info_ = source->extend_info_;
    replacement.amplify_option_ = source->amplify_option_;
    replacement.upgrade_separate_.SetUpgradeSeparate(
        static_cast<uint8_t>(source->upgrade_separate_.GetUpgradeSeparate()));

    const INVEN_TYPE inventory_type =
        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(item_space)));
    CInventory* const inventory = user->getCurCharacInvenW();
    if (inventory->delete_item(inventory_type,
                               slot,
                               1,
                               static_cast<eItemDelReason>(54),
                               true) != 1) {
        return 17;
    }

    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(item_space),
                         slot);
    inventory->update_item(inventory_type, slot, replacement);
    user->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_ITEMSPACE>(item_space),
                         slot);

    const short timepiece = static_cast<short>(user->growth_weapon_event_.timepiece_);
    DB_EventUpdateGrowthWeaponTimepiece::makeRequest(
        user->get_acc_id(), timepiece, replacement.item_id_);
    user->growth_weapon_event_.infinity_weapon_id_ =
        static_cast<uint32_t>(replacement.item_id_);
    return 0;
}

// IDA_EA: 0x80F7226
// IDA_NAME: _ZN16GrowthEquipEvent25SendGrowthWeaponEventInfoER5CUser
// DEMANGLED: GrowthEquipEvent::SendGrowthWeaponEventInfo(CUser &)
void GrowthEquipEvent::SendGrowthWeaponEventInfo(CUser& user) {
    PacketGuard packet;
    packet.put_header(0, 441);

    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user.GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    if (user.growth_weapon_event_.infinity_weapon_id_ != 0) {
        packet.put_byte(2);
    } else if (user.get_charac_no(-1) ==
                   static_cast<int>(user.growth_weapon_event_.charac_no_) &&
               user.growth_weapon_event_.server_group_ ==
                   static_cast<uint16_t>(server_group)) {
        packet.put_byte(1);
    } else {
        packet.put_byte(0);
    }
    packet.put_int(user.growth_weapon_event_.event_value_);
    packet.put_short(user.growth_weapon_event_.timepiece_);
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x80F73BE
// IDA_NAME: _ZN16GrowthEquipEvent18SelectGrowthWeaponER5CUserii
// DEMANGLED: GrowthEquipEvent::SelectGrowthWeapon(CUser &,int,int)
void GrowthEquipEvent::SelectGrowthWeapon(CUser& user,
                                          int source_item_id,
                                          int result_item_id) {
    if (G_CDataManager()->get_GiftGrowthWeaponBox() != source_item_id) {
        return;
    }

    user.growth_weapon_event_.charac_no_ =
        static_cast<uint32_t>(user.get_charac_no(-1));
    user.growth_weapon_event_.event_value_ =
        static_cast<uint32_t>(result_item_id);
    const int server_group = G_GameWorld()->IsIntegratedPvPBaseChannel()
                                 ? user.GetServerGroup()
                                 : G_CEnvironment()->get_server_group();
    user.growth_weapon_event_.server_group_ =
        static_cast<uint16_t>(server_group);

    DB_EventUpdateGrowthWeaponCharac::makeRequest(
        static_cast<unsigned int>(user.get_acc_id()),
        static_cast<unsigned int>(user.get_charac_no(-1)),
        result_item_id,
        static_cast<short>(server_group));
    SendGrowthWeaponEventInfo(user);
}
