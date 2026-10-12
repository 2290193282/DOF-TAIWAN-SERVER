#include "old_game/player/core/cuser.h"
#include "common/config.h"
#include "infra/security/policy/hades_public_abi.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/world/region/game_world.h"

#include "old_game/player/core/charac_expand_data_mgr.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/player/core/character_manage_script.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/player/core/character_view_data.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/packet_dispatcher_give_gift_to_npc.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/skill/combo_skill.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/progression/quest/quest_skill_expert_job.h"

#include <algorithm>
#include <ctime>
#include <cstring>
#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "old_data/PVF/event/seria_blessing_script.h"
#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_data/PVF/town/town_script.h"
#include "old_data/db/avatar_aura/db_update_aura_avatar_option.h"
#include "old_data/db/character/db_update_charac_play_time.h"
#include "old_data/db/character/db_load_charac_hotkey_option.h"
#include "old_data/db/dungeon/db_update_revenge_data.h"
#include "old_data/db/statistics_log/db_log_buy_item.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/activity/reward/growth_booster_abi.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/money_log_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/os_api.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/transport/epoll_handler.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/thread/mutex.h"
#include "infra/network/transport/network_endpoint.h"


namespace WongWork {
class CItemUpgrade;
}  // namespace WongWork

struct CommonTime;
struct DB_LoadCharacHotKeyOption;
struct DB_UpdateRevengeData;
class CGameManager;
class GameWorld;

extern int GetInvenTypeFromItemSpace(ENUM_ITEMSPACE item_space)
    __asm__("_Z25GetInvenTypeFromItemSpace14ENUM_ITEMSPACE");
CDataManager* G_CDataManager();
namespace expert_job {
}  // namespace expert_job


namespace {

const unsigned int kInventoryAvatarSlotCount =
    oldsrv::config::kCharacterViewEquipmentTypeCount;
const int kLoadCharacHotkeyPacketId = 452;
const int kSkillSlotCount = 204;

typedef std::map<int, STSkillCommandData> SkillCommandMap;

bool InventoryCheckValidSlot(int inventory_type, int slot) {
    if (slot < 0) {
        return false;
    }
    switch (inventory_type) {
        case 0:
            return slot < oldsrv::config::kEquipmentTypeCount;
        case 1:
            return slot <= oldsrv::config::kMainInventoryLastSlot;
        case 2:
            return slot <= oldsrv::config::kAvatarInventoryLastSlot;
        case 3:
            return slot <= oldsrv::config::kCreatureInventoryLastSlot;
        case 5:
            return slot >= oldsrv::config::kOrdinaryEquipmentTypeFirst &&
                   slot <= oldsrv::config::kOrdinaryEquipmentTypeLast;
        default:
            return true;
    }
}

}  // namespace

// IDA_EA: 0x84FB01E
// IDA_NAME: _ZNK10CInventory14CheckValidSlotEii
// DEMANGLED: CInventory::CheckValidSlot(int,int)const
int CInventory::CheckValidSlot(int inventory_type, int slot) const {
    return InventoryCheckValidSlot(inventory_type, slot) ? 1 : 0;
}

