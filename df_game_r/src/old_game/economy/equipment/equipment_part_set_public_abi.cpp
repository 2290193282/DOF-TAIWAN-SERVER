#include "old_game/economy/equipment/equipment_part_set_public_abi.h"

#include <new>

#include "old_data/PVF/equipment/equipment_script.h"

std::map<int, STEquipmentPartSetData> g_equipmentPartSetMap_
    __asm__("g_equipmentPartSetMap_");

std::map<int, STEquipmentPartSetData> g_equipmentEffectSetMap_
    __asm__("g_equipmentEffectSetMap_");

bool FindEquipmentPartSetParameterExtension0828(
    int part_set_index,
    int parameter_key,
    equipment_parameter_0828::Extension* extension) {
    if (extension) *extension = equipment_parameter_0828::Extension();
    const std::map<int, STEquipmentPartSetData>::const_iterator set =
        g_equipmentPartSetMap_.find(part_set_index);
    if (set == g_equipmentPartSetMap_.end() || !set->second.script) {
        return false;
    }
    const std::map<int, EquipmentParameterInfo>::const_iterator parameter =
        set->second.script->set_parameters_678.find(parameter_key);
    if (parameter == set->second.script->set_parameters_678.end()) {
        return false;
    }
    return equipment_parameter_0828::Find(&parameter->second, extension);
}

// IDA_EA: 0x891AFD0
// IDA_NAME: _ZN23STEquipmentPartSetPieceC2Ev
// DEMANGLED: STEquipmentPartSetPiece::STEquipmentPartSetPiece(void)
STEquipmentPartSetPiece::STEquipmentPartSetPiece()
    : part_name(), part_type(-1), part_grade(-1), part_rarity(-1) {}

// IDA_EA: 0x8928596
// IDA_NAME: _ZN23STEquipmentPartSetPieceC2ERKS_
// DEMANGLED: STEquipmentPartSetPiece::STEquipmentPartSetPiece(STEquipmentPartSetPiece const&)
STEquipmentPartSetPiece::STEquipmentPartSetPiece(
    const STEquipmentPartSetPiece& rhs)
    : part_name(rhs.part_name),
      part_type(rhs.part_type),
      part_grade(rhs.part_grade),
      part_rarity(rhs.part_rarity) {}

// IDA_EA: 0x891C6D8
// IDA_NAME: _ZN23STEquipmentPartSetPieceD2Ev
// DEMANGLED: STEquipmentPartSetPiece::~STEquipmentPartSetPiece()
STEquipmentPartSetPiece::~STEquipmentPartSetPiece() {}

// IDA_EA: 0x8928632
// IDA_NAME: _ZN23STEquipmentPartSetPieceaSERKS_
// DEMANGLED: STEquipmentPartSetPiece::operator=(STEquipmentPartSetPiece const&)
STEquipmentPartSetPiece& STEquipmentPartSetPiece::operator=(
    const STEquipmentPartSetPiece& rhs) {
    if (this != &rhs) {
        part_name = rhs.part_name;
        part_type = rhs.part_type;
        part_grade = rhs.part_grade;
        part_rarity = rhs.part_rarity;
    }
    return *this;
}

// IDA_EA: 0x891B002
// IDA_NAME: _ZN22STEquipmentPartSetDataC2Ev
// DEMANGLED: STEquipmentPartSetData::STEquipmentPartSetData(void)
STEquipmentPartSetData::STEquipmentPartSetData()
    : set_id(-1), script_filename(), script(0), pieces() {}

// IDA_EA: 0x89289DA
// IDA_NAME: _ZN22STEquipmentPartSetDataC2ERKS_
// DEMANGLED: STEquipmentPartSetData::STEquipmentPartSetData(STEquipmentPartSetData const&)
STEquipmentPartSetData::STEquipmentPartSetData(
    const STEquipmentPartSetData& rhs)
    : set_id(rhs.set_id),
      script_filename(rhs.script_filename),
      script(rhs.script),
      pieces(rhs.pieces) {}

// IDA_EA: 0x8907348
// IDA_NAME: _ZN22STEquipmentPartSetDataD2Ev
// DEMANGLED: STEquipmentPartSetData::~STEquipmentPartSetData()
STEquipmentPartSetData::~STEquipmentPartSetData() {
    if (script != 0) {
        STEquipmentScript* ptr = script;
        ptr->~STEquipmentScript();
        ::operator delete(ptr);
    }
    script = 0;
}

// IDA_EA: 0x891C6EC
// IDA_NAME: _ZN22STEquipmentPartSetDataaSERKS_
// DEMANGLED: STEquipmentPartSetData::operator=(STEquipmentPartSetData const&)
STEquipmentPartSetData& STEquipmentPartSetData::operator=(
    const STEquipmentPartSetData& rhs) {
    if (this != &rhs) {
        set_id = rhs.set_id;
        script_filename = rhs.script_filename;
        script = rhs.script;
        pieces = rhs.pieces;
    }
    return *this;
}
