#include "old_game/player/core/cuser.h"

#include "old_data/PVF/data/datamanager/server_parameter_script.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/statistics_abi.h"
#include "common/manager/statistics_runtime.h"
#include "old_data/PVF/data/data_manager.h"

// IDA_EA: 0x865798A
// IDA_NAME: _ZN5CUser15serviceOfShusiaEv
// DEMANGLED: CUser::serviceOfShusia(void)
int CUser::serviceOfShusia() {
    if (static_cast<unsigned char>(IsCurCharacGhost())) {
        return 19;
    }

    const int current_stamina = getCurCharacStamina();
    if (current_stamina > 99) {
        return 18;
    }

    ServiceRestrictManager* const restrictions =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (static_cast<unsigned char>(restrictions->isRestricted(
            this,
            static_cast<RestrictCategory::Enum>(1),
            22))) {
        return 209;
    }

    const int character_level = get_charac_level();
    const int price = G_CDataManager()
                          ->get_server_parameter_script()
                          ->getRecoverStaminaPrice(character_level,
                                                   current_stamina);
    if (getCurCharacMoney() < price) {
        return 22;
    }

    getCurCharacInvenW()->use_money(
        price, static_cast<eMoneySubReason>(10), true);
    GetInstanceValueStatistic()->AddValueStatistic(
        static_cast<VALUE_STATISTIC_FIELD>(16), this, price);
    setCurCharacStamina(100);
    Statistics::UseShusiaService(character_level);
    return 0;
}
