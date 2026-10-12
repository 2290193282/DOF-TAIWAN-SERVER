#include <algorithm>
#include <stdint.h>

#include "old_game/player/core/cuser.h"

// IDA_EA: 0x822EEA4
// IDA_NAME: _ZN8WongWork12CMCAPManager8getPointEv
// DEMANGLED: WongWork::CMCAPManager::getPoint(void)
unsigned int WongWork::CMCAPManager::getPoint() {
    return std::max(0u, std::min(point_, 10000u));
}

// IDA_EA: 0x82870DE
// IDA_NAME: _ZN8WongWork12CMCAPManager8setPointEt
// DEMANGLED: WongWork::CMCAPManager::setPoint(unsigned short)
WongWork::CMCAPManager* WongWork::CMCAPManager::setPoint(uint16_t point) {
    point_ = std::max<uint16_t>(0u, std::min<uint16_t>(point, 10000u));
    save_clean_pad_ = 1u;
    return this;
}

// IDA_EA: 0x869504C
// IDA_NAME: _ZNK8WongWork12CMCAPManager15getExposedCountEv
// DEMANGLED: WongWork::CMCAPManager::getExposedCount(void)const
int WongWork::CMCAPManager::getExposedCount() const {
    return exposed_count_;
}

// IDA_EA: 0x822EEDE
// IDA_NAME: _ZN8WongWork12CMCAPManager19IsSaveCleanPadPointEv
// DEMANGLED: WongWork::CMCAPManager::IsSaveCleanPadPoint(void)
int WongWork::CMCAPManager::IsSaveCleanPadPoint() {
    return save_clean_pad_;
}

// IDA_EA: 0x822EEEA
// IDA_NAME: _ZN8WongWork12CMCAPManager20disableCleanPadPointEv
// DEMANGLED: WongWork::CMCAPManager::disableCleanPadPoint(void)
WongWork::CMCAPManager* WongWork::CMCAPManager::disableCleanPadPoint() {
    save_clean_pad_ = 0;
    return this;
}

// IDA_EA: 0x8694FE0
// IDA_NAME: _ZN8WongWork12CMCAPManager12resetIncRateEv
// DEMANGLED: WongWork::CMCAPManager::resetIncRate(void)
WongWork::CMCAPManager* WongWork::CMCAPManager::resetIncRate() {
    increment_rate_ = 1000;
    return this;
}

// IDA_EA: 0x8694FF0
// IDA_NAME: _ZN8WongWork12CMCAPManager10decIncRateEi
// DEMANGLED: WongWork::CMCAPManager::decIncRate(int)
WongWork::CMCAPManager* WongWork::CMCAPManager::decIncRate(int decrement) {
    increment_rate_ = std::max(0, increment_rate_ - decrement);
    return this;
}
