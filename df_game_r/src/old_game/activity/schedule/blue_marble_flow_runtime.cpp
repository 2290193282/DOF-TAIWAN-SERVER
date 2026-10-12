#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "core/config/env/environment.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x80D82F2
// IDA_NAME: _ZN10BlueMarble16setLeaveUserSlotEi
// DEMANGLED: BlueMarble::setLeaveUserSlot(int)
void BlueMarble::setLeaveUserSlot(int user_slot) {
    BlueMarbleUserInfo& info = users_[user_slot];
    CUser* const user = info.getUser();
    if (user) {
        user->setBlueMarbleIndex(-1);
        user->setMoveSpace(static_cast<UserSpace::T>(0));
        user->resetBlueMarbleUserDeadCount();
    } else {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::setLeaveUserSlot(int)",
                              708,
                              "BlueMarble::setLeaveUserSlot slot(%d)",
                              user_slot);
    }
    info.resetBlueMarbleUserInfo();
}

// IDA_EA: 0x80D8412
// IDA_NAME: _ZN10BlueMarble27setLeaveUserBlueMarbleStateEv
// DEMANGLED: BlueMarble::setLeaveUserBlueMarbleState(void)
bool BlueMarble::setLeaveUserBlueMarbleState() {
    if (getWaitUserCount() != 0) {
        if (getState() == 1) {
            changeState(BlueMarbleState::kInitial);
            return true;
        }
        return false;
    }

    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        users_[slot].resetBlueMarbleUserInfo();
    }
    changeState(BlueMarbleState::kInitial);
    resetBlueMarble();
    return true;
}

// IDA_EA: 0x80D8D02
// IDA_NAME: _ZN10BlueMarble33checkUserConditionEnterBlueMarbleEP5CUser
// DEMANGLED: BlueMarble::checkUserConditionEnterBlueMarble(CUser *)
int BlueMarble::checkUserConditionEnterBlueMarble(CUser* user) {
    if (!user) {
        return 23;
    }

    int duplicate_count = 0;
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (users_[slot].getUser() == user) {
            ++duplicate_count;
        }
    }
    if (duplicate_count > 0) {
        leaveUser(user);
        return 18;
    }

    if (user->get_state() != 3 || user->checkInBlueMarble() ||
        user->getMoveSpace() == 1 ||
        user->get_charac_level() < script_->info_script.enter_level ||
        user->get_charac_level() > script_->info_script.enter_max_level ||
        user->getBlueMarbleEnterCount() >=
            script_->info_script.enter_count ||
        user->getCurCharacVill() == 7 ||
        private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user) ||
        user->IsBusyDisjointStore() || user->IsCurCharacGhost() ||
        user->CheckInTrade() || user->isCompetitionMercenary()) {
        return 23;
    }

    if (user->CheckInParty()) {
        CParty* const party = user->GetParty();
        if (party) {
            party->send_invite_bluemarble(user->get_unique_id());
            G_CGameManager()->CheckOutParty(user, false);
        }
    }
    return 0;
}

// IDA_EA: 0x80D8F58
// IDA_NAME: _ZN10BlueMarble4joinEP5CUser
// DEMANGLED: BlueMarble::join(CUser *)
int BlueMarble::join(CUser* user) {
    if (!user) {
        return 23;
    }

    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        if (!users_[slot].getUser()) {
            users_[slot].setUser(user);
            user->setBlueMarbleIndex(
                static_cast<short>(getMemoryPoolIndex()));
            user->setMoveSpace(static_cast<UserSpace::T>(1));
            break;
        }
    }
    G_GameWorld()->gotoBlueMarble(user);
    sendBlueMarbleRoomInfo();
    return 0;
}

// IDA_EA: 0x80D9136
// IDA_NAME: _ZN10BlueMarble9leaveUserEP5CUser
// DEMANGLED: BlueMarble::leaveUser(CUser *)
void BlueMarble::leaveUser(CUser* user) {
    if (!user) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::leaveUser(CUser*)",
                              1017,
                              "BlueMarble::leaveUser user is NULL");
        return;
    }

    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::leaveUser(CUser*)",
                              1024,
                              "BlueMarble::leaveUser slot(%d)",
                              -1);
        return;
    }

    G_GameWorld()->outFromBlueMarble(user);
    setLeaveUserSlot(user_slot);
    setLeaveUserBlueMarbleState();
    sendBlueMarbleRoomInfo();
}

