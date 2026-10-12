#include "old_game/player/core/premium_runtime.h"

namespace WongWork {

// IDA_EA: 0x84E99FC
// IDA_NAME: _ZNK8WongWork12CUserPremium21GetOverEquipableLevelE18ENUM_EQUIPMENTTYPE
// DEMANGLED: WongWork::CUserPremium::GetOverEquipableLevel(ENUM_EQUIPMENTTYPE)const
int CUserPremium::GetOverEquipableLevel(
    ENUM_EQUIPMENTTYPE equipment_type) const {
    return over_equipable_level_[static_cast<int>(equipment_type)];
}

}  // namespace WongWork
