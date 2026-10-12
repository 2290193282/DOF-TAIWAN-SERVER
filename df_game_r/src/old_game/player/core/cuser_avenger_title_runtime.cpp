#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_save_give_avenger_title_flag.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "common/manager/system_time.h"

#include <cstring>
#include <string>

namespace {

const char kAvengerEventManagerName[] = "Arad_EventPeriodDataManager";
const char kAvengerTitleKey[] = "ready_to_avenger_title";
const char kAvengerMailKey[] = "ready_to_avenger_mail";
const int kAvengerEventCode = 142;
const uint32_t kAvengerTitleItemId = 2675631u;

ARAD::Arad_EventPeriodDataManager* GetAvengerEventPeriodManager() {
    ARAD::Arad_DataManager* const data_manager =
        ARAD::Singleton<ARAD::Arad_DataManager>::Get();
    return static_cast<ARAD::Arad_EventPeriodDataManager*>(
        data_manager->findGameScript(
            std::string(kAvengerEventManagerName)));
}

}  // namespace

// IDA_EA: 0x868B552
// IDA_NAME: _ZN5CUser25VerifyPresentAvengerTitleEv
// DEMANGLED: CUser::VerifyPresentAvengerTitle(void)
bool CUser::VerifyPresentAvengerTitle() {
    const ENUM_REPEAT_EVENT_CODE event_code =
        static_cast<ENUM_REPEAT_EVENT_CODE>(kAvengerEventCode);
    if (!GetAvengerEventPeriodManager()->checkCreateTime(
            event_code,
            static_cast<uint32_t>(getCurCharacCreateTime()),
            static_cast<uint32_t>(get_acc_id()))) {
        return false;
    }

    if (get_charac_level() <= 19 ||
        get_charac_job() != oldsrv::config::kCharacterJobPriest ||
        getCurCharFirstGrowType() != 0) {
        return false;
    }

    const uint32_t current_sec =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    if (getFlagPresentAvengerTitle() == 1 ||
        !GetAvengerEventPeriodManager()->isApplied(event_code,
                                                   current_sec)) {
        return false;
    }

    Inven_Item item;
    item.item_id_ = kAvengerTitleItemId;
    G_CDataManager()->get_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, item, 1);
    if (item.item_id_ == 0 ||
        item.item_id_ == -1) {
        return false;
    }

    setFlagPresentAvengerTitle(true);

    char mail_text[256];
    char mail_title[21];
    std::memset(mail_text, 0, sizeof(mail_text));
    std::memset(mail_title, 0, sizeof(mail_title));
    std::strncpy(
        mail_title,
        g_scriptStringManager_.findString(4, kAvengerTitleKey, 0),
        20u);
    std::strncpy(
        mail_text,
        g_scriptStringManager_.findString(4, kAvengerMailKey, 0),
        255u);

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        mail_title,
        item,
        0u,
        static_cast<uint32_t>(getCurCharacNo()),
        mail_text,
        static_cast<int>(std::strlen(mail_text)),
        0u,
        static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
        false,
        false);

    DB_SaveGiveAvengerTitleFlag::makeRequest(
        GetUID(),
        static_cast<uint32_t>(get_acc_id()),
        static_cast<uint32_t>(getCurCharacNo()));

    cMyTrace trace("bool CUser::VerifyPresentAvengerTitle()", 42731, 0);
    trace("avenger plan event item post m_id(%s),charac_no(%u), itemidx(%u)",
          NumberToString(static_cast<uint32_t>(get_acc_id()), 0),
          static_cast<uint32_t>(getCurCharacNo()),
          static_cast<uint32_t>(item.item_id_));
    return true;
}
