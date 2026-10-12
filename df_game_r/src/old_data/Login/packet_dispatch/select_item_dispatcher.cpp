#include "old_data/Login/packet_dispatch/select_item_dispatcher.h"

#include <cstring>
#include <vector>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/pvp_mission/fair_pvp_equipment_script.h"
#include "old_game/pvp/battlefield/fair_pvp_score_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

void MakeFairPvpSelectionItem(const CItem* prototype, Inven_Item& item) {
    // Dispatcher_SelectItem @0x8229D4A dispatches CItem vptr+0x08, the
    // original make_item slot.  Keep that virtual ABI call rather than
    // guessing an equipment/stackable branch from an item id.
    const CItemVTable* vtable = 0;
    std::memcpy(&vtable, prototype, sizeof(vtable));
    vtable->make_item(prototype, item);
}

}  // namespace

// IDA_EA: 0x85977DE
// IDA_NAME: _ZN21Dispatcher_SelectItemC2Ev
// DEMANGLED: Dispatcher_SelectItem::Dispatcher_SelectItem(void)
Dispatcher_SelectItem::Dispatcher_SelectItem() {}

// IDA_EA: 0x8240000
// IDA_NAME: _ZN21Dispatcher_SelectItemD2Ev
// DEMANGLED: Dispatcher_SelectItem::~Dispatcher_SelectItem()
// IDA_EA: 0x8240030
// IDA_NAME: _ZN21Dispatcher_SelectItemD0Ev
// DEMANGLED: Dispatcher_SelectItem::~Dispatcher_SelectItem()
Dispatcher_SelectItem::~Dispatcher_SelectItem() {}

// IDA_EA: 0x822997A
// IDA_NAME: _ZN21Dispatcher_SelectItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SelectItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SelectItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return 0;
    }

    const int channel_type = G_GameWorld()->GetChannelType();
    if (channel_type != 15 && channel_type != 16) {
        return 0;
    }

    fair_pvp::CFairPvPScore* const fair_pvp_score =
        static_cast<fair_pvp::CFairPvPScore*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(7)));
    const int pvp_grade = user->get_pvp_grade();
    if (fair_pvp_score->GetGiveItem() >
        G_CDataManager()->get_server_parameter_script()->get_pvp_item_give(
            pvp_grade)) {
        return 0;
    }

    uint8_t selection_count = 0;
    uint8_t selection_index = 0;
    if (packet.get_byte(selection_count) != 1) {
        return LineFunc(59269,
                        "virtual int Dispatcher_SelectItem::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
    }

    const FairPvpEquipmentMap& equipment_packages =
        G_CDataManager()->get_fair_pvp_equipment_map();
    int package_key = makeFairPVPEquipmentPackageKey(
        user->get_charac_job(), pvp_grade);
    FairPvpEquipmentMap::const_iterator package_it = equipment_packages.end();

    // The old loop probes the current PvP grade and then exactly one lower
    // grade per unsuccessful lookup, never below grade zero.
    for (int grade_try = 0; grade_try <= user->get_pvp_grade(); ++grade_try) {
        package_it = equipment_packages.find(package_key);
        if (package_it != equipment_packages.end()) {
            break;
        }
        package_key = decreaseRankOfFairPVPEquipmentPackageKey(package_key);
    }

    if (package_it == equipment_packages.end() ||
        package_it->second.size() != selection_count) {
        return 0;
    }

    std::vector<Inven_Item> selected_items;
    for (FairPvpEquipmentPackageList::const_iterator group_it =
             package_it->second.begin();
         group_it != package_it->second.end();
         ++group_it) {
        if (packet.get_byte(selection_index) != 1) {
            return LineFunc(59297,
                            "virtual int Dispatcher_SelectItem::dispatch_sig(CUser*, PacketBuf&)",
                            0,
                            0);
        }
        if (group_it->size() <= selection_index) {
            return LineFunc(59300,
                            "virtual int Dispatcher_SelectItem::dispatch_sig(CUser*, PacketBuf&)",
                            0,
                            0);
        }

        const STStackablePackageData& chosen = group_it->at(selection_index);
        Inven_Item item;
        // The original stack aliases make this look like a temporary local;
        // disassembly at 0x8229CFB writes the selected id to Inven_Item+0x02
        // before calling CItem::make_item through vptr+0x08.
        item.item_id_ = chosen.item_index;
        CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
        if (!prototype) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SELECT_ITEM, 17);
            return 0;
        }

        MakeFairPvpSelectionItem(prototype, item);
        // Inven_Item+0x01 == 1 is equipment.  Every other original category
        // receives the [package selection] count at Inven_Item+0x07.
        if (item.item_category_ != 1) {
            item.add_info_ = chosen.item_count;
        }
        selected_items.push_back(item);
    }

    for (std::vector<Inven_Item>::const_iterator item_it =
             selected_items.begin();
         item_it != selected_items.end();
         ++item_it) {
        const int slot = user->getCurCharacInvenW()->insertItemIntoInventory(
            *item_it, static_cast<eItemAddReason>(8), true, true);
        if (slot >= 0) {
            user->SendUpdateItemList(
                static_cast<CUser::eSendTarget>(1),
                static_cast<ENUM_ITEMSPACE>(0),
                slot);
            continue;
        }

        char title[21] = {};
        char text[256] = {};
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_171", 0),
                     0x14u);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_172", 0),
                     0xffu);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            *item_it,
            0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
            false,
            false);
    }

    fair_pvp_score->SetGiveItem(user->get_pvp_grade() + 1);
    user->SendCmdOkPacket(ENUM_CMDPACKET_SELECT_ITEM);
    return 0;
}

// IDA_EA: 0x8234352
// IDA_NAME: _ZN21Dispatcher_SelectItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SelectItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SelectItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823435C
// IDA_NAME: _ZN21Dispatcher_SelectItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SelectItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SelectItem::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// Deliberately not registered in the root 606-slot PacketDispatcher here.
// Root dispatcher construction/teardown and full table ownership remain a
// separate closure; exposing this handler early would create an incomplete
// runtime registration graph.
