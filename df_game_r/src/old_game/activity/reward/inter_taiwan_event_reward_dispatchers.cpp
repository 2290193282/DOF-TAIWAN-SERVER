#include "old_game/activity/reward/inter_taiwan_event_reward_dispatchers.h"

#include <cstring>

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_DATA_EA: 0x8B800A0
// IDA_NAME: _ZN14CEventStayTime12kRewardItem_E
const int CEventStayTime::kRewardItem_ __attribute__((used)) = 690000013;

// IDA_DATA_EA: 0x8B7CD20
// IDA_NAME: _ZN14EventGiveMeBox12kRewardItem_E
const int EventGiveMeBox::kRewardItem_ __attribute__((used)) = 690000268;

// IDA_EA: 0x8161FA8
// IDA_NAME: _ZN23InterCheckStayTimeEventC2Ev
// DEMANGLED: InterCheckStayTimeEvent::InterCheckStayTimeEvent(void)
InterCheckStayTimeEvent::InterCheckStayTimeEvent()
    : TaiwanInterfaceInterDispatcher(TaiwanInternalPack::CHECK_STAY_TIME_EVENT) {
}

// IDA_EA: 0x8161FCC (D2), 0x8161FFC (D0)
// IDA_NAME: _ZN23InterCheckStayTimeEventD2Ev / D0Ev
InterCheckStayTimeEvent::~InterCheckStayTimeEvent() {
}

// IDA_EA: 0x8160A42
// IDA_NAME: _ZN23InterCheckStayTimeEvent12dispatch_sigEP5CUserPci
// DEMANGLED: InterCheckStayTimeEvent::dispatch_sig(CUser *,char *,int)
int InterCheckStayTimeEvent::dispatch_sig(CUser* user,
                                          char* payload,
                                          int payload_size) {
    (void)payload_size;
    if (!payload) {
        cMyTrace trace(
            "virtual int InterCheckStayTimeEvent::dispatch_sig(CUser*, char*, int)",
            201, 5);
        trace("[Taiwan, StayEvent] SigStayTimeEvent is null.");
        return 202;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(159);
    if (repeat_event->IsEventing(0) != 1) {
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(CEventStayTime::kRewardItem_);
    if (!item) {
        cMyTrace trace(
            "virtual int InterCheckStayTimeEvent::dispatch_sig(CUser*, char*, int)",
            211, 5);
        trace("[Taiwan, StayEvent] Invalid item index. (%u)",
              CEventStayTime::kRewardItem_);
        return 0;
    }

    Inven_Item reward;
    // The original keeps this otherwise-unused CItem scalar call before the
    // virtual make_item at vtable slot 2.
    (void)item->get_index();
    item->vtable_->make_item(item, reward);
    reward.set_add_info(1);

    const char* const title = g_scriptStringManager_.findString(
        4, "tw_ccb_stress_title_01", 0);
    const char* const body = g_scriptStringManager_.findString(
        4, "tw_ccb_stress_mail_01", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title, reward, 0, static_cast<unsigned int>(user->getCurCharacNo()),
        body, static_cast<int>(std::strlen(body)), 14,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false, false);
    return 0;
}

// IDA_EA: 0x8162254
// IDA_NAME: _ZN29InterInsertGiveBoxEventRewardC2Ev
// DEMANGLED: InterInsertGiveBoxEventReward::InterInsertGiveBoxEventReward(void)
InterInsertGiveBoxEventReward::InterInsertGiveBoxEventReward()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::INSERT_GIVE_BOX_EVENT_REWARD) {
}

// IDA_EA: 0x8162278 (D2), 0x81622A8 (D0)
// IDA_NAME: _ZN29InterInsertGiveBoxEventRewardD2Ev / D0Ev
InterInsertGiveBoxEventReward::~InterInsertGiveBoxEventReward() {
}

// IDA_EA: 0x8161A8A
// IDA_NAME: _ZN29InterInsertGiveBoxEventReward12dispatch_sigEP5CUserPci
// DEMANGLED: InterInsertGiveBoxEventReward::dispatch_sig(CUser *,char *,int)
int InterInsertGiveBoxEventReward::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    (void)payload_size;
    if (!payload) {
        cMyTrace trace(
            "virtual int InterInsertGiveBoxEventReward::dispatch_sig(CUser*, char*, int)",
            476, 5);
        trace("[Taiwan, GiveMeBoxEvent] msg is null.");
        return 477;
    }

    CEventBase* const repeat_event =
        GlobalData::s_event_manager->GetRepeatEvent(165);
    if (repeat_event->IsEventing(0) != 1) {
        return 0;
    }

    CItem* const item = G_CDataManager()->find_item(EventGiveMeBox::kRewardItem_);
    if (!item) {
        cMyTrace trace(
            "virtual int InterInsertGiveBoxEventReward::dispatch_sig(CUser*, char*, int)",
            486, 5);
        trace("[Taiwan, GiveMeBoxEvent] Invalid item index. (%u)",
              EventGiveMeBox::kRewardItem_);
        return 0;
    }

    Inven_Item reward;
    // The original keeps this otherwise-unused CItem scalar call before the
    // virtual make_item at vtable slot 2.
    (void)item->get_index();
    item->vtable_->make_item(item, reward);
    reward.set_add_info(1);

    const char* const title = g_scriptStringManager_.findString(
        4, "tw_givemebox2_title", 0);
    const char* const body = g_scriptStringManager_.findString(
        4, "tw_givemebox2_mail", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title, reward, 0, static_cast<unsigned int>(user->getCurCharacNo()),
        body, static_cast<int>(std::strlen(body)), 14,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()), false, false);
    return 0;
}
