#include "old_data/PVF/passiveobject/passive_object_script_types.h"

// IDA_EA: 0x8A3AD5A
// IDA_NAME: _ZN23SpeedCtrlPassiveObjInfoC2Ev
// DEMANGLED: SpeedCtrlPassiveObjInfo::SpeedCtrlPassiveObjInfo(void)
SpeedCtrlPassiveObjInfo::SpeedCtrlPassiveObjInfo()
    : passive_object_index(-1), enabled(0) {}

// IDA_EA: 0x8A4EAB2
// IDA_NAME: _ZN23SpeedCtrlPassiveObjInfoD2Ev
// DEMANGLED: SpeedCtrlPassiveObjInfo::~SpeedCtrlPassiveObjInfo()
SpeedCtrlPassiveObjInfo::~SpeedCtrlPassiveObjInfo() {}

// IDA_EA: 0x8A3AECA
// IDA_NAME: _ZN15WarningDistanceC2Ev
// DEMANGLED: WarningDistance::WarningDistance(void)
WarningDistance::WarningDistance() : first(0), second(0), third(0) {}

// IDA_EA: 0x8A3AEEC
// IDA_NAME: _ZN15WarningDistanceC2Eiii
// DEMANGLED: WarningDistance::WarningDistance(int,int,int)
WarningDistance::WarningDistance(int first_value,
                                 int second_value,
                                 int third_value)
    : first(first_value), second(second_value), third(third_value) {}

// IDA_EA: 0x8A3DB5E
// IDA_NAME: _ZN15RandomItemFairyC2Ev
// DEMANGLED: RandomItemFairy::RandomItemFairy(void)
RandomItemFairy::RandomItemFairy() { clear(); }

// IDA_EA: 0x8A3DB72
// IDA_NAME: _ZN15RandomItemFairy5clearEv
// DEMANGLED: RandomItemFairy::clear(void)
bool RandomItemFairy::clear() {
    item_index = 0;
    item_count = 0;
    probability = 0;
    condition = 0;
    racing_dungeon_probabilities.clear();
    return false;
}

// IDA_EA: 0x8A4F436
// IDA_NAME: _ZN15RandomItemFairyD2Ev
// DEMANGLED: RandomItemFairy::~RandomItemFairy()
RandomItemFairy::~RandomItemFairy() {}

// IDA_EA: 0x8A3DA06
// IDA_NAME: _ZN24STPassiveObjectAddEffect5clearEv
// DEMANGLED: STPassiveObjectAddEffect::clear(void)
void STPassiveObjectAddEffect::clear() {}

// IDA_EA: 0x8A3DA0C
// IDA_NAME: _ZN24STPassiveObjectAddEffectC2Ev
// DEMANGLED: STPassiveObjectAddEffect::STPassiveObjectAddEffect(void)
STPassiveObjectAddEffect::STPassiveObjectAddEffect()
    : effect_files(), effect_values() {
    clear();
}

// IDA_EA: 0x8A3E110
// IDA_NAME: _ZN24STPassiveObjectAddEffectD2Ev
// DEMANGLED: STPassiveObjectAddEffect::~STPassiveObjectAddEffect()
STPassiveObjectAddEffect::~STPassiveObjectAddEffect() {}

// IDA_EA: 0x8A3DA5A
// IDA_NAME: _ZN37STPassiveObjectAdditionalParticleInfo5clearEv
// DEMANGLED: STPassiveObjectAdditionalParticleInfo::clear(void)
std::string* STPassiveObjectAdditionalParticleInfo::clear() {
    file_name.clear();
    create_sound.clear();
    reserved_name.clear();
    create_delay = 0;
    follow_parent[0] = 0;
    follow_parent[1] = 0;
    follow_parent[2] = 0;
    loop = 0;
    return &file_name;
}

