#include "old_game/player/core/cuser.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

// IDA_EA: 0x8689D74
// IDA_NAME: _ZN5CUser19CalLevelUpItemStateERcRiii
// DEMANGLED: CUser::CalLevelUpItemState(char &,int &,int,int)
int CUser::CalLevelUpItemState(char& level_up_item_state,
                               int& remaining_exp,
                               int minimum_level,
                               int maximum_level) {
    if (get_charac_level() < minimum_level ||
        get_charac_level() > maximum_level) {
        return 0;
    }

    level_up_item_state = 1;
    const int level_up_exp = get_level_up_exp(get_charac_level());
    const int character_level = get_charac_level();
    const int required_exp = G_CDataManager()->get_level_exp(character_level);
    remaining_exp = level_up_exp + required_exp - get_charac_exp();

    const int charac_no = getCurCharacNo();
    const char* const account_id = NumberToString(
        static_cast<unsigned int>(get_acc_id()), 0);
    if (remaining_exp >= 0) {
        cMyTrace trace(
            "bool CUser::CalLevelUpItemState(char&, int&, int, int)",
            41178,
            0);
        trace("Level_up_item:success value(%d) m_id(%s),charno(%u)",
              remaining_exp,
              account_id,
              static_cast<unsigned int>(charac_no));
        return 1;
    }

    cMyTrace trace(
        "bool CUser::CalLevelUpItemState(char&, int&, int, int)",
        41175,
        0);
    trace("Level_up_item:wrong value(%d) m_id(%s),charno(%u)",
          remaining_exp,
          account_id,
          static_cast<unsigned int>(charac_no));
    return 0;
}
