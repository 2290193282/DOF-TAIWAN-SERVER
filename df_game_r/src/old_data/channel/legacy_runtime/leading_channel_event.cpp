#include "old_game/activity/reward/event_base.h"

#include "infra/log/format/trace.h"


// IDA unit: 03_channel_select/CLeadingChannelEvent.
class CLeadingChannelEvent : public CEventBase {
public:
    CLeadingChannelEvent();
    virtual ~CLeadingChannelEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

private:
    int exp_rate_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CLeadingChannelEvent) == 0x10u,
              "CLeadingChannelEvent IDA ABI size must stay 0x10");
#endif

// IDA_EA: 0x811FD64
// IDA_NAME: _ZN20CLeadingChannelEventC2Ev
// DEMANGLED: CLeadingChannelEvent::CLeadingChannelEvent(void)
CLeadingChannelEvent::CLeadingChannelEvent()
    : CEventBase(),
      exp_rate_(100) {}

// IDA_EA: 0x811FD8A
// IDA_NAME: _ZN20CLeadingChannelEventD2Ev
// DEMANGLED: CLeadingChannelEvent::~CLeadingChannelEvent()
// IDA_EA: 0x811FDBA
// IDA_NAME: _ZN20CLeadingChannelEventD0Ev
// DEMANGLED: CLeadingChannelEvent::~CLeadingChannelEvent()
// The virtual C++ destructor emits this adjacent deleting ABI variant.
CLeadingChannelEvent::~CLeadingChannelEvent() {}

// IDA_EA: 0x811FDD8
// IDA_NAME: _ZN20CLeadingChannelEvent10StartEventEv
// DEMANGLED: CLeadingChannelEvent::StartEvent(void)
void CLeadingChannelEvent::StartEvent() {
    SetEventFlag(true);
    exp_rate_ = 100;

    cMyTrace trace("StartEvent", 19, 0);
    trace("Leading Channel Event Start!");
}

// IDA_EA: 0x811FE34
// IDA_NAME: _ZN20CLeadingChannelEvent10StartEventE10Word_Param
// DEMANGLED: CLeadingChannelEvent::StartEvent(Word_Param)
void CLeadingChannelEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    exp_rate_ = static_cast<int>(param);

    cMyTrace trace("StartEvent", 27, 0);
    trace("Leading Channel Event Start!");
}

// IDA_EA: 0x811FE92
// IDA_NAME: _ZN20CLeadingChannelEvent8EndEventEv
// DEMANGLED: CLeadingChannelEvent::EndEvent(void)
void CLeadingChannelEvent::EndEvent() {
    SetEventFlag(false);
    exp_rate_ = 100;

    cMyTrace trace("EndEvent", 35, 0);
    trace("Leading Channel Event End!");
}