// IDA_EA: 0x8A3DAD0
// IDA_NAME: _ZN37STPassiveObjectAdditionalParticleInfoC2Ev
// DEMANGLED: STPassiveObjectAdditionalParticleInfo::STPassiveObjectAdditionalParticleInfo(void)
STPassiveObjectAdditionalParticleInfo::STPassiveObjectAdditionalParticleInfo()
    : file_name(), create_sound(), reserved_name(), create_delay(0),
      follow_parent(), loop(0), padding_1d(0), padding_1e(0), padding_1f(0) {
    clear();
}

// IDA_EA: 0x8A3D8C2
// IDA_NAME: _ZN22STPassiveObjectDestroy5clearEv
// DEMANGLED: STPassiveObjectDestroy::clear(void)
void STPassiveObjectDestroy::clear() {
    destroy_type = -1;
    destroy_delay = 0;
    field_38 = -1;
    field_3c = -1;
    field_40 = -1;
    item_indices.clear();
    value_list.clear();
    effect_files.clear();
}

// IDA_EA: 0x8A3D926
// IDA_NAME: _ZN22STPassiveObjectDestroyC2Ev
// DEMANGLED: STPassiveObjectDestroy::STPassiveObjectDestroy(void)
STPassiveObjectDestroy::STPassiveObjectDestroy()
    : destroy_type(0), destroy_delay(0), item_indices(), effect_files(),
      sound_files(), value_list(), field_38(0), field_3c(0), field_40(0),
      additional_values() {
    clear();
}

// IDA_EA: 0x8A3E04C
// IDA_NAME: _ZN22STPassiveObjectDestroyD2Ev
// DEMANGLED: STPassiveObjectDestroy::~STPassiveObjectDestroy()
STPassiveObjectDestroy::~STPassiveObjectDestroy() {}

// IDA_EA: 0x8A3AD70
// IDA_NAME: _ZN26PassiveObjGeneratorPatternC2Ev
// DEMANGLED: PassiveObjGeneratorPattern::PassiveObjGeneratorPattern(void)
PassiveObjGeneratorPattern::PassiveObjGeneratorPattern()
    : enabled(1), pattern_name(), first_value(0), second_value(0), values() {
    pattern_name.clear();
    values.clear();
}

// IDA_EA: 0x8A3E70A
// IDA_NAME: _ZN26PassiveObjGeneratorPatternD2Ev
// DEMANGLED: PassiveObjGeneratorPattern::~PassiveObjGeneratorPattern()
PassiveObjGeneratorPattern::~PassiveObjGeneratorPattern() {}

// IDA_EA: 0x8A47C6C
// IDA_NAME: _ZN26PassiveObjGeneratorPatternC2ERKS_
// DEMANGLED: PassiveObjGeneratorPattern::PassiveObjGeneratorPattern(PassiveObjGeneratorPattern const&)
PassiveObjGeneratorPattern::PassiveObjGeneratorPattern(
    const PassiveObjGeneratorPattern& other)
    : enabled(other.enabled), pattern_name(other.pattern_name),
      first_value(other.first_value),
      second_value(other.second_value), values(other.values) {}

// IDA_EA: 0x8A47D72
// IDA_NAME: _ZN26PassiveObjGeneratorPatternaSERKS_
// DEMANGLED: PassiveObjGeneratorPattern::operator=(PassiveObjGeneratorPattern const&)
PassiveObjGeneratorPattern& PassiveObjGeneratorPattern::operator=(
    const PassiveObjGeneratorPattern& other) {
    enabled = other.enabled;
    pattern_name = other.pattern_name;
    first_value = other.first_value;
    second_value = other.second_value;
    values = other.values;
    return *this;
}

// IDA_EA: 0x8A3AE0A
// IDA_NAME: _ZN27PassiveObjGeneratorSequenceC2Ev
// DEMANGLED: PassiveObjGeneratorSequence::PassiveObjGeneratorSequence(void)
PassiveObjGeneratorSequence::PassiveObjGeneratorSequence()
    : sequence_name(), value(0) {
    sequence_name.clear();
}

