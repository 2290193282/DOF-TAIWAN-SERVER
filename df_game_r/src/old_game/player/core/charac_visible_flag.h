#ifndef OLDSRV_OLD_GAME_USERINFO_CHARAC_VISIBLE_FLAG_H_
#define OLDSRV_OLD_GAME_USERINFO_CHARAC_VISIBLE_FLAG_H_

#include <stddef.h>
#include <stdint.h>


struct tagCharacVisibleFlag {
    tagCharacVisibleFlag();

    tagCharacVisibleFlag* reset();

    uint8_t visible_flag;
};

static_assert(sizeof(tagCharacVisibleFlag) == 0x01u,
              "tagCharacVisibleFlag IDA ABI size must stay 0x01");
static_assert(alignof(tagCharacVisibleFlag) == 0x01u,
              "tagCharacVisibleFlag IDA ABI alignment must stay 1");
static_assert(offsetof(tagCharacVisibleFlag, visible_flag) == 0x00u,
              "tagCharacVisibleFlag::visible_flag must stay at +0x00");

#endif  // OLDSRV_OLD_GAME_USERINFO_CHARAC_VISIBLE_FLAG_H_
