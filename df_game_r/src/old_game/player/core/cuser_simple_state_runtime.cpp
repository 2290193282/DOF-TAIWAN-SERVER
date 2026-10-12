#include "old_game/player/core/cuser.h"
#include "old_game/player/core/inter_set_secoun_user_data.h"
#include "old_game/player/core/cuser.h"
#include "core/config/env/environment.h"
#include "old_data/db/anti_cheat/db_auto_punished_second_user_data.h"
#include "old_data/db/anti_cheat/db_insert_auto_punish_second_user.h"
#include "common/manager/global_data.h"
#include "old_game/economy/equipment/item_option.h"
#include "common/manager/user.h"
#include "old_data/Login/security/auto_punish_hack_type_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <map>


// IDA_EA: 0x822F084
// IDA_NAME: _ZN15cUserHistoryLog12SetItemXYPosEtt
// DEMANGLED: cUserHistoryLog::SetItemXYPos(unsigned short,unsigned short)
cUserHistoryLog* cUserHistoryLog::SetItemXYPos(uint16_t item_x,
                                                uint16_t item_y) {
    item_x_ = item_x;
    item_y_ = item_y;
    return this;
}

// IDA_EA: 0x8684DFA
// IDA_NAME: _ZN15cUserHistoryLog9CharacDieEPKctt
// DEMANGLED: cUserHistoryLog::CharacDie(char const*,unsigned short,unsigned short)
int cUserHistoryLog::CharacDie(const char* character_name,
                               uint16_t death_type,
                               uint16_t death_detail) {
    return owner_->LogHistory(
        "Die,\"%s\",%d,%d", character_name, death_type, death_detail);
}

// IDA_EA: 0x80F9968
// IDA_NAME: _ZN5CUser6SetETCEsi
// DEMANGLED: CUser::SetETC(short,int)
int CUser::SetETC(short key, int value) {
    etc_values_[key] = value;
    return value;
}

// IDA_EA: 0x84EC850
// IDA_NAME: _ZN5CUser24SetVillageAttackedRewardEb
// DEMANGLED: CUser::SetVillageAttackedReward(bool)
CUser* CUser::SetVillageAttackedReward(bool reward) {
    village_attacked_reward_ = reward;
    return this;
}

// IDA_EA: 0x822FE3E
// IDA_NAME: _ZN5CUser13GetTradeSpaceEv
// DEMANGLED: CUser::GetTradeSpace(void)
int CUser::GetTradeSpace() {
    return trade_space_index_;
}

// IDA_EA: 0x85320B4
// IDA_NAME: _ZN5CUser13SetTradeSpaceEi
// DEMANGLED: CUser::SetTradeSpace(int)
int CUser::SetTradeSpace(int trade_space) {
    this->lock();
    trade_space_index_ = static_cast<int16_t>(trade_space);
    return this->unlock();
}

// IDA_EA: 0x822FD3C
// IDA_NAME: _ZNK5CUser10isHackUserEv
// DEMANGLED: CUser::isHackUser(void)const
int CUser::isHackUser() const {
    return hack_user_flag_;
}

// IDA_EA: 0x822FD4C
// IDA_NAME: _ZNK5CUser15isHackUserTimerEv
// DEMANGLED: CUser::isHackUserTimer(void)const
int CUser::isHackUserTimer() const {
    return hack_user_timer_;
}

// IDA_EA: 0x8532078
// IDA_NAME: _ZNK5CUser25getTotalHackUserTradeGoldEv
// DEMANGLED: CUser::getTotalHackUserTradeGold(void)const
unsigned long long CUser::getTotalHackUserTradeGold() const {
    return total_hack_user_trade_gold_;
}

// IDA_EA: 0x853208C
// IDA_NAME: _ZNK5CUser24getTotalHackUserTradeCntEv
// DEMANGLED: CUser::getTotalHackUserTradeCnt(void)const
int CUser::getTotalHackUserTradeCnt() const {
    return static_cast<int>(total_hack_user_trade_count_);
}

