#include "old_game/dungeon/instance/luck_point_runtime.h"

#include <cstdlib>

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8550A84
// IDA_NAME: _ZN10CLuckPoint12UseLuckPointEP5CUseriii
// DEMANGLED: CLuckPoint::UseLuckPoint(CUser *,int,int,int)
int CLuckPoint::UseLuckPoint(CUser* user,
                             int use_luck_point,
                             int rarity,
                             int item_grade) {
    if (!user) {
        return 0;
    }
    if (use_luck_point != 1) {
        return 0;
    }
    if (rarity != 2) {
        return 0;
    }

    const int charac_level = user->get_charac_level();
    const int level_difference = std::abs(item_grade - charac_level);
    const int deduction_rate =
        G_CDataManager()->get_server_parameter_script()
            ->getLuckPointDeductionRate(charac_level, level_difference);
    user->DecCurCharacLuckPoint(deduction_rate);
    return 1;
}