// IDA_EA: 0x84FC1DE
// IDA_NAME: _ZNK10CInventory11GetInvenRefEii
// DEMANGLED: CInventory::GetInvenRef(int,int)const
const Inven_Item* CInventory::GetInvenRef(int inventory_type, int slot) const {
    if (static_cast<unsigned char>(CheckValidSlot(inventory_type, slot)) != 1u) {
        return 0;
    }
    switch (inventory_type) {
        case 0:
            return &equipment_items_[slot];
        case 1:
            return &stackable_items_[slot];
        case 2:
            return &avatar_items_[slot];
        case 3:
            return &creature_items_[slot];
        case 5: {
            CUser* user = parent_;
            const charac_expand::CData* expand =
                user->GetCharacExpandDataR(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
            return expand
                       ? reinterpret_cast<const CExpandEquipslot*>(expand)
                             ->GetInvenRefR(static_cast<INVEN_TYPE>(5), slot)
                       : 0;
        }
        default:
            return 0;
    }
}

// IDA_EA: 0x84FC2AE
// IDA_NAME: _ZN10CInventory11GetInvenRefEii
// DEMANGLED: CInventory::GetInvenRef(int,int)
Inven_Item* CInventory::GetInvenRef(int inventory_type, int slot) {
    if (static_cast<unsigned char>(CheckValidSlot(inventory_type, slot)) != 1u) {
        return 0;
    }
    switch (inventory_type) {
        case 0:
            return &equipment_items_[slot];
        case 1:
            return &stackable_items_[slot];
        case 2:
            return &avatar_items_[slot];
        case 3:
            return &creature_items_[slot];
        case 5: {
            CUser* user = parent_;
            charac_expand::CData* expand = user->GetCharacExpandData(
                static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
            return expand ? reinterpret_cast<CExpandEquipslot*>(expand)
                                ->GetInvenRefW(static_cast<INVEN_TYPE>(5), slot)
                          : 0;
        }
        default:
            return 0;
    }
}

// IDA_EA: 0x8685172
// IDA_NAME: _ZN15cUserHistoryLog8MoveAreaEiiiii
// DEMANGLED: cUserHistoryLog::MoveArea(int,int,int,int,int)
int cUserHistoryLog::MoveArea(int from_village,
                              int from_area,
                              int to_village,
                              int to_area,
                              int move_type) {
    static_cast<void>(move_type);  // The original writer records four fields.
    return owner_->LogHistory("MoveArea,%d,%d,%d,%d",
                              from_village,
                              from_area,
                              to_village,
                              to_area);
}

// IDA_EA: 0x8683A80
// IDA_NAME: _ZN15cUserHistoryLog8MoneySubEii15eMoneySubReason
// DEMANGLED: cUserHistoryLog::MoneySub(int,int,eMoneySubReason)
int cUserHistoryLog::MoneySub(int money,
                              int sub_count,
                              eMoneySubReason reason) {
    if (reason == static_cast<eMoneySubReason>(2) ||
        reason == static_cast<eMoneySubReason>(3)) {
        return MoneySubTrade(money, sub_count, reason);
    }
    return owner_->LogHistory(
        "Money-,%d,%d,%d", money, sub_count, static_cast<int>(reason));
}

// IDA_EA: 0x8683B38
// IDA_NAME: _ZN15cUserHistoryLog13MoneySubTradeEii15eMoneySubReason
// DEMANGLED: cUserHistoryLog::MoneySubTrade(int,int,eMoneySubReason)
int cUserHistoryLog::MoneySubTrade(int money,
                                   int sub_count,
                                   eMoneySubReason reason) {
    if (!trader_account_name_[0]) {
        return 0;
    }
    return owner_->LogHistory("Money-,%d,%d,%d,%s(%s)",
                              money,
                              sub_count,
                              static_cast<int>(reason),
                              trader_account_name_,
                              trader_character_name_);
}

// IDA_EA: 0x8683A20
// IDA_NAME: _ZN15cUserHistoryLog8MoneyAddEii15eMoneyAddReason
// DEMANGLED: cUserHistoryLog::MoneyAdd(int,int,eMoneyAddReason)
int cUserHistoryLog::MoneyAdd(int money,
                              int add_count,
                              eMoneyAddReason reason) {
    if (reason == static_cast<eMoneyAddReason>(1) ||
        reason == static_cast<eMoneyAddReason>(2)) {
        return MoneyAddTrade(money, add_count, reason);
    }
    return owner_->LogHistory(
        "Money+,%d,%d,%d", money, add_count, static_cast<int>(reason));
}

// IDA_EA: 0x8683AE0
// IDA_NAME: _ZN15cUserHistoryLog13MoneyAddTradeEii15eMoneyAddReason
// DEMANGLED: cUserHistoryLog::MoneyAddTrade(int,int,eMoneyAddReason)
int cUserHistoryLog::MoneyAddTrade(int money,
                                   int add_count,
                                   eMoneyAddReason reason) {
    if (!trader_account_name_[0]) {
        return 0;
    }
    return owner_->LogHistory("Money+,%d,%d,%d,%s(%s)",
                              money,
                              add_count,
                              static_cast<int>(reason),
                              trader_account_name_,
                              trader_character_name_);
}

// IDA_EA: 0x8684B22
// IDA_NAME: _ZN15cUserHistoryLog8MapClearEtii
// DEMANGLED: cUserHistoryLog::MapClear(unsigned short,int,int)
int cUserHistoryLog::MapClear(unsigned short hit_count,
                              int play_time_sec,
                              int map_index) {
    return owner_->LogHistory(
        "MapClear,%d,%d,%d", hit_count, play_time_sec, map_index);
}

// IDA_EA: 0x80CBC90
// IDA_NAME: _ZNK5CUser14GetServerGroupEv
// DEMANGLED: CUser::GetServerGroup(void)const
int CUser::GetServerGroup() const {
    return server_group_;
}

// IDA_EA: 0x8110DC2
// IDA_NAME: _ZN5CUser11GetPCRoomNoEv
// DEMANGLED: CUser::GetPCRoomNo(void)
int CUser::GetPCRoomNo() {
    return static_cast<int>(pc_room_no_);
}

// IDA_EA: 0x80FDF7A
// IDA_NAME: _ZN5CUser7GetCeraEv
// DEMANGLED: CUser::GetCera(void)
int CUser::GetCera() {
    return static_cast<int>(cera_);
}

// IDA_EA: 0x8101056
// IDA_NAME: _ZN5CUser12GetSsnStringEv
// DEMANGLED: CUser::GetSsnString(void)
char* CUser::GetSsnString() {
    return ssn_;
}

// IDA_EA: 0x812CE28
// IDA_NAME: _ZNK5CUser14GetPremiumInfoEv
// DEMANGLED: CUser::GetPremiumInfo(void)const
WongWork::CUserPremium* CUser::GetPremiumInfo() const {
    return const_cast<WongWork::CUserPremium*>(&user_premium_);
}

// IDA_EA: 0x868DE0A
// IDA_NAME: _ZN5CUser22UpdateAuraAvatarOptionEii
// DEMANGLED: CUser::UpdateAuraAvatarOption(int,int)
void CUser::UpdateAuraAvatarOption(int option_type, int value) {
    if (static_cast<unsigned int>(option_type) <= 2u) {
        DB_UpdateAuraAvatarOption::makeRequest(
            GetUID(),
            static_cast<unsigned int>(getCurCharacNo()),
            option_type,
            static_cast<char>(value));
    }
}

// IDA_EA: 0x868DE56
// IDA_NAME: _ZN5CUser28get_aura_avatar_option_valueEi
// DEMANGLED: CUser::get_aura_avatar_option_value(int)
int CUser::get_aura_avatar_option_value(int option_type) {
    const _Charac_info* cur = getCurCharacR();
    if (!cur) {
        return -1;
    }
    if (static_cast<unsigned int>(option_type) > 2u) {
        return -1;
    }
    return cur->fields_.aura_avatar_option_.values[option_type];
}

// IDA_EA: 0x868DFF8
// IDA_NAME: _ZN5CUser20is_equip_aura_avatarEcRi
// DEMANGLED: CUser::is_equip_aura_avatar(char,int &)
int CUser::is_equip_aura_avatar(char option_type, int& value) {
    CUser* const user = this;
    if (!user->getCurCharacR()) {
        return 1;
    }

    CInventory* inventory = static_cast<CInventory*>(
        user->getCurCharacInvenRefW());
    Inven_Item* item = inventory->GetInvenRef(0, 9);
    if (!item) {
        return 17;
    }
    if (item->isEmpty()) {
        return 17;
    }
    if (!item->isEquipableItemType()) {
        return 17;
    }

    const int item_id = item->item_id_;
    CItem* data_item = G_CDataManager()->find_item(item_id);
    if (!data_item) {
        return 17;
    }
    if (!reinterpret_cast<CEquipItem*>(data_item)
             ->is_aura_avatar_option(option_type)) {
        return 23;
    }

    value = reinterpret_cast<CEquipItem*>(data_item)
                ->get_aura_avatar_option_value();
    return 0;
}

// IDA_EA: 0x80E5FE6
// IDA_NAME: _ZNK8WongWork12CUserPremium12CheckPremiumEi
// DEMANGLED: WongWork::CUserPremium::CheckPremium(int)const
int WongWork::CUserPremium::CheckPremium(int premium_type) const {
    return premium_info_[premium_type].premium_type != 0;
}

// IDA_EA: 0x80E600E
// IDA_NAME: _ZNK5CUser17isAffectedPremiumE17ENUM_PREMIUM_TYPE
// DEMANGLED: CUser::isAffectedPremium(ENUM_PREMIUM_TYPE)const
int CUser::isAffectedPremium(ENUM_PREMIUM_TYPE premium_type) const {
    return GetPremiumInfo()->CheckPremium(static_cast<int>(premium_type));
}

// IDA_EA: 0x865C65C
// IDA_NAME: _ZN5CUser24IsPermissionPrivateStoreEv
// DEMANGLED: CUser::IsPermissionPrivateStore(void)
int CUser::IsPermissionPrivateStore() {
    return isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(8));
}

// IDA_EA: 0x812CE36
// IDA_NAME: _ZNK5CUser22IsHavePremiumAdvantageEv
// DEMANGLED: CUser::IsHavePremiumAdvantage(void)const
bool CUser::IsHavePremiumAdvantage() const {
    return GetPremiumInfo()->GetAdvPremiumCount() != 0;
}

// IDA_EA: 0x812CDA0
// IDA_NAME: _ZNK8WongWork12CUserPremium18GetAdvPremiumCountEv
// DEMANGLED: WongWork::CUserPremium::GetAdvPremiumCount(void)const
int WongWork::CUserPremium::GetAdvPremiumCount() const {
    return advantage_premium_count_;
}

// IDA_EA: 0x812CDAE
// IDA_NAME: _ZNK8WongWork12CUserPremium23GetAdvantageFatigueRateEv
// DEMANGLED: WongWork::CUserPremium::GetAdvantageFatigueRate(void)const
int WongWork::CUserPremium::GetAdvantageFatigueRate() const {
    return advantage_fatigue_rate_;
}

// IDA_EA: 0x814AA08
// IDA_NAME: _ZNK8WongWork12CUserPremium19GetAdvantageExpRateEv
// DEMANGLED: WongWork::CUserPremium::GetAdvantageExpRate(void)const
int WongWork::CUserPremium::GetAdvantageExpRate() const {
    return advantage_exp_rate_;
}

// IDA_EA: 0x822E4BE
// IDA_NAME: _ZNK8WongWork12CUserPremium16GetAdvantageCoinEv
// DEMANGLED: WongWork::CUserPremium::GetAdvantageCoin(void)const
int WongWork::CUserPremium::GetAdvantageCoin() const {
    return advantage_coin_;
}

// IDA_EA: 0x81347D6
// IDA_NAME: _ZNK10CInventory9get_moneyEv
// DEMANGLED: CInventory::get_money(void)const
int CInventory::get_money() const {
    return static_cast<int>(money_);
}

// IDA_EA: 0x8101072
// IDA_NAME: _ZNK5CUser12get_acc_nameEv
// DEMANGLED: CUser::get_acc_name(void)const
char* CUser::get_acc_name() const {
    return const_cast<char*>(account_name_);
}

// IDA_EA: 0x865DD14
// IDA_NAME: _ZN5CUser10send_equipEi
// DEMANGLED: CUser::send_equip(int)
void CUser::send_equip(int slot) {
    if (slot > oldsrv::config::kEquipmentTypeTitle &&
        slot != oldsrv::config::kEquipmentTypeCreature) {
        return;
    }

    const int state = static_cast<int>(state_);
    if (state == 12 || state == 5 || state == 8 || state == 10) {
        if (slot == oldsrv::config::kEquipmentTypeCreature) {
            PacketGuard packet;
            packet.put_header(0, 2);
            packet.put_byte(0);
            packet.put_short(1);
            make_basic_info(packet, 0);
            packet.finalize(true);
            Send(packet);
        }
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, this);
}

// IDA_EA: 0x865EDE0
// IDA_NAME: _ZN5CUser13use_equipmentEi
// DEMANGLED: CUser::use_equipment(int)
void CUser::use_equipment(int equipment_slot) {
    const int result = getCurCharacInvenW()->use_equip(equipment_slot);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_DECREASE_DURABILITY);
    if (result <= 0) {
        packet.put_byte(1);
        packet.put_byte(equipment_slot);
    } else {
        packet.put_byte(0);
        packet.put_byte(result);
    }
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x84ED008
// IDA_NAME: _ZN5CUser14GetGoldControlEv
// DEMANGLED: CUser::GetGoldControl(void)
void* CUser::GetGoldControl() {
    return &gold_control_;
}

// IDA_EA: 0x84FF54C
// IDA_NAME: _ZN10CInventory9use_moneyEi15eMoneySubReasonb
// DEMANGLED: CInventory::use_money(int,eMoneySubReason,bool)
int CInventory::use_money(int money,
                          eMoneySubReason reason,
                          bool log_history) {
    CInventory* const inventory = this;
    if (static_cast<int>(money_) < money || money < 0) {
        return 0;
    }

    const int before_money = static_cast<int>(money_);
    money_ -= static_cast<uint32_t>(money);

    int money_limit = 0x7FFFFFFF;
    CUser* user = parent_;
    if (user) {
        const char* account_name = user->get_acc_name();
        const int charac_level = user->get_charac_level();
        money_limit =
            G_CDataManager()->GetMoneyLimitPerLevel(charac_level,
                                                    account_name);
    }

    if (static_cast<int>(money_) > money_limit) {
        money_ = static_cast<uint32_t>(money_limit);
        const int charac_level = user->get_charac_level();
        const int charac_no = user->get_charac_no(-1);
        const unsigned int account_id =
            static_cast<unsigned int>(user->get_acc_id());
        const char* account_id_text = NumberToString(account_id, 0);
        cMyTrace trace("bool CInventory::use_money(int, eMoneySubReason, bool)",
                       3888,
                       4);
        trace("over money limit:m_id(%s),charac_no(%d),level(%d) "
              "use_money inven(%d)-sub(%d)",
              account_id_text,
              charac_no,
              charac_level,
              before_money,
              money);
    }

    if (user) {
        const int used_money = before_money - static_cast<int>(money_);
        write_log_use_money(reason, used_money, user);
        if (used_money > 0) {
            reinterpret_cast<Secu_GoldControl*>((user->GetGoldControl()))->SubGold(
                                     static_cast<unsigned int>(used_money),
                                     reason);
        }
    }

    if (log_history && user && money > 0) {
        CUser_history_log(user)->MoneySub(
            static_cast<int>(money_), money, reason);
    }

    return 1;
}

// IDA_EA: 0x80DA2FE
// IDA_NAME: _ZN5CUser12CheckInTradeEv
// DEMANGLED: CUser::CheckInTrade(void)
short CUser::CheckInTrade() {
    const unsigned short trade_index =
        static_cast<unsigned short>(trade_space_index_);
    return static_cast<unsigned short>(~trade_index) >> 15;
}

// IDA_EA: 0x80DD5A4
// IDA_NAME: _ZN5CUser22GetAccountLastPlayTimeEv
// DEMANGLED: CUser::GetAccountLastPlayTime(void)
int CUser::GetAccountLastPlayTime() {
    return static_cast<int>(account_last_play_time_);
}

// IDA_EA: 0x80DD584
// IDA_NAME: _ZNK5CUser19GetCharacExpandDataE23ENUM_CHARAC_EXPAND_TYPE
// DEMANGLED: CUser::GetCharacExpandData(ENUM_CHARAC_EXPAND_TYPE)const
charac_expand::CData* CUser::GetCharacExpandData(
    ENUM_CHARAC_EXPAND_TYPE expand_type) const {
    return charac_expand_data_.GetData(expand_type);
}

// IDA_EA: 0x828B5DE
// IDA_NAME: _ZNK5CUser20GetCharacExpandDataRE23ENUM_CHARAC_EXPAND_TYPE
// DEMANGLED: CUser::GetCharacExpandDataR(ENUM_CHARAC_EXPAND_TYPE)const
charac_expand::CData* CUser::GetCharacExpandDataR(
    ENUM_CHARAC_EXPAND_TYPE expand_type) const {
    return charac_expand_data_.GetDataR(expand_type);
}

// IDA_EA: 0x80F995A
// IDA_NAME: _ZN5CUser15getHackAnalyzerEv
// DEMANGLED: CUser::getHackAnalyzer(void)
WongWork::CHackAnalyzer* CUser::getHackAnalyzer() {
    return &hack_analyzer_;
}

// IDA_EA: 0x813492C
// IDA_NAME: _ZN5CUser8get_posXEv
// DEMANGLED: CUser::get_posX(void)
int CUser::get_posX() {
    return position_x_;
}

// IDA_EA: 0x813493C
// IDA_NAME: _ZN5CUser8get_posYEv
// DEMANGLED: CUser::get_posY(void)
int CUser::get_posY() {
    return position_y_;
}

// IDA_EA: 0x813494C
// IDA_NAME: _ZN5CUser13get_directionEv
// DEMANGLED: CUser::get_direction(void)
int CUser::get_direction() {
    return direction_;
}

// IDA_EA: 0x814589C
// IDA_NAME: _ZN5CUser8isGMUserEv
// DEMANGLED: CUser::isGMUser(void)
int CUser::isGMUser() {
    return game_master_mode_;
}

// IDA_EA: 0x814AA7A
// IDA_NAME: _ZN5CUser14SetRevivalTimeEl
// DEMANGLED: CUser::SetRevivalTime(long)
CUser* CUser::SetRevivalTime(long revival_time) {
    revival_time_ = static_cast<uint32_t>(revival_time);
    return this;
}

// IDA_EA: 0x814AA9A
// IDA_NAME: _ZN5CUser19SetUseAPCPotionTimeEl
// DEMANGLED: CUser::SetUseAPCPotionTime(long)
CUser* CUser::SetUseAPCPotionTime(long potion_time) {
    use_apc_potion_time_ = static_cast<uint32_t>(potion_time);
    return this;
}

// IDA_EA: 0x814AA5E
// IDA_NAME: _ZN5CUser18getCurCharacQuestWEv
// DEMANGLED: CUser::getCurCharacQuestW(void)
UserQuest* CUser::getCurCharacQuestW() {
    CUser* const user = this;
    user->enableSaveQuest();
    return &quest_;
}

// IDA_EA: 0x819A8A6
// IDA_NAME: _ZNK5CUser18getCurCharacQuestREv
// DEMANGLED: CUser::getCurCharacQuestR(void)const
UserQuest* CUser::getCurCharacQuestR() const {
    return const_cast<UserQuest*>(&quest_);
}

// IDA_EA: 0x868BC7C
// IDA_NAME: _ZN5CUser20GetCurExpertJobLevelEi
// DEMANGLED: CUser::GetCurExpertJobLevel(int)
int CUser::GetCurExpertJobLevel(int exp) {
    if (!this->getCurCharacR()) {
        return -1;
    }

    const int expert_job_type = this->GetCurCharacExpertJobType();
    if (!expert_job_type) {
        return 0;
    }

    STExpertJobScript* script =
        G_CDataManager()->GetExpertJobScript(expert_job_type);
    const bool is_boundary = script->isBoundaryExpValue(exp) != 0;
    const int level = static_cast<int>(
        script->GetLevel(static_cast<uint32_t>(exp)));
    if (!is_boundary) {
        return level;
    }

    STExpertJobEtcScript* etc_script =
        G_CDataManager()->GetExpertJobEtcScript();
    std::map<unsigned char, short>::iterator it =
        etc_script->expert_job_limit_levels.find(
            static_cast<unsigned char>(level));
    if (it != etc_script->expert_job_limit_levels.end() &&
        it->second <= this->get_charac_level()) {
        return level;
    }
    return level - 1;
}

// IDA_EA: 0x822D68A
// IDA_NAME: _ZNK10CInventory7GetCoinEv
// DEMANGLED: CInventory::GetCoin(void)const
int CInventory::GetCoin() const {
    return static_cast<int>(coin_);
}

// IDA_EA: 0x822D6FC
// IDA_NAME: _ZNK10CInventory22get_inventory_capacityEv
// DEMANGLED: CInventory::get_inventory_capacity(void)const
int CInventory::get_inventory_capacity() const {
    return inventory_capacity_;
}

// IDA_EA: 0x822FC06
// IDA_NAME: _ZN5CUser29SetLoadRentalInfoFromExchangeEb
// DEMANGLED: CUser::SetLoadRentalInfoFromExchange(bool)
CUser* CUser::SetLoadRentalInfoFromExchange(bool load) {
    load_rental_info_from_exchange_ = load ? 1 : 0;
    return this;
}

// IDA_EA: 0x822FC8E
// IDA_NAME: _ZN5CUser14setGmQuestFlagEb
// DEMANGLED: CUser::setGmQuestFlag(bool)
CUser* CUser::setGmQuestFlag(bool flag) {
    gm_quest_flag_ = flag ? 1 : 0;
    return this;
}

// IDA_EA: 0x822FCC6
// IDA_NAME: _ZN5CUser12getPowerSideEv
// DEMANGLED: CUser::getPowerSide(void)
int CUser::getPowerSide() {
    return guild_info_.power_side;
}

// IDA_EA: 0x822FCAA
// IDA_NAME: _ZN5CUser28getOnlinePreliminaryCharacNoEv
// DEMANGLED: CUser::getOnlinePreliminaryCharacNo(void)
unsigned int CUser::getOnlinePreliminaryCharacNo() {
    return online_preliminary_charac_no_;
}

// IDA_EA: 0x822FDB6
// IDA_NAME: _ZN5CUser4lockEv
// DEMANGLED: CUser::lock(void)
int CUser::lock() {
    return first_mutex_.lock();
}

// IDA_EA: 0x822FDCE
// IDA_NAME: _ZN5CUser6unlockEv
// DEMANGLED: CUser::unlock(void)
int CUser::unlock() {
    return first_mutex_.unlock();
}

// IDA_EA: 0x822FDE6
// IDA_NAME: _ZNK5CUser10getRegTimeEv
// DEMANGLED: CUser::getRegTime(void)const
int CUser::getRegTime() const {
    return static_cast<int>(registration_time_);
}

// IDA_EA: 0x822FE02
// IDA_NAME: _ZN5CUser11GetSchoolNoEv
// DEMANGLED: CUser::GetSchoolNo(void)
int CUser::GetSchoolNo() {
    return static_cast<int>(character_statistic_.school_no_);
}

// IDA_EA: 0x823020C
// IDA_NAME: _ZN5CUser10GetMailBoxEv
// DEMANGLED: CUser::GetMailBox(void)
WongWork::CMailBox* CUser::GetMailBox() {
    return mailbox_;
}

// IDA_EA: 0x823021A
// IDA_NAME: _ZN5CUser16isHumanCertifiedEv
// DEMANGLED: CUser::isHumanCertified(void)
int CUser::isHumanCertified() {
    return human_certified_;
}

// IDA_EA: 0x823022A
// IDA_NAME: _ZN5CUser17setHumanCertifiedEb
// DEMANGLED: CUser::setHumanCertified(bool)
CUser* CUser::setHumanCertified(bool certified) {
    human_certified_ = certified ? 1 : 0;
    return this;
}

// IDA_EA: 0x8230800
// IDA_NAME: _ZN5CUser8getHadesEv
// DEMANGLED: CUser::getHades(void)
XNuclear::CHades* CUser::getHades() {
    return &hades_;
}

// IDA_EA: 0x823080E
// IDA_NAME: _ZN5CUser12GetHackCheckEv
// DEMANGLED: CUser::GetHackCheck(void)
Secu_HackLogCheck* CUser::GetHackCheck() {
    return &hack_log_check_;
}

// IDA_EA: 0x823081C
// IDA_NAME: _ZN5CUser12GetCSHashSetEv
// DEMANGLED: CUser::GetCSHashSet(void)
CUserCSHashSet* CUser::GetCSHashSet() {
    return &cs_hashes_;
}

// IDA_EA: 0x8275668
// IDA_NAME: _ZN5CUser20GetPIReceiverManagerEv
// DEMANGLED: CUser::GetPIReceiverManager(void)
PIReceiverManager* CUser::GetPIReceiverManager() {
    return &pi_receiver_;
}

// IDA_EA: 0x82A6796
// IDA_NAME: _ZN5CUser12IsProgLogoutEv
// DEMANGLED: CUser::IsProgLogout(void)
int CUser::IsProgLogout() {
    return program_logout_;
}

// IDA_EA: 0x869735E
// IDA_NAME: _ZN5CUser13SetProgLogoutEv
// DEMANGLED: CUser::SetProgLogout(void)
CUser* CUser::SetProgLogout() {
    program_logout_ = 1;
    return this;
}

// IDA_EA: 0x82A683C
// IDA_NAME: _ZNK5CUser10GetIncreIDEv
// DEMANGLED: CUser::GetIncreID(void)const
int CUser::GetIncreID() const {
    return increment_id_;
}

// IDA_EA: 0x84ECD42
// IDA_NAME: _ZNK5CUser10GetSlotIDXEv
// DEMANGLED: CUser::GetSlotIDX(void)const
int CUser::GetSlotIDX() const {
    return slot_index_;
}

// IDA_EA: 0x863BEC0
// IDA_NAME: _ZN5CUser12GetLoginTimeEv
// DEMANGLED: CUser::GetLoginTime(void)
int CUser::GetLoginTime() {
    return static_cast<int>(login_time_);
}

// IDA_EA: 0x822FCD6
// IDA_NAME: _ZN5CUser15getClientHWSpecEv
// DEMANGLED: CUser::getClientHWSpec(void)
char* CUser::getClientHWSpec() {
    return reinterpret_cast<char*>(&client_spec_);
}

// IDA_EA: 0x867CB82
// IDA_NAME: _ZN5CUser22getPlayingDungeonIndexEv
// DEMANGLED: CUser::getPlayingDungeonIndex(void)
int CUser::getPlayingDungeonIndex() {
    return playing_dungeon_index_;
}

// IDA_EA: 0x80DA314
// IDA_NAME: _ZN5CUser12CheckInPartyEv
// DEMANGLED: CUser::CheckInParty(void)
short CUser::CheckInParty() {
    const unsigned short party_index =
        static_cast<unsigned short>(party_index_);
    return static_cast<unsigned short>(~party_index) >> 15;
}

// IDA_EA: 0x864870E
// IDA_NAME: _ZN5CUser29SendClientCrashDownLogToLogDBEi
// DEMANGLED: CUser::SendClientCrashDownLogToLogDB(int)
int CUser::SendClientCrashDownLogToLogDB(int src) {
    Stream* stream = GlobalData::stream_pool()->Acquire("user.cpp", 2220);
    CStreamGuard guard(stream, true);
    guard << 204;
    guard << -1;
    guard << src;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    return 1;
}

// IDA_EA: 0x86489F4
// IDA_NAME: _ZN5CUser10DisConnSigE11DISCONN_SIGbi
// DEMANGLED: CUser::DisConnSig(DISCONN_SIG,bool,int)
void CUser::DisConnSig(DISCONN_SIG source, bool flag, int arg) {
    CUser* const user = this;
    int src = static_cast<int>(source);
    (void)arg;

    user->lock();

    std::string event_manager_name("Arad_EventPeriodDataManager");
    ARAD::Arad_EventPeriodDataManager* event_period_manager =
        static_cast<ARAD::Arad_EventPeriodDataManager*>(
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()->findGameScript(
                event_manager_name));
    event_period_manager->delUserRewardData(user);

    CNetwork4096_450000& network = user->network_;
    const int socket = network.get_socket();
    if (socket >= 0 && user->get_state()) {
        if (!user->IsProgLogout()) {
            user->SetProgLogout();

            char peer_ip[16] = {};
            network.GetPeerIP2(peer_ip, sizeof(peer_ip));
            if (src == 5) {
                const int acc_id = user->get_acc_id();
                if (acc_id == 0 && peer_ip[0] != 0) {
                    src = 48;
                    flag = true;
                } else if (acc_id != 0 && peer_ip[0] == 0) {
                    src = 49;
                    flag = true;
                } else if (acc_id == 0 && peer_ip[0] == 0) {
                    src = 50;
                    flag = true;
                }
            }

            user->SendClientCrashDownLogToLogDB(src);
            if (network.get_socket() != -1) {
                if (src != 6 && src != 48 && src != 49 && src != 50) {
                    PacketGuard packet;
                    packet.put_header(0, 199);
                    packet.put_int(src);
                    packet.finalize(true);
                    user->Send(packet);
                }

                G_EpollHandler()->ResetEpoll(network.get_socket());

                Stream* stream =
                    GlobalData::stream_pool()->Acquire("user.cpp", 2449);
                CStreamGuard guard(stream, true);
                guard << 1;
                // IDA 0x86491B8 calls CStreamGuard::operator<<(int).  Keep
                // the disconnect signal's dispatcher header identical to
                // CUser::Parsing: route code followed by a 32-bit user id.
                guard << user->GetUID();
                GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
            }
        }
    }

    (void)flag;
    user->unlock();
}

// IDA_EA: 0x80DA32A
// IDA_NAME: _ZN5CUser17checkInBlueMarbleEv
// DEMANGLED: CUser::checkInBlueMarble(void)
short CUser::checkInBlueMarble() {
    const unsigned short index =
        static_cast<unsigned short>(blue_marble_index_);
    return static_cast<unsigned short>(~index) >> 15;
}

// IDA_EA: 0x80DA340
// IDA_NAME: _ZN5CUser18setBlueMarbleIndexEs
// DEMANGLED: CUser::setBlueMarbleIndex(short)
CUser* CUser::setBlueMarbleIndex(short index) {
    blue_marble_index_ = index;
    return this;
}

// IDA_EA: 0x80DA35E
// IDA_NAME: _ZN5CUser18getBlueMarbleIndexEv
// DEMANGLED: CUser::getBlueMarbleIndex(void)
int CUser::getBlueMarbleIndex() {
    return static_cast<unsigned short>(blue_marble_index_);
}

// IDA_EA: 0x82A67A6
// IDA_NAME: _ZN5CUser14CheckInWarRoomEv
// DEMANGLED: CUser::CheckInWarRoom(void)
short CUser::CheckInWarRoom() {
    const unsigned short index = static_cast<unsigned short>(war_room_index_);
    return static_cast<unsigned short>(~index) >> 15;
}

// IDA_EA: 0x822FE84
// IDA_NAME: _ZN5CUser15GetWarRoomIndexEv
// DEMANGLED: CUser::GetWarRoomIndex(void)
int CUser::GetWarRoomIndex() {
    return static_cast<unsigned short>(war_room_index_);
}

// IDA_EA: 0x86C0B50
// IDA_NAME: _ZN5CUser15SetWarRoomIndexEs
// DEMANGLED: CUser::SetWarRoomIndex(short)
CUser* CUser::SetWarRoomIndex(short index) {
    war_room_index_ = index;
    return this;
}

// IDA_EA: 0x82A67BC
// IDA_NAME: _ZN5CUser17checkInDeathTowerEv
// DEMANGLED: CUser::checkInDeathTower(void)
short CUser::checkInDeathTower() {
    const unsigned short index =
        static_cast<unsigned short>(death_tower_index_);
    return static_cast<unsigned short>(~index) >> 15;
}

// IDA_EA: 0x82A67D2
// IDA_NAME: _ZN5CUser18getDeathTowerIndexEv
// DEMANGLED: CUser::getDeathTowerIndex(void)
int CUser::getDeathTowerIndex() {
    return static_cast<unsigned short>(death_tower_index_);
}

// IDA_EA: 0x8469A6E
// IDA_NAME: _ZN5CUser18setDeathTowerIndexEs
// DEMANGLED: CUser::setDeathTowerIndex(short)
CUser* CUser::setDeathTowerIndex(short index) {
    death_tower_index_ = index;
    return this;
}

// IDA_EA: 0x82A67E2
// IDA_NAME: _ZN5CUser16checkInBossTowerEv
// DEMANGLED: CUser::checkInBossTower(void)
short CUser::checkInBossTower() {
    const unsigned short index =
        static_cast<unsigned short>(boss_tower_index_);
    return static_cast<unsigned short>(~index) >> 15;
}

// IDA_EA: 0x822FE94
// IDA_NAME: _ZN5CUser17getBossTowerIndexEv
// DEMANGLED: CUser::getBossTowerIndex(void)
int CUser::getBossTowerIndex() {
    return static_cast<unsigned short>(boss_tower_index_);
}

// IDA_EA: 0x814AAAC
// IDA_NAME: _ZN5CUser17setBossTowerIndexEs
// DEMANGLED: CUser::setBossTowerIndex(short)
CUser* CUser::setBossTowerIndex(short index) {
    boss_tower_index_ = index;
    return this;
}

// IDA_EA: 0x82A67F8
// IDA_NAME: _ZNK5CUser19checkInAdvanceAltarEv
// DEMANGLED: CUser::checkInAdvanceAltar(void)const
short CUser::checkInAdvanceAltar() const {
    const unsigned short index =
        static_cast<unsigned short>(advance_altar_index_);
    return static_cast<unsigned short>(~index) >> 15;
}

// IDA_EA: 0x82A680E
// IDA_NAME: _ZNK5CUser20getAdvanceAltarIndexEv
// DEMANGLED: CUser::getAdvanceAltarIndex(void)const
int CUser::getAdvanceAltarIndex() const {
    return static_cast<unsigned short>(advance_altar_index_);
}

// IDA_EA: 0x813490E
// IDA_NAME: _ZN5CUser20setAdvanceAltarIndexEs
// DEMANGLED: CUser::setAdvanceAltarIndex(short)
CUser* CUser::setAdvanceAltarIndex(short index) {
    advance_altar_index_ = index;
    return this;
}

CGameManager* G_CGameManager() __asm__("_Z14G_CGameManagerv");

// IDA_EA: 0x8655220
// IDA_NAME: _ZN5CUser12getBossTowerEv
// DEMANGLED: CUser::getBossTower(void)
WongWork::CBossTower* CUser::getBossTower() {
    if (boss_tower_index_ < 0) {
        return 0;
    }

    return G_CGameManager()->getBossTower(boss_tower_index_);
}

// IDA_EA: 0x86552A4
// IDA_NAME: _ZN5CUser13getDeathTowerEv
// DEMANGLED: CUser::getDeathTower(void)
WongWork::CDeathTower* CUser::getDeathTower() {
    if (death_tower_index_ < 0) {
        return 0;
    }

    return G_CGameManager()->getDeathTower(death_tower_index_);
}

// IDA_EA: 0x8655262
// IDA_NAME: _ZN5CUser15getAdvanceAltarEv
// DEMANGLED: CUser::getAdvanceAltar(void)
advancealtar::StageControl* CUser::getAdvanceAltar() {
    const int16_t index = advance_altar_index_;
    if (index < 0) {
        return 0;
    }
    return G_CGameManager()->getAdvanceAltar(index);
}

// IDA_EA: 0x822FE4E
// IDA_NAME: _ZN5CUser13GetPartyIndexEv
// DEMANGLED: CUser::GetPartyIndex(void)
int CUser::GetPartyIndex() {
    return static_cast<unsigned short>(party_index_);
}

// IDA_EA: 0x85BFD96
// IDA_NAME: _ZN5CUser13SetPartyIndexEs
// DEMANGLED: CUser::SetPartyIndex(short)
int CUser::SetPartyIndex(short party_index) {
    lock();
    party_index_ = party_index;
    return unlock();
}

// IDA_EA: 0x8657804
// IDA_NAME: _ZNK5CUser27getCurCharacTotalMaxFatigueEv
// DEMANGLED: CUser::getCurCharacTotalMaxFatigue(void)const
int CUser::getCurCharacTotalMaxFatigue() const {
    unsigned short max_fatigue = static_cast<unsigned short>(
        getCurCharacMaxFatigue());
    const WongWork::CUserPremium* premium = GetPremiumInfo();
    if (static_cast<unsigned short>(premium->GetAdvantageFatigueRate()) != 0) {
        max_fatigue = static_cast<unsigned short>(
            max_fatigue +
            getCurCharacMaxPremiumFatigue());
    }
    return max_fatigue;
}

// IDA_EA: 0x8657766
// IDA_NAME: _ZNK5CUser24getCurCharacTotalFatigueEv
// DEMANGLED: CUser::getCurCharacTotalFatigue(void)const
int CUser::getCurCharacTotalFatigue() const {
    const unsigned short fatigue =
        static_cast<unsigned short>(getCurCharacFatigue());
    const WongWork::CUserPremium* premium = GetPremiumInfo();
    if (static_cast<unsigned short>(premium->GetAdvantageFatigueRate()) != 0) {
        const unsigned short total_max = static_cast<unsigned short>(
            getCurCharacTotalMaxFatigue());
        const unsigned short max_premium = static_cast<unsigned short>(
            getCurCharacMaxPremiumFatigue());
        const unsigned short premium_fatigue = static_cast<unsigned short>(
            getCurCharacPremiumFatigue());
        const unsigned short premium_used =
            premium_fatigue < max_premium ? premium_fatigue : max_premium;
        const unsigned short total =
            static_cast<unsigned short>(fatigue + premium_used);
        return total < total_max ? total : total_max;
    }
    return fatigue;
}

// IDA_EA: 0x864F7FE
// IDA_NAME: _ZN5CUser20gainCurCharacFatigueEi
// DEMANGLED: CUser::gainCurCharacFatigue(int)
int CUser::gainCurCharacFatigue(int refill_fatigue) {
    int max_premium_fatigue = 0;
    if (static_cast<uint16_t>(user_premium_.GetAdvantageFatigueRate()) != 0) {
        max_premium_fatigue =
            static_cast<uint16_t>(getCurCharacMaxPremiumFatigue());
    }

    const int premium_fatigue =
        static_cast<uint16_t>(getCurCharacPremiumFatigue());
    const int usable_premium_fatigue =
        std::min<int>(premium_fatigue, max_premium_fatigue);
    const int remaining_fatigue = usable_premium_fatigue - refill_fatigue;
    if (remaining_fatigue >= 0) {
        return setCurCharacPremiumFatigue(
            static_cast<uint16_t>(remaining_fatigue));
    }

    if (usable_premium_fatigue > 0) {
        setCurCharacPremiumFatigue(0);
    }

    int fatigue = static_cast<uint16_t>(getCurCharacFatigue());
    if (fatigue + remaining_fatigue >= 0) {
        fatigue += remaining_fatigue;
    } else {
        fatigue = 0;
    }
    return setCurCharacFatigue(fatigue);
}

// IDA_EA: 0x8656500
// IDA_NAME: _ZN5CUser12CheckFatigueEv
// DEMANGLED: CUser::CheckFatigue(void)
bool CUser::CheckFatigue() {
    if (!account_id_) {
        return true;
    }
    const unsigned short total_fatigue = static_cast<unsigned short>(
        getCurCharacTotalFatigue());
    return total_fatigue < static_cast<unsigned short>(
                               getCurCharacTotalMaxFatigue());
}

// IDA_EA: 0x8656540
// IDA_NAME: _ZN5CUser11SendFatigueEv
// DEMANGLED: CUser::SendFatigue(void)
void CUser::SendFatigue() {
    PacketGuard packet;
    packet.put_header(0, 36);
    packet.put_short(static_cast<short>(getCurCharacTotalFatigue()));
    packet.put_short(static_cast<short>(getCurCharacTotalMaxFatigue()));
    packet.put_short(static_cast<short>(getCurCharacUsedFatigue()));
    packet.put_short(static_cast<short>(getCurCharacFatigueGrownUpBuff()));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x822FE5E
// IDA_NAME: _ZN5CUser10CheckInPvpEv
// DEMANGLED: CUser::CheckInPvp(void)
short CUser::CheckInPvp() {
    const unsigned short pvp_index = static_cast<unsigned short>(pvp_index_);
    return static_cast<unsigned short>(~pvp_index) >> 15;
}

// IDA_EA: 0x822FE74
// IDA_NAME: _ZN5CUser11GetPvpIndexEv
// DEMANGLED: CUser::GetPvpIndex(void)
int CUser::GetPvpIndex() {
    return static_cast<unsigned short>(pvp_index_);
}

// IDA_EA: 0x865518E
// IDA_NAME: _ZN5CUser10GetPVPRoomEv
// DEMANGLED: CUser::GetPVPRoom(void)
PvP_Room* CUser::GetPVPRoom() {
    if (pvp_index_ < 0) {
        return 0;
    }

    const int pvp_index = static_cast<unsigned short>(pvp_index_);
    return G_CGameManager()->GetPvp(pvp_index, this, 0);
}

// IDA_EA: 0x865514C
// IDA_NAME: _ZN5CUser8GetPartyEv
// DEMANGLED: CUser::GetParty(void)
CParty* CUser::GetParty() {
    const short party_index = party_index_;
    if (party_index < 0) {
        return 0;
    }
    CGameManager* const manager = G_CGameManager();
    return manager ? manager->GetParty(static_cast<int>(party_index)) : 0;
}

// IDA_EA: 0x86551DE
// IDA_NAME: _ZN5CUser10GetWarRoomEv
// DEMANGLED: CUser::GetWarRoom(void)
WarRoom* CUser::GetWarRoom() {
    const short war_room_index = war_room_index_;
    if (war_room_index < 0) {
        return 0;
    }
    CGameManager* const manager = G_CGameManager();
    return manager ? manager->GetWarRoom(war_room_index) : 0;
}

// IDA_EA: 0x8688638
// IDA_NAME: _ZN5CUser17GetUserPosInPartyEv
// DEMANGLED: CUser::GetUserPosInParty(void)
int CUser::GetUserPosInParty() {
    CParty* party = GetParty();
    if (party) {
        return party->GetUserPosition(this);
    }
    return -1;
}

// IDA_EA: 0x8678526
// IDA_NAME: _ZN5CUser13CheckMoveTownEi
// DEMANGLED: CUser::CheckMoveTown(int)
int CUser::CheckMoveTown(int village_index) {
    const unsigned int village_index_u =
        static_cast<unsigned int>(village_index);
    if (G_GameWorld()->IsPVPChannel()) {
        return 0;
    }
    if (village_index_u <= 0x0au &&
        ((1u << village_index_u) & 0x580u) != 0) {
        return 0;
    }

    Village* village = G_GameWorld()->GetVillage(village_index_u);
    if (!village) {
        return 8;
    }
    const TownPermissionRequirement* const permission =
        GetTownPermissionRequirement(village_index, get_charac_job());
    const int required_level =
        permission ? permission->required_level : village->required_level;
    if (get_charac_level() < required_level) {
        return 8;
    }

    if (permission) {
        if (permission->required_quest_ids.empty()) {
            return 0;
        }
        UserQuest* const quest = getCurCharacQuestR();
        if (!quest) {
            return 7;
        }
        for (std::vector<int32_t>::const_iterator required_quest =
                 permission->required_quest_ids.begin();
             required_quest != permission->required_quest_ids.end();
             ++required_quest) {
            if (static_cast<unsigned char>(quest->isClearQuest(
                    static_cast<unsigned int>(*required_quest))) != 1) {
                return 7;
            }
        }
        return 0;
    }

    const unsigned int required_quest =
        static_cast<unsigned int>(village->required_quest_id);
    if (!required_quest) {
        return 0;
    }

    UserQuest* const quest = getCurCharacQuestR();
    if (quest && static_cast<unsigned char>(
                     quest->isClearQuest(required_quest)) == 1) {
        return 0;
    }
    return 7;
}

// IDA_EA: 0x8692F36
// IDA_NAME: _ZN5CUser23isAllowLocationGeoStateEv
// DEMANGLED: CUser::isAllowLocationGeoState(void)
int CUser::isAllowLocationGeoState() {
    return allow_location_geo_state_;
}

// IDA_EA: 0x8654B94
// IDA_NAME: _ZN5CUser20findSkillCommandDataEiR18STSkillCommandData
// DEMANGLED: CUser::findSkillCommandData(int,STSkillCommandData &)
bool CUser::findSkillCommandData(int skill_index,
                                 STSkillCommandData& out) {
    CUser* const user = this;
    SkillCommandMap* command_map = getSkillCommand();
    if (!command_map) {
        return 0;
    }
    SkillCommandMap::iterator found = command_map->find(skill_index);
    if (found != command_map->end()) {
        out.word_00 = found->second.word_00;
        out.byte_02 = found->second.byte_02;
        for (int index = 0; index < out.byte_02; ++index) {
            out.raw_03[index] = found->second.raw_03[index];
            if (out.raw_03[index] == 7) {
                out.raw_03[index] = 8;
            }
        }
        return 1;
    }

    CSkill* skill = G_CDataManager()->find_skill(
        user->get_charac_job(), skill_index);
    if (!skill) {
        LogManager::logFormat(
            1, "user.cpp",
            "bool CUser::findSkillCommandData(int, STSkillCommandData&)",
            8467, "SKILL_COMMAND_LOG : KEY INDEX DUPLICATE!!");
        return 0;
    }

    const std::vector<STKeyCommandUnit>& key_commands = skill->key_commands_;
    const int command_count = static_cast<int>(key_commands.size());
    if (command_count <= 5 && command_count > 0) {
        out.word_00 = static_cast<uint16_t>(skill_index);
        out.byte_02 = static_cast<uint8_t>(command_count);
        for (int index = 0; index < command_count; ++index) {
            const STKeyCommandUnit& command_unit = key_commands[index];
            const int command =
                command_unit.keys.empty() ? 0 : command_unit.keys.front();
            out.raw_03[index] = static_cast<uint8_t>(command);
            if (out.raw_03[index] == 7) {
                out.raw_03[index] = 8;
            }
        }
        return 1;
    }

    if (command_count) {
        LogManager::logFormat(
            1, "user.cpp",
            "bool CUser::findSkillCommandData(int, STSkillCommandData&)",
            8450, "SKILL_COMMAND_LOG : ABNOMAL COMMAND CNT!! (%d) (%d)",
            skill_index,
            command_count);
    }
    return 0;
}

// IDA_EA: 0x8654E6A
// IDA_NAME: _ZN5CUser20findSkillCommandDataEiR18STSkillCommandDatab
// DEMANGLED: CUser::findSkillCommandData(int,STSkillCommandData &,bool)
bool CUser::findSkillCommandData(int skill_index,
                                 STSkillCommandData& out,
                                 bool use_saved_only) {
    (void)use_saved_only;
    SkillCommandMap* command_map = getSkillCommand();
    if (!command_map) {
        return 0;
    }
    SkillCommandMap::iterator found = command_map->find(skill_index);
    if (found == command_map->end()) {
        return 0;
    }
    out.word_00 = found->second.word_00;
    out.byte_02 = found->second.byte_02;
    for (int index = 0; index < out.byte_02; ++index) {
        out.raw_03[index] = found->second.raw_03[index];
        if (out.raw_03[index] == 7) {
            out.raw_03[index] = 8;
        }
    }
    return 1;
}

// IDA_EA: 0x86813BE
// IDA_NAME: _ZN5CUser8get_areaEb
// DEMANGLED: CUser::get_area(bool)
int CUser::get_area(bool current) {
    if (current && G_GameWorld()->IsSchoolPvPChannel()) {
        if (static_cast<unsigned char>(this->getCurCharacVill()) ==
                2u &&
            static_cast<unsigned int>(current_area_) == 4u) {
            return current_area_;
        }
    }
    if (current &&
        static_cast<unsigned char>(this->getCurCharacVill()) ==
            8u &&
        static_cast<unsigned int>(current_area_) < 4u) {
        return current_area_;
    }
    return area_;
}

// IDA_EA: 0x82F0E2A
// IDA_NAME: _ZN5CUser12set_positionEttc
// DEMANGLED: CUser::set_position(unsigned short,unsigned short,char)
CUser* CUser::set_position(uint16_t pos_x,
                            uint16_t pos_y,
                            char direction) {
    position_x_ = pos_x;
    position_y_ = pos_y;
    direction_ = static_cast<uint8_t>(direction);
    return this;
}

// IDA_EA: 0x8697418
// IDA_NAME: _ZN5CUser15set_before_areaEi
// DEMANGLED: CUser::set_before_area(int)
CUser* CUser::set_before_area(int area_index) {
    before_area_ = area_index;
    return this;
}

// IDA_EA: 0x8647864
// IDA_NAME: _ZN5CUser8set_areaEi
// DEMANGLED: CUser::set_area(int)
CUser* CUser::set_area(int area_index) {
    set_before_area(area_);
    area_ = area_index;
    return this;
}

// IDA_EA: 0x86D20E8
// IDA_NAME: _ZN5CUser14set_trick_areaEi
// DEMANGLED: CUser::set_trick_area(int)
CUser* CUser::set_trick_area(int area_index) {
    current_area_ = area_index;
    return this;
}

// IDA_EA: 0x86D20FA
// IDA_NAME: _ZN5CUser15get_before_areaEv
// DEMANGLED: CUser::get_before_area(void)
int CUser::get_before_area() {
    return before_area_;
}

// IDA_EA: 0x868BA38
// IDA_NAME: _ZNK5CUser17getBeforeAreaTimeEv
// DEMANGLED: CUser::getBeforeAreaTime(void)const
int CUser::getBeforeAreaTime() const {
    return static_cast<int>(before_area_time_);
}

// IDA_EA: 0x868BA46
// IDA_NAME: _ZN5CUser17setBeforeAreaTimeEl
// DEMANGLED: CUser::setBeforeAreaTime(long)
CUser* CUser::setBeforeAreaTime(long area_time) {
    before_area_time_ = static_cast<uint32_t>(area_time);
    return this;
}

// IDA_EA: 0x864A97C
// IDA_NAME: _ZNK5CUser19GetServerGroupToPvPEv
// DEMANGLED: CUser::GetServerGroupToPvP(void)const
int CUser::GetServerGroupToPvP() const {
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return server_group_;
    }
    return G_CEnvironment()->get_server_group();
}

// IDA_EA: 0x8690ECA
// IDA_NAME: _ZNK5CUser12getMoveSpaceEv
// DEMANGLED: CUser::getMoveSpace(void)const
int CUser::getMoveSpace() const {
    return static_cast<int>(move_space_);
}

// C06 CUser character-select state bodies moved to
// old_game/userinfo/cuser_character_select_state.cpp. This legacy unit has no
// duplicate implementation for the moved IDA symbols.

// IDA_EA: 0x8690098
// IDA_NAME: _ZN5CUser16setTagCharacInfoEijP15TAG_CHARAC_DATA
// DEMANGLED: CUser::setTagCharacInfo(int,unsigned int,TAG_CHARAC_DATA *)
int CUser::setTagCharacInfo(int slot,
                            unsigned int charac_no,
                            TAG_CHARAC_DATA* data) {
    if (slot < 0 || static_cast<int>(characters_.size()) <= slot) {
        return 0;
    }
    _Charac_info& character = characters_[slot];
    if (static_cast<unsigned int>(character.fields_.character_no_) !=
        charac_no) {
        return 0;
    }

    character.fields_.character_no_ = data->character_no;
    std::strncpy(character.fields_.character_name_,
                 data->character_name,
                 sizeof(data->character_name));
    character.fields_.job_ = data->job;
    character.fields_.level_ = data->level;
    character.fields_.grow_type_.growth_type = data->grow_type;
    character.fields_.skill_tree_index_ = data->skill_tree_index;
    character.fields_.additional_info_ = data->additional_info;
    return 1;
}

// IDA_EA: 0x864A956
// IDA_NAME: _ZN5CUser31GetGuildIdBeforeDeleteCharacterEi
// DEMANGLED: CUser::GetGuildIdBeforeDeleteCharacter(int)
int CUser::GetGuildIdBeforeDeleteCharacter(int slot) {
    if (slot < 0 || static_cast<int>(characters_.size()) <= slot) {
        return 0;
    }
    return static_cast<int>(characters_[slot].fields_.guild_id_);
}

// IDA_EA: 0x868E672
// IDA_NAME: _ZN5CUser25RemoveCharacMercenaryInfoEj
// DEMANGLED: CUser::RemoveCharacMercenaryInfo(unsigned int)
int CUser::RemoveCharacMercenaryInfo(unsigned int charac_no) {
    return mercenary_info_.RemoveCharac(charac_no);
}

// IDA_EA: 0x822FC4E
// IDA_NAME: _ZN5CUser16GetGameOptionRefEv
// DEMANGLED: CUser::GetGameOptionRef(void)
CGameOption* CUser::GetGameOptionRef() {
    return &game_option_;
}

// IDA_EA: 0x868BF14
// IDA_NAME: _ZN5CUser32select_charac_set_visible_valuesEc
// DEMANGLED: CUser::select_charac_set_visible_values(char)
int CUser::select_charac_set_visible_values(char values) {
    CUser* const user = this;
    user->set_charac_visible_values(values);
    user->SetVisible();
    CGameOption* game_option = user->GetGameOptionRef();
    if (static_cast<unsigned char>(
            user->get_charac_visible_grow_avatar())) {
        return game_option->set_etc_option(1, 1);
    }
    return game_option->set_etc_option(1, 0);
}

// IDA_EA: 0x868AC24
// IDA_NAME: _ZN5CUser12sendCharacQpEv
// DEMANGLED: CUser::sendCharacQp(void)
void CUser::sendCharacQp() {
    CUser* const user = this;
    CQuestShop* quest_shop =
        static_cast<CQuestShop*>(user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (quest_shop) {
        quest_shop->sendCharacQp(user, 0);
    }
}

// IDA_EA: 0x868FE02
// IDA_NAME: _ZN5CUser14GetManageLevelEv
// DEMANGLED: CUser::GetManageLevel(void)
int CUser::GetManageLevel() {
    return manage_level_;
}

// IDA_EA: 0x868FE10
// IDA_NAME: _ZN5CUser14GetManagePointEv
// DEMANGLED: CUser::GetManagePoint(void)
int CUser::GetManagePoint() {
    return manage_point_;
}

// IDA_EA: 0x868FE1E
// IDA_NAME: _ZN5CUser15GetUserMaxLevelEv
// DEMANGLED: CUser::GetUserMaxLevel(void)
int CUser::GetUserMaxLevel() {
    return user_max_level_;
}

// IDA_EA: 0x868FE2C
// IDA_NAME: _ZN5CUser14SetManageLevelEi
// DEMANGLED: CUser::SetManageLevel(int)
void CUser::SetManageLevel(int level) {
    CDataManager* data_manager = G_CDataManager();
    CharacManageScript* script = data_manager->character_manage_script_;
    const int max_level = script->manage_level_max_;
    manage_level_ = max_level >= level ? level : max_level;
}

// IDA_EA: 0x868FE72
// IDA_NAME: _ZN5CUser14SetManagePointEi
// DEMANGLED: CUser::SetManagePoint(int)
void CUser::SetManagePoint(int point) {
    CDataManager* data_manager = G_CDataManager();
    CharacManageScript* script = data_manager->character_manage_script_;
    const unsigned int max_level =
        static_cast<unsigned int>(script->manage_level_max_);
    const int require_point = script->GetLevelUpRequirePoint(max_level);
    if (point > require_point) {
        point = require_point;
    }
    manage_point_ = point;
}

// IDA_EA: 0x868FEC8
// IDA_NAME: _ZN5CUser15SetUserMaxLevelEi
// DEMANGLED: CUser::SetUserMaxLevel(int)
void CUser::SetUserMaxLevel(int level) {
    if (level <= oldsrv::config::kCharacterLevelMaximum) {
        user_max_level_ = level >= oldsrv::config::kCharacterLevelMinimum
            ? level : oldsrv::config::kCharacterLevelMinimum;
    } else {
        user_max_level_ = oldsrv::config::kCharacterLevelMaximum;
    }
}

// IDA_EA: 0x868FF04
// IDA_NAME: _ZN5CUser21CalcurateUserMaxLevelEv
// DEMANGLED: CUser::CalcurateUserMaxLevel(void)
void CUser::CalcurateUserMaxLevel() {
    int max_level = 0;
    const int count = static_cast<int>(characters_.size());
    for (int index = 0; index < count; ++index) {
        const int level = characters_[index].fields_.level_;
        if (level > max_level) {
            max_level = level;
        }
    }
    if (max_level > oldsrv::config::kCharacterLevelMaximum) {
        max_level = oldsrv::config::kCharacterLevelMaximum;
    }
    SetUserMaxLevel(max_level);
}

// IDA_EA: 0x868FFA0
// IDA_NAME: _ZN5CUser20CalcurateManagePointEv
// DEMANGLED: CUser::CalcurateManagePoint(void)
void CUser::CalcurateManagePoint() {
    int point = 0;
    const int count = static_cast<int>(characters_.size());
    for (int index = 0; index < count; ++index) {
        const int level = characters_[index].fields_.level_;
        CDataManager* data_manager = G_CDataManager();
        CharacManageScript* script = data_manager->character_manage_script_;
        point += script->GetPoint(level);
    }
    SetManagePoint(point);
}

// IDA_EA: 0x8690026
// IDA_NAME: _ZN5CUser20CalcurateManageLevelEv
// DEMANGLED: CUser::CalcurateManageLevel(void)
bool CUser::CalcurateManageLevel() {
    CalcurateManagePoint();
    const int point = GetManagePoint();
    CDataManager* data_manager = G_CDataManager();
    CharacManageScript* script = data_manager->character_manage_script_;
    const int level = script->GetLevel(point);
    const bool increased = GetManageLevel() < level;
    SetManageLevel(level);
    return increased;
}

// IDA_EA: 0x868DACE
// IDA_NAME: _ZN5CUser27set_growth_power_exp_rewardEi
// DEMANGLED: CUser::set_growth_power_exp_reward(int)
void CUser::set_growth_power_exp_reward(int charac_level) {
    CUser* const user = this;
    CEventBase* repeat_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(93)
            : 0;
    if (repeat_event && repeat_event->IsEventing(0)) {
        stSeriaBlessingScript::Value value;
        CDataManager* data_manager = G_CDataManager();
        stSeriaBlessingScript* script =
            data_manager->get_seria_blessing_script();
        if (script->get_value(charac_level, value) &&
            user->is_growth_power_exp_reward_right() != 1) {
            user->exp_reward_.gained_exp_bonus_rate =
                static_cast<unsigned char>(value.value_08);
            user->exp_reward_.clear_reward_exp_bonus_rate =
                static_cast<unsigned char>(value.value_0c);
            user->exp_reward_.additional_reward_rate =
                static_cast<unsigned char>(value.value_10);
            return;
        }
        user->exp_reward_ = STExpReward();
        return;
    }

    CDataManager* data_manager = G_CDataManager();
    STGrowthPowerData* growth_power = data_manager->get_growth_power_data();
    const int section_index =
        growth_power->get_idx_break_away_section(charac_level);
    const int reward_count =
        static_cast<int>(growth_power->exp_rewards.size());
    if (section_index >= 0 && section_index < reward_count &&
        user->is_growth_power_exp_reward_right() != 1) {
        user->exp_reward_ = growth_power->exp_rewards[section_index];
        return;
    }

    user->exp_reward_ = STExpReward();
}

// IDA_EA: 0x868D780
// IDA_NAME: _ZN5CUser33decide_growth_power_reward_systemEv
// DEMANGLED: CUser::decide_growth_power_reward_system(void)
void CUser::decide_growth_power_reward_system() {
    CUser* const user = this;
    bool reward_right = false;
    bool exp_reward_right = false;
    CEventBase* repeat_event =
        GlobalData::s_event_manager
            ? GlobalData::s_event_manager->GetRepeatEvent(93)
            : 0;
    CDataManager* data_manager = G_CDataManager();
    STGrowthPowerData* growth_power = data_manager->get_growth_power_data();

    if (repeat_event && repeat_event->IsEventing(0)) {
        if (repeat_event->IsEventing(0)) {
            const int charac_level =
                user->get_charac_level();
            stSeriaBlessingScript* script =
                data_manager->get_seria_blessing_script();
            if (script->is_possible_lv(charac_level)) {
                exp_reward_right = true;
                user->set_growth_power_exp_reward_right(true);
            }
        }

        const int charac_level = user->get_charac_level();
        const STBreakAwaySection section =
            growth_power->get_break_away_section_data(charac_level);
        if (oldsrv::policy::kGrowthPowerEquipmentRewardEnabled &&
            user->get_growth_power_reward() < section.low_level) {
            reward_right = true;
        }
        if (reward_right || exp_reward_right) {
            PacketGuard packet;
            packet.put_header(0, 376);
            packet.put_byte(user->get_charac_level());
            packet.put_byte(reward_right ? 1 : 0);
            packet.put_byte(exp_reward_right ? 1 : 0);
            packet.finalize(true);
            user->Send(packet);
        }
        user->set_growth_power_exp_reward(user->get_charac_level());
        return;
    }

    const int charac_level = user->get_charac_level();
    const STBreakAwaySection section =
        growth_power->get_break_away_section_data(charac_level);
    if (oldsrv::policy::kGrowthPowerEquipmentRewardEnabled &&
        user->get_growth_power_reward() < section.low_level) {
        PacketGuard packet;
        packet.put_header(0, 376);
        packet.put_byte(user->get_charac_level());
        packet.put_byte(1);
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
        user->set_growth_power_exp_reward_right(true);
    }
    user->set_growth_power_exp_reward(user->get_charac_level());
}

// IDA_EA: 0x8689968
// IDA_NAME: _ZN5CUser20SendEventBringUpInfoEv
// DEMANGLED: CUser::SendEventBringUpInfo(void)
void CUser::SendEventBringUpInfo() {
    CUser* const user = this;
    if (user->getCurCharacNo()) {
        PacketGuard packet;
        packet.put_header(0, 406);
        packet.put_byte(user->getCurUseBringUpMaterialTotal());
        packet.finalize(true);
        user->Send(packet);
    }
}

// IDA_EA: 0x8692F0A
// IDA_NAME: _ZN5CUser16setComboSkillSetEb
// DEMANGLED: CUser::setComboSkillSet(bool)
CUser* CUser::setComboSkillSet(bool enabled) {
    combo_skill_set_ = enabled ? 1 : 0;
    return this;
}

// IDA_EA: 0x8230164
// IDA_NAME: _ZN5CUser14GetGuildDBInfoEv
// DEMANGLED: CUser::GetGuildDBInfo(void)
STGuildDBInfoOnly* CUser::GetGuildDBInfo() {
    return &guild_info_;
}

// IDA_EA: 0x8692F26
// IDA_NAME: _ZN5CUser15isComboSkillSetEv
// DEMANGLED: CUser::isComboSkillSet(void)
int CUser::isComboSkillSet() {
    return combo_skill_set_;
}

// IDA_EA: 0x86924AA
// IDA_NAME: _ZN5CUser18sendComboSkillListEv
// DEMANGLED: CUser::sendComboSkillList(void)
void CUser::sendComboSkillList() {
    CUser* const user = this;
    if (user->get_charac_job() ==
        oldsrv::config::kCharacterJobDemonicSwordman) {
        SkillSlot* writable = user->getCurCharacSkillW();
        writable->eraseDuplicationComboSKill();

        PacketGuard packet;
        packet.put_header(0, 445);
        packet.put_byte(user->isComboSkillSet() != 0);
        packet.put_byte(2);
        SkillSlot* readable = user->getCurCharacSkillR();
        readable->getComboSkillSendInfo(packet);
        packet.finalize(true);
        user->Send(packet);
        user->setComboSkillSet(false);
    }
}

// IDA_EA: 0x866C46A
// IDA_NAME: _ZN5CUser15send_skill_infoEv
// DEMANGLED: CUser::send_skill_info(void)
void CUser::send_skill_info() {
    CUser* const user = this;
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_SKILLINFO);

    SkillSlot* skill_slot = user->getCurCharacSkillR();
    packet.put_short(static_cast<short>(
        skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_0)));
    int count_index = packet.get_index();
    int skill_count = 0;
    packet.put_byte(0);

    const _Mastered_skill* const tree0 =
        skill_slot->get_skillslot_buf(ENUM_SKILL_TREE_0);
    for (int slot = 0; slot < kSkillSlotCount; ++slot) {
        const uint8_t skill_index = tree0[slot].skill_index;
        if (!skill_index) {
            continue;
        }
        packet.put_byte(slot);
        packet.put_byte(skill_index);
        packet.put_byte(tree0[slot].skill_level);
        ++skill_count;

        STSkillCommandData command_data;
        if (user->findSkillCommandData(skill_index, command_data)) {
            packet.put_byte(command_data.byte_02);
            for (int index = 0; index < command_data.byte_02; ++index) {
                packet.put_byte(static_cast<signed char>(
                    command_data.raw_03[index]));
            }
        } else {
            packet.put_byte(0);
        }
    }

    const STGuildDBInfoOnly& guild_db_info = *user->GetGuildDBInfo();
    int guild_skill_count = guild_db_info.skill_count;
    if (G_GameWorld()->IsPvPSkilTreeChannel()) {
        guild_skill_count = 0;
    }
    for (int index = 0; index < guild_skill_count; ++index) {
        const STGuildSkill& guild_skill = guild_db_info.skills[index];
        if (static_cast<unsigned char>(
                user->getGuildMemberGrade()) &&
            static_cast<unsigned char>(
                user->getGuildMemberGrade()) &&
            guild_skill.skill_id > 199) {
            packet.put_byte(index + 138);
            packet.put_byte(guild_skill.skill_id);
            packet.put_byte(guild_skill.level);
            ++skill_count;
            packet.put_byte(0);
        }
    }
    packet.put_byte(count_index, skill_count);

    packet.put_short(static_cast<short>(
        skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_1)));
    count_index = packet.get_index();
    skill_count = 0;
    packet.put_byte(0);

    const _Mastered_skill* const tree1 =
        skill_slot->get_skillslot_buf(ENUM_SKILL_TREE_1);
    for (int slot = 0; slot < kSkillSlotCount; ++slot) {
        const uint8_t skill_index = tree1[slot].skill_index;
        if (!skill_index) {
            continue;
        }
        packet.put_byte(slot);
        packet.put_byte(skill_index);
        packet.put_byte(tree1[slot].skill_level);
        ++skill_count;

        STSkillCommandData command_data;
        if (user->findSkillCommandData(skill_index, command_data)) {
            packet.put_byte(command_data.byte_02);
            for (int index = 0; index < command_data.byte_02; ++index) {
                packet.put_byte(static_cast<signed char>(
                    command_data.raw_03[index]));
            }
        } else {
            packet.put_byte(0);
        }
    }

    for (int index = 0; index < guild_skill_count; ++index) {
        const STGuildSkill& guild_skill = guild_db_info.skills[index];
        if (static_cast<unsigned char>(
                user->getGuildMemberGrade()) &&
            static_cast<unsigned char>(
                user->getGuildMemberGrade()) &&
            guild_skill.skill_id > 199) {
            packet.put_byte(index + 138);
            packet.put_byte(guild_skill.skill_id);
            packet.put_byte(guild_skill.level);
            ++skill_count;
            packet.put_byte(0);
        }
    }
    packet.put_byte(count_index, skill_count);

    packet.put_short(static_cast<short>(
        skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_0)));
    packet.put_short(static_cast<short>(
        skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_1)));
    packet.finalize(true);
    user->Send(packet);
    user->sendComboSkillList();
}

