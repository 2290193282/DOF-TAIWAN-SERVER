#include "old_game/activity/schedule/bingo_runtime.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/event/bingo_script.h"
#include "common/manager/mt_rand.h"

#include <bitset>
#include <ctime>

// IDA_EA: 0x80CA82C
// IDA_NAME: _ZN6CBingoC2Ev
// DEMANGLED: CBingo::CBingo(void)
CBingo::CBingo()
    : today_board_(),
      quiz_value_(0),
      random_(new CMTRand(static_cast<unsigned int>(std::time(0)))),
      today_(0) {
    today_board_.clear();
}

// IDA_EA: 0x80CA8F0
// IDA_NAME: _ZN6CBingoD2Ev
// DEMANGLED: CBingo::~CBingo()
CBingo::~CBingo() {}

// IDA_EA: 0x80CA936
// IDA_NAME: _ZN6CBingo8getTodayEv
// DEMANGLED: CBingo::getToday(void)
int CBingo::getToday() {
    return today_;
}

// IDA_EA: 0x80CA942
// IDA_NAME: _ZN6CBingo8getTodayEli
// DEMANGLED: CBingo::getToday(long,int)
int CBingo::getToday(std::time_t current_time, int reset_hour) {
    const std::time_t previous_time = current_time - 86400;
    struct tm current_tm;
    struct tm previous_tm;
    const struct tm* current = localtime_r(&current_time, &current_tm);
    const struct tm* previous = localtime_r(&previous_time, &previous_tm);
    if (current->tm_hour < 0 || current->tm_hour >= reset_hour) {
        today_ = current->tm_mday;
    } else {
        today_ = previous->tm_mday;
    }
    return today_;
}

// IDA_EA: 0x80CA9B8
// IDA_NAME: _ZN6CBingo10resetBoardEi
// DEMANGLED: CBingo::resetBoard(int)
int CBingo::resetBoard(int day) {
    stBingoScript* const script = G_CDataManager()->get_bingo_script();
    std::map<int, std::vector<int> >::iterator found =
        script->boards.find(day);
    if (found == script->boards.end()) {
        return 0;
    }
    today_board_.clear();
    today_board_ = found->second;
    return 1;
}

// IDA_EA: 0x80CAA64
// IDA_NAME: _ZN6CBingo9resetQuizEi
// DEMANGLED: CBingo::resetQuiz(int)
int CBingo::resetQuiz(int day) {
    stBingoScript* const script = G_CDataManager()->get_bingo_script();
    std::map<int, int>::iterator found = script->quiz_values.find(day);
    if (found == script->quiz_values.end()) {
        return 0;
    }
    quiz_value_ = found->second;
    return 1;
}

// IDA_EA: 0x80CAAFA
// IDA_NAME: _ZN6CBingo18getIndexTodayBoardEi
// DEMANGLED: CBingo::getIndexTodayBoard(int)
int CBingo::getIndexTodayBoard(int value) {
    if (value < 0) {
        return -1;
    }
    for (unsigned int index = 0; index < today_board_.size(); ++index) {
        if (today_board_[index] == value) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x80CAB5A
// IDA_NAME: _ZN6CBingo17calBoradMatchLineEi
// DEMANGLED: CBingo::calBoradMatchLine(int)
int CBingo::calBoradMatchLine(int board_bits) {
    int matched_lines = 0;
    std::bitset<25> board(static_cast<unsigned int>(board_bits));
    int diagonal_down = 0;
    int diagonal_up = 0;
    for (int row = 0; row <= 4; ++row) {
        int row_count = 0;
        int column_count = 0;
        for (int column = 0; column <= 4; ++column) {
            if (board[column + 5 * row]) {
                ++row_count;
            }
            if (board[row + 5 * column]) {
                ++column_count;
            }
            if (column + 1 > row_count && column + 1 > column_count) {
                break;
            }
        }
        if (row_count == 5) {
            ++matched_lines;
        }
        if (column_count == 5) {
            ++matched_lines;
        }
        if (board[6 * row]) {
            ++diagonal_down;
        }
        if (board[4 * (row + 1)]) {
            ++diagonal_up;
        }
    }
    if (diagonal_down == 5) {
        ++matched_lines;
    }
    if (diagonal_up == 5) {
        ++matched_lines;
    }
    return matched_lines;
}

// IDA_EA: 0x80CAD4C
// IDA_NAME: _ZN6CBingo11calNewValueERji
// DEMANGLED: CBingo::calNewValue(unsigned int &,int)
int CBingo::calNewValue(unsigned int& board_bits, int value) {
    const int index = getIndexTodayBoard(value);
    if (index == -1) {
        return 0;
    }
    if (index > 24) {
        return -1;
    }
    const unsigned int mask = 1u << index;
    if ((board_bits & mask) != 0) {
        return 0;
    }
    board_bits |= mask;
    return calBoradMatchLine(static_cast<int>(board_bits));
}

// IDA_EA: 0x80CADD4
// IDA_NAME: _ZN6CBingo12initUserDataER9BingoData
// DEMANGLED: CBingo::initUserData(BingoData &)
int CBingo::initUserData(BingoData& data) {
    data.line_count = static_cast<unsigned char>(
        calBoradMatchLine(static_cast<int>(data.bingo_count)));
    return 1;
}

// IDA_EA: 0x80CADFE
// IDA_NAME: _ZN6CBingo13resetUserDataER9BingoData
// DEMANGLED: CBingo::resetUserData(BingoData &)
BingoData* CBingo::resetUserData(BingoData& data) {
    return data.clear();
}

// IDA_EA: 0x80CAE12
// IDA_NAME: _ZN6CBingo12getQuizValueEv
// DEMANGLED: CBingo::getQuizValue(void)
int CBingo::getQuizValue() {
    return quiz_value_;
}

// IDA_EA: 0x80CAE1E
// IDA_NAME: _ZN6CBingo12getQuizIndexEv
// DEMANGLED: CBingo::getQuizIndex(void)
int CBingo::getQuizIndex() {
    for (unsigned int index = 0; index < today_board_.size(); ++index) {
        if (today_board_[index] == 0) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

// IDA_EA: 0x80CAE70
// IDA_NAME: _ZN6CBingo19getBingoRandomVauleEi
// DEMANGLED: CBingo::getBingoRandomVaule(int)
int CBingo::getBingoRandomVaule(int center_value) {
    const unsigned int spread = 9;
    const int value = center_value +
                      static_cast<int>(random_->randInt(spread)) - 9;
    return value > 0 && value <= 50 ? value : 0;
}

// IDA_EA: 0x80CAEC4
// IDA_NAME: _ZN6CBingo13getTodayBoardEv
// DEMANGLED: CBingo::getTodayBoard(void)
std::vector<int>* CBingo::getTodayBoard() {
    return &today_board_;
}
