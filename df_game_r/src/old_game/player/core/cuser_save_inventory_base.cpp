#include "old_game/player/core/cuser.h"

#include "old_data/db/inventory/db_save_inven.h"
#include "old_game/economy/inventory/avatar_coin.h"
#include "common/manager/system_time.h"
#include "common/manager/global_data.h"

#include <stdint.h>


// IDA_EA: 0x86966B4
// IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacCoinEv
// DEMANGLED: CUserCharacInfo::getCurCharacCoin(void)const
int CUserCharacInfo::getCurCharacCoin() const {
    if (!current_charac_) {
        return 0;
    }
    return static_cast<int>(current_charac_->fields_.inventory_.coin_);
}

// IDA_EA: 0x86966E0
// IDA_NAME: _ZNK15CUserCharacInfo19getCurCharacPayCoinEv
// DEMANGLED: CUserCharacInfo::getCurCharacPayCoin(void)const
int CUserCharacInfo::getCurCharacPayCoin() const {
    if (!current_charac_) {
        return 0;
    }
    return static_cast<int>(current_charac_->fields_.inventory_.pay_coin_);
}

// IDA_EA: 0x869670C
// IDA_NAME: _ZNK15CUserCharacInfo21getCurCharacEventCoinEv
// DEMANGLED: CUserCharacInfo::getCurCharacEventCoin(void)const
int CUserCharacInfo::getCurCharacEventCoin() const {
    if (!current_charac_) {
        return 0;
    }
    return static_cast<int>(current_charac_->fields_.inventory_.event_coin_);
}

// IDA_EA: 0x864FD80
// IDA_NAME: _ZN5CUser17_GetSaveInvenBaseEP18SIG_SAVE_INVENTORY
// DEMANGLED: CUser::_GetSaveInvenBase(SIG_SAVE_INVENTORY *)
int CUser::_GetSaveInvenBase(SIG_SAVE_INVENTORY* payload) {
    payload->account_id = static_cast<unsigned int>(get_acc_id());
    payload->character_no = static_cast<unsigned int>(getCurCharacNo());
    payload->money = static_cast<unsigned int>(getCurCharacMoney());
    payload->coin = static_cast<unsigned int>(getCurCharacCoin());
    payload->pay_coin = static_cast<unsigned int>(getCurCharacPayCoin());
    payload->event_coin = static_cast<unsigned int>(getCurCharacEventCoin());

    const _Charac_info* const current_charac = getCurCharacR();
    if (current_charac) {
        const AvatarCoin* const avatar_coin =
            &current_charac->fields_.inventory_.avatar_coin_;
        payload->avatar_coin = static_cast<unsigned int>(avatar_coin->Get());
    } else {
        payload->avatar_coin = 0;
    }

    return this->setCurCharacLastPlayTick(
        static_cast<long>(GlobalData::system_time()->getCurSec()));
}
