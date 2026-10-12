#include "old_game/activity/schedule/hero_mission_runtime.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x816748E
// IDA_NAME:
// _ZN16HeroMissionEvent13rewardMissionEP5CUserN20HeroMissionCondition9MissionNo1TE
// DEMANGLED: HeroMissionEvent::rewardMission(
//     CUser *,HeroMissionCondition::MissionNo::T)
bool HeroMissionEvent::rewardMission(
    CUser* user,
    HeroMissionCondition::MissionNo::T mission_no) {
    if (IsEventing(0) != 1) {
        return false;
    }
    if (!user) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "bool HeroMissionEvent::rewardMission(CUser*, HeroMissionCondition::MissionNo::T)",
            0x2d8,
            "[HeroMission] User is null.");
        return false;
    }

    std::map<HeroMissionCondition::MissionNo::T,
             BaseHeroMissionCondition*>::iterator condition =
        conditions_.find(mission_no);
    if (condition == conditions_.end()) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "bool HeroMissionEvent::rewardMission(CUser*, HeroMissionCondition::MissionNo::T)",
            0x2df,
            "[HeroMission] Invalid mission. (charac:%u, mission:%d)",
            static_cast<unsigned int>(user->get_charac_no(-1)),
            static_cast<int>(mission_no));
        user->DisConnSig(static_cast<DISCONN_SIG>(62), true, 736);
        return false;
    }

    HeroMissionValue* value = 0;
    bool rewarded = false;
    bool trigger_total_mission = false;
    if (findMissionValue(user, mission_no, value)) {
        if (condition->second->getMissionType() == 3) {
            std::map<unsigned int, std::vector<HeroMissionValue> >::iterator
                values = values_by_charac_.find(
                    static_cast<unsigned int>(user->get_charac_no(-1)));
            rewarded = static_cast<TotalMission*>(condition->second)->
                rewardMission(user, *value, values->second);
        } else {
            const uint16_t old_reward_count = value->reward_count;
            rewarded = condition->second->rewardMission(user, *value);
            if (old_reward_count == 0 && value->reward_count != 0) {
                trigger_total_mission = true;
            }
        }
    }
    if (trigger_total_mission) {
        processMission(user, HeroMissionCondition::MissionNo::TOTAL, 1);
    }
    return rewarded;
}

// IDA_EA: 0x816770E
// IDA_NAME: _ZN16HeroMissionEvent15sendMissionDataEP5CUser
// DEMANGLED: HeroMissionEvent::sendMissionData(CUser *)
void HeroMissionEvent::sendMissionData(CUser* user) {
    if (IsEventing(0) != 1) {
        return;
    }
    if (!user) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::sendMissionData(CUser*)",
            0x307,
            "[HeroMission] Invalid user pointer. (processMission)");
        return;
    }

    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    std::map<unsigned int, std::vector<HeroMissionValue> >::iterator values =
        values_by_charac_.find(charac_no);
    if (values == values_by_charac_.end()) {
        LogManager::logFormat(
            1,
            "localtaiwan/Event/EventHeroMission.cpp",
            "void HeroMissionEvent::sendMissionData(CUser*)",
            0x30e,
            "[HeroMission] Not exist hero mission value. (charac:%u)",
            charac_no);
        return;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_HERO_MISSION_STATE);
    packet.put_short(static_cast<int>(values->second.size()));
    for (std::vector<HeroMissionValue>::const_iterator value =
             values->second.begin();
         value != values->second.end();
         ++value) {
        packet.put_short(static_cast<int>(value->mission_no));
        packet.put_short(value->progress);
        packet.put_short(value->state);
        packet.put_short(value->value);
    }
    packet.finalize(true);
    user->Send(packet);
}
