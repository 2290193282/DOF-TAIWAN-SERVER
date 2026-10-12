#pragma once

#include "old_game/appearance/avatar/avatar_roulette.h"

#include <string>


// IDA_EA: 0x89115B1
// IDA_NAME: _Z30importAvatarRouletteListDetailR14AvatarRouletteRSs17ENUM_CHARACTERJOB
// DEMANGLED: importAvatarRouletteListDetail(AvatarRoulette&,std::string&,ENUM_CHARACTERJOB)
int importAvatarRouletteListDetail(AvatarRoulette& roulette,
                                   std::string& tag,
                                   ENUM_CHARACTERJOB job);

// IDA_EA: 0x89116EB
// IDA_NAME: _Z24importAvatarRouletteListPcR14AvatarRoulette
// DEMANGLED: importAvatarRouletteList(char*,AvatarRoulette&)
int importAvatarRouletteList(char* filename, AvatarRoulette& roulette);