// IDA_EA: 0x864658A
// IDA_NAME: _ZN5CUser18IncrementTradeGoldEji
// DEMANGLED: CUser::IncrementTradeGold(unsigned int,int)
char CUser::IncrementTradeGold(unsigned int gold, int count_over_trade_gold) {
    if (count_over_trade_gold) {
        const bool was_below_threshold =
            GetOverTradeGoldCount() <= 0x98967f;
        SetOverTradeGoldCount(
            gold + GetOverTradeGoldCount());
        if (was_below_threshold &&
            GetOverTradeGoldCount() > 0x98967f) {
            CStreamGuard guard(
                GlobalData::stream_pool()->Acquire("user.cpp", 1032), true);
            guard << 208;
            guard << this->GetUID();
            guard << this->get_acc_id();
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        }
    }

    if (!this->getCurCharacR()) {
        return 1;
    }
    return static_cast<char>(reinterpret_cast<uintptr_t>(
        IncrementCurCharacTradeGoldDaily(gold)));
}

// IDA_EA: 0x853209A
// IDA_NAME: _ZN5CUser13incTradeCountEv
// DEMANGLED: CUser::incTradeCount(void)
CUser* CUser::incTradeCount() {
    ++trade_count_;
    return this;
}

// IDA_EA: 0x8689890
// IDA_NAME: _ZN5CUser18SetTradePunishTypeEi
// DEMANGLED: CUser::SetTradePunishType(int)
CUser* CUser::SetTradePunishType(int punish_type) {
    printf(" CUser::SetTradePunishType(%d) \n", punish_type);
    trade_punish_type_ |= punish_type;
    return this;
}

// IDA_EA: 0x868991C
// IDA_NAME: _ZN5CUser17isTradePunishTypeEi
// DEMANGLED: CUser::isTradePunishType(int)
int CUser::isTradePunishType(int punish_type) {
    const int current = trade_punish_type_;
    return (punish_type == 0 && current != 0) ||
           (current & punish_type) != 0;
}

// IDA_EA: 0x86972B6
// IDA_NAME: _ZN5CUser25incTotalHackUserTradeGoldEy
// DEMANGLED: CUser::incTotalHackUserTradeGold(unsigned long long)
unsigned long long CUser::incTotalHackUserTradeGold(
    unsigned long long amount) {
    total_hack_user_trade_gold_ += amount;
    return total_hack_user_trade_gold_;
}

// IDA_EA: 0x86972EE
// IDA_NAME: _ZN5CUser24incTotalHackUserTradeCntEj
// DEMANGLED: CUser::incTotalHackUserTradeCnt(unsigned int)
CUser* CUser::incTotalHackUserTradeCnt(unsigned int count) {
    total_hack_user_trade_count_ += count;
    return this;
}

// IDA_EA: 0x8697448
// IDA_NAME: _ZN5CUser17isLoadingHackGoldEv
// DEMANGLED: CUser::isLoadingHackGold(void)
int CUser::isLoadingHackGold() {
    return loading_hack_gold_;
}

// IDA_EA: 0x8697458
// IDA_NAME: _ZN5CUser15setLoadHackGoldEb
// DEMANGLED: CUser::setLoadHackGold(bool)
CUser* CUser::setLoadHackGold(bool loading) {
    loading_hack_gold_ = static_cast<unsigned char>(loading);
    return this;
}

// IDA_EA: 0x84ECB1A
// IDA_NAME: _ZN5CUser19getAcccountHackInfoEv
// DEMANGLED: CUser::getAcccountHackInfo(void)
Secu_AccountHacking* CUser::getAcccountHackInfo() {
    return &account_hacking_;
}

// IDA_EA: 0x867FBBA
// IDA_NAME: _ZN5CUser21gainMoneyFromHackUserEjyjy
// DEMANGLED: CUser::gainMoneyFromHackUser(unsigned int,unsigned long long,unsigned int,unsigned long long)
void CUser::gainMoneyFromHackUser(unsigned int acc_id,
                                  unsigned long long trade_gold,
                                  unsigned int trade_count,
                                  unsigned long long other_trade_gold) {
    if (isLoadingHackGold() == 1) {
        incTotalHackUserTradeGold(trade_gold + other_trade_gold);
        incTotalHackUserTradeCnt(trade_count);
        if (GlobalData::g_autoPunishRuleHackTypeMgr->checkTotalTradeRuleOver(
                static_cast<unsigned int>(getTotalHackUserTradeCnt()),
                getTotalHackUserTradeGold())) {
            SetTradePunishType(8);
        }
        DB_InsertAutoPunishSecondUser::makeRequest(
            static_cast<unsigned int>(this->get_acc_id()),
            acc_id,
            trade_gold,
            isTradePunishType(8) != 0);
        return;
    }

    setLoadHackGold(true);
    DB_AutoPunishedSecondUserData::makeRequest(
        static_cast<unsigned int>(this->get_acc_id()),
        trade_gold,
        static_cast<unsigned int>(this->GetUID()),
        acc_id);
}

