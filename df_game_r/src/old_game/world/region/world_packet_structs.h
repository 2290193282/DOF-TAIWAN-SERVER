#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PACKET_STRUCTS_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PACKET_STRUCTS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x822C3C2 in the companion source.


#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"
#include "old_game/activity/reward/advance_altar_timeline_parameter.h"


#pragma pack(push, 1)

// IDA unit:
// 05_world_enter_village/Packet_Game_Monitor_GM_Village_Attacked.
struct Packet_Game_Monitor_GM_Village_Attacked : public PacketHeader {
    Packet_Game_Monitor_GM_Village_Attacked();

    uint8_t payload[0x0d];
};

// IDA unit: 05_world_enter_village/Packet_VillageAttackedGMCommand.
struct Packet_VillageAttackedGMCommand : public PacketHeader {
    Packet_VillageAttackedGMCommand();

    // 0x84AA144 writes these four dwords in this wire order after the
    // 10-byte PacketHeader.  The prior names made the GM command payload
    // appear as an opaque tail and obscured the actual monitor-server ABI.
    uint32_t uid;          // +0x0a
    int32_t charac_no;     // +0x0e
    int32_t value;         // +0x12
    uint32_t command;      // +0x16 (the start command sends zero)
};

// IDA unit: 05_world_enter_village/Packet_VillageMonsterFightResult.
struct Packet_VillageMonsterFightResult : public PacketHeader {
    Packet_VillageMonsterFightResult();

    uint32_t monster_counts[4];
    uint32_t clear_counts[4];
    uint8_t result;
};

namespace NOTIPacketStruct {

// IDA_EA: 0x81346A6
// IDA_NAME: _ZN16NOTIPacketStruct26_NotiAdvanceAltarSpawnUnitC2Ev
// The advance-altar spawn notification carries the allocated monster id
// followed by the complete PVF summon record, including the original x/y
// position and direction.
struct _NotiAdvanceAltarSpawnUnit {
    _NotiAdvanceAltarSpawnUnit();

    uint16_t monster_unique_id;                 // +0x00
    uint16_t reserved_02;                       // +0x02
    advancealtar::_SummonObject summon_object; // +0x04
};

static_assert(sizeof(_NotiAdvanceAltarSpawnUnit) == 0x28u,
              "advance-altar spawn notification ABI must stay 0x28");

// IDA unit:
// 05_world_enter_village/NOTIPacketStruct/_NotiAdvanceAltarMapInfo.
struct _NotiAdvanceAltarMapInfo {
    _NotiAdvanceAltarMapInfo();

    int32_t stage_type;                  // +0x00
    int32_t survival_round_no;           // +0x04
    int32_t map_index;                   // +0x08
    int32_t timeline_total_duration_ms;  // +0x0c
    int32_t map_basis_level;             // +0x10
};

// IDA_EA: 0x8134752
// IDA_NAME: _ZN16NOTIPacketStruct26_NotiAdvanceAltarEplpStateC2Ev
// DEMANGLED: NOTIPacketStruct::_NotiAdvanceAltarEplpState::_NotiAdvanceAltarEplpState(void)
struct _NotiAdvanceAltarEplpState {
    _NotiAdvanceAltarEplpState();

    int32_t end_type;             // +0x00
    uint8_t has_available_ticket; // +0x04
    uint8_t reserved_05[3];       // +0x05
};

static_assert(sizeof(_NotiAdvanceAltarEplpState) == 0x08u,
              "advance-altar EPLP notification ABI must stay 0x08");

// IDA_EA: 0x8134720
// IDA_NAME: _ZN16NOTIPacketStruct27_NotiAdvanceAltarClearStageC2Ev
// The clear-stage notification payload is packed: the embedded stage record
// begins at +0x0d, so its two ints cannot be represented by a naturally
// aligned base class.
struct _NotiAdvanceAltarClearStage {
    _NotiAdvanceAltarClearStage();

    int32_t stage_type;                 // +0x00
    int32_t clear_survival_round_no;    // +0x04
    int32_t survival_best;              // +0x08 (sign-extended from data +0x18)
    uint8_t improved_survival_record;   // +0x0c
    int32_t stage_index;                // +0x0d
    int32_t stage_difficulty;           // +0x11
    int32_t reward_star;                // +0x15
    uint8_t reserved_19[3];             // +0x19
};

static_assert(sizeof(_NotiAdvanceAltarClearStage) == 0x1cu,
              "advance-altar clear-stage notification ABI must stay 0x1C");

}  // namespace NOTIPacketStruct

namespace advancealtar {

// IDA unit: 05_world_enter_village/advancealtar/_MapMonsterSummary.
struct _MapMonsterSummary {
    _MapMonsterSummary();

    int32_t map_monster_id;       // +0x00; _mapMonster +0x2c
    uint16_t unique_id;           // +0x04; ProcStage sequence
    int16_t tower_index;          // +0x06; tower FieldType::INDEX
    uint8_t tower_level;          // +0x08; purchased tower level
    uint8_t map_value_18;         // +0x09; _mapMonster +0x18
    uint8_t map_value_20;         // +0x0a; _mapMonster +0x20
    uint8_t map_value_1c;         // +0x0b; _mapMonster +0x1c
    uint8_t killed_;              // +0x0c; die-monster command marks it once
};

}  // namespace advancealtar

#pragma pack(pop)


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PACKET_STRUCTS_H_
