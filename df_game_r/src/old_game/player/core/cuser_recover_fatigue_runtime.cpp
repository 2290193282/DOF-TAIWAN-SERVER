#include "old_game/player/core/cuser.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/reliable_person.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_update_school_point.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/premium_runtime.h"

namespace {

uint16_t PremiumFatigueFromRate(int rate, int maximum_fatigue) {
    return static_cast<uint16_t>(maximum_fatigue *
                                 static_cast<uint16_t>(rate) / 100);
}

}  // namespace

// IDA_EA: 0x8657ADA
// IDA_NAME: _ZN5CUser14RecoverFatigueEi
// DEMANGLED: CUser::RecoverFatigue(int)
char CUser::RecoverFatigue(int fatigue) {
    const _Charac_info* const current = getCurCharacR();
    if (!current) {
        return 1;
    }

    setCurCharacPremiumFatigue(static_cast<uint16_t>(fatigue));
    setCurCharacFatigue(fatigue);
    if (getCurCharacSchoolPoint()) {
        DB_UpdateSchoolPoint::makeRequest(
            static_cast<unsigned int>(get_acc_id()),
            getCurCharacSchoolPoint(),
            GetUID());
        resetCurCharacSchoolPoint();
    }

    _Charac_info* const character = const_cast<_Charac_info*>(current);
    CMaxFatigueFactorEvent* const fatigue_event =
        static_cast<CMaxFatigueFactorEvent*>(
            GlobalData::s_event_manager->GetRepeatEvent(2));
    if (fatigue_event) {
        character->fields_.maximum_fatigue_ =
            static_cast<uint16_t>(fatigue_event->IsEventing(0)
                ? DEFAULT_MAX_FATIGUE * fatigue_event->GetMaxFatigueFactor() / 100
                : DEFAULT_MAX_FATIGUE);
    }

    if (IsHavePremiumAdvantage()) {
        const int rate = GetPremiumInfo()->GetAdvantageFatigueRate();
        if (fatigue_event && fatigue_event->IsEventing(0)) {
            const int extra_fatigue =
                DEFAULT_MAX_FATIGUE * fatigue_event->GetMaxFatigueFactor() / 100 -
                DEFAULT_MAX_FATIGUE;
            const int base_fatigue =
                PremiumFatigueFromRate(rate, DEFAULT_MAX_FATIGUE);
            setCurCharacMaxPremiumFatigue(
                static_cast<uint16_t>(base_fatigue +
                                      static_cast<uint16_t>(rate) *
                                          extra_fatigue / 100));
        } else {
            setCurCharacMaxPremiumFatigue(
                PremiumFatigueFromRate(rate, getCurCharacMaxFatigue()));
        }
    }

    CEventBase* const weekend_event =
        GlobalData::s_event_manager->GetRepeatEvent(87);
    if (weekend_event->IsEventing(0)) {
        character->fields_.maximum_fatigue_ = static_cast<uint16_t>(
            character->fields_.maximum_fatigue_ +
            G_CDataManager()->GetWeekendBonusFatigue());
        const int rate = GetPremiumInfo()->GetAdvantageFatigueRate();
        if (static_cast<uint16_t>(rate) != 0u && GetPCRoomNo()) {
            setCurCharacMaxPremiumFatigue(static_cast<uint16_t>(
                PremiumFatigueFromRate(rate, DEFAULT_MAX_FATIGUE) +
                G_CDataManager()->GetWeekendBonusFatigue()));
        } else {
            setCurCharacMaxPremiumFatigue(
                PremiumFatigueFromRate(rate, DEFAULT_MAX_FATIGUE));
        }
    } else if (IsHavePremiumAdvantage()) {
        setCurCharacMaxPremiumFatigue(PremiumFatigueFromRate(
            GetPremiumInfo()->GetAdvantageFatigueRate(),
            getCurCharacMaxFatigue()));
    }

    return static_cast<char>(resetCurCharacUsedFatigue());
}

// IDA_EA: 0x867CBE4
// IDA_NAME: _ZN5CUser18ProcPremiumFatigueEv
// DEMANGLED: CUser::ProcPremiumFatigue(void)
char CUser::ProcPremiumFatigue() {
    char result = getCurCharacR() == 0;
    if (!result) {
        result = static_cast<char>(IsHavePremiumAdvantage());
        if (result) {
            CEventBase* const weekend_event =
                GlobalData::s_event_manager->GetRepeatEvent(87);
            const uint16_t fatigue_rate = static_cast<uint16_t>(
                GetPremiumInfoW()->GetAdvantageFatigueRate());
            if (weekend_event->IsEventing(0) && fatigue_rate != 0u &&
                GetPCRoomNo()) {
                return static_cast<char>(setCurCharacMaxPremiumFatigue(
                    static_cast<uint16_t>(
                        DEFAULT_MAX_FATIGUE * fatigue_rate / 100 +
                        G_CDataManager()->GetWeekendBonusFatigue())));
            }
            return static_cast<char>(setCurCharacMaxPremiumFatigue(
                static_cast<uint16_t>(DEFAULT_MAX_FATIGUE * fatigue_rate /
                                      100)));
        }
    }
    return result;
}