// IDA_EA: 0x8692EF2
// IDA_NAME: _ZN5CUser19GetCharacHotKeyTypeEi
// DEMANGLED: CUser::GetCharacHotKeyType(int)
bool CUser::GetCharacHotKeyType(int character_job) {
    return character_job == oldsrv::config::kCharacterJobCreatorMage;
}

// IDA_EA: 0x8692E9C
// IDA_NAME: _ZN5CUser31CheckLoadHotKeyCashedCharacTypeEii
// DEMANGLED: CUser::CheckLoadHotKeyCashedCharacType(int,int)
bool CUser::CheckLoadHotKeyCashedCharacType(
    int character_job,
    int cached_character_job) {
    if (character_job == -1) {
        return 1;
    }
    return GetCharacHotKeyType(character_job) !=
           GetCharacHotKeyType(cached_character_job);
}

// IDA_EA: 0x8692D2A
// IDA_NAME: _ZN5CUser20LoadCharacTypeHotKeyEii
// DEMANGLED: CUser::LoadCharacTypeHotKey(int,int)
void CUser::LoadCharacTypeHotKey(int character_job, int cached_job) {
    CUser* const user = this;
    const unsigned char hotkey_type =
        static_cast<unsigned char>(user->GetCharacHotKeyType(cached_job));
    CGameOption* game_option = user->GetGameOptionRef();
    if (game_option->get_load_hotkey_flag(hotkey_type)) {
        if (user->CheckLoadHotKeyCashedCharacType(character_job,
                                                  cached_job)) {
            PacketGuard packet;
            packet.put_header(0, kLoadCharacHotkeyPacketId);
            packet.put_byte(hotkey_type);
            const unsigned int hotkey_option_length =
                game_option->get_hotkey_option_length(hotkey_type);
            packet.put_int(hotkey_option_length);
            if (hotkey_option_length != 0) {
                char* hotkey_option =
                    (user->GetGameOptionRef())->get_hotkey_option(
                                                  hotkey_type);
                packet.put_binary(hotkey_option, hotkey_option_length);
            }
            packet.finalize(true);
            user->Send(packet);
        }
    } else {
        DB_LoadCharacHotKeyOption::makeRequest(
            user->GetUID(),
            static_cast<unsigned int>(user->get_acc_id()),
            static_cast<int>(user->GetCharacHotKeyType(cached_job)));
    }
}

