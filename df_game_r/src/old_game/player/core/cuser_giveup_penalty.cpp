#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x86786BE
// IDA_NAME: _ZN5CUser14giveup_panaltyEv
// DEMANGLED: CUser::giveup_panalty(void)
void CUser::giveup_panalty() {
    if (!IsLightServer() && !game_master_mode_ &&
        !isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12)) &&
        get_charac_level() > 17 &&
        !isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12)) &&
        getMoveSpace() != 1) {
        PacketGuard packet;
        packet.put_header(0, 33);
        setCurCharacStamina(10);
        packet.put_byte(10);
        packet.finalize(true);
        Send(packet);
    }
}
