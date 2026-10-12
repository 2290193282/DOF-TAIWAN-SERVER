#include "old_game/gm/core/game_master_hero_mission.h"

#include "common/manager/global_tools.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_game/activity/schedule/hero_mission_runtime.h"

namespace game_master {

// IDA_EA: 0x82A89FE
// IDA_NAME: _ZN11game_master19CHeroMissionCommandC2Ev
// DEMANGLED: game_master::CHeroMissionCommand::CHeroMissionCommand(void)
CHeroMissionCommand::CHeroMissionCommand() : CCommand() {}

// IDA_EA: 0x84B3DB2
// IDA_NAME: _ZN11game_master19CHeroMissionCommand5parseEv
// DEMANGLED: game_master::CHeroMissionCommand::parse(void)
void CHeroMissionCommand::parse() {
    if (!parser_) {
        return;
    }
    operation_ = parser_->token_vector()[1];
    mission_no_ = static_cast<int>(
        StringToNumber(parser_->token_vector()[2].c_str()));
    value_ = StringToNumber(parser_->token_vector()[3].c_str());
}

// IDA_EA: 0x84B3E5E
// IDA_NAME: _ZN11game_master19CHeroMissionCommand7executeEv
// DEMANGLED: game_master::CHeroMissionCommand::execute(void)
void CHeroMissionCommand::execute() {
    CUser* const user = GetUser();

    if (operation_ == "set") {
        HeroMissionEvent* const event =
            ARAD::Singleton<HeroMissionEvent>::Get();
        event->processMission(
            user,
            static_cast<HeroMissionCondition::MissionNo::T>(mission_no_),
            value_);
    } else if (operation_ == "reset") {
        HeroMissionEvent* const event =
            ARAD::Singleton<HeroMissionEvent>::Get();
        event->setEventData(user, "");
    }
    HeroMissionEvent* const event = ARAD::Singleton<HeroMissionEvent>::Get();
    event->sendMissionData(user);
}

}  // namespace game_master
