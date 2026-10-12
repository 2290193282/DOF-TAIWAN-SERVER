#include "old_data/PVF/arad/datamanager/arad_reward_functor.h"

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_data/PVF/data/arad_item.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "old_data/db/event/reward_event_request.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/global_data.h"

#include <cstring>

namespace ARAD {

namespace {

int saveRewardEventItem(CUser* user,
                        int event_code,
                        const EventRewardItemArad& reward_data,
                        unsigned long item_index,
                        int16_t item_count,
                        uint32_t expire_time,
                        int source_line) {
    arad::SigSaveRewardEventItem save;
    save.set(event_code,
             reward_data.account_id,
             reward_data.charac_no,
             item_index,
             item_count,
             expire_time);

    CStreamGuard guard(GlobalData::stream_pool()->Acquire(
                           "localjapan/Arad_EventPeriodDataManager.cpp",
                           source_line),
                       true);
    DISPATCHER::make_internal_stream_jpn(
        guard,
        static_cast<ENUM_INTERNALPACKET_JPN>(39),
        user->GetUID());
    guard.put_binary(&save, sizeof(save));
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

void logSystemMailFailure(const char* function_name,
                          int line,
                          const char* message,
                          int event_code,
                          CUser* user,
                          unsigned long item_index,
                          int16_t item_count) {
    LogManager::logFormat(1,
                          "localjapan/Arad_EventPeriodDataManager.cpp",
                          function_name,
                          line,
                          message,
                          event_code,
                          user->get_charac_no(-1),
                          item_index,
                          item_count);
}

}  // namespace

// IDA_EA: 0x819A8C8
// IDA_NAME: _ZN4ARAD17FnRewardInterfaceC2EP5CUserb
// DEMANGLED: ARAD::FnRewardInterface::FnRewardInterface(CUser *,bool)
FnRewardInterface::FnRewardInterface(CUser* user, bool character_scoped)
    : user_(user),
      character_scoped_(character_scoped),
      reserved_09_() {}

// IDA_EA: 0x8194E58
// IDA_NAME: _ZN4ARAD17FnRewardInterface9make_dataEi
// DEMANGLED: ARAD::FnRewardInterface::make_data(int)
EventRewardItemArad FnRewardInterface::make_data(int event_code) {
    return EventRewardItemArad(
        event_code,
        static_cast<uint32_t>(user_->get_acc_id()),
        character_scoped_
            ? static_cast<uint32_t>(user_->get_charac_no(-1))
            : 0u);
}

// IDA_EA: 0x8194AC8
// IDA_NAME: _ZN4ARAD17FnRewardInterfaceclEimsj
// DEMANGLED: ARAD::FnRewardInterface::operator()(int,unsigned long,short,unsigned int)
int FnRewardInterface::operator()(int event_code,
                                  unsigned long item_index,
                                  int16_t item_count,
                                  uint32_t expire_time) {
    if (!user_ || !user_->get_charac_no(-1)) {
        return 0;
    }

    Inven_Item item;
    if (ARAD_ITEM::arad_make_item(item_index, item_count, item) != 1) {
        return 0;
    }

    char text[256] = {};
    char title[21] = {};
    GetMailString(text, title);
    if (WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<uint32_t>(user_->get_charac_no(-1)),
            text,
            static_cast<int>(std::strlen(text)),
            12,
            static_cast<ENUM_SERVER_GROUP>(user_->GetServerGroup()),
            false,
            false)) {
        logSystemMailFailure(
            "bool ARAD::FnRewardInterface::operator()(int, itemindex_t, short int, arad_uint32)",
            0x2f,
            "[FnReward] send mail fail. (code:%u, charac:%u, item:%u, cnt:%d)",
            event_code,
            user_,
            item_index,
            item_count);
        return 0;
    }

    return saveRewardEventItem(user_,
                               event_code,
                               make_data(event_code),
                               item_index,
                               item_count,
                               expire_time,
                               54);
}

// IDA_EA: 0x819A8F2
// IDA_NAME: _ZN4ARAD13FnRewardLetheC2EP5CUser
// DEMANGLED: ARAD::FnRewardLethe::FnRewardLethe(CUser *)
FnRewardLethe::FnRewardLethe(CUser* user)
    : user_(user) {}

// IDA_EA: 0x8195340
// IDA_NAME: _ZN4ARAD13FnRewardLethe9make_dataEi
// DEMANGLED: ARAD::FnRewardLethe::make_data(int)
EventRewardItemArad FnRewardLethe::make_data(int event_code) {
    return EventRewardItemArad(
        event_code,
        static_cast<uint32_t>(user_->get_acc_id()),
        0);
}

// IDA_EA: 0x8194EE2
// IDA_NAME: _ZN4ARAD13FnRewardLetheclEimsj
// DEMANGLED: ARAD::FnRewardLethe::operator()(int,unsigned long,short,unsigned int)
int FnRewardLethe::operator()(int event_code,
                              unsigned long item_index,
                              int16_t item_count,
                              uint32_t expire_time) {
    if (!user_ || !user_->get_charac_no(-1)) {
        return 0;
    }

    Inven_Item item;
    if (ARAD_ITEM::arad_make_item(item_index, item_count, item) != 1) {
        return 0;
    }

    char text[256] = {};
    char title[21] = {};
    const char* title_key = event_code == 96
                                ? "2012_soccer_goldmedal_title"
                                : "lethe_2012_summer_event_title";
    const char* text_key = event_code == 96
                               ? "2012_soccer_goldmedal_mail"
                               : "lethe_2012_summer_event_mail";
    std::strncpy(title, g_scriptStringManager_.findString(4, title_key, 0),
                 0x14u);
    std::strncpy(text, g_scriptStringManager_.findString(4, text_key, 0),
                 0xffu);

    if (WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            title,
            item,
            0,
            static_cast<uint32_t>(user_->get_charac_no(-1)),
            text,
            static_cast<int>(std::strlen(text)),
            12,
            static_cast<ENUM_SERVER_GROUP>(user_->GetServerGroup()),
            false,
            false)) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_EventPeriodDataManager.cpp",
            "bool ARAD::FnRewardLethe::operator()(int, itemindex_t, short int, arad_uint32)",
            0x66,
            "[RewardLethe] send mail fail. (charac:%u, item:%u, cnt:%d)",
            user_->get_charac_no(-1),
            item_index,
            item_count);
        return 0;
    }

    return saveRewardEventItem(user_,
                               event_code,
                               make_data(event_code),
                               item_index,
                               item_count,
                               expire_time,
                               109);
}

// IDA_EA: 0x819A900
// IDA_NAME: _ZN4ARAD14FnRewardMomojiC2EP5CUser
// DEMANGLED: ARAD::FnRewardMomoji::FnRewardMomoji(CUser *)
FnRewardMomoji::FnRewardMomoji(CUser* user)
    : FnRewardInterface(user, true) {}

// IDA_EA: 0x819537C
// IDA_NAME: _ZN4ARAD14FnRewardMomoji13GetMailStringEPcS1_
// DEMANGLED: ARAD::FnRewardMomoji::GetMailString(char *,char *)
char* FnRewardMomoji::GetMailString(char* text, char* title) {
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "momiji_event_title", 0),
                 0x14u);
    return std::strncpy(text,
                        g_scriptStringManager_.findString(
                            4, "momiji_event_mail", 0),
                        0xffu);
}

}  // namespace ARAD