// IDA_EA: 0x8A3E756
// IDA_NAME: _ZN27PassiveObjGeneratorSequenceD2Ev
// DEMANGLED: PassiveObjGeneratorSequence::~PassiveObjGeneratorSequence()
PassiveObjGeneratorSequence::~PassiveObjGeneratorSequence() {}

// IDA_EA: 0x8A4812A
// IDA_NAME: _ZN27PassiveObjGeneratorSequenceC2ERKS_
// DEMANGLED: PassiveObjGeneratorSequence::PassiveObjGeneratorSequence(PassiveObjGeneratorSequence const&)
PassiveObjGeneratorSequence::PassiveObjGeneratorSequence(
    const PassiveObjGeneratorSequence& other)
    : sequence_name(other.sequence_name), value(other.value) {}

// IDA_EA: 0x8A481D4
// IDA_NAME: _ZN27PassiveObjGeneratorSequenceaSERKS_
// DEMANGLED: PassiveObjGeneratorSequence::operator=(PassiveObjGeneratorSequence const&)
PassiveObjGeneratorSequence& PassiveObjGeneratorSequence::operator=(
    const PassiveObjGeneratorSequence& other) {
    sequence_name = other.sequence_name;
    value = other.value;
    return *this;
}

// IDA_EA: 0x8A3AE56
// IDA_NAME: _ZN25PassiveObjGeneratorObjectC2Ev
// DEMANGLED: PassiveObjGeneratorObject::PassiveObjGeneratorObject(void)
PassiveObjGeneratorObject::PassiveObjGeneratorObject()
    : object_index(0), probability(0.0f), warning_name() {}

// IDA_EA: 0x8A3E6F4
// IDA_NAME: _ZN25PassiveObjGeneratorObjectD2Ev
// DEMANGLED: PassiveObjGeneratorObject::~PassiveObjGeneratorObject()
PassiveObjGeneratorObject::~PassiveObjGeneratorObject() {}

// IDA_EA: 0x8A47834
// IDA_NAME: _ZN25PassiveObjGeneratorObjectC2ERKS_
// DEMANGLED: PassiveObjGeneratorObject::PassiveObjGeneratorObject(PassiveObjGeneratorObject const&)
PassiveObjGeneratorObject::PassiveObjGeneratorObject(
    const PassiveObjGeneratorObject& other)
    : object_index(other.object_index), probability(other.probability),
      warning_name(other.warning_name) {}

// IDA_EA: 0x8A478EE
// IDA_NAME: _ZN25PassiveObjGeneratorObjectaSERKS_
// DEMANGLED: PassiveObjGeneratorObject::operator=(PassiveObjGeneratorObject const&)
PassiveObjGeneratorObject& PassiveObjGeneratorObject::operator=(
    const PassiveObjGeneratorObject& other) {
    object_index = other.object_index;
    probability = other.probability;
    warning_name = other.warning_name;
    return *this;
}

// IDA_EA: 0x8A3E156
// IDA_NAME: _ZN23PassiveObjGeneratorInfoD2Ev
// DEMANGLED: PassiveObjGeneratorInfo::~PassiveObjGeneratorInfo()
PassiveObjGeneratorInfo::~PassiveObjGeneratorInfo() {}

// IDA_EA: 0x8A3E8AE
// IDA_NAME: _ZN23PassiveObjGeneratorInfoC2Ev
// DEMANGLED: PassiveObjGeneratorInfo::PassiveObjGeneratorInfo(void)
PassiveObjGeneratorInfo::PassiveObjGeneratorInfo()
    : patterns(), sequences(), objects() {}

