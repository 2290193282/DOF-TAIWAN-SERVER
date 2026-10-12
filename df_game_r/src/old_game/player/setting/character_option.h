#ifndef OLDSRV_OLD_GAME_USERINFO_CHARACTER_OPTION_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARACTER_OPTION_H_

#include <stdint.h>


// IDA unit: 04_character_select_create/CharacterOption.
struct CharacterOption {
    CharacterOption();

    uint8_t raw_00[0x200];  // IDA +0x000..+0x1ff
};


#endif  // OLDSRV_OLD_GAME_USERINFO_CHARACTER_OPTION_H_
