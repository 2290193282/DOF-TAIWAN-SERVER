#ifndef OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_RUNTIME_H_

#include <set>
#include <stdint.h>
#include <vector>

#include "old_data/PVF/event/blue_marble_script.h"

class CUser;
class CMTRand;
class PacketGuard;
struct Inven_Item;
enum eItemAddReason : int;

// IDA unit: BlueMarbleState.  The state transitions themselves remain in the
// owning BlueMarble bodies; values are applied at their original call sites.
namespace BlueMarbleState {
enum T {
    kInitial = 0,
};
}

// IDA unit: BlueMarbleUserState.  The original ABI uses an int-sized nested
// enum; values are kept at their recovered call sites rather than invented
// here.
namespace BlueMarbleUserState {
enum T {
    kInitial = 0,
};
}

// IDA unit: BlueMarbleUserGrade.  The numeric grade ordering is applied by
// BlueMarble's ranking body, not synthesized by this state record.
namespace BlueMarbleUserGrade {
enum T {
    kInitial = 0,
};
}

namespace BlueMarbleBuffType {
enum T {
    kExperience = 0,
    kGold = 1,
    kDrop = 2,
};
}

// IDA unit: BlueMarbleUserInfo @ 0x80D6A90.  This is the live per-player
// record held in BlueMarble's vector, not a packet or DB representation.
class BlueMarbleUserInfo {
public:
    // IDA_EA: 0x80D6A90
    // IDA_NAME: _ZN18BlueMarbleUserInfoC2Ev
    // DEMANGLED: BlueMarbleUserInfo::BlueMarbleUserInfo(void)
    BlueMarbleUserInfo();
    // IDA_EA: 0x80D6B02
    // IDA_NAME: _ZN18BlueMarbleUserInfoD2Ev
    // DEMANGLED: BlueMarbleUserInfo::~BlueMarbleUserInfo()
    ~BlueMarbleUserInfo();

    // IDA_EA: 0x80D6B08
    // IDA_NAME: _ZN18BlueMarbleUserInfo23resetBlueMarbleUserInfoEv
    // DEMANGLED: BlueMarbleUserInfo::resetBlueMarbleUserInfo(void)
    int resetBlueMarbleUserInfo();
    // IDA_EA: 0x80D6B4E
    // IDA_NAME: _ZN18BlueMarbleUserInfo9resetBuffEv
    // DEMANGLED: BlueMarbleUserInfo::resetBuff(void)
    int resetBuff();
    // IDA_EA: 0x80D6B64
    // IDA_NAME: _ZN18BlueMarbleUserInfo10resetIndexEv
    // DEMANGLED: BlueMarbleUserInfo::resetIndex(void)
    int resetIndex();

    // IDA_EA: 0x80D6B7A
    // IDA_NAME: _ZN18BlueMarbleUserInfo7setUserEP5CUser
    // DEMANGLED: BlueMarbleUserInfo::setUser(CUser *)
    BlueMarbleUserInfo* setUser(CUser* user);
    // IDA_EA: 0x80D6B88
    // IDA_NAME: _ZNK18BlueMarbleUserInfo7getUserEv
    // DEMANGLED: BlueMarbleUserInfo::getUser(void)const
    CUser* getUser() const;
    // IDA_EA: 0x80D6B92
    // IDA_NAME: _ZN18BlueMarbleUserInfo9setScriptEP23BlueMarbleScriptManager
    // DEMANGLED: BlueMarbleUserInfo::setScript(BlueMarbleScriptManager *)
    BlueMarbleUserInfo* setScript(BlueMarbleScriptManager* script);

