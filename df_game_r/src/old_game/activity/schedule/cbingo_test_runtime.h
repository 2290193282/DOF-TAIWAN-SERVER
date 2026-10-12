#ifndef OLDSRV_OLD_GAME_EVENT_CBINGO_TEST_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_CBINGO_TEST_RUNTIME_H_

#include "old_game/activity/schedule/bingo_runtime.h"

#include <ctime>
#include <string>

// The old executable's standalone Bingo diagnostic derives directly from the
// real Bingo state object.  It has no additional data members.
class CBingoTest : public CBingo {
public:
    CBingoTest();
    ~CBingoTest();

    int run();
    std::time_t getTimeFromString(std::string value);
    void getToday();
    void resetBoard();
    void resetQuiz();
    void calBoradMatchLine();
    void calNewValue();
    bool getBingoRandomVaule();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CBingoTest) == sizeof(CBingo),
              "CBingoTest must not add state beyond the old CBingo base");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_CBINGO_TEST_RUNTIME_H_
