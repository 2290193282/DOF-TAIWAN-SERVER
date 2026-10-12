#ifndef OLDSRV_OLD_DATA_DB_SKILL_ITEM_MAKING_SKILL_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_SKILL_ITEM_MAKING_SKILL_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// Layout evidence: the old constructor at 0x848FA82 writes charac_no and the
// eleven signed shorts only.  The final two bytes are the old compiler's
// natural 4-byte tail alignment, not a business field.
struct SIG_ITEM_MAKING_SKILL {
    SIG_ITEM_MAKING_SKILL();

    uint32_t charac_no;   // +0x00
    int16_t weapon;       // +0x04
    int16_t cloth;        // +0x06
    int16_t leather;      // +0x08
    int16_t light_armor;  // +0x0a
    int16_t heavy_armor;  // +0x0c
    int16_t plate;        // +0x0e
    int16_t amulet;       // +0x10
    int16_t wrist;        // +0x12
    int16_t ring;         // +0x14
    int16_t support;      // +0x16
    int16_t magic_stone;  // +0x18
};

static_assert(offsetof(SIG_ITEM_MAKING_SKILL, charac_no) == 0x00u,
              "item-making charac_no offset must stay 0x00");
static_assert(offsetof(SIG_ITEM_MAKING_SKILL, weapon) == 0x04u,
              "item-making weapon offset must stay 0x04");
static_assert(offsetof(SIG_ITEM_MAKING_SKILL, magic_stone) == 0x18u,
              "item-making magic-stone offset must stay 0x18");
static_assert(sizeof(SIG_ITEM_MAKING_SKILL) == 0x1cu,
              "SIG_ITEM_MAKING_SKILL IDA ABI size must stay 0x1c");
static_assert(alignof(SIG_ITEM_MAKING_SKILL) == 4u,
              "SIG_ITEM_MAKING_SKILL IDA ABI alignment must stay 4");

#endif  // OLDSRV_OLD_DATA_DB_SKILL_ITEM_MAKING_SKILL_PAYLOAD_H_
