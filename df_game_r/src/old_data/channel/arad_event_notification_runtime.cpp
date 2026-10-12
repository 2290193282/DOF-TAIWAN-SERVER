#include "old_data/channel/arad_event_notification_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace ARAD {

// IDA_EA: 0x81978DD
// IDA_NAME: _ZN4ARAD23notifyOpenMessageDialogEP5CUseriiRK22AradServerStateMessage
// DEMANGLED: ARAD::notifyOpenMessageDialog(CUser *,int,int,AradServerStateMessage const&)
int notifyOpenMessageDialog(CUser* user,
                            int notify_type,
                            int event_code,
                            const AradServerStateMessage& state) {
    if (!user) {
        return 0;
    }

    PacketGuard packet;
    // The original body obtains the server time immediately after C2.  The
    // value is not added to the server-state notification payload; retain
    // that observable call without inventing an extra wire field.
    (void)GlobalData::s_systemTime_.getCurSec();
    packet.put_header(0, ENUM_NOTIPACKET_SERVER_STATE_MESSAGE);
    packet.put_int(notify_type);
    packet.put_int(event_code);
    packet.put_binary(reinterpret_cast<const char*>(&state),
                      static_cast<int>(sizeof(state)));
    packet.finalize(true);
    return user->Send(packet);
}

}  // namespace ARAD
