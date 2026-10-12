#include "old_data/Login/inter_returns/inter_load_hero_mission_event_data.h"

#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x81622C6
// IDA_NAME: _ZN29InterLoadHeroMissionEventDataC2Ev
// DEMANGLED: InterLoadHeroMissionEventData::InterLoadHeroMissionEventData(void)
InterLoadHeroMissionEventData::InterLoadHeroMissionEventData()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::LOAD_HERO_MISSION_EVENT_DATA) {
}

// IDA_EA: 0x81622EA (D2), 0x816231A (D0)
// IDA_NAME: _ZN29InterLoadHeroMissionEventDataD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
InterLoadHeroMissionEventData::~InterLoadHeroMissionEventData() {
}

// IDA_EA: 0x8161C92
// IDA_NAME: _ZN29InterLoadHeroMissionEventData12dispatch_sigEP5CUserPci
// DEMANGLED: InterLoadHeroMissionEventData::dispatch_sig(CUser *,char *,int)
int InterLoadHeroMissionEventData::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    (void)payload_size;
    if (!payload) {
        cMyTrace trace(
            "virtual int InterLoadHeroMissionEventData::dispatch_sig(CUser*, char*, int)",
            511,
            5);
        trace("[HeroMission] msg is null.");
        return 512;
    }

    HeroMissionEvent* event = ARAD::Singleton<HeroMissionEvent>::Get();
    event->setEventData(user, payload + sizeof(unsigned int));
    if (user->IsFirstTimeLogin()) {
        ARAD::Singleton<HeroMissionEvent>::Get()->processMission(
            user,
            HeroMissionCondition::MissionNo::ATTENDANCE,
            0);
    }
    ARAD::Singleton<HeroMissionEvent>::Get()->sendMissionData(user);
    return 0;
}
