#include "old_game/gm/core/game_master_premium_delete_command.h"

#include <vector>

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/premium_public_abi.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"

namespace game_master {

// IDA_EA: 0x82A8522
// IDA_NAME: _ZN11game_master18CUserPremiumDeleteC2Ev
// DEMANGLED: game_master::CUserPremiumDelete::CUserPremiumDelete(void)
CUserPremiumDelete::CUserPremiumDelete() : CCommand() {}

// IDA_EA: 0x84ABF4E
// IDA_NAME: _ZN11game_master18CUserPremiumDelete7executeEv
// DEMANGLED: game_master::CUserPremiumDelete::execute(void)
void CUserPremiumDelete::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    WongWork::CUserPremium* const premium = user->GetPremiumInfo();
    if (!premium) {
        return;
    }

    // 0x84ABF4E snapshots the active records before it mutates the table.
    // handleSetUserPremium changes an entry in-place, so processing the table
    // directly would skip records after the first mutation.
    std::vector<WongWork::SUserPremiumInfo> entries;
    for (int premium_type = 1; premium_type <= 105; ++premium_type) {
        const WongWork::SUserPremiumInfo* const entry =
            premium->GetPremiumInfo(premium_type);
        if (entry && entry->premium_type != 0) {
            entries.push_back(*entry);
        }
    }

    const long now = GlobalData::s_systemTime_.getCurSec();
    const long end_time = now + 1;
    for (std::vector<WongWork::SUserPremiumInfo>::const_iterator entry =
             entries.begin();
         entry != entries.end();
         ++entry) {
        int target_server = 0;
        if (entry->premium_type > 0 && entry->premium_type <= 105 &&
            g_SPremiumInfo[entry->premium_type].apply_target_server == 1) {
            target_server = G_CEnvironment()->get_server_group();
        }

        CStreamGuard guard(
            GlobalData::s_stream_pool->Acquire("GameMaster.cpp", 2441),
            true);
        guard << 84;
        guard << user->GetUID();
        guard << user->get_acc_id();
        guard << entry->premium_type;
        guard << static_cast<int>(now);
        guard << static_cast<int>(end_time);
        guard << entry->param;
        guard << target_server;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);

        WongWork::CHandlePremium::handleSetUserPremium(
            user,
            static_cast<ENUM_PREMIUM_TYPE>(entry->premium_type),
            now,
            end_time,
            entry->param,
            false);
        WongWork::CHandlePremium::handlePremiumEffect(
            user, static_cast<ENUM_PREMIUM_TYPE>(entry->premium_type));
        WongWork::CHandlePremium::handleNotifyPremiumInfo(
            user, entry->premium_type, static_cast<int>(end_time - now));
    }
}

}  // namespace game_master
