#include "old_game/gm/core/game_master_mobile_restrict.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_game/player/core/cuser.h"

namespace game_master {

CMobileRestrictCommand::CMobileRestrictCommand() : CCommand() {}

void CMobileRestrictCommand::parse() {
    if (!parser_) {
        return;
    }
    target_ = parser_->token_vector()[1];
    restricted_ = StringToNumber(parser_->token_vector()[2].c_str()) != 0;
}

void CMobileRestrictCommand::execute() {
    // 0x84B3C60 treats "character" as a recognized no-op and ignores every
    // other selector.  Only "mobile" mutates the verified Garena auth byte.
    if (target_ != "mobile") {
        return;
    }

    CUser* const user = GetUser();
    user->getGarenaAuthData()->mobile_auth = restricted_ ? 1 : 0;

    cMyTrace trace("virtual void game_master::CMobileRestrictCommand::execute()",
                   6205,
                   0);
    trace("[GMCommand] Set mobile restricted. mobile auth:%s",
          restricted_ ? "yes" : "no");
}

}  // namespace game_master
