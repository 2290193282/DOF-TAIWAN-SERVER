#include "old_game/activity/schedule/bingo_runtime.h"

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/event/bingo_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/system_time.h"

#include <cstring>

namespace {

const int kOneMatchLine = 1;
const int kMaxMatchLine = 12;

}  // namespace

// IDA_EA: 0x80C9984
// IDA_NAME: _ZN10BingoEventC2Ev
// DEMANGLED: BingoEvent::BingoEvent(void)
BingoEvent::BingoEvent() : CEventBase(), bingo_(new CBingo()) {}

// IDA_EA: 0x80C9A06
// IDA_NAME: _ZN10BingoEventD2Ev
// DEMANGLED: BingoEvent::~BingoEvent()
// IDA_EA: 0x80C9A68
// IDA_NAME: _ZN10BingoEventD0Ev
// DEMANGLED: BingoEvent::~BingoEvent() deleting destructor
BingoEvent::~BingoEvent() {}

// IDA_EA: 0x80C9A86
// IDA_NAME: _ZN10BingoEvent10StartEventEv
// DEMANGLED: BingoEvent::StartEvent(void)
void BingoEvent::StartEvent() {
    SetEventFlag(true);
    const int reset_hour = G_CEnvironment()->get_fatigue_time();
    const std::time_t current_time =
        static_cast<std::time_t>(GlobalData::system_time()->getCurSec());
    const int today = bingo_->getToday(current_time, reset_hour);
    if (today > 0 && today <= 31) {
        bingo_->resetBoard(today);
        bingo_->resetQuiz(today);
        cMyTrace trace("virtual void BingoEvent::StartEvent()", 42, 0);
        trace("BingoEvent Day(%d)", today);
    }
}

// IDA_EA: 0x80C9B68
// IDA_NAME: _ZN10BingoEvent10StartEventE10Word_Param
// DEMANGLED: BingoEvent::StartEvent(Word_Param)
void BingoEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
    cMyTrace trace("virtual void BingoEvent::StartEvent(Word_Param)",
                   48,
                   0);
    trace("BingoEvent Event Start!");
}

// IDA_EA: 0x80C9D30
// IDA_NAME: _ZN10BingoEvent8EndEventEv
// DEMANGLED: BingoEvent::EndEvent(void)
void BingoEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void BingoEvent::EndEvent()", 63, 0);
    trace("BingoEvent Event End!");
}

// IDA_EA: 0x80C9FEA
// IDA_NAME: _ZN10BingoEvent14getBingoSystemEv
// DEMANGLED: BingoEvent::getBingoSystem(void)
boost::shared_ptr<CBingo>& BingoEvent::getBingoSystem() {
    return bingo_;
}

