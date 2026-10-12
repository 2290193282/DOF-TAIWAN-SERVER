#ifndef OLDSRV_OLD_GAME_EVENT_BINGO_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_BINGO_RUNTIME_H_

#include "old_game/activity/schedule/event_value_types.h"

#include <boost/shared_ptr.hpp>

#include <ctime>
#include <vector>

class CMTRand;
class CUser;
namespace StackableAction {
struct input_param;
}

class CBingo {
public:
    CBingo();
    ~CBingo();

    int getToday();
    int getToday(std::time_t current_time, int reset_hour);
    int resetBoard(int day);
    int resetQuiz(int day);
    int getIndexTodayBoard(int value);
    int calBoradMatchLine(int board_bits);
    int calNewValue(unsigned int& board_bits, int value);
    int initUserData(BingoData& data);
    BingoData* resetUserData(BingoData& data);
    int getQuizValue();
    int getQuizIndex();
    int getBingoRandomVaule(int center_value);
    std::vector<int>* getTodayBoard();

private:
    std::vector<int> today_board_;
    int quiz_value_;
    boost::shared_ptr<CMTRand> random_;
    int today_;
};

class BingoEvent : public CEventBase {
public:
    BingoEvent();
    virtual ~BingoEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();
    virtual void dailyresetData();

    boost::shared_ptr<CBingo>& getBingoSystem();
    int reqBingoMark(StackableAction::input_param& param);
    int reqBingoQuizMark(CUser& user, int quiz_value);
    int sendOneMatchLineReward(CUser& user);
    int sendMaxMatchLineReward(CUser& user);

private:
    boost::shared_ptr<CBingo> bingo_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(boost::shared_ptr<CMTRand>) == 0x08u,
              "CBingo random_ must retain the old Boost shared_ptr ABI");
static_assert(sizeof(boost::shared_ptr<CBingo>) == 0x08u,
              "BingoEvent bingo_ must retain the old Boost shared_ptr ABI");
static_assert(sizeof(CBingo) == 0x1Cu,
              "CBingo old ABI size must stay 0x1C");
static_assert(sizeof(BingoEvent) == 0x14u,
              "BingoEvent old ABI size must stay 0x14");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_BINGO_RUNTIME_H_
