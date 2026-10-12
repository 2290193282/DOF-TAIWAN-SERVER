#include "old_data/Login/packet_dispatch/select_item_growth_power_dispatcher.h"

#include <cstring>
#include <vector>

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kGrowthPowerReadSignature[] =
    "virtual int Dispatcher_Select_Item_Grwoth_Power::read(PacketBuf&, MSG_BASE&)";
const char kGrowthPowerProcessSignature[] =
    "virtual int Dispatcher_Select_Item_Grwoth_Power::process(CUser*, MSG_BASE&, ParamBase&)";

void MakeGrowthPowerRewardItem(const CItem* prototype, Inven_Item& item) {
    // process @0x81DB93B invokes CItem vptr+0x08.  This is the old
    // make_item virtual slot; the handler must not infer an item subtype.
    const CItemVTable* vtable = 0;
    std::memcpy(&vtable, prototype, sizeof(vtable));
    vtable->make_item(prototype, item);
}

}  // namespace

// CMD432 owns the one concrete MSG_GROWTH_POWER/state-3 template provider.
// It is deliberately not inserted into the still-incomplete 606-slot root.
// IDA_EA: 0x824E522
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824E67E
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824E688
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825BDBE / 0x825BDEE
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EED0Ev
// IDA_EA: 0x825D660
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598D86
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_GROWTH_POWER18stGrowthPowerParamL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_GROWTH_POWER,
                                 stGrowthPowerParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x85978F6
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_PowerC2Ev
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::Dispatcher_Select_Item_Grwoth_Power(void)
Dispatcher_Select_Item_Grwoth_Power::Dispatcher_Select_Item_Grwoth_Power()
    : IPacketDispatcher<MSG_GROWTH_POWER,
                        stGrowthPowerParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x824496E
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_PowerD2Ev
// IDA_EA: 0x824499E
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_PowerD0Ev
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::~Dispatcher_Select_Item_Grwoth_Power()
Dispatcher_Select_Item_Grwoth_Power::~Dispatcher_Select_Item_Grwoth_Power() {}

// IDA_EA: 0x81DB4C4
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Select_Item_Grwoth_Power::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    MSG_GROWTH_POWER& request = static_cast<MSG_GROWTH_POWER&>(message);
    if (packet.get_byte(request.selection_count) != 1) {
        return LineFunc(19029, kGrowthPowerReadSignature, 0, 0);
    }
    for (int index = 0; index < request.selection_count; ++index) {
        if (packet.get_byte(request.reward_selections[index]) != 1) {
            return LineFunc(19034, kGrowthPowerReadSignature, 0, 0);
        }
    }
    return 0;
}

// IDA_EA: 0x81DB580
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Select_Item_Grwoth_Power::check_error(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    (void)message;
    (void)param;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }

    STGrowthPowerData* const growth_power =
        G_CDataManager()->get_growth_power_data();
    return user->get_growth_power_reward() <
                   growth_power->get_break_away_section_low_level(
                       user->get_charac_level())
               ? 0
               : -1;
}

// IDA_EA: 0x81DB60C
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Select_Item_Grwoth_Power::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    stGrowthPowerParam& response = static_cast<stGrowthPowerParam&>(param);
    if (!oldsrv::policy::kGrowthPowerEquipmentRewardEnabled) {
        // Do not silently accept a stale or forged selector request after the
        // server has stopped advertising the equipment reward.
        response.result = 17;
        return 0;
    }

    const int error = check_error(user, message, param);
    if (error > 0) {
        return error;
    }
    if (error < 0) {
        return -1;
    }

    const MSG_GROWTH_POWER& request =
        static_cast<const MSG_GROWTH_POWER&>(message);
    STGrowthPowerData* const growth_power =
        G_CDataManager()->get_growth_power_data();
    const STBreakAwaySection section =
        growth_power->get_break_away_section_data(user->get_charac_level());
    const int package_key = makeFairPVPEquipmentPackageKey(
        user->get_charac_job(), static_cast<int>(section.index));
    const GrowthPowerPackageRewardMap& package_rewards =
        growth_power->package_rewards;
    GrowthPowerPackageRewardMap::const_iterator package_it =
        package_rewards.find(package_key);
    if (package_it == package_rewards.end()) {
        // The raw handler returns success here, so send() records the current
        // level even when this server PVF has no package for the section.
        return 0;
    }

    std::vector<Inven_Item> selected_items;
    int group_index = 0;
    for (std::vector<std::vector<STStackablePackageData> >::const_iterator
             group_it = package_it->second.begin();
         group_it != package_it->second.end();
         ++group_it, ++group_index) {
        const uint8_t selection = request.reward_selections[group_index];
        if (group_it->size() <= selection) {
            cMyTrace trace(kGrowthPowerProcessSignature, 19083, 0);
            trace("[GrowthPower](%d)(j:%d)(l:%d)(s:%d)(p:%d)",
                  user->get_acc_id(),
                  user->get_charac_job(),
                  user->get_charac_level(),
                  static_cast<int>(group_it->size()),
                  static_cast<int>(selection));
            return LineFunc(19084,
                            kGrowthPowerProcessSignature,
                            0,
                            static_cast<unsigned int>(user->get_acc_id()));
        }
        if (group_index > 4) {
            return LineFunc(19087,
                            kGrowthPowerProcessSignature,
                            0,
                            static_cast<unsigned int>(user->get_acc_id()));
        }

        const STStackablePackageData& chosen = group_it->at(selection);
        Inven_Item item;
        // process @0x81DB8FD writes the chosen id at Inven_Item+0x02 before
        // the CItem vptr+0x08 make_item call.
        item.item_id_ = chosen.item_index;
        CItem* const prototype = G_CDataManager()->find_item(item.item_id_);
        if (!prototype) {
            response.result = 17;
            return 0;
        }

        MakeGrowthPowerRewardItem(prototype, item);
        // The old body writes the package count at Inven_Item+0x07 only for
        // categories other than equipment (Inven_Item+0x01 != 1).
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
            user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                     static_cast<ENUM_ITEMSPACE>(0),
                                     slot);
            continue;
        }

        char title[21] = {};
        char text[256] = {};
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_213", 0),
                     0x14u);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "game_server_msg_214", 0),
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
    return 0;
}

// IDA_EA: 0x81DBC66
// IDA_NAME: _ZN35Dispatcher_Select_Item_Grwoth_Power4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Select_Item_Grwoth_Power::send(CUser *,ParamBase &)
void Dispatcher_Select_Item_Grwoth_Power::send(CUser* user,
                                               ParamBase& param) {
    const stGrowthPowerParam& response =
        static_cast<const stGrowthPowerParam&>(param);
    if (response.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_SELECT_ITEM_GROWTH_POWER,
                                 static_cast<uint8_t>(response.result));
        return;
    }

    user->set_growth_power_reward(user->get_charac_level());
    user->SendCmdOkPacket(ENUM_CMDPACKET_SELECT_ITEM_GROWTH_POWER);
}

// Root PacketDispatcher registration is deliberately deferred.  Restoring
// this concrete handler alone must not manufacture a partial 606-slot table.
