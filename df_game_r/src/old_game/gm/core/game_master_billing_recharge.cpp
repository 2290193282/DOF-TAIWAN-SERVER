#include "old_game/gm/core/game_master_billing_recharge.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/ipg_runtime.h"

namespace game_master {

CBillingRechargeCommand::CBillingRechargeCommand() : CCommand() {}

void CBillingRechargeCommand::parse() {
    if (!parser_) {
        return;
    }
    currency_ = parser_->token_vector()[1];
    value_ = StringToNumber(parser_->token_vector()[2].c_str());
}

void CBillingRechargeCommand::execute() {
    WongWork::IPG::CIPGHelper* const helper = GlobalData::s_pIPGHelper;
    CUser* const user = GetUser();

    // 0x84B3AD4 queues the exact IPG transaction first, then always requests
    // the current balance.  The strings and null optional fields are not
    // placeholders: they are the original cera input tuple.
    if (currency_ == "cera") {
        helper->IPGInput(user, 5, value_, "", "", "GM", 0, 0, 0);
    } else if (currency_ == "point") {
        helper->IPGInputPoint(user, value_, 4, 0, 0);
    }
    helper->IPGQuery(user);
}

}  // namespace game_master