    // IDA_EA: 0x80D6BA0
    // IDA_NAME: _ZN18BlueMarbleUserInfo12setUserStateEN19BlueMarbleUserState1TE
    // DEMANGLED: BlueMarbleUserInfo::setUserState(BlueMarbleUserState::T)
    BlueMarbleUserInfo* setUserState(BlueMarbleUserState::T state);
    // IDA_EA: 0x80D6BAE
    // IDA_NAME: _ZNK18BlueMarbleUserInfo12getUserStateEv
    // DEMANGLED: BlueMarbleUserInfo::getUserState(void)const
    BlueMarbleUserState::T getUserState() const;
    // IDA_EA: 0x80D6BBA
    // IDA_NAME: _ZN18BlueMarbleUserInfo12setBoardZoneEii
    // DEMANGLED: BlueMarbleUserInfo::setBoardZone(int,int)
    BlueMarbleUserInfo* setBoardZone(int delta, int max_board_size);
    // IDA_EA: 0x80D6C04
    // IDA_NAME: _ZNK18BlueMarbleUserInfo12getBoardZoneEv
    // DEMANGLED: BlueMarbleUserInfo::getBoardZone(void)const
    int getBoardZone() const;
    // IDA_EA: 0x80D6C10
    // IDA_NAME: _ZN18BlueMarbleUserInfo8setGradeEN19BlueMarbleUserGrade1TE
    // DEMANGLED: BlueMarbleUserInfo::setGrade(BlueMarbleUserGrade::T)
    BlueMarbleUserInfo* setGrade(BlueMarbleUserGrade::T grade);
    // IDA_EA: 0x80D6C1E
    // IDA_NAME: _ZNK18BlueMarbleUserInfo8getGradeEv
    // DEMANGLED: BlueMarbleUserInfo::getGrade(void)const
    BlueMarbleUserGrade::T getGrade() const;
    // IDA_EA: 0x80D6C2A
    // IDA_NAME: _ZN18BlueMarbleUserInfo10setDungeonEi
    // DEMANGLED: BlueMarbleUserInfo::setDungeon(int)
    BlueMarbleUserInfo* setDungeon(int dungeon_index);
    // IDA_EA: 0x80D6C38
    // IDA_NAME: _ZNK18BlueMarbleUserInfo10getDungeonEv
    // DEMANGLED: BlueMarbleUserInfo::getDungeon(void)const
    int getDungeon() const;
    // IDA_EA: 0x80D6C44
    // IDA_NAME: _ZN18BlueMarbleUserInfo7setItemEj
    // DEMANGLED: BlueMarbleUserInfo::setItem(unsigned int)
    BlueMarbleUserInfo* setItem(uint32_t item_index);
    // IDA_EA: 0x80D6C52
    // IDA_NAME: _ZNK18BlueMarbleUserInfo7getItemEv
    // DEMANGLED: BlueMarbleUserInfo::getItem(void)const
    uint32_t getItem() const;

    // IDA_EA: 0x80D6C5E
    // IDA_NAME: _ZN18BlueMarbleUserInfo6setExpEi
    // DEMANGLED: BlueMarbleUserInfo::setExp(int)
    BlueMarbleUserInfo* setExp(int multiplier);
    // IDA_EA: 0x80D6CA0
    // IDA_NAME: _ZNK18BlueMarbleUserInfo6getExpEv
    // DEMANGLED: BlueMarbleUserInfo::getExp(void)const
    int getExp() const;
    // IDA_EA: 0x80D6CAC
    // IDA_NAME: _ZN18BlueMarbleUserInfo7setGoldEi
    // DEMANGLED: BlueMarbleUserInfo::setGold(int)
    BlueMarbleUserInfo* setGold(int multiplier);
    // IDA_EA: 0x80D6CEE
    // IDA_NAME: _ZNK18BlueMarbleUserInfo7getGoldEv
    // DEMANGLED: BlueMarbleUserInfo::getGold(void)const
    int getGold() const;
    // IDA_EA: 0x80D6CFA
    // IDA_NAME: _ZN18BlueMarbleUserInfo7setDropEi
    // DEMANGLED: BlueMarbleUserInfo::setDrop(int)
    BlueMarbleUserInfo* setDrop(int multiplier);
    // IDA_EA: 0x80D6D3C
    // IDA_NAME: _ZNK18BlueMarbleUserInfo7getDropEv
    // DEMANGLED: BlueMarbleUserInfo::getDrop(void)const
    int getDrop() const;

private:
    struct BuffInfo {
        int exp;
        int gold;
        int drop;
    };

