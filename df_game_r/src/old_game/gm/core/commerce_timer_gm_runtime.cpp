#include "old_game/economy/mall/auction_runtime.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "core/config/env/environment.h"
#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/auction_cashshop/db_load_auto_market_conditions_control.h"
#include "old_data/db/auction_cashshop/db_update_auto_market_conditions_control.h"
#include "old_game/economy/trade/timer_vending_machine_item_notify.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <cstring>
#include <string>

// IDA_EA: 0x82F8108
// IDA_NAME: _ZN28CAutoMarketConditionsControlC2Ev
// DEMANGLED: CAutoMarketConditionsControl::CAutoMarketConditionsControl(void)
CAutoMarketConditionsControl::CAutoMarketConditionsControl()
    : equipment_drop_level_ranges_() {
    gold_control_state_ = 0;
    item_control_state_ = 0;
    durability_control_state_ = 0;
}

// IDA_EA: 0x82F8144
// IDA_NAME: _ZN28CAutoMarketConditionsControlD2Ev
// DEMANGLED: CAutoMarketConditionsControl::~CAutoMarketConditionsControl()
CAutoMarketConditionsControl::~CAutoMarketConditionsControl() {}

// IDA_EA: 0x82F815C
// IDA_NAME: _ZN28CAutoMarketConditionsControl10LoadScriptEPKc
// DEMANGLED: CAutoMarketConditionsControl::LoadScript(char const*)
int CAutoMarketConditionsControl::LoadScript(const char* filename) {
    // IDA 0x82F816B passes dir__2 at 0x8C1ECC3.  The first byte there is
    // NUL, so this importer uses the filename as supplied by CDataManager
    // (for example, "Etc/ItemDropInfo_Control.etc").
    if (loadRDARScriptFile("", filename) != 1) {
        return 30;
    }

    bool scan_ok = false;
    int level = 0;
    std::string section;
    while (ScanType(section, true) == 1) {
        if (section == "[control of gold drop amount rate]") {
            for (int index = 0; index <= 9; ++index) {
                gold_drop_amount_rates_[index] = ScanFloat(3, &scan_ok);
                if (!scan_ok) {
                    return 47;
                }
            }
        } else if (section == "[control of gold drop amount rate per level]") {
            std::memset(gold_drop_amount_rate_per_level_,
                        0,
                        sizeof(gold_drop_amount_rate_per_level_));
            while (true) {
                level = ScanInt(&scan_ok);
                if (!scan_ok) {
                    break;
                }
                if (level <= 0 || level > 200) {
                    return 60;
                }
                gold_drop_amount_rate_per_level_[level] = ScanInt(&scan_ok);
                if (!scan_ok) {
                    return 64;
                }
            }
        } else if (section == "[control of equipment drop prob]") {
            for (int index = 0; index <= 9; ++index) {
                equipment_drop_rates_[index] = ScanFloat(3, &scan_ok);
                if (!scan_ok) {
                    return 73;
                }
            }
        } else if (section ==
                   "[control of equipment drop prob per level range]") {
            while (true) {
                level = ScanInt(&scan_ok);
                if (!scan_ok) {
                    break;
                }
                equipment_drop_level_ranges_.push_back(level);
                if (!scan_ok) {
                    return 87;
                }
            }
        } else if (section == "[control of durability decrease rate]") {
            for (int index = 0; index <= 9; ++index) {
                durability_decrease_rates_[index] = ScanFloat(3, &scan_ok);
                if (!scan_ok) {
                    return 96;
                }
            }
        } else {
            return 100;
        }
    }
    return 0;
}

