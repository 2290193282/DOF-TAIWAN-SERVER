#ifndef OLDSRV_OLD_GAME_EVENT_CERASHOP_REWARD_POINT_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_CERASHOP_REWARD_POINT_EVENT_H_

#include <stdint.h>

#include "old_game/activity/reward/event_base.h"

class CUser;

// Taiwan repeat event 161.  The two scalar fields occupy the exact words
// immediately after CEventBase and are consumed by the mobile-auth reply.
class CEventCeraShopRewardPoint : public CEventBase {
public:
    // IDA_EA: 0x816459A
    // IDA_NAME: _ZN25CEventCeraShopRewardPointC2Ev
    CEventCeraShopRewardPoint();

    // IDA_EA: 0x81645CA (D2), 0x81645FA (D0)
    // IDA_NAME: _ZN25CEventCeraShopRewardPointD2Ev / D0Ev
    virtual ~CEventCeraShopRewardPoint();

    // IDA_EA: 0x8164618
    // IDA_NAME: _ZN25CEventCeraShopRewardPoint10StartEventEv
    virtual void StartEvent();

    // IDA_EA: 0x8164684
    // IDA_NAME: _ZN25CEventCeraShopRewardPoint10StartEventE10Word_Param
    virtual void StartEvent(Word_Param param);

    // IDA_EA: 0x81646B8
    // IDA_NAME: _ZN25CEventCeraShopRewardPoint8EndEventEv
    virtual void EndEvent();

    // IDA_EA: 0x816471E
    // IDA_NAME: _ZN25CEventCeraShopRewardPoint10SendRewardEP5CUser
    void SendReward(CUser* user);

private:
    uint32_t cera_point_count_;  // +0x0c
    uint32_t timing_;            // +0x10
};

#if defined(__i386__)
static_assert(sizeof(CEventCeraShopRewardPoint) == 0x14u,
              "CEventCeraShopRewardPoint must retain the event ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_CERASHOP_REWARD_POINT_EVENT_H_
