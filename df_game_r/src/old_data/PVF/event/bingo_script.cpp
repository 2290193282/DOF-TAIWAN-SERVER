#include "old_data/PVF/event/bingo_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <utility>
#include <vector>


typedef char stBingoScript_Size_Must_Be_0x54[
    (sizeof(stBingoScript) == 0x54) ? 1 : -1];

// IDA_EA: 0x83715B6
// IDA_NAME: _ZN13stBingoScriptC2Ev
// DEMANGLED: stBingoScript::stBingoScript(void)
stBingoScript::stBingoScript()
    : quiz_values(), boards(), rewards(), use_items() {
    quiz_values.clear();
    boards.clear();
    rewards.clear();
    use_items.clear();
}

// IDA_EA: 0x837DDFC
// IDA_NAME: _ZN13stBingoScriptD2Ev
// DEMANGLED: stBingoScript::~stBingoScript()
stBingoScript::~stBingoScript() {}

// IDA_EA: 0x89187B9
// IDA_NAME: _Z17ImportBingoScriptPKcR13stBingoScript
// DEMANGLED: ImportBingoScript(char const*,stBingoScript &)
int ImportBingoScript(const char* filename, stBingoScript& script) {
    if (loadRDARScriptFile("", filename)) {
        std::string tag;
        bool success = false;
        while (ScanType(tag, true) == 1) {
            if (tag == "[board]") {
                const int row_count = ScanInt(&success);
                if (!success) {
                    break;
                }

                for (int row = 0; row < row_count; ++row) {
                    const int board_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    std::vector<int> cells;
                    cells.clear();
                    for (int index = 0; index < 25; ++index) {
                        const int value = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        cells.push_back(value);
                    }
                    script.boards.insert(std::make_pair(board_id, cells));
                }
            } else if (tag == "[quiz value]") {
                const int row_count = ScanInt(&success);
                if (!success) {
                    break;
                }

                for (int row = 0; row < row_count; ++row) {
                    const int key = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    const int value = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script.quiz_values.insert(std::make_pair(key, value));
                }
            } else if (tag == "[reward]") {
                const int row_count = ScanInt(&success);
                if (!success) {
                    break;
                }

                for (int row = 0; row < row_count; ++row) {
                    std::pair<int, int> reward;
                    const int key = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    reward.first = ScanInt(&success);
                    if (!success) {
                        break;
                    }

                    reward.second = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script.rewards.insert(std::make_pair(key, reward));
                }
            } else if (tag == "[use item]") {
                while (1) {
                    const int item_id = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    script.use_items.push_back(item_id);
                }
            }
        }
    }
    return 0;
}