// IDA_EA: 0x82F8540
// IDA_NAME: _ZN28CAutoMarketConditionsControl21SendNotiSetDurabilityEi
// DEMANGLED: CAutoMarketConditionsControl::SendNotiSetDurability(int)
void CAutoMarketConditionsControl::SendNotiSetDurability(int durability_rate) {
    PacketGuard packet;
    MakeNotiSetDurability(packet, durability_rate);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x82F85A8
// IDA_NAME: _ZN28CAutoMarketConditionsControl21SendNotiSetDurabilityEv
// DEMANGLED: CAutoMarketConditionsControl::SendNotiSetDurability(void)
void CAutoMarketConditionsControl::SendNotiSetDurability() {
    SendNotiSetDurability(static_cast<int>(durability_current_rate_ * 1000.0));
}

// IDA_EA: 0x82F85EA
// IDA_NAME: _ZN28CAutoMarketConditionsControl21SendNotiSetDurabilityEP5CUser
// DEMANGLED: CAutoMarketConditionsControl::SendNotiSetDurability(CUser *)
void CAutoMarketConditionsControl::SendNotiSetDurability(CUser* user) {
    if (durability_control_state_) {
        PacketGuard packet;
        MakeNotiSetDurability(
            packet, static_cast<int>(durability_current_rate_ * 1000.0));
        user->Send(packet);
    }
}

// IDA_EA: 0x82F8684
// IDA_NAME: _ZN28CAutoMarketConditionsControl12LoadDatabaseEP34SIG_AUTO_MARKET_CONDITIONS_CONTROL
// DEMANGLED: CAutoMarketConditionsControl::LoadDatabase(SIG_AUTO_MARKET_CONDITIONS_CONTROL *)
int CAutoMarketConditionsControl::LoadDatabase(
    SIG_AUTO_MARKET_CONDITIONS_CONTROL* payload) {
    uint64_t market_gold = 0;
    if (payload->request_flag) {
        market_gold = payload->control_over_gold;
    } else {
        market_gold = payload->current_day.total_gold +
                      payload->current_day.auction_gold -
                      (payload->previous_day.total_gold +
                       payload->previous_day.auction_gold) +
                      payload->previous_day.over_gold;
    }

    GetMarketConditionRate(
        static_cast<double>(payload->control_optimum_gold_supply),
        static_cast<double>(market_gold),
        gold_drop_amount_rates_,
        payload->previous_day.gold_phase,
        gold_control_state_,
        gold_current_rate_);
    GetMarketConditionRate(
        static_cast<double>(payload->control_optimum_gold_supply),
        static_cast<double>(market_gold),
        equipment_drop_rates_,
        payload->previous_day.item_phase,
        item_control_state_,
        item_current_rate_);
    GetMarketConditionRate(
        static_cast<double>(payload->control_optimum_gold_supply),
        static_cast<double>(market_gold),
        durability_decrease_rates_,
        payload->previous_day.durability_phase,
        durability_control_state_,
        durability_current_rate_);

    if (payload->previous_day.durability_phase != durability_control_state_) {
        SendNotiSetDurability();
    }

    cMyTrace trace(
        "int CAutoMarketConditionsControl::LoadDatabase(SIG_AUTO_MARKET_CONDITIONS_CONTROL*)",
        174,
        0);
    trace("LoadDatabase() %qd %u %d %d %d",
          market_gold,
          static_cast<unsigned int>(payload->control_optimum_gold_supply),
          gold_control_state_,
          item_control_state_,
          durability_control_state_);

    uint64_t over_gold = 0;
    if (market_gold >= payload->control_optimum_gold_supply) {
        over_gold = market_gold - payload->control_optimum_gold_supply;
    }

    if (G_CEnvironment()->get_channel_no() == 1) {
        DB_UpdateAutoMarketConditionsControl::makeRequest(
            payload->current_day.total_gold,
            payload->current_day.auction_gold,
            over_gold,
            payload->control_optimum_gold_supply,
            gold_control_state_,
            item_control_state_,
            durability_control_state_);
    }

    return 0;
}

// IDA_EA: 0x82F89BC
// IDA_NAME: _ZN28CAutoMarketConditionsControl21GetGoldDropAmountRateEii
// DEMANGLED: CAutoMarketConditionsControl::GetGoldDropAmountRate(int,int)
int CAutoMarketConditionsControl::GetGoldDropAmountRate(int charac_level,
                                                        int base_gold) {
    static int log_count = 0;

    if (!gold_control_state_) {
        return base_gold;
    }

    double rate =
        (1.0 - gold_current_rate_) *
            static_cast<long double>(
                gold_drop_amount_rate_per_level_[charac_level]) /
            -1000.0 +
        1.0;
    if (rate < 0.0 || rate > 1.0) {
        rate = gold_current_rate_;
    }

    const int gold = static_cast<int>(static_cast<long double>(base_gold) * rate);
    const bool should_log = (log_count % 1000) == 0;
    ++log_count;
    if (should_log) {
        cMyTrace trace(
            "int CAutoMarketConditionsControl::GetGoldDropAmountRate(int, int)",
            206,
            0);
        trace("goldDropAmountRate(%lf) gold(%d), baseGold(%d)",
              rate,
              gold,
              base_gold);
    }
    return gold;
}

// IDA_EA: 0x82F8AE8
// IDA_NAME: _ZN28CAutoMarketConditionsControl15GetItemDropRateEi
// DEMANGLED: CAutoMarketConditionsControl::GetItemDropRate(int)
float CAutoMarketConditionsControl::GetItemDropRate(int level_range) {
    static int log_count = 0;

    float rate = 1.0f;
    if (!item_control_state_) {
        return rate;
    }

    if (equipment_drop_level_ranges_.size() >
            static_cast<std::vector<int>::size_type>(level_range) &&
        level_range >= 0) {
        if (equipment_drop_level_ranges_[level_range]) {
            const double delta = 1.0 - item_current_rate_;
            rate = static_cast<float>(
                static_cast<long double>(
                    equipment_drop_level_ranges_[level_range]) *
                    delta / -1000.0 +
                1.0);
        }

        const bool should_log = (log_count % 1000) == 0;
        ++log_count;
        if (should_log) {
            cMyTrace trace("float CAutoMarketConditionsControl::GetItemDropRate(int)",
                           232,
                           0);
            trace("itemDropRate(%f), levelRange(%d), curRate(%lf)",
                  rate,
                  level_range,
                  item_current_rate_);
        }
        return rate;
    }

    cMyTrace trace("float CAutoMarketConditionsControl::GetItemDropRate(int)",
                   222,
                   0);
    trace("GetItemDropRate() error levelRange(%d)", level_range);
    return rate;
}

// IDA_EA: 0x82F8C8C
// IDA_NAME: _ZN28CAutoMarketConditionsControl16ChangeItemRarityEPiS0_id
// DEMANGLED: CAutoMarketConditionsControl::ChangeItemRarity(int *,int *,int,double)
bool CAutoMarketConditionsControl::ChangeItemRarity(int* item_rarity,
                                                    int* out_item_rarity,
                                                    int item_rarity_size,
                                                    double control_rarity) {
    static int log_count = 0;
    static int log_count2 = 0;

    if (!item_control_state_) {
        return false;
    }

    const bool should_log = (log_count % 1000) == 0;
    ++log_count;
    if (should_log) {
        cMyTrace trace(
            "bool CAutoMarketConditionsControl::ChangeItemRarity(int*, int*, int, double)",
            244,
            0);
        trace("ChangeItemRarity itemRarity[%d,%d,%d,%d], "
              "outItemRarity[%d,%d,%d,%d], itemRaritySize(%d), "
              "controlRarity(%lf)",
              item_rarity[0],
              item_rarity[1],
              item_rarity[2],
              item_rarity[3],
              out_item_rarity[0],
              out_item_rarity[1],
              out_item_rarity[2],
              out_item_rarity[3],
              item_rarity_size,
              control_rarity);
    }

    int cumulative = 0;
    for (int index = 2; index <= 5; ++index) {
        out_item_rarity[index] = item_rarity[index];
    }
    for (int index = 2; index >= 0; --index) {
        const int difference = item_rarity[index + 1] - item_rarity[index];
        cumulative += static_cast<int>(
            static_cast<long double>(difference) * control_rarity);
        out_item_rarity[index] = item_rarity[index] - cumulative;
        if (out_item_rarity[index] < 0) {
            out_item_rarity[index] = 0;
        }
    }

    const bool should_log2 = (log_count2 % 1000) == 0;
    ++log_count2;
    if (should_log2) {
        cMyTrace trace(
            "bool CAutoMarketConditionsControl::ChangeItemRarity(int*, int*, int, double)",
            276,
            0);
        trace("ChangeItemRarity outItemRarity[%d,%d,%d,%d]",
              out_item_rarity[0],
              out_item_rarity[1],
              out_item_rarity[2],
              out_item_rarity[3]);
    }

    return true;
}

// IDA_EA: 0x8109C14
// IDA_NAME: _ZN33CAutoMarketConditionsControlEventC2Ev
// DEMANGLED: CAutoMarketConditionsControlEvent::CAutoMarketConditionsControlEvent(void)
CAutoMarketConditionsControlEvent::CAutoMarketConditionsControlEvent()
    : CEventBase() {}

// IDA_EA: 0x8109C30
// IDA_NAME: _ZN33CAutoMarketConditionsControlEventD2Ev
// DEMANGLED: CAutoMarketConditionsControlEvent::~CAutoMarketConditionsControlEvent()
// IDA_EA: 0x8109C60
// IDA_NAME: _ZN33CAutoMarketConditionsControlEventD0Ev
// DEMANGLED: CAutoMarketConditionsControlEvent::~CAutoMarketConditionsControlEvent() deleting destructor
CAutoMarketConditionsControlEvent::~CAutoMarketConditionsControlEvent() {}

// IDA_EA: 0x8109C7E
// IDA_NAME: _ZN33CAutoMarketConditionsControlEvent10StartEventEv
// DEMANGLED: CAutoMarketConditionsControlEvent::StartEvent(void)
void CAutoMarketConditionsControlEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("StartEvent", 19, 0);
    trace("CAutoMarketConditionsControlEvent Start!");
}