// IDA_EA: 0x8A3AF0C
// IDA_NAME: _ZN21PassiveIObjSniperInfoC2Ev
// DEMANGLED: PassiveIObjSniperInfo::PassiveIObjSniperInfo(void)
PassiveIObjSniperInfo::PassiveIObjSniperInfo()
    : first_values(), second_values(), third_values(), fourth_values(),
      field_30(0), field_34(0), field_38(0), padding_39(0), padding_3a(0),
      padding_3b(0), field_3c(0), field_40(0), padding_41(0),
      padding_42(0), padding_43(0), fields_44_58(), field_5c(0),
      field_60(0), field_64(0), field_68(0), field_6c(0), field_70(0),
      fields_74_7c(), field_80(0), sniper_type(0) {
    clear();
}

// IDA_EA: 0x8A3AFC0
// IDA_NAME: _ZN21PassiveIObjSniperInfo5clearEv
// DEMANGLED: PassiveIObjSniperInfo::clear(void)
PassiveIObjSniperInfo* PassiveIObjSniperInfo::clear() {
    first_values.clear();
    second_values.clear();
    third_values.clear();
    fourth_values.clear();
    field_30 = 0;
    field_34 = 0;
    field_38 = 0;
    field_3c = 0;
    field_40 = 0;
    for (int i = 0; i != 6; ++i) {
        fields_44_58[i] = 0;
    }
    field_5c = 0;
    field_60 = 0;
    field_64 = 0;
    field_6c = 0;
    field_70 = 0;
    for (int i = 0; i != 3; ++i) {
        fields_74_7c[i] = 0;
    }
    return this;
}

// IDA_EA: 0x8A3E1C4
// IDA_NAME: _ZN21PassiveIObjSniperInfoD2Ev
// DEMANGLED: PassiveIObjSniperInfo::~PassiveIObjSniperInfo()
PassiveIObjSniperInfo::~PassiveIObjSniperInfo() {}

// IDA_EA: 0x8A3E76A
// IDA_NAME: _ZN21PassiveIObjSniperInfoaSERKS_
// DEMANGLED: PassiveIObjSniperInfo::operator=(PassiveIObjSniperInfo const&)
PassiveIObjSniperInfo& PassiveIObjSniperInfo::operator=(
    const PassiveIObjSniperInfo& other) {
    first_values = other.first_values;
    second_values = other.second_values;
    third_values = other.third_values;
    fourth_values = other.fourth_values;
    field_30 = other.field_30;
    field_34 = other.field_34;
    field_38 = other.field_38;
    field_3c = other.field_3c;
    field_40 = other.field_40;
    for (int i = 0; i != 6; ++i) {
        fields_44_58[i] = other.fields_44_58[i];
    }
    field_5c = other.field_5c;
    field_60 = other.field_60;
    field_64 = other.field_64;
    field_68 = other.field_68;
    field_6c = other.field_6c;
    field_70 = other.field_70;
    for (int i = 0; i != 3; ++i) {
        fields_74_7c[i] = other.fields_74_7c[i];
    }
    field_80 = other.field_80;
    sniper_type = other.sniper_type;
    return *this;
}

