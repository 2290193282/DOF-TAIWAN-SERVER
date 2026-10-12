#include "old_game/player/core/cuser.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/player/core/cuser_global_info_handle.h"

// IDA_EA: 0x8658910
// IDA_NAME: _ZN5CUser7log_outEv
// DEMANGLED: CUser::log_out(void)
CUser* CUser::log_out() {
    trade_count_ = 0;
    resetPlayExpAdd();
    set_multiboxLotteryItemFailCnt(0);

    const unsigned int account_id = static_cast<unsigned int>(get_acc_id());
    if (GlobalData::s_pGMAccounts_) {
        GlobalData::s_pGMAccounts_->removeGM(account_id, 3u);
    }

    history_stream_ = 0;
    state_ = 0;
    account_id_ = 0;

    CUserGlobalInfoHandleInstance()->reset_uniqueid_flag(
        static_cast<uint16_t>(get_unique_id()));
    set_unique_id(0);
    SetSlotIDX(0);
    SetIncreID(0);
    std::memset(account_name_, 0, sizeof(account_name_));
    connection_check_count_ = 0u;
    logout_state_byte_ = 0u;
    network_.disconnect();
    return reset();
}
