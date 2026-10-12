#include "old_game/activity/reward/cerashop_reward_point_event.h"

#include <cstring>

#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x816459A
// IDA_NAME: _ZN25CEventCeraShopRewardPointC2Ev
// DEMANGLED: CEventCeraShopRewardPoint::CEventCeraShopRewardPoint(void)
CEventCeraShopRewardPoint::CEventCeraShopRewardPoint()
    : CEventBase(), cera_point_count_(0), timing_(0) {
}

// IDA_EA: 0x81645CA (D2), 0x81645FA (D0)
// IDA_NAME: _ZN25CEventCeraShopRewardPointD2Ev / D0Ev
// The compiler emits the deleting D0 variant for this virtual destructor.
CEventCeraShopRewardPoint::~CEventCeraShopRewardPoint() {
}

// IDA_EA: 0x8164618
// IDA_NAME: _ZN25CEventCeraShopRewardPoint10StartEventEv
// DEMANGLED: CEventCeraShopRewardPoint::StartEvent(void)
void CEventCeraShopRewardPoint::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CEventCeraShopRewardPoint::StartEvent()",
                   314,
                   0);
    trace("[Taiwan, CeraShopRewardPoint] Start event. Timing:%d, CeraPoint:%d ",
          timing_,
          cera_point_count_);
}

// IDA_EA: 0x8164684
// IDA_NAME: _ZN25CEventCeraShopRewardPoint10StartEventE10Word_Param
// DEMANGLED: CEventCeraShopRewardPoint::StartEvent(Word_Param)
void CEventCeraShopRewardPoint::StartEvent(Word_Param param) {
    const uint32_t raw = param.value;
    timing_ = static_cast<uint16_t>(raw);
    cera_point_count_ = static_cast<uint16_t>(raw >> 16);
    StartEvent();
}

// IDA_EA: 0x81646B8
// IDA_NAME: _ZN25CEventCeraShopRewardPoint8EndEventEv
// DEMANGLED: CEventCeraShopRewardPoint::EndEvent(void)
void CEventCeraShopRewardPoint::EndEvent() {
    cMyTrace trace("virtual void CEventCeraShopRewardPoint::EndEvent()",
                   327,
                   0);
    trace("[Taiwan, CeraShopRewardPoint] End event.");
    timing_ = 0;
    cera_point_count_ = 0;
    SetEventFlag(false);
}

// IDA_EA: 0x816471E
// IDA_NAME: _ZN25CEventCeraShopRewardPoint10SendRewardEP5CUser
// DEMANGLED: CEventCeraShopRewardPoint::SendReward(CUser *)
void CEventCeraShopRewardPoint::SendReward(CUser* user) {
    if (!user) {
        return;
    }

    char title[30];
    char text[256];
    std::strncpy(title,
                 g_scriptStringManager_.findString(4, "tw_shop_open_title", 0),
                 29);
    std::strncpy(text,
                 g_scriptStringManager_.findString(
                     4, "tw_shop_open_content", 0),
                 255);

    Inven_Item reward;
    reward.item_id_ = 2681917;
    reward.set_add_info(static_cast<int>(cera_point_count_));
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        reward,
        cera_point_count_,
        static_cast<unsigned int>(user->get_charac_no(-1)),
        text,
        static_cast<int>(std::strlen(text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}
