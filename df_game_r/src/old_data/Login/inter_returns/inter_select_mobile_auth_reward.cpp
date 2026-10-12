#include "old_data/Login/inter_returns/inter_select_mobile_auth_reward.h"

#include <cstring>
#include <utility>
#include <vector>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/reward/cerashop_reward_point_event.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x81620FE
// IDA_NAME: _ZN27InterSelectMobileAuthRewardC2Ev
// DEMANGLED: InterSelectMobileAuthReward::InterSelectMobileAuthReward(void)
InterSelectMobileAuthReward::InterSelectMobileAuthReward()
    : TaiwanInterfaceInterDispatcher(
          TaiwanInternalPack::SELECT_MOBILE_AUTH_REWARD) {
}

// IDA_EA: 0x8162122
// IDA_NAME: _ZN27InterSelectMobileAuthRewardD2Ev
// DEMANGLED: InterSelectMobileAuthReward::~InterSelectMobileAuthReward()
// IDA_EA: 0x8162152
// IDA_NAME: _ZN27InterSelectMobileAuthRewardD0Ev
// DEMANGLED: InterSelectMobileAuthReward::~InterSelectMobileAuthReward()
// The compiler emits the deleting D0 variant for this virtual destructor.
InterSelectMobileAuthReward::~InterSelectMobileAuthReward() {
}

// IDA_EA: 0x816132A
// IDA_NAME: _ZN27InterSelectMobileAuthReward12dispatch_sigEP5CUserPci
// DEMANGLED: InterSelectMobileAuthReward::dispatch_sig(CUser *,char *,int)
int InterSelectMobileAuthReward::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)payload_size;
    if (!payload) {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "virtual int InterSelectMobileAuthReward::dispatch_sig(CUser*, char*, int)",
            353,
            "[Taiwan, MobileAuth] interMsg is null.");
        return 354;
    }

    const Msg* const message = reinterpret_cast<const Msg*>(payload);
    if (message->rewarded != 1) {
        std::vector<std::pair<int, int> > rewards;
        rewards.push_back(std::make_pair(2660410, 1));

        char title[30];
        char text[256];
        std::strncpy(title,
                     g_scriptStringManager_.findString(
                         4, "tw_mobile_authentication_title_01", 0),
                     29);
        std::strncpy(text,
                     g_scriptStringManager_.findString(
                         4, "tw_mobile_authentication_mail_01", 0),
                     255);

        Inven_Item mail_items[10];
        WongWork::CMailBoxHelper::MakeSystemMultiMailPostal(
            rewards, mail_items, 10);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMultiMail(
            title,
            mail_items,
            static_cast<unsigned int>(rewards.size()),
            0,
            static_cast<unsigned int>(user->getCurCharacNo()),
            text,
            static_cast<int>(std::strlen(text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
            false);

        CEventBase* const repeat_event =
            GlobalData::s_event_manager->GetRepeatEvent(161);
        if (repeat_event && repeat_event->IsEventing(0)) {
            static_cast<CEventCeraShopRewardPoint*>(repeat_event)
                ->SendReward(user);
        }
    }

    user->mobile_auth_reward_state_ = message->rewarded;
    return 0;
}

// IDA_EA: 0x81615AE
// IDA_NAME: _ZN27InterSelectMobileAuthReward11makeRequestEijb
// DEMANGLED: InterSelectMobileAuthReward::makeRequest(int,unsigned int,bool)
void InterSelectMobileAuthReward::makeRequest(int user_idx,
                                              unsigned int account_id,
                                              bool rewarded) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp", 390);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard,
                            TaiwanInternalPack::SELECT_MOBILE_AUTH_REWARD,
                            user_idx);
    Msg* const message = guard.GetInBuffer<Msg>();
    if (message) {
        message->account_id = account_id;
        message->rewarded = rewarded ? 1u : 0u;
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    } else {
        LogManager::logFormat(
            1,
            "localtaiwan/Dispatcher/Taiwan_InterDispatcher.cpp",
            "static void InterSelectMobileAuthReward::makeRequest(int, memberIdentificationNumber_t, bool)",
            395,
            "[Taiwan, MobileAuth] m_id:%d rewarded:%d",
            account_id,
            rewarded ? 1 : 0);
    }
}
