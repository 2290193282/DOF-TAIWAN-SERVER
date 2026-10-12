#include "old_data/Login/inter_returns/inter_event_trigger.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"

// IDA_EA: 0x84CE5F4
// IDA_NAME: _ZN23Inter_TriggerEventStart12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TriggerEventStart::dispatch_sig(CUser *,char *,int)
int Inter_TriggerEventStart::dispatch_sig(CUser* /*user*/,
                                          char* payload,
                                          int /*payload_size*/) {
    Packet_Monitor_TriggerEventStart& packet =
        *reinterpret_cast<Packet_Monitor_TriggerEventStart*>(payload);
    if (!GlobalData::s_event_manager) {
        return 0;
    }
    return GlobalData::s_event_manager->TriggerEventStart(packet.event_id,
                                                           packet.parameter)
               ? 0
               : 10891;
}

// IDA_EA: 0x84EB1EE
// IDA_NAME: _ZN23Inter_TriggerEventStartC2Ev
// DEMANGLED: Inter_TriggerEventStart::Inter_TriggerEventStart(void)
Inter_TriggerEventStart::Inter_TriggerEventStart() : ISigDispatcher() {}

// IDA_EA: 0x84EB20A / 0x84EB23A
// IDA_NAME: _ZN23Inter_TriggerEventStartD2Ev / D0Ev
// DEMANGLED: Inter_TriggerEventStart::~Inter_TriggerEventStart()
Inter_TriggerEventStart::~Inter_TriggerEventStart() {}

// IDA_EA: 0x84CE646
// IDA_NAME: _ZN21Inter_TriggerEventEnd12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_TriggerEventEnd::dispatch_sig(CUser *,char *,int)
int Inter_TriggerEventEnd::dispatch_sig(CUser* /*user*/,
                                        char* payload,
                                        int /*payload_size*/) {
    Packet_Monitor_TriggerEventEnd& packet =
        *reinterpret_cast<Packet_Monitor_TriggerEventEnd*>(payload);
    if (!GlobalData::s_event_manager) {
        return 0;
    }
    return GlobalData::s_event_manager->TriggerEventEnd(packet.event_id) ? 0
                                                                          : 10906;
}

// IDA_EA: 0x84EB258
// IDA_NAME: _ZN21Inter_TriggerEventEndC2Ev
// DEMANGLED: Inter_TriggerEventEnd::Inter_TriggerEventEnd(void)
Inter_TriggerEventEnd::Inter_TriggerEventEnd() : ISigDispatcher() {}

// IDA_EA: 0x84EB274 / 0x84EB2A4
// IDA_NAME: _ZN21Inter_TriggerEventEndD2Ev / D0Ev
// DEMANGLED: Inter_TriggerEventEnd::~Inter_TriggerEventEnd()
Inter_TriggerEventEnd::~Inter_TriggerEventEnd() {}
