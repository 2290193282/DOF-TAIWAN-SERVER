#ifndef OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GROWTH_H_
#define OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GROWTH_H_

#include <stdint.h>


namespace equipment_growth {

// Immutable 0828 prototype identity projected out of Equipment.lst scripts.
// Keeping this sidecar avoids moving the fixed 0627 CEquipItem ABI.
struct PrototypeIdentity {
    PrototypeIdentity();

    uint8_t type;
    uint8_t stage;
};

// Strictly imports Etc/equipment_grow.etc.  Publication is transactional: a
// malformed file never replaces the last complete configuration.
bool ImportEquipmentGrowthScript(const char* filename);
bool IsConfigLoaded();

void RegisterPrototype(int item_id, uint8_t type, uint8_t stage);
bool FindPrototype(int item_id, PrototypeIdentity* identity);
unsigned int GetPrototypeCount();

// Returns the absolute progress ceiling for a type/stage pair, or zero when
// the pair is absent/final.  The shipped 0828 table has type 1, stages 1..7.
uint16_t GetStageMaximum(uint8_t type, uint8_t stage);

// RECONSTRUCTED producer for the server-only `death` award.  The exact 0828
// server implementation is unavailable; this owner applies the PVF point,
// monster-kind and difficulty corrections, then the per-stage fatigue
// threshold/percentage while maintaining the packet-visible u8 state.
uint16_t CalculateDeathAward(uint8_t type,
                             uint8_t stage,
                             int monster_kind,
                             int monster_level,
                             int character_level,
                             int dungeon_difficulty,
                             uint8_t current_fatigue_state,
                             uint8_t* next_fatigue_state);

}  // namespace equipment_growth_0828

#endif  // OLDSRV_OLD_DATA_PVF_ETC_EQUIPMENT_GROWTH_0828_H_
