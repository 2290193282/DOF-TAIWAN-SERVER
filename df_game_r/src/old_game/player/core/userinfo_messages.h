#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_MESSAGES_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_MESSAGES_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822DF1C in the companion source.


#include <stdint.h>

#include "infra/network/packet/message_base.h"


// IDA message body used by DisPatcher_GetUserInfo::read.  The outer packing
// is required because the 13-byte MSG_BASE is immediately followed by a
// u16 and an i8 at offsets 0x0d and 0x0f respectively.
#pragma pack(push, 1)
struct MSG_USERINFO_REQ : public MSG_BASE {
    uint16_t target_uid;  // +0x0d
    char detail_type;     // +0x0f
};
#pragma pack(pop)

static_assert(sizeof(MSG_USERINFO_REQ) == 0x10u,
              "MSG_USERINFO_REQ must retain its old 16-byte extent");

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_HatchCreature.
// The packet reader at 0x81CD3AE receives the one-byte item space first,
// followed immediately by the two-byte creature slot.
#pragma pack(push, 1)
struct MSG_HATCH_CREATURE : public MSG_BASE {
    uint8_t item_space;       // IDA +0x0d
    uint16_t creature_slot;   // IDA +0x0e
};
#pragma pack(pop)

static_assert(sizeof(MSG_HATCH_CREATURE) == 0x10u,
              "MSG_HATCH_CREATURE must retain its IDA 16-byte extent");

// IDA unit: 99_misc_global/stOnRecoverStaminaParam.
struct stOnRecoverStaminaParam : public ParamBase {
    // IDA_EA: 0x8230C62
    // IDA_NAME: _ZN23stOnRecoverStaminaParamC2Ev
    // DEMANGLED: stOnRecoverStaminaParam::stOnRecoverStaminaParam(void)
    stOnRecoverStaminaParam();

    // IDA_EA: 0x824814C
    // IDA_NAME: _ZN23stOnRecoverStaminaParamD2Ev
    // IDA_EA: 0x824817C
    // IDA_NAME: _ZN23stOnRecoverStaminaParamD0Ev
    // DEMANGLED: stOnRecoverStaminaParam::~stOnRecoverStaminaParam()
    virtual ~stOnRecoverStaminaParam();

    int result;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stOnRecoverStaminaParam) == 0x08u,
              "stOnRecoverStaminaParam must retain its i386 ABI size");
#endif

// IDA message body used by DisPatcher_UseEquip::read.
struct MSG_DECREASE_DURABILITY : public MSG_BASE {
    uint8_t equip_slot;  // IDA +0x0d
};

// IDA message body used by DisPatcher_RepairEquip::read.
#pragma pack(push, 1)
struct MSG_REPAIR_EQUIP : public MSG_BASE {
    uint8_t inven_type;  // IDA +0x0d
    uint16_t slot;       // IDA +0x0e
    uint16_t word_10;    // IDA +0x10
};
#pragma pack(pop)

// Constructor-bearing raw messages from 06_userinfo_stat_inventory.
#pragma pack(push, 1)
struct MSG_RENAME_CREATURE : public MSG_BASE {
    MSG_RENAME_CREATURE();

    uint16_t creature_slot;  // IDA +0x0d
    uint8_t rename_mode;     // IDA +0x0f
    uint32_t name_length;    // IDA +0x10
    char name[13];           // IDA +0x14
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MSG_GROWTH_EQUIPMENT : public MSG_BASE {
    MSG_GROWTH_EQUIPMENT();

    uint32_t item_space;   // IDA +0x0d
    uint32_t slot;         // IDA +0x11
};

struct MSG_CHANGE_AVATAR_OPTION : public MSG_BASE {
    MSG_CHANGE_AVATAR_OPTION();

    uint16_t material_slot;     // IDA +0x0d
    uint32_t material_item_id;  // IDA +0x0f
    uint16_t avatar_slot;       // IDA +0x13
    uint32_t avatar_item_id;    // IDA +0x15
    uint8_t option;             // IDA +0x19
};
#pragma pack(pop)


#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_MESSAGES_H_
