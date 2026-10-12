#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "old_data/PVF/event/blue_marble_script.h"

#include <stdint.h>


// IDA_EA: 0x80D6A90
// IDA_NAME: _ZN18BlueMarbleUserInfoC2Ev
// DEMANGLED: BlueMarbleUserInfo::BlueMarbleUserInfo(void)
BlueMarbleUserInfo::BlueMarbleUserInfo()
    : user_(0),
      state_(static_cast<BlueMarbleUserState::T>(0)),
      board_zone_(1),
      grade_(static_cast<BlueMarbleUserGrade::T>(4)),
      buff_(),
      indexes_(),
      script_(0) {
    resetBuff();
    resetIndex();
}

// IDA_EA: 0x80D6B02
// IDA_NAME: _ZN18BlueMarbleUserInfoD2Ev
// DEMANGLED: BlueMarbleUserInfo::~BlueMarbleUserInfo()
BlueMarbleUserInfo::~BlueMarbleUserInfo() {}

// IDA_EA: 0x80D6B08
// IDA_NAME: _ZN18BlueMarbleUserInfo23resetBlueMarbleUserInfoEv
// DEMANGLED: BlueMarbleUserInfo::resetBlueMarbleUserInfo(void)
int BlueMarbleUserInfo::resetBlueMarbleUserInfo() {
    user_ = 0;
    state_ = static_cast<BlueMarbleUserState::T>(0);
    board_zone_ = 1;
    grade_ = static_cast<BlueMarbleUserGrade::T>(4);
    resetBuff();
    return resetIndex();
}

// IDA_EA: 0x80D6B4E
// IDA_NAME: _ZN18BlueMarbleUserInfo9resetBuffEv
// DEMANGLED: BlueMarbleUserInfo::resetBuff(void)
int BlueMarbleUserInfo::resetBuff() {
    buff_.exp = 1;
    buff_.gold = 1;
    buff_.drop = 1;
    return static_cast<int>(reinterpret_cast<intptr_t>(&buff_));
}

// IDA_EA: 0x80D6B64
// IDA_NAME: _ZN18BlueMarbleUserInfo10resetIndexEv
// DEMANGLED: BlueMarbleUserInfo::resetIndex(void)
int BlueMarbleUserInfo::resetIndex() {
    indexes_.dungeon_index = 0;
    indexes_.item_index = 0;
    return static_cast<int>(reinterpret_cast<intptr_t>(&indexes_));
}

// IDA_EA: 0x80D6B7A
// IDA_NAME: _ZN18BlueMarbleUserInfo7setUserEP5CUser
// DEMANGLED: BlueMarbleUserInfo::setUser(CUser *)
BlueMarbleUserInfo* BlueMarbleUserInfo::setUser(CUser* user) {
    user_ = user;
    return this;
}

// IDA_EA: 0x80D6B88
// IDA_NAME: _ZNK18BlueMarbleUserInfo7getUserEv
// DEMANGLED: BlueMarbleUserInfo::getUser(void)const
CUser* BlueMarbleUserInfo::getUser() const {
    return user_;
}

// IDA_EA: 0x80D6B92
// IDA_NAME: _ZN18BlueMarbleUserInfo9setScriptEP23BlueMarbleScriptManager
// DEMANGLED: BlueMarbleUserInfo::setScript(BlueMarbleScriptManager *)
BlueMarbleUserInfo* BlueMarbleUserInfo::setScript(
    BlueMarbleScriptManager* script) {
    script_ = script;
    return this;
}

// IDA_EA: 0x80D6BA0
// IDA_NAME: _ZN18BlueMarbleUserInfo12setUserStateEN19BlueMarbleUserState1TE
// DEMANGLED: BlueMarbleUserInfo::setUserState(BlueMarbleUserState::T)
BlueMarbleUserInfo* BlueMarbleUserInfo::setUserState(
    BlueMarbleUserState::T state) {
    state_ = state;
    return this;
}

// IDA_EA: 0x80D6BAE
// IDA_NAME: _ZNK18BlueMarbleUserInfo12getUserStateEv
// DEMANGLED: BlueMarbleUserInfo::getUserState(void)const
BlueMarbleUserState::T BlueMarbleUserInfo::getUserState() const {
    return state_;
}

// IDA_EA: 0x80D6BBA
// IDA_NAME: _ZN18BlueMarbleUserInfo12setBoardZoneEii
// DEMANGLED: BlueMarbleUserInfo::setBoardZone(int,int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setBoardZone(int delta,
                                                      int max_board_size) {
    if (delta + board_zone_ < max_board_size) {
        if (delta + board_zone_ > 1) {
            board_zone_ += delta;
        } else {
            board_zone_ = 1;
        }
    } else {
        board_zone_ = max_board_size;
    }
    return this;
}