// IDA_EA: 0x80D9210
// IDA_NAME: _ZN10BlueMarble15enterBlueMarbleEP5CUser
// DEMANGLED: BlueMarble::enterBlueMarble(CUser *)
int BlueMarble::enterBlueMarble(CUser* user) {
    if (!user || !checkJoin()) {
        return 23;
    }
    int result = checkUserConditionEnterBlueMarble(user);
    if (result > 0) {
        return result;
    }
    result = join(user);
    if (result > 0) {
        return result;
    }
    if (getWaitUserCount() == script_->info_script.player_max) {
        changeState(static_cast<BlueMarbleState::T>(1));
    }
    return 0;
}

// IDA_EA: 0x80D92BE
// IDA_NAME: _ZN10BlueMarble15startBlueMarbleEv
// DEMANGLED: BlueMarble::startBlueMarble(void)
void BlueMarble::startBlueMarble() {
    if (!checkStartBlueMarble()) {
        return;
    }

    resetBlueMarble();
    changeState(static_cast<BlueMarbleState::T>(2));
    setBlueMarbleMap();
    setMaxBoardSize();
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        CUser* const user = users_[slot].getUser();
        if (user) {
            users_[slot].setUserState(
                static_cast<BlueMarbleUserState::T>(1));
            user->increaseBlueMarbleEnterCount();
            user->sendBlueMarbleEnterCount();
        }
    }
    sendStartBlueMarble();
}

// IDA_EA: 0x80D93D8
// IDA_NAME: _ZN10BlueMarble13userThrowDiceEP5CUser
// DEMANGLED: BlueMarble::userThrowDice(CUser *)
void BlueMarble::userThrowDice(CUser* user) {
    if (!user) {
        return;
    }
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::userThrowDice(CUser*)",
                              1094,
                              "BlueMarble::userThrowDice slot(%d)",
                              -1);
        return;
    }
    if (!checkThrowDice(user_slot)) {
        return;
    }

    resetIndexInfo(user_slot);
    throwDice(user_slot);
    setMovePosition(user_slot, getDiceNumber());
    intoBoardZone(user_slot);
    sendBoardZoneActionInfo(user_slot,
                            getDungeonIndex(user_slot),
                            getItemIndex(user_slot));
}

// IDA_EA: 0x80D94F6
// IDA_NAME: _ZN10BlueMarble12enterDungeonEP5CUser
// DEMANGLED: BlueMarble::enterDungeon(CUser *)
void BlueMarble::enterDungeon(CUser* user) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::enterDungeon(CUser*)",
                              1116,
                              "BlueMarble::enterDungeon slot(%d)",
                              -1);
        return;
    }
    setUserState(user_slot, static_cast<BlueMarbleUserState::T>(2));
}

// IDA_EA: 0x80D9572
// IDA_NAME: _ZN10BlueMarble13giveupDungeonEP5CUser
// DEMANGLED: BlueMarble::giveupDungeon(CUser *)
void BlueMarble::giveupDungeon(CUser* user) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::giveupDungeon(CUser*)",
                              1131,
                              "BlueMarble::giveupDungeon slot(%d)",
                              -1);
        return;
    }

    BlueMarbleMapScript* const map = script_->getMap(getBlueMarbleMap());
    if (!map) {
        return;
    }
    BlueMarbleTileScript* const tile =
        map->getTile(users_[user_slot].getBoardZone());
    if (!tile) {
        return;
    }

    if (checkMoveFirstZone(
            user_slot,
            static_cast<BlueMarbleTileScript::Type>(tile->tile_type))) {
        setMovePosition(user_slot, 1 - getMovePosition(user_slot));
        resetBuffInfo(user_slot);
        sendTileGiveUpInfo(user_slot);
    }
    setUserState(user_slot, static_cast<BlueMarbleUserState::T>(1));
}

// IDA_EA: 0x80D96F4
// IDA_NAME: _ZN10BlueMarble12clearDungeonEP5CUser
// DEMANGLED: BlueMarble::clearDungeon(CUser *)
void BlueMarble::clearDungeon(CUser* user) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::clearDungeon(CUser*)",
                              1162,
                              "BlueMarble::clearDungeon slot(%d)",
                              -1);
        return;
    }
    if (checkLastZoneDungeonClear(user_slot)) {
        finishBlueMarble(user_slot);
        resultReward();
    }
}

// IDA_EA: 0x80D978C
// IDA_NAME: _ZN10BlueMarble11exitDungeonEP5CUser
// DEMANGLED: BlueMarble::exitDungeon(CUser *)
void BlueMarble::exitDungeon(CUser* user) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::exitDungeon(CUser*)",
                              1181,
                              "BlueMarble::exitDungeon slot(%d)",
                              -1);
        return;
    }
    setUserState(user_slot, static_cast<BlueMarbleUserState::T>(1));
}

