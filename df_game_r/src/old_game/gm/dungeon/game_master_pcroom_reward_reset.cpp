#include "old_game/gm/dungeon/game_master_pcroom_reward_reset.h"

#include "infra/log/format/trace.h"
#include "old_data/db/event/db_pcroom_daily_reward.h"
#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"
#include "old_game/player/core/cuser.h"

#include "common/manager/global_tools.h"

namespace game_master {

// IDA_EA: 0x82A89E2
// IDA_NAME: _ZN11game_master18CPcroomRewardResetC2Ev
// DEMANGLED: game_master::CPcroomRewardReset::CPcroomRewardReset(void)
CPcroomRewardReset::CPcroomRewardReset() : CCommand() {}

// IDA_EA: 0x84B3D12
// IDA_NAME: _ZN11game_master18CPcroomRewardReset5parseEv
// DEMANGLED: game_master::CPcroomRewardReset::parse(void)
void CPcroomRewardReset::parse() {
    // 0x84B3D12 returns parser_ only; it has no parse-time mutation.
}

// IDA_EA: 0x84B3D20
// IDA_NAME: _ZN11game_master18CPcroomRewardReset7executeEv
// DEMANGLED: game_master::CPcroomRewardReset::execute(void)
void CPcroomRewardReset::execute() {
    CUser* const user = GetUser();
    // expanded +0x8ef82 is the named PcroomDailyRewardRemain field.  Keep the
    // old value 4 and persist/synchronize it in the original order.
    user->setPcroomDailyRewardRemain(4);
    DBUpdatePcroomDailyReward::makeRequest(user);
    InterSelectPcroomDailyReward::SendPacket(user);

    cMyTrace trace("virtual void game_master::CPcroomRewardReset::execute()",
                   6226,
                   0);
    trace("[GMCommand] Pcroom daily reward reset m_id:%s",
          NumberToString(
              static_cast<unsigned int>(user->get_acc_id()), 0));
}

}  // namespace game_master
