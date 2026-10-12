#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "old_data/PVF/event/blue_marble_script.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"

// Generated from the direct BlueMarble parent-state call cluster in the
// 4.10.2 IDA export.  These bodies own player-state transitions; the scripts
// and their PVF parsing remain owned by DataManager.

// IDA_EA: 0x80D6D48
// IDA_NAME: _ZN10BlueMarble15resetBlueMarbleEv
// DEMANGLED: BlueMarble::resetBlueMarble(void)
BlueMarble* BlueMarble::resetBlueMarble() {
    dice_number_ = 0;
    map_index_ = 0;
    visit_zones_.clear();
    max_board_size_ = 0;
    return this;
}

// IDA_EA: 0x80D6D7C
// IDA_NAME: _ZN10BlueMarble13resetBuffInfoEi
// DEMANGLED: BlueMarble::resetBuffInfo(int)
int BlueMarble::resetBuffInfo(int user_slot) {
    return users_[user_slot].resetBuff();
}

// IDA_EA: 0x80D6DA2
// IDA_NAME: _ZN10BlueMarble14resetIndexInfoEi
// DEMANGLED: BlueMarble::resetIndexInfo(int)
int BlueMarble::resetIndexInfo(int user_slot) {
    return users_[user_slot].resetIndex();
}

// IDA_EA: 0x80D6DC8
// IDA_NAME: _ZNK10BlueMarble8getStateEv
// DEMANGLED: BlueMarble::getState(void)const
int BlueMarble::getState() const {
    return state_;
}

// IDA_EA: 0x80D6E34
// IDA_NAME: _ZN10BlueMarble16setBlueMarbleMapEv
// DEMANGLED: BlueMarble::setBlueMarbleMap(void)
BlueMarble* BlueMarble::setBlueMarbleMap() {
    map_index_ = static_cast<int>(randoms_[1].randInt() %
                                  script_->getMapTotalCount()) +
                 1;
    return this;
}

// IDA_EA: 0x80D6E7C
// IDA_NAME: _ZNK10BlueMarble16getBlueMarbleMapEv
// DEMANGLED: BlueMarble::getBlueMarbleMap(void)const
int BlueMarble::getBlueMarbleMap() const {
    return map_index_;
}

// IDA_EA: 0x80D6E88
// IDA_NAME: _ZN10BlueMarble15setMaxBoardSizeEv
// DEMANGLED: BlueMarble::setMaxBoardSize(void)
void BlueMarble::setMaxBoardSize() {
    BlueMarbleMapScript* const map = script_->getMap(getBlueMarbleMap());
    if (map) {
        max_board_size_ = static_cast<int>(map->tiles.size());
    }
}

// IDA_EA: 0x80D6ECC
// IDA_NAME: _ZNK10BlueMarble15getMaxBoardSizeEv
// DEMANGLED: BlueMarble::getMaxBoardSize(void)const
int BlueMarble::getMaxBoardSize() const {
    return max_board_size_;
}

// IDA_EA: 0x80D6ED8
// IDA_NAME: _ZN10BlueMarble14checkThrowDiceEi
// DEMANGLED: BlueMarble::checkThrowDice(int)
bool BlueMarble::checkThrowDice(int user_slot) {
    return users_[user_slot].getBoardZone() < getMaxBoardSize() &&
           users_[user_slot].getUserState() ==
               static_cast<BlueMarbleUserState::T>(1) &&
           getState() == 2;
}

// IDA_EA: 0x80D6F88
// IDA_NAME: _ZN10BlueMarble13setDiceNumberEv
// DEMANGLED: BlueMarble::setDiceNumber(void)
BlueMarble* BlueMarble::setDiceNumber() {
    dice_number_ = script_->info_script.dice_min +
                   static_cast<int>(randoms_[0].randInt() %
                                    script_->info_script.dice_max);
    return this;
}

// IDA_EA: 0x80D6FC4
// IDA_NAME: _ZNK10BlueMarble13getDiceNumberEv
// DEMANGLED: BlueMarble::getDiceNumber(void)const
int BlueMarble::getDiceNumber() const {
    return dice_number_;
}

// IDA_EA: 0x80D6FD0
// IDA_NAME: _ZN10BlueMarble15setMovePositionEii
// DEMANGLED: BlueMarble::setMovePosition(int,int)
BlueMarbleUserInfo* BlueMarble::setMovePosition(int user_slot,
                                                 int dice_number) {
    return users_[user_slot].setBoardZone(dice_number, getMaxBoardSize());
}

// IDA_EA: 0x80D7012
// IDA_NAME: _ZNK10BlueMarble15getMovePositionEi
// DEMANGLED: BlueMarble::getMovePosition(int)const
int BlueMarble::getMovePosition(int user_slot) const {
    return users_[user_slot].getBoardZone();
}

// IDA_EA: 0x80D738A
// IDA_NAME: _ZN10BlueMarble15setDungeonIndexEii
// DEMANGLED: BlueMarble::setDungeonIndex(int,int)
BlueMarbleUserInfo* BlueMarble::setDungeonIndex(int user_slot,
                                                 int dungeon_index) {
    return users_[user_slot].setDungeon(dungeon_index);
}

