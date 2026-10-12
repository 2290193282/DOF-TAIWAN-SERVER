#ifndef OLDSRV_OLD_GAME_EVENT_EVENT_MGR_RUNTIME_H_
#define OLDSRV_OLD_GAME_EVENT_EVENT_MGR_RUNTIME_H_

#include <stdint.h>

namespace WongWork {

// IDA unit: 11_event_runtime/WongWork/CEventMgr.  This facade has no object
// storage: the recovered accessors read the process-wide CEventManager.
class CEventMgr {
public:
    enum EVENT_TYPE {
        EVENT_TYPE_ALL_DAY = 0,
        EVENT_TYPE_WEEKDAY = 1,
    };

    // IDA_EA: 0x848EF0C
    // IDA_NAME: _ZN8WongWork9CEventMgr11_IsEventingEiiiiiiiiNS0_10EVENT_TYPEE
    // DEMANGLED: WongWork::CEventMgr::_IsEventing(int,int,int,int,int,int,int,int,WongWork::CEventMgr::EVENT_TYPE)
    int _IsEventing(int start_year,
                    int start_month,
                    int start_day,
                    int start_hour,
                    int end_year,
                    int end_month,
                    int end_day,
                    int end_hour,
                    EVENT_TYPE event_type);

    // IDA_EA: 0x848F002
    // IDA_NAME: _ZN8WongWork9CEventMgr19GetCoinCharacCreateEv
    // DEMANGLED: WongWork::CEventMgr::GetCoinCharacCreate(void)
    int GetCoinCharacCreate();

    // IDA_EA: 0x848EFF8
    // IDA_NAME: _ZN8WongWork9CEventMgr13GetCoinPerDayEv
    // DEMANGLED: WongWork::CEventMgr::GetCoinPerDay(void)
    int GetCoinPerDay();

    // IDA_EA: 0x848F06E
    // IDA_NAME: _ZN8WongWork9CEventMgr20GetMoneyCharacCreateEv
    // DEMANGLED: WongWork::CEventMgr::GetMoneyCharacCreate(void)
    int GetMoneyCharacCreate();

    // IDA_EA: 0x848F0DC
    // IDA_NAME: _ZN8WongWork9CEventMgr16IsPCRoomEventingEv
    // DEMANGLED: WongWork::CEventMgr::IsPCRoomEventing(void)
    int IsPCRoomEventing();

    // IDA_EA: 0x848F140
    // IDA_NAME: _ZN8WongWork9CEventMgr22IsChildrensDayEventingEv
    // DEMANGLED: WongWork::CEventMgr::IsChildrensDayEventing(void)
    int IsChildrensDayEventing();

    // IDA_EA: 0x848F078
    // IDA_NAME: _ZN8WongWork9CEventMgr22GetPremiumServiceBonusEv
    // DEMANGLED: WongWork::CEventMgr::GetPremiumServiceBonus(void)
    int GetPremiumServiceBonus();
};

}  // namespace WongWork


#endif  // OLDSRV_OLD_GAME_EVENT_EVENT_MGR_RUNTIME_H_
