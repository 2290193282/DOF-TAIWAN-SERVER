#ifndef OLDSRV_OLD_GAME_EVENT_PREMIUM_GOLD_CARD_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_PREMIUM_GOLD_CARD_EVENT_H_

#include "old_game/activity/reward/event_base.h"

// IDA unit: 99_misc_global/CPremiumGoldCard.
// C2 at 0x811C432 only constructs CEventBase and installs vtable 0x8B52088.
class CPremiumGoldCard : public CEventBase {
public:
    // IDA_EA: 0x811C432
    // IDA_NAME: _ZN16CPremiumGoldCardC2Ev
    CPremiumGoldCard();

    // IDA_EA: 0x811C44E (D2), 0x811C47E (D0)
    // IDA_NAME: _ZN16CPremiumGoldCardD2Ev
    // IDA_NAME: _ZN16CPremiumGoldCardD0Ev
    virtual ~CPremiumGoldCard();

    // IDA_EA: 0x811C49C / 0x811C508 / 0x811C4EE
    // IDA_NAME: _ZN16CPremiumGoldCard10StartEventEv
    // IDA_NAME: _ZN16CPremiumGoldCard8EndEventEv
    // IDA_NAME: _ZN16CPremiumGoldCard10StartEventE10Word_Param
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CPremiumGoldCard) == 0x0cu,
              "CPremiumGoldCard IDA size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PREMIUM_GOLD_CARD_EVENT_H_