    struct TileIndexInfo {
        int dungeon_index;
        uint32_t item_index;
    };

    CUser* user_;
    BlueMarbleUserState::T state_;
    int board_zone_;
    BlueMarbleUserGrade::T grade_;
    BuffInfo buff_;
    TileIndexInfo indexes_;
    BlueMarbleScriptManager* script_;
};

static_assert(sizeof(BlueMarbleUserInfo) == 0x28u,
              "BlueMarbleUserInfo IDA ABI size must stay 0x28");

class BlueMarble {
public:
    BlueMarble();
    ~BlueMarble();

    // IDA_EA: 0x80D6D48
    // IDA_NAME: _ZN10BlueMarble15resetBlueMarbleEv
    // DEMANGLED: BlueMarble::resetBlueMarble(void)
    BlueMarble* resetBlueMarble();
    // IDA_EA: 0x80D6D7C
    // IDA_NAME: _ZN10BlueMarble13resetBuffInfoEi
    // DEMANGLED: BlueMarble::resetBuffInfo(int)
    int resetBuffInfo(int user_slot);
    // IDA_EA: 0x80D6DA2
    // IDA_NAME: _ZN10BlueMarble14resetIndexInfoEi
    // DEMANGLED: BlueMarble::resetIndexInfo(int)
    int resetIndexInfo(int user_slot);
    // IDA_EA: 0x80D6DC8
    // IDA_NAME: _ZNK10BlueMarble8getStateEv
    // DEMANGLED: BlueMarble::getState(void)const
    int getState() const;
    // IDA_EA: 0x80D6E34
    // IDA_NAME: _ZN10BlueMarble16setBlueMarbleMapEv
    // DEMANGLED: BlueMarble::setBlueMarbleMap(void)
    BlueMarble* setBlueMarbleMap();
    // IDA_EA: 0x80D6E7C
    // IDA_NAME: _ZNK10BlueMarble16getBlueMarbleMapEv
    // DEMANGLED: BlueMarble::getBlueMarbleMap(void)const
    int getBlueMarbleMap() const;
    // IDA_EA: 0x80D6E88
    // IDA_NAME: _ZN10BlueMarble15setMaxBoardSizeEv
    // DEMANGLED: BlueMarble::setMaxBoardSize(void)
    void setMaxBoardSize();
    // IDA_EA: 0x80D6ECC
    // IDA_NAME: _ZNK10BlueMarble15getMaxBoardSizeEv
    // DEMANGLED: BlueMarble::getMaxBoardSize(void)const
    int getMaxBoardSize() const;
    // IDA_EA: 0x80D6ED8
    // IDA_NAME: _ZN10BlueMarble14checkThrowDiceEi
    // DEMANGLED: BlueMarble::checkThrowDice(int)
    bool checkThrowDice(int user_slot);
    // IDA_EA: 0x80D6F88
    // IDA_NAME: _ZN10BlueMarble13setDiceNumberEv
    // DEMANGLED: BlueMarble::setDiceNumber(void)
    BlueMarble* setDiceNumber();
    // IDA_EA: 0x80D6FC4
    // IDA_NAME: _ZNK10BlueMarble13getDiceNumberEv
    // DEMANGLED: BlueMarble::getDiceNumber(void)const
    int getDiceNumber() const;
    // IDA_EA: 0x80D6FD0
    // IDA_NAME: _ZN10BlueMarble15setMovePositionEii
    // DEMANGLED: BlueMarble::setMovePosition(int,int)
    BlueMarbleUserInfo* setMovePosition(int user_slot, int dice_number);
    // IDA_EA: 0x80D7012
    // IDA_NAME: _ZNK10BlueMarble15getMovePositionEi
    // DEMANGLED: BlueMarble::getMovePosition(int)const
    int getMovePosition(int user_slot) const;
    // IDA_EA: 0x80D738A
    // IDA_NAME: _ZN10BlueMarble15setDungeonIndexEii
    // DEMANGLED: BlueMarble::setDungeonIndex(int,int)
    BlueMarbleUserInfo* setDungeonIndex(int user_slot, int dungeon_index);
    // IDA_EA: 0x80D73B6
    // IDA_NAME: _ZNK10BlueMarble15getDungeonIndexEi
    // DEMANGLED: BlueMarble::getDungeonIndex(int)const
    int getDungeonIndex(int user_slot) const;
    // IDA_EA: 0x80D7746
    // IDA_NAME: _ZN10BlueMarble12setItemIndexEii
    // DEMANGLED: BlueMarble::setItemIndex(int,int)
    BlueMarbleUserInfo* setItemIndex(int user_slot, int item_index);
    // IDA_EA: 0x80D7778
    // IDA_NAME: _ZNK10BlueMarble12getItemIndexEi
    // DEMANGLED: BlueMarble::getItemIndex(int)const
    uint32_t getItemIndex(int user_slot) const;
    // IDA_EA: 0x80D779E
    // IDA_NAME: _ZN10BlueMarble10setExpBuffEii
    // DEMANGLED: BlueMarble::setExpBuff(int,int)
    BlueMarbleUserInfo* setExpBuff(int user_slot, int multiplier);
    // IDA_EA: 0x80D77CA
    // IDA_NAME: _ZNK10BlueMarble10getExpBuffEi
    // DEMANGLED: BlueMarble::getExpBuff(int)const
    int getExpBuff(int user_slot) const;
    // IDA_EA: 0x80D77F0
    // IDA_NAME: _ZN10BlueMarble11setGoldBuffEii
    // DEMANGLED: BlueMarble::setGoldBuff(int,int)
    BlueMarbleUserInfo* setGoldBuff(int user_slot, int multiplier);
    // IDA_EA: 0x80D781C
    // IDA_NAME: _ZNK10BlueMarble11getGoldBuffEi
    // DEMANGLED: BlueMarble::getGoldBuff(int)const
    int getGoldBuff(int user_slot) const;
    // IDA_EA: 0x80D7842
    // IDA_NAME: _ZN10BlueMarble11setDropBuffEii
    // DEMANGLED: BlueMarble::setDropBuff(int,int)
    BlueMarbleUserInfo* setDropBuff(int user_slot, int multiplier);
    // IDA_EA: 0x80D786E
    // IDA_NAME: _ZNK10BlueMarble11getDropBuffEi
    // DEMANGLED: BlueMarble::getDropBuff(int)const
    int getDropBuff(int user_slot) const;
    // IDA_EA: 0x80D7894
    // IDA_NAME: _ZN10BlueMarble16setUserDeadCountEi
    // DEMANGLED: BlueMarble::setUserDeadCount(int)
    int setUserDeadCount(int user_slot);
    // IDA_EA: 0x80D78C2
    // IDA_NAME: _ZNK10BlueMarble16getUserDeadCountEi
    // DEMANGLED: BlueMarble::getUserDeadCount(int)const
    int getUserDeadCount(int user_slot) const;
    // IDA_EA: 0x80D78F0
    // IDA_NAME: _ZN10BlueMarble18checkMoveFirstZoneEiN20BlueMarbleTileScript4TypeE
    // DEMANGLED: BlueMarble::checkMoveFirstZone(int,BlueMarbleTileScript::Type)
    bool checkMoveFirstZone(int user_slot, BlueMarbleTileScript::Type tile_type);
    // IDA_EA: 0x80D793A
    // IDA_NAME: _ZN10BlueMarble12setUserStateEiN19BlueMarbleUserState1TE
    // DEMANGLED: BlueMarble::setUserState(int,BlueMarbleUserState::T)
    BlueMarbleUserInfo* setUserState(int user_slot,
                                     BlueMarbleUserState::T state);
    // IDA_EA: 0x80D7966
    // IDA_NAME: _ZNK10BlueMarble12getUserStateEi
    // DEMANGLED: BlueMarble::getUserState(int)const
    BlueMarbleUserState::T getUserState(int user_slot) const;
    // IDA_EA: 0x80D798C
    // IDA_NAME: _ZN10BlueMarble25checkLastZoneDungeonClearEi
    // DEMANGLED: BlueMarble::checkLastZoneDungeonClear(int)
    bool checkLastZoneDungeonClear(int user_slot);

