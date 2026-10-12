#include "old_game/activity/reward/event_base.h"


// IDA_EA: 0x810AF10
// IDA_NAME: _ZN9stADDINFOC2Ev
// DEMANGLED: stADDINFO::stADDINFO(void)
stADDINFO::stADDINFO()
    : word_00(-1),
      word_02(-1) {}

// IDA_EA: 0x810AE2C
// IDA_NAME: _ZN10CEventBaseC2Ev
// DEMANGLED: CEventBase::CEventBase(void)
CEventBase::CEventBase()
    : add_info_(),
      event_flag_(0),
      reserved_09_() {}

// IDA_EA: 0x810AE52
// IDA_NAME: _ZN10CEventBaseD2Ev
// DEMANGLED: CEventBase::~CEventBase()
// IDA deleting destructor: 0x810AE78.
CEventBase::~CEventBase() {}

// IDA_EA: 0x810AF26
// IDA_NAME: _ZN10CEventBase10StartEventE10Word_Param
// DEMANGLED: CEventBase::StartEvent(Word_Param)
void CEventBase::StartEvent(Word_Param param) {
    (void)param;
}

// IDA_EA: 0x80C84C4
// IDA_NAME: _ZN10CEventBase11StartActionEv
// DEMANGLED: CEventBase::StartAction(void)
void CEventBase::StartAction() {}

// IDA_EA: 0x80C84CA
// IDA_NAME: _ZN10CEventBase10AppendInfoER11PacketGuard
// DEMANGLED: CEventBase::AppendInfo(PacketGuard &)
void CEventBase::AppendInfo(PacketGuard& guard) {
    (void)guard;
}

// IDA_EA: 0x80C84D0
// IDA_NAME: _ZN10CEventBase10GetAddInfoEv
// DEMANGLED: CEventBase::GetAddInfo(void)
stADDINFO CEventBase::GetAddInfo() {
    return add_info_;
}

// IDA_EA: 0x80C84E2
// IDA_NAME: _ZN10CEventBase12SetEventFlagEb
// DEMANGLED: CEventBase::SetEventFlag(bool)
CEventBase* CEventBase::SetEventFlag(bool enabled) {
    event_flag_ = static_cast<uint8_t>(enabled);
    return this;
}

// IDA_EA: 0x80C84FA
// IDA_NAME: _ZNK10CEventBase10IsEventingEP5CUser
// DEMANGLED: CEventBase::IsEventing(CUser *)const
int CEventBase::IsEventing(CUser* user) const {
    (void)user;
    return event_flag_;
}

bool CEventBase::IsEventFlagSet() const {
    return event_flag_ != 0u;
}

// IDA_EA: 0x810AE96
// IDA_NAME: _ZN10CEventBase16changeCharacNameEP5CUseriPci
// DEMANGLED: CEventBase::changeCharacName(CUser *,int,char *,int)
void CEventBase::changeCharacName(CUser* user,
                                  int charac_no,
                                  char* name,
                                  int length) {
    (void)user;
    (void)charac_no;
    (void)name;
    (void)length;
}

// IDA_EA: 0x810AE9C
// IDA_NAME: _ZN10CEventBase14dailyresetDataEv
// DEMANGLED: CEventBase::dailyresetData(void)
void CEventBase::dailyresetData() {}

// IDA_EA: 0x810AEA2
// IDA_NAME: _ZN10CEventBase13deliveryStartEP9CDelivery
// DEMANGLED: CEventBase::deliveryStart(CDelivery *)
void CEventBase::deliveryStart(CDelivery* delivery) {
    (void)delivery;
}

// IDA_EA: 0x810AEA8
// IDA_NAME: _ZN10CEventBase11deliveryEndEP9CDelivery
// DEMANGLED: CEventBase::deliveryEnd(CDelivery *)
void CEventBase::deliveryEnd(CDelivery* delivery) {
    (void)delivery;
}

// IDA_EA: 0x810AEAE
// IDA_NAME: _ZN10CEventBase8deliveryER11DeliveryMsg
// DEMANGLED: CEventBase::delivery(DeliveryMsg &)
void CEventBase::delivery(DeliveryMsg& msg) {
    (void)msg;
}
