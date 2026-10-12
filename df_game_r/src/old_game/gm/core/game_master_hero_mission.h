#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_GAME_MASTER_HERO_MISSION_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_GAME_MASTER_HERO_MISSION_H_

#include "old_game/gm/core/game_master_runtime_abi.h"

#include <string>

namespace game_master {

// IDA_EA: C2 0x82A89FE, parse 0x84B3DB2, execute 0x84B3E5E.
// CGameMasterMgr registers this object under the string command "heromission".
class CHeroMissionCommand : public CCommand {
public:
    // IDA_EA: 0x82A89FE
    // IDA_NAME: _ZN11game_master19CHeroMissionCommandC2Ev
    // DEMANGLED: game_master::CHeroMissionCommand::CHeroMissionCommand(void)
    CHeroMissionCommand();

    // IDA_EA: 0x84B3E5E
    // IDA_NAME: _ZN11game_master19CHeroMissionCommand7executeEv
    // DEMANGLED: game_master::CHeroMissionCommand::execute(void)
    virtual void execute();

    // IDA_EA: 0x84B3DB2
    // IDA_NAME: _ZN11game_master19CHeroMissionCommand5parseEv
    // DEMANGLED: game_master::CHeroMissionCommand::parse(void)
    virtual void parse();

private:
    std::string operation_;
    int mission_no_;
    unsigned int value_;
};

#if defined(__i386__)
static_assert(sizeof(CHeroMissionCommand) == 0x14,
              "CHeroMissionCommand must remain the old 20-byte command");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_GAME_MASTER_HERO_MISSION_H_
