#include "old_game/activity/schedule/new_account_level_up_event_to_job_abi.h"

#include "infra/log/format/trace.h"

// IDA_EA: 0x81B8170
// IDA_NAME: _ZN27NewAccountLevelUpEventToJobC2Ev
// DEMANGLED: NewAccountLevelUpEventToJob::NewAccountLevelUpEventToJob(void)
NewAccountLevelUpEventToJob::NewAccountLevelUpEventToJob()
    : CEventBase() {}

// IDA_EA: 0x81B818C
// IDA_NAME: _ZN27NewAccountLevelUpEventToJobD2Ev
// DEMANGLED: NewAccountLevelUpEventToJob::~NewAccountLevelUpEventToJob()
// IDA_EA: 0x81B81BC
// IDA_NAME: _ZN27NewAccountLevelUpEventToJobD0Ev
// DEMANGLED: NewAccountLevelUpEventToJob::~NewAccountLevelUpEventToJob()
NewAccountLevelUpEventToJob::~NewAccountLevelUpEventToJob() {}

// IDA_EA: 0x81B81DA
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob10StartEventEv
// DEMANGLED: NewAccountLevelUpEventToJob::StartEvent(void)
void NewAccountLevelUpEventToJob::StartEvent() {
    SetEventFlag(true);
}

// IDA_EA: 0x81B81F6
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob10StartEventE10Word_Param
// DEMANGLED: NewAccountLevelUpEventToJob::StartEvent(Word_Param)
void NewAccountLevelUpEventToJob::StartEvent(Word_Param param) {
    // 0x81B81F6 dispatches the no-argument virtual and never reads param.
    (void)param;
    StartEvent();
    cMyTrace trace(
        "virtual void NewAccountLevelUpEventToJob::StartEvent(Word_Param)",
        24,
        0);
    trace("NewAccountLevelUpEvent Event Start!");
}

// IDA_EA: 0x81B8246
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob11StartActionEv
// DEMANGLED: NewAccountLevelUpEventToJob::StartAction(void)
// The original body is exactly a five-byte `ret`; retaining that proven
// no-op is required for virtual slot 5, rather than an absent or synthetic
// implementation.
void NewAccountLevelUpEventToJob::StartAction() {}

// IDA_EA: 0x81B824C
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob8EndEventEv
// DEMANGLED: NewAccountLevelUpEventToJob::EndEvent(void)
void NewAccountLevelUpEventToJob::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void NewAccountLevelUpEventToJob::EndEvent()",
                   43,
                   0);
    trace("NewAccountLevelUpEvent Event End!");
}

// IDA_EA: 0x81B829E
// IDA_NAME: _ZN27NewAccountLevelUpEventToJob30sendNewAccountLevelUpToJobDataER5CUser
// DEMANGLED:
// NewAccountLevelUpEventToJob::sendNewAccountLevelUpToJobData(CUser &)
// The original body is exactly a five-byte `ret` and does not inspect user.
void NewAccountLevelUpEventToJob::sendNewAccountLevelUpToJobData(CUser& user) {
    (void)user;
}

// IDA_EA: 0x81B8816
// IDA_NAME: _ZNK27NewAccountLevelUpEventToJob10IsEventingEP5CUser
// DEMANGLED: NewAccountLevelUpEventToJob::IsEventing(CUser *)const
int NewAccountLevelUpEventToJob::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user);
}