// IDA_EA: 0x80CA262
// IDA_NAME: _ZN10BingoEvent16reqBingoQuizMarkER5CUseri
// DEMANGLED: BingoEvent::reqBingoQuizMark(CUser &,int)
int BingoEvent::reqBingoQuizMark(CUser& user, int quiz_value) {
    if (bingo_->getQuizValue() != quiz_value) {
        return 23;
    }
    const int quiz_index = bingo_->getQuizIndex();
    BingoData* const data = user.getBingoData();
    if (quiz_index < 0 ||
        (data->bingo_count & (1u << quiz_index)) != 0) {
        return 18;
    }

    const unsigned int old_board = data->bingo_count;
    const int line_count = bingo_->calNewValue(data->bingo_count, 0);
    if (data->bingo_count != old_board) {
        data->reward_state = 1;
        int added_lines = line_count - data->line_count;
        if (added_lines > 0) {
            if (added_lines > 3) {
                added_lines = 3;
            }
            data->line_count = static_cast<uint8_t>(line_count);
            for (int count = 0; count < added_lines; ++count) {
                sendOneMatchLineReward(user);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x80CA3D4
// IDA_NAME: _ZN10BingoEvent22sendOneMatchLineRewardER5CUser
// DEMANGLED: BingoEvent::sendOneMatchLineReward(CUser &)
int BingoEvent::sendOneMatchLineReward(CUser& user) {
    stBingoScript* const script = G_CDataManager()->get_bingo_script();
    std::map<int, std::pair<int, int> >::const_iterator found =
        script->rewards.find(kOneMatchLine);
    if (found == script->rewards.end()) {
        LogManager::logFormat(
            1,
            "BingoEvent.cpp",
            "void BingoEvent::sendOneMatchLineReward(CUser&)",
            192,
            "bingo script error");
        return 0;
    }

    const int reward_type = found->second.first;
    const int reward_count = found->second.second;
    Inven_Item item;
    unsigned int money = 0;
    int make_seed = kOneMatchLine;
    if (reward_type == 0) {
        money = static_cast<unsigned int>(reward_count);
    } else if (reward_type == 1) {
        item.item_id_ = 1;
        item.item_category_ = 2;
        item.set_add_info(reward_count);
        item.avatar_select_ = 1;
        item.ResetItemAttr();
    } else {
        CItem* const prototype = G_CDataManager()->find_item(reward_type);
        if (!prototype) {
            return 0;
        }
        item.item_id_ = reward_type;
        typedef void (*MakeInvenItem)(CItem*, Inven_Item*, int*);
        void** const vtable = *reinterpret_cast<void***>(prototype);
        reinterpret_cast<MakeInvenItem>(vtable[2])(
            prototype, &item, &make_seed);
        item.set_add_info(reward_count);
    }

    char title[21] = {};
    const char* const title_text = g_scriptStringManager_.findString(
        4, "game_server_msg_292", 0);
    std::strncpy(title, title_text, 20u);
    char body[256] = {};
    const char* const body_text = g_scriptStringManager_.findString(
        4, "game_server_msg_293", 0);
    std::strncpy(body, body_text, 255u);

    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        money,
        static_cast<unsigned int>(user.getCurCharacNo()),
        body,
        static_cast<int>(std::strlen(body)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user.GetServerGroup()),
        false,
        false);
}

// IDA_EA: 0x80CA622
// IDA_NAME: _ZN10BingoEvent22sendMaxMatchLineRewardER5CUser
// DEMANGLED: BingoEvent::sendMaxMatchLineReward(CUser &)
int BingoEvent::sendMaxMatchLineReward(CUser& user) {
    stBingoScript* const script = G_CDataManager()->get_bingo_script();
    std::map<int, std::pair<int, int> >::const_iterator found =
        script->rewards.find(kMaxMatchLine);
    if (found == script->rewards.end()) {
        LogManager::logFormat(
            1,
            "BingoEvent.cpp",
            "void BingoEvent::sendMaxMatchLineReward(CUser&)",
            239,
            "bingo script error");
        return 0;
    }

    const int reward_type = found->second.first;
    const int reward_count = found->second.second;
    Inven_Item item;
    CItem* const prototype = G_CDataManager()->find_item(reward_type);
    if (!prototype) {
        return 0;
    }
    item.item_id_ = reward_type;
    int make_seed = kMaxMatchLine;
    typedef void (*MakeInvenItem)(CItem*, Inven_Item*, int*);
    void** const vtable = *reinterpret_cast<void***>(prototype);
    reinterpret_cast<MakeInvenItem>(vtable[2])(
        prototype, &item, &make_seed);
    item.set_add_info(reward_count);

    char title[21] = {};
    const char* const title_text = g_scriptStringManager_.findString(
        4, "game_server_msg_292", 0);
    std::strncpy(title, title_text, 20u);
    char body[256] = {};
    const char* const body_text = g_scriptStringManager_.findString(
        4, "game_server_msg_294", 0);
    std::strncpy(body, body_text, 255u);

    return WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        static_cast<unsigned int>(user.getCurCharacNo()),
        body,
        static_cast<int>(std::strlen(body)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user.GetServerGroup()),
        false,
        false);
}