// IDA_EA: 0x80D9808
// IDA_NAME: _ZN10BlueMarble13giveBonusItemEP5CUser
// DEMANGLED: BlueMarble::giveBonusItem(CUser *)
void BlueMarble::giveBonusItem(CUser* user) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::giveBonusItem(CUser*)",
                              1196,
                              "BlueMarble::giveBonusItem slot(%d)",
                              -1);
        return;
    }
    if (getState() != 2 ||
        getUserState(user_slot) !=
            static_cast<BlueMarbleUserState::T>(1)) {
        return;
    }

    Inven_Item item;
    item.item_id_ = static_cast<int>(getItemIndex(user_slot));
    item.set_add_info(1);
    if (item.item_id_ != 0) {
        insertItem(user_slot, item, static_cast<eItemAddReason>(65));
        resetIndexInfo(user_slot);
    }
}

// IDA_EA: 0x80D990E
// IDA_NAME: _ZN10BlueMarble7getBuffEP5CUserN18BlueMarbleBuffType1TE
// DEMANGLED: BlueMarble::getBuff(CUser *,BlueMarbleBuffType::T)
int BlueMarble::getBuff(CUser* user, BlueMarbleBuffType::T buff_type) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(
            1,
            "BlueMarble.cpp",
            "int BlueMarble::getBuff(CUser*, BlueMarbleBuffType::T)",
            1223,
            "BlueMarble::getBuff slot(%d)",
            -1);
        return 0;
    }
    switch (buff_type) {
        case BlueMarbleBuffType::kExperience:
            return getExpBuff(user_slot);
        case BlueMarbleBuffType::kGold:
            return getGoldBuff(user_slot);
        case BlueMarbleBuffType::kDrop:
            return getDropBuff(user_slot);
        default:
            return 0;
    }
}

// IDA_EA: 0x80D9A4C
// IDA_NAME: _ZN10BlueMarble12movePositionEP5CUseri
// DEMANGLED: BlueMarble::movePosition(CUser *,int)
void BlueMarble::movePosition(CUser* user, int position) {
    const int user_slot = getUserSlot(user);
    if (user_slot == -1) {
        LogManager::logFormat(1,
                              "BlueMarble.cpp",
                              "void BlueMarble::movePosition(CUser*, int)",
                              1268,
                              "BlueMarble::movePosition slot(%d)",
                              -1);
        return;
    }
    if (!checkThrowDice(user_slot)) {
        return;
    }

    PacketGuard packet;
    packet.put_header(0, 431);
    packet.put_byte(user_slot);
    packet.put_byte(position - getMovePosition(user_slot));
    packet.finalize(true);
    sendToBlueMarble(packet);

    resetIndexInfo(user_slot);
    setMovePosition(user_slot, position - getMovePosition(user_slot));
    intoBoardZone(user_slot);
    sendBoardZoneActionInfo(user_slot,
                            getDungeonIndex(user_slot),
                            getItemIndex(user_slot));
}

// IDA_EA: 0x80D9C12
// IDA_NAME: _ZN10BlueMarble26checkBlueMarbleUserConfirmEP5CUser
// DEMANGLED: BlueMarble::checkBlueMarbleUserConfirm(CUser *)
void BlueMarble::checkBlueMarbleUserConfirm(CUser* user) {
    if (!user) {
        return;
    }

    char log_prefix[1024] = {};
    std::snprintf(log_prefix,
                  sizeof(log_prefix),
                  "./log/%s/BlueMarbleConfirmInfo",
                  G_CEnvironment()->get_file_name());
    for (int slot = 0; slot < script_->info_script.player_max; ++slot) {
        CUser* const current = users_[slot].getUser();
        if (current != user) {
            continue;
        }
        CMyFileLog log("void BlueMarble::checkBlueMarbleUserConfirm(CUser*)",
                       1300);
        log(log_prefix,
            "checkBlueMarbleUserConfirm() m_id(%u) charac_no(%u) "
            "userState(%d) userSpace(%d) blueMarble_Index(%d) "
            "user_bluemarble_index(%d) slot_index(%d)",
            static_cast<unsigned int>(current->get_acc_id()),
            static_cast<unsigned int>(current->get_charac_no(-1)),
            current->get_state(),
            current->getMoveSpace(),
            getMemoryPoolIndex(),
            current->getBlueMarbleIndex(),
            slot);
    }
}
