#ifndef OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_CHARACTER_SELECT_COMMANDS_H_
#define OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_CHARACTER_SELECT_COMMANDS_H_

#include <string>

#include "old_game/gm/core/game_master_runtime_abi.h"

namespace game_master {

// IDA unit: 04_character_select_create/game_master/CMoveCharacCmd.
class CMoveCharacCmd : public CCommand {
public:
    CMoveCharacCmd();

    virtual void parse();
    virtual void execute();

private:
    std::string character_name_;
};

// IDA unit: 04_character_select_create/game_master/CCharacStatCmd.
class CCharacStatCmd : public CCommand {
public:
    CCharacStatCmd();

    virtual void execute();
};

// IDA unit: 04_character_select_create/game_master/CChangeCharacName.
class CChangeCharacName : public CCommand {
public:
    CChangeCharacName();

    virtual void parse();
    virtual void execute();

private:
    std::string character_name_;
};

// IDA unit: 04_character_select_create/game_master/CAdjustCharacStat.
class CAdjustCharacStat : public CCommand {
public:
    CAdjustCharacStat();

    virtual void execute();
};

#if defined(__i386__)
static_assert(sizeof(CMoveCharacCmd) == 0x0cu,
              "CMoveCharacCmd must retain the old 12-byte ABI");
static_assert(sizeof(CCharacStatCmd) == 0x08u,
              "CCharacStatCmd must retain the old 8-byte ABI");
static_assert(sizeof(CChangeCharacName) == 0x0cu,
              "CChangeCharacName must retain the old 12-byte ABI");
static_assert(sizeof(CAdjustCharacStat) == 0x08u,
              "CAdjustCharacStat must retain the old 8-byte ABI");
#endif

}  // namespace game_master

#endif  // OLDSRV_OLD_GAME_GAME_MASTER_GAME_MASTER_CHARACTER_SELECT_COMMANDS_H_
