#include "old_game/player/core/timer_avatar_expire_runtime.h"

#include "core/config/env/environment.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x8632A78
// IDA_NAME: _ZN17TimerAvatarExpire12dispatch_sigEiij
// DEMANGLED: TimerAvatarExpire::dispatch_sig(int,int,unsigned int)
int TimerAvatarExpire::dispatch_sig(int object_id,
                                    int param1,
                                    unsigned int param2) {
    (void)object_id;
    (void)param1;
    (void)param2;
    return 1;
}

// IDA_EA: 0x8632A82
// IDA_NAME: _ZN17TimerAvatarExpire18CheckExpiredAvatarEP5CUser
// DEMANGLED: TimerAvatarExpire::CheckExpiredAvatar(CUser *)
int TimerAvatarExpire::CheckExpiredAvatar(CUser* user) {
    PacketGuard packet;

    if (user->get_state()) {
        if (!user->getCurCharacR()) {
            return 0;
        }

        CInventory* const inventory_r = user->getCurCharacInvenR();
        if (inventory_r->isAnyExpiredAvatarItem()) {
            CInventory* const inventory_w = user->getCurCharacInvenW();
            inventory_w->ExpiredAvatarItem(&packet);
        }
    }
    return 1;
}

// IDA_EA: 0x8632B32
// IDA_NAME: _ZN17TimerAvatarExpire11RegistTimerEP5CUser
// DEMANGLED: TimerAvatarExpire::RegistTimer(CUser *)
int TimerAvatarExpire::RegistTimer(CUser* user) {
    const int charac_no = user->getCurCharacNo();
    const unsigned int unique_id =
        static_cast<unsigned short>(user->get_unique_id());
    const int delay_seconds = 60 * G_CEnvironment()->get_avatar_time();
    const int slot_index = static_cast<short>(user->GetSlotIDX());
    G_TimerQueue()->InsertTimer(0,
                                slot_index,
                                81,
                                delay_seconds,
                                unique_id,
                                charac_no);
    return 1;
}
