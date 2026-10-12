#include "old_game/player/core/cuser.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"

// IDA_EA: 0x866AF1C
// IDA_NAME: _ZN5CUser10CheckMoneyEi
// DEMANGLED: CUser::CheckMoney(int)
bool CUser::CheckMoney(int money) {
    return getCurCharacMoney() + money <=
           G_CDataManager()->GetMoneyLimitPerLevel(get_charac_level(),
                                                    get_acc_name());
}