// IDA_EA: 0x80D6C04
// IDA_NAME: _ZNK18BlueMarbleUserInfo12getBoardZoneEv
// DEMANGLED: BlueMarbleUserInfo::getBoardZone(void)const
int BlueMarbleUserInfo::getBoardZone() const {
    return board_zone_;
}

// IDA_EA: 0x80D6C10
// IDA_NAME: _ZN18BlueMarbleUserInfo8setGradeEN19BlueMarbleUserGrade1TE
// DEMANGLED: BlueMarbleUserInfo::setGrade(BlueMarbleUserGrade::T)
BlueMarbleUserInfo* BlueMarbleUserInfo::setGrade(
    BlueMarbleUserGrade::T grade) {
    grade_ = grade;
    return this;
}

// IDA_EA: 0x80D6C1E
// IDA_NAME: _ZNK18BlueMarbleUserInfo8getGradeEv
// DEMANGLED: BlueMarbleUserInfo::getGrade(void)const
BlueMarbleUserGrade::T BlueMarbleUserInfo::getGrade() const {
    return grade_;
}

// IDA_EA: 0x80D6C2A
// IDA_NAME: _ZN18BlueMarbleUserInfo10setDungeonEi
// DEMANGLED: BlueMarbleUserInfo::setDungeon(int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setDungeon(int dungeon_index) {
    indexes_.dungeon_index = dungeon_index;
    return this;
}

// IDA_EA: 0x80D6C38
// IDA_NAME: _ZNK18BlueMarbleUserInfo10getDungeonEv
// DEMANGLED: BlueMarbleUserInfo::getDungeon(void)const
int BlueMarbleUserInfo::getDungeon() const {
    return indexes_.dungeon_index;
}

// IDA_EA: 0x80D6C44
// IDA_NAME: _ZN18BlueMarbleUserInfo7setItemEj
// DEMANGLED: BlueMarbleUserInfo::setItem(unsigned int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setItem(uint32_t item_index) {
    indexes_.item_index = item_index;
    return this;
}

// IDA_EA: 0x80D6C52
// IDA_NAME: _ZNK18BlueMarbleUserInfo7getItemEv
// DEMANGLED: BlueMarbleUserInfo::getItem(void)const
uint32_t BlueMarbleUserInfo::getItem() const {
    return indexes_.item_index;
}

// IDA_EA: 0x80D6C5E
// IDA_NAME: _ZN18BlueMarbleUserInfo6setExpEi
// DEMANGLED: BlueMarbleUserInfo::setExp(int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setExp(int multiplier) {
    const int multiplied = multiplier * buff_.exp;
    buff_.exp = multiplied < script_->info_script.buff_max_value
        ? multiplied
        : script_->info_script.buff_max_value;
    return this;
}

// IDA_EA: 0x80D6CA0
// IDA_NAME: _ZNK18BlueMarbleUserInfo6getExpEv
// DEMANGLED: BlueMarbleUserInfo::getExp(void)const
int BlueMarbleUserInfo::getExp() const {
    return buff_.exp;
}

// IDA_EA: 0x80D6CAC
// IDA_NAME: _ZN18BlueMarbleUserInfo7setGoldEi
// DEMANGLED: BlueMarbleUserInfo::setGold(int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setGold(int multiplier) {
    const int multiplied = multiplier * buff_.gold;
    buff_.gold = multiplied < script_->info_script.buff_max_value
        ? multiplied
        : script_->info_script.buff_max_value;
    return this;
}

// IDA_EA: 0x80D6CEE
// IDA_NAME: _ZNK18BlueMarbleUserInfo7getGoldEv
// DEMANGLED: BlueMarbleUserInfo::getGold(void)const
int BlueMarbleUserInfo::getGold() const {
    return buff_.gold;
}

// IDA_EA: 0x80D6CFA
// IDA_NAME: _ZN18BlueMarbleUserInfo7setDropEi
// DEMANGLED: BlueMarbleUserInfo::setDrop(int)
BlueMarbleUserInfo* BlueMarbleUserInfo::setDrop(int multiplier) {
    const int multiplied = multiplier * buff_.drop;
    buff_.drop = multiplied < script_->info_script.buff_max_value
        ? multiplied
        : script_->info_script.buff_max_value;
    return this;
}

// IDA_EA: 0x80D6D3C
// IDA_NAME: _ZNK18BlueMarbleUserInfo7getDropEv
// DEMANGLED: BlueMarbleUserInfo::getDrop(void)const
int BlueMarbleUserInfo::getDrop() const {
    return buff_.drop;
}