// IDA_EA: 0x867ED68
// IDA_NAME: _ZN5CUser11is_fightingEv
// DEMANGLED: CUser::is_fighting(void)
int CUser::is_fighting() {
    const unsigned int state = state_;
    if (state <= 0xAu) {
        const unsigned int state_bit = 1u << state;
        if ((state_bit & 0x720u) != 0) {
            return 1;
        }
        if ((state_bit & 0xDFu) != 0) {
            return 0;
        }
    }
    return 0;
}

// IDA_EA: 0x867EDB2
// IDA_NAME: _ZN5CUser9set_stateE8ch_state
// DEMANGLED: CUser::set_state(ch_state)
char CUser::set_state(int state) {
    state_ = static_cast<uint32_t>(state);
    if (state_ == 5u) {
        ClearItemSellCount();
    }
    char result = this->getCurCharacR() == 0;
    if (!result) {
        char fighting =
            static_cast<unsigned char>(is_fighting()) != 0;
        CInventory* inventory = reinterpret_cast<CInventory*>(
            this->getCurCharacInvenR());
        void* creature_manager = inventory->GetCreatureMgrW();
        return reinterpret_cast<user_creature::CCreatureMgr*>(creature_manager)
            ->TurnStomach(fighting != 0);
    }
    return result;
}

