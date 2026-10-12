#ifndef OLDSRV_OLD_DATA_PVF_ETC_NOVICE_COMMAND_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_NOVICE_COMMAND_SCRIPT_H_

#include <vector>

#include "common/config.h"

// IDA layout used by importNoviceCommandScript @ 0x890402A.  Each slot is a
// vector of ten-command rows; the 0828 table retains all twelve job slots.
struct stNoviceCommand {
    std::vector<std::vector<int> >
        job_commands[oldsrv::config::kCharacterJobCount];
};

static_assert(sizeof(stNoviceCommand) == 0x90u,
              "stNoviceCommand 0828 i386 ABI size must stay 0x90");

// IDA_EA: 0x890402A
// IDA_NAME: _Z25importNoviceCommandScriptPKcR15stNoviceCommand
// DEMANGLED: importNoviceCommandScript(char const*,stNoviceCommand &)
int importNoviceCommandScript(const char* filename, stNoviceCommand& command);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_NOVICE_COMMAND_SCRIPT_H_
