#include "old_game/activity/schedule/blue_marble_runtime.h"

#include "old_game/player/core/cuser.h"
#include "common/manager/mt_rand.h"

#include <vector>

// IDA_EA: 0x80D7038
// IDA_NAME: _ZN10BlueMarble13intoBoardZoneEi
// DEMANGLED: BlueMarble::intoBoardZone(int)
int BlueMarble::intoBoardZone(int user_slot) {
    setDungeonIndex(
        user_slot,
        getRandomDungeon(user_slot,
                         static_cast<BlueMarbleTileScript::Type>(14)));
    tileType(user_slot);
    return setVisitZone(users_[user_slot].getBoardZone());
}

// IDA_EA: 0x80D70AE
// IDA_NAME: _ZN10BlueMarble8tileTypeEi
// DEMANGLED: BlueMarble::tileType(int)
int BlueMarble::tileType(int user_slot) {
    const int board_zone = users_[user_slot].getBoardZone();
    BlueMarbleMapScript* const map = script_->getMap(getBlueMarbleMap());
    if (!map) {
        return 0;
    }

    BlueMarbleTileScript* const tile = map->getTile(board_zone);
    if (!tile) {
        return 0;
    }

    const BlueMarbleTileScript::Type type =
        static_cast<BlueMarbleTileScript::Type>(tile->tile_type);
    switch (tile->tile_type) {
        case 2:
        case 8:
            setDungeonIndex(
                user_slot,
                getRandomDungeon(
                    user_slot,
                    static_cast<BlueMarbleTileScript::Type>(2)));
            break;
        case 3:
            setGoldBuff(user_slot, tile->value);
            break;
        case 4:
            setExpBuff(user_slot, tile->value);
            break;
        case 5:
            setDropBuff(user_slot, tile->value);
            break;
        case 6:
            setItemIndex(user_slot, getRandomItem());
            break;
        case 7:
            if (!getVisitZone(board_zone)) {
                setDungeonIndex(
                    user_slot,
                    getRandomDungeon(
                        user_slot,
                        static_cast<BlueMarbleTileScript::Type>(7)));
            }
            break;
        case 9:
            setVisitZone(board_zone);
            setMovePosition(user_slot, tile->value);
            if (checkMoveFirstZone(user_slot, type)) {
                resetBuffInfo(user_slot);
            }
            return intoBoardZone(user_slot);
        case 10: {
            setVisitZone(board_zone);
            const int delta = tile->value - getMovePosition(user_slot);
            setMovePosition(user_slot, delta);
            if (checkMoveFirstZone(user_slot, type)) {
                resetBuffInfo(user_slot);
            }
            return intoBoardZone(user_slot);
        }
        case 11:
        case 12:
        case 13:
            setDungeonIndex(user_slot, tile->value);
            break;
        default:
            return 0;
    }
    return 1;
}

// IDA_EA: 0x80D73DC
// IDA_NAME: _ZNK10BlueMarble16getRandomDungeonEiN20BlueMarbleTileScript4TypeE
// DEMANGLED: BlueMarble::getRandomDungeon(int,BlueMarbleTileScript::Type)const
int BlueMarble::getRandomDungeon(
    int user_slot, BlueMarbleTileScript::Type tile_type) const {
    CUser* const user = users_[user_slot].getUser();
    if (!user) {
        return 0;
    }

    BlueMarbleMapScript* const map = script_->getMap(getBlueMarbleMap());
    if (!map) {
        return 0;
    }

    BlueMarbleTileScript* const tile =
        map->getTile(users_[user_slot].getBoardZone());
    if (!tile || tile->enter_dungeon != 1) {
        return 0;
    }

    const int level = user->get_charac_level();
    std::vector<int> dungeons;
    const int type = static_cast<int>(tile_type);
    if (type == 7) {
        dungeons = script_->info_script.getUniqueDungeonIndex(level);
    } else if (type == 8 || type == 2) {
        dungeons = script_->info_script.getBossDungeonIndex(level);
    } else {
        dungeons = script_->info_script.getRandomDungeonIndex(level);
    }

    if (dungeons.empty()) {
        return 0;
    }
    const unsigned int selected =
        randoms_[2].randInt() % static_cast<unsigned int>(dungeons.size());
    return dungeons[selected];
}

// IDA_EA: 0x80D7690
// IDA_NAME: _ZNK10BlueMarble13getRandomItemEv
// DEMANGLED: BlueMarble::getRandomItem(void)const
int BlueMarble::getRandomItem() const {
    const unsigned int total =
        static_cast<unsigned int>(script_->info_script.item_probability_total);
    if (total == 0) {
        return 0;
    }

    const int roll = static_cast<int>(randoms_[3].randInt(total));
    int cumulative = 0;
    const std::vector<BlueMarbleRandomItemInfo>& items =
        script_->info_script.random_items;
    for (std::vector<BlueMarbleRandomItemInfo>::const_iterator it =
             items.begin();
         it != items.end(); ++it) {
        cumulative += it->second_dword;
        if (cumulative >= roll) {
            return it->first_dword;
        }
    }
    return 0;
}
