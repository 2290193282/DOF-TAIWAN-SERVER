#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8684C28
// IDA_NAME: _ZN15cUserHistoryLog23Enchant3rdChronicleItemEbiiii
// DEMANGLED: cUserHistoryLog::Enchant3rdChronicleItem(bool,int,int,int,int)
int cUserHistoryLog::Enchant3rdChronicleItem(bool succeeded,
                                              int target_item_index,
                                              int material_item_index,
                                              int enchant_type,
                                              int applied_enchant_count) {
    return owner_->LogHistory(
        succeeded ? "3rdEnchant+,%d,%d,%d,%d" : "3rdEnchant-,%d,%d,%d,%d",
        target_item_index,
        material_item_index,
        enchant_type,
        applied_enchant_count);
}

// IDA_EA: 0x8684CA0
// IDA_NAME: _ZN15cUserHistoryLog11UpgradeItemEiiib
// DEMANGLED: cUserHistoryLog::UpgradeItem(int,int,int,bool)
int cUserHistoryLog::UpgradeItem(int item_index,
                                 int old_upgrade,
                                 int new_upgrade,
                                 bool succeeded) {
    return succeeded
        ? owner_->LogHistory(
              "Upgrade+,%d,%d,%d", item_index, old_upgrade, new_upgrade)
        : owner_->LogHistory(
              "Upgrade-,%d,%d,%d", item_index, old_upgrade, new_upgrade);
}

// IDA_EA: 0x8684D74
// IDA_NAME: _ZN15cUserHistoryLog11EnchantItemEiiibPKcS1_
// DEMANGLED: cUserHistoryLog::EnchantItem(int,int,int,bool,char const*,char const*)
int cUserHistoryLog::EnchantItem(int card_item_id,
                                 int old_enchant,
                                 int new_enchant,
                                 bool succeeded,
                                 const char* account_name,
                                 const char* character_name) {
    return succeeded
        ? owner_->LogHistory(
              "Enchant+,%d,%d,%d,\"%s\",\"%s\"",
              card_item_id,
              old_enchant,
              new_enchant,
              account_name,
              character_name)
        : owner_->LogHistory(
              "Enchant-,%d,%d,%d,\"%s\",\"%s\"",
              card_item_id,
              old_enchant,
              new_enchant,
              account_name,
              character_name);
}

// IDA_EA: 0x8686490
// IDA_NAME: _ZN15cUserHistoryLog6DiePvPE23ENUM_PVP_HISTORY_REASONjjiiiiPKc
// DEMANGLED: cUserHistoryLog::DiePvP(ENUM_PVP_HISTORY_REASON,unsigned int,unsigned int,int,int,int,int,char const*)
int cUserHistoryLog::DiePvP(ENUM_PVP_HISTORY_REASON reason,
                            unsigned int damage_value,
                            unsigned int damage_count,
                            int attacker_stat0,
                            int attacker_stat1,
                            int attacker_stat2,
                            int attacker_stat3,
                            const char* killer_name) {
    return owner_->LogHistory("PVP,%d,%u,%u,%d,%d,%d,%d,\"%s\"",
                              static_cast<int>(reason),
                              damage_value,
                              damage_count,
                              attacker_stat0,
                              attacker_stat1,
                              attacker_stat2,
                              attacker_stat3,
                              killer_name);
}

// IDA_EA: 0x8686C7C
// IDA_NAME: _ZN15cUserHistoryLog10apply_ItemEii16eApplyItemReason
// DEMANGLED: cUserHistoryLog::apply_Item(int,int,eApplyItemReason)
int cUserHistoryLog::apply_Item(int item_index,
                                int add_info,
                                eApplyItemReason reason) {
    if (!owner_) {
        return 0;
    }
    return owner_->LogHistory(
        "ApplyItem ,%d,%d,%d", item_index, add_info,
        static_cast<int>(reason));
}