// IDA_EA: 0x8532036
// IDA_NAME: _ZN15cUserHistoryLog9SetTraderEPKcS1_
// DEMANGLED: cUserHistoryLog::SetTrader(char const*,char const*)
char* cUserHistoryLog::SetTrader(const char* account_name,
                                 const char* character_name) {
    std::strncpy(trader_account_name_,
                 account_name,
                 sizeof(trader_account_name_));
    return std::strncpy(trader_character_name_,
                        character_name,
                        sizeof(trader_character_name_));
}

// IDA_EA: 0x868503E
// IDA_NAME: _ZN15cUserHistoryLog10TradeBeginEv
// DEMANGLED: cUserHistoryLog::TradeBegin(void)
int cUserHistoryLog::TradeBegin() {
    const int enabled = trader_account_name_[0];
    if (enabled) {
        return owner_->LogHistory(
            "Trade+,\"%s\",\"%s\"",
            trader_account_name_,
            trader_character_name_);
    }
    return enabled;
}

// IDA_EA: 0x8683D5A
// IDA_NAME: _ZN15cUserHistoryLog16TradeItemAddFailEii
// DEMANGLED: cUserHistoryLog::TradeItemAddFail(int,int)
int cUserHistoryLog::TradeItemAddFail(int item_id, int item_count) {
    return owner_->LogHistory(
        "Item!,%d,%d", item_id, item_count);
}

// IDA_EA: 0x8684652
// IDA_NAME: _ZN15cUserHistoryLog13AvatarItemAddEiiPKc20eAvatarItemAddReason
// DEMANGLED: cUserHistoryLog::AvatarItemAdd(int,int,char const*,eAvatarItemAddReason)
int cUserHistoryLog::AvatarItemAdd(int item_id,
                                   int avatar_ui_id,
                                   const char* ipg_no,
                                   eAvatarItemAddReason reason) {
    const int reason_value = static_cast<int>(reason);
    if (reason_value != 1 && reason_value != 3 && reason_value != 4) {
        return owner_->LogHistory("Avatar+,%d,%d,\"%s\",%d",
                                              item_id,
                                              avatar_ui_id,
                                              ipg_no,
                                              reason_value);
    }
    const int enabled = trader_account_name_[0];
    if (enabled) {
        return owner_->LogHistory(
            "Avatar+,%d,%d,%s,%d,\"%s\",\"%s\"",
            item_id,
            avatar_ui_id,
            ipg_no,
            reason_value,
            trader_account_name_,
            trader_character_name_);
    }
    return enabled;
}

// IDA_EA: 0x86846FA
// IDA_NAME: _ZN15cUserHistoryLog13AvatarItemDelEiiPKc20eAvatarItemDelReason
// DEMANGLED: cUserHistoryLog::AvatarItemDel(int,int,char const*,eAvatarItemDelReason)
int cUserHistoryLog::AvatarItemDel(int item_id,
                                   int avatar_ui_id,
                                   const char* ipg_no,
                                   eAvatarItemDelReason reason) {
    const int reason_value = static_cast<int>(reason);
    if (reason_value > 1) {
        return owner_->LogHistory("Avatar-,%d,%d,%s,%d",
                                              item_id,
                                              avatar_ui_id,
                                              ipg_no,
                                              reason_value);
    }
    const int enabled = trader_account_name_[0];
    if (enabled) {
        return owner_->LogHistory(
            "Avatar-,%d,%d,%s,%d,\"%s\",\"%s\"",
            item_id,
            avatar_ui_id,
            ipg_no,
            reason_value,
            trader_account_name_,
            trader_character_name_);
    }
    return enabled;
}

