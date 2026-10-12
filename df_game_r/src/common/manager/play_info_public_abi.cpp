#include "common/manager/play_info_public_abi.h"

namespace {

int16_t AddInt16Wrapping(int16_t lhs, int16_t rhs) {
    return static_cast<int16_t>(static_cast<uint16_t>(lhs) +
                                static_cast<uint16_t>(rhs));
}

int16_t SubtractInt16Wrapping(int16_t lhs, int16_t rhs) {
    return static_cast<int16_t>(static_cast<uint16_t>(lhs) -
                                static_cast<uint16_t>(rhs));
}

int32_t AddInt32Wrapping(int32_t lhs, int32_t rhs) {
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) +
                                static_cast<uint32_t>(rhs));
}

int32_t SubtractInt32Wrapping(int32_t lhs, int32_t rhs) {
    return static_cast<int32_t>(static_cast<uint32_t>(lhs) -
                                static_cast<uint32_t>(rhs));
}

}  // namespace

// IDA_EA: 0x8486640
// IDA_NAME: _ZN8PlayInfo5ResetEv
// DEMANGLED: PlayInfo::Reset(void)
PlayInfo* PlayInfo::Reset() {
    clear_time_seconds_ = 0;
    die_count_ = 0;
    hp_consume_ = 0;
    mp_consume_ = 0;
    hit_count_ = 0;
    hit_per_average_damage_ = 0;
    hp_recovery_ = 0;
    mp_recovery_ = 0;
    party_member_count_ = 0;
    character_level_ = 0;
    dungeon_exp_ = 0;
    value_2c_ = 0;
    dungeon_bonus_exp_ = 0;
    reason_one_or_two_exp_ = 0;
    value_38_ = 0;
    return this;
}

// IDA_EA: 0x84866D4
// IDA_NAME: _ZN8PlayInfoC2Ev
// DEMANGLED: PlayInfo::PlayInfo(void)
PlayInfo::PlayInfo() {
    Reset();
}

// IDA_EA: 0x84866E8
// IDA_NAME: _ZN8PlayInfopLERKS_
// DEMANGLED: PlayInfo::operator+=(PlayInfo const&)
PlayInfo& PlayInfo::operator+=(const PlayInfo& rhs) {
    clear_time_seconds_ += rhs.clear_time_seconds_;
    die_count_ += rhs.die_count_;
    hp_consume_ += rhs.hp_consume_;
    mp_consume_ += rhs.mp_consume_;
    hit_count_ += rhs.hit_count_;
    hit_per_average_damage_ += rhs.hit_per_average_damage_;
    hp_recovery_ += rhs.hp_recovery_;
    mp_recovery_ += rhs.mp_recovery_;
    party_member_count_ += rhs.party_member_count_;
    character_level_ += rhs.character_level_;
    dungeon_exp_ += rhs.dungeon_exp_;
    dungeon_bonus_exp_ += rhs.dungeon_bonus_exp_;
    reason_one_or_two_exp_ += rhs.reason_one_or_two_exp_;
    return *this;
}

// IDA_EA: 0x8486836
// IDA_NAME: _ZN5IDataC2Ev
// DEMANGLED: IData::IData(void)
IData::IData() {}

// IDA_EA: 0x8146300
// IDA_NAME: _ZN15_Additioal_info3addERKS_
// DEMANGLED: _Additioal_info::add(_Additioal_info const&)
_Additioal_info* _Additioal_info::add(const _Additioal_info& rhs) {
    max_hp += rhs.max_hp;
    max_mp += rhs.max_mp;
    physical_attack += rhs.physical_attack;
    physical_defense += rhs.physical_defense;
    magical_attack += rhs.magical_attack;
    magical_defense += rhs.magical_defense;
    for (int index = 0; index <= 3; ++index) {
        element_resist[index] += rhs.element_resist[index];
    }
    for (int index = 0; index <= 16; ++index) {
        spec_property[index] += rhs.spec_property[index];
    }
    inventory_weight = AddInt32Wrapping(inventory_weight,
                                        rhs.inventory_weight);
    hp_regen_speed = AddInt16Wrapping(hp_regen_speed,
                                      rhs.hp_regen_speed);
    mp_regen_speed = AddInt16Wrapping(mp_regen_speed,
                                      rhs.mp_regen_speed);
    move_speed += rhs.move_speed;
    attack_speed += rhs.attack_speed;
    cast_speed += rhs.cast_speed;
    hit_recovery = AddInt16Wrapping(hit_recovery, rhs.hit_recovery);
    jump_power = AddInt16Wrapping(jump_power, rhs.jump_power);
    character_weight = AddInt32Wrapping(character_weight,
                                        rhs.character_weight);
    return this;
}

// IDA_EA: 0x8348254
// IDA_NAME: _ZN15_Additioal_infoC2Ev
// DEMANGLED: _Additioal_info::_Additioal_info(void)
_Additioal_info::_Additioal_info() {
    clear();
}

// IDA_EA: 0x8348268
// IDA_NAME: _ZN15_Additioal_info5clearEv
// DEMANGLED: _Additioal_info::clear(void)
void* _Additioal_info::clear() {
    max_hp = 0u;
    max_mp = 0u;
    physical_attack = 0u;
    physical_defense = 0u;
    magical_attack = 0u;
    magical_defense = 0u;
    for (int index = 0; index <= 3; ++index) {
        element_resist[index] = 0u;
    }
    for (int index = 0; index <= 16; ++index) {
        spec_property[index] = 0u;
    }
    inventory_weight = 0;
    hp_regen_speed = 0;
    mp_regen_speed = 0;
    move_speed = 0u;
    attack_speed = 0u;
    cast_speed = 0u;
    hit_recovery = 0;
    jump_power = 0;
    character_weight = 0;
    return this;
}

// IDA_EA: 0x8693CEA
// IDA_NAME: _ZN15_Additioal_infopLERKS_
// DEMANGLED: _Additioal_info::operator+=(_Additioal_info const&)
_Additioal_info& _Additioal_info::operator+=(const _Additioal_info& rhs) {
    add(rhs);
    return *this;
}

// IDA_EA: 0x8693D04
// IDA_NAME: _ZN15_Additioal_info8subtractERKS_
// DEMANGLED: _Additioal_info::subtract(_Additioal_info const&)
_Additioal_info* _Additioal_info::subtract(const _Additioal_info& rhs) {
    max_hp -= rhs.max_hp;
    max_mp -= rhs.max_mp;
    physical_attack -= rhs.physical_attack;
    physical_defense -= rhs.physical_defense;
    magical_attack -= rhs.magical_attack;
    magical_defense -= rhs.magical_defense;
    for (int index = 0; index <= 3; ++index) {
        element_resist[index] -= rhs.element_resist[index];
    }
    for (int index = 0; index <= 16; ++index) {
        spec_property[index] -= rhs.spec_property[index];
    }
    inventory_weight = SubtractInt32Wrapping(inventory_weight,
                                             rhs.inventory_weight);
    hp_regen_speed = SubtractInt16Wrapping(hp_regen_speed,
                                           rhs.hp_regen_speed);
    mp_regen_speed = SubtractInt16Wrapping(mp_regen_speed,
                                           rhs.mp_regen_speed);
    move_speed -= rhs.move_speed;
    attack_speed -= rhs.attack_speed;
    cast_speed -= rhs.cast_speed;
    hit_recovery = SubtractInt16Wrapping(hit_recovery,
                                         rhs.hit_recovery);
    jump_power = AddInt16Wrapping(jump_power, rhs.jump_power);
    character_weight = AddInt32Wrapping(character_weight,
                                        rhs.character_weight);
    return this;
}
