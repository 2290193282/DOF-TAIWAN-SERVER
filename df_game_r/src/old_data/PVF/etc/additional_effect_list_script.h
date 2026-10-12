#ifndef OLDSRV_OLD_DATA_PVF_ETC_ADDITIONAL_EFFECT_LIST_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_ADDITIONAL_EFFECT_LIST_SCRIPT_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

// IDA unit: 99_misc_global/AdditionalEffectData.  The member order is fixed
// by the C2/D2/copy paths at 0x891B05E, 0x891C73E and 0x8929074.
struct AdditionalEffectData {
    int32_t index;
    std::vector<std::string> animation_names;
    int32_t effect_type;
    int32_t fix_position_x;
    int32_t custom_ani_delay;
    int32_t custom_ani_delay_in_town;
    int32_t default_term_in_town;
    int32_t default_term;
    int32_t dash_term;
    bool draw_over;

    AdditionalEffectData();
    AdditionalEffectData(const AdditionalEffectData& other);
    ~AdditionalEffectData();
    AdditionalEffectData& operator=(const AdditionalEffectData& other);
};

static_assert(sizeof(AdditionalEffectData) == 0x30u,
              "AdditionalEffectData IDA ABI size must stay 0x30");

// IDA data: g_additionalEffectMap_ @0x94F7160.
extern std::map<int, AdditionalEffectData> g_additionalEffectMap_;

// IDA_EA: 0x8907E3A
// IDA_NAME: _Z32importAdditionalEffectListScriptPKc
// DEMANGLED: importAdditionalEffectListScript(char const*)
int importAdditionalEffectListScript(const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_ETC_ADDITIONAL_EFFECT_LIST_SCRIPT_H_
