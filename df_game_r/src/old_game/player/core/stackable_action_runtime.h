#ifndef OLDSRV_OLD_GAME_USERINFO_STACKABLE_ACTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_STACKABLE_ACTION_RUNTIME_H_

#include <map>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "old_game/economy/inventory/inventory.h"

// IDA unit: 99_misc_global/StackableAction/Action.  This is the process-wide
// action router constructed by GlobalData::Init before packet dispatch is
// available.  The two std::map members are the original +0x00/+0x18 objects;
// action classes must not replace this routing table with a generic switch.
namespace StackableAction {

class Action : private boost::noncopyable {
public:
    typedef boost::function<int(input_param&)> Handler;

    // IDA_EA: 0x827F6D8 / 0x827F760.
    Action();
    ~Action();

    // IDA_EA: 0x827F7C8.
    int init_function_type();

    // IDA_EA: 0x827FA26 / 0x827FA9A.
    int check(int type, input_param& param);
    int run(int type, input_param& param);

    // IDA_EA: 0x827FB14 / 0x827FB86 / 0x827FBD6.
    int proc_bingo(input_param& param);
    int proc_lotteryCount(input_param& param);
    int proc_addFatigue(input_param& param);
    int check_initDimensionDungeonCount(input_param& param);
    int proc_initDimensionDungeonCount(input_param& param);

private:
    std::map<int, Handler> check_functions_;  // +0x00
    std::map<int, Handler> run_functions_;    // +0x18
};

#if defined(__i386__)
static_assert(sizeof(Action) == 0x30u,
              "StackableAction::Action must retain its 0x30-byte i386 ABI");
#endif

}  // namespace StackableAction

#endif  // OLDSRV_OLD_GAME_USERINFO_STACKABLE_ACTION_RUNTIME_H_
