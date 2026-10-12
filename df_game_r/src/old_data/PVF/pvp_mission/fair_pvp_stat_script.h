#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_STAT_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_STAT_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>


struct CharacterKind {
    CharacterKind();
    CharacterKind(uint8_t charac_job_value,
                  uint8_t first_grow_type_value);

    bool operator<(const CharacterKind& rhs) const;

    uint8_t charac_job;
    uint8_t first_grow_type;
};

struct CharacterStatus {
    uint32_t max_hp;
    uint32_t max_mp;
    uint16_t physical_attack;
    uint16_t physical_defense;
    uint16_t magical_attack;
    uint16_t magical_defense;
    uint16_t element_resist[4];
    uint16_t spec_property[17];
    int32_t inventory_limit;
    int16_t hp_regen_speed;
    int16_t mp_regen_speed;
    uint32_t move_speed;
    uint16_t attack_speed;
    uint16_t cast_speed;
    int16_t hit_recovery;
    int16_t jump_power;
    int32_t weight;
};

class FairPvPStatScript {
public:
    FairPvPStatScript();
    ~FairPvPStatScript();

    void clear();
    CharacterStatus* FindCharacStat(int charac_job,
                                    int first_grow_type);

    std::map<CharacterKind, CharacterStatus> character_stats;
};

static_assert(sizeof(CharacterKind) == 0x02u,
              "CharacterKind IDA ABI size must stay 0x02");
static_assert(offsetof(CharacterKind, charac_job) == 0x00u,
              "CharacterKind::charac_job must stay at +0x00");
static_assert(offsetof(CharacterKind, first_grow_type) == 0x01u,
              "CharacterKind::first_grow_type must stay at +0x01");

static_assert(sizeof(CharacterStatus) == 0x54u,
              "CharacterStatus IDA ABI size must stay 0x54");
static_assert(alignof(CharacterStatus) == 0x04u,
              "CharacterStatus IDA ABI alignment must stay 4");
static_assert(offsetof(CharacterStatus, max_hp) == 0x00u,
              "CharacterStatus::max_hp must stay at +0x00");
static_assert(offsetof(CharacterStatus, max_mp) == 0x04u,
              "CharacterStatus::max_mp must stay at +0x04");
static_assert(offsetof(CharacterStatus, physical_attack) == 0x08u,
              "CharacterStatus::physical_attack must stay at +0x08");
static_assert(offsetof(CharacterStatus, physical_defense) == 0x0Au,
              "CharacterStatus::physical_defense must stay at +0x0a");
static_assert(offsetof(CharacterStatus, magical_attack) == 0x0Cu,
              "CharacterStatus::magical_attack must stay at +0x0c");
static_assert(offsetof(CharacterStatus, magical_defense) == 0x0Eu,
              "CharacterStatus::magical_defense must stay at +0x0e");
static_assert(offsetof(CharacterStatus, element_resist) == 0x10u,
              "CharacterStatus::element_resist must stay at +0x10");
static_assert(offsetof(CharacterStatus, spec_property) == 0x18u,
              "CharacterStatus::spec_property must stay at +0x18");
static_assert(offsetof(CharacterStatus, inventory_limit) == 0x3Cu,
              "CharacterStatus::inventory_limit must stay at +0x3c");
static_assert(offsetof(CharacterStatus, hp_regen_speed) == 0x40u,
              "CharacterStatus::hp_regen_speed must stay at +0x40");
static_assert(offsetof(CharacterStatus, mp_regen_speed) == 0x42u,
              "CharacterStatus::mp_regen_speed must stay at +0x42");
static_assert(offsetof(CharacterStatus, move_speed) == 0x44u,
              "CharacterStatus::move_speed must stay at +0x44");
static_assert(offsetof(CharacterStatus, attack_speed) == 0x48u,
              "CharacterStatus::attack_speed must stay at +0x48");
static_assert(offsetof(CharacterStatus, cast_speed) == 0x4Au,
              "CharacterStatus::cast_speed must stay at +0x4a");
static_assert(offsetof(CharacterStatus, hit_recovery) == 0x4Cu,
              "CharacterStatus::hit_recovery must stay at +0x4c");
static_assert(offsetof(CharacterStatus, jump_power) == 0x4Eu,
              "CharacterStatus::jump_power must stay at +0x4e");
static_assert(offsetof(CharacterStatus, weight) == 0x50u,
              "CharacterStatus::weight must stay at +0x50");

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(FairPvPStatScript) == 0x18u,
              "FairPvPStatScript IDA ABI size must stay 0x18");
static_assert(offsetof(FairPvPStatScript, character_stats) == 0x00u,
              "FairPvPStatScript::character_stats must stay at +0x00");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_FAIR_PVP_STAT_SCRIPT_H_
