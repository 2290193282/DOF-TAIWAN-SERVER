#include "old_game/player/core/timer_creature_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8635E96
// IDA_NAME: _ZN13TimerCreature12dispatch_sigEiij
// DEMANGLED: TimerCreature::dispatch_sig(int,int,unsigned int)
int TimerCreature::dispatch_sig(int user_uid,
                                int unique_id,
                                unsigned int charac_no) {
    CUser* const user = G_CGameManager()->getUser(user_uid, 0);
    if (!user) {
        return 0;
    }

    // The binary compares the zero-extended 16-bit user token directly to
    // the int timer parameter.  Do not truncate the timer parameter first.
    if (static_cast<unsigned short>(user->get_unique_id()) != unique_id) {
        return 0;
    }
    if (static_cast<unsigned int>(user->getCurCharacNo()) != charac_no) {
        return 0;
    }

    const int was_save_inven = user->isSaveInven();
    CInventory* const inventory = user->getCurCharacInvenW();
    inventory->GetCreatureMgrW()->CheckDiedCreature();
    if (was_save_inven != 1) {
        user->disableSaveInven();
    }
    return 1;
}