// IDA_EA: 0x8697526
// IDA_NAME: _ZN5CUser18ClearItemSellCountEv
// DEMANGLED: CUser::ClearItemSellCount(void)
void CUser::ClearItemSellCount() {
    in_dungeon_item_sell_count_ = 0;
    std::memset(in_dungeon_item_sell_ids_, 0,
                sizeof(in_dungeon_item_sell_ids_));
}

// IDA_EA: 0x864D84E
// IDA_NAME: _ZN5CUser18SetRestrictedGoodsEi
// DEMANGLED: CUser::SetRestrictedGoods(int)
void CUser::SetRestrictedGoods(int item_id) {
    if (!item_id) {
        return;
    }
    restricted_goods_.push_back(item_id);
}

// IDA_EA: 0x864D878
// IDA_NAME: _ZN5CUser17IsRestrictedGoodsEi
// DEMANGLED: CUser::IsRestrictedGoods(int)
bool CUser::IsRestrictedGoods(int item_id) {
    if (!item_id) {
        return false;
    }

    for (std::list<int>::iterator it = restricted_goods_.begin();
         it != restricted_goods_.end(); ++it) {
        if (*it == item_id) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x864D9A4
// IDA_NAME: _ZN5CUser19SendRestrictedGoodsEbi
// DEMANGLED: CUser::SendRestrictedGoods(bool,int)
void CUser::SendRestrictedGoods(bool immediate, int item_id) {
    PacketGuard packet;
    packet.put_header(0, 68);
    if (immediate) {
        packet.put_byte(0);
        packet.put_short(1);
        packet.put_int(item_id);
    } else {
        packet.put_byte(1);
        packet.put_short(static_cast<short>(restricted_goods_.size()));
        for (std::list<int>::const_iterator it = restricted_goods_.begin();
             it != restricted_goods_.end(); ++it) {
            packet.put_int(*it);
        }
    }
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x822FD2E
// IDA_NAME: _ZN5CUser6getPadEv
// DEMANGLED: CUser::getPad(void)
Sanicova::CPad* CUser::getPad() {
    return &pad_;
}

// IDA_EA: 0x850D348
// IDA_NAME: _ZN10CInventory17AvatarVisibleData4initEv
// DEMANGLED: CInventory::AvatarVisibleData::init(void)
CInventory::AvatarVisibleData* CInventory::AvatarVisibleData::init() {
    avatar_expansion_1 = 0;
    avatar_expansion_0 = avatar_expansion_1;
    item_id = static_cast<int16_t>(avatar_expansion_0);
    return this;
}

// IDA_EA: 0x8651628
// IDA_NAME: _ZN5CUser40UpdateCurCharacVisibleToCharacViewMemoryEv
// DEMANGLED: CUser::UpdateCurCharacVisibleToCharacViewMemory(void)
void CUser::UpdateCurCharacVisibleToCharacViewMemory() {
    CUser* const user = this;
    CInventory* inventory =
        static_cast<CInventory*>(user->getCurCharacInvenW());
    if (!inventory) {
        return;
    }

    for (unsigned int slot = 0; slot < kInventoryAvatarSlotCount; ++slot) {
        CInventory::AvatarVisibleData& visible =
            inventory->avatar_visible_data_[slot];
        const Inven_Item& item = inventory->equipment_items_[slot];
        const uint32_t item_id = static_cast<uint32_t>(item.item_id_);
        if (item_id) {
            const int clear_avatar = inventory->GetClearAvatar(
                static_cast<int>(slot));
            visible.item_id =
                clear_avatar ? static_cast<uint32_t>(clear_avatar) : item_id;

            const WongWork::CAvatarItemMgr* const avatar_manager =
                static_cast<const WongWork::CAvatarItemMgr*>(
                    inventory->GetAvatarItemMgrR());
            const stAvatarExpansionInfo_t* const expansion =
                avatar_manager->GetExpansionInfo(item.add_info_);
            if (expansion) {
                visible.avatar_expansion_0 = expansion->avatar_expansion[0];
                visible.avatar_expansion_1 = expansion->avatar_expansion[1];
            }
        } else {
            visible.init();
        }
    }
}

// IDA_EA: 0x86893AC
// IDA_NAME: _ZN5CUser12AddDBLogItemEjj20ENUM_DBLOG_ITEM_TYPE
// DEMANGLED: CUser::AddDBLogItem(unsigned int,unsigned int,ENUM_DBLOG_ITEM_TYPE)
int CUser::AddDBLogItem(unsigned int item_id,
                        unsigned int count,
                        ENUM_DBLOG_ITEM_TYPE log_type) {
    for (std::vector<DBLogItemInfo>::iterator it = db_log_items_.begin();
         it != db_log_items_.end();
         ++it) {
        if (it->item_id == item_id) {
            it->count += count;
            return reinterpret_cast<int>(&*it);
        }
    }

    DBLogItemInfo item;
    item.item_id = item_id;
    item.count = count;
    item.item_type = static_cast<uint32_t>(log_type);
    db_log_items_.push_back(item);
    return reinterpret_cast<int>(&db_log_items_.back());
}

// IDA_EA: 0x8681224
// IDA_NAME: _ZN5CUser13addAvatarItemEmccPKc20eAvatarItemAddReason
// DEMANGLED: CUser::addAvatarItem(unsigned long,char,char,char const*,eAvatarItemAddReason)
int CUser::addAvatarItem(unsigned long item_id,
                          char first_flag,
                          char avatar_select,
                          const char* ipg_no,
                          eAvatarItemAddReason reason) {
    CItem* const item = G_CDataManager()->find_item(
        static_cast<int>(item_id));
    if (!item) {
        return -1;
    }

    int usable_period = item->getUsablePeriod();
    const std::vector<stAvatarTypeSelect_t>* const avatar_type_select =
        reinterpret_cast<const std::vector<stAvatarTypeSelect_t>*>(
            reinterpret_cast<const CEquipItem*>(item)->getAvatarTypeSelect());
    if (static_cast<size_t>(static_cast<unsigned char>(avatar_select)) <
        avatar_type_select->size()) {
        usable_period =
            (*avatar_type_select)[static_cast<unsigned char>(avatar_select)]
                .usable_period;
    }

    CInventory* const inventory = reinterpret_cast<CInventory*>(
        this->getCurCharacInvenW());
    return inventory->AddAvatarItem(static_cast<int>(item_id),
                                    usable_period,
                                    false,
                                    first_flag,
                                    avatar_select,
                                    ipg_no,
                                    reason,
                                    false,
                                    0);
}

// IDA_EA: 0x867C2D8
// IDA_NAME: _ZN5CUser14SendUpdateItemENS_11eSendTargetE14ENUM_ITEMSPACEi
// DEMANGLED: CUser::SendUpdateItem(CUser::eSendTarget,ENUM_ITEMSPACE,int)
int CUser::SendUpdateItem(eSendTarget target,
                          ENUM_ITEMSPACE item_space,
                          int slot) {
    const int inventory_type = GetInvenTypeFromItemSpace(item_space);
    const CInventory* const inventory = static_cast<const CInventory*>(
        this->getCurCharacInvenR());
    const Inven_Item* const item =
        inventory->GetInvenRef(inventory_type, slot);
    if (!item) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(static_cast<int>(item_space));
    packet.put_short(1);
    packet.put_short(slot);

    const int item_key = item->add_info_;
    packet.put_int(item_key ? item->item_id_ : -1);
    packet.put_int(item_key);
    packet.put_byte(item->GetItemAttr());
    packet.put_short(item->avatar_select_);
    packet.put_byte(item->seal_or_state_);
    packet.put_int(item->extend_info_);
    packet.put_byte(item->amplify_option_.getAbilityType());
    packet.put_short(item->amplify_option_.getAbilityValue());

    if (item->isAvatarItemType()) {
        packet.put_int(30);
        const int jewel_key = item->get_add_info();
        const WongWork::CAvatarItemMgr* const avatar_manager =
            static_cast<const WongWork::CAvatarItemMgr*>(
                static_cast<const CInventory*>(this->getCurCharacInvenR())
                    ->GetAvatarItemMgrR());
        const stAvatarEmblemInfo_t* const jewel =
            avatar_manager->getJewelSocketData(jewel_key);
        // NOTI14 keeps the fixed 30-byte socket block for avatars without a
        // persisted emblem record.  Serialize the real empty value object so
        // PacketBuf::put_binary receives valid storage.
        stAvatarEmblemInfo_t empty_jewel;
        empty_jewel.init();
        packet.put_binary(reinterpret_cast<const char*>(
                              jewel ? jewel : &empty_jewel),
                          30);

        const int expansion_key = item->get_add_info();
        const CInventory* const expansion_inventory =
            static_cast<const CInventory*>(this->getCurCharacInvenR());
        const WongWork::CAvatarItemMgr* const expansion_manager =
            static_cast<const WongWork::CAvatarItemMgr*>(
                expansion_inventory->GetAvatarItemMgrR());
        const stAvatarExpansionInfo_t* const expansion =
            expansion_manager->GetExpansionInfo(expansion_key);
        packet.put_int(4);
        if (expansion) {
            packet.put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet.put_binary(
                reinterpret_cast<const char*>(&empty_expansion), 4);
        }
    }

    /*
       0828 NOTI14 creature records branch on the PVF prototype's virtual
       item type (23), not the restored item's legacy category byte.  The
       client then consumes one explicit marker byte before the shared item
       tail, even when no remain-date dword is present.  Keep this single-slot
       writer byte-identical to CInventory::MakeItemPacket so move/equip
       updates cannot shift the following record fields.
     */
    bool is_creature_record = false;
    if (item->item_id_ != 0) {
        CItem* const item_script = G_CDataManager()->find_item(
            static_cast<unsigned int>(item->item_id_));
        is_creature_record = item_script && item_script->vtable_ &&
            item_script->vtable_->get_item_type(item_script) ==
                oldsrv::config::kEquipmentTypeCreature;
    }
    if (is_creature_record) {
        const int tick = OS_API::GetDateTimeTick();
        const int creature_key = item->get_add_info();
        const CInventory* const creature_inventory =
            static_cast<const CInventory*>(this->getCurCharacInvenR());
        const int remain_date = creature_inventory->GetCreatureMgrR()
                                    ->GetRemainDate(creature_key, tick);
        packet.put_int(remain_date);
    }
    if (static_cast<int>(item_space) == 7) {
        packet.put_byte(0);
    }

    CDataManager* const data_manager = G_CDataManager();
    data_manager->item_upgrade_->make3rdChroniclePacket(
        this, *item, &packet);
    packet.put_packet(*item);
    packet.finalize(true);
    SendPacket(target, packet);
    return 1;
}

// IDA_EA: 0x867C65A
// IDA_NAME: _ZN5CUser18SendUpdateItemListENS_11eSendTargetE14ENUM_ITEMSPACEi
// DEMANGLED: CUser::SendUpdateItemList(CUser::eSendTarget,ENUM_ITEMSPACE,int)
int CUser::SendUpdateItemList(eSendTarget target,
                              ENUM_ITEMSPACE item_space,
                              int slot) {
    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(static_cast<int>(item_space));
    packet.put_short(1);

    CInventory* const inventory = static_cast<CInventory*>(
        this->getCurCharacInvenR());
    if (!inventory) {
        return 0;
    }

    INVEN_TYPE inventory_type = static_cast<INVEN_TYPE>(1);
    if (item_space == static_cast<ENUM_ITEMSPACE>(1)) {
        inventory_type = static_cast<INVEN_TYPE>(2);
    } else if (item_space == static_cast<ENUM_ITEMSPACE>(7)) {
        inventory_type = static_cast<INVEN_TYPE>(3);
    } else if (item_space == static_cast<ENUM_ITEMSPACE>(3)) {
        inventory_type = static_cast<INVEN_TYPE>(0);
    }
    if (static_cast<unsigned char>(
            inventory->MakeItemPacket(inventory_type, slot, packet)) != 1u) {
        return 0;
    }

    packet.finalize(true);
    return SendPacket(target, packet);
}

// IDA_EA: 0x867B8FE
// IDA_NAME: _ZN5CUser10SendPacketENS_11eSendTargetER11PacketGuard
// DEMANGLED: CUser::SendPacket(CUser::eSendTarget,PacketGuard &)
int CUser::SendPacket(eSendTarget target, PacketGuard& packet) {
    const int target_value = static_cast<int>(target);
    if (target_value == 1) {
        this->Send(packet);
        return 1;
    }
    if (target_value > 1) {
        if (target_value == 2) {
            CParty* party = GetParty();
            if (party) {
                party->send_to_party(packet);
                return 1;
            }
        } else {
            if (target_value != 3) {
                return 1;
            }
            WarRoom* room = GetWarRoom();
            if (room) {
                room->SendToRoom(packet);
                return 1;
            }
        }
        this->Send(packet);
        return 1;
    }
    if (target_value == 0) {
        G_GameWorld()->send_all(packet, this);
    }
    return 1;
}

// IDA_EA: 0x8689494
// IDA_NAME: _ZNK5CUser17DBUpdateDBLogItemEv
// DEMANGLED: CUser::DBUpdateDBLogItem(void)const
int CUser::DBUpdateDBLogItem() const {
    const CUser* const user = this;
    for (std::vector<DBLogItemInfo>::const_iterator it =
             db_log_items_.begin();
         it != db_log_items_.end();
         ++it) {
        DB_LogBuyItem::makeRequest(
            static_cast<unsigned int>((const_cast<CUser*>(user))->get_acc_id()),
            static_cast<unsigned int>((const_cast<CUser*>(user))->getCurCharacNo()),
            it->item_id,
            it->count,
            static_cast<ENUM_DBLOG_ITEM_TYPE>(
                static_cast<unsigned char>(it->item_type)));
    }
    return 0;
}

// IDA_EA: 0x8697262
// IDA_NAME: _ZN5CUser14ResetDBLogItemEv
// DEMANGLED: CUser::ResetDBLogItem(void)
int CUser::ResetDBLogItem() {
    db_log_items_.clear();
    return 0;
}

// IDA_EA: 0x86973DC
// IDA_NAME: _ZN5CUser27setLinkCharacDisconnectFlagEb
// DEMANGLED: CUser::setLinkCharacDisconnectFlag(bool)
CUser* CUser::setLinkCharacDisconnectFlag(bool value) {
    link_charac_disconnect_flag_ = static_cast<unsigned char>(value);
    return this;
}

// IDA_EA: 0x86973F8
// IDA_NAME: _ZN5CUser26isLinkCharacDisconnectFlagEv
// DEMANGLED: CUser::isLinkCharacDisconnectFlag(void)
int CUser::isLinkCharacDisconnectFlag() {
    return link_charac_disconnect_flag_;
}

// IDA_EA: 0x8697408
// IDA_NAME: _ZN5CUser30getDisconnectLinkCharacSlotIdxEv
// DEMANGLED: CUser::getDisconnectLinkCharacSlotIdx(void)
int CUser::getDisconnectLinkCharacSlotIdx() {
    return static_cast<unsigned char>(disconnect_link_charac_slot_index_);
}

// IDA_EA: 0x8652C8E
// IDA_NAME: _ZN5CUser22doLinkCharacDisconnectEv
// DEMANGLED: CUser::doLinkCharacDisconnect(void)
void CUser::doLinkCharacDisconnect() {
    CUser* const user = this;
    if (!static_cast<unsigned char>(user->isLinkCharacDisconnectFlag())) {
        return;
    }

    const bool locked =
        static_cast<unsigned char>(user->isLocked4DataLoad()) ||
        static_cast<unsigned char>(user->isLockedCreateCharac()) ||
        static_cast<unsigned char>(user->isLockedDeleteCharac());
    if (locked) {
        LogManager::logFormat(
            1,
            "user.cpp",
            "void CUser::doLinkCharacDisconnect()",
            0x1CC1,
            "LINK_LOG : checkLinkCharacDisconnect ERROR!! m_id(%s)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0));
        return;
    }

    const int disconnect_slot =
        static_cast<signed char>(user->getDisconnectLinkCharacSlotIdx());
    const int charac_count = static_cast<int>(characters_.size());
    user->ChangeCharacSlot(static_cast<unsigned int>(disconnect_slot - 1),
                           static_cast<unsigned int>(charac_count - 1));
    const int charac_count_after = static_cast<int>(characters_.size());
    user->ChangeCharacSlot(static_cast<unsigned int>(disconnect_slot - 1),
                           static_cast<unsigned int>(charac_count_after - 1));
    user->setLinkCharacDisconnectFlag(false);
    user->UpdateCharacView();
}

// IDA_EA: 0x869724A
// IDA_NAME: _ZN5CUser21ResetCharacExpandDataEv
// DEMANGLED: CUser::ResetCharacExpandData(void)
bool CUser::ResetCharacExpandData() {
    return charac_expand_data_.reset();
}

// IDA_EA: 0x8686FEE
// IDA_NAME: _ZN5CUser24ReturnToSelectCharacListEb
// DEMANGLED: CUser::ReturnToSelectCharacList(bool)
void CUser::ReturnToSelectCharacList(bool send_packet) {
    CUser* const user = this;
    // Returning to the character list is a session boundary just like reset
    // and disconnect.  Release any queued/source/target Zombie authority
    // before party/PvP containers and the selected-character identity vanish.
    channel_zombie_matching_producer::ResetUser(user);
    user->UpdateCurCharacVisibleToCharacViewMemory();
    user->WorkPerFiveMin();

    if (user->get_charac_memberkey()) {
        const int money_tax =
            user->get_member_pay_tex_money_to_upper();
        const int fatigue_tax =
            user->get_member_pay_tex_fatigue_to_upper();
        const unsigned int charac_no =
            static_cast<unsigned int>(user->getCurCharacNo());
        const bool connected =
            static_cast<unsigned char>(
                user->is_connect_upper_member()) != 0;
        ::CMonitorServerProxy* proxy =
            ::GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendMemberPayTax(
                                             connected,
                                             charac_no,
                                             money_tax,
                                             fatigue_tax);
        if (money_tax > 0) {
            user->set_member_pay_tex_money_to_upper(0);
        }
        if (fatigue_tax > 0) {
            user->set_member_pay_tex_fatigue_to_upper(0);
        }
    }

    CGameManager* game_manager = G_CGameManager();
    user->set_uppermember_exp_level(0);
    user->set_connect_upper_member(0);
    game_manager->CheckOutTrade(user);
    game_manager->CheckOutParty(user, false);
    game_manager->CheckOutPvp(user, false);
    game_manager->CheckOutWarRoom(user);
    game_manager->checkOutBlueMarble(user);

    private_store::GetInstancePrivateStoreMgr()->Exit(user);
    pvp_assault::GetInstanceAssaultMgr()->OnLeaveAssaultPlace(user, true);
    GlobalData::s_ExpertJobMgr->OnLeaveUser(user);

    DB_UpdateCharacPlayTime::makeRequest(
        user->get_charac_no(-1),
        user->GetCharacPlayTick());
    GlobalData::s_onlinePreliminaryTeamMgr->OnLeaveUser(user);

    const unsigned int cur_charac_no =
        static_cast<unsigned int>(user->getCurCharacNo());
    const unsigned char attack_count = static_cast<unsigned char>(
        user->GetCurVillageAttackCount());
    const unsigned char revenge_count =
        static_cast<unsigned char>(user->GetCurRevengeDungeonCount());
    if (static_cast<unsigned char>(
            user->IsVillageAttackDBUpdate())) {
        DB_UpdateRevengeData::makeRequest(cur_charac_no,
                                         attack_count,
                                         revenge_count);
    }

    user->DBUpdateDBLogItem();
    user->ResetDBLogItem();
    user->UpdateData();
    // CacheCharacterMemory serializes the 0828 NPC relationship sidecar.
    // Keep it alive until the exchange-server snapshot has been built; the
    // old order erased the sidecar first and produced a zero-identity/empty
    // SIG_LOAD_ETC cache record on every fast relog.
    user->LogoutCachedCharacter(1u);
    npc_gift::Erase(user);
    G_GameWorld()->leave_game_world(user);
    user->ResetCharacExpandData();

    std::string event_manager_name("Arad_EventPeriodDataManager");
    ARAD::Arad_EventPeriodDataManager* event_period_manager =
        static_cast<ARAD::Arad_EventPeriodDataManager*>(
            ARAD::Singleton<ARAD::Arad_DataManager>::Get()->findGameScript(
                event_manager_name));
    event_period_manager->delUserRewardData(user);

    user->ResetCurCharac();
    user->resetSaveFlags();
    user->set_state(2);
    user->doLinkCharacDisconnect();
    G_GameWorld()->InsertLoginUser(user);

    user->inventory_memory_.reset();
    user->quest_.reset();
    user->getPad()->setCheckTime(static_cast<long>(std::time(0)));
    CerashopAddRestrict::Manager::ClearBuyRestrictItem(user);
    WongWork::CHandlePremium::handleReturn2CharacView(user);

    if (send_packet) {
        PacketGuard packet;
        packet.put_header(1, 7);
        packet.put_byte(1);
        packet.finalize(true);
        user->Send(packet);
    }
}
