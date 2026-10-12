#include "old_game/player/core/cuser.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x868E704
// IDA_NAME: _ZN5CUser22SetMercenaryInfoPacketER11PacketGuard
// DEMANGLED: CUser::SetMercenaryInfoPacket(PacketGuard &)
bool CUser::SetMercenaryInfoPacket(PacketGuard& packet) {
    if (CalcurateManageLevel()) {
        SendManageLevelUpNoti();
    }

    packet.put_byte(GetManageLevel());
    packet.put_int(GetManagePoint());
    packet.put_byte(static_cast<int>(mercenary_info_.mercenaries_.size()));

    for (std::vector<CHARAC_LOAD_MERCENARY>::size_type index = 0;
         index < mercenary_info_.mercenaries_.size(); ++index) {
        CHARAC_LOAD_MERCENARY& mercenary = mercenary_info_.mercenaries_[index];
        const int remaining_time = static_cast<int>(mercenary.end_time) -
                                   GlobalData::s_systemTime_.getCurSec();

        packet.put_int(static_cast<int>(mercenary.charac_no));
        const size_t name_length = std::strlen(mercenary.name);
        packet.put_int(static_cast<int>(name_length));
        if (name_length != 0u) {
            packet.put_str(mercenary.name, static_cast<int>(name_length));
        }

        if (remaining_time <= 0) {
            packet.put_byte(mercenary.competition_area == 0xffu ? 0 : 2);
        } else {
            packet.put_byte(1);
        }
        packet.put_int(remaining_time);
        packet.put_byte(mercenary.competition_area);
        packet.put_byte(mercenary.competition_period);
        packet.put_byte(mercenary.avatar_bonus_index);
    }

    // In the original loop the carried result is the final size > index
    // predicate, hence false after the full serialization completes.
    return false;
}