// IDA_EA: 0x8684F40
// IDA_NAME: _ZN15cUserHistoryLog15CreatureItemDelE10INVEN_TYPEiiii14eItemDelReason
// DEMANGLED: cUserHistoryLog::CreatureItemDel(INVEN_TYPE,int,int,int,int,eItemDelReason)
void cUserHistoryLog::CreatureItemDel(INVEN_TYPE inventory_type,
                                      int item_id,
                                      int add_info,
                                      int item_count,
                                      int item_type,
                                      eItemDelReason reason) {
    const int reason_value = static_cast<int>(reason);
    if (item_type == 5) {
        if (reason_value == 20 || reason_value == 21) {
            if (trader_account_name_[0]) {
                owner_->LogHistory(
                    "CTItem-,%d,%d,%d,%d,\"%s\",\"%s\"",
                    item_id,
                    add_info,
                    item_count,
                    reason_value,
                    trader_account_name_,
                    trader_character_name_);
            }
        } else {
            owner_->LogHistory("CTItem-,%d,%d,%d,%d",
                                           item_id,
                                           add_info,
                                           item_count,
                                           reason_value);
        }
        return;
    }

    Inven_Item empty_item;
    empty_item.reset();
    ItemDel(inventory_type, add_info, item_count, empty_item, reason);
}

// IDA_EA: 0x8684E3C
// IDA_NAME: _ZN15cUserHistoryLog15CreatureItemAddE10INVEN_TYPEiiii14eItemAddReason
// DEMANGLED: cUserHistoryLog::CreatureItemAdd(INVEN_TYPE,int,int,int,int,eItemAddReason)
void cUserHistoryLog::CreatureItemAdd(INVEN_TYPE inventory_type,
                                      int item_id,
                                      int add_info,
                                      int item_count,
                                      int item_type,
                                      eItemAddReason reason) {
    const int reason_value = static_cast<int>(reason);
    if (item_type == 5) {
        if (reason_value == 26 || reason_value == 28 || reason_value == 29) {
            if (trader_account_name_[0]) {
                owner_->LogHistory(
                    "CTItem+,%d,%d,%d,%d,\"%s\",\"%s\")",
                    item_id,
                    add_info,
                    item_count,
                    reason_value,
                    trader_account_name_,
                    trader_character_name_);
            }
        } else {
            owner_->LogHistory("CTItem+,%d,%d,%d,%d",
                                           item_id,
                                           add_info,
                                           item_count,
                                           reason_value);
        }
        return;
    }

    Inven_Item empty_item;
    empty_item.reset();
    ItemAdd(inventory_type, add_info, item_count, empty_item, reason);
}

// IDA_EA: 0x8682E84
// IDA_NAME: _ZN15cUserHistoryLog7ItemAddE10INVEN_TYPEiiRK10Inven_Item14eItemAddReason
// DEMANGLED: cUserHistoryLog::ItemAdd(INVEN_TYPE,int,int,Inven_Item const&,eItemAddReason)
void cUserHistoryLog::ItemAdd(INVEN_TYPE inventory_type,
                              int value_1,
                              int value_2,
                              const Inven_Item& item,
                              eItemAddReason reason) {
    const int reason_value = static_cast<int>(reason);
    const int item_add_info = item.add_info_;
    const int item_extend_info = item.extend_info_;
    const int item_id = item.item_id_;
    const uint16_t item_avatar_select = item.avatar_select_;
    const uint16_t item_amplify_value = item.amplify_option_.ability_value_;
    const ::RandomOption& random_option =
        item.chronicle_random_option_.random_option();

    const int reseal_count = static_cast<unsigned char>(item.GetReSealCount());
    const int upgrade = static_cast<unsigned char>(item.GetUpgrade());
    CUser* const user = owner_;

    if (reason_value == 1 || reason_value == 2) {
        if (trader_account_name_[0]) {
            CItemGloballyUniqueIdentifierPrint unique_id_printer;
            const char* const unique_id = unique_id_printer(item.globally_unique_id_);
            user->LogHistory(
                "Item+,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
                static_cast<int>(inventory_type),
                item_id,
                value_1,
                value_2,
                reason_value,
                trader_account_name_,
                trader_character_name_,
                upgrade,
                item_avatar_select,
                random_option.option_fields_[0].option_index_,
                random_option.option_fields_[0].first_value_,
                random_option.option_fields_[0].second_value_,
                random_option.option_fields_[1].option_index_,
                random_option.option_fields_[1].first_value_,
                random_option.option_fields_[1].second_value_,
                random_option.option_fields_[2].option_index_,
                random_option.option_fields_[2].first_value_,
                random_option.option_fields_[2].second_value_,
                random_option.initial_seed_.seed_,
                random_option.changed_option_.option_index_,
                random_option.changed_option_.first_value_,
                random_option.changed_option_.second_value_,
                random_option.changed_option_seed_.seed_,
                reseal_count,
                static_cast<int>(item.seal_or_state_),
                unique_id);
        }
        return;
    }

    CItemGloballyUniqueIdentifierPrint unique_id_printer;
    const char* const unique_id = unique_id_printer(item.globally_unique_id_);
    user->LogHistory(
        "Item+,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
        static_cast<int>(inventory_type),
        item_id,
        value_1,
        value_2,
        reason_value,
        getItemXPos(),
        getItemYPos(),
        item_add_info,
        item_extend_info,
        static_cast<int>(item.amplify_option_.ability_type_and_identification_),
        item_amplify_value,
        upgrade,
        item_avatar_select,
        random_option.option_fields_[0].option_index_,
        random_option.option_fields_[0].first_value_,
        random_option.option_fields_[0].second_value_,
        random_option.option_fields_[1].option_index_,
        random_option.option_fields_[1].first_value_,
        random_option.option_fields_[1].second_value_,
        random_option.option_fields_[2].option_index_,
        random_option.option_fields_[2].first_value_,
        random_option.option_fields_[2].second_value_,
        random_option.initial_seed_.seed_,
        random_option.changed_option_.option_index_,
        random_option.changed_option_.first_value_,
        random_option.changed_option_.second_value_,
        random_option.changed_option_seed_.seed_,
        reseal_count,
        static_cast<int>(item.seal_or_state_),
        unique_id);
}

