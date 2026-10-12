#include "old_game/player/core/cuser.h"

#include <vector>

#include "common/manager/global_data.h"
#include "common/manager/premium_public_abi.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/premium_runtime.h"
#include "common/manager/system_time.h"

extern "C" int g_alchemistInfo_0;

// IDA_EA: 0x8694A28
// IDA_NAME: _ZN8WongWork12CUserPremium23GetAddQuestItemDropRateEi
// DEMANGLED: WongWork::CUserPremium::GetAddQuestItemDropRate(int)
int WongWork::CUserPremium::GetAddQuestItemDropRate(int premium_type) {
    const unsigned int index =
        static_cast<unsigned int>(premium_type);
    return index < 106u
               ? g_SPremiumInfo[index].add_quest_item_drop_rate
               : 0;
}

// IDA_EA: 0x869733E
// IDA_NAME: _ZN5CUser34getAddQuestItemDropRateFromPremiumE17ENUM_PREMIUM_TYPE
// DEMANGLED: CUser::getAddQuestItemDropRateFromPremium(ENUM_PREMIUM_TYPE)
int CUser::getAddQuestItemDropRateFromPremium(
    ENUM_PREMIUM_TYPE premium_type) {
    return GetPremiumInfo()->GetAddQuestItemDropRate(
        static_cast<int>(premium_type));
}

// IDA_EA: 0x85D121E
// IDA_NAME: _ZN10expert_job10CAlchemist27IsExistContinuousEffectItemEP5CUseri
// DEMANGLED: expert_job::CAlchemist::IsExistContinuousEffectItem(
//             CUser *,int)
int expert_job::CAlchemist::IsExistContinuousEffectItem(
    CUser* user,
    int item_id) {
    if (!user->getCurCharacR()) {
        return 0;
    }

    const std::vector<ContinuousItemInfo>* const effect_items =
        user->GetCurCharacEffectItemListR();
    for (std::vector<ContinuousItemInfo>::const_iterator item =
             effect_items->begin();
         item != effect_items->end();
         ++item) {
        if (item->item_index == item_id) {
            return GlobalData::s_systemTime_.getCurSec() <
                   item->expire_tick;
        }
    }
    return 0;
}

// IDA_EA: 0x85D12EE
// IDA_NAME: _ZN10expert_job10CAlchemist20GetQuestItemDropRateEP5CUseri
// DEMANGLED: expert_job::CAlchemist::GetQuestItemDropRate(CUser *,int)
int expert_job::CAlchemist::GetQuestItemDropRate(CUser* user,
                                                  int base_rate) {
    if (IsExistContinuousEffectItem(user, 0x27AC52) ||
        IsExistContinuousEffectItem(user, 0x1F8E)) {
        return base_rate * (g_alchemistInfo_0 + 10000) / 10000;
    }
    return base_rate;
}
