#include "old_game/player/core/cuser.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/activity/condition/condition_event_manager_runtime.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8681820
// IDA_NAME: _ZN5CUser27SendProperDungeonClearCountEv
// DEMANGLED: CUser::SendProperDungeonClearCount(void)
void CUser::SendProperDungeonClearCount() {
    if (get_state() <= 2 || !getCurCharacR()) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 267);
    packet.put_short(static_cast<short>(GetProperDungeonClearCount()));
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x86818FC
// IDA_NAME: _ZN5CUser22SendConditionEventInfoEv
// DEMANGLED: CUser::SendConditionEventInfo(void)
void CUser::SendConditionEventInfo() {
    PacketGuard packet;
    packet.put_header(0, 234);

    CConditionEventManager* const manager =
        G_CGameManager()->GetConditionEventManager();
    if (!manager) {
        return;
    }

    packet.put_short(static_cast<short>(manager->GetCurEventIndex()));
    packet.put_byte(static_cast<char>(GetCurConditionEventStep()));
    packet.put_byte(static_cast<char>(GetCurConditionEventRewardStep()));
    packet.finalize(true);
    Send(packet);
}
