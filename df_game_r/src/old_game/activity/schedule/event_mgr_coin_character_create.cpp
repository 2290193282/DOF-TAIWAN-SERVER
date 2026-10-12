#include "old_game/activity/schedule/event_mgr_runtime.h"

#include <cstring>
#include <time.h>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x848EF0C
// IDA_NAME: _ZN8WongWork9CEventMgr11_IsEventingEiiiiiiiiNS0_10EVENT_TYPEE
// DEMANGLED: WongWork::CEventMgr::_IsEventing(int,int,int,int,int,int,int,int,WongWork::CEventMgr::EVENT_TYPE)
int WongWork::CEventMgr::_IsEventing(int start_year,
                                     int start_month,
                                     int start_day,
                                     int start_hour,
                                     int end_year,
                                     int end_month,
                                     int end_day,
                                     int end_hour,
                                     EVENT_TYPE event_type) {
    tm begin;
    tm end;
    std::memset(&begin, 0, sizeof(begin));
    std::memset(&end, 0, sizeof(end));
    begin.tm_year = start_year - 1900;
    begin.tm_mon = start_month - 1;
    begin.tm_mday = start_day;
    begin.tm_hour = start_hour;
    end.tm_year = end_year - 1900;
    end.tm_mon = end_month - 1;
    end.tm_mday = end_day;
    end.tm_hour = end_hour;

    const time_t now = GlobalData::s_systemTime_.getCurSec();
    const time_t begin_time = mktime(&begin);
    const time_t end_time = mktime(&end);
    if (now < begin_time || now >= end_time) {
        return 0;
    }
    if (event_type != EVENT_TYPE_WEEKDAY) {
        return 1;
    }

    tm local;
    if (!localtime_r(&now, &local)) {
        return 0;
    }
    return local.tm_wday != 0 && local.tm_wday != 6;
}


// IDA_EA: 0x848F002
// IDA_NAME: _ZN8WongWork9CEventMgr19GetCoinCharacCreateEv
// DEMANGLED: WongWork::CEventMgr::GetCoinCharacCreate(void)
int WongWork::CEventMgr::GetCoinCharacCreate() {
    const int default_coin = 5;
    CEventBase* const event = GlobalData::s_event_manager->GetRepeatEvent(5);
    if (!event || !event->IsEventing(0)) {
        return default_coin;
    }
    return static_cast<CCoinEventOnCharCreate*>(event)
        ->GetCoinNoOnCreateCharac();
}

// IDA_EA: 0x848EFF8
// IDA_NAME: _ZN8WongWork9CEventMgr13GetCoinPerDayEv
// DEMANGLED: WongWork::CEventMgr::GetCoinPerDay(void)
int WongWork::CEventMgr::GetCoinPerDay() {
    return 1;
}

// IDA_EA: 0x848F06E
// IDA_NAME: _ZN8WongWork9CEventMgr20GetMoneyCharacCreateEv
// DEMANGLED: WongWork::CEventMgr::GetMoneyCharacCreate(void)
int WongWork::CEventMgr::GetMoneyCharacCreate() {
    return 0;
}

// IDA_EA: 0x848F0DC
// IDA_NAME: _ZN8WongWork9CEventMgr16IsPCRoomEventingEv
// DEMANGLED: WongWork::CEventMgr::IsPCRoomEventing(void)
int WongWork::CEventMgr::IsPCRoomEventing() {
    return _IsEventing(2006,
                       1,
                       24,
                       0,
                       2006,
                       2,
                       7,
                       6,
                       EVENT_TYPE_ALL_DAY) != 0;
}

// IDA_EA: 0x848F140
// IDA_NAME: _ZN8WongWork9CEventMgr22IsChildrensDayEventingEv
// DEMANGLED: WongWork::CEventMgr::IsChildrensDayEventing(void)
int WongWork::CEventMgr::IsChildrensDayEventing() {
    return _IsEventing(2006,
                       5,
                       5,
                       6,
                       2006,
                       5,
                       8,
                       6,
                       EVENT_TYPE_ALL_DAY) != 0;
}

// IDA_EA: 0x848F078
// IDA_NAME: _ZN8WongWork9CEventMgr22GetPremiumServiceBonusEv
// DEMANGLED: WongWork::CEventMgr::GetPremiumServiceBonus(void)
int WongWork::CEventMgr::GetPremiumServiceBonus() {
    return _IsEventing(2006,
                       1,
                       2,
                       0,
                       2006,
                       1,
                       11,
                       0,
                       EVENT_TYPE_ALL_DAY)
               ? 7
               : 0;
}