// IDA_EA: 0x8109CD0
// IDA_NAME: _ZN33CAutoMarketConditionsControlEvent10StartEventE10Word_Param
// DEMANGLED: CAutoMarketConditionsControlEvent::StartEvent(Word_Param)
void CAutoMarketConditionsControlEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    DB_LoadAutoMarketConditionsControl::makeRequest(true);
}

// IDA_EA: 0x8109CF8
// IDA_NAME: _ZN33CAutoMarketConditionsControlEvent8EndEventEv
// DEMANGLED: CAutoMarketConditionsControlEvent::EndEvent(void)
void CAutoMarketConditionsControlEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 31, 0);
    trace("CAutoMarketConditionsControlEvent End!");
}

// IDA_EA: 0x863E8CE
// IDA_NAME: _ZN23Timer_CheckAuctionReadyD2Ev
// DEMANGLED: Timer_CheckAuctionReady::~Timer_CheckAuctionReady()
// IDA_EA: 0x863E8FE
// IDA_NAME: _ZN23Timer_CheckAuctionReadyD0Ev
// DEMANGLED: Timer_CheckAuctionReady::~Timer_CheckAuctionReady() deleting destructor
Timer_CheckAuctionReady::~Timer_CheckAuctionReady() {}

// IDA_EA: 0x863A808
// IDA_NAME: _ZN23Timer_CheckAuctionReady12dispatch_sigEiij
// DEMANGLED: Timer_CheckAuctionReady::dispatch_sig(int,int,unsigned int)
int Timer_CheckAuctionReady::dispatch_sig(int object_id,
                                          int param1,
                                          unsigned int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;

    if (static_cast<unsigned char>(
            GlobalData::s_auction_proxy->IsRunning()) != 1) {
        GlobalData::s_auction_proxy->SendCheckAuctionReadyPacket();
    }
    if (static_cast<unsigned char>(
            GlobalData::s_cera_auction_proxy->IsRunning()) != 1) {
        GlobalData::s_cera_auction_proxy->SendCheckCeraAuctionReadyPacket();
    }
    return 1;
}