    int checkStartBlueMarble();
    void throwDice(int user_slot);
    void finishBlueMarble(int user_slot);
    int setVisitZone(int board_zone);
    bool getVisitZone(int board_zone) const;
    void setUserGrade(int user_slot);
    int getWinner() const;
    int intoBoardZone(int user_slot);
    int tileType(int user_slot);
    int getRandomDungeon(int user_slot,
                         BlueMarbleTileScript::Type tile_type) const;
    int getRandomItem() const;
    bool resultReward();
    void setResultItem(int user_slot, Inven_Item& item);
    void insertItem(int user_slot,
                    Inven_Item& item,
                    eItemAddReason reason);
    void setLeaveUserSlot(int user_slot);
    bool setLeaveUserBlueMarbleState();
    void setUserInfo(BlueMarbleScriptManager* script);
    BlueMarble* setScript(BlueMarbleScriptManager* script);
    bool checkJoin();
    int getUserSlot(CUser* user) const;
    int getWaitUserCount() const;
    BlueMarble* changeState(BlueMarbleState::T state);

    void sendBlueMarbleRoomInfo() const;
    void sendBlueMarbleDiceInfo(int user_slot) const;
    void sendStartBlueMarble() const;
    void sendFinishBlueMarble() const;
    void sendBoardZoneActionInfo(int user_slot,
                                 int dungeon_index,
                                 unsigned int item_index) const;
    void sendTileGiveUpInfo(int user_slot) const;
    void sendUpdateItemInfo(int user_slot, int inventory_slot) const;
    int checkUserConditionEnterBlueMarble(CUser* user);
    int join(CUser* user);
    void leaveUser(CUser* user);
    int enterBlueMarble(CUser* user);
    void startBlueMarble();
    void userThrowDice(CUser* user);
    void enterDungeon(CUser* user);
    void giveupDungeon(CUser* user);
    void clearDungeon(CUser* user);
    void exitDungeon(CUser* user);
    void giveBonusItem(CUser* user);
    int getBuff(CUser* user, BlueMarbleBuffType::T buff_type);
    void sendToBlueMarble(PacketGuard& packet) const;
    void movePosition(CUser* user, int position);
    void checkBlueMarbleUserConfirm(CUser* user);

    // IDA_EA: 0x80D8C26
    // IDA_NAME: _ZN10BlueMarble18setMemoryPoolIndexEi
    // DEMANGLED: BlueMarble::setMemoryPoolIndex(int)
    BlueMarble* setMemoryPoolIndex(int index);
    // IDA_EA: 0x80D8C34
    // IDA_NAME: _ZNK10BlueMarble18getMemoryPoolIndexEv
    // DEMANGLED: BlueMarble::getMemoryPoolIndex(void)const
    int getMemoryPoolIndex() const;

private:
    int memory_pool_index_;
    std::vector<BlueMarbleUserInfo> users_;
    int state_;
    CMTRand* randoms_;
    int dice_number_;
    int map_index_;
    std::set<int> visit_zones_;
    BlueMarbleScriptManager* script_;
    int max_board_size_;
};

static_assert(sizeof(BlueMarble) == 0x40u,
              "BlueMarble IDA ABI size must stay 0x40");

#endif  // OLDSRV_OLD_GAME_EVENT_BLUE_MARBLE_RUNTIME_H_
