#include "old_data/PVF/etc/novice_command_script.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>
#include <vector>

namespace {

enum NoviceCommandSlot {
    kSwordman = oldsrv::config::kCharacterJobSwordman,
    kFighter = oldsrv::config::kCharacterJobFighter,
    kGunner = oldsrv::config::kCharacterJobGunner,
    kMage = oldsrv::config::kCharacterJobMage,
    kPriest = oldsrv::config::kCharacterJobPriest,
    kAtGunner = oldsrv::config::kCharacterJobAtGunner,
    kThief = oldsrv::config::kCharacterJobThief,
    kAtFighter = oldsrv::config::kCharacterJobAtFighter,
};

void importCommandRows(std::vector<std::vector<int> >& target, int row_count) {
    std::vector<int> row;
    for (int row_index = 0; row_index < row_count; ++row_index) {
        row.clear();
        for (int column_index = 0; column_index <= 9; ++column_index) {
            row.push_back(ScanInt(static_cast<bool*>(0)));
        }
        target.push_back(row);
    }
}

}  // namespace

// IDA_EA: 0x890402A
// IDA_NAME: _Z25importNoviceCommandScriptPKcR15stNoviceCommand
// DEMANGLED: importNoviceCommandScript(char const*,stNoviceCommand &)
int importNoviceCommandScript(const char* filename, stNoviceCommand& command) {
    if (!loadRDARScriptFile("", filename)) {
        return 4556;
    }

    for (int slot = 0; slot < oldsrv::config::kCharacterJobCount; ++slot) {
        command.job_commands[slot].clear();
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[gunner]") {
            importCommandRows(command.job_commands[kGunner], 10);
        } else if (tag == "[fighter]") {
            importCommandRows(command.job_commands[kFighter], 8);
        } else if (tag == "[swordman]") {
            importCommandRows(command.job_commands[kSwordman], 8);
        } else if (tag == "[at fighter]") {
            importCommandRows(command.job_commands[kAtFighter], 8);
        } else if (tag == "[thief]") {
            importCommandRows(command.job_commands[kThief], 6);
        } else if (tag == "[mage]") {
            importCommandRows(command.job_commands[kMage], 10);
        } else if (tag == "[at gunner]") {
            importCommandRows(command.job_commands[kAtGunner], 10);
        } else if (tag == "[priest]") {
            importCommandRows(command.job_commands[kPriest], 10);
        }
    }
    return 1;
}
