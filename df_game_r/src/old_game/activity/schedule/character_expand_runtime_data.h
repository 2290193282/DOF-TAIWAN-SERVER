#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_EXPAND_RUNTIME_DATA_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_EXPAND_RUNTIME_DATA_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x812D024 in the companion source.


#include <stdint.h>

#include <map>

#include "old_game/activity/schedule/advance_altar_data.h"


class CUser;

namespace advancealtar {

// IDA unit: 04_character_select_create/advancealtar/CharacData.
struct CharacData {
    CharacData();
    ~CharacData();

    int reset();
    int upGage();
    int upgreadGage();
    int setSlotItem();
    int summonSlotUnit(uint8_t slot, int item_id);
    void sendTestCheckGage();
    CharacData* setUser(CUser* new_user);
    CUser* getUser() const;
    int getGage() const;
    int getGageLevel() const;

    int updated;
    CUser* user;
    int gage_level;
    int gage;
    std::map<uint8_t, _SlotItem> slot_items;
};

}  // namespace advancealtar


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_EXPAND_RUNTIME_DATA_H_
