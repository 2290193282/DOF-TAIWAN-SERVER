#include "old_game/activity/schedule/hero_mission_runtime.h"

#include "old_data/PVF/arad/arad_singleton.h"

namespace ARAD {

// IDA_EA: 0x811C9C1
// IDA_NAME: _ZN4ARAD9SingletonI16HeroMissionEventE6GetPtrEv
// DEMANGLED: ARAD::Singleton<HeroMissionEvent>::GetPtr(void)
template <>
HeroMissionEvent* Singleton<HeroMissionEvent>::GetPtr() {
    return Singleton<HeroMissionEvent>::Get();
}

}  // namespace ARAD

// IDA_EA: 0x81679AC
// IDA_NAME: _ZN16HeroMissionEvent15setMissionForGmEP5CUserN20HeroMissionCondition9MissionNo1TEj
// DEMANGLED: HeroMissionEvent::setMissionForGm(
//     CUser *,HeroMissionCondition::MissionNo::T,unsigned int)
int HeroMissionEvent::setMissionForGm(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no,
    unsigned int value) {
    HeroMissionValue* mission_value = 0;
    if (!findMissionValue(user, mission_no, mission_value)) {
        return 0;
    }

    std::map<HeroMissionCondition::MissionNo::T,
             BaseHeroMissionCondition*>::iterator condition =
        conditions_.find(mission_no);
    if (condition == conditions_.end()) {
        return 1;
    }

    mission_value->progress = static_cast<uint16_t>(value);
    return static_cast<int>(reinterpret_cast<intptr_t>(mission_value));
}

// IDA_EA: 0x8167A3A
// IDA_NAME: _ZN16HeroMissionEvent17resetMissionForGmEP5CUserN20HeroMissionCondition9MissionNo1TE
// DEMANGLED: HeroMissionEvent::resetMissionForGm(
//     CUser *,HeroMissionCondition::MissionNo::T)
HeroMissionValue* HeroMissionEvent::resetMissionForGm(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no) {
    HeroMissionValue* mission_value = 0;
    if (!findMissionValue(user, mission_no, mission_value)) {
        return 0;
    }

    std::map<HeroMissionCondition::MissionNo::T,
             BaseHeroMissionCondition*>::iterator condition =
        conditions_.find(mission_no);
    if (condition == conditions_.end()) {
        // The old pointer-returning body propagates the map equality flag in
        // EAX on this path, which is integer value 1 rather than null.
        return reinterpret_cast<HeroMissionValue*>(static_cast<intptr_t>(1));
    }

    return mission_value->reset();
}