// IDA_EA: 0x8A3A538
// IDA_NAME: _ZN21STPassiveObjectScriptC2Ev
// DEMANGLED: STPassiveObjectScript::STPassiveObjectScript(void)
STPassiveObjectScript::STPassiveObjectScript()
    : object_index(0), object_name(), resource_name(), fields_0c_24(),
      spawn_values(), display_name(), first_string_values(), first_effect(),
      second_string_values(), second_effect(), state_58(0), state_59(0),
      state_5a(0), padding_5b(0), field_5c(0), state_60(0), state_61(0),
      state_62(0), state_63(0), third_effect(), fourth_effect(),
      third_string_values(), additional_spawn_values(), fourth_string_values(),
      script_filename(), field_94(0), field_98(0), field_9c(0), field_a0(0),
      field_a4(0), field_a8(0), field_ac(0), field_b0(0), state_b4(0),
      padding_b5(0), padding_b6(0), padding_b7(0), field_b8(0), field_bc(0),
      field_c0(0.0f), field_c4(0), field_c8(0), field_cc(0), field_d0(0),
      state_d4(0), state_d5(0), padding_d6(0), padding_d7(0), destroy(),
      add_effect(), additional_particles(), for_tournament(0), for_school(0),
      die_when_parent_dies(0), transparent_on_meet_player(0),
      pvp_observer_transparency(0), reserved_152(0),
      particle_name(), state_158(0), state_159(0), padding_15a(0),
      padding_15b(0), field_15c(0), particle_values(), state_16c(0), padding_16d(0),
      padding_16e(0), padding_16f(0), field_170(0.0f), field_174(0),
      field_178(0), speed_control_name(), speed_control_objects(),
      generator_patterns(), generator_sequences(), generator_objects(),
      generator_info(), warning_distance_near(), warning_distance_far(),
      sniper_info(), random_item_fairies(), state_280(0), padding_281(0),
      padding_282(0), padding_283(0), final_name(), blocking_area(0),
      apply_parent_ability_category(0), reserved_295(),
      party_hp_max_rates(), damage_absorb(0.0f), minimap_marker_mode(0),
      minimap_marker_image(), absolute_zpos(0) {
    clear();
}

// IDA_EA: 0x8A3A9E6
// IDA_NAME: _ZN21STPassiveObjectScript5clearEv
// DEMANGLED: STPassiveObjectScript::clear(void)
STPassiveObjectScript* STPassiveObjectScript::clear() {
    object_index = 0;
    object_name.clear();
    resource_name.clear();
    for (int i = 0; i != 7; ++i) {
        fields_0c_24[i] = 0;
    }
    fields_0c_24[3] = 1;
    spawn_values.clear();
    display_name.clear();
    first_string_values.clear();
    fourth_effect.clear();
    third_effect.clear();
    third_string_values.clear();
    first_effect.clear();
    second_string_values.clear();
    second_effect.clear();
    state_58 = 0;
    state_59 = 0;
    state_5a = 0;
    field_5c = -1;
    state_60 = 0;
    state_61 = 0;
    state_62 = 0;
    additional_spawn_values.clear();
    fourth_string_values.clear();
    script_filename.clear();
    field_94 = -1;
    field_98 = 0;
    field_9c = 0;
    field_a0 = 1;
    field_a4 = 0;
    field_a8 = 0;
    field_ac = 0;
    field_b0 = 500;
    state_b4 = 0;
    field_b8 = 0;
    field_bc = 0;
    field_c0 = 176.0f;
    field_c4 = 0;
    field_c8 = 0;
    field_cc = -1;
    field_d0 = 0;
    state_d4 = 0;
    state_d5 = 0;
    additional_particles.clear();
    particle_name.clear();
    for_tournament = 0;
    for_school = 0;
    die_when_parent_dies = 0;
    transparent_on_meet_player = 0;
    pvp_observer_transparency = 0;
    reserved_152 = 0;
    state_158 = 0;
    state_159 = 0;
    field_15c = 0;
    particle_values.clear();
    state_16c = 0;
    field_170 = -1.0f;
    field_174 = 0;
    field_178 = 0;
    speed_control_name.clear();
    speed_control_objects.clear();
    sniper_info.sniper_type = 2;
    state_280 = 0;
    blocking_area = 0;
    apply_parent_ability_category = 0;
    reserved_295[0] = 0;
    reserved_295[1] = 0;
    reserved_295[2] = 0;
    party_hp_max_rates.clear();
    damage_absorb = 0.0f;
    minimap_marker_mode = 0;
    minimap_marker_image.clear();
    absolute_zpos = 0;
    return this;
}

// IDA_EA: 0x8A3E25A
// IDA_NAME: _ZN21STPassiveObjectScriptD2Ev
// DEMANGLED: STPassiveObjectScript::~STPassiveObjectScript()
STPassiveObjectScript::~STPassiveObjectScript() {}