// IDA_EA: 0x868340A
// IDA_NAME: _ZN15cUserHistoryLog7ItemDelE10INVEN_TYPEiiRK10Inven_Item14eItemDelReason
// DEMANGLED: cUserHistoryLog::ItemDel(INVEN_TYPE,int,int,Inven_Item const&,eItemDelReason)
void cUserHistoryLog::ItemDel(INVEN_TYPE inventory_type,
                              int value_1,
                              int value_2,
                              const Inven_Item& item,
                              eItemDelReason reason) {
    const int reason_value = static_cast<int>(reason);
    const int item_add_info = item.add_info_;
    const int item_extend_info = item.extend_info_;
    const int item_id = item.item_id_;
    const uint16_t item_avatar_select = item.avatar_select_;
    const uint16_t item_amplify_value = item.amplify_option_.ability_value_;
    const ::RandomOption& random_option =
        item.chronicle_random_option_.random_option();

    const int reseal_count = static_cast<unsigned char>(item.GetReSealCount());
    const int upgrade = static_cast<unsigned char>(item.GetUpgrade());
    CUser* const user = owner_;

    if (reason_value == 1 || reason_value == 2) {
        if (trader_account_name_[0]) {
            CItemGloballyUniqueIdentifierPrint unique_id_printer;
            const char* const unique_id = unique_id_printer(item.globally_unique_id_);
            user->LogHistory(
                "Item-,%d,%d,%d,%d,%d,\"%s\",\"%s\",%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
                static_cast<int>(inventory_type),
                item_id,
                value_1,
                value_2,
                reason_value,
                trader_account_name_,
                trader_character_name_,
                upgrade,
                item_avatar_select,
                random_option.option_fields_[0].option_index_,
                random_option.option_fields_[0].first_value_,
                random_option.option_fields_[0].second_value_,
                random_option.option_fields_[1].option_index_,
                random_option.option_fields_[1].first_value_,
                random_option.option_fields_[1].second_value_,
                random_option.option_fields_[2].option_index_,
                random_option.option_fields_[2].first_value_,
                random_option.option_fields_[2].second_value_,
                random_option.initial_seed_.seed_,
                random_option.changed_option_.option_index_,
                random_option.changed_option_.first_value_,
                random_option.changed_option_.second_value_,
                random_option.changed_option_seed_.seed_,
                reseal_count,
                static_cast<int>(item.seal_or_state_),
                unique_id);
        }
    } else {
        CItemGloballyUniqueIdentifierPrint unique_id_printer;
        const char* const unique_id = unique_id_printer(item.globally_unique_id_);
        user->LogHistory(
            "Item-,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%d,%d,%s",
            static_cast<int>(inventory_type),
            item_id,
            value_1,
            value_2,
            reason_value,
            item_add_info,
            item_extend_info,
            static_cast<int>(item.amplify_option_.ability_type_and_identification_),
            item_amplify_value,
            upgrade,
            item_avatar_select,
            random_option.option_fields_[0].option_index_,
            random_option.option_fields_[0].first_value_,
            random_option.option_fields_[0].second_value_,
            random_option.option_fields_[1].option_index_,
            random_option.option_fields_[1].first_value_,
            random_option.option_fields_[1].second_value_,
            random_option.option_fields_[2].option_index_,
            random_option.option_fields_[2].first_value_,
            random_option.option_fields_[2].second_value_,
            random_option.initial_seed_.seed_,
            random_option.changed_option_.option_index_,
            random_option.changed_option_.first_value_,
            random_option.changed_option_.second_value_,
            random_option.changed_option_seed_.seed_,
            reseal_count,
            static_cast<int>(item.seal_or_state_),
            unique_id);
    }
    if (reason_value == 8 && item.item_category_ == 1u) {
        const stAmplifyOption_t* const amplify = &item.amplify_option_;
        user->LogHistory("Compound-,%u,%d,%d,%d,%d,%d,%d,%d,",
                         static_cast<unsigned int>(item_id),
                         upgrade,
                         item.get_add_info(),
                         static_cast<int>(item.upgrade_separate_.bits_ & 0x1fu),
                         item_extend_info,
                         amplify->getAbilityType(),
                         amplify->getAbilityValue(),
                         amplify->GetLock());
    }
}