// IDA_EA: 0x80D73B6
// IDA_NAME: _ZNK10BlueMarble15getDungeonIndexEi
// DEMANGLED: BlueMarble::getDungeonIndex(int)const
int BlueMarble::getDungeonIndex(int user_slot) const {
    return users_[user_slot].getDungeon();
}

// IDA_EA: 0x80D7746
// IDA_NAME: _ZN10BlueMarble12setItemIndexEii
// DEMANGLED: BlueMarble::setItemIndex(int,int)
BlueMarbleUserInfo* BlueMarble::setItemIndex(int user_slot,
                                              int item_index) {
    return users_[user_slot].setItem(static_cast<uint32_t>(item_index));
}

// IDA_EA: 0x80D7778
// IDA_NAME: _ZNK10BlueMarble12getItemIndexEi
// DEMANGLED: BlueMarble::getItemIndex(int)const
uint32_t BlueMarble::getItemIndex(int user_slot) const {
    return users_[user_slot].getItem();
}

// IDA_EA: 0x80D779E
// IDA_NAME: _ZN10BlueMarble10setExpBuffEii
// DEMANGLED: BlueMarble::setExpBuff(int,int)
BlueMarbleUserInfo* BlueMarble::setExpBuff(int user_slot, int multiplier) {
    return users_[user_slot].setExp(multiplier);
}

// IDA_EA: 0x80D77CA
// IDA_NAME: _ZNK10BlueMarble10getExpBuffEi
// DEMANGLED: BlueMarble::getExpBuff(int)const
int BlueMarble::getExpBuff(int user_slot) const {
    return users_[user_slot].getExp();
}

// IDA_EA: 0x80D77F0
// IDA_NAME: _ZN10BlueMarble11setGoldBuffEii
// DEMANGLED: BlueMarble::setGoldBuff(int,int)
BlueMarbleUserInfo* BlueMarble::setGoldBuff(int user_slot, int multiplier) {
    return users_[user_slot].setGold(multiplier);
}

// IDA_EA: 0x80D781C
// IDA_NAME: _ZNK10BlueMarble11getGoldBuffEi
// DEMANGLED: BlueMarble::getGoldBuff(int)const
int BlueMarble::getGoldBuff(int user_slot) const {
    return users_[user_slot].getGold();
}

// IDA_EA: 0x80D7842
// IDA_NAME: _ZN10BlueMarble11setDropBuffEii
// DEMANGLED: BlueMarble::setDropBuff(int,int)
BlueMarbleUserInfo* BlueMarble::setDropBuff(int user_slot, int multiplier) {
    return users_[user_slot].setDrop(multiplier);
}

// IDA_EA: 0x80D786E
// IDA_NAME: _ZNK10BlueMarble11getDropBuffEi
// DEMANGLED: BlueMarble::getDropBuff(int)const
int BlueMarble::getDropBuff(int user_slot) const {
    return users_[user_slot].getDrop();
}

// IDA_EA: 0x80D7894
// IDA_NAME: _ZN10BlueMarble16setUserDeadCountEi
// DEMANGLED: BlueMarble::setUserDeadCount(int)
int BlueMarble::setUserDeadCount(int user_slot) {
    return users_[user_slot].getUser()->increaseBlueMarbleUserDeadCount();
}

// IDA_EA: 0x80D78C2
// IDA_NAME: _ZNK10BlueMarble16getUserDeadCountEi
// DEMANGLED: BlueMarble::getUserDeadCount(int)const
int BlueMarble::getUserDeadCount(int user_slot) const {
    return users_[user_slot].getUser()->getBlueMarbleUserDeadCount();
}

// IDA_EA: 0x80D78F0
// IDA_NAME: _ZN10BlueMarble18checkMoveFirstZoneEiN20BlueMarbleTileScript4TypeE
// DEMANGLED: BlueMarble::checkMoveFirstZone(int,BlueMarbleTileScript::Type)
bool BlueMarble::checkMoveFirstZone(
    int user_slot, BlueMarbleTileScript::Type tile_type) {
    const int tile_type_value = static_cast<int>(tile_type);
    if (tile_type_value >= 9) {
        if (tile_type_value <= 10) {
            if (getMovePosition(user_slot) != 1) {
                return false;
            }
        } else if (tile_type_value <= 13) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x80D793A
// IDA_NAME: _ZN10BlueMarble12setUserStateEiN19BlueMarbleUserState1TE
// DEMANGLED: BlueMarble::setUserState(int,BlueMarbleUserState::T)
BlueMarbleUserInfo* BlueMarble::setUserState(
    int user_slot, BlueMarbleUserState::T state) {
    return users_[user_slot].setUserState(state);
}

// IDA_EA: 0x80D7966
// IDA_NAME: _ZNK10BlueMarble12getUserStateEi
// DEMANGLED: BlueMarble::getUserState(int)const
BlueMarbleUserState::T BlueMarble::getUserState(int user_slot) const {
    return users_[user_slot].getUserState();
}

// IDA_EA: 0x80D798C
// IDA_NAME: _ZN10BlueMarble25checkLastZoneDungeonClearEi
// DEMANGLED: BlueMarble::checkLastZoneDungeonClear(int)
bool BlueMarble::checkLastZoneDungeonClear(int user_slot) {
    return getState() == 2 &&
           users_[user_slot].getBoardZone() == getMaxBoardSize();
}
