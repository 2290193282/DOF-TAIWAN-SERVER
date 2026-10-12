#pragma once

#include "common/manager/cdata_manager_tail_public_abi.h"


namespace AvatarVariation {

// IDA_EA: 0x88D444F
// IDA_NAME: _ZN15AvatarVariation26importAvatarColorVariationEPNS_15AvatarColorInfoE
// DEMANGLED: AvatarVariation::importAvatarColorVariation(AvatarVariation::AvatarColorInfo*)
int importAvatarColorVariation(AvatarColorInfo* info);

// IDA_EA: 0x88D46D3
// IDA_NAME: _ZN15AvatarVariation22getAvatarColorInfoInstEv
// DEMANGLED: AvatarVariation::getAvatarColorInfoInst(void)
AvatarColorInfo* getAvatarColorInfoInst();

// IDA_EA: 0x88D4782
// IDA_NAME: _ZN15AvatarVariation19getAvatarColorValueEi
// DEMANGLED: AvatarVariation::getAvatarColorValue(int)
colorRGB getAvatarColorValue(int color_key);

// IDA_EA: 0x88D4856
// IDA_NAME: _ZN15AvatarVariation16isEnAbleColorKeyERNS_8colorRGBE
// DEMANGLED: AvatarVariation::isEnAbleColorKey(AvatarVariation::colorRGB&)
bool isEnAbleColorKey(colorRGB& color);

// IDA_EA: 0x88D4896
// IDA_NAME: _ZN15AvatarVariation18getAvatarColorNameEi
// DEMANGLED: AvatarVariation::getAvatarColorName(int)
const char* getAvatarColorName(int color_key);

}  // namespace AvatarVariation