// IDA_EA: 0x868507C
// IDA_NAME: _ZN15cUserHistoryLog8TradeEndE15eTradeEndReasoniiii
// DEMANGLED: cUserHistoryLog::TradeEnd(eTradeEndReason,int,int,int,int)
int cUserHistoryLog::TradeEnd(eTradeEndReason reason,
                              int own_item_count,
                              int own_gold,
                              int peer_item_count,
                              int peer_gold) {
    const int enabled = trader_account_name_[0];
    if (!enabled) {
        return enabled;
    }
    const int own_total = own_item_count + own_gold;
    const int peer_total = peer_item_count + peer_gold;
    int imbalance = 0;
    if (10 * own_total >= peer_total) {
        if (10 * peer_total < own_total) {
            imbalance = 2;
        }
    } else {
        imbalance = 1;
    }
    return owner_->LogHistory(
        "Trade-,\"%s\",\"%s\",%d, %d, %d, %d",
        trader_account_name_,
        trader_character_name_,
        static_cast<int>(reason),
        own_total,
        peer_total,
        imbalance);
}

// IDA_EA: 0x8686D54
// IDA_NAME: _ZN15cUserHistoryLog20ChangeItemLimitCountEiiiii
// DEMANGLED: cUserHistoryLog::ChangeItemLimitCount(int,int,int,int,int)
CUser* cUserHistoryLog::ChangeItemLimitCount(int direction,
                                              int slot,
                                              int item_id,
                                              int value,
                                              int reason) {
    CUser* const user = owner_;
    if (user) {
        user->LogHistory("ChangeItemTradeLimitCount,%d,%d,%d,%d,%d",
                         direction,
                         slot,
                         item_id,
                         value,
                         reason);
    }
    return user;
}

// IDA_EA: 0x8649BDC
// IDA_NAME: _ZN5CUser23isDungeonClearHackCheckEv
// DEMANGLED: CUser::isDungeonClearHackCheck(void)
int CUser::isDungeonClearHackCheck() {
    return dungeon_clear_hack_check_ == 3;
}

// IDA_EA: 0x86802AA
// IDA_NAME: _ZN5CUser14onDungeonClearEb
// DEMANGLED: CUser::onDungeonClear(bool)
int CUser::onDungeonClear(bool cleared) {
    return cleared;
}

// IDA_EA: 0x8649B96
// IDA_NAME: _ZN5CUser24setDungeonClearHackCheckEb
// DEMANGLED: CUser::setDungeonClearHackCheck(bool)
CUser* CUser::setDungeonClearHackCheck(bool cleared) {
    dungeon_clear_hack_check_ |= cleared ? 1u : 2u;
    return this;
}

// IDA_EA: 0x867EC12
// IDA_NAME: _ZN5CUser6getSexEv
// DEMANGLED: CUser::getSex(void)
int CUser::getSex() {
    char* const ssn = GetSsnString();
    if (std::strlen(ssn) <= 12u) {
        return -1;
    }
    return ssn[6] != '2' && ssn[6] != '4';
}

