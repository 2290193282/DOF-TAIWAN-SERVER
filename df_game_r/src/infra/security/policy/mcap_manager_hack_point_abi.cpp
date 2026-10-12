#include "old_game/player/core/cuser.h"


// IDA_NAME: _ZN8WongWork12CMCAPManager17pointPerHackType_E
int WongWork::CMCAPManager::pointPerHackType_[3900];

// IDA_EA: 0x8695025
// IDA_NAME: _ZN8WongWork12CMCAPManager19getPointPerHackTypeENS_13ENUM_HACKTYPEE
// DEMANGLED: WongWork::CMCAPManager::getPointPerHackType(WongWork::ENUM_HACKTYPE)
int WongWork::CMCAPManager::getPointPerHackType(ENUM_HACKTYPE hack_type) {
    return pointPerHackType_[static_cast<int>(hack_type) - 101];
}

// IDA_EA: 0x84EB53E
// IDA_NAME: _ZN8WongWork12CMCAPManager8incPointEi
// DEMANGLED: WongWork::CMCAPManager::incPoint(int)
WongWork::CMCAPManager* WongWork::CMCAPManager::incPoint(int point) {
    point_ += point * increment_rate_ / 1000;
    save_clean_pad_ = 1u;
    return this;
}
