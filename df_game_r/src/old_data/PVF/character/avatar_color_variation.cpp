#include "old_data/PVF/character/avatar_color_variation.h"

#include "old_data/PVF/common/rdar_script.h"

#include <map>
#include <string>
#include <utility>


namespace AvatarVariation {

// IDA_EA: 0x88D444F
// IDA_NAME: _ZN15AvatarVariation26importAvatarColorVariationEPNS_15AvatarColorInfoE
// DEMANGLED: AvatarVariation::importAvatarColorVariation(AvatarVariation::AvatarColorInfo*)
int importAvatarColorVariation(AvatarColorInfo* info) {
    if (!loadRDARScriptFile("", "Etc/AvatarColorVariation.etc")) {
        return 0;
    }

    info->colors.clear();
    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[avatar color map]") {
            while (1) {
                colorRGB color;
                const int color_key = ScanInt(&success);
                if (!success) {
                    break;
                }
                color.red = static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                color.green = static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                color.blue = static_cast<unsigned char>(ScanInt(&success));
                if (!success) {
                    break;
                }

                std::string name;
                success = ScanStr(&name);
                if (!success) {
                    break;
                }
                info->colors.insert(std::make_pair(color_key, color));
                info->names.insert(std::make_pair(color_key, name));
            }
        }
    }
    return 1;
}

// IDA_EA: 0x88D46D3
// IDA_NAME: _ZN15AvatarVariation22getAvatarColorInfoInstEv
// DEMANGLED: AvatarVariation::getAvatarColorInfoInst(void)
AvatarColorInfo* getAvatarColorInfoInst() {
    static AvatarColorInfo s_avatar_color_info;
    if (importAvatarColorVariation(&s_avatar_color_info)) {
        return &s_avatar_color_info;
    }
    return 0;
}

// IDA_EA: 0x88D4782
// IDA_NAME: _ZN15AvatarVariation19getAvatarColorValueEi
// DEMANGLED: AvatarVariation::getAvatarColorValue(int)
colorRGB getAvatarColorValue(int color_key) {
    if (color_key < 0 || color_key > 0x1ff) {
        color_key = 0;
    }

    AvatarColorInfo* info = getAvatarColorInfoInst();
    if (info == 0) {
        return colorRGB();
    }

    std::map<int, colorRGB>::const_iterator found =
        info->colors.find(color_key);
    if (found != info->colors.end()) {
        return found->second;
    }
    return colorRGB();
}

// IDA_EA: 0x88D4856
// IDA_NAME: _ZN15AvatarVariation16isEnAbleColorKeyERNS_8colorRGBE
// DEMANGLED: AvatarVariation::isEnAbleColorKey(AvatarVariation::colorRGB&)
bool isEnAbleColorKey(colorRGB& color) {
    return !(color == getAvatarColorValue(0));
}

// IDA_EA: 0x88D4896
// IDA_NAME: _ZN15AvatarVariation18getAvatarColorNameEi
// DEMANGLED: AvatarVariation::getAvatarColorName(int)
const char* getAvatarColorName(int color_key) {
    if (color_key < 0 || color_key > 0x1ff) {
        return 0;
    }

    std::map<int, std::string>* names =
        &getAvatarColorInfoInst()->names;
    if (names == 0) {
        return 0;
    }

    std::map<int, std::string>::const_iterator found =
        names->find(color_key);
    if (found == names->end()) {
        return 0;
    }
    return found->second.c_str();
}

}  // namespace AvatarVariation
