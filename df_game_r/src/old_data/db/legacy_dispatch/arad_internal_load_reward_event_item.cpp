#include "old_game/appearance/avatar/arad_internal_return_dispatchers.h"

#include "old_data/PVF/arad/datamanager/arad_event_period_data_manager.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_data/db/event/reward_event_request.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

#include <string>

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x819A46E
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemC2Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadRewardEventItem::Arad_INTER_kLoadRewardEventItem(void)
Arad_INTER_kLoadRewardEventItem::Arad_INTER_kLoadRewardEventItem()
    : Arad_IInterDispatcher(kInternalPacketLoadRewardEventItem) {}

// IDA_EA: 0x819A492
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemD2Ev
// IDA_EA: 0x819A4C2
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemD0Ev
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadRewardEventItem::~Arad_INTER_kLoadRewardEventItem()
Arad_INTER_kLoadRewardEventItem::~Arad_INTER_kLoadRewardEventItem() {}

// IDA_EA: 0x8198FD0
// IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItem12dispatch_sigEP5CUserPci
// DEMANGLED: ARAD::DISPATCHER::Arad_INTER_kLoadRewardEventItem::dispatch_sig(CUser *,char *,int)
int Arad_INTER_kLoadRewardEventItem::dispatch_sig(CUser* user,
                                                   char* payload,
                                                   int length) {
    // The original checks only the typed inner payload pointer.  It does not
    // compare the queued length against SigLoadRewardEventItem's fixed size.
    (void)length;

    if (user == 0 || user->getCurCharacR() == 0) {
        return 1235;
    }

    arad::SigLoadRewardEventItem* const reward_items =
        reinterpret_cast<arad::SigLoadRewardEventItem*>(payload);
    if (reward_items == 0) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_InterDispatcher.cpp",
            "virtual int ARAD::DISPATCHER::Arad_INTER_kLoadRewardEventItem::dispatch_sig(CUser*, char*, int)",
            0x4D8,
            "SigLoadRewardEventItem is null.");
        return 1241;
    }

    // The three independent findGameScript calls and their order are part of
    // the old transaction: loaded DB rows are recorded before the Lethe
    // reward test, and the Momoji reward is gated by repeat event 155.
    Arad_EventPeriodDataManager* const load_manager =
        static_cast<Arad_EventPeriodDataManager*>(
            Singleton<Arad_DataManager>::Get()->findGameScript(
                std::string("Arad_EventPeriodDataManager")));
    load_manager->addUserRewardData(user, *reward_items);

    const uint32_t lethe_current_sec =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    Arad_EventPeriodDataManager* const lethe_manager =
        static_cast<Arad_EventPeriodDataManager*>(
            Singleton<Arad_DataManager>::Get()->findGameScript(
                std::string("Arad_EventPeriodDataManager")));
    lethe_manager->eventReward<FnRewardLethe>(
        static_cast<ENUM_REPEAT_EVENT_CODE>(143), lethe_current_sec, user);

    // The original dereferences the returned event directly; a missing
    // repeat-event registration is not converted into a fabricated success.
    if (GlobalData::s_event_manager->GetRepeatEvent(155)->IsEventing(0)) {
        const uint32_t momoji_current_sec =
            static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
        Arad_EventPeriodDataManager* const momoji_manager =
            static_cast<Arad_EventPeriodDataManager*>(
                Singleton<Arad_DataManager>::Get()->findGameScript(
                    std::string("Arad_EventPeriodDataManager")));
        momoji_manager->eventReward<FnRewardMomoji>(
            static_cast<ENUM_REPEAT_EVENT_CODE>(155), momoji_current_sec,
            user);
    }

    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
