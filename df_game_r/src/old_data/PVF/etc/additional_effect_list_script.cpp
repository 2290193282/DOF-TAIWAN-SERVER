#include "old_data/PVF/etc/additional_effect_list_script.h"

#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>

// IDA data: g_additionalEffectMap_ @0x94F7160.
std::map<int, AdditionalEffectData> g_additionalEffectMap_;

// IDA_EA: 0x891B05E
// IDA_NAME: _ZN20AdditionalEffectDataC2Ev
// DEMANGLED: AdditionalEffectData::AdditionalEffectData(void)
AdditionalEffectData::AdditionalEffectData() : animation_names() {
    index = -1;
    animation_names.clear();
    effect_type = 2;
    fix_position_x = 0;
    custom_ani_delay = -1;
    custom_ani_delay_in_town = -1;
    default_term_in_town = 0;
    default_term = 0;
    dash_term = 0;
    draw_over = false;
}

// IDA_EA: 0x891C73E
// IDA_NAME: _ZN20AdditionalEffectDataD2Ev
// DEMANGLED: AdditionalEffectData::~AdditionalEffectData()
AdditionalEffectData::~AdditionalEffectData() {}

// IDA_EA: 0x891C754
// IDA_NAME: _ZN20AdditionalEffectDataaSERKS_
// DEMANGLED: AdditionalEffectData::operator=(AdditionalEffectData const&)
AdditionalEffectData& AdditionalEffectData::operator=(
    const AdditionalEffectData& other) {
    index = other.index;
    animation_names = other.animation_names;
    effect_type = other.effect_type;
    fix_position_x = other.fix_position_x;
    custom_ani_delay = other.custom_ani_delay;
    custom_ani_delay_in_town = other.custom_ani_delay_in_town;
    default_term_in_town = other.default_term_in_town;
    default_term = other.default_term;
    dash_term = other.dash_term;
    draw_over = other.draw_over;
    return *this;
}

// IDA_EA: 0x8929074
// IDA_NAME: _ZN20AdditionalEffectDataC2ERKS_
// DEMANGLED: AdditionalEffectData::AdditionalEffectData(AdditionalEffectData const&)
AdditionalEffectData::AdditionalEffectData(const AdditionalEffectData& other)
    : index(other.index),
      animation_names(other.animation_names),
      effect_type(other.effect_type),
      fix_position_x(other.fix_position_x),
      custom_ani_delay(other.custom_ani_delay),
      custom_ani_delay_in_town(other.custom_ani_delay_in_town),
      default_term_in_town(other.default_term_in_town),
      default_term(other.default_term),
      dash_term(other.dash_term),
      draw_over(other.draw_over) {}

// IDA_EA: 0x8907E3A
// IDA_NAME: _Z32importAdditionalEffectListScriptPKc
// DEMANGLED: importAdditionalEffectListScript(char const*)
int importAdditionalEffectListScript(const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importAdditionalEffectListScript",
                              0x172c,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    std::string tag;
    g_additionalEffectMap_.clear();
    while (ScanType(tag, true) == 1) {
        if (tag != "[effect]") {
            continue;
        }

        AdditionalEffectData effect;
        while (ScanType(tag, true) == 1) {
            if (tag == "[index]") {
                effect.index = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[type]") {
                effect.effect_type = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[draw over]") {
                effect.draw_over =
                    ScanInt(static_cast<bool*>(0)) != 0;
            } else if (tag == "[fix position x]") {
                effect.fix_position_x = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[custom ani delay]") {
                effect.custom_ani_delay = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[custom ani delay in town]") {
                effect.custom_ani_delay_in_town =
                    ScanInt(static_cast<bool*>(0));
            } else if (tag == "[default term in town]") {
                effect.default_term_in_town =
                    ScanInt(static_cast<bool*>(0));
            } else if (tag == "[default term]") {
                effect.default_term = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[dash term]") {
                effect.dash_term = ScanInt(static_cast<bool*>(0));
            } else if (tag == "[animation]") {
                std::string animation;
                while (ScanStr(&animation) == 1) {
                    effect.animation_names.push_back(animation);
                }
            } else if (tag == "[/effect]") {
                g_additionalEffectMap_[effect.index] = effect;
                break;
            }
        }
    }
    return 1;
}
