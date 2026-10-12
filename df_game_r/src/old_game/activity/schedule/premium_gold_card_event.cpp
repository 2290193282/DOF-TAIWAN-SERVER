#include "old_game/activity/schedule/premium_gold_card_event.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x811C432
// IDA_NAME: _ZN16CPremiumGoldCardC2Ev
// DEMANGLED: CPremiumGoldCard::CPremiumGoldCard(void)
CPremiumGoldCard::CPremiumGoldCard() : CEventBase() {}

// IDA_EA: 0x811C44E
// IDA_NAME: _ZN16CPremiumGoldCardD2Ev
// DEMANGLED: CPremiumGoldCard::~CPremiumGoldCard(void)
// IDA_EA: 0x811C47E
// IDA_NAME: _ZN16CPremiumGoldCardD0Ev
// DEMANGLED: CPremiumGoldCard::~CPremiumGoldCard(void)
CPremiumGoldCard::~CPremiumGoldCard() {}

// IDA_EA: 0x811C49C
// IDA_NAME: _ZN16CPremiumGoldCard10StartEventEv
// DEMANGLED: CPremiumGoldCard::StartEvent(void)
void CPremiumGoldCard::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CPremiumGoldCard::StartEvent()", 16, 0);
    trace("CPremiumGoldCard Event Start!");
}

// IDA_EA: 0x811C4EE
// IDA_NAME: _ZN16CPremiumGoldCard10StartEventE10Word_Param
// DEMANGLED: CPremiumGoldCard::StartEvent(Word_Param)
void CPremiumGoldCard::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x811C508
// IDA_NAME: _ZN16CPremiumGoldCard8EndEventEv
// DEMANGLED: CPremiumGoldCard::EndEvent(void)
void CPremiumGoldCard::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CPremiumGoldCard::EndEvent()", 26, 0);
    trace("CPremiumGoldCard Event End!");
}