// IDA_EA: 0x868491A
// IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEiiiPKcci
// DEMANGLED: cUserHistoryLog::EnterDungeon(int,int,int,char const*,char,int)
int cUserHistoryLog::EnterDungeon(int stage_index,
                                  int stage_difficulty,
                                  int enter_type,
                                  const char* member_names,
                                  char stage_kind,
                                  int reserved) {
    CUser* const user = owner_;
    return user->LogHistory(
        "AdvanceDungeonEnter,%d,%d,%d,%s,%d,%d",
        stage_index,
        stage_difficulty,
        enter_type,
        member_names,
        stage_kind,
        reserved);
}

// IDA_EA: 0x86840C4
// IDA_NAME: _ZN15cUserHistoryLog6LogoutEPKcS1_iiih
// DEMANGLED: cUserHistoryLog::Logout(char const*,char const*,int,int,int,unsigned char)
int cUserHistoryLog::Logout(const char* web_address,
                            const char* peer_ip,
                            int port,
                            int process_sequence,
                            int play_time,
                            unsigned char login_out_state) {
    static_cast<void>(web_address);
    CUser* const user = owner_;
    const int server_group = G_CEnvironment()->get_server_group();
    return user->LogHistory("IP-,%s,%d,%d,%d,%d,%d,%p",
                            peer_ip,
                            port,
                            process_sequence,
                            static_cast<int>(login_out_state),
                            server_group,
                            play_time,
                            user);
}

// IDA_EA: 0x86849A2
// IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEPKciS1_i
// DEMANGLED: cUserHistoryLog::EnterDungeon(char const*,int,char const*,int)
int cUserHistoryLog::EnterDungeon(const char* dungeon_name,
                                  int difficulty,
                                  const char* member_names,
                                  int map_count) {
    CUser* user = owner_;
    return user->LogHistory("DungeonEnter,\"%s\",%d,%d,%s",
                            dungeon_name,
                            difficulty,
                            map_count,
                            member_names);
}

// IDA_EA: 0x8684A16
// IDA_NAME: _ZN15cUserHistoryLog12EnterDungeonEPKci
// DEMANGLED: cUserHistoryLog::EnterDungeon(char const*,int)
int cUserHistoryLog::EnterDungeon(const char* dungeon_name, int difficulty) {
    CUser* user = owner_;
    return user->LogHistory("DungeonEnter,\"%s\",%d", dungeon_name, difficulty);
}

// IDA_EA: 0x8684968
// IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEiiPKci
// DEMANGLED: cUserHistoryLog::LeaveDungeon(int,int,char const*,int)
int cUserHistoryLog::LeaveDungeon(int stage_index,
                                  int stage_difficulty,
                                  const char* member_names,
                                  int stage_kind) {
    CUser* const user = owner_;
    return user->LogHistory("DungeonLeave,%d,%d,%d,%s",
                            stage_index,
                            stage_difficulty,
                            stage_kind,
                            member_names);
}

// IDA_EA: 0x86849DC
// IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEPKciS1_i
// DEMANGLED: cUserHistoryLog::LeaveDungeon(char const*,int,char const*,int)
int cUserHistoryLog::LeaveDungeon(const char* dungeon_name,
                                  int difficulty,
                                  const char* member_names,
                                  int map_count) {
    CUser* user = owner_;
    return user->LogHistory("DungeonLeave,\"%s\",%d,%d,%s",
                            dungeon_name,
                            difficulty,
                            map_count,
                            member_names);
}

// IDA_EA: 0x8684A42
// IDA_NAME: _ZN15cUserHistoryLog12LeaveDungeonEPKci
// DEMANGLED: cUserHistoryLog::LeaveDungeon(char const*,int)
int cUserHistoryLog::LeaveDungeon(const char* dungeon_name, int difficulty) {
    CUser* user = owner_;
    return user->LogHistory("DungeonLeave,\"%s\",%d", dungeon_name, difficulty);
}

