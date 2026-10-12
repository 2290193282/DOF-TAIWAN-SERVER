#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "old_data/PVF/event/blue_marble_script.h"
#include "common/manager/mt_rand.h"

#include <ctime>
#include <functional>
#include <map>
#include <utility>

// IDA_EA: 0x80D6DD4
// IDA_NAME: _ZN10BlueMarble20checkStartBlueMarbleEv
// DEMANGLED: BlueMarble::checkStartBlueMarble(void)
int BlueMarble::checkStartBlueMarble() {
    if (getState() != 1) {
        return 0;
    }
    if (getWaitUserCount() >= script_->info_script.player_max) {
        return 1;
    }
    changeState(BlueMarbleState::kInitial);
    return 0;
}

// IDA_EA: 0x80D6F62
// IDA_NAME: _ZN10BlueMarble9throwDiceEi
// DEMANGLED: BlueMarble::throwDice(int)
void BlueMarble::throwDice(int user_slot) {
    setDiceNumber();
    sendBlueMarbleDiceInfo(user_slot);
}

// IDA_EA: 0x80D79EE
// IDA_NAME: _ZN10BlueMarble16finishBlueMarbleEi
// DEMANGLED: BlueMarble::finishBlueMarble(int)
void BlueMarble::finishBlueMarble(int user_slot) {
    setUserGrade(user_slot);
    changeState(static_cast<BlueMarbleState::T>(3));
    sendFinishBlueMarble();
}

// IDA_EA: 0x80D7A26
// IDA_NAME: _ZN10BlueMarble12setVisitZoneEi
// DEMANGLED: BlueMarble::setVisitZone(int)
int BlueMarble::setVisitZone(int board_zone) {
    if (visit_zones_.find(board_zone) != visit_zones_.end()) {
        return 0;
    }
    return visit_zones_.insert(board_zone).second;
}

// IDA_EA: 0x80D7A9A
// IDA_NAME: _ZNK10BlueMarble12getVisitZoneEi
// DEMANGLED: BlueMarble::getVisitZone(int)const
bool BlueMarble::getVisitZone(int board_zone) const {
    return visit_zones_.find(board_zone) != visit_zones_.end();
}

// IDA_EA: 0x80D7AFC
// IDA_NAME: _ZN10BlueMarble12setUserGradeEi
// DEMANGLED: BlueMarble::setUserGrade(int)
void BlueMarble::setUserGrade(int user_slot) {
    typedef std::multimap<int, int, std::greater<int> > Ranking;
    users_[user_slot].setGrade(BlueMarbleUserGrade::kInitial);

    Ranking ranking;
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (users_[slot].getUser() &&
            users_[slot].getGrade() != BlueMarbleUserGrade::kInitial) {
            ranking.insert(std::make_pair(users_[slot].getBoardZone(), slot));
        }
    }

    int tied_grade = 0;
    int next_grade = 1;
    int previous_zone = 0;
    for (Ranking::const_iterator it = ranking.begin(); it != ranking.end();
         ++it) {
        if (it->first == previous_zone) {
            previous_zone = it->first;
            users_[it->second].setGrade(
                static_cast<BlueMarbleUserGrade::T>(tied_grade));
        } else {
            previous_zone = it->first;
            users_[it->second].setGrade(
                static_cast<BlueMarbleUserGrade::T>(next_grade));
            tied_grade = next_grade++;
        }
    }
}

// IDA_EA: 0x80D7D5E
// IDA_NAME: _ZNK10BlueMarble9getWinnerEv
// DEMANGLED: BlueMarble::getWinner(void)const
int BlueMarble::getWinner() const {
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (users_[slot].getUser() &&
            users_[slot].getGrade() == BlueMarbleUserGrade::kInitial) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x80D8A5C
// IDA_NAME: _ZN10BlueMarbleC2Ev
// DEMANGLED: BlueMarble::BlueMarble(void)
BlueMarble::BlueMarble()
    : memory_pool_index_(-1),
      users_(),
      state_(0),
      randoms_(new CMTRand[4]),
      dice_number_(0),
      map_index_(0),
      visit_zones_(),
      script_(0),
      max_board_size_(0) {
    const unsigned int seed = static_cast<unsigned int>(std::time(0));
    for (int index = 0; index <= 3; ++index) {
        randoms_[index].seed(seed);
    }
    users_.clear();
    visit_zones_.clear();
}

// IDA_EA: 0x80D8BAE
// IDA_NAME: _ZN10BlueMarbleD2Ev
// DEMANGLED: BlueMarble::~BlueMarble()
BlueMarble::~BlueMarble() {
    delete[] randoms_;
    randoms_ = 0;
}

// IDA_EA: 0x80D8C3E
// IDA_NAME: _ZN10BlueMarble11setUserInfoEP23BlueMarbleScriptManager
// DEMANGLED: BlueMarble::setUserInfo(BlueMarbleScriptManager *)
void BlueMarble::setUserInfo(BlueMarbleScriptManager* script) {
    BlueMarbleUserInfo info;
    info.setScript(script);
    users_.push_back(info);
}

// IDA_EA: 0x80D8CA8
// IDA_NAME: _ZN10BlueMarble9setScriptEP23BlueMarbleScriptManager
// DEMANGLED: BlueMarble::setScript(BlueMarbleScriptManager *)
BlueMarble* BlueMarble::setScript(BlueMarbleScriptManager* script) {
    script_ = script;
    return this;
}

// IDA_EA: 0x80D8CB6
// IDA_NAME: _ZN10BlueMarble9checkJoinEv
// DEMANGLED: BlueMarble::checkJoin(void)
bool BlueMarble::checkJoin() {
    return getState() == 0 &&
           getWaitUserCount() < script_->info_script.player_max;
}

// IDA_EA: 0x80D9074
// IDA_NAME: _ZNK10BlueMarble11getUserSlotEP5CUser
// DEMANGLED: BlueMarble::getUserSlot(CUser *)const
int BlueMarble::getUserSlot(CUser* user) const {
    if (!user) {
        return -1;
    }
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (users_[slot].getUser() == user) {
            return slot;
        }
    }
    return -1;
}

// IDA_EA: 0x80D90DA
// IDA_NAME: _ZNK10BlueMarble16getWaitUserCountEv
// DEMANGLED: BlueMarble::getWaitUserCount(void)const
int BlueMarble::getWaitUserCount() const {
    int count = 0;
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (users_[slot].getUser()) {
            ++count;
        }
    }
    return count;
}

// IDA_EA: 0x80D9202
// IDA_NAME: _ZN10BlueMarble11changeStateEN15BlueMarbleState1TE
// DEMANGLED: BlueMarble::changeState(BlueMarbleState::T)
BlueMarble* BlueMarble::changeState(BlueMarbleState::T state) {
    state_ = state;
    return this;
}
