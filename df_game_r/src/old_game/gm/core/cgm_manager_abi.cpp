#include "old_game/gm/core/cgm_manager_abi.h"

namespace {

// The original member creates readGuard<rwLock> over CGM_Manager+0x1c.
// This local guard preserves that lifetime around the canonical map lookup
// without adding a replacement exported ABI type.
class CGMReadGuard {
public:
    explicit CGMReadGuard(pthread_rwlock_t& lock) : lock_(lock) {
        pthread_rwlock_rdlock(&lock_);
    }

    ~CGMReadGuard() {
        pthread_rwlock_unlock(&lock_);
    }

private:
    pthread_rwlock_t& lock_;

    CGMReadGuard(const CGMReadGuard&);
    CGMReadGuard& operator=(const CGMReadGuard&);
};

}  // namespace

// IDA_EA: 0x829917C
// IDA_NAME: _ZN11CGM_Manager10TurnGmModeEj
// DEMANGLED: CGM_Manager::TurnGmMode(unsigned int)
int CGM_Manager::TurnGmMode(unsigned int account_id) {
    pthread_rwlock_rdlock(&rwlock);
    std::map<unsigned int, CGMRecord>::iterator it =
        gm_records.find(account_id);
    int result = 0;
    if (it != gm_records.end()) {
        it->second.gm_mode = it->second.gm_mode ? 0 : 1;
        result = it->second.gm_mode;
    }
    pthread_rwlock_unlock(&rwlock);
    return result;
}

// IDA_EA: 0x8299276
// IDA_NAME: _ZN11CGM_Manager16GetCurrentGmModeEj
// DEMANGLED: CGM_Manager::GetCurrentGmMode(unsigned int)
int CGM_Manager::GetCurrentGmMode(unsigned int account_id) {
    pthread_rwlock_rdlock(&rwlock);
    std::map<unsigned int, CGMRecord>::iterator it =
        gm_records.find(account_id);
    const int result =
        it == gm_records.end() ? 0 : it->second.gm_mode;
    pthread_rwlock_unlock(&rwlock);
    return result;
}

// IDA_EA: 0x82993D8
// IDA_NAME: _ZN11CGM_Manager10GetGmLevelEj
// DEMANGLED: CGM_Manager::GetGmLevel(unsigned int)
int CGM_Manager::GetGmLevel(unsigned int account_id) {
    CGMReadGuard guard(rwlock);
    std::map<unsigned int, CGMRecord>::const_iterator it =
        gm_records.find(account_id);
    return it == gm_records.end() ? 0 : it->second.gm_level;
}

// IDA_EA: 0x829948C
// IDA_NAME: _ZN11CGM_Manager4IsGmEj
// DEMANGLED: CGM_Manager::IsGm(unsigned int)
bool CGM_Manager::IsGm(unsigned int account_id) {
    CGMReadGuard guard(rwlock);
    return gm_records.find(account_id) != gm_records.end();
}
