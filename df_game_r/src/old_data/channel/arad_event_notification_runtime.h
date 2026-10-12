#ifndef OLDSRV_OLD_DATA_CHANNEL_ARAD_EVENT_NOTIFICATION_RUNTIME_H_
#define OLDSRV_OLD_DATA_CHANNEL_ARAD_EVENT_NOTIFICATION_RUNTIME_H_

#include "old_data/channel/arad_server_state_message.h"

class CUser;

namespace ARAD {

// IDA_EA: 0x81978DD
// IDA_NAME: _ZN4ARAD23notifyOpenMessageDialogEP5CUseriiRK22AradServerStateMessage
// DEMANGLED: ARAD::notifyOpenMessageDialog(CUser *,int,int,AradServerStateMessage const&)
int notifyOpenMessageDialog(CUser* user,
                            int notify_type,
                            int event_code,
                            const AradServerStateMessage& state);

}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_CHANNEL_ARAD_EVENT_NOTIFICATION_RUNTIME_H_
