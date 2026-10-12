#ifndef OLDSRV_OLD_GAME_EVENT_NEW_ACCOUNT_LEVEL_UP_EVENT_TO_JOB_ABI_H_
#define OLDSRV_OLD_GAME_EVENT_NEW_ACCOUNT_LEVEL_UP_EVENT_TO_JOB_ABI_H_

#include "old_game/activity/reward/event_base.h"

class CUser;

// IDA unit: 02_login_account/NewAccountLevelUpEventToJob.
// C2 @0x81B8170 allocates no derived storage: the object is exactly the
// 0x0c CEventBase subobject with the derived vtable at 0x8BBC948.
class NewAccountLevelUpEventToJob : public CEventBase {
public:
    // IDA_EA: 0x81B8170
    // IDA_NAME: _ZN27NewAccountLevelUpEventToJobC2Ev
    // DEMANGLED: NewAccountLevelUpEventToJob::NewAccountLevelUpEventToJob(void)
    NewAccountLevelUpEventToJob();

    // IDA_EA: 0x81B818C
    // IDA_NAME: _ZN27NewAccountLevelUpEventToJobD2Ev
    // DEMANGLED: NewAccountLevelUpEventToJob::~NewAccountLevelUpEventToJob()
    // IDA_EA: 0x81B81BC
    // IDA_NAME: _ZN27NewAccountLevelUpEventToJobD0Ev
    // DEMANGLED: NewAccountLevelUpEventToJob::~NewAccountLevelUpEventToJob()
    virtual ~NewAccountLevelUpEventToJob();

    // vtable slots 2..5 at 0x8BBC948.
    // IDA_EA: 0x81B81DA / 0x81B824C / 0x81B81F6 / 0x81B8246
    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();

    // IDA_EA: 0x81B8816, vtable slot 13 at 0x8BBC948.
    // IDA_NAME: _ZNK27NewAccountLevelUpEventToJob10IsEventingEP5CUser
    virtual int IsEventing(CUser* user) const;

    // IDA_EA: 0x81B829E
    // IDA_NAME: _ZN27NewAccountLevelUpEventToJob30sendNewAccountLevelUpToJobDataER5CUser
    // DEMANGLED:
    // NewAccountLevelUpEventToJob::sendNewAccountLevelUpToJobData(CUser &)
    void sendNewAccountLevelUpToJobData(CUser& user);

    // IDA_EA: 0x81B82A4
    // IDA_NAME: _ZN27NewAccountLevelUpEventToJob19processLevelUpToJobER5CUserb
    // DEMANGLED: NewAccountLevelUpEventToJob::processLevelUpToJob(CUser &,bool)
    bool processLevelUpToJob(CUser& user, bool growup);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(NewAccountLevelUpEventToJob) == 0x0cu,
              "NewAccountLevelUpEventToJob IDA size must stay 0x0c");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_NEW_ACCOUNT_LEVEL_UP_EVENT_TO_JOB_ABI_H_
