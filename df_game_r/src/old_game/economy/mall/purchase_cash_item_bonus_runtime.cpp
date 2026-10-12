#include "old_game/economy/mall/purchase_cash_item_bonus_runtime.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/PVF/event/action_mail_script.h"

// IDA_EA: 0x8985CE6
// IDA_NAME: _ZN27PurchaseCashItemBonusScript17PurchaseBonusRuleC2Ev
// DEMANGLED: PurchaseCashItemBonusScript::PurchaseBonusRule::PurchaseBonusRule(void)
PurchaseCashItemBonusScript::PurchaseBonusRule::PurchaseBonusRule() {
    reset();
}

// IDA_EA: 0x8985CFA
// IDA_NAME: _ZN27PurchaseCashItemBonusScript17PurchaseBonusRule5resetEv
// DEMANGLED: PurchaseCashItemBonusScript::PurchaseBonusRule::reset(void)
PurchaseCashItemBonusScript::PurchaseBonusRule*
PurchaseCashItemBonusScript::PurchaseBonusRule::reset() {
    purchase_start_count = -1;
    purchase_end_count = -1;
    purchase_multiple_number = -1;
    return this;
}

// IDA_EA: 0x8985D1C
// IDA_NAME: _ZN27PurchaseCashItemBonusScriptC2Ev
// DEMANGLED: PurchaseCashItemBonusScript::PurchaseCashItemBonusScript(void)
PurchaseCashItemBonusScript::PurchaseCashItemBonusScript()
    : InGameEventScript(),
      db_table_name(),
      item_indexes(),
      bonus_rules() {
    db_table_name.clear();
}

// IDA_EA: 0x89834A6
// IDA_NAME: _Z38importPurchaseCashItemBonusEventScriptP27PurchaseCashItemBonusScriptPKc
// DEMANGLED: importPurchaseCashItemBonusEventScript(PurchaseCashItemBonusScript *,char const*)
int importPurchaseCashItemBonusEventScript(
    PurchaseCashItemBonusScript* script,
    const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    unsigned int item_idx_start = 0;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[db table]") {
            ScanStr(&script->db_table_name);
        } else if (tag == "[item idx]") {
            while (true) {
                const int item_idx = ScanInt(&success);
                if (!success) {
                    break;
                }
                script->item_indexes.insert(
                    static_cast<unsigned int>(item_idx));
            }
        } else if (tag == "[item idx start]") {
            item_idx_start = static_cast<unsigned int>(
                ScanInt(static_cast<bool*>(0)));
        } else if (tag == "[item idx end]") {
            const unsigned int item_idx_end =
                static_cast<unsigned int>(ScanInt(&success));
            if (success && item_idx_start && item_idx_end) {
                while (item_idx_start <= item_idx_end) {
                    script->item_indexes.insert(item_idx_start);
                    ++item_idx_start;
                }
            }
        } else if (tag == "[send]") {
            PurchaseCashItemBonusScript::PurchaseBonusRule rule;
            while (ScanType(tag, true) == 1) {
                if (tag == "[purchase start count]") {
                    rule.purchase_start_count = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[purchase end count]") {
                    rule.purchase_end_count = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[purchase mutiple number]") {
                    rule.purchase_multiple_number = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                } else if (tag == "[send mail]") {
                    Action_SendMail mail;
                    mail.reset();
                    success = importActionMailScript(&mail) == 1;
                    if (!success) {
                        break;
                    }
                    if (rule.purchase_start_count != -1 &&
                        rule.purchase_end_count != -1 &&
                        rule.purchase_multiple_number != -1) {
                        script->bonus_rules.push_back(
                            std::make_pair(rule, mail));
                    }
                    rule.reset();
                }
            }
        }
    }

    return success ? 1 : 0;
}