// IDA_EA: 0x8685452
// IDA_NAME: _ZN15cUserHistoryLog7AuctionEP26SIG_AUCTION_LOG_MESSAGE_AG
// DEMANGLED: cUserHistoryLog::Auction(SIG_AUCTION_LOG_MESSAGE_AG *)
int cUserHistoryLog::Auction(SIG_AUCTION_LOG_MESSAGE_AG* message) {
    CUser* user = owner_;
    const int kind = message->kind;
    if (kind == 1) {
        return user->LogHistory(
            "Auction_reg_f,%llu,%d,%u,%hhu,%d,%hu,%u,%hhu",
            message->auction_id,
            message->item_id,
            message->item_count,
            message->item_grade,
            message->register_price,
            message->register_period,
            message->fee,
            message->fail_reason);
    }
    if (kind > 1) {
        if (kind == 2) {
            return user->LogHistory("Auction_bid_s,%llu,%d,%d,%d",
                                    message->auction_id,
                                    message->item_id,
                                    message->item_count,
                                    message->bid_price);
        }
        if (kind == 3) {
            return user->LogHistory("Auction_bid_f,%llu,%d,%d",
                                    message->auction_id,
                                    message->item_id,
                                    message->bid_price);
        }
    } else if (!kind) {
        return user->LogHistory("Auction_reg_s,%llu,%d,%u,%hhu,%d,%hu,%u",
                                message->auction_id,
                                message->item_id,
                                message->item_count,
                                message->item_grade,
                                message->register_price,
                                message->register_period,
                                message->fee);
    }
    return user->LogHistory("Auction_unknown,%d", message->kind);
}

// IDA_EA: 0x8685654
// IDA_NAME: _ZN15cUserHistoryLog12AuctionPointEP26SIG_AUCTION_LOG_MESSAGE_PG
// DEMANGLED: cUserHistoryLog::AuctionPoint(SIG_AUCTION_LOG_MESSAGE_PG *)
int cUserHistoryLog::AuctionPoint(SIG_AUCTION_LOG_MESSAGE_PG* message) {
    CUser* user = owner_;
    const int kind = message->kind;
    if (kind == 1) {
        return user->LogHistory("POINT_reg_f,%llu,%d,%u,%hhu,%d,%hu,%u",
                                message->auction_id,
                                message->item_id,
                                message->item_count,
                                message->item_grade,
                                message->register_price,
                                message->register_period,
                                message->fee);
    }
    if (kind > 1) {
        if (kind == 2) {
            return user->LogHistory("POINT_bid_s,%llu,%d,%d",
                                    message->auction_id,
                                    message->item_id,
                                    message->bid_price);
        }
        if (kind == 3) {
            return user->LogHistory("POINT_bid_f,%llu,%d,%d",
                                    message->auction_id,
                                    message->item_id,
                                    message->bid_price);
        }
    } else if (!kind) {
        return user->LogHistory("POINT_reg_s,%llu,%d,%u,%hhu,%d,%hu,%u",
                                message->auction_id,
                                message->item_id,
                                message->item_count,
                                message->item_grade,
                                message->register_price,
                                message->register_period,
                                message->fee);
    }
    return user->LogHistory("POINT_unknown,%d", message->kind);
}

// IDA_EA: 0x84EE618
// IDA_NAME: _ZN23Inter_SetSecounUserDataC2Ev
Inter_SetSecounUserData::Inter_SetSecounUserData() : ISigDispatcher() {}

// IDA_EA: 0x84F7176 / 0x84F71A6
Inter_SetSecounUserData::~Inter_SetSecounUserData() {}

// IDA_EA: 0x84E0972
// IDA_NAME: _ZN23Inter_SetSecounUserData12dispatch_sigEP5CUserPci
int Inter_SetSecounUserData::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;

    // The database payload is exactly the old overlapping 24-byte shape:
    // +20 u32, +0 u64, +16 u32, +8 u64.  Do not normalize it into a guessed
    // record or reorder these four ABI arguments.
    uint32_t account_id = 0;
    uint64_t trade_gold = 0;
    uint32_t trade_count = 0;
    uint64_t other_trade_gold = 0;
    std::memcpy(&account_id, payload + 20, sizeof(account_id));
    std::memcpy(&trade_gold, payload, sizeof(trade_gold));
    std::memcpy(&trade_count, payload + 16, sizeof(trade_count));
    std::memcpy(&other_trade_gold, payload + 8, sizeof(other_trade_gold));
    user->gainMoneyFromHackUser(account_id,
                                trade_gold,
                                trade_count,
                                other_trade_gold);
    return 0;
}
