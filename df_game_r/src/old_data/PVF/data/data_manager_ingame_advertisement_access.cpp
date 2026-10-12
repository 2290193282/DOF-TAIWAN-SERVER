#include "old_data/PVF/data/data_manager.h"

// CInGameAdvertisementManager::checkCondition @0x80FCD2E uses this exact
// CDataManager-owned InGameAdvertisementScript age-condition table after
// CUser::GetAge().  This is intentionally a B-side typed-layout accessor.
bool CDataManager::is_in_game_ad_age_allowed(unsigned int age) const {
    return in_game_advertisement_script_.age_condition_flags[age] != 0;
}
