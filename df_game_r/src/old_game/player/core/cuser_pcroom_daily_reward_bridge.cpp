#include "old_game/player/core/cuser.h"

char CUser::getPcroomDailyRewardRemain() const {
    // The old CMD601 gate is `test al, al; jg`, so preserve the target's
    // signed-char interpretation rather than treating this as a byte count.
    return static_cast<char>(pcroom_daily_reward_remain_);
}

CUser* CUser::setPcroomDailyRewardRemain(char remain_reward_count) {
    pcroom_daily_reward_remain_ = static_cast<int8_t>(remain_reward_count);
    return this;
}

CUser* CUser::decrementPcroomDailyRewardRemain() {
    // CMD601 decrements exactly one byte after a successful insert.  Perform
    // the arithmetic in the byte domain to preserve the old x86 wraparound
    // behavior even if corrupt state reaches a signed-char boundary.
    pcroom_daily_reward_remain_ = static_cast<int8_t>(
        static_cast<uint8_t>(pcroom_daily_reward_remain_) - 1u);
    return this;
}