// IDA_EA: 0x863A858
// IDA_NAME: _ZN23Timer_CheckAuctionReady15RegistNextTimerEv
// DEMANGLED: Timer_CheckAuctionReady::RegistNextTimer(void)
int Timer_CheckAuctionReady::RegistNextTimer() {
    TimerQueue* timer_queue = G_TimerQueue();
    return timer_queue->InsertTimer(2, 0, 152, 60, 0, 0);
}

// IDA_EA: 0x863CBB2
// IDA_NAME: _ZN29TimerVendingMachineItemNotifyC2Ev
// DEMANGLED: TimerVendingMachineItemNotify::TimerVendingMachineItemNotify(void)
TimerVendingMachineItemNotify::TimerVendingMachineItemNotify()
    : TimerSig() {}

// IDA_EA: 0x863F3D2
// IDA_NAME: _ZN29TimerVendingMachineItemNotifyD2Ev
// DEMANGLED: TimerVendingMachineItemNotify::~TimerVendingMachineItemNotify()
// IDA_EA: 0x863F402
// IDA_NAME: _ZN29TimerVendingMachineItemNotifyD0Ev
// DEMANGLED: TimerVendingMachineItemNotify::~TimerVendingMachineItemNotify() deleting destructor
TimerVendingMachineItemNotify::~TimerVendingMachineItemNotify() {}

// IDA_EA: 0x8637F40
// IDA_NAME: _ZN29TimerVendingMachineItemNotify12dispatch_sigEiij
// DEMANGLED: TimerVendingMachineItemNotify::dispatch_sig(int,int,unsigned int)
int TimerVendingMachineItemNotify::dispatch_sig(int user_uid,
                                                int item_id,
                                                unsigned int item_count) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 2);
    if (!user) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 86);
    packet.put_byte(3);
    packet.put_byte(1);
    packet.put_short(user->get_unique_id());
    packet.put_item_idx(
        static_cast<unsigned long>(static_cast<unsigned int>(item_id)));
    packet.put_byte(0);
    packet.put_int(static_cast<int>(item_count));
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 1;
}

// IDA_EA: 0x8638074
// IDA_NAME: _ZN29TimerVendingMachineItemNotify11registTimerEimjj
// DEMANGLED: TimerVendingMachineItemNotify::registTimer(int,unsigned long,unsigned int,unsigned int)
int TimerVendingMachineItemNotify::registTimer(int user_uid,
                                               unsigned long item_index,
                                               unsigned int item_count,
                                               unsigned int delay_seconds) {
    return G_TimerQueue()->InsertTimer(0,
                                       user_uid,
                                       10,
                                       static_cast<int>(delay_seconds),
                                       static_cast<unsigned int>(item_index),
                                       static_cast<int>(item_count));
}
