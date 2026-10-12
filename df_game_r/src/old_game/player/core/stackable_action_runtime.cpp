#include "old_game/player/core/stackable_action_runtime.h"

#include <algorithm>
#include <utility>
#include <vector>

#include <boost/bind.hpp>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/schedule/bingo_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_lifecycle.h"

namespace {

typedef std::vector<std::pair<int, int> > DimensionResetTargets;

int ResolveDimensionResetTargets(StackableAction::input_param& param,
                                 DimensionResetTargets* targets) {
    if (!param.user || !param.stackable_item || !targets) {
        return 19;
    }

    const std::vector<int>& values = param.stackable_item->action_values();
    if (values.empty() || values[0] < 1 || values[0] > 6 ||
        values.size() != static_cast<std::size_t>(values[0] + 1)) {
        return 4;
    }

    bool used_slots[6] = {false, false, false, false, false, false};
    bool changes_any_counter = false;
    targets->clear();
    targets->reserve(static_cast<std::size_t>(values[0]));
    for (int value_index = 1; value_index <= values[0]; ++value_index) {
        CDungeon* const dungeon =
            G_CDataManager()->find_dungeon(values[value_index]);
        if (!dungeon) {
            targets->clear();
            return 19;
        }

        const int slot = dungeon->get_dimension_possible() - 1;
        if (slot < 0 || slot >= 6 || used_slots[slot]) {
            targets->clear();
            return 19;
        }
        used_slots[slot] = true;

        const int maximum = G_CDataManager()->get_dimensionInout(slot);
        if (maximum <= 0 || maximum > 127) {
            targets->clear();
            return 19;
        }
        if (param.user->getDemensionInoutValue(
                static_cast<char>(slot)) < maximum) {
            changes_any_counter = true;
        }
        targets->push_back(std::make_pair(slot, maximum));
    }
    return changes_any_counter ? 0 : 19;
}

}  // namespace

namespace StackableAction {

// IDA_EA: 0x827F6D8
// IDA_NAME: _ZN15StackableAction6ActionC2Ev
// DEMANGLED: StackableAction::Action::Action(void)
Action::Action()
    : check_functions_(),
      run_functions_() {
    init_function_type();
}

// IDA_EA: 0x827F760
// IDA_NAME: _ZN15StackableAction6ActionD2Ev
// DEMANGLED: StackableAction::Action::~Action()
Action::~Action() {}

// IDA_EA: 0x827F7C8
// IDA_NAME: _ZN15StackableAction6Action18init_function_typeEv
// DEMANGLED: StackableAction::Action::init_function_type(void)
int Action::init_function_type() {
    // The old constructor deliberately leaves the check table empty.  These
    // three run handlers are stored as boost::function values in insertion
    // order 52, 54, 55, which is distinct from accepting unknown action IDs.
    run_functions_[52] = boost::bind(&Action::proc_bingo, this, _1);
    run_functions_[54] = boost::bind(&Action::proc_lotteryCount, this, _1);
    run_functions_[55] = boost::bind(&Action::proc_addFatigue, this, _1);
    check_functions_[56] = boost::bind(
        &Action::check_initDimensionDungeonCount, this, _1);
    run_functions_[56] = boost::bind(
        &Action::proc_initDimensionDungeonCount, this, _1);
    return 0;
}

// IDA_EA: 0x827FA26
// IDA_NAME: _ZN15StackableAction6Action5checkEiRNS_11input_paramE
// DEMANGLED: StackableAction::Action::check(int,StackableAction::input_param &)
int Action::check(int type, input_param& param) {
    std::map<int, Handler>::iterator it = check_functions_.find(type);
    return it == check_functions_.end() ? 0 : it->second(param);
}

// IDA_EA: 0x827FA9A
// IDA_NAME: _ZN15StackableAction6Action3runEiRNS_11input_paramE
// DEMANGLED: StackableAction::Action::run(int,StackableAction::input_param &)
int Action::run(int type, input_param& param) {
    std::map<int, Handler>::iterator it = run_functions_.find(type);
    return it == run_functions_.end() ? 0 : it->second(param);
}

// IDA_EA: 0x827FB14
// IDA_NAME: _ZN15StackableAction6Action10proc_bingoERNS_11input_paramE
// DEMANGLED: StackableAction::Action::proc_bingo(StackableAction::input_param &)
int Action::proc_bingo(input_param& param) {
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(106);
    if (event->IsEventing(0) == 1) {
        BingoEvent* const bingo = static_cast<BingoEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(106));
        if (bingo) {
            return bingo->reqBingoMark(param);
        }
    }
    return 19;
}

// IDA_EA: 0x827FB86
// IDA_NAME: _ZN15StackableAction6Action17proc_lotteryCountERNS_11input_paramE
// DEMANGLED: StackableAction::Action::proc_lotteryCount(StackableAction::input_param &)
int Action::proc_lotteryCount(input_param& param) {
    if (!param.user || !param.stackable_item) {
        return 19;
    }
    param.user->giveLotteryCountItem(param.stackable_item->get_index(), false);
    return 0;
}

// IDA_EA: 0x827FBD6
// IDA_NAME: _ZN15StackableAction6Action15proc_addFatigueERNS_11input_paramE
// DEMANGLED: StackableAction::Action::proc_addFatigue(StackableAction::input_param &)
int Action::proc_addFatigue(input_param& param) {
    if (!param.user || !param.stackable_item) {
        return 19;
    }

    const std::vector<int>& values = param.stackable_item->action_values();
    if (values.empty()) {
        return 4;
    }

    const int amount = std::max(0, values.at(0));
    if (param.user->getCurCharacFatigue() < amount) {
        return 95;
    }

    param.user->setCurCharacFatigue(
        std::max(0, param.user->getCurCharacFatigue() - amount));
    param.user->SendFatigue();
    return 0;
}

int Action::check_initDimensionDungeonCount(input_param& param) {
    DimensionResetTargets targets;
    return ResolveDimensionResetTargets(param, &targets);
}

int Action::proc_initDimensionDungeonCount(input_param& param) {
    DimensionResetTargets targets;
    const int validation = ResolveDimensionResetTargets(param, &targets);
    if (validation != 0) {
        return validation;
    }

    // Resolve every dungeon and slot before the first mutation.  This keeps a
    // malformed mixed payload from partially resetting a character.  The
    // setters mark the existing task-335 DB owner dirty; normal UpdateData
    // persists the same six-column row used at login and daily reset.
    for (DimensionResetTargets::const_iterator target = targets.begin();
         target != targets.end(); ++target) {
        param.user->setDemensionInoutValue(
            static_cast<char>(target->first),
            static_cast<char>(target->second));
    }
    return 0;
}

}  // namespace StackableAction
