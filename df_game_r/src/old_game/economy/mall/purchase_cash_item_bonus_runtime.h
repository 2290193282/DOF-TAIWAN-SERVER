#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PURCHASE_CASH_ITEM_BONUS_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PURCHASE_CASH_ITEM_BONUS_RUNTIME_H_

#include <stdint.h>

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_game/activity/schedule/event_value_types.h"

// IDA unit: 10_mail_trade_auction_cash/PurchaseCashItemBonusScript.
// The script keeps the old event base, a database table name, the allowed
// cash-item indexes, and one typed mail action for each purchase-count rule.
struct PurchaseCashItemBonusScript : public InGameEventScript {
    struct PurchaseBonusRule {
        PurchaseBonusRule();
        PurchaseBonusRule* reset();

        int32_t purchase_start_count;
        int32_t purchase_end_count;
        int32_t purchase_multiple_number;
    };

    PurchaseCashItemBonusScript();

    std::string db_table_name;
    std::set<unsigned int> item_indexes;
    std::vector<std::pair<PurchaseBonusRule, Action_SendMail> > bonus_rules;
};

int importPurchaseCashItemBonusEventScript(
    PurchaseCashItemBonusScript* script,
    const char* filename);

typedef char PurchaseCashItemBonusScript_PurchaseBonusRule_Runtime_Size_Must_Be_0x0c[
    (sizeof(PurchaseCashItemBonusScript::PurchaseBonusRule) == 0x0c) ? 1 : -1];
typedef char PurchaseCashItemBonusScript_Runtime_Size_Must_Be_0x30[
    (sizeof(PurchaseCashItemBonusScript) == 0x30) ? 1 : -1];

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_PURCHASE_CASH_ITEM_BONUS_RUNTIME_H_
