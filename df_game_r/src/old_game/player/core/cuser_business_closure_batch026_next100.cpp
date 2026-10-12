#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8690EB8
// IDA_NAME: _ZN5CUser12setMoveSpaceEN9UserSpace1TE
// DEMANGLED: CUser::setMoveSpace(UserSpace::T)
CUser* CUser::setMoveSpace(UserSpace::T space) {
    move_space_ = static_cast<uint32_t>(space);
    return this;
}

// IDA_EA: 0x868E9C0
// IDA_NAME: _ZNK5CUser22isCompetitionMercenaryEv
// DEMANGLED: CUser::isCompetitionMercenary(void)const
bool CUser::isCompetitionMercenary() const {
    if (!getCurCharacR()) {
        return false;
    }

    const unsigned int charac_no =
        static_cast<unsigned int>(getCurCharacNo());
    const std::vector<CHARAC_LOAD_MERCENARY>& mercenaries =
        mercenary_info_.mercenaries_;
    for (std::vector<CHARAC_LOAD_MERCENARY>::const_iterator it =
             mercenaries.begin();
         it != mercenaries.end(); ++it) {
        if (it->charac_no != charac_no) {
            continue;
        }
        const int now = GlobalData::s_systemTime_.getCurSec();
        if (static_cast<int>(it->end_time) - now > 0) {
            return true;
        }
        return false;
    }
    return false;
}

// IDA_EA: 0x8690FCC
// IDA_NAME: _ZN5CUser24sendBlueMarbleEnterCountEv
// DEMANGLED: CUser::sendBlueMarbleEnterCount(void)
void CUser::sendBlueMarbleEnterCount() {
    PacketGuard packet;
    packet.put_header(0, 436);
    packet.put_byte(static_cast<unsigned char>(getBlueMarbleEnterCount()));
    packet.finalize(1);
    Send(packet);
}
