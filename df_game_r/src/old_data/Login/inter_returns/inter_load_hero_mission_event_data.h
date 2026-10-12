#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_HERO_MISSION_EVENT_DATA_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_HERO_MISSION_EVENT_DATA_H_

#include "common/manager/taiwan_interface_inter_dispatcher.h"

// Taiwan internal Inter route 18, consuming the typed 0x104-byte
// SigLoadHeroMissionData response sent by DBLoadHeroMissionEventData.
class InterLoadHeroMissionEventData : public TaiwanInterfaceInterDispatcher {
public:
    // IDA_EA: 0x81622C6
    // IDA_NAME: _ZN29InterLoadHeroMissionEventDataC2Ev
    InterLoadHeroMissionEventData();

    // IDA_EA: 0x81622EA (D2), 0x816231A (D0)
    // IDA_NAME: _ZN29InterLoadHeroMissionEventDataD2Ev / D0Ev
    virtual ~InterLoadHeroMissionEventData();

    // IDA_EA: 0x8161C92
    // IDA_NAME: _ZN29InterLoadHeroMissionEventData12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(InterLoadHeroMissionEventData) == 0x08u,
              "InterLoadHeroMissionEventData must retain the Taiwan Inter ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_HERO_MISSION_EVENT_DATA_H_
