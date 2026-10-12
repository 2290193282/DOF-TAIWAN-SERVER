#include "old_game/social/party/party.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x8116D4A
// IDA_NAME: _ZN19CPartyExpBonusEventC2Ev
// DEMANGLED: CPartyExpBonusEvent::CPartyExpBonusEvent(void)
CPartyExpBonusEvent::CPartyExpBonusEvent()
    : CEventBase(),
      bonus_rate_(0) {}

// IDA_EA: 0x8116D70
// IDA_NAME: _ZN19CPartyExpBonusEventD2Ev
// DEMANGLED: CPartyExpBonusEvent::~CPartyExpBonusEvent()
// IDA_EA: 0x8116DA0
// IDA_NAME: _ZN19CPartyExpBonusEventD0Ev
// DEMANGLED: CPartyExpBonusEvent::~CPartyExpBonusEvent() deleting destructor
CPartyExpBonusEvent::~CPartyExpBonusEvent() {}

// IDA_EA: 0x8116C70
// IDA_NAME: _ZN19CPartyExpBonusEvent10StartEventEv
// DEMANGLED: CPartyExpBonusEvent::StartEvent(void)
void CPartyExpBonusEvent::StartEvent() {
    SetEventFlag(true);
    const int bonus_rate = bonus_rate_;
    cMyTrace trace("virtual void CPartyExpBonusEvent::StartEvent()", 16, 0);
    trace("Party Exp Bonus Event Start! (%x%%)", bonus_rate);
}

// IDA_EA: 0x8116CD0
// IDA_NAME: _ZN19CPartyExpBonusEvent10StartEventE10Word_Param
// DEMANGLED: CPartyExpBonusEvent::StartEvent(Word_Param)
void CPartyExpBonusEvent::StartEvent(Word_Param param) {
    bonus_rate_ = static_cast<int>(static_cast<uint16_t>(param));
    StartEvent();
}

// IDA_EA: 0x8116CF8
// IDA_NAME: _ZN19CPartyExpBonusEvent8EndEventEv
// DEMANGLED: CPartyExpBonusEvent::EndEvent(void)
void CPartyExpBonusEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CPartyExpBonusEvent::EndEvent()", 27, 0);
    trace("Party Exp Bonus Event End!");
}

// IDA_EA: 0x85BFE1A
// IDA_NAME: _ZN19CPartyExpBonusEvent12getBonusRateEib
// DEMANGLED: CPartyExpBonusEvent::getBonusRate(int,bool)
long double CPartyExpBonusEvent::getBonusRate(int member_count,
                                              bool abuse_party) {
    if (IsEventing(0) && member_count > 1 && !abuse_party) {
        return static_cast<long double>(static_cast<unsigned int>(bonus_rate_)) /
               100.0L;
    }
    return 0.0L;
}
