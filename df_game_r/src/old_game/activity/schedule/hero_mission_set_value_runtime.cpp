#include "old_game/activity/schedule/hero_mission_runtime.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8167216
// IDA_NAME: _ZN16HeroMissionEvent15setMissionValueEP5CUserN20HeroMissionCondition9MissionNo1TEj
// DEMANGLED: HeroMissionEvent::setMissionValue(
//     CUser *,HeroMissionCondition::MissionNo::T,unsigned int)
void HeroMissionEvent::setMissionValue(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no,
    unsigned int value) {
    if (IsEventing(0) != 1) {
        return;
    }

    if (!user) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::setMissionValue(CUser*, HeroMissionCondition::MissionNo::T, unsigned int)",
            0x2a6,
            "[HeroMission] User is null.");
        return;
    }

    HeroMissionValue* mission_value = 0;
    if (findMissionValue(user, mission_no, mission_value)) {
        mission_value->value = static_cast<uint16_t>(value);
    }
    sendMissionData(user);
}
