#include "old_game/social/party/party.h"

#include "old_game/player/core/cuser.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x859A34E
// IDA_NAME: _ZN6CParty27TimeoutCompleteAfterAssaultEv
// DEMANGLED: CParty::TimeoutCompleteAfterAssault(void)
int CParty::TimeoutCompleteAfterAssault() {
    if (recv_result_flag_ != 0) {
        recv_result_flag_ = 0;
        return 0;
    }

    if (recv_party_packet_all() != 1) {
        for (int seat = 0; seat <= 3; ++seat) {
            if (_checkValidUser(seat) == 1 &&
                result_recv_flag_.IsRecv(seat) != 1) {
                CUser* const user = get_user(seat);
                cMyTrace trace("bool CParty::TimeoutCompleteAfterAssault()",
                               506,
                               0);
                trace("CParty::TimeoutCompleteAfterAssault(index:%d, name:%s)",
                      GetPartyIndex(),
                      user->getCurCharacName());
                leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2));
            }
        }
    }
    return 1;
}
