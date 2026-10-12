#ifndef OLDSRV_OLD_GAME_EVENT_REDUCE_UPGRADE_ITEM_PAY_H_
#define OLDSRV_OLD_GAME_EVENT_REDUCE_UPGRADE_ITEM_PAY_H_

#include "old_game/activity/reward/event_base.h"

// IDA owner boundary: 99_misc_global/CReduceUpgradeItemPay.
// IDA_EA: ctor/dtor/start/end/rate @0x811715C/@0x8117178/@0x81171A8/
// @0x81171C6/@0x81171E2/@0x8117270/@0x854B56E.
class CReduceUpgradeItemPay : public CEventBase {
public:
    CReduceUpgradeItemPay();
    virtual ~CReduceUpgradeItemPay();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetReduceRate();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CReduceUpgradeItemPay) == 0x0cu,
              "CReduceUpgradeItemPay IDA ABI size must stay 0x0C");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_REDUCE_UPGRADE_ITEM_PAY_H_
