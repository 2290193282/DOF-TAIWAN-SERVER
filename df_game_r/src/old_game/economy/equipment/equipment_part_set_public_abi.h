#ifndef OLDSRV_GLOBAL_EQUIPMENT_PART_SET_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_EQUIPMENT_PART_SET_PUBLIC_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>

struct EquipmentParameterInfo;
struct STEquipmentScript;
namespace equipment_parameter_0828 {
struct Extension;
}

// IDA unit: 06_userinfo_stat_inventory/STEquipmentPartSetPiece.
struct STEquipmentPartSetPiece {
    STEquipmentPartSetPiece();
    STEquipmentPartSetPiece(const STEquipmentPartSetPiece& rhs);
    ~STEquipmentPartSetPiece();

    STEquipmentPartSetPiece& operator=(const STEquipmentPartSetPiece& rhs);

    std::string part_name;
    int32_t part_type;
    int32_t part_grade;
    int32_t part_rarity;
};

static_assert(sizeof(STEquipmentPartSetPiece) == 0x10u,
              "STEquipmentPartSetPiece IDA ABI size must stay 0x10");
static_assert(offsetof(STEquipmentPartSetPiece, part_type) == 0x04u,
              "STEquipmentPartSetPiece part_type offset must stay 0x04");
static_assert(offsetof(STEquipmentPartSetPiece, part_grade) == 0x08u,
              "STEquipmentPartSetPiece part_grade offset must stay 0x08");
static_assert(offsetof(STEquipmentPartSetPiece, part_rarity) == 0x0Cu,
              "STEquipmentPartSetPiece part_rarity offset must stay 0x0C");

// IDA unit: 06_userinfo_stat_inventory/STEquipmentPartSetData.
struct STEquipmentPartSetData {
    STEquipmentPartSetData();
    STEquipmentPartSetData(const STEquipmentPartSetData& rhs);
    ~STEquipmentPartSetData();

    STEquipmentPartSetData& operator=(const STEquipmentPartSetData& rhs);

    int32_t set_id;
    std::string script_filename;
    STEquipmentScript* script;
    std::vector<STEquipmentPartSetPiece> pieces;
};

static_assert(sizeof(STEquipmentPartSetData) == 0x18u,
              "STEquipmentPartSetData IDA ABI size must stay 0x18");
static_assert(offsetof(STEquipmentPartSetData, script_filename) == 0x04u,
              "STEquipmentPartSetData script_filename offset must stay 0x04");
static_assert(offsetof(STEquipmentPartSetData, script) == 0x08u,
              "STEquipmentPartSetData script offset must stay 0x08");
static_assert(offsetof(STEquipmentPartSetData, pieces) == 0x0Cu,
              "STEquipmentPartSetData pieces offset must stay 0x0C");

// IDA_EA: importEquipmentPartSetInfo @0x89073D9 clears/fills this map from
// Etc/EquipmentPartSet.etc; CDataManager::init iterates it at
// @0x835E4B9..0x835E745 for DB sync.
extern std::map<int, STEquipmentPartSetData> g_equipmentPartSetMap_
    __asm__("g_equipmentPartSetMap_");

// IDA_EA: importEquipmentEffectSetInfo @0x890793F uses the same value ABI.
extern std::map<int, STEquipmentPartSetData> g_equipmentEffectSetMap_
    __asm__("g_equipmentEffectSetMap_");

// Runtime-safe bridge for 0828-only EquipmentParameterInfo state.  Callers
// request one exact PVF threshold; this deliberately does not invent an
// accumulation rule across multiple active piece thresholds.
bool FindEquipmentPartSetParameterExtension0828(
    int part_set_index,
    int parameter_key,
    equipment_parameter_0828::Extension* extension);

// IDA_EA: 0x89073D9 / active init callsite 0x835E41B.
// IDA_NAME: _Z26importEquipmentPartSetInfoPKc16ENUM_MODULE_TYPE
// DEMANGLED: importEquipmentPartSetInfo(char const*, ENUM_MODULE_TYPE)
// C/06 owns STEquipmentScript, EquipmentParameterInfo, and ImportEquipmentScript;
// A keeps module_type as int until the real public enum lands.
int importEquipmentPartSetInfo(const char* filename, int module_type)
    __asm__("_Z26importEquipmentPartSetInfoPKc16ENUM_MODULE_TYPE");

// IDA_EA: 0x890793F
// IDA_NAME: _Z28importEquipmentEffectSetInfoPKc16ENUM_MODULE_TYPE
// DEMANGLED: importEquipmentEffectSetInfo(char const*, ENUM_MODULE_TYPE)
int importEquipmentEffectSetInfo(const char* filename, int module_type)
    __asm__("_Z28importEquipmentEffectSetInfoPKc16ENUM_MODULE_TYPE");

// C/06 owns the concrete equipment script and parameter object layouts. A only
// declares the old lifecycle symbols used by the public part-set value dtor.

using ::EquipmentParameterInfo;
using ::STEquipmentPartSetData;
using ::STEquipmentPartSetPiece;
using ::STEquipmentScript;


#endif  // OLDSRV_GLOBAL_EQUIPMENT_PART_SET_PUBLIC_ABI_H_
